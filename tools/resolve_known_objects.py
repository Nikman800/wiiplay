"""Resolve dtk "Symbol X overlaps with symbol lbl_Y" errors after library renames.

Naming runtime/SDK functions lets dtk's analyzer recreate known data objects (e.g. MSL's
__files). Generic lbl_ labels from the first unnamed analysis then overlap them. This loop runs
`dtk dol split`, and for each overlap between a named object and generic lbl_ labels inside its
range, replaces those labels with the named object. Any other conflict stops the loop.

Usage: resolve_known_objects.py <root> [--max 200]
"""
import argparse
import os
import re
import subprocess
from pathlib import Path

ERR = re.compile(r"Symbol (\S+) \(0x([0-9A-F]+)\.\.0x([0-9A-F]+)\) overlaps with symbol (\S+) \(0x([0-9A-F]+)\.\.0x([0-9A-F]+)")
LINE = re.compile(r"^(\S+) = (\S+):0x([0-9A-F]+); // (.*)$")
GENERIC = re.compile(r"^(lbl|jumptable|@stringBase\d*)_?[0-9A-F]{8}$|^lbl_")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("root")
    ap.add_argument("--max", type=int, default=200)
    args = ap.parse_args()
    root = Path(args.root)
    sym = root / "config/RHAE01/symbols.txt"
    dtk = root / ("build/tools/dtk.exe" if os.name == "nt" else "build/tools/dtk")
    for it in range(args.max):
        proc = subprocess.run([str(dtk), "dol", "split", "config/RHAE01/config.yml", "build/RHAE01"],
                              cwd=root, capture_output=True, text=True, encoding="utf-8", errors="replace")
        out = re.sub(r"\x1b\[[0-9;]*m", "", proc.stdout + proc.stderr)
        if proc.returncode == 0:
            print(f"dtk split OK after {it} fixes")
            return 0
        m = ERR.search(out)
        if not m:
            print(out[-1500:])
            return 1
        a_name, a0, a1, b_name, b0, b1 = m.groups()
        a0, a1, b0, b1 = (int(x, 16) for x in (a0, a1, b0, b1))
        # the named object keeps its range; generic labels fully inside it are dropped
        if GENERIC.match(b_name) and not GENERIC.match(a_name):
            keep, k0, k1 = a_name, a0, a1
        elif GENERIC.match(a_name) and not GENERIC.match(b_name):
            keep, k0, k1 = b_name, b0, b1
        else:
            print(f"stopping: non-generic overlap {a_name} / {b_name}")
            return 1
        lines = sym.read_text(encoding="utf-8").splitlines()
        new, dropped = [], []
        for l in lines:
            lm = LINE.match(l)
            if lm and GENERIC.match(lm.group(1)) and "type:function" not in lm.group(4):
                addr = int(lm.group(3), 16)
                if k0 <= addr < k1:
                    dropped.append(lm.group(1))
                    continue
            new.append(l)
        if not dropped:
            print(f"stopping: nothing to drop for {keep} 0x{k0:X}..0x{k1:X}")
            return 1
        sym.write_text("\n".join(new) + "\n", encoding="utf-8", newline="\n")
        print(f"{keep} 0x{k0:X}..0x{k1:X}: dropped {', '.join(dropped)}")
    print("gave up after max iterations")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
