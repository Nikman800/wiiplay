"""Validate and merge method-name proposals into the naming ledger.

  merge_proposals.py check     validate dtk/proposals/*.tsv, print problems and stats
  merge_proposals.py apply     also write dtk/methods.tsv and rename functions in config/RHAE01/symbols.txt

dtk/methods.tsv columns: address, dtk_name, module_name (pass 1), method_name, module,
confidence, note, batch. Rejected or missing proposals fall back to the pass-1 name.
"""
import collections
import csv
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SYMBOLS = "config/RHAE01/symbols.txt"
NAME_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]{0,63}$")
CONF = {"high", "medium", "low", "keep"}
MOVABLE = {"position", "callgraph"}
FRAMEWORK = {"Sys", "Snd", "Gfx", "Lyt", "Menu", "Party", "Mii", "Health", "HomeMenu", "Mpe", "RP"}
MODULES = {"Fsh", "Hky", "Pnp", "Duc", "Tnk", "Wly", "Cow", "Bil", "Bom", "Sys", "Snd", "Gfx", "Lyt", "Menu",
           "Party", "Mii", "Health", "HomeMenu", "Mpe", "RP"}


def main():
    mode = sys.argv[1]
    pass1 = {r["address"]: r for r in csv.DictReader(open(ROOT / "dtk/names.tsv", encoding="utf-8"), delimiter="\t")}
    game = {a: r for a, r in pass1.items() if not r["module"].startswith("egg")}
    chosen, problems = {}, collections.defaultdict(list)
    for path in sorted((ROOT / "dtk/proposals").glob("*.tsv")):
        if path.stem.endswith("_classes"):
            continue
        batch = path.stem
        with open(path, encoding="utf-8") as f:
            for i, row in enumerate(csv.DictReader(f, delimiter="\t"), start=2):
                a = (row.get("address") or "").strip().upper().removeprefix("0X")
                name = (row.get("new_name") or "").strip()
                conf = (row.get("confidence") or "").strip().lower()
                note = (row.get("note") or "").strip()
                if a not in game:
                    problems[batch].append(f"line {i}: {a} not a game function")
                    continue
                if conf not in CONF:
                    problems[batch].append(f"line {i}: bad confidence {conf!r}")
                    continue
                if not NAME_RE.match(name):
                    problems[batch].append(f"line {i}: bad name {name!r}")
                    continue
                old = game[a]
                prefix = name.split("_", 1)[0]
                if prefix not in MODULES:
                    problems[batch].append(f"line {i}: unknown module prefix in {name}")
                    continue
                framework_move = old["module"] in FRAMEWORK and prefix in FRAMEWORK
                if prefix != old["module"] and not (old["module"] == "RP" or old["confidence"] in MOVABLE or framework_move):
                    problems[batch].append(f"line {i}: {name} moves {old['module']}({old['confidence']}) module")
                    continue
                if a in chosen:
                    problems[batch].append(f"line {i}: {a} also proposed by {chosen[a][3]}")
                    continue
                chosen[a] = (name, conf, note, batch)
    # global uniqueness: on collision keep the first by address, suffix the rest with the address
    seen = {}
    for a in sorted(chosen):
        name, conf, note, batch = chosen[a]
        if name in seen:
            chosen[a] = (f"{name}_{a}"[:64] if len(name) <= 55 else f"{name[:55]}_{a}", conf, note, batch)
            problems[batch].append(f"{a}: duplicate {name} (also {seen[name]}), suffixed")
        seen[chosen[a][0]] = a
    missing = sorted(set(game) - set(chosen))
    by_conf = collections.Counter(c[1] for c in chosen.values())
    by_batch = collections.Counter(c[3] for c in chosen.values())
    for b, ps in problems.items():
        print(f"[{b}] {len(ps)} problems", *ps[:8], sep="\n  ")
    print(f"proposals: {len(chosen)}/{len(game)} game functions; missing {len(missing)}")
    print("by confidence:", dict(by_conf))
    print("by batch:", dict(by_batch))
    if mode != "apply":
        return
    with open(ROOT / "dtk/methods.tsv", "w", encoding="utf-8", newline="\n") as out:
        out.write("address\tdtk_name\tmodule_name\tmethod_name\tmodule\tconfidence\tnote\tbatch\n")
        for a, r in sorted(pass1.items()):
            if a in chosen and chosen[a][1] != "keep":
                name, conf, note, batch = chosen[a]
            else:
                name, conf, note, batch = r["new_name"], "keep", "", chosen.get(a, ("", "", "", ""))[3]
            out.write(f"{a}\t{r['old_name']}\t{r['new_name']}\t{name}\t{name.split('_', 1)[0] if a in game else r['module']}"
                      f"\t{conf}\t{note}\t{batch}\n")
    # Rename in place by address so section fixes and dtk's own refinements in the live
    # symbols file survive (rebuilding from symbols.orig.txt would discard them).
    # Game code only: library functions (incl. EGG) are named by tools/apply_library_names.py.
    final = {r["address"]: r["method_name"]
             for r in csv.DictReader(open(ROOT / "dtk/methods.tsv", encoding="utf-8"), delimiter="\t")
             if not r["module"].startswith("egg")}
    ref_names = ROOT / "dtk/game_ref_names.tsv"
    if ref_names.exists():  # real Wii Sports names take precedence over agent proposals
        for r in csv.DictReader(open(ref_names, encoding="utf-8"), delimiter="\t"):
            final.pop(r["address"], None)
    symbols = ROOT / SYMBOLS
    lines, n = [], 0
    for line in open(symbols, encoding="utf-8"):
        m = re.match(r"(\S+)( = \S+:0x)([0-9A-F]+)(;.*type:function.*)", line, re.S)
        if m and m.group(3) in final and m.group(1) != final[m.group(3)]:
            line = final[m.group(3)] + m.group(2) + m.group(3) + m.group(4)
            n += 1
        lines.append(line)
    symbols.write_text("".join(lines), encoding="utf-8", newline="\n")
    print(f"wrote dtk/methods.tsv and {SYMBOLS} ({n} renamed)")


if __name__ == "__main__":
    main()
