#!/bin/bash
# usage: [UNIT=<splits unit>] tools/mwcc_trace/vn_probe/vn.sh src.cpp func [out]
#   src relative to ~/wii-play or absolute; func = the plain identifier CodeWarrior's codegen sees
#   (a ctor is "__ct"). Flags come from the unit's project command (default RPKokeshiMgr_3).
#   Writes out (default src.vn): every load seen by backend value numbering + MERGED markers.
cd "$(dirname "$0")/../../.."
src=$1; out=${3:-${src%.*}.vn}
eval "VN_FUNC=$2 VN_OUT=$out gdb -q -nx -batch -iex 'set debuginfod enabled off' -x tools/mwcc_trace/vn_probe/gdb_vn.py --args build/tools/wibo build/compilers/GC/3.0a5.2/mwcceppc.exe $(python3 tools/mwcc_trace/vn_probe/flags.py $UNIT) -c $src -o tools/mwcc_trace/vn_probe/scratch.o" >tools/mwcc_trace/vn_probe/last_gdb.log 2>&1
echo "$out: $(grep -c LOAD $out) loads, $(grep -c MERGED $out) merged"
