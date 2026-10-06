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
`tests/replay/replay.py run tests/replay/scripts/new_game.json --record` re-records it.

## The script engine
`port/src/script.c` runs `tests/replay/scripts/*.json` with `tests/replay/run.lua`'s semantics, frame for frame: the step
runs after the frame log's sample on every vsync tick, instant steps (a checkpoint, a wait already satisfied, a `press`
whose `until` holds, a `walk` that arrived) chain within the frame (at most 100), the held buttons go to the pad
(`psyq_pad_set`, the physical buttons in the PS1 pad's bit order; the game rotates the face buttons itself) and to the
record's `inputs`. Every step type but `reset` is implemented (`walk` reads the player actor from `heap_objects` on the
host). Exit status: 0 "script complete", 5 a step's timeout or `max_frames` (run.lua's message), 6 `reset` (not in M1: it
would restart `game_main` with every global, the arena and the shim back at their startup state), 1 a bad script or a
`wait_mem` address the port does not map (`port/src/state.c` maps only layout-identical ranges). The JSON reader is
`port/src/json.c` (strict RFC 8259).

`DW3_PORT_CHECKPOINT_DIR=<dir>` makes every checkpoint also write the image it hashes to `<dir>/cpNN_<name>.bin`, as
run.lua names its dumps (`run.py` sets it: `build/port-test/run1_checkpoints/`). When a stable hash differs, diff it with
the emulator's dump (`tests/replay/replay.py run tests/replay/scripts/new_game.json -v --out DIR` keeps
`DIR/run1/cpNN_*.bin`) and explain every differing byte range against `include/gamestate.h`: a port bug, a field that
depends on timing (it joins `VOLATILE_RANGES` only by the rule in tests/README.md "The stable hash and the RNG", and
only with the maintainer's approval), or a real divergence.

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
