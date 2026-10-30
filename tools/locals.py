"""Print where CodeWarrior put each parameter and local variable of a function (register or stack).

Usage: locals.py <unit> <function substring> [--src FILE] [--include-first DIR] [--asm]

  locals.py egg/math/eggQuat.cpp Quatf::normalise
  locals.py Pack/RPParty/RPPartyCommon/RPDollBase.cpp HitTestPart --src /tmp/variant.cpp

The unit is compiled lock-free (tools/cccmd.py) with `-sym on` added, which makes CW emit DWARF 1.1
debug info naming every variable's register or stack offset; `dtk dwarf dump` decodes it. A
register swap then shows which variable sits in which register, instead of guessing declaration
orders. CW records one location per variable (its first live range), and `-sym on` does not change
the code. Locals of functions inlined into this one are not listed. Compare with the original's use of each register in the asm (unit.py diff).
Idea from pokebw2's tools/decomp/locals.py (their build already uses -sym on); written here for
dtk's DWARF 1.1 dump, no code taken.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import cccmd  # noqa: E402

ROOT = cccmd.ROOT


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("unit")
    ap.add_argument("function")
    ap.add_argument("--src")
    ap.add_argument("--include-first")
    a = ap.parse_args()
    obj = cccmd.compile_variant(a.unit, src=a.src, add=["-sym on"], include_first=a.include_first)
    if not obj:
        sys.exit("compile failed")
    out = subprocess.run([str(ROOT / "build/tools/dtk"), "dwarf", "dump", "--no-color", str(obj)],
                         capture_output=True, text=True).stdout
    # dtk prints each function as "// Range: ..." comment lines, then its signature and a {...} body
    blocks = re.split(r"\n(?=// Range: )", out)
    hits = [b for b in blocks if b.startswith("// Range: ") and a.function in b.split("{", 1)[0]]
    if not hits:
        sys.exit(f"no function matching {a.function!r} in the DWARF info")
    for b in hits:
        # stop at the end of the function body (the next top-level definition follows a blank line)
        body = re.split(r"\n\}\n", b, maxsplit=1)[0] + "\n}"
        print(body.strip() + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
