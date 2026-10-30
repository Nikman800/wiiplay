"""Apply reference-derived library data names (dtk/library_data_matches.tsv, tools/align_data.py).

Renames only data symbols that still have a generic name (lbl_*, DAT_*). Names that occur more
than once are static objects from different source files and get scope:local, as for functions.

Usage: apply_library_data.py <root>
"""
import collections
import csv
import re
import sys
from pathlib import Path

LINE = re.compile(r"^(\S+)( = \S+:0x)([0-9A-F]+)(; // )(.*)$")
GENERIC = re.compile(r"^(lbl|DAT)_[0-9A-F]{8}$")
VALID = re.compile(r"^[A-Za-z_@$.][A-Za-z0-9_@$.<>,:\\]{0,250}$")


def main():
    root = Path(sys.argv[1])
    ledger = root / "dtk/library_data_matches.tsv"
    names = {r["address"]: r["ref_name"] for r in csv.DictReader(open(ledger, encoding="utf-8"), delimiter="\t")
             if VALID.match(r["ref_name"])}
    path = root / "config/RHAE01/symbols.txt"
    lines = path.read_text(encoding="utf-8").splitlines()
    parsed = [LINE.match(l) for l in lines]
    final = []
    for m in parsed:
        if m and "type:function" not in m.group(5) and m.group(3) in names and GENERIC.match(m.group(1)):
            final.append(names[m.group(3)])
        else:
            final.append(m.group(1) if m else None)
    # Duplicate static names make dtk split each copy into its own unit, which fails when copies
    # share an automatic unit (e.g. two nw4r g3d bbFunc tables). Leave duplicates generic until their
    # data sits in real source-file units.
    counts = collections.Counter(n for n in final if n)
    out, renamed, skipped = [], 0, 0
    for line, m, name in zip(lines, parsed, final):
        if not m:
            out.append(line)
            continue
        if name != m.group(1) and counts[name] > 1:
            name = m.group(1)
            skipped += 1
        elif name != m.group(1):
            renamed += 1
        out.append(f"{name}{m.group(2)}{m.group(3)}{m.group(4)}{m.group(5)}")
    path.write_text("\n".join(out) + "\n", encoding="utf-8", newline="\n")
    print(f"renamed {renamed} library data objects ({skipped} duplicate static names left generic)")


if __name__ == "__main__":
    main()
