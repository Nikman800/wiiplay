"""Read the original DOL (extracted/sys/main.dol) by virtual address.

read(addr, size) -> bytes        (.bss reads as zeros)
find(needle, lo, hi, mask=None) -> addresses where needle occurs in [lo, hi), 4-byte aligned;
                                   mask[i] == 0 makes byte i a wildcard (relocated fields)
"""
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
_dol = None


def _load():
    global _dol
    if _dol is None:
        d = (ROOT / "extracted/sys/main.dol").read_bytes()
        offs = struct.unpack(">18I", d[0:72])
        addrs = struct.unpack(">18I", d[0x48:0x90])
        sizes = struct.unpack(">18I", d[0x90:0xD8])
        bss_addr, bss_size = struct.unpack(">II", d[0xD8:0xE0])
        _dol = (d, [(a, o, s) for o, a, s in zip(offs, addrs, sizes) if s], (bss_addr, bss_size))
    return _dol


def read(addr, size):
    d, segs, (ba, bs) = _load()
    for a, o, s in segs:
        if a <= addr and addr + size <= a + s:
            return d[o + addr - a:o + addr - a + size]
    if ba <= addr and addr + size <= ba + bs:
        return bytes(size)
    raise ValueError(f"{addr:#x}+{size:#x} is not inside one DOL section")


def find(needle, lo, hi, mask=None):
    d, segs, _ = _load()
    out = []
    for a, o, s in segs:
        start, end = max(lo, a), min(hi, a + s)
        if end - start < len(needle):
            continue
        hay = d[o + start - a:o + end - a]
        for k in range(0, len(hay) - len(needle) + 1, 4):
            if mask is None:
                if hay[k:k + len(needle)] == needle:
                    out.append(start + k)
            elif all(m == 0 or x == y for x, y, m in zip(hay[k:k + len(needle)], needle, mask)):
                out.append(start + k)
    return out
