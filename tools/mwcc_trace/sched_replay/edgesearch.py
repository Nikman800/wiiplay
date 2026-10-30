import os
"""Find minimal interference-edge toggles (1..N) that give a target colouring.
Usage: edgesearch.py trace.json 'v=r,...' 'v,v,...(node set)' [maxk]"""
import sys, copy, itertools
sys.path.insert(0, os.path.join(os.getcwd(), 'tools', 'mwcc_trace'))
import sim
p, nodes = sim.load(sys.argv[1])
tgt = dict((int(a), int(b)) for a, b in (x.split('=') for x in sys.argv[2].split(',')))
S = [int(x) for x in sys.argv[3].split(',')]
maxk = int(sys.argv[4]) if len(sys.argv) > 4 else 2
base = sim.run(nodes, set(p['order']))[1]
nb0 = {v: set(nodes[v]['neighbours']) for v in nodes}
pairs = list(itertools.combinations(S, 2))
def test(tog):
    nb = dict(nb0)
    for a, b in tog:
        nb[a] = set(nb[a]) ^ {b}; nb[b] = set(nb[b]) ^ {a}
    nn = {v: dict(nodes[v], neighbours=sorted(nb[v])) for v in nodes}
    r = sim.run(nn, set(p['order']))
    if not r: return None
    reg = r[1]
    if all(reg[v] == t for v, t in tgt.items()):
        return sum(1 for v in reg if reg[v] != base[v])
for k in range(1, maxk + 1):
    hits = []
    for tog in itertools.combinations(pairs, k):
        s = test(tog)
        if s is not None:
            hits.append((s, [('+' if b not in nb0[a] else '-') + '%d-%d' % (a, b) for a, b in tog]))
    hits.sort()
    print('k=%d: %d hits' % (k, len(hits)))
    for h in hits[:25]: print('  ', h)
    if hits: break
