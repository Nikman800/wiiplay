#!/bin/bash
# Steps 1-6 of tools/rhae/README.md for every library in tools/rhae_libs.py.
# Usage: RHAE=<Rhae clone> tools/rhae/adopt_all.sh; then tools/phase2_pipeline.sh
set -euo pipefail
cd "$(dirname "$0")/../.."
: "${RHAE:?set RHAE to a clone of github.com/akiramusic000/Rhae}"
python3 tools/rhae/adopt_rhae_splits.py -w
python3 tools/rhae/straddle.py -w
python3 tools/rhae/align_from_rhae.py
python3 tools/rhae/gap_align.py
python3 tools/rhae/names_from_rhae.py -w
python3 tools/rhae/pad_align.py
