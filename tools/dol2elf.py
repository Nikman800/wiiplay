"""Convert a GameCube/Wii DOL into a big-endian ELF32 PowerPC ET_EXEC.

Optionally imports a decomp-toolkit symbols.txt so the ELF carries function
and object symbols (STT_FUNC / STT_OBJECT with sizes). _SDA_BASE_ and
_SDA2_BASE_ can be supplied so Ghidra's PowerPC ELF extension can seed r13/r2.

Usage: dol2elf.py main.dol out.elf [--symbols symbols.txt]
                  [--sda 0x...] [--sda2 0x...]
"""
import argparse
import re
import struct

EM_PPC = 20
SHT_PROGBITS, SHT_SYMTAB, SHT_STRTAB, SHT_NOBITS = 1, 2, 3, 8
SHF_WRITE, SHF_ALLOC, SHF_EXECINSTR = 1, 2, 4
STT_NOTYPE, STT_OBJECT, STT_FUNC = 0, 1, 2
STB_LOCAL, STB_GLOBAL = 0, 1
SHN_ABS = 0xFFF1
PT_LOAD = 1
PF_X, PF_W, PF_R = 1, 2, 4

# Section names in address order, matching `dtk dol info` for this layout.
TEXT_NAMES = [".init", ".text"] + [f".text{i}" for i in range(2, 7)]
DATA_NAMES = ["extab", "extabindex", ".ctors", ".dtors", ".data6", ".rodata",
              ".data", ".sdata", ".sdata2"]
READONLY_DATA = {"extab", "extabindex", ".ctors", ".dtors", ".rodata", ".data6", ".sdata2"}

SYMBOL_RE = re.compile(r"^(\S+) = (\S+):0x([0-9A-Fa-f]+); // (.*)$")


def parse_dol(data):
    offs = struct.unpack_from(">18I", data, 0x00)
    addrs = struct.unpack_from(">18I", data, 0x48)
    sizes = struct.unpack_from(">18I", data, 0x90)
    bss_addr, bss_size, entry = struct.unpack_from(">3I", data, 0xD8)
    sections = []
    text_i = data_i = 0
    # DOL slot order is not address order; name each group by address.
    for i in sorted(range(7), key=lambda i: addrs[i]) + sorted(range(7, 18), key=lambda i: addrs[i]):
        if sizes[i] == 0:
            continue
        if i < 7:
            name, kind = TEXT_NAMES[text_i], "code"
            text_i += 1
        else:
            name = DATA_NAMES[data_i] if data_i < len(DATA_NAMES) else f".data{data_i}"
            kind = "rodata" if name in READONLY_DATA else "data"
            data_i += 1
        sections.append(dict(name=name, kind=kind, addr=addrs[i], size=sizes[i],
                             data=data[offs[i]:offs[i] + sizes[i]]))
    # The DOL's single BSS range spans .sdata/.sdata2 too; carve out the gaps.
    end = bss_addr + bss_size
    initialized = sorted((s["addr"], s["addr"] + s["size"]) for s in sections)
    cursor, bss_parts = bss_addr, []
    for lo, hi in initialized:
        if hi <= cursor or lo >= end:
            continue
        if lo > cursor:
            bss_parts.append((cursor, lo))
        cursor = max(cursor, hi)
    if cursor < end:
        bss_parts.append((cursor, end))
    bss_names = [".bss", ".sbss", ".bss2"]
    for i, (lo, hi) in enumerate(bss_parts):
        name = bss_names[i] if i < len(bss_names) else f".bss{i}"
        sections.append(dict(name=name, kind="bss", addr=lo, size=hi - lo, data=b""))
    sections.sort(key=lambda s: s["addr"])
    return sections, entry


def parse_symbols(path):
    symbols = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            m = SYMBOL_RE.match(line.strip())
            if not m:
                continue
            name, _section, addr, attrs = m.groups()
            fields = dict(a.split(":", 1) for a in attrs.split() if ":" in a)
            kind = fields.get("type")
            if kind not in ("function", "object"):
                continue
            size = int(fields["size"], 16) if "size" in fields else 0
            local = fields.get("scope") == "local"
            symbols.append((name, int(addr, 16), size, kind, local))
    return symbols


def build_elf(sections, entry, symbols, extra_abs):
    shstrtab, strtab = bytearray(b"\0"), bytearray(b"\0")

    def add_str(table, s):
        off = len(table)
        table += s.encode() + b"\0"
        return off

    # Section index 0 is null; allocated sections start at 1.
    sec_index = {id(s): i + 1 for i, s in enumerate(sections)}

    def find_section(addr):
        for s in sections:
            if s["addr"] <= addr < s["addr"] + max(s["size"], 1):
                return sec_index[id(s)]
        return None

    local_syms, global_syms = [], []
    for name, addr, size, kind, local in symbols:
        shndx = find_section(addr)
        if shndx is None:
            continue
        stt = STT_FUNC if kind == "function" else STT_OBJECT
        (local_syms if local else global_syms).append((name, addr, size, stt, shndx, local))
    for name, addr in extra_abs:
        global_syms.append((name, addr, 0, STT_NOTYPE, SHN_ABS, False))

    symtab = bytearray(struct.pack(">IIIBBH", 0, 0, 0, 0, 0, 0))
    for name, addr, size, stt, shndx, local in local_syms + global_syms:
        bind = STB_LOCAL if local else STB_GLOBAL
        symtab += struct.pack(">IIIBBH", add_str(strtab, name), addr, size,
                              (bind << 4) | stt, 0, shndx)
    first_global = 1 + len(local_syms)

    ehsize, phentsize, shentsize = 52, 32, 40
    loadable = sections
    phoff = ehsize
    cursor = phoff + phentsize * len(loadable)
    body = bytearray()
    file_offsets = {}
    for s in sections:
        if s["kind"] == "bss":
            file_offsets[id(s)] = cursor
            continue
        pad = (-cursor) % 32
        body += b"\0" * pad
        cursor += pad
        file_offsets[id(s)] = cursor
        body += s["data"]
        cursor += len(s["data"])
    symtab_off = cursor
    body += symtab
    cursor += len(symtab)
    strtab_off = cursor
    body += strtab
    cursor += len(strtab)

    shdrs = [struct.pack(">10I", *[0] * 10)]
    for s in sections:
        flags = SHF_ALLOC
        if s["kind"] == "code":
            flags |= SHF_EXECINSTR
        elif s["kind"] in ("data", "bss"):
            flags |= SHF_WRITE
        shtype = SHT_NOBITS if s["kind"] == "bss" else SHT_PROGBITS
        shdrs.append(struct.pack(">10I", add_str(shstrtab, s["name"]), shtype, flags,
                                 s["addr"], file_offsets[id(s)], s["size"], 0, 0, 32, 0))
    symtab_idx = len(shdrs)
    shdrs.append(struct.pack(">10I", add_str(shstrtab, ".symtab"), SHT_SYMTAB, 0, 0,
                             symtab_off, len(symtab), symtab_idx + 1, first_global, 4, 16))
    shdrs.append(struct.pack(">10I", add_str(shstrtab, ".strtab"), SHT_STRTAB, 0, 0,
                             strtab_off, len(strtab), 0, 0, 1, 0))
    shstrndx = len(shdrs)
    name_off = add_str(shstrtab, ".shstrtab")
    shstrtab_off = cursor
    body += shstrtab
    cursor += len(shstrtab)
    shdrs.append(struct.pack(">10I", name_off, SHT_STRTAB, 0, 0, shstrtab_off,
                             len(shstrtab), 0, 0, 1, 0))
    pad = (-cursor) % 4
    body += b"\0" * pad
    cursor += pad
    shoff = cursor

    phdrs = bytearray()
    for s in loadable:
        flags = PF_R | (PF_X if s["kind"] == "code" else 0) | (PF_W if s["kind"] in ("data", "bss") else 0)
        filesz = 0 if s["kind"] == "bss" else s["size"]
        phdrs += struct.pack(">8I", PT_LOAD, file_offsets[id(s)], s["addr"], s["addr"],
                             filesz, s["size"], flags, 32)

    ident = b"\x7fELF" + bytes([1, 2, 1, 0]) + b"\0" * 8
    ehdr = ident + struct.pack(">HHIIIIIHHHHHH", 2, EM_PPC, 1, entry, phoff, shoff, 0,
                               ehsize, phentsize, len(loadable), shentsize, len(shdrs), shstrndx)
    return ehdr + phdrs + body + b"".join(shdrs), len(local_syms) + len(global_syms)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("dol")
    ap.add_argument("out")
    ap.add_argument("--symbols")
    ap.add_argument("--sda", type=lambda v: int(v, 0))
    ap.add_argument("--sda2", type=lambda v: int(v, 0))
    args = ap.parse_args()
    with open(args.dol, "rb") as f:
        sections, entry = parse_dol(f.read())
    symbols = parse_symbols(args.symbols) if args.symbols else []
    extra = [(n, v) for n, v in (("_SDA_BASE_", args.sda), ("_SDA2_BASE_", args.sda2)) if v is not None]
    elf, count = build_elf(sections, entry, symbols, extra)
    with open(args.out, "wb") as f:
        f.write(elf)
    for s in sections:
        print(f"{s['name']:<12} {s['kind']:<7} 0x{s['addr']:08X} 0x{s['size']:X}")
    print(f"entry 0x{entry:08X}, {count} symbols -> {args.out}")


if __name__ == "__main__":
    main()
