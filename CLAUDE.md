# Digimon World 2003 (PS1, EU) decompilation

**Goal:** C source that recompiles into a byte-identical PS1 executable (SLES-03936, built
with the original-era toolchain). The same C also builds the native PC port (`port/`, `docs/PORT.md`).

## Session protocol
0. **Worktrees (Orca):** sessions usually run in a git worktree, which has **only tracked files**: no
   `iso/`, `extracted/`, built `tools/*`, `tools/local.env`, `build/`, `asm/`. If `tools/binutils` or
   `iso/dw2003.bin` is missing, run `scripts/worktree_init.sh` first (~1 s: links the main checkout's tools,
   links the disc, extracts it). Commit on the worktree's branch, push it, and open a **pull request** to `main`
   (`gh pr create`; CI must be green; a docs-only pull request starts no run); the user reviews and merges. Commit straight to `main` only when the user says
   so for that change (DECISIONS "Pull requests, ours and from forks").
   **Cloud sessions** (fresh clone): the SessionStart hook (`.claude/hooks/session-start.sh`) runs `setup.sh` and
   `worktree_init.sh` before the session starts; the disc comes from the data checkout beside the repo (below).
   In a session that selects **both repositories** the project directory is their parent, so this repo's hook does
   **not** fire: if `tools/binutils` or `iso/dw2003.bin` is missing, run it yourself (~4 min cold):
   `CLAUDE_PROJECT_DIR=$PWD .claude/hooks/session-start.sh` (session 14).
1. **Start:** read `docs/STATUS.md` (current state). Planned work is on the GitHub Project board, problems are GitHub
   issues (`gh issue list`; `gh project item-list` needs the token's `project` scope).
2. Work in phases. Stop at the end of each phase to summarize, and wait for the user's go-ahead.
3. **End:** update `docs/STATUS.md` when the state changed; record a decision in `docs/DECISIONS.md` only when it
   changes how the project works (a few lines: what and why); turn leftover work into issues (bugs) or project items
   (plans), not into docs; commit with a clear message. There is no session log: the pull request says what was done
   (DECISIONS "Docs are lean").

## Rules
- **Never commit game data:** no ISO/BIN, extracted files, split asm, assets, BIOS, or Psy-Q/SDK files.
  The repo is code-only, and users supply their own disc. Check `git status` before committing.
  **The data checkout:** the owner keeps a separate private checkout (`scripts/gamedata_dir.sh`: `$DW3_GAMEDATA`,
  `tools/local.env`, or the sibling `../dw2003-gamedata`) holding `gamedata/` (what `setup.sh gamedata` and the
  `--bios retail` cross-checks read) and `tools/prebuilt/` (the pinned emulator build, for networks that can't download
  it). Cloud sessions and CI select it beside this repo. Never copy anything from it into this repo (DECISIONS
  "Going public").
- Ask before anything that needs sudo or installs system-wide. Install project-locally (`tools/`).
- Don't claim something works until it has been run. When unsure, say so and test.
- Everything reproducible: setup steps go in `scripts/` or `docs/`, never only in shell history.
- **New tools** get a `setup.sh` step that installs into `tools/<gitignored dir>` (`gcc`, `ext`, `bin`, … are
  already ignored; else add a `.gitignore` line without a trailing slash). `setup.sh` always installs into the
  **main checkout's** `tools/` and symlinks it into the current worktree, so tools are built once and shared.
  Changing a pinned version therefore affects every worktree.
- **EU only, independent project.** The US decomp (juandav/dw3_decomp) is a reference, not a dependency.
  Record anything borrowed in `docs/THIRD_PARTY.md` when borrowing it.
- Base image is the **unpatched** EU disc (SHA-1 `457cb233…`). The NTSC/60fps patch is 2 data
  bytes (`0x8005CCAC`, `0x8005CCB0`), not a code change; treat it as a later build option.

## Layout
| Path | Tracked | Contents |
|---|---|---|
| `iso/` | no | `dw2003.bin/.cue` (symlink to user's disc, made by `setup.sh disc`) |
| `extracted/` | no | Disc contents from dumpsxiso, plus its rebuild XML |
| `tools/` | scripts only | Built tools `venv/`, `binutils/`, `mkpsxiso/`, `gcc/<ver>/`, `bin/` (objdiff-cli), `ext/` (pinned clones), `redux/` (PCSX-Redux, headless), `src/` (live in the main checkout; symlinks in worktrees); our scripts (`psyq_match.py`, …); `mcp/` (the MCP server for the port's debug channel, `tools/mcp/README.md`; `.mcp.json` at the root registers it); `requirements.txt`; `local.env` (machine paths, untracked; see `local.env.example`) |
| `port/` | yes | The PC port: `CMakeLists.txt`, `src/` (arena, overlay manager, pump), `psyq/` (the Psy-Q shim), `README.md`; builds into `build/port/` (`docs/PORT.md`; hooks: `include/port.h`) |
| `launcher/` | yes | The launcher (`launcher/README.md`, `docs/LAUNCHER.md`): C++, Dear ImGui on SDL3; its own CMake project, builds into `build/launcher/` |
| `config/` | yes | splat YAML per link unit (`main.yaml` = EXE, `<overlay>.yaml`), `symbol_addrs.txt` (EXE names), `<overlay>.symbols.txt` (overlay names), `*.sha1` |
| `src/`, `include/` | yes | Decompiled C, recovered headers/structs |
| `asm/`, `assets/` | no | splat output per target (`asm/main/`, …), regenerated by `configure.py` |
| `build/` | no | Build outputs; `build/<target>/` has splat's linker script, ELF, map |
| `configure.py`, `diff_settings.py` | yes | Ninja/objdiff generator; asm-differ settings. `build.ninja`, `objdiff.json`, `include/asm_generated/` are generated (untracked) |
| `scripts/` | yes | `setup.sh`, `extract.sh`, `build.sh`, … |
| `packaging/` | yes | The release AppImage's own files: `appimage/` (.desktop, icon, font notices); `scripts/package_appimage.sh` uses them (`docs/RELEASE.md`) |
| `tests/` | yes | Reference tests for the port (`tests/README.md`): `golden/` (layer 1: the oracle `oracle.lua`/`oracle.py`, families, goldens JSON), `host/` (the goldens replayed on the host, findings), `replay/` (layer 2: pad scripts, expected hashes). Tests never touch `src/`, `include/`, `config/` |
| `docs/` | yes | See below |

## Commands
```sh
scripts/worktree_init.sh    # fresh worktree: share main checkout's tools, link disc, extract (run first!)
scripts/setup.sh && scripts/worktree_init.sh   # cloud session setup (the SessionStart hook runs it; ~4 min cold)
scripts/setup.sh            # toolchain + disc symlink (idempotent; steps: binutils mkpsxiso venv gcc objdiff ext link disc)
scripts/extract.sh          # disc -> extracted/disc + extracted/dw2003.xml (dumpsxiso -pt)
scripts/rebuild_iso.sh      # rebuild image from extracted/, must be bit-identical
tools/venv/bin/python tools/disc_survey.py   # EXE header, file stats, overlay base check
scripts/build.sh            # configure (splat + build.ninja) if needed, ninja, SHA-1 check; --check = from scratch
scripts/merge_checkpoint.sh <branch> "<msg>"   # merge an agent branch, build --check, update README progress
tools/venv/bin/python tools/progress.py --readme   # README progress table (code matched from C, per part/overlay)
tools/venv/bin/python configure.py   # re-split + regenerate build.ninja/objdiff.json (ninja does this itself on config changes)
scripts/check_toolchain.sh  # smoke test: every old-gcc through maspsx/as, m2c, objdiff, asm-differ, permuter
scripts/setup.sh redux && scripts/check_emulator.sh [--bios retail]   # PCSX-Redux (data checkout or pinned download); boots the disc headlessly to CNTY_SEL (~16 s)
scripts/test.sh [--layer 1|2|3|port|mods] [--m32]   # reference tests (tests/README.md): goldens regenerated in the emulator + host replay, pad-script replays, formats, the port's M1 test and checks, the mods; DW3_JOBS=N runs replays and mods N at a time
tests/golden/oracle.py gen|check|list [families] [--bios retail]   # layer-1 goldens: one boot calls the game's functions (DECISIONS "Layer-1 goldens: calls on the running game")
tests/host/replay.py [families] [--findings]   # the goldens through the C compiled with gcc -m64; mismatches = tests/host/FINDINGS.md
tests/replay/replay.py run tests/replay/scripts/<name>.json [--record] [--repeat 2]   # layer-2 script; `check` replays them all
tools/venv/bin/python tools/coverage.py run [--oracle] [--replay NAME ...] | report [--module X]   # function coverage of the emulator tests -> build/coverage/report.md (~23 min; --oracle alone ~40 s)
tests/holdouts/run.sh [--control] [--no-probe]   # NON_MATCHING build -> scratch disc image -> replays: validates the holdouts' WIP C (~11 min; not in test.sh)
tools/redux/pcsx-redux -no-ui -stdout -testmode -run -iso iso/dw2003.cue -bios <bin> -dofile <lua>   # headless run with a Lua script (tools/redux_boot_check.lua is the template)
tools/venv/bin/python tools/port_inventory.py counts [--sites KIND[:TAG]]   # PC port inventory (`docs/PORT.md`): Psy-Q calls, macros, literal sizes, PS1 addresses, port.h hooks; --sites lists file:line
tools/venv/bin/python tools/port_inventory.py probe [FILES...] [-v] [--warnings] [--m32]   # host-compile gate (-m64, -Werror on pointer/int casts, implicit declarations, incompatible pointers); exit 0 = clean (~1 s); test.sh and CI run it
tools/venv/bin/python tools/port_inventory.py link | structs   # duplicate/undefined globals over the probe's objects; struct sizes at -m32/-m64 vs the documented PS1 sizes
tools/venv/bin/python tests/port/run.py [--m32] [--sanitize] [--cd-speed instant] [--exe build/port-win/dw2003.exe --wine]   # the port's M1 test: new_game from the disc, twice, against the emulator's record (tests/port/README.md); --exe --wine: the Windows build under Wine
build/port/dw2003 --disc iso/dw2003.cue --debug /tmp/dw3.sock [--cd-speed instant]   # the debug channel (port/src/debug.c: pause/step/wait, pad, peek/poke, screenshot, hash over a Unix socket); `.mcp.json` registers tools/mcp/server.py as an MCP server that drives it (tools/mcp/README.md); tools/venv/bin/python tools/mcp/selftest.py = its offline self-test; tests/port/debug.py = the gate (in test.sh's port layer)
scripts/setup.sh sdl3 && cmake -S port -B build/port-sdl -G Ninja -DDW3_PORT_SDL=ON && cmake --build build/port-sdl && build/port-sdl/dw2003 --disc iso/dw2003.cue --window   # play it (M2: software GPU, SDL3 window, keyboard/gamepad)
scripts/setup.sh llvm-mingw sdl3-windows && scripts/build_windows.sh [--launcher] [--test]   # the Windows cross-build (cmake/windows-x86_64.cmake: build/port-win, build/launcher-win; --test: the launcher's self-test under Wine); tools/venv/bin/python tools/port_inventory.py probe --target windows = the units through llvm-mingw's clang
scripts/setup.sh imgui sdl3-desktop appimage && scripts/package_appimage.sh --test   # the release AppImage (docs/RELEASE.md; Fedora without libstdc++-static: --shared-libstdcxx); a tag vX.Y.Z runs release.yml: a DRAFT release
scripts/setup.sh imgui llvm-mingw sdl3-windows && scripts/package_windows.sh --test   # the release's Windows zip + its PDB symbols zip (docs/RELEASE.md "The Windows package"); --test runs the unzipped package under Wine
scripts/release_local.sh vX.Y.Z [--build-only]   # the same DRAFT release (the AppImage and the Windows zip) built locally in Docker ubuntu:24.04, release.yml's run cancelled (~10 min vs ~30)
scripts/ci_areas.sh --diff origin/main   # which CI areas (game, port, launcher) a change runs (DECISIONS "CI per area")
tools/venv/bin/python tests/sound/spu_trace.py run|check|diff ...   # the emulator's SPU write trace (M3's oracle; docs/SOUND.md); tests/sound/key_trace.py gen|check: per-key goldens (~22 min)
build/port/dw2003 --disc iso/dw2003.cue --script tests/replay/scripts/new_game.json --wav out.wav [--spu-trace out.trace]   # the port's audio (M3) headless
scripts/setup.sh sdl3 imgui && cmake -S launcher -B build/launcher -G Ninja && cmake --build build/launcher && build/launcher/dw2003-launcher   # the launcher; SDL_VIDEO_DRIVER=offscreen ... --self-test DIR: its headless self-test
tools/venv/bin/python tests/saves/run.py   # save round trips port <-> emulator and the two cards (layer 3, ~1 min)
cmake -S port -B build/port -G Ninja && cmake --build build/port && build/port/dw2003 --max-frames 60   # the PC port (port/README.md): all 388 units + port/psyq stubs, 30 s; runs the game's main() to a frame cap
tools/venv/bin/python tools/psyq_match.py extracted/disc/SLES_039.36   # Psy-Q signature matcher
tools/venv/bin/python tools/compiler_id.py   # compiler-ID experiment: m2c drafts x 5 GCCs x -G0/-G8, objdiff-scored
tools/venv/bin/python tools/compiler_id.py --target fieldstg --rodata-end 0x80083784   # same, on an overlay
tools/venv/bin/python tools/overlay_layout.py [NAME]   # a tier-1 overlay's .rodata/.text/.data boundaries
tools/venv/bin/python tools/overlay_layout.py --tier2 | --wstag-table   # tier-2 layouts; regenerates config/wstag.txt
tools/venv/bin/python tools/data_owners.py [-t <overlay>] [--yaml]   # owner C file of every data symbol (per-file data split)
tools/venv/bin/python tools/data_to_c.py [-t <overlay>] <symbol>   # a data symbol as a C initializer
tools/venv/bin/python tools/wstag_groups.py [--funcs|--add|--propagate]   # WSTAG code groups; copy a matched function to its group
tools/venv/bin/python tools/overlay_xref.py fieldstg   # per-function evidence for an overlay's file boundaries
tools/venv/bin/python tools/disc_files.py   # file ID <-> disc path map (importable read(id)/subfile() helpers; docs/FORMATS.md)
tools/venv/bin/python tools/dump_text.py TALK08 [-s N] [-r] | --check   # print/decode the game's text files
tools/venv/bin/python tools/flag_census.py [--type 0x72] | --check   # every flag word the game reads/writes, by type and index (from the C; asm/ for the residual-asm sweep); --check = layer 3's range check
scripts/setup.sh psyq && tools/psyq_compare.sh [-D NON_MATCHING] src/<t>/<unit>.c <func>   # same function through Sony's real Psy-Q chain vs ours
tools/cc_psx.sh -V 2.8.1 -Iinclude -Iinclude/asm_generated -I. in.c -o out.o   # compile like the build does
tools/venv/bin/python tools/unit_diff.py gfx [func_X] [-t fieldstg]  # one C unit vs the original, per function + data sections (no ninja; parallel-safe)
tools/venv/bin/python tools/split_case_sizes.py asm/<t>/X.s   # size: lines for functions cut at a switch case
tools/venv/bin/python tools/ext/asm-differ/diff.py -mw func_X       # asm diff of a function vs the original
tools/venv/bin/python tools/ext/decomp-permuter/import.py src/main/X.c asm/main/nonmatchings/X/func_Y.s
source tools/venv/bin/activate; export PATH="$PWD/tools/binutils/bin:$PWD/tools/mkpsxiso/bin:$PATH"
```

## Conventions (details: `docs/DECISIONS.md`)
- Unnamed symbols keep splat defaults: `func_8001xxxx`, `D_8001xxxx`, `jtbl_…`; in overlays `func_FIELDSTG_8008xxxx`.
  Renamed ones go in `config/symbol_addrs.txt` (`name = 0x80012345; // type:func`), the single source of truth
  for the EXE; overlays use `config/<overlay>.symbols.txt` (they share addresses).
- Names (DECISIONS "Naming conventions"): `<module>_<verb>_<noun>` snake_case for game functions/globals, with the
  module prefix = source file name (`src/main/filetable.c` → `filetable_get_lba`). Types PascalCase, constants
  `MODULE_UPPER_SNAKE`, unknown fields `unk_1C`. Name only what is understood. Psy-Q names stay Sony's.
- Match status: a function is **matching** when it compiles from C and the full build is
  byte-identical. Non-matching functions stay as `INCLUDE_ASM`; WIP C goes under `#ifdef NON_MATCHING`.
- Never decompile Psy-Q library functions. Identify them by signature, name them, keep them as split asm
  (excluded from progress). No Psy-Q SDK is needed or used.

## Docs
`docs/` holds reference docs only (how things are, and why); plans go to the project board, problems to issues.
`docs/STATUS.md` current state · `docs/DECISIONS.md` the key decisions · `docs/MATCHING.md` matching workflow, tools,
C patterns (the agent brief) · `docs/TOOLCHAIN.md` compiler/SDK evidence · `docs/DISC_LAYOUT.md` disc files, memory
map, overlays · `docs/FORMATS.md` file formats · `docs/MECHANICS.md` game mechanics mapped to functions and tests ·
`docs/PORT.md` the PC port · `docs/LAUNCHER.md` the launcher/game contract, settings, mods · `docs/SOUND.md` sound ·
`docs/RELEASE.md` making a release · `docs/THIRD_PARTY.md` borrowings · `tests/README.md` the reference tests ·
`CONTRIBUTING.md` for contributors · `.github/ISSUE_TEMPLATE/`, `.github/PULL_REQUEST_TEMPLATE.md` (fork pull requests)
