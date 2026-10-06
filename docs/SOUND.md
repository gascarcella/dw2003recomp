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
`0 × 16 + 28` = sequence 12 of access 1, i.e. COMMON's second SEP (**assumed**: inferred from the table layout; the
LIBSND task must check it against the trace's key-ons). A faithful LIBSND must keep this aliasing.

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
- **More coverage for LIBSND**: a trace driver that plays every sound of a bank without the CD (SOUNDTST, stage 8: the
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
- **How close to Sony's code may LIBSND follow?** Exact traces need LIBSND's exact arithmetic (tick/tempo
  accumulation, volume and pan curves, voice allocation order). Reading the disassembly for numbers and order is
  probably unavoidable; the port's code must be our own (DECISIONS "Going public": MIT/BSD/zlib only). A decision for
  the user; the interpreter fallback (PC_PORT_PLAN 2.7) stays if a faithful reimplementation proves too costly.
- **Tick alignment** at bank loads and screen changes depends on the CD timing; how strict the port comparison can be
  between two loads is to be measured with the first LIBSND build.
- The sequence aliasing (section 1) is inferred, not yet confirmed note by note.
- 60 Hz (`SsSetTickMode(0x1000)`, the NTSC patch) is not traced.
