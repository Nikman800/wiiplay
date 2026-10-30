"""Per-batch inventory for the Phase 3 header pass.

For each batch (groups of module prefixes) writes dtk/types/batches/<batch>.txt with:
  classes: ledger class (from Mod_Class_method names), suggested C++ name, vtables,
           allocation sizes (dtk/types/alloc_sizes.tsv), method count and sample methods
  globals: data symbols referenced by the batch's functions (current names), with section,
           size and how many of the batch's functions reference each

Usage: class_inventory.py <root>
"""
import collections
import json
import re
import sys
from pathlib import Path

BATCHES = {
    "fw_rp": ["RP"],
    "fw_gfx": ["Gfx", "Sys", "Snd", "Mpe", "Health", "HomeMenu"],
    "fw_ui": ["Lyt", "Menu", "Mii"],
    "fw_party": ["Party"],
    "hky_pnp": ["Hky", "Pnp"],
    "fsh": ["Fsh"], "duc": ["Duc"], "tnk": ["Tnk"], "wly": ["Wly"],
    "cow": ["Cow"], "bil": ["Bil"], "bom": ["Bom"],
}
SYM = re.compile(r"^(\S+) = (\S+):0x([0-9A-F]+); // (.*)$")
GAME_START, GAME_END = 0x80182930, 0x802E9CC0


def cpp_name(mod, cls):
    return f"RP{cls}" if mod in ("RP", "Gfx", "Sys", "Snd", "Mpe", "Health", "HomeMenu", "Lyt", "Menu", "Mii",
                                  "Party") else f"RP{mod}{cls}"


def main():
    root = Path(sys.argv[1])
    funcs, data = {}, {}
    for line in open(root / "config/RHAE01/symbols.txt", encoding="utf-8"):
        m = SYM.match(line.strip())
        if not m:
            continue
        name, sec, addr, attrs = m.groups()
        size = re.search(r"size:0x([0-9A-F]+)", attrs)
        size = int(size.group(1), 16) if size else 0
        if "type:function" in attrs:
            funcs[int(addr, 16)] = name
        else:
            data[int(addr, 16)] = (name, sec, size)
    idx = json.loads((root / "dtk/index.json").read_text(encoding="utf-8"))
    orig_addr = {n: f["addr"] for n, f in idx["functions"].items() if f["addr"]}
    sym_addr = {}
    for line in open(root / "dtk/symbols.orig.txt", encoding="utf-8"):
        m = SYM.match(line.strip())
        if m:
            sym_addr[m.group(1)] = int(m.group(3), 16)
    vt_info = {}  # vtable -> (class name as recorded, naming batch)
    for line in (root / "dtk/classes.tsv").read_text(encoding="utf-8").splitlines()[1:]:
        f = line.split("\t")
        vt_info.setdefault(f[0].upper(), (f[1], f[3] if len(f) > 3 else ""))
    allocs_by_vt = collections.defaultdict(list)
    allocs_by_ctor = collections.defaultdict(list)
    for line in (root / "dtk/types/alloc_sizes.tsv").read_text(encoding="utf-8").splitlines()[1:]:
        f = line.split("\t")
        size, ctor, cls, caller, vt = f[0], f[1], f[2], f[3], f[6]
        if vt:
            allocs_by_vt[vt].append(f"{size} via {ctor} in {caller} (derived vtable {vt})")
        elif not f[5]:
            allocs_by_ctor[cls].append(f"{size} via {ctor} in {caller}")
    out = root / "dtk/types/batches"
    out.mkdir(parents=True, exist_ok=True)
    for batch, mods in BATCHES.items():
        classes = collections.defaultdict(list)
        own = []
        for addr, name in funcs.items():
            m = re.match(r"^([A-Za-z]+)_([A-Z][A-Za-z0-9]*)_(\w+)$", name)
            mod = name.split("_", 1)[0]
            if mod in mods and GAME_START <= addr < GAME_END:
                own.append(addr)
                if m and m.group(1) in mods:
                    classes[(m.group(1), m.group(2))].append(m.group(3))
        lines = [f"# batch {batch}: modules {', '.join(mods)}; {len(own)} functions; {len(classes)} classes",
                 "", "## classes (ledger class -> suggested C++ name; vtables; allocation sizes; methods)"]
        for (mod, cls), methods in sorted(classes.items()):
            led = f"{mod}_{cls}"
            # bare class names in classes.tsv are only trusted from this module's naming batch
            vts = [v for v, (c, b) in vt_info.items()
                   if c == led or (c == cls and mod.lower() in b.lower())]
            sizes = allocs_by_ctor.get(led, []) + [a for v in vts for a in allocs_by_vt.get(v, [])]
            lines.append(f"{led} -> {cpp_name(mod, cls)}; vtables {', '.join(sorted(vts)) or '-'}; "
                         f"sizes {'; '.join(sorted(set(sizes))[:4]) or '-'}; {len(methods)} methods: "
                         f"{', '.join(sorted(methods)[:12])}{' ...' if len(methods) > 12 else ''}")
        # globals referenced by the batch's functions
        refs = collections.Counter()
        own_set = set(own)
        for n, f in idx["functions"].items():
            if f["addr"] in own_set:
                for r in f["refs"]:
                    a = sym_addr.get(r)
                    if a in data:
                        refs[a] += 1
        lines += ["", "## globals referenced (address name section size refs)"]
        for a, c in sorted(refs.items()):
            name, sec, size = data[a]
            if sec in (".sdata2",) and size <= 8:
                continue  # float/double literals
            lines.append(f"{a:08X} {name} {sec} 0x{size:X} {c}")
        (out / f"{batch}.txt").write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
        print(f"{batch}: {len(own)} functions, {len(classes)} classes, {len(refs)} referenced data symbols")


if __name__ == "__main__":
    main()
