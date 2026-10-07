# The launcher

`dw2003-launcher` finds the user's settings, edits `settings.json` and starts the PC port. The contract with the game
(the settings file, the mods' manifests) is `docs/LAUNCHER.md`, the decisions behind it DECISIONS "Launcher and mods".
It is C++17 with Dear ImGui on SDL3 + `SDL_Renderer`; the game stays C.

**Status (Linux):** the window and its navigation, the settings directory, reading and writing `settings.json`, the disc screen (file dialog, drag-and-drop, typed path, the SHA-1
check), the play button (the game's exit status and last lines on an error), the settings screen (video, 50/60 Hz,
audio, memory cards), the controls screen (keyboard, gamepad, hotkeys), the mods screen (each mod's page generated from
its `mod.json`), the self-test. The code uses SDL's calls, not POSIX, so the same source cross-builds for Windows
(`scripts/build_windows.sh --launcher`: a GUI-subsystem `dw2003-launcher.exe` with a manifest, `launcher/windows/`;
the self-test passes under Wine, `docs/RELEASE.md` "The Windows package"); on real Windows it has not run yet.

## Build

```sh
scripts/setup.sh sdl3 imgui        # SDL3 (static) and Dear ImGui at their pinned versions, into tools/
cmake -S launcher -B build/launcher -G Ninja && cmake --build build/launcher
build/launcher/dw2003-launcher [--config-dir DIR] [--game PATH]
```

Its own CMake project beside `port/`: the game's build and tests do not depend on it. SDL3 comes from `tools/sdl3`
(or `SDL3_DIR`), Dear ImGui from `tools/imgui` (or `-DDW3_IMGUI_DIR=...`); in a worktree whose `tools/` has no link
to them yet, the main checkout's are used. For a desktop window, SDL3 needs the X11/Wayland `-dev` headers at its
build (`scripts/setup.sh` "sdl3"); without them only the offscreen driver exists (the self-test's).

Two files of the port's runtime are compiled into the launcher as they are, never changed for it:
`port/src/json.c` (the strict JSON reader) and `port/src/sha1.c`.

## The settings directory

The first of these that applies:

1. `--config-dir DIR`, else `$DW3_CONFIG_DIR` (relative to the current directory; created at the first save).
2. A file `portable.txt` beside the launcher: the launcher's own directory (portable mode).
3. A `settings.json` that already exists in the current directory (for local testing; never created there by itself).
4. The per-user directory, `SDL_GetPrefPath("", "dw2003")`: `~/.local/share/dw2003/` on Linux (`$XDG_DATA_HOME`
   when set), `%APPDATA%\dw2003\` on Windows.

The status bar and the Settings screen show the directory and which rule chose it. The directory holds
`settings.json`, the memory card (`card1.mcd`) and, later, `mods/`.

## settings.json

Schema 1, as settled by the game's side (`docs/LAUNCHER.md` "Settings file": `port/src/settings.c` reads it, `dw2003
--config FILE --print-settings` validates it). The launcher edits these members and **keeps every other one as it was**
(order included): `input`, `mods`, `watchdog`, `video.window` and keys it does not know.

| Member | Type | Default | Meaning |
|---|---|---|---|
| `schema` | number | 1 | The file's version; a newer one than the launcher's is read but never written |
| `disc.path` | string | none | The disc image (`.cue` or `.bin`); the launcher stores it absolute |
| `disc.sha1` | string | none | The SHA-1 the launcher verified for that path (the game checks the disc again itself) |
| `video.scale` | 1-16 | 2 | The window is 320*scale x 240*scale |
| `video.fullscreen` | bool | false | |
| `video.refresh` | 50 or 60 | 50 | PAL, or the game's own 60 Hz mode (docs/LAUNCHER.md "50/60 Hz") |
| `audio.mute` | bool | false | |
| `memcard1`, `memcard2` | string or null | `"card1.mcd"`, `"card2.mcd"` | The memory cards (created by the game when missing); null: no card |
| `input.keyboard.<button>` | name or list | the game's | Keys (SDL3 scancode names) for a PS1 button |
| `input.gamepad.<button>` | name or list | the game's | Gamepad inputs (docs/LAUNCHER.md "Input bindings": `south`, `dpup`, `lefty-`, ...) |
| `input.hotkeys.<action>` | binding | `pause` P, `fullscreen` F11 | A string or a list of triggers; a trigger is an input or a chord (a list); pads as `"pad:<name>"` |
| `launcher.last_dir` | string | none | The launcher's own state: where the file dialog opens. The game never reads `launcher` |

**Only the bindings the user changed are written.** A button or action absent from `input` keeps the game's default
(`src/input.cpp` mirrors `port/src/input.c`'s defaults for display), so a default changed in a later game reaches
everyone who never rebound it; **Default** removes the entry again, and an `input` object the launcher emptied is
removed.

**Paths in the file are relative to the file's directory** (or absolute). The file is written only when its text
changes, through a temporary file and a rename. An unreadable file is reported, the defaults are used, and the first
save keeps the old file as `settings.json.broken`. A value of the wrong type or out of range is reported and its
default used.

## The disc

The Disc screen takes the image three ways: **Choose a file...** (`SDL_ShowOpenFileDialog`: the XDG portal, else
`zenity`, on Linux; when neither is there it says so), **dropping** the `.cue` or `.bin` on the window, or **typing**
its path. The launcher reads the `.cue` itself (the first `FILE` line, relative to the cue: the rules of
`port/src/disc.c`) and hashes the whole BIN with `port/src/sha1.c` on a worker thread (~4 s warm, longer from a cold
disk), with a progress bar and Cancel. Only the European disc (SHA-1 `457cb233...`) is stored, with its SHA-1; a wrong
file is refused and the previous disc stays. A stored disc is not hashed again while its SHA-1 matches and its BIN has
the expected size (692,146,560 bytes); a disc set by hand without a SHA-1 is checked at start.

## Starting the game

**Play** is enabled when the disc is verified and the game is found: `--game PATH`, else `$DW3_GAME`, else `dw2003`
beside the launcher, else the development tree's SDL build (`build/port-sdl/dw2003` beside `build/launcher/`). Play
saves the settings, runs `dw2003 --config FILE --print-settings` (the game's own check of the file), then starts
`dw2003 --config FILE --crash-dir <dir>/crashes` in the settings directory and hides the launcher's window. The
game's output (stdout and stderr) is read without blocking, copied to the launcher's stderr, streamed whole to
`<dir>/logs/last-run.log` (the previous run's kept as `last-run.1.log`), and its last 200 lines kept. When the game
ends the window comes back; on an error status it shows the status in words (1 a fatal error, 4 the watchdog, 64 bad
settings, a signal), the crash report the game wrote (`docs/LAUNCHER.md` "Crash report") and the last 40 lines, with
**Copy** (the launcher's and the game's versions, the command, the result, the report's text, the last lines) and
**Open folder**. Closing the launcher does not end a running game.

A game build from before `--config` or `--crash-dir` (its usage, exit 64, at the probe) is reported as too old, with how
to rebuild it.

## Settings and controls

**Settings:** the window's scale (1-16, with its size), fullscreen, 50 or 60 Hz (60 is the game's own 60 Hz mode; a game
built before that mode logs it and runs at 50), mute, the two memory card slots (a file name relative to the
settings directory, or no card), and the settings file's location and state.

**Controls**, three tabs: **Keyboard** and **Gamepad** list the 14 PS1 buttons with their inputs as chips (x removes
one, + adds one, Default restores the game's); **Hotkeys** has the port's actions (pause, fullscreen) with their
bindings. **+** opens a prompt that takes the next key (Keyboard), gamepad button, trigger or stick direction past
half way (Gamepad), or, for a hotkey, any of them or a chord (everything held together until all are released; at
most 4 inputs, and 8 triggers a binding: the game's limits). Escape alone, or Cancel, closes it. The prompt takes its
events before ImGui, and gamepad navigation pauses until the gamepad is released, so the press that was bound does
not also click a button. An input used by two buttons (or a button and a single-input hotkey) is marked, with a
tooltip naming the other use. A mod's own bindings are its options (Mods, below).

## Mods

The launcher lists the manifests beside the game, `<game dir>/mods/<id>/mod.json` (the port's CMake copies
`port/mods/` there), sorted by name. Each mod has an on/off switch and a page generated from its manifest: its name,
version and description, then its options, the ungrouped ones first and then each `group` under its title. One widget
per type: `bool` a checkbox; `int` and `float` a slider between `min` and `max` (a number field with `step` when
either is missing; floats are rounded to `step`); `enum` a list of the values' labels; `binding` the chips and the
prompt of the hotkeys (any key, gamepad input or chord). An option's `description` is its tooltip; `applies: restart`
is marked. A manifest that cannot be used (not JSON, another schema, an `id` that is not its directory's name, a bad
option, a `kind` other than `builtin` until data mods exist) is listed with the reason and cannot be switched on.

In `settings.json` (`mods.<id>`, docs/LAUNCHER.md "Mods section"): `"enabled": true` when on; an option is written only when it differs
from the manifest's default (setting it back, or **Default**, removes it); a mod turned off keeps its changed options
with `"enabled": false`; a mod with nothing left is removed, and so is an empty `mods`. Unknown mods and options are
kept. A stored value the manifest rejects is reported on the mod's page, and its default shown. The mods' bindings
take part in the conflict marks (a key both on a PS1 button and a mod's hold, say).

## Keys

The mouse, the keyboard (arrows, Space/Enter, Escape) and a gamepad (ImGui's navigation) all work. Ctrl+PageDown /
Ctrl+PageUp, or the gamepad's R1 / L1, go to the next / previous screen. A first run (no disc set, or the disc gone)
starts on the Disc screen.

## The self-test

```sh
SDL_VIDEO_DRIVER=offscreen build/launcher/dw2003-launcher --self-test DIR    # exit 0 = passed, 1 = failed
```

(`DW3_SELFTEST_VIDEO_DRIVER=offscreen` does the same through SDL's hint, for the Windows build under Wine, which drops
`SDL_VIDEO_DRIVER` from the program's environment: `scripts/build_windows.sh --test`.)

```sh
```

No disc and no display needed (CI runs it). It replaces `DIR/launcher-self-test/` and checks: the path helpers (Windows
forms too), the JSON writer, the lookup order, the settings file's round trips (the documented example with its unknown
members kept, invalid values, a broken file, a newer schema), the binding grammar and the input prompt (keys, gamepad buttons, stick directions, chords, Escape), the `.cue` reader
and the SHA-1 check, the launch path
with **the launcher itself as the game's stand-in** (`DW3_LAUNCHER_FAKE_GAME=mode`: a game with and without
`--config`, one that rejects the file, one that crashes, one that fails after 250 lines), then opens the window, walks
every screen with injected key events and a virtual gamepad, plays with the stand-in (a file of the BIN's size stands
for the verified disc), rebinds a key through the prompt and checks the file, drops a wrong file on the window, renders the mods screen over test manifests (path B's `fast_forward`, the documented
example, one with every option type, one with presets and a slider whose toggle makes it a typed number, three
unusable ones; the presets' and toggles' manifest errors) and rebinds a mod's key through the prompt, and saves a picture of each screen in
`DIR/launcher-self-test/screens/`.

Optional, with the data: `DW3_SELFTEST_DISC=iso/dw2003.cue` also checks the real disc;
`DW3_SELFTEST_GAME=build/port-sdl/dw2003` also probes the real game and, with the disc, runs it 300 frames from the
launcher's command (offscreen, unthrottled), which must end with status 0 and the disc checked. The file it probes has
rebinds, a chord, an unbound hotkey and every mod beside that game switched on with every option changed: the game's
own parser must accept it. CI runs both in its
data-gated part. `DW3_SELFTEST_GAME=beside` takes the game the launcher finds by itself beside its own executable
(it must be there, with at least one mod manifest beside it): the release's smoke test runs it inside the AppImage
(`docs/RELEASE.md`).

## Releases

The players' build is one AppImage with the launcher, the game and the mods (`scripts/package_appimage.sh`,
`.github/workflows/release.yml` on a tag `vX.Y.Z`, a draft release only): `docs/RELEASE.md`. Its launcher is built with
`-DCMAKE_BUILD_TYPE=Release -DDW3_LAUNCHER_STATIC_RUNTIME=ON` (libstdc++ and libgcc static) against an SDL3 with the
desktop backends (`scripts/setup.sh sdl3-desktop`).

## Files

| File | Contents |
|---|---|
| `src/main.cpp` | Options, the main loop |
| `src/app.cpp`, `app.h` | The window, the style, the screens |
| `src/settings.cpp`, `settings.h` | The settings directory's lookup, the settings file |
| `src/json_value.cpp`, `json_value.h` | An editable JSON tree over the port's reader, and a writer |
| `src/disc.cpp`, `disc.h` | The `.cue` reader and the SHA-1 check on a thread |
| `src/game.cpp`, `game.h` | Finding the game, its `--print-settings` probe, the command, the running game |
| `src/input.cpp`, `input.h` | The PS1 buttons, the game's default bindings, the binding grammar, the input prompt |
| `src/mods.cpp`, `mods.h` | The manifests (reading, checking), the user's values in `mods.<id>` |
| `src/paths.cpp`, `paths.h` | Paths and files through SDL's calls only (no POSIX; `/` and `\` and drive letters alike) |
| `src/selftest.cpp`, `selftest.h` | The self-test |
