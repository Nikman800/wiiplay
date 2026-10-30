# gdb -x script: logs how memory aliases enter CodeWarrior GC/3.0a5.2's worst-case set.
# Env: PROBE_OUT (log path), PROBE_FUNC (optional substring filter on the function name).
import os, struct, sys, gdb
sys.path.insert(0, os.path.join(os.getcwd(), 'tools', 'mwcc_trace'))
import addrs as A
WC = 0x715C54
inf = None
def mem(a, n): return bytes(inf.read_memory(a, n))
def u8(a): return mem(a, 1)[0]
def u32(a): return struct.unpack("<I", mem(a, 4))[0]
def reg(n): return int(gdb.parse_and_eval(f"${n}")) & 0xFFFFFFFF
def cstr(a):
    try: return mem(a, 64).split(b"\0", 1)[0].decode("latin-1")
    except gdb.MemoryError: return "?"
def name(o):
    try:
        hn = u32(o + A.OBJ_NAME); return cstr(hn + A.HASHNAME_TEXT) if hn else "<anon>"
    except gdb.MemoryError: return "?"
def objdesc(o):
    try:
        t = u32(o + 0x10)
        if t == 0x6AD62E:
            return f"DUMMY@{o:#x}(const={u32(o+0x14)&1})"
        return f"{name(o)}@{o:#x}(dt={u8(o+2)} addr={u8(o+0x5a)} q={u32(o+0x14):#x} tk={u8(t)})"
    except gdb.MemoryError: return f"?{o:#x}"
def adesc(al):
    try:
        k = u8(al + 0x2c)
        if k == 2: return f"set{u32(al+0x28)}" + ("=WORST" if al == u32(WC) else "")
        return f"a{u32(al+0x28)}:{objdesc(u32(al+0x10))}+{struct.unpack('<i',mem(al+0x14,4))[0]}:{u32(al+0x18)}k{k}"
    except gdb.MemoryError: return f"?{al:#x}"
out = open(os.environ["PROBE_OUT"], "w")
filt = os.environ.get("PROBE_FUNC", "")
for c in ["set pagination off", "set confirm off", "set displaced-stepping off",
          "handle SIGSEGV nostop noprint pass"]:
    gdb.execute(c)
gdb.execute("starti", to_string=True); inf = gdb.selected_inferior()
gdb.execute(f"hbreak *{A.ENTRY:#x}", to_string=True); gdb.execute("continue", to_string=True)
gdb.execute("delete", to_string=True)
BPS = {A.CODEGEN: "codegen", 0x5A0CB0: "mkdummy", 0x5A0D58: "mkdummy_ret", 0x5A04C0: "add",
       0x63D460: "sched_start", 0x5A0B80: "sched_scan", 0x5A0370: "rebuild"}
for a in BPS: gdb.execute(f"break *{a:#x}", to_string=True)
on = not filt
seen = set()
_w = out.write
class W:
    @staticmethod
    def write(t):
        if t.startswith("\nFUNC") or t not in seen:
            seen.add(t); _w(t)
out2 = out; out = W
while True:
    try: gdb.execute("continue", to_string=True)
    except gdb.error: break
    if not inf.pid or not inf.threads(): break
    pc = reg("pc"); sp = reg("rsp")
    try:
        if pc == A.CODEGEN:
            a0 = u32(sp + 4); n = name(u32(a0 + 4)) if a0 else "?"; seen.clear(); on = (not filt) or filt in n
            if on: out.write(f"\nFUNC {n}\n")
        if not on: continue
        if pc == 0x5A0CB0:
            o = u32(sp + 4)
            out.write(f"  mkdummy for ptr {objdesc(o)} qual-arg={u32(sp+8):#x}\n")
        elif pc == 0x5A0D58:
            out.write(f"    -> {adesc(reg('rdi'))} add_now={reg('rbx')&0xff}\n")
        elif pc == 0x5A04C0:
            s, m = u32(sp + 4), u32(sp + 8)
            if s == u32(WC):
                out.write(f"  ADD-WORST {adesc(m)} caller={u32(sp):#x}\n")
        elif pc == 0x63D460:
            out.write(f"  SCHED-START flag710394={u32(0x710394)} done725e6e={u8(0x725e6e)}\n")
        elif pc == 0x5A0B80:
            out.write(f"  scan {adesc(u32(sp+4))} caller={u32(sp):#x}\n")
        elif pc == 0x5A0370:
            out.write(f"  rebuild bitvectors (n aliases {u32(0x6e9100)})\n")
    except gdb.MemoryError as e:
        out.write(f"  memerr {e}\n")
out2.close()
