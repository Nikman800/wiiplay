"""gdb-side half of tools/mwcc_trace.py: runs inside `gdb -batch -x` while wibo runs mwcceppc.exe.

It stops in CodeWarrior's register allocator and writes the interference graph and the colouring
of one function as JSON. Parameters come from the environment:
  MWTRACE_FUNC   function identifier to trace (the plain name, e.g. "conjugate")
  MWTRACE_NTH    which compiled function of that name to take (1-based, default 1)
  MWTRACE_OUT    JSON output path
Addresses and structure layouts are in addrs.py (found by reading the 3.0a5.2 binary; see there).
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


def s16(addr):
    return struct.unpack("<h", mem(addr, 2))[0]


def u8(addr):
    return mem(addr, 1)[0]


def cstr(addr, limit=256):
    try:
        b = mem(addr, limit)
    except gdb.MemoryError:
        return None
    return b.split(b"\0", 1)[0].decode("latin-1")


def esp():
    return int(gdb.parse_and_eval("$rsp")) & 0xFFFFFFFF


def obj_name(obj):
    """Object -> its identifier (HashNameNode text), or None"""
    if not obj:
        return None
    try:
        hn = u32(obj + A.OBJ_NAME)
        if not hn:
            return None
        return cstr(hn + A.HASHNAME_TEXT)
    except gdb.MemoryError:
        return None


def read_node(base, v):
    n = base + v * A.IG_SIZE
    raw = mem(n, A.IG_SIZE)
    node = {
        "addr": n,
        "next": struct.unpack_from("<I", raw, A.IG_NEXT)[0],
        "obj": struct.unpack_from("<I", raw, A.IG_OBJ)[0],
        "cost": struct.unpack_from("<i", raw, A.IG_COST)[0],
        "vreg": struct.unpack_from("<H", raw, A.IG_VREG)[0],
        "reg": struct.unpack_from("<h", raw, A.IG_REG)[0],
        "flags": struct.unpack_from("<H", raw, A.IG_FLAGS)[0],
        "degree_left": struct.unpack_from("<h", raw, A.IG_DEGREE)[0],
        "edges": struct.unpack_from("<I", raw, A.IG_EDGES)[0],
    }
    # neighbour list: each edge {.., next_lo@+4, next_hi@+8, lo@+0xc, hi@+0xe} sits on both ends' lists
    nb = []
    e = node["edges"]
    guard = 0
    while e and guard < 100000:
        lo, hi = struct.unpack("<HH", mem(e + A.EDGE_LO, 4))
        if v < hi:
            nb.append(hi)
            e = u32(e + A.EDGE_NEXT_LO)
        else:
            nb.append(lo)
            e = u32(e + A.EDGE_NEXT_HI)
        guard += 1
    node["neighbours"] = nb
    node["var"] = obj_name(node["obj"])
    return node


def coalesce_root(v, limit):
    table = u32(A.COALESCE_TABLE)
    if not table:
        return v
    seen = 0
    while seen < 1000:
        p = u16(table + 2 * v)
        if p == v or p > limit:
            return p
        v = p
        seen += 1
    return v


def dump_pcode():
    """the function's PCode before colouring, every class still in virtual registers"""
    lines = []
    b = u32(A.PCBASICBLOCKS)
    opname = {}
    nb = 0
    while b and nb < 10000:
        lines.append(f"B{nb}:")
        p = u32(b + A.BLOCK_FIRST)
        while p:
            raw = mem(p, A.PC_ARGS)
            op = struct.unpack_from("<H", raw, A.PC_OP)[0]
            argc = struct.unpack_from("<h", raw, A.PC_ARGC)[0]
            if op not in opname:
                opname[op] = (cstr(u32(A.OPCODEINFO + A.OPCODEINFO_SIZE * op), 16) or f"op{op}").lower()
            args = []
            data = mem(p + A.PC_ARGS, A.ARG_SIZE * max(argc, 0)) if argc > 0 else b""
            for i in range(max(argc, 0)):
                a = data[i * A.ARG_SIZE:(i + 1) * A.ARG_SIZE]
                kind, cls = a[A.ARG_KIND], a[A.ARG_CLASS]
                if kind == 0:
                    eff = struct.unpack_from("<H", a, A.ARG_EFFECT)[0]
                    reg = struct.unpack_from("<h", a, A.ARG_REG)[0]
                    args.append([0, cls, reg, eff])
                elif kind == 2:
                    args.append([2, struct.unpack_from("<i", a, A.ARG_IMM)[0]])
                elif kind in (3, 4):
                    obj = struct.unpack_from("<I", a, A.ARG_OBJ)[0]
                    args.append([kind, struct.unpack_from("<i", a, A.ARG_IMM)[0], obj_name(obj)])
                else:
                    args.append([kind])
            lines.append([opname[op], args])
            p = u32(p + A.PC_NEXT)
        b = u32(b + A.BLOCK_NEXT)
        nb += 1
    return lines


class State:
    target_hit = 0
    active = False
    passes = []
    funcs = []
    cost_picks = set()
    pcode = None


def dump_pass(list_head):
    rclass = u8(A.COLORING_CLASS)
    n_real = u32(A.N_REAL_REGISTERS + 4 * rclass)
    used = u32(A.USED_VIRTUAL_REGISTERS + 4 * rclass)
    base = u32(A.IG_NODES)
    nodes = {}
    for v in range(0, used):
        try:
            nodes[v] = read_node(base, v)
        except gdb.MemoryError:
            pass
    order = []
    p = list_head
    while p and len(order) < used + 5:
        v = (p - base) // A.IG_SIZE
        order.append(v)
        p = u32(p + A.IG_NEXT)
    for v, n in nodes.items():
        if n["flags"] & A.FLAG_COALESCED:
            n["coalesced_into"] = coalesce_root(v, used)
    State.passes.append({
        "class": rclass,
        "n_real": n_real,
        "used": used,
        "order": order,
        "cost_picks": sorted(State.cost_picks),
        "result": int(gdb.parse_and_eval("$rax")) & 0xFFFFFFFF,
        "nodes": {str(v): {k: n[k] for k in ("cost", "reg", "flags", "var", "neighbours",
                                              "coalesced_into") if k in n}
                  for v, n in nodes.items() if v >= n_real or n["neighbours"]},
    })
    State.cost_picks = set()


def finish():
    with open(os.environ["MWTRACE_OUT"], "w") as f:
        json.dump({"functions": State.funcs, "passes": State.passes, "pcode": State.pcode}, f)
    gdb.execute("kill")
    gdb.execute("quit")


def main():
    global inf
    want = os.environ["MWTRACE_FUNC"]
    nth = int(os.environ.get("MWTRACE_NTH", "1"))
    gdb.execute("set pagination off")
    gdb.execute("set confirm off")
    gdb.execute("handle SIGSEGV nostop noprint pass")
    gdb.execute("starti", to_string=True)
    inf = gdb.selected_inferior()
    # the PE image is mapped by wibo after exec; a hardware breakpoint needs no mapped memory
    gdb.execute(f"hbreak *{A.ENTRY:#x}", to_string=True)
    gdb.execute("continue", to_string=True)
    gdb.execute("delete", to_string=True)
    gdb.execute(f"hbreak *{A.COLORINSTRUCTIONS:#x}", to_string=True)
    gdb.execute(f"hbreak *{A.COLORGRAPH_RET:#x}", to_string=True)
    gdb.execute(f"hbreak *{A.SIMPLIFY_COST_PICK:#x}", to_string=True)
    while True:
        try:
            gdb.execute("continue", to_string=True)
        except gdb.error:
            break
        if not inf.pid or not inf.threads():
            break
        pc = int(gdb.parse_and_eval("$pc")) & 0xFFFFFFFF
        if pc == A.COLORINSTRUCTIONS:
            name = obj_name(u32(esp() + 4))
            if State.active:  # the next function: done
                finish()
                return
            State.funcs.append(name)
            if name == want:
                State.target_hit += 1
                if State.target_hit == nth:
                    State.active = True
                    State.pcode = dump_pcode()
        elif pc == A.COLORGRAPH_RET and State.active:
            dump_pass(u32(esp() + 4 + A.COLORGRAPH_RET_ARG_OFS))
        elif pc == A.SIMPLIFY_COST_PICK and State.active:
            edi = int(gdb.parse_and_eval("$rdi")) & 0xFFFFFFFF
            base = u32(A.IG_NODES)
            State.cost_picks.add((edi - base) // A.IG_SIZE)
    with open(os.environ["MWTRACE_OUT"], "w") as f:
        json.dump({"functions": State.funcs, "passes": State.passes, "pcode": State.pcode}, f)


main()
