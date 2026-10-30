# usage: WATCH_ADDR=0x... (a PCode address from a vn.sh log, same source) -> prints every eip that
# writes PCode+0x1c (alias) and +0xc (flags) of that PCode, with the value written
import os, struct, gdb
gdb.execute("set pagination off"); gdb.execute("set confirm off")
gdb.execute("handle SIGSEGV nostop noprint pass")
gdb.execute("starti", to_string=True)
gdb.execute("hbreak *0x401000", to_string=True); gdb.execute("continue", to_string=True); gdb.execute("delete", to_string=True)
a = int(os.environ["WATCH_ADDR"], 16)
gdb.execute(f"watch *(unsigned int*){a + 0x1c:#x}", to_string=True)
gdb.execute(f"watch *(unsigned int*){a + 0xc:#x}", to_string=True)
inf = gdb.selected_inferior(); n = 0
while n < 60:
    try: gdb.execute("continue", to_string=True)
    except gdb.error: break
    if not inf.pid or not inf.threads(): break
    pc = int(gdb.parse_and_eval("$pc")) & 0xFFFFFFFF
    al = struct.unpack("<I", bytes(inf.read_memory(a + 0x1c, 4)))[0]
    fl = struct.unpack("<I", bytes(inf.read_memory(a + 0xc, 4)))[0]
    esp = int(gdb.parse_and_eval("$rsp")) & 0xFFFFFFFF
    stk = struct.unpack("<8I", bytes(inf.read_memory(esp, 32)))
    print(f"eip={pc:#x} alias={al:#x} flags={fl:#x} stack={' '.join(hex(x) for x in stk)}"); n += 1
