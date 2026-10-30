"""Minimal big-endian ELF32 symbol reader (stdlib only) for CodeWarrior/dtk objects.

symbols(path) -> list of dict(name, section, value, size, type, bind), section by name.
relocations(path) -> list of dict(section, offset, type, addend, symbol, ...).
"""
import struct
from pathlib import Path

STT = {0: "notype", 1: "object", 2: "func", 3: "section", 4: "file"}
STB = {0: "local", 1: "global", 2: "weak"}


def symbols(path):
    return read(path)[0]


def relocations(path):
    """list of dict(section, offset, type, symbol) for every .rela section"""
    return read(path)[1]


def read(path):
    d = Path(path).read_bytes()
    if d[:4] != b"\x7fELF":
        raise ValueError(f"{path}: not an ELF file")
    shoff, = struct.unpack(">I", d[0x20:0x24])
    shentsize, shnum, shstrndx = struct.unpack(">HHH", d[0x2E:0x34])
    secs = []
    for i in range(shnum):
        o = shoff + i * shentsize
        name, typ, flags, addr, off, size, link, info, align, entsize = struct.unpack(">IIIIIIIIII", d[o:o + 40])
        secs.append(dict(name=name, type=typ, off=off, size=size, link=link, info=info, entsize=entsize))
    shstr = secs[shstrndx]

    def cstr(base, k):
        e = d.index(b"\0", base + k)
        return d[base + k:e].decode("latin-1")
    for s in secs:
        s["name"] = cstr(shstr["off"], s["name"])
    out, table = [], []
    for s in secs:
        if s["type"] != 2:  # SHT_SYMTAB
            continue
        strtab = secs[s["link"]]
        for k in range(s["size"] // 16):
            o = s["off"] + k * 16
            name, value, size, info, other, shndx = struct.unpack(">IIIBBH", d[o:o + 16])
            sym = dict(name=cstr(strtab["off"], name) if name else "", value=value, size=size,
                       section=secs[shndx]["name"] if 0 < shndx < len(secs) else None,
                       type=STT.get(info & 15, str(info & 15)), bind=STB.get(info >> 4, "?"))
            table.append(sym)
            if name and 0 < shndx < len(secs):
                out.append(sym)
    relocs = []
    for s in secs:
        if s["type"] != 4:  # SHT_RELA
            continue
        target = secs[s["info"]]["name"] if "info" in s else None
        for k in range(s["size"] // 12):
            o = s["off"] + k * 12
            off, info, addend = struct.unpack(">IIi", d[o:o + 12])
            sym = table[info >> 8] if (info >> 8) < len(table) else None
            relocs.append(dict(section=target, offset=off, type=info & 0xFF, addend=addend,
                               symbol=sym["name"] if sym else "",
                               symbol_section=sym["section"] if sym else None,
                               symbol_value=sym["value"] if sym else 0))
    return out, relocs
