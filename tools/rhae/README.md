# Library layout from Rhae

Rhae (github.com/akiramusic000/Rhae, CC0-1.0) decompiles the same DOL. Its sources for the
libraries in tools/rhae_libs.py are imported under src/, and those libraries' unit layout in
config/RHAE01/splits.txt is Rhae's. These scripts did that adoption; `adopt_all.sh` runs steps
1-6 in order (from the repo root, RHAE=<path to a Rhae clone>), then run tools/phase2_pipeline.sh:

1. `adopt_rhae_splits.py -w`: replace our blocks (outside game code) with Rhae's blocks for the
   units Rhae matches, in place; drop or trim our blocks that clash with them. Blocks with dtk
   section renames are not adopted (our dtk has no `rename:`).
2. `straddle.py -w`: our data symbols that cross one of the new boundaries take Rhae's symbols.
3. `align_from_rhae.py`: Rhae's align: hints for symbols in those libraries' data ranges.
4. `gap_align.py`: a range after a padding-only gap gets an align: hint on its first symbol,
   so the asm of units not yet linked from source keeps the padding.
5. `names_from_rhae.py -w`: generic labels (lbl_*) in the data ranges of units Rhae matches
   take Rhae's names (and its object boundaries, where its objects tile or cover our labels),
   so data that asm units reference is defined under the name the source compiles to.
6. `pad_align.py`: inside those data ranges, a symbol after padding gets the align: hint that
   reproduces it (needed after 5 shrinks labels that had covered the padding).
7. `fn_names_from_rhae.py -w` (run once, after 1-6): functions in the units Rhae matches take
   Rhae's names where ours differ (names game code uses, and dtk/name_corrections.tsv rows, are
   kept); the renames go to dtk/name_corrections.tsv so the pipeline keeps them.
8. `weak_from_rhae.py -w`: data Rhae marks weak (string pools and statics of inline functions)
   takes Rhae's name and weak scope, so a source-linked unit's copy merges with the asm one.

tools/make_lib_splits.py and tools/add_data_splits.py keep these blocks as written. Labels a
source file still declares by name (extern "C" stand-ins in game code) keep their lbl_ name in
step 5; the library unit that defines them can link only once that source uses the real name.
Rhae's per-library flags and per-object compiler options are in configure.py (lib_options,
object_options).
