# Building and testing

The goal is C source that rebuilds a **byte-identical** game executable (built with the original-era toolchain: GCC
2.8.1 and Psy-Q 4.7 libraries), reference tests that pin down what the original does, and the native PC port
(`port/`), checked against those tests.

Contributors: read [`CONTRIBUTING.md`](CONTRIBUTING.md) too.

## Requirements

Linux with git, Python 3.10+, make, gcc/g++, cmake, ninja, curl, xz, unzip (and `dpkg-deb` on hosts with glibc older
than 2.43, for the emulator's runtime sysroot). Everything else is built or unpacked into `tools/` by the setup script,
with no root access needed. You need the same disc image as for playing ([the README](../README.md#provide-your-disc-image)).

## Setup

```sh
cp tools/local.env.example tools/local.env   # set DW3_DISC_BIN to your disc image's .bin
scripts/setup.sh                             # mipsel binutils, mkpsxiso, Python venv, old GCCs, objdiff, decomp tools; links the disc into iso/
```

**The psxstack submodule.** The PC port's runtime, Psy-Q shim and launcher are
[psxstack](https://github.com/gascarcella/psxstack), pinned as the `psxstack` submodule: clone with `--recursive`, or
run `git submodule update --init psxstack` (until psxstack is public, that needs access to it; the PS1 build needs no
submodule). `scripts/worktree_init.sh` does it in a worktree.

**Git worktrees.** A worktree has only tracked files. Run `scripts/worktree_init.sh` in a new worktree: it links the
main checkout's built tools and `tools/local.env`, links the disc and extracts it, and checks the submodule out (about
a second, no rebuild). `scripts/setup.sh` always installs tools into the main checkout and links them into worktrees.

**Optional data checkout.** Maintainers may keep a separate, private checkout (`DW3_GAMEDATA` in `tools/local.env`,
default `../dw2003-gamedata`; `scripts/gamedata_dir.sh`) from which `setup.sh gamedata` and `setup.sh redux` take
their inputs when no disc or download is available (cloud sessions, CI). Nothing in this repository depends on it.

## Extract and build

```sh
scripts/extract.sh          # disc -> extracted/disc + extracted/dw2003.xml
scripts/rebuild_iso.sh      # rebuild build/dw2003.bin and check it is bit-identical to your disc
scripts/build.sh            # split with splat, build with ninja, check every SHA-1 against config/*.sha1
scripts/build.sh --check    # the same from scratch (deletes asm/ and build/ first)
scripts/check_toolchain.sh  # smoke test of the compilers and diff tools
```

Outputs: `build/SLES_039.36` and every overlay (`build/*.PRO`), each checked byte-identical against the original.
Functions not yet decompiled are built from split assembly.

## Tests

```sh
scripts/test.sh                  # every reference-test layer available on this machine; ~2 min
tests/golden/oracle.py check     # layer 1: regenerate the goldens in the emulator, compare byte for byte
tests/host/replay.py             # layer 1, host side: the same C compiled with gcc, replayed through every golden
tests/replay/replay.py check     # layer 2: replay the pad scripts in the emulator, compare checkpoint hashes
```

The reference tests pin down what the original game does: **goldens** of the pure logic (RNG, save checksum, flags,
battle rules, card CPU, GTE, GPU) recorded by calling the game's own functions inside PCSX-Redux, **record/replay**
pad scripts from boot with hashes at checkpoints, and save **round trips**. The emulator comes from
`scripts/setup.sh redux`. Details: [`tests/README.md`](../tests/README.md).

## The PC port and the launcher

```sh
scripts/setup.sh sdl3 imgui
cmake -S port -B build/port-sdl -G Ninja -DPSXSTACK_SDL=ON && cmake --build build/port-sdl
build/port-sdl/dw2003 --disc iso/dw2003.cue --window                 # the game in a window
cmake -S psxstack/launcher -B build/launcher -G Ninja -DPSXSTACK_GAME_JSON=$PWD/port/game/game.json -DPSXSTACK_VERSION_ROOT=$PWD -DPSXSTACK_TOOLS_DIR=$PWD/tools && cmake --build build/launcher
build/launcher/dw2003-launcher                                        # the launcher
scripts/setup.sh imgui sdl3-desktop appimage && scripts/package_appimage.sh --test   # the release AppImage
```

See [`port/README.md`](../port/README.md), psxstack's `docs/RUNTIME.md` and `launcher/README.md`, and
[`docs/RELEASE.md`](RELEASE.md).
