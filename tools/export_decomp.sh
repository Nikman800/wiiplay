#!/bin/bash
# Run on Linux (or inside WSL on Windows): headless Ghidra import of main.elf with the Gekko/Broadway
# language, SDA registers seeded before analysis, then pseudocode export.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GHIDRA="${GHIDRA:-$HOME/tools/ghidra_12.1.4_PUBLIC}"
export JAVA_HOME="${JAVA_HOME:-$HOME/tools/jdk21}"
PROJ="$HOME/ghidra-projects/wii-play"
OUT="$ROOT/decomp"
mkdir -p "$PROJ"
rm -rf "$PROJ/WiiPlay.gpr" "$PROJ/WiiPlay.rep" "$OUT"
"$GHIDRA/support/analyzeHeadless" "$PROJ" WiiPlay \
  -import "$ROOT/main.elf" \
  -processor PowerPC:BE:32:Gekko_Broadway -cspec default \
  -scriptPath "$ROOT/tools/ghidra_scripts" \
  -preScript SetSdaRegisters.java 0x80442AC0 0x80444B00 \
  -postScript ExportDecompiledC.java "$OUT" "$ROOT/dtk/units.tsv" \
  -max-cpu 12
