Probes for "is a setup-order / colouring difference decided before or after register allocation?"
(ef_animcurve U8/F32 setup block, 2026-10-29). All take a `tools/mwcc_trace.py <unit> <fn> --class gpr
--json X.json` dump and replay the GPR simplify/colour with tools/mwcc_trace/sim.py's model.

renum.py X.json v=key ... [--show=v,v]
    Renumber vregs (float key = position in the ascending simplify sweep) and print which
    registers change.
edges.py X.json [-a:A-B] [-d:A-B] [v=key] ... [--show=v,v]
    Add (-a) or delete (-d) interference edges, optionally renumber, and print the changes.
    Use it to test "what if the pre-RA schedule had placed X before/after Y" (an edge exists
    iff one value is defined while the other is live in the run-0 output order).
edgesearch.py X.json 'v=r,...' 'v,v,...' [maxk]
    Exhaustive search for the smallest set of edge toggles among the listed vregs that gives
    the target registers. Slow above k=3 with ~12 nodes (minutes).
search.py X.json 'v=r,...' [cands] [--pairs]
    Same for renumberings (single moves, or pairs).
mkvar.py src func out.cpp spec.py
    Build a tools/try_variants.py file from the current definition plus (old, new) text
    replacements (spec.py: VARIANTS = [[(old, new), ...], ...]).

Method: 1) --sched --run 0 gives the order the allocator sees; --sched --run 1 the post-RA
picks. 2) Map the original's registers onto the vregs of that block. 3) If edges.py with the
original's ordering edges reproduces the original's registers and no renumbering does (search.py),
the difference is in the pre-RA schedule (the interference), not in numbering or the post-RA pass.
