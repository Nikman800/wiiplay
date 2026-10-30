# Setup

The repository holds only our own work: source, headers, tools, configs, ledgers and docs.
It never ships Nintendo files. Everything derived from the game is rebuilt from your own disc.

The project builds and is tested on Linux (x86-64). The build system is the dtk-template one, which
also knows macOS (through wibo) and Windows (native), but those are not tested here.

## Requirements

- Python 3.9 or newer, standard library only. No pip packages.
- ninja (any recent version; developed with 1.13).
- git, bash, curl and the usual coreutils (`sha1sum`).
- gdb, only for `tools/mwcc_trace.py`.

Wine is not needed. On Linux the build downloads everything else into `build/` on first run
(versions pinned in `configure.py`):

| Tool | Version | Where | Used for |
|---|---|---|---|
| decomp-toolkit (dtk) | v1.8.4 | `build/tools/dtk` | disc extraction, DOL split, ELF to DOL, SHA-1 check |
| wibo | 1.0.3 | `build/tools/wibo` | runs the Windows CodeWarrior compilers and linker |
| CodeWarrior compilers | archive `20251118` | `build/compilers/` | `GC/3.0a5.2` compiles, `GC/1.3.2` links |
| objdiff-cli | v3.6.1 | `build/tools/objdiff-cli` | per-function match report and progress |

The compilers are proprietary Metrowerks software redistributed by the decomp community.
Game code uses `GC/3.0a5.2` (CodeWarrior 4.1 build 60831). The link uses `GC/1.3.2`, which
reproduces the DOL exactly; the 3.0a5.2 linker does not.

## Your disc

You need Wii Play (USA) Rev 1, game ID **RHAE01**, dumped from your own disc. Any format dtk reads
works (RVZ, ISO, WBFS, ...). The only code on the disc is `sys/main.dol`; there are no RELs.
Its SHA-1 must be `0da5e7e51135219f580ad011d1b635bc83569bb9` (`config/RHAE01/config.yml`).

## Extract

```sh
bash tools/reproduce.sh "/path/to/Wii Play (USA) (Rev 1).rvz"
```

This downloads dtk if `build/tools/dtk` is missing, extracts the data partition to `extracted/`,
checks `main.dol` against the expected SHA-1, and writes the analysis files the inspection tools
read: `dtk/split/`, `dtk/index.json` and `main.elf` (the DOL as an ELF with the project's names).
It deletes and rewrites `extracted/` each time. The build itself needs only `extracted/`.

To extract by hand instead: `build/tools/dtk disc extract "/path/to/disc.rvz" extracted`.

## Build and verify

```sh
python3 configure.py
ninja
sha1sum -c config/RHAE01/build.sha1
```

The first `ninja` downloads the tools above, splits the original DOL into
`build/RHAE01/asm/` and `build/RHAE01/obj/`, compiles every unit listed in
`config/RHAE01/matching.txt` from `src/`, links `build/RHAE01/main.dol`, checks its SHA-1 and
prints progress. The last line must say `build/RHAE01/main.dol: OK`.

`python3 configure.py -h` lists the options (`--map`, `--non-matching`, `--wrapper`, tool paths).
Re-run `configure.py` after changing `matching.txt`, `splits.txt` or `configure.py`.

Do not trust a printed `OK` on its own after a failed build: ninja keeps the old DOL when a step
fails. Check that `ninja` exited with status 0.

dtk warns `Unknown section .data6`; that is expected. `.data6` is an extra data section with four
embedded blobs used only by the health-and-safety screen. `config/RHAE01/splits.txt` fixes the
section names dtk would otherwise guess wrong.

## Matching workflow

Start with `tools/MATCHING.md`: the loop for one function, the natural-C rules, the tools and
what we know about the compiler. `tools/LEVERS.md` lists what to change for each kind of
difference.

| Command | What it does |
|---|---|
| `tools/unit.py status <unit>` | compile `src/<unit>`, show the match % of every function and data object |
| `tools/unit.py cmp <unit>` | real differences per function, plus the function-order check |
| `tools/unit.py diff <unit> <symbol>` | one function side by side, original and yours |
| `tools/unit.py sync <unit>` | give the original symbols the names your source compiles to |
| `tools/unit.py check <unit>` | link the unit from source; keep it in `matching.txt` if the DOL matches |
| `tools/locals.py <unit> <function>` | register use of each local, for register differences |
| `tools/try_variants.py <unit> <name> <file>` | score several rewrites of one function in one run |
| `tools/link_diag.py <unit>` | why `check` fails: section and symbol differences after linking |
| `tools/mwcc_trace.py <unit> <function>` | trace CodeWarrior's register allocator (`--sched`: scheduler, `--frontend`: optimizer IR); needs gdb |
| `tools/land.py -m "message" <unit>...` | rebuild and verify the DOL, check the commit's scope, commit and push (`--dry-run`, `--no-push`) |
| `tools/fn_info.py show <addr>` | callers, callees, literals and pseudocode of one function |

`<unit>` is the name in `config/RHAE01/splits.txt`, for example
`Pack/RPParty/RPHkyScene/RPHkyField.cpp`. `unit.py -h`, `land.py -h` and `mwcc_trace.py -h`
describe the options. `link_diag.py` and `fn_info.py` take no `-h`: `link_diag.py` treats its
first argument as a unit and relinks.

A function that does not match keeps a `// NONMATCHING:` comment and a row in
`docs/NONMATCHING.md`.

## Optional: Ghidra pseudocode

`tools/fn_info.py` and the typed pseudocode in `decomp_typed/all/` need a Ghidra export.
This is a reading aid, not part of the build.

1. Install Temurin JDK 21 and Ghidra 12.1.4. The scripts look in `~/tools/jdk21` and
   `~/tools/ghidra_12.1.4_PUBLIC`; export `JAVA_HOME` and `GHIDRA` to use other paths.
2. Install Cuyler36/Ghidra-GameCube-Loader 1.3.1 (Ghidra 12.1 build) into
   `$GHIDRA/Ghidra/Extensions` and compile its language once:
   ```sh
   JAVA_HOME=~/tools/jdk21 $GHIDRA/support/sleigh \
     $GHIDRA/Ghidra/Extensions/GameCubeLoader/data/languages/ppc_gekko_broadway.slaspec
   ```
   Without it the import fails with `Unsupported language: PowerPC:BE:32:Gekko_Broadway`.
3. Pseudocode (~10 min): `bash tools/reproduce.sh "/path/to/disc.rvz" --decomp` writes `decomp/`
   and `dtk/literals.json`.
4. Typed pseudocode (~15 min):
   ```sh
   python3 tools/gen_ghidra_types.py .
   python3 tools/gen_this_map.py .
   bash tools/export_typed.sh        # -> decomp_typed/all/
   ```

## Ledgers

The naming and splitting ledgers are rebuilt by `bash tools/phase2_pipeline.sh` (idempotent; it ends
with `ninja` and the SHA-1 check). Size ledgers are checked with
`for f in dtk/types/*_sizes.tsv; do python3 tools/sizeof_probe.py $f; done`.
Header conventions: `tools/HEADER_CONVENTIONS.md`. Naming conventions: `tools/NAMING_CONVENTIONS.md`.

## What is committed

| Path | Content |
|---|---|
| `configure.py`, `tools/project.py` | dtk-template build system (CC0) |
| `config/RHAE01/` | `config.yml`, `splits.txt`, `symbols.txt`, `matching.txt`, `build.sha1` |
| `src/`, `include/` | decompiled source and headers |
| `dtk/` | naming, type and data-ownership ledgers with evidence |
| `tools/` | scripts and conventions |
| `docs/` | plan, setup and status |

Not committed (`.gitignore`): disc images, `extracted/`, `main.elf`, `build/`, `dtk/split/`,
`dtk/index.json`, `dtk/literals.json`, `decomp/`, `decomp_typed/`.

## Status

- `docs/PLAN.md`: phases and where the project stands.
- `docs/NONMATCHING.md`: functions that do not match yet, and natural-C debt.
- `docs/PHASE4_PENDING.md`: open game-code items and the dated log.
