import os
"""Replay a mwcc_trace --json GPR colouring with some vregs renumbered.
Usage: renum.py trace.json 'v=newkey' ... [--show v,v,...]
newkey is a float position in the simplify sweep (vreg numbers are the default keys).
Prints the registers that change vs the traced colouring."""
import sys
sys.path.insert(0, os.path.join(os.getcwd(), 'tools', 'mwcc_trace'))
import sim
p, nodes = sim.load(sys.argv[1])
moves = {}; show = []
for a in sys.argv[2:]:
    if a.startswith('--show='):
        show = [int(x) for x in a[7:].split(',')]
    else:
        v, k = a.split('='); moves[int(v)] = float(k)
res = sim.run(nodes, set(p['order']), key=lambda v: moves.get(v, v))
if res is None:
    print('spill'); sys.exit()
col, reg, lvl = res
base = sim.run(nodes, set(p['order']))[1]
ch = [(v, nodes[v].get('variable') or nodes[v].get('name',''), base[v], reg[v]) for v in col if reg[v] != base[v]]
print(len(ch), 'changed')
for c in ch[:40]: print('  r%d %s: r%d -> r%d' % c)
for v in show: print('  show r%d -> r%d' % (v, reg[v]))
