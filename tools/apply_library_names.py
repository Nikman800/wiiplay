"""Apply reference-derived library names (dtk/library_matches.tsv) to config/RHAE01/symbols.txt.

Only real reference names are used (placeholders like fn_80405994 are skipped). Names that occur
more than once in the final symbol table are static/inline copies and get scope:local, the
convention the reference projects use. Renames by address. Game code is only touched for strict
Wii Sports (ogws) matches of the shared RP framework, listed in dtk/game_ref_names.tsv.

Usage: apply_library_names.py <root>
"""
import collections
import csv
import re
import sys
from pathlib import Path

GAME_START, GAME_END = 0x80182E90, 0x802E9CC0
PLACEHOLDER = re.compile(r"^(fn|FUN|lbl|func|sub|zz|unk)_?[0-9A-Fa-f]{6,8}\b|^@")
LINE = re.compile(r"^(\S+)( = \S+:0x)([0-9A-F]+)(; // )(.*)$")


def main():
    root = Path(sys.argv[1])
    sym_path = root / "config/RHAE01/symbols.txt"
    matches = {r["address"]: r["ref_name"]
               for r in csv.DictReader(open(root / "dtk/library_matches.tsv", encoding="utf-8"), delimiter="\t")
               if not PLACEHOLDER.match(r["ref_name"]) and not GAME_START <= int(r["address"], 16) < GAME_END}
    # Ordered-alignment names (tools/fill_clusters.py) only fill functions nothing else named.
    cluster = root / "dtk/cluster_matches.tsv"
    if cluster.exists():
        for r in csv.DictReader(open(cluster, encoding="utf-8"), delimiter="\t"):
            if not PLACEHOLDER.match(r["ref_name"]):  # rows are never exact matches
                matches.setdefault(r["address"], r["ref_name"])
    # Game code: the RP framework is shared with Wii Sports. Only strict matches (ogws, runs of 8+
    # functions) override the agents' names; dtk/game_ref_names.tsv records them for merge_proposals.
    game = [r for r in csv.DictReader(open(root / "dtk/game_ref_matches.tsv", encoding="utf-8"), delimiter="\t")
            if r["repo"] == "ogws" and int(r["run_funcs"]) >= 8 and not PLACEHOLDER.match(r["ref_name"])]
    with open(root / "dtk/game_ref_names.tsv", "w", encoding="utf-8", newline="\n") as f:
        f.write("address\tref_name\tref_unit\trun_funcs\n")
        for r in game:
            matches[r["address"]] = r["ref_name"]
            f.write(f"{r['address']}\t{r['ref_name']}\t{r['ref_unit']}\t{r['run_funcs']}\n")
    # addresses with a code-verified correction never take a reference name (it would briefly
    # duplicate the corrected neighbour's name and mark both scope:local)
    corr = root / "dtk/name_corrections.tsv"
    if corr.exists():
        for r in csv.DictReader(open(corr, encoding="utf-8"), delimiter="\t"):
            matches.pop(r["address"], None)
    # likewise addresses whose name comes from decompiled source (dtk/source_names, Phase 4)
    for path in sorted((root / "dtk/source_names").glob("*.tsv")):
        for r in csv.DictReader(open(path, encoding="utf-8"), delimiter="\t"):
            matches.pop(r["address"], None)
    lines = sym_path.read_text(encoding="utf-8").splitlines()
    parsed = [LINE.match(l) for l in lines]
    final = []
    for m in parsed:
        if m and "type:function" in m.group(5) and m.group(3) in matches:
            final.append(matches[m.group(3)])
        else:
            final.append(m.group(1) if m else None)
    counts = collections.Counter(n for n in final if n)
    out, renamed, localized = [], 0, 0
    for line, m, name in zip(lines, parsed, final):
        if not m:
            out.append(line)
            continue
        attrs = m.group(5)
        if counts[name] > 1 and "type:function" in attrs and "scope:local" not in attrs:
            attrs = re.sub(r"\s*scope:\S+", "", attrs) + " scope:local"
            localized += 1
        if name != m.group(1):
            renamed += 1
        out.append(f"{name}{m.group(2)}{m.group(3)}{m.group(4)}{attrs}")
    sym_path.write_text("\n".join(out) + "\n", encoding="utf-8", newline="\n")
    dup_global = [n for n, c in counts.items() if c > 1]
    print(f"renamed {renamed} library functions; {localized} marked scope:local "
          f"({len(dup_global)} names occur more than once)")


if __name__ == "__main__":
    main()
