# Decisions

The decisions that still shape the project, each in its final form, grouped by theme. How to match a function is in
`docs/MATCHING.md`; the current state is in `docs/STATUS.md`.

# Repository and data

## Independent EU-only project on the unpatched disc
_Decided: 2026-10-01_

The project targets only `Digimon World 2003 (Europe)`, SLES-03936, the **unpatched** image (one MODE2/2352 track,
SHA-1 `457cb233349ba841e03b33d8060f8fbcadd45cb3`): a byte-matching build needs a verifiable, unmodified image. The
community 60 fps/NTSC patch changes only two data words (`0x8005CCAC`, `0x8005CCB0`), so it is a build or runtime
option, not a target. The US decomp (juandav/dw3_decomp, MIT) is a reference, never a dependency: we make our own
tooling and convention choices, port its shapes in our own names, and treat its "match depends on" forms as candidates,
not accepted C. Anything copied or closely adapted from another project is recorded in `docs/THIRD_PARTY.md` when it is
borrowed.

## Code only: no game data, SDK or BIOS in the repo
_Decided: 2026-10-05_

The repository holds only code we wrote. Users supply their own disc; `asm/`, `assets/`, `extracted/` and `build/` are
regenerated from it and never tracked; `config/*.sha1` holds the hashes builds must reproduce, never the files. The
owner's private data checkout (`scripts/gamedata_dir.sh`: `$DW3_GAMEDATA`, `tools/local.env` or `../dw2003-gamedata`)
holds the disc, a PAL BIOS for cross-checks only and the pinned emulator build, for cloud sessions and CI; nothing from
it is copied here. Game data as C initializers is accepted, as in every decomp.

## Going public: the working repo is public
_Decided: 2026-10-05_

The public repository is the working repository (one source of truth, no mirror to sync), started from a clean
snapshot of the matching milestone (tag `v0.1-matching-closed`); the private repository is now only the data checkout
and the archive of the earlier history. Everything we wrote is MIT, with the decomp notice in the README; the port and
the launcher admit only MIT/BSD/zlib code, and emulators are external test oracles, never linked or copied. The
takedown risk is accepted with these mitigations: no assets, a user-supplied disc, no SDK files.

## Pull requests, ours and from forks
_Decided: 2026-10-06_

Our own work lands on `main` through a pull request: a branch (usually a worktree), `gh pr create`, CI green, and the
owner reviews and merges. Direct commits to `main` only when the owner asks for that change. Pull requests from forks
are **open** (this replaces "closed by policy until the port exists"); CI gives them the disc-free checks only, since
forks get no secrets, and a maintainer re-runs the full checks on a branch when needed.

## Docs are lean
_Decided: 2026-10-06_

`docs/` holds reference documents only (formats, layout, toolchain, matching, the port's design, these decisions).
The current state is a short `docs/STATUS.md`; planned work lives in the GitHub Project and problems in issues. There
is no session log, no plan or idea document: history is in git and the pull requests.

# Toolchain and build

## Project-local, pinned toolchain
_Decided: 2026-09-30_

`scripts/setup.sh` builds or downloads every tool into `tools/` (binutils, mkpsxiso, the old-gcc PSX builds, objdiff,
pinned git clones of maspsx, m2c, asm-differ, decomp-permuter, SDL3, PCSX-Redux, Python packages in `tools/venv`), each
pinned by version and checksum or commit. No sudo, no system install; anything system-wide is asked first. Tools go into
the main checkout's `tools/` and are symlinked into every worktree, so they build once (a pin change affects every
worktree). The emulator is pinned like a compiler: reference traces are comparable only within one emulator build, so
bumping it is a decision. The disc is extracted with `dumpsxiso -pt` (the directory records are obfuscated), and a
rebuild must be bit-identical.

## Compiler: GCC 2.8.1 with maspsx
_Decided: 2026-10-01_

Game code is built with old-gcc `gcc-2.8.1-psx -O2`, then maspsx emulating ASPSX 2.80, then GNU as, all through
`tools/cc_psx.sh`. EXE game files are `-G8` and overlay code `-G0` (`configure.py` `DEFAULT_G`, `G_OVERRIDES`). Why: the
compiler-ID experiment over unedited m2c drafts (`docs/TOOLCHAIN.md`), and Sony's own CC1PSX/ASPSX (Psy-Q 4.3/4.4, run
once through wibo) produce the same code (`tools/psyq_compare.sh`). Compilers are scored on natural C only, never on what
the permuter can force. No Psy-Q SDK is needed; Sony's binaries stay an optional local cross-check, never committed.

## Build: splat, configure.py and ninja, checked by SHA-1
_Decided: 2026-10-01_

splat 0.50 splits each link unit (the EXE `main` and every overlay) from `config/<target>.yaml`; `configure.py` writes
`build.ninja` and `objdiff.json`. Success means every output (the EXE, 21 tier-1/tier-2 overlays, 293 WSTAG files)
equals its `config/*.sha1`; `scripts/build.sh --check` rebuilds from scratch and is the proof for any config or symbol
change. The layout reproduces psylink: `.rodata` before `.text`, objects packed at 4 bytes, `.bss` written into the
image as zeros.

# Conventions

## Naming conventions
_Decided: 2026-10-01_

Game functions and globals are `<module>_<verb>_<noun>` in snake_case, the prefix being the defining C file
(`filetable_get_lba`); C has no namespaces and the port links everything into one binary. Types are PascalCase
(`<Module><Noun>`), constants `MODULE_UPPER_SNAKE`, unknown fields `unk_<offset>`, alignment padding `pad_<offset>`.
Only what is understood is named; otherwise splat's `func_<addr>`/`D_<addr>`, and `func_<OVERLAY>_<addr>` in overlays.
An original name found in the binary wins; Psy-Q names stay Sony's. EXE names live in `config/symbol_addrs.txt`,
overlay names in `config/<overlay>.symbols.txt` (overlays share addresses).

## Match status and forced (FAKE) matches
_Decided: 2026-10-04_

A function is matching when it compiles from C and the whole build is byte-identical; otherwise it stays `INCLUDE_ASM`,
with any WIP C under `#ifdef NON_MATCHING`. Matching C must be natural: no dummy temporaries, bare `do {} while (0)` or
other permuter-only shapes, and permuter output is only a hint. As a last resort, once natural C has been given up on, a
forced shape is allowed with a `/* FAKE: <what and why> */` comment at the line (`grep -rn "FAKE:" src`). What neither
reaches stays a holdout (`INCLUDE_ASM` plus its WIP C and notes). Accepted judgement calls: `heap_run_object`'s inline
asm, one `goto`, a GNU case range, inline accessors, reads past an array the original makes.

**Enforced in CI (2026-10-07, issue #57).** `tools/hacks.py --check` (the `check` job, `scripts/test.sh`) fails on game
code that is not C (any `INCLUDE_ASM` or `NON_MATCHING` since #55 left no holdouts: its `HOLDOUTS` list is empty; split
asm other than Psy-Q;
inline asm outside `heap_run_object`; `#if 0`), on a `FAKE:` without its reason or a `LOOP_BLOCK` without its evidence
class, and when the counts `docs/STATUS.md` quotes differ from the code's: the docs cannot drift from the workarounds.

## LOOP_BLOCK: loop-scoped blocks as a named macro
_Decided: 2026-10-03_

Some functions match only with a block wrapped in a loop construct: the original very likely used a macro expanding to
`do { } while (0)`, whose loop notes are scheduling barriers and move exits out of line. This is allowed only through
`LOOP_BLOCK(body)` and `LOOP_BARRIER()` in `include/common.h`, each use with a comment naming its evidence class
(`docs/MATCHING.md` "LOOP_BLOCK and LOOP_BARRIER"). The weaker class B uses (register priority only) are kept by the
owner's decision; all are found with `grep -rn LOOP_ src`.

## Psy-Q libraries are identified, never decompiled
_Decided: 2026-10-01_

The SDK is Psy-Q 4.7. `tools/psyq_match.py` names library code from signatures (ties broken by link facts, ambiguity left
unnamed rather than guessed); each library object is its own split-asm unit (`psyq/<lib>/<obj>`), excluded from
progress. The port replaces the SDK with its own shim instead. For the port, LIBSND's disassembly may be read for facts
(constants, tables, order of operations; decided 2026-10-06); the code written from it is original, never a
translation, and `docs/SOUND.md` records what came from where.

# Code and data split

## File boundaries come from evidence
_Decided: 2026-10-01_

Each target is split into the C files the original had, and every boundary carries its evidence in the YAML: jump-table
parity (a table at a different address mod 8 starts a new object), `.data`/`.bss` link order, `$gp` users sharing a file
with their variable, module function tables, duplicated static helpers, create/update pairs and define-before-use.
Weak positions are marked as weak; without evidence there is no split (one file per target). Unclear splits are kept
as they are (the object list in `heap`, the random module in `pad`, `gamestate` as one file, `object` vs `main`).

## Module state structs
_Decided: 2026-10-01_

A module keeps its state and its function table in one struct in `.data`, and other modules call through it. Each such
struct gets its real type and a `size:` in the symbol file, so splat writes `base + offset` instead of inventing labels
inside it, and callers use its fields. Why: labels inside a struct gave label-only mismatches, and GCC 2.8 assumes a
struct field and a scalar global never alias, so scalar externs for table entries schedule differently.

## Data in C, split per object
_Decided: 2026-10-02_

`.data`, `.sdata`, `.sbss` and `.bss` are split per object like the code, and every game unit's data is defined in its C
file (`configure.py` `DATA_IN_C`, compiled with `cc_psx.sh --data-in-c`). Owners come from `tools/data_owners.py`
(references, pointer tables, `$gp` use, link-order monotonicity), and `tools/data_to_c.py` prints a symbol as an
initializer. Weak owners are marked in the configs. Undecoded records are typed later, as the naming and the port need.

## Overlays link against the EXE and their parent
_Decided: 2026-10-01_

Each overlay is its own link unit at its load base, with its own YAML, SHA-1 and symbol file; it links against the EXE's
global symbols, and a tier-2 overlay (WFIGHTMN/WFIGHTTS, the WSTAG stages) also against its resident parent's globals.
A parent calls its children through `absolute:True` names. The 293 WSTAG configs are generated from one template and
`config/wstag.txt`; code shared by several stages is written once and copied into each stage's C file
(`tools/wstag_groups.py --propagate`), as the original was copy-paste per stage.

# Reference tests

## Reference tests: three layers
_Decided: 2026-10-04_

The game's behaviour is pinned by tests that the port must pass (`tests/README.md`): layer 1, goldens of pure logic
recorded from the original in the emulator; layer 2, pad-input replay scripts with hashes of `gamestate_data` at
checkpoints; layer 3, the tools' `--check` modes and save round trips. Facts are read from the matching C rather than
from new disc decoders (`tools/flag_census.py`). Tests never touch `src/`, `include/` or `config/`: a host mismatch
is a finding (`tests/host/FINDINGS.md`, `known_mismatches.json`) for the port to handle, never a change to the game's
C. `scripts/test.sh` is the one entry point.

## Layer-1 goldens: calls on the running game
_Decided: 2026-10-05_

Goldens are made by calling the game's own functions inside the running game from Lua (`tests/golden/oracle.lua`), not
by a separate PS1 test EXE: the real data, tables and overlays are in place and no stubs can diverge. Every case has its
own fixture with save and restore; overlay families copy their overlay into its slot in a kept setup case. Fixtures
write everything the function may read. Goldens are generated with OpenBIOS only and must reproduce with the retail
BIOS.

## Replay and golden contracts
_Decided: 2026-10-05_

Replay scripts wait on state (stage, map, memory), never on frame counts, so they hold on both emulator cores and in the
port. The port compares each checkpoint's stable hash, with fields that depend on timing or the CPU core zeroed (the
checksum byte, `encounter_timer`, play time, `spot_target`); frames and `random_index` are compared only within one
emulator build. Whole-`gamestate_data` reads in goldens skip `playtime_frames` (the vsync handler counts it during
long calls). No memory-poke steps: the debug overlays stay unreached. Random battles are not replayed (their draws
differ between cores).

# PC port

## PC port architecture
_Decided: 2026-10-05_

The port lives in `port/` and compiles the matching C itself. Hooks are macros in `include/port.h` (the plain PS1 code
without `PC_PORT`), plus a few `#ifdef PC_PORT` blocks in `src/`, each verified byte-identical. It has its own Psy-Q
shim, a VRAM-exact software GPU first (a hardware renderer later on the same primitive stream), our own SPU/LIBSND,
MDEC and XA, SDL3 pinned and built into `tools/`, a 64-bit host (the `-m32` build is kept as a layout check), overlays
linked statically with a manager that snapshots and restores their data, LIBCD over the user's hash-checked BIN/CUE,
and PAL 50 Hz by default with the game's own 60 Hz mode as an option.

## The port is checked against the emulator
_Decided: 2026-10-06_

Every port component has an emulator oracle: replays compare the record's cross-core view with layer 2's expected
files, the GPU, GTE and MDEC have layer-1 families, LIBSND reproduces the emulator's SPU write traces on its timeline,
and saves move both ways between port and emulator. Where psx-spx and the emulator disagree, the documented rule is
kept as a listed known mismatch. Quirks the original's results depend on (division by zero, `void` creators returning
`v0`, the pad read a frame late) are reproduced on purpose. Two runs must be byte-identical, with no ASan/UBSan report.

Rendered frames are compared only where the PS1 is not CPU-bound. In battle a game frame takes the PS1 2 or 3 vsyncs
and the port one, and the game scales its animations by the ticks elapsed, so the two never draw the same frame
(issue #7). There the 3D maths is checked by layer-1 families (`libgs_view`) and the VRAM by what is not a frame:
the textures and CLUTs (`tests/port/vram.py`).

## The port's debug channel and the MCP server
_Decided: 2026-10-06_

A tool inspects and drives the running port through an in-process channel (`--debug SOCKET`, `psxstack/runtime/debug.c`),
not through ptrace or gdb: the game thread polls the socket at the vsync boundary, so every read, write and press
lands between two frames and a driven run stays deterministic, and nothing depends on Yama or a debugger. Without
`--debug` nothing of it exists. One Python server, `tools/mcp/`, registered by `.mcp.json`, speaks MCP over it;
`game.py` is the dependency-free client for scripts. Memory is raw bytes by address or symbol name (`nm`,
`config/symbol_addrs.txt`); typed access through DWARF is deferred until a use needs it.

## The arena assumes no alignment and no link address
_Decided: 2026-10-07_

The arena was a 16 MB-aligned `.bss` block of a non-PIE binary, with its regions as ld-script symbols, so that a
pointer's low 24 bits were its ordering-table tag and a truncated host pointer still worked below 4 GB. A PE build can
do none of that (COFF aligns sections to 8 KB at most, ASLR moves the image, lld takes no linker script). Now the
regions are macros on `port_arena` (`include/port.h`), a tag is a pointer's word offset in the tag window (the units'
data regions and the arena, measured at startup, up to 64 MB so that ASan's redzones fit; `port_ptr_to_u32`, resolved
against `port_tag_base` by the shim),
the last `s32` that carried half a host pointer is a pointer. (The ELF link stayed non-PIE one day longer, for the
ld script alone: its `INSERT BEFORE .data` sections fell inside GNU_RELRO in a PIE link with `-z now`, Ubuntu's
defaults, and the game's data came out read-only; "Overlay sections by renaming" below replaced the script.) The symbol sizes of `port_gen.py state` come from compiling the units (asm comments printing `sizeof`), not
from `nm -S`, which COFF cannot give. The Windows track on the project board builds on this. The tag window also
fixed a silent drop: a static ordering table (FIGHTSTG's cursor) lay outside the old walkable window.

## Overlay sections by renaming, no linker script
_Decided: 2026-10-07_

The overlay manager needs each overlay's (and the EXE's) writable data in its own bracketed range, to snapshot at
startup and restore on a load. That was a generated GNU ld script (`-T`, `INSERT BEFORE .data`) collecting the units'
sections by object path, which lld for PE cannot take, and which put the game's data inside GNU_RELRO in a PIE link
with `-z now` (CI's Ubuntu). Now each unit's object has its `.data`/`.bss` sections renamed right after the compile
(`port_gen.py rename`, the units' CMake compiler launcher, with the host's GNU objcopy, which reads COFF too): on ELF
into `dw3_data_<ovl>`/`dw3_bss_<ovl>`, orphan sections for which GNU ld makes the `__start_`/`__stop_` symbols
itself and which it places after `.data`/`.bss`, outside RELRO (a PIE link would be fine for them; the ELF link
stays non-PIE because the debug channel, the MCP server and `tests/port/mods.py` resolve host symbols with `nm`'s
link-time addresses, which a PIE relocates: CI's Ubuntu GCC links PIE by default); on PE into `$`-sorted chunk
groups of two output sections, `.dw3data` and `.dw3bss`, between generated
empty marker chunks that carry the same symbols (one section per overlay would be 600 PE sections). The post-link
check reads the objects (`objdump -h`), not a link map, so it is the same on both. The PE side moved from objcopy to
`#pragma clang section` (forced into each unit's compile) the same day: GNU objcopy drops the COMDAT flag of the COFF
sections it rewrites, and lld then discards the units' `.pdata`/`.xdata`, the x64 unwind tables the Windows crash
report's stack walk needs. GCC has no such pragma, so ELF keeps the objcopy pass: two paths after all.

# Launcher and mods

## Launcher and mods
_Decided: 2026-10-06_

The launcher is its own thin executable in `launcher/` (C++, Dear ImGui on SDL3; C++ only there, the game stays C): it
edits one JSON settings file and starts the game with `--config FILE`. Mods in v1 are built-in features compiled into
the game, switched and configured through a `mod.json` manifest the launcher renders; data overrides come next, no
third-party code mods. The game never loads a configuration on its own (tests run the bare binary), mods are off under
`--script` unless asked for, a mod's change in the game's C sits in a `PC_PORT` block, and hotkeys never reach the pad.
Linux first; Windows after the launcher works on Linux.

## Crash reports are one text file, named on stderr, kept by the launcher
_Decided: 2026-10-07_

A crash or a fatal stop of the port writes one text report (`psxstack/runtime/crash.c`: the build, the vsync, the overlays, the
log's tail, the registers and a stack of executable-relative addresses) and names it on its last stderr line, which
is the launcher's only contract with it. The launcher owns where things go (`<settings dir>/crashes/`, `logs/`) and
what a tester pastes (its Copy text). Every build is stamped from `git describe` (`port/cmake/version.cmake`) so a
report says which build it is, and a release ships the game's debug info beside the AppImage, so reports from
players symbolize without rebuilding. The frame log and the record never change.

## The settings file
_Decided: 2026-10-06_

The settings file has a `schema` number (1). Errors are strict (a wrong type or range exits 64 naming the key), unknown
keys are logged and ignored, so an older game runs a newer launcher's file. `--print-settings` is the validator the
launcher calls before Play. The launcher keeps members it does not know, writes only what the user changed, and never
writes a file with a newer schema. Under `--config` the watchdog is off and both card slots are files beside the
settings; the bare binary keeps its test defaults.

# Workflow, CI and releases

## CI per area
_Decided: 2026-10-06_

`.github/workflows/ci.yml` runs on pull requests and pushes to `main` only, skips changes that touch only documentation
(`paths-ignore`), and a newer push to a pull request cancels the older run. `scripts/ci_areas.sh` sorts the changed
files into areas, each implying the next: **game** (the build `--check`, every test layer, and everything below),
**port**, **launcher**; an unknown path counts as `game`, and changes to the CI itself run everything. A docs-only pull
request shows no checks; "CI green" means green when a run started. Draft pull requests run nothing: marking one
ready for review starts the run (saves runner minutes on work in progress). The Windows build (2026-10-07) is a second
job of the **port** area, `windows`, beside `build`: the cross-build, the launcher's self-test and the layer-2 replays
under Wine, with its own tool cache; the fork pull requests get its disc-free part.

**Parallel jobs (2026-10-07).** One job doing everything in sequence took 15-20 min (the reference tests 10 of them,
the mods' tests 4), so CI is now an `areas` job and, gated by its outputs, parallel jobs: `check` (disc-free, always:
the scripts parse, the probes), `game` (the PS1 side: smoke test, byte-identical rebuild, layer 2), `port` (layer 1 and
the port layer, with `--m32`), `port-mods` (layer 3 and the mods, the longest test: its own job), `launcher` and
`windows`, ~6 min on the critical path. Jobs, not workflows (one check list, one concurrency group, `release.yml`'s
`workflow_call` unchanged), and no artifacts between them: a port build is 30 s, a job's setup about a minute, so each
job builds what it needs. Every job sets itself up with the composite action `.github/actions/setup`: one tool cache per
job keyed on the pins (`scripts/setup.sh --pins`), not on `setup.sh`'s text (an edit to the script's logic cost two
cold toolchain builds, the pull request's and main's, six times in 27 pull requests); ccache (a pinned static binary,
`setup.sh ccache`) for the host builds of the port and the launcher (a comment-only change recompiles nothing; not
the Windows cross-build, whose compile launcher is the overlay renamer); apt only for gcc-multilib and Wine, cmake and
ninja from the venv instead (the runners' Ubuntu mirror stalled a job's `apt-get install` for 4.5 min). The PS1 rebuild stays from scratch: being byte-identical from nothing is the test, and
it is a minute off the critical path. Inside the scripts, what is independent runs `DW3_JOBS` at a time: the mods
(`tests/port/mods.py`, one process per mod) and the layer-2 replays; CI sets 4, the runners' cores.

## Releases: tagged drafts, published by hand
_Decided: 2026-10-06_

Players get one Linux x86_64 AppImage (the launcher, the game, the mods' manifests) and supply their own disc. A pushed
tag `vX.Y.Z` runs `release.yml`: the whole CI, the port's test on the Release build, the AppImage and its smoke test,
then a **draft** GitHub release. Publishing a release (binaries built from the decompiled code, no game data) is the
owner's explicit decision each time. Built on `ubuntu-24.04`; both programs depend only on the C library. The owner
may build the same draft locally instead (`scripts/release_local.sh`, 2026-10-06: Docker `ubuntu:24.04`, the
appimage job's steps, release.yml's run cancelled), after `main`'s CI is green: ~6 min, not ~30.
Since 2026-10-07 the same draft also carries **the Windows x86_64 zip** (the launcher, the game, the mods' manifests,
licences and a README in one folder) and its PDB symbols zip, cross-built on the same runner by a `windows` job
beside `appimage` (`scripts/package_windows.sh --test`: the unzipped package's self-test and a forced crash under
Wine), so one tag releases both platforms at once; the notes say the Windows build was tested under Wine and Proton
only until real-Windows reports come in. `release_local.sh` builds both too (its container has `wine64`).

## Windows: cross-built from Linux with llvm-mingw, tested under Wine and Proton
_Decided: 2026-10-07_

The Windows build of the game and the launcher is cross-compiled from Linux with **llvm-mingw** (clang + lld, UCRT),
one pinned release tarball that `scripts/setup.sh llvm-mingw` unpacks into `tools/llvm-mingw` (no system package, no
sudo, the same build on CI, Fedora and Ubuntu), with SDL3 cross-built the same way (`sdl3-windows`) and one CMake
toolchain file (`cmake/windows-x86_64.cmake`) for both projects. Not GCC mingw-w64: there is no pinned project-local
build of it short of compiling GCC, and lld writes the PDBs the crash minidumps need. lld takes no linker script, so
the port's ld script went ("Overlay sections by renaming, no linker script"). Everything links statically (no DLL beside the executables),
x86_64 only, Windows 10 or newer. It is tested on Linux: Wine for the automated runs (the replays must give the Linux
build's logs and record hashes; CI), Proton for the play-test by hand; real Windows comes from testers afterwards
(#37).

## Mods extend the save in the slot's unused tail, never in its checksummed bytes
_Decided: 2026-10-06_

The save-anywhere mod needs state the game does not save (the map's layer, depth, per-visit flags). It goes into the
slot's unused tail (slot offset 0x26C4, 0x3C bytes of stale buffer the game writes and reads back with the slot but
never looks at), under its own magic, version and checksum. The slot's 0x26C4 bytes and the header stay exactly the
game's, so every card the port writes still loads on a PS1 or an emulator (as a fresh entry to the map), and a card
from either still loads in the port. A mod that needs more than the tail would need a sidecar file, not a changed slot.

## The hardware renderer: SDL_GPU beside the software GPU
_Decided: 2026-10-07_ (issue #31, where the plan and its measurements are)

The hardware renderer uses SDL_GPU directly (Vulkan now, D3D12 later on Windows), one backend, no OpenGL. It is a
second consumer of the software GPU's decoded command stream, never a replacement: `port/psyq/gpu.c` stays the
reference and the default renderer, and CI, the frame hash, `--screenshot`, the replays and every existing test keep
using it. The renderer's own tests run where a GPU device exists and say so when they skip; CI compiles it and checks
the fallback. Its shaders are HLSL compiled to SPIR-V at build time by the pinned DXC (`scripts/setup.sh dxc`; it also
writes DXIL for D3D12) and embedded; DXC writes the same bytes on every machine, so nothing compiled is committed.
Blending is done in the shader from a copy of the target, not by the fixed-function blender: on this machine NVIDIA's
blender gets the PS1's (B+F)/2 wrong for a quarter of the values, stacked halvings drift on every device, and the
PS1's dither after the blend cannot be expressed at all.

After the play-test (2026-10-08: internal scale 4 through the field and a battle, without a fault) the default stays
`software` (`video.renderer`, internal scale 1): the GPU renderer is a choice in the launcher, not the default.

## The port is split into runtime and game adapter (phase 1 of psxstack)
_Decided: 2026-10-08_

The PC port stack (the runtime, the Psy-Q shim, the launcher, the build and packaging) is being extracted into its own
repository, [psxstack](https://github.com/gascarcella/psxstack), to be reused by ports of other PS1 decompilations;
its `docs/GAME_CONTRACT.md` says what a game provides and what the stack provides. Phase 1, here: `port/runtime/` is
generic (it includes no game header and names no game symbol; its game facts are macros generated from
`port/game/game.json`), `port/game/` is this game's adapter behind `port/include/psxstack/game.h` (`game_main`,
`game_apply_rate`, the `game_state_*` probes and checkpoint image, `game_mods`), with a weak default for each function.
The game's `include/port.h` keeps the PS1 side of every hook macro itself and includes the stack's `psxstack/hooks.h`
only under `PC_PORT`, so the matching build never needs the stack. The overlay manager and the arena take the slot table
from the description (N slots; the macros' `tier` is the slot's 1-based index), so the game's C did not change. The
`DW3_*` CMake names, the `dw3_data_*` section names and `tools/port_gen.py`'s place stay until the move (phase 2).

## The PC port is built on psxstack (2026-10-08)
Phase 2 of the extraction (DECISIONS "The port is split into runtime and game adapter"): the runtime, the Psy-Q
shim, the shaders, the CMake build, the launcher, `tools/port_gen.py`'s generic half, the inventory's generic half and
the MCP tools moved to gascarcella/psxstack. `port/CMakeLists.txt` finds the stack at `psxstack/` (the sibling clone
linked by `scripts/worktree_init.sh`; the submodule in phase 4) and calls `psxstack_add_game()` with this game's
inputs, which `tools/port_inputs.py` writes from the tracked sources (the units, the overlays with their slot, file
ID and symbol file, the known tag sites, the volatile ranges): the stack reads no game C. Its names won where the two
had diverged: the generated header is `psxstack_game_gen.h` from psxstack's `game_gen.py` (`PsxstackGameDisc`,
`PSXSTACK_GAME_ABOUT`), the sections are `dw2003_data_<ovl>` on ELF and `.psxdata$<ovl>` on PE, the CMake options
`PSXSTACK_*`, the hook `port_file_wait_read`. The `DW3_PORT_*` environment variables keep their names (the
description's `env_prefix`). The tests that compile the shim or the runtime on their own (`tests/host/*_replay.py`,
`tests/spu`, `tests/xa`, `tests/port/sound.py`) take them from `psxstack/`; `tests/port/settings.py` reads the
manifests of both `psxstack/mods/` and `port/mods/`.

## The port stack lives in psxstack, consumed as a pinned submodule (2026-10-08)
Phase 4 of the extraction: `psxstack/` is a git submodule of gascarcella/psxstack pinned by commit (`.gitmodules`), no
longer a link to a sibling clone. What a pin buys: this repository's `main` always names the exact runtime, shim and
launcher its tests passed against, a stack change reaches the game as a reviewable pin bump, and other games pin their
own. `scripts/worktree_init.sh` checks the submodule out in a worktree (from the main checkout's copy when it has one;
offline it links the sibling clone and says so, so a session without network still builds). The tools are one tree,
this repository's `tools/` (its `setup.sh` installs SDL3, DXC, ImGui, llvm-mingw, the AppImage tools), handed to the
stack's CMake as `PSXSTACK_TOOLS_DIR`; psxstack's own `setup.sh` is for its own CI and for a stack developer, and its
pins must stay equal to ours. The release's version is `DW3_VERSION` in release.yml and `release_local.sh`, exported
as `PSXSTACK_VERSION` by the packaging scripts for psxstack's `version.cmake`. While psxstack is private, CI and
release.yml fetch it with a read-only deploy key on psxstack stored as the secret `PSXSTACK_DEPLOY_KEY` here (the data
checkout's pattern); a fork pull request has no secrets, so the areas job then skips the port and launcher areas with a
warning and a maintainer's run tests them. **2026-10-08, phase 5:** psxstack is public and tagged `v0.1.0`, the pin
is on the tag, the key, the secret and that gate are gone: the submodule is a plain HTTPS fetch for everyone, fork
pull requests included.
