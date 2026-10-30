"""Check that a header change leaves every linked unit's object unchanged.

Usage:
  header_deps.py                 compare the working tree's modified headers against HEAD's
  header_deps.py --shadow DIR    compare a shadow include tree (searched first) against the tree
  options: --jobs N, --units U... (only these units), --all (not only linked units)

For every unit in config/RHAE01/matching.txt whose ninja deps include a changed header, the unit
is compiled twice into a scratch directory (old and new headers; tools/cccmd.py, lock-free) and
the objects are compared: section bytes, plus relocations by (section, offset, type, symbol,
addend) with compiler-generated label numbers (@1234, name$1234) normalized, since those shift
harmlessly when a header declares more. Exit status 1 if any dependent changed.
"""
import argparse
import re
import shutil
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import cccmd  # noqa: E402
import objcmp  # noqa: E402
from elf_symbols import read  # noqa: E402

ROOT = cccmd.ROOT
SKIP = {".strtab", ".symtab", ".comment", ".shstrtab", ""}


def all_deps():
    """object path -> set of repo-relative headers, from ninja's dependency log"""
    r = subprocess.run(["ninja", "-t", "deps"], cwd=ROOT, capture_output=True, text=True)
    deps, cur = {}, None
    for l in r.stdout.splitlines():
        if l and not l.startswith(" "):
            cur = l.split(":")[0].strip()
            deps[cur] = set()
        elif cur and l.strip():
            h = re.sub(r".*?/(?=include/)", "", l.strip())
            if h.startswith("include/"):
                deps[cur].add(h)
    return deps


def fingerprint(path):
    secs = {k: v for k, v in objcmp.sections(path).items() if k not in SKIP and not k.startswith(".rela")}
    norm = lambda n: re.sub(r"\$\d+$", "$N", re.sub(r"^@\d+", "@N", n or ""))  # noqa: E731
    relocs = sorted((r["section"], r["offset"], r["type"], norm(r["symbol"]), r["addend"]) for r in read(path)[1])
    return secs, relocs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--shadow")
    ap.add_argument("--jobs", type=int, default=8)
    ap.add_argument("--units", nargs="*")
    ap.add_argument("--all", action="store_true")
    a = ap.parse_args()
    tmp = Path(tempfile.mkdtemp(prefix="hdeps_"))
    try:
        if a.shadow:
            new_dir, old_dir = Path(a.shadow), None
            changed = {"include/" + p.relative_to(new_dir).as_posix() for p in new_dir.rglob("*.h")}
        else:
            st = subprocess.run(["git", "status", "--porcelain", "include"], cwd=ROOT, capture_output=True, text=True).stdout
            changed = {l[3:] for l in st.splitlines() if l[:2].strip() == "M"}
            old_dir, new_dir = tmp / "head", None
            for h in changed:
                dst = old_dir / Path(h).relative_to("include")
                dst.parent.mkdir(parents=True, exist_ok=True)
                dst.write_bytes(subprocess.run(["git", "show", f"HEAD:{h}"], cwd=ROOT, capture_output=True).stdout)
        if not changed:
            print("no changed headers")
            return 0
        print("changed headers: " + ", ".join(sorted(changed)))
        linked = [l.strip() for l in (ROOT / "config/RHAE01/matching.txt").read_text().splitlines() if l.strip()]
        deps = all_deps()
        pool = a.units or ([u for u in linked] if not a.all else
                           [k.removeprefix("build/RHAE01/src/").rsplit(".", 1)[0] for k in deps])
        units = []
        for u in pool:
            obj = "build/RHAE01/src/" + Path(u).with_suffix(".o").as_posix()
            if deps.get(obj, set()) & changed:
                units.append(u)
        print(f"{len(units)} linked unit(s) include them")

        def run(i_u):
            i, u = i_u
            try:
                o1 = cccmd.compile_variant(u, outdir=tmp / f"{i}a", include_first=old_dir)
                o2 = cccmd.compile_variant(u, outdir=tmp / f"{i}b", include_first=new_dir)
            except RuntimeError as e:  # no build rule (configure.py mid-update): report, don't crash
                return u, f"skipped: {e}"
            if not o1 or not o2:
                return u, "does not compile with the " + ("old" if not o1 else "new") + " headers"
            (s1, r1), (s2, r2) = fingerprint(o1), fingerprint(o2)
            diff = sorted(k for k in set(s1) | set(s2) if s1.get(k) != s2.get(k))
            if r1 != r2:
                diff.append("relocations")
            return u, ("differs in " + ", ".join(diff)) if diff else None

        with ThreadPoolExecutor(a.jobs) as ex:
            res = [r for r in ex.map(run, enumerate(units)) if r[1]]
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    for u, why in res:
        print(f"  CHANGED {u}: {why}")
    print(f"{len(units) - len(res)} of {len(units)} dependents identical")
    return 1 if res else 0


if __name__ == "__main__":
    sys.exit(main())
