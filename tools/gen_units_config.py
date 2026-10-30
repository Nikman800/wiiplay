"""Generate config/RHAE01/units.json: every named source-file unit in splits.txt, grouped
into libraries with progress categories, for configure.py to register as NonMatching objects.

Units already listed by hand in configure.py are skipped there; minigames get one library each.
dtk's auto_* units are not listed.

Usage: gen_units_config.py <root>
"""
import collections
import json
import re
import sys
from pathlib import Path

UNIT = re.compile(r"^(?!Sections:)(\S.*):\s*$")

# top-level path -> (library name, progress categories)
GROUPS = {
    "revolution": ("RVL SDK", ["sdk", "rvl"]),
    "rvl": ("RVL SDK", ["sdk", "rvl"]),
    "nw4r": ("NintendoWare", ["sdk", "nw4r"]),
    "egg": ("EGG", ["sdk", "egg"]),
    "RVLFaceLib": ("RVLFaceLib", ["sdk", "rfl"]),
    "homebuttonMiniLib": ("HomeButtonMiniLib", ["sdk", "hbm"]),
    "MSL": ("MSL", ["sdk", "runtime"]),
    "MSL_C": ("MSL", ["sdk", "runtime"]),
    "runtime": ("Runtime", ["sdk", "runtime"]),
    "MetroTRK": ("MetroTRK", ["sdk", "runtime"]),
    "Pack": ("RP framework", ["game"]),
    "main.cpp": ("RP framework", ["game"]),
}


MINIGAMES = {"Fsh": "Fishing", "Hky": "Laser Hockey", "Pnp": "Table Tennis", "Duc": "Shooting Range",
             "Tnk": "Tanks", "Wly": "Find Mii", "Cow": "Charge", "Bil": "Billiards", "Bom": "Pose Mii"}


def main():
    root = Path(sys.argv[1])
    units = [m.group(1) for m in map(UNIT.match, (root / "config/RHAE01/splits.txt").read_text(encoding="utf-8").splitlines())
             if m and not m.group(1).startswith("auto_")]
    libs = collections.OrderedDict()
    for u in units:
        top = u.split("/", 1)[0]
        name, cats = GROUPS.get(top, ("Other", ["sdk"]))
        game = re.match(r"^Pack/RPParty/RP(\w{3})Scene/", u)
        if game:  # one library per minigame, so progress reports show each game
            name, cats = MINIGAMES[game.group(1)], ["game", "minigame"]
        elif top in ("Pack", "main.cpp"):
            cats = ["game", "framework"]
        libs.setdefault(name, {"lib": name, "progress_category": cats, "objects": []})["objects"].append(u)
    out = root / "config/RHAE01/units.json"
    out.write_text(json.dumps(list(libs.values()), indent=1) + "\n", encoding="utf-8", newline="\n")
    print(", ".join(f"{k}: {len(v['objects'])}" for k, v in libs.items()), "->", out)


if __name__ == "__main__":
    main()
