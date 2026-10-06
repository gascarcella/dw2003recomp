# Sound: what the game asks of LIBSND, what reaches the SPU, and the M3 plan

Groundwork for the PC port's milestone M3 (`docs/PC_PORT_PLAN.md` 2.7 and M3; DECISIONS "PC port decisions (session 15)"
item 6: our own SPU core and LIBSND, checked against emulator SPU register-write traces). Written in session 16 (T11).
Every claim is marked **verified** (with how) or **assumed** (with the source). "The trace" is the SPU write trace of
`tests/sound/spu_trace.py` (section 4); `new_game` and `first_battle_save` are the layer-2 scripts.

The file formats (VAB header and body, SEP) are in `docs/FORMATS.md` "Sound".

## 1. The game's sound layer (`src/main/sound.c`)
The game never touches the SPU or LIBSPU itself (**verified**: no `Spu*` call and no SPU address in `src/`). Every SPU
store of the `new_game` trace (with `--detail`, which records the pc) comes from LIBSND/LIBSPU, except five from
**LIBCD**: `CdInit` → `CD_initvol` (`0x8002DB40`) at tick 523 writes main volume `0x3FFF` (L, R), CD volume `0x3FFF`
(L, R) and SPU control `0xC001` (**verified**, pcs `0x8002DB6C`-`0x8002DB8C`). The port's `CdInit` must make the same
five stores. Everything goes through 24 LIBSND functions at 28
call sites, all in `sound.c` except `SsInit` (`main`) and `SsSeqCalledTbyT` (`gfx`'s vsync callback) (PC_PORT_PLAN
section 1, **verified** with `grep -rn "Ss[A-Z]" src`).

`sound_module` (`include/sound.h`) is the only way in for the rest of the game; the overlays call its function table
(**verified**, `grep -rhoE "sound_module\.\w+" src`):

| Method | Calls in `src/` | What it does in LIBSND terms |
|---|---|---|
| `play(key)` | 616 (every overlay, WSTAG 34 files) | key bit 31 set: `SsUtKeyOn(vab, prog, tone, note, 0, 0x7F, 0x7F)`, a one-shot note; clear: `SsSepStop` + `SsSepSetVol(.., 0x7F, 0x7F)` + `SsSepPlay(sep, seq, SSPLAY_PLAY = 1, 1)`. Bit 30: the "current" sound (stops the previous one) |
| `key_off(key, voice)` | 18 (FIELDSTG, FIGHTSTG, SOUNDTST, STFGTREP, STGTRAIN, WSTAG) | `SsUtKeyOff(voice, vab, prog, tone, note)` |
| `stop(key)` | 10 | `SsSepStop(sep, seq)` |
| `stop_all()` | 4 (FIELDSTG, SOUNDTST, STDWTITL) | `SsSepStop` on all 16 sequences of every open SEP, then `SsUtAllKeyOff(0)` |
| `fade_out(key)` | 1 (FIGHTSTG) | `SsSepSetDecrescendo(sep, seq, 0x80, 50)` (60 at 60 Hz) |
| `load_extra_bank(id)` / `load_bank(index, id)` | 13 / 1 | close the entry's SEPs and VAB (`SsSepStop` ×16, `SsSepClose`, `SsVabClose`), queue the header file |
| `update_loading()` | 1 (`main`, every frame) | step 1: `SsVabOpenHeadSticky(vh, entry, sound_spu_addrs[entry])`; step 2: `SsVabTransBody(vb, vab)`; step 3: poll `SsVabTransCompleted(0)`, then `SsSepOpen(sep, vab, 16)` per SEP |
| `init()` | 1 (`main`) | `SsSetTableSize(seq_table, 6, 16)`, `SsSetTickMode(0x1032)` (0x1000 at 60 Hz), `SsStart2`, `SsSetMVol(0x7F, 0x7F)`, `SsSetSerialAttr(0, 0, 1)`, `SsSetSerialVol(0, 0x7F, 0x7F)`, `SsUtSetReverbType(3)`, `SsUtSetReverbDepth(0, 0)`, `SsUtReverbOn()`, then bank 1 (COMMON) into entry 0, waiting for it |
| `key_on(index, prog, note)` | 0 (only in the table) | `SsUtKeyOn(.., tone 0, ..)` |

Three entries hold a bank each (entry 0 = COMMON for the whole game; entries 1 and 2 alternate,
`sound_load_extra_bank`). The SPU RAM bases are fixed: `sound_spu_addrs` = `0x1010`, `0x49C10`, `0x62410`
(**verified**: the DMA blocks of the trace land there, section 3).

`SsSetTickMode(0x1032)`: in Psy-Q's `libsnd.h`, `0x1000` is `SS_NOTICK` (the application calls `SsSeqCalledTbyT`
itself) and the low bits the tick rate, here 50 per second (**assumed** from Psy-Q's documentation; consistent with the
trace: `SsSeqCalledTbyT` runs once per vsync, 1,948 times in `new_game`'s 1,951 ticks after the EXE starts).

The sound keys of `play` (FORMATS "Sound"): a SEP sequence is addressed as (access number of the SEP, sequence). The
sequence is not range-checked against the SEP's 16: CNTY_SEL and the battle play COMMON's SEP 0 with sequence `0x1C`,
`0x19`, `0x1A` (**verified** in the trace: `SsSepPlay(0, 0x1c, 1, 1)` at tick 908 of `new_game`). With
`SsSetTableSize(table, 6, 16)` the score table is one array of 6 × 16 entries, so sequence 28 of access 0 is entry
`0 × 16 + 28` = sequence 12 of access 1, i.e. COMMON's second SEP (**verified**, section 7: LIBSND's disassembly
addresses the table as `table[access] + sequence × entry size`, and the port's LIBSND replayed on `new_game`'s timeline
matches the emulator's stores only with the aliasing: bounded to 16, the effect's notes at tick 910 are missing). A
faithful LIBSND must keep this aliasing.

## 2. The LIBSND/LIBSPU functions and what they reach
The EXE links 91 LIBSND objects (119 functions) and 27 LIBSPU objects (43 functions) (docs/TOOLCHAIN.md; names in
`config/symbol_addrs.txt`). The trace tool's `--calls` mode puts an exec breakpoint on every one of them and counts
the calls (`calls.txt`); the public ones the game calls are logged with their arguments as comments in the trace.

**Observed call sequence, `new_game`** (ticks are vsyncs since boot; the EXE starts at tick 511; **verified**, trace):

| Tick | Calls (args) | Context |
|---|---|---|
| 513 | `SsInit` | `main`: LIBSPU init (all 24 voices reset and keyed on the 16-byte self-loop block at SPU `0x1000`, reverb work area cleared) |
| 524 | `SsSetTableSize(0x800519EC, 6, 16)`, `SsSetTickMode(0x1032)`, `SsStart2`, `SsSetMVol(0x7F, 0x7F)`, `SsSetSerialAttr(0, 0, 1)`, `SsSetSerialVol(0, 0x7F, 0x7F)`, `SsUtSetReverbType(3)`, `SsUtSetReverbDepth(0, 0)`, `SsUtReverbOn` | `sound_init` |
| 543, 602, 603 | `SsVabOpenHeadSticky(vh, 0, 0x1010)`, `SsVabTransBody(vb, 0)` + 104 polls of `SsVabTransCompleted(0)`, `SsSepOpen` ×2 (access 0, 1) | bank 1 (COMMON) into entry 0 |
| 821-844 | `SsVabOpenHeadSticky(.., 1, 0x49C10)`, `SsVabTransBody`, `SsSepOpen` (access 2) | CNTY_SEL's bank |
| 858 | `SsSepStop(2, 2)`, `SsSepSetVol(2, 2, 0x7F, 0x7F)`, `SsSepPlay(2, 2, 1, 1)` | CNTY_SEL's music |
| 908 | `SsSepStop/SetVol/Play(0, 0x1C)` | a sound effect (the aliased sequence above) |
| 987 | `SsSepStop(2, 2)` | leaving CNTY_SEL |
| 1003 | `SsSepStop` × 48, `SsUtAllKeyOff(0)` | `sound_stop_all` (STDWTITL) |
| 1060-1086 | VAB open / body / `SsSepOpen` (access 3) into entry 2 at `0x62410` | the title's bank (TTLBGM) |
| 1147 | `SsSepPlay(3, 0, 1, 1)` | the title music |
| 1186, 1197, 1906, 1912 | `SsUtKeyOn(0, 10, 0, 60)`, `SsUtKeyOn(0, 2, 2, 62)`, ... | menu sounds (notes of COMMON) |
| 1957-2035 | `SsSepStop(3, 0)`; entry 1 closed (`SsSepStop` ×16, `SsSepClose(2)`, `SsVabClose(1)`) and the field's bank loaded | New Game: FIELDSTG |

`first_battle_save` up to `battle_won` adds (**verified**, trace): 12 VAB loads in all, `SsSepPlay` 209 times,
`SsUtKeyOn` 211 times (one every tick for a while: a typing sound in the registration), `SsUtAllKeyOff` 3 times;
at `battle_start` (tick 20375) the field bank is closed (`SsSepStop` ×16, `SsSepClose(3)`, `SsVabClose(2)`) and the
battle's bank opened into entry 2.

**What the 24 public functions reach** (call counts, `first_battle_save` to `battle_won`, 21,844 ticks;
**verified**, `calls.txt`): per tick, `SsSeqCalledTbyT` → `_SsSeqPlay`/`_SsSndPlay` per playing sequence (22,422) →
`_SsGetSeqData` (14,424) → `_SsReadDeltaValue`, `_SsNoteOn` (12,815), `_SsSetProgramChange` (306),
`_SsSetControlChange` (130) → `_SsContDataEntry`/`_SsContNrpn1`/`_SsContNrpn2` → `_SsSetNrpnVabAttr15`/`16` (13 each:
reverb type and depth, FORMATS "SEP"), `_SsSetPitchBend` (963), `_SsGetMetaEvent` (25), `_SsSeqGetEof` (185); the voice
manager `_SsVmKeyOn` (6,442) / `_SsVmAlloc` + `_SsVmDoAllocate` (6,790) / `_SsVmSelectToneAndVag` / `_SsVmKeyOnNow`,
`_SsVmKeyOff` (6,373) / `_SsVmKeyOffNow` (9,380), `_SsVmPBVoice` (23,112), `note2pitch` (6,579) / `note2pitch2`
(2,024) / `SsPitchFromNote` (8,603), `_SsVmSetSeqVol` (218), `_SsVmSeqKeyOff` (1,105), `_SsVmDamperOff` (911); and
`_SsVmFlush` once a tick (21,331), which reads every voice's envelope (`SpuGetVoiceEnvelope` 511,920 = 24 × 21,330 `SsSeqCalledTbyT` calls) and
writes the SPU through `SpuSetKey` (42,662), `SpuSetVoiceAttr` (8,629), `SpuSetReverbVoice`, `SpuSetNoiseVoice`.
Bank loads reach `_SsVabOpenHeadWithMode`, `SpuWrite` → `_spu_Fw` → `_spu_t` (DMA), `SpuSetTransferStartAddr`,
`SpuIsTransferCompleted`, `SpuFree`/`_spu_gcSPU` on close.

**Linked but never reached** in these two runs (66 of 162; **verified**, the same counts): the crescendo/decrescendo,
pause, replay, next-SEP and tempo-change paths (`_SsSndCrescendo`, `_SsSndSetDecres`, `_SsSndPause`, `_SsSndReplay`,
`_SsSndNextSep`, `_SsSndTempo`, `SsSeqSetDecrescendo`, `SsSepSetDecrescendo`), `SsSeqClose`, `SsSeqStop`,
`SsSeqSetVol`, `SsUtKeyOff`, `SsUtGetVagAtr`, `SsUtSetVagAtr`, `SsUtReverbOff`, `SsUtSetReverbDelay`/`Feedback`,
`SsVabFakeHead`, 18 of the 20 NRPN attribute setters, the control changes `_SsContBankChange`, `_SsContDamper`,
`_SsContExpression`, `_SsContExternal`, `_SsContMainVol`, `_SsContPanpot`, `_SsContResetAll`, `_SsContRpn1/2`, the
noise (`vmNoiseOn/Off`, `SpuSetNoiseClock`), `_SsVmSeKeyOn/Off`, `_SsVmSetVol`, `_SsVmSetProgVol`, `_SsVmDamperOn`,
`_SsUtBuildADSR`/`_SsUtResolveADSR`, and LIBSPU's read-back (`_spu_Fr`, `_spu_FgetRXXa`, `_spu_note2pitch`, ...). Some
of them must still exist in the port (`SsUtKeyOff` and `SsSepSetDecrescendo` are called by the game, CC 7 and CC 10
occur in the SEPs: FORMATS "SEP"), but they are not covered by these two traces.

## 3. What reaches the SPU (from the traces)
All **verified** on the `new_game` trace unless marked.
- **Before the EXE**, the BIOS shell (OpenBIOS's, or the retail BIOS's boot logo) runs its own copy of LIBSND/LIBSPU
  from `0x8003xxxx` and writes the SPU (955 stores in `new_game`). The tracer starts at the EXE's entry point (section 4);
  the port does not run a shell, so these are not part of the oracle.
- **`SsInit` (tick 513)**: main volume 0, SPU control off, `xfer.ctrl` 4, reverb volume 0, key-off all, EON/PMON/NON 0,
  CD and external volume 0; transfer address `0x1000` and 8 FIFO writes of `0x0707` (a 16-byte block whose flags byte
  is 7: start + repeat + end, a self-loop); then every voice: volume 0, pitch `0x3FFF`, start address `0x1000`, ADSR 0;
  key-on all 24 voices, key-off all 24; SPU control `0xC000` (enabled, unmuted); reverb base `0xFFFE`; then the reverb
  work area cleared by DMA: 96 blocks of 1,024 bytes and one of 64 from one zero buffer, SPU `0x67FC0`-`0x7FFFF`
  (`0x18040` bytes; the largest preset work area, assumed from psx-spx).
- **`CdInit`** (tick 523, LIBCD, above): `mvol` `0x3FFF`, `cdvol` `0x3FFF`, `spucnt` `0xC001`.
- **`sound_init`**: `SsSetMVol(0x7F, 0x7F)` → main volume `0x3FFF`; `SsSetSerialAttr(0, 0, 1)` → SPU control `0xC001`
  (CD audio on); `SsSetSerialVol(0, 0x7F, 0x7F)` → CD volume `0x7FFE`; `SsUtSetReverbType(3)` → the 32 reverb
  registers (`dAPF1` `0x00B1`, `dAPF2` `0x007F`, `vIIR` `0x70F0`, ..., `vLIN`/`vRIN` `0x8000`: **verified**, the trace),
  and later the base `0xF6F8` (work area `0x7B7C0`-`0x7FFFF`, `0x4840` bytes). Type 3 is Psy-Q's `SS_REV_TYPE_STUDIO_B`
  and the values are psx-spx's "Studio Medium" example (**assumed**, from memory of both documents: psx-spx could not
  be fetched from this machine; the LIBSPU task must check its preset table against psx-spx).
- **Bank bodies**: one DMA each, 16-word blocks: COMMON's 297,200-byte body as 297,216 bytes at `0x1010` (BCR
  `0x1224_0010`), CNTY_SEL's at `0x49C10`, TTLBGM's at `0x62410` (`spu=` of the DMA lines; the data's SHA-1 is in the
  trace). So SPU RAM is: `0x0000`-`0x0FFF` unused by the game (the hardware's capture buffers, psx-spx), `0x1000` the
  16-byte self-loop block (all bytes `0x07`: shift 7, filter 0, flags 7; nearly silent), `0x1010` entry 0, `0x49C10` entry 1, `0x62410` entry 2, the reverb work area at the top.
- **Every tick**, `_SsVmFlush` (from `SsSeqCalledTbyT`) writes, in this order: `koff.lo`, `koff.hi`, `kon.lo`, `kon.hi`,
  `eon.lo`, `eon.hi`, `non.lo`, `non.hi`, even when all are 0 (8 stores a tick: 15,610 of `new_game`'s 20,406). Before
  them, the voices that change: `pitch`, `vol.l`, `vol.r`, `addr`, `adsr.lo`, `adsr.hi` per voice, voice by voice.
  The game never writes `adsr.vol` (the current envelope) or `loop` (the repeat address): loops come from the ADPCM
  flags alone.
- **Latency**: a note found by the sequencer in tick N, or keyed by `SsUtKeyOn` from game code in frame N, reaches the
  SPU at tick N + 1's flush (`SsUtKeyOn(0, 0xB, 0, 0x3C)` at tick 9042 of `first_battle_save`: voice 3's registers
  and `kon.lo 0008` at tick 9043). `SsSeqCalledTbyT` flushes first, then advances the sequences.
- **Voices**: CNTY_SEL's music keys voices 0-9 at once (`kon.lo 03FF` at tick 869), the effects take free voices
  (`SsUtKeyOn` → voice 3 above). Which voice a note gets is LIBSND's allocator (`_SsVmAlloc`), and the voice number is
  in every register address of the trace: the port's allocator must choose exactly the same voices.
- **Reverb from the music**: CNTY_SEL's sequence sets reverb type 3 and depth `0x38` through NRPN (tick 859:
  `SsUtSetReverbType(3)` rewrites all reverb registers and the base, `SsUtSetReverbDepth(0x38, 0x38)` → `rvol`
  `0x3870`). EON follows the tones' `mode` 4.

Register writes per run: `new_game` 20,406 SPU stores (koff/kon/eon/non 15,610; voice registers 6 × 665; SPU control
215; transfer address 102; ...), 101 DMA blocks; no store's value was undecodable (0 "unknown values").

## 4. The trace oracle (`tests/sound/`)
- **`tests/sound/spu_trace.lua`**, loaded into PCSX-Redux before `tests/replay/run.lua` (the same wrapper scheme as
  `tools/coverage.py`): one `Write` breakpoint over `0x1F801C00`-`0x1F801DFF` and one over DMA4's `0x1F8010C0`-`CF`.
  Breakpoints need **`-debugger -interpreter`** (**verified**: with `-debugger` alone, under the dynarec, a write
  breakpoint over the SPU got 0 hits in 900 frames; with `-interpreter` 5,000). One breakpoint catches every segment
  alias (a breakpoint at `0x1F801C00`, `0x9F801C00` or `0xBF801C00` gets the same hits: Redux normalises addresses).
  The invoker is not given the value: it decodes the store at `pc` (`sb`/`sh`/`sw`, base + offset must equal the
  address) and reads `rt` from `PCSX.getRegisters()`. A DMA4 start (CHCR bit 24, direction RAM → SPU) copies the RAM
  block (MADR, BCR blocks × words) at that moment. Gate: recording starts at the EXE's entry point (`0x80010E90`) once two
  words of the EXE's LIBSND are in RAM (the shell's code sits at the same addresses), and stops at a reset.
- **`tests/sound/spu_trace.py`**: `run <script> [--until CHECKPOINT] [--extra-frames N] [--repeat N] [--calls]
  [--detail]`, `check [--record]`, `diff A B [--align MARKER] [--no-ticks]`. The trace is text: `<tick> <reg> <name>
  <value>` per store, `<tick> dma4 spu=<addr> len=<n> sha1=<..>` per DMA block, `#` comments for the calls, the
  markers (`exe_start`, checkpoints, overlay and map changes) and with `--detail` the pc of each store. `diff` compares
  two traces with the comments dropped, optionally rebased at a marker (the port's boot will not reach `exe_start` at
  tick 511): this is the comparison the LIBSND task needs.
- **Determinism (verified)**: `new_game` twice → identical traces (2,462 ticks, 20,406 stores, 101 DMA blocks), and
  the committed `cnty_sel` case three times (two at `--record`, one `check`).
- **Cost (measured, 4-core machine)**: `new_game` 54 s a run, 459 KB with `--calls` (812 KB with `--detail`);
  `first_battle_save` cut at `battle_won` (21,844 ticks) 522 s, 215,174 stores, 109 DMA blocks, 5.4 MB; the committed
  check case (`new_game` to `cnty_sel` + 300 frames: the boot, COMMON, CNTY_SEL's bank, music and first effect) 23 s,
  156 KB (15 s without `--calls`). It runs in `scripts/test.sh` layer 2. The interpreter is ~2-3× slower than the
  dynarec runs of `replay.py`, and its frame counts differ slightly from them (21,844 vs 21,786 at `battle_won`): the
  trace's ticks are the interpreter's.
- **The Lua stack leak** (`run.lua`, PCSX-Redux listener leak): the tracer adds a second vsync listener; `run.lua`'s
  reset every 8,192 frames still keeps the stack below its limit (the 21,844-tick run passed two resets).

**Audio output, for the SPU core (checked):** Redux has no WAV/audio dump option and no Lua access to the SPU's output
or RAM (its Lua API: memory, registers, breakpoints, GPU dumps, `PCSX.SPU.playAudio` for *playing* sounds). But it
plays through SDL3, and SDL3's `disk` driver records what it plays: `SDL_AUDIO_DRIVER=disk
SDL_AUDIO_DISK_OUTPUT_FILE=out.raw` writes the mixed output as F32LE stereo at 44,100 Hz (**verified**: 7.4 MB for
`new_game` to `cnty_sel` + 300 at speed 1). It is **not deterministic** as a file: the host paces it, so two runs
differ in length (7,386,112 vs 7,284,736 bytes) by inserted gaps; aligned on the first non-zero sample, the two
captures were identical for the first 169,448 samples, then a gap shifted one of them. Usable for listening and for
segment-wise comparisons, not as a byte-exact golden. Speed 0 (unthrottled) drops most of the audio.

## 5. Plan for M3
### Scope
**SPU core** (port side, ours, from psx-spx "SPU"): 24 voices with ADPCM decoding (5 filters, shift, the block flags:
loop start/repeat/end, ENDX), the ADSR envelope (attack/decay/sustain/release, linear/exponential, increase/decrease,
the rate tables), pitch (`0x1000` = 44,100 Hz) with the 4-point gaussian interpolation (psx-spx's 512-entry table),
pitch modulation (PMON; the game writes 0 there, low priority), noise (NON; never set in the traces, low priority),
key on/off semantics, main volume and the volume sweeps (the game writes fixed volumes), the reverb (the documented
all-pass/comb/IIR network over the work area at 22,050 Hz, with the preset registers), CD audio input mixed in (M5:
XA from the movies; the game turns CD audio on in `sound_init`), the capture buffers (unused by the game), DMA/FIFO
writes into SPU RAM (512 KB), SPUCNT/SPUSTAT as far as LIBSPU polls them. Output: 44,100 Hz stereo s16 into SDL3
(M2's window code owns the device); headless runs render into a buffer that a test can hash.

**LIBSND** (port side, ours): only the 24 functions the game calls (section 1), and of LIBSND's internals only what
those reach in practice (section 2): SEP opening and the score table (6 × 16, with the sequence aliasing of section 1),
VAB header parsing and the SPU allocation (`SsVabOpenHeadSticky` at a fixed address), the body transfer (immediate on
the host; `SsVabTransCompleted` must still poll as the PS1 does: the trace shows 104 polls for COMMON, which cost no SPU writes),
the sequencer (resolution 480, tempo in µs/qn, the 50 Hz tick of `SsSetTickMode(0x1032)`, the per-tick delta
arithmetic, note on/off, program change, pitch bend, volume/pan/data entry, the NRPN loop and attribute controls, tempo
meta, end of track, `SsSepPlay`'s play mode and loop count, decrescendo), the voice manager (allocation by priority,
key-on/off, the per-tick flush in the observed order), `SsUtKeyOn`/`SsUtKeyOff`/`SsUtAllKeyOff`, the reverb utilities
(type presets = psx-spx's table; depth), `SsSetMVol`, `SsSetSerialAttr/Vol`. LIBSPU becomes the port's internal
interface between LIBSND and the SPU core (no Psy-Q LIBSPU API needed: the game never calls it).

### Verification
- **LIBSND, exact**: the port writes the same trace format (a hook in its SPU register write function, plus the DMA
  blocks' SHA-1), and `spu_trace.py diff` compares it with the emulator's, aligned at a marker (`checkpoint cnty_sel`,
  or the first `SsInit` store): the same stores in the same order, tick for tick, for `new_game` and
  `first_battle_save` (the scripts the port already replays). Ticks may need a tolerance at bank loads (CD timing:
  the port's `--cd-speed realistic` is close but not equal); everything between two loads must be exact. Start with the
  committed `cnty_sel` trace (in `test.sh`).
- **More coverage for LIBSND** (done in session 16 by `tests/sound/key_trace.py`: 343 steps over the 71 banks, the BGMs
  and the game's literal keys, in 4 emulator boots, goldens in `tests/sound/expected/keys/`; it reaches `SsUtKeyOff`,
  `SsSepSetDecrescendo`, CC 7/10 and the NRPN loops; a port run of the same driver is still to do): a trace driver that plays every sound of a bank without the CD (SOUNDTST, stage 8: the
  debug sound test, `src/soundtst/`, is not reachable by the pad; a Lua step that sets the next map, or the layer-1
  oracle's call mechanism calling `sound_module.play(key)` per key and letting N vsyncs run): one golden trace per
  key, every BGM and SFX of the 71 banks. This also covers `SsUtKeyOff`, the decrescendo and the CCs the two scripts
  never reach.
- **SPU core**: (1) unit goldens from psx-spx's formulas (ADPCM blocks with every filter/shift, the gaussian table,
  ADSR rate steps, the reverb's address arithmetic) in host tests; (2) an emulator oracle for the envelope: the layer-1
  oracle can call LIBSPU in the EXE (`SpuSetVoiceAttr`, `SpuSetKey`, `SpuGetVoiceEnvelope`) and read a voice's
  envelope register every vsync: the ADSR timing at vsync resolution against Redux's SPU; (3) the SDL disk capture
  (section 4) of a whole song against the port's rendering of the same register trace: by ear, and per segment between
  the emulator's gaps (Redux's SPU is itself an emulation; bit-exactness with it is not the goal, psx-spx's documented
  hardware behaviour is).

### Tasks (2-3 agents, disjoint files)
1. **T-spu: the SPU core.** Owns `port/src/spu.c` + `port/src/spu.h` (new): registers (16-bit write/read by offset),
   SPU RAM with DMA/FIFO writes, `spu_render(int16_t *out, int frames)` at 44,100 Hz, ADPCM, ADSR, gaussian, reverb,
   CD input stub; `tests/spu/` host unit goldens (new). A trace hook: `spu_set_write_hook(fn)` for the trace writer.
   Check: the unit goldens; render the committed `cnty_sel` trace (fed as register writes per tick, DMA blocks from the
   disc's bank files) to a WAV for listening.
2. **T-libsnd: LIBSND over the SPU core.** Owns `port/psyq/libsnd.c` (replaces the stub), `port/psyq/libsnd_*.c` (new,
   if split), the port's trace writer (`port/src/spu_trace.c`, new: the trace format of section 4, written with
   `--spu-trace FILE`), `tests/port/` additions for the trace comparison. Uses only `spu.h`. Check: `spu_trace.py diff`
   against the emulator's `new_game` and `first_battle_save` traces (aligned), and the committed `cnty_sel` trace;
   sanitizer-clean. Needs one change outside its files (the orchestrator's): `port/psyq/libcd.c`'s `CdInit` makes
   LIBCD's five SPU stores (`CD_initvol`, section 1) through `spu.h`.
3. **T-audio (optional, small): the output path.** Owns the SDL3 audio device in `port/src/video.c`'s neighbour
   (`port/src/audio.c`, new): a callback pulling `spu_render`, paced by the emulated vsyncs, muted headless; `--wav FILE`
   for headless capture. Plus the SOUNDTST/oracle trace driver of "More coverage" if time allows (owns
   `tests/sound/` additions).
T-spu and T-libsnd can run in parallel once `spu.h` is fixed (the orchestrator writes it first, as `port_harness.h`
was in M1). T-libsnd's exact check needs no audio at all; T-spu's needs no LIBSND.

### Open points
- **How close to Sony's code may LIBSND follow?** Settled by the user (session 16): the disassembly may be read for
  constants, tables, formulas and the order of operations; the C is our own (section 7, "Provenance").
- **Tick alignment**: measured with the LIBSND build (section 7): the game's own frames between two LIBSND calls differ
  between the port and the emulator (not only at bank loads), so the exact check replays the emulator's calls on its
  timeline, and the port's run is checked against the replay of its own calls.
- The sequence aliasing (section 1): confirmed (section 7).
- 60 Hz (`SsSetTickMode(0x1000)`, the NTSC patch) is not traced.

## 6. The SPU core (`port/src/spu.c`, `spu_dsp.c`; T-spu, session 16)
Our own, written from psx-spx "Sound Processing Unit (SPU)" and, for the ADPCM filter tables, its CD-ROM page
("XA-ADPCM", which psx-spx names as the same algorithm). psx-spx's site and its mirror are blocked by this machine's
proxy, but its source is not: `https://raw.githubusercontent.com/psx-spx/psx-spx.github.io/master/docs/ps1/spu/soundprocessingunitspu.md`
(and `.../ps1/cdr/cdromformat.md`). No emulator source was read; PCSX-Redux served as an oracle only. It implements
`port/include/spu.h` as given; the pure pieces and a read-only view of the voices are in `port/src/spu_internal.h`
(for the tests and tools). With psx-spx in hand, section 3's assumption checks out (**verified**): the 32 registers
`SsUtSetReverbType(3)` writes are psx-spx's "Studio Medium" preset word for word, its size `4840h` gives the base
`F6F8h`, and `18040h`, SsInit's cleared area, is the size of the two largest presets ("Chaos Echo", "Delay").

### What is modelled
- **Registers** by offset from `0x1F801C00`, 16-bit: each stores what is written and reads it back, except ENVX
  (`+0x0C`: the envelope), the repeat address (`+0x0E`: the voice's), ENDX, STATX and the current main volume
  (`0x1B8`/`0x1BA`), which read the state (writes to the last three are ignored: psx-spx's read-only). An odd offset
  acts as the even one below it; `0x200` and up (the voices' current volumes) are outside `spu.h`'s range.
- **SPU RAM**: 512 KB. The transfer address (`0x1A6` × 8) is copied to an internal address that advances (psx-spx
  "TSA"); `spu_dma_write` writes there (the transfer mode is not checked; the hook gets the address where the block
  lands); FIFO writes (`0x1A8`, 32 halfwords) go to RAM when ATTR's transfer mode becomes 1 (manual write), and at once
  while it stays 1 (the BIOS's multi-block case). STATX: ATTR bits 5-0 at once (no apply delay), bit 7 = ATTR bit 5,
  bits 8/9 the DMA write/read request in those modes, bit 10 (busy) never set (a transfer completes when it is made),
  bit 11 the capture buffers' half: whatever LIBSPU polls is ready. IRQA is stored only (no interrupt).
- **ADPCM**: a 16-byte block is decoded whole when the voice reaches it (filters 0-4 with psx-spx's tables; shifts
  13-15 act as 9; filters 5-7 as 0, never on this disc); flags: loop start copies the block's address to the repeat
  address, loop end sets ENDX and jumps to the repeat address, and without repeat also releases the voice with its
  envelope at 0 (code 1, "End+Mute"); code 2 is code 0.
- **Pitch and interpolation**: psx-spx's pitch counter (PMON from the previous voice's output in the same sample; the
  4000h clip) and its 4-point interpolation with the 512-entry table, each product `>> 15` on its own.
- **Envelope** (one generator for the ADSR and the volume sweeps, `spu_envelope_tick`): psx-spx's step/counter
  formula once per 44,100 Hz sample. Attack (linear, or exponential: slowed above 6000h) until the level saturates at
  7FFFh; decay (exponential, step -8) until the level is at or below the sustain level (N+1)×800h; sustain (both modes,
  both directions) until key off; release (linear or exponential) to 0. Rate 7Fh never steps; the counter's minimum
  increment of 1 makes shifts 26-31 alike (psx-spx: rate 76h behaves like 6Ah). Volumes: bit 15 clear sets
  `value × 2` when written, bit 15 set sweeps from the current volume (mode, direction, phase, shift, step).
- **Key on / key off**: a write acts at once, between two samples: key on copies the start address, starts the
  attack from 0 and clears ENDX; key off starts the release. Several writes between two renders act in their order at
  the same sample: LIBSND's flush (key off, then key on) restarts a voice; registers written before a key on are used
  by it, after it only by the next one (the start address) or at once (pitch, volume, ADSR).
- **Noise** (NON; psx-spx's generator with ATTR's shift and step) and **PMON**, for completeness (the game sets neither).
- **The mix**: per voice `sample × ENVX >> 15`, then `× volume >> 15` per side; the voices' sum saturated; the
  reverb's output added (when ATTR bits 15 and 14 are both set; otherwise voices and reverb are silent, the CD input is
  not); the CD input (`spu_cd_input`: a queue of 16,384 frames, the newest dropped beyond it) `× CD volume >> 15`,
  added with ATTR bit 0 and fed to the reverb with bit 2; saturated; `× main volume >> 15`; saturated.
- **Reverb**: psx-spx's formula at 22,050 Hz (every other sample), its reads and writes in psx-spx's "Reverb
  Computation Order", each step saturated, the work area wrapped into ESA..7FFFEh (writing ESA sets the current
  address); ATTR bit 7 clear stops the writes, not the reads. Input and output go through psx-spx's 39-tap resampling
  filter (the output zero-stuffed and filtered at twice the gain): 38 samples of delay, psx-spx's measurement.
- **Capture buffers**: CD left/right (before the CD volume) and voices 1 and 3 (after their envelope) written to
  `0x000`-`0xFFF` every sample.

### Readings where psx-spx is silent (assumptions)
- The envelope counter loses 8000h when it steps (with increments that divide 8000h this is psx-spx's earlier "wait
  1 SHL (shift-11) cycles") and starts at 0 at every key on, key off and phase change. A phase's end is checked after
  every tick (with the sustain level 8000h the decay ends on its first tick: no decay at all).
- psx-spx's two divisions (the ADPCM filter's "/64", exponential decrease's "/8000h") are arithmetic shifts (floor):
  truncation would stop an exponential release above 0 (at shift 11 the step is 0 below level 4096).
- Key on resets the pitch counter and the ADPCM and interpolation history to 0, and leaves the repeat address alone.
- Power-on: everything 0, every voice released at level 0.
- The mix's saturation points and the main volume over the CD input (above); left and right reverb computed together
  (the hardware alternates them on the two 44,100 Hz cycles: psx-spx measures 1-2 LSB); the `vIIR = -8000h` sign quirk
  is not modelled; a DMA into a block a voice is playing is heard from that voice's next block.

### How it is checked
- **Unit goldens** (`tests/spu/run.sh`, 1.4 s with the build): `spu_ref.py` is a second model of the SPU, in Python,
  from the same psx-spx text; it generates `goldens.txt` (72 cases, 2,497 checked operations; `spu_ref.py check`
  regenerates it in 40 s), which `spu_test.c` replays through the C core: `spu.h` (register writes and reads, DMA, CD
  input, renders hashed sample by sample, ENVX/ENDX/repeat-address traces) and `spu_internal.h`'s pieces. Covered: the
  table (and psx-spx's sums), ADPCM blocks with every filter and shift and the clamps, the interpolation at phases 0, 1,
  7Fh, 80h, FFh and random ones, the envelope generator at all 128 rates in each mode (3,000 ticks each), ADSR words
  through a voice (the game's seven and sixteen others; `goldens.txt`'s comments give the samples to 7FFFh and back
  to 0), pitches (0, 4000h, beyond), the loop flags (one-shot, whole and mid-sample loops, no start flag, code 2), key
  on/off orders within a tick, ENVX and repeat-address writes, every volume sweep mode, the clamps with 24 loud voices,
  noise, PMON, the FIFO/DMA/STATX, the CD input and the capture buffers, the reverb's address arithmetic and the
  impulse responses of "Studio Medium" (writes on and off) and "Room". All match at `-m64`, `-m32` and under ASan and
  UBSan (`run.sh --all`). A mutation check (16 one-token changes to the C, a scratch script) was caught 15 times; the
  miss is an equivalent change (`step > 4000h` for `step > 3FFFh`). Two models of one reading agree; a reading both get
  wrong passes, which is what the next two checks are for.
- **The envelope against the emulator** (`tests/spu/envelope_oracle.py gen`: the layer-1 oracle's machinery, 35 s; the
  readings are committed as `envelope_oracle.json`, and `check` re-fits them in 3 s): a MIPS routine keys voice 23 on
  with an ADSR word (silent, on a looping sample of COMMON) and reads ENVX at every vsync. For 14 ADSR words (linear and
  exponential attacks at shifts 15 and 16, decay to a sustain level, sustain decreasing linear and exponential and
  increasing, releases linear and exponential, three of the game's words) every one of the 800 readings equals our
  envelope at sample `(k + 1) × S + t0` within 2 samples, with `S` ≈ 877.4 samples per vsync (Redux's pace; a 50 Hz tick
  is 882): levels, rates and phase ends agree exactly. One difference: the releases fit only with the key-off moved by
  one step period (16 samples at shift 15, 4 at 13, 1 at 10): Redux takes its first release step at once, our counter
  (psx-spx's, from 0) a full period later; psx-spx does not settle it. Also found: on SsInit's silent block at 1000h
  (flags 7: start, end, repeat) Redux reads ENVX as 0 at every vsync after a key-on, where psx-spx's code 3 keeps the
  envelope going (and so do we; the oracle uses a real looping sample).
- **The `cnty_sel` trace rendered** (`tests/spu/render_trace.py`, 1 s; the WAV goes to `build/spu_test/`): the trace's
  6,573 stores replayed per tick (882 samples a tick), its 99 DMA blocks fetched from the disc by SHA-1: all 99 matched
  (97 blocks of zeros, SsInit's reverb clear; COMMON's body, 297,216 bytes: the 297,200-byte VAB body and the sector's
  zero padding; CNTY_SEL's bank, `BGM031`), and the write hook saw each one land where the trace says. 593 ticks
  (11.86 s): silence until the music's key-ons at tick 869, then RMS 4,380-5,200 per second (L and R), peaks
  28,281-31,416, no clipped sample. 208 key-ons: 24 keyed off in the same tick (SsInit), the other 184 all started;
  54 on looping samples (32 reached their loop end; none muted), 130 on one-shots (80 reached their end and were muted
  there, before any loop, at exactly the time their block count and pitch give; 50 were keyed again before their end).
  The WAV is the same bytes every run. Speed: 53× real time at `-O2` (11.86 s in 0.22 s), 13× at `-O0`.
- **The rendering against what the emulator played** (`tests/spu/capture.py`, one run at speed 1, ~20 s): Redux's SDL
  disk capture of the same script (section 4) against our rendering of the trace at Redux's 877.4 samples per tick,
  from the music's start, in 0.1 s windows located by cross-correlation: the stretch's 10 ms envelope correlates 0.971
  over 3 s; per window the mono mix correlates 0.88 on average (30 of 47 windows at 0.9 or more, the best 0.997) and
  the capture's level over ours is 0.95-1.05 in most windows. With the reverb left out of our rendering (EON forced to
  0) the mean drops to 0.85. Not a golden (the capture is host-paced, Redux is an emulation), but the whole chain
  (samples, pitch, interpolation, envelopes, volumes, reverb) comes out as Redux's does.

### Open
- The envelope counter's start (the release difference above): hardware would settle it.
- Speed once the game drives it (T-audio): the port's CMake build has no `-O`; `-O2` on `spu*.c` gives 4×.
- Not modelled: SPU interrupts (IRQA; the game never enables IRQ9), DMA reads (SPU → RAM), the external input,
  `0x1AC`'s RAM-size modes, the hardware's write latency. Nothing calls `spu_init`/`spu_reset` yet: `port/src/main.c`
  and `reset.c` (the console's reset) should, once LIBSND drives the SPU.

## 7. LIBSND (`port/psyq/libsnd*.c`; T-libsnd, session 16)
The 24 functions the game calls (section 1) and what they reach (section 2), over the SPU core of section 6, written so
that the SPU sees what it sees on the PS1: the same stores in the same order at the same vsync, the same DMA blocks.
`libsnd.c` has the public calls, start-up and the VABs; `libsnd_seq.c` the score table and the sequencer;
`libsnd_voice.c` the voice manager; `libsnd_spu.c` the SPU side (what LIBSPU does on the PS1: the game never calls
LIBSPU, so the port has no LIBSPU API); `libsnd_internal.h` the shared state. LIBCD's `CdInit` makes its five SPU
stores (`port/psyq/libcd.c`). `--spu-trace FILE` (`port/src/spu_trace.c`) writes the port's trace in section 4's format,
with the game's LIBSND calls as comments (`# <tick> call SsSepPlay(2, 2, 1, 1)`, a SEP's pointer as its offset from the
VAB header, the console's reset as `reset()`).

### Provenance
The user's decision (session 16): LIBSND is Sony's library code, in the EXE as split asm that this project never
decompiles; its disassembly may be read to learn constants, tables, formulas and the order of operations, and the
port's C is our own, with no routine translated instruction by instruction or function by function and no data copied
beyond numeric facts (a table of numbers is a fact, cited where it is used). The code is MIT like the rest of the repo;
its structure (the state, the files, the names) is ours. The disassembly (`asm/main/psyq/libsnd/`, `libspu/`; m2c's
pseudo-C of it served as a reading aid only, in scratch) gave:
- **Start-up**: `SsInit`'s order of stores (LIBSPU's register set-up, the 16-byte silent block written through the
  FIFO, every voice pointed at it, keyed on and off; the reverb work area of types 7/8 cleared in 1 KB DMA blocks, each
  waited for; LIBSND's 16 control registers; every voice set to pitch 1000h, address 1000h, ADSR 80FFh/4000h and keyed
  off). One quirk the trace shows: the start-up clears the low half of the pending key-off mask only, so the first flush
  writes `koff.hi 00ff`.
- **The flush** (once a tick, before the sequences): every voice's envelope read, a 16-entry ring of "read 0" masks of
  which 15 are ANDed (a note is over once its envelope read 0 for 15 flushes), key-ons cancelled by key-offs, then per
  voice the changed registers (pitch, volume L/R, start address, ADSR: LIBSPU's order), KOFF, KON, EON (the voice
  count's bits from LIBSND, the rest read back from the SPU), NON; the masks cleared except EON.
- **The allocator**: the first voice whose status is free and whose envelope read 0; else the voice of the lowest
  priority below the note's, among equal priorities the one with the lowest envelope, then the oldest (an age every
  allocation increments).
- **Volume**: velocity × channel volume / 127; × (VAB volume × 16383) / 16129; × program volume × tone volume /
  16129; × sequence volume / 127 per side; three pans (tone, program, note: below 64 the right side × pan / 63, from 64
  the left × (127 − pan) / 63); sequence notes squared (/ 16383), `SsUtKeyOn`'s not. `SsSepSetVol` and CC 7/10
  recompute the sounding notes' volumes, each with its own small differences (which program record, which pan),
  reproduced as found.
- **Pitch**: LIBSND's two tables (12 semitones and 128 fine steps, the EXE's `0x8005C0E0`/`0x8005C0F8`; not exactly
  2^(k/12) rounded either way, so carried as numbers), the octave shift with rounding, 3FFFh from two octaves above the
  centre note; a sequence note's fine tune is clamped to 7Fh, a pitch bend uses the tone's range (`pbmax` up in 1/63
  steps, `pbmin` down in 1/64 steps).
- **Time**: delta times × 10; a sequence advances resolution × bpm × 10 / (rate × 60) units per tick (rounded to
  nearest; below 1, a frame counter instead); the header's tempo rounded to whole bpm, a tempo event's not.
- **Events and controls**: note-on (velocity 0 = off), CC 0 (VAB), 6 (data entry), 7, 10, 98/99 (NRPN), the rest
  skipped with their value; program change; pitch bend (MSB only); a meta event other than end of track is read as a
  3-byte tempo, and after any meta event the running status is FF; NRPN 20/30 bracket a loop (count by data entry,
  127 endless); NRPN 16/15 and 16/16 set the reverb type and depth; data entry looks the program up, which selects its
  VAB as a side effect later calls see (as do several other calls: the "current note" state is global, as in LIBSND).
- **Play, stop, close**: play resets the position and applies the sequence volume; stop keys the notes off and is done
  again at the next tick; the end of the track keys off at once and stops in the same tick; `SsSepSetVol` only reaches
  sounding notes while the sequence's flags are exactly "playing"; close sets sequence 0's volume to 0 (its notes' too).
  The score table is `table[access] + sequence`, unchecked (section 1's aliasing).
- **Reverb**: a type change silences the reverb (SPUCNT bit 7 cleared if set, depth 0), writes the 32 registers and the
  work area's start, and restores bit 7; the depth is d × 7FFFh / 127. The ten presets and work-area starts are LIBSPU's
  table (`0x8005C840`, `0x8005C810`), type 3 = psx-spx's "Studio Medium" (section 6).
- **VABs**: each program's tone block, each VAG's SPU address (the size table × 8 from version 5), the 0x7EFF0 transfer
  cap, the body sent in 64-byte DMA blocks (so up to 63 bytes after the body go too); `SsVabTransCompleted` reads the
  DMA's completion event.
- **Tick mode**: `SsSetTickMode`'s codes (here 0x1032: no timer, 50 ticks a second).

The traces gave what the code cannot: when the DMA's completion interrupt comes (below), that the emulator reads the
current main volume as 0 (so `CdInit`'s five stores all happen; the port makes them unconditionally, the same main
volume either way), that the vsync falls inside `SsInit` (after 69 of the 97 clearing blocks), and the emulator's SPU
pace of 877.3 samples per vsync (section 6's envelope fits), which decides which voice LIBSND finds free.

### Timing models (the port's)
- **The body DMA's completion**: the oracle completes a body of 30-83 KB (each of the 11 in-game loads of both
  scripts) in its frame, right after `sound_update_loading`'s first poll (that poll returns 0, the next frame's 1), and
  COMMON's 297 KB (`sound_init`) after the next vsync, before that tick's flush (the poll running across the vsync
  returns 1). The port: a body up to `SND_DMA_BYTES_PER_FRAME` (0x30000) completes after the first poll, a longer one
  at the next vsync (`snd_spu_vsync`, at the start of `SsSeqCalledTbyT`). A fit to the oracle, between its 83 KB and
  297 KB.
- **`SsInit`'s vsync**: not modelled (the port's `SsInit` runs in one frame; the PS1's vsync after its 69th clearing
  block is CPU time).
- **The trace's tick** (`spu_trace.c`): the port's frame; a store made by the vsync handler's sequencer tick gets the
  frame being started (`psyq_snd_in_vsync`), as the emulator counts it.

### How it is checked (`tests/port/sound.py`, `tests/port/sound_replay.c`; run by `tests/port/run.py`)
The game calls LIBSND at frames that depend on CPU time the port does not reproduce, so the port's own run cannot be
compared tick for tick with the emulator's (2 below). The exact check takes the game out:
1. **LIBSND on the emulator's timeline.** `sound.py` turns an emulator trace (recorded with `spu_trace.py run --calls`)
   into a replay script: every LIBSND call the game made with its arguments (pointers resolved to the bank files on the
   disc, the bank found by its body DMA's SHA-1), and the emulator's vsyncs (one inside `SsInit`). `sound_replay`,
   built from `port/psyq/libsnd*.c` and `port/src/spu*.c`, makes those calls at those ticks, rendering 877.3 samples per
   vsync, and writes its trace, which must equal the emulator's: every store and DMA block, in order, at the same tick.
   **Result: identical** for the committed `cnty_sel` (6,672 events) and `new_game` (20,204;
   `tests/port/sound/new_game.trace.gz`, 70 KB), and for `first_battle_save` to `battle_won` (214,956 events over
   21,331 ticks: COMMON and 11 bank loads, the field, registration and battle music, 211 `SsUtKeyOn`, pitch bends,
   tempo events, NRPN reverb, three `SsUtAllKeyOff`; 4.9 MB, not committed: `spu_trace.py run
   tests/replay/scripts/first_battle_save.json --until battle_won --calls`, ~9 min, then `sound.py TRACE`, 13 s). One
   DMA block's SHA-1 is not compared: bank 64's body ends 4 bytes before its last 64-byte block, which the DMA reads
   from the RAM after the file (no file holds them; the port sends zeros there, the file padding every other bank has:
   all zero); its address and length are. Identical too in the -m32 and
   ASan/UBSan builds (no report), and at 876, 877, 878 and 880 samples per vsync; at the port's 882 the first
   difference is `new_game`'s tick 1,754: a release that ends 18 samples after the emulator's flush reads it ends 14
   samples before the port's, so LIBSND reuses another voice.
2. **The port's own run** (`sound.py port`, on `run.py`'s `new_game` run): (a) its trace (19,290 events) equals the
   replay of its own calls on its own timeline at 882 samples per vsync: the integration (the handler's tick, the DMA
   model, `CdInit`) is exact; (b) the game makes the same 108 LIBSND calls as on the emulator, in the same order, with
   the same arguments (completion polls aside: 104 for COMMON in the emulator's busy loop, 1 in the port); (c) the frames
   between two calls differ at 14 of them, and every segment that differs (22 of 108: the stores after a call, ticks
   counted from it) follows one of those: the boot (+5 frames before `sound_init`), the CD's timing at loads, a frame the
   PS1 drops before CNTY_SEL's music (+1), the script's START taps meeting the menu at another frame (CNTY_SEL's effect
   +15, the title's first menu sound −14); a different voice then carries the state on (the reverb mask, which voices
   are free). So the port's sound is LIBSND's exact output for the port's game timing, and the game timing is the only
   difference from the emulator's.
3. **`first_battle_save` in the port** (41,826 frames, with a console reset): its trace (451,356 events) equals the
   replay of its own calls when the SPU renders a vsync's samples **before** the game's vsync handler (LIBSND's flush);
   rendered after it (where `port_audio_frame` runs, in the pump's per-frame hook), the first difference is at tick
   19,648: `SsUtAllKeyOff` writes every voice's ADSR at once from game code, and the next flush reads the envelopes
   after 0 samples instead of a frame's, so LIBSND finds other voices free. On the PS1 the SPU runs through the frame
   while the game code runs early in it: the samples of frame N belong between frame N's game code and the flush of
   N + 1.

### Findings for the rest of M3
- **The audio output must render before the vsync handler** (above): `psyq_vsync_tick` runs the game's handler, then
  the runtime's hook (`port_frame`: the CD, the frame log, the script, `port_audio_frame`, the video). The samples for a
  frame must be rendered after that frame's game code and before the next flush, e.g. by a pre-handler hook in
  `psyq_vsync_tick` that renders 882 samples. And every vsync, headless too (LIBSND reads the envelopes); `run.py` skips
  2(a) while `port/src/audio.c` is the step-0 stub.
- **882 vs 877.3 samples per vsync**: the PAL rate the port renders (`SPU_RATE` / 50) is not the emulator's pace; a note
  whose release ends within ~0.5 % of a frame of a flush may get another voice than in the emulator (seen once in
  `new_game`, at tick 1,754). Hardware: 44,100 Hz against 49.76 vsyncs a second would be 886 samples.

### Not covered by the traces (implemented from the disassembly, untested)
`SsSepSetDecrescendo` (FIGHTSTG's fade-out), `SsUtKeyOff`, CC 7 and CC 10 (in SEPs the two scripts do not play), the
slow-tempo frame counter, `SsSetTickMode`'s other codes. Left out (on no SEP on the disc, or never called by the game):
noise voices, RPN/VAG attributes and NRPN attributes other than 15 and 16 (a trace line when one occurs), CC
11/64/91/100/101/121, timer tick modes, the next-SEP chaining, crescendo and tempo changes by call. Section 5's "More
coverage" (a trace per sound key through SOUNDTST or the layer-1 oracle) would cover the first group.

## 8. CD audio: the movies' XA (`port/psyq/xa.c`, `libcd.c`; M5, T17, session 16)
**The data (verified, every sector of the 14 `AAA/STR/MOVIE*.STR`):** one audio channel per movie, file 1 channel 1,
submode `64h` (audio, Form 2, real time), coding `01h` (stereo, 37,800 Hz, 4 bits), in every eighth sector (index 7
mod 8; the video sectors are file 0 channel 1, submode `48h`). At double speed that is 18.75 sectors/s = 37,800
frames/s. The player (`stdwtitl_start_cd_stream`) reads with `CdRead2(0x1E0)`: Setmode `E0h` = double speed, XA-ADPCM
on, 2340-byte sectors, **no XA filter** (no `CdlSetfilter`); the game never calls `CdMix` or Mute/Demute.

**The decoder** (`xa.c`, our own from psx-spx "CDROM XA Audio ADPCM Compression"): the 18 sound groups, 8 units of 28
samples at 4 bits (4 at 8 bits), the four filters, ranges 13..15 as 9, the history per channel carried across groups
and sectors; then the "zigzag" resampler, 37,800 -> 44,100 Hz (seven outputs per six inputs, psx-spx's seven 29-point
tables): 2016 stereo frames per sector -> 2352. Readings where psx-spx is ambiguous (the Python model takes the same):
"/64" and "/8000h" are arithmetic shifts, the latter per term (where the pseudo-code puts it; psx-spx itself calls the
tables "nearly correct": their sums are 0x73E5..0x741D, a gain of ~0.906); a reserved coding field reads as its 0 value;
no emphasis; an 18,900 Hz sample is played twice; mono goes to both sides; the six-step counter starts at 6.

**Delivery** (`libcd.c`; deterministic, in vsync ticks): psx-spx's sector filtering (Setmode bit `40h`, submode audio +
real time, the filter with bit `08h`) sends a sector to the decoder in the CD tick that reads it; its 2352 frames go to
a FIFO in libcd.c (16,384 frames), which hands the SPU's CD input (`spu_cd_input`) 882 frames at the end of every CD
tick: the next vsync's render (`audio.c` renders in the vsync pre-hook, before the tick). The sectors arrive in whole
ticks (3 per tick, an audio one every 2, 3, 3 ticks) where the hardware's arrive continuously, so a run's first sector
waits one tick (20 ms) before it plays; the FIFO's low point is then 588 frames and it never runs dry while the drive
streams. A run ends when the FIFO runs dry; `Pause`, a new read, `StUnSetRing`, `CdInit` and the reset flush it and the
decoder's state (after a flush at most the tick already in the SPU's queue plays). **Assumed:** when the hardware clears
the ADPCM history and the resampler's ring; the one-tick wait (a model of ours); at `--fps` other than 50 the render's
count per vsync changes and the drive's does not (the SPU's queue fills and drops at 60).

**The SPU side (verified in the trace):** `CdInit` sets SPUCNT `C001h` (bit 0: CD audio on) and the CD volume `3FFFh`;
`sound_init`'s `SsSetSerialVol(0, 127, 127)` sets the CD volume to `7FFEh`; the main volume is `3FFFh`; SPUCNT keeps bit
0 through the movie (`C081h`). The SPU mixes the CD input `× CD volume >> 15` with SPUCNT bit 0 (section 6), so the
movie plays at unity: the decoded stream × `7FFEh` × the main volume.

**How it is checked:**
- `tests/xa/run.sh [--m32] [--sanitize] [--all]` (~1 s per variant): `tests/xa/xa_test.c` replays `tests/xa/goldens.txt`
  through `xa.c`: 1317 checks in 32 cases, made by `tests/xa/xa_ref.py gen`, an independent Python model written in the
  pseudo-code's shape (the C goes unit by unit, the tables by column): the tables' SHA-1s; every filter × range at 4 and
  8 bits, mono and stereo, from random history; saturation; whole random sectors two in a row in every coding and the
  reserved ones (decoded and resampled); the resampler on impulses at each of the six phases, a full-scale square wave,
  a step and random frames. Mutating the C (a range, the rounding, the shift, a coefficient, the clamp, the counter,
  the nibble order) fails the goldens.
- `tests/xa/xa_ref.py disc [MOVIE...]` (needs the disc; ~35 s for MOVIEOPN): every audio sector of the movie through
  both models: MOVIEOPN.STR's 2261 sectors give identical 37,800 Hz PCM (4,558,176 frames) and 44,100 Hz PCM (5,317,872
  frames, 120.59 s).
- The game (`--wav` of a scratch script that lets the opening movie play, no START): the soundtrack is in the WAV from
  vsync 364 (first audio sector read in tick 362) to the game's `Pause` at vsync 6281: 2220 sectors, 118.36 s (the game
  stops at the last video frame, 41 sectors before the file's end); the left channel equals the decoded stream ×
  `7FFEh` × `7FFEh` (the main volume's level) sample for sample, the right one is 1 LSB lower (the reverb's residual
  from CNTY_SEL's music, also there before the movie). RMS per second (of 32,767): ~10 for the first 2 s, then 1,241 to
  10,329, ~4,000 in the last seconds. Against the emulator (PCSX-Redux through SDL's disk driver, as `tests/spu/
  capture.py`, the same script cut at 1600 frames of movie): from the music's start, 289 windows of 0.1 s, correlation
  ≥ 0.9997, level ratio 1.000..1.002, the lag exactly 4410 frames per window (same rate, no gaps).
- `new_game` (`tests/port/run.py`) skips the movie after a few frames: the XA plays during them; checkpoints and hashes
  do not change.
