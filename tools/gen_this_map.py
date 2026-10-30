"""Map every function to the struct its `this` points to, for Ghidra typing (tools/ApplyTypes).

- CodeWarrior-mangled methods: the class is encoded after the first "__" (or after __ct__/__dt__):
  `calc__10RPSysSceneFv` -> RPSysScene, `Calc__Q34nw4r3g3d6ScnObjFv` -> ScnObj (namespaces are
  dropped, as in the generated types). Names whose encoding starts with F are free functions.
- Project names `<Mod>_<Class>_<method>` -> `RP<Mod><Class>` for minigames, `RP<Class>` for
  framework modules (the header naming convention), unless dtk/types/class_aliases.tsv maps the
  ledger class to the header that differs from the convention.
Only classes that exist as structs in build/ghidra/types.h are mapped.

Usage: gen_this_map.py <root>  -> build/ghidra/this_map.tsv (function<TAB>struct)
"""
import re
import sys
from pathlib import Path

FRAMEWORK = {"RP", "Gfx", "Sys", "Snd", "Mpe", "Health", "HomeMenu", "Lyt", "Menu", "Mii", "Party"}
MINIGAMES = {"Fsh", "Hky", "Pnp", "Duc", "Tnk", "Wly", "Cow", "Bil", "Bom"}


def mangled_class(name):
    if name.startswith(("__ct__", "__dt__")):
        enc = name[6:]
    else:
        k = name.find("__", 1)
        if k <= 0:
            return None
        enc = name[k + 2:]
    if enc.startswith("Q"):
        m = re.match(r"Q(\d)", enc)
        rest, last = enc[2:], None
        for _ in range(int(m.group(1))):
            n = re.match(r"(\d+)", rest)
            if not n:
                return None
            length = int(n.group(1))
            rest = rest[len(n.group(1)):]
            last, rest = rest[:length], rest[length:]
        return last
    m = re.match(r"(\d+)", enc)
    if not m:
        return None  # starts with F: free function
    length = int(m.group(1))
    return enc[len(m.group(1)):len(m.group(1)) + length]


def load_aliases(root):
    """dtk/types/class_aliases.tsv: ledger class (<Mod>_<Class>) -> header struct, or '-' for none."""
    path = root / "dtk/types/class_aliases.tsv"
    if not path.exists():
        return {}
    rows = [l.split("\t") for l in path.read_text(encoding="utf-8").splitlines()[1:] if l.strip()]
    return {r[0].strip(): r[1].strip() for r in rows if len(r) >= 2}


def project_class(name, aliases={}):
    m = re.match(r"^([A-Za-z]+)_([A-Z][A-Za-z0-9]*)_\w+$", name)
    if not m:
        return None
    mod, cls = m.groups()
    alias = aliases.get(f"{mod}_{cls}")
    if alias:
        return None if alias == "-" else alias
    if mod in MINIGAMES:
        return f"RP{mod}{cls}"
    if mod in FRAMEWORK:
        return f"RP{cls}"
    return None


def main():
    root = Path(sys.argv[1])
    sizes = dict(re.findall(r"^(?:struct|union) (\w+) \{ // size (0x[0-9A-F]+)",
                            (root / "build/ghidra/types.h").read_text(encoding="utf-8"), re.M))
    structs = set(sizes)
    # template instances share one struct name (later instances get _2, _3...): map a method of
    # `Name<args>` to `Name` only when every emitted instance has the same size
    same_size = {n for n in structs
                 if len({s for k, s in sizes.items() if k == n or re.fullmatch(re.escape(n) + r"_\d+", k)}) == 1}
    aliases = load_aliases(root)
    rows, unmapped = [], 0
    for line in open(root / "config/RHAE01/symbols.txt", encoding="utf-8"):
        m = re.match(r"^(\S+) = \.(?:text|init):0x[0-9A-F]+; // type:function", line)
        if not m:
            continue
        name = m.group(1)
        cls = mangled_class(name) if "__" in name[1:] else project_class(name, aliases)
        if cls and "<" in cls and cls.split("<")[0] in same_size:
            cls = cls.split("<")[0]
        cls = re.sub(r"[^A-Za-z0-9_]", "_", cls) if cls else None
        if cls and cls in structs:
            rows.append((name, cls))
        elif cls:
            unmapped += 1
    out = root / "build/ghidra/this_map.tsv"
    out.write_text("".join(f"{n}\t{c}\n" for n, c in rows), encoding="utf-8", newline="\n")
    print(f"{len(rows)} methods mapped to {len({c for _, c in rows})} structs; "
          f"{unmapped} methods of classes without a struct yet -> {out}")
    write_globals(root, structs)


PRIMS = {"f32", "f64", "s8", "u8", "s16", "u16", "s32", "u32", "BOOL", "bool", "char", "wchar_t", "void",
         "int", "float", "double"}


def write_globals(root, structs):
    """Global data types from dtk/types/*_globals.tsv -> build/ghidra/globals.tsv
    (name, base type as named in types.h or a primitive, pointer depth, element count).
    Types that are neither (member-function pointers, unknown templates) are left out."""
    rows, skipped = [], 0
    for path in sorted((root / "dtk/types").glob("*_globals.tsv")):
        for line in path.read_text(encoding="utf-8").splitlines()[1:]:
            f = line.split("\t")
            if len(f) < 3 or not re.match(r"^[A-Za-z_]\w*$", f[1].strip()):
                continue
            t = re.sub(r"\b(const|volatile|struct|class|union)\s+", "", f[2].strip())
            dims = [int(d, 0) for d in re.findall(r"\[(0x[0-9A-Fa-f]+|\d+)\]", t)]
            if len(dims) != t.count("["):
                skipped += 1  # symbolic array bound
                continue
            t = re.sub(r"\[[^]]*\]", "", t).strip()
            ptr = t.count("*")
            base = t.replace("*", "").strip()
            if "<" in base or "(" in base:
                skipped += 1
                continue
            base = base.split("::")[-1]
            if base not in PRIMS and base not in structs:
                skipped += 1
                continue
            count = 1
            for d in dims:
                count *= d
            rows.append((f[1].strip(), base, ptr, count))
    out = root / "build/ghidra/globals.tsv"
    out.write_text("".join(f"{n}\t{b}\t{p}\t{c}\n" for n, b, p, c in rows), encoding="utf-8", newline="\n")
    print(f"{len(rows)} typed globals ({skipped} with types Ghidra cannot take) -> {out}")


if __name__ == "__main__":
    main()
