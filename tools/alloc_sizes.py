"""Collect object sizes from allocation sites: operator new(size, ...) followed by a constructor.

Scans decomp/*.c (Ghidra pseudocode with current names). For each call to an operator new
variant, the first constructor called within the next few lines gives the class. A constructor
seen with several sizes is a base class called from derived constructors, so its rows are marked
`base?`: the size belongs to the derived class, identified by the vtable that caller stores.

Usage: alloc_sizes.py <root>   -> dtk/types/alloc_sizes.tsv
  size  ctor  class  caller  caller_addr  flag
"""
import collections
import glob
import re
import sys
from pathlib import Path

HEADER = re.compile(r"^// (\S+) @ ([0-9a-f]{8})$")
NEW = re.compile(r"\b(__nwa?__FUl(?:PQ23EGG4Heapi|Pv|i)?)\((0x[0-9a-f]+|\d+)")
CTOR = re.compile(r"\b(__ct__\w+|\w+_ctor)\(")
MANGLED_CLASS = re.compile(r"^__ct__(?:(\d+)(\w+)|Q(\d)(\w+))")


def class_of(ctor):
    m = MANGLED_CLASS.match(ctor)
    if not m:
        return ctor[:-len("_ctor")] if ctor.endswith("_ctor") else ctor
    if m.group(1):
        return m.group(2)[:int(m.group(1))]
    parts, rest = [], m.group(4)
    for _ in range(int(m.group(3))):
        n = re.match(r"(\d+)", rest)
        if not n:
            break
        length = int(n.group(1))
        rest = rest[len(n.group(1)):]
        parts.append(rest[:length])
        rest = rest[length:]
    return "::".join(parts)


VTSTORE = re.compile(r"=\s*&?(?:lbl_|PTR_|DAT_)?([0-9A-Fa-f]{8})\b")
VTABLES = {}  # vtable address -> class name (dtk/classes.tsv)


def main():
    root = Path(sys.argv[1])
    for line in open(root / "dtk/classes.tsv", encoding="utf-8").read().splitlines()[1:]:
        f = line.split("\t")
        if len(f) >= 2:
            VTABLES.setdefault(f[0].upper(), f[1])
    rows = []
    for path in glob.glob(str(root / "decomp" / "*.c")):
        func, addr, lines = None, None, open(path, encoding="utf-8", errors="replace").read().split("\n")
        for i, line in enumerate(lines):
            h = HEADER.match(line)
            if h:
                func, addr = h.group(1), h.group(2).upper()
                continue
            m = NEW.search(line)
            if not m or func is None:
                continue
            size = int(m.group(2), 0)
            for k, nxt in enumerate(lines[i:i + 8]):
                c = CTOR.search(nxt.split(m.group(1), 1)[-1] if nxt is line else nxt)
                if c:
                    # the derived constructor (often inlined) stores its vtable right after
                    vt = ""
                    for later in lines[i + k + 1:i + k + 14]:
                        v = VTSTORE.search(later)
                        if v and v.group(1).upper() in VTABLES:
                            vt = v.group(1).upper()
                            break
                    rows.append([size, c.group(1), class_of(c.group(1)), func, addr, vt])
                    break
    sizes = collections.defaultdict(set)
    for r in rows:
        sizes[r[1]].add(r[0])
    out = root / "dtk/types/alloc_sizes.tsv"
    out.parent.mkdir(parents=True, exist_ok=True)
    seen = set()
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write("size\tctor\tclass\tcaller\tcaller_addr\tflag\tvtable\tvtable_class\n")
        for size, ctor, cls, caller, a, vt in sorted(rows, key=lambda r: (r[2], r[0], r[4])):
            key = (size, ctor, a)
            if key in seen:
                continue
            seen.add(key)
            flag = "base?" if len(sizes[ctor]) > 1 else ""
            f.write(f"0x{size:X}\t{ctor}\t{cls}\t{caller}\t{a}\t{flag}\t{vt}\t{VTABLES.get(vt, '')}\n")
    exact = {c for c, s in sizes.items() if len(s) == 1}
    with_vt = sum(1 for r in rows if r[5])
    print(f"{len(seen)} allocation sites; {len(sizes)} constructors; {len(exact)} with a single size; "
          f"{with_vt} sites resolved to a derived vtable -> {out}")


if __name__ == "__main__":
    main()
