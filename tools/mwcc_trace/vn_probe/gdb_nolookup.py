# what-if: value numbering of the PCodes listed in NOLOOKUP (hex addresses from a vn.sh log of the
# same source) runs with lookup disabled (the load is only recorded, never merged). Output object as
# given by -o.
import os, gdb
gdb.execute("set pagination off"); gdb.execute("set confirm off")
gdb.execute("handle SIGSEGV nostop noprint pass")
gdb.execute("starti", to_string=True)
gdb.execute("hbreak *0x401000", to_string=True); gdb.execute("continue", to_string=True); gdb.execute("delete", to_string=True)
want = {int(x, 16) for x in os.environ["NOLOOKUP"].split(",")}
gdb.execute("break *0x598560", to_string=True); gdb.execute("break *0x5987a0", to_string=True)
inf = gdb.selected_inferior(); hits = 0
while True:
    try: gdb.execute("continue", to_string=True)
    except gdb.error: break
    if not inf.pid or not inf.threads(): break
    esp = int(gdb.parse_and_eval("$rsp")) & 0xFFFFFFFF
    p = int.from_bytes(bytes(inf.read_memory(esp + 4, 4)), "little")
    if p in want:
        inf.write_memory(esp + 8, b"\0"); hits += 1
print("NOLOOKUP hits", hits)
