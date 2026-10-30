"""Compare functions of two CodeWarrior objects word by word, with relocation fields masked.

Fast and lock-free (no objdiff, no ninja): the scorer behind tools/flag_sweep.py and
tools/permute.py. Functions pair by name; a relocated field (branch target, @ha/@l, sda21, ADDR32)
compares equal whatever symbol it names, so only real code differences count.

Usage: objcmp.py <target.o> <source.o> [function ...]
"""
import difflib
from collections import Counter
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from elf_symbols import read  # noqa: E402

# R_PPC_* relocation types -> mask of the bits the linker fills in (applied to the 32-bit word)
MASK = {1: 0xFFFFFFFF,           # ADDR32
        4: 0x0000FFFF, 5: 0x0000FFFF, 6: 0x0000FFFF,  # ADDR16_LO/HI/HA (offset is the halfword)
        10: 0x03FFFFFC,          # REL24
        11: 0x0000FFFC,          # REL14
        109: 0x001FFFFF}         # EMB_SDA21


def sections(path):
    d = Path(path).read_bytes()
    shoff, = struct.unpack(">I", d[0x20:0x24])
    shentsize, shnum, shstrndx = struct.unpack(">HHH", d[0x2E:0x34])
    secs = []
    for i in range(shnum):
        o = shoff + i * shentsize
        name, typ, _, _, off, size = struct.unpack(">IIIIII", d[o:o + 24])
        secs.append([name, typ, off, size])
    so = secs[shstrndx][2]
    out = {}
    for name, typ, off, size in secs:
        n = d[so + name:d.index(b"\0", so + name)].decode("latin-1")
        out[n] = d[off:off + size] if typ != 8 else bytes(size)  # NOBITS (.bss) reads as zeros
    return out


class Obj:
    def __init__(self, path):
        self.syms, self.relocs = read(path)
        self.secs = sections(path)
        self.funcs = {s["name"]: s for s in self.syms if s["type"] == "func" and s["size"]}

    def words(self, name):
        """the function's instruction words with relocated fields zeroed, each paired with the
        relocation's target when that target has a real name (generated names like lbl_X, @N or
        name$N compare equal to anything, since the two sides number them differently)"""
        s = self.funcs[name]
        code = bytearray(self.secs[s["section"]][s["value"]:s["value"] + s["size"]])
        tags = {}
        for r in self.relocs:
            if r["section"] != s["section"] or not (s["value"] <= r["offset"] < s["value"] + s["size"]):
                continue
            k = (r["offset"] - s["value"]) & ~3
            w, = struct.unpack(">I", code[k:k + 4])
            code[k:k + 4] = struct.pack(">I", w & ~MASK.get(r["type"], 0xFFFFFFFF))
            tags[k] = (r["symbol"], r["addend"])
        return [(bytes(code[i:i + 4]), tags.get(i)) for i in range(0, len(code), 4)]


def real_name(n):
    if not n or n.startswith(("@", "lbl_", "jumptable_", "...", ".")) or "$" in n or "@" in n[1:]:
        return None
    return n


def _aligned(a, b):
    m = difflib.SequenceMatcher(None, [x[0] for x in a], [y[0] for y in b], autojunk=False)
    for blk in m.get_matching_blocks():
        for k in range(blk.size):
            yield a[blk.a + k], b[blk.b + k]


def reloc_map(target, source):
    """source symbol -> the target symbol it lines up with most often, over every function of the
    unit (pairs generated target names like lbl_8043BD4C with the source's real names, so a load
    or store of the wrong variable shows up even where the target is unnamed)"""
    votes = {}
    for n in target.funcs:
        if n in source.funcs:
            for x, y in _aligned(target.words(n), source.words(n)):
                if x[1] and y[1]:
                    votes.setdefault(y[1], Counter())[x[1]] += 1
    return {s: c.most_common(1)[0][0] for s, c in votes.items()}


def same(x, y, rmap=None):
    """x from the target, y from the source"""
    if x[0] != y[0]:
        return False
    if not (x[1] and y[1]):
        return True
    rx, ry = real_name(x[1][0]), real_name(y[1][0])
    if rx and ry:
        return x[1] == y[1]
    if rmap and y[1] in rmap:
        return rmap[y[1]] == x[1]
    return True


def compare(target, source, name, rmap=None):
    """(matching words, max of both lengths): words are aligned like a diff on the masked bytes
    (difflib); aligned words also need the same relocation target (by name, or through rmap)"""
    a = target.words(name)
    if name not in source.funcs:
        return 0, len(a)
    b = source.words(name)
    return sum(1 for x, y in _aligned(a, b) if same(x, y, rmap)), max(len(a), len(b))


def score(target, source, names=None):
    """fraction of matching words over the given functions (default: all target functions)"""
    names = names or list(target.funcs)
    rmap = reloc_map(target, source)
    eq = tot = 0
    for n in names:
        e, t = compare(target, source, n, rmap)
        eq += e
        tot += t
    return eq / tot if tot else 0.0


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 1
    t, s = Obj(sys.argv[1]), Obj(sys.argv[2])
    names = sys.argv[3:] or sorted(t.funcs, key=lambda n: t.funcs[n]["value"])
    rmap = reloc_map(t, s)
    for n in names:
        e, tot = compare(t, s, n, rmap)
        flag = "OK" if e == tot and n in s.funcs and len(s.words(n)) == tot else "~ "
        size_note = "" if n not in s.funcs or len(s.words(n)) == tot else f" (source {len(s.words(n))} words)"
        print(f"  {flag} {e:5}/{tot:<5} {n}{size_note}" if n in s.funcs else f"  -- missing in source: {n}")
    print(f"  score {score(t, s, names):.4f}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
