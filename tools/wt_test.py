"""Test a ledger/header patch in a throwaway git worktree before it touches the shared tree.

Usage: wt_test.py [--patch FILE]... [unit ...] [--keep] [--jobs N]

1. `git worktree add` at HEAD, then copy in the shared tree's uncommitted work under src/,
   include/, config/, dtk/, configure.py and tools/ (agents' drafts), and symlink the large inputs
   (extracted/, build/tools, build/compilers).
2. Apply each --patch (git apply), run configure.py and build main.dol: the asm-only build must
   still match (moving data ranges can break it even when no unit is linked from source).
3. `tools/unit.py check` each named unit there (links it from source; matching.txt in the worktree
   only).
Prints one verdict per step; exit status 0 only if every step passed. The worktree is removed
unless --keep. Nothing in the shared tree changes.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
COPY = ("src/", "include/", "config/", "dtk/", "configure.py", "tools/")


def sh(cmd, cwd, **kw):
    return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, **kw)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("units", nargs="*")
    ap.add_argument("--patch", action="append", default=[])
    ap.add_argument("--keep", action="store_true")
    a = ap.parse_args()
    wt = Path(f"/tmp/wt_test_{os.getpid()}")
    r = sh(["git", "worktree", "add", "--detach", str(wt), "HEAD"], ROOT)
    if r.returncode:
        sys.exit(r.stderr)
    ok = True
    try:
        st = sh(["git", "status", "--porcelain", "--untracked-files=all"], ROOT).stdout.splitlines()
        n = 0
        for l in st:
            path = l[3:].split(" -> ")[-1]
            if l[:2].strip() == "D" or not path.startswith(COPY) or not (ROOT / path).is_file():
                continue
            dst = wt / path
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT / path, dst)
            n += 1
        # extracted/ and the compilers are read-only inputs: symlink them. build/tools is COPIED,
        # because a fresh worktree's tools/download_tool.py is newer than the binaries and ninja
        # would rewrite them through a symlink while other processes run them ("Text file busy")
        for link in ("extracted", "build/compilers"):
            (wt / link).parent.mkdir(parents=True, exist_ok=True)
            if not (wt / link).exists():
                os.symlink(ROOT / link, wt / link)
        if not (wt / "build/tools").exists():
            shutil.copytree(ROOT / "build/tools", wt / "build/tools", symlinks=True)
        for f in (wt / "build/tools").iterdir():  # newer than download_tool.py: ninja keeps them
            os.utime(f, None)
        print(f"worktree {wt}: {n} uncommitted file(s) copied from the shared tree")
        for p in a.patch:
            r = sh(["git", "apply", os.path.abspath(p)], wt)
            print(f"  patch {p}: " + ("applied" if r.returncode == 0 else "FAILED " + r.stderr.strip()))
            ok &= r.returncode == 0
        if ok:
            sh([sys.executable, "configure.py"], wt)
            r = sh(["ninja", "build/RHAE01/main.dol"], wt, timeout=7200)
            h = sh([str(wt / "build/tools/dtk"), "shasum", "-c", "config/RHAE01/build.sha1"], wt)
            out = re.sub(r"\x1b\[[0-9;]*m", "", h.stdout + h.stderr)
            asm_ok = r.returncode == 0 and "main.dol: OK" in out
            errs = [l for l in (r.stdout + r.stderr).splitlines() if re.search(r"error|Error|overlap", l)]
            why = "" if r.returncode == 0 else " (" + (errs[0] if errs else "ninja failed").strip()[:240] + ")"
            print("  asm-only build: " + ("OK" if asm_ok else "FAILED" + why))
            ok &= asm_ok
        if ok:
            for u in a.units:
                r = sh([sys.executable, "tools/unit.py", "check", u], wt, timeout=7200)
                last = (r.stdout.strip().splitlines() or ["(no output)"])[-1]
                good = "DOL matches" in last
                print(f"  {u}: {last.strip()}")
                ok &= good
    finally:
        if a.keep:
            print(f"kept {wt} (remove with: git worktree remove --force {wt})")
        else:
            sh(["git", "worktree", "remove", "--force", str(wt)], ROOT)
    print("PASS" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
