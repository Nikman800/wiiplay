import os
"""Search vreg renumberings (1 or 2 moves) that give a target colouring.
Usage: search.py trace.json 'v=r,v=r,...' [cand vregs comma list] [--pairs]
Target: each v must get physical r. Moves: each candidate vreg to just below/above every other vreg."""
import sys, itertools
sys.path.insert(0, os.path.join(os.getcwd(), 'tools', 'mwcc_trace'))
import sim
p, nodes = sim.load(sys.argv[1])
tgt = dict((int(a), int(b)) for a, b in (x.split('=') for x in sys.argv[2].split(',')))
cands = [int(x) for x in sys.argv[3].split(',')] if len(sys.argv) > 3 and sys.argv[3] != '-' else sorted(nodes)
pairs = '--pairs' in sys.argv
allv = sorted(p['order'])
anchors = sorted(set([v - 0.5 for v in allv] + [max(allv) + 0.5]))
base = sim.run(nodes, set(p['order']))[1]
def score(moves):
    r = sim.run(nodes, set(p['order']), key=lambda v: moves.get(v, v))
    if r is None: return None
    reg = r[1]
    ok = all(reg[v] == t for v, t in tgt.items())
    return ok, sum(1 for v in reg if reg[v] != base[v])
hits = []
for c in cands:
    for a in anchors:
        s = score({c: a})
        if s and s[0]: hits.append((s[1], {c: a}))
if pairs:
    for c1, c2 in itertools.combinations(cands, 2):
        for a1 in anchors[::1]:
            for a2 in anchors[::1]:
                s = score({c1: a1, c2: a2})
                if s and s[0]: hits.append((s[1], {c1: a1, c2: a2}))
hits.sort(key=lambda h: h[0])
print(len(hits), 'hits')
for h in hits[:30]: print(h)
