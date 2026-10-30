#!/usr/bin/env python3
"""usage: run.py UNIT [SRC] [FUNCSUBSTR] > log
Compile UNIT (e.g. Pack/RPParty/RPPartyCommon/RPCPadCursor_2.cpp) -- or SRC with UNIT's flags and
include dir -- under gdb with probe.py; prints the alias log (mkdummy / worst-case adds / scan)."""
import sys, subprocess, os, importlib.util, tempfile
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..')); sys.path.insert(0, 'tools')
unit = sys.argv[1]; src = sys.argv[2] if len(sys.argv) > 2 and sys.argv[2] != '-' else None
filt = sys.argv[3] if len(sys.argv) > 3 else ''
argv = sys.argv; sys.argv = ['x']
spec = importlib.util.spec_from_file_location('mt', 'tools/mwcc_trace.py')
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
with tempfile.TemporaryDirectory() as tmp:
    cmd = m.build_command(unit, src, tmp)
    log = os.path.join(tmp, 'log.txt')
    g = f"gdb -q -nx -batch -iex 'set debuginfod enabled off' -x tools/mwcc_trace/alias_probe/probe.py --args {cmd}"
    r = subprocess.run(g, shell=True, env=dict(os.environ, PROBE_OUT=log, PROBE_FUNC=filt),
                       capture_output=True, text=True, timeout=1800)
    if os.path.exists(log) and os.path.getsize(log) > 0: print(open(log).read())
    else: print(r.stdout[-2000:], r.stderr[-2000:])
