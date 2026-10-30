"""Validate and apply library names proposed by reading code (tools/LIB_NAMING_CONVENTIONS.md).

  apply_read_names.py <root> merge   validate dtk/lib_naming/*.proposals.tsv -> dtk/lib_read_names.tsv
  apply_read_names.py <root> apply   rename functions still called fn_* in config/RHAE01/symbols.txt

The ledger (dtk/lib_read_names.tsv) is committed; `apply` runs in tools/phase2_pipeline.sh after the
reference-based steps, so names from reference matching always take precedence.
"""
import collections
import csv
import re
import sys
from pathlib import Path

MANGLED = re.compile(r"^[A-Za-z0-9_<>,@$.:*&\\]{1,250}$")  # \ appears in __sinit_\file_cpp
FLAT = re.compile(r"^[A-Za-z0-9_]{1,80}$")
CONF = {"high", "medium", "low", "keep"}
LINE = re.compile(r"^(\S+)( = \S+:0x)([0-9A-F]+)(; // .*)$")


def symbols(root):
    path = root / "config/RHAE01/symbols.txt"
    return path, path.read_text(encoding="utf-8").splitlines()


def merge(root):
    _, lines = symbols(root)
    current = {}
    for l in lines:
        m = LINE.match(l)
        if m and "type:function" in m.group(4):
            current[m.group(3)] = m.group(1)
    taken = collections.Counter(current.values())
    expected = {}
    for ctx in (root / "dtk/lib_naming").glob("*.txt"):
        for m in re.finditer(r"^== ([0-9A-F]{8}) (\S+)", ctx.read_text(encoding="utf-8"), re.M):
            expected[m.group(1)] = (ctx.stem, m.group(2))
    rows, problems = {}, collections.defaultdict(list)
    for path in sorted((root / "dtk/lib_naming").glob("*.proposals.tsv")):
        batch = path.name.split(".")[0]
        for i, r in enumerate(csv.DictReader(open(path, encoding="utf-8"), delimiter="\t"), start=2):
            a = (r.get("address") or "").strip().upper()
            sym = (r.get("symbol") or "").strip()
            conf = (r.get("confidence") or "").strip().lower()
            if a not in expected:
                problems[batch].append(f"line {i}: {a} not in the batch")
                continue
            if conf not in CONF:
                problems[batch].append(f"line {i}: bad confidence {conf!r}")
                continue
            if conf != "keep" and not (FLAT.match(sym) or MANGLED.match(sym)):
                problems[batch].append(f"line {i}: bad symbol {sym!r}")
                continue
            if conf != "keep" and taken[sym] and current.get(a) != sym:
                problems[batch].append(f"line {i}: {sym} already used by another function")
                continue
            rows[a] = (sym, (r.get("qualified") or "").strip(), conf, (r.get("note") or "").strip(), batch)
    missing = sorted(set(expected) - set(rows))
    for b, ps in problems.items():
        print(f"[{b}] {len(ps)} problems", *ps[:10], sep="\n  ")
    print(f"{len(rows)}/{len(expected)} proposals valid; missing {len(missing)}")
    print("by confidence:", dict(collections.Counter(v[2] for v in rows.values())))
    with open(root / "dtk/lib_read_names.tsv", "w", encoding="utf-8", newline="\n") as f:
        f.write("address\tdtk_name\tsymbol\tqualified\tconfidence\tnote\tbatch\n")
        for a in sorted(rows):
            sym, q, conf, note, batch = rows[a]
            f.write(f"{a}\t{expected[a][1]}\t{sym}\t{q}\t{conf}\t{note}\t{batch}\n")


def apply(root):
    # 1. dtk/name_corrections.tsv overrides any name (fixes reference matches proven wrong by code)
    # 2. dtk/lib_read_names.tsv names only functions still called fn_*
    corr_path, ledger = root / "dtk/name_corrections.tsv", root / "dtk/lib_read_names.tsv"
    corrections = ({r["address"]: r["symbol"] for r in csv.DictReader(open(corr_path, encoding="utf-8"), delimiter="\t")}
                   if corr_path.exists() else {})
    names = ({r["address"]: r["symbol"] for r in csv.DictReader(open(ledger, encoding="utf-8"), delimiter="\t")
              if r["confidence"] != "keep"} if ledger.exists() else {})
    path, lines = symbols(root)
    out, nc, n = [], 0, 0
    for l in lines:
        m = LINE.match(l)
        if m and "type:function" in m.group(4):
            a = m.group(3)
            if a in corrections and m.group(1) != corrections[a]:
                l = corrections[a] + m.group(2) + a + m.group(4)
                nc += 1
            elif a not in corrections and m.group(1).startswith("fn_") and a in names:
                l = names[a] + m.group(2) + a + m.group(4)
                n += 1
        out.append(l)
    path.write_text("\n".join(out) + "\n", encoding="utf-8", newline="\n")
    print(f"applied {nc} corrections and {n} names read from code")


if __name__ == "__main__":
    {"merge": merge, "apply": apply}[sys.argv[2]](Path(sys.argv[1]))
