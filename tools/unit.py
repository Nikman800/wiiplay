"""Matching workflow for one translation unit (Phase 4). See tools/MATCHING.md.

  unit.py status <unit>           compile src/<unit>, show every function and data object's match
  unit.py diff <unit> <symbol>    one function side by side: target (original) | base (source)
  unit.py cmp <unit>              real differences per function, ignoring relocation names (lbl_X vs @N),
                                  plus the function-order check against the original (catches reversed order)
  unit.py sync <unit> [--force]   give the target symbols the names the source compiles to, apply
                                  them and rebuild, so objdiff pairs every function and callee
  unit.py check <unit>...         link the unit(s) from source; keep them (matching.txt) if the DOL matches
  unit.py lib                     library name of every unit, for orientation

<unit> is the splits.txt name, e.g. Pack/RPParty/RPHkyScene/RPHkyField.cpp. Several agents can
work at once: everything that touches the shared build (ninja, symbols.txt) runs under a lock.

sync pairs the source object's functions with the target's in order (with -inline deferred,
CodeWarrior emits a file's functions last-defined first, so write the source in natural order:
it then lines up with the binary in reverse). Every function of the unit must be defined, stubs
for unfinished ones. Where a function has the same size on both sides, relocations at the same
offset name the same callee or data, so those target symbols take the source names too (literal
pool entries keep their labels; symbols that already carry a real mangled name are only
reported). Renames go to dtk/source_names/<unit>.tsv, applied by tools/apply_source_names.py.
"""
import contextlib
import csv
import json
import os
import re
import subprocess
import sys
import tempfile
import threading
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from elf_symbols import relocations, symbols  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
PY = sys.executable
FIELDS = ["address", "old_name", "new_name", "unit", "previous"]


def ninja_exe():
    exe = os.environ.get("NINJA") or str(Path(PY).parent / "Scripts" / "ninja.exe")
    return exe if Path(exe).exists() else "ninja"


STALE = 120  # seconds without a heartbeat after which a lock's holder is taken to be dead


def heartbeat(lock, stop):
    """keep the lock's mtime fresh while it is held. Liveness cannot be checked by PID: agents
    run in sandboxes as other Windows users, whose tasklist does not show our processes."""
    while not stop.wait(10):
        try:
            os.utime(lock)
        except OSError:
            return


@contextlib.contextmanager
def build_lock(what):
    """one agent at a time in the shared build directory"""
    lock = ROOT / "build/.unit.lock"
    lock.parent.mkdir(parents=True, exist_ok=True)
    waited = 0
    while True:
        try:
            fd = os.open(lock, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
            os.write(fd, f"{os.getpid()} {what} {time.ctime()}".encode())
            os.close(fd)
            break
        except FileExistsError:
            try:
                if time.time() - lock.stat().st_mtime > STALE:  # no heartbeat: the holder died
                    lock.unlink(missing_ok=True)
                    continue
                if waited % 60 == 0:
                    print(f"  waiting for the build lock ({lock.read_text(errors='replace').strip()})", flush=True)
            except FileNotFoundError:  # released meanwhile
                continue
            time.sleep(2)
            waited += 2
    stop = threading.Event()
    beat = threading.Thread(target=heartbeat, args=(lock, stop), daemon=True)
    beat.start()
    try:
        # a check that was killed mid-link (an agent's command timeout) left its unit listed in
        # matching.txt; put the list back before anything else builds
        pending = ROOT / "build/.check_restore"
        if pending.exists():
            print("  restoring config/RHAE01/matching.txt after an interrupted check", flush=True)
            (ROOT / "config/RHAE01/matching.txt").write_text(pending.read_text(encoding="utf-8"),
                                                             encoding="utf-8", newline="\n")
            subprocess.run([PY, str(ROOT / "configure.py")], cwd=ROOT, capture_output=True)
            pending.unlink()
        yield
    finally:
        # stop the heartbeat first: Windows will not delete a file another thread is touching
        stop.set()
        beat.join()
        for _ in range(20):
            try:
                lock.unlink(missing_ok=True)
                break
            except PermissionError:  # another process's virus scan or stat; retry briefly
                time.sleep(0.1)


def paths(unit):
    stem = Path(unit).with_suffix(".o").as_posix()
    return ROOT / "build/RHAE01/src" / stem, ROOT / "build/RHAE01/obj" / stem


def ninja(*targets):
    # a source file created since configure.py last ran has no build rule yet
    rules = (ROOT / "build.ninja").read_text(encoding="utf-8", errors="replace") if targets else ""
    if any(t.replace("/", "\\") not in rules for t in targets):
        subprocess.run([PY, str(ROOT / "configure.py")], cwd=ROOT, capture_output=True)  # not -I: it imports tools.project
    r = subprocess.run([ninja_exe(), *targets], cwd=ROOT, capture_output=True, text=True)
    if r.returncode != 0:
        out = "\n".join(l for l in r.stdout.splitlines() if not l.startswith("build\\tools\\sjiswrap"))
        print(out[-4000:], r.stderr[-1000:])
    return r.returncode == 0


def compile_unit(unit):
    base, target = paths(unit)
    with build_lock(f"compile {unit}"):
        return ninja(base.relative_to(ROOT).as_posix(), target.relative_to(ROOT).as_posix())


def objdiff(unit, symbol=None):
    name = "main/" + Path(unit).with_suffix("").as_posix()
    with tempfile.TemporaryDirectory() as tmp:
        out = Path(tmp) / "diff.json"
        cmd = [str(ROOT / ("build/tools/objdiff-cli" + (".exe" if os.name == "nt" else ""))), "diff", "-p", str(ROOT), "-u", name, "-o", str(out),
               "--format", "json", "-c", "functionRelocDiffs=data_value"]  # literals compare by value
        r = subprocess.run(cmd + ([symbol] if symbol else []), capture_output=True, text=True)
        if r.returncode != 0 or not out.exists():
            print(r.stdout[-2000:], r.stderr[-2000:])
            return None
        return json.loads(out.read_text(encoding="utf-8"))


def status(unit):
    if not compile_unit(unit):
        return 1
    d = objdiff(unit)
    if d is None:
        return 1
    rows = [s for s in d["left"]["symbols"] if "match_percent" in s or s.get("size")]
    done = total = 0
    print(f"{unit}")
    for s in rows:
        pct = s.get("match_percent")
        size = int(s.get("size", 0) or 0)
        if not size or s["name"].startswith("["):
            continue
        total += size
        done += size if pct == 100.0 else 0
        mark = "OK " if pct == 100.0 else "   " if pct is None else "~  "
        print(f"  {mark}{(pct or 0):6.2f}%  {size:5X}  {s.get('demangled_name') or s['name']}")
    print(f"  {done}/{total} bytes in fully matching symbols")
    return 0


def diff(unit, symbol, show_all=False):
    if not compile_unit(unit):
        return 1
    d = objdiff(unit, symbol)
    if d is None:
        return 1

    def rows(side):
        for s in side["symbols"]:
            if s["name"] == symbol:
                return s.get("instructions", []), s.get("match_percent")
        return None, None
    left, pct = rows(d["left"])
    right, _ = rows(d["right"])
    if left is None:
        print(f"{symbol}: not in the target object (run unit.py sync first?)")
        return 1
    right = right or []
    print(f"{symbol}: {pct or 0:.2f}% match")
    marks = []
    for k in range(max(len(left), len(right))):
        a = left[k] if k < len(left) else {}
        b = right[k] if k < len(right) else {}
        kind = a.get("diff_kind") or b.get("diff_kind")
        mark = " " if not kind else "<" if kind == "DIFF_DELETE" else ">" if kind == "DIFF_INSERT" else "|"
        marks.append((mark, a.get("instruction", {}).get("formatted", ""), b.get("instruction", {}).get("formatted", "")))
    near = {j for k, (m, _, _) in enumerate(marks) if m != " " for j in range(k - 2, k + 3)}
    for k, (m, ta, tb) in enumerate(marks):
        if show_all or k in near:
            print(f"{k * 4:5X} {ta:44.44} {m} {tb}")
    return 0


def _norm(ins):
    """An instruction with relocation operands masked: equal when only the target's name differs."""
    out = []
    for p in ins.get("instruction", {}).get("parts", []):
        if "opcode" in p:
            out.append(p["opcode"].get("mnemonic"))
        elif "arg" in p:
            out.append("<reloc>" if p["arg"].get("reloc") else json.dumps(p["arg"], sort_keys=True))
        elif "basic" in p:
            out.append(p["basic"])
    return tuple(out)


def cmp(unit):
    """Per function: real instruction differences, ignoring relocation names (lbl_X vs @123,
    Packet_8040EE80 vs Packet), plus a function-order check against the original object."""
    if not compile_unit(unit):
        return 1
    d = objdiff(unit)
    if d is None:
        return 1
    base, target = paths(unit)
    print(f"{unit}")
    real_total = 0
    for s in d["left"]["symbols"]:
        pct = s.get("match_percent")
        if pct is None or pct == 100.0 or s["name"].startswith("[") or not int(s.get("size", 0) or 0):
            continue
        one = objdiff(unit, s["name"])
        rows = {}
        for side in ("left", "right"):
            for t in (one or {}).get(side, {}).get("symbols", []):
                if t["name"] == s["name"] and t.get("instructions"):
                    rows[side] = t["instructions"]
        if "left" not in rows:  # data object: objdiff gives no instruction rows
            continue
        left, right = rows["left"], rows.get("right", [])
        real = names = 0
        for k in range(max(len(left), len(right))):
            a = left[k] if k < len(left) else {}
            b = right[k] if k < len(right) else {}
            if not (a.get("diff_kind") or b.get("diff_kind")):
                continue
            if a and b and _norm(a) == _norm(b):
                names += 1
            else:
                real += 1
        real_total += real
        name = s.get("demangled_name") or s["name"]
        if real:
            print(f"  ~  {pct:6.2f}%  {real} real difference(s)" + (f", {names} name-only" if names else "") + f"  {name}")
        else:
            print(f"  OK {pct:6.2f}%  identical except {names} relocation name(s)  {name}")
    # function order: the linked object must follow the original's order
    tgt = [f["name"] for f in funcs(target)]
    src = [f["name"] for f in funcs(base, source=True) if not f.get("optional")]
    common = [n for n in src if n in set(tgt)]
    want = [n for n in tgt if n in set(common)]
    if common != want:
        print("  ORDER: functions are not in the original's order" +
              (" (exactly reversed: this compiler emits in reverse source order)" if common == want[::-1] else ""))
        print("    original order: " + ", ".join(want))
    emitted = {f["name"] for f in funcs(base, source=True)}  # weak copies count as present
    missing = [n for n in tgt if n not in emitted]
    if missing:
        print("  missing from source: " + ", ".join(missing))
    print(f"  {real_total} real instruction difference(s) in total" if real_total else "  code identical up to relocation names")
    return 0


def funcs(path, source=False):
    """.text functions in address order. In our object (source=True), local unmangled functions
    are the SDK headers' static inlines and @unnamed@ locals are header code: both dropped. Weak
    functions (header inlines, template instances) and compiler thunks (@16@...) are marked
    optional: the original unit holds the copy the linker kept, or the copy lives elsewhere."""
    out = []
    for s in symbols(path):
        if s["type"] != "func" or s["section"] != ".text":
            continue
        if source:
            if "@unnamed@" in s["name"] or (s["bind"] == "local" and "__" not in s["name"][1:]
                                            and not s["name"].startswith(("@", "__sinit"))):
                continue
            s["optional"] = s["bind"] == "weak" or s["name"].startswith("@")
        out.append(s)
    return sorted(out, key=lambda s: s["value"])


def align(tgt, src):
    """pair every target function with a source function, in order; optional source functions
    may stay unpaired. Minimises size differences (same name: free). None if impossible."""
    INF = float("inf")
    n, m = len(tgt), len(src)
    cost = [[INF] * (m + 1) for _ in range(n + 1)]
    back = [[None] * (m + 1) for _ in range(n + 1)]
    cost[0][0] = 0
    for i in range(n + 1):
        for j in range(m + 1):
            c = cost[i][j]
            if c == INF:
                continue
            if j < m and src[j]["optional"] and c + 1 < cost[i][j + 1]:  # leave src j out
                cost[i][j + 1], back[i][j + 1] = c + 1, "skip"
            if i < n and j < m:
                t, s = tgt[i], src[j]
                d = 0 if t["name"] == s["name"] else 2 + min(abs(t["size"] - s["size"]), 4096) / 16
                if c + d < cost[i + 1][j + 1]:
                    cost[i + 1][j + 1], back[i + 1][j + 1] = c + d, "pair"
    if cost[n][m] == INF:
        return None
    pairs, i, j = [], n, m
    while i or j:
        if back[i][j] == "pair":
            pairs.append((tgt[i - 1], src[j - 1]))
            i, j = i - 1, j - 1
        else:
            j -= 1
    return pairs[::-1]


def ledger_path(unit):
    return ROOT / "dtk/source_names" / (unit.replace("/", "__") + ".tsv")


def linked_sources():
    """(unit, source text) of every unit linked from source (config/RHAE01/matching.txt)"""
    out = []
    for l in (ROOT / "config/RHAE01/matching.txt").read_text(encoding="utf-8").splitlines():
        l = l.strip()
        p = ROOT / "src" / l
        if l and not l.startswith("#") and p.exists():
            out.append((l, p.read_text(encoding="utf-8", errors="replace")))
    return out


def sync(unit):
    base, target = paths(unit)
    with build_lock(f"sync {unit}"):
        if not ninja(base.relative_to(ROOT).as_posix(), target.relative_to(ROOT).as_posix()):
            return 1
        src, tgt = funcs(base, source=True), funcs(target)
        pairs = align(tgt, src)
        if pairs is None:
            print(f"{unit}: cannot pair the original's {len(tgt)} functions with your "
                  f"{sum(1 for x in src if not x['optional'])} (+{sum(1 for x in src if x['optional'])} optional). "
                  f"Define every function of the unit (stubs for unfinished ones), in reverse address order:")
            for k in range(max(len(src), len(tgt))):
                a = tgt[k] if k < len(tgt) else None
                b = src[k] if k < len(src) else None
                print(f"  {k:3} {a['name'] if a else '-':50.50} {a['size'] if a else 0:6X}   "
                      f"{b['name'] if b else '-':50.50} {b['size'] if b else 0:6X}{' (optional)' if b and b['optional'] else ''}")
            return 1
        addr_of = {}
        for line in open(ROOT / "config/RHAE01/symbols.txt", encoding="utf-8"):
            m = re.match(r"^(\S+) = \.\w+:0x([0-9A-F]+);", line)
            if m:
                addr_of.setdefault(m.group(1), m.group(2))
        path = ledger_path(unit)
        rows = {}
        if path.exists():
            rows = {r["address"]: r for r in csv.DictReader(open(path, encoding="utf-8"), delimiter="\t")}

        def record(addr, old, new):
            # keep the first name seen at an address, so the ledger applies to a fresh symbols.txt,
            # and the names given since, so a rename after a signature change applies too.
            # A name another address already has is a mispairing (inline copies, swapped
            # registers): never write it, it would end up applied to the wrong address.
            elsewhere = addr_of.get(new)
            if (elsewhere and elsewhere != addr) or any(
                    r["new_name"] == new and a != addr for a, r in rows.items()):
                print(f"  check: {new} is already the name at another address; {addr} not renamed")
                return False
            # a linked unit that calls the function by its placeholder name would stop linking
            users = [u for u, text in linked_sources() if u != unit and re.search(rf"\b{re.escape(old)}\b", text)]
            if users:
                print(f"  check: {old} is used by name in linked {', '.join(users)}; {addr} not renamed "
                      f"(ask the main session to rename it there first)")
                return False
            if addr in rows:
                r = rows[addr]
                prev = [p for p in r.get("previous", "").split(";") if p]
                if r["new_name"] not in prev and r["new_name"] != new:
                    prev.append(r["new_name"])
                rows[addr] = {"address": addr, "old_name": r["old_name"], "new_name": new, "unit": unit,
                              "previous": ";".join(prev)}
            else:
                rows[addr] = {"address": addr, "old_name": old, "new_name": new, "unit": unit, "previous": ""}
            return True
        # Guards: a badly aligned unit (the boundary is off, a function is missing) pairs functions
        # of very different sizes; renaming from that writes wrong names into the shared symbols.
        # Stubs are small, so only pairs where the source function is real count against it.
        def far_off(t, s):
            return s["size"] > 0x10 and abs(t["size"] - s["size"]) > max(8, t["size"] // 4)
        bad = [(t, s) for t, s in pairs if far_off(t, s)]
        if len(bad) > max(1, len(pairs) // 4) and "--force" not in sys.argv:
            print(f"{unit}: {len(bad)} of {len(pairs)} paired functions differ a lot in size; the unit is "
                  f"probably misaligned (wrong boundary, missing or extra function). Nothing renamed:")
            for t, s in bad[:15]:
                print(f"  {t['name']:50.50} {t['size']:6X}   {s['name']:50.50} {s['size']:6X}")
            print("  (unit.py sync <unit> --force renames anyway)")
            return 1
        owned = {r["new_name"] for r in rows.values()}
        changed = 0
        for t, s in pairs:
            if t["name"] != s["name"] and t["name"] in addr_of:
                real = "__" in t["name"][1:] and not t["name"].startswith(("lbl_", "fn_"))
                if real and t["name"] not in owned:
                    # a real (mangled) name this unit did not give: never overwrite it blindly
                    print(f"  check: {t['name']} has a real name; your source calls it {s['name']}")
                    continue
                far = "   <- sizes differ a lot" if far_off(t, s) else ""
                if record(addr_of[t["name"]], t["name"], s["name"]):
                    print(f"  {addr_of[t['name']]} {t['name']} -> {s['name']}{far}")
                    changed += 1
        t_rel = [r for r in relocations(target) if r["section"] == ".text"]
        s_rel = [r for r in relocations(base) if r["section"] == ".text"]
        seen = set()
        for t, s in pairs:
            if t["size"] != s["size"]:
                continue
            tr = {r["offset"] - t["value"]: r for r in t_rel if t["value"] <= r["offset"] < t["value"] + t["size"]}
            sr = {r["offset"] - s["value"]: r for r in s_rel if s["value"] <= r["offset"] < s["value"] + s["size"]}
            for off, a in tr.items():
                b = sr.get(off)
                if not b or not a["symbol"] or not b["symbol"] or a["symbol"] == b["symbol"]:
                    continue
                if b["symbol"].startswith(("@", ".")):
                    continue
                seen.add((a["symbol"], b["symbol"], s["name"]))
        # only one-to-one pairings: a source symbol seen against two originals (or the reverse)
        # comes from code that does not match yet, and naming from it is a guess
        # and only same-size objects: a 0x3C table named after a 0xC vector is a mispairing
        size_t = {x["name"]: x["size"] for x in symbols(target)}
        size_s = {x["name"]: x["size"] for x in symbols(base)}
        by_a, by_b = {}, {}
        for a, b, _ in seen:
            by_a.setdefault(a, set()).add(b)
            by_b.setdefault(b, set()).add(a)
        for a, b, where in sorted(seen):
            if len(by_a[a]) > 1 or len(by_b[b]) > 1:
                print(f"  check: {b} pairs with {' and '.join(sorted(by_b[b]))}; ambiguous, not renamed")
                continue
            if size_t.get(a) and size_s.get(b) and size_t[a] != size_s[b]:
                print(f"  check: {b} (size {size_s[b]:#x}) pairs with {a} (size {size_t[a]:#x}); not renamed")
                continue
            addr = addr_of.get(a)
            if addr is None or any(x == a and y == b for x, y, w in seen if w < where):
                continue
            if "__" in a[1:] and not a.startswith(("lbl_", "fn_")):
                print(f"  check: {where} refers to {b} where the original has {a}")
                continue
            if record(addr, a, b):
                print(f"  {addr} {a} -> {b} (referenced)")
                changed += 1
        if changed:
            path.parent.mkdir(parents=True, exist_ok=True)
            with open(path, "w", encoding="utf-8", newline="\n") as f:
                f.write("\t".join(FIELDS) + "\n")
                for a in sorted(rows):
                    f.write("\t".join(rows[a].get(k) or "" for k in FIELDS) + "\n")
            r = subprocess.run([PY, "-I", str(ROOT / "tools/apply_source_names.py"), str(ROOT)],
                               capture_output=True, text=True)
            print("  " + r.stdout.strip().replace("\n", "\n  "))
            print("  rebuilding (dtk split + objects)...", flush=True)
            if not ninja():
                return 1
        print(f"{unit}: {len(tgt)} functions, {changed} renames")
    return 0


def check(*units):
    """link the units from source (config/RHAE01/matching.txt) and keep them there only if the DOL
    still matches byte for byte. Units that depend on each other's data are checked together."""
    listing = ROOT / "config/RHAE01/matching.txt"
    # configure.py only recognises entries with the source extension: a bare unit name links the
    # original object and "matches" trivially
    units = tuple(u if u.endswith((".c", ".cpp")) else u + ".cpp" for u in units)
    unit = " + ".join(units)
    with build_lock(f"check {unit}"):
        # read the list under the lock: another agent's check may have changed it meanwhile, and a
        # failed check restores exactly this text
        before = listing.read_text(encoding="utf-8")
        names = {l.strip() for l in before.splitlines() if l.strip() and not l.startswith("#")}
        new = [u for u in units if u not in names]
        if not new:
            print(f"{unit}: already linked from source")
            return 0
        unit = " + ".join(new)
        print(f"  linking the whole DOL with {unit} from source: this takes several minutes", flush=True)
        pending = ROOT / "build/.check_restore"
        pending.write_text(before, encoding="utf-8", newline="\n")
        listing.write_text(before.rstrip("\n") + "\n" + "\n".join(new) + "\n", encoding="utf-8", newline="\n")
        subprocess.run([PY, str(ROOT / "configure.py")], cwd=ROOT, capture_output=True)  # not -I: it imports tools.project
        # only the DOL and its hash: a full build would also fail on another agent's unlinked,
        # half-edited source, which says nothing about this unit
        r = subprocess.run([ninja_exe(), "build/RHAE01/main.dol"], cwd=ROOT, capture_output=True, text=True)
        if r.returncode == 0:
            h = subprocess.run([str(ROOT / ("build/tools/dtk" + (".exe" if os.name == "nt" else ""))), "shasum", "-c", "config/RHAE01/build.sha1"],
                               cwd=ROOT, capture_output=True, text=True)
            r.stdout += re.sub(r"\x1b\[[0-9;]*m", "", h.stdout + h.stderr)  # dtk colours "OK"
            r.returncode = h.returncode
        ok = r.returncode == 0 and "main.dol: OK" in r.stdout
        if ok:
            pending.unlink(missing_ok=True)
        else:
            listing.write_text(before, encoding="utf-8", newline="\n")
            subprocess.run([PY, str(ROOT / "configure.py")], cwd=ROOT, capture_output=True)  # not -I: it imports tools.project
            pending.unlink(missing_ok=True)
            subprocess.run([ninja_exe()], cwd=ROOT, capture_output=True)
            out = r.stdout.splitlines()
            tail = []
            for k, l in enumerate(out):  # each linker error with the two lines that say what failed
                if "Linker Error" in l or "FAILED" in l or "main.dol" in l:
                    tail += [l] + [x for x in out[k + 1:k + 3] if x.startswith("#")]
            print(f"{unit}: the DOL does not match when linked from source; left as NonMatching")
            for l in tail[:24]:
                print("  " + l)
            return 1
    print(f"{unit}: linked from source, DOL matches; added to config/RHAE01/matching.txt")
    return 0


def lib():
    for l in json.loads((ROOT / "config/RHAE01/units.json").read_text(encoding="utf-8")):
        print(f"{l['lib']}: {len(l['objects'])} units")
    return 0


def main():
    a = sys.argv[1:]
    if not a:
        print(__doc__)
        return 1
    if a[0] == "status" and len(a) == 2:
        return status(a[1])
    if a[0] == "cmp" and len(a) == 2:
        return cmp(a[1])
    if a[0] == "diff" and len(a) >= 3:
        return diff(a[1], a[2], "--all" in a)
    if a[0] == "sync" and len(a) >= 2:
        return sync(a[1])
    if a[0] == "check" and len(a) >= 2:
        return check(*a[1:])
    if a[0] == "lib":
        return lib()
    print(__doc__)
    return 1


if __name__ == "__main__":
    sys.exit(main())
