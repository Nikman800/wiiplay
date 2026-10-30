"""Generate plain-C struct layouts for Ghidra from the C++ headers, via CodeWarrior debug info.

1. Writes a translation unit that includes every umbrella header plus a pointer to every class
   defined under include/Pack (so its debug info is emitted), compiles it with -sym on.
2. `dtk dwarf dump` gives each class's members with exact offsets and total size.
3. Converts to C: a class becomes a struct whose first field is its primary base, a secondary base
   is placed in the gap before the first own member, members sit at their dumped offsets with
   explicit padding, and the struct is padded to its total size. vtable pointers become
   `void* __vptr`. Namespaces are dropped by the dump; a later definition reusing a name with a
   different size is emitted as Name_2.

Usage: gen_ghidra_types.py <root>  -> build/ghidra/types.h (+ build/ghidra/types_report.txt)
"""
import collections
import os
import re
import subprocess
import sys
from pathlib import Path

FLAGS = ("-nodefaults -proc gekko -align powerpc -enum int -fp hardware -Cpp_exceptions off -O4,p "
         "-inline auto -RTTI off -fp_contract on -str reuse -maxerrors 1 -nosyspath -enc SJIS "
         "-i include -i include/MSL -i include/MSL/internal -ir include/revolution/BTE -DREVOLUTION "
         "-i include/nw4r -ir include/egg -i include/Pack -DPACK_PARTY -DVERSION_RHAE01 -sym on").split()
UMBRELLAS = ["revolution/OS.h", "revolution/GX.h", "revolution/MTX.h", "revolution/WPAD.h",
             "revolution/KPAD.h", "revolution/DVD.h", "revolution/NAND.h", "revolution/AX.h",
             "nw4r/ut.h", "nw4r/math.h", "nw4r/g3d.h", "nw4r/lyt.h", "nw4r/snd.h", "nw4r/ef.h",
             "egg/core.h", "egg/math.h", "egg/gfx.h", "egg/gfxe.h", "egg/audio.h", "egg/util.h",
             "egg/prim.h", "egg/geom.h", "RVLFaceLib.h", "homebuttonMiniLib.h",
             "Pack/RPSystem.h", "Pack/RPGraphics.h", "Pack/RPKernel.h", "Pack/RPAudio.h",
             "Pack/RPUtility.h"]
PRIM = {"char": "char", "signed char": "char", "unsigned char": "unsigned char", "bool": "unsigned char",
        "short": "short", "unsigned short": "unsigned short", "wchar_t": "unsigned short",
        "int": "int", "unsigned int": "unsigned int", "long": "long", "unsigned long": "unsigned long",
        "long long": "long long", "unsigned long long": "unsigned long long", "float": "float",
        "double": "double", "void": "void"}
DEF = re.compile(r"^(class|struct|union) (.+?)(?: : (.+?))?\s*\{(\}?;?)\s*$")
ENUM = re.compile(r"^enum (\S+) \{")
MEMBER = re.compile(r"^\s+(.+?);\s*// offset 0x([0-9A-Fa-f]+), size 0x([0-9A-Fa-f]+)")
TOTAL = re.compile(r"^// total size: 0x([0-9A-Fa-f]+)")


def ident(name):
    return re.sub(r"[^A-Za-z0-9_]", "_", name.strip())


def parse_dump(text):
    types, enums, order = {}, set(), []
    lines = text.splitlines()
    pending_size, i = None, 0
    while i < len(lines):
        line = lines[i]
        t = TOTAL.match(line)
        if t:
            pending_size = int(t.group(1), 16)
            i += 1
            continue
        e = ENUM.match(line)
        if e:
            enums.add(ident(e.group(1)))
            pending_size = None
            i += 1
            continue
        d = DEF.match(line)
        if not d:
            i += 1
            continue
        kind, name, bases, closed = d.group(1), ident(d.group(2)), d.group(3), d.group(4)
        base_list = [ident(re.sub(r"^(public|private|protected|virtual)\s+", "", b.strip()))
                     for b in (bases.split(",") if bases else [])]
        members = []
        i += 1
        if not closed:
            # anonymous nested structs/unions are printed inline; their inner members carry offsets
            # relative to the nested block, so only depth-0 lines are members of this type and the
            # block's closing line ("} name; // offset .., size ..") is the member itself
            depth = 0
            while i < len(lines) and not (depth == 0 and lines[i].startswith("};")):
                code = lines[i].split("//")[0].rstrip()
                closing = code.strip().startswith("}")
                if closing:
                    depth -= 1
                m = MEMBER.match(lines[i])
                if m and depth == 0:
                    decl = m.group(1)
                    if closing:  # anonymous aggregate member: keep its size as bytes
                        decl = "__anon_t " + re.sub(r"^\}\s*", "", decl).strip()
                    members.append((decl, int(m.group(2), 16), int(m.group(3), 16)))
                if code.endswith("{"):
                    depth += 1
                i += 1
            i += 1
        rec = dict(kind=kind, name=name, bases=base_list, members=members, size=pending_size)
        pending_size = None
        if name in types:
            if types[name]["size"] in (None, 0) and rec["size"]:
                types[name] = rec
            elif rec["size"] and types[name]["size"] != rec["size"] and rec["members"]:
                alt = name + "_2"
                rec["name"] = alt
                types[alt] = rec
                order.append(alt)
            continue
        types[name] = rec
        order.append(name)
    return types, enums, order


def split_decl(decl):
    """('type text', 'name', [dims]) for a member declaration; function pointers -> void*."""
    decl = re.sub(r"\b(class|struct|union|enum|const|volatile)\s+", "", decl).strip()
    if "(*" in decl or "( *" in decl:
        m = re.search(r"\(\s*\*\s*(\w+)", decl)
        return "void*", m.group(1) if m else "fp", []
    dims = re.findall(r"\[(\d+)\]", decl)
    decl = re.sub(r"\[\d+\]", "", decl).strip()
    m = re.match(r"^(.*?)([A-Za-z_$][\w$]*)$", decl)
    if not m:
        return None
    return m.group(1).strip(), m.group(2), dims


def tag(name, types):
    return "union" if types[name]["kind"] == "union" else "struct"


def c_type(t, types, enums, size):
    ptr = t.count("*") + t.count("&")
    base = t.replace("*", "").replace("&", "").strip()
    if base in PRIM:
        return PRIM[base] + "*" * ptr, False
    name = ident(base)
    if ptr:
        return (f"{tag(name, types)} {name}" + "*" * ptr) if name in types else "void" + "*" * ptr, False
    if name in enums:
        return "int", False
    if name in types and types[name]["size"]:
        return f"{tag(name, types)} {name}", name
    return None, False  # unknown by-value type: caller emits bytes


def main():
    root = Path(sys.argv[1])
    out_dir = root / "build/ghidra"
    out_dir.mkdir(parents=True, exist_ok=True)
    classes = set()
    for h in (root / "include/Pack").rglob("*.h"):
        for m in re.finditer(r"^(?:class|struct) (\w+)\s*(?::[^{;]*)?\{", h.read_text(encoding="utf-8", errors="replace"), re.M):
            classes.add(m.group(1))
    tu = [f"#include <{u}>" for u in UMBRELLAS]
    tu += [f"#include <{p.relative_to(root / 'include').as_posix()}>" for p in sorted((root / "include/Pack/RPParty").glob("*.h"))]
    tu += [f"{c}* __gt_{c};" for c in sorted(classes)]
    # template instances named by typedefs (RPStateMachine<T> as RP<X>Seq, ...): a pointer does
    # not instantiate a template, so without sizeof the debug info has only an empty declaration
    typedefs = set()
    for h in (root / "include/Pack").rglob("*.h"):
        typedefs.update(re.findall(r"^\s*typedef\s+[A-Za-z_][\w:]*<[^;{}]*>\s+(\w+)\s*;",
                                   h.read_text(encoding="utf-8", errors="replace"), re.M))
    tu += [f"const unsigned long __gtsz_{t} = sizeof({t});" for t in sorted(typedefs)]
    # template instances whose methods are in game code, when their arguments are numbers
    # (RPGrpModelAnmImplement<1>): mangled names carry them as `Name<args>`
    insts = set()
    for line in open(root / "config/RHAE01/symbols.txt", encoding="utf-8"):
        m = re.match(r"^\S*?__(\d+)([A-Za-z_]\w*<[\d,]+>)F\S* = \.text:0x([0-9A-F]+)", line)
        if m and int(m.group(1)) == len(m.group(2)) and 0x80182930 <= int(m.group(3), 16) < 0x802E9CC0:
            insts.add(m.group(2))
    # a non-type parameter of enum type needs its value cast (template <RPGrpModel::Kind K>)
    params = {}
    for h in (root / "include/Pack").rglob("*.h"):
        for m in re.finditer(r"template\s*<([^<>]*)>\s*(?:class|struct)\s+(\w+)",
                             h.read_text(encoding="utf-8", errors="replace")):
            params.setdefault(m.group(2), [p.strip().rsplit(" ", 1)[0] for p in m.group(1).split(",")])

    def spelled(inst):
        name, args = inst[:-1].split("<")
        types = params.get(name, [])
        vals = args.split(",")
        return f"{name}<" + ",".join(
            f"({types[k]}){v}" if k < len(types) and types[k] not in ("int", "s32", "u32", "u16", "u8") else v
            for k, v in enumerate(vals)) + ">"
    # sizeof instantiates; a pointer variable then puts the complete type into the debug info
    tu += [f"const unsigned long __gtsz_inst{i} = sizeof({spelled(t)}); {spelled(t)}* __gtp_inst{i};"
           for i, t in enumerate(sorted(insts))]
    tu += [f"{t}* __gtp_{t};" for t in sorted(typedefs)]
    # library classes live in namespaces: take their qualified names from mangled method symbols
    qualified = set()
    for line in open(root / "config/RHAE01/symbols.txt", encoding="utf-8"):
        m = re.match(r"^\S*?__Q(\d)(\w+?)F", line)
        if not m:
            continue
        rest, parts = m.group(2), []
        for _ in range(int(m.group(1))):
            n = re.match(r"(\d+)", rest)
            if not n:
                break
            k = int(n.group(1))
            rest = rest[len(n.group(1)):]
            parts.append(rest[:k])
            rest = rest[k:]
        if len(parts) == int(m.group(1)) and all(re.match(r"^[A-Za-z_]\w*$", p) for p in parts):
            qualified.add("::".join(parts))
    tu += [f"{q}* __gtq_{i};" for i, q in enumerate(sorted(qualified))]
    cpp = out_dir / "types_tu.cpp"
    obj = out_dir / "types_tu.o"
    cpp.write_text("\n".join(tu) + "\n", encoding="utf-8")
    exe = root / "build/compilers/GC/3.0a5.2/mwcceppc.exe"
    flags = [f for f in FLAGS if f not in ("-maxerrors", "1")] + ["-maxerrors", "5000"]
    for _ in range(12):
        # off Windows the compiler runs under wibo, as in the build (configure.py downloads it)
        wrap = [] if os.name == "nt" else [str(root / "build/tools/wibo")]
        proc = subprocess.run([*wrap, str(exe), *flags, "-c", str(cpp), "-o", str(obj)], cwd=root,
                              capture_output=True, text=True, errors="replace")
        if proc.returncode == 0:
            break
        # pointer declarations that fail (class unreachable from the umbrellas, template, ...)
        # are dropped by the TU line number the error reports
        bad = set()
        for num, text in re.findall(r"#\s+(\d+): (.*)", proc.stdout):
            k = int(num) - 1
            # the compiler may cut the echoed line short at either end
            if 0 <= k < len(tu) and "__gt" in tu[k] and text.strip() and text.strip() in tu[k]:
                bad.add(k)
        if not bad:
            print(proc.stdout[-3000:])
            return 1
        tu = [l for k, l in enumerate(tu) if k not in bad]
        cpp.write_text("\n".join(tu) + "\n", encoding="utf-8")
    else:
        print(proc.stdout[-3000:])
        return 1
    dump = out_dir / "types_dump.h"
    subprocess.run([str(root / ("build/tools/dtk.exe" if os.name == "nt" else "build/tools/dtk")), "dwarf", "dump", str(obj), "-o", str(dump)],
                   cwd=root, capture_output=True)
    types, enums, order = parse_dump(dump.read_text(encoding="utf-8", errors="replace"))

    emitted, out, report = set(), [], collections.Counter()

    def emit(name, stack=()):
        if name in emitted or name in stack or name not in types:
            return
        rec = types[name]
        for b in rec["bases"]:
            emit(b, stack + (name,))
        for decl, _, _ in rec["members"]:
            sd = split_decl(decl)
            if sd:
                ct, dep = c_type(sd[0], types, enums, 0)
                if dep:
                    emit(dep, stack + (name,))
        fields, cursor = [], 0
        bases = [b for b in rec["bases"] if b in types and types[b]["size"]]
        own = [o for _, o, _ in rec["members"] if not (o == 0 and bases)]
        if bases:
            fields.append(f"    {tag(bases[0], types)} {bases[0]} base; // 0x0")
            cursor = types[bases[0]]["size"]
            first_own = min([o for _, o, _ in rec["members"]] + [rec["size"] or cursor])
            for b in bases[1:]:
                off = (cursor + 3) & ~3
                if off + types[b]["size"] <= first_own:
                    fields.append(f"    {tag(b, types)} {b} base_{b}; // 0x{off:X}")
                    cursor = off + types[b]["size"]
                    report["secondary bases placed"] += 1
                else:
                    report["secondary bases dropped"] += 1
        pad = 0
        for decl, off, size in rec["members"]:
            if size == 0:
                report["zero-size members skipped"] += 1  # take no space; C parsers reject [0]
                continue
            if off < cursor:
                report["overlapping members skipped"] += 1
                continue
            if off > cursor:
                fields.append(f"    char __pad{pad}[0x{off - cursor:X}];")
                pad += 1
            sd = split_decl(decl)
            ct = None
            if sd:
                ct, _ = c_type(sd[0], types, enums, size)
            if sd and sd[1] == "__vptr$":
                fields.append(f"    void* __vptr; // 0x{off:X}")
            elif ct:
                dims = "".join(f"[{d}]" for d in sd[2])
                fields.append(f"    {ct} {ident(sd[1])}{dims}; // 0x{off:X}")
            else:
                fields.append(f"    unsigned char {ident(sd[1]) if sd else 'unk'}_{off:X}[0x{size:X}]; // 0x{off:X}")
                report["unknown by-value types as bytes"] += 1
            cursor = off + size
        total = rec["size"] or cursor
        if total > cursor:
            fields.append(f"    char __pad{pad}[0x{total - cursor:X}];")
        kw = tag(name, types)
        if not fields:
            fields.append(f"    char __empty[0x{max(total, 1):X}];")
        out.append(f"{kw} {name} {{ // size 0x{total:X}\n" + "\n".join(fields) + "\n};")
        emitted.add(name)
        report["structs"] += 1

    for name in order:
        emit(name)
    header = ["/* Generated by tools/gen_ghidra_types.py from CodeWarrior debug info. Do not edit. */"]
    # forward declarations with the right keyword: a union declared as a struct cannot be
    # replaced by its definition in Ghidra
    header += [f"{tag(n, types)} {n};" for n in order if n in emitted]
    (out_dir / "types.h").write_text("\n".join(header + out) + "\n", encoding="utf-8", newline="\n")
    (out_dir / "types_report.txt").write_text("\n".join(f"{k}: {v}" for k, v in report.items()) + "\n")
    print(", ".join(f"{k}: {v}" for k, v in report.items()), "->", out_dir / "types.h")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
