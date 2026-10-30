import os
"""Replay a mwcc_trace --json GPR colouring with interference edges added/removed and vregs renumbered.
Usage: edges.py trace.json [-a:A-B] [-d:A-B] [v=key] ... [--show=v,v]
-a:A-B adds the edge A-B, -d:A-B removes it; v=key renumbers v in the simplify sweep."""
import sys, copy
sys.path.insert(0, os.path.join(os.getcwd(), 'tools', 'mwcc_trace'))
import sim
p, nodes = sim.load(sys.argv[1])
base = sim.run(nodes, set(p['order']))[1]
nodes = copy.deepcopy(nodes)
moves = {}; show = []
for a in sys.argv[2:]:
    if a.startswith('--show='): show = [int(x) for x in a[7:].split(',')]
    elif a[:3] in ('-a:', '-d:'):
        x, y = (int(t) for t in a[3:].split('-'))
        for u, w in ((x, y), (y, x)):
            nb = set(nodes[u]['neighbours'])
            (nb.add if a[1] == 'a' else nb.discard)(w)
            nodes[u]['neighbours'] = sorted(nb)
    else:
        v, k = a.split('='); moves[int(v)] = float(k)
res = sim.run(nodes, set(p['order']), key=lambda v: moves.get(v, v))
if res is None: print('spill'); sys.exit()
col, reg, lvl = res
ch = [(v, nodes[v].get('var') or '', base[v], reg[v]) for v in col if reg[v] != base[v]]
print(len(ch), 'changed')
for c in ch[:40]: print('  r%d %s: r%d -> r%d' % c)
for v in show: print('  show r%d -> r%d (lvl %d)' % (v, reg[v], lvl[v]))
