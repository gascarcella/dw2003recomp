# Toolchain: what built the original game

## Evidence from the EU executable (session 1)
| Evidence | Where | Meaning |
|---|---|---|
| `$Id: sys.c,v 1.140 1998/01/12 07:52:27 noda Exp yos $` | `0x800102D8` | libgpu `SYS.OBJ` |
| `$Id: bios.c,v 1.86 1997/03/28 07:42:42 makoto Exp yos $` | `0x80010A00` | libcd `BIOS.OBJ` |
| `$Id: intr.c,v 1.75 1997/02/07 09:00:36 makoto Exp $` | `0x80010A88` | libetc `INTR.OBJ` |
| `Library Programs (c) 1993-1997 Sony Computer Entertainment Inc.` | `0x8005AEF8` | Psy-Q runtime banner |
| libpress strings (`MDEC_rest:bad option`, `MDEC_in_sync`) | `STDWTITL.PRO` | libpress is linked into the title overlay, not the EXE |
| libcd/libspu/libetc debug strings (`CdlReadS`, `SPU:T/O`, `VSync: timeout`) | `.rodata` | Usual Psy-Q set |
| crt0 at entry: clear `.bss`, `InitHeap`, set `$gp`, call `main`; RAM size word `0x00200000` | `0x80010E90` | Psy-Q `2MBYTE.OBJ`-style startup (SN Systems psylink) |
| `.rodata` placed before `.text` at `0x80010000` | layout | psylink ordering |
| `$gp = 0x8005CB50`, 74 gp-relative loads/stores in `.text` | crt0 / code | Some objects built with `-G n > 0`. Most code is likely `-G0` |

The `$Id` dates bound the SDK from below (libgpu from Jan 1998 means Psy-Q ≥ 4.3-era libraries). Later
Psy-Q releases kept those object revisions, so the strings alone can't pin the exact version.

## Psy-Q libraries in the EU EXE (session 2, verified)
`tools/psyq_match.py` (our matcher, over lab313ru/psx_psyq_signatures for Psy-Q 2.6–4.7) on `SLES_039.36`:
- **Psy-Q 4.7.** 284 library objects (123 KB) placed, every one consistent with 4.7, and 4.7 is the only
  version consistent with all of them. Four objects exist only in 4.7: LIBPAD `PDRESRES`, LIBGTE `MSC00`,
  LIBAPI `C114`, LIBSPU `S_SR`. ghidra_psx_ldr's own detection (headless) also reports `4.7.0`.
- **Cross-check:** Ghidra's signature analyzer gives the same name at the same address for 403 of the 433 compared
  names. The rest are explained (Ghidra calls the entry `start`; it lays two 32-byte signatures over the
  middle of `SSSTART.OBJ`, which we match as one 784-byte object).
- **Layout:** crt0 (`2MBYTE.OBJ`) at `0x80010E88`, game code `0x80010F4C`–`0x80020D8C`, libraries
  `0x80020D8C`–`0x8003EDCC` with no unmatched gaps. Overlays: only `STDWTITL` has library code
  (LIBPRESS, `0x8008714C`–`0x80087C4C`); the 293 `WSTAG###` were not scanned yet.
- 4.7 shipped GCC 2.95.2, but games often used an older compiler with newer libraries, so this does
  not settle the game's compiler (see the compiler-ID experiment below).

| Library | Objects | Bytes |
|---|---|---|
| LIBSND | 91 | 35,776 |
| LIBPAD | 13 | 17,136 |
| LIBGPU | 11 | 13,216 |
| LIBSPU | 27 | 12,512 |
| LIBMCRD | 6 | 11,296 |
| LIBCD | 15 | 11,152 |
| LIBGS | 14 | 6,528 |
| LIBC2 | 17 | 3,936 |
| LIBGTE | 18 | 3,440 |
| LIBETC | 5 | 3,264 |
| LIBAPI | 34 | 2,448 |
| LIBCARD | 12 | 1,120 |
| ambiguous (see below) | 20 | 1,120 |
| crt0 `2MBYTE.OBJ` | 1 | 196 |

**Ambiguous objects:** 20 tiny objects (16–96 B) have identical masked bytes in several library objects. One
(`_bu_init`, the same name in LIBAPI and LIBCARD) needs no choice. The other 19 were settled in session 3 from
what the masked bytes hide: the call targets, the variables each one shares with uniquely-matched functions, and
the tables that store them. All are named in `config/symbol_addrs.txt`.

| Address | Name (object) | Evidence |
|---|---|---|
| `8002998C` | `GsSetProjection` (LIBGS GS_106) | only calls `SetGeomScreen` |
| `8002E23C` | `CdReady` (LIBCD S_013) | only calls `CD_ready` |
| `8002E65C` | `CdGetSector` (LIBCD S_021) | returns `CD_getsector(...) == 0` (not `CD_getsector2`) |
| `8002E21C` | `CdSetDebug` (LIBCD S_009) | its variable `D_8005AB28` gates `printf`s in `CD_cw`/BIOS (`>= 3`) |
| `8002E25C` | `CdSyncCallback` (LIBCD S_014) | its variable `D_8005AB20` is saved/cleared by `CdControl`/`CdControlB`/`CdControlF` and called by `CD_sync` |
| `8002E27C` | `CdReadyCallback` (LIBCD S_015) | variable `D_8005AB24` (used by `CD_ready`); `CdRead2` installs the streaming interrupt through it |
| `8002E77C` | `CdDataCallback` (LIBCD S_023) | `StUnSetRing` calls it with `CdReadyCallback(0)` in CD mode; `CdRead2` uses it |
| `8002E7CC` | `DsDataCallback` (LIBDS DSCB_4) | `StUnSetRing`'s other branch (`D_8005AB40 == 1`, DS mode) calls it... |
| `8002E7AC` | `DsReadyCallback` (LIBDS DSCB_2) | ...with this one, mirroring the CD branch's Data + Ready pair |
| `8002BADC`, `8002E10C` | `CdPosToInt` (LIBCD SYS), `CdIntToPos` (LIBCD S_002) | the LIBDS twins would only be linked if referenced; the only LIBDS code in the EXE is the two callbacks above, pulled in by `StUnSetRing`. Callers: `StGetBackloc` (LIBCD) and the game (`filetable_get_cdloc`, `func_80013694`) |
| `8003224C`, `800322AC`, `800322DC`, `8003230C` | `_SsSetNrpnVabAttr15/17/18/19` (LIBSND DE_15/17/18/19) | LIBSND fills its attribute-handler table in code: attribute N is stored at `0x44 + 4N`, and the four go to `0x80/0x88/0x8C/0x90` (`_SsSetNrpnVabAttr16`, uniquely matched, sits at `0x84`) |
| `8003498C` | `SsUtReverbOff` (LIBSND UT_ROFF) | `SpuSetReverb(0)` (`SpuInit` would call `_SpuInit`) |
| `800349AC` | `SsUtReverbOn` (LIBSND UT_RON) | `SpuSetReverb(1)` |
| `800383CC` | `SpuInit` (LIBSPU S_I) | `_SpuInit(0)`, called by `SsInit` |
| `8003A7DC` | `SpuWrite` (LIBSPU S_W) | calls `_spu_Fw` (`SpuRead` would call `_spu_Fr`) |

Confidence: all are direct evidence, except `CdPosToInt`/`CdIntToPos` (an argument from what is linked) and
`DsReadyCallback` (symmetry with the CD branch; `DsSyncCallback`/`DsStartCallback` are the alternatives). LIBDS is
linked only through those callbacks; Ghidra's picks (`CdPosToInt`, `CdIntToPos`, `CdGetSector`, `SpuRead`) were
just the first candidate, and `SpuRead` was wrong.

## What the US decomp established (not yet verified for EU)
[juandav/dw3_decomp](https://github.com/juandav/dw3_decomp) (USA, SLUS-01436) reports:
- **SDK: Psy-Q 4.7**, from [psx_psyq_signatures](https://github.com/lab313ru/psx_psyq_signatures).
- **Game code: GCC 2.8.x `-O2 -G0`**, chosen empirically: m2c output for 342 functions built
  with several compilers. 2.8.x matched 82 untouched, 2.7.2 matched 49, 2.91.66 matched 46.
  2.8.0 and 2.8.1 give identical results.
- One file (`gfx.c`) is built with `-G8`. `main` and one other function also use `$gp`.
- **Psy-Q libraries: GCC 2.7.2 `-O2`** (Sony's own build), with ASPSX-specific delay-slot behaviour
  that maspsx doesn't reproduce; they post-process it with `tools/aspsx_reorder.py`.
- ASPSX 2.56–2.86 behave the same. No divide-by-zero checks (maspsx without `--expand-div`). `-fsigned-char`.

EU was built 2002-10-01 (EXE date), a few months after the US release, from the same codebase
(the save-ID table names all three releases). The same toolchain is the strong prior, but it's
**unconfirmed for EU** until we compile a function and match it.

## Smoke test of the matching toolchain (session 2)
`scripts/check_toolchain.sh` builds `func_800141E4` (`return D_80044F6C[i] != 0;`) with every old-gcc
version + maspsx. The EXE's 7 instructions (`lui v0 / addiu v0 / sll / addu a0,a0,v0 / lw / jr / sltu`)
come out exactly with 2.8.0, 2.8.1 and 2.95.2. 2.7.2 addresses through `$at` (6 instructions), and
2.91.66 allocates `v1` instead of `v0`. One function proves little, but it fits the 2.8.x prior.

## Compiler-ID experiment (session 2): GCC 2.8.x, mostly `-G0`
`tools/compiler_id.py` (reproducible, ~15 s): 275 game functions (4–139 instructions; no jump tables,
rodata references or split switch cases). m2c `--valid-syntax` drafts each one with no hand edits, the
draft is compiled with all 5 GCCs at `-G0` and `-G8` via `tools/cc_psx.sh`, and objdiff scores it against
the original. 259 functions scored; 16 drafts fail to compile on every compiler (m2c errors).

| GCC | Exact matches | Mean match % |
|---|---|---|
| 2.7.2 | 36 | 73.2 |
| **2.8.0** | **68** | **81.6** |
| **2.8.1** | **68** | **81.6** |
| 2.91.66 | 31 | 74.8 |
| 2.95.2 | 30 | 74.1 |

- **No contradictions:** every function another compiler matches, 2.8.x matches too. 35 functions are
  matched by 2.8.x and by none or only some of the others.
- **2.8.0 and 2.8.1 are indistinguishable** (identical scores on all 259 functions).
- Psy-Q 4.7 shipped GCC 2.95.2, so the game used an older compiler (Psy-Q 4.3/4.4 era) with 4.7 libraries,
  like the US build.
- Only 26% of unedited m2c drafts match outright; the misses have a median of 80.5% and 39 are ≥95%. That is
  normal for raw m2c output and doesn't count against 2.8.x.
- **`-G`:** 12 functions match only at `-G0` (none use `$gp`); 11 match only at `-G8` (all in the `$gp` area).
  34 of 343 game functions use `$gp`: `main`, `func_80013780`, and a cluster `0x8001D3F4`–`0x8001FF9C`, over
  9 variables at `0x8005CCB4`–`0x8005CD20`.
- **ASPSX (and maspsx) only use `$gp` for symbols defined earlier in the same file**, never for `.extern`s
  (cc1 writes those at the end of the file, and ASPSX is one-pass). So the original defined each `$gp`
  variable in the same C file as the functions that use it. The experiment turns m2c's `extern`s of those
  symbols into definitions for `-G8`. Without that, 0 of 23 `$gp` functions matched; with it, 11 do.
- **Not measured:** functions over 139 instructions, jump tables, rodata users.

## Unknowns
- Exact file boundaries of the `-G8` code (the `$gp` cluster and `main`'s file).
- Whether EU-only code (the post-game, from the JP build) was compiled differently.
