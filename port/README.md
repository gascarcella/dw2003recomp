# port/: the PC port

The game's C (`src/`, 388 units) linked with the port runtime (`port/src/`) and our Psy-Q shim (`port/psyq/`) into
one 64-bit host binary (`docs/PC_PORT_PLAN.md`; DECISIONS "PC port decisions (session 15)"). The PS1 build
(`configure.py`, `build.ninja`) is untouched: the hooks in `include/port.h` expand to the original code without
`PC_PORT`, and `scripts/build.sh` proves the EXE and the overlays byte-identical.

**M1 skeleton (this state):** headless, no rendering, no sound, no disc. `build/port/dw2003` runs the game's `main()`
against the shim's stubs and ends at a frame cap.

## Build and run
```sh
cmake -S port -B build/port -G Ninja      # CMake >= 3.20; Ninja or Make; GCC (or Clang) with GNU ld
cmake --build build/port                  # ~30 s from scratch with -j6
build/port/dw2003 --max-frames 60         # exit 0 at the frame cap; 3 from port_unimplemented; 2 PLATFORM_HALT; 4 watchdog
build/port/dw2003 --trace                 # every tick, overlay load/resolve and stub call, to stderr
```
Options: `-DDW3_PORT_SANITIZE=ON` (`-fsanitize=address,undefined`; needs libasan/libubsan installed),
`-DDW3_PORT_ALLOW_UNRESOLVED=ON` (link with undefined symbols ignored: a private build without `port/psyq/`),
`-DDW3_PORT_PSYQ_DIR=<dir>` (another shim directory), `-DDW3_PORT_UNIT_OVERRIDES="src/main/x.c=<path>;..."`
(experiments: build a unit from another file, the tree untouched), `-DDW3_PORT_PSYQ_WERROR=OFF`.

## Layout
| Path | Contents |
|---|---|
| `CMakeLists.txt` | The build (below) |
| `include/port_runtime.h` | The runtime's internal interface (tables, arena, pump, logging) |
| `src/main.c` | Options, setup, `game_main()` (the game's `main`, renamed by `-Dmain=game_main` on `src/main/main.c`) |
| `src/arena.c` | The memory arena: `port_arena`, `port_ptr_to_s32`/`port_s32_to_ptr`, the stand-in BIOS |
| `src/overlay.c` | The overlay manager: `port_overlay_load` (snapshot restore), `port_overlay_resolve` (tag -> function) |
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
function pointer and comes back unchanged.

**Pump**: `PLATFORM_WAIT()` -> `port_wait()` runs one vsync tick (`psyq_vsync_tick`) and one CD tick
(`psyq_cd_tick`), counts a frame, and exits 0 at `--max-frames` (default 600). A watchdog (`--watchdog SEC`,
default 10) exits 4 when no `port_wait()` ran for that long: a loop that no hook reaches (see below).

## Known gaps (M1 skeleton)
- **Two CD polling loops have no `PLATFORM_WAIT()` hook** (`src/main/cdload.c` `cdload_load_file`:
  `do { cdload_update(); } while (cdload_is_loading(id))`, and `src/main/sound.c` `sound_init`:
  `while (sound_is_loading()) { ... }`). On the PS1 the CD interrupt ends them; on the host nothing runs the
  shim's tick inside them, so the first file load spins until the watchdog (exit 4). Adding `PLATFORM_WAIT();` as
  the last statement of each loop body is byte-identical on the PS1 (verified with `scripts/build.sh`) and lets the
  run reach the frame cap; with a sector source for the shim it then boots to CNTY_SEL (verified in an experiment
  through `DW3_PORT_UNIT_OVERRIDES`). Those two files are not the port's to change; it is the first thing to do
  in `src/`.
- No disc: the shim's reads end with no data (the game retries). `psyq_cd_set_reader` over the user's BIN is the
  next step (M1 "LIBCD over the BIN"), hash-checked (DECISIONS item 7).
- The BIOS is a stand-in: `BIOS_PTR` serves a 256-byte region at `0x1FC00100` holding a version string.
- Windows/macOS: the ld script (`INSERT`, `-T`) and the 16 MB `.bss` alignment are GNU ld/ELF; PE needs another
  arrangement for the arena (allocate at startup; `port.h` would need the slot symbols as pointers) and for the
  per-overlay sections.
- The snapshot copies with plain byte loops in `no_sanitize_address` functions (ASan's redzones between globals
  are inside the ranges); unverified under ASan on this machine (no libasan installed).
