# Digimon World 2003 decompilation

[![Listed on PlayPorted](https://playported.com/badge.svg)](https://playported.com/ports/210-digimon-world-3/)

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

**Updates:** when it starts, the launcher asks GitHub for this repository's latest release (one request to
api.github.com's releases endpoint through your system's `curl`, with the launcher's name and version as the
User-Agent: no identifiers, nothing about you or your machine). When a newer one is out it says so and can download
and install it for you (**Update now**, checked against GitHub's SHA-256 of the file), or open its page. Drafts and
prereleases are never offered. To turn the check off: untick "Check for updates when the launcher starts" on the
**Settings** screen (`launcher.updates.check` in `settings.json`), or start the launcher with `--no-update-check`;
the Settings screen's **Check now** still works. Details: [`docs/RELEASE.md`](docs/RELEASE.md) "The launcher's
update check".

### Provide your disc image

The game is read from **your own disc image** of *Digimon World 2003 (Europe) (En,Fr,De,Es,It)*:

- It must be the raw **`.bin` + `.cue`** dump (MODE2/2352, one track, 692,146,560 bytes), SHA-1
  `457cb233349ba841e03b33d8060f8fbcadd45cb3`. Dumping tools such as Redump-style rippers produce this format.
- A 2048-byte `.iso` conversion will **not** work: it loses the movies' audio sectors.
- Other releases (the US *Digimon World 3*, the Japanese release) are not supported.

In the launcher's **Disc** screen choose the `.cue` (or `.bin`), drag it onto the window, or type its path. The
launcher checks the SHA-1 once and remembers the disc; then press **Play**.

### Settings, controls and saves

- **Settings** (window scale, fullscreen, scaling (fit, integer or stretch), 50 or 60 Hz, mute, memory cards), **controls** (keyboard and gamepad
  rebinding, hotkeys, a second gamepad as player 2) and **mods** (fast-forward, faster loading, skip dialogues, disable battle animations) are all in the launcher.
- **The in-game menu:** Escape, or Back (Select) held with Start on a gamepad, pauses the game and opens the same
  Settings, Controls and Mods inside its window (the Steam Deck's way: no switching windows). Changes apply at once,
  except the few marked *(restart)*; Resume or the key again goes on, Quit returns to the launcher. It fits itself to
  the screen; **Menu size** in Settings makes it smaller or larger.
- **Steam:** Settings > Steam > **Add to Steam** puts the game in your Steam library as a non-Steam game (for the Steam
  Deck's Game Mode, Steam's controller settings and overlay); quit Steam first. **Remove from Steam** takes it out
  again, and **Update shortcut** follows the AppImage when you move it.
- **Quick save and load:** F5 saves the whole game to a quick slot, F9 goes on from it (one slot, `quick.state` beside
  the settings; a slot is valid for the build that wrote it). Both keys are rebindable in the launcher's hotkeys.
- Settings and memory cards (`card1.mcd`, `card2.mcd`, the same format as PCSX-Redux's) are kept in
  `~/.local/share/dw2003/`. A `portable.txt` file beside the launcher keeps them beside it instead.

Details: [`docs/LAUNCHER.md`](docs/LAUNCHER.md) and psxstack's `launcher/README.md`.

## Reporting an issue

Found a bug, a crash, or something that plays differently from the original? Please
[open an issue](https://github.com/gascarcella/dw2003recomp/issues/new/choose) and pick the matching template; what a
useful report has is in [`docs/REPORTING.md`](docs/REPORTING.md). Never attach the game's files or a disc image.

## Documentation

| Document | Contents |
|---|---|
| [`docs/REPORTING.md`](docs/REPORTING.md) | Reporting a bug or a crash: what a useful report has |
| [`docs/BUILDING.md`](docs/BUILDING.md) | Building and testing: requirements, setup, the byte-identical build, the tests, the port and the launcher |
| [`docs/PROGRESS.md`](docs/PROGRESS.md) | How much of the game's code is matched, per part and per overlay |
| [`docs/CONTRIBUTING.md`](docs/CONTRIBUTING.md) | How to contribute |
| [`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md) | The working guide: repository layout, workflow, conventions, commands |
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
on your machine. Read [`docs/CONTRIBUTING.md`](docs/CONTRIBUTING.md) before opening a pull request.

## License

Our own work (source, tools, tests, documentation) is under the [MIT licence](LICENSE). The decompiled source was
written by reading the game's machine code; every borrowing from other projects is recorded in
[`docs/THIRD_PARTY.md`](docs/THIRD_PARTY.md). The game itself remains the property of its publisher.
