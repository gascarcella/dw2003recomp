# The launcher

`dw2003-launcher` finds the user's settings, edits `settings.json` and starts the PC port. The plan and the decisions
behind it are in `docs/LAUNCHER_MODS_PLAN.md` (sections 4.1-4.4 are the contract with the game; 4.6 is this program)
and DECISIONS "Launcher and mods (session 18)". It is C++17 with Dear ImGui on SDL3 + `SDL_Renderer`; the game stays C.

**Status (phase 1 of 4):** the window and its navigation, the settings directory, reading and writing
`settings.json`, the self-test. The disc screen and the play button, the settings and controls screens, and the mods
screen follow.

## Build

```sh
scripts/setup.sh sdl3 imgui        # SDL3 (static) and Dear ImGui at their pinned versions, into tools/
cmake -S launcher -B build/launcher -G Ninja && cmake --build build/launcher
build/launcher/dw2003-launcher [--config-dir DIR]
```

Its own CMake project beside `port/`: the game's build and tests do not depend on it. SDL3 comes from `tools/sdl3`
(or `SDL3_DIR`), Dear ImGui from `tools/imgui` (or `-DDW3_IMGUI_DIR=...`); in a worktree whose `tools/` has no link
to them yet, the main checkout's are used. For a desktop window, SDL3 needs the X11/Wayland `-dev` headers at its
build (`scripts/setup.sh` "sdl3"); without them only the offscreen driver exists (the self-test's).

Two files of the port's runtime are compiled into the launcher as they are, never changed for it:
`port/src/json.c` (the strict JSON reader) and `port/src/sha1.c`.

## The settings directory (plan 4.2)

The first of these that applies:

1. `--config-dir DIR`, else `$DW3_CONFIG_DIR` (relative to the current directory; created at the first save).
2. A file `portable.txt` beside the launcher: the launcher's own directory (portable mode).
3. A `settings.json` that already exists in the current directory (for local testing; never created there by itself).
4. The per-user directory, `SDL_GetPrefPath("", "dw2003")`: `~/.local/share/dw2003/` on Linux (`$XDG_DATA_HOME`
   when set), `%APPDATA%\dw2003\` on Windows.

The status bar and the Settings screen show the directory and which rule chose it. The directory holds
`settings.json`, the memory card (`card1.mcd`) and, later, `mods/`.

## settings.json (plan 4.3)

Schema 1. The launcher reads and writes these members; **every other member is kept as it was** (order included), so
the game's side can add keys the launcher does not know yet:

| Member | Type | Default | Meaning |
|---|---|---|---|
| `schema` | number | 1 | The file's version; a newer one than the launcher's is read but never written |
| `disc.path` | string | `""` | The disc image (`.cue` or `.bin`) |
| `disc.sha1` | string | `""` | The SHA-1 the launcher verified for that path |
| `video.scale` | 1-16 | 3 | The window is 320*scale x 240*scale |
| `video.fullscreen` | bool | false | |
| `video.refresh` | 50 or 60 | 50 | PAL, or the game's own 60 Hz mode (plan 5.5) |
| `audio.mute` | bool | false | |
| `memcard1` | string | `"card1.mcd"` | Memory card 1 |

**Paths in the file are relative to the file's directory** (or absolute). The file is written only when its text
changes, through a temporary file and a rename. An unreadable file is reported, the defaults are used, and the first
save keeps the old file as `settings.json.broken`. A value of the wrong type or out of range is reported and its
default used.

## Keys

The mouse, the keyboard (arrows, Space/Enter, Escape) and a gamepad (ImGui's navigation) all work. Ctrl+PageDown /
Ctrl+PageUp, or the gamepad's R1 / L1, go to the next / previous screen. A first run (no disc set, or the disc gone)
starts on the Disc screen.

## The self-test

```sh
SDL_VIDEO_DRIVER=offscreen build/launcher/dw2003-launcher --self-test DIR    # exit 0 = passed, 1 = failed
```

No disc and no display needed (CI runs it). It replaces `DIR/launcher-self-test/` and checks: the path helpers (Windows
forms too), the JSON writer, the lookup order, the settings file's round trips (the plan's sample with its unknown
members kept, invalid values, a broken file, a newer schema), then opens the window and walks every screen with injected
key events and a virtual gamepad, saving a picture of each in `DIR/launcher-self-test/screens/`.

## Files

| File | Contents |
|---|---|
| `src/main.cpp` | Options, the main loop |
| `src/app.cpp`, `app.h` | The window, the style, the screens |
| `src/settings.cpp`, `settings.h` | The settings directory's lookup, the settings file |
| `src/json_value.cpp`, `json_value.h` | An editable JSON tree over the port's reader, and a writer |
| `src/paths.cpp`, `paths.h` | Paths and files through SDL's calls only (no POSIX: Windows comes later) |
| `src/selftest.cpp`, `selftest.h` | The self-test |
