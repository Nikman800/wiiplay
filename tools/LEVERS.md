# Matching levers, by symptom

The levers that closed a CodeWarrior (GC/3.0a3-3.0a5.2, PowerPC) difference in this project, indexed
by what the difference looks like. Classify first (MATCHING.md, "Triage"), then read only that
section. Each lever names the unit where it worked; the dated entries in MATCHING.md's lesson log
have the details. Every lever here is natural C (MATCHING.md, "Natural C"); a lever that needs
anything else is listed under "Diagnostics" only.

## 0. Before any lever

- **Is the code all there?** Compare sizes first. ef_particlemanager's Calc was a third of the
  original (473 of 1382 words), eggShadowTexture's Draw and RPCPadCursor_3's SetTarget were also
  missing code. A size difference of more than a few instructions is structural, not registers.
- **Check the CC0 references** (`tools/refsearch.py`): Petari, pik1wii, ogws, Rhae. NWC24MsgCommit,
  lyt_window/lyt_material and all of BTE came from there.
- **Check the prototypes** of SDK calls against the real SDK: GXSetFog takes its color last, and a
  by-value struct's position changes when CW copies it.
- **`tools/mwcc_trace.py <unit> <function>`** shows the allocator's colouring order and each
  value's interference neighbours: find which value is coloured in the wrong order, then change
  when it is created (virtual registers are numbered in creation order; a sweep colours them in
  descending order).
- **`tools/locals.py <unit> <function>`** prints each local's register or stack slot. Read it
  before guessing at register swaps (inlined callees' locals are not listed).

## 1. Registers swapped (same instructions)

Virtual-register numbering, measured with tools/mwcc_trace.py: the outer function's named locals
and parameters get the lowest numbers (coloured LAST in a sweep); locals of inlined functions and
compiler temporaries (@N) are numbered in reverse @ order (inline locals in reverse declaration
order, inner scopes before outer); CSE and getter-load temporaries come after the inline's named
locals. A local assigned once from a load is propagated into the load temporary, so moving its
declaration does nothing. Among volatile registers r0 goes to the first node coloured that can take
it. So: to colour a value later, make it a CSE temporary; earlier, a named inline local declared
later. Reusing a local for a repeated field read (`datas[rangeIdx]` instead of a second
`pKey->rangeIdx`) stopped a CSE temporary taking r0 (ef_animcurve Texture, 76 -> 25).
A value is coloured in the second sweep (the top register) only if its degree is still >= K (about
29 GPRs) when simplify reaches it; check the tracer's `deg`/`lvl`. A value one neighbour short of K
is the target: removing a named local from an inline turns it into a high-numbered temporary that
stays a neighbour (RPDebugLog Recent(): count r25 -> r31). A nested inline constructor's `this`
inside an inlined member gets the lowest vregs (coloured last); written directly in the function it
gets the highest (RPPathWalker). If the difference is already in `--pcode` (an interleaving, one
vreg for two loads), colouring cannot fix it (RPKokeshiMgr_3).
A variable assigned in both branches of an if/else is split per branch by IRO_SplitLifetimes (see
`--frontend`): the split-off web is a new high temporary coloured first, the other keeps its low
vreg. If the original has one register in both branches, use `#pragma opt_lifetimes off` around the
function (RPPathWalker ctor).
A local `const T& x = expr;` gets a front-end binding temporary numbered BEFORE every inline
temporary; the backend then promotes it (address taken) to a new TOP vreg, so it is coloured first
among its peers: ef_animcurve U8's `const int& rangeSize = num * 2` took r22 above CalcTime's
temporaries as in the original (319 -> 289).
More from ef_animcurve (1048 -> 800): wrap `a + b * (f32)x` in an inline taking (f32 a, f32 b, u32 x)
by value so the operands are created before the conversion (flips the float colouring); declare a
loop's block-scope float values at function scope before an early-coloured local; read a byte field
directly in both places instead of a named local to make it a late CSE temporary; `(x & 1) == 0` and
`!(x & 1)` give different branch senses and colourings.
An argument bound to a `const T&` parameter gets a front-end temporary numbered very early when the
call is in the function itself (coloured before every inline-time temporary), and at inline time
when the call sits inside an inline helper: use that to place a value between a hoisted constant and
the loop pointers. A literal bound to `const&` becomes a static constant load (RPDebugLog).
A by-value index is propagated into the inline's body and created only where it is used; bound to a
`const int&` parameter it is created when the argument is bound, before the inline's other values:
that made RPDebugLog link (`GraphX(graph, const int& index, u16 half)`). To create a value
EARLIER, pass it by const reference to the inline that uses it.
A value assigned inside an expression at its first use (`f(id0 = table[i * 2])`) keeps the named
local's early vreg instead of becoming a late CSE temporary, so its declaration position sets its
colouring order (RPGrpModelMaterial Calc, which then linked). The const-ref lever does not work for
getter loads: copies coalesce into the load temporary, which keeps its late number (RPSysTagProcessor).
More from ef_animcurve (Child 291 -> 32, Texture 25 -> 3): a parameter copied into a local
(`pHeader = (T*)pCmdList`) is merged under the parameter's number while the parameter is still used:
route every use through the local to make it its own node; an inline's out-value assigned once to a
local loses the local (propagated into the inline's temporary): pass it out through a reference
(`u16& rFrame`); function-scope (C89-style) loop counters are split per loop into temporaries
numbered just above the outer named locals, block-scope ones below; index form vs pointer form per
loop decides which of i or the pointer gets which register. `tools/mwcc_trace/sim.py` replays
simplify/colour from a `mwcc_trace.py --json` dump, so a renumbering can be tested before editing.

Callee-saved registers: in colouring order each variable takes the lowest-numbered register already
in use and conflict-free, else opens a new one counting down from r31. A value "too high or too low"
is a priority problem; these levers change priority:

- **An accessor inline instead of a direct field read, or the reverse.** The most productive lever
  of Phase 5: RPGrpRenderer::Draw (`u8 s = pView->GetDrawScene();`, a new inline), RPDollBase
  (RayHitJoints), eggDrawPathDOF (GetScreenDistance reads the fields directly), eggLightManager
  CalcView (GetLightObjSet, a pointer-returning inline outranks compiler temporaries).
  Try one for every field the function reads before enumerating orders.
- **A named local inside a shared header inline, or its removal**: RPUtlRandom getRandF32 (one
  expression fixed three Fish functions), TRPSndObject LabelToSoundId (a named local fixed six).
- **A read through a `const T*` local** (eggScreenEffectBase getBuffer), a `T* const` pointer
  (RPDucShot, RPFshFish), a `const T&` bound to a converted copy (RPGrpRenderer::Rendering).
- **A double read the compiler merges**: `x >= m ? m : x` with x read through a pointer twice
  (eggLightManager SetBinaryInner); reading a singleton twice (RPSportsMessageScene Start).
- **Type of the local**: a u8 loaded from memory always takes the top register; widen it to int
  (RPSysTagProcessor). `bool` vs `BOOL` (Find Mii), `int` vs `s32` (Charge! HUD), an enum-typed
  local or enum accessor (RPTnkFade, RPHkyGame, Find Mii modes).
- **Exhaustive declaration-order enumeration** of 3-5 locals (eggShadowTexture: one order of all
  permutations matched; hill-climbing and permute.py missed it). Locals declared at the top of
  each case block (eggLightManager).
- **Assignment order, not declaration order, for float loads** (RPTnkCamera).
- **A separate loop variable** instead of looping on a count (RPWlyAvatarMgr CreateAvatars); a loop
  bound through an inline accessor (`s32 num = GetTargetNum();`).
- **Move a block into a file-local `static inline` with reference parameters**: fresh virtual
  registers for its locals (batch AV: four RP units); which block matters.
- **A struct copy hoisted above a pointer local** (RPWlyAvatarMgr_2); copy into the local first,
  then add (RPDucBalloon).
- **How a VEC3 is built**: member by member vs the three-argument constructor (RPSysLytBounding).
- **Ternary vs if/else** into one local (RPTnkMinePool); a `cond ? a : b` pointer select
  (Billiards).
- **Class hierarchy**: deriving directly from the real base removes an inlined destructor level
  (RPFshFish, Laser Hockey's RPSimpleModel objects).

- **Named locals from calls are coloured after the loop counter**: a named local assigned from a
  call/inline gets a low vreg (variables are numbered below expression temps) and is coloured late.
  Routing it through a small accessor inline (`GetPointPos(pRes, idx)` returning `->pos` by const
  reference) makes it an inline-result temporary numbered above the named locals, coloured first
  (RPPathWalker ctor 22 -> 0). Replay `mwcc_trace --json` with sim.py over candidate colouring
  orders first to see which values have to come first.

- **Colouring an inline-time load temp after an outer named local**: read the value into a named
  block-scope local in the caller and pass it to the inline by value; its vreg lands among the
  caller's named locals (block locals in reverse declaration order, below function-scope locals)
  (ef_animcurve U8: random0 back in r0, 204 -> 166). Find the target position with sim.py first.

- **Live-range splitting decides which vreg a variable's uses get**: CW splits a variable into
  separate webs; the first web keeps the declaration's (low) vreg, later webs get late temp numbers.
  A function-scope local assigned in one branch can therefore be numbered low there, while sharing it
  with another branch pushes that branch's load to a late temp. Pick which branches share a local
  from a sim.py replay (ef_animcurve Rotate 241 -> 71: one rangeIdx for the single-random branches,
  a rangeIdx0/rangeIdx1 pair for the both-random branch).

- **Strength-reduction walkers are high temporaries**: the optimizer's loop walker is numbered above
  named locals, so it is coloured first. If the original colours the walker last, the loop's other
  values must be temporaries too: move the search/value into an inline whose locals become inline
  temporaries (RPAnimButton_3: one shared search helper, `index` declared before `i`; RPGrpModelRecord:
  `MTX34Copy` instead of `PSMTXCopy`, so the element address is an argument temporary). If the replay
  needs the walker numbered below named locals and it is this-relative, no natural form reaches it
  (RPDucField_2). Search script: /tmp/ploop/bin/perm.py.

- **Search two moves at once before concluding "interference"**: a swap may need two values
  renumbered together (one above a third value, one below it); single-move sim.py scans miss it
  (ef_animcurve Rotate pKeys/idx: `{pKeys: above loop1, idx: below loop1}`, then a declaration
  swap; 71 -> 48).

- **Which side of the walker a loop's values fall on**: optimizer walkers are numbered below inline
  temporaries and above the enclosing function's named locals. Counter coloured after the walkers:
  write the loop in the caller with named locals (RPWlyClearPhase, RPGrpModelMaterial SyncChanCtrl,
  RPPlayerNumSelect lamps). Counter coloured before them: move the loop into a member inline
  (RPPlayerNumSelect Calculate); a named per-iteration value orders the walkers (RPTnkLyt_10). Stack
  slots: temporaries created directly in a function take the top slots; keep their creating call in
  an inline to keep the original's slot order. A `u32` counter stops an index product merging with a
  walker's start value.

- **Where vregs come from** (gdb, tools/mwcc_trace/vreg_watch/run.sh): (1) at codegen start every
  register-eligible local gets the next number, named locals first, then @temps in list order
  (reverse @); address-taken objects get none. (2) Codegen expression temps follow in PCode order.
  (3) After codegen the O4 backend optimizer (0x62dcc0) promotes address-taken scalars that are only
  loaded/stored directly into fresh TOP vregs, numbered in reverse order of their first load/store
  (the binding stored first gets the highest number). A `const T&` binding is promoted only if its
  address survives IRO_ExpressionPropagation, which substitutes `&tmp` only within one flowgraph node:
  the use must be past a branch and join, else the binding is a plain/getter/fmr temp. Local structs
  and arrays are never promoted. So a promoted value can't be numbered below a non-promoted one, and
  two promoted values reorder only by their first stores (RPSysTagProcessor ProcessRuby: descent can't
  get above y). sim.py can model it by renumbering promoted nodes (`@N'` in --json) to the top.

- **A predicate computed then discarded** (`li rX,0 ... beqlr ... li rX,1; blr`): write
  `if (!pred()) return;` with nothing after it (a stripped debug/draw body). A bare expression
  statement, an unused local or an empty `if` lets CW delete the predicate (RPFshPlayer::UserDraw,
  which replaced an inline-asm body).

- **`T* const p = Inline(...)` survives copy propagation**: the const initialisation wraps the value
  in a cast (ETYPCON) that blocks IRO_CopyAndConstantPropagation, so the local keeps a low named vreg;
  without const it is propagated into the inline's temporary and gets no node. Fails when the inline's
  argument is not a plain variable (a member read adds a parameter-temp copy that ExpressionPropagation
  substitutes); writing the inline's body in the caller keeps the named local then (RPCowKakasiGroup).
  Tooling: run `dtk elf disasm` with absolute paths (a relative path after `cd` silently keeps old output).

## 2. Instructions in another order (scheduling)

- **Check `tools/mwcc_trace.py --pcode` first**: when the registers already match and only the order
  differs, the order usually comes from the pre-allocation PCode, not the scheduler (eggQuat
  conjugate, RPCPadCursor's size loads).
- An inline's by-value parameter temporary is created at the first inline, gets a low virtual
  register and is coloured late (a high scratch register); a lower register in the original means
  the value was created later (RPCPadCursor `GetHorizontalPosition(u8)`).
- Dot-product operand order shows in fmuls/fmadds operands: pick `normal.dot(v)` vs `v.dot(normal)`
  from the asm (eggPlane).

- **Split statements finely**: separate sums from divides, named intermediates stored right after
  they are computed (eggFrustum GetViewToScreen; division results as named locals in
  RPMiiSelect_2). A named local for a product keeps `a*b + c` from fusing into fmadds (RPCowCamera).
- **`const` changes alias analysis**: drop a guessed `const` on `this` or a pointer parameter
  (seven Tanks TestHit functions, RPTnkBlockPool, RPHkyPaddle). EGG's Quatf::conjugate/rotateVector and
  Plane3f::project matched only once made non-const (as in the CC0 bba-wd decomp), after every
  source form, the tracer and the pragma sweep had failed: try this FIRST on any member function
  whose const-ness was our guess. Assert loads after the register
  saves mean a non-const member function (RPSndUtility).
- **Statement order of member updates**: CW knows fields of `this` don't alias each other
  (RPTnkAiProbe Step).
- **Structs by value**: a `VEC2&` parameter made stores alias `this` (RPTnkHitPopup); a by-value
  matrix reproduces a word-copy schedule (RPKokeshiMgr_3 lead).
- **Where a region boundary falls earlier in the function** decides one load's region (RPTnkTank
  OnSpawn): chained assignments or a shared constant local.
- **Argument setup order**: a string local declared right before the call (RPWlyEarth); a bool
  expression straight into the call (RPHkyGame); an extern's parameter order (RP_approachStep); an
  inline helper's `const char*` comes before the object pointer.
- **Copy-construct vs assign**: `VEC3 t = ...` copies words, `t = ...` copies floats; a struct with
  a user-written `operator=` copies through lfs/stfs.
- **Hidden block splits in long straight-line code**: when one stretch of a struct copy is ordered
  differently, check `--sched` for two scheduling blocks with no branch between them. Codegen starts a
  new block after about 96 instructions at a member-copy boundary, so the split point moves with the
  number of instructions emitted before the copy (RPSysTagProcessor ProcessRuby: one extra
  instruction ahead of the copy reproduces the original's block exactly). Fix found 2026-10-29: a
  two-step pointer initialisation (`pBase = pRuby; pBase += rubyLen;`) adds one PCode copy that
  coalescing removes, moving the split one member earlier with no extra instruction in the object.
- **Header asserts and getters** in the inlines a function uses (SetCanvasMode's assert,
  SetFovy's): derive every inline from the asm, operand order included (eggFrustum had inlines
  computing the wrong expression).

- **Load merges, CommonSubs vs backend**: IRO_CommonSubs (0x615780) treats any store through a
  pointer, field or array element, any call, and any store to an optimizer temporary without a
  VarRecord (e.g. the unroller's overflow flag) as killing every memory load; a first-in-node
  occurrence that also has a local user keeps its own reload until the next pass. A const read and a
  plain read are different expressions. Afterwards backend value numbering (0x5976e0, first pass of
  optimizelevel4) merges identical loads within extended basic blocks, killed by any store or call.
  If `opt_common_subs off` still shows the merge, the lever is in the backend (RPKokeshiMgr_3 ctor,
  RPCPadCursor SetTarget). gdb helpers: /tmp/pcse/tool/cse.sh, pc.sh.

- **Grouping of an inline helper's sum when CSE spans two inlined calls**: the source grouping decides
  where the constant and the shared term attach; single-use callers can stay identical while the
  shared case changes (ef_animcurve CalcRandomSeed `A + B + (L + R + K)` -> `A + B + (L + R) + K`).
  Check the helper's other callers still match.

- **What aliases what** (gdb, tools/mwcc_trace/alias_probe): a load/store through a pointer variable
  carries that variable's pointee dummy; two different objects conflict only if both are in the
  worst-case set and one is a dummy (may_alias 0x5a0170). A pointer's dummy is in the set unless the
  variable is pointer-to-const; `__restrict` keeps it out only while the pointer never escapes (passed
  to a call, even as `p->name`, or stored: scheduler-start scan 0x63d460 -> 0x5a0b80). Address-taken
  locals and non-const globals are always in; locals whose address is never taken never are.
  Const-qualified inline parameters, local copies, reference bindings and accessor returns do not help:
  copy propagation restores the original non-const variable. So loads through a non-const pointer
  parameter always conflict with stores through `this` (RPCPadCursor SetTarget: proven blocked).

- **Backend load merging** (value numbering 0x5976e0 / valuenumberblock 0x597980; gdb probe
  tools/mwcc_trace/vn_probe): extended basic blocks (a single-predecessor successor inherits the table;
  a multi-predecessor block starts fresh). Two identical register+immediate loads merge unless one is
  volatile (flag 0x80) or has 0x100/0x200000, a may-alias store or call lies between, or every register
  holding the first value was redefined. Copies don't stop a merge. A load's alias comes from a backend
  points-to pass on its base register (0x64407a/0x643e49), not from the source spelling, so pointer vs
  array vs reference forms can't separate two loads. RPKokeshiMgr_3 ctor: the unroller's `@a` is the only
  copy of the bound inside a cast node (ETYPCON) — the one structural difference left to exploit.

- **Colouring differences can come from the pre-RA schedule**: map the original's registers onto the
  run-0 block and replay with tools/mwcc_trace/sched_replay/edges.py. If adding interference edges for
  an other-order placement reproduces the original exactly and no renumbering does, look for a
  dependence-graph or timing change in that block: unit occupancy (a divwu holds the multiply unit for
  19 cycles), copies, block contents. Statement order and variable types don't reach it (ef_animcurve
  U8 setup: 4 edges = the original's map).

- **Prologue saves are worst-set stores**: in `--sched` run 1 the callee-save `stw rN`/`stwu` are
  `set1=worst`. Member loads the original schedules after them had mem-raw edges, so their object was
  in the worst-case set (a non-const pointer or `this`); in a const function or through a
  pointer-to-const, natural C cannot reproduce that order (RPPolyline Calc). Interleaved word copies
  likewise need the source pointer's dummy in the set: copying into by-value inline parameters from a
  non-const pointer reproduces it (RPTnkAi_4), a const source cannot (RPTnkPlayerController).

## 3. An instruction too many or too few

- Whether CW reassociates a constant out of an inline argument depends on how the expression is
  written: `P - (t - Y)` gave the original `subi; subf` where `X - (t + 1)` did not (ef_animcurve).
- Reusing an existing shared inline with a u16 parameter recreates a narrowing (`clrlslwi`).
- A local copy instead of writing straight through a reference parameter (`u32 loop = ...; rLoop =
  loop;`) changes the pre-allocation order (ef_animcurve Texture's (f32)tick f0 -> f1).

- **Types**: `clrlwi` before a u16 call means the value was wider; `extsh` means a signed short;
  `cmplwi` loop tests mean an unsigned counter. Signed vs unsigned int->float load different
  .sdata2 constants (RPDucClay).
- **Dead code that survives**: computed-but-unused values leave their int->float conversions
  (RPDucUfo), dead stores survive only through a written-through object (RPPnpScene), a dead
  if/else-if chain keeps all compares but the last arm's. Keep these only with the binary's
  evidence.
- **`#pragma opt_unroll_loops off`** when the original keeps a short `mtctr` loop; a 2-4 iteration
  constant `for` is unrolled anyway, `do { } while (++i < N)` keeps it.
- **A reload that ours omits**: store to the member and read it back (RPSysFontManager); a pointer
  constructor (`VEC3 trans(&r.mTrans.x)`) stops CW reusing an earlier load (RPCPadCursor_3).
- **`__fabsf`** instead of `std::fabs` (no frsp).

## 4. Stack slots or frame size

- Named locals take slots top-down in declaration order; temporaries go below all named locals;
  locals of inlined functions in call order below those (Find Mii batch).
- Locals over 16 bytes sit above the small ones, 8-aligned: split a big struct into <=16-byte
  objects (RPTnkShell).
- Temporaries of several inline calls stack in reverse call order: merge or split helpers rather
  than reordering declarations (RPTnkAi_2, RPBilScene).
- By-value argument temporaries get their own slots in creation order (RPTnkAi_4).
- An inline function's named local goes below inlined callees' temporaries: wrapping one
  `GetTarget(x).mValid` in an inline that copies into a named local fixed a rotated order of three
  0x88-byte slots (RPCPadCursor_2/_3).
- A local whose only job is a stack-sized copy (a Size, a VEC2) adds 8-byte temporaries and grows
  the frame; pass the values as f32 arguments instead (RPCPadCursor SetTargetInfo).
- int->float scratch slots alternate between two slots on every conversion the compiler emits,
  including ones later removed as dead code (constant conversions are folded first and do not
  count): one extra (dead) conversion swaps them (RPFshFish SwimCalc, ef_animcurve Child).
- `#pragma opt_lifetimes off` makes scattered slots of a dead struct contiguous.

## 5. Branches and block layout

- `bcc L; b end; L:` comes from an inline helper's `return` inside an if: use early returns
  (RPHkyPaddle). `bnelr; ...; blr` from `if (a || b) return;` (RPVsResultLyt).
- An enum ternary stays a branch; an int ternary goes branch-free (RPHkyPuck).
- `case 1: break;` and a `case 5:` sharing `default:` change the compare tree; write case bodies
  in the original's block order.
- A test before the switch: `if (v < X) return;` gives `blt cr1,default` (RPSysHomeMenuMgr).
- CW folds identical if/else arms; one arm through a reference keeps them (RPMenuScene).

## 6. Loops

- Strength-reduced loop pointers are incremented in reverse creation order, and creation follows
  first use in the body: use the pointer that must be incremented first LAST in the body (RPDebugLog
  graphs B/C: the colour reference after the samples).

- The type and form of the index arithmetic decide the counter structure: a signed `i * 2` byte
  offset, `sizeof(T) * i` vs `i * 4`, a u32 loop variable (eggLightManager LoadScnLightInner,
  RPSelectGameLyt).
- Counter outside the `for`, then element pointer, then walker (RPGrpModelMaterial lead).
- `#pragma opt_common_subs off` makes a loop body reload a global; `opt_loop_invariants off`
  removes an extra loop-entry test (RPSportsMessageScene).

## 7. Float constants and pools (.sdata2, .sbss2)

- Pool order = first use in compile order; with `-inline deferred` the last-defined function is
  compiled first. Pull a constant into an earlier local to reorder within a function.
- Angle literals and fractions: write them the original way (`SinDeg(42.0f)`, `26/19`), check
  the DOL bits.
- Named `const` statics defined after the code are loaded from memory and come first in the pool;
  `= 0` goes to .sbss2, `= 0.0f` to .sdata2.
- A constant first used by a function that no longer exists: see Stand-ins (MATCHING.md).

## 8. Data, sections, weak objects

- `status`/`cmp` match pools and strings by value; only `check` (the DOL) and
  `tools/link_diag.py` show order and ownership. Judge by the DOL, not by object diffs: unreferenced
  weak copies are dead-stripped, so a header change can alter 25 objects and leave the DOL
  identical (SetCanvasMode's assert, IBinary's declaration order).
- Weak `@STRING@` literals of a header inline land in the first unit in link order that uses it.
- A weak copy emitted by an earlier-linked object wins over the original owner (RPGrpRenderer's
  AttachCamera): avoid the instantiation (a typed local).
- Every object's data sections start 8-aligned: an 8-byte variable reproduces a 4-byte gap
  (RPGrpRenderer lbl_8043C5B8); a range starting at ...4 means the variable before belongs here.
- A static whose initializer copies words in place is an aggregate copy-initialisation
  (RPKcl_2's `VEC3 sMtxIdentity[3] = {sVecEx, sVecEy, sVecEz}`).
- Local aggregate initializers: .rodata above 8 bytes, .sdata2 below; a later-defined static
  function's goes to .data (cnt.c).
- Duplicate weak data: zero-filled in place when an earlier weak copy exists, removed when it
  loses to a strong one (RPDucGame).

## 9. Function order, thunks, file boundaries

- Normal functions come out in reverse definition order, then weak copies after their first user,
  then vtable-only weak functions; template members in the class template's declaration order
  (eggLightManager's IBinary).
- Thunks end a file; a thunk mid-range, two `.ctors` entries, a duplicated pool constant or two
  string pools mean two files (or one file split in two ranges: RPGrpRenderer absorbed three).
- Thunk order follows vtable construction; it comes out forward only when a derived class's vtable
  is built in the same file (eggDrawPathBase).
- Strong vtables come out in reverse class-declaration order; the key function is the first
  non-inline virtual declared.

## 10. Per-function pragmas (allowed, last resort, comment why)

`#pragma push` ... `#pragma pop` around ONE function: `opt_propagation off` (RPFshPlayer,
RPSportsMessageScene), `opt_loop_invariants off`, `opt_dead_assignments off` (RPPnpServePhase),
`opt_common_subs off`, `scheduling 604` or `off` (RPTnkTankManager, RPSysParticleManager),
`optimization_level 3`; Petari (CC0 SMG decomp) also uses `global_optimizer off` (18 times,
including game code), `optimization_level 0` and `opt_prelinearize off`. A sweep of those on our 12 last functions
(2026-10-28) matched none: `global_optimizer off` and levels 0-2 were far worse, and
`opt_prelinearize off` crashed the compiler (IroLinearForm.c) on five. The pragma state at the function's closing brace applies to the whole
function, inlined code included. Linked units use each of these, so they are plausible original
build settings, but try the natural-C levers first.

## Diagnostics (never committed)

`-opt noschedule`, `#pragma scheduling off` on a variant, `-inline off`, a standalone 20-line
reproducer compiled with the game flags: use them to learn which pass causes a difference, then
find the source form.
