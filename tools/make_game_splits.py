"""Write the game-code translation units of dtk/game_units.tsv into config/RHAE01/splits.txt.

dtk/game_units.tsv (start, end, name, classes, functions, evidence) is the curated unit ledger
for game code, first proposed by tools/tu_boundaries.py. Every unit whose .text starts in game
code is replaced by the ledger's units (reference units from tools/make_lib_splits.py keep their
names but take the ledger's range); data ranges are added afterwards by add_data_splits.py.
Runs in tools/phase2_pipeline.sh after make_lib_splits.py.

Usage: make_game_splits.py <root>
"""
import csv
import re
import sys
from pathlib import Path

GAME = (0x801827F4, 0x802E9CC0)  # RPSysSystem.cpp starts with its disk-check callbacks at 0x801827F4
UNIT_HEADER = re.compile(r"^(?!Sections:)(\S.*):\s*$")
TEXT = re.compile(r"^\s+\.text\s+start:0x([0-9A-F]+)\s+end:0x([0-9A-F]+)")


def main():
    root = Path(sys.argv[1])
    ledger = list(csv.DictReader(open(root / "dtk/game_units.tsv", encoding="utf-8"), delimiter="\t"))
    path = root / "config/RHAE01/splits.txt"
    lines = path.read_text(encoding="utf-8").splitlines()
    # group into blocks: header line + indented section lines
    head, blocks, cur = [], [], None
    for line in lines:
        if UNIT_HEADER.match(line) and not line.startswith("\t"):
            cur = [line]
            blocks.append(cur)
        elif cur is None:
            head.append(line)
        else:
            cur.append(line)
    kept, dropped = [], 0
    for b in blocks:
        text = next((TEXT.match(l) for l in b if TEXT.match(l)), None)
        if text and GAME[0] <= int(text.group(1), 16) < GAME[1]:
            dropped += 1
            continue
        kept.append(b)
    out = head[:]
    for b in kept:
        while b and not b[-1].strip():
            b.pop()
        out += [""] + b
    for r in ledger:
        out += ["", f"{r['name']}:", f"\t.text       start:0x{r['start']} end:0x{r['end']}"]
    path.write_text("\n".join(out).rstrip("\n") + "\n", encoding="utf-8", newline="\n")
    print(f"{len(ledger)} game units written ({dropped} earlier game-code units replaced)")


if __name__ == "__main__":
    main()
