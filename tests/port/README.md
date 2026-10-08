# tests/port: the PC port's M1 test

`tests/port/run.py` checks the PC port (`port/`) against the emulator: the port replays layer 2's `new_game` script (boot,
CNTY_SEL, the opening movie skipped with START, the title, New Game, FIELDSTG's intro map 0x2D7 with WSTAG780) and must
reach what PCSX-Redux reached in `tests/replay/expected/new_game.json`.

```sh
tests/port/run.py                     # build/port, two runs, compare (~1 s once built; a build from scratch ~1 min)
tests/port/run.py --m32 --sanitize    # also the -m32 build (same log and record) and an ASan/UBSan build (no report)
tests/port/run.py --cd-speed instant  # the port's CD without seek or transfer time (the default is realistic)
tests/port/run.py --exe build/port-win/dw2003.exe --wine   # the Windows build (scripts/build_windows.sh) under Wine, headless;
                                                           # with --exe build/port is only configured (the generated headers) and
                                                           # LIBSND's replay on the emulator's timeline is left to the plain run;
                                      # its log, record and SPU trace must be the Linux build's (build/port is still built)
scripts/test.sh --layer port          # the same as the plain run, skipped when cmake/ninja/gcc/the disc are missing
```
It needs the disc (`iso/dw2003.cue`), the host gcc, CMake and Ninja (on PATH, or in `tools/venv`: `scripts/setup.sh
cmake`) and the venv, no emulator. Outputs go to `build/port-test/` (`--out DIR`): each run's log, record, stderr and
checkpoint dumps. Exit 0 pass, 1 fail, 2 something missing.

## What it compares, and why
1. **Determinism:** two runs of `build/port/dw2003 --disc iso/dw2003.cue --script tests/replay/scripts/new_game.json
   --log ... --record ...` must write byte-identical logs (one line per frame: stage, file, map, the primitive stream's
   hash; events: loads, inputs, checkpoints; `port/README.md`) and records. `--m32` requires the 32-bit build's log and
   record to be the 64-bit build's too: pointers are 4 bytes there, as on the PS1, so a difference is a pointer-size bug
   on one side. `--sanitize` runs an ASan/UBSan build once and fails on any report (its log and record must match too).
2. **The emulator's record, cross-core view:** `tests/replay/replay.py`'s own `cross_core_view` and `compare` (imported,
   not copied): per checkpoint its name, stage, map and **stable hash** (`gamestate_data`'s 0x275C-byte PS1 image with
   `replay.py`'s `VOLATILE_RANGES` zeroed), and the overlay and map sequences **without frames**. This is what the
   emulator's two CPU cores must agree on (tests/README.md "Both CPU cores"), and what the port must agree on.

Not compared, as between the emulator's dynarec and interpreter cores: the **frames** (the port's CD timing, movie
decoding and CPU time differ: new_game ends at frame 1,831 in the port and 2,462 in the emulator), **`random_index`**
(`pad_random` advances about once per field frame, so it follows the frame count), the **full hash** (the playtime, the
encounter timer: frame counts again) and the **input trace** (its frames follow the steps' frames). The script waits on
game state, not frame counts, so it holds on every timing.

The script must be the one the expected file was recorded from (`script_sha1`); a changed script fails until
`tests/replay/replay.py run tests/replay/scripts/<name>.json --record` re-records it. `run.py` runs every script that has
an expected file (`new_game`, `first_battle_save`), or the names given; its sanitizer run reads
`tests/port/ubsan.supp`, which names the game's own in-struct overruns (tests/host/FINDINGS.md 5 and 10) and nothing
else.

## The script engine
`psxstack/runtime/script.c` runs `tests/replay/scripts/*.json` with `tests/replay/run.lua`'s semantics, frame for frame: the step
runs after the frame log's sample on every vsync tick, instant steps (a checkpoint, a wait already satisfied, a `press`
whose `until` holds, a `walk` that arrived) chain within the frame (at most 100), the held buttons go to the pad
(`psyq_pad_set`, the physical buttons in the PS1 pad's bit order; the game rotates the face buttons itself) and to the
record's `inputs`. Every step type is implemented: `walk` reads the player actor from `heap_objects` on the host, and
`reset` restarts `game_main` with the game's data, the arena and the shim at power-on, the memory cards kept
(`psxstack/runtime/reset.c`). Exit status: 0 "script complete", 5 a step's timeout or `max_frames` (run.lua's message), 1 a bad
script or a `wait_mem` address the port does not map (`port/game/state.c`: layout-identical ranges and an explicit
field table). The JSON reader is
`psxstack/runtime/json.c` (strict RFC 8259).

`DW3_PORT_CHECKPOINT_DIR=<dir>` makes every checkpoint also write the image it hashes to `<dir>/cpNN_<name>.bin`, as
run.lua names its dumps (`run.py` sets it: `build/port-test/run1_checkpoints/`). When a stable hash differs, diff it with
the emulator's dump (`tests/replay/replay.py run tests/replay/scripts/new_game.json -v --out DIR` keeps
`DIR/run1/cpNN_*.bin`) and explain every differing byte range against `include/gamestate.h`: a port bug, a field that
depends on timing (it joins `VOLATILE_RANGES` only by the rule in tests/README.md "The stable hash and the RNG", and
only with the maintainer's approval), or a real divergence.

## The sound (`sound.py`, `sound_replay.c`; M3, docs/SOUND.md section 7)
```sh
tests/port/sound.py [--m32] [--sanitize]          # LIBSND on the emulator's timeline: the committed traces (~5 s)
tests/port/sound.py TRACE                         # any emulator trace (tests/sound/spu_trace.py run SCRIPT --calls)
tests/port/sound.py port PORT.trace [EMU.trace]   # a port run's trace (dw2003 --spu-trace)
```
The port does not reproduce the frames at which the game calls LIBSND (the PS1 drops frames, the CD and the script's
taps land elsewhere), so its own trace cannot equal the emulator's tick for tick. `sound.py` turns an emulator SPU
trace (with the `--calls` comments) into a replay script: the game's LIBSND calls, their pointers resolved to the sound
bank files on the disc, and the emulator's vsyncs at its ticks; `sound_replay` (built with the host gcc from
`psxstack/psyq/libsnd*.c` and `psxstack/runtime/spu*.c` into `build/port-sound/`) makes them and must write the emulator's trace:
every store and DMA block at the same tick. `run.py` runs it once on the committed traces (`tests/sound/expected/`,
`tests/port/sound/new_game.trace.gz`), then on each script's run: the three builds' SPU traces identical, the run's
trace equal to the replay of its own calls (once `psxstack/runtime/audio.c` renders), and for `new_game` the same LIBSND calls
as the emulator's, with a report of where the two games' frames differ.

## The settings file (`settings.py`; docs/LAUNCHER.md "Settings file")
`tests/port/settings.py` checks `--config FILE`, the contract with the launcher, through `--print-settings` (the
effective settings as a file with every key and absolute paths): the round trip (printed, loaded from another directory,
printed again: the same text), the defaults of an empty file, relative paths from the file's directory, `null` cards,
the command line overriding the file, the errors (exit 64 naming the key) and unknown keys (logged, ignored). With the
disc, a headless run under `--config` must replay `new_game` with the bare binary's log and record byte for byte and
create the two card files beside the settings, and with a mod enabled under `--script` the mod stays off (the same log)
unless `--script-mods`. The mods: each `port/mods/<id>/mod.json` (and psxstack's `mods/`) must equal the game's registry (`--print-mods`: ids,
types, defaults, ranges, enum ids; names, descriptions and labels present) and be copied beside the binary. With
`build/port-sdl`, the window's `--input-test` (offscreen) runs with the defaults and with
`tests/port/settings/rebound.json` (rebound keys and pads, chord hotkeys, a mod's bindings, the pause's round trip).
`scripts/test.sh --layer port` runs it after `run.py` (~5 s built).

## The 60 Hz mode (`hz60.py`; docs/LAUNCHER.md "50/60 Hz")
`tests/port/hz60.py` runs the port with `--refresh 60` (and `DW3_PORT_RESET_CHECK=1`) and compares it with the 60 Hz
game in the emulator, whose records are committed in `tests/port/hz60/` (`hz60.py record`: the unpatched disc with
`tests/port/ntsc_patch.lua`, which writes the NTSC patch's two words at `main`'s entry): `new_game` (twice: the same
log and record) and `first_battle_save` cut after `card_shop_left` (its next walk is timed for 50 Hz), each with the
emulator's cross-core view; `new_game`'s SPU trace against LIBSND's replay of its own calls (735 samples a vsync, the
NTSC tick) and the emulator's calls (`sound.py`'s `port` check). The fast-forward test is in `settings.py`.
`scripts/test.sh --layer port` runs it (~20 s built).

## The battle's VRAM (`vram.py`; issue #7)
`tests/port/vram.py` runs `first_battle_save` cut at `battle_start` (FIGHTSTG loaded) with two `vram` steps 300 frames
apart, in the emulator and in the port (~30 s), and compares the VRAM outside the battle's two display buffers (x < 320):
every pixel either run changed during the battle (its textures and CLUTs, ~150,000 pixels) must be equal at the end.
The displayed frames are not compared, and cannot be: the PS1 is CPU-bound in the battle (a game frame takes 2 or 3
vsyncs, the port's one) and the game advances its animations by the ticks elapsed, so the runs pass through different
frames (forcing the ticks per frame equal is not enough: the vsync time, the random index and the CD loads differ
too). The camera is checked by the layer-1 family `libgs_view` (`GsSetRefView2`'s matrix to the bit). Differences left
from before the battle are reported, not failed: the VRAM is equal at `asuka_lobby`; by `battle_start` the field picture
kept at x 640..959 differs where the sprites stand (the runs leave the field on different frames) and a few areas drawn
off screen differ in content (issue #23). In `scripts/test.sh --layer port`.

## The mods with the mod on (`mods.py`; docs/LAUNCHER.md "Mod runtime")
A run with a mod on has its own expected results (the emulator has no mods; the random generator follows the frames),
so `tests/port/mods.py` keeps the port's own record (`tests/port/mods/expected/`, `mods.py record` after a review) and
checks what the mod must keep from the emulator's run. `skip_dialogues`: `tests/port/mods/scripts/skip_dialogues.json`
(first_battle_save's route, no press through the scenes, one press per talk and per choice, each choice after 300
idle frames) twice with the same log and record, its cross-core view as recorded, the choices still open, the six
shared checkpoints with the emulator's stable hashes, the script stuck at the first dialogue with the mod off, the
first battle with fewer presses with the mod on, `fast_forward_waits` with the same log. `battle_animations`:
`first_battle_save` itself with the mod on (`hit_reaction` on and off) gives the emulator's cross-core view, its first
battle's three actions cut and the knock-out's KO reaction kept. `tests/port/battle.py` (no emulator, no port build:
gcc and the extracted disc) reads every battle script on the disc twice, with the port's `battle_scan.c` and with its
own reading, and requires them to agree, every script to end, reactions without a child command and every KO script
to end on animation 10. ~1 min together; in `scripts/test.sh --layer port`.

## The debug channel (`debug.py`; docs/PORT.md "Debug channel and the MCP server")
`tests/port/debug.py` drives one headless run (`--debug`, `--cd-speed instant`, ~2 s) through psxstack's `tools/mcp/game.py` (through `tools/mcp_game.py`), the
channel's plain client: `wait` on stage 22 (CNTY_SEL, `new_game.json`'s first `wait_stage`) hits and leaves the game
paused; `step 10` advances the frame by exactly 10 and `pad START` with sync by frames + release; `overlay_module.stage`
read by its PS1 address (the state map) and by the host global (`nm`) agree; a byte poked into the arena reads back by
both routes; `screenshot` writes a P6 PPM 320 wide; `hash` gives two SHA-1s; `save_state` while paused, 30 vsyncs,
`load_state`, the same 30 vsyncs give the same hash and screenshot, and a `save_state` while running is written at its
vsync's end and loads; `quit 3` exits with status 3. It also
checks that the socket path fits `sun_path` (108 bytes). Then it runs psxstack's `tools/mcp/selftest.py` (the client, the symbol
lookup and the server's tools against `fake_game.py`, offline; skipped with a message when the `mcp` package is not
installed). Needs `build/port/dw2003` (`run.py` builds it) and the disc. In `scripts/test.sh --layer port`.

## Save states (`savestate.py`; docs/PORT.md "Save states", issue #80)
`tests/port/savestate.py [--m32] [--sanitize]` runs `first_battle_save` from boot with `--save-state battle_start:FILE`
(its checkpoints must still match the emulator's cross-core view: the save changes nothing), then the same script with
`--load-state FILE`: the resumed run's record must equal the straight run's byte for byte, its frame log the straight
log's lines after the saved frame (about 24,700), and its `--wav` audio the straight run's samples from the next
vsync on. `--m32` and `--sanitize` repeat it on those builds (the sanitizer runs with
`ASAN_OPTIONS=detect_stack_use_after_return=0`, which states need; no ASan/UBSan report allowed); `--exe BIN [--wine]`
tests another binary (`build/port-win/dw2003.exe` under Wine passes). About 30 s for the plain build. `savestate.py
path [CHECKPOINT] [--exe BIN]` prints the path of a state of BIN at that checkpoint of `first_battle_save`, made first
when missing (`build/states/<the binary's SHA-1>/`). In `scripts/test.sh --layer port`. `debug.py` checks the debug
channel's save and load.

## The hardware renderer (`render_gpu.py`; docs/PORT.md "Rendering", issue #31)
`tests/port/render_gpu.py` builds `build/port-sdl` (needs `tools/sdl3` and `tools/dxc`; skipped without them) and runs it
on SDL's offscreen driver. **Everywhere** (CI too): `--window --renderer gpu --input-test` with a Vulkan loader that finds
no driver must log the fallback and pass on SDL_Renderer; a bad `--renderer` or `--gpu-screenshot`, and
`--gpu-screenshot` in the headless build, exit 64. **With a GPU device and the disc** (locally; the reason is printed
when skipped): `new_game` replayed with `--screenshot` and `--gpu-screenshot` at five vsyncs (CNTY_SEL's 320x576 screen,
the title, the field): the hardware picture must be the software image byte for byte, and its present into 960x720,
1366x768, 1920x1080, 1280x1024, 1000x700 and 300x200 must be the test's reference (video.c's 4:3 rectangle, an integer
nearest mapping) pixel for pixel; where the device can present to the offscreen driver's window (lavapipe can, NVIDIA
cannot) the input self-test also runs with the window through SDL_GPU. **With the disc:** SDL_Renderer's own present
(a 960x720 window, read back with `DW3_PORT_PRESENT_READBACK`) must be the same reference, so both present paths agree.
**The rasteriser, with a device and the disc:** `new_game` and `first_battle_save` with its whole VRAM target compared
with the software VRAM every 10 vsyncs (`DW3_PORT_GPU_VRAM_CHECK=10`): no difference. At internal scales 2 and 4, field
and battle frames of `first_battle_save` are the display's size times the scale and, averaged back over each block,
within `SCALED_BUDGET` of the software image. The debug channel's screenshot with `"renderer": "gpu"` (a headless
`--renderer gpu --internal-scale 2`) is 640x480. `--lavapipe` uses Mesa's software Vulkan driver. ~70 s.
**The Windows build:** `--exe build/port-win/dw2003.exe --wine` runs the device, the pictures, the VRAM checks and the
internal scales with that binary under Wine (`scripts/build_windows.sh`; the prefix `build/wine-prefix` unless
`WINEPREFIX` names one): SDL_GPU on Direct3D 12 there (Wine's vkd3d, or vkd3d-proton in a prefix that has it), on
Vulkan with `SDL_GPU_DRIVER=vulkan`; ~3 min. Skipped without a device (CI's Wine 9.0 has none: issue #67). In `scripts/test.sh --layer port`, followed by `tests/host/gpu_hw_replay.py` (the gpu golden
family's 715 lists through the rasteriser, the whole target after each; `tests/README.md`).

## Sub-pixel precision (`subpixel_jitter.py`; psxstack docs/PORT.md "Sub-pixel precision", issue #68)
`tests/port/subpixel_jitter.py run` replays `first_battle_save` to the end of its first battle with
`DW3_PORT_SUBPIXEL_LOG` (the GTE shadow on from boot, every RTPS vertex of the battle logged) and checks that at least
90 % of the polygon vertices gpu.c drew in the battle found their sub-pixel value, that no 16.16 sum disagrees with
the integer the game got, and that the shadow's positions move smoothly: at scale 4, at most 5 % of the vertices'
frame-to-frame motions are off from the float projection's by a target pixel or more (the integer SXY: about 20 %;
the shadow: about 2 %). `analyze LOG` prints the same numbers for a kept log (`--keep`). Needs `build/port/dw2003` and
the disc; ~40 s. In `scripts/test.sh --layer port`.

## The crash report (`crash.py`; docs/PORT.md "Crash report")
`tests/port/crash.py` runs the headless build three times (~3 s): with `DW3_PORT_CRASH_AT=30` it must die of SIGSEGV
(status -11), name its report on the last stderr line, and the report must carry the build of `--version`, the
platform, vsync 30, the signal, fault address 0, a pc and a stack that `scripts/symbolize.py` resolves to
`port_crash_test_write` and `port_frame`; a missing disc must give a `fatal` report with the reason; a normal run
writes none. Needs `build/port/dw2003`, the disc and `addr2line`. In `scripts/test.sh --layer port`.
`crash.py --wine [--exe PATH]` runs the same three with the Windows build (`build/port-win/dw2003.exe`,
`scripts/build_windows.sh`) through `wine`, headless: the crash is an access violation (exit status 0xC0000005, which
`wine` reports as 5), the report names the exception, the fault address and the access, its stack walked from the
exception's context resolves through `port_crash_test_write`, `port_frame`, the game's frames and `main` with
llvm-symbolizer and the PDB beside the exe, and the minidump beside it is read by the test's own minidump reader: the
exception record (code 0xC0000005, a write at 0, the address equal to the report's pc inside `dw2003.exe`), the module
list and the crashing thread's context. Needs `wine` and `tools/llvm-mingw` (~15 s).

## What it found (session 16)
- **The overlay copy takes CPU time on the PS1.** The game copies FIELDSTG (0x19000 bytes) into its slot with LIBC2's
  byte-loop `memcpy`, about 1.8 frames, so the emulator samples FIELDSTG's stage with no stage file yet (`(2, -1)` in the
  overlay sequence) and takes the `new_game_field` checkpoint there, before FIELDSTG's start-up writes `map_is_new`,
  `field_last_map` and `meter_random_count` (`gamestate_data` + 0x26D8, 0x26DC, 0x26F8). The port's copy was instant:
  no `(2, -1)`, and those three fields already set. `psxstack/runtime/overlay.c` now runs the whole frames a copy takes (12
  cycles a byte, 677,376 cycles a PAL frame) after it.
- **Primitive padding.** A textured polygon's third and fourth texture words carry padding in their high half, which the
  game never writes (it holds the packet buffer's previous contents); the -m32 and -m64 builds' heaps differ, so the
  primitive hash differed from frame 1,649 on. The shim's hash (`psxstack/psyq/libgpu.c`) now hashes that padding as 0.
- **-m32 build:** `src/main/heap.c`'s `PC_PORT` static assertion required an 8-byte multiple header, which the 12-byte
  -m32 header is not; it now requires a pointer-aligned one.

The longer `first_battle_save` script also runs in the port (exit 5, not a test): the stable hashes of `login_movie` and
`asuka_lobby` match and the overlay and map sequences agree up to the Main Lobby (map 0x203), where its four `walk` steps
arrive; step 21 (talk to Tamer Service until map 0x204) times out.
