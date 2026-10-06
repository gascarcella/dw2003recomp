# tests/port: the PC port's M1 test

`tests/port/run.py` checks the PC port (`port/`) against the emulator: the port replays layer 2's `new_game` script (boot,
CNTY_SEL, the opening movie skipped with START, the title, New Game, FIELDSTG's intro map 0x2D7 with WSTAG780) and must
reach what PCSX-Redux reached in `tests/replay/expected/new_game.json`.

```sh
tests/port/run.py                     # build/port, two runs, compare (~1 s once built; a build from scratch ~1 min)
tests/port/run.py --m32 --sanitize    # also the -m32 build (same log and record) and an ASan/UBSan build (no report)
tests/port/run.py --cd-speed instant  # the port's CD without seek or transfer time (the default is realistic)
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
`port/src/script.c` runs `tests/replay/scripts/*.json` with `tests/replay/run.lua`'s semantics, frame for frame: the step
runs after the frame log's sample on every vsync tick, instant steps (a checkpoint, a wait already satisfied, a `press`
whose `until` holds, a `walk` that arrived) chain within the frame (at most 100), the held buttons go to the pad
(`psyq_pad_set`, the physical buttons in the PS1 pad's bit order; the game rotates the face buttons itself) and to the
record's `inputs`. Every step type is implemented: `walk` reads the player actor from `heap_objects` on the host, and
`reset` restarts `game_main` with the game's data, the arena and the shim at power-on, the memory cards kept
(`port/src/reset.c`). Exit status: 0 "script complete", 5 a step's timeout or `max_frames` (run.lua's message), 1 a bad
script or a `wait_mem` address the port does not map (`port/src/state.c`: layout-identical ranges and an explicit
field table). The JSON reader is
`port/src/json.c` (strict RFC 8259).

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
`port/psyq/libsnd*.c` and `port/src/spu*.c` into `build/port-sound/`) makes them and must write the emulator's trace:
every store and DMA block at the same tick. `run.py` runs it once on the committed traces (`tests/sound/expected/`,
`tests/port/sound/new_game.trace.gz`), then on each script's run: the three builds' SPU traces identical, the run's
trace equal to the replay of its own calls (once `port/src/audio.c` renders), and for `new_game` the same LIBSND calls
as the emulator's, with a report of where the two games' frames differ.

## The settings file (`settings.py`; docs/LAUNCHER_MODS_PLAN.md 4.3)
`tests/port/settings.py` checks `--config FILE`, the contract with the launcher, through `--print-settings` (the
effective settings as a file with every key and absolute paths): the round trip (printed, loaded from another directory,
printed again: the same text), the defaults of an empty file, relative paths from the file's directory, `null` cards,
the command line overriding the file, the errors (exit 64 naming the key) and unknown keys (logged, ignored). With the
disc, a headless run under `--config` must replay `new_game` with the bare binary's log and record byte for byte and
create the two card files beside the settings, and with a mod enabled under `--script` the mod stays off (the same log)
unless `--script-mods`. The mods: each `port/mods/<id>/mod.json` must equal the game's registry (`--print-mods`: ids,
types, defaults, ranges, enum ids; names, descriptions and labels present) and be copied beside the binary. With
`build/port-sdl`, the window's `--input-test` (offscreen) runs with the defaults and with
`tests/port/settings/rebound.json` (rebound keys and pads, chord hotkeys, a mod's bindings, the pause's round trip).
`scripts/test.sh --layer port` runs it after `run.py` (~5 s built).

## The 60 Hz mode (`hz60.py`; docs/LAUNCHER_MODS_PLAN.md 5.5)
`tests/port/hz60.py` runs the port with `--refresh 60` (and `DW3_PORT_RESET_CHECK=1`) and compares it with the 60 Hz
game in the emulator, whose records are committed in `tests/port/hz60/` (`hz60.py record`: the unpatched disc with
`tests/port/ntsc_patch.lua`, which writes the NTSC patch's two words at `main`'s entry): `new_game` (twice: the same
log and record) and `first_battle_save` cut after `card_shop_left` (its next walk is timed for 50 Hz), each with the
emulator's cross-core view; `new_game`'s SPU trace against LIBSND's replay of its own calls (735 samples a vsync, the
NTSC tick) and the emulator's calls (`sound.py`'s `port` check). The fast-forward test is in `settings.py`.
`scripts/test.sh --layer port` runs it (~20 s built).

## What it found (session 16)
- **The overlay copy takes CPU time on the PS1.** The game copies FIELDSTG (0x19000 bytes) into its slot with LIBC2's
  byte-loop `memcpy`, about 1.8 frames, so the emulator samples FIELDSTG's stage with no stage file yet (`(2, -1)` in the
  overlay sequence) and takes the `new_game_field` checkpoint there, before FIELDSTG's start-up writes `map_is_new`,
  `field_last_map` and `meter_random_count` (`gamestate_data` + 0x26D8, 0x26DC, 0x26F8). The port's copy was instant:
  no `(2, -1)`, and those three fields already set. `port/src/overlay.c` now runs the whole frames a copy takes (12
  cycles a byte, 677,376 cycles a PAL frame) after it.
- **Primitive padding.** A textured polygon's third and fourth texture words carry padding in their high half, which the
  game never writes (it holds the packet buffer's previous contents); the -m32 and -m64 builds' heaps differ, so the
  primitive hash differed from frame 1,649 on. The shim's hash (`port/psyq/libgpu.c`) now hashes that padding as 0.
- **-m32 build:** `src/main/heap.c`'s `PC_PORT` static assertion required an 8-byte multiple header, which the 12-byte
  -m32 header is not; it now requires a pointer-aligned one.

The longer `first_battle_save` script also runs in the port (exit 5, not a test): the stable hashes of `login_movie` and
`asuka_lobby` match and the overlay and map sequences agree up to the Main Lobby (map 0x203), where its four `walk` steps
arrive; step 21 (talk to Tamer Service until map 0x204) times out.
