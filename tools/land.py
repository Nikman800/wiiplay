"""Land linked units: verify the DOL, check the commit is complete and scoped, commit, push.

Usage: land.py -m "message" <unit>... [--header <path>...] [--file <path>...] [--no-push] [--dry-run]

  <unit>        a unit linked from source (e.g. MSL/strtoul.c); its src file is staged
  --header      a header changed for these units (staged; dependents are checked below)
  --file        any other file to include (e.g. a game caller changed with a rename)

Refuses to commit when:
  - main.dol does not rebuild cleanly or does not match (ninja failure is not hidden by a stale DOL);
  - matching.txt adds a unit not named here (an agent linked another unit: land it separately),
    or a named unit is not in matching.txt;
  - a named unit compiles against an uncommitted header that is not passed with --header.
The ledger files that changed (splits.txt, symbols.txt, config.yml, configure.py, dtk/*.tsv and the
units' dtk/source_names/*.tsv) are staged too, and listed so the scope can be reviewed.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import unit as U  # noqa: E402

LEDGER = ["config/RHAE01/splits.txt", "config/RHAE01/symbols.txt", "config/RHAE01/config.yml",
          "configure.py", "dtk/data_owners.tsv", "dtk/name_corrections.tsv"]
TRAILER = "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"


def git(*a, check=True):
    r = subprocess.run(["git", *a], cwd=ROOT, capture_output=True, text=True)
    if check and r.returncode:
        sys.exit(f"git {' '.join(a)} failed:\n{r.stderr}")
    return r.stdout


def verify():
    with U.build_lock("land verify"):
        subprocess.run([U.PY, "configure.py"], cwd=ROOT, capture_output=True)
        r = subprocess.run([U.ninja_exe(), "build/RHAE01/main.dol"], cwd=ROOT, capture_output=True, text=True)
        h = subprocess.run([str(ROOT / "build/tools/dtk"), "shasum", "-c", "config/RHAE01/build.sha1"],
                           cwd=ROOT, capture_output=True, text=True)
    out = re.sub(r"\x1b\[[0-9;]*m", "", h.stdout + h.stderr)
    if r.returncode:
        print(r.stdout[-1500:], r.stderr[-500:])
        return "ninja FAILED: main.dol was not rebuilt"
    if "main.dol: OK" not in out:
        return "main.dol does NOT match:\n" + out
    return None


def header_deps(unit):
    """Headers the unit's object depends on (ninja -t deps), repo-relative."""
    base, _ = U.paths(unit)
    r = subprocess.run([U.ninja_exe(), "-t", "deps", base.relative_to(ROOT).as_posix()], cwd=ROOT,
                       capture_output=True, text=True)
    return {re.sub(r".*?/(?=include/)", "", l.strip()) for l in r.stdout.splitlines()[1:] if "include/" in l}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("units", nargs="+")
    ap.add_argument("-m", "--message", required=True)
    ap.add_argument("--header", action="append", default=[])
    ap.add_argument("--file", action="append", default=[])
    ap.add_argument("--no-push", action="store_true")
    ap.add_argument("--dry-run", action="store_true", help="run every check, print what would be staged")
    a = ap.parse_args()

    # 1. matching.txt: exactly the named units are newly linked (or already committed)
    added = [l[1:].strip() for l in git("diff", "config/RHAE01/matching.txt").splitlines()
             if l.startswith("+") and not l.startswith("+++")]
    committed = set(git("show", "HEAD:config/RHAE01/matching.txt").split())
    extra = [u for u in added if u not in a.units]
    absent = [u for u in a.units if u not in added and u not in committed]
    if extra:
        sys.exit(f"matching.txt also adds {extra}: land those separately (or name them here)")
    if absent:
        sys.exit(f"{absent} not in matching.txt: link them first (unit.py check)")

    # 2. headers: every uncommitted header a unit compiles against must be in --header
    # untracked ones too: a clean clone fails without them (nine headers were missed that way)
    modified = {l[3:] for l in git("status", "--porcelain", "--untracked-files=all", "include").splitlines()
                if l[:2].strip() in ("M", "A", "??")}
    for u in a.units:
        stray = sorted((header_deps(u) & modified) - set(a.header))
        if stray:
            sys.exit(f"{u} compiles against uncommitted headers not passed with --header: {stray}")

    # 3. the DOL matches with the tree as it stands
    err = verify()
    if err:
        sys.exit(err)

    # 4. stage by name
    files = [f"src/{u}" for u in a.units] + a.header + a.file + ["config/RHAE01/matching.txt"]
    files += [f for f in LEDGER if git("status", "--porcelain", f).strip()]
    for u in a.units:
        tsv = Path("dtk/source_names") / (u.replace("/", "__") + ".tsv")
        if (ROOT / tsv).exists() and git("status", "--porcelain", str(tsv)).strip():
            files.append(tsv.as_posix())
    if a.dry_run:
        print("checks passed; would stage:\n  " + "\n  ".join(files))
        return 0
    git("add", "--", *files)
    print(git("diff", "--cached", "--stat"))
    msg = a.message.rstrip() + "\n\n" + TRAILER + "\n"
    r = subprocess.run(["git", "commit", "-q", "-F", "-"], cwd=ROOT, input=msg, text=True, capture_output=True)
    if r.returncode:
        sys.exit(f"commit failed:\n{r.stdout}{r.stderr}")
    print(git("log", "--oneline", "-1").strip())
    if not a.no_push:
        git("push", "-q", "origin", "HEAD")
        print("pushed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
