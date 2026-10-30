"""gdb probe of CodeWarrior GC/3.0a5.2 backend value numbering (loads).

Run via tools/mwcc_trace/vn_probe/vn.sh src.cpp func [out]. For every load the backend value numbering sees in the
traced function (0x5987a0 = value-number a load, args: PCode *, Boolean lookup_allowed) it logs the
PCode: op, flags (+0xc), word +0x10, Alias * (+0x1c) with kind/object/offset/size and the alias's
memory value number (+0x1c), operand registers with their current value numbers; then whether it was
merged (0x5988b2 / 0x598a54: the PCode is deleted) or recorded.
"""
import os, struct, sys
import gdb
sys.path.insert(0, os.path.join(os.getcwd(), 'tools', 'mwcc_trace'))
import addrs as A

inf = None
def mem(a, n): return bytes(inf.read_memory(a, n))
def u32(a): return struct.unpack("<I", mem(a, 4))[0]
def s32(a): return struct.unpack("<i", mem(a, 4))[0]
def u16(a): return struct.unpack("<H", mem(a, 2))[0]
def s16(a): return struct.unpack("<h", mem(a, 2))[0]
def cstr(a, n=64):
    try: return mem(a, n).split(b"\0", 1)[0].decode("latin-1")
    except gdb.MemoryError: return "?"
def esp(): return int(gdb.parse_and_eval("$rsp")) & 0xFFFFFFFF
def obj_name(o):
    if not o: return None
    try:
        hn = u32(o + A.OBJ_NAME)
        return cstr(hn + A.HASHNAME_TEXT) if hn else f"obj@{o:x}"
    except gdb.MemoryError: return f"obj@{o:x}"
REGVALUE = 0x6E6904  # regvalue[class] -> {vn, ?, ?} 12 bytes per register
def vn(cls, reg):
    try: return s32(u32(REGVALUE + 4 * cls) + 12 * reg)
    except gdb.MemoryError: return None
def alias_s(al):
    if not al: return "alias=NULL"
    kind = u8 = mem(al + A.ALIAS_KIND, 1)[0]
    return (f"alias@{al:x} k{kind} obj={obj_name(u32(al + A.ALIAS_OBJ))} off={s32(al + A.ALIAS_OFFSET)}"
            f" size={s32(al + A.ALIAS_SIZE)} idx={s32(al + A.ALIAS_INDEX)} memvn={s32(al + 0x1c)}")
def pcode_s(p):
    op = u16(p + A.PC_OP); argc = s16(p + A.PC_ARGC)
    name = cstr(u32(A.OPCODEINFO + A.OPCODEINFO_SIZE * op), 16).lower()
    args = []
    for i in range(max(argc, 0)):
        a = mem(p + A.PC_ARGS + A.ARG_SIZE * i, A.ARG_SIZE)
        k, c = a[0], a[1]
        if k == 0:
            r = struct.unpack_from("<h", a, 4)[0]; e = struct.unpack_from("<H", a, 2)[0]
            args.append(f"{'scvfr'[c] if c < 5 else '?'}{r}{'=' if e & 2 else ''}<vn{vn(c, r)}>")
        elif k == 2: args.append(str(struct.unpack_from("<i", a, 2)[0]))
        elif k in (3, 4):
            args.append(f"[{k}:{obj_name(struct.unpack_from('<I', a, 6)[0])}+{struct.unpack_from('<i', a, 2)[0]}]")
        else: args.append(f"k{k}")
    return (f"pc@{p:x} {name} {', '.join(args)} | fl={u32(p+0xc):#x} w10={u32(p+0x10):#x} "
            f"blk={u32(p+8):x}/{s16(u32(p+8)+0x28)}/{u16(u32(p+8)+0x2a):#x} {alias_s(u32(p + A.PC_ALIAS))}")

def main():
    global inf
    want = os.environ["VN_FUNC"]; out = open(os.environ["VN_OUT"], "w")
    gdb.execute("set pagination off"); gdb.execute("set confirm off")
    gdb.execute("handle SIGSEGV nostop noprint pass")
    gdb.execute("starti", to_string=True); inf = gdb.selected_inferior()
    gdb.execute(f"hbreak *{A.ENTRY:#x}", to_string=True); gdb.execute("continue", to_string=True)
    gdb.execute("delete", to_string=True)
    # software breakpoints work once the image is mapped (after ENTRY)
    bps = [A.COLORINSTRUCTIONS, 0x5987A0, 0x598560, 0x5988B2, 0x598A54, 0x59864D]
    if os.environ.get("VN_ALL"): bps.append(0x597A4C)
    for b in bps:
        gdb.execute(f"break *{b:#x}", to_string=True)
    # value numbering runs before colouring: buffer each function's loads, keep them when the
    # following colorinstructions names the wanted function (nth time, VN_NTH, default 1)
    buf = []; hits = 0; nth = int(os.environ.get("VN_NTH", "1"))
    while True:
        try: gdb.execute("continue", to_string=True)
        except gdb.error: break
        if not inf.pid or not inf.threads(): break
        pc = int(gdb.parse_and_eval("$pc")) & 0xFFFFFFFF
        if pc == A.COLORINSTRUCTIONS:
            nm = obj_name(u32(esp() + 4))
            if os.environ.get("VN_LIST"): out.write(f"#fn {nm}\n")
            if nm == want:
                hits += 1
                if hits == nth:
                    out.write(f"#function {nm}\n" + "".join(buf)); break
            buf = []
        elif pc in (0x5987A0, 0x598560):
            p = u32(esp() + 4); buf.append(f"{'LOAD' if pc == 0x5987A0 else 'LOADX'} lookup={mem(esp()+8,1)[0]} {pcode_s(p)}\n")
        elif pc == 0x597A4C:
            buf.append(f"PC {pcode_s(u32(esp() + 0x1c))}\n")
        else:
            buf.append(f"  MERGED pc@{u32(esp() + 0x28):x}\n")
    out.close()
main()
