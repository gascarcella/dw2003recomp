# The PC port

How the decompiled game runs as a native program: the architecture, the PS1 assumptions it handles, how it is tested,
and what it does not do yet. Building and running it, the command-line options, the per-frame log and the record
formats are in `port/README.md`; the shim's per-function behaviour is in `port/psyq/README.md`; sound is in
`docs/SOUND.md`. The choices behind this design are in `docs/DECISIONS.md` (its PC port entries).

## Overview
- The game's C (`src/`, all 388 units) is compiled for the host with `-DPC_PORT -DNON_MATCHING` and linked with the
  port runtime (`port/src/`) and our own Psy-Q shim (`port/psyq/`) into one 64-bit executable, `dw2003`.
- No emulator is linked and no Sony code is used: the game code runs natively, and the PS1 hardware the game reaches
  through Sony's libraries (GPU, GTE, SPU, MDEC, CD drive, pads, memory cards) is reimplemented in C behind the shim.
  Emulators (PCSX-Redux) serve only as external test oracles.
- The PS1 matching build is untouched: every port change in `src/` or `include/` is either a hook macro from
  `include/port.h` that expands to the original code without `PC_PORT`, or an `#ifdef PC_PORT` block.
  `scripts/build.sh --check` proves the EXE and overlays byte-identical after each one.
- The default build is headless (what the tests run); `-DDW3_PORT_SDL=ON` adds the SDL3 window, input and audio device.
- The game reads the user's own disc image (BIN/CUE, SHA-1 checked); no game data is in the repository.

## Source layout
| Path | Role |
|---|---|
| `include/port.h` | The hook macros (both sides) and the `port_*` interface the game C calls |
| `port/src/` | The runtime: `main.c` (options, setup), `arena.c`, `overlay.c`, `pump.c`, `reset.c`, `disc.c`, `memcard.c`, `video.c`, `input.c`, `audio.c` and `spu*.c`, `script.c`, `framelog.c`, `state.c`, `settings.c`, `mods.c` |
| `port/psyq/` | The Psy-Q shim: one file per library, plus the hardware models `gpu.c`, `gte.c`, `mdec.c`, `xa.c` |
| `port/CMakeLists.txt` | The port's own CMake project; builds out of tree into `build/port*/` |
| `tools/port_gen.py` | Generators CMake runs: the unit list, the override headers, the ld script, the overlay and state tables |
| `tools/port_inventory.py` | The inventory of what the game C needs from the PS1 (`counts`), the host-compile gate (`probe`, `link`, `structs`, `object-sizes`) |

## Compiling the game C for the host
- **Flags:** C99 with GNU extensions (`gnu99`: unprototyped `f()` declarations are common in the game C and C23 would
  read them as `(void)`), `-fsigned-char` (the code relies on signed `char`), `-fwrapv`, `-fno-strict-aliasing`,
  `-fno-pie`/`-no-pie` (the arena needs a fixed link address; see "Memory arena").
- **`INCLUDE_ASM`** is empty on the host. No game function is left in asm except the 8 holdouts, and with
  `NON_MATCHING` their WIP C is compiled instead: the WIP C is the port's code. `tests/holdouts/run.sh` validates it
  by running a `NON_MATCHING` PS1 image through the replays (see "Testing").
- **FAKE matches** (`grep -rn "FAKE:" src`) are valid C and compile as they are.
- **`gte_*` macros** (`include/psyq/gtemac.h`, MIPS `cop2` sequences) are replaced at build time by a generated header
  that turns each sequence into the same register accesses on the software GTE (`tools/port_gen.py overrides`;
  see "GTE").
- **The host-compile gate:** `tools/port_inventory.py probe` compiles every unit at `-m64` with
  `-Werror` on pointer/integer casts, `int-conversion`, implicit declarations and incompatible pointer types; `link`
  checks the objects for duplicate globals. `scripts/test.sh` and CI run both. The game's remaining host warnings
  (missing returns, `-Wmissing-braces`, ...) are in the matching C and left as they are.
- **Undefined behaviour the PS1 tolerated** is fixed in the C under `PC_PORT` or in a form that keeps the PS1 bytes,
  as the sanitizer runs find it. `tests/host/FINDINGS.md` lists each case (for example the first battle's end computing
  `next() % 0`, which does not trap on the R3000A but raises SIGFPE on x86). In-struct overruns the game relies on
  are named in `tests/port/ubsan.supp` and nowhere else.

## Hook macros (`include/port.h`)
`common.h` includes `port.h`; no unit includes it directly. Each macro is the original code without `PC_PORT`:

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
| `PTR_TO_U32(p)` | The 24-bit ordering-table tags (`setaddr`, `gfx_compact_ot`) | The low 32 bits; the low 24 are the arena offset |
| `BIOS_PTR(type, addr)` | STAGSLCT's read of the BIOS version string | A 256-byte stand-in region (`arena.c`) |

`tools/port_inventory.py counts` lists every use of every macro, and every fixed PS1 address left in the C (all are
wrapped). The mods' flags (`port_mod_skip_dialogues`, `port_mod_battle_animations`, `port_battle_cut`) are also
declared there; they are read only inside `#ifdef PC_PORT` blocks.

## Memory arena
One `.bss` block, `port_arena`, stands for the PS1 RAM from the tier-1 slot up (`port/src/arena.c`; sizes generated
into `port_arena_gen.h` by `tools/port_gen.py`):

| Region | PS1 address | Size | Symbols |
|---|---|---|---|
| Tier-1 slot | `0x80082CB0` | `0x23130` | `port_slot1` |
| Tier-2 slot | `0x800A5DE0` | `0x5A20` | `port_slot2` |
| Heap | `0x800AB800` | 4 MB (the PS1's is 1.3 MB; 64-bit runtime structs are larger) | `port_heap_start`, `port_heap_end` |

- The block is **16 MB-aligned and smaller than 16 MB**, so the low 24 bits of any arena pointer are its offset in
  the arena. A pointer's PS1-style address is `0x80082CB0 + offset` (`port_ptr_to_s32`).
- Everything `heap_funcs` hands out (objects, packet buffers, ordering tables, the file cache) lives in the heap
  region, so every primitive and ordering table is in the arena.
- The alignment is a property of a non-PIE ELF linked with GNU ld; `arena.c` checks it at startup. A non-PIE `.bss`
  also keeps the arena below 4 GB, which one remaining `s32`-typed field (`FieldstgEventDef.start`) still relies on.
- The slot buffers hold data files loaded into a slot (WSTAG260, FIELDSTG's data files) and the targets of
  `SLOT_PTR`; code overlays are linked in, not copied (see "Overlays").

## Ordering tables on 64-bit
- PS1 primitives start with a tag whose address field is 24 bits (`P_TAG.addr`). The game sets it through
  `setaddr`/`addPrim` and `gfx_compact_ot` compares `ptr & 0xFFFFFF` with tags; `ClearOTagR` terminates with
  `0xFFFFFF`.
- Because the arena is 16 MB-aligned, this code works unchanged on the host: a tag's low 24 bits are the arena
  offset, and the shim's `DrawOTag`/`ContinueDraw` follow a link as `(ot & ~0xFFFFFF) + (tag & 0xFFFFFF)`, inside the
  window `psyq_set_arena` gives (the heap by default).
- Primitive layouts therefore stay PS1-sized; no primitive type is widened.

## 64-bit layout
- **Runtime structs grow** on 64-bit (any struct holding a pointer). Code that hard-coded a PS1 size now uses
  `sizeof`: object allocations, heap allocations and `bzero`s. The literal sizes left are true byte counts, each
  marked `PC_PORT:` (`tools/port_inventory.py counts` lists them; `object-sizes` keeps the object data blocks from
  regressing).
- **Disc formats are unaffected:** the structs that describe disc data are pointer-free or are compiled from C (WSTAG
  and EXE tables), so they re-lay themselves out.
- **Save data is pointer-free:** a save slot is the first `0x26C4` bytes of `gamestate_data`; every pointer field
  comes after it. Cards written by the port and by the PS1 (or an emulator) are byte-compatible.
- **The `-m32` build is the layout oracle:** pointers are 4 bytes there, as on the PS1, and the per-frame log holds no
  host address, so a run whose log differs between `-m32` and `-m64` has a pointer-size bug on one side.
  `tools/port_inventory.py structs` compares struct sizes at both widths with the documented PS1 sizes.

## Overlays
All 19 tier-1 overlays, WFIGHTMN/WFIGHTTS and the WSTAG files are **linked statically** into the binary (the naming
convention's module prefixes leave no duplicate global). The overlay manager (`port/src/overlay.c`) stands in for the
PS1's copy into the slot:

- **Data reset:** on the PS1 every load also resets the overlay's `.data`/`.bss`, because the file contains them.
  The units are compiled with `-fdata-sections`, and a generated GNU ld script puts each overlay's (and the EXE's)
  writable sections into `.dw3.data.<ovl>`/`.dw3.bss.<ovl>` with start/stop symbols. The manager snapshots them at
  startup and restores them when `OVERLAY_COPY` loads a file, under the same "a different stage/file" condition the
  game checks. A post-link check (`port_gen.py sections`) fails the build if any writable game section lies outside
  those ranges.
- **Current overlay per tier:** a code file becomes its tier's current overlay; a data file (no table) is copied into
  the slot buffer, exactly as the PS1's `memcpy` would.
- **Address tables:** generated after compilation from `config/<overlay>.symbols.txt` and
  `config/wstag/<n>.symbols.txt`, keeping each function that `nm` finds as a global. The generator checks every tag
  site in the C (`WSTAG_ENTRY`, `OVERLAY_ENTRY`, `SLOT_FUNC`, `LATE_FUNC`) and fails the build on one that does not
  resolve. At run time `port_overlay_resolve` looks a tag up in its tier's current overlay; a tag the current
  overlay does not define is fatal.
- **Copy time:** the PS1's byte-loop `memcpy` takes measurable time (FIELDSTG's 0x19000 bytes are about 1.8 frames),
  and the emulator's checkpoints can observe the state in between. The manager runs `size * 12 / 677376` vsync
  ticks after a copy to keep that order.
- **WSTAG260** is data only and has no unit; it is loaded raw.

## Interrupts, the pump and timing
- The port is **single-threaded and deterministic**. A frame is one vsync tick, run from the game's `VSync()`, from
  `PLATFORM_WAIT()` (`port_wait`), or from LIBCD's `StGetNext` once per 5000 empty polls (the movie player spins with
  no wait hook).
- Each tick renders the vsync's audio, runs the game's vsync callback (frame counters, play time, the display flip,
  LIBSND's sequencer tick), then `port_frame` (`port/src/pump.c`): the CD tick, the frame log, the window's input or
  the script's step, screenshots and the window's present.
- **CD timing** is modelled in vsync ticks (`--cd-speed realistic`: 3 sectors per tick at double speed after a seek;
  `instant`: no seek, up to 75 per tick).
- **Real time** applies only with a window: vsyncs are paced to the nominal rate against `CLOCK_MONOTONIC`. Pacing
  changes only the time between vsyncs, so a window run's log and record equal the headless run's.
- **Frame rate:** PAL 50 Hz by default. `--refresh 60` sets the game's own 60 Hz mode (the NTSC patch's
  `records_60hz`) with the pace, audio and CD rates to match.
- **Watchdog:** `--watchdog SEC` exits when no `port_wait()` ran for that long (a loop no hook reaches).
- **Pause:** the pump can hold the game between two vsyncs (the window's pause key; the debug channel's pause, step
  and wait). Nothing of it reaches the game, the log or the record.
- **Debug channel** (`--debug SOCKET`): a tool drives the running game between two vsyncs (below).
- **Console reset** (`port/src/reset.c`): a script's `reset` step longjmps from the vsync tick back to `main()`,
  restores every game section from the startup snapshot, zeroes the arena and resets the shim, then runs the game's
  `main()` again. `DW3_PORT_RESET_CHECK=1` verifies the restore.

## Debug channel and the MCP server
`--debug SOCKET` (`port/src/debug.c`, whose header comment is the protocol, v1) opens a Unix stream socket of
newline-delimited JSON requests (`{"id", "op", ...}`), answered in order, on which a tool drives and inspects the
running game, headless or in a window. Without the option nothing of it exists (`port_frame` pays one branch): the
bare binary, the replays and the goldens are unchanged. The ops:
- `status`: frame, stage, file, map, paused, pace, pad owner. `pause` / `resume`: hold the game at the next vsync
  boundary (the pump's pause, shared with the window's pause key). `step frames`: exactly N vsyncs, then pause.
- `wait`: run until a host or PS1 read, the stage or the map equals a value, or a timeout in frames; leaves the game
  paused. `pad buttons frames release [sync]`: the channel owns pad 1 (`psyq_pad_set` each vsync) until `pad_free`;
  a script keeps precedence.
- `peek` / `poke`: raw host memory, any range `/proc/self/maps` says is mapped (a bad address never faults the game).
  `peek_ps1` / `poke_ps1`: a PS1 address, the arena (from `0x80082CB0`) directly at any length, anything else through
  the state map (`port_state_read`'s layout-identical objects, 1/2/4 bytes).
- `screenshot path` (the display image as a binary PPM), `hash` (`gamestate_data`'s PS1 image, as a checkpoint hashes
  it), `pace fps`, `reset` (the console reset, after the answer), `quit status`.

The game thread polls the socket itself: once per vsync from `port_frame` (after the script's step, before the video)
and 50 times a second while the pump holds it paused. So every command runs between two vsyncs, reads and writes are
frame-consistent, and a driven run is as deterministic as a scripted one (the same presses at the same frames give the
same log and record). A deferred op (`step`, `wait`, `pad` with sync) is answered when it completes, and nothing else is
read meanwhile. `--debug` turns the watchdog and the default frame cap off; a client that disconnects frees the pad and
resumes the game. `--debug-hold` holds the game paused at its first vsync until the client resumes it, so a run is
reproducible from frame 1 (without it an unthrottled headless game is past the boot by the time the client connects).

`tools/mcp/` (`tools/mcp/README.md`) is the MCP server on top of it, registered for Claude Code by `.mcp.json` at the
root: `game.py` is the plain client (no MCP dependency; `Game.spawn`, one method per op), `symbols.py` adds names
(host addresses from `nm` on the ELF, PS1 addresses from `config/symbol_addrs.txt` and the overlays' tables:
`mem_read("ps1:gamestate_data+8")`), `server.py` the tools (`game_start`, `pad_press`, `wait_stage`, `mem_read`,
`screenshot` as a PNG image, `state_hash`, ...). `fake_game.py` is the protocol double for `selftest.py`.

## The Psy-Q shim
`port/psyq/` implements the **123 Psy-Q functions** the game C calls (the count is `tools/port_inventory.py counts`;
`port/psyq/check.sh` checks that each, and the SDK data symbols the C uses, is defined) against the prototypes in
`include/psyq/`. It is our own code, MIT, written from the game's use of the API and public hardware documentation
(psx-spx). The libraries' internals (`_spu_*`, `_card_*`, ...) are never needed: only what the game calls.

| Library | Game's use | In the port |
|---|---|---|
| LIBGPU | 24 functions, ordering tables, environments, VRAM transfers | Real; drives the software GPU (`libgpu.c`, `gpu.c`) |
| LIBGTE | `rsin`/`rcos`, rotation/scale matrices, geometry set-up | Real, on the software GTE (`libgte.c`, `gte.c`) |
| LIBGS | TIM info, the world-screen and light matrices FIGHTSTG uses | `GsGetTimInfo` real; owns the matrices `D_80081358`/`D_800812F8`; the rest records (see "Known limitations") |
| LIBETC | `VSync`, `VSyncCallback`, `SetVideoMode`, `ResetCallback` | Real, on the pump |
| LIBCD | `cdload`'s interrupt-driven sector reads, movie streaming (`St*`, `CdRead2`) | Real command model over the disc image, timed in ticks; XA sectors to the XA decoder (`libcd.c`, `xa.c`) |
| LIBPRESS | MDEC movie decoding (STDWTITL) | Real (`libpress.c`, `mdec.c`) |
| LIBSND | All the game's sound (`sound.c`), the sequencer ticked from vsync | Real, over the SPU core (`libsnd*.c`; `docs/SOUND.md`) |
| LIBPAD | Pad init, state, modes, actuators (`pad.c`) | A digital pad on port 0; actuator calls accepted and ignored |
| LIBMCRD | Memory card access (`memcard.c`, STGMCARD) | Real, over `.mcd` images (`libmcrd.c`) |
| LIBC2, LIBAPI | String/memory functions; SHOCKTST's `sim:` host files | Not defined: they resolve to the host libc (`open("sim:...")` fails, as SHOCKTST expects) |

`DW3_PORT_TRACE=1` traces every shim call. The behaviours each library still assumes (rather than checked against the
PS1 or the emulator) are listed in `port/psyq/README.md` "Behaviour assumed".

## Rendering
- **Software GPU** (`port/psyq/gpu.c`): a 1024x512 VRAM of 16-bit pixels and every GP0 drawing command the game's
  13 primitive types produce: flat/Gouraud/textured polygons with 4/8/15-bit textures and CLUTs, texture windows,
  the four blend modes, dithering, mask bits, lines, rectangles, fills, VRAM copies and transfers, the draw area and
  offset. Exact VRAM semantics matter: the game reuses VRAM (FIELDSTG's shatter effect, `MoveImage`, FIGHTSTG's
  `DR_MOVE` cursor with `BreakDraw`/`ContinueDraw`). It is built at `-O3` in every build.
- **Video output** (`port/src/video.c`): every vsync the display area set by `PutDispEnv` is read from the VRAM in 15-
  or 24-bit mode (movies and the title are 24-bit; 320x480 interlaced frames are shown whole) and converted to 32-bit
  pixels. `--screenshot` writes it as a PPM in any build.
- **Window** (SDL3, static, pinned in `scripts/setup.sh`): the image at 4:3, nearest-neighbour, integer-scaled;
  fullscreen toggle. SDL selects X11/Wayland and the audio backend at run time.
- There is **no hardware renderer**; the software GPU is the only one.

## GTE
`port/psyq/gte.c` is the geometry coprocessor in software: the 64 registers with their read/write rules, every
command (RTPS/RTPT with the UNR division, NCLIP, MVMVA, the lighting and depth-cue commands, AVSZ3/4, GPF/GPL, ...),
FLAG and the saturations, in the PS1's fixed point. The game's GTE code (all in FIGHTSTG: `fightstg_model.c`,
`fightstg_8008D3B4.c`) reaches it through the generated `gtemac.h`; LIBGTE's functions call it too.

## Input
- **Window input** (`port/src/input.c`): the keyboard and every gamepad SDL sees are ORed into pad 1, a digital pad,
  once per vsync. Bindings are by key position and SDL's positional (PlayStation-layout) gamepad buttons, rebindable
  in the settings file; hotkeys (fullscreen, pause, the mods') never reach the pad. The mapping and defaults are in
  `port/README.md` "The window".
- **Scripted input** (`port/src/script.c`): `--script` replays a layer-2 pad script (`tests/replay/scripts/*.json`)
  with the same step engine as the emulator's `tests/replay/run.lua`; the script then owns the pad.
- `--input-test` injects every binding through SDL (keys, a virtual gamepad, chords, hotkeys) and checks what reaches
  the pad.

## Sound
The SPU core (`port/src/spu.c`, `spu_dsp.c`), LIBSND (`port/psyq/libsnd*.c`), the XA path and the audio output
(`port/src/audio.c`: rendered once per vsync, `--wav`, the SDL3 audio stream with drift correction) are described in
`docs/SOUND.md`. The audio is a function of the SPU writes and the vsync count, so it is deterministic and identical
with or without an output device.

## Disc, memory cards and movies
- **Disc** (`port/src/disc.c`): the user's BIN/CUE, its SHA-1 checked against the unpatched EU image (cached with a
  stamp; `--no-disc-check` skips it). LIBCD reads it at sector level, so the file table's LBAs, `cdload`'s 2340-byte
  mode and movie streaming work unchanged.
- **Memory cards** (`port/src/memcard.c`, `port/psyq/libmcrd.c`): raw 128 KB `.mcd` images (the usual emulator
  format), one per slot; every change is written back to the file. A new card is formatted as PCSX-Redux formats
  one. Saves move both ways between the port and the emulator.
- **Movies:** the 14 `MOVIE*.STR` files play through the movie stream (`libcd.c`), MDEC decoding (`mdec.c`) and the
  XA-ADPCM decoder (`xa.c`); no FFmpeg.

## Settings and mods
`dw2003 --config FILE` reads the settings file the launcher writes (disc, window, audio, memory cards, input bindings,
mods); without `--config` the binary depends on nothing on the machine. Built-in mods (`port/src/mods.c`, manifests in
`port/mods/<id>/mod.json`) are off under `--script` unless `--script-mods`. Formats and behaviour: `port/README.md`
"The settings file" and "The mods", `launcher/README.md`.

## Testing
| Check | What it proves | Where |
|---|---|---|
| Byte-identical PS1 build | No hook or `#ifdef PC_PORT` changes a PS1 byte | `scripts/build.sh --check` (CI) |
| Host-compile gate | Every unit compiles at `-m64` with pointer/int casts and implicit declarations as errors; no duplicate global | `tools/port_inventory.py probe`, `link` (in `scripts/test.sh`, CI) |
| Shim coverage | Every Psy-Q function the game calls is defined, nothing twice | `port/psyq/check.sh` |
| Layer-1 goldens on the host | GPU (715 cases), GTE (865), LIBGS's view (230: the battle camera), MDEC, XA and other families recorded in the emulator replay byte-for-byte through the shim's C; known GPU differences in `tests/host/known_mismatches.json` | `tests/golden/`, `tests/host/replay.py`, `tests/xa/` |
| The battle's VRAM | The first battle's textures and CLUTs equal to the emulator's (its frames cannot be compared: the PS1 is CPU-bound in battle) | `tests/port/vram.py` |
| The port against the emulator | `new_game` and `first_battle_save` replayed by the port: two runs byte-identical, `-m32` equal to `-m64`, no ASan/UBSan report, and the emulator's cross-core view (checkpoint stable hashes, overlay and map sequences) | `tests/port/run.py [--m32] [--sanitize]`, the `port` layer of `scripts/test.sh` |
| Sound | LIBSND's SPU writes against the emulator's trace on its timeline; SPU unit goldens | `tests/port/sound.py`, `tests/spu/`, `docs/SOUND.md` |
| Saves | Port and emulator load each other's saves (layer 3) | `tests/saves/run.py` |
| 60 Hz | The port's 60 Hz mode against the NTSC-patched game in the emulator | `tests/port/hz60.py` |
| Settings, mods, input | Settings round trip; mods keep the emulator's stable hashes; the battle-script scanner on every script on the disc; `--input-test` | `tests/port/settings.py`, `mods.py`, `battle.py`, `dw2003 --input-test` |
| Debug channel | One `--debug` run to CNTY_SEL: step and pad advance the frame exactly, the state map and the host symbol read the same, poke/peek, screenshot, hash, quit status; `tools/mcp`'s offline self-test | `tests/port/debug.py` (the `port` layer), `tools/mcp/selftest.py` |
| Holdouts | A `NON_MATCHING` PS1 image (the holdouts' WIP C) replayed in the emulator without divergence | `tests/holdouts/run.sh` |

What the port and the emulator are *not* compared on: frame numbers (the port's CD timing and CPU time differ), the
random index and the full checkpoint hash (both follow the frame count). See `tests/port/README.md`.

Play-tests on a desktop (window, gamepads, audio device, real time) cover what the headless tests cannot. Their
findings are filed as issues.

## Known limitations
- **Battle lighting:** LIBGS's GTE set-up is real for the camera (`GsSetRefView2`, `GsSetProjection`'s H; issue #7)
  but not for the lights: NCS lights with a zero colour matrix (`GsSetFlatLight`'s colours are recorded, not loaded;
  issue #19).
- **Pads:** one digital pad on port 0; no analog mode, no rumble (`PadSetAct` is accepted and ignored), no second
  port or multitap.
- **No reset key** in the window (the console reset exists only as a script step).
- **Coverage of the game:** the replays reach 14 of 19 tier-1 overlays and a small share of the WSTAG files; the
  debug overlays (STAGSLCT, SOUNDTST, SHOCKTST) and WFIGHTTS cannot be reached by pad input, and the three holdouts in
  them (card booster, WFIGHTTS, SHOCKTST) are unvalidated.
- **Timing stand-ins:** the CD seek times (3 ticks + 1 per 8192 sectors, at most 40) and `StGetNext`'s 5000 polls
  per vsync are estimates, not measurements.
- **State probes:** `port_state_read` (a script's `wait_mem`) maps only layout-identical data and an explicit field
  table; other pointer-bearing objects and overlay data read as unmapped.
- **BIOS:** a stand-in string, not the user's BIOS.
- **Linux only:** the ld script (`INSERT`, `-T`) and the 16 MB-aligned `.bss` are GNU ld/ELF features; a Windows
  (PE) or macOS build needs another arrangement for the arena and for the per-overlay sections. The SDL window is
  64-bit only.
- **Sanitizer builds** see other section sizes (ASan's redzones), so logs compare only between builds of the same
  kind.
