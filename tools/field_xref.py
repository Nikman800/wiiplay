"""Collect `this`-relative field accesses for a class from Ghidra pseudocode.

Usage: field_xref.py <ClassPrefix> [<ClassPrefix>...]     e.g. Hky_Puck Hky_Game
For each function named <ClassPrefix>_*, treats param_1 as `this` and records
  *(T *)(param_1 + off)   typed access (read or write)
  param_1[i]              indexed access (element size from param_1's declared type)
  (param_1 + off)         address taken (embedded sub-object or array)
Prints one table per class: offset, types seen, reads, writes, address-taken count, methods.
"""
import collections
import glob
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HEADER = re.compile(r"^// (\S+) @ ([0-9a-f]{8})$")
SIG = re.compile(r"^[\w\s\*]+?\b(\w+)\s*\((.*)\)\s*$")
TYPED = re.compile(r"\*\((\w[\w ]*?\s*\**)\s*\*\)\(param_1 \+ (0x[0-9a-f]+|\d+)\)")
INDEXED = re.compile(r"\bparam_1\[(0x[0-9a-f]+|\d+)\]")
ADDR = re.compile(r"(?<![\*\w])\(?param_1 \+ (0x[0-9a-f]+|\d+)\)?(?!\))")
SIZES = {"char": 1, "byte": 1, "undefined1": 1, "bool": 1, "short": 2, "ushort": 2, "undefined2": 2,
         "int": 4, "uint": 4, "undefined4": 4, "float": 4, "code": 4, "longlong": 8, "double": 8, "undefined8": 8}


def functions():
    for path in glob.glob(str(ROOT / "decomp" / "*.c")):
        name, addr, body = None, None, []
        for line in open(path, encoding="utf-8", errors="replace"):
            m = HEADER.match(line.rstrip("\n"))
            if m:
                if name:
                    yield name, addr, body
                name, addr, body = m.group(1), m.group(2), []
            elif name:
                body.append(line.rstrip("\n"))
        if name:
            yield name, addr, body


def param1_elem_size(body):
    for line in body[:6]:
        m = SIG.match(line)
        if m:
            first = m.group(2).split(",")[0].strip()
            t = first.replace("param_1", "").strip()
            if t.endswith("*"):
                base = t.rstrip("*").strip()
                return SIZES.get(base, 4), base
            return None, t
    return None, None


def main():
    prefixes = sys.argv[1:]
    stats = {p: collections.defaultdict(lambda: dict(types=collections.Counter(), r=0, w=0, a=0, methods=set()))
             for p in prefixes}
    sizes = {}
    for name, addr, body in functions():
        cls = next((p for p in prefixes if name.startswith(p + "_")), None)
        if not cls:
            continue
        method = name[len(cls) + 1:]
        esize, etype = param1_elem_size(body)
        for line in body:
            code = line.split("//")[0]
            is_write = re.search(r"^\s*\*?\(?[^=]*param_1[^=]*\)?\s*=(?!=)", code) is not None
            lhs = code.split("=")[0] if is_write else ""
            for t, off in TYPED.findall(code):
                o = int(off, 0)
                f = stats[cls][o]
                f["types"][t.strip()] += 1
                f["methods"].add(method)
                if is_write and f"param_1 + {off})" in lhs:
                    f["w"] += 1
                else:
                    f["r"] += 1
            if esize:
                for idx in INDEXED.findall(code):
                    o = int(idx, 0) * esize
                    f = stats[cls][o]
                    f["types"][etype] += 1
                    f["methods"].add(method)
                    if is_write and f"param_1[{idx}]" in lhs:
                        f["w"] += 1
                    else:
                        f["r"] += 1
            stripped = TYPED.sub("", code)
            for off in ADDR.findall(stripped):
                o = int(off, 0)
                f = stats[cls][o]
                f["a"] += 1
                f["methods"].add(method)
    for cls in prefixes:
        print(f"== {cls}  ({len(stats[cls])} offsets)")
        print("  off    types                         R   W   &   methods")
        for o in sorted(stats[cls]):
            f = stats[cls][o]
            types = ",".join(f"{t}" for t, _ in f["types"].most_common(3))
            ms = ",".join(sorted(f["methods"]))
            print(f"  0x{o:03x}  {types:28.28} {f['r']:3} {f['w']:3} {f['a']:3}   {ms[:90]}")
        print()


if __name__ == "__main__":
    main()
