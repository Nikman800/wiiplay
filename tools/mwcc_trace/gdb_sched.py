"""gdb-side half of `tools/mwcc_trace.py --sched`: the backend instruction scheduler (Scheduler.c).

Records, for every basic block the scheduler handles in one function (both runs: before register
allocation, scheduleinstructions(0), and after it, scheduleinstructions(1)), the dependence graph it
built and the order it picked the instructions in. Environment as in gdb_regalloc.py
(MWTRACE_FUNC, MWTRACE_NTH, MWTRACE_OUT). Addresses and layouts: addrs.py, "Scheduler".
"""
import json
import os
import struct
import sys

import gdb

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import addrs as A  # noqa: E402

inf = None


def mem(addr, n):
    return bytes(inf.read_memory(addr, n))


def u32(addr):
    return struct.unpack("<I", mem(addr, 4))[0]


def u16(addr):
    return struct.unpack("<H", mem(addr, 2))[0]


def reg(name):
    return int(gdb.parse_and_eval(f"${name}")) & 0xFFFFFFFF


def cstr(addr, limit=256):
    try:
        b = mem(addr, limit)
    except gdb.MemoryError:
        return None
    return b.split(b"\0", 1)[0].decode("latin-1")


def obj_name(obj):
    if not obj:
        return None
    try:
        hn = u32(obj + A.OBJ_NAME)
        return cstr(hn + A.HASHNAME_TEXT) if hn else None
    except gdb.MemoryError:
        return None


OPNAME = {}


def read_instr(p):
    """PCode -> [opname, args, flags] (args as in gdb_regalloc.dump_pcode)"""
    raw = mem(p, A.PC_ARGS)
    op = struct.unpack_from("<H", raw, A.PC_OP)[0]
    argc = struct.unpack_from("<h", raw, A.PC_ARGC)[0]
    flags = struct.unpack_from("<I", raw, A.PC_FLAGS)[0]
    if op not in OPNAME:
        OPNAME[op] = (cstr(u32(A.OPCODEINFO + A.OPCODEINFO_SIZE * op), 16) or f"op{op}").lower()
    args = []
    data = mem(p + A.PC_ARGS, A.ARG_SIZE * argc) if argc > 0 else b""
    for i in range(max(argc, 0)):
        a = data[i * A.ARG_SIZE:(i + 1) * A.ARG_SIZE]
        kind, cls = a[A.ARG_KIND], a[A.ARG_CLASS]
        if kind == 0:
            args.append([0, cls, struct.unpack_from("<h", a, A.ARG_REG)[0],
                         struct.unpack_from("<H", a, A.ARG_EFFECT)[0]])
        elif kind == 2:
            args.append([2, struct.unpack_from("<i", a, A.ARG_IMM)[0]])
        elif kind in (3, 4):
            obj = struct.unpack_from("<I", a, A.ARG_OBJ)[0]
            args.append([kind, struct.unpack_from("<i", a, A.ARG_IMM)[0], obj_name(obj)])
        else:
            args.append([kind])
    return [OPNAME[op], args, flags, read_alias(u32(p + A.PC_ALIAS)) if flags & 0x60006 else None]


def read_alias(al):
    """the memory alias a load/store carries (what the scheduler's may-alias test compares)"""
    if not al:
        return None
    kind = mem(al + A.ALIAS_KIND, 1)[0]
    obj = u32(al + A.ALIAS_OBJ)
    if kind == 2:
        return f"set{u32(al + A.ALIAS_INDEX)}" + ("=worst" if al == u32(A.WORST_CASE_ALIAS) else "")
    off, size = struct.unpack("<ii", mem(al + A.ALIAS_OFFSET, 8))
    name = obj_name(obj) or f"obj{obj:#x}"
    # may_alias treats two different objects as aliasing only when both are in the worst-case
    # set and one of them has the special type 0x6ad62e: mark those ("?" in worst case, "!" both)
    tag = ""
    try:
        wc = u32(A.WORST_CASE_ALIAS)
        idx = struct.unpack("<i", mem(al + A.ALIAS_INDEX, 4))[0]
        if wc and idx >= 0 and (u32(u32(wc + A.ALIAS_BITS) + 4 * (idx >> 5)) >> (idx & 31)) & 1:
            tag = "!" if obj and u32(obj + A.OBJ_TYPE) == A.ALIAS_ANY_TYPE else "?"
    except gdb.MemoryError:
        tag = ""
    return (f"{name}+{off}:{size}" if kind == 0 else f"{name}[*]") + tag


def read_dg(n):
    raw = mem(n, A.DG_SIZE)
    g = lambda o: struct.unpack_from("<H", raw, o)[0]  # noqa: E731
    return {"next": struct.unpack_from("<I", raw, A.DG_NEXT)[0],
            "succ": struct.unpack_from("<I", raw, A.DG_SUCC)[0],
            "instr": struct.unpack_from("<I", raw, A.DG_INSTR)[0],
            "lat": g(A.DG_LATENCY), "earliest": g(A.DG_EARLIEST), "latest": g(A.DG_LATEST),
            "height": g(A.DG_HEIGHT), "npred": g(A.DG_NPRED)}


class S:
    funcs = []
    hit = 0
    active = False
    run = -1           # scheduleinstructions call index within the function
    run_flag = []
    blocks = []
    cur = None         # block being scheduled
    edges = {}         # (from, to) -> set of kinds, recorded at addsuccessor


def instr_text(node):
    """as mwcc_trace.py prints it (op, operands, {alias}), for --drop-edge's regexes"""
    op, args, _, al = read_instr(u32(node + A.DG_INSTR))
    out = []
    for a in args:
        if a[0] == 0:
            out.append(f"{A.CLASS_PREFIX.get(a[1], '?')}{a[2]}")
        elif a[0] == 2:
            out.append(str(a[1]))
        elif a[0] in (3, 4):
            out.append(f"{a[2] or '?'}{'+' + str(a[1]) if a[1] else ''}")
    return f"{op} " + ", ".join(out) + (f"  {{{al}}}" if al else "")


DROP = []  # [(kind, from regex, to regex)] from MWTRACE_DROP "kind:from:to;..."


def dropped(kind, frm, to):
    import re
    for k, rf, rt in DROP:
        if k in (kind, "*") and (not rf or re.search(rf, instr_text(frm))) and \
                (not rt or re.search(rt, instr_text(to))):
            return True
    return False


def dump_graph(head):
    """the block's DGNode list (program order from head) -> nodes with successors"""
    order, idx = [], {}
    n = head
    while n and len(order) < 5000:
        d = read_dg(n)
        idx[n] = len(order)
        order.append((n, d))
        n = d["next"]
    nodes = []
    for n, d in order:
        succ = []
        e = d["succ"]
        while e:
            to, lat = u32(e + A.DGS_NODE), u16(e + A.DGS_LATENCY)
            succ.append([idx.get(to, -1), lat, sorted(S.edges.get((n, to), []))])
            e = u32(e + A.DGS_NEXT)
        nodes.append({"instr": read_instr(d["instr"]), "lat": d["lat"], "height": d["height"],
                      "latest": d["latest"], "npred": d["npred"], "succ": succ})
    S.cur["nodes"] = nodes
    S.cur["idx"] = idx
    S.cur["maxheight"] = u16(A.SCHED_MAXHEIGHT)


def pick(head, node, cycle):
    idx = S.cur["idx"]
    ready = []
    n = head
    while n:
        d = read_dg(n)
        if d["npred"] == 0 and d["earliest"] <= cycle:
            nunblock = 0
            e = d["succ"]
            while e:
                if u16(u32(e + A.DGS_NODE) + A.DG_NPRED) == 1:
                    nunblock += 1
                e = u32(e + A.DGS_NEXT)
            ready.append([idx.get(n, -1), d["latest"], nunblock, d["height"]])
        n = d["next"]
    S.cur["picks"].append({"cycle": cycle, "node": idx.get(node, -1), "ready": ready})


def finish():
    with open(os.environ["MWTRACE_OUT"], "w") as f:
        for b in S.blocks:
            b.pop("idx", None)
        json.dump({"functions": S.funcs, "runs": S.run_flag, "blocks": S.blocks,
                   "tiebreak_opclass": S.tiebreak,
                   "dropped": S.ndropped}, f)


def main():
    global inf
    want = os.environ["MWTRACE_FUNC"]
    nth = int(os.environ.get("MWTRACE_NTH", "1"))
    gdb.execute("set pagination off")
    gdb.execute("set confirm off")
    # stepping over a software breakpoint must not copy the 32-bit instruction elsewhere
    gdb.execute("set displaced-stepping off")
    gdb.execute("handle SIGSEGV nostop noprint pass")
    gdb.execute("starti", to_string=True)
    inf = gdb.selected_inferior()
    gdb.execute(f"hbreak *{A.ENTRY:#x}", to_string=True)
    gdb.execute("continue", to_string=True)
    gdb.execute("delete", to_string=True)
    # the image is mapped now: software breakpoints (more than the 4 debug registers)
    for a in (A.CODEGEN, A.SCHEDULEINSTRUCTIONS, A.SCHEDULEBLOCK, A.ADDSUCCESSOR,
              A.SCHED_GRAPH_DONE, A.SCHED_PICKED):
        gdb.execute(f"break *{a:#x}", to_string=True)
    S.tiebreak = None
    S.ndropped = 0
    for spec in filter(None, os.environ.get("MWTRACE_DROP", "").split(";")):
        k, rf, rt = (spec.split(":", 2) + ["", ""])[:3]
        DROP.append((k, rf, rt))
    while True:
        try:
            gdb.execute("continue", to_string=True)
        except gdb.error:
            break
        if not inf.pid or not inf.threads():
            break
        pc = reg("pc")
        sp = reg("rsp")
        if pc == A.CODEGEN:
            arg = u32(sp + 4)  # codegen(struct *): +4 = Object *func
            name = obj_name(u32(arg + 4)) if arg else None
            if S.active:
                if os.environ.get("MWTRACE_FINISH"):
                    # let the compiler write the object: what-if drops apply to the traced function only
                    gdb.execute("delete", to_string=True)
                    finish()
                    gdb.execute("continue", to_string=True)
                    gdb.execute("quit")
                break
            S.funcs.append(name)
            if name == want:
                S.hit += 1
                S.active = S.hit == nth
        elif not S.active:
            continue
        elif pc == A.SCHEDULEINSTRUCTIONS:
            S.run += 1
            S.run_flag.append(mem(sp + 4, 1)[0])
            S.tiebreak = u32(A.SCHED_TIEBREAK_ON)
        elif pc == A.SCHEDULEBLOCK:
            blk = u32(sp + 4)
            S.edges = {}
            S.cur = {"run": S.run, "block": blk, "picks": []}
            S.blocks.append(S.cur)
        elif pc == A.ADDSUCCESSOR:
            ret, frm, to, lat = u32(sp), u32(sp + 4), u32(sp + 8), mem(sp + 12, 1)[0]
            kind = A.SCHED_EDGE_KINDS.get(ret, f"?{ret:#x}")
            if frm != to and dropped(kind, frm, to):
                # what-if: return from addsuccessor at once (cdecl, the caller pops the args)
                gdb.execute(f"set $rip = {ret:#x}")
                gdb.execute(f"set $rsp = $rsp + 4")
                S.ndropped += 1
                continue
            S.edges.setdefault((frm, to), set()).add(kind)
        elif pc == A.SCHED_GRAPH_DONE:
            dump_graph(reg("rbp"))
        elif pc == A.SCHED_PICKED:
            node = reg("rax")
            if node:
                pick(reg("rbp"), node, u16(sp + 8))
    finish()
    try:
        gdb.execute("kill")
    except gdb.error:
        pass
    gdb.execute("quit")


main()
