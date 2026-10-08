# The PC port

How this game runs as a native program on [psxstack](https://github.com/gascarcella/psxstack): what is this game's
(the hooks in its C, the adapter, the build inputs, the tests) and where the rest is. The architecture of the port
runtime, the memory arena, the overlays, the pump, the debug channel, the Psy-Q shim, rendering, sound and the crash
report are psxstack's `docs/PORT.md`; building and running the binary, its options, the per-frame log and the record
formats are psxstack's `docs/RUNTIME.md`; the shim's per-function behaviour is psxstack's `psyq/README.md`; sound is
in `docs/SOUND.md`. The choices behind this design are in `docs/DECISIONS.md` (its PC port entries).

## Overview
- The game's C (`src/`, all 388 units) is compiled for the host with `-DPC_PORT -DNON_MATCHING` and linked with
  psxstack's runtime, this game's adapter (`port/game/`) and psxstack's Psy-Q shim into one 64-bit executable,
  `dw2003`, by `psxstack_add_game()` (`port/CMakeLists.txt`).
- psxstack is the sibling clone linked at `psxstack/` (`scripts/worktree_init.sh`), the submodule from phase 4 on.
  It holds no fact about this game: everything it needs comes from `port/game/game.json` (the description) and the
  files `tools/port_inputs.py` writes at configure time (below).
- No emulator is linked and no Sony code is used: the game code runs natively, and the PS1 hardware the game reaches
  through Sony's libraries is reimplemented in C behind the shim. PCSX-Redux serves only as an external test oracle.
- The PS1 matching build is untouched: every port change in `src/` or `include/` is either a hook macro from
  `include/port.h` that expands to the original code without `PC_PORT`, or an `#ifdef PC_PORT` block.
  `scripts/build.sh --check` proves the EXE and overlays byte-identical after each one.
- The default build is headless (what the tests run); `-DPSXSTACK_SDL=ON` adds the SDL3 window, input and audio device.
- The game reads the user's own disc image (BIN/CUE, SHA-1 checked); no game data is in the repository.

## Source layout
| Path | Role |
|---|---|
| `include/port.h` | The hook macros' PS1 side (the original code), and under `PC_PORT` the game's own hooks (`WSTAG_ENTRY`, `OVERLAY_ENTRY`, the mods' flags) over the stack's `psxstack/hooks.h` |
| `port/CMakeLists.txt` | The port's CMake project: runs `tools/port_inputs.py`, then `psxstack_add_game(dw2003 ...)`; builds out of tree into `build/port*/` |
| `port/game/game.json` | The game's description (identity, the EU disc, the rate, the two slots, the heap, the BIOS stand-in), generated into `psxstack_game_gen.h` at configure time (psxstack's `tools/game_gen.py`): the runtime's only source of game facts |
| `port/game/state.c`, `game.c` | The adapter (psxstack's `include/psxstack/game.h`): the probes (`game_state_*`), the checkpoint image, `game_state_read`/`game_state_host` (the layout-identical ranges and the field tables), `game_apply_rate` (the 60 Hz mode) |
| `port/game/game_mods.c`, `battle_scan.c` | The six mods that change the game (`PortMod` records for psxstack's engine), the battle scripts' scanner |
| `port/game/asmdata.c` | Weak stand-ins for the data the matching build keeps in asm, in FIELDSTG's section |
| `port/mods/` | The game's mods' manifests (psxstack's `mods/fast_forward` joins them beside the binary) |
| `tools/port_inputs.py` | The build inputs psxstack takes: the units, the overlays (slot, file ID, symbol file), the known tag sites, the volatile ranges |
| `tools/port_inventory.py` | psxstack's inventory configured for this tree (`counts`, `probe`, `link`) plus this game's `structs` and `object-sizes` |
| `tools/mcp_game.py`, `.mcp.json` | The debug tools' configuration: the MCP server's arguments, the Python module the tests use |
| `psxstack/` | The stack (untracked here: the sibling clone, linked by `scripts/worktree_init.sh`; the submodule from phase 4) |

## Compiling the game C for the host
The flags, the override headers and the host-compile gate are psxstack's (`docs/PORT.md` "Compiling the game C for
the host"). This game's part:
- **`INCLUDE_ASM`** is empty on the host. No game function is left in asm (the last 8 holdouts matched on
  2026-10-07), so the port compiles the same matching C as the PS1 build. `tools/hacks.py --check` keeps it so (no
  `INCLUDE_ASM` or `NON_MATCHING` in game code).
- **FAKE matches** (`grep -rn "FAKE:" src`) are valid C and compile as they are.
- **`gte_*` macros** (`include/psyq/gtemac.h`) are translated onto the software GTE at configure time (`GTEMAC`).
- **The gate:** `tools/port_inventory.py probe` and `link` (in `scripts/test.sh`, CI). The game's remaining host
  warnings (missing returns, `-Wmissing-braces`, ...) are in the matching C and left as they are.
- **Undefined behaviour the PS1 tolerated** is fixed in the C under `PC_PORT` or in a form that keeps the PS1 bytes,
  as the sanitizer runs find it. `tests/host/FINDINGS.md` lists each case (for example the first battle's end computing
  `next() % 0`, which does not trap on the R3000A but raises SIGFPE on x86). In-struct overruns the game relies on
  are named in `tests/port/ubsan.supp` and nowhere else.

## Hook macros (`include/port.h`)
`common.h` includes `port.h`; no unit includes it directly. Each macro is the original code without `PC_PORT`; with it,
`port.h` includes the stack's `psxstack/include/psxstack/hooks.h`, where the host side lives (the PS1 build never
needs that path). `tier` is a slot's 1-based index in `port/game/game.json`'s `memory.slots`:

| Macro | Where the game uses it | Host meaning |
|---|---|---|
| `PLATFORM_WAIT()` | The body of the 7 busy-waits that only an interrupt can end (`gfx_end_frame`, `cdload`, `main` and STDWTITL's `CdControl` loops, FIGHTSTG's `IsIdleGPU`) | `port_wait()`: runs the pending vsync/CD/GPU "interrupts" |
| `PLATFORM_HALT()` | FIELDSTG's deliberate endless loop (no stage for the map) | `port_halt()`: reports the place and exits |
| `PORT_SCRATCHPAD_STACK_ENTER/LEAVE` | `heap_run_object`'s switch to a 1 KB stack in the scratchpad (the only raw `$sp` asm) | Nothing: the normal stack |
| `OVERLAY_COPY(tier, file, dst, src, size)` | The two overlay `memcpy`s (`overlay_load_stage`, `overlay_load_file`) | `port_overlay_load()` (see "Overlays") |
| `OVERLAY_ENTRY`, `WSTAG_ENTRY`, `SLOT_FUNC` | Functions at fixed addresses in a slot: 20 tier-1 entries, 293 WSTAG entries, 67 other slot functions | A *tag*: the PS1 address kept as an integer in a function pointer (valid in static initializers) |
| `OVERLAY_FN(tier, fn)` | Every call through a pointer that may hold a tag | `port_overlay_resolve()`: a tag becomes the current overlay's host function; a real pointer passes unchanged |
| `LATE_FUNC` / `LATE_CALL` | Calls by name into whatever overlay is loaded (`gamestate.c` into FIELDSTG, FIELDSTG into the loaded WSTAG) | Resolved by address through the current overlay's table |
| `SLOT_PTR(tier, type, addr)` | Data in a slot: FIELDSTG's built-in stage's event scripts, `main_overlay_base`, `main_file_base` | The same offset into the arena's slot buffer |
| `HEAP_START/END/SIZE_FROM/ADDR` | The heap's bounds (`heap.c`, `records.c`) and FIELDSTG's `free_above` constant | The arena's heap region |
| `PTR_ADD(type, ofs, base)` | Offset-table resolves, written on the PS1 as `ofs + (s32)base` | Pointer arithmetic |
| `PTR_TO_S32` / `S32_TO_PTR` | A pointer kept in an `s32` field or argument | The pointer's PS1-style address in the arena, and back (fatal outside the arena) |
| `PTR_TO_U32(p)` | The 24-bit ordering-table tags (`setaddr`, `gfx_compact_ot`) | The pointer's word offset in the tag window (the game's data and the arena; fatal outside it) |
| `BIOS_PTR(type, addr)` | STAGSLCT's read of the BIOS version string | A stand-in region holding `game.json`'s `bios_standin` texts (`arena.c`) |

`tools/port_inventory.py counts` lists every use of every macro, and every fixed PS1 address left in the C (all are
wrapped). The mods' flags (`port_mod_skip_dialogues`, `port_mod_battle_animations`, `port_battle_cut`) are also
declared there; they are read only inside `#ifdef PC_PORT` blocks.

**The adapter** (DECISIONS "The port is split into runtime and game adapter"; psxstack's `GAME_CONTRACT.md` "4"): the
runtime reaches the game only through `psxstack/game.h` (`game_main`, `game_apply_rate`, the `game_state_*` probes and
checkpoint image, `game_state_read`/`game_state_host` for PS1 addresses outside the arena, `game_mods`), implemented
in `port/game/`. The probes read `overlay_module.stage`/`.file`, `gamestate_data.map`, `pad_random.index` and the
field player's position from the host's objects; `game_state_read` maps a PS1 address only inside a layout-identical
range (a whole data symbol whose `-m64` size is its PS1 size, or the prefix `state.c` lists for a pointer-bearing
object: `overlay_module` 8 bytes, `gamestate_data` 0x26FC, `pad_random` 4, each checked by `_Static_assert`) or in
the explicit field tables of `memcard_state` and FIELDSTG's `fieldstg_stage`. The checkpoint image is
`gamestate_data`'s first 0x26FC bytes, then its 24 `funcs` entries as the PS1 addresses of the host functions they
point to: 0x275C bytes, the emulator's dump.

## The build inputs (`tools/port_inputs.py`)
`port/CMakeLists.txt` runs it at configure time into `build/port/gen/inputs/`, and passes the files to
`psxstack_add_game()` (psxstack's `GAME_CONTRACT.md` "5"):
- `units.txt`: the 388 units, `src/<target>/*.c` for the EXE and the tier-1/tier-2 overlays, `src/wstag/<unit>.c`
  for the WSTAG files of `config/wstag_c.txt` (the same set `configure.py` compiles; it fails if a C file under
  `src/` is not one of them), each with its overlay (`MAIN` for the EXE's). WSTAG260 is data-only and has no unit.
- `overlays.txt`: the 19 stage overlays (slot 1), WFIGHTMN/WFIGHTTS and the 293 WSTAG files (slot 2), with the file
  ID each is loaded by (tier 1 from `overlay_files` in `src/main/overlay.c`, WFIGHTMN/WFIGHTTS 0x208/0x209, the WSTAG
  files from FIELDSTG's stage tables joined with `config/wstag.txt`) and its symbol file (`config/<overlay>.symbols.txt`,
  `config/wstag/<n>.symbols.txt`).
- `tag_sites.txt`: the tag sites whose overlay this game knows (`WSTAG_ENTRY` by the record's file, `OVERLAY_ENTRY` by
  its comment, `LATE_FUNC` tier 1 = FIELDSTG), which psxstack checks against that overlay's table.
- `volatile.txt`: `tests/replay/replay.py`'s `VOLATILE_RANGES`, one definition for the emulator's records and the port's.
The EXE's symbol file (`config/symbol_addrs.txt`) and `include/psyq/gtemac.h` go to psxstack as they are.

## The Psy-Q shim: what this game uses
psxstack's shim implements the **123 Psy-Q functions** the game C calls (`tools/port_inventory.py counts`;
`psxstack/psyq/check.sh --game-root .` checks that each, and the SDK data symbols the C uses, is defined) against the
prototypes in `include/psyq/`.

| Library | Game's use | In the port |
|---|---|---|
| LIBGPU | 24 functions, ordering tables, environments, VRAM transfers | Real; drives the software GPU (`libgpu.c`, `gpu.c`) |
| LIBGTE | `rsin`/`rcos`, rotation/scale matrices, geometry set-up | Real, on the software GTE (`libgte.c`, `gte.c`) |
| LIBGS | TIM info, the GTE set-up, the world-screen and light matrices FIGHTSTG uses | `GsGetTimInfo`, `GsSetProjection`, `GsSetRefView2`, `GsGetLw`, `GsSetFlatLight` real (the matrices and the GTE's H and LCM as the PS1's, the layer-1 family `libgs_view`); `GsInitGraph`/`GsInit3D` do the PS1's GTE and matrix set-up and skip its draw environments (the game draws with its own) |
| LIBETC | `VSync`, `VSyncCallback`, `SetVideoMode`, `ResetCallback` | Real, on the pump |
| LIBCD | `cdload`'s interrupt-driven sector reads, movie streaming (`St*`, `CdRead2`) | Real command model over the disc image, timed in ticks; XA sectors to the XA decoder (`libcd.c`, `xa.c`) |
| LIBPRESS | MDEC movie decoding (STDWTITL) | Real (`libpress.c`, `mdec.c`) |
| LIBSND | All the game's sound (`sound.c`), the sequencer ticked from vsync | Real, over the SPU core (`libsnd*.c`; `docs/SOUND.md`) |
| LIBPAD | Pad init, state, modes, actuators (`pad.c`) | A digital pad on port 0; actuator calls accepted and ignored |
| LIBMCRD | Memory card access (`memcard.c`, STGMCARD) | Real, over `.mcd` images (`libmcrd.c`) |
| LIBC2, LIBAPI | String/memory functions; SHOCKTST's `sim:` host files | Not defined: they resolve to the host libc (`open("sim:...")` fails, as SHOCKTST expects) |

`DW3_PORT_TRACE=1` traces every shim call.

## Testing
| Check | What it proves | Where |
|---|---|---|
| Byte-identical PS1 build | No hook or `#ifdef PC_PORT` changes a PS1 byte | `scripts/build.sh --check` (CI) |
| Host-compile gate | Every unit compiles at `-m64` with pointer/int casts and implicit declarations as errors; no duplicate global | `tools/port_inventory.py probe`, `link` (in `scripts/test.sh`, CI) |
| Shim coverage | Every Psy-Q function the game calls is defined, nothing twice | `psxstack/psyq/check.sh --game-root .` |
| Layer-1 goldens on the host | GPU (715 cases), GTE (865), LIBGS's view (230: the battle camera), MDEC, XA and other families recorded in the emulator replay byte-for-byte through the shim's C; known GPU differences in `tests/host/known_mismatches.json` | `tests/golden/`, `tests/host/replay.py`, `tests/xa/` |
| The battle's VRAM | The first battle's textures and CLUTs equal to the emulator's (its frames cannot be compared: the PS1 is CPU-bound in battle) | `tests/port/vram.py` |
| The port against the emulator | `new_game` and `first_battle_save` replayed by the port: two runs byte-identical, `-m32` equal to `-m64`, no ASan/UBSan report, and the emulator's cross-core view (checkpoint stable hashes, overlay and map sequences) | `tests/port/run.py [--m32] [--sanitize]`, the `port` layer of `scripts/test.sh` |
| Sound | LIBSND's SPU writes against the emulator's trace on its timeline; SPU unit goldens | `tests/port/sound.py`, `tests/spu/`, `docs/SOUND.md` |
| Saves | Port and emulator load each other's saves (layer 3) | `tests/saves/run.py` |
| 60 Hz | The port's 60 Hz mode against the NTSC-patched game in the emulator | `tests/port/hz60.py` |
| Settings, mods, input | Settings round trip; mods keep the emulator's stable hashes; the battle-script scanner on every script on the disc; `--input-test` | `tests/port/settings.py`, `mods.py`, `battle.py`, `dw2003 --input-test` |
| Debug channel | One `--debug` run to CNTY_SEL: step and pad advance the frame exactly, the state map and the host symbol read the same, poke/peek, screenshot, hash, quit status; `tools/mcp`'s offline self-test | `tests/port/debug.py` (the `port` layer), `psxstack/tools/mcp/selftest.py` |
| Hardware renderer | Its picture byte for byte the software image, its present at six output sizes against the reference, SDL_Renderer's present against the same reference; its rasteriser's whole VRAM target equal to the software VRAM every 10 vsyncs of both replays and after each of the gpu golden family's 715 lists; the fallback without a device (CI); everything else needs a GPU device (local, or lavapipe) | `tests/port/render_gpu.py`, `tests/host/gpu_hw_replay.py` (the `port` layer) |
| Save states | `first_battle_save` resumed from its `battle_start` state: the straight run's record, its log after the saved frame and its audio, in the `-m64`, `-m32` and sanitizer builds (and the Windows build under Wine); the debug channel's save, 30 vsyncs, load, the same 30 vsyncs: the same hash and picture | `tests/port/savestate.py [--m32] [--sanitize]` (the `port` layer), `tests/port/debug.py` |
| Texture dump | `--dump-textures` changes neither the log nor the record; new_game's keys are `tests/port/textures/new_game.keys`, each PNG of its key's size and format; a second run continues the dump; the same keys with the rasteriser (with a device) | `tests/port/textures.py` (the `port` layer, CI) |
| Crash report | A forced NULL write dies of SIGSEGV with a report whose pc symbolizes to the hook's function; a fatal error's report; `--version` | `tests/port/crash.py` (the `port` layer) |

What the port and the emulator are *not* compared on: frame numbers (the port's CD timing and CPU time differ), the
random index and the full checkpoint hash (both follow the frame count). See `tests/port/README.md`.

Play-tests on a desktop (window, gamepads, audio device, real time) cover what the headless tests cannot. Their
findings are filed as issues.

## Save states
psxstack's save states (its `docs/PORT.md` "Save states", `docs/RUNTIME.md` "Save states") let a test or a session
start anywhere the game has been: a state is the whole machine at the end of a vsync, loadable by **the binary that
saved it** (a rebuild makes it invalid; its header names the binary). A state holds the game's data: it is generated
from the disc into `build/states/` and never committed. The first battle (`first_battle_save`'s `battle_start`:
FIGHTSTG loaded, about vsync 19760) is the fixture the tests and the renderer work use:
```sh
# the state of this binary at battle_start (made in ~10 s headless when missing; cached per binary in build/states/<sha1>/)
tools/venv/bin/python tests/port/savestate.py path --exe build/port-sdl/dw2003
# play or debug from it (any options of a normal run; a script given with it starts at the loaded frame)
build/port-sdl/dw2003 --disc iso/dw2003.cue --window --renderer gpu \
    --load-state "$(tools/venv/bin/python tests/port/savestate.py path --exe build/port-sdl/dw2003)"
# by hand: any checkpoint of a script, or a frame number, then exit
build/port/dw2003 --disc iso/dw2003.cue --script tests/replay/scripts/first_battle_save.json \
    --save-state battle_start:build/states/battle_start.state --save-state-exit
```
The adapter's part is `game_savestate` (`port/game/game_mods.c`): what the mods carry between vsyncs (save-anywhere's
map, party experience's knocked-out slots, XP boost's tenths). The debug channel's `save_state`/`load_state` (MCP:
`state_save`, `state_load`) use the same files.

## Known limitations
- **Coverage of the game:** the replays reach 14 of 19 tier-1 overlays and a small share of the WSTAG files; the
  debug overlays (STAGSLCT, SOUNDTST, SHOCKTST) and WFIGHTTS cannot be reached by pad input.
- **Windows (tested under Wine and Proton, not yet on real Windows):** the Windows cross build (DECISIONS "Windows:
  cross-built from Linux"): `scripts/setup.sh llvm-mingw sdl3-windows`, psxstack's CMake toolchain file
  `psxstack/cmake/windows-x86_64.cmake`, `scripts/build_windows.sh [--launcher] [--test]` (the game into `build/port-win`,
  the launcher into `build/launcher-win`, the launcher's self-test under Wine) and `tools/port_inventory.py probe
  --target windows` (the units through llvm-mingw's clang). `dw2003.exe` links (the overlay sections need no linker
  script, see "Overlays") and, run under Wine headless (`tests/port/run.py --exe build/port-win/dw2003.exe --wine`),
  replays `new_game` and `first_battle_save` with the frame log, the record and the SPU trace byte for byte equal to
  the Linux build's. CI's `windows` job (the port area, beside `port`) does the same on ubuntu-24.04 with Ubuntu's
  Wine 9.0: the cross-build, the launcher's self-test under Wine (`DW3_SELFTEST_VIDEO_DRIVER=offscreen`: Wine drops
  `SDL_VIDEO_DRIVER` from the Windows environment), the crash report under Wine (`tests/port/crash.py --wine`), and
  with the disc both replays; `build`'s probe step compiles every unit with llvm-mingw's clang too. Locally the same
  gate is `scripts/build_windows.sh --test`, every Wine run under a timeout. The release carries a Windows zip
  (`scripts/package_windows.sh`, `docs/RELEASE.md` "The Windows package"). Play-tested on this desktop with the
  packaged `dw2003.exe` under Wine 11.17 and GE-Proton 10-25 (umu-run): a window at scale 2, WASAPI audio with no
  refill or drop, 49.97 frames a second, the `new_game` route to the first field map. Nothing has run on real Windows
  yet (the board's Windows 9); the SDL window is 64-bit only; macOS
  is not planned. The launcher links and passes its self-test under Wine. The runtime's operating-system calls are in one
  file with a POSIX and a Windows half, `psxstack/runtime/platform.c` (`platform.h`: paths with drive letters
  and `\`, a replacing rename for the memory cards and the stamp cache, the per-user cache directory, positional
  reads of the disc image, a monotonic clock and a high-resolution sleep for the pace, the watchdog as a thread), the
  frame log, the record and the SPU trace are written in binary mode (the same bytes on both), stderr is unbuffered
  on Windows (UCRT has no line buffering), the console reset's `setjmp` takes no SEH frame on mingw (`port_setjmp`),
  and `--debug` is refused there (the channel is a Unix socket). The Windows executable is a GUI-subsystem program
  (no console window behind it when the launcher starts it; stderr still reaches the launcher's pipe) with a manifest
  (psxstack's `windows/` templates: the UTF-8 code page, long paths, per-monitor DPI). A crash writes the same report as on Linux
  plus a minidump ("Crash report" above; `tests/port/crash.py --wine` checks both under Wine).
- **`long` on Windows (LLP64) was audited (2026-10-07):** `long` is 32-bit there, 64-bit on Linux x86_64. The game's
  structs and headers use the sized types (`s32`, `u32`, `s64`); the `long`s left are Psy-Q prototypes (`CdRead2`,
  `MemCardInit`) and counters and option values in `port/runtime` (`port_frames`, `port_max_frames`, the pace, the step
  counts), none of which holds a pointer or a byte count over 2 GB. The pointer-to-`unsigned long` casts went: the
  shim's `PSYQ_PTR` and `VSyncCallback` use `uintptr_t`, and the three `bzero` offsets of `gfx.c`, `pad.c` and
  `gamestate.c` use `OFFSETOF` (the PS1 bytes unchanged). The `-m32` build, where `long` is 32-bit too, replays with
  the same log as the 64-bit build, which brackets LLP64 between the two.
- The generic ones (one digital pad, no reset key, the CD timing stand-ins, the BIOS stand-in, sanitizer builds'
  section sizes) are psxstack's `docs/PORT.md` "Known limitations".
