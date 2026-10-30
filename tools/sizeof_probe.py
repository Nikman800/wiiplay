"""Compare class sizes from the headers (as CodeWarrior lays them out) with sizes observed in Wii Play.

Generates a C++ file that stores sizeof(Class) in an exported constant per class, compiles it with
the RP framework flags, reads the constants back from the object file, and compares them with the
expected sizes (from `operator new(size)` calls followed by that class's constructor).

Usage: sizeof_probe.py <expected.tsv> [--compiler GC/3.0a5.2] [--include <header>]...
  expected.tsv: class<TAB>size_hex<TAB>header (header relative to include/Pack or include/)
"""
import argparse
import re
import struct
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FLAGS = ("-nodefaults -proc gekko -align powerpc -enum int -fp hardware -Cpp_exceptions off -O4,p "
         "-inline auto -RTTI off -fp_contract on -str reuse -maxerrors 1 -nosyspath -enc SJIS "
         "-i include -i include/MSL -i include/MSL/internal -ir include/revolution/BTE -DREVOLUTION "
         "-i include/nw4r -ir include/egg -i include/Pack -DPACK_PARTY -DVERSION_RHAE01").split()


def object_constants(path):
    """{symbol: u32 value} for data symbols in an ELF32 big-endian relocatable object."""
    data = Path(path).read_bytes()
    shoff, = struct.unpack_from(">I", data, 0x20)
    shentsize, shnum = struct.unpack_from(">HH", data, 0x2E)
    secs = [struct.unpack_from(">10I", data, shoff + i * shentsize) for i in range(shnum)]
    out = {}
    for sh in secs:
        if sh[1] != 2:
            continue
        strtab = secs[sh[6]]
        for off in range(sh[4], sh[4] + sh[5], 16):
            name_off, value, size, info, _, shndx = struct.unpack_from(">IIIBBH", data, off)
            if info & 0xF != 1 or shndx == 0 or shndx >= 0xFF00 or size != 4:
                continue
            name = data[strtab[4] + name_off:data.index(b"\0", strtab[4] + name_off)].decode()
            sec = secs[shndx]
            if sec[1] == 8:  # NOBITS
                continue
            out[name] = struct.unpack_from(">I", data, sec[4] + value)[0]
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("expected")
    ap.add_argument("--compiler", default="GC/3.0a5.2")
    args = ap.parse_args()
    rows = [l.split("\t") for l in Path(args.expected).read_text(encoding="utf-8").splitlines()
            if l.strip() and not l.startswith("#")]
    headers = sorted({r[2].strip() for r in rows})
    skipped = {}  # class -> reason the headers can't size it (incomplete, template)
    exe = ROOT / "build/compilers" / args.compiler / "mwcceppc.exe"
    with tempfile.TemporaryDirectory() as tmp:
        cpp = Path(tmp) / "sizeof_probe.cpp"
        obj = Path(tmp) / "sizeof_probe.o"
        for _ in range(len(rows) + 1):
            src = [f"#include <{h}>" for h in headers]
            src += [f'extern "C" const unsigned long SIZEOF_{r[0]};\n'
                    f'const unsigned long SIZEOF_{r[0]} = sizeof({r[0]});' for r in rows if r[0] not in skipped]
            cpp.write_text("\n".join(src) + "\n", encoding="utf-8")
            proc = subprocess.run([str(exe), *FLAGS, "-c", str(cpp), "-o", str(obj)],
                                  cwd=ROOT, capture_output=True, text=True, errors="replace")
            if proc.returncode == 0:
                break
            out = proc.stdout + proc.stderr
            line = re.search(r"SIZEOF_(\w+) = sizeof", out)
            reason = re.search(r"\(\d+\) ([^\n]+)", out)
            if not line or line.group(1) in skipped:
                print(out[-3000:])
                return 1
            skipped[line.group(1)] = reason.group(1).strip() if reason else "compile error"
        sizes = object_constants(obj)
    match = 0
    for cls, exp, _ in rows:
        if cls in skipped:
            print(f"SKIP {cls:32} wii play 0x{int(exp, 16):X}  header: {skipped[cls]}")
            continue
        got = sizes.get(f"SIZEOF_{cls}")
        exp_i = int(exp, 16)
        ok = got == exp_i
        match += ok
        print(f"{'OK ' if ok else 'DIFF'} {cls:32} wii play 0x{exp_i:X}  header {('0x%X' % got) if got is not None else '?'}")
    print(f"{match}/{len(rows) - len(skipped)} comparable class sizes match ({len(skipped)} skipped)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
