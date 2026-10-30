"""Map each function address to the string literals in its Ghidra pseudocode.

Usage: extract_literals.py <root>   -> writes <root>/dtk/literals.json
"""
import glob
import json
import re
import sys
from pathlib import Path

HEADER = re.compile(r"^// (\S+) @ ([0-9a-f]{8})$")
LITERAL = re.compile(r'"((?:[^"\\]|\\.){3,})"')

root = Path(sys.argv[1])
lit = {}
for path in glob.glob(str(root / "decomp" / "*.c")):
    cur = None
    for line in open(path, encoding="utf-8", errors="replace"):
        m = HEADER.match(line.rstrip("\n"))
        if m:
            cur = m.group(2).upper()
            lit.setdefault(cur, [])
            continue
        if cur:
            for s in LITERAL.findall(line):
                if s not in lit[cur]:
                    lit[cur].append(s)
(root / "dtk" / "literals.json").write_text(json.dumps(lit), encoding="utf-8")
game = {a: v for a, v in lit.items() if 0x80182E90 <= int(a, 16) < 0x802E9CC0}
print(len(lit), "functions;", sum(1 for v in game.values() if v), "game functions use string literals")
