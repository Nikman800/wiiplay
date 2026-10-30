"""Apply the source-name ledgers (dtk/source_names/*.tsv, written by tools/unit.py sync) to
config/RHAE01/symbols.txt.

Each row (address, old_name, new_name, unit) renames the symbol at an address to the name the
decompiled source compiles to (usually a mangled C++ name). old_name is the first name the sync
saw there and previous lists earlier source names, so the ledger applies to a fresh symbols.txt
and after renames; a symbol is renamed while it still
carries one of those. When units disagree about an address (a callee named from two callers), the
row from the unit that contains the address wins. Runs in tools/phase2_pipeline.sh.

Usage: apply_source_names.py <root>
"""
import bisect
import collections
import csv
import re
import sys
from pathlib import Path

LINE = re.compile(r"^(\S+)( = \S+:0x)([0-9A-F]+)(; // .*)$")


def load_rows(root):
    """address -> row, resolving disagreements in favour of the address's own unit"""
    units = []
    ledger = root / "dtk/game_units.tsv"
    if ledger.exists():
        units = sorted((int(r["start"], 16), int(r["end"], 16), r["name"])
                       for r in csv.DictReader(open(ledger, encoding="utf-8"), delimiter="\t"))
    starts = [u[0] for u in units]

    def owner(addr):
        i = bisect.bisect_right(starts, addr) - 1
        return units[i][2] if i >= 0 and addr < units[i][1] else None
    rows, conflicts = {}, []
    for path in sorted((root / "dtk/source_names").glob("*.tsv")):
        for r in csv.DictReader(open(path, encoding="utf-8"), delimiter="\t"):
            a = r["address"]
            if a in rows and rows[a]["new_name"] != r["new_name"]:
                own = owner(int(a, 16))
                if r["unit"] == own:
                    conflicts.append((a, rows[a]["new_name"], r["new_name"]))
                    rows[a] = r
                elif rows[a]["unit"] != own:
                    conflicts.append((a, rows[a]["new_name"], r["new_name"]))
                continue
            rows.setdefault(a, r)
    return rows, conflicts


def main():
    root = Path(sys.argv[1])
    rows, conflicts = load_rows(root)
    if not rows:
        return
    path = root / "config/RHAE01/symbols.txt"
    lines = path.read_text(encoding="utf-8").splitlines()
    matches = [LINE.match(l) for l in lines]
    def known(r):
        """names the address may carry: the first one seen, and earlier source names"""
        return {r["old_name"], *[p for p in (r.get("previous") or "").split(";") if p]}
    todo = {m.group(3): rows[m.group(3)]["new_name"] for m in matches
            if m and m.group(3) in rows and m.group(1) in known(rows[m.group(3)])}
    final = collections.Counter(todo.get(m.group(3), m.group(1)) for m in matches if m)
    out, renamed, clash, unscoped = [], 0, [], 0
    source_names = {r["new_name"] for r in rows.values()}
    for l, m in zip(lines, matches):
        new = todo.get(m.group(3)) if m else None
        if new and new != m.group(1) and final[new] == 1:
            l = new + m.group(2) + m.group(3) + m.group(4)
            renamed += 1
        elif new and new != m.group(1):
            clash.append(new)
        # A name our sources define is one symbol, not a static copy: earlier steps mark library
        # names that occur twice scope:local (two Wii Sports copies), which then sticks after the
        # rename makes it unique, and the asm can no longer reach it by name.
        m2 = LINE.match(l)
        if m2 and m2.group(1) in source_names and final[m2.group(1)] == 1 and " scope:local" in l \
                and "$" not in m2.group(1) and not m2.group(1).startswith("@"):
            l = l.replace(" scope:local", "")
            unscoped += 1
        out.append(l)
    path.write_text("\n".join(out) + "\n", encoding="utf-8", newline="\n")
    print(f"{renamed} symbols renamed to their source names; {unscoped} made global again"
          + (f"; not unique: {clash[:5]}" if clash else ""))
    for a, x, y in conflicts[:10]:
        print(f"  {a}: units disagree ({x} / {y})")


if __name__ == "__main__":
    main()
