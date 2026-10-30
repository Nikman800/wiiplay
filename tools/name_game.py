"""Evidence-based module/class naming for Wii Play game code (and EGG files).

Reads dtk/index.json, writes dtk/names.tsv:
  address  old_name  new_name  module  confidence  evidence

Confidence:
  anchor     function references a string unique to the module
  bounded    lies between two anchors of the same module (link-order contiguity)
  callgraph  >= 2/3 of call-graph neighbours agree on the module
  position   nearest anchor by address (weakest)
"""
import collections
import json
import re
import sys
from pathlib import Path

GAME_START = 0x80182E90
GAME_END = 0x802E9CC0
EGG_START, EGG_END = 0x80083000, 0x800B1BD4

# Minigame anchors (lowercased string search). Order irrelevant; ranges don't overlap.
MINIGAMES = {
    "Fsh": r"fsh|fishing|rp_fsh",
    "Hky": r"hky|hck_|airhockey|basic_hokey",
    "Pnp": r"\bpnp|pin_|pingpong|rallylevel",
    "Duc": r"\bduc|sht_|duckhunt|basic_shooting",
    "Tnk": r"tnk|tank\.arc|\btank|basic_tank",
    "Wly": r"wly|wtd|wally|wntscene|basic_wanted",
    "Cow": r"\bcow|rp_cow",
    "Bil": r"\bbil_|bad_|billiard|rp_bil",
    "Bom": r"\bbom|poseman|ira_pose|basic_bomb",
}
# Framework subsystem anchors, checked only inside the framework range.
FRAMEWORK = [
    ("Mpe", r"rpmpescene"),
    ("HomeMenu", r"/homemenu/"),
    ("Health", r"strap\.arc|rp_health_sound"),
    ("Mii", r"kokeshi|clothes\.brres"),
    ("Party", r"\bpt[a-z0-9]+_0|medal_|n_newrec|party_doll|leadcursor|party_message|bsm_|balloon_open|ptpause|pttitle"),
    ("Menu", r"menucommon|menu_bg|n_frameup|rimoconentry|playerselect|seq_choice_mii|nunchukcheck|pt_menu|p_lamp|playadvice|logo_model|common_message"),
    ("Lyt", r"messagewindow|def_cursor|pause_00|systemwindow|abutton|abpress|b_backbutton|ccursorcircle|p_cpadcutl"),
    ("Gfx", r"scnrfl|pmmisc|\bpblm|rpgrpmodelscene|sports_common\.breff|dummy_tex"),
    ("Snd", r"rpsndutility\.cpp|\bsymb\b|rp_static|rp_common_party\.brsar"),
    ("Sys", r"gameconfig|rootscene|timestamp\.txt|banner\.bin|sportsstatic|now loading|se_pause"),
]
GENERIC = re.compile(r"\.h$|rpsnd|wrong sound name|party_message\.bmg|^item$|^dummy|index<|smcommon|"
                     r"cam_num|getnum|not for this class|eggibinary|^rootpane$|scenefile|^rpscene$|"
                     r"^[^a-z]*$|%", re.I)


def module_of_strings(strings, table):
    blob = " ".join(strings).lower()
    found = [m for m, p in table if re.search(p, blob)]
    return found[0] if len(found) == 1 else None


def camel(token):
    token = re.sub(r"\.(brres|brlyt|brlan|arc|bin|breff|breft|brsar|bmg|plight|plmap|pblm|kcl|pmp)$", "", token)
    token = token.strip("/").split("/")[-1]
    parts = re.split(r"[^A-Za-z0-9]+", token)
    out = "".join(p[:1].upper() + p[1:] for p in parts if p)
    return out[:32]


def best_hint(strings, freq):
    cands = [s for s in strings if not GENERIC.search(s) and re.search(r"[A-Za-z]{3}", s) and len(s) <= 48]
    if not cands:
        return None, None
    s = min(cands, key=lambda x: (freq[x], len(x)))
    return camel(s), s


def main():
    root = Path(sys.argv[1])
    idx = json.loads((root / "dtk/index.json").read_text(encoding="utf-8"))
    F = idx["functions"]
    fa = {n: f["addr"] for n, f in F.items() if f["addr"]}
    lits = json.loads((root / "dtk/literals.json").read_text(encoding="utf-8"))
    own = {n: lits.get(f"{f['addr']:08X}", []) for n, f in F.items() if f["addr"]}
    freq = collections.Counter(s for v in own.values() for s in set(v))

    game = sorted((f for f in F.values() if f["addr"] and GAME_START <= f["addr"] < GAME_END), key=lambda f: f["addr"])
    mg_table = list(MINIGAMES.items())
    anchors = {}
    for f in game:
        m = module_of_strings(f["strings"], mg_table)
        if m:
            anchors[f["name"]] = m
    mg_first = min((fa[n] for n, m in anchors.items() if fa[n] > 0x801F0000), default=GAME_END)
    for f in game:
        if f["addr"] < mg_first and f["name"] not in anchors:
            m = module_of_strings(f["strings"], FRAMEWORK)
            if m:
                anchors[f["name"]] = m
        elif f["addr"] < mg_first and anchors.get(f["name"]) in MINIGAMES:
            del anchors[f["name"]]  # framework code referencing a minigame asset
    module, conf, evid = {}, {}, {}
    for n, m in anchors.items():
        module[n], conf[n], evid[n] = m, "anchor", "anchor-string"
    # between two same-module anchors
    order = [f["name"] for f in game]
    pos = {n: i for i, n in enumerate(order)}
    anchor_pos = sorted(pos[n] for n in anchors)
    for a, b in zip(anchor_pos, anchor_pos[1:]):
        ma, mb = anchors[order[a]], anchors[order[b]]
        if ma == mb:
            for i in range(a + 1, b):
                n = order[i]
                if n not in module:
                    module[n], conf[n], evid[n] = ma, "bounded", f"between {ma} anchors {order[a]}..{order[b]}"
    # call-graph propagation
    changed = True
    while changed:
        changed = False
        for n in order:
            if n in module:
                continue
            nb = [module[x] for x in F[n]["callers"] + F[n]["callees"] if x in module]
            if len(nb) >= 2:
                m, c = collections.Counter(nb).most_common(1)[0]
                if c * 3 >= len(nb) * 2:
                    module[n], conf[n], evid[n] = m, "callgraph", f"call-graph {c}/{len(nb)} neighbours"
                    changed = True
    # positional fallback
    for i, n in enumerate(order):
        if n in module:
            continue
        prev = next((order[j] for j in range(i - 1, -1, -1) if order[j] in anchors), None)
        nxt = next((order[j] for j in range(i + 1, len(order)) if order[j] in anchors), None)
        if fa[n] < mg_first:
            m = "RP"
        else:
            cand = [x for x in (prev, nxt) if x]
            m = anchors[min(cand, key=lambda x: abs(fa[x] - fa[n]))] if cand else "RP"
        module[n], conf[n], evid[n] = m, "position", "address position"

    # weakly placed vtable methods follow the module their class's own (non-inherited)
    # methods agree on; entries shared with other vtables are inherited and ignored
    vt_count = collections.Counter(e for v in idx["vtables"] for e in set(v["entries"]))
    for v in idx["vtables"]:
        if not (v["addr"] and v["entries"][:2] == ["0x00000000", "0x00000000"]):
            continue
        members = [e for e in v["entries"] if e in module and vt_count[e] == 1]
        strong = [module[e] for e in members if conf[e] in ("anchor", "bounded")]
        if not strong:
            continue
        m, c = collections.Counter(strong).most_common(1)[0]
        if c * 3 < len(strong) * 2:
            continue
        for e in members:
            if conf[e] in ("position", "callgraph") and module[e] != m:
                module[e], conf[e], evid[e] = m, "class", f"class {v['name']}: {c}/{len(strong)} strong members in {m}"

    # classes from vtables (most-base vtable containing the function)
    vt_of = {}
    idx["vtables"] = [v for v in idx["vtables"] if v["addr"] and v["entries"][:2] == ["0x00000000", "0x00000000"]]
    for v in idx["vtables"]:
        ents = v["entries"]
        for slot, e in enumerate(ents):
            if e in F and F[e]["addr"] and GAME_START <= F[e]["addr"] < GAME_END:
                prev = vt_of.get(e)
                if prev is None or len(ents) < len(prev[0]["entries"]):
                    vt_of[e] = (v, slot)
    class_name = {}
    for v in idx["vtables"]:
        members = [e for e in v["entries"] if e in F] + [r for r in v["referenced_by"] if r in F]
        strs = [s for e in members for s in own.get(e, [])]
        hint, _ = best_hint(strs, freq)
        class_name[v["name"]] = (hint or "Cls") + f"_{v['addr']:08X}"

    rows = []
    used = set()
    for f in game:
        n = f["name"]
        m = module[n]
        hint, src = best_hint(own.get(n, []), freq)
        ev = evid[n]
        if n in vt_of:
            v, slot = vt_of[n]
            base = f"{m}_{class_name[v['name']]}_vf{slot - 2:02d}"
            ev += f"; vtable {v['name']} slot {slot}"
            new = base
        elif any(r in class_name for r in f["refs"]) and not hint:
            vt = next(r for r in f["refs"] if r in class_name)
            new = f"{m}_{class_name[vt]}_init_{f['addr']:08X}"
            ev += f"; stores/reads vtable {vt}"
        elif hint:
            new = f"{m}_{hint}_{f['addr']:08X}"
            ev += f"; literal '{src}'"
        else:
            new = f"{m}_{f['addr']:08X}"
        if new in used:
            new += f"_{f['addr']:08X}"
        used.add(new)
        rows.append((f["addr"], n, new, m, conf[n], ev))

    # EGG: functions between two anchors of the same egg*.cpp file
    egg = sorted((f for f in F.values() if f["addr"] and EGG_START <= f["addr"] < EGG_END and f["name"].startswith("fn_")),
                 key=lambda f: f["addr"])
    def egg_file(f):
        fs = {m for s in f["strings"] for m in re.findall(r"(egg[A-Za-z0-9]+)\.cpp", s)}
        return fs.pop() if len(fs) == 1 else None
    tags = [(i, egg_file(f)) for i, f in enumerate(egg)]
    tags = [(i, t) for i, t in tags if t]
    egg_mod = {}
    for i, t in tags:
        egg_mod[i] = (t, "anchor", "assert file string")
    for (a, ta), (b, tb) in zip(tags, tags[1:]):
        if ta == tb:
            for i in range(a + 1, b):
                egg_mod.setdefault(i, (ta, "bounded", f"between {ta}.cpp anchors"))
    for i, (t, c, e) in sorted(egg_mod.items()):
        f = egg[i]
        rows.append((f["addr"], f["name"], f"{t}_{f['addr']:08X}", t, c, e))

    rows.sort()
    with open(root / "dtk/names.tsv", "w", encoding="utf-8") as out:
        out.write("address\told_name\tnew_name\tmodule\tconfidence\tevidence\n")
        for a, o, nn, m, c, e in rows:
            out.write(f"{a:08X}\t{o}\t{nn}\t{m}\t{c}\t{e}\n")
    by = collections.Counter((r[3] if not r[3].startswith("egg") else "egg*", r[4]) for r in rows)
    mods = sorted({k[0] for k in by})
    for m in mods:
        print(f"{m:9} " + "  ".join(f"{c}={by[(m, c)]}" for c in ("anchor", "bounded", "class", "callgraph", "position") if by[(m, c)]))
    print(len(rows), "names;", sum(1 for r in rows if "_vf" in r[2]), "virtual methods;",
          sum(1 for r in rows if re.search(r"_[A-Z][A-Za-z0-9]+_[0-9A-F]{8}$", r[2]) and "_vf" not in r[2]), "with string/class hints")


if __name__ == "__main__":
    main()
