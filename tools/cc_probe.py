"""Compile a C/C++ file with candidate CodeWarrior compilers and compare each function's
bytes against the original object from the dtk split (build/RHAE01/obj).

Usage: cc_probe.py <source.c> [--compilers GC/3.0a5.2,Wii/1.0,...] [--flags "..."]
Both objects are unlinked, so relocated fields are zero on both sides and compare equal.
Prints, per compiler, each function: MATCH, or the first differing instruction.
"""
import argparse
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_FLAGS = ("-nodefaults -proc gekko -align powerpc -enum int -fp hardware -Cpp_exceptions off "
                 "-O4,p -inline auto -RTTI off -fp_contract on -str reuse -maxerrors 1 -nosyspath "
                 "-i include")


def elf_functions(path):
    """{name: bytes} for FUNC symbols in an ELF32 big-endian relocatable object."""
    data = Path(path).read_bytes()
    shoff, = struct.unpack_from(">I", data, 0x20)
    shentsize, shnum = struct.unpack_from(">HH", data, 0x2E)
    sections = [struct.unpack_from(">10I", data, shoff + i * shentsize) for i in range(shnum)]
    out = {}
    for sh in sections:
        if sh[1] != 2:  # SHT_SYMTAB
            continue
        strtab = sections[sh[6]]
        for off in range(sh[4], sh[4] + sh[5], 16):
            name_off, value, size, info, _, shndx = struct.unpack_from(">IIIBBH", data, off)
            if info & 0xF != 2 or shndx == 0 or shndx >= 0xFF00:
                continue
            s = data[strtab[4] + name_off:data.index(b"\0", strtab[4] + name_off)].decode()
            sec = sections[shndx]
            out[s] = data[sec[4] + value:sec[4] + value + size]
    return out


def original_functions(names):
    found = {}
    for obj in (ROOT / "build/RHAE01/obj").glob("*.o"):
        funcs = elf_functions(obj)
        for n in names:
            if n in funcs:
                found[n] = funcs[n]
        if len(found) == len(names):
            break
    return found


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("source")
    ap.add_argument("--compilers", default="GC/3.0a5.2,GC/3.0a5,GC/3.0a3.4,Wii/1.0RC1,Wii/1.0a,Wii/1.0,Wii/1.1")
    ap.add_argument("--flags", default=DEFAULT_FLAGS)
    args = ap.parse_args()
    src = Path(args.source).resolve()
    results = {}
    with tempfile.TemporaryDirectory() as tmp:
        for cc in args.compilers.split(","):
            exe = ROOT / "build/compilers" / cc / "mwcceppc.exe"
            obj = Path(tmp) / (cc.replace("/", "_") + ".o")
            proc = subprocess.run([str(exe), *args.flags.split(), "-c", str(src), "-o", str(obj)],
                                  cwd=ROOT, capture_output=True, text=True)
            if proc.returncode != 0:
                print(f"{cc}: compile failed\n{proc.stdout}{proc.stderr}")
                continue
            results[cc] = elf_functions(obj)
    names = sorted({n for funcs in results.values() for n in funcs})
    orig = original_functions(names)
    for cc, funcs in results.items():
        matched = 0
        lines = []
        for n in names:
            a, b = funcs.get(n), orig.get(n)
            if b is None:
                lines.append(f"  {n}: not in original objects")
            elif a == b:
                matched += 1
                lines.append(f"  {n}: MATCH ({len(b)} bytes)")
            else:
                i = next((k for k in range(0, min(len(a), len(b)), 4) if a[k:k + 4] != b[k:k + 4]), min(len(a), len(b)))
                lines.append(f"  {n}: differs at +0x{i:x} (ours {len(a)} bytes, original {len(b)})")
        print(f"{cc}: {matched}/{len(names)} match")
        print("\n".join(lines))


if __name__ == "__main__":
    sys.exit(main())
