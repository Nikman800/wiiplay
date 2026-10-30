"""Name unmatched library functions that sit in a gap between two matched functions.

If our gap of n unmatched functions lies between functions matched to reference positions j and
j+n+1 of the same reference, the reference's n functions in between are taken as candidates,
even when sizes differ (usually a slightly different library version). Writes
dtk/gap_matches.tsv with the size difference so weak candidates can be filtered.

Usage: fill_gaps.py <root>
"""
import csv
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from align_refs import functions, units, unit_of  # noqa: E402

GAME_START, GAME_END = 0x80182E90, 0x802E9CC0
PLACEHOLDER = re.compile(r"^(fn|FUN|lbl|func|sub|zz|unk)_?[0-9A-Fa-f]{6,8}\b|^@")


def main():
    root = Path(sys.argv[1])
    ours = [f for f in functions(root / "config/RHAE01/symbols.txt") if not GAME_START <= f[0] < GAME_END]
    matched = {}
    for r in csv.DictReader(open(root / "dtk/library_matches.tsv", encoding="utf-8"), delimiter="\t"):
        matched[int(r["address"], 16)] = (r["repo"], r["ref_name"])
    refs = {}
    for repo_dir in (root / "ref").iterdir():
        if repo_dir.is_dir():
            fl = functions(repo_dir / "symbols.txt")
            refs[repo_dir.name] = (fl, units(repo_dir / "splits.txt"), {})
            for k, (_, _, name, _) in enumerate(fl):
                refs[repo_dir.name][2].setdefault(name, []).append(k)
    rows = []
    i = 0
    while i < len(ours):
        if ours[i][0] in matched:
            i += 1
            continue
        start = i
        while i < len(ours) and ours[i][0] not in matched:
            i += 1
        if start == 0 or i == len(ours):
            continue
        left, right = matched[ours[start - 1][0]], matched[ours[i][0]]
        n = i - start
        if left[0] != right[0]:
            continue
        fl, ru, index = refs[left[0]]
        best = None
        for jl in index.get(left[1], []):
            for jr in index.get(right[1], []):
                if jr - jl - 1 == n and (best is None or jl < best):
                    best = jl
        if best is None:
            continue
        for k in range(n):
            addr, size, name, _ = ours[start + k]
            raddr, rsize, rname, _ = fl[best + 1 + k]
            if PLACEHOLDER.match(rname):
                continue
            rows.append((addr, size, name, rname, unit_of(ru, raddr), left[0], rsize - size, n))
    with open(root / "dtk/gap_matches.tsv", "w", encoding="utf-8", newline="\n") as f:
        f.write("address\tsize\tour_name\tref_name\tref_unit\trepo\tsize_diff\tgap_len\n")
        for r in rows:
            f.write("{:08X}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\n".format(*r))
    exact = sum(1 for r in rows if r[6] == 0)
    print(f"{len(rows)} gap candidates ({exact} same size, {len(rows) - exact} different size) -> dtk/gap_matches.tsv")


if __name__ == "__main__":
    main()
