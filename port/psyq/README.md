# port/psyq: the Psy-Q shim

Our own implementation of the Psy-Q library functions the game calls (docs/PORT.md "The Psy-Q shim";
DECISIONS "PC port architecture"): one C file per library, against the prototypes in `include/psyq/*.h`.
MIT, like the repo. Written from the game's own use of the API and public hardware documentation; no SDK file, no
emulator code (PsyCross, MIT, was consulted for signatures only).

**M1 skeleton: headless, records** (but a real GTE: `gte.c`, since M2 a real GPU: `gpu.c`, and since M3 a real LIBSND: `libsnd*.c`). Every stub returns what lets the game go on; nothing
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
| LIBGPU `libgpu.c` | `SetDefDispEnv`, `SetDefDrawEnv` (`dfe = h < 289`), `SetDrawEnv` (the PS1's packet: E3, E4 clamped to the VRAM, E5, E1, E2, E6, and with `isbg` a TILE over the clip area), `SetDrawTPage`, `SetDrawMove`, `GetTPage`, `GetClut`, `SetSemiTrans`, `SetSprt`, `ClearOTag`, `ClearOTagR`; `DrawOTag`/`ContinueDraw` walk the list, hash the primitives and draw them; `LoadImage`, `MoveImage` (-1 and nothing for an empty rectangle), `ClearImage`/`ClearImage2` (a fill, or E1 + a TILE when x or w is not a multiple of 64, as LIBGPU does) into the VRAM; `ResetGraph` 0/3 resets the drawing state; `BreakDraw` NULL (idle, as on the PS1); `psyq_gpu_vram`/`psyq_gpu_display` (the video output, `psyq.h`) | `DrawSync` 0, `IsIdleGPU` 0 (drawing completes when queued), `SetGraphDebug` 0, `SetDispMask`, `PutDispEnv` (returns env, stores the display) |
| GPU `gpu.c` | The GPU in software (session 16, M2): a 1024x512 VRAM of 16-bit pixels and every GP0 drawing command (polygons flat/Gouraud/textured with 4/8/15-bit textures and CLUTs, the texture window, raw and modulated texels, the four blend modes, dithering, mask set/check, lines and polylines, rectangles, fill, VRAM copy, CPU-to-VRAM transfer; E1..E6), the draw area and offset; the emulator's rasterisation rules taken over where documentation is silent (`gpu.c`'s header) | GP1 beyond the drawing state; timing, the texture cache, VRAM-to-CPU |
| GTE `gte.c` | The geometry coprocessor in software (session 16): the 64 registers with their read/write rules, every command (RTPS/RTPT with the UNR division, NCLIP, OP, DPCS/DPCT, INTPL, MVMVA, NCDS/NCDT, CDP, NCCS/NCCT, CC, NCS/NCT, SQR, DCPL, AVSZ3/4, GPF, GPL), FLAG, the saturations; the game's `gte_*` macros call it (`tools/port_gen.py overrides` translates each MIPS sequence of `include/psyq/gtemac.h` into the same register accesses: `psyq_gte_mtc2/mfc2/ctc2/cfc2/cmd`, declared in `psyq_internal.h`) | — |
| LIBGTE `libgte.c` | `rsin`, `rcos` (computed 4096-entry table), `RotMatrixYXZ_gte`, `RotMatrixZYX_gte` (LIBGTE's GPF sequence), `ScaleMatrix`, `ApplyMatrixSV` (MVMVA); the register setters write the GTE: `InitGeom` (ZSF3/4, H, DQA/DQB, OFX/OFY), `SetGeomOffset`, `SetBackColor`, `SetGeomScreen` (LIBGS's `GsSetProjection`), and for LIBGS (not called yet: see below) `SetFarColor`, `SetColorMatrix`; for LIBGS's view: `MulMatrix`, `MulMatrix2`, `ApplyMatrixLV` (MVMVA), `TransposeMatrix`, `SquareRoot0` (LZCS/LZCR and the table) | — |
| LIBGS `libgs.c` | `GsGetTimInfo` (parses the TIM header); `GsSetProjection` (H); `GsSetRefView2` (the world-screen matrix from viewpoint, reference point, twist and `super`) and `GsGetLw` (the coordinate hierarchy, PSDCNT 1), checked against the PS1 by the layer-1 family `libgs_view` (issue #7: the battle camera; `tests/host/libgs_replay.py`, with LIBGTE's `MulMatrix`/`ApplyMatrixLV`/`TransposeMatrix`/`SquareRoot0`); `GsInitGraph`'s view base (the aspect in `m[1][1]`); owns `D_80081358` (world-screen matrix) and `D_800812F8` (flat-light matrix) | `GsInitGraph` (records the rest), `GsInit3D` (both matrices = identity), `GsSetLightMode`, `GsSetFlatLight` (stores the raw direction in its row) |
| LIBETC `libetc.c` | `VSyncCallback` (stores, returns the previous), `SetVideoMode` (returns the previous), `VSync` (ticks the vsync inline for modes 0 and > 1, returns the count) | `ResetCallback` 0 |
| LIBCD `libcd.c` | `CdIntToPos`, `CdPosToInt`; the command model: `CdControl`/`CdControlB` apply at once and return 1, `CdControlF` completes on the next tick with `CdlComplete` to the sync handler; a read delivers its sectors from the sector source (the BIN, `port/runtime/disc.c`), one `CdlDataReady` call per sector, at the drive's rate in vsync ticks (`psyq_cd_set_timing`: "realistic" = 3 sectors per tick at double speed, 1.5 at single, after a seek of 3..40 ticks; "instant" = up to 75 per tick, no seek), stops at a command a handler issues, ends with `CdlDataEnd` past the source's end; `CdGetSector` reads through the delivered sector in the size the mode byte selects; `CdReadyCallback`/`CdSyncCallback` store and return the previous. The movie stream: `CdRead2` streams from the Setloc position at the drive's rate, video sectors (StHEADER magic `0x80010160`) are assembled into whole frames in the game's `StSetRing` buffer (a slot = the 32-byte StHEADER + the frame's data); XA audio sectors (Mode 2, submode audio + real time, with Setmode bit `0x40`; `CdlSetfilter`'s file/channel with bit `0x08`) go to the XA decoder in a read as in the stream, never to the CPU, and reach the SPU's CD input (`spu_cd_input`) as 882 frames per tick after a one-tick wait (M5, docs/SOUND.md "CD audio"); `StGetNext` hands out the oldest complete frame (and runs a vsync tick every 5000 empty polls, as the PS1's interrupts run while the player spins), `StFreeRing` releases it, `StSetStream` keeps the frame range, `StUnSetRing` ends the stream (and, as `Pause`, a new read, `CdInit` and the reset, flushes the XA audio). Owns `D_80081454` (StCdIntrFlag, always 0) | `CdInit` 1, `CdSetDebug` 0, `StCdInterrupt` (nothing is deferred); `StSetStream`'s callbacks are not called (the game passes none); the drive's volume matrix (`CdMix`, ATV0..3) stays at unity and Mute/Demute are not modelled (the game uses neither) |
| XA `xa.c` | The drive's XA-ADPCM decoder (psx-spx "CDROM XA Audio ADPCM Compression": 18 sound groups, 4- and 8-bit units, the four filters, ranges 0..12 and 13..15 as 9, mono/stereo, 37,800/18,900 Hz) and its 37,800 -> 44,100 Hz "zigzag" resampler (the seven 29-point tables); libcd.c drives it. Checked against an independent Python model (`tests/xa/`: 1317 unit checks; MOVIEOPN.STR's 2261 audio sectors decode to identical PCM) and against the emulator's audio (the movie's soundtrack, correlation 1.0000 per 0.1 s window) | emphasis (coding bit 6: its formula is unknown, no game uses it) is not applied; the 18,900 Hz path (each sample played twice) is our reading; the six-step counter starts at 6 |
| LIBPAD `libpad.c` | A digital pad on port 0 (none on port 1): `PadInitDirect`/`PadInitMtap` fill the buffers (status 0, id 0x41, buttons active low), `PadChkVsync` 1 once per vsync tick, `PadGetState` 6 (stable) / 0, `PadInfoMode(…, 2, …)` 4 (digital) | `PadStartCom` 0, `PadStopCom`, `PadInfoAct` 0, `PadSetAct`, `PadSetActAlign` 0, `PadSetMainMode` 0 |
| LIBMCRD `libmcrd.c` | The cards over raw 128 KB `.mcd` images (`port/runtime/memcard.c` inserts them with `psyq_mcrd_set_card`; the layout: docs/FORMATS.md "The card image"): `MemCardExist`/`Accept`/`ReadFile`/`WriteFile` register an asynchronous command (1; 0 while one is pending) that `MemCardSync` reports done with its number (1 Exist, 2 Accept, 3 ReadFile, 4 WriteFile) and LIBMCRD's result (0 none, 1 no card, 2 invalid, 3 new card, 4 not formatted, 5 no such file); `MemCardCreateFile` (first free blocks; 6 exists, 7 full), `Format` (the directory a new PCSX-Redux card has), `Unformat` (frame 0 cleared), `GetDirentry` (`*`/`?` patterns; name, size, head) answer at once. The first access after insertion or `psyq_mcrd_reset` answers 3 (new card) and clears the flag, as in the emulator. Every change of an image goes back to its file | `MemCardInit`, `MemCardStart`; the timing: a command completes at the 2nd (Exist), 4th (Accept) or (1 + bytes/128)th (a transfer) `MemCardSync` poll, `MemCardSync(0, …)` at once (the emulator's card is slower; the scripts wait on `memcard_state`) |
| LIBSND `libsnd*.c` | The 24 functions the game calls, over the SPU core (`port/include/spu.h`; M3, session 16, docs/SOUND.md section 7): VAB headers and bodies (DMA to the PS1's SPU addresses; `SsVabTransCompleted` 0 until the DMA's completion), SEPs and the score table (with its aliasing), the sequencer at `SsSetTickMode`'s rate (notes, program change, pitch bend, CC 0/6/7/10, NRPN loops and reverb attributes, tempo, end of track, play counts, decrescendo), the voice manager (allocation, volumes, pitches, the per-tick flush), `SsUtKeyOn/KeyOff/AllKeyOff`, the reverb presets and depth, main/CD volume, `SsInit` with LIBSPU's start-up stores (`libsnd_spu.c`: no LIBSPU API). Exact against the emulator's SPU write traces when replayed on its timeline (`tests/port/sound.py`). `CdInit` (`libcd.c`) makes LIBCD's five SPU stores | noise voices, RPN/VAG attributes and NRPN attributes other than 15/16, CC 11/64/91/100/101/121, timer tick modes (none used by the game or its SEPs) |
| LIBPRESS `libpress.c`, MDEC `mdec.c` | The movie decoder (session 16, M5; our own, from psx-spx: no FFmpeg): `DecDCTvlc2` expands a .STR version 2 frame (the disc's movies are all v2, 320x416) into the MDEC's run-level words with MPEG-1's AC code table, word for word as LIBPRESS does; `DecDCTin` writes the mode into the command word (bit 0: 24-bit, clears bit 27; bit 1: sets bit 25) and starts the MDEC; `DecDCTout` writes `size` words of pixels: per 16x16 macroblock (Cr, Cb, Y1..Y4) the run-level decode (quantiser 1..63 with LIBPRESS's table, quantiser 0, 11-bit saturation, zig-zag), the IDCT on the scale table, YCbCr to 24-bit or 15-bit RGB; then the `DecDCToutCallback` handler runs, and a `DecDCTout` it makes runs after it returns (a loop, so the handler's `LoadImage` of the column just decoded happens before the next column overwrites its buffer); `DecDCTReset(0)` loads the default tables | `DecDCTvlcBuild` (the decoder has its own table; the game's 0x11000 bytes are left as they are); no DMA timing: a frame decodes inside `DecDCTout` |
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
`DrawOTag`/`ContinueDraw` follow the 24-bit tags (docs/PORT.md "Ordering tables on 64-bit": `(ot & ~0xFFFFFF) + (tag & 0xFFFFFF)`),
only inside the window `psyq_set_arena` gave (by default the heap, `port_heap_start..port_heap_end`; a link outside
it stops the walk with a trace line). Every primitive's `len` words after its tag go into an FNV-1a hash that
`psyq_gpu_take_hash` returns and resets: the M1 test's "hash of the primitive stream per frame", then to `gpu.c` as
GP0 words.

## Behaviour assumed, to verify against the emulator later
Each file's header comment lists its own; the ones a later milestone must check first:
- **GTE and LIBGTE:** checked against the PS1 (session 16): the layer-1 family `gte` (`tests/golden/families/gte.py`)
  runs MIPS routines in the emulator that load all 64 registers, issue one command and store them back (every
  command with sf/lm 0/1, MVMVA's 64 mx/v/cv combinations, the game's nine command words, the registers' write/read
  rules, MAC 44-bit overflow, saturations, NCLIP/AVSZ limits, RTPS sweeps over every UNR table entry and H >= 2 * SZ3)
  and calls LIBGTE's own `rsin`/`rcos` (a whole turn and outside it), `RotMatrix*_gte`, `ScaleMatrix` and
  `ApplyMatrixSV`, with the GTE state each leaves; `tests/host/gte_replay.py` (run by `tests/host/replay.py`) replays
  all 865 cases through `gte.c`/`libgte.c`: every register and word equal. What the goldens changed: the sine table was
  right; `RotMatrix*_gte` floors each product on its own (it was one exact product, off by one in 80 of 96 cases);
  `ScaleMatrix` writes the pad halfword after `m[2][2]` and multiplies in 32 bits; the MAC accumulator wraps at 44 bits.
  Not covered: the GTE's timing (a command is instant here) and the power-on register values (zero here).
- **LIBGS (M2), what the GTE now needs from it:** on the PS1 `GsSetProjection` calls `SetGeomScreen` (H),
  `GsInitGraph`'s `gte_init` calls `InitGeom`, `SetFarColor(0, 0, 0)` and `SetGeomOffset`, `GsInit3D`'s
  `GsSetDrawBuffOffset` calls `SetGeomOffset`, `GsSetFlatLight` writes the light colours with `SetColorMatrix`
  (`asm/main/psyq/libgs/`); `libgs.c` does the `GsSetProjection` one (issue #7: the battle's H is 280, not
  InitGeom's 1000), not the others yet, so NCS lights with a zero colour matrix (FIGHTSTG's `GsSetFlatLight` calls).
- **LIBGPU and the GPU:** checked against the PS1 (session 16): the layer-1 family `gpu`
  (`tests/golden/families/gpu.py`) runs GP0 lists through the game's own `DrawOTag`/`LoadImage`/`MoveImage`/
  `ClearImage(2)` in the emulator and reads the VRAM back with `StoreImage` (every primitive type and mode, random and
  at the edges: shared edges, thin and degenerate triangles, the 1023/511 size limit, 11-bit wrapping, draw areas,
  texture windows, mask bits, every blend mode and depth; probes that pin the rasteriser's rules), and calls
  `SetDrawEnv`, `SetDefDrawEnv`, `SetDrawMove` and `BreakDraw` for their packets and results; `tests/host/gpu_replay.py`
  (run by `tests/host/replay.py`) replays its 715 cases through `libgpu.c`/`gpu.c`. What the goldens changed:
  `SetDrawEnv`'s packet (E3/E4 clamped, E1 and E2 after E5, the E6 word, a TILE rather than a fill for `isbg`; the
  per-frame primitive hashes changed with it), `SetDefDrawEnv`'s `dfe` (`h < 289`), `MoveImage` of an empty rectangle,
  `ClearImage`'s TILE path, `BreakDraw` (NULL while idle: FIGHTSTG's cursor copies are drawn). The known differences
  (`tests/host/known_mismatches.json`, 133 cases, all 1..few pixels or emulator-only behaviour): exact .5
  interpolation ties (the emulator's fixed-point rounding), semi-transparency modes 2/3 of modulated textures (the
  emulator's two-pixels-at-once arithmetic), and three hardware rules kept over the emulator's (fill rounding/wrap,
  copies obey the mask, a 1x1 draw area). Not covered: GP1, interlaced `dfe`, timing.
- **GPU speed (session 16):** `gpu.c` (built at `-O3`) draws `first_battle_save` (41,779 frames, ~180,000 pixels a
  frame) in ~7 s, the whole run ~8.5 s against ~1.6 s without drawing (22 s before): span loops per primitive kind,
  edge walking, sprite rows from decoded texture segments kept by VRAM write stamps, vectorisable select loops
  (`gpu.c`, "Speed"). Every step byte-identical: the 715 `gpu` cases' raw output, both scripts' logs, screenshots and
  the whole VRAM every 250 frames, and a randomized comparison with the unoptimised rasteriser.
- **LIBGS (M2):** whether `GsSetFlatLight` normalises the direction; what `GsInit3D` resets (the PS1's touches neither
  matrix: `GsInitGraph` makes the light matrix zero, and the world-screen matrix stays zero until `GsSetRefView2`).
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
- **XA audio (M5, T17):** the opening movie played to its end (no START) puts its soundtrack in the port's `--wav` from
  vsync 364 (the first audio sector arrives in tick 362; one tick of the render's pipeline, one of wait) to the `Pause`
  at vsync 6281: 2220 of the file's 2261 audio sectors (118.4 s; the game stops at its last video frame), and the
  left channel is exactly the decoded stream x the CD volume (7FFEh, `SsSetSerialVol`) x the main volume (3FFFh), the
  right one within 1 LSB (the reverb's tail). The emulator's capture of the same script agrees (0.1 s windows from the
  music's start: correlation >= 0.9997, levels 1.000..1.002, no drift). Assumed: when the hardware clears the decoder's
  history and ring (here: at every flush); the one-tick wait (ours: the sectors arrive in whole ticks); what plays
  after a `Pause` (here: nothing beyond the tick already handed to the SPU).
- **LIBPAD (M3):** the mode ids (4 digital, 7 analog), state 6, a digital pad without actuators.
- **LIBMCRD:** checked against the emulator (session 16, `first_battle_save`: `memcard_state` traced each frame): the
  command numbers, results 3 (the first Accept after boot and after the reset), 5 (ReadFile of a missing file) and 0.
  Still assumed: a missing card is result 1 for every command; ReadFile/WriteFile also answer 3 on a new card; the
  results for a transfer outside the file (2) and for CreateFile's errors; `DIRENTRY.attr`/`head` (the game reads
  only `size`).
- **LIBPRESS (M5):** checked against the emulator (session 16): the layer-1 family `mdec` (`tests/golden/families/mdec.py`: DecDCTvlc2 on 5 frames of
  3 movies, equal word for word; DecDCTin's command words; the MDEC on those frames and on made-up run-level data, `tests/host/mdec_replay.py`) and
  the opening movie: the port's VRAM after MOVIEOPN frames 30, 60, 100, 150, 200, 250, 300 against PCSX-Redux's after the same frames (an exec
  breakpoint on `stdwtitl_update_movie`, `PCSX.GPU.getVRAM()`): everything outside the two 24-bit movie buffers equal, inside them 39-46% of the
  bytes differ, by 1-3 (one or two bytes by 4 per frame). That is PCSX-Redux's IDCT and colour conversion, which round otherwise than psx-spx's
  (an exact floating-point IDCT differs from it as much); the port follows psx-spx. Where the two disagree beyond rounding, psx-spx is kept: 0xFE00
  before a block's DC is skipped (the emulator decodes it as a block) and out-of-range colours clamp (the emulator's wrap); neither occurs in the
  movies but for 2 bytes of one frame. 15-bit output rounds to 5 bits as the emulator does (not checked against psx-spx). Still assumed: the
  callback running synchronously (the real MDEC runs it from the DMA's end); DecDCTvlc2 on a damaged stream; the mono (4/8-bit) MDEC modes and
  custom tables, which LIBPRESS's API as the game uses it never selects.

## Checks
`port/psyq/check.sh` (no CMake, no SDL): compiles every file with the port's flags and `-Wall -Wextra -Werror`,
archives `build/port_psyq/libpsyq.a`, then checks coverage against `tools/port_inventory.py`'s probe objects
(`build/port_inventory/m64/obj`, made by `port_inventory.py probe`): every one of the 123 Psy-Q functions plus the
three data symbols is defined by the shim or by the host libc, no global is defined twice, and what stays undefined
is the game's own asm-only data and the runtime's `port_*`. `EXTRA="-O2 -fsanitize=address,undefined"
port/psyq/check.sh --compile` compiles with more flags.
