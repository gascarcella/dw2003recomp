# Disc layout

Reproduce with `scripts/extract.sh` and `tools/venv/bin/python tools/disc_survey.py`.

## Image
| | |
|---|---|
| Title | Digimon World 2003 (Europe) (En,Fr,De,Es,It), Redump disc [#3160](http://redump.org/disc/3160/) |
| Serial | SLES-03936 (Volume ID `DMW3`, publisher BANDAI, PVD date 2002-10-01) |
| Image | 1 track, MODE2/2352, 294,280 sectors, 692,146,560 bytes |
| Hashes | SHA-1 `457cb233349ba841e03b33d8060f8fbcadd45cb3`, MD5 `da76a5685b875c353343102b7587fded`, CRC-32 `007df18e` (all match Redump) |
| Rebuild | `scripts/rebuild_iso.sh` produces a **bit-identical** image (checked 2026-09-30) |

**Obfuscated directory records.** A normal ISO 9660 directory walk sees only `SYSTEM.CNF`,
`SLES_039.36` and `DUMMY.`. Everything under `AAA/` can only be reached through the path table,
so dumpsxiso needs `-pt`. With it: 2,385 files, 97 directories.

## Boot executable `SLES_039.36`
`SYSTEM.CNF`: `BOOT = cdrom:\SLES_039.36;1`, `TCB = 4`, `EVENT = 16`, `STACK = 801FFF00`.

PS-X EXE header: entry `pc0 = 0x80010E90`, `gp0 = 0`, text at `0x80010000`, size `0xE1000`
(921,600 bytes, the whole payload), no data/bss fields, stack `0x801FFFF0`, marker
"Sony Computer Entertainment Inc. for Europe area". File SHA-1 `d1b7e4d6…` (`config/SLES_039.36.sha1`).

Memory map (from crt0, Psy-Q signatures and the byte-identical Milestone 0 rebuild; all boundaries exact):

| Range | File offset | Contents |
|---|---|---|
| `0x80010000`–`0x80010E88` | `0x800` | `.rodata` (psylink puts it before `.text`). Starts with a save-ID table for all 3 releases: `BISLPS-03446DMW3-JPN`, `BASLUS-01436DMW3-USA`, `BESLES-03936DMW3-EUR` |
| `0x80010E88`–`0x80010F4C` | `0x1688` | crt0 = Psy-Q `2MBYTE.OBJ`: `__main` stub, `__SN_ENTRY_POINT` (entry `0x80010E90`), 4 data words `0x00200000` |
| `0x80010F4C`–`0x80020D8C` | `0x174C` | **Game code** (65,088 B), one contiguous block |
| `0x80020D8C`–`0x8003EDCC` | `0x1158C` | **Psy-Q 4.7 libraries** (121,496 B), contiguous, no unmatched gaps (`docs/TOOLCHAIN.md`) |
| `0x8003EDCC`–`0x8005CCE8` | `0x2F5CC` | `.data` (incl. small data). NTSC/60fps flags at `0x8005CCAC`/`0x8005CCB0` |
| `0x8005CCE8`–`0x80082CB0` | `0x4D4E8` | `.bss` (cleared by crt0; zeros in the file); `$gp = 0x8005CB50` |
| `0x80082CB0`–`0x800A5DE0` | `0x734B0` | Tier-1 overlay slot = start of heap (crt0 passes `0x80082CB0` to `InitHeap`). Zeros in the file |
| `0x800A5DE0`–`0x800F0DF4` | `0x965E0` | TIM image (8-byte header + `0x4B00C` pixel block), 320×480 16bpp, loaded with the EXE and later overwritten by tier-2/stage overlays |
| `0x800F0DF4`–`0x800F1000` | `0xE15F4` | Zero padding to the 2 KB sector boundary |

crt0: clears `.bss`, sets `sp`, calls `InitHeap(0x80082CB0, …)` at `0x8002507C`, sets `gp`, calls `main` at `0x80014524`.
The header has no bss fields: the whole range up to `0x800F1000` is file content, so the build links
`.bss` as part of the loaded image (DECISIONS 2026-10-01, "Milestone 0 build layout").

## File table in the EXE
The game addresses disc files by **file ID**, not by name. Two parallel tables in `.data`, checked against
dumpsxiso's LBAs for all entries:

| Table | Type | Contents |
|---|---|---|
| `filetable_lba` (`0x80044F6C`) | `u32[2382]` | Start sector (LBA) of each file |
| `filetable_sectors` (`0x800474A4`) | `u16[2382]` | Size in sectors (2048 B; 2336 B for `.STR`) |

The 2,382 entries are every file under `AAA/` (the disc's 2,385 files minus `SYSTEM.CNF`, `SLES_039.36`,
`DUMMY.`). ID 0 = `AAA/DAT/NONE____.BIN`, ID 1 = `AAA/DAT/COUNTRY/JPN/MSAMTMAP.BIN` (LBA `0xE2E`), followed
by the U/E/F/I/D/S variants of the same file. Accessors: `src/main/filetable.c` (`0x800141E4`–`0x80014274`),
called through `filetable_funcs` (`0x80048740`), a table of the four function pointers.

## Files
| Path | Files | Size | Contents |
|---|---|---|---|
| `SLES_039.36` | 1 | 0.9 MB | Boot EXE |
| `DUMMY.` | 1 | 33.7 MB | All zeros (disc padding at the end of the image) |
| `AAA/PRO/*.PRO` | 318 | 2.1 MB | **Code overlays**, raw headerless MIPS (see below) |
| `AAA/STR/MOVIE*.STR` | 14 | 299 MB | FMV, mixed-mode sectors (MDEC video + XA audio), stored by dumpsxiso as 2336-byte sectors |
| `AAA/DAT/Z_STAGE/S###PACK.BIN` | 237 | 228 MB | Per-stage packs (largest data set) |
| `AAA/DAT/FIELD/STAGE/` | 594 | 12.4 MB | Stage data: `S###MASK`, `S###TMPK`, … |
| `AAA/DAT/FIELD/SPRT/` | 179 | 3.1 MB | Field sprites/characters (`P###PLAY`, …) |
| `AAA/DAT/FIGHT/MODEL/` | 245 | 12.1 MB | Battle models (`M###<NAME>`, e.g. `M003AGUM` = Agumon) |
| `AAA/DAT/FIGHT/EFFECT/` | 239 | 3.8 MB | Battle effects |
| `AAA/DAT/FIGHT/` | 6 | 0.1 MB | Battle common data |
| `AAA/DAT/COUNTRY/{ENG,FRA,GER,ITA,SPN,USA,JPN}/` | 7×50 | 4 MB | Localized text/UI. The prefix letter is the language (E, F, D, I, S, U, M). **USA and JPN sets are present on the EU disc** |
| `AAA/DAT/COUNTRY/` | 3 | 0.07 MB | Country-select UI, fonts |
| `AAA/DAT/SOUND/<NAME>/` | 71×2 | 5.7 MB | `MP<NAME>.BIN` (VAB header + SEP) + `MV<NAME>.BIN` (VAB body) per BGM/ENV/BOSS/… |
| `AAA/DAT/CARD/` | 8 | 1.0 MB | Card game data |
| `AAA/DAT/TRAINING/`, `SCREEN/`, `NAMEENT/`, `MCARD/`, `DGLB/` | 45 | 3 MB | Per-screen UI (`*CS`/`*TM` pairs) |
| `AAA/DAT/NONE____.BIN` | 1 | 0.03 MB | ? |

No file starts with a standard Sony header: nearly every file is an offset-table container whose sub-files are
TIMs, sprite banks, VAB/SEP, text tables and so on (`docs/FORMATS.md`, which also maps file IDs to these files).

## Overlays (`AAA/PRO/`)
Raw MIPS code and data with no header, loaded at fixed addresses. Their `jal`s land in the main EXE
and in the overlay itself. Load addresses were found by matching `lui`/`addiu` absolute references
against candidate bases. `tools/disc_survey.py` checks every overlay.

| Tier | Base | Files | Notes |
|---|---|---|---|
| 1 | `0x80082CB0` | 19 | Game modes and menus: `FIELDSTG` (field), `FIGHTSTG` (battle), `CARDGAME`, `STSTATUS`, `STDWTITL` (title, contains Psy-Q LIBPRESS at `0x8008714C`–`0x80087C4C`), `STITSHOP`, `STGTRAIN`, `STGDGLAB`, `STCRD*`, `STGMCARD`, `STDGNAME`, `STPLNMET`, `STFGTREP`, `STAGSLCT`, `CNTY_SEL`, `SOUNDTST`, `SHOCKTST` |
| 2 | `0x800A5DE0` | 2 | `WFIGHTMN`, `WFIGHTTS`: battle sub-overlays, loaded by and linked against `FIGHTSTG` (below) |
| 2 | `0x800A5DE0` | 293 | `WSTAG###` per-stage code and event data, loaded by and linked against `FIELDSTG` (below) |
| none | | 4 | `SDIGIEDT`, `SFSTDATA`, `SMDLDATA`, `WSTAG260`: no code (data only) |

`0x800A5DE0 = 0x80082CB0 + 0x23130`, the size of the largest tier-1 overlay (`CARDGAME`).

Inside a tier-1 overlay, psylink's layout is the same as the EXE's: `.rodata` first (jump tables, constants),
then `.text`, then `.data`, then `.bss`, **written into the file as zeros** like the EXE's (session 5: FIELDSTG's
`0x8009B8EC`–`0x8009BB48` and FIGHTSTG's `0x800A46D0`–`0x800A47A0` are zero variables in code order, not `.data`
order, and nothing is referenced past the file's end). Every tier-1 overlay builds byte-identical
(`config/<overlay>.yaml`; boundaries from `tools/overlay_layout.py`). Functions = splat's count (game code only).
Every tier-1 overlay's code is split into C files (session 6; evidence in each config and in DECISIONS "C patterns
learned in session 6"); each overlay's `.data`/`.bss` is still one asm unit. Matching = functions objdiff scores 100%
(2026-10-02, `objdiff-cli report generate`); the others are `INCLUDE_ASM` or WIP C. `STGTRAIN`'s `.text` starts
0x1CC bytes before where `overlay_layout.py` put it (11 frameless setters), hence 94 functions rather than 85.

| Overlay | Size | `.text` | `.data`+`.bss` | Functions | C files | Matching | Psy-Q |
|---|---|---|---|---|---|---|---|
| `FIELDSTG` | `0x18E98` | `0x80083784`–`0x80092DFC` | to `0x8009BB48` | 222 | 5 | 207 | none |
| `FIGHTSTG` | `0x21AF0` | `0x800832D0`–`0x800A2498` | to `0x800A47A0` | 301 | 7 | 274 | none |
| `CARDGAME` | `0x23130` | `0x80083E34`–`0x800A4DF4` | to `0x800A5DE0` | 306 | 7 | 180 | none |
| `CNTY_SEL` | `0x0177B` | `0x80082CE8`–`0x800843C0` | to `0x8008442B` | 26 | 1 | 26 | none |
| `SHOCKTST` | `0x01E44` | `0x80082DA0`–`0x800849C8` | to `0x80084AF4` | 17 | 1 | 15 | none |
| `SOUNDTST` | `0x02D98` | `0x80084370`–`0x80084D74` | to `0x80085A48` | 8 | 1 | 6 | none |
| `STAGSLCT` | `0x044FC` | `0x800849CC`–`0x80085ED0` | to `0x800871AC` | 8 | 1 | 6 | none |
| `STCRDABM` | `0x02DCC` | `0x80082D88`–`0x80085A4C` | to `0x80085A7C` | 29 | 1 | 29 | none |
| `STCRDDEK` | `0x08BA4` | `0x800831F0`–`0x8008A484` | to `0x8008B854` | 55 | 4 | 50 | none |
| `STCRDSHP` | `0x0A250` | `0x8008300C`–`0x80089288` | to `0x8008CF00` | 45 | 3 | 40 | none |
| `STDGNAME` | `0x06190` | `0x80082F8C`–`0x80086830` | to `0x80088E40` | 32 | 2 | 31 | none |
| `STDWTITL` | `0x065C0` | `0x80082D70`–`0x80087C4C` | to `0x80089270` | 74 | 7 | 74 | LIBPRESS (3 objects, `0x8008714C`–`0x80087C4C`) |
| `STFGTREP` | `0x065F8` | `0x80082E70`–`0x80086A88` | to `0x800892A8` | 36 | 1 | 33 | none |
| `STGDGLAB` | `0x0D100` | `0x80082F48`–`0x8008F470` | to `0x8008FDB0` | 71 | 9 | 59 | none |
| `STGMCARD` | `0x05CA0` | `0x80082CD0`–`0x80088088` | to `0x80088950` | 45 | 2 | 43 | none |
| `STGTRAIN` | `0x0A658` | `0x80083058`–`0x8008C224` | to `0x8008D308` | 94 | 3 | 85 | none |
| `STITSHOP` | `0x0A528` | `0x8008321C`–`0x8008CAA8` | to `0x8008D1D8` | 69 | 2 | 67 | none |
| `STPLNMET` | `0x06F0C` | `0x80082F58`–`0x800885E4` | to `0x80089BBC` | 53 | 3 | 52 | none |
| `STSTATUS` | `0x18A48` | `0x80083558`–`0x8009A784` | to `0x8009B6F8` | 123 | 7 | 107 | none |

`CNTY_SEL` ends 3 bytes into a word: its last 7-byte table is a `bin` subsegment (splat drops a data section's
partial last word), and `configure.py` truncates every output to its config's size.

FIELDSTG and FIGHTSTG use no `$gp` (overlay code is `-G0`). They call into the EXE (81 and 134 `jal`s) and into tier-2 code at `0x800A5DE0`+.
Overlay SHA-1s: `config/overlays.sha1`. `SOUNDTST`, `SHOCKTST` and `STAGSLCT` look like debug/test modes.

### Tier-2 overlays
The EXE's `overlay_load_file(id)` (`overlay_module.unk_C`) copies a file to `main_file_base` = `0x800A5DE0`, just past
the largest tier-1 overlay, while the tier-1 overlay that called it (its **parent**) stays loaded. Tier-2 code
therefore calls and reads its parent as well as the EXE, and links against both (`configure.py` `TIER2_PARENT`,
DECISIONS "Tier-2 overlays link against their parent"). Same psylink layout as tier 1 (`.rodata`, `.text`,
`.data`); `tools/overlay_layout.py --tier2` finds the boundaries. File IDs from the EXE's file table.

| Overlay | ID | Parent | Size | `.text` | `.data` to | Functions | Evidence |
|---|---|---|---|---|---|---|---|
| `WFIGHTMN` | `0x208` | `FIGHTSTG` | `0x050C8` | `0x800A5ECC`–`0x800AAD08` | `0x800AAEA8` | 42 | `func_FIGHTSTG_80087070`: `overlay_load_file(0x208)`, then `jal 0x800A9FF0` (its entry); FIGHTSTG also calls `800A6920`, `800AA110`, `800AA1F0`, `800AA9F0`, `800AAB10`. FIELDSTG queues the file before a battle (`8008A20C`, with FIGHTSTG `0x167` and the data files `0x1CB`/`0x1CC`/`0x1CF`) |
| `WFIGHTTS` | `0x209` | `FIGHTSTG` | `0x0390C` | `0x800A67B8`–`0x800A9368` | `0x800A96EC` | 14 | Same function: `overlay_load_file(0x209)` and `jal 0x800A7A60` when `gamestate_data.unk_26FC.unk_0C()` is non-zero. Its `.rodata` is mostly Shift-JIS strings (a test/debug battle?) |

References (`jal` + `lui`/`lo` pairs) by target: WFIGHTMN 90 into the EXE, 187 into FIGHTSTG
(`0x80082CB0`–`0x800A47A0`), 42 into itself; WFIGHTTS 51 / 26 / 88. None reaches past either file's end (no
`.bss`) or into another tier-1 overlay's range beyond FIGHTSTG's. Both build byte-identical. WFIGHTMN is two C files
(`wfightmn_800A5ECC`, `wfightmn_800A6440`: jump-table parity proves a break before `800A6C44`; the position
follows an update/create pair and helpers that only `800A6C44` calls), WFIGHTTS one (`wfightts_800A67B8`). First C:
4 of 42 WFIGHTMN functions, 6 of 14 WFIGHTTS functions (creators and the two entries; `compiler_id.py --target`).

**`WSTAG###` (293 stage overlays, parent `FIELDSTG`).** Each is a field stage's own code and data. FIELDSTG's
`func_FIELDSTG_800920F8` looks the current stage up in one of two tables of 12-byte records (stage, file ID,
entry), back to back in its `.data`: `fieldstg_stages_2d` (55 records, used when `gamestate_data.unk_263C == 0x2D`)
and `fieldstg_stages` (239 records), and stores the file ID and entry in `fieldstg_stage.code_file`/`entry`;
FIELDSTG then calls `overlay_load_file(unk_00)` and the entry. The 293 tier-2 records name each `WSTAG` file
exactly once (one record points into FIELDSTG itself), and every entry is a function start inside the `.text`
that `overlay_layout.py` finds at `0x800A5DE0`: `tools/overlay_layout.py --wstag-table` checks this and writes
`config/wstag.txt` (per file: size, `.text` and `.data` offsets, table, stage, entry).
- **Layout:** `.text` at offset 0 (192 files; in 32 the first function is a frameless list search that jumps back to
  itself), after one `.rodata` word (85; a constant such as `0x00808080`) or after jump tables (16, up to `0xFC`
  bytes); then `.data` (the stage's event scripts and tables, full of pointers into the file) to the end. No `.bss`.
  `WSTAG925` ends 3 bytes into a word (`0x9E7`). Sizes `0x2A0`–`0x4B28`.
- **Code:** 3–32 functions per file (138 files have 3: an object update, the entry, which creates that object with `object_new`, and a setup filling `fieldstg_stage`), 286,144 B of
  code and 948,883 B of data in all. 196 different code shapes once relocated immediates are masked (largest group 21
  files, 167 files unique), so the code is per stage, not one shared object.
- **References into FIELDSTG:** `fieldstg_stage` (stage state, filled by the setup: all files),
  `fieldstg_attr` (a module table: 292), `fieldstg_event_start` (creates the event runner: 78),
  `fieldstg_sprites_find_first` (1), `D_FIELDSTG_8009B704`/`714` (1). Three files (`WSTAG635`, `660`, `661`) build
  `0x801C001C`/`0x801C041C`, far above any overlay (unexplained; a fixed buffer?).
- **Build:** `configure.py` generates one splat config per line of `config/wstag.txt` (`build/config/wstag###.yaml`,
  one template) and splits them in parallel (~55 s on 4 cores); all 293 build byte-identical as asm units
  (`.rodata`, `.text`, `.data`), objdiff category `stage`.
- `WSTAG260` is in neither table and has no code (16,738 bytes of data); it stays unbuilt with the other data-only files.

## Other releases
The EXE contains the save IDs of all three releases, so they likely share one codebase:

| Release | Serial | Notes |
|---|---|---|
| Japan | SLPS-03446 | Original; has the post-game |
| USA | SLUS-01436 | No post-game. **Active matching decomp: [juandav/dw3_decomp](https://github.com/juandav/dw3_decomp)** (MIT) |
| Europe | SLES-03936 | This project; JP post-game + 5 languages |

The US decomp's layout matches ours, shifted: US `.bss` ends and overlays load at `0x80082448`,
and US tier-2 is at `0x800A4CA4`. EU is `0x868` bytes larger by the end of `.bss`.
