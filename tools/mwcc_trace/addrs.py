"""Addresses and layouts in CodeWarrior GC/3.0a5.2 mwcceppc.exe (image base 0x400000).

Found by reading the binary (objdump -d -M intel), not taken from any other tool:
- the assert-file strings "Coloring.c" (0x6ad5cc), "InterferenceGraph.c" (0x6baac4) and "Spill.c"
  (0x6baad8) are pushed by CError asserts; their callers are the allocator's functions;
- colorinstructions(Object *proc) loops the five register classes (byte global coloring_class),
  and for each class whose used_virtual_registers differs from n_real_registers runs
    do { buildinterferencegraph(proc); list = simplifygraph(); ok = colorgraph(list);
         if (!ok) insertspillcode(); else rewritepcode(); } while (!ok)
  with an assert "pass <= 10" (Coloring.c line 501);
- simplifygraph pushes every node with degree < K (repeatedly, in vreg order), and when none is
  left picks the node with the lowest cost/degree from the rest (SIMPLIFY_COST_PICK); it returns
  the stack top, colorgraph walks it through IG_NEXT and gives each node the lowest free register
  of the class mask, else asks for a new non-volatile register, else marks it spilled.
"""

ENTRY = 0x401000

COLORINSTRUCTIONS = 0x596DF0       # void colorinstructions(Object *proc); arg at [esp+4] on entry
COLORGRAPH = 0x5970E0              # Boolean colorgraph(IGNode *stack_top)
COLORGRAPH_RET = 0x59728D          # its `ret`: eax = 1 coloured / 0 spilled, arg still at [esp+4]
COLORGRAPH_RET_ARG_OFS = 0
SIMPLIFYGRAPH = 0x597300           # IGNode *simplifygraph(void)
SIMPLIFY_COST_PICK = 0x597551      # edi = the node chosen by lowest cost/degree (potential spill)
BUILDINTERFERENCEGRAPH = 0x61F280  # InterferenceGraph.c
INSERTSPILLCODE = 0x620500         # Spill.c
REWRITEPCODE = 0x596F90
AVAILABLE_REGS = 0x594700          # int (char rclass): K for simplify

COLORING_CLASS = 0x726060          # s8; 3 = FPR, 4 = GPR (0 SPR, 1 CR field, 2 VR)
N_REAL_REGISTERS = 0x710274        # s32[5]: first virtual register number per class
USED_VIRTUAL_REGISTERS = 0x716EA0  # s32[5]: one past the last virtual register per class
IG_NODES = 0x711AFC                # IGNode * (array indexed by register number)
COALESCE_TABLE = 0x6EBF00          # u16 * : union-find parent per register (coalescing)

# IGNode, 0x1e bytes each
IG_SIZE = 0x1E
IG_NEXT = 0x00     # IGNode * (simplify stack link)
IG_OBJ = 0x04      # Object * (the variable, NULL for compiler temporaries)
IG_COST = 0x0C     # s32 spill cost
IG_VREG = 0x10     # u16
IG_REG = 0x12      # s16 physical register, -1 none
IG_FLAGS = 0x14    # u16
IG_DEGREE = 0x16   # s16 remaining degree (decremented by simplify)
IG_EDGES = 0x1A    # edge list head

FLAG_SPILLED = 0x01
FLAG_REMOVED = 0x02     # pushed on the simplify stack
FLAG_COALESCED = 0x04
FLAG_COALESCED_INTO = 0x08
FLAG_NOSPILL = 0x80     # simplify uses a constant (huge) metric instead of cost/degree
FLAG_PAIR = 0x200       # register pair: counts 2 against its neighbours
FLAG_CONSTMETRIC = 0x400

# interference edge: on both ends' lists
EDGE_NEXT_LO = 0x04     # next edge of the lower-numbered end
EDGE_NEXT_HI = 0x08     # next edge of the higher-numbered end
EDGE_LO = 0x0C          # u16
EDGE_HI = 0x0E          # u16

# Object / HashNameNode
OBJ_NAME = 0x0C         # HashNameNode *
HASHNAME_TEXT = 0x0A    # char[]

CLASS_NAMES = {0: "SPR", 1: "CRF", 2: "VR", 3: "FPR", 4: "GPR"}
CLASS_PREFIX = {0: "spr", 1: "cr", 2: "v", 3: "f", 4: "r"}

# PCode (the backend's instruction list), read before colouring for the listing
PCBASICBLOCKS = 0x7108C8   # PCodeBlock * first block
BLOCK_NEXT = 0x00
BLOCK_FIRST = 0x14         # PCode * first instruction
PC_NEXT = 0x00
PC_OP = 0x28               # u16 opcode, index into OPCODEINFO
PC_ARGC = 0x2A             # s16
PC_ARGS = 0x2C             # PCodeArg[argc], 0xe bytes each
ARG_SIZE = 0x0E
ARG_KIND = 0x00            # 0 register, 2 immediate, 3/4 memory or symbol, 8 extra
ARG_CLASS = 0x01           # register class (3 FPR, 4 GPR)
ARG_EFFECT = 0x02          # u16: 1 use, 2 def
ARG_REG = 0x04             # s16 register number (virtual >= n_real)
ARG_IMM = 0x02             # s32
ARG_OBJ = 0x06             # Object *
OPCODEINFO = 0x6B52EC      # backend opcodeinfo {char *name; char *format; ...} 0x16 bytes (0 B, 1 BL, 63 ADDI, 139 MR, 142 LFS)
OPCODEINFO_SIZE = 0x16

# Front end: the optimizer's built-in IR logger (IroDump), used by --frontend (gdb_frontend.py)
IRO_LOG_ENABLE = 0x725E5C   # u8; cleared by the per-file IRO init (0x5775b2), never set in release
IRO_LOG_FILE = 0x6EAC84     # FILE *
IRO_INIT = 0x5775A0         # per-file IRO init; returns early when 0x6e38f1 (initialized) is set
FE_INIT_TEST = 0x577683     # in IRO_INIT: `if (options->field_38 == 0) IRO_InitializeLog();`
FE_INIT_CALL_LOG = 0x5776E0 # the `call IRO_InitializeLog; jmp 0x57768e` it skips to
IRO_INITIALIZELOG = 0x5ED500
FE_LOG_FOPEN = 0x5ED52B     # in IRO_InitializeLog: name buffer (0x100 bytes) at [esp], fopen(name, "wt") next
IRO_DUMP = 0x5ED4C0         # IRO_Dump(fmt, ...) -> vfprintf(log)
IRO_DUMPAFTERPHASE = 0x5ED5E0   # (char *phase, Boolean force): full dump if force or COPTS_DUMP_ALL
IRO_DUMPBEFOREPHASE = 0x5ED680
COPTS_DUMP_ALL = 0x70F24D   # u8; reloaded from the options per function at 0x4e0fa2
IRO_OPTIMIZER = 0x5776F0    # IRO_Optimizer(Object *func, Statement *stmts); func at [esp+4]

# Back end: the instruction scheduler (Scheduler.c, assert string 0x6ad5e0), used by --sched
# (gdb_sched.py). codegen (0x468750, Object *func at [esp+4]) calls scheduleinstructions(0) before
# colorinstructions when the scheduling option byte 0x70effd == 2, and scheduleinstructions(1) after
# it when 0x70effd != 0 (`#pragma scheduling off` -> 0). scheduleinstructions picks the MachineInfo
# by the processor byte 0x70effc and calls scheduleblock for each block with more than 2 PCodes
# (prologue/epilogue blocks only in the second run). scheduleblock walks the block from its LAST
# instruction backwards, building one DGNode per PCode (so successor = later instruction), then
# list-schedules top down: per cycle up to MI->issue_width picks by selectinstruction, appending
# each pick to the emptied block (appendpcode 0x582aa0).
# selectinstruction(head, cycle) takes the ready nodes (npred == 0, earliest <= cycle,
# MI->can_issue) in program order and keeps the first unless a later one wins:
#   1. latest <= cycle (on the critical path now) beats latest > cycle;
#   2. more successors that it alone still holds back (successor npred == 1);
#   3. greater height;
#   4. if [0x710394]: smaller opcodeinfo byte +9 (op class);
#   otherwise the earlier instruction in the block wins.
CODEGEN = 0x468750
SCHEDULEINSTRUCTIONS = 0x5995C0    # (Boolean also_prologue_epilogue)
SCHEDULEBLOCK = 0x599740           # (PCodeBlock *)
ADDSUCCESSOR = 0x599E90            # (DGNode *earlier, DGNode *later, Boolean use_latency)
SCHED_GRAPH_DONE = 0x599817        # in scheduleblock: graph built, ebp = head (first instruction)
SCHED_PICKED = 0x599863            # after `call selectinstruction`: eax = node, cycle u16 at [esp+8]
SCHED_MAXHEIGHT = 0x6E70DC         # u16: the block's critical path (max height)
SCHED_TIEBREAK_ON = 0x710394       # s32: rule 4 on when nonzero
MACHINEINFO = 0x6E70D8             # MachineInfo *: +0 issue width, +8 extra latency into a branch,
                                   # +0xc latency(pcode), +0x14 can_issue, +0x18 issue, +0x1c advance
PC_FLAGS = 0x0C                    # u32 PCode flags: 1 branch, 0x20002 load, 0x40004 store,
                                   # 0x9|0x100 or MI->serializes: depends on everything after it
DG_SIZE = 0x1A
DG_NEXT = 0x00                     # next in program order
DG_SUCC = 0x08                     # DGSuccessor * list
DG_INSTR = 0x0C                    # PCode *
DG_LATENCY = 0x10                  # u16 MI->latency(pcode)
DG_EARLIEST = 0x12                 # u16 earliest cycle (raised as predecessors are picked)
DG_LATEST = 0x14                   # u16 maxheight - height: latest start on the critical path
DG_HEIGHT = 0x16                   # u16 longest latency path from here to the block end
DG_NPRED = 0x18                    # u16 unscheduled predecessors
DGS_NEXT = 0x00
DGS_NODE = 0x04
DGS_LATENCY = 0x08                 # u16
# addsuccessor call sites (return address) -> dependence kind
SCHED_EDGE_KINDS = {
    0x599B80: "after-0x80",        # 0x80-flagged later instructions
    0x599BE1: "serial",            # serializing instruction -> every later one
    0x599C20: "serial",            # -> a later serializing instruction
    0x599C47: "branch",            # leaf -> the block's branch
    0x599CB2: "mem-war",           # load -> later store that may alias (0x5a0150)
    0x599D70: "mem-raw",           # store -> later load that may alias
    0x599D80: "mem-waw",           # store -> later store that may alias
    0x599DD1: "raw",               # register def -> later use
    0x599E07: "waw",               # register def -> later def
    0x599E5E: "war",               # register use -> later def
}
SCHED_MAY_ALIAS = 0x5A0150         # Boolean (PCode *a, PCode *b)
PC_ALIAS = 0x1C                    # Alias * of a load/store
# Alias (backend, compared by may_alias 0x5a0170): kind 0 = object+offset/size, 1 = whole object,
# 2 = a set (bit vector +0x24 of alias indices, own index +0x28); worst_case = every alias
ALIAS_OBJ = 0x10
ALIAS_OFFSET = 0x14
ALIAS_SIZE = 0x18
ALIAS_INDEX = 0x28
ALIAS_KIND = 0x2C
WORST_CASE_ALIAS = 0x715C54        # Alias *worst_case (0x6e9100 is the number of aliases)
ALIAS_BITS = 0x24                  # u32 * bit vector (kind 2: members; worst_case: every alias)
OBJ_TYPE = 0x10                    # Object: Type *
ALIAS_ANY_TYPE = 0x6AD62E          # may_alias: an object of this type may alias any other in
                                   # the worst-case set (otherwise different objects never alias)
