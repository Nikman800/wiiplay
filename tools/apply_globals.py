"""Apply global-data names from dtk/types/*_globals.tsv to config/RHAE01/symbols.txt.

Each ledger row: address, name, type, confidence, note. Only data symbols that still carry a
generic name (lbl_*, DAT_*) are renamed; names must be valid C identifiers and unique.
Run in tools/phase2_pipeline.sh after the function-name steps so re-runs keep the names.

Usage: apply_globals.py <root> [check]
"""
import collections
import csv
import re
import sys
from pathlib import Path

LINE = re.compile(r"^(\S+)( = \S+:0x)([0-9A-F]+)(; // .*)$")
NAME = re.compile(r"^[A-Za-z_][A-Za-z0-9_]{0,99}$")
GENERIC = re.compile(r"^(lbl|DAT)_[0-9A-F]{8}$")


def main():
    root = Path(sys.argv[1])
    check_only = len(sys.argv) > 2 and sys.argv[2] == "check"
    wanted, problems = {}, []
    for path in sorted((root / "dtk/types").glob("*_globals.tsv")):
        lines = path.read_text(encoding="utf-8").splitlines()
        if lines:
            lines[0] = lines[0].lstrip("# ")  # tolerate a commented header line ("# address\tname...")
        for i, r in enumerate(csv.DictReader(lines, delimiter="\t"), start=2):
            a = (r.get("address") or "").strip().upper().removeprefix("0X")
            n = (r.get("name") or "").strip()
            if not re.match(r"^[0-9A-F]{8}$", a) or not NAME.match(n):
                problems.append(f"{path.name}:{i}: bad address/name {a!r} {n!r}")
                continue
            if a in wanted and wanted[a][0] != n:
                problems.append(f"{path.name}:{i}: {a} also named {wanted[a][0]} in {wanted[a][1]}")
                continue
            wanted[a] = (n, path.name)
    sym = root / "config/RHAE01/symbols.txt"
    lines = sym.read_text(encoding="utf-8").splitlines()
    taken = collections.Counter(m.group(1) for m in map(LINE.match, lines) if m)
    out, renamed = [], 0
    for l in lines:
        m = LINE.match(l)
        if m and m.group(3) in wanted and "type:function" not in m.group(4):
            n = wanted[m.group(3)][0]
            if m.group(1) != n:
                if not GENERIC.match(m.group(1)):
                    problems.append(f"{m.group(3)}: already named {m.group(1)}, not renamed to {n}")
                elif taken[n]:
                    problems.append(f"{m.group(3)}: {n} already used by another symbol")
                else:
                    l = n + m.group(2) + m.group(3) + m.group(4)
                    taken[n] += 1
                    renamed += 1
        out.append(l)
    for p in problems[:20]:
        print("  " + p)
    print(f"{len(wanted)} global names in ledgers; {renamed} applied; {len(problems)} problems")
    if not check_only:
        sym.write_text("\n".join(out) + "\n", encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
