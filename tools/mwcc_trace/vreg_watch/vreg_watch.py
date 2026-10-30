"""gdb script: log every FPR/GPR virtual-register allocation while one function is generated.

Run via tools/mwcc_trace/vreg_watch/run.sh UNIT FUNC [SRC] [CLASS]. Env: RV_FUNC, RV_OUT, RV_CLASS (3 FPR, 4 GPR),
RV_NTH. Writes one line per new vreg: number, eip of the increment, stack return-address chain,
and (for a variable) the Object name if a pointer to an Object is found in eax/ecx/edx/esi/edi/ebx.
"""
import os, struct, sys, json
import gdb
sys.path.insert(0, os.path.join(os.getcwd(), 'tools', 'mwcc_trace'))
import addrs as A

inf = None
def mem(a, n): return bytes(inf.read_memory(a, n))
def u32(a): return struct.unpack("<I", mem(a, 4))[0]
def reg(r): return int(gdb.parse_and_eval("$" + r)) & 0xFFFFFFFF
def cstr(a, lim=128):
    try: return mem(a, lim).split(b"\0", 1)[0].decode("latin-1")
    except Exception: return None
def objname(o):
    if not o or o < 0x10000: return None
    try:
        hn = u32(o + A.OBJ_NAME)
        if not hn: return None
        s = cstr(hn + A.HASHNAME_TEXT, 64)
        if s and all(32 <= ord(c) < 127 for c in s) and s: return s
    except Exception: return None
    return None

def stack_rets(n=12, depth=0x400):
    sp = reg("rsp"); out = []
    try: data = mem(sp, depth)
    except Exception: return out
    for i in range(0, depth, 4):
        v = struct.unpack_from("<I", data, i)[0]
        if 0x401000 <= v < 0x6a0000:
            # plausible return address: preceded by a call (e8 rel32 or ff /2)
            try:
                b = mem(v - 5, 5)
                if b[0] == 0xE8 or mem(v - 2, 2)[0] == 0xFF or mem(v - 3, 3)[0] == 0xFF or mem(v-6,6)[0]==0xFF:
                    out.append(hex(v))
            except Exception: pass
        if len(out) >= n: break
    return out

def main():
    global inf
    want = os.environ["RV_FUNC"]; nth = int(os.environ.get("RV_NTH", "1"))
    cls = int(os.environ.get("RV_CLASS", "3"))
    addr = A.USED_VIRTUAL_REGISTERS + 4 * cls
    gdb.execute("set pagination off"); gdb.execute("set confirm off")
    gdb.execute("handle SIGSEGV nostop noprint pass")
    gdb.execute("starti", to_string=True)
    inf = gdb.selected_inferior()
    gdb.execute(f"hbreak *{A.ENTRY:#x}", to_string=True)
    gdb.execute("continue", to_string=True); gdb.execute("delete", to_string=True)
    gdb.execute(f"hbreak *{A.CODEGEN:#x}", to_string=True)
    gdb.execute(f"hbreak *{A.COLORINSTRUCTIONS:#x}", to_string=True)
    out = open(os.environ["RV_OUT"], "w")
    hit = 0; active = False; wp = None
    while True:
        try: gdb.execute("continue", to_string=True)
        except gdb.error: break
        if not inf.pid or not inf.threads(): break
        pc = reg("rip")
        if pc == A.CODEGEN:
            name = objname(u32(u32(reg("rsp") + 4) + 4)); out.write(f"# seen {name}\n") if os.environ.get("RV_DEBUG") else None
            if active: break
            if name == want:
                hit += 1
                if hit == nth:
                    active = True
                    out.write(f"# codegen {name}: start count {u32(addr)}\n")
                    gdb.execute(f"watch *(int*){addr:#x}", to_string=True)
        elif pc == A.COLORINSTRUCTIONS and active:
            out.write(f"# colorinstructions reached: count {u32(addr)}\n"); break
        elif active:
            v = u32(addr)
            names = {r: objname(reg(r)) for r in ("rax","rbx","rcx","rdx","rsi","rdi","rbp")}
            names = {k: n for k, n in names.items() if n}
            out.write(json.dumps({"new": v - 1, "pc": hex(pc), "rets": stack_rets(), "objs": names}) + "\n")
    out.close()
    gdb.execute("kill", to_string=True)
main()
