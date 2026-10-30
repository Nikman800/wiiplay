"""Context for naming the still-unnamed library functions (fn_*) by reading their code.

For each one: size, the named source-file unit containing it (or the units on either side),
nearest named neighbours, and every reference project's function names for those source files
(the likely naming vocabulary). Writes one file per batch to dtk/lib_naming/<batch>.txt.

Usage: lib_context.py <root>
Batches: nw4r (NintendoWare) and rest (EGG, RVL SDK, HomeButtonMiniLib, other).
"""
import bisect
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from align_refs import functions, units, unit_of  # noqa: E402

GAME_START, GAME_END = 0x80182930, 0x802E9CC0


def main():
    root = Path(sys.argv[1])
    ours = functions(root / "config/RHAE01/symbols.txt")
    our_units = [u for u in units(root / "config/RHAE01/splits.txt") if not u[2].startswith("auto_")]
    starts = [u[0] for u in our_units]
    refs = {p.name: (functions(p / "symbols.txt"), units(p / "splits.txt"))
            for p in (root / "ref").iterdir() if p.is_dir()}
    vocab_cache = {}

    def vocab(unit):
        if unit not in vocab_cache:
            names = []
            for repo, (fl, ru) in refs.items():
                rng = [u for u in ru if u[2] == unit]
                for lo, hi, _ in rng:
                    names += [f"{n} (0x{s:X})" for a, s, n, _ in fl if lo <= a < hi]
            vocab_cache[unit] = sorted(set(names))
        return vocab_cache[unit]

    batches = {"nw4r": [], "rest": []}
    for idx, (addr, size, name, _) in enumerate(ours):
        if not name.startswith("fn_") or GAME_START <= addr < GAME_END:
            continue
        k = bisect.bisect_right(starts, addr) - 1
        inside = our_units[k][2] if k >= 0 and our_units[k][0] <= addr < our_units[k][1] else None
        before = our_units[k][2] if k >= 0 else None
        after = our_units[k + 1][2] if k + 1 < len(our_units) else None
        cands = [inside] if inside else [u for u in (before, after) if u]
        prev = [n for _, _, n, _ in ours[max(0, idx - 3):idx]]
        nxt = [n for _, _, n, _ in ours[idx + 1:idx + 4]]
        lines = [f"== {addr:08X} {name} size=0x{size:X}",
                 f"   unit: {inside}" if inside else f"   between units: {before} | {after}",
                 f"   previous: {', '.join(prev)}",
                 f"   next: {', '.join(nxt)}"]
        for u in cands:
            v = vocab(u)
            lines.append(f"   reference names in {u} ({len(v)}): " + "; ".join(v[:60]) + (" ..." if len(v) > 60 else ""))
        top = (inside or before or after or "").split("/")[0]
        batches["nw4r" if top == "nw4r" else "rest"].append("\n".join(lines))
    out = root / "dtk/lib_naming"
    out.mkdir(exist_ok=True)
    for b, items in batches.items():
        (out / f"{b}.txt").write_text("\n\n".join(items) + "\n", encoding="utf-8", newline="\n")
        print(f"{b}: {len(items)} functions -> {out / (b + '.txt')}")


if __name__ == "__main__":
    main()
