"""Inspect functions for naming work.

  fn_info.py list <Module> [<Module>...]      functions of modules in address order
  fn_info.py show <addr> [<addr>...]          dossier + pseudocode for each address
  fn_info.py vtable <vtable_addr>             entries of one vtable with current names

Addresses are hex without 0x (e.g. 80217750). Names are the current names from
dtk/names.tsv (falls back to dtk's name). Run from anywhere with -I.
"""
import csv
import glob
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HEADER = re.compile(r"^// (\S+) @ ([0-9a-f]{8})$")


def load():
    idx = json.loads((ROOT / "dtk/index.json").read_text(encoding="utf-8"))
    rows = {r["old_name"]: r for r in csv.DictReader(open(ROOT / "dtk/names.tsv", encoding="utf-8"), delimiter="\t")}
    methods = ROOT / "dtk/methods.tsv"
    if methods.exists():  # final method names override the pass-1 module names
        for m in csv.DictReader(open(methods, encoding="utf-8"), delimiter="\t"):
            if m["dtk_name"] in rows:
                rows[m["dtk_name"]] = dict(rows[m["dtk_name"]], new_name=m["method_name"], module=m["module"])
    lits = json.loads((ROOT / "dtk/literals.json").read_text(encoding="utf-8"))
    # live names (game + library) from the symbols file, keyed by dtk's original name via address
    live = {}
    for line in open(ROOT / "config/RHAE01/symbols.txt", encoding="utf-8"):
        m = re.match(r"(\S+) = \.(?:text|init):0x([0-9A-F]+); // type:function", line)
        if m:
            live[int(m.group(2), 16)] = m.group(1)
    for f in idx["functions"].values():
        if f["addr"] in live:
            LIVE[f["name"]] = live[f["addr"]]
    return idx, rows, lits


LIVE = {}  # dtk original name -> current name


def cur(rows, old):
    if old in LIVE:
        return LIVE[old]
    return rows[old]["new_name"] if old in rows else old


def pseudocode(addr):
    want = addr.lower()
    for path in glob.glob(str(ROOT / "decomp" / "*.c")):
        lines = open(path, encoding="utf-8", errors="replace").read().split("\n")
        for i, line in enumerate(lines):
            m = HEADER.match(line)
            if m and m.group(2) == want:
                out = []
                for nxt in lines[i + 1:]:
                    if HEADER.match(nxt):
                        break
                    if "WARNING: Removing unreachable block" in nxt:
                        continue
                    out.append(nxt)
                return "\n".join(out).strip()
    return "(no pseudocode)"


def main():
    idx, rows, lits = load()
    F = idx["functions"]
    by_addr = {f"{f['addr']:08X}": f for f in F.values() if f["addr"]}
    cmd = sys.argv[1]
    if cmd == "list":
        mods = set(sys.argv[2:])
        sel = sorted((r for r in rows.values() if r["module"] in mods), key=lambda r: r["address"])
        for r in sel:
            f = by_addr.get(r["address"], {})
            print(f"{r['address']}\t{r['new_name']}\t{f.get('size') or 0}\t{r['confidence']}")
        print(f"# {len(sel)} functions", file=sys.stderr)
    elif cmd == "show":
        for a in sys.argv[2:]:
            a = a.upper().removeprefix("0X")
            f = by_addr.get(a)
            if not f:
                print(f"== {a}: unknown function\n")
                continue
            r = rows.get(f["name"], {})
            vts = [f"{v['name']} slot {v['entries'].index(f['name']) - 2}" for v in idx["vtables"]
                   if f["name"] in v["entries"]]
            print(f"== {cur(rows, f['name'])} @ {a}  size={f['size']}  module={r.get('module', '?')} ({r.get('confidence', '?')})")
            print(f"   callers: {', '.join(cur(rows, c) for c in f['callers'][:20])}{' ...' if len(f['callers']) > 20 else ''}")
            print(f"   callees: {', '.join(cur(rows, c) for c in f['callees'][:30])}")
            print(f"   literals: {lits.get(a, [])}")
            if vts:
                print(f"   vtables: {'; '.join(vts[:6])}")
            print(pseudocode(a))
            print()
    elif cmd == "vtable":
        a = sys.argv[2].upper().removeprefix("0X")
        for v in idx["vtables"]:
            if v["addr"] and f"{v['addr']:08X}" == a:
                for i, e in enumerate(v["entries"]):
                    print(f"slot {i - 2:3}\t{cur(rows, e)}")
                print(f"referenced by: {', '.join(cur(rows, r) for r in v['referenced_by'])}")


if __name__ == "__main__":
    main()
