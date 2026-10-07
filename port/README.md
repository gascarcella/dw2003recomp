# port/: the PC port

The game's C (`src/`, 388 units) linked with the port runtime (`port/src/`) and our Psy-Q shim (`port/psyq/`) into
one 64-bit host binary (`docs/PORT.md`; DECISIONS "PC port architecture"). The PS1 build
(`configure.py`, `build.ninja`) is untouched: the hooks in `include/port.h` expand to the original code without
`PC_PORT`, and `scripts/build.sh` proves the EXE and the overlays byte-identical.

**M1 (this state, session 16):** headless, no rendering, no sound. `build/port/dw2003 --disc iso/dw2003.cue` reads
the user's disc (SHA-1 checked), and `--script` replays a layer-2 pad script (`tests/replay/scripts/*.json`) with a
per-frame log and a record that `tests/port/` compares with the emulator's (`tests/port/README.md`).

## Build and run
```sh
cmake -S port -B build/port -G Ninja      # CMake >= 3.20; Ninja or Make; GCC (or Clang) with GNU ld
cmake --build build/port                  # ~30 s from scratch with -j6
build/port/dw2003 --max-frames 60         # exit 0 at the frame cap; 3 from port_unimplemented; 2 PLATFORM_HALT; 4 watchdog
build/port/dw2003 --trace                 # every tick, overlay load/resolve and stub call, to stderr
build/port/dw2003 --max-frames 600 --log run.log --record run.json   # the per-frame log and the record (below)
build/port/dw2003 --disc iso/dw2003.cue --script tests/replay/scripts/new_game.json --log run.log --record run.json
build/port/dw2003 --help                  # every option (--cd-speed instant|realistic, --no-disc-check, ...)
build/port/dw2003 --disc iso/dw2003.cue --debug /tmp/dw3.sock   # the debug channel (src/debug.c; tools/mcp drives it)
```
Options: `-DDW3_PORT_SANITIZE=ON` (`-fsanitize=address,undefined`; needs libasan/libubsan installed),
`-DDW3_PORT_M32=ON` (a 32-bit binary: `-m32` on every compile and link; needs gcc-multilib),
`-DDW3_PORT_ALLOW_UNRESOLVED=ON` (link with undefined symbols ignored: a private build without `port/psyq/`),
`-DDW3_PORT_PSYQ_DIR=<dir>` (another shim directory), `-DDW3_PORT_UNIT_OVERRIDES="src/main/x.c=<path>;..."`
(experiments: build a unit from another file, the tree untouched), `-DDW3_PORT_PSYQ_WERROR=OFF`,
`-DDW3_PORT_SDL=ON` (the window: below).

The sanitizer and 32-bit builds go into their own build directories:
```sh
cmake -S port -B build/port-san -G Ninja -DDW3_PORT_SANITIZE=ON && cmake --build build/port-san
build/port-san/dw2003 --max-frames 600    # any ASan/UBSan report goes to stderr (and ASan's exits non-zero)
cmake -S port -B build/port-m32 -G Ninja -DDW3_PORT_M32=ON && cmake --build build/port-m32
build/port-m32/dw2003 --max-frames 600 --log m32.log && build/port/dw2003 --max-frames 600 --log m64.log && cmp m32.log m64.log
```
The `-m32` build is the layout check: pointers are 4 bytes there, as on the PS1, so a run whose log differs from the
64-bit build's has a pointer-size bug on one side (the log holds no host address). The arena is 16 MB-aligned and
non-PIE in both.

## Layout
| Path | Contents |
|---|---|
| `CMakeLists.txt` | The build (below) |
| `include/port_runtime.h` | The runtime's internal interface (tables, arena, pump, logging) |
| `src/main.c` | Options, setup, `game_main()` (the game's `main`, renamed by `-Dmain=game_main` on `src/main/main.c`) |
| `src/arena.c` | The memory arena: `port_arena`, `port_ptr_to_s32`/`port_s32_to_ptr`, the stand-in BIOS |
| `src/overlay.c` | The overlay manager: `port_overlay_load` (snapshot restore), `port_overlay_resolve` (tag -> function); the game's `.data`/`.bss` snapshots (the reset's `port_overlay_reset`/`_check`) |
| `src/framelog.c` | The per-frame log (`--log`) and the run's record (`--record`): `port_harness.h` |
| `src/state.c` | The game-state probes (`port_state_*`), `port_state_read` (a PS1-address read), gamestate_data's PS1 image and hashes |
| `src/sha1.c` | SHA-1 (our own): the disc check, the checkpoint hashes |
| `src/disc.c` | The disc (`--disc`): the CUE/BIN, its SHA-1 check (with a stamp cache), LIBCD's sector source, `--cd-speed` |
| `src/script.c` | The input script (`--script`): `tests/replay/run.lua`'s step engine in C, the pad through `psyq_pad_set` |
| `src/json.c` | A small strict JSON reader and writer (our own), for the scripts and the settings |
| `src/settings.c` | The settings file (`--config`, `--print-settings`; `settings.h`): schema 1 of docs/LAUNCHER.md "Settings file" |
| `include/port_harness.h` | The M1 harness's interfaces (disc, frame log and probes, script) |
| `src/pump.c` | `port_wait` (the vsync and CD ticks, the frame cap, the watchdog), `port_halt`, `port_unimplemented` |
| `src/reset.c` | The console's reset (the script's `reset` step): `port_reset_request` (longjmp to `main()`), `port_reset_state`, `DW3_PORT_RESET_CHECK` |
| `src/video.c` | The video output: the display area of the VRAM as 32-bit pixels, `--screenshot`, the SDL3 window (below) |
| `src/input.c` | The window's input: keyboard and gamepads to the pad (rebindable), the hotkeys, the window's close, `--input-test` (below) |
| `src/mods.c` | The built-in mods' registry, their settings and hotkeys, `port_mods_frame` (manifests: `mods/<id>/mod.json`) |
| `src/battle_scan.c` | The battle scripts' command lengths (battle_animations' cut; `tests/port/battle.py` checks it on the disc) |
| `mods/` | The built-in mods' manifests (copied beside the binary) |
| `src/audio.c` | The audio output: the SPU core rendered every vsync, `--wav`, the SDL3 audio device (below) |
| `src/asmdata.c` | Zero data the PS1 build keeps in asm (FIELDSTG's `.bss` block; weak LIBGS/LIBCD data) |
| `include/spu.h`, `src/spu.c`, `src/spu_dsp.c`, `src/spu_internal.h` | The SPU core (M3, below): registers, SPU RAM, voices, mix, reverb; the DSP pieces; the internals the tests see |
| `psyq/` | The Psy-Q shim (its own README) |
| `../tools/port_gen.py` | The generators CMake runs (never by hand in the normal flow) |

## The settings file (`--config FILE`)
What the launcher starts the game with (docs/LAUNCHER.md "Contract" to "Settings file", schema 1; `src/settings.c`): the disc, the
window (`video.window`, `scale`, `fullscreen`, `refresh`), `audio.mute`, the memory cards (default `card1.mcd` and
`card2.mcd` beside the file, created formatted when missing; `null` for no card), the watchdog (default off), and the
`input` and `mods` sections. Paths in the file are relative to its directory. The options given on the command line
override the file. The game reads a settings file only when `--config` names it: the bare binary, which the tests run,
never depends on the machine. A bad value exits 64 naming the key; an unknown key is logged and ignored.
```sh
build/port-sdl/dw2003 --config <settings dir>/settings.json                   # as the launcher starts it
build/port/dw2003 --config settings.json --print-settings                     # the effective settings, every key
tests/port/settings.py                                                        # the round trip, overrides, errors
```

## The window (M2: `-DDW3_PORT_SDL=ON`)
SDL3 (zlib licence; DECISIONS "PC port architecture") shows the PS1's display and reads the keyboard and
gamepads. It is optional: the default build has no SDL and stays headless (the tests use it). SDL3 is pinned
(`scripts/setup.sh` `SDL3_VER`/`SDL3_SHA256`) and built from its release tarball into `tools/sdl3/` as a static library,
so the binary needs nothing beside it; SDL loads X11/Wayland/ALSA/PulseAudio/... with `dlopen` at run time.
```sh
scripts/setup.sh sdl3                     # ~70 s with 2 jobs; again: "already installed"
cmake -S port -B build/port-sdl -G Ninja -DDW3_PORT_SDL=ON && cmake --build build/port-sdl
build/port-sdl/dw2003 --disc iso/dw2003.cue --window            # 640x480, 50 vsyncs per second (PAL)
build/port-sdl/dw2003 --disc iso/dw2003.cue --scale 3 --fullscreen
build/port-sdl/dw2003 --disc iso/dw2003.cue --script tests/replay/scripts/new_game.json --window   # watch a replay
SDL_VIDEO_DRIVER=offscreen build/port-sdl/dw2003 --input-test --fps 0   # no display: the input self-test (exit 0)
```
SDL's backends follow the `-dev` headers present when `setup.sh sdl3` runs: missing optional ones are turned off one by
one (logged), and with no X11/Wayland headers at all only the `offscreen` and `dummy` video drivers are built (enough
for the tests). For a desktop window install the headers (Debian/Ubuntu: `libx11-dev libxext-dev libwayland-dev
libxkbcommon-dev wayland-protocols libasound2-dev libpulse-dev libudev-dev`), then `rm -rf tools/sdl3 && scripts/setup.sh
sdl3`. Another SDL3 (3.2 or newer) is used when CMake is pointed to it (`-DSDL3_DIR=<dir with SDL3Config.cmake>` or
`CMAKE_PREFIX_PATH`). The window is 64-bit only (`DW3_PORT_M32` with `DW3_PORT_SDL` is refused).

**The picture** (`src/video.c`): every vsync the display area that `PutDispEnv` set (`psyq.h` "The video output":
`psyq_gpu_display`) is read from the VRAM (`psyq_gpu_vram`, the software GPU's) and converted to 32-bit pixels at its
own size: 15-bit (one VRAM pixel per screen pixel, 5-bit components widened as `(c << 3) | (c >> 2)`, the mask bit not
shown) or 24-bit (`isrgb24`, the movies and the title: 3 bytes per screen pixel, so a 320-pixel line spans 480 VRAM
pixels; `disp.w` counts screen pixels), any width up to 640 and height up to 576 (the title's 320x480 interlaced frame
is shown whole, both fields), wrapping in the VRAM as the GPU does; black while `SetDispMask(0)` holds. The image is
drawn at 4:3, nearest-neighbour, as tall as an integer multiple of its lines fits the window (centred; scaled to fit when
the window is smaller than the image). `--scale N` opens a 320N x 240N window (default 2; with an even N a 240-line and
a 480-line display come out the same size), `--fullscreen` (F11 toggles). `--screenshot FRAME:PATH` (repeatable; any
build, also headless) writes the image of vsync FRAME as a binary PPM: the texture's pixels, independent of the window.
`--debug SOCKET` (any build) opens the debug channel on a Unix socket (`src/debug.c`'s header comment is the protocol;
`tools/mcp/` drives it): pause, step, wait, the pad, memory by host or PS1 address, screenshots, the hash, reset, quit,
each between two vsyncs; it turns the watchdog and the default frame cap off; `--debug-hold` starts the game paused at
its first vsync until the client resumes it (docs/PORT.md "Debug channel").

**Real time**: with a window the vsyncs are paced to `--fps N` per second (default 50, PAL; 60 for a later NTSC build;
0: unthrottled) against `CLOCK_MONOTONIC`; a run more than 0.1 s late starts the pace over instead of hurrying. Only the
time between vsyncs changes: the log and the record of a window run are the headless run's, byte for byte. Without a
window nothing is paced.

**Input** (`src/input.c`): the keyboard and every gamepad SDL sees are ORed into pad 1, a digital pad (`psyq_pad_set`),
once per vsync. Keys (by position): arrows the D-pad; X cross, C circle, Z square, S triangle; Enter (or keypad Enter)
START; Backspace (or right Shift) SELECT; Q L1, E R1, 1 L2, 3 R2; F11 fullscreen, P pause. Gamepads (SDL's positional
buttons, the PlayStation layout): south cross, east circle, west square, north triangle, back SELECT, start START,
shoulders L1/R1, triggers L2/R2, the D-pad and the left stick the D-pad. All of it is rebindable in the settings
(`input.keyboard`, `input.gamepad`, `input.hotkeys`; docs/LAUNCHER.md "Input bindings"), and the mods add hotkeys. A
hotkey never reaches the pad: a completed trigger masks its inputs until they are all released. **The pause** stops the
game between two vsyncs (the window keeps presenting the last image, the audio device pauses) until the key again. Every change of the pad goes to the log's `I` lines and the
record's `inputs`, as a script's do. With `--script` the script owns the pad: the window's input never reaches it.
Closing the window (or SIGINT/SIGTERM, which SDL turns into a quit) ends the run: `port_exit(0, "window closed")`, the
log and the record written. `--input-test` checks the path: it injects every key of the map (`SDL_PushEvent`), then
attaches a virtual SDL gamepad and presses each of its buttons, triggers and stick directions, and a chord, one per
frame with its release in the next, then every hotkey trigger, and requires the buttons sent to `psyq_pad_set` to be the
map's (none for a hotkey's, which must fire; with `--script`: none sent, and the script's log stays byte-identical to a
run without the test), then (without a script) the pause's round trip; exit 0 when all the checks pass (74 with the
defaults), 6 otherwise. With `--config` it tests the settings' map. CI runs it on SDL's offscreen driver, with the
defaults and with `tests/port/settings/rebound.json`.

**The mods** (`src/mods.c`; docs/LAUNCHER.md "Mod manifest", "Mod runtime"): the built-in mods' registry; each has a manifest
`port/mods/<id>/mod.json`, copied beside the binary (`build/port*/mods/`) for the launcher. `--print-mods` prints the
registry; the settings' `mods.<id>` enable and configure them; under `--script` they are off unless `--script-mods`.

**Two rates** (`src/pump.c`): the nominal rate (`port_rate`, 50; `--fps N` sets it: the audio's samples per vsync) and
the pace (the wall clock's vsyncs per second; `port_pace_set`, which starts the schedule over on every change, so a
fast-forward that ends never makes the game wait for the vsyncs it ran ahead).

**Fast-forward** (the `fast_forward` mod; docs/LAUNCHER.md "Fast-forward"): enabled in the settings, hold `Tab` (or a
toggle binding) runs the game at the nominal rate times its speed (default `4x`), presents at most 60 images a second
and mutes the audio device; the game, its log and its record are unchanged.

**Skip dialogues** (the `skip_dialogues` mod; docs/LAUNCHER.md "Skip dialogues"): toggled with `F2` (by default), the game's text appears at once
and goes on by itself (`port_mod_skip_dialogues`, read by three `#ifdef PC_PORT` blocks in `src/main/message.c` and
the battle's `fightstg_message_step`); choices and menus still wait. `fast_forward_waits` also fast-forwards the field
events' scripted waits.

**Battle animations** (the `battle_animations` mod; docs/LAUNCHER.md "Disable battle animations"): attacks', techniques' and items' animation scripts are cut
at their start (`port_mod_battle_animations`, one `#ifdef PC_PORT` block in `fightstg_script_update`; the stream
scanned by `src/battle_scan.c`): the target's short reaction plays instead (`hit_reaction`, the default), or nothing but
a knock-out's. The battle's rules ran before; its outcome is unchanged.

**60 Hz** (`--refresh 60`, or the settings' `video.refresh`; docs/LAUNCHER.md "50/60 Hz"): the game's own 60 Hz mode (the NTSC patch's
`records_60hz`, set before the reset's snapshot), paced at 60, the audio at 735 samples a vsync, the CD's ticks at 60.
`tests/port/hz60.py` compares it with the patched game in the emulator. (`--fps 60` alone is the old meaning: the PAL
game at 60, 20% fast.)

## What CMake generates (`build/port/gen/`, by `tools/port_gen.py`)
At configure time:
- `units.cmake`: the 388 units: `src/<target>/*.c` for the EXE and the tier-1/tier-2 overlays, `src/wstag/<unit>.c`
  for the WSTAG files of `config/wstag_c.txt` (the same set `configure.py` compiles; `port_gen.py units` fails if a C
  file under `src/` is not one of them). WSTAG260 is data-only and has no unit.
- `include/include_asm.h` (empty `INCLUDE_ASM`/`INCLUDE_RODATA`) and `include/psyq/gtemac.h` (every `gte_*` macro
  translated from its MIPS sequence into calls of the software GTE, `port/psyq/gte.c`), first on the include path with the real headers' guards, as `tools/port_inventory.py probe` does.
- `overlays.ld`: a GNU ld script (`-T`, `INSERT BEFORE .data`/`.bss`) that puts the EXE's (`src/main/`) and each
  overlay's `.data`/`.bss` input sections (the units are compiled with `-fdata-sections`, matched by path
  `*src/<dir>/*.c.o`; plus any object's `.data.dw3.<ovl>`/`.bss.dw3.<ovl>`) into `.dw3.data.<ovl>`/`.dw3.bss.<ovl>`
  (`<ovl>` = `main` for the EXE) with `__start_dw3_*`/`__stop_dw3_*` symbols, and defines `port_slot1`, `port_slot2`,
  `port_heap_start`, `port_heap_end` inside `port_arena`.
- `include/port_arena_gen.h`: the arena's sizes.

At build time, after the units are compiled:
- `overlay_tables.c`: per overlay `{ tier, file ID, name, [{ PS1 address, host function }], section bounds }`.
  The addresses are the `type:func` lines of `config/<overlay>.symbols.txt` and `config/wstag/<n>.symbols.txt`;
  a function is in the table only if `nm` of the overlay's objects shows it as a global (a `static` function, or
  one still `INCLUDE_ASM`, has no host symbol). File IDs: tier 1 from `overlay_files` (`src/main/overlay.c`),
  WFIGHTMN/WFIGHTTS 0x208/0x209, the WSTAG files from FIELDSTG's stage tables joined with `config/wstag.txt`.
  The generator also checks every tag site in the C (`WSTAG_ENTRY`, `OVERLAY_ENTRY`, `SLOT_FUNC`, `LATE_FUNC`)
  against the tables and fails the build if one does not resolve.
- `port_state_tables.c` (`port_gen.py state`): the EXE's functions (`{ PS1 address, host function }`, the
  `type:func` lines of `config/symbol_addrs.txt` that nm finds in the EXE's objects), its data symbols with a `size:`
  there (`{ PS1 address, PS1 size, host object, layout-identical length }`), and `VOLATILE_RANGES`, read from
  `tests/replay/replay.py`. A data symbol is layout-identical as a whole when its `sizeof` at `-m64` is its PS1 size
  (a pointer or a `long` makes it larger at -m64); the generator measures that size by compiling the defining units
  at `-m64` (to assembly, ~1 s) in every build, so the `-m32` build's table is the same, and it fails if a
  pointer-free object's size differs between the two.

After the link (`port_gen.py sections`, POST_BUILD): the link map (`build/port/dw2003.map`, `-Wl,-Map`) must show every
writable input section of a game object (`.data*`, `.bss*`, `COMMON` of the units in CMake's `dw3_game.dir`) inside a
`.dw3.*` output section, the ranges the console's reset restores; one outside fails the build (`-v` lists the
runtime's and the shim's own writable data, which reset themselves).

## The runtime
**Arena** (docs/PORT.md "Memory arena"): one 16 MB-aligned `.bss` block, smaller than 16 MB, mirroring the PS1 from
`0x80082CB0` up: slot 1 (`0x23130` bytes), slot 2 (`0x5A20`), then the heap (4 MB, larger than the PS1's
1.3 MB because 64-bit structs are bigger). A pointer's PS1-style address (`PTR_TO_S32`) is `0x80082CB0 + offset`;
the low 24 bits of any arena pointer are its offset, which is what the ordering-table tags keep (`PTR_TO_U32`), and
the shim's `DrawOTag` walks them from the arena's base. The 16 MB alignment is an ELF/GNU ld property of a
non-PIE executable (`-no-pie`; checked at startup).

**Overlay manager** (2.5): every overlay is linked in. `OVERLAY_COPY` (the game's two `memcpy` sites) calls
`port_overlay_load(tier, file, ...)`: a file with a table becomes the tier's current overlay and gets its `.data` and
`.bss` restored from the startup snapshot (`.bss`: zero; what the PS1's copy of the file did); a file without one
(WSTAG260, FIELDSTG's data files) is copied into the slot buffer, exactly the PS1's memcpy. `OVERLAY_FN`/`LATE_CALL`
call `port_overlay_resolve(tier, addr)`: a tag (an address in `0x80000000..0x80200000`) is looked up in the tier's
current overlay's table (fatal if absent: static, still asm, or a data file is loaded); anything else is a host
function pointer and comes back unchanged. **The copy's time:** on the PS1 the copy is LIBC2's byte-loop `memcpy`
(about 12 cycles a byte), so FIELDSTG's 0x19000 bytes take ~1.8 frames, and the emulator's checkpoints can see the
state between the copy and the overlay's start-up (`new_game_field` does). `port_overlay_load` therefore runs
`size * 12 / 677376` vsync ticks (`port_wait`) after a copy (FIELDSTG: 1; the smaller files: 0).

**Game-state probes** (`state.c`): what `tests/replay/run.lua` reads from PS1 RAM, read from the host's objects:
`overlay_module.stage`/`.file`, `gamestate_data.map`, `pad_random.index`, and the slot's first word (run.lua's
`wait_stage` checks `0x80082CB0`: on the host `port_overlay_load` keeps the first word of every file it copies,
per tier). `port_state_read(addr, size, signed, &v)` (the script's `wait_mem`) maps a PS1 address only inside a
layout-identical range: a whole data symbol by the rule above, or the prefix `state.c` lists for a pointer-bearing
object (`overlay_module` 8 bytes, `gamestate_data` 0x26FC bytes, `pad_random` 4; each checked by `_Static_assert`);
anything else returns 0 (unmapped). Besides those ranges, an explicit field table maps the pointer-bearing objects'
fields the scripts read, each checked by `_Static_assert`: `memcard_state`'s (`state`, `command`, `done`, ...; the type
is in `include/memcard.h`) and FIELDSTG's `fieldstg_stage` non-pointer fields (`menu_open`, ...), the latter only while
FIELDSTG is the tier-1 overlay.

**The checkpoint hash**: `gamestate_data`'s PS1 image is its first 0x26FC bytes as they are (no pointer before
`funcs`), then the 24 `funcs` entries as the PS1 addresses of the host functions they point to (the generated table;
fatal if one is not there): 0x275C bytes, the emulator's dump (`run.lua` `checkpoint`). The record has its SHA-1 and
the SHA-1 with the volatile ranges zeroed (`gamestate_sha1_stable`), as `replay.py` computes them.

**Pump**: a frame is one vsync tick (`psyq_vsync_tick`), from the game's `VSync()` or from `PLATFORM_WAIT()` ->
`port_wait()`, or from LIBCD's `StGetNext` once per 5000 empty polls (the movie player spins without a wait hook).
Each tick runs `port_frame` (`pump.c`): the CD tick (`psyq_cd_tick`), the frame log, the window's input (with a
window), the script's step, the screenshots and the window's present (`video.c`), the exit at `--max-frames` (default
600; none with `--script`), and with a window the real-time pace ("The window"). `DW3_PORT_CHECKPOINT_DIR=<dir>` writes each checkpoint's PS1
image as `cpNN_<name>.bin`, named like `run.lua`'s dumps. A watchdog (`--watchdog SEC`,
default 10) exits 4 when no `port_wait()` ran for that long: a loop that no hook reaches (see below).

**The console's reset** (`reset.c`; the script's `reset` step, run.lua's `PCSX.hardResetEmulator()`): the step releases
the pad and ends its frame as run.lua does (the input trace, the end-of-script check), then `port_reset_request`
longjmps from the vsync tick, however deep in the game's stack (a `VSync`, a `PLATFORM_WAIT`, `StGetNext`'s polling
tick, an overlay copy's ticks), to `main()`, which calls `port_reset_state` and `game_main()` again. The reset puts
back: every game global (the EXE's and every overlay's `.data`/`.bss` from the startup snapshot, static locals
included; no overlay current; `port_overlay_reset`), the arena (zero: the PS1's RAM is cleared), the shim
(`psyq_reset`: each library's `psyq_<lib>_reset`; the CD's sector source and timing model, the vsync hook, the GPU walk
window, the memory cards' contents and the trace setting stay). The frame count, the frame log's sequences (the next
frame records `(0, 0)` and map 0 as a change, as the emulator's listener records the cleared RAM), the checkpoints,
the input trace and the script's next step go on; the log gets an `R` line. `DW3_PORT_RESET_CHECK=1` (or `--trace`)
proves the restore: at startup (the last setup call before `game_main`) and after every reset, every game section is
compared with its startup snapshot and the arena with zero, fatal on a difference; with `port_gen.py sections` (no
game data outside the sections) that is the whole of the game's writable state. Session 16's check: `new_game` with a
reset inserted (after `title_screen`, mid-movie in `StGetNext`'s tick, after `first_field_map`; and after
`first_battle_save`'s `back_on_field`) and the boot steps repeated runs, after the reset, a log byte-identical to a
plain `new_game` run's (frames shifted), with the same full and stable checkpoint hashes, in the -m64, -m32 and
sanitizer builds.

## The per-frame log (`--log FILE`)
Text, line-buffered, one line per frame and one per event; nothing in it depends on the host (no time, no address),
so two runs, and the `-m32` and `-m64` builds, write the same bytes. `<frame>` is `port_frames`: the vsync ticks so
far (an event between two ticks carries the last tick's number, as the emulator's listener would see it at the next).
| Line | Meaning |
|---|---|
| `# dw2003 port frame log 1 (port/README.md)` | The header (the format's version) |
| `F <frame> st <stage> fl <file> map 0x<map> prims <n> hash <8 hex>` | Every frame: `overlay_module.stage`/`.file`, `gamestate_data.map`, and the primitive stream of the frame (`psyq_gpu_take_hash`: the primitives DrawOTag walked since the previous frame, their FNV-1a hash) |
| `S <frame> stage <stage> file <file>` | An overlay-sequence entry: `(stage, file)` changed (frame 1 always) |
| `M <frame> map 0x<map>` | A map-sequence entry: the map changed (frame 1 always) |
| `L <frame> tier <t> file 0x<id> <name> word0 0x<8 hex> size 0x<n>` | A file copied into a slot (`port_overlay_load`): the overlay's name, or `(data)`; its first word; its size |
| `C <frame> <name> stage <stage> map 0x<map> rnd <index> sha1 <40 hex> stable <40 hex>` | A checkpoint |
| `I <frame> buttons 0x<4 hex>` | The script's pad changed (`port_framelog_input`; PS1 bit order, active high) |
| `R <frame> reset` | The console's reset (the script's `reset` step): the next frame runs the game from `main()` again |
| `X <frame> status <status> <reason>` | The exit (`port_exit`) |

## The record (`--record FILE`)
JSON written at exit, with the keys of `tests/replay`'s records (`replay.py` `cross_core_view` reads it as it is):
`runner` (`"port"`), `status` (the exit status), `reason`, `frames`, `checkpoints` (`name`, `frame`, `stage`, `map`,
`random_index`, `gamestate_sha1`, `gamestate_sha1_stable`, as run.lua and replay.py record them), `overlay_sequence`
(`{frame, stage, file}`), `map_sequence` (`{frame, map}`), and `inputs` (`{frame, buttons: [names]}`, the names
sorted as run.lua sorts them) once the script has called `port_framelog_input`. The sequences follow run.lua's vsync
listener: an entry at every change, the first frame always (`{1, 0, 0}`, map 0).

## The SPU core (M3)
`src/spu.c` and `src/spu_dsp.c` are the PS1's sound chip, our own from psx-spx (docs/SOUND.md section 6 has what is
modelled, the readings taken where psx-spx is silent, and the checks): the register file by offset from
`0x1F801C00` (`spu_write16`/`spu_read16`), 512 KB of SPU RAM (`spu_dma_write`, the FIFO), 24 voices (ADPCM, pitch
with the 4-point interpolation, ADSR, volume sweeps, noise, PMON), the mix with its clamps, the reverb at 22,050 Hz,
the CD input (`spu_cd_input`) and the capture buffers. `spu_render(out, frames)` renders 44,100 Hz stereo; time moves
only there (a register write acts between two samples), so the output is a function of the writes and the frame
counts. `spu_set_write_hook` sees every write and DMA block (the trace writer). Nothing drives it yet (LIBSND is a
stub); its tests are host-only: `tests/spu/run.sh [--all]` (unit goldens from a Python model of the same psx-spx text,
`-m64`/`-m32`/sanitizers), `tests/spu/render_trace.py` (the committed `cnty_sel` SPU trace to a WAV, with checks),
`tests/spu/envelope_oracle.py` and `tests/spu/capture.py` (against PCSX-Redux). 53× real time at `-O2`, 13× at `-O0`.

## Audio (M3: `--wav`, the SDL3 audio device)
`src/audio.c` renders the SPU core once per vsync (`port_audio_frame`, `pump.c`'s vsync pre-hook: at the start of the
tick, before the game's VSyncCallback handler, where LIBSND's flush reads the envelopes and writes the SPU): vsync n gets `floor((n + 1) * 44100 / rate) - floor(n * 44100 / rate)` stereo frames
with `rate` = `--fps` (50 by default: 882 frames a vsync; `--fps 60`: 735, the NTSC patch's 60 ticks a second; `--fps
0`: 50). So the audio lasts exactly as long as the vsyncs at their nominal pace and its pitch never changes (a PAL game
paced at 60 plays its music 20 % faster). It renders whether anything listens or not: LIBSND reads the voices'
envelopes back, so the game must not run differently with or without an output. The samples depend only on the SPU
writes and the vsync count: two runs give the same bytes.
```sh
build/port/dw2003 --disc iso/dw2003.cue --script tests/replay/scripts/new_game.json --wav run.wav   # any build, headless
build/port-sdl/dw2003 --disc iso/dw2003.cue --window            # sound through SDL3's default device
build/port-sdl/dw2003 --disc iso/dw2003.cue --window --mute     # no device
SDL_VIDEO_DRIVER=offscreen SDL_AUDIO_DRIVER=disk SDL_AUDIO_DISK_OUTPUT_FILE=out.raw build/port-sdl/dw2003 --disc iso/dw2003.cue --window --max-frames 3000
```
`--wav FILE`: 44,100 Hz, stereo, signed 16-bit little-endian PCM; the header's sizes are written at exit
(`port_exit`), so the file is `44 + vsyncs × 882 × 4` bytes (every vsync of the run, the last one too). The window's device (unless `--mute`) is an SDL3 audio stream fed the
same samples. Its clock and the window's pace (`CLOCK_MONOTONIC`, `pump.c`) drift apart; the queue (frames put and
not yet played, measured before each vsync's put) is held without touching the game's timing or the samples: a moving
average above the band (target ± one vsync, target = the device's period + two vsyncs: 63 ms ± 20 ms at 1,024 frames)
speeds the stream's playback up by 0.2 % (`SDL_SetAudioStreamFrequencyRatio`, 3.5 cents) until it is back at the
target, below the band slows it down as much; an empty queue (the host was late) is refilled with the target's worth
of silence; above target + three vsyncs (an unthrottled `--fps 0`, a device that stalled) the vsync's samples are
dropped. The log (stderr) gets the device, the watermarks, and every 10 s and at exit the queue's minimum, maximum and
mean, the ratio changes, refills and drops. Without a device (none on the host, or SDL fails) the run goes on silent.
SDL's `disk` driver plays in real time into a file (S16LE stereo at 44,100 Hz) and `dummy` discards: headless tests of
the path (CI runs the input self-test with `dummy`). Measured (session 16): the WAV of a window run equals the
headless run's, byte for byte; over 60 s of the game (CNTY_SEL's music, LIBSND) on the `disk` driver the queue stayed
at 24-85 ms (mean 57) with no refill and no drop, and the disk file holds the WAV's sound ~90 ms later (the same RMS,
5,198 vs 5,200). `new_game` headless: silent until CNTY_SEL's music at 4.1 s (vsync 207), RMS ~3,000-6,400 a second.

## Known gaps (M1)
- Fixed in session 16 (kept here as the record of what the `-m32`/`-m64` log comparison and the sanitizer found):
  the overlay entry's `Object *` truncated through `s32 *result` and 122 object data blocks sized in PS1 bytes
  (`tests/host/FINDINGS.md` 9d, `tools/port_inventory.py object-sizes` keeps the class closed), the host heap's
  4-byte block alignment (now 8 under `PC_PORT`, `src/main/heap.c` `HEAP_ALIGN`), FIELDSTG's NULL map-event list and
  `FieldstgBackgroundView`'s byte pad. The disc is read through `--disc` (`src/disc.c`, SHA-1 checked).
- The BIOS is a stand-in: `BIOS_PTR` serves a 256-byte region at `0x1FC00100` holding a version string.
- Windows/macOS: the ld script (`INSERT`, `-T`) and the 16 MB `.bss` alignment are GNU ld/ELF; PE needs another
  arrangement for the arena (allocate at startup; `port.h` would need the slot symbols as pointers) and for the
  per-overlay sections.
- The snapshot copies with plain byte loops in `no_sanitize_address` functions (ASan's redzones between globals
  are inside the ranges); so a sanitizer build's overlay-load log lines show other section sizes (ASan's redzones)
  than a normal build's: compare logs only between builds of the same kind.
- Done since this list was written (session 16): drawing (M2, `port/psyq/gpu.c`), sound (M3, `port/src/spu*.c`,
  `port/psyq/libsnd*.c`), memory cards (`.mcd` images), the movies (M5: `port/psyq/mdec.c` decodes them, the VRAM equal
  to the emulator's outside the movie buffers and within IDCT rounding inside; their XA audio through `port/psyq/xa.c`).
