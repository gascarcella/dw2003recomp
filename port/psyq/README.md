# port/psyq: the Psy-Q shim

Our own implementation of the Psy-Q library functions the game calls (`docs/PC_PORT_PLAN.md` 1.2 and 2.2;
DECISIONS "PC port decisions (session 15)"): one C file per library, against the prototypes in `include/psyq/*.h`.
MIT, like the repo. Written from the game's own use of the API and public hardware documentation; no SDK file, no
emulator code (PsyCross, MIT, was consulted for signatures only).

**M1 skeleton: headless, records, no drawing, no sound.** Every stub returns what lets the game go on; nothing
allocates host memory the game sees, and nothing stores a game pointer in a 32-bit field.

## Interface to the port runtime: `psyq.h`
`psyq_vsync_tick()` and `psyq_cd_tick()` are the "interrupts": the port's pump (`port_wait()`) calls them from the
game's busy-waits. `psyq_set_trace()` turns tracing on. `port_unimplemented()` is declared here and defined by the
runtime (no stub needs it today). The optional extras (`psyq_set_arena`, `psyq_cd_set_reader`,
`psyq_cd_set_timing`, `psyq_pad_set`, `psyq_gpu_take_hash`) are what the M1 test ("boot to STDWTITL's menu with scripted input, a primitive-stream hash
per frame") plugs into; the runtime may ignore them.

## What is real, what is a stub
| Library | Real (computes the right answer) | Fixed answer / recorded only |
|---|---|---|
| LIBGPU `libgpu.c` | `SetDefDispEnv`, `SetDefDrawEnv`, `SetDrawEnv` (the DR_ENV words), `SetDrawTPage`, `SetDrawMove`, `GetTPage`, `GetClut`, `SetSemiTrans`, `SetSprt`, `ClearOTag`, `ClearOTagR`; `DrawOTag`/`ContinueDraw` walk the list and hash the primitives | `DrawSync` 0, `ResetGraph` 0, `SetGraphDebug` 0, `SetDispMask`, `PutDispEnv` (returns env), `ClearImage`/`ClearImage2` 0, `LoadImage`, `MoveImage` 0, `IsIdleGPU` 0 (idle), `BreakDraw` (an empty list, so FIGHTSTG's cursor copies are still recorded) |
| LIBGTE `libgte.c` | `rsin`, `rcos` (computed 4096-entry table), `RotMatrixYXZ_gte`, `RotMatrixZYX_gte`, `ScaleMatrix`, `ApplyMatrixSV` | `InitGeom`, `SetGeomOffset`, `SetBackColor` (values kept for M2) |
| LIBGS `libgs.c` | `GsGetTimInfo` (parses the TIM header); owns `D_80081358` (world-screen matrix) and `D_800812F8` (flat-light matrix) | `GsInitGraph`, `GsInit3D` (both matrices = identity), `GsSetProjection`, `GsSetLightMode`, `GsSetFlatLight` (stores the raw direction in its row), `GsSetRefView2` 0 (leaves the matrix) |
| LIBETC `libetc.c` | `VSyncCallback` (stores, returns the previous), `SetVideoMode` (returns the previous), `VSync` (ticks the vsync inline for modes 0 and > 1, returns the count) | `ResetCallback` 0 |
| LIBCD `libcd.c` | `CdIntToPos`, `CdPosToInt`; the command model: `CdControl`/`CdControlB` apply at once and return 1, `CdControlF` completes on the next tick with `CdlComplete` to the sync handler; a read delivers its sectors from the sector source (the BIN, `port/src/disc.c`), one `CdlDataReady` call per sector, at the drive's rate in vsync ticks (`psyq_cd_set_timing`: "realistic" = 3 sectors per tick at double speed, 1.5 at single, after a seek of 3..40 ticks; "instant" = up to 75 per tick, no seek), stops at a command a handler issues, ends with `CdlDataEnd` past the source's end; `CdGetSector` reads through the delivered sector in the size the mode byte selects; `CdReadyCallback`/`CdSyncCallback` store and return the previous. The movie stream: `CdRead2` streams from the Setloc position at the drive's rate, video sectors (StHEADER magic `0x80010160`) are assembled into whole frames in the game's `StSetRing` buffer (a slot = the 32-byte StHEADER + the frame's data), XA audio is skipped; `StGetNext` hands out the oldest complete frame (and runs a vsync tick every 5000 empty polls, as the PS1's interrupts run while the player spins), `StFreeRing` releases it, `StSetStream` keeps the frame range, `StUnSetRing` ends the stream. Owns `D_80081454` (StCdIntrFlag, always 0) | `CdInit` 1, `CdSetDebug` 0, `StCdInterrupt` (nothing is deferred); `StSetStream`'s callbacks are not called (the game passes none) |
| LIBPAD `libpad.c` | A digital pad on port 0 (none on port 1): `PadInitDirect`/`PadInitMtap` fill the buffers (status 0, id 0x41, buttons active low), `PadChkVsync` 1 once per vsync tick, `PadGetState` 6 (stable) / 0, `PadInfoMode(…, 2, …)` 4 (digital) | `PadStartCom` 0, `PadStopCom`, `PadInfoAct` 0, `PadSetAct`, `PadSetActAlign` 0, `PadSetMainMode` 0 |
| LIBMCRD `libmcrd.c` | — | no card: the asynchronous commands return 1 (accepted) and `MemCardSync` reports them done with result 1 (`McErrCardNotExist`); `MemCardCreateFile`/`Format`/`Unformat`/`GetDirentry` return 1 at once |
| LIBSND `libsnd.c` | — | every call a no-op reporting success: `SsVabOpenHeadSticky`/`SsVabTransBody` return the id given, `SsVabTransCompleted` 1, `SsSepOpen` a fresh access number, `SsUtKeyOn` a voice number, `SsUtKeyOff` 0, `SsUtSetReverbType` 0 |
| LIBPRESS `libpress.c` | — | nothing is decoded or written; `DecDCTout` runs the `DecDCToutCallback` handler at once (STDWTITL's decode wait needs it), `DecDCTvlc2` 0 |
| LIBC2, LIBAPI | the host libc (see below) | — |

Nothing calls `port_unimplemented` yet: every function the game uses has a fake result that lets it continue.

### LIBC2 and LIBAPI
`strlen`, `strcpy`, `strncpy`, `memcpy`, `strcspn`, `atoi` and `open`, `read`, `write`, `close` are **not** defined by
the shim: they resolve to the host libc. A definition in the executable would replace libc's for every shared
library in the process (SDL included), and the libc ones already do what the game wants: `open("sim:C:\\...")`
fails with -1 (SHOCKTST checks for it), the string functions are the same functions. The prototype differences in
`include/psyq/libc2.h`/`libapi.h` (`s32` returns and lengths where libc has `size_t`) are harmless on the LP64 ABIs
(x86-64, AArch64): the low 32 bits of the return register and a 32-bit length register are what both sides use.

## Tracing
`DW3_PORT_TRACE=1` in the environment (decided at the first stub call), or `psyq_set_trace(1, stream)`, logs one line
per stub call (`psyq: CdControlF 02 param ...`) to stderr or the stream given. Off, each call costs one predictable
branch. The two hot paths that run every frame are not traced: `SsSeqCalledTbyT` and `StGetNext`/`PadChkVsync`
polls. `DrawOTag` logs the primitive count and the running hash per call. LIBCD logs commands (with only the
parameter bytes each takes), each read's or stream's start (sector, head, seek ticks, speed), the sector range a tick
delivered, and per movie frame its arrival, `StGetNext` and `StFreeRing`; a ring position is an offset in the ring.

## The primitive stream
`DrawOTag`/`ContinueDraw` follow the 24-bit tags (`PC_PORT_PLAN.md` 2.4: `(ot & ~0xFFFFFF) + (tag & 0xFFFFFF)`),
only inside the window `psyq_set_arena` gave (by default the heap, `port_heap_start..port_heap_end`; a link outside
it stops the walk with a trace line). Every primitive's `len` words after its tag go into an FNV-1a hash that
`psyq_gpu_take_hash` returns and resets: the M1 test's "hash of the primitive stream per frame".

## Behaviour assumed, to verify against the emulator later
Each file's header comment lists its own; the ones a later milestone must check first:
- **LIBGTE (M2):** Psy-Q's sine table is reproduced as round-to-nearest of `4096 * sin` (Sony's may differ by 1 in
  places); matrix products are 64-bit then `>> 12` with sign (the GTE's rounding); `RotMatrixYXZ` is `Ry*Rx*Rz`
  and `RotMatrixZYX` is `Rz*Ry*Rx` with Psy-Q's right-handed matrices; `ScaleMatrix` scales column j by `v_j`. The
  layer-1 oracle (`tests/golden/oracle.py`) can call these on the PS1 and dump the words.
- **LIBGPU (M2):** `SetDefDrawEnv`'s `dfe = (h <= 256)`; `SetDrawEnv`'s packet is E1, E2, E3, E4, E5 (+ the 0x02
  fill with isbg) and nothing else; `SetDrawMove`'s five words; what `BreakDraw` returns while the GPU is idle.
- **LIBGS (M2):** `GsSetRefView2`'s matrix (not computed here); whether `GsSetFlatLight` normalises the direction;
  what `GsInit3D` resets.
- **LIBCD:** verified with the BIN (session 16): mode `0xA0` (cdload) gets the 2340-byte window from the 12-byte
  offset, header + subheader + data, and `cdload_check_sector` accepts every sector (boot, sound banks, CNTY_SEL and
  STDWTITL load at both timings); `CdlReadN` acknowledging with `CdlComplete` before its first sector is the order
  cdload's state machine runs through; several sectors per tick (one handler call each) and a `Pause` issued from the
  ready handler (no later sector delivered) work; the movie stream (mode `0x1E0`) plays MOVIEOPN.STR's 1776 frames
  in order at 15 frames/s in "realistic" timing (5916 vsyncs from frame 1 to 1776 = its real 118.3 s) and ends by
  itself (`stdwtitl_movie_done`). Still assumed: a blocking `CdControl` runs no sync handler; the `0x10` window
  (2328 bytes); the seek times (a deterministic stand-in: 3 ticks + 1 per 8192 sectors, at most 40); what LIBCD does
  when the ring is full ("realistic" drops the frame, "instant" waits); the 5000 `StGetNext` polls per vsync (an
  estimate of the PS1 loop's speed); our ring layout (LIBCD's own is not documented publicly; the game only needs
  `*addr` contiguous and `*header`'s frameCount, width and height).
- **LIBPAD (M3):** the mode ids (4 digital, 7 analog), state 6, a digital pad without actuators.
- **LIBMCRD (M4):** the command numbers in `*cmds` and that a missing card is result 1 for every command.
- **LIBPRESS (M5):** running the out-callback synchronously (the real MDEC runs it from the DMA end).

## Checks
`port/psyq/check.sh` (no CMake, no SDL): compiles every file with the port's flags and `-Wall -Wextra -Werror`,
archives `build/port_psyq/libpsyq.a`, then checks coverage against `tools/port_inventory.py`'s probe objects
(`build/port_inventory/m64/obj`, made by `port_inventory.py probe`): every one of the 123 Psy-Q functions plus the
three data symbols is defined by the shim or by the host libc, no global is defined twice, and what stays undefined
is the game's own asm-only data and the runtime's `port_*`. `EXTRA="-O2 -fsanitize=address,undefined"
port/psyq/check.sh --compile` compiles with more flags.
