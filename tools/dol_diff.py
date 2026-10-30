"""Locate where a rebuilt DOL first diverges from the original, per section.

Code sections compare instructions with relocated fields masked (branches, lis/addi, loads and
stores), so a layout shift shows up where code actually moves rather than at the first branch
whose target changed. Reports the containing symbol and, when found, the shift distance.

Usage: dol_diff.py [original.dol] [rebuilt.dol]
"""
import bisect
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RELOC_OPS = {14, 15, 16, 18, 32, 34, 36, 38, 40, 44, 48, 52}


def sections(path):
    d = Path(path).read_bytes()
    offs = struct.unpack_from(">18I", d, 0)
    addrs = struct.unpack_from(">18I", d, 0x48)
    sizes = struct.unpack_from(">18I", d, 0x90)
    return d, [(addrs[i], sizes[i], offs[i], i < 7) for i in range(18) if sizes[i]]


def word(buf, i, code):
    w = struct.unpack_from(">I", buf, i)[0]
    return 0 if code and (w >> 26) in RELOC_OPS else w


def main():
    orig = sys.argv[1] if len(sys.argv) > 1 else ROOT / "extracted/sys/main.dol"
    new = sys.argv[2] if len(sys.argv) > 2 else ROOT / "build/RHAE01/main.dol"
    A, sa = sections(orig)
    B, sb = sections(new)
    syms = []
    for l in open(ROOT / "config/RHAE01/symbols.txt", encoding="utf-8"):
        m = re.match(r"(\S+) = (\S+):0x([0-9A-F]+); // .*size:0x([0-9A-F]+)", l)
        if m:
            syms.append((int(m[3], 16), int(m[4], 16), m[1]))
    syms.sort()
    starts = [s[0] for s in syms]
    clean = True
    for (aa, asz, ao, code), (ba, bsz, bo, _) in zip(sa, sb):
        if (aa, asz) != (ba, bsz):
            print(f"section {aa:08X}: address/size differ ({asz:X} vs {ba:08X} {bsz:X})")
            clean = False
            continue
        a, b = A[ao:ao + asz], B[bo:bo + bsz]
        if a == b:
            continue
        clean = False
        first = next((i for i in range(0, asz, 4) if word(a, i, code) != word(b, i, code)), None)
        if first is None:
            print(f"section {aa:08X}: only relocated fields differ (targets moved elsewhere)")
            continue
        k = bisect.bisect_right(starts, aa + first) - 1
        where = f"{syms[k][2]} @ {syms[k][0]:08X} size {syms[k][1]:X}" if k >= 0 else "?"
        shift = next((d for d in range(-256, 260, 4) if d and first + d >= 0 and
                      all(word(a, first + j, code) == word(b, first + j + d, code)
                          for j in range(0, 64, 4) if first + j + d + 4 <= asz and first + j + 4 <= asz)), None)
        print(f"section {aa:08X}: first difference at {aa + first:08X} in {where}"
              + (f"; code shifted by {shift:+d} bytes" if shift else ""))
    if clean:
        print("identical")


if __name__ == "__main__":
    main()
