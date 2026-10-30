"""Build a function index from decomp-toolkit split asm + symbols.txt.

For each function: callees (bl/b targets), referenced symbols, strings found
inside referenced data objects, and callers. Also lists vtable-like data
objects (runs of function pointers) with their referencing functions.

Usage: build_index.py <root>   (root = wii-play project dir)
Writes <root>/dtk/index.json
"""
import json
import re
import sys
from pathlib import Path

SYM_RE = re.compile(r"^(\S+) = (\S+):0x([0-9A-Fa-f]+); // (.*)$")
FN_RE = re.compile(r"^\.fn (\S+),")
ENDFN_RE = re.compile(r"^\.endfn (\S+)")
OBJ_RE = re.compile(r"^\.obj (\S+),")
ENDOBJ_RE = re.compile(r"^\.endobj (\S+)")
INSN_RE = re.compile(r"^/\* ([0-9A-F]{8}) [0-9A-F]{8}  [0-9A-F ]{11} \*/\t(\S+)\s*(.*)$")
REF_RE = re.compile(r"\b((?:fn|lbl|jumptable|@etb|@eti|@stringBase\d*)_?[0-9A-Fa-f]{8}|[A-Za-z_][A-Za-z0-9_@$]*)(?:@ha|@l|@h|@sda21)?")
BRANCH_OPS = {"bl", "b"}
STRING_RE = re.compile(rb"[\x20-\x7e\t\n]{4,}\x00")


def load_symbols(path):
    syms = {}
    for line in Path(path).read_text(encoding="utf-8").splitlines():
        m = SYM_RE.match(line.strip())
        if not m:
            continue
        name, section, addr, attrs = m.groups()
        fields = dict(a.split(":", 1) for a in attrs.split() if ":" in a)
        syms[name] = dict(name=name, section=section, addr=int(addr, 16),
                          size=int(fields.get("size", "0"), 16), type=fields.get("type"))
    return syms


def load_dol(path):
    data = Path(path).read_bytes()
    import struct
    offs = struct.unpack_from(">18I", data, 0)
    addrs = struct.unpack_from(">18I", data, 0x48)
    sizes = struct.unpack_from(">18I", data, 0x90)
    segs = [(addrs[i], sizes[i], offs[i]) for i in range(18) if sizes[i]]

    def read(addr, n):
        for a, s, o in segs:
            if a <= addr < a + s:
                n = min(n, a + s - addr)
                return data[o + addr - a:o + addr - a + n]
        return b""
    return read


def strings_in(read, addr, size):
    if size <= 0 or size > 0x4000:
        size = min(max(size, 0), 0x4000)
    blob = read(addr, size)
    out = []
    for m in STRING_RE.finditer(blob):
        s = m.group()[:-1].decode("ascii", "replace").strip()
        if len(s) >= 4 and re.search(r"[A-Za-z]{3}", s):
            out.append(s)
    return out


def main():
    root = Path(sys.argv[1])
    syms = load_symbols(root / "dtk/symbols.orig.txt")  # dtk split asm uses dtk's original names
    read = load_dol(root / "extracted/sys/main.dol")
    funcs = {}
    objs = {}
    for asm in sorted((root / "dtk/split/asm").glob("*.s")):
        unit = asm.stem
        cur = None
        cur_obj = None
        for line in asm.read_text(encoding="utf-8", errors="replace").splitlines():
            m = FN_RE.match(line)
            if m:
                name = m.group(1)
                s = syms.get(name, {})
                cur = funcs.setdefault(name, dict(name=name, addr=s.get("addr"), size=s.get("size"),
                                                  unit=unit, callees=[], refs=[]))
                continue
            if ENDFN_RE.match(line):
                cur = None
                continue
            m = OBJ_RE.match(line)
            if m:
                cur_obj = objs.setdefault(m.group(1), dict(name=m.group(1), unit=unit, ptrs=[]))
                continue
            if ENDOBJ_RE.match(line):
                cur_obj = None
                continue
            if cur_obj is not None and line.strip().startswith(".4byte"):
                target = line.strip().split(None, 1)[1].split("+")[0].strip()
                cur_obj["ptrs"].append(target)
                continue
            if cur is None:
                continue
            m = INSN_RE.match(line)
            if not m:
                continue
            op, operands = m.group(2), m.group(3)
            for ref in REF_RE.findall(operands):
                if ref not in syms or ref == cur["name"]:
                    continue
                if op in BRANCH_OPS and syms[ref]["type"] == "function":
                    if ref not in cur["callees"]:
                        cur["callees"].append(ref)
                elif ref not in cur["refs"]:
                    cur["refs"].append(ref)
            if op == "bctrl" or op == "bctr":
                cur["indirect"] = True

    # strings: from data refs (object range), plus refs reached through pointer tables
    for f in funcs.values():
        strs = []
        for r in f["refs"]:
            s = syms[r]
            if s["type"] == "function":
                continue
            strs += strings_in(read, s["addr"], s["size"] or 0x40)
        seen = set()
        f["strings"] = [x for x in strs if not (x in seen or seen.add(x))][:40]
    callers = {}
    for f in funcs.values():
        for c in f["callees"]:
            callers.setdefault(c, []).append(f["name"])
    for name, f in funcs.items():
        f["callers"] = callers.get(name, [])

    # vtable-like objects: >= 3 pointers, mostly to functions
    vtables = []
    obj_refs = {}
    for f in funcs.values():
        for r in f["refs"]:
            obj_refs.setdefault(r, []).append(f["name"])
    for o in objs.values():
        fptrs = [p for p in o["ptrs"] if syms.get(p, {}).get("type") == "function"]
        if len(fptrs) >= 3 and len(fptrs) >= 0.6 * len(o["ptrs"]):
            s = syms.get(o["name"], {})
            vtables.append(dict(name=o["name"], addr=s.get("addr"), entries=o["ptrs"],
                                referenced_by=obj_refs.get(o["name"], [])))
    out = dict(functions=funcs, vtables=vtables)
    (root / "dtk/index.json").write_text(json.dumps(out), encoding="utf-8")
    with_str = sum(1 for f in funcs.values() if f["strings"])
    print(f"{len(funcs)} functions, {with_str} reference strings, {len(vtables)} vtable-like tables")


if __name__ == "__main__":
    main()
