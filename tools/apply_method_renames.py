"""Validate and apply game method renames from reading typed pseudocode (Phase 3 name review).

  apply_method_renames.py <root> merge   validate dtk/method_review/*.tsv -> dtk/method_renames.tsv
  apply_method_renames.py <root> apply   rename in config/RHAE01/symbols.txt and dtk/methods.tsv

Review rows: address, old_name, new_name, confidence (high/medium/low), evidence. A row whose
new_name equals old_name confirms the name (only the confidence changes). Names follow the
project convention `<Mod>_<Class>_<method>`, where <Mod>_<Class> maps to the header class of
`this` by convention or through dtk/types/class_aliases.tsv. `apply` renames a symbol only while
it still carries old_name, so re-runs are no-ops; it runs in tools/phase2_pipeline.sh.
"""
import collections
import csv
import re
import sys
from pathlib import Path

NAME = re.compile(r"^[A-Za-z][A-Za-z0-9_]{0,99}$")
CONF = {"high", "medium", "low"}
LINE = re.compile(r"^(\S+)( = \S+:0x)([0-9A-F]+)(; // .*)$")
FIELDS = ["address", "old_name", "new_name", "confidence", "evidence"]


def read_tsv(path):
    lines = path.read_text(encoding="utf-8").splitlines()
    if lines:
        lines[0] = lines[0].lstrip("# ")
    return list(csv.DictReader(lines, delimiter="\t"))


def merge(root):
    sym = {}
    for l in (root / "config/RHAE01/symbols.txt").read_text(encoding="utf-8").splitlines():
        m = LINE.match(l)
        if m and "type:function" in m.group(4):
            sym[m.group(3)] = m.group(1)
    ledger = root / "dtk/method_renames.tsv"
    rows = {r["address"]: r for r in read_tsv(ledger)} if ledger.exists() else {}
    problems = []
    for path in sorted((root / "dtk/method_review").glob("*.tsv")):
        for i, r in enumerate(read_tsv(path), start=2):
            a = (r.get("address") or "").strip().upper().removeprefix("0X")
            old, new = (r.get("old_name") or "").strip(), (r.get("new_name") or "").strip()
            conf = (r.get("confidence") or "").strip().lower()
            where = f"{path.name}:{i}"
            if a not in sym:
                problems.append(f"{where}: {a} is not a function")
            elif not NAME.match(new) or conf not in CONF:
                problems.append(f"{where}: bad name/confidence {new!r} {conf!r}")
            elif sym[a] not in (old, new) and rows.get(a, {}).get("new_name") != sym[a]:
                problems.append(f"{where}: {a} is now {sym[a]}, not {old}")
            else:
                rows[a] = {"address": a, "old_name": old, "new_name": new, "confidence": conf,
                           "evidence": (r.get("evidence") or "").strip().replace("\t", " ")}
    # a new name must not collide with another symbol or another rename
    final = dict(sym)
    for a, r in rows.items():
        final[a] = r["new_name"]
    count = collections.Counter(final.values())
    for a, r in list(rows.items()):
        if count[r["new_name"]] > 1:
            problems.append(f"{a}: {r['new_name']} would not be unique; dropped")
            del rows[a]
    with open(ledger, "w", encoding="utf-8", newline="\n") as f:
        f.write("\t".join(FIELDS) + "\n")
        for a in sorted(rows):
            f.write("\t".join(rows[a][k] for k in FIELDS) + "\n")
    for p in problems[:30]:
        print("  " + p)
    renamed = sum(1 for r in rows.values() if r["new_name"] != r["old_name"])
    print(f"{len(rows)} reviewed ({renamed} renames, {len(rows) - renamed} confirmations); "
          f"{len(problems)} problems -> {ledger}")


def apply(root):
    ledger = root / "dtk/method_renames.tsv"
    rows = {r["address"]: r for r in read_tsv(ledger)} if ledger.exists() else {}
    path = root / "config/RHAE01/symbols.txt"
    lines = path.read_text(encoding="utf-8").splitlines()
    # decide on the final names first, so swaps and chains (A takes B's old name) work in one pass
    matches = [LINE.match(l) for l in lines]
    todo = {}
    for m in matches:
        r = rows.get(m.group(3)) if m else None
        if r and m.group(1) == r["old_name"] and r["new_name"] != r["old_name"]:
            todo[m.group(3)] = r["new_name"]
    final = collections.Counter(todo.get(m.group(3), m.group(1)) for m in matches if m)
    out, renamed = [], 0
    for l, m in zip(lines, matches):
        new = todo.get(m.group(3)) if m else None
        if new and final[new] == 1:
            l = new + m.group(2) + m.group(3) + m.group(4)
            renamed += 1
        out.append(l)
    path.write_text("\n".join(out) + "\n", encoding="utf-8", newline="\n")
    # keep the method ledger in step: method_name and confidence from the reviews, and the real
    # (mangled) names that Phase 2 reference matching gave to functions first named by the project
    live = {m.group(3): m.group(1) for m in map(LINE.match, out) if m}
    mpath = root / "dtk/methods.tsv"
    mlines = mpath.read_text(encoding="utf-8").splitlines()
    head = mlines[0].split("\t")
    ia, iname, iconf = head.index("address"), head.index("method_name"), head.index("confidence")
    updated = 0
    for k in range(1, len(mlines)):
        f = mlines[k].split("\t")
        r = rows.get(f[ia])
        cur = live.get(f[ia], "")
        if r:
            want = (r["new_name"], r["confidence"])
        elif "__" in cur[1:] and cur != f[iname] and f[iconf] != "keep":
            want = (cur, "high")
        else:
            continue
        if (f[iname], f[iconf]) != want:
            f[iname], f[iconf] = want
            mlines[k] = "\t".join(f)
            updated += 1
    mpath.write_text("\n".join(mlines) + "\n", encoding="utf-8", newline="\n")
    print(f"{renamed} symbols renamed; {updated} method ledger rows updated")


if __name__ == "__main__":
    {"merge": merge, "apply": apply}[sys.argv[2]](Path(sys.argv[1]))
