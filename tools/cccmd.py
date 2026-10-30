"""Compile one unit with its exact build command, optionally varied, into a scratch directory.

compile_variant(unit, src=None, compiler=None, add=(), remove=(), outdir=None) -> object path or None
  src       compile this file instead of src/<unit> (it keeps quoted #includes working by adding
            the unit's own directory to the search path)
  compiler  e.g. "GC/3.0a5.2" (a directory under build/compilers)
  add       extra flags appended, e.g. ["-ipa file"]
  remove    flags dropped (exact text, e.g. "-Cpp_exceptions off")
  include_first  a directory of shadow headers searched before include/
Lock-free: it never touches build/, so it can run in parallel with workers and other compiles.
"""
import os
import re
import shlex
import subprocess
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


_commands = {}


def base_command(unit):
    """cached per unit, and retried: build.ninja is briefly missing while configure.py rewrites it"""
    if unit in _commands:
        return _commands[unit]
    obj = "build/RHAE01/src/" + Path(unit).with_suffix(".o").as_posix()
    for attempt in range(6):
        r = subprocess.run(["ninja", "-t", "commands", obj], cwd=ROOT, capture_output=True, text=True)
        lines = [l for l in r.stdout.splitlines() if "mwcceppc" in l]
        if lines:
            _commands[unit] = lines[-1].split(" && ")[0]
            return _commands[unit]
        time.sleep(5)
    raise RuntimeError(f"no compile command for {obj} (run configure.py?)")


def compile_variant(unit, src=None, compiler=None, add=(), remove=(), outdir=None, timeout=120,
                    include_first=None):
    cmd = base_command(unit)
    if include_first:  # a shadow include tree searched before include/ and its sub-paths
        sh = os.path.abspath(include_first)
        cmd = re.sub(r" -i include(/\S*)? ", lambda m: f" -i {sh}{m.group(1) or ''} -i include{m.group(1) or ''} ", cmd)
    if compiler:
        cmd = re.sub(r"build/compilers/[^ ]+/mwcceppc\.exe", f"build/compilers/{compiler}/mwcceppc.exe", cmd)
    for f in remove:
        cmd = cmd.replace(" " + f, "", 1)
    if add:
        cmd = cmd.replace(" -c ", " " + " ".join(add) + " -c ", 1)
    out = Path(outdir or tempfile.mkdtemp(prefix="cc_"))
    out.mkdir(parents=True, exist_ok=True)
    orig = "src/" + unit
    if src:
        src = os.path.abspath(src)
        cmd = cmd.replace(" -c " + orig, f" -i {Path(orig).parent.as_posix()} -c {src}", 1)
        obj = out / (Path(src).stem + ".o")
    else:
        obj = out / (Path(unit).stem + ".o")
    cmd = re.sub(r" -o \S+", f" -o {out.as_posix()}", cmd, count=1)
    cmd = cmd.replace(" -MMD", "")
    try:
        r = subprocess.run(cmd, shell=True, cwd=ROOT, capture_output=True, text=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return None
    if obj.exists() and r.returncode == 0:
        return obj
    return None


if __name__ == "__main__":
    import sys
    print(compile_variant(sys.argv[1]))
