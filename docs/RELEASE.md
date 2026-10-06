# Releases

The rules are in DECISIONS "Releases: tagged drafts, published by hand": **one Linux x86_64 AppImage** for players (the launcher, the game and the mods'
manifests in one file; the player supplies the disc in the launcher and never compiles anything), built on CI's
`ubuntu-24.04` runner (glibc 2.39), **only for a pushed tag `vX.Y.Z`**, and only as a **draft** GitHub Release: the
maintainer downloads it, tests it and presses Publish. Windows comes later (the Windows track on the project board).

## Making a release

1. Merge what goes into it to `main`; CI is green there.
2. Tag the commit and push the tag (the maintainer only):
   ```sh
   git tag -a v0.1.0 -m "dw2003recomp 0.1.0" && git push origin v0.1.0
   ```
   The pattern is `v[0-9]+.[0-9]+.[0-9]+*` (`v0.1.0`, `v0.2.0-rc1`); the decomp's milestone tags such as
   `v0.1-matching-closed` do not match it and start nothing.
3. `.github/workflows/release.yml` runs (`gh run watch`): the whole of `ci.yml` (every area, with the disc), then the
   game's Release build through the port's M1 test, the AppImage build and its smoke test, then a **draft** release named
   `dw2003recomp v0.1.0` with `dw2003-0.1.0-x86_64.AppImage`, `SHA256SUMS` and notes generated from the merged pull
   requests.
4. Download the draft's AppImage, run it on a desktop (Play with the real disc, a window, sound, a gamepad), and
   **Publish** it on GitHub when it is good (or delete the draft and the tag). Publishing puts binaries built from the
   decompiled code on the public page: the maintainer's decision each time (DECISIONS "Releases: tagged drafts, published by hand").

A failed run creates no release. To retry after a fix, delete the tag (`git push --delete origin v0.1.0; git tag -d
v0.1.0`), tag the fixed commit and push again.

## Trying a branch's AppImage

```sh
gh workflow run release.yml --ref <branch>     # tests + AppImage; no release
gh run download <run-id> -n dw2003-appimage     # the AppImage and SHA256SUMS (kept 30 days)
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

## The smoke test

`scripts/package_appimage.sh --test` runs the AppImage itself (extract-and-run, SDL's offscreen video and dummy
audio) with `--self-test` and `DW3_SELFTEST_GAME=beside`: the launcher's whole self-test, then the game **the launcher
finds by its own lookup beside its executable** (it must be `usr/bin/dw2003` inside the AppImage) with at least one
mod manifest beside it; with `DW3_SELFTEST_DISC` (the disc) the bundled game runs 300 frames from the launcher's command
and must end normally with the disc checked. release.yml always has the disc.
