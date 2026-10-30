Alias probes for CodeWarrior GC/3.0a5.2 (Wii Play decomp). gdb + wibo, same setup as tools/mwcc_trace.

  python3 tools/mwcc_trace/alias_probe/run.py UNIT [SRC|-] [FUNC_SUBSTR] > log
    UNIT  splits.txt name (e.g. Pack/RPParty/RPPartyCommon/RPCPadCursor_2.cpp); its compile flags
          and include dir are used.
    SRC   optional test .cpp compiled with UNIT's flags instead of the unit ("-" = the unit itself).
    FUNC  only log functions whose name contains this substring (blank = all).
  Example: python3 tools/mwcc_trace/alias_probe/run.py Pack/RPParty/RPPartyCommon/RPCPadCursor_2.cpp - SetTarget

Log lines (deduplicated per function; FUNC order is CW's codegen order):
  mkdummy for ptr X       0x5a0cb0 creates the pointee dummy for pointer/array variable X
    -> aN:DUMMY(const=c) add_now=b   the dummy alias; add_now=1 = added to worst case at creation
  ADD-WORST <alias> caller=..   0x5a04c0(worst_case, alias): who put it in
      0x5a0d8c mkdummy (non-restrict, non-const pointee)
      0x5a08c0 alias creation of a non-const global/static (or worst_case itself)
      0x6442ec 0x644260: an instruction takes a local's address (addi rX,r1,local) - no const test
      0x5a0c5d/0x5a0c15  0x5a0b80 scan (escape) - const objects skipped, locals (dt=1) always
      0x5a05ae/0x5a05bc  merge 0x5a0570 into the worst set; 0x63d7d8 / 0x64409f pre-RA pointer
                         analysis (0x63d6a0 / 0x643d..) folding multi-def results into worst
  scan <alias> caller=0x63d9da  0x63d900 escape callback calls 0x5a0b80 on a memory op's alias
  dt= object +2 (0 global/static/dummy, 1 local/param), addr= +0x5a, q= +0x14 qualifiers, tk= type kind
Test inputs: t1..t6.cpp with logs t*.log; settarget.log = the real SetTarget.
var/ holds SetTarget source variants and their --sched --score traces.
