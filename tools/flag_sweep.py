"""Try a unit under every installed compiler and a set of flag toggles; rank by match.

Usage: flag_sweep.py <unit> [function ...] [--jobs N] [--compilers GC/3.0a3,GC/3.0a5.2,...]

Scores with tools/objcmp.py against build/RHAE01/obj/<unit>.o (only the named functions, or all).
Toggles are the ones that decided matches in this project: -ipa file (MSL_C/printf, ansi_fp,
eggController), -Cpp_exceptions on (most MSL units with extab), -inline deferred and -sdata2 0
(MetroTRK). The current configuration is listed first so improvements are easy to see.
"""
import argparse
import itertools
import shutil
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import cccmd  # noqa: E402
import objcmp  # noqa: E402

ROOT = cccmd.ROOT
TOGGLES = [("-ipa file", None), ("-Cpp_exceptions on", "-Cpp_exceptions off"),
           ("-inline deferred", "-inline auto"), ("-sdata2 0", None)]


def installed():
    out = []
    for fam in ("GC", "Wii"):
        for d in sorted((ROOT / "build/compilers" / fam).glob("*")):
            if (d / "mwcceppc.exe").exists():
                out.append(f"{fam}/{d.name}")
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("unit")
    ap.add_argument("functions", nargs="*")
    ap.add_argument("--jobs", type=int, default=8)
    ap.add_argument("--compilers", default="")
    a = ap.parse_args()
    target = objcmp.Obj(ROOT / "build/RHAE01/obj" / Path(a.unit).with_suffix(".o"))
    names = a.functions or None
    base = cccmd.base_command(a.unit)
    compilers = a.compilers.split(",") if a.compilers else installed()
    variants, seen = [(None, ())], set()  # the current configuration first
    for c in compilers:
        for k in range(len(TOGGLES) + 1):
            for combo in itertools.combinations(TOGGLES, k):
                add = tuple(on for on, _ in combo if on not in base)
                rem = tuple(off for _, off in combo if off and off in base)
                if (c, add, rem) in seen or (k and not add and not rem):
                    continue  # same command as another variant
                seen.add((c, add, rem))
                variants.append((c, combo))
    tmp = Path(tempfile.mkdtemp(prefix="sweep_"))

    def run(i_v):
        i, (comp, combo) = i_v
        add = [on for on, _ in combo if on not in base]
        rem = [off for _, off in combo if off and off in base]
        obj = cccmd.compile_variant(a.unit, compiler=comp, add=add, remove=rem, outdir=tmp / str(i))
        if obj is None:
            return None
        return objcmp.score(target, objcmp.Obj(obj), names), comp or "(current)", add + [f"(no {r})" for r in rem]

    try:
        with ThreadPoolExecutor(a.jobs) as ex:
            res = [r for r in ex.map(run, enumerate(variants)) if r]
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    cur = next((r for r in res if r[1] == "(current)"), None)
    print(f"{a.unit}: {len(res)} of {len(variants)} variants compiled"
          + (f"; current configuration scores {cur[0]:.4f}" if cur else ""))
    for s, comp, add in sorted(res, key=lambda r: -r[0])[:15]:
        print(f"  {s:.4f}  {comp:14} {' '.join(add) or '-'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
