Backend value-numbering probes for CodeWarrior GC/3.0a5.2 (run from anywhere; they cd to ~/wii-play)
  vn.sh SRC FUNC [OUT]        every load backend VN sees in FUNC (LOAD = 0x5987a0 symbol/plain path,
                              LOADX = 0x598560 reg+imm path), with op, flags(+0xc), word +0x10, block
                              (addr/+0x28/+0x2a flags), Alias (kind/obj/off/size/memvn), operand VNs,
                              then "MERGED pc@.." when the load was replaced. VN_ALL=1 also logs every
                              PCode dispatched (0x597a4c). UNIT=<splits unit> picks the compile flags.
                              FUNC is the codegen identifier (a ctor is __ct). VN_NTH=n for the nth.
  gdb_nolookup.py             what-if: NOLOOKUP=0xPCODE[,..] disables the lookup for those PCodes
                              (addresses from a vn.sh log of the same source; stable run to run)
  gdb_watch.py                WATCH_ADDR=0xPCODE: eip of every write to that PCode's alias/flags
  variant.py NAME [--ncs] 'old=>new'..  edit a copy of k0.cpp (the Kokeshi unit), score + VN loads
  score.sh X.cpp              objcmp word score of the Kokeshi ctor vs the original
