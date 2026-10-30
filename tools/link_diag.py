"""Why a unit's check fails: link it from source (under the build lock), then compare the DOL's
sections with the original and list the unit's symbols whose size or presence differ from the
original's split object. Leaves config/RHAE01/matching.txt as it was.

Usage: link_diag.py <unit, e.g. Pack/RPParty/RPFshScene/RPFshGame.cpp>
"""
import bisect
import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
sys.argv, args = [sys.argv[0]], sys.argv[1:]
import unit  # noqa: E402
from elf_symbols import symbols  # noqa: E402


def sections(path):
    d = path.read_bytes()
    addrs = struct.unpack(">18I", d[72:144])
    sizes = struct.unpack(">18I", d[144:216])
    return [(a, s) for a, s in zip(addrs, sizes) if s]


def main():
    name = args[0]
    stem = name.rsplit(".", 1)[0]
    listing = ROOT / "config/RHAE01/matching.txt"
    with unit.build_lock(f"link_diag {name}"):
        before = listing.read_text(encoding="utf-8")
        listing.write_text(before.rstrip("\n") + f"\n{name}\n", encoding="utf-8", newline="\n")
        try:
            subprocess.run([unit.PY, "configure.py"], cwd=ROOT, capture_output=True)
            r = subprocess.run([unit.ninja_exe(), "build/RHAE01/main.dol"], cwd=ROOT, capture_output=True, text=True)
            if r.returncode:
                print("\n".join(l for l in r.stdout.splitlines() if "Error" in l or l.startswith("#"))[:3000])
                return 1
            got = sections(ROOT / "build/RHAE01/main.dol")
            linked = (ROOT / "build/RHAE01/main.dol").read_bytes()
            (ROOT / "build/link_diag.dol").write_bytes(linked)  # kept for a closer look
        finally:
            listing.write_text(before, encoding="utf-8", newline="\n")
            subprocess.run([unit.PY, "configure.py"], cwd=ROOT, capture_output=True)
            subprocess.run([unit.ninja_exe()], cwd=ROOT, capture_output=True)
    want = sections(ROOT / "extracted/sys/main.dol")
    for (a, s), (b, t) in zip(want, got):
        if (a, s) != (b, t):
            print(f"  DOL section {a:#x} size {s:#x} -> linked at {b:#x} size {t:#x} ({t - s:+#x})")
    if want == got:
        # same layout: list the original symbols whose bytes differ (first 15)
        orig_dol = (ROOT / "extracted/sys/main.dol").read_bytes()
        offs = struct.unpack(">18I", orig_dol[0:72])
        addrs = struct.unpack(">18I", orig_dol[72:144])
        sizes = struct.unpack(">18I", orig_dol[144:216])
        syms = []
        for line in open(ROOT / "config/RHAE01/symbols.txt", encoding="utf-8"):
            m = re.match(r"^(\S+) = (\S+):0x([0-9A-F]+); // .*?size:0x([0-9A-F]+)", line)
            if m:
                syms.append((int(m.group(3), 16), int(m.group(4), 16), m.group(1)))
        syms.sort()
        starts = [s[0] for s in syms]
        hits = {}
        for o, a, z in zip(offs, addrs, sizes):
            for i in range(o, o + z):
                if orig_dol[i] != linked[i]:
                    va = a + i - o
                    k = bisect.bisect_right(starts, va) - 1
                    name = syms[k][2] if k >= 0 and va < syms[k][0] + syms[k][1] else f"{va:#x}"
                    hits.setdefault(name, va)
                    if len(hits) >= 15:
                        break
            if len(hits) >= 15:
                break
        if not hits:
            print("  linked DOL matches the original")
        for n, va in hits.items():
            print(f"  bytes differ in {n} (first at {va:#x})")
    mine = {x["name"]: x for x in symbols(ROOT / f"build/RHAE01/src/{stem}.o") if x["size"]}
    orig = {x["name"]: x for x in symbols(ROOT / f"build/RHAE01/obj/{stem}.o") if x["size"]}
    for n, x in sorted(mine.items(), key=lambda kv: kv[1]["section"]):
        o = orig.get(n)
        if x.get("bind") == "weak" or x["section"] == ".text":
            continue
        if o is None:
            print(f"  only in source: {x['section']} {n} size {x['size']:#x}")
        elif o["size"] != x["size"]:
            print(f"  size differs: {x['section']} {n} source {x['size']:#x}, original {o['size']:#x}")
    for n, o in orig.items():
        if n not in mine and o["section"] != ".text":
            print(f"  only in original: {o['section']} {n} size {o['size']:#x}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
