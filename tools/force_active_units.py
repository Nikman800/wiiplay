"""Mark one symbol force_active in every unit that no other unit references.

Without CodeWarrior .comment sections mwld never strips individual symbols, but it does drop
whole objects nothing references (e.g. revolution/NET/NETVersion.c once it is its own unit).
dtk adds force_active symbols to the linker script's FORCEACTIVE list, which keeps the object.
This scans build/RHAE01/asm/**.s (from dtk split), finds units whose defined symbols are never
mentioned by another unit, and tags the first symbol of each in config/RHAE01/symbols.txt.

Usage: force_active_units.py <root>
"""
import re
import sys
from pathlib import Path

DEF = re.compile(r'^\.(?:fn|obj|sym)\s+("[^"]+"|[^\s,]+)', re.M)
TOKEN = re.compile(r'"[^"]+"|[A-Za-z_.$@][\w.$@]*')
LINE = re.compile(r"^(\S+) = (\S+):0x([0-9A-F]+); // (.*)$")
ENTRY = {"__start"}  # referenced by the linker script itself
# Functions the original keeps although nothing calls them (no object-level reference keeps
# them once their unit links from source, because mwld then strips unreferenced functions):
# MetroTRK targsupp.c's trap stubs other than TRKAccessFile.
KEEP = {"TRKOpenFile", "TRKCloseFile", "TRKPositionFile",
        # 0x40 of .bss padding before SIBios's Packet (dvd_broadway statics unreferenced here)
        "lbl_8040EE40"}


def main():
    root = Path(sys.argv[1])
    asm_dir = root / "build/RHAE01/asm"
    units = {}
    for path in asm_dir.rglob("*.s"):
        text = path.read_text(encoding="utf-8", errors="replace")
        defs = [d.strip('"') for d in DEF.findall(text)]
        units[path] = (defs, {t.strip('"') for t in TOKEN.findall(text)})
    defined_in = {}
    for path, (defs, _) in units.items():
        for d in defs:
            defined_in.setdefault(d, path)
    referenced = set()
    for path, (_, tokens) in units.items():
        for t in tokens:
            owner = defined_in.get(t)
            if owner is not None and owner != path:
                referenced.add(owner)
    unref = [(p, defs) for p, (defs, _) in units.items()
             if defs and p not in referenced and not ENTRY.intersection(defs)]
    sym_path = root / "config/RHAE01/symbols.txt"
    lines = sym_path.read_text(encoding="utf-8").splitlines()
    parsed = [LINE.match(l) for l in lines]
    by_name = {m.group(1): i for i, m in enumerate(parsed) if m}
    by_addr = {}
    for i, m in enumerate(parsed):
        if m:
            by_addr.setdefault(m.group(3), i)
    # asm names can differ from symbols.txt: duplicates get an _ADDR suffix, gap functions fn_ADDR
    def locate(name):
        if name in by_name:
            return by_name[name]
        a = re.search(r"_([0-9A-F]{8})$", name)
        return by_addr.get(a.group(1)) if a else None
    target_lines, targets = set(), set()
    for _, defs in unref:
        idx = next((locate(d) for d in defs if locate(d) is not None), None)
        if idx is None:
            targets.add(defs[0])
        else:
            target_lines.add(idx)
    # dtk 1.8.4 has no per-symbol force_active flag; it takes a force_active list in config.yml.
    names = sorted({parsed[i].group(1) for i in target_lines} | KEEP)
    cfg_path = root / "config/RHAE01/config.yml"
    cfg = cfg_path.read_text(encoding="utf-8")
    begin, end = "# BEGIN force_active (tools/force_active_units.py)", "# END force_active"
    if begin in cfg:
        cfg = cfg[:cfg.index(begin)].rstrip("\n") + "\n" + cfg[cfg.index(end) + len(end):].lstrip("\n")
    block = [begin, "# One symbol per unit no other unit references, so mwld keeps the object.", "force_active:"]
    block += ['  - "' + n.replace("\\", "\\\\").replace('"', '\\"') + '"' for n in names]
    block.append(end)
    cfg_path.write_text(cfg.rstrip("\n") + "\n" + "\n".join(block) + "\n", encoding="utf-8", newline="\n")
    tagged = len(names)
    print(f"{len(units)} units scanned; {len(unref)} unreferenced; {tagged} symbols listed in config.yml force_active")
    for p, defs in sorted(unref)[:12]:
        print(f"  {p.relative_to(asm_dir)}: {defs[0]}")
    if targets:
        print(f"  not found in symbols.txt: {sorted(targets)[:5]}")


if __name__ == "__main__":
    main()
