"""Show how CodeWarrior GC/3.0a5.2 coloured the registers of one function.

Usage: mwcc_trace.py <unit> <function> [--nth N] [--src FILE] [--class gpr|fpr|all] [--json OUT]
       mwcc_trace.py <unit> <function> --frontend [--nth N] [--src FILE] [--full] [--log OUT]
       mwcc_trace.py <unit> <function> --sched [--nth N] [--src FILE] [--run 0|1] [--grep RE]
                     [--drop-edge KIND[:FROM_RE[:TO_RE]]]... [--score MANGLED]... [--obj OUT]

  mwcc_trace.py egg/math/eggQuat.cpp conjugate
  mwcc_trace.py Pack/RPGraphics/RPGrpModelMaterial.cpp Calc --nth 2

<function> is the plain identifier (no class, no mangling); --nth picks among functions of that
name in compile order (the tool lists them). The unit is compiled with its exact build command
(tools/cccmd.py) under gdb, with wibo as the loader; gdb stops in the register allocator
(tools/mwcc_trace/gdb_regalloc.py, addresses in tools/mwcc_trace/addrs.py) and reads its state.

For each colouring pass of each register class it prints the virtual registers in colouring order
(the simplify stack from the top: the first one coloured has the highest priority), the physical
register each got, the spill cost, the full degree, the source variable when the node has one,
and the neighbours (other virtual registers and the physical ones it may not take). Nodes simplify
had to pick by lowest cost/degree (no node left with degree < K) are marked "pick"; "lvl" counts
the simplify sweeps (a new level starts where the vreg order wraps around).
Coalesced registers (an `mr` merged away) are listed after, with the register they joined:
they keep their edges, so they still count as neighbours (see tools/LEVERS.md, registers).

--frontend instead turns on the optimizer's own IR logger (IroDump, still in the release binary;
tools/mwcc_trace/gdb_frontend.py) and prints the function's flowgraph and linear IR as it was
after the first phase, then, per later phase, a diff against the previous dump ("no change" when
the phase left the IR alone). Linear nodes are "index: op operands <flags>"; operands are node
indices; "@123" are compiler temporaries. Constructors are "__ct", destructors "__dt".

--sched instead traces the backend instruction scheduler (Scheduler.c; tools/mwcc_trace/gdb_sched.py,
selection rules in addrs.py). It runs per basic block, once before register allocation (run 0,
virtual registers) and once after it (run 1, real registers; the order that reaches the object).
Per block it prints the dependence graph in the unscheduled order: latency, height (longest
latency path to the block end), "late" (critical path - height: the cycle by which it must
start), predecessor count and the successors "#n:lat kind" (raw/war/waw registers, mem-raw/
mem-war/mem-waw for a load/store pair the alias test could not separate, serial, branch).
Loads (L) and stores (S) show their alias in braces: object+offset:size, object[*] (anywhere in
it), or setN (a points-to set; "=worst" is the worst case, which aliases everything).
Then the schedule: per pick the cycle, the node picked and the ready list as
"#n late/unblocks/height" in the order selectinstruction compares them (program order; a later
node replaces the current best only by: late <= cycle, then more unblocked successors, then
greater height). --grep keeps the blocks with an instruction matching the regex.
--drop-edge is a what-if: the scheduler skips the matching dependence edges, so the trace shows
the order it would pick without them (e.g. which alias edge holds an instruction back), and
--score MANGLED lets the compile finish and counts the differing words against the original
object (tools/objcmp.py), so a what-if can be scored before looking for a source form for it
(--obj keeps the object). Example: the pane loads of RPCPadCursor::SetTarget may not alias the
target stores in the original:
  mwcc_trace.py Pack/RPParty/RPPartyCommon/RPCPadCursor_2.cpp SetTarget --sched --run 1 \
      --grep 'lfs .*, 76 ' --drop-edge 'mem-war:@18889:' --score SetTarget__12RPCPadCursorFPQ34nw4r3lyt4Pane

Requires gdb (x86-64; wibo runs the 32-bit code in compatibility mode, hardware breakpoints).
sjiswrap is dropped from the command: the source is read as is (fine for ASCII sources).
"""
import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parent / "mwcc_trace"))
import cccmd  # noqa: E402
import addrs as A  # noqa: E402

ROOT = cccmd.ROOT
HERE = Path(__file__).resolve().parent / "mwcc_trace"


def build_command(unit, src, outdir):
    cmd = cccmd.base_command(unit)
    if "3.0a5.2" not in cmd:
        sys.exit("this unit is not built with GC/3.0a5.2; only that compiler's addresses are known")
    cmd = cmd.replace(" build/tools/sjiswrap.exe", "")
    orig = "src/" + unit
    if src:
        src = os.path.abspath(src)
        cmd = cmd.replace(" -c " + orig, f" -i {Path(orig).parent.as_posix()} -c {src}", 1)
    cmd = re.sub(r" -o \S+", f" -o {outdir}", cmd, count=1)
    return cmd.replace(" -MMD", "")


def run(unit, func, nth, src, timeout):
    with tempfile.TemporaryDirectory(prefix="mwtrace_") as tmp:
        out = Path(tmp) / "trace.json"
        cmd = build_command(unit, src, tmp)
        env = dict(os.environ, MWTRACE_FUNC=func, MWTRACE_NTH=str(nth), MWTRACE_OUT=str(out))
        gdbcmd = f"gdb -q -nx -batch -iex 'set debuginfod enabled off' -x {HERE / 'gdb_regalloc.py'} --args {cmd}"
        r = subprocess.run(gdbcmd, shell=True, cwd=ROOT, env=env, capture_output=True, text=True,
                           timeout=timeout)
        if not out.exists():
            sys.stderr.write(r.stdout[-3000:] + r.stderr[-3000:])
            sys.exit("gdb produced no trace")
        return json.loads(out.read_text())


def run_frontend(unit, func, nth, src, timeout, keep_log=None):
    """compile under gdb with the optimizer's IR logger on; returns (info, log text)"""
    with tempfile.TemporaryDirectory(prefix="mwtrace_") as tmp:
        out = Path(tmp) / "fe.json"
        log = Path(tmp) / "iro.log"
        cmd = build_command(unit, src, tmp)
        env = dict(os.environ, MWTRACE_FUNC=func, MWTRACE_NTH=str(nth), MWTRACE_OUT=str(out),
                   MWTRACE_LOG=str(log))
        gdbcmd = f"gdb -q -nx -batch -iex 'set debuginfod enabled off' -x {HERE / 'gdb_frontend.py'} --args {cmd}"
        r = subprocess.run(gdbcmd, shell=True, cwd=ROOT, env=env, capture_output=True, text=True,
                           timeout=timeout)
        if not out.exists() or not log.exists():
            sys.stderr.write(r.stdout[-3000:] + r.stderr[-3000:])
            sys.exit("gdb produced no IR log")
        text = log.read_text(errors="replace")
        if keep_log:
            Path(keep_log).write_text(text)
        return json.loads(out.read_text()), text


def fe_sections(text, func):
    """the traced function's part of the log -> [(header, notes, dump lines)] per phase"""
    lines = text.splitlines()
    first = next((i for i, l in enumerate(lines) if l.startswith(f"Dumping function {func} ")), None)
    if first is None:
        return []
    start = max((i for i in range(first) if lines[i].startswith("Starting function")), default=0)
    end = next((i for i in range(first, len(lines)) if lines[i].startswith("Starting function")),
               len(lines))
    secs, notes = [], []
    for l in lines[start + 1:end]:
        m = re.match(r"(Dumping|Optimizing) function \S+ ((?:after|before) .*?)(?: \(tick \d+\))?$", l)
        if m:
            secs.append([m.group(2), notes, []])
            notes = []
        elif l.startswith("Dumps for pass"):
            notes.append(l)
        elif secs and (l.startswith((" ", "Flowgraph", "Succ =", "Pred =", "MustReach", "LoopDepth",
                                     "Dom:")) or re.match(r"\d+: ", l) or not l.strip()):
            if l.strip() and not re.match(r"\s*\d+: Nop\b", l):  # Nops: deleted nodes
                secs[-1][2].append(l)
        elif l.strip() and not l.startswith(("*****", "-----")):
            notes.append(l)
    return secs


def fe_blocks(dump):
    """a flowgraph dump -> [(header line, body lines)]; MustReach/Dom lines dropped"""
    blocks = []
    for l in dump:
        if l.startswith("Flowgraph node"):
            blocks.append((l, []))
        elif blocks and not l.startswith(("MustReach", "Dom:")):
            blocks[-1][1].append(l)
    return blocks


def fe_select(dump, pats, after):
    """the blocks whose lines match every regex in pats, each with the `after` blocks after it"""
    blocks = fe_blocks(dump)
    out = []
    for i, (hdr, body) in enumerate(blocks):
        if all(any(re.search(p, l) for l in body) for p in pats):
            for h, b in blocks[i:i + 1 + after]:
                out += [h] + b
    return out


def print_frontend(secs, full, context, grep=None, after=0):
    import difflib
    prev = None
    quiet = []

    def flush():
        if quiet:
            print(f"     [{quiet[0]}..{quiet[-1]}] no change" if len(quiet) > 1 else f"     [{quiet[0]}] no change")
            quiet.clear()

    for k, (hdr, notes, dump) in enumerate(secs):
        if grep:
            dump = fe_select(dump, grep, after)
        for n in notes:
            flush()
            print(f"   ## {n}")
        if prev is None or (full and dump != prev):
            flush()
            print(f"==== [{k}] {hdr}")
            print("\n".join(dump))
        elif dump != prev:
            flush()
            print(f"==== [{k}] {hdr}: changed")
            for d in difflib.unified_diff(prev, dump, lineterm="", n=context):
                if not d.startswith(("---", "+++")):
                    print(d)
        else:
            quiet.append(k)
        prev = dump
    flush()


def run_sched(unit, func, nth, src, timeout, drop=(), obj_out=None):
    with tempfile.TemporaryDirectory(prefix="mwtrace_") as tmp:
        out = Path(tmp) / "sched.json"
        cmd = build_command(unit, src, tmp)
        env = dict(os.environ, MWTRACE_FUNC=func, MWTRACE_NTH=str(nth), MWTRACE_OUT=str(out),
                   MWTRACE_DROP=";".join(drop or ()))
        if obj_out:
            env["MWTRACE_FINISH"] = "1"
        gdbcmd = f"gdb -q -nx -batch -iex 'set debuginfod enabled off' -x {HERE / 'gdb_sched.py'} --args {cmd}"
        r = subprocess.run(gdbcmd, shell=True, cwd=ROOT, env=env, capture_output=True, text=True,
                           timeout=timeout)
        if not out.exists():
            sys.stderr.write(r.stdout[-3000:] + r.stderr[-3000:])
            sys.exit("gdb produced no scheduler trace")
        t = json.loads(out.read_text())
        if obj_out:
            objs = list(Path(tmp).glob("*.o"))
            if not objs:
                sys.stderr.write(r.stdout[-2000:] + r.stderr[-2000:])
                sys.exit("the compiler wrote no object")
            import shutil
            shutil.copy(objs[0], obj_out)
        return t


def sched_instr_text(ins):
    op, args, flags = ins[:3]
    txt = f"{op:<7} " + ", ".join(t for t in (fmt_arg(a) for a in args) if t)
    if len(ins) > 3 and ins[3]:
        txt += f"  {{{ins[3]}}}"
    tag = "L" if flags & 0x20002 else "S" if flags & 0x40004 else " "
    return tag, txt


def print_sched(t, want_run, grep, ready_max):
    names = {0: "before register allocation", 1: "after register allocation"}
    for bi, b in enumerate(t["blocks"]):
        if want_run is not None and b["run"] != want_run:
            continue
        nodes = b.get("nodes", [])
        texts = [sched_instr_text(n["instr"]) for n in nodes]
        if grep and not any(re.search(grep, x) for _, x in texts):
            continue
        flag = t["runs"][b["run"]] if b["run"] < len(t["runs"]) else "?"
        print(f"== run {b['run']} ({names.get(flag, flag)}), block {bi}: {len(nodes)} instructions, "
              f"critical path {b.get('maxheight')}")
        print(f"{'#':>4}   {'instruction':<52} {'lat':>3} {'hgt':>3} {'late':>4} {'np':>2}  successors")
        for i, (n, (tag, x)) in enumerate(zip(nodes, texts)):
            succ = " ".join(f"#{j}:{lat}{' ' + ','.join(k) if k else ''}" for j, lat, k in
                            sorted(n["succ"]))
            print(f"{i:>4} {tag} {x:<52} {n['lat']:>3} {n['height']:>3} {n['latest']:>4} "
                  f"{n['npred']:>2}  {succ}")
        print("schedule (cycle: picked  | ready #n late/unblocks/height, program order):")
        for p in b["picks"]:
            rd = p["ready"]
            more = ""
            if ready_max and len(rd) > ready_max:
                more, rd = f" +{len(rd) - ready_max}", rd[:ready_max]
            r = " ".join(f"{'*' if j == p['node'] else ''}#{j} {l}/{u}/{h}" for j, l, u, h in rd)
            tag, x = texts[p["node"]] if 0 <= p["node"] < len(texts) else (" ", "?")
            print(f"  {p['cycle']:>3}: #{p['node']:<3} {tag} {x:<30} | {r}{more}")
        print("order: " + " ".join(f"#{p['node']}" for p in b["picks"]))
        print()


def rname(cls, v, n_real):
    p = A.CLASS_PREFIX.get(cls, "?")
    if v < 0:
        return "-"
    return f"{p}{v}" if v < n_real else f"{p}{v}"


def fmt_neighbours(cls, nb, n_real, nodes):
    phys = sorted({v for v in nb if v < n_real})
    virt = sorted({v for v in nb if v >= n_real})
    out = []
    # compress physical ranges r3-r12 etc.
    runs = []
    for v in phys:
        if runs and v == runs[-1][1] + 1:
            runs[-1][1] = v
        else:
            runs.append([v, v])
    p = A.CLASS_PREFIX.get(cls, "?")
    out += [f"{p}{a}" if a == b else f"{p}{a}-{b}" for a, b in runs]
    for v in virt:
        n = nodes.get(str(v), {})
        tag = ""
        if n.get("flags", 0) & A.FLAG_COALESCED:
            tag = "~"
        out.append(f"{p}{v}{tag}")
    return " ".join(out)


def fmt_arg(a):
    if a[0] == 0:
        return f"{A.CLASS_PREFIX.get(a[1], '?')}{a[2]}"
    if a[0] == 2:
        return str(a[1])
    if a[0] in (3, 4):
        return f"{a[2] or '?'}{'+' + str(a[1]) if a[1] else ''}"
    return None


def pcode_text(pcode):
    """listing lines, and per (class, vreg) the first instruction that defines it"""
    out, first_def = [], {}
    for ln in pcode or []:
        if isinstance(ln, str):
            out.append(ln)
            continue
        op, args = ln
        txt = f"    {op:<8} " + ", ".join(t for t in (fmt_arg(a) for a in args) if t)
        out.append(txt)
        for a in args:
            if a[0] == 0 and a[3] & 2:
                first_def.setdefault((a[1], a[2]), txt.strip())
    return out, first_def


def print_pass(i, ps, want_cls, first_def=None):
    cls = ps["class"]
    if want_cls != "all" and A.CLASS_NAMES.get(cls, "").lower() != want_cls:
        return
    n_real = ps["n_real"]
    nodes = ps["nodes"]
    p = A.CLASS_PREFIX.get(cls, "?")
    ok = "coloured" if ps["result"] else "SPILLED, recoloured after spill code"
    print(f"== {A.CLASS_NAMES.get(cls, cls)} pass {i}: virtual {p}{n_real}..{p}{ps['used'] - 1}, {ok}")
    print(f"{'#':>3} {'lvl':>3} {'vreg':>6} {'->':>4} {'cost':>6} {'deg':>4}  {'flags':<10} {'variable':<16} neighbours")
    picks = set(ps["cost_picks"])
    level = 1
    # stack order is reversed push order: levels are visible in push order (vreg ascending per sweep)
    push = list(reversed(ps["order"]))
    lv = {}
    prev = -1
    for v in push:
        if v in picks or v < prev:
            level += 1
        lv[v] = level
        prev = v if v not in picks else -1
    for k, v in enumerate(ps["order"], 1):
        n = nodes.get(str(v), {})
        fl = []
        f = n.get("flags", 0)
        if f & A.FLAG_SPILLED:
            fl.append("SPILL")
        if v in picks:
            fl.append("pick")
        if f & A.FLAG_PAIR:
            fl.append("pair")
        if f & A.FLAG_NOSPILL:
            fl.append("nospill")
        if f & A.FLAG_COALESCED_INTO:
            fl.append("join")
        nb = n.get("neighbours", [])
        if first_def and (not n.get("var") or n["var"].startswith("@")):
            d = first_def.get((cls, v), "")
            n["var"] = f"{n.get('var') or ''} = {d}".strip() if d else n.get("var")
        print(f"{k:>3} {lv.get(v, 0):>3} {p}{v:<5} {rname(cls, n.get('reg', -1), 999) if n.get('reg', -1) >= 0 else '-':>4} "
              f"{n.get('cost', 0):>6} {len(set(nb)):>4}  {','.join(fl):<10} {(n.get('var') or ''):<16} "
              f"{fmt_neighbours(cls, nb, n_real, nodes)}")
    co = [(int(v), n) for v, n in nodes.items() if n.get("flags", 0) & A.FLAG_COALESCED]
    if co:
        print("coalesced (merged by an mr; still counted as neighbours):")
        for v, n in sorted(co):
            into = n.get("coalesced_into", -1)
            print(f"    {p}{v:<5} -> {p}{into:<5} {(n.get('var') or ''):<16} deg {len(set(n.get('neighbours', [])))}")
    print()


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("unit")
    ap.add_argument("function")
    ap.add_argument("--nth", type=int, default=1)
    ap.add_argument("--src")
    ap.add_argument("--class", dest="cls", default="all", choices=["gpr", "fpr", "all"])
    ap.add_argument("--json", help="also write the raw trace here")
    ap.add_argument("--pcode", action="store_true", help="print the PCode before colouring")
    ap.add_argument("--timeout", type=int, default=600)
    ap.add_argument("--frontend", action="store_true",
                    help="dump the optimizer's IR (CodeWarrior's own IRO log) after each phase")
    ap.add_argument("--full", action="store_true", help="--frontend: every dump in full, not diffs")
    ap.add_argument("--context", type=int, default=4, help="--frontend: diff context lines")
    ap.add_argument("--log", help="--frontend: also save the raw IRO log (all functions) here")
    ap.add_argument("--grep", action="append",
                    help="--frontend: only the flowgraph blocks with a line matching this regex "
                         "(repeat: every regex must match in the block)")
    ap.add_argument("--after", type=int, default=0,
                    help="--frontend --grep: also show this many blocks after each matching one")
    ap.add_argument("--no-flags", action="store_true",
                    help="--frontend: drop the <flag> annotations of the linear nodes (less churn)")
    ap.add_argument("--notes", action="store_true",
                    help="--frontend: keep the optimizer's own notes (propagations, dead stores)")
    ap.add_argument("--sched", action="store_true",
                    help="trace the backend instruction scheduler (dependence graph and picks)")
    ap.add_argument("--run", type=int, choices=[0, 1],
                    help="--sched: only run 0 (before register allocation) or 1 (after)")
    ap.add_argument("--ready-max", type=int, default=12,
                    help="--sched: ready-list entries shown per pick (0 = all)")
    ap.add_argument("--drop-edge", action="append", metavar="KIND[:FROM_RE[:TO_RE]]",
                    help="--sched what-if: the scheduler never adds dependence edges of this kind "
                         "(raw, mem-war, ..., * for any) from an instruction matching FROM_RE to one "
                         "matching TO_RE (instruction text as printed, e.g. 'lfs f\\d+, r33, 76'); "
                         "repeatable. Only the trace changes; nothing is written")
    ap.add_argument("--score", action="append", metavar="SYMBOL",
                    help="--sched: let the compile finish and compare this function (mangled "
                         "name) with the original object (tools/objcmp.py); with --drop-edge this "
                         "scores the what-if")
    ap.add_argument("--obj", help="--sched: let the compile finish and keep the object here")
    a = ap.parse_args()
    if a.sched:
        obj_out = a.obj
        if a.score and not obj_out:
            obj_out = tempfile.mktemp(suffix=".o", prefix="mwtrace_")
        t = run_sched(a.unit, a.function, a.nth, a.src, a.timeout, a.drop_edge, obj_out)
        if a.drop_edge:
            print(f"what-if: {t.get('dropped', 0)} dependence edge(s) dropped ({'; '.join(a.drop_edge)})")
        if a.score:
            import objcmp
            target = next(ROOT.glob(f"build/*/obj/{Path(a.unit).with_suffix('.o').as_posix()}"))
            tgt, src = objcmp.Obj(str(target)), objcmp.Obj(obj_out)
            rmap = objcmp.reloc_map(tgt, src)
            for name in a.score:
                eq, tot = objcmp.compare(tgt, src, name, rmap)
                print(f"score {name}: {tot - eq} of {tot} words differ")
            if not a.obj:
                os.unlink(obj_out)
        hits = sum(1 for f in t["functions"] if f == a.function)
        print(f"{a.function}: {hits} function(s) of that name in this unit; tracing #{a.nth}")
        if a.json:
            Path(a.json).write_text(json.dumps(t, indent=1))
        if not t["blocks"]:
            print("no block scheduled (function not found, scheduling off, or blocks of <= 2 instructions)")
            print("compiled functions:", " ".join(f or "?" for f in t["functions"]))
            return 1
        print_sched(t, a.run, a.grep[0] if a.grep else None, a.ready_max)
        return 0
    if a.frontend:
        info, text = run_frontend(a.unit, a.function, a.nth, a.src, a.timeout, a.log)
        hits = sum(1 for f in info["functions"] if f == a.function)
        print(f"{a.function}: {hits} optimizer run(s) of that name in this unit; dumping #{a.nth}")
        secs = fe_sections(text, a.function) if info["traced"] else []
        if not secs:
            print("function not found; optimized functions:",
                  " ".join(f or "?" for f in info["functions"]))
            return 1
        if a.no_flags:
            secs = [(h, n, [re.sub(r" <[^<>]*>", "", l) for l in d]) for h, n, d in secs]
        if not a.notes:
            secs = [(h, [n for n in notes if n.startswith("Dumps for pass")], d) for h, notes, d in secs]
        print_frontend(secs, a.full, a.context, a.grep, a.after)
        return 0
    t = run(a.unit, a.function, a.nth, a.src, a.timeout)
    hits = [i for i, f in enumerate(t["functions"]) if f == a.function]
    print(f"{a.function}: {len(hits)} function(s) of that name in this unit; tracing #{a.nth}")
    if a.json:
        Path(a.json).write_text(json.dumps(t, indent=1))
    if not t["passes"]:
        print("no colouring pass recorded (function not found, or it needed no virtual registers)")
        print("compiled functions:", " ".join(f for f in t["functions"] if f))
        return 1
    listing, first_def = pcode_text(t.get("pcode"))
    if a.pcode:
        print("PCode before register allocation (virtual registers):")
        print("\n".join(listing))
        print()
    for i, ps in enumerate(t["passes"], 1):
        print_pass(i, ps, a.cls, first_def)
    return 0


if __name__ == "__main__":
    sys.exit(main())
