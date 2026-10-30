"""Add data-section ranges to the named source-file units in config/RHAE01/splits.txt.

A data object referenced by exactly one unit is taken to belong to that unit; a vtable belongs to
the unit with its destructor (slot 8), where CodeWarrior emits it. Each unit gets, per
data section, the span of its exclusively referenced objects. Spans that overlap a neighbour or
break the units' code (link) order are dropped and left to dtk's auto-splitting. Run after
tools/make_lib_splits.py and a dtk split (needs build/RHAE01/asm).

Usage: add_data_splits.py <root>
"""
import bisect
import collections
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from rhae_libs import RHAE_LIBS  # noqa: E402

SYM = re.compile(r"(\S+) = (\S+):0x([0-9A-F]+); // (.*)")
UNIT = re.compile(r"^(?!Sections:)(\S.*):\s*$")
RANGE = re.compile(r"^\s+(\S+)\s+start:0x([0-9A-F]+)\s+end:0x([0-9A-F]+)")
TOKEN = re.compile(r'"[^"]+"|[A-Za-z_.$][\w.$]*')
CODE = {".text", ".init"}
# constructor/destructor tables and exception tables: dtk lays these out and validates them itself
SKIP = {".ctors", ".dtors", "extab", "extabindex"}
# units of the libraries laid out like Rhae (tools/rhae_libs.py) outside game code keep the data
# ranges written for them; their units placed among the game units are laid out like those
GAME = (0x801827F4, 0x802E9CC0)


def read_dol(path):
    """address -> 32-bit word reader over the DOL's sections"""
    d = path.read_bytes()
    offs = struct.unpack(">18I", d[0:72])
    addrs = struct.unpack(">18I", d[72:144])
    sizes = struct.unpack(">18I", d[144:216])

    def word(a):
        for o, s, z in zip(offs, addrs, sizes):
            if s <= a < s + z - 3:
                return struct.unpack(">I", d[o + a - s:o + a - s + 4])[0]
        return None
    return word


def main():
    root = Path(sys.argv[1])
    syms = {}
    for line in open(root / "config/RHAE01/symbols.txt", encoding="utf-8"):
        m = SYM.match(line)
        if m and "type:function" not in m.group(4) and m.group(2) not in CODE | SKIP:
            size = re.search(r"size:0x([0-9A-F]+)", m.group(4))
            if size and int(size.group(1), 16) > 0:  # zero-size labels would give empty spans
                syms[m.group(1)] = (m.group(2), int(m.group(3), 16), int(size.group(1), 16))
    path = root / "config/RHAE01/splits.txt"
    lines = path.read_text(encoding="utf-8").splitlines()
    units = [m.group(1) for m in map(UNIT.match, lines) if m and not m.group(1).startswith("auto_")]
    order = {u: i for i, u in enumerate(units)}
    refs = collections.defaultdict(set)
    asm = root / "build/RHAE01/asm"
    for u in units:
        p = asm / (u.rsplit(".", 1)[0] + ".s")
        if p.exists():
            for t in {x.strip('"') for x in TOKEN.findall(p.read_text(encoding="utf-8", errors="replace"))}:
                if t in syms:
                    refs[t].add(u)
    # A vtable belongs to the file that defines the class's out-of-line virtuals, which is where
    # CodeWarrior emits it, not to the file whose (often inlined) constructor stores it. The
    # destructor slot (offset 8) is the class's own function, so its unit owns the vtable.
    text = []
    cur = None
    for line in lines:
        m = UNIT.match(line)
        if m:
            cur = m.group(1)
            continue
        r = RANGE.match(line)
        if r and cur and r.group(1) == ".text" and not cur.startswith("auto_"):
            text.append((int(r.group(2), 16), int(r.group(3), 16), cur))
    text.sort()
    tstarts = [t[0] for t in text]
    dol = read_dol(root / "extracted/sys/main.dol")
    vt_addrs = set()
    classes = root / "dtk/classes.tsv"
    if classes.exists():
        vt_addrs = {int(l.split("\t")[0], 16) for l in open(classes, encoding="utf-8").read().splitlines()[1:] if l}
    by_addr = {(sec, a): s for s, (sec, a, _) in syms.items()}
    moved = 0
    for a in vt_addrs:
        s = by_addr.get((".data", a))
        if s is None:
            continue
        dtor = dol(a + 8)
        k = bisect.bisect_right(tstarts, dtor) - 1 if dtor else -1
        if k >= 0 and dtor < text[k][1] and refs.get(s) != {text[k][2]}:
            refs[s] = {text[k][2]}
            moved += 1
    # dtk/data_owners.tsv (start, end, unit, note): ranges a decompiled unit is known to own,
    # from matching it (its strings, tables, statics); they override the reference evidence
    owners = root / "dtk/data_owners.tsv"
    forced = 0
    if owners.exists():
        for l in owners.read_text(encoding="utf-8").splitlines()[1:]:
            f = l.split("\t")
            if len(f) < 3 or not f[0].strip():
                continue
            lo, hi, u = int(f[0], 16), int(f[1], 16), f[2].strip()
            if u == "-":  # shared data no unit owns (e.g. one referenced from a single unit's extern)
                for s, (sec, a, size) in syms.items():
                    if lo <= a < hi:
                        refs.pop(s, None)
                continue
            if u not in order:  # a unit renamed or merged since the row was written
                print(f"  data_owners.tsv: no unit {u} ({f[0]}-{f[1]}); row ignored")
                continue
            for s, (sec, a, size) in syms.items():
                if lo <= a < hi:
                    refs[s] = {u}
                    forced += 1
    spans = collections.defaultdict(dict)
    for s, us in refs.items():
        if len(us) != 1:
            continue
        u = next(iter(us))
        sec, a, size = syms[s]
        r = spans[sec].setdefault(u, [a, a + size])
        r[0], r[1] = min(r[0], a), max(r[1], a + size)
    # Unit sections must start aligned: extend each end to the next object when only padding
    # lies between (< 0x20), else round up to 4; drop spans whose start is unaligned (their first
    # object probably belongs to the tail of the previous unit).
    starts = collections.defaultdict(list)
    for sec, a, _ in syms.values():
        starts[sec].append(a)
    for sec in starts:
        starts[sec].sort()
    unaligned = 0
    for sec, d in spans.items():
        for u in list(d):
            r = d[u]
            if r[0] % 4:
                del d[u]
                unaligned += 1
                continue
            k = bisect.bisect_left(starts[sec], r[1])
            nxt = starts[sec][k] if k < len(starts[sec]) else None
            rounded = (r[1] + 3) & ~3
            if nxt is not None and nxt % 4 and nxt < rounded:
                # an unaligned object packed right after the span most likely belongs to it too
                del d[u]
                unaligned += 1
                continue
            r[1] = nxt if nxt is not None and nxt % 4 == 0 and nxt - r[1] < 0x20 else rounded
    # nw4r units carry Rhae's data ranges as written: keep them, and drop other spans they overlap
    fixed_units, fixed, cur = set(), collections.defaultdict(list), None
    for line in lines:
        m, r = UNIT.match(line), RANGE.match(line)
        if m:
            cur = m.group(1)
        elif r and cur and cur.startswith(RHAE_LIBS) and r.group(1) == ".text":
            if not GAME[0] <= int(r.group(2), 16) < GAME[1]:
                fixed_units.add(cur)
    cur = None
    for line in lines:
        m, r = UNIT.match(line), RANGE.match(line)
        if m:
            cur = m.group(1)
        elif r and cur in fixed_units and r.group(1) not in CODE | SKIP:
            fixed[r.group(1)].append((int(r.group(2), 16), int(r.group(3), 16)))
    for sec, d in spans.items():
        for u in list(d):
            if u in fixed_units or any(a < d[u][1] and d[u][0] < b for a, b in fixed[sec]):
                if u not in fixed_units:
                    unaligned += 1
                del d[u]
    keep = collections.defaultdict(dict)
    dropped = unaligned
    for sec, d in spans.items():
        rs = sorted(d.items(), key=lambda kv: kv[1][0])
        bad = set()
        for (u1, r1), (u2, r2) in zip(rs, rs[1:]):
            if r2[0] < r1[1] or order[u2] < order[u1]:
                bad.update((u1, u2))
        for u, r in rs:
            if u in bad:
                dropped += 1
            else:
                keep[u][sec] = r
        # dtk will not auto-split tiny gaps between two splits: give a small gap (< 0x20) to the
        # earlier unit, most plausibly an unreferenced variable of that file.
        kept = sorted(((u, r) for u, r in rs if u not in bad), key=lambda kv: kv[1][0])
        for (u1, r1), (u2, r2) in zip(kept, kept[1:]):
            if 0 < r2[0] - r1[1] < 0x20 and not any(r1[1] <= a < r2[0] for a, _ in fixed[sec]):
                r1[1] = r2[0]
    # rewrite: strip existing data lines from named units, then add the kept spans
    out, cur = [], None
    for line in lines:
        m = UNIT.match(line)
        if m:
            if cur in keep:
                out += [f"\t{sec:<11} start:0x{a:08X} end:0x{b:08X}" for sec, (a, b) in sorted(keep[cur].items(), key=lambda kv: kv[1][0])]
            cur = m.group(1)
            out.append(line)
            continue
        r = RANGE.match(line)
        if r and cur and not cur.startswith("auto_") and cur not in fixed_units and r.group(1) not in CODE:
            continue
        if line.strip() == "" and cur in keep:
            out += [f"\t{sec:<11} start:0x{a:08X} end:0x{b:08X}" for sec, (a, b) in sorted(keep[cur].items(), key=lambda kv: kv[1][0])]
            keep.pop(cur)
        out.append(line)
    if cur in keep:
        out += [f"\t{sec:<11} start:0x{a:08X} end:0x{b:08X}" for sec, (a, b) in sorted(keep[cur].items(), key=lambda kv: kv[1][0])]
    path.write_text("\n".join(out) + "\n", encoding="utf-8", newline="\n")
    total = sum(len(v) for v in spans.values())
    print(f"{total} unit/section data spans from exclusive references; kept {total - dropped}, dropped {dropped} "
          f"(conflicts); {moved} vtables assigned to their class's unit; {forced} objects from dtk/data_owners.tsv")


if __name__ == "__main__":
    main()
