# PC port plan (proposal)

**Status: section 4 decided by the user on 2026-10-05 (session 15; DECISIONS "PC port decisions (session 15)"):
every recommendation was taken.** The rest of the file is the proposal of 2026-10-04 (agent port-plan) as written. Background notes and ideas: `docs/PC_PORT_RESEARCH.md`.

The numbers below were **measured on the code at `a424d38`** (end of session 7). The tools that produced them
are described under "How this was measured"; at M0 they become a tracked `tools/port_inventory.py`.

## 1. Inventory: what the game C needs from the PS1

### 1.1 Starting point
- 388 C files (194,633 lines; 108,183 of them in `src/wstag/`). 3,606 game functions: 3,598 matching C, the 8
  holdouts as WIP C under `NON_MATCHING`, which all compile.
- **Host compile probe:** every C file compiled with the host GCC 13 (`-std=gnu99 -DNON_MATCHING`, with
  `INCLUDE_ASM` empty and the GTE macros stubbed), for `-m32` and `-m64`. **385 of 388 compile as they are.**
  The 3 that don't:
  - `src/main/heap.c`: the `$sp` inline asm (below).
  - `fieldstg_80087DB0.c` (`fieldstg_spots_create`) and `fightstg_80086A00.c` (`fightstg_enemy_check_condition`):
    each is called before it is declared, so the implicit `int f()` conflicts with the later definition.
    (Kept that way for matching.)

  With those 3 patched in a scratch copy, the **388 host objects link with no duplicate global symbol**. The naming
  rule's module prefix ("Naming conventions") and splat's per-overlay names already make one static link
  possible.
- Host `-O0` objects come to 1.5 MB of text and 1.6 MB of data. That is no reason to keep the overlays as loaded
  code.

### 1.2 Psy-Q functions the game calls (the shim's checklist)
The game calls **123 distinct Psy-Q functions at 251 call sites**. The EXE links 284 library objects, but the shim
only needs what the game calls; the libraries' own internals (`_spu_*`, `_padInit*`, `_card_*`, ...) never need
replacing.

| Library | Functions | Sites | Where called |
|---|---|---|---|
| LIBGPU | 24 | 63 | `gfx` 17, `main` 9, fieldstg 11, fightstg 8, stdwtitl 6, `message`/`sprite`/`tim`/`card`, cnty_sel |
| LIBGTE | 9 | 41 | cardgame, fightstg, `message`, `sprite`, stgmcard, stgtrain, stplnmet, wstag (`rsin`/`rcos`) |
| LIBSND | 24 | 28 | all in `sound.c`, except `SsInit` (`main`) and `SsSeqCalledTbyT` (`gfx`'s vsync callback) |
| LIBCD | 17 | 27 | `cdload` 12 (interrupt-driven sector reads), `main` 4, `filetable` 1, stdwtitl 10 (movie streaming `St*`, `CdRead2`) |
| LIBC2 | 6 | 24 | `strcpy`, `strlen`, `memcpy`, `strncpy` (game), `strcspn`, `atoi` (SHOCKTST WIP only); maps to host libc |
| LIBPAD | 11 | 19 | all in `pad.c` (direct + multitap init, actuator / DualShock modes) |
| LIBMCRD | 11 | 17 | all in `memcard.c` |
| LIBGS | 7 | 9 | `GsInitGraph`, `GsInit3D`, `GsGetTimInfo`, `GsSetProjection` (EXE); `GsSetRefView2`, `GsSetFlatLight`, `GsSetLightMode` (fightstg) |
| LIBETC | 4 | 9 | `VSync`, `VSyncCallback`, `ResetCallback`, `SetVideoMode` (EXE only) |
| LIBPRESS | 6 | 8 | MDEC (`DecDCT*`), STDWTITL movies only |
| LIBAPI | 4 | 6 | `open`/`read`/`write`/`close` of `sim:` PC-host files in SHOCKTST, a debug tool |

The function lists:
- **LIBGPU:** `SetDrawTPage` 12, `DrawSync` 8, `SetDefDispEnv` 8, `LoadImage` 4, `GetTPage` 3, `SetDrawMove` 3,
  `ClearOTagR` 3, `SetDispMask`, `SetSprt`, `PutDispEnv`, `ClearImage2`, `ResetGraph` 2 each, then 1 each:
  `SetDrawEnv`, `IsIdleGPU`, `BreakDraw`, `ContinueDraw`, `GetClut`, `MoveImage`, `SetGraphDebug`, `SetSemiTrans`,
  `DrawOTag`, `SetDefDrawEnv`, `ClearOTag`, `ClearImage`.
- **LIBGTE:** `rsin` 14, `RotMatrixYXZ_gte` 8, `ScaleMatrix` 7, `ApplyMatrixSV` 6, `rcos` 2, `InitGeom`,
  `SetBackColor`, `SetGeomOffset`, `RotMatrixZYX_gte`.
- **LIBSND:** `SsSepStop` 4, `SsSetTickMode` 2, then `SsInit`, `SsStart2`, `SsSetTableSize`, `SsSetMVol`,
  `SsSetSerialAttr`, `SsSetSerialVol`, `SsVabOpenHeadSticky`, `SsVabTransBody`, `SsVabTransCompleted`, `SsVabClose`,
  `SsSepOpen`, `SsSepPlay`, `SsSepClose`, `SsSepSetVol`, `SsSepSetDecrescendo`, `SsSeqCalledTbyT`,
  `SsUtSetReverbType`, `SsUtSetReverbDepth`, `SsUtReverbOn`, `SsUtAllKeyOff`, `SsUtKeyOn`, `SsUtKeyOff`. The game
  never calls LIBSPU directly.
- **LIBCD:** `CdInit`, `CdSetDebug`, `CdControl`, `CdControlB`, `CdControlF`, `CdGetSector`, `CdReadyCallback`,
  `CdSyncCallback`, `CdIntToPos`, `CdPosToInt`, `CdRead2`, `StSetRing`, `StSetStream`, `StGetNext`, `StFreeRing`,
  `StUnSetRing`, `StCdInterrupt`. `cdload` reads with CdlSetmode `0xA0` (double speed, 2340-byte sectors:
  `docs/FORMATS.md`), so the shim must hand out sector-level data with headers.
- **LIBPAD:** `PadInitDirect`, `PadInitMtap`, `PadStartCom`, `PadStopCom`, `PadChkVsync`, `PadGetState`,
  `PadInfoMode` 6, `PadInfoAct` 2, `PadSetAct` 2, `PadSetActAlign`, `PadSetMainMode` 2.
- **LIBMCRD:** `MemCardInit`, `MemCardStart`, `MemCardExist` 2, `MemCardAccept` 2, `MemCardReadFile` 3,
  `MemCardWriteFile` 3, `MemCardCreateFile`, `MemCardFormat`, `MemCardUnformat`, `MemCardSync`, `MemCardGetDirentry`.
- SDK **data** used by game C: only LIBGS's world-screen and camera matrices in its `.bss` (`D_800812F8`,
  `D_80081358`; `gfx` saves/restores them, FIGHTSTG feeds them to GTE macros). The shim's LIBGS must own them.
  The identity matrix `D_8004DC20` (FIGHTSTG) sits in the EXE's still-asm matrix block.

**Header-level PS1 API (in our own `include/psyq/`):**
- `libgpu.h`: 254 macro uses of 27 macros (`addPrim` 83, `setSemiTrans` 32, `getTPage` 29, `setPolyF4` 20,
  `setDrawTPage` 18, `setRGB0` 17, ...).
- 13 primitive types: `DR_TPAGE` 58 mentions, `POLY_FT4` 42, `POLY_F4` 34, `SPRT` 33, `DR_MOVE` 4, `POLY_G4` 4,
  `LINE_F2` 2, and one each of `POLY_FT3`, `POLY_GT3`, `POLY_GT4`, `LINE_F4`, `DR_ENV`, `POLY_G3`. Primitives are
  built only through these types and macros: no raw GP0 command words appear in game C. A few `setcode` /
  `->code =` calls set semi-transparency variants.
- The GPU command set a renderer must handle is therefore small: GP0 `0x20–0x3F` (polygons), `0x40`/`0x48` (lines),
  `0x64`/`0x66` (sprites), `0xE1` (draw mode), `0x80` (VRAM copy) and the `DR_ENV`/`0xE3–0xE6` environment, plus
  `LoadImage`/`MoveImage`/`ClearImage`.

### 1.3 GTE inline asm
- **63 uses of 29 `gte_*` macros, all in FIGHTSTG:** `fightstg_model.c` 48, `fightstg_8008D3B4.c` 15. That is
  battle models: meshes, lights, camera.
- The macros (`include/psyq/gtemac.h`, MIPS `cop2` words) are the only GTE access. Elsewhere the game uses the
  LIBGTE/LIBGS functions above.
- Most frequent: `ldv0_u` 6, `SetRotMatrix`/`SetTransMatrix`/`rtps`/`stsxy`/`CompMatrix` 4 each, `ncs`, `nclip`,
  `avsz3/4`, `gpf12`, `lddp`, `stotz`, ...
- One holdout, `fightstg_model_mesh_draw`, is GTE code (its WIP C uses the same macros).

### 1.4 Direct hardware, scratchpad and `$sp` tricks
- **No hardware register access in game C:** no `0x1F801xxx` constant anywhere.
- **Scratchpad:** one use. `heap_run_object` (`SET_SCRATCHPAD_STACK(0x1F8003FC)` / `RESTORE_STACK()`, raw `$29`
  inline asm in `heap.c`) runs every object's update on a 1 KB stack in the scratchpad. On the host both macros
  become empty and the normal stack is used. That is the only raw inline asm outside `gtemac.h`.
- **Interrupt-driven waits** (7 busy-wait loops), which need a host "interrupt pump":
  - `gfx_end_frame`'s `while (gfx_frame_pending != 0)` (a `volatile`, cleared by the vsync callback);
  - `cdload`'s `while (cdload_reader.is_busy())`;
  - `main`'s and STDWTITL's `while (CdControl(...) == 0)` (3);
  - FIGHTSTG's `while (IsIdleGPU(0) != 0)` around `BreakDraw`/`ContinueDraw`;
  - one `while (1)` in FIELDSTG.
- **VSync callback does real work:** the frame counters, the play time (`+0x100`/frame at 60 Hz, `+0x133` at 50 Hz),
  the display flip (`PutDispEnv`), and the **sound sequencer tick** (`SsSeqCalledTbyT`).

### 1.5 Overlays (fixed load addresses)
- **Tier 1** (19 files, `0x80082CB0`) and **tier 2** (2 WFIGHT + 293 WSTAG, `0x800A5DE0`) are loaded by `memcpy` into
  `main_overlay_base`/`main_file_base` (`overlay_load_stage`, `overlay_load_file`).
- **Each load also resets the overlay's `.data`/`.bss`,** because the file includes them. A load is skipped only when
  the same stage index (or file ID) is already resident. A static port has to emulate that reset.
- **Addresses that only mean something for "whatever is loaded in the slot"** (measured by a cast-of-constant scan):
  - `overlay_entries`: 20 tier-1 entry points (`OVERLAY_ENTRY(0x8008xxxx)`, already a macro).
  - `fieldstg_stages` / `fieldstg_stages_2d`: **293 WSTAG entry points** as `(void *(*)())0x800A5xxx` (118 of them are
    the same `0x800A5E28`). Each record also names its WSTAG file, so **every one can be resolved at build time.**
  - FIELDSTG's built-in stage (`func_FIELDSTG_80091E60`): **50 event-script pointers** `(s16 *)0x800A5DE0+` into the
    file then in the tier-2 slot. Which file is still to be identified; resolve as slot-relative offsets.
  - **4 late-bound functions:** `gamestate.c` calls `func_8008B770`, `func_8008BFA4` and `func_8008C000` (in the
    loaded tier-1 overlay; the comment says FIELDSTG), and FIELDSTG calls `func_800A6024` (in the loaded WSTAG).
  - `heap.c`: the heap bounds `0x800AB800`–`0x801FF000` (1,331,200 bytes) as constants.
- **Asm-only data still referenced by C:**
  - **106 `INCLUDE_RODATA` items:** mostly one-word WSTAG constants such as a stage colour, plus a few jump tables
    and `main`'s `main_overlay_base`/`C8`.
  - **Data blocks:** the EXE matrix block `0x8004DC10` (incl. `D_8004DC20`), FIELDSTG's zero block
    (`fieldstg_shatter_*`, `fieldstg_background_size`, ...) and STDWTITL's LIBPRESS data.
  - The **boot TIM** that ships inside the EXE image at `0x800A5DE0` (file offset `0x965E0`): `main` draws it from
    `main_file_base + 1`. The port reads it from the user's EXE.
  - **`WSTAG260`:** data-only, not built. Load it raw.

### 1.6 The 8 holdouts and the FAKE matches
The WIP C was compiled and probed as above; the holdouts' match percentages and sizes are below.

| Holdout (asm lines) | Match | Notes for the port |
|---|---|---|
| `stcrdshp_run_booster` (1,029) | 99.99% | scheduling only |
| `fieldstg_manager_update` (894) | 99.48% | |
| `fightstg_rules_get_stats` (689) | 98.2% | |
| `fightstg_model_mesh_draw` (592) | 99.2% | GTE |
| `wfightts_digimon_menu_update` (372) | 98.9% | |
| `stitshop_info_create_windows` (261) | 99.1% | |
| `shocktst_convert_table` (257) | 89% | debug tool (SHOCKTST) |
| `fieldstg_background_update_visible` (169) | 88.7% | field background streaming: **check by hand** |

The **9 FAKE matches** (`grep -rn "FAKE:" src`) are valid C with odd but equivalent forms (`volatile` casts,
`q - -count`, a local copy). They compile on the host as they are and need nothing for the port.

### 1.7 32-bit assumptions
- **Struct sizes:** the probe measured `sizeof` of all **282** typedef'd structs and unions in `include/`.
  - At `-m32`, **all 249 that document a PS1 size match it.**
  - At `-m64`, **166 change size**. 146 of the 282 hold a pointer directly; the rest embed a struct that does.
  - **None of the 166 is a disc-file layout** documented in `docs/FORMATS.md`. The ones that are documented
    (`FieldstgEventDef`, `FieldstgTalk`, `RecordsItem`, ...) are WSTAG/EXE data that is now **compiled from C**, so it
    re-lays itself out. The others are runtime objects.
- **Save data is pointer-free:** a slot is the first `0x26C4` bytes of `gamestate_data`, and every pointer field
  comes after `map` at `0x26C4`. Memory-card saves stay byte-compatible with the PS1 (and emulator `.mcd` files) on
  a 64-bit host.
- **Pointer/integer casts** (`-m64`, `-Wpointer-to-int-cast` / `-Wint-to-pointer-cast` / `-Wint-conversion`):
  - 83 come from the `libgpu.h` tag macros (`setaddr`/`getaddr` → `addPrim`).
  - **27 are game sites in 7 files**, mostly `(T *)(data[i] + (s32)data)` offset-table resolves (`cdload` 4,
    `fieldstg_80087DB0` 13, `gfx` 3 ...).
- **Ordering-table tags are 24-bit** (`P_TAG.addr : 24`), and `gfx_compact_ot` compares `(u32)ptr & 0xFFFFFF`
  with tags. This is a problem **even on a 32-bit host**. Two primitive arrays are globals (`fightstg_cursor_ot`,
  `fightstg_cursor_moves`); every other primitive and OT comes from the heap (packet buffers).
- **Hard-coded object sizes:** 48 of 565 `object_new` calls pass a literal size (e.g. `overlay_create_object`'s
  `0x50`), plus 13 heap allocations and 10 `bzero`s. Each is `sizeof(T)` on the PS1 and too small on 64-bit.
  Replacing them with `sizeof` changes no PS1 byte. Raw `(u8 *)obj + 0x..` field access is down to 1 site.
- **UB that the PS1 tolerates** (host `-Wall` at `-m64`):
  - 68 `-Wreturn-type`: 34 "control reaches end of non-void function", 34 `return;` in a non-void function.
  - 19 uninitialized or maybe-uninitialized reads.
  - 4 implicit declarations: `MoveImage`, `strlen`, and the 2 conflicts above. On 64-bit an implicit `int`
    return truncates a pointer.
  - 173 unprototyped declarations `f();` and 214 unprototyped function-pointer types. **C23 makes `()` mean
    `(void)`**, so the host build must pin `-std=gnu99`/`gnu11`.
  - Integer division has **no divide-by-zero check** (the toolchain has none) and would trap on x86.
  - Code relies on `char` being signed.

## 2. Approaches

### 2.1 How to run the game code
| | (a) Native C + Psy-Q shim **(recommended)** | (b) Static recompilation | (c) Embedded emulator core |
|---|---|---|---|
| What | Compile `src/` for the host. Implement the 123 Psy-Q functions over SDL3. GPU/GTE/SPU/MDEC/CD as C modules behind the shim | Translate the MIPS of EXE + overlays (+ Psy-Q) to C, run it on emulated hardware | Run the original binaries in an emulator; replace functions by native C step by step |
| Precedent | REDRIVER2 (Driver 2) via PsyCross; PsyDoom (native Doom code with emulated PS1 hardware parts from Avocado) | N64Recomp (Zelda 64: Recompiled) on N64 | The usual way of using decomps in emulators. Here: as a **test oracle**, not as the port |
| Pros | Readable, moddable, debuggable; 98.6% of the code is already C and it compiles (1.1); widescreen/enhancements become game-side changes | Exact, holdouts irrelevant | Exact; zero porting at the start |
| Cons | Shim and hardware modules to write; UB and 32-bit assumptions to clean up (1.7) | Throws the decomp's value away; still needs the full hardware emulation; 32-bit address space; mods hard | Ships Sony's libraries (from the user's disc) inside an emulator: not a port. Inherits an emulator's license |
| Effort | Large but bounded: 123 API functions, 13 primitive types, 1 sequencer | Large, and less useful | Small at first, then it never ends |

**Recommendation:** (a). Use an emulator (c) only as a **test oracle**: frame and VRAM captures, GPU-command traces
and SPU register-write traces from the original disc, compared with the port's. Do not link an emulator into the
port.

### 2.2 Own shim or fork PsyCross
PsyCross (MIT, checked 2026-10-04 at `e56e4cd`):
- **Size and backends:** ~10.6k lines of C/C++ over **SDL2 + OpenGL + OpenAL**.
- **Covers:** LIBGPU, LIBGTE, LIBCD (BIN/CUE), LIBPAD, LIBSPU, LIBMCRD, LIBETC and LIBAPI.
- **Does not cover:** **LIBSND** (all of the game's sound API), **LIBGS** or **LIBPRESS** (MDEC).
- **64-bit:** handled by `USE_EXTENDED_PRIM_POINTERS=1`, which widens `P_TAG.addr` to `uintptr_t` and so changes
  every primitive's size. It is tied to PGXP.

For this game PsyCross would cover about 60 of the 123 functions. Its renderer is GL-based rather than a VRAM-exact
rasterizer, and its audio backend (OpenAL) and SDL version are ones the research notes already plan to replace.

**Recommendation:** write our own shim, `port/psyq/`, against the checklist in 1.2. Use PsyCross as a reference;
code may be borrowed under MIT with a `THIRD_PARTY.md` entry. A fork is the alternative if the user wants the
fastest first picture; it would still leave sound, MDEC and LIBGS to us.

### 2.3 Rendering
- **First a software GPU:**
  - A 1024×512×16-bit VRAM and a rasterizer for the 13 primitive types: CLUT 4/8/15-bit, the 4 semi-transparency
    modes, dithering, mask bit, draw area/offset, `DR_MOVE`, `LoadImage`/`MoveImage`/`ClearImage`.
  - The game draws to VRAM and reuses it: FIELDSTG's shatter effect, `MoveImage`, FIGHTSTG's `DR_MOVE` cursor with
    `BreakDraw`/`ContinueDraw`, and 320×480 interlaced screens (`gfx_init_display`). Exact VRAM semantics first makes
    every screen right and makes **pixel-exact comparison with an emulator's software renderer** possible.
  - 320×240 at 50/60 Hz is trivial on a CPU. The result is shown through an SDL3 texture.
- **Later, a hardware renderer** (SDL3 GPU API, or OpenGL 3.3) for upscaling and the enhancements in
  `PC_PORT_RESEARCH.md`. It consumes the same primitive stream, and the software GPU stays as the reference.

### 2.4 OT tags, memory and 64-bit
**Memory arena.** A host "PS1 RAM" arena replaces the fixed regions:
- **The heap** (`0x800AB800`–`0x801FF000`; it can be made larger) holds everything `heap_funcs` hands out:
  packet buffers, OTs, file cache.
- **Tier-2 slot buffer:** for data loaded into the tier-2 slot (WSTAG260, the 50 script pointers).
- **Alignment:** the arena is allocated **16 MB-aligned** and smaller than 16 MB.

**Tags and casts:**
- The low 24 bits of any arena pointer are then its offset. The existing `setaddr` (`(u32)p` into a 24-bit field),
  `gfx_compact_ot`'s `(u32)q & 0xFFFFFF`, and `ClearOTagR`'s `0xFFFFFF` terminator all keep working **unchanged on
  32- and 64-bit**. `DrawOTag` in the shim walks the list as `arena + (tag & 0xFFFFFF)`.
- The two global primitive arrays in FIGHTSTG move into the arena in the port build.
- This keeps primitive layouts PS1-sized (unlike PsyCross's extended pointers) and needs no game-code change.
- The 27 `(s32)ptr` offset-table sites get a macro, e.g. `PTR_ADD(base, ofs)`. It expands to the current form on
  the PS1 (same bytes) and to pointer arithmetic on the host.

**64-bit:** runtime structs grow, which is harmless once the ~71 hard-coded sizes are `sizeof` (1.7). Disc formats
and save data are unaffected (1.7).

### 2.5 Overlays: link statically (recommended) or load as modules
- **Static (recommended):** link all 19 + 2 + 293 overlays into the binary; 1.1 shows no symbol clash. An
  **overlay manager** replaces the two `memcpy`s:
  - It snapshots every overlay's `.data`/`.bss` at startup and restores them on each load, under the same
    "different stage/file" condition the game uses. Each overlay's objects go into named sections with start/end
    symbols, via a GNU ld script.
  - It records the "current" tier-1 and tier-2 file.
  - It resolves the late-bound addresses (1.5) through tables generated from `config/*.symbols.txt` /
    `config/wstag/*.symbols.txt` (original address → host symbol, per overlay):
    - the 293 stage entries and the 20 `overlay_entries` **statically**;
    - the 4 late-bound functions **through the current overlay's table**;
    - the 50 script pointers **as slot-buffer offsets**.
- **As modules:** one `.so`/`.dll` per overlay, `dlopen`ed on load. That gives the data reset for free, but means
  313 shared objects, symbol export back into the executable, and Windows import libraries. Not worth it.

### 2.6 The 8 holdouts and the FAKEs
- The port builds with `-DNON_MATCHING`, so the **WIP C is the port's code**; matching is irrelevant there. For six of
  the eight the remaining difference is register allocation or instruction scheduling, which does not change
  behaviour.
- **Check by hand:** the two lowest, `fieldstg_background_update_visible` (88.7%) and `shocktst_convert_table`
  (89%).
- **Check by run:** a `NON_MATCHING` PS1 build (it compiles today) run in an emulator over the scenes that use them,
  compared with the original. If a WIP is wrong, fix the C; matching is not needed.
- **FAKEs:** keep as they are.

### 2.7 Sound
**Recommended:**
- **An SPU core of our own:** 24 voices, ADPCM, ADSR, pitch, the documented reverb (psx-spx). Several emulator SPUs
  are GPL or non-commercial; see the decisions.
- **A LIBSND reimplementation:** the 24 functions over a SEP/VAB sequencer, ticked from the vsync callback like the
  original.
- **Verification:** SPU register-write traces from the emulator for the same songs (exact for the sequencer), then
  by ear.

**Fallback if the sequencer can't be made faithful:** run the user's own copy of Sony's LIBSND/LIBSPU from
`SLES_039.36` in a small embedded R3000 interpreter against the same SPU core. That is exact, ships no Sony code,
but is more complex (shared memory with native code).

### 2.8 Disc, saves, input, movies
- **Disc:** the shim's LIBCD reads the **user's BIN/CUE** (SHA-1 checked against `457cb233…`, as the matching build
  does) at sector level. The file table's LBAs then work unchanged, including the 2340-byte mode and movie streaming
  (`St*`). A file-ID-keyed mod VFS (research notes) can sit on top later. No game data goes into the repo.
- **Saves:** LIBMCRD over raw 128 KB `.mcd` card images (the usual emulator format), so saves can move between
  emulator and port.
- **Input:** LIBPAD over the SDL3 gamepad API with keyboard fallback, multitap, and rumble (`PadSetAct`).
- **Movies:** 14 `MOVIE*.STR` (MDEC + XA ADPCM). Our own MDEC + XA decoder (a few hundred lines from psx-spx), or
  FFmpeg (LGPL; it has PS1 STR/MDEC/XA support). Before M5 movies are skipped (the stream reports its end).

## 3. Phased plan
Every milestone ends with a test that can be run.

**The matching build must not change: `scripts/build.sh --check` stays byte-identical after every source change.**

### M0: Groundwork in the matching tree (no host build yet)
**Done in session 15** (STATUS "PC port, M0"; the counts above were the plan's, the tree had more: 67 extra slot functions, 88 literal sizes, a fifth late-bound reference).
Only changes that leave every PS1 byte identical:
- Turn the 106 `INCLUDE_RODATA` items and the remaining asm data (matrix block, FIELDSTG zero block) into C
  (`tools/data_to_c.py`).
- Replace the ~71 literal sizes with `sizeof`.
- Wrap the late-bound addresses in macros, as `OVERLAY_ENTRY` already does: `WSTAG_ENTRY(0x800A5E28)`,
  `SLOT_PTR(...)`, the 4 late-bound calls.
- Add hook macros, empty on the PS1, for:
  - the 7 busy-waits (`PLATFORM_WAIT()`);
  - the scratchpad stack;
  - the 2 overlay `memcpy`s (`OVERLAY_COPY(...)`);
  - `PTR_ADD` for the 27 casts;
  - `PC_PORT` prototypes for the 2 implicit declarations.
- Track `tools/port_inventory.py`. It re-runs the counts in section 1, and its host-compile probe passes with
  `-Werror=pointer-to-int-cast,int-to-pointer-cast,int-conversion,implicit-function-declaration` at `-m64`.

**Test:** `build.sh --check`; the probe is clean.

### M1: Headless host build that boots the title logic
**Skeleton done in session 15** (STATUS "PC port, M1 skeleton"): the build, the arena, the overlay manager with its tables and data restore, the pump and the stub shim; the game's `main()` runs to a frame cap. **Done in session 16** (DECISIONS "M1: the headless port replays new_game"): LIBCD over the BIN, the scripted pad (the layer-2 scripts themselves), the per-frame log and record; the test below passes through FIELDSTG's first map (`tests/port/run.py --m32 --sanitize`).
- **Build:** `port/CMakeLists.txt`, with the `src/` file list generated from `configure.py`'s unit list
  (`-DNON_MATCHING -DPC_PORT -std=gnu99 -fno-strict-aliasing -fwrapv -fsigned-char`).
- **Stub shim:**
  - LIBGPU records the OT/primitive stream without drawing;
  - LIBCD over the BIN;
  - an interrupt pump drives vsync and CD callbacks deterministically from the busy-wait hooks;
  - no sound (LIBSND no-ops);
  - pad input from a script file.
- **Runtime pieces:** the arena (2.4), the overlay manager (2.5), and an `-m64` build (plus `-m32` if multilib is
  available).

**Test:** a headless run from boot through CNTY_SEL to STDWTITL's menu (movies skipped) with a scripted input:
- it logs overlay loads and a hash of the primitive stream per frame;
- two runs give identical logs, and `-m32` gives the same log as `-m64`;
- ASan + UBSan report nothing on this path.

### M2: Rendering
**Done in session 16** (DECISIONS "M2: the software GPU and the SDL3 window"): the GPU is pixel-checked against the
emulator's software renderer at the primitive level (715 golden cases) and whole-VRAM at seven `new_game` points; the
screen-level comparison at the other checkpoints listed below waits on matching timing (frames differ) and MDEC.
- The software GPU (2.3) with display environments (incl. 320×480 interlace) and an SDL3 window.

**Test:** pixel comparison with emulator captures (software renderer) at fixed checkpoints: country select, title,
first field map, field menu, a shop. Exact for 2D screens.

### M3: Input and sound
**Done in session 16** (DECISIONS "M3: sound"): input with M2's window; the SPU core and LIBSND, checked against the
emulator's SPU write traces (on the emulator's timeline: the port's own frames differ at CD loads), `--wav` and SDL3 audio.
- LIBPAD over SDL3 (gamepad and keyboard, rumble).
- The SPU core and LIBSND (2.7).

**Test:**
- SPU register-write traces vs the emulator for a set of BGM/SFX: same writes in the same tick order.
- Audio checked by ear.
- SOUNDTST (the debug sound test overlay) as the driver.

### M4: Save data
**Done in session 16:** LIBMCRD over `.mcd` images (a fresh card equal to PCSX-Redux's) and the round trips with the
emulator as a layer-3 test (`tests/saves/`).
- LIBMCRD over `.mcd` files, through `memcard.c` and STGMCARD.

**Test:** save, quit, load; load a save made in an emulator; the port's save loads in an emulator.

### M5: Full game
**Movies done in session 16:** our MDEC (`port/psyq/mdec.c`) and XA decoder (`port/psyq/xa.c`), checked against the PS1
and the emulator. Left: the rest below.
- GTE in C for FIGHTSTG: the 29 macros as calls into a software GTE with the PS1's fixed-point and flag behaviour,
  plus the LIBGTE/LIBGS functions.
- CARDGAME and every overlay.
- MDEC/XA movies.

**Test:**
- Every tier-1 overlay and all 293 WSTAGs entered at least once, through STAGSLCT (the debug stage select) with
  scripted input, with no sanitizer error.
- Battles compared against emulator captures.
- A full playthrough (manual, long).

### M6 (later, separate decisions)
Hardware renderer, enhancements, 60 Hz/NTSC option (the 2-byte patch, `records_60hz`), mod VFS, Windows CI build.

### Build-system changes
- The PS1 build (`configure.py` → `build.ninja`) is untouched.
- The port gets its own CMake project under `port/`, building out-of-tree in `build/port/` (already ignored).
- **New tools:** SDL3 (pinned, built from source into `tools/` by a new `setup.sh` step, per the CLAUDE.md tool rule).
  Optionally gcc-multilib for `-m32` (system-wide, so it needs the user's OK).
- **Generated tables:** the overlay address tables come from `config/` symbol files, by a Python generator run from
  CMake or committed as generated sources.
- **CI (when there is CI):** `build.sh --check` and the port's headless M1 test.

### Risks
| Risk | Mitigation |
|---|---|
| Behaviour that relies on PS1 UB: missing returns (68), uninitialized reads (19), unchecked out-of-bounds reads (e.g. `sprite_draw`'s frame search), divide by zero | `-fwrapv -fno-strict-aliasing -fsigned-char`; ASan/UBSan runs at every milestone; fix in the C with the PS1 bytes unchanged |
| Interrupt-driven code (vsync, CD) on a host thread | Single-threaded, deterministic interrupt pump at the 7 wait sites; no real threads at first |
| Overlay data-reset semantics and late-bound addresses | Overlay manager with snapshot/restore; generated tables; tests that enter every overlay |
| 64-bit layout assumptions not caught by the scans | `-m32` vs `-m64` trace comparison in M1; the probe gated at `-Werror` |
| WIP holdouts not equivalent | 2.6 checks; they are small and few |
| Audio fidelity (LIBSND's exact sequencing, ADSR, reverb) | SPU trace oracle; interpreter fallback (2.7) |
| Timing: instant CD reads or a fast host change behaviour (cdload state machines, fades) | The pump can simulate CD latency; vsync-paced main loop as on the PS1 |
| Source drift between the two builds | Hooks are macros that are empty on the PS1; `build.sh --check` gates every change |
| Licence contamination | Decision 8 |

## 4. Decisions needed from the user
**All decided on 2026-10-05 (session 15); each item's outcome is marked "Decided". Record: DECISIONS "PC port
decisions (session 15)".**
1. **Approach:** our own Psy-Q shim against the 123-function checklist, with PsyCross as an MIT reference *(recommended)*,
   or fork PsyCross for bring-up.
   **Decided:** own shim (`port/psyq/`), PsyCross as an MIT reference only.
2. **Renderer:** a VRAM-exact software GPU first, then a hardware renderer on the SDL3 GPU API *(recommended)*. Or go
   straight to OpenGL 3.3 / SDL3 GPU and skip exactness.
   **Decided:** software GPU first, hardware renderer later.
3. **SDL2 or SDL3:** SDL3 *(recommended: current major version, GPU API, gamepad and audio-stream APIs)*. SDL2 only
   if we fork PsyCross as it is.
   **Decided:** SDL3, pinned and built into `tools/`.
4. **32- or 64-bit host:** 64-bit *(recommended)*, with the measured cleanup (2.4, M0). Plus a headless `-m32` build
   as a layout oracle during M1, which needs `gcc-multilib` installed system-wide (sudo: your call; without it,
   rely on the probe and sanitizers).
   **Decided:** 64-bit host; the `-m32` layout oracle is wanted for M1. The user installs the 32-bit glibc
   headers themselves (on the Nobara dev machine `sudo dnf install glibc-devel.i686`, measured: `gcc -m32` does not link
   without it; `gcc-multilib` is the Ubuntu/CI name). Not needed for M0.
5. **Overlays:** static link with an overlay manager (data snapshot/restore, generated address tables)
   *(recommended)*, or one shared library per overlay.
   **Decided:** static link with an overlay manager.
6. **Sound:** our own SPU core and LIBSND reimplementation, checked against emulator SPU traces *(recommended)*; the
   interpreter running the user's LIBSND is the fallback.
   **Decided:** own SPU core and LIBSND; the interpreter stays the fallback.
7. **Disc input:** LIBCD over the user's BIN/CUE, hash-checked *(recommended)*, rather than extracted files.
   **Decided:** the user's BIN/CUE, hash-checked.
8. **Licensing:** pick the repo's licence before borrowing code. MIT *(recommended: matches the US decomp and
   PsyCross)*.
   - Under MIT, allow only MIT/BSD/zlib code in the port.
   - Emulators (GPL Mednafen/Beetle, PCSX-Redux; DuckStation is non-commercial/no-derivatives since 2024) are used
     only as external test oracles, never linked or copied.
   - FFmpeg (LGPL) only if dynamically linked, or not at all (own MDEC/XA decoder recommended).
   **Decided:** MIT, only MIT/BSD/zlib code in the port, emulators as external oracles only (DECISIONS "Going
   public"; confirmed unchanged). Own MDEC/XA decoder, so no FFmpeg.
9. **Repo layout:** a `port/` directory in this repo (shim, platform layer, CMake), plus hook macros in
   `include/` and a few `#ifdef PC_PORT` in `src/`, each verified byte-identical *(recommended)*. Or a separate
   repo that uses this one as a submodule (no port code in `src/` at all, but the hooks still have to live here).
   **Decided:** `port/` in this repo (DECISIONS "Going public"; confirmed unchanged).
10. **Movies:** our own MDEC + XA decoder *(recommended)* or FFmpeg; either way not before M5.
   **Decided:** own MDEC + XA decoder, not before M5.
11. **Frame rate:** PAL 50 Hz by default (faithful EU), with 60 Hz as an option through `records_60hz`
    *(recommended)*.
   **Decided:** PAL 50 Hz by default, 60 Hz as an option.

## How this was measured
Scratch scripts (agent port-plan; to be tracked as `tools/port_inventory.py` at M0), run from the repo root after
`configure.py` (for `include/asm_generated`).

**Psy-Q call inventory:**
- SDK names and their library/object come from the `// LIBxxx.LIB/OBJ.OBJ` comments in `config/symbol_addrs.txt`
  and `config/stdwtitl.symbols.txt` (LIBPRESS).
- Every `src/**/*.c` and `include/*.h` is scanned with comments and strings stripped. Identifiers that follow `.` or
  `->` and prototypes are skipped. Calls are counted per module, with `NON_MATCHING` branches counted separately.
- `libgpu.h`/`gtemac.h` macro uses are counted the same way.
- Hand corrections: `open`/`close`/`read`/`write` are also module-struct fields; the real LIBAPI calls are SHOCKTST's 6.
  `bzero` in `heap.h` is `heap_funcs.bzero`, not LIBC2.

**Host compile probe:**
- `include/` is copied to scratch. `include_asm.h` is replaced by empty `INCLUDE_ASM`/`INCLUDE_RODATA`, and each
  `gte_*` macro by `((void)0)`.
- Each C file is compiled with `gcc -m32`/`-m64 -std=gnu99 -O0 -fno-builtin -fsigned-char -DNON_MATCHING -Wall
  -Wpointer-to-int-cast -Wint-to-pointer-cast -Wint-conversion -Wincompatible-pointer-types`, and the warnings are
  counted by flag.
- The 3 failing files are patched in scratch to finish the link probe.

**Link probe:** `nm` over the 388 objects: globals defined twice (0), and symbols undefined everywhere (the Psy-Q
functions, the asm-only data, the late-bound addresses).

**Struct sizes:** for every `} Name;` typedef in `include/**/*.h`, a `char size__Name[sizeof(Name)]` compiled at
`-m32` and `-m64`, read back with `nm -S`, and compared with the `/* size 0xNN */` comments.

**Fixed addresses:** pointer casts of `0x80xxxxxx` constants (and `OVERLAY_ENTRY`), classified by memory region.
Literal sizes: `object_new`, `heap_funcs.alloc*` and `bzero` calls with a hex size argument.
