# Releases

The rules are in DECISIONS "Releases: tagged drafts, published by hand": **one Linux x86_64 AppImage** and **one
Windows x86_64 zip** for players (each the launcher, the game and the mods' manifests; the player supplies the disc in
the launcher and never compiles anything), both built on CI's `ubuntu-24.04` runner (the AppImage against glibc 2.39,
the Windows programs cross-compiled with llvm-mingw: DECISIONS "Windows: cross-built from Linux"), **only for a pushed
tag `vX.Y.Z`**, and only as a **draft** GitHub Release: the maintainer downloads them, tests them and presses Publish.
The Windows build is tested on Linux under Wine and Proton; real Windows comes from testers (the board's Windows 9).

## Making a release

1. Merge what goes into it to `main`; CI is green there.
2. Tag the commit and push the tag (the maintainer only):
   ```sh
   git tag -a v0.1.0 -m "dw2003recomp 0.1.0" && git push origin v0.1.0
   ```
   The pattern is `v[0-9]+.[0-9]+.[0-9]+*` (`v0.1.0`, `v0.2.0-rc1`); the decomp's milestone tags such as
   `v0.1-matching-closed` do not match it and start nothing.
3. `.github/workflows/release.yml` runs (`gh run watch`): the whole of `ci.yml` (every area, with the disc), then two
   jobs side by side: `appimage` (the game's Release build through the port's M1 test, the AppImage build and its
   smoke test) and `windows` (the Windows package cross-built and tested unzipped under Wine, "The Windows package"
   below), then a **draft** release named `dw2003recomp v0.1.0` with `dw2003-0.1.0-x86_64.AppImage`,
   `dw2003-0.1.0-x86_64.debug` (the game's debug info, for crash reports; below), `dw2003-0.1.0-windows-x86_64.zip`,
   `dw2003-0.1.0-windows-x86_64-symbols.zip` (the PDBs), `SHA256SUMS`, `SHA256SUMS-windows` and notes generated from
   the merged pull requests.
4. Download the draft's AppImage, run it on a desktop (Play with the real disc, a window, sound, a gamepad), run the
   Windows zip under Wine or Proton the same way (or on a Windows machine when one is at hand), and **Publish** the
   release on GitHub when it is good (or delete the draft and the tag). Publishing puts binaries built from the
   decompiled code on the public page: the maintainer's decision each time (DECISIONS "Releases: tagged drafts, published by hand").

A failed run creates no release. To retry after a fix, delete the tag (`git push --delete origin v0.1.0; git tag -d
v0.1.0`), tag the fixed commit and push again.

## The port stack in a release

The port and the launcher are built from the `psxstack` submodule at its pin (`.gitmodules`; DECISIONS "The port stack
lives in psxstack"). release.yml checks it out with a plain `git submodule update --init` (psxstack is public);
`release_local.sh` fetches it on the host. The release's tag is the input in one place, `DW3_VERSION` (release.yml, `release_local.sh`);
`package_appimage.sh` and `package_windows.sh` export it as `PSXSTACK_VERSION`, which psxstack's `cmake/version.cmake`
stamps into both programs (over `git describe` of this repository: `-DPSXSTACK_VERSION_ROOT`). A new runtime or
launcher reaches a release by bumping the pin.

## A local release

release.yml takes about half an hour; `scripts/release_local.sh` makes the same draft in about 6 minutes (cold) on
the maintainer's machine:

```sh
scripts/release_local.sh v0.2.0                 # origin/main; --commit REF for another pushed commit
scripts/release_local.sh v0.2.0 --build-only    # the packages only: no tag, no release
```

It builds in a Docker `ubuntu:24.04` container (the runner's base, so the same glibc floor; with `wine64` for the
Windows package's test) with the appimage and windows jobs' own steps: the tools, the disc, the game's Release build
through the port's M1 test, `package_appimage.sh --test`, `package_windows.sh --test`. Then it pushes the tag,
cancels the release.yml run the tag starts and creates the same draft with the local files. It does **not** run the
whole CI: check that `main`'s CI is green for that commit first. Needs `docker`, `gh` and the disc (`iso/dw2003.bin`
or `DW3_DISC_BIN`); the container's tools stay in `build/release-local/`. Step 4 above (test, then Publish) is
unchanged.

## Trying a branch's packages

```sh
gh workflow run release.yml --ref <branch>     # tests + the AppImage + the Windows zip; no release
gh run download <run-id> -n dw2003-appimage     # the AppImage, the .debug file and SHA256SUMS (kept 30 days)
gh run download <run-id> -n dw2003-windows      # the Windows zip, its symbols zip and SHA256SUMS-windows
```

Locally (Linux x86_64):

```sh
sudo apt-get install $(scripts/setup.sh --sdl3-desktop-apt)   # Ubuntu; once (Fedora: the same libraries' -devel)
scripts/setup.sh venv imgui sdl3-desktop appimage
scripts/package_appimage.sh --test             # build/release/dw2003-<version>-x86_64.AppImage + SHA256SUMS
```

`--test` runs the smoke test (below) with `iso/dw2003.cue` when it exists. On a system without the static libstdc++
(Fedora: `libstdc++-static`) add `--shared-libstdcxx`: the launcher then needs the system's libstdc++, fine for a test
on that machine, not for a release. `--version V` names the file; the default is the tag being built, else
`git describe` over the release tags, else `dev-<commit>`.

## What the AppImage holds

```
AppRun -> usr/bin/dw2003-launcher        the launcher starts when the AppImage is opened
usr/bin/dw2003                           the game (the SDL build), found by the launcher beside itself
usr/bin/mods/<id>/mod.json               the mods' manifests (port/mods/), listed by the launcher
dw2003recomp.desktop, dw2003recomp.svg   packaging/appimage/ (the icon is our own drawing)
LICENSES/                                ours, SDL3, Dear ImGui, its two fonts, the AppImage runtime (README.txt)
```

Both programs are stripped, link SDL3 statically (`tools/sdl3-desktop`: X11, Wayland, PipeWire, PulseAudio and ALSA
compiled in, each opened with `dlopen` at run time), and need only the C library (`libc`, `libm`, the loader): the
launcher links `libstdc++`/`libgcc` statically (`-DDW3_LAUNCHER_STATIC_RUNTIME=ON`). `package_appimage.sh` fails when
`ldd` shows anything else. The runtime is AppImage's static type-2 runtime: no `libfuse2` is needed (it mounts with
`fusermount`/`fusermount3`; without FUSE, `--appimage-extract-and-run` or `APPIMAGE_EXTRACT_AND_RUN=1` works).

Settings live where the launcher's lookup puts them (`launcher/README.md`): inside a read-only AppImage that is the
per-user directory `~/.local/share/dw2003/` (or `--config-dir`, `$DW3_CONFIG_DIR`, a `settings.json` in the current
directory). Known limit: the game runs from the AppImage's mount, so closing the launcher while the game runs
(the launcher's window is hidden then) can end the mount under the game.

## The Windows package

```sh
scripts/setup.sh venv imgui llvm-mingw sdl3-windows
scripts/package_windows.sh --test              # build/release/dw2003-<version>-windows-x86_64.zip, -symbols.zip, SHA256SUMS-windows
```

`package_windows.sh` cross-builds the game (the SDL build) and the launcher in Release through
`scripts/build_windows.sh` (llvm-mingw's clang and lld, everything static, a PDB beside each executable: DECISIONS
"Windows: cross-built from Linux"), checks both executables (GUI subsystem, the application manifest, no DLL of the
toolchain's or SDL's imported: only Windows' own), and zips one folder:

```
dw2003-<version>-windows-x86_64/
  dw2003-launcher.exe        the launcher (run this)
  dw2003.exe                 the game, found by the launcher beside itself
  mods/<id>/mod.json         the mods' manifests (port/mods/)
  README.txt                 packaging/windows/README.txt: the disc, SmartScreen, where settings and crash reports live
  LICENSES/                  ours, SDL3, Dear ImGui, its two fonts, LLVM's runtime, the mingw-w64 runtime, winpthreads
                             (packaging/windows/LICENSES/README.txt)
dw2003-<version>-windows-x86_64-symbols/
  dw2003.pdb, dw2003-launcher.pdb   the symbols of crash reports and minidumps (not needed to play)
```

`--test` unzips both and, under Wine (`wine` on PATH, the prefix in `build/wine-prefix/`): the launcher's self-test
from the unzipped folder with `DW3_SELFTEST_GAME=beside` (the bundled game and mods found by its own lookup; with
`iso/dw2003.cue` the game runs 300 frames from the launcher's command), then a forced crash of the bundled game
(`tests/port/crash.py --wine`) whose report must symbolize with the symbols zip's PDB. Settings, memory cards, logs
and crash reports go to `%APPDATA%\dw2003\` (`C:\Users\<name>\AppData\Roaming\dw2003\`), or beside the launcher with
a `portable.txt` there (`launcher/README.md`). The programs are not signed: SmartScreen warns the first time.

## Crash reports from a release

A player's crash report (`docs/LAUNCHER.md` "Crash report": the launcher's Copy text, pasted into an issue) names its
build on the `build:` line and its code addresses as `exe+0x...`. The release's `.debug` file is the game's debug info
(`objcopy --only-keep-debug` of the Release build, which keeps `-g`; the AppImage's game is stripped):

```sh
gh release download v0.2.2 -p '*.debug'
scripts/symbolize.py report.txt --binary dw2003-0.2.2-x86_64.debug   # each pc/stack line with its function and line
```

A Windows report (its `platform:` line) resolves through the symbols zip's PDB, and its `crash-<date>.dmp` opens in
WinDbg or Visual Studio with the same PDB (`docs/PORT.md` "Crash report"):

```sh
gh release download v0.2.2 -p '*-windows-x86_64.zip' -p '*-symbols.zip' && unzip -q '*.zip'
scripts/symbolize.py report.txt --binary dw2003-0.2.2-windows-x86_64/dw2003.exe --pdb dw2003-0.2.2-windows-x86_64-symbols/dw2003.pdb
```

## The AppImage's smoke test

`scripts/package_appimage.sh --test` runs the AppImage itself (extract-and-run, SDL's offscreen video and dummy
audio) with `--self-test` and `DW3_SELFTEST_GAME=beside`: the launcher's whole self-test, then the game **the launcher
finds by its own lookup beside its executable** (it must be `usr/bin/dw2003` inside the AppImage) with at least one
mod manifest beside it; with `DW3_SELFTEST_DISC` (the disc) the bundled game runs 300 frames from the launcher's command
and must end normally with the disc checked. release.yml always has the disc.
