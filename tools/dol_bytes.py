"""Print bytes of the original DOL as a C initializer (data tables for a source unit).

Usage: dol_bytes.py <addr> <size> [--type u8|s8|u16|s16|u32|s32|f32|f64] [--name NAME] [--per-line N]

  dol_bytes.py 0x8033FAC8 0x41E --type u8 --name gammaSet
prints `static u8 gammaSet[0x41E] = { ... };`. Floats print with enough digits to round-trip.
Words that hold pointers in the original are relocations: check symbols.txt / the asm before
turning a u32 table into data, and write those entries as &symbol instead.
"""
import argparse
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import dolio  # noqa: E402

FMT = {"u8": ("B", 1), "s8": ("b", 1), "u16": ("H", 2), "s16": ("h", 2), "u32": ("I", 4),
       "s32": ("i", 4), "f32": ("f", 4), "f64": ("d", 8)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("addr", type=lambda x: int(x, 0))
    ap.add_argument("size", type=lambda x: int(x, 0))
    ap.add_argument("--type", default="u8", choices=FMT)
    ap.add_argument("--name", default="data")
    ap.add_argument("--per-line", type=int, default=0)
    a = ap.parse_args()
    code, w = FMT[a.type]
    if a.size % w:
        sys.exit(f"size {a.size:#x} is not a multiple of {a.type}'s {w} bytes")
    raw = dolio.read(a.addr, a.size)
    vals = struct.unpack(f">{a.size // w}{code}", raw)
    if a.type in ("f32", "f64"):
        text = [repr(float(v)) + ("f" if a.type == "f32" else "") for v in vals]
    elif a.type.startswith("u"):
        text = [f"0x{v:0{w * 2}X}" for v in vals]
    else:
        text = [str(v) for v in vals]
    per = a.per_line or {1: 16, 2: 8, 4: 4, 8: 2}[w]
    print(f"static {a.type} {a.name}[{len(vals):#x}] = {{")
    for i in range(0, len(text), per):
        print("    " + ", ".join(text[i:i + per]) + ",")
    print("};")
    return 0


if __name__ == "__main__":
    sys.exit(main())
