"""Name library functions by aligning function-size sequences against reference decomps.

A library built from the same source with the same compiler yields identical function
sizes in the same link order in every game. For each reference (ref/<repo>/symbols.txt +
splits.txt) this finds maximal runs of consecutive functions whose sizes match ours exactly,
then assigns each of our functions the best run covering it.

Usage: align_refs.py <root> [--min-funcs 4] [--min-bytes 384]
Writes dtk/library_matches.tsv:
  address size our_name ref_name ref_unit repo run_id run_funcs run_bytes agree
"""
import argparse
import collections
import re
from pathlib import Path

SYM_RE = re.compile(r"^(\S+) = (\S+):0x([0-9A-Fa-f]+); // (.*)$")
UNIT_RE = re.compile(r"^(\S.*):\s*$")
RANGE_RE = re.compile(r"^\s+(\S+)\s+start:0x([0-9A-Fa-f]+)\s+end:0x([0-9A-Fa-f]+)")
GAME_START, GAME_END = 0x80182E90, 0x802E9CC0
SEED = 4


def functions(path, code_only=True):
    out = []
    for line in Path(path).read_text(encoding="utf-8", errors="replace").splitlines():
        m = SYM_RE.match(line.strip())
        if not m:
            continue
        name, sec, addr, attrs = m.groups()
        f = dict(a.split(":", 1) for a in attrs.split() if ":" in a)
        if f.get("type") != "function" or "size" not in f:
            continue
        if code_only and sec not in (".text", ".init"):
            continue
        out.append((int(addr, 16), int(f["size"], 16), name, sec))
    out.sort()
    return out


def units(path):
    """[(start, end, unit)] for code ranges in a dtk splits.txt."""
    out, cur = [], None
    for line in Path(path).read_text(encoding="utf-8", errors="replace").splitlines():
        m = UNIT_RE.match(line)
        if m and not line.startswith("Sections"):
            cur = m.group(1)
            continue
        m = RANGE_RE.match(line)
        if m and cur and m.group(1) in (".text", ".init"):
            out.append((int(m.group(2), 16), int(m.group(3), 16), cur))
    out.sort()
    return out


def unit_of(ranges, addr):
    lo, hi = 0, len(ranges)
    while lo < hi:
        mid = (lo + hi) // 2
        if ranges[mid][1] <= addr:
            lo = mid + 1
        else:
            hi = mid
    if lo < len(ranges) and ranges[lo][0] <= addr < ranges[lo][1]:
        return ranges[lo][2]
    return ""


def runs(ours, ref):
    """Maximal exact runs of equal consecutive sizes: [(i_ours, j_ref, length)]."""
    a = [s for _, s, _, _ in ours]
    b = [s for _, s, _, _ in ref]
    index = collections.defaultdict(list)
    for j in range(len(b) - SEED + 1):
        index[tuple(b[j:j + SEED])].append(j)
    found, seen = [], set()
    for i in range(len(a) - SEED + 1):
        for j in index.get(tuple(a[i:i + SEED]), ()):
            if (i - j) in seen and (i - 1, j - 1) in seen:
                continue
            # extend backwards and forwards
            s_i, s_j = i, j
            while s_i > 0 and s_j > 0 and a[s_i - 1] == b[s_j - 1]:
                s_i, s_j = s_i - 1, s_j - 1
            e = 0
            while s_i + e < len(a) and s_j + e < len(b) and a[s_i + e] == b[s_j + e]:
                e += 1
            key = (s_i, s_j)
            if key not in seen:
                seen.add(key)
                found.append((s_i, s_j, e))
    return found


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("root")
    ap.add_argument("--min-funcs", type=int, default=4)
    ap.add_argument("--min-bytes", type=int, default=384)
    ap.add_argument("--game", action="store_true",
                    help="align game code instead (RP framework is shared with Wii Sports); "
                         "writes dtk/game_ref_matches.tsv")
    args = ap.parse_args()
    root = Path(args.root)
    in_game = lambda a: GAME_START <= a < GAME_END
    ours = [f for f in functions(root / "config/RHAE01/symbols.txt") if in_game(f[0]) == args.game]
    best = {}       # our index -> (score, repo, ref_name, unit, run_id, n, nbytes)
    votes = collections.defaultdict(collections.Counter)
    run_id = 0
    for repo_dir in sorted(p for p in (root / "ref").iterdir() if (p / "symbols.txt").exists()):
        ref = functions(repo_dir / "symbols.txt")
        ref_units = units(repo_dir / "splits.txt")
        kept = 0
        for i, j, n in runs(ours, ref):
            nbytes = sum(s for _, s, _, _ in ours[i:i + n])
            if n < args.min_funcs or nbytes < args.min_bytes:
                continue
            run_id += 1
            kept += 1
            for k in range(n):
                _, _, rname, _ = ref[j + k]
                runit = unit_of(ref_units, ref[j + k][0])
                votes[i + k][rname] += 1
                cand = (n * 1000 + nbytes, repo_dir.name, rname, runit, run_id, n, nbytes)
                if i + k not in best or cand[0] > best[i + k][0]:
                    best[i + k] = cand
        print(f"{repo_dir.name}: {len(ref)} ref functions, {kept} runs kept")
    out = root / ("dtk/game_ref_matches.tsv" if args.game else "dtk/library_matches.tsv")
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write("address\tsize\tour_name\tref_name\tref_unit\trepo\trun_id\trun_funcs\trun_bytes\tagree\n")
        for k, (addr, size, name, _) in enumerate(ours):
            if k not in best:
                continue
            _, repo, rname, runit, rid, n, nbytes = best[k]
            agree = votes[k][rname]
            f.write(f"{addr:08X}\t{size}\t{name}\t{rname}\t{runit}\t{repo}\t{rid}\t{n}\t{nbytes}\t{agree}\n")
    covered = len(best)
    lib_bytes = sum(s for _, s, _, _ in ours)
    cov_bytes = sum(ours[k][1] for k in best)
    print(f"{'game' if args.game else 'library'} functions: {len(ours)}; matched {covered} ({100 * covered / len(ours):.1f}%), "
          f"{cov_bytes}/{lib_bytes} bytes ({100 * cov_bytes / lib_bytes:.1f}%) -> {out}")


if __name__ == "__main__":
    main()
