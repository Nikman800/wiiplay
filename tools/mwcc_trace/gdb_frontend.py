"""gdb-side half of `tools/mwcc_trace.py --frontend`: turns on CodeWarrior's own IR logger.

GC/3.0a5.2 still contains the optimizer's IR dumper (IroDump); only its enable byte is cleared.
This script, run inside `gdb -batch -x` while wibo runs mwcceppc.exe:
  - at the IRO per-file init (FE_INIT_TEST) sets the log enable byte and jumps to the call of
    the log setup (normally skipped: it is only called when an options field is 0);
  - at the log setup's fopen (FE_LOG_FOPEN) replaces the file name with MWTRACE_LOG;
  - at each IRO_Optimizer entry sets the "dump everything" options byte for the wanted function
    only (MWTRACE_FUNC / MWTRACE_NTH), so the log holds the full flowgraph + linear IR after
    every optimizer phase for that function and a one-line note per phase for the others;
  - lets the compiler finish (its exit closes the log); a JSON with the names of the optimized
    functions goes to MWTRACE_OUT.
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


def cstr(addr, limit=256):
    try:
        return mem(addr, limit).split(b"\0", 1)[0].decode("latin-1")
    except gdb.MemoryError:
        return None


def esp():
    return int(gdb.parse_and_eval("$rsp")) & 0xFFFFFFFF


def obj_name(obj):
    try:
        hn = u32(obj + A.OBJ_NAME) if obj else 0
        return cstr(hn + A.HASHNAME_TEXT) if hn else None
    except gdb.MemoryError:
        return None


def main():
    global inf
    want = os.environ["MWTRACE_FUNC"]
    nth = int(os.environ.get("MWTRACE_NTH", "1"))
    log = os.environ["MWTRACE_LOG"].encode() + b"\0"
    assert len(log) < 0x100
    gdb.execute("set pagination off")
    gdb.execute("set confirm off")
    gdb.execute("handle SIGSEGV nostop noprint pass")
    gdb.execute("starti", to_string=True)
    inf = gdb.selected_inferior()
    gdb.execute(f"hbreak *{A.ENTRY:#x}", to_string=True)
    gdb.execute("continue", to_string=True)
    gdb.execute("delete", to_string=True)
    gdb.execute(f"hbreak *{A.FE_INIT_TEST:#x}", to_string=True)
    gdb.execute(f"hbreak *{A.FE_LOG_FOPEN:#x}", to_string=True)
    gdb.execute(f"hbreak *{A.IRO_OPTIMIZER:#x}", to_string=True)
    funcs, hits, log_open, traced = [], 0, False, False
    while True:
        try:
            gdb.execute("continue", to_string=True)
        except gdb.error:
            break
        if not inf.pid or not inf.threads():
            break
        pc = int(gdb.parse_and_eval("$pc")) & 0xFFFFFFFF
        if pc == A.FE_INIT_TEST:
            # per-file IRO init, after its clear of the enable byte: enable, call the log setup
            inf.write_memory(A.IRO_LOG_ENABLE, b"\1")
            gdb.execute(f"set $pc = {A.FE_INIT_CALL_LOG:#x}")
        elif pc == A.FE_LOG_FOPEN:
            inf.write_memory(esp(), log)  # the name buffer is at [esp] here
            log_open = True
        elif pc == A.IRO_OPTIMIZER:
            name = obj_name(u32(esp() + 4))
            funcs.append(name)
            on = False
            if name == want:
                hits += 1
                on = hits == nth
                traced |= on
            inf.write_memory(A.COPTS_DUMP_ALL, b"\1" if on else b"\0")
    with open(os.environ["MWTRACE_OUT"], "w") as f:
        json.dump({"functions": funcs, "log_open": log_open, "traced": traced}, f)


main()
