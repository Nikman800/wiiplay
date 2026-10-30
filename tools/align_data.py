"""Name library data objects by aligning per-section object-size sequences against references.

Like tools/align_refs.py for functions, but over data sections (.data, .rodata, .sdata, .sdata2,
.bss, .sbss, .sbss2, .ctors, .dtors). Data sizes are less distinctive than code sizes, so runs must
be longer (>= 6 objects, >= 128 bytes). Only real reference names are kept. Writes
dtk/library_data_matches.tsv. Validation: a matched vtable (__vt__<class>) must have entries that
are functions of that class (reported, not filtered).

Usage: align_data.py <root>
"""
import collections
import struct
import re
import sys
from pathlib import Path

SYM = re.compile(r"^(\S+) = (\S+):0x([0-9A-Fa-f]+); // (.*)$")
SECTIONS = {".data", ".rodata", ".sdata", ".sdata2", ".bss", ".sbss", ".sbss2", ".ctors", ".dtors"}
PLACEHOLDER = re.compile(r"^(lbl|DAT|jumptable|@|\$\$|gap|fn)_?|^@|\.\.\.")
GAME_DATA_HINT = re.compile(r"^(Hky|Pnp|Fsh|Duc|Tnk|Wly|Cow|Bil|Bom|Party|Menu|Lyt|Mii|Sys|Gfx|Snd|Mpe|RP)_")
SEED, MIN_OBJS, MIN_BYTES = 6, 6, 128


def objects(path):
    out = collections.defaultdict(list)
    for line in Path(path).read_text(encoding="utf-8", errors="replace").splitlines():
        m = SYM.match(line.strip())
        if not m:
            continue
        name, sec, addr, attrs = m.groups()
        if sec not in SECTIONS or "type:function" in attrs:
            continue
        size = re.search(r"size:0x([0-9A-Fa-f]+)", attrs)
        if not size or int(size.group(1), 16) == 0:
            continue
        out[sec].append((int(addr, 16), int(size.group(1), 16), name))
    for sec in out:
        out[sec].sort()
    return out


def runs(a, b):
    index = collections.defaultdict(list)
    for j in range(len(b) - SEED + 1):
        index[tuple(b[j:j + SEED])].append(j)
    found, seen = [], set()
    for i in range(len(a) - SEED + 1):
        for j in index.get(tuple(a[i:i + SEED]), ()):
            s_i, s_j = i, j
            while s_i > 0 and s_j > 0 and a[s_i - 1] == b[s_j - 1]:
                s_i, s_j = s_i - 1, s_j - 1
            if (s_i, s_j) in seen:
                continue
            seen.add((s_i, s_j))
            e = 0
            while s_i + e < len(a) and s_j + e < len(b) and a[s_i + e] == b[s_j + e]:
                e += 1
            found.append((s_i, s_j, e))
    return found


LIBRARY_UNIT = re.compile(r"^(revolution|rvl|RVL_SDK|nw4r|egg|MSL|MSL_C|runtime|Runtime|MetroTRK|"
                          r"RVLFaceLib|homebuttonMiniLib|NdevExi2AD|NDEV)/")


def ref_data_units(path):
    """[(start, end, unit)] for every section range in a dtk splits.txt."""
    out, cur = [], None
    for line in Path(path).read_text(encoding="utf-8", errors="replace").splitlines():
        m = re.match(r"^(?!Sections:)(\S.*):\s*$", line)
        if m:
            cur = m.group(1)
            continue
        m = re.match(r"^\s+(\S+)\s+start:0x([0-9A-Fa-f]+)\s+end:0x([0-9A-Fa-f]+)", line)
        if m and cur:
            out.append((int(m.group(2), 16), int(m.group(3), 16), cur))
    out.sort()
    return out


def unit_at(ranges, starts, addr):
    import bisect
    k = bisect.bisect_right(starts, addr) - 1
    return ranges[k][2] if k >= 0 and ranges[k][0] <= addr < ranges[k][1] else ""


def game_only_data(root):
    """Data symbols (current address) referenced only by game code, never by library code."""
    import json
    idx = json.loads((root / "dtk/index.json").read_text(encoding="utf-8"))
    orig = {}
    for line in (root / "dtk/symbols.orig.txt").read_text(encoding="utf-8").splitlines():
        m = SYM.match(line.strip())
        if m:
            orig[m.group(1)] = int(m.group(3), 16)
    users = collections.defaultdict(set)
    for f in idx["functions"].values():
        if not f["addr"]:
            continue
        game = 0x80182930 <= f["addr"] < 0x802E9CC0
        for r in f["refs"]:
            if r in orig:
                users[orig[r]].add(game)
    return {a for a, kinds in users.items() if kinds == {True}}


def main():
    root = Path(sys.argv[1])
    ours = objects(root / "config/RHAE01/symbols.txt")
    game_only = game_only_data(root)
    best = {}
    for repo in sorted(p for p in (root / "ref").iterdir() if (p / "symbols.txt").exists()):
        ref = objects(repo / "symbols.txt")
        ranges = ref_data_units(repo / "splits.txt")
        starts = [r[0] for r in ranges]
        kept = 0
        for sec, mine in ours.items():
            theirs = ref.get(sec, [])
            for i, j, n in runs([s for _, s, _ in mine], [s for _, s, _ in theirs]):
                nbytes = sum(s for _, s, _ in mine[i:i + n])
                if n < MIN_OBJS or nbytes < MIN_BYTES:
                    continue
                kept += 1
                for k in range(n):
                    addr, size, name = mine[i + k]
                    raddr, _, rname = theirs[j + k]
                    if PLACEHOLDER.match(rname) or GAME_DATA_HINT.match(name):
                        continue
                    # library data only: the reference object must sit in a library source file,
                    # and ours must not be used exclusively by game code
                    if not LIBRARY_UNIT.match(unit_at(ranges, starts, raddr)) or addr in game_only:
                        continue
                    cand = (n * 1000 + nbytes, repo.name, rname, sec, n)
                    if addr not in best or cand[0] > best[addr][0]:
                        best[addr] = cand
        print(f"{repo.name}: {kept} data runs kept")
    # validation: vtables
    funcs = {}
    for line in (root / "config/RHAE01/symbols.txt").read_text(encoding="utf-8").splitlines():
        m = SYM.match(line.strip())
        if m and "type:function" in m.group(4):
            funcs[int(m.group(3), 16)] = m.group(1)
    read = dol_reader(root / "extracted/sys/main.dol")
    vt_ok = vt_total = 0
    out = root / "dtk/library_data_matches.tsv"
    current = {a: (n, s) for sec in ours for a, s, n in ours[sec]}
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write("address\tsection\tsize\tour_name\tref_name\trepo\trun_objs\n")
        for addr in sorted(best):
            _, repo, rname, sec, n = best[addr]
            name, size = current[addr]
            f.write(f"{addr:08X}\t{sec}\t{size}\t{name}\t{rname}\t{repo}\t{n}\n")
            if rname.startswith("__vt__"):
                # the class's own name is the last length-prefixed component of the mangled name
                parts = re.findall(r"(\d+)", rname[6:])
                cls = mangled_last_name(rname[6:])
                entries = [struct.unpack(">I", read(addr + o, 4))[0] for o in range(8, size, 4)]
                named = [funcs.get(e, "") for e in entries if e]
                if named:
                    vt_total += 1
                    vt_ok += any(cls and cls in fn for fn in named)
    print(f"{len(best)} library data objects matched -> {out}")
    print(f"vtable check: {vt_ok}/{vt_total} matched vtables point at functions of the named class")


def mangled_last_name(s):
    """Last class name in a CodeWarrior mangled qualified name (Q3...4nw4r3g3d6ScnObj -> ScnObj)."""
    s = re.sub(r"^Q\d", "", s)
    last = ""
    while True:
        m = re.match(r"(\d+)", s)
        if not m:
            return last
        n = int(m.group(1))
        s = s[len(m.group(1)):]
        last, s = s[:n], s[n:]


def dol_reader(path):
    data = Path(path).read_bytes()
    offs = struct.unpack_from(">18I", data, 0)
    addrs = struct.unpack_from(">18I", data, 0x48)
    sizes = struct.unpack_from(">18I", data, 0x90)

    def read(addr, n):
        for o, a, s in zip(offs, addrs, sizes):
            if s and a <= addr and addr + n <= a + s:
                return data[o + addr - a:o + addr - a + n]
        return b"\0" * n
    return read


if __name__ == "__main__":
    main()
