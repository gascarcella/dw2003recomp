# Toolchain and workflow plan

Status: **approved 2026-10-01** (summarized in `DECISIONS.md`). This file keeps the detail and tradeoffs.
Tool versions were checked on 2026-10-01.

## 1. Splitting: splat
**Proposal: splat 0.50 (`splat64`, already in the venv), one YAML per overlay.**

There's no real alternative for PSX: every active PS1 decomp uses splat on top of spimdisasm.
The other options are hand-rolled spimdisasm scripts (more work, nothing gained) or Ghidra exports
(not reproducible or diffable).

`config/main.yaml` for the EXE:
- `platform: psx`, `compiler: GCC`
- `[0x0, header]` segment
- code segment from `0x800` at `0x80010000`, with `section_order: [.rodata, .text, .data, .bss]`
  (psylink puts rodata first)
- `gp_value: 0x8005CB50`
- `bss_size` from crt0: `0x80082CB0 - 0x8005CCE8`
- `migrate_rodata_to_functions: True`, `find_file_boundaries: True`
- tail TIM as a `bin` segment

The 0.50 caveat: breaking symbol/overlay model changes (`prioritized_segments`, stricter scoping).
Pin `splat64==0.50.0` (done). Older guides (BFM is on 0.41) won't map one-to-one.

## 2. Compiler
What we know: Psy-Q libs ≥ 1998 (the `$Id`s), the US build of the same code matches **GCC 2.8.x `-O2`**,
and some objects use `$gp` (`-G8`). decomp.me's Psy-Q mapping:

| Psy-Q | GCC | ASPSX |
|---|---|---|
| 4.0 | 2.7.2 | 2.56 |
| 4.3 | 2.8.0 | 2.77 |
| 4.4 | 2.8.1 | 2.79 |
| 4.5 | 2.91.66 | 2.81 |
| 4.6 | 2.95.2 | 2.86 |

Games often linked newer libraries with an older compiler, so the libs don't fix the compiler.

| Option | What it is | Redistributable? | Verdict |
|---|---|---|---|
| **decompals/old-gcc `gcc-2.8.1-psx`** (release 0.17) + **maspsx** | GNU GCC built for PSX; maspsx turns GNU `as` into an ASPSX emulator | Yes (GPL / MIT) | **Proposed default.** Fully open and reproducible |
| Sony `CC1PSX.EXE`/`ASPSX.EXE` via wibo (or wine) | The real thing | **No.** User-obtained, matching release (4.3/4.4) | Fallback only, if maspsx is suspected (DECISIONS 2026-10-01) |
| Other old-gcc builds (2.7.2, 2.91.66, 2.95.2) | Alternatives | Yes | Installed too, for the compiler-ID experiment |

**You supply nothing for the default path.** The Psy-Q SDK (LIBs, headers, ASPSX) is optional. It's
only worth having for cross-checks or the real headers. We write our own SDK declarations
in `include/psyq/` as needed. `jype0/psyq_headers` exists, but it's Sony's headers, which is a grey area.

**Compiler-ID experiment (session 2; done, result: GCC 2.8.x `-G0`, see `TOOLCHAIN.md`):** run m2c over ~150 small/medium EU game functions, compile each
with 2.7.2 / 2.8.0 / 2.8.1 / 2.91.66 / 2.95.2 (+ maspsx at the matching ASPSX version), and score
exact matches. The winner becomes the default; per-file overrides are allowed (Vagrant Story mixes three
compilers). Score `-G0` vs `-G8` per function from the gp usage.

## 3. Diffing and matching tools
| Tool | Role | Install |
|---|---|---|
| **objdiff** (v3.8.2): `objdiff-cli` + GUI | Per-function diff, progress report (`report generate`) | Prebuilt binary into `tools/`, checksum-pinned |
| **m2c** | First-draft C from asm (`--target mips-gcc-c`) | git clone at a pinned commit (PyPI is stale) |
| **maspsx** | ASPSX emulation | git clone at a pinned commit (no tags/PyPI) |
| **asm-differ** | Terminal diff with live rebuild (`diff.py -mwo`); nicer than objdiff for tight loops | git, pinned |
| **decomp-permuter** | Brute-force register-allocation fixes | git, pinned; `pycparser<3` |
| decomp.me | Real Sony compiler presets, community help | Fallback, case by case with the user's approval (**scratches are public**) |

Pinned clones via `scripts/setup.sh` (not submodules), matching how binutils and mkpsxiso are already handled.
Alternative: git submodules. Tradeoff: submodules pin in-tree and `git clone --recursive` just works,
but they're clumsier to update.

## 4. Build system
**Proposal: `configure.py` → `build.ninja`** (ninja is installed). It generates per-object rules for the
EXE and ~315 overlays, plus `objdiff.json`.

Alternative: a Makefile (the US repo, sotn). Make is simpler to read, but 300+ overlay link units with
per-file compiler flags get unwieldy, and ninja's incremental rebuilds are faster.

Pipeline per C file: `cpp` → `cc1` (old-gcc) → `maspsx` → `mipsel-linux-gnu-as` → link with a
splat-generated linker script → `objcopy` → prepend the 0x800 header → compare against `config/*.sha1`.

## 5. Milestone: the first function
1. **Milestone 0, "asm build matches":** split everything with splat (all `INCLUDE_ASM`), build
   `SLES_039.36`, and check SHA-1 = `d1b7e4d6…`. This proves the split, linker script and header before any C exists.
2. **Milestone 1, "hello world":** `func_800141E4` (7 instructions, leaf, called through a table):
   ```
   lui v0,0x8004 ; addiu v0,v0,0x4F6C ; sll a0,a0,2 ; addu a0,a0,v0 ; lw v0,0(a0) ; jr ra ; sltu v0,zero,v0
   ```
   ```c
   extern void* D_80044F6C[];
   s32 func_800141E4(s32 i) { return D_80044F6C[i] != 0; }
   ```
   Proof: (a) objdiff reports the function 100%. (b) The **whole EXE** still hashes to `d1b7e4d6…`
   with the function built from C. (c) CI-style `scripts/build.sh --check` fails on any mismatch.
   Caveat: a function this simple compiles the same under most GCCs, so it proves the pipeline, not the
   compiler (that's the experiment in §2).

## 6. Library (SDK) functions
**Identify:** apply lab313ru/psx_psyq_signatures (JSON for Psy-Q 2.6–4.7) with our own small matcher
script that writes `config/symbol_addrs.txt` entries. Cross-check with ghidra_psx_ldr's "PsyQ Signatures"
analyzer (already installed) and xsig-style hashing for code duplicated across overlays.

**Treat:**

| Option | Cost | Port value | Verdict |
|---|---|---|---|
| **Keep as split asm**, named and excluded from progress | None | None needed: the port replaces the SDK with PC shims | **Proposed** |
| Decompile to C (US repo, sotn) | High: the US repo binary-patches two `cc1`s plus an ASPSX reorder pass | Low | No |
| Link Sony's original objects (BFM) | Needs your Psy-Q 4.7; LIB→ELF conversion | None | Optional later |

## 7. Overlays
- One splat YAML per overlay: 19 tier-1 at `0x80082CB0`, plus `WFIGHTMN`/`WFIGHTTS` and 293 `WSTAG###`
  at `0x800A5DE0`. Stage YAMLs are **generated** by a script from a list, never hand-written.
- Each overlay is its own link unit at its base, resolving EXE symbols via the EXE's symbol list (and
  `FIGHTSTG`'s for the fight sub-overlays, `FIELDSTG`'s for stages). Each one checks against `config/<NAME>.PRO.sha1`.
  Done for FIELDSTG and FIGHTSTG (session 4; DECISIONS "Overlays link against the EXE").
- Naming: overlays overlap in VRAM, so addresses aren't unique. Unnamed overlay symbols are
  `func_FIELDSTG_80083000` (splat's per-segment `symbol_name_format`), names in `config/<overlay>.symbols.txt`.
- Duplicated code across the 293 stages (likely shared script helpers): find it by relocation-masked
  hashing and write it once (shared `.inc.c`).
- Order: main EXE first (in the US build, game code is ~64 KB and the SDK ~120 KB; EU not yet measured), then `FIELDSTG`/`FIGHTSTG`
  (the core), then the stages in bulk.

## 8. Dynamic analysis
| | PCSX-Redux | DuckStation |
|---|---|---|
| License | MIT, open | CC BY-NC-ND (fine to use, not to modify/redistribute) |
| Scripting | **Lua** (breakpoints with callbacks, memory read/write, savestates, symbols), headless `-no-ui -exec/-dofile` (checked working here), **web API** (port 8080) | None |
| GDB server | Port 3333 (`-gdb`): breakpoints and watchpoints | Port 2345 (settings), breakpoints and watchpoints |
| Debugger UI | Assembly, call stack, memory, VRAM, typed debugger | Qt debugger, memory scanner |
| Accuracy | Good | Excellent (reference for "does the real game do X") |

**Status (session 8):** PCSX-Redux is installed by `scripts/setup.sh redux` (a pinned build, downloaded or from the data checkout,
run headlessly; `scripts/check_emulator.sh` boots the disc to the first screen; DECISIONS "PCSX-Redux pinned").

**Proposal:** PCSX-Redux is the scriptable oracle: Lua scripts in `tools/redux/` (dump RAM at a
breakpoint, trace calls into a function, check which overlay is resident). DuckStation is for
playing to a point and checking accuracy.

`gdb` isn't installed. If we want GDB, add a `setup.sh` step that builds `gdb` for
`mipsel-linux-gnu` into `tools/` (same as binutils; no sudo).

**MCP: not now.** No maintained PCSX-Redux MCP exists (the one DuckStation MCP is Windows-only and
unmaintained). Redux's Lua and web API are easy to script from the CLI. Revisit if we keep doing
the same interactive debugging dance, since a thin MCP over the web API would be ~200 lines.

**Ghidra MCP: worth a trial, later.** Useful for exploring overlays, xrefs and struct propagation without
leaving the CLI. Options: `clearbluejar/pyghidra-mcp` (headless, `uvx`) or `bethington/ghidra-mcp`
(Ghidra 12.1.3; you have 12.0, so check compatibility). Watch the port clash: both default to 8080
like Redux. Ghidra is an exploration aid; the repo (symbols, C) stays the source of truth.

## 9. Reference tests (session 9)
Three layers, all headless from `scripts/test.sh` (`tests/README.md`; DECISIONS "Reference tests for the port",
"Layer-1 oracle", "Replay runner", "Overlay goldens"):
1. **Goldens of the pure logic** (`tests/golden/`): `oracle.lua` boots the disc in PCSX-Redux (`-debugger -interpreter`),
   hijacks the main loop at `pad_update` and calls the game's own functions with fixtures written into RAM; `oracle.py`
   turns family modules (`families/*.py`) into jobs and JSON goldens, one boot for every family. Overlay code is tested
   by copying the overlay into its slot from a kept setup case. `tests/host/replay.py` replays the goldens through the
   same C compiled with gcc for the host; differences are findings (`tests/host/FINDINGS.md`), never a change to `src/`.
2. **Record/replay** (`tests/replay/`): pad scripts from boot, the SHA-1 of `gamestate_data` at checkpoints, the overlay
   and map sequences; `--repeat 2` proves the emulator deterministic.
3. **Formats and save round trips** (`tests/formats/`): `run.sh` runs the tools' `--check` modes (file table, text, WSTAG stage table, flag ranges; `tests/README.md`); save round trips not written yet.
`docs/MECHANICS.md` maps the mechanics to the functions and tables the tests cover.
