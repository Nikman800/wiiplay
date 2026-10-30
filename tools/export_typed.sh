#!/bin/bash
# Run on Linux (or inside WSL on Windows) after export_decomp.sh and, on Windows, tools/gen_ghidra_types.py +
# tools/gen_this_map.py: apply the generated types to the analyzed project (read-only, not saved)
# and export typed pseudocode for every function to decomp_typed/all.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GHIDRA="${GHIDRA:-$HOME/tools/ghidra_12.1.4_PUBLIC}"
export JAVA_HOME="${JAVA_HOME:-$HOME/tools/jdk21}"
OUT="$ROOT/decomp_typed/all"
rm -rf "$OUT"
"$GHIDRA/support/analyzeHeadless" "$HOME/ghidra-projects/wii-play" WiiPlay \
  -process main.elf -noanalysis -readOnly \
  -scriptPath "$ROOT/tools/ghidra_scripts" \
  -postScript ApplyTypes.java "$ROOT/build/ghidra/types.h" "$ROOT/build/ghidra/this_map.tsv" \
    "$ROOT/build/ghidra/globals.tsv" \
  -postScript ExportDecompiledC.java "$OUT" "$ROOT/dtk/units.tsv"
