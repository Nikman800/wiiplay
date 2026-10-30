"""Score several rewrites of one function at once (lock-free, in parallel).

Usage: try_variants.py <unit> <name> <variants file> [--symbol MANGLED]... [--keep N]
                       [--src FILE] [--include-first DIR] [--jobs J]

  try_variants.py egg/math/eggQuat.cpp Quatf::conjugate /tmp/conj.cpp --symbol conjugate__Q23EGG5QuatfCFv

The variants file holds 2 to ~20 rewrites separated by lines of `=====`. Each replaces the
definition of <name> (as written in the source: `conjugate`, `Quatf::conjugate`) in src/<unit>
(or --src); a variant may start with extra definitions (a static inline helper, say), which go
where the definition was. Every variant is compiled with the unit's exact command (tools/cccmd.py)
and compared with the original object (tools/objcmp.py) on --symbol (default: every function of
the unit), and a table is printed: differing words, matched/total, and the unit-wide score.
Variant 0 is the current source, for reference. Nothing in src/ changes unless --keep N writes
variant N there (check it is natural C first: tools/MATCHING.md, "Natural C").
Write one idea per variant, so the score says which idea mattered. Idea from pokebw2's
try_variants.py; written here for CodeWarrior/PowerPC, no code taken.
"""
import argparse
import re
import shutil
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import cccmd  # noqa: E402
import objcmp  # noqa: E402

ROOT = cccmd.ROOT


def definition_span(text, name):
    """start and end of the definition of name (qualified or not): signature through its closing brace"""
    pat = re.compile(r"^[^\n;{}#]*?(?<![\w:])" + re.escape(name) + r"\s*\([^;{]*\)[^;{]*\{", re.M)
    hits = list(pat.finditer(text))
    if not hits:
        sys.exit(f"no definition of {name} in the source")
    if len(hits) > 1:
        sys.exit(f"{len(hits)} definitions match {name}: qualify it (Class::{name.split('::')[-1]})")
    i, depth = hits[0].end(), 1
    while depth:
        c = text[i]
        if c in "\"'":  # skip literals, which may hold braces
            j = i + 1
            while text[j] != c:
                j += 2 if text[j] == "\\" else 1
            i = j
        elif text.startswith("//", i):
            i = text.index("\n", i)
        elif text.startswith("/*", i):
            i = text.index("*/", i) + 1
        else:
            depth += {"{": 1, "}": -1}.get(c, 0)
        i += 1
    return hits[0].start(), i


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("unit")
    ap.add_argument("name")
    ap.add_argument("variants")
    ap.add_argument("--symbol", action="append", default=[])
    ap.add_argument("--keep", type=int)
    ap.add_argument("--src")
    ap.add_argument("--include-first")
    ap.add_argument("--jobs", type=int, default=8)
    a = ap.parse_args()
    src_path = Path(a.src) if a.src else ROOT / "src" / a.unit
    text = src_path.read_text()
    lo, hi = definition_span(text, a.name)
    variants = [text[lo:hi]] + [v.strip("\n") for v in
                                re.split(r"^=====+\s*$", Path(a.variants).read_text(), flags=re.M)
                                if v.strip()]
    target_path = ROOT / "build/RHAE01/obj" / Path(a.unit).with_suffix(".o")
    if not target_path.exists():
        sys.exit(f"no original object {target_path.relative_to(ROOT)} (a linked unit has none; "
                 "use the asm or a saved copy)")
    target = objcmp.Obj(str(target_path))
    names = a.symbol or list(target.funcs)
    missing = [n for n in names if n not in target.funcs]
    if missing:
        sys.exit("not in the original object: " + ", ".join(missing))
    tmp = Path(tempfile.mkdtemp(prefix="tryvar_"))
    cccmd.base_command(a.unit)  # fail early if the build has no command for the unit

    def run(i):
        f = tmp / f"v{i}" / src_path.name
        f.parent.mkdir()
        f.write_text(text[:lo] + variants[i] + text[hi:])
        obj = cccmd.compile_variant(a.unit, src=str(f), outdir=tmp / f"v{i}",
                                    include_first=a.include_first)
        if not obj:
            return i, None
        s = objcmp.Obj(str(obj))
        rmap = objcmp.reloc_map(target, s)
        eq = tot = 0
        for n in names:
            e, t = objcmp.compare(target, s, n, rmap)
            eq, tot = eq + e, tot + t
        return i, (tot - eq, eq, tot, objcmp.score(target, s))

    try:
        with ThreadPoolExecutor(a.jobs) as ex:
            res = dict(ex.map(run, range(len(variants))))
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    best = min((r[0], i) for i, r in res.items() if r) if any(res.values()) else None
    print(f"{'#':>3} {'diff':>5} {'words':>11} {'unit':>7}  first line")
    for i in range(len(variants)):
        r = res[i]
        first = next((l.strip() for l in variants[i].splitlines() if l.strip()), "")[:60]
        mark = " <- best" if best and best[1] == i else ""
        if r is None:
            print(f"{i:>3}  compile failed  {first}")
        else:
            print(f"{i:>3} {r[0]:>5} {r[1]:>5}/{r[2]:<5} {r[3]:7.4f}  {first}{mark}")
    if a.keep is not None:
        if a.keep == 0 or not res.get(a.keep):
            sys.exit(f"variant {a.keep} did not compile or is the current source; nothing written")
        src_path.write_text(text[:lo] + variants[a.keep] + text[hi:])
        print(f"variant {a.keep} written to {src_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
