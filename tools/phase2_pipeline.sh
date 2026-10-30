#!/bin/bash
# Phase 2: name library (and shared RP framework) functions from reference decomps, derive
# source-file units with code and data ranges, and rebuild a byte-identical DOL.
# Run (Git Bash on Windows, or Linux) after tools/reproduce.sh and one `python configure.py && ninja`.
# Order matters: data ranges and force_active need the asm of a fresh dtk split.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*)
    PY="${PY:-python}"
    NINJA="${NINJA:-ninja}" ;;
  *) PY="${PY:-python3}"; NINJA="${NINJA:-ninja}" ;;
esac
cd "$ROOT"

# The build directory and symbols.txt are shared with matching agents (tools/unit.py): hold the
# same lock for the whole run, refreshed by a heartbeat.
LOCK="$ROOT/build/.unit.lock"
mkdir -p "$ROOT/build"
until ( set -C; echo "$(cat /proc/$$/winpid 2>/dev/null || echo $$) pipeline $(date)" > "$LOCK" ) 2>/dev/null; do
  echo "  waiting for the build lock ($(cat "$LOCK" 2>/dev/null))"
  sleep 10
done
# heartbeat: other holders take a lock not refreshed for 2 minutes to be dead (tools/unit.py)
( while [ -f "$LOCK" ]; do touch -c "$LOCK"; sleep 10; done ) &
HEARTBEAT=$!
trap 'kill $HEARTBEAT 2>/dev/null; rm -f "$LOCK"' EXIT

# Every step is idempotent. Windows sometimes holds a file just written by the step before
# (OSError 22 on config/RHAE01/symbols.txt), so a failed step is retried twice.
py() {
  for attempt in 1 2 3; do
    "$PY" -I "$@" && return 0
    [ "$attempt" = 3 ] || { echo "  retrying: $*"; sleep 3; }
  done
  return 1
}

[ -f ref/ogws/symbols.txt ] || bash tools/fetch_refs.sh    # reference tables (pinned commits)
py tools/align_refs.py .                  # exact size runs, library code
py tools/align_refs.py . --game           # exact size runs, game code (RP framework)
py tools/fill_clusters.py .               # ordered alignment for remaining clusters
py tools/apply_library_names.py .         # rename in config/RHAE01/symbols.txt
py tools/apply_read_names.py . apply      # names read from code, only for remaining fn_*
py tools/align_data.py .                  # library data names from references (size runs)
py tools/apply_library_data.py .          # ... applied to generic data labels only
py tools/apply_globals.py .               # Phase 3 global-data names (dtk/types/*_globals.tsv)
py tools/apply_method_renames.py . apply  # Phase 3 reviewed game method names (dtk/method_renames.tsv)
py tools/apply_source_names.py .         # Phase 4 names the decompiled source compiles to (dtk/source_names.tsv)
py tools/make_lib_splits.py .             # .text units from reference source files
py tools/make_game_splits.py .            # game .text units from dtk/game_units.tsv
py tools/resolve_known_objects.py .       # dtk split (+ fix known-object overlaps)
py tools/add_data_splits.py .             # data ranges from exclusive references
py tools/resolve_known_objects.py .       # dtk split again with data ranges
py tools/add_data_splits.py .             # again, on the asm of that split: references settle (Phase 4)
py tools/resolve_known_objects.py .       # and split once more
py tools/force_active_units.py .          # keep objects nothing references
py tools/gen_units_config.py .            # register units for progress tracking
"$PY" configure.py > /dev/null
"$NINJA"
