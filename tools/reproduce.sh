#!/bin/bash
# Regenerate everything excluded from git from the user's own Wii Play disc image.
# Run from the repo root (Git Bash on Windows, or Linux):
#   bash tools/reproduce.sh "/path/to/Wii Play (USA) (Rev 1).rvz" [--decomp]
#
#   1. extract the data partition           -> extracted/
#   2. check main.dol against config/RHAE01/config.yml (SHA-1)
#   3. fresh dtk analysis + split           -> dtk/split/  (dtk's own names; ledgers map them)
#   4. function index                       -> dtk/index.json
#   5. ELF with the project's names         -> main.elf
#   --decomp also runs (needs Ghidra + the GameCube loader in ~/tools, natively on Linux or in WSL;
#   see docs/SETUP.md):
#   6. headless Ghidra pseudocode           -> decomp/
#   7. per-function string literals         -> dtk/literals.json
#
# Tool paths can be overridden with DTK and PY.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DISC="${1:?usage: tools/reproduce.sh <disc image> [--decomp]}"
case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*) EXE=.exe; DEF_PY=python; DEF_DTK= ;;
  *) EXE=; DEF_PY=python3; DEF_DTK= ;;
esac
PY="${PY:-$DEF_PY}"
cd "$ROOT"
# dtk: $DTK, else the Windows copy used so far, else the build's own (downloaded here if missing,
# same version as configure.py's dtk_tag)
if [ -z "${DTK:-}" ]; then
  if [ -n "$DEF_DTK" ] && [ -x "$DEF_DTK" ]; then
    DTK="$DEF_DTK"
  else
    DTK="build/tools/dtk$EXE"
    if [ ! -x "$DTK" ]; then
      tag=$(sed -n 's/^config.dtk_tag = "\(.*\)"/\1/p' configure.py)
      echo "[0/5] downloading dtk $tag"
      mkdir -p build/tools
      "$PY" tools/download_tool.py dtk "$DTK" --tag "$tag"
    fi
  fi
fi

echo "[1/5] extracting disc"
rm -rf extracted
"$DTK" disc extract -q "$DISC" extracted

echo "[2/5] checking main.dol"
want=$(sed -n 's/^hash: *//p' config/RHAE01/config.yml)
have=$(sha1sum extracted/sys/main.dol | cut -c1-40)
if [ "$want" != "$have" ]; then
  echo "main.dol SHA-1 $have does not match $want: wrong disc or revision (need RHAE01 Rev 1)" >&2
  exit 1
fi

echo "[3/5] dtk analysis and split"
# Split from a clean symbol state: dtk cannot re-load its own symbols for this DOL
# (two sections share the name .rodata), so analysis always starts fresh in a temp dir.
# Paths stay relative to the project: dtk reads ':' as an archive separator, so "C:/..." breaks.
tmp=dtk/.tmp
rm -rf "$tmp"
mkdir -p "$tmp"
trap 'rm -rf "$ROOT/dtk/.tmp"' EXIT
: > "$tmp/symbols.txt"
: > "$tmp/splits.txt"
{
  echo "object: extracted/sys/main.dol"
  echo "hash: $want"
  echo "symbols: $tmp/symbols.txt"
  echo "splits: $tmp/splits.txt"
} > "$tmp/config.yml"
rm -rf dtk/split
"$DTK" dol split "$tmp/config.yml" dtk/split > "$tmp/split.log" 2>&1 || { cat "$tmp/split.log" >&2; exit 1; }
if ! diff -q <(sort "$tmp/symbols.txt") <(sort dtk/symbols.orig.txt) > /dev/null; then
  echo "warning: fresh dtk symbols differ from dtk/symbols.orig.txt (different dtk version?)" >&2
fi

echo "[4/5] function index"
"$PY" -I tools/build_index.py .

echo "[5/5] main.elf"
"$PY" -I tools/dol2elf.py extracted/sys/main.dol main.elf \
  --symbols config/RHAE01/symbols.txt --sda 0x80442AC0 --sda2 0x80444B00 | tail -1

if [ "${2:-}" = "--decomp" ]; then
  if [ -n "$EXE" ]; then
    echo "[6/7] Ghidra pseudocode (WSL, ~10 min)"
    win=$(cygpath -m "$ROOT")   # Windows drive path -> WSL mount path
    # MSYS_NO_PATHCONV: stop Git Bash rewriting /mnt/... into a Windows path for wsl.exe
    MSYS_NO_PATHCONV=1 wsl.exe -d Ubuntu -- bash "/mnt/$(echo "${win:0:1}" | tr 'A-Z' 'a-z')${win:2}/tools/export_decomp.sh" > /dev/null
  else
    echo "[6/7] Ghidra pseudocode (~10 min)"
    bash tools/export_decomp.sh > /dev/null
  fi
  echo "[7/7] literals"
  "$PY" -I tools/extract_literals.py .
fi
echo "done"
