# Digimon World 2003 decompilation

> [!NOTE]
> **AI disclosure.** This project is built with heavy use of AI: most of the decompiled C, the tools, the tests, the
> PC port and this documentation were written by AI coding agents (Anthropic's Claude, through Claude Code), directed
> and checked by a human maintainer. The matching C is still verified mechanically: every game file must rebuild
> byte-identical to the original, and the port is checked against an emulator by the reference tests.

A decompilation of **Digimon World 2003** (PlayStation, Europe, SLES-03936), the PAL release of *Digimon World 3*, and
a native **PC port** built from it.

The decompiled C rebuilds a byte-identical copy of the game's executable and overlays, and the same C, compiled for
your PC, runs the game natively, with a launcher and a few optional mods.

> [!IMPORTANT]
> **This repository contains no game data.** You need your own copy of the game: the port reads everything from your
> disc image. This project is not affiliated with or endorsed by Bandai, Bandai Namco or Sony.

## Contents

- [Playing the PC port](#playing-the-pc-port)
  - [Supported platforms](#supported-platforms)
  - [Download](#download)
  - [Provide your disc image](#provide-your-disc-image)
  - [Settings, controls and saves](#settings-controls-and-saves)
- [Reporting an issue](#reporting-an-issue)
- [Development](#development)
  - [Progress](#progress)
  - [Requirements](#requirements)
  - [Setup](#setup)
  - [Extract and build](#extract-and-build)
  - [Tests](#tests)
  - [The PC port and the launcher](#the-pc-port-and-the-launcher)
  - [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)

## Playing the PC port

### Supported platforms

| Platform | State |
|---|---|
| **Linux** x86_64 (glibc 2.39 or newer: Ubuntu 24.04, Fedora 40, or later) | Supported |
| **Windows** | On the way |
| macOS | Not planned yet |

### Download

1. Open the [Releases page](https://github.com/gascarcella/dw2003recomp/releases) and download the latest
   `dw2003-<version>-x86_64.AppImage` (and `SHA256SUMS`, to check it with `sha256sum -c SHA256SUMS`).
2. Make it executable and run it:
   ```sh
   chmod +x dw2003-*-x86_64.AppImage
   ./dw2003-*-x86_64.AppImage
   ```
   The AppImage opens the **launcher**, which holds the game and its mods. Nothing needs installing or compiling. If
   your system has no FUSE, run it with `--appimage-extract-and-run`.

### Provide your disc image

The game is read from **your own disc image** of *Digimon World 2003 (Europe) (En,Fr,De,Es,It)*:

- It must be the raw **`.bin` + `.cue`** dump (MODE2/2352, one track, 692,146,560 bytes), SHA-1
  `457cb233349ba841e03b33d8060f8fbcadd45cb3`. Dumping tools such as Redump-style rippers produce this format.
- A 2048-byte `.iso` conversion will **not** work: it loses the movies' audio sectors.
- Other releases (the US *Digimon World 3*, the Japanese release) are not supported.

In the launcher's **Disc** screen choose the `.cue` (or `.bin`), drag it onto the window, or type its path. The
launcher checks the SHA-1 once and remembers the disc; then press **Play**.

### Settings, controls and saves

- **Settings** (window scale, fullscreen, 50 or 60 Hz, mute, memory cards), **controls** (keyboard and gamepad
  rebinding, hotkeys) and **mods** (fast-forward, skip dialogues, disable battle animations) are all in the launcher.
- Settings and memory cards (`card1.mcd`, `card2.mcd`, the same format as PCSX-Redux's) are kept in
  `~/.local/share/dw2003/`. A `portable.txt` file beside the launcher keeps them beside it instead.

Details: [`launcher/README.md`](launcher/README.md) and [`docs/LAUNCHER.md`](docs/LAUNCHER.md).

## Reporting an issue

Found a bug, a crash, or something that plays differently from the original? Please
[open an issue](https://github.com/gascarcella/dw2003recomp/issues/new/choose) and pick the matching template. A
useful report has:

- the **version** (the AppImage's file name, or the commit you built) and your **system** (distribution, desktop,
  GPU, gamepad);
- **what happened** and **what you expected**, ideally compared with the original game on a PS1 or an emulator;
- **steps to reproduce** from a new game or a save (attach the memory card from `~/.local/share/dw2003/` if it helps);
- the **output**: the launcher shows the game's last lines after an error and has a **Copy** button for them;
- screenshots or a short video when it is visual.

Please check the [open issues](https://github.com/gascarcella/dw2003recomp/issues) first, and never attach the game's
files or a disc image. Questions about the code are welcome as issues too. Planned work is tracked in the
[project board](https://github.com/users/gascarcella/projects/1).

## Development

The goal is C source that rebuilds a **byte-identical** game executable (built with the original-era toolchain: GCC
2.8.1 and Psy-Q 4.7 libraries), reference tests that pin down what the original does, and the native PC port
(`port/`), checked against those tests.

### Progress

<!-- progress:start -->
_Code compiled from matching C (objdiff, Psy-Q SDK excluded); updated 2026-10-06. Every file already rebuilds byte-identical from split assembly._

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

The current state, in words: [`docs/STATUS.md`](docs/STATUS.md).

### Requirements

Linux with git, Python 3.10+, make, gcc/g++, cmake, ninja, curl, xz, unzip (and `dpkg-deb` on hosts with glibc older
than 2.43, for the emulator's runtime sysroot). Everything else is built or unpacked into `tools/` by the setup script,
with no root access needed. You need the same disc image as for playing (above).

### Setup

```sh
cp tools/local.env.example tools/local.env   # set DW3_DISC_BIN to your disc image's .bin
scripts/setup.sh                             # mipsel binutils, mkpsxiso, Python venv, old GCCs, objdiff, decomp tools; links the disc into iso/
```

**Git worktrees.** A worktree has only tracked files. Run `scripts/worktree_init.sh` in a new worktree: it links the
main checkout's built tools and `tools/local.env`, links the disc and extracts it (about a second, no rebuild).
`scripts/setup.sh` always installs tools into the main checkout and links them into worktrees.

**Optional data checkout.** Maintainers may keep a separate, private checkout (`DW3_GAMEDATA` in `tools/local.env`,
default `../dw2003-gamedata`; `scripts/gamedata_dir.sh`) from which `setup.sh gamedata` and `setup.sh redux` take
their inputs when no disc or download is available (cloud sessions, CI). Nothing in this repository depends on it.

### Extract and build

```sh
scripts/extract.sh          # disc -> extracted/disc + extracted/dw2003.xml
scripts/rebuild_iso.sh      # rebuild build/dw2003.bin and check it is bit-identical to your disc
scripts/build.sh            # split with splat, build with ninja, check every SHA-1 against config/*.sha1
scripts/build.sh --check    # the same from scratch (deletes asm/ and build/ first)
scripts/check_toolchain.sh  # smoke test of the compilers and diff tools
```

Outputs: `build/SLES_039.36` and every overlay (`build/*.PRO`), each checked byte-identical against the original.
Functions not yet decompiled are built from split assembly.

### Tests

```sh
scripts/test.sh                  # every reference-test layer available on this machine; ~2 min
tests/golden/oracle.py check     # layer 1: regenerate the goldens in the emulator, compare byte for byte
tests/host/replay.py             # layer 1, host side: the same C compiled with gcc, replayed through every golden
tests/replay/replay.py check     # layer 2: replay the pad scripts in the emulator, compare checkpoint hashes
```

The reference tests pin down what the original game does: **goldens** of the pure logic (RNG, save checksum, flags,
battle rules, card CPU, GTE, GPU) recorded by calling the game's own functions inside PCSX-Redux, **record/replay**
pad scripts from boot with hashes at checkpoints, and save **round trips**. The emulator comes from
`scripts/setup.sh redux`. Details: [`tests/README.md`](tests/README.md).

### The PC port and the launcher

```sh
scripts/setup.sh sdl3 imgui
cmake -S port -B build/port-sdl -G Ninja -DDW3_PORT_SDL=ON && cmake --build build/port-sdl
build/port-sdl/dw2003 --disc iso/dw2003.cue --window                 # the game in a window
cmake -S launcher -B build/launcher -G Ninja && cmake --build build/launcher
build/launcher/dw2003-launcher                                        # the launcher
scripts/setup.sh imgui sdl3-desktop appimage && scripts/package_appimage.sh --test   # the release AppImage
```

See [`port/README.md`](port/README.md), [`launcher/README.md`](launcher/README.md) and
[`docs/RELEASE.md`](docs/RELEASE.md).

### Documentation

| Document | Contents |
|---|---|
| [`docs/STATUS.md`](docs/STATUS.md) | Where the project stands |
| [`docs/DECISIONS.md`](docs/DECISIONS.md) | The key decisions and why |
| [`docs/MATCHING.md`](docs/MATCHING.md) | How to match a function: workflow, tools, C patterns |
| [`docs/TOOLCHAIN.md`](docs/TOOLCHAIN.md) | What built the original game |
| [`docs/DISC_LAYOUT.md`](docs/DISC_LAYOUT.md) | Disc files, memory map, overlays |
| [`docs/FORMATS.md`](docs/FORMATS.md) | The game's file formats |
| [`docs/MECHANICS.md`](docs/MECHANICS.md) | Game mechanics mapped to functions and tests |
| [`docs/PORT.md`](docs/PORT.md) | How the PC port works |
| [`docs/LAUNCHER.md`](docs/LAUNCHER.md) | The launcher/game contract, settings and mods |
| [`docs/SOUND.md`](docs/SOUND.md) | Sound: LIBSND, the SPU, CD audio |
| [`docs/RELEASE.md`](docs/RELEASE.md) | Making a release |
| [`docs/THIRD_PARTY.md`](docs/THIRD_PARTY.md) | Everything borrowed from other projects |
| [`tests/README.md`](tests/README.md) | The reference tests |

Planned work lives in the [project board](https://github.com/users/gascarcella/projects/1), problems in
[issues](https://github.com/gascarcella/dw2003recomp/issues).

## Contributing

Contributions are welcome: matching a function, naming symbols, documenting a format, fixing the port, or testing it
on your machine. Read [`CONTRIBUTING.md`](CONTRIBUTING.md) before opening a pull request.

## License

Our own work (source, tools, tests, documentation) is under the [MIT licence](LICENSE). The decompiled source was
written by reading the game's machine code; every borrowing from other projects is recorded in
[`docs/THIRD_PARTY.md`](docs/THIRD_PARTY.md). The game itself remains the property of its publisher.
