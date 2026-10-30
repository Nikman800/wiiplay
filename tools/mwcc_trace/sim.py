"""Replay CodeWarrior 3.0a5.2's GPR simplify/colour from a tools/mwcc_trace.py --json dump.

Usage: python3 tools/mwcc_trace/sim.py trace.json
Prints whether the replay reproduces the traced colouring order and registers. To test a
hypothesis before editing source ("what if this value were numbered lower?"), import run() and
pass a different `key` for the virtual-register order.

Model (measured on ef_animcurve): simplify sweeps the virtual registers in ascending order, removing
every node whose remaining degree is below K (29 GPRs), level by level; nodes are coloured in reverse
removal order; a node takes the lowest volatile register (r0, r3-r12) free of its neighbours, else the
lowest callee-saved register already opened, else opens a new one counting down from r31.
"""
import json
import sys

K_GPR = 29


def load(path):
    d = json.load(open(path))
    p = [p for p in d["passes"] if p["class"] == 4][-1]  # last GPR pass
    nodes = {int(k): v for k, v in p["nodes"].items()}
    return p, nodes


def run(nodes, order_set, key=lambda v: v, K=K_GPR):
    real = sorted(order_set, key=key)
    deg = {v: len(set(nodes[v]["neighbours"])) for v in real}
    removed, stack, lvl, level = set(), [], {}, 0
    while len(removed) < len(real):
        level += 1
        progress = False
        for v in real:
            if v in removed or deg[v] >= K:
                continue
            removed.add(v)
            stack.append(v)
            lvl[v] = level
            progress = True
            for u in set(nodes[v]["neighbours"]):
                if u in deg:
                    deg[u] -= 1
        if not progress:
            return None  # would spill: the model does not cover spill selection
    col = stack[::-1]
    reg, used = {}, []
    for v in col:
        nb = set(nodes[v]["neighbours"])
        taken = {u for u in nb if u < 32} | {reg[u] for u in nb if u in reg}
        r = next((c for c in list(range(0, 13)) + sorted(used) if c not in (1, 2) and c not in taken), None)
        if r is None:
            r = 31 - len(used)
            used.append(r)
        reg[v] = r
    return col, reg, lvl


def main():
    p, nodes = load(sys.argv[1])
    res = run(nodes, set(p["order"]))
    if res is None:
        print("the replay needs a spill; not modelled")
        return 1
    col, reg, _ = res
    print("order reproduced:", col == p["order"])
    bad = [(v, reg[v], nodes[v]["reg"]) for v in col if reg[v] != nodes[v]["reg"]]
    print("registers reproduced:", not bad, bad[:10])
    return 0


if __name__ == "__main__":
    sys.exit(main())
