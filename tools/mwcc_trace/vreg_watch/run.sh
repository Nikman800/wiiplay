#!/bin/sh
# usage: tools/mwcc_trace/vreg_watch/run.sh UNIT FUNC [SRC|-] [CLASS 3=FPR 4=GPR] [NTH]
# Logs every new virtual register of CLASS created while FUNC is generated (one JSON line per
# vreg: "new", "pc" after the increment of used_virtual_registers[class], "rets"). Sites:
#   0x594892  codegen start, per local (named locals first, then @temps)
#   0x590f6d / 0x4fac92 / 0x58ff2d.. / 0x5e6b2c  codegen expression temps (PCode order)
#   0x62e2b4  post-codegen promotion of an address-taken scalar (0x62dcc0, O4 backend optimizer)
# See tools/LEVERS.md "Where vregs come from".
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../../.." && pwd)
WORK=${TMPDIR:-/tmp}/vreg_watch
mkdir -p "$WORK/obj"
cd "$ROOT"
OUT=$(mktemp "$WORK/log.XXXX")
CMD=$(python3 - "$1" "$3" "$WORK/obj" <<'PY'
import sys
sys.path.insert(0, "tools")
import mwcc_trace as M
src = sys.argv[2] if sys.argv[2] not in ("", "-") else None
print(M.build_command(sys.argv[1], src, sys.argv[3]))
PY
)
export RV_FUNC="$2" RV_OUT=$OUT RV_CLASS=${4:-3} RV_NTH=${5:-1}
eval "timeout 600 gdb -q -nx -batch -iex 'set debuginfod enabled off' -x '$HERE/vreg_watch.py' --args $CMD" >"$WORK/gdb.out" 2>&1
cat "$OUT"
