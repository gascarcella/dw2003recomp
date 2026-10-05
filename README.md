# Digimon World 2003 decompilation

A work-in-progress decompilation of **Digimon World 2003** (PlayStation, Europe, SLES-03936),
the PAL release of *Digimon World 3* with the extra post-game content.

The goal is C source that rebuilds a **byte-identical** game executable, with reference tests that pin down what the
original does, and then a native PC port (`port/`, in this repository) checked against them.

> **This repository contains no game data.** No disc image, no extracted files, no assets, no SDK files and no
> console BIOS: you need your own copy of the game, and the rebuilt executable is only ever produced from your disc.
> The decompiled source was written by reading the game's machine code, with every borrowing from other projects
> recorded in [`docs/THIRD_PARTY.md`](docs/THIRD_PARTY.md). Our own work (source, tools, tests, documentation) is
> under the [MIT licence](LICENSE); the game itself remains the property of its publisher, and this project is not
> affiliated with or endorsed by Bandai, Bandai Namco or Sony.
>
> **Pull requests are closed by policy for now** (the project is public read-only while the port takes shape);
> issues are welcome. The history starts at a snapshot of the owner's private working history; the initial commit
> names the commit it was taken from.

## Progress
<!-- progress:start -->
_Code compiled from matching C (objdiff, Psy-Q SDK excluded); updated 2026-10-05. Every file already rebuilds byte-identical from split assembly._

| Part | Progress | Matched | Functions | Code bytes |
|---|---|---:|---:|---:|
| **All game code** | `███████████████████░` |  98.6% | 3598 / 3606 | 1,107,536 |
| EXE (`SLES_039.36`) | `████████████████████` | 100.0% | 346 / 346 | 65,088 |
| Tier-1 overlays (19) | `███████████████████░` |  98.1% | 1607 / 1614 | 725,092 |
| Tier-2 battle overlays | `███████████████████░` |  95.7% | 55 / 56 | 31,212 |
| WSTAG stage overlays (293) | `████████████████████` | 100.0% | 1590 / 1590 | 286,144 |

<details><summary>Per overlay</summary>

| Part | Progress | Matched | Functions | Code bytes |
|---|---|---:|---:|---:|
| `FIELDSTG` | `██████████████████░░` |  93.9% | 220 / 222 | 63,096 |
| `FIGHTSTG` | `███████████████████░` |  96.4% | 299 / 301 | 127,432 |
| `CARDGAME` | `████████████████████` | 100.0% | 306 / 306 | 135,104 |
| `CNTY_SEL` | `████████████████████` | 100.0% | 26 / 26 | 5,848 |
| `SHOCKTST` | `█████████████████░░░` |  88.8% | 16 / 17 | 7,208 |
| `SOUNDTST` | `████████████████████` | 100.0% | 8 / 8 | 2,564 |
| `STAGSLCT` | `████████████████████` | 100.0% | 8 / 8 | 5,380 |
| `STCRDABM` | `████████████████████` | 100.0% | 29 / 29 | 11,460 |
| `STCRDDEK` | `████████████████████` | 100.0% | 55 / 55 | 29,332 |
| `STCRDSHP` | `█████████████████░░░` |  85.7% | 44 / 45 | 25,212 |
| `STDGNAME` | `████████████████████` | 100.0% | 32 / 32 | 14,500 |
| `STDWTITL` | `████████████████████` | 100.0% | 74 / 74 | 17,372 |
| `STFGTREP` | `████████████████████` | 100.0% | 36 / 36 | 15,384 |
| `STGDGLAB` | `████████████████████` | 100.0% | 71 / 71 | 50,472 |
| `STGMCARD` | `████████████████████` | 100.0% | 45 / 45 | 21,432 |
| `STGTRAIN` | `████████████████████` | 100.0% | 94 / 94 | 37,324 |
| `STITSHOP` | `███████████████████░` |  97.4% | 68 / 69 | 39,052 |
| `STPLNMET` | `████████████████████` | 100.0% | 53 / 53 | 22,156 |
| `STSTATUS` | `████████████████████` | 100.0% | 123 / 123 | 94,764 |
| `WFIGHTMN` | `████████████████████` | 100.0% | 42 / 42 | 20,028 |
| `WFIGHTTS` | `█████████████████░░░` |  88.0% | 13 / 14 | 11,184 |

</details>
<!-- progress:end -->

## Requirements
Linux with git, Python 3.10+, make, gcc/g++, cmake, ninja, curl, xz, unzip (and `dpkg-deb` on hosts with glibc
older than 2.43, for the emulator's runtime sysroot). Everything else is built or unpacked into `tools/` by the setup
script, with no root access needed.

## Setup
```sh
cp tools/local.env.example tools/local.env   # set DW3_DISC_BIN to your disc image
scripts/setup.sh                             # mipsel binutils, mkpsxiso, Python venv, old GCCs, objdiff, decomp tools; links disc into iso/
```
The disc must be the raw single-track `.bin` (MODE2/2352) of
*Digimon World 2003 (Europe) (En,Fr,De,Es,It)*, SHA-1 `457cb233349ba841e03b33d8060f8fbcadd45cb3`.
A 2048-byte `.iso` conversion will **not** work, because it loses the XA audio sectors.

### Optional data checkout
`scripts/setup.sh` and the tests take their inputs from your disc. Maintainers may additionally keep a separate,
private *data checkout* (`DW3_GAMEDATA` in `tools/local.env`, default `../dw2003-gamedata`; `scripts/gamedata_dir.sh`)
from which `setup.sh gamedata` and `setup.sh redux` take the same inputs when no disc or download is available (cloud
sessions, CI). Nothing in this repository depends on it.

### Git worktrees
A worktree only contains tracked files. Run `scripts/worktree_init.sh` in a new worktree: it symlinks the
main checkout's built tools and `tools/local.env`, links the disc and extracts it (about 1 second, no
rebuild). `scripts/setup.sh` always installs tools into the main checkout and links them into worktrees.

## Extract
```sh
scripts/extract.sh        # disc -> extracted/disc + extracted/dw2003.xml
scripts/rebuild_iso.sh    # rebuild build/dw2003.bin and check it's bit-identical to your disc
tools/venv/bin/python tools/disc_survey.py   # EXE header, file stats, overlay load-address check
```

## Build
```sh
scripts/build.sh            # split with splat, build with ninja, check SHA-1 against config/*.sha1
scripts/build.sh --check    # the same from scratch (deletes asm/ and build/ first)
scripts/check_toolchain.sh  # smoke test of the compilers and diff tools
scripts/check_emulator.sh   # boots the disc headlessly in PCSX-Redux (tools/redux, a pinned download) to the first screen
```
Outputs: `build/SLES_039.36` and every overlay (`build/*.PRO`), each checked byte-identical against the original.
Functions not yet decompiled are built from split assembly. Progress table: `tools/venv/bin/python tools/progress.py
--readme` (updated after every checkpoint).
See [`docs/STATUS.md`](docs/STATUS.md) and [`docs/WORKFLOW.md`](docs/WORKFLOW.md).

## Tests
```sh
scripts/test.sh             # every reference-test layer available on this machine (headless, DW3_JOBS=1); ~90 s
tests/golden/oracle.py check   # layer 1: regenerate the goldens in the emulator, require the committed files byte for byte
tests/host/replay.py        # layer 1, host side: the same C compiled with gcc, replayed through every golden case
tests/replay/replay.py check   # layer 2: replay the pad scripts in the emulator, compare the checkpoint hashes
```
Reference tests pin down what the original game does, for the native port to be checked against
([`tests/README.md`](tests/README.md)): **goldens** of the pure logic (RNG, save checksum, gamestate flags, battle rules,
card CPU: 2,646 calls recorded by calling the game's own functions inside PCSX-Redux), **record/replay** scripts from
boot with hashes of the save struct at checkpoints, and (later) **format round trips**. The goldens replayed on the host
show where the C depends on the PS1 ([`tests/host/FINDINGS.md`](tests/host/FINDINGS.md)). Tests never change the
matching build: they live in `tests/` and `tools/`, never in `src/`. The emulator comes from `scripts/setup.sh redux`.

## Documentation
- [Status](docs/STATUS.md) · [Decisions](docs/DECISIONS.md) · [Disc layout](docs/DISC_LAYOUT.md) ·
  [File formats](docs/FORMATS.md) ·
  [Toolchain](docs/TOOLCHAIN.md) · [Workflow](docs/WORKFLOW.md) · [Session log](docs/SESSION_LOG.md) ·
  [Mechanics](docs/MECHANICS.md) · [Reference tests](tests/README.md)
