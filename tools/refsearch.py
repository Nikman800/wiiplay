"""Find a function's definition in the local reference decomps.

Usage: refsearch.py <name> [--show N] [--refs DIR,...]

Searches *.c/*.cpp/*.h/*.s under each reference repo (default: every directory in ~/ext that
has a LICENSE file: Rhae, Petari, pik1wii, ogws, ...) for a definition of <name> (the identifier
followed by "(" and a "{" within a few lines), and prints repo, license (first line of LICENSE),
path:line and, with --show N, the first N lines of each definition. Only use code from repos whose
license allows it (CC0 here); the license is printed next to every hit for that reason.
To add a repo: git clone --depth 1 <url> ~/ext/<name>.
"""
import argparse
import re
import sys
from pathlib import Path

EXT = Path.home() / "ext"


def license_of(repo):
    for f in sorted(repo.glob("LICENSE*")) + sorted(repo.glob("COPYING*")):
        for line in f.read_text(errors="replace").splitlines():
            line = line.strip()
            if line and not line.lower().startswith("creative commons legal code"):
                return line[:60]
    return "no LICENSE file: do not copy from it"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("name")
    ap.add_argument("--show", type=int, default=0)
    ap.add_argument("--refs", default="")
    a = ap.parse_args()
    repos = [Path(p).expanduser() for p in a.refs.split(",") if p] or \
        [d for d in sorted(EXT.iterdir()) if d.is_dir() and (list(d.glob("LICENSE*")) or list(d.glob("COPYING*")))]
    pat = re.compile(r"\b" + re.escape(a.name) + r"\s*\(")
    hits = 0
    for repo in repos:
        lic = license_of(repo)
        for f in repo.rglob("*"):
            if f.suffix not in (".c", ".cpp", ".h", ".hpp", ".inl") or ".git" in f.parts:
                continue
            try:
                lines = f.read_text(errors="replace").splitlines()
            except OSError:
                continue
            for i, l in enumerate(lines):
                if not pat.search(l) or l.rstrip().endswith(";") or l.lstrip().startswith(("//", "*", "#")):
                    continue
                if not any("{" in x for x in lines[i:i + 4]):
                    continue
                hits += 1
                print(f"{repo.name} [{lic}] {f.relative_to(repo)}:{i + 1}")
                for x in lines[i:i + a.show]:
                    print("    " + x)
    if not hits:
        print(f"no definition of {a.name} in: " + ", ".join(r.name for r in repos))
    return 0


if __name__ == "__main__":
    sys.exit(main())
