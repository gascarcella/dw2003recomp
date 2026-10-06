# port/: the PC port

The game's C (`src/`, 388 units) linked with the port runtime (`port/src/`) and our Psy-Q shim (`port/psyq/`) into
one 64-bit host binary (`docs/PC_PORT_PLAN.md`; DECISIONS "PC port decisions (session 15)"). The PS1 build
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
```
Options: `-DDW3_PORT_SANITIZE=ON` (`-fsanitize=address,undefined`; needs libasan/libubsan installed),
`-DDW3_PORT_M32=ON` (a 32-bit binary: `-m32` on every compile and link; needs gcc-multilib),
`-DDW3_PORT_ALLOW_UNRESOLVED=ON` (link with undefined symbols ignored: a private build without `port/psyq/`),
`-DDW3_PORT_PSYQ_DIR=<dir>` (another shim directory), `-DDW3_PORT_UNIT_OVERRIDES="src/main/x.c=<path>;..."`
(experiments: build a unit from another file, the tree untouched), `-DDW3_PORT_PSYQ_WERROR=OFF`.

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
| `src/overlay.c` | The overlay manager: `port_overlay_load` (snapshot restore), `port_overlay_resolve` (tag -> function) |
| `src/framelog.c` | The per-frame log (`--log`) and the run's record (`--record`): `port_harness.h` |
| `src/state.c` | The game-state probes (`port_state_*`), `port_state_read` (a PS1-address read), gamestate_data's PS1 image and hashes |
| `src/sha1.c` | SHA-1 (our own): the disc check, the checkpoint hashes |
| `src/disc.c` | The disc (`--disc`): the CUE/BIN, its SHA-1 check (with a stamp cache), LIBCD's sector source, `--cd-speed` |
| `src/script.c` | The input script (`--script`): `tests/replay/run.lua`'s step engine in C, the pad through `psyq_pad_set` |
| `src/json.c` | A small strict JSON reader (our own), for the scripts |
| `include/port_harness.h` | The M1 harness's interfaces (disc, frame log and probes, script) |
| `src/pump.c` | `port_wait` (the vsync and CD ticks, the frame cap, the watchdog), `port_halt`, `port_unimplemented` |
| `src/asmdata.c` | Zero data the PS1 build keeps in asm (FIELDSTG's `.bss` block; weak LIBGS/LIBCD data) |
| `psyq/` | The Psy-Q shim (its own README) |
| `../tools/port_gen.py` | The generators CMake runs (never by hand in the normal flow) |

## What CMake generates (`build/port/gen/`, by `tools/port_gen.py`)
At configure time:
- `units.cmake`: the 388 units: `src/<target>/*.c` for the EXE and the tier-1/tier-2 overlays, `src/wstag/<unit>.c`
  for the WSTAG files of `config/wstag_c.txt` (the same set `configure.py` compiles; `port_gen.py units` fails if a C
  file under `src/` is not one of them). WSTAG260 is data-only and has no unit.
- `include/include_asm.h` (empty `INCLUDE_ASM`/`INCLUDE_RODATA`) and `include/psyq/gtemac.h` (every `gte_*` macro a
  no-op until M5), first on the include path with the real headers' guards, as `tools/port_inventory.py probe` does.
- `overlays.ld`: a GNU ld script (`-T`, `INSERT BEFORE .data`/`.bss`) that puts each overlay's `.data`/`.bss` input
  sections (its objects are compiled with `-fdata-sections`, matched by path `*src/<dir>/*.c.o`; plus any object's
  `.data.dw3.<ovl>`/`.bss.dw3.<ovl>`) into `.dw3.data.<ovl>`/`.dw3.bss.<ovl>` with `__start_dw3_*`/`__stop_dw3_*`
  symbols, and defines `port_slot1`, `port_slot2`, `port_heap_start`, `port_heap_end` inside `port_arena`.
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

## The runtime
**Arena** (`PC_PORT_PLAN.md` 2.4): one 16 MB-aligned `.bss` block, smaller than 16 MB, mirroring the PS1 from
`0x80082CB0` up: slot 1 (`0x23130` bytes), slot 2 (`0x5A20`), then the heap (4 MB, larger than the PS1's
1.3 MB because 64-bit structs are bigger). A pointer's PS1-style address (`PTR_TO_S32`) is `0x80082CB0 + offset`;
the low 24 bits of any arena pointer are its offset, which is what the ordering-table tags keep (`PTR_TO_U32`), and
the shim's `DrawOTag` walks them from the arena's base. The 16 MB alignment is an ELF/GNU ld property of a
non-PIE executable (`-no-pie`; checked at startup).

**Overlay manager** (2.5): every overlay is linked in. `OVERLAY_COPY` (the game's two `memcpy` sites) calls
`port_overlay_load(tier, file, ...)`: a file with a table becomes the tier's current overlay and gets its `.data`
restored from the startup snapshot and its `.bss` zeroed (what the PS1's copy of the file did); a file without one
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
anything else returns 0 (unmapped). Not mapped yet: `memcard_state` (its type is private to `memcard.c`, and its
fields at 0x90 and 0x300 follow a pointer) and overlay data (e.g. `0x80099DD0`).

**The checkpoint hash**: `gamestate_data`'s PS1 image is its first 0x26FC bytes as they are (no pointer before
`funcs`), then the 24 `funcs` entries as the PS1 addresses of the host functions they point to (the generated table;
fatal if one is not there): 0x275C bytes, the emulator's dump (`run.lua` `checkpoint`). The record has its SHA-1 and
the SHA-1 with the volatile ranges zeroed (`gamestate_sha1_stable`), as `replay.py` computes them.

**Pump**: a frame is one vsync tick (`psyq_vsync_tick`), from the game's `VSync()` or from `PLATFORM_WAIT()` ->
`port_wait()`, or from LIBCD's `StGetNext` once per 5000 empty polls (the movie player spins without a wait hook).
Each tick runs `port_frame` (`pump.c`): the CD tick (`psyq_cd_tick`), the frame log, the script's step, and the exit
at `--max-frames` (default 600; none with `--script`). `DW3_PORT_CHECKPOINT_DIR=<dir>` writes each checkpoint's PS1
image as `cpNN_<name>.bin`, named like `run.lua`'s dumps. A watchdog (`--watchdog SEC`,
default 10) exits 4 when no `port_wait()` ran for that long: a loop that no hook reaches (see below).

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
| `X <frame> status <status> <reason>` | The exit (`port_exit`) |

## The record (`--record FILE`)
JSON written at exit, with the keys of `tests/replay`'s records (`replay.py` `cross_core_view` reads it as it is):
`runner` (`"port"`), `status` (the exit status), `reason`, `frames`, `checkpoints` (`name`, `frame`, `stage`, `map`,
`random_index`, `gamestate_sha1`, `gamestate_sha1_stable`, as run.lua and replay.py record them), `overlay_sequence`
(`{frame, stage, file}`), `map_sequence` (`{frame, map}`), and `inputs` (`{frame, buttons: [names]}`, the names
sorted as run.lua sorts them) once the script has called `port_framelog_input`. The sequences follow run.lua's vsync
listener: an entry at every change, the first frame always (`{1, 0, 0}`, map 0).

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
- Movies are streamed (frame headers, timing) but not decoded (LIBPRESS is a stub until M5); no drawing (M2), no
  sound (M3), no memory card (M4: every card command reports "no card").
