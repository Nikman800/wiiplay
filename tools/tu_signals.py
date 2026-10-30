"""Per-function evidence for translation-unit (source file) boundaries in game code.

For every function in build/RHAE01/asm (the split assembly ninja writes), list the data it
references, by section. CodeWarrior gives each translation unit its own pools in .sdata2 (float
and double constants), .rodata and .data (strings, tables, vtables), laid out in link order, the
same order as .text. So the data a function references says which unit's pools it uses.

Usage: tu_signals.py <root>  -> build/tu/signals.tsv
  address, name, size, then for .sdata2/.rodata/.data/.sdata/.bss/.sbss: referenced addresses
"""
import re
import sys
from pathlib import Path

GAME = (0x80182930, 0x802E9CC0)
SECTIONS = (".sdata2", ".rodata", ".data", ".sdata", ".bss", ".sbss", ".sbss2", ".ctors")
SYM = re.compile(r"^(\S+) = (\.\w+):0x([0-9A-F]+);")
FN = re.compile(r'^\.fn "?(.+?)"?, \w+$')
INSN = re.compile(r"^/\* ([0-9A-F]{8}) [0-9A-F]{8} .*?\*/\s+\S+\s+(.*)$")
REF = re.compile(r"([A-Za-z_$.@][\w$.@<>,\\]*?)(?:@(?:ha|l|h|sda21))?(?:\+0x([0-9A-Fa-f]+))?(?=[@,()\s]|$)")


def load_symbols(root):
    syms = {}
    for line in open(root / "config/RHAE01/symbols.txt", encoding="utf-8"):
        m = SYM.match(line)
        if m:
            syms[m.group(1)] = (m.group(2), int(m.group(3), 16))
    return syms


def main():
    root = Path(sys.argv[1])
    syms = load_symbols(root)
    rows = []
    # newest first: the asm directory keeps files from earlier splits, which ninja no longer uses
    seen = set()
    for path in sorted((root / "build/RHAE01/asm").rglob("*.s"), key=lambda p: -p.stat().st_mtime):
        cur, pending = None, None
        for line in open(path, encoding="utf-8", errors="replace"):
            m = FN.match(line.rstrip())
            if m:
                pending, cur = m.group(1), None  # the address comes from the first instruction
                continue
            m = INSN.match(line)
            if not m:
                continue
            if pending is not None:
                addr = int(m.group(1), 16)
                if GAME[0] <= addr < GAME[1] and addr not in seen:
                    seen.add(addr)
                    cur = {"name": pending, "addr": addr, "end": addr, "refs": {s: set() for s in SECTIONS}}
                    rows.append(cur)
                pending = None
            if cur is None:
                continue
            cur["end"] = int(m.group(1), 16) + 4
            for name, off in REF.findall(m.group(2)):
                sec, addr = syms.get(name, (None, 0))
                if sec in cur["refs"]:
                    cur["refs"][sec].add(addr + int(off or "0", 16))
    rows.sort(key=lambda r: r["addr"])
    out = root / "build/tu/signals.tsv"
    out.parent.mkdir(parents=True, exist_ok=True)
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write("address\tname\tsize\t" + "\t".join(s.lstrip(".") for s in SECTIONS) + "\n")
        for r in rows:
            f.write(f"{r['addr']:08X}\t{r['name']}\t{r['end'] - r['addr']:X}\t" +
                    "\t".join(",".join(f"{a:08X}" for a in sorted(r["refs"][s])) for s in SECTIONS) + "\n")
    print(f"{len(rows)} game functions -> {out}")


if __name__ == "__main__":
    main()
