"""Propose a unit's data ranges for config/RHAE01/splits.txt from its compiled object.

Usage: propose_ledger.py <unit> [--patch FILE]

Compiles src/<unit> (tools/cccmd.py, lock-free) and, for every data section of the object
(.rodata .data .sdata .sdata2 .bss .sbss), anchors it on the original addresses of its named
symbols (symbols.txt; a static like name$123 also matches the ledger's name$NNN): start = ledger
address - offset in the section, end = start + section size. Prints the proposed block next to the
current one and flags:
  - a range that overlaps another unit's range in the same section;
  - a gap before the next unit's range (padding: decide who owns it, or add an align: hint);
  - sections with no named anchor (run unit.py sync, or rename in symbols.txt, first).
Sections without a unique named symbol (anonymous string/float pools) are located by searching the
DOL for their bytes near the current range; several hits are flagged AMBIGUOUS. A section can also
hold weak copies (header inlines' data) that another unit owns, making it look too big: the
proposal is a starting point to test with tools/wt_test.py, not something to apply blind.
--patch writes the proposal as a unified diff of splits.txt for tools/wt_test.py.
Nothing in the tree is changed.
"""
import argparse
import collections
import difflib
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import cccmd  # noqa: E402
import dolio  # noqa: E402
import objcmp  # noqa: E402
from elf_symbols import read  # noqa: E402

ROOT = cccmd.ROOT
DATA = [".rodata", ".data", ".bss", ".sdata", ".sbss", ".sdata2"]
LINE = re.compile(r"^(\S+) = (\S+):0x([0-9A-Fa-f]+); // (.*)$")
RANGE = re.compile(r"^\s+(\S+)\s+start:0x([0-9A-Fa-f]+)\s+end:0x([0-9A-Fa-f]+)")


def section_info(path):
    d = Path(path).read_bytes()
    shoff, = struct.unpack(">I", d[0x20:0x24])
    shentsize, shnum, shstrndx = struct.unpack(">HHH", d[0x2E:0x34])
    secs = []
    for i in range(shnum):
        o = shoff + i * shentsize
        name, typ, flags, addr, off, size, link, info, align, ent = struct.unpack(">IIIIIIIIII", d[o:o + 40])
        secs.append((name, size, align))
    so = struct.unpack(">IIIIIIIIII", d[shoff + shstrndx * shentsize:shoff + shstrndx * shentsize + 40])[4]
    return {d[so + n:d.index(b"\0", so + n)].decode("latin-1"): (size, align) for n, size, align in secs}


def ledger():
    by_name, by_stem, seen = {}, collections.defaultdict(list), collections.Counter()
    for l in (ROOT / "config/RHAE01/symbols.txt").read_text(encoding="utf-8").splitlines():
        m = LINE.match(l)
        if m:
            seen[m.group(1)] += 1
            by_name[m.group(1)] = (m.group(2), int(m.group(3), 16))
            if "$" in m.group(1):
                by_stem[m.group(1).split("$")[0]].append((m.group(2), int(m.group(3), 16)))
    return {n: v for n, v in by_name.items() if seen[n] == 1}, by_stem


def anchorable(s):
    """own, uniquely named symbols: weak copies (header inlines, templates) and compiler-numbered
    names (@1234, @stringBase0) recur in other units and would anchor on their copies"""
    n = s["name"]
    return n and s["bind"] != "weak" and not n.startswith(("@", "...", ".")) and "@" not in n[1:]


def splits():
    """unit -> {section: (start, end)}, plus the raw lines"""
    lines = (ROOT / "config/RHAE01/splits.txt").read_text(encoding="utf-8").splitlines(keepends=True)
    out, cur = collections.defaultdict(dict), None
    for l in lines:
        if re.match(r"^\S.*:\s*$", l) and not l.startswith("Sections"):
            cur = l.strip()[:-1]
        elif cur and (m := RANGE.match(l)):
            out[cur][m.group(1)] = (int(m.group(2), 16), int(m.group(3), 16))
    return out, lines


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("unit")
    ap.add_argument("--patch")
    a = ap.parse_args()
    obj = cccmd.compile_variant(a.unit)
    if not obj:
        sys.exit(f"{a.unit} does not compile")
    info = section_info(obj)
    syms = read(obj)[0]
    by_name, by_stem = ledger()
    ranges, lines = splits()
    cur = ranges.get(a.unit, {})
    proposal = {}
    print(f"{a.unit}")
    for sec in DATA:
        size, align = info.get(sec, (0, 0))
        if not size:
            if sec in cur:
                print(f"  {sec:8} current {cur[sec][0]:08X}-{cur[sec][1]:08X}, but the object has no {sec}: drop it?")
            continue
        votes = collections.Counter()
        for s in syms:
            if s["section"] != sec or not anchorable(s):
                continue
            hit = by_name.get(s["name"])
            if not hit and "$" in s["name"]:
                cands = [x for x in by_stem.get(s["name"].split("$")[0], []) if x[0] == sec]
                hit = cands[0] if len(cands) == 1 else None
            if hit and hit[0] == sec:
                votes[hit[1] - s["value"]] += 1
        how = "named symbols"
        if not votes and sec not in (".bss", ".sbss"):
            # anonymous pool (strings, float constants): find its bytes in the DOL near the current range
            data = objcmp.sections(obj).get(sec, b"")
            mask = bytearray(b"\xff" * len(data))
            for r in read(obj)[1]:
                if r["section"] == sec:
                    for k in range(r["offset"] & ~3, min(len(data), (r["offset"] & ~3) + 4)):
                        mask[k] = 0
            lo = (cur[sec][0] if sec in cur else 0x80000000) - 0x10000
            hits = dolio.find(data, lo, lo + 0x20000, bytes(mask)) if data else []
            if hits:
                near = min(hits, key=lambda h: abs(h - (cur[sec][0] if sec in cur else h)))
                votes[near] += 1
                how = f"content ({len(hits)} hit(s))" if len(hits) == 1 else \
                    f"content: AMBIGUOUS, {len(hits)} hits, nearest taken; verify by hand"
        if not votes:
            print(f"  {sec:8} size {size:#x}: no unique named symbol and no content hit (sync/rename first)")
            continue
        start, n = votes.most_common(1)[0]
        end = start + size
        proposal[sec] = (start, end)
        note = []
        if len(votes) > 1:
            note.append(f"anchors disagree {dict(votes)}")
        for u, rs in ranges.items():
            if u != a.unit and sec in rs and rs[sec][0] < end and start < rs[sec][1]:
                note.append(f"OVERLAPS {u} {rs[sec][0]:08X}-{rs[sec][1]:08X}")
        nxt = min((rs[sec][0] for u, rs in ranges.items() if u != a.unit and sec in rs and rs[sec][0] >= end),
                  default=None)
        if nxt is not None and nxt > end:
            note.append(f"gap of {nxt - end:#x} before the next unit at {nxt:08X} (padding; object align {align})")
        old = f"{cur[sec][0]:08X}-{cur[sec][1]:08X}" if sec in cur else "none"
        same = sec in cur and cur[sec] == (start, end)
        print(f"  {sec:8} {'same   ' if same else 'PROPOSE'} {start:08X}-{end:08X} (current {old}; by {how})"
              + ("; " + "; ".join(note) if note else ""))
    if a.patch:
        new, unit_hdr, i = [], f"{a.unit}:", 0
        while i < len(lines):
            l = lines[i]
            new.append(l)
            if l.strip() == unit_hdr:
                block = []
                i += 1
                while i < len(lines) and RANGE.match(lines[i]):
                    block.append(lines[i])
                    i += 1
                keep = {RANGE.match(b).group(1): b for b in block}
                for sec, (s0, e0) in proposal.items():
                    keep[sec] = f"\t{sec:<11} start:0x{s0:08X} end:0x{e0:08X}\n"
                order = [".init", "extab", "extabindex", ".text", ".ctors", ".dtors"] + DATA
                new += [keep[k] for k in sorted(keep, key=lambda k: order.index(k) if k in order else 99)]
                continue
            i += 1
        Path(a.patch).write_text("".join(difflib.unified_diff(lines, new, "a/config/RHAE01/splits.txt",
                                                              "b/config/RHAE01/splits.txt")), encoding="utf-8")
        print(f"patch written to {a.patch}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
