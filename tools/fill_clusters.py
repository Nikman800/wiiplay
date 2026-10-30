"""Name unmatched library functions by ordered alignment within known source-file neighbours.

For each cluster of library functions without an exact match, the source-file units just before
and after it (from consecutive exact matches) bound the matching segment in each reference. The longest common
subsequence of function sizes between our cluster and that segment pairs functions in order. A
pair is accepted only if its sizes are equal and it is part of an aligned run of 2+ consecutive
pairs or is at least 0x100 bytes, so small coincidental sizes are not named.
Writes dtk/cluster_matches.tsv.

Usage: fill_clusters.py <root>
"""
import bisect
import csv
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from align_refs import functions, units, unit_of  # noqa: E402

GAME_START, GAME_END = 0x80182930, 0x802E9CC0
PLACEHOLDER = re.compile(r"^(fn|FUN|lbl|func|sub|zz|unk)_?[0-9A-Fa-f]{6,8}\b|^@")


def lcs_pairs(a, b):
    n, m = len(a), len(b)
    dp = [[0] * (m + 1) for _ in range(n + 1)]
    for i in range(n - 1, -1, -1):
        for j in range(m - 1, -1, -1):
            dp[i][j] = dp[i + 1][j + 1] + 1 if a[i] == b[j] else max(dp[i + 1][j], dp[i][j + 1])
    pairs, i, j = [], 0, 0
    while i < n and j < m:
        if a[i] == b[j] and dp[i][j] == dp[i + 1][j + 1] + 1:
            pairs.append((i, j))
            i, j = i + 1, j + 1
        elif dp[i + 1][j] >= dp[i][j + 1]:
            i += 1
        else:
            j += 1
    return pairs


def main():
    root = Path(sys.argv[1])
    ours = functions(root / "config/RHAE01/symbols.txt")
    # Depend only on exact-match evidence, never on names or units earlier runs applied, so the
    # pipeline is idempotent: candidates are library functions without an exact real-name match,
    # and bounding units come from consecutive exact matches with the same reference unit.
    exact = {}
    for r in csv.DictReader(open(root / "dtk/library_matches.tsv", encoding="utf-8"), delimiter="\t"):
        if not PLACEHOLDER.match(r["ref_name"]):
            exact[int(r["address"], 16)] = r["ref_unit"]
    our_units = []
    for addr, size, _, _ in ours:
        u = exact.get(addr)
        if not u:
            continue
        if our_units and our_units[-1][2] == u:
            our_units[-1][1] = addr + size
        else:
            our_units.append([addr, addr + size, u])
    starts = [u[0] for u in our_units]
    unnamed = [f for f in ours if f[0] not in exact and not GAME_START <= f[0] < GAME_END]
    clusters = []
    for f in unnamed:
        k = bisect.bisect_right(starts, f[0]) - 1
        before = our_units[k][2] if k >= 0 else None
        after = our_units[k + 1][2] if k + 1 < len(our_units) else None
        if clusters and clusters[-1]["bounds"] == (before, after):
            clusters[-1]["fns"].append(f)
        else:
            clusters.append(dict(bounds=(before, after), fns=[f]))
    refs = {p.name: (functions(p / "symbols.txt"), units(p / "splits.txt"))
            for p in (root / "ref").iterdir() if (p / "symbols.txt").exists()}
    best = {}
    for c in clusters:
        before, after = c["bounds"]
        # our segment: all functions between the bounding units (named or not)
        lo = next(u[1] for u in our_units if u[2] == before) if before else 0
        hi = next(u[0] for u in our_units if u[2] == after) if after else 1 << 32
        seg = [f for f in ours if lo <= f[0] < hi]
        for repo, (fl, ru) in refs.items():
            ua = [u for u in ru if u[2] == before]
            ub = [u for u in ru if u[2] == after]
            if not ua or not ub or ua[-1][1] > ub[0][0]:
                continue
            rseg = [f for f in fl if ua[-1][1] <= f[0] < ub[0][0]]
            pairs = lcs_pairs([f[1] for f in seg], [f[1] for f in rseg])
            for idx, (i, j) in enumerate(pairs):
                run = any(0 <= idx + d < len(pairs) and pairs[idx + d] == (i + d, j + d) for d in (-1, 1))
                f, r = seg[i], rseg[j]
                if f[0] in exact or PLACEHOLDER.match(r[2]):
                    continue
                if not (run or f[1] >= 0x100):
                    continue
                score = len(pairs)
                if f[0] not in best or score > best[f[0]][0]:
                    best[f[0]] = (score, f, r[2], unit_of(ru, r[0]), repo, run)
    # our_name is dtk's original name (stable across runs), not whatever an earlier run applied
    orig = {a: n for a, _, n, _ in functions(root / "dtk/symbols.orig.txt")}
    with open(root / "dtk/cluster_matches.tsv", "w", encoding="utf-8", newline="\n") as out:
        out.write("address\tsize\tour_name\tref_name\tref_unit\trepo\tin_run\n")
        for addr in sorted(best):
            _, f, rname, runit, repo, run = best[addr]
            out.write(f"{addr:08X}\t{f[1]}\t{orig.get(addr, f[2])}\t{rname}\t{runit}\t{repo}\t{int(run)}\n")
    print(f"{len(unnamed)} unnamed library functions in {len(clusters)} clusters; "
          f"{len(best)} named by ordered alignment -> dtk/cluster_matches.tsv")


if __name__ == "__main__":
    main()
