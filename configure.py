#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import json
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "RHAE01",  # 0: Wii Play (USA) Rev 1
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--ninja",
    metavar="BINARY",
    type=Path,
    help="path to ninja binary (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--warn",
    dest="warn",
    type=str,
    choices=["all", "off", "error"],
    help="how to handle warnings",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.ninja_path = args.ninja
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-2"
config.compilers_tag = "20251118"
config.dtk_tag = "v1.8.4"
config.objdiff_tag = "v3.6.1"
config.sjiswrap_tag = "v1.2.2"
config.wibo_tag = "1.0.3"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym BUILD_VERSION={version_num}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    config.ldflags.append("-mapunused")
    # config.ldflags.append("-listclosure") # For Wii linkers

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = None

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    "-O4,p",
    "-inline auto",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-fp_contract on",
    "-str reuse",
    "-multibyte",  # For Wii compilers, replace with `-enc SJIS`
    "-i include",
    f"-i build/{config.version}/include",
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION_{config.version}",
]

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Warning flags
if args.warn == "all":
    cflags_base.append("-W all")
elif args.warn == "off":
    cflags_base.append("-W off")
elif args.warn == "error":
    cflags_base.append("-W error")

# Metrowerks library flags
cflags_runtime = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
    "-inline auto",
]

# REL flags
cflags_rel = [
    *cflags_base,
    "-sdata 0",
    "-sdata2 0",
]

# Game code flags (verified by byte-matching Laser Hockey functions; game is C++)
# RP game code: the flags doldecomp/ogws uses for Wii Sports' RP code, with PACK_PARTY. Wii Play's
# symbols name the precompiled header SportsPack.pch (@unnamed@SportsPack_pch@), as in Wii Sports.
# NintendoWare (nw4r) units, as in the Rhae decomp (github.com/akiramusic000/Rhae, CC0): SJIS
# sources, no FMA contraction, file-level IPA
cflags_nw4r = [
    *(f for f in cflags_base if f != "-multibyte"),
    "-i include/MSL",
    "-i include/MSL/internal",
    "-ir include/revolution/BTE",
    "-DREVOLUTION",
    "-enc SJIS",
    "-fp_contract off",
    "-ipa file",
    "-i include/nw4r",
]

# Library units imported from Rhae take Rhae's flags: its base adds the MSL/BTE include paths,
# REVOLUTION and the version macro its sources test (its version is named RHAE01_01)
cflags_rhae = [
    *(f for f in cflags_base if f != "-multibyte"),
    "-i include/MSL",
    "-i include/MSL/internal",
    "-ir include/revolution/BTE",
    "-DREVOLUTION",
    "-DVERSION_RHAE01_01",
]
cflags_egg = [
    *cflags_rhae,
    "-enc SJIS",
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-i include/nw4r",
    "-ir include/egg",
]
cflags_rvl = [
    *cflags_rhae,
    "-enc SJIS",
    "-fp_contract off",
    "-ipa file",
]
cflags_rfl = [
    *cflags_rhae,
    "-enc SJIS",
    "-fp_contract off",
    "-Cpp_exceptions on",
    "-ipa file",
]
cflags_hbm = [
    *cflags_rhae,
    "-sdata 0",
    "-sdata2 0",
    "-enc SJIS",
    "-lang c++",
    "-fp_contract off",
    "-ipa file",
    "-i include/nw4r",
    "-i include/homebuttonMiniLib",
]
cflags_runtime = [
    *cflags_rhae,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-fp_contract off",
    "-D_IEEE_LIBM",
]
cflags_trk = [
    *cflags_rhae,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-inline deferred",
    "-sdata 0",
    "-sdata2 0",
]
# BTE (Broadcom's Bluetooth stack), imported from Petari (github.com/SMGCommunity/Petari, CC0; the
# sources keep Broadcom's Apache-2.0 headers, see src/revolution/BTE/LICENSE-APACHE-2.0 and NOTICE).
# Petari's SDK flags and compilers; its private headers live next to the sources.
cflags_bte = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    "-O4,p",
    "-inline auto,level=3",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-str reuse",
    "-enc SJIS",
    "-ipa file",
    "-sdata 8",
    "-sdata2 8",
    "-i src/revolution/BTE",
    "-i include",
    "-i include/MSL",
    "-i include/MSL/internal",
    f"-i build/{config.version}/include",
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION_{config.version}",
    "-DREVOLUTION",
]
if not args.debug:
    cflags_bte.append("-DNDEBUG=1")

# units.json library -> (cflags, compiler), as Rhae builds them
lib_options = {
    "EGG": (cflags_egg, "GC/3.0a5.2"),
    "RVL SDK": (cflags_rvl, "GC/3.0a5.2"),
    "RVLFaceLib": (cflags_rfl, "GC/3.0a5.2"),
    "HomeButtonMiniLib": (cflags_hbm, "GC/3.0a5.2"),
    "MSL": (cflags_runtime, "GC/3.0a3"),
    "Runtime": (cflags_runtime, "GC/3.0a5.2"),
    "MetroTRK": (cflags_trk, "GC/2.7"),
}

# Per-unit options Rhae's configure.py gives single objects
object_options = {
    "egg/gfxe/eggG3DUtility.cpp": {"extra_cflags": ["-opt noloop"]},
    "egg/prim/eggAssert.cpp": {"extra_cflags": ["-Cpp_exceptions on"]},
    "egg/gfx/eggDrawHelper.cpp": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/math_sun.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/mem.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/strtoul.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/buffer_io.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/mbstring.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/ansi_files.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/alloc.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/printf.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/file_io.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL_C/uart_console_io_gcn.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL_C/math_api.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/direct_io.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/FILE_POS.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/scanf.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/strtold.c": {"extra_cflags": ["-Cpp_exceptions on"]},
    "MSL/ansi_fp.c": {"extra_cflags": ["-Cpp_exceptions on", "-ipa file"]},
    "egg/core/eggController.cpp": {"extra_cflags": ["-ipa file"]},
    "MSL_C/printf.c": {"mw_version": "GC/3.0a5.2", "extra_cflags": ["-Cpp_exceptions on", "-ipa file"]},
    "MSL_C/wprintf.c": {"mw_version": "GC/3.0a5.2", "extra_cflags": ["-Cpp_exceptions on", "-ipa file"]},
    "egg/geom/eggPlane.cpp": {"extra_cflags": ["-Cpp_exceptions on"]},
    "egg/math/eggMath.cpp": {"extra_cflags": ["-Cpp_exceptions on"]},
    "egg/math/eggMatrix.cpp": {"extra_cflags": ["-Cpp_exceptions on"]},
    "egg/math/eggQuat.cpp": {"extra_cflags": ["-Cpp_exceptions on"]},
    "egg/math/eggVector.cpp": {"extra_cflags": ["-Cpp_exceptions on"]},
    "runtime/NMWException.cpp": {"extra_cflags": ["-Cpp_exceptions on"]},
    "revolution/EXI/EXIBios.c": {"extra_cflags": ["-schedule off"]},
    "revolution/VF/pf_entry_iterator.c": {"mw_version": "GC/3.0a5"},
    "revolution/VF/pf_cp932.c": {"mw_version": "GC/3.0a5"},
}

cflags_game = [
    *(f for f in cflags_base if f != "-multibyte"),
    "-DPACK_PARTY",
    "-enc SJIS",
    "-inline deferred",
    "-fp_contract on",
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-i include/MSL",
    "-i include/MSL/internal",
    "-ir include/revolution/BTE",
    "-DREVOLUTION",
    "-i include/nw4r",
    "-ir include/egg",
    "-i include/Pack",
]
cflags_game_pch = [
    *cflags_game,
    "-prefix SportsPack.mch",
]
# g3d_scnrfl sits among the game units and was built with the game flags (functions in
# -inline deferred order)
object_options["nw4r/g3d/g3d_scnrfl.cpp"] = {"cflags": cflags_game_pch}
config.precompiled_headers = [
    {
        "source": "SportsPack.pch",
        "mw_version": "GC/3.0a5.2",
        "cflags": ["-lang=c++", *cflags_game],
    },
]

# Compiler: GC/3.0a5.2 (CodeWarrior Ver 4.1 build 60831) for game code, see the Hky lib below.
# It matches the SDK/NW4R build stamps (0x4199_60831) in main.dol; game functions match under it
# and not under the Wii/1.x builds. GC/3.0a3.4 and GC/3.0a5 also matched every probe so far.
#
# Linker: GC/1.3.2 reproduces main.dol byte-for-byte; GC/3.0a5.2's linker does not (the DOL
# checksum fails with identical objects). Any linker that reproduces the DOL is valid here.
config.linker_version = "GC/1.3.2"


# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": objects,
    }


# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_rel,
        "progress_category": "game",
        "objects": objects,
    }


Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions


config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    {
        "lib": "Runtime.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "progress_category": "sdk",  # str | List[str]
        "objects": [
            Object(NonMatching, "Runtime.PPCEABI.H/global_destructor_chain.c"),
            Object(NonMatching, "Runtime.PPCEABI.H/__init_cpp_exceptions.cpp"),
        ],
    },
    {
        "lib": "BTE",
        "mw_version": "GC/3.0a3",
        "cflags": cflags_bte,
        "progress_category": ["sdk", "rvl"],
        "objects": [
            Object(NonMatching, "revolution/BTE/gki_buffer.c"),
            Object(NonMatching, "revolution/BTE/gki_time.c"),
            Object(NonMatching, "revolution/BTE/gki_ppc.c"),
            Object(NonMatching, "revolution/BTE/hcisu_h2.c"),
            Object(NonMatching, "revolution/BTE/uusb_ppc.c", mw_version="GC/3.0a5.2"),
            Object(NonMatching, "revolution/BTE/bta_dm_cfg.c"),
            Object(NonMatching, "revolution/BTE/bta_hh_cfg.c"),
            Object(NonMatching, "revolution/BTE/bta_sys_cfg.c"),
            Object(NonMatching, "revolution/BTE/bte_hcisu.c"),
            Object(NonMatching, "revolution/BTE/bte_init.c"),
            Object(NonMatching, "revolution/BTE/bte_logmsg.c"),
            Object(NonMatching, "revolution/BTE/bte_main.c"),
            Object(NonMatching, "revolution/BTE/btu_task1.c"),
            Object(NonMatching, "revolution/BTE/bd.c"),
            Object(NonMatching, "revolution/BTE/bta_sys_conn.c"),
            Object(NonMatching, "revolution/BTE/bta_sys_main.c"),
            Object(NonMatching, "revolution/BTE/ptim.c"),
            Object(NonMatching, "revolution/BTE/utl.c"),
            Object(NonMatching, "revolution/BTE/bta_dm_act.c"),
            Object(NonMatching, "revolution/BTE/bta_dm_api.c"),
            Object(NonMatching, "revolution/BTE/bta_dm_main.c"),
            Object(NonMatching, "revolution/BTE/bta_dm_pm.c"),
            Object(NonMatching, "revolution/BTE/bta_hh_act.c"),
            Object(NonMatching, "revolution/BTE/bta_hh_api.c"),
            Object(NonMatching, "revolution/BTE/bta_hh_main.c"),
            Object(NonMatching, "revolution/BTE/bta_hh_utils.c"),
            Object(NonMatching, "revolution/BTE/btm_acl.c"),
            Object(NonMatching, "revolution/BTE/btm_dev.c"),
            Object(NonMatching, "revolution/BTE/btm_devctl.c"),
            Object(NonMatching, "revolution/BTE/btm_discovery.c"),
            Object(NonMatching, "revolution/BTE/btm_inq.c"),
            Object(NonMatching, "revolution/BTE/btm_main.c"),
            Object(NonMatching, "revolution/BTE/btm_pm.c"),
            Object(NonMatching, "revolution/BTE/btm_sco.c"),
            Object(NonMatching, "revolution/BTE/btm_sec.c"),
            Object(NonMatching, "revolution/BTE/btu_hcif.c"),
            Object(NonMatching, "revolution/BTE/btu_init.c"),
            Object(NonMatching, "revolution/BTE/wbt_ext.c"),
            Object(NonMatching, "revolution/BTE/gap_api.c"),
            Object(NonMatching, "revolution/BTE/gap_conn.c"),
            Object(NonMatching, "revolution/BTE/gap_utils.c"),
            Object(NonMatching, "revolution/BTE/hcicmds.c"),
            Object(NonMatching, "revolution/BTE/hidd_api.c"),
            Object(NonMatching, "revolution/BTE/hidd_conn.c"),
            Object(NonMatching, "revolution/BTE/hidd_mgmt.c"),
            Object(NonMatching, "revolution/BTE/hidd_pm.c"),
            Object(NonMatching, "revolution/BTE/hidh_api.c"),
            Object(NonMatching, "revolution/BTE/hidh_conn.c"),
            Object(NonMatching, "revolution/BTE/l2c_api.c"),
            Object(NonMatching, "revolution/BTE/l2c_csm.c"),
            Object(NonMatching, "revolution/BTE/l2c_link.c"),
            Object(NonMatching, "revolution/BTE/l2c_main.c"),
            Object(NonMatching, "revolution/BTE/l2c_utils.c"),
            Object(NonMatching, "revolution/BTE/port_api.c"),
            Object(NonMatching, "revolution/BTE/port_rfc.c"),
            Object(NonMatching, "revolution/BTE/port_utils.c"),
            Object(NonMatching, "revolution/BTE/rfc_l2cap_if.c"),
            Object(NonMatching, "revolution/BTE/rfc_mx_fsm.c"),
            Object(NonMatching, "revolution/BTE/rfc_port_fsm.c"),
            Object(NonMatching, "revolution/BTE/rfc_port_if.c"),
            Object(NonMatching, "revolution/BTE/rfc_ts_frames.c"),
            Object(NonMatching, "revolution/BTE/rfc_utils.c"),
            Object(NonMatching, "revolution/BTE/sdp_api.c"),
            Object(NonMatching, "revolution/BTE/sdp_db.c"),
            Object(NonMatching, "revolution/BTE/sdp_discovery.c"),
            Object(NonMatching, "revolution/BTE/sdp_main.c"),
            Object(NonMatching, "revolution/BTE/sdp_server.c"),
            Object(NonMatching, "revolution/BTE/sdp_utils.c"),
        ],
    },
]


# Every other named source-file unit (generated by tools/gen_units_config.py from splits.txt),
# registered as NonMatching so progress reports cover the whole program. SDK/NW4R build stamps
# say CodeWarrior build 60831 (GC/3.0a5.2); HBM's 60726 is not in the archive, so it is a
# placeholder there until that library is matched.
# Units listed in config/<version>/matching.txt (one per line) compile from src/ and are linked;
# the rest link from the split assembly. A unit goes on the list once objdiff shows every one of
# its functions and data matching.
_listed = {obj.name for lib in config.libs for obj in lib["objects"]}
_matching_path = Path("config") / config.version / "matching.txt"
_matching = set()
if _matching_path.exists():
    _matching = {l.strip() for l in _matching_path.read_text(encoding="utf-8").splitlines()
                 if l.strip() and not l.startswith("#")}
# Hand-listed library units (BTE) also link from source once listed in matching.txt
for _lib in config.libs:
    for _obj in _lib["objects"]:
        if _obj.name in _matching:
            _obj.completed = True
with open(Path("config") / config.version / "units.json", encoding="utf-8") as _f:
    for _lib in json.load(_f):
        _objects = [Object(Matching if name in _matching else NonMatching, name,
                           **object_options.get(name, {}))
                    for name in _lib["objects"] if name not in _listed]
        if _objects:
            _cflags, _mw = lib_options.get(_lib["lib"], (cflags_base, "GC/3.0a5.2"))
            config.libs.append({
                "lib": _lib["lib"],
                "mw_version": _mw,
                # game code takes the RP flags and precompiled header, libraries Rhae's
                "cflags": (cflags_game_pch if "game" in _lib["progress_category"]
                           else cflags_nw4r if "nw4r" in _lib["progress_category"]
                           else _cflags),
                "progress_category": _lib["progress_category"],
                "objects": _objects,
            })


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    if not config.non_matching:
        return objects
    if module_id == 0:  # DOL
        return objects + ["dummy.c"]
    return objects


# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("game", "Game Code"),
    ProgressCategory("framework", "RP framework (shared game code)"),
    ProgressCategory("minigame", "Minigames"),
    ProgressCategory("sdk", "SDK Code"),
    ProgressCategory("rvl", "RVL SDK"),
    ProgressCategory("nw4r", "NintendoWare"),
    ProgressCategory("egg", "EGG"),
    ProgressCategory("rfl", "RVLFaceLib"),
    ProgressCategory("hbm", "HomeButtonMiniLib"),
    ProgressCategory("runtime", "Runtime, MSL, MetroTRK"),
]
config.progress_each_module = args.verbose
# Optional extra arguments to `objdiff-cli report generate`
config.progress_report_args = [
    # Marks relocations as mismatching if the target value is different
    # Default is "functionRelocDiffs=none", which is most lenient
    # "--config functionRelocDiffs=data_value",
]

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress information
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
