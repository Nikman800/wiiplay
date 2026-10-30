"""Propose translation-unit (source file) boundaries for game code.

Evidence (build/tu/signals.tsv from tools/tu_signals.py, constant values from the DOL):

- CodeWarrior pools each unit's anonymous constants (.sdata2 floats/doubles, .rodata) on its
  own, in link order. One unit never holds the same constant value twice, so a function that
  uses value v at address A while the current unit already has v at B != A starts a new unit
  somewhere after the last user of B (a proven split).
- Pools of later units lie at higher addresses: a class whose constants fall inside or below the
  previous class's pool shares its unit (a proven merge).
- Static initializers (__sinit_*, *_sinit*) end a unit. Template copies, thunks and small helpers
  without a class of their own stay with the class before them (CodeWarrior emits them after
  the unit's own functions).
- Otherwise each class starts a unit (one class per file, the RP framework's usual layout).

Usage: tu_boundaries.py <root> [check]
  -> build/tu/units.tsv (start, end, name, classes, evidence). Game units already in
     config/RHAE01/splits.txt (from reference matching) are kept and extended over further
     methods of their classes; `check` reports those extensions.
"""
import bisect
import collections
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from gen_this_map import load_aliases, mangled_class  # noqa: E402

GAME = (0x80182930, 0x802E9CC0)
SYM = re.compile(r"^(\S+) = (\.\w+):0x([0-9A-F]+); // (.*)$")
MINIGAMES = {"Fsh": "Fsh", "Hky": "Hky", "Pnp": "Pnp", "Duc": "Duc", "Tnk": "Tnk", "Wly": "Wly",
             "Cow": "Cow", "Bil": "Bil", "Bom": "Bom"}
# classes whose code CodeWarrior emits into the unit that uses them
HELPER = re.compile(r"(StateMachine\d*|SM|Seq|IBinary|SndActorBase|SndHandleArray|SndPlayerBase|"
                    r"Sound3DBase|Sound3DPart|SoundActorPart|Cls\w*|Trail|SceneSound|SoundHolder)$")


def read_dol(path):
    d = path.read_bytes()
    offs = struct.unpack(">18I", d[0:72])
    addrs = struct.unpack(">18I", d[72:144])
    sizes = struct.unpack(">18I", d[144:216])

    def rd(a, n):
        for o, s, z in zip(offs, addrs, sizes):
            if s <= a and a + n <= s + z:
                return d[o + a - s:o + a - s + n]
        return None
    return rd


def class_key(name, aliases):
    """(key, helper) for grouping: the class a function belongs to, or None for free functions."""
    if re.match(r"^__sinit|_sinit", name) or "sinit" in name.split("_")[1:2]:
        return None, "sinit"
    if "__" in name[1:]:
        c = mangled_class(name)
        # namespaced classes (nw4r::ut::LinkList iterators, RPGrpView::Clipper...) are inline or
        # template copies emitted into the unit that uses them
        nested = re.search(r"__(?:[a-z]\w*__)?Q\d", name) is not None
        return (c.split("<")[0] if c else None), ("template" if c and ("<" in c or nested) else None)
    m = re.match(r"^([A-Za-z]+)_([A-Z][A-Za-z0-9]*)_\w+$", name)
    if not m:
        return None, "free"
    mod, cls = m.groups()
    alias = aliases.get(f"{mod}_{cls}")
    if alias == "-":
        return None, "free"  # not methods of one class (class_aliases.tsv)
    key = alias if alias and alias != "-" else (f"RP{mod}{cls}" if mod in MINIGAMES else f"RP{cls}")
    return key, ("helper" if HELPER.search(cls) else None)


def main():
    root = Path(sys.argv[1])
    check = len(sys.argv) > 2 and sys.argv[2] == "check"
    syms = {}
    for line in open(root / "config/RHAE01/symbols.txt", encoding="utf-8"):
        m = SYM.match(line)
        if m:
            size = re.search(r"size:0x([0-9A-F]+)", m.group(4))
            syms[int(m.group(3), 16)] = (m.group(1), m.group(2), int(size.group(1), 16) if size else 4)
    rd = read_dol(root / "extracted/sys/main.dol")
    aliases = load_aliases(root)

    def anon_value(addr):
        """bytes of an anonymous constant (lbl_/@ symbols) in .sdata2/.rodata, else None"""
        s = syms.get(addr)
        if not s or s[1] not in (".sdata2", ".rodata") or not re.match(r"^(lbl_|@)", s[0]):
            return None
        return (s[1], rd(addr, min(s[2], 64)))

    # nested game classes (Calc__Q29RPGrpView7ClipperFv): inner -> outer
    outer = {}
    for name, _, _ in syms.values():
        m = re.search(r"__Q2(\d+)(RP\w+)", name)
        if m:
            k = int(m.group(1))
            rest = m.group(2)
            o, rest = rest[:k], rest[k:]
            n = re.match(r"(\d+)", rest)
            if n:
                outer[rest[len(n.group(1)):len(n.group(1)) + int(n.group(1))]] = o
    rows = [l.rstrip("\n").split("\t") for l in open(root / "build/tu/signals.tsv", encoding="utf-8")][1:]
    funcs = []
    for r in rows:
        addr, name, size = int(r[0], 16), r[1], int(r[2], 16)
        refs = [int(a, 16) for col in (r[3], r[4]) for a in col.split(",") if a]
        anon = [(a, anon_value(a)) for a in refs]
        anon = [(a, v) for a, v in anon if v and v[1] is not None]
        key, kind = class_key(name, aliases)
        if key in outer:  # a class nested in a game class lives in its outer class's unit
            key, kind = outer[key], None
        funcs.append(dict(addr=addr, end=addr + size, name=name, key=key, kind=kind, anon=anon))

    known = load_known(root)
    kstarts = [k[0] for k in known]

    def known_at(addr):
        i = bisect.bisect_right(kstarts, addr) - 1
        return known[i][2] if i >= 0 and known[i][0] <= addr < known[i][1] else None

    for f in funcs:
        f["known"] = known_at(f["addr"])
    known_classes = collections.defaultdict(set)
    for f in funcs:
        if f["known"] and f["key"]:
            known_classes[f["known"]].add(f["key"])

    # pass 1: runs. A function joins the run before it when it is in the same known (reference)
    # unit, has the same class, has no class of its own (free function, template copy, thunk,
    # helper class), or extends a known unit with another method of one of its classes.
    runs = []
    for f in funcs:
        cur = runs[-1] if runs else None
        if cur is None:
            joins = False
        elif f["known"] or cur["known"] and f["known"] != cur["known"] and f["known"]:
            joins = f["known"] == cur["known"]
        elif cur["known"]:
            joins = f["key"] is None or f["kind"] in ("helper", "template", "sinit") or \
                f["key"] in known_classes[cur["known"]]
        else:
            joins = f["key"] is None or f["kind"] in ("helper", "template", "sinit") or f["key"] == cur["key"]
            if cur.get("after_sinit") and f["key"] is not None and f["kind"] is None:
                joins = False  # a static initializer ends its unit
        if joins:
            cur["funcs"].append(f)
        else:
            runs.append(dict(key=f["key"], known=f["known"], funcs=[f]))
        if f["kind"] == "sinit":
            runs[-1]["after_sinit"] = True

    # a run of one of a known unit's classes right before it belongs to it as well
    k = len(runs) - 1
    while k > 0:
        a, b = runs[k - 1], runs[k]
        if b["known"] and not a["known"] and a["key"] in known_classes[b["known"]] and not a.get("after_sinit"):
            b["funcs"][:0] = a["funcs"]
            del runs[k - 1]
        k -= 1

    # functions before the first class of the game code belong to the unit after them
    if len(runs) > 1 and runs[0]["key"] is None and not runs[0]["known"]:
        runs[1]["funcs"][:0] = runs[0]["funcs"]
        del runs[0]

    # pass 1b: a short run of another class between two runs of the same class is an inline or
    # template copy emitted into that class's unit (A B A -> one run)
    k = 1
    while k + 1 < len(runs):
        a, b, c = runs[k - 1], runs[k], runs[k + 1]
        if not a["known"] and not b["known"] and not c["known"] and a["key"] and a["key"] == c["key"] \
                and len(b["funcs"]) <= 3 and not a.get("after_sinit"):
            a["funcs"] += b["funcs"] + c["funcs"]
            a["after_sinit"] = c.get("after_sinit", False)
            del runs[k:k + 2]
        else:
            k += 1

    # pass 2: merge runs into units, walking the pool evidence (known units never merge)
    units = []
    for run in runs:
        pool = collections.defaultdict(list)  # per section: pools are ordered within a section
        for f in run["funcs"]:
            for a, v in f["anon"]:
                pool[v[0]].append(a)
        conflict = same = False
        if units and not run["known"] and not units[-1]["known"]:
            u = units[-1]
            conflict = any(v in u["vals"] and u["vals"][v] != a for f in run["funcs"] for a, v in f["anon"])
            same = not conflict and any(pool[sec] and u["pool"][sec] and min(pool[sec]) <= max(u["pool"][sec])
                                        for sec in pool)
            if (same or run["key"] is None) and not conflict and not u.get("after_sinit"):
                u["runs"].append(run)
                for sec, addrs in pool.items():
                    u["pool"][sec] += addrs
                for f in run["funcs"]:
                    for a, v in f["anon"]:
                        u["vals"].setdefault(v, a)
                u["why"].append("pool overlap" if same else "no class")
                u["after_sinit"] = run.get("after_sinit", False)
                continue
        vals = {}
        for f in run["funcs"]:
            for a, v in f["anon"]:
                vals.setdefault(v, a)
        units.append(dict(runs=[run], known=run["known"], pool=collections.defaultdict(list, pool), vals=vals,
                          why=["reference unit" if run["known"] else "value conflict" if conflict else "class change"],
                          after_sinit=run.get("after_sinit", False)))

    # names: reference units keep theirs; others take the header path of their first class
    headers = {}
    for h in (root / "include/Pack").rglob("*.h"):
        for m in re.finditer(r"^(?:class|struct)\s+(\w+)\s*(?::[^{;]*)?\{", h.read_text(encoding="utf-8", errors="replace"), re.M):
            headers.setdefault(m.group(1), h.relative_to(root / "include").with_suffix(".cpp").as_posix())
    taken = {u["known"] for u in units if u["known"]}  # reference names are fixed
    used = collections.Counter()
    out = root / "build/tu/units.tsv"
    with open(out, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("start\tend\tname\tclasses\tfunctions\tevidence\n")
        for u in units:
            fs = [f for r in u["runs"] for f in r["funcs"]]
            keys = list(dict.fromkeys(f["key"] for f in fs if f["key"] and f["kind"] is None))
            if u["known"]:
                name = u["known"]
            else:
                main_cls = max(keys, key=lambda c: sum(1 for f in fs if f["key"] == c)) if keys else None
                base = headers.get(main_cls) or (f"Pack/RPParty/unknown/{main_cls or 'free'}_{fs[0]['addr']:08X}.cpp")
                name, n = base, 1
                while name in taken:  # several units for one header: RPHkyLyt.cpp, RPHkyLyt_2.cpp...
                    n += 1
                    name = base[:-4] + f"_{n}.cpp"
                taken.add(name)
            fh.write(f"{fs[0]['addr']:08X}\t{fs[-1]['end']:08X}\t{name}\t{','.join(keys)}\t{len(fs)}\t"
                     f"{';'.join(u['why'])}\n")
    print(f"{len(funcs)} functions, {len(runs)} runs -> {len(units)} units "
          f"({sum(1 for u in units if u['known'])} reference units, extended where their classes continue) -> {out}")
    if check:
        ext = 0
        for u in units:
            if u["known"]:
                fs = [f for r in u["runs"] for f in r["funcs"]]
                ext += sum(1 for f in fs if f["known"] is None)
        unnamed = sum(1 for u in units if not u["known"] and not any(f["key"] for r in u["runs"] for f in r["funcs"]))
        print(f"functions added to reference units: {ext}; units without any class: {unnamed}")


def load_known(root):
    """game-code .text units already in config/RHAE01/splits.txt (from reference matching)"""
    known, cur = [], None
    for l in open(root / "config/RHAE01/splits.txt", encoding="utf-8"):
        m = re.match(r"^(\S.*):\s*$", l)
        if m:
            cur = m.group(1)
            continue
        m = re.match(r"^\s+\.text\s+start:0x([0-9A-F]+)\s+end:0x([0-9A-F]+)", l)
        # reference units only: dtk's auto_/unsplit units and our hand-made pilot unit
        # (Hky/FieldWallIter.cpp, part of the Field file) are re-derived
        if m and cur and GAME[0] <= int(m.group(1), 16) < GAME[1] and not cur.startswith("auto_") \
                and not cur.endswith("_unsplit.o") and cur.startswith(("Pack/", "main.cpp")):
            known.append((int(m.group(1), 16), int(m.group(2), 16), cur))
    return sorted(known)

if __name__ == "__main__":
    main()
