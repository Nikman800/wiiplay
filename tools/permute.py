"""Search natural source variations of one function for a better match (register/scheduling).

Usage: permute.py <unit> <symbol> [--name SRCNAME] [--iters N] [--jobs J] [--seed S] [--out FILE]

  symbol   the mangled function symbol to score (as in `unit.py status`/`cmp`)
  --name   the identifier to find the definition by in the source (default: symbol before "__")

Mutations stay inside the function body and are the kinds of change that decided matches here:
swapping two adjacent simple statements, moving a local declaration within its block, and swapping
the operands of a commutative operator (+ * & | ^ == !=). Every round compiles J variants in
parallel (tools/cccmd.py, lock-free) and scores the symbol with tools/objcmp.py; better variants
are kept, equal ones are accepted half the time so the search can cross plateaus.

The source tree is never written: the best variant goes to --out (default
/tmp/permute/<symbol>.<ext>) with a diff printed. Review it before using it: a permuted source must
still read like plausible original code (scrambled declaration lists are not).
"""
import argparse
import difflib
import random
import re
import shutil
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import cccmd  # noqa: E402
import objcmp  # noqa: E402

ROOT = cccmd.ROOT
KEYWORDS = ("return", "break", "continue", "goto", "case", "default", "if", "else", "for", "while",
            "do", "switch", "#")
DECL = re.compile(r"^\s*(?:const\s+|static\s+|register\s+|volatile\s+)*[A-Za-z_][\w:<>]*(?:\s*[*&]+\s*|\s+)"
                  r"[*&]*[A-Za-z_]\w*(?:\[[^\]]*\])?(?:\s*=\s*[^;]+)?(?:\s*,\s*[*&]*[A-Za-z_]\w*(?:\s*=\s*[^;,]+)?)*;\s*$")
COMM = re.compile(r"([A-Za-z_][\w.\->\[\]]*|\d[\w.]*)\s*(\+|\*|&|\||\^|==|!=)\s*([A-Za-z_][\w.\->\[\]]*|\d[\w.]*)")


def find_body(lines, name):
    """(first, last) line indexes of the body braces of name's definition"""
    for i, l in enumerate(lines):
        if re.search(r"\b" + re.escape(name) + r"\s*\(", l) and not l.rstrip().endswith(";"):
            j = i
            while j < len(lines) and "{" not in lines[j]:
                if lines[j].rstrip().endswith(";"):
                    break
                j += 1
            if j >= len(lines) or "{" not in lines[j]:
                continue
            depth = 0
            for k in range(j, len(lines)):
                depth += lines[k].count("{") - lines[k].count("}")
                if depth == 0:
                    return j, k
    return None


def simple(l):
    s = l.strip()
    return s.endswith(";") and "{" not in s and "}" not in s and not s.startswith(KEYWORDS)


def indent(l):
    return len(l) - len(l.lstrip())


def mutate(lines, lo, hi, rng):
    body = list(range(lo + 1, hi))
    kinds = ["swap", "decl", "comm"]
    rng.shuffle(kinds)
    for kind in kinds:
        if kind == "swap":
            cand = [i for i in body[:-1] if simple(lines[i]) and simple(lines[i + 1])
                    and indent(lines[i]) == indent(lines[i + 1])]
            if cand:
                i = rng.choice(cand)
                lines[i], lines[i + 1] = lines[i + 1], lines[i]
                return True
        elif kind == "decl":
            cand = [i for i in body if DECL.match(lines[i]) and not lines[i].strip().startswith(KEYWORDS)]
            moves = [(i, d) for i in cand for d in (-1, 1)
                     if lo < i + d < hi and DECL.match(lines[i + d]) and indent(lines[i + d]) == indent(lines[i])]
            if moves:
                i, d = rng.choice(moves)
                lines[i], lines[i + d] = lines[i + d], lines[i]
                return True
        else:
            cand = [(i, m) for i in body for m in COMM.finditer(lines[i]) if "#" not in lines[i]]
            if cand:
                i, m = rng.choice(cand)
                l = lines[i]
                lines[i] = l[:m.start()] + f"{m.group(3)} {m.group(2)} {m.group(1)}" + l[m.end():]
                return True
    return False


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("unit")
    ap.add_argument("symbol")
    ap.add_argument("--name")
    ap.add_argument("--iters", type=int, default=40, help="rounds (each compiles --jobs variants)")
    ap.add_argument("--jobs", type=int, default=8)
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--out")
    ap.add_argument("--src", help="start from this file instead of src/<unit> (still never written)")
    ap.add_argument("--include-first", help="shadow include tree searched before include/ (for header experiments)")
    a = ap.parse_args()
    rng = random.Random(a.seed)
    src_path = ROOT / "src" / a.unit
    orig = Path(a.src or src_path).read_text(encoding="utf-8", errors="replace").splitlines(keepends=True)
    name = a.name or a.symbol.split("__")[0] or a.symbol
    span = find_body(orig, name)
    if not span:
        sys.exit(f"definition of {name} not found in {src_path}")
    target = objcmp.Obj(ROOT / "build/RHAE01/obj" / Path(a.unit).with_suffix(".o"))
    if a.symbol not in target.funcs:
        sys.exit(f"{a.symbol} not in the target object")
    tmp = Path(tempfile.mkdtemp(prefix="permute_"))
    ext = src_path.suffix

    errors, rmap = [], {}

    def evaluate(i, lines):
        try:
            return _evaluate(i, lines)
        except Exception as e:  # a broken variant must not stop the search
            errors.append(repr(e))
            return -1.0

    def _evaluate(i, lines):
        d = tmp / str(i)
        d.mkdir(exist_ok=True)
        f = d / (src_path.stem + ext)
        f.write_text("".join(lines), encoding="utf-8")
        obj = cccmd.compile_variant(a.unit, src=f, outdir=d, include_first=a.include_first)
        if obj is None:
            return -1.0
        o = objcmp.Obj(obj)
        if not rmap:  # pair the target's generated names with the source's once, from the start
            rmap.update(objcmp.reloc_map(target, o))
        e, t = objcmp.compare(target, o, a.symbol, rmap)
        return e / t if t else 0.0

    try:
        best = cur = orig[:]
        best_s = cur_s = evaluate("base", orig)
        print(f"{a.symbol}: start {best_s:.4f}", flush=True)
        with ThreadPoolExecutor(a.jobs) as ex:
            for it in range(a.iters):
                if best_s >= 1.0:
                    break
                cands = []
                for j in range(a.jobs):
                    c = cur[:]
                    lo, hi = find_body(c, name)
                    for _ in range(rng.randint(1, 3)):
                        mutate(c, lo, hi, rng)
                    cands.append(c)
                scores = list(ex.map(lambda p: evaluate(f"{it}_{p[0]}", p[1]), enumerate(cands)))
                s, c = max(zip(scores, cands), key=lambda x: x[0])
                if s > cur_s or (s == cur_s and rng.random() < 0.5):
                    cur, cur_s = c, s
                if s > best_s:
                    best, best_s = c, s
                    print(f"  round {it}: {best_s:.4f}", flush=True)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    out = Path(a.out or f"/tmp/permute/{a.symbol[:80]}{ext}")
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text("".join(best), encoding="utf-8")
    print(f"best {best_s:.4f} -> {out}" + ("  (MATCH)" if best_s >= 1.0 else "")
          + (f"  ({len(errors)} variants raised, first: {errors[0]})" if errors else ""))
    sys.stdout.writelines(difflib.unified_diff(orig, best, "src/" + a.unit, str(out), n=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
