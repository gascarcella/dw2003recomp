# The launcher, the settings file and the mods

How the launcher (`launcher/`) and the PC port (`port/`) talk to each other, the settings file they share, the mod
manifests, and what each built-in mod does. Building and running the launcher, its screens and its self-test:
`launcher/README.md`. The port's options and runtime: `port/README.md`. The decisions behind this design: DECISIONS
"Launcher and mods (session 18)".

Code cites this file as `docs/LAUNCHER.md "<heading>"`; keep the headings stable.

## Contract

- The game takes **one option for it, `--config FILE`** (JSON, "Settings file"). Command-line options override the
  file.
- **The game never loads a configuration on its own.** Without `--config` it behaves as the bare binary always did:
  the tests and CI run it that way and must not depend on the machine.
- The launcher edits that file and starts `dw2003 --config <dir>/settings.json` in the settings directory
  (`SDL_CreateProcess`). Before that it runs `dw2003 --config FILE --print-settings`, the game's own check of the file.
  The game stays startable without the launcher (`--config`, or plain options).
- A game started with `--config` has, by default: a window, memory card files beside the settings file, and the
  watchdog off.
- The disc's SHA-1 check stays the game's (`port/src/disc.c`, `sha1.c`). The launcher compiles `port/src/sha1.c` and
  `port/src/json.c` in as they are, to verify the disc before storing it and to read JSON with the same parser.
- Exit statuses the launcher reports: 0 normal, 1 a fatal error, 4 the watchdog, 64 bad options or settings.
- The launcher's screens use the same SDL3 + `SDL_Renderer` as the game, so they could later be drawn inside the
  game's window.

## Settings directory

The launcher picks one directory, the first of these that applies (`launcher/src/settings.cpp`):

1. `--config-dir DIR`, else `$DW3_CONFIG_DIR` (relative to the current directory; created at the first save).
2. A file `portable.txt` beside the launcher's executable: that directory (portable mode).
3. A `settings.json` that already exists in the current directory (local testing; never created there implicitly).
4. The per-user directory, `SDL_GetPrefPath("", "dw2003")`: `~/.local/share/dw2003/` on Linux (`$XDG_DATA_HOME` when
   set), `%APPDATA%\dw2003\` on Windows.

The directory holds `settings.json`, the memory cards and (later) `mods/`. The launcher shows which directory it chose
and why. The game itself never looks for a settings directory: it reads only the file `--config` names.

## Settings file

Schema 1, read by `port/src/settings.c` (the top level), `port/src/input.c` (`input`) and `port/src/mods.c` (`mods`).
`dw2003 --config FILE --print-settings` prints the effective settings (the file, then the command line) with every key
and absolute paths and exits 0, or exits 64 naming the bad key. `tests/port/settings.py` checks the round trip, the
defaults, the overrides and the errors.

```json
{
  "schema": 1,
  "disc": { "path": "dw2003.cue", "sha1": "457cb233..." },
  "video": { "window": true, "scale": 2, "fullscreen": false, "refresh": 50 },
  "audio": { "mute": false },
  "memcard1": "card1.mcd",
  "memcard2": "card2.mcd",
  "watchdog": 0,
  "input": {
    "keyboard": { "cross": "X", "start": ["Return", "Keypad Enter"] },
    "gamepad":  { "cross": "south", "up": ["dpup", "lefty-"] },
    "hotkeys":  { "pause": "P", "fullscreen": "F11" }
  },
  "mods": {
    "fast_forward":   { "enabled": true, "hold": "Tab" },
    "skip_dialogues": { "enabled": false, "toggle": "F2", "fast_forward_waits": false }
  },
  "launcher": { }
}
```

### General rules

- **Every key but `schema` is optional;** an absent key keeps its default. `schema` must be 1: a missing one, or any
  other number, ends the game with status 64 (a higher one is named "a newer launcher's").
- **Paths** (`disc.path`, `memcard1`, `memcard2`) are relative to the settings file's directory, or absolute.
- **Errors:** a value of the wrong type or out of range ends the game with status 64 and a message naming the key
  (`port: settings FILE: video.scale: an integer from 1 to 16, not 17`), as a bad option does. **An unknown key is
  logged and ignored** (at every level: top, `disc`, `video`, `audio`, `input`, a PS1 button, a hotkey action, a mod,
  a mod's option), so an older game runs a newer launcher's file.
- **The command line overrides the file:** `--disc`, `--scale`, `--fullscreen`, `--window`, `--mute`,
  `--memcard1|2 PATH|none`, `--watchdog`, `--refresh`, `--fps`.

### Members

| Member | Type | Default | Meaning |
|---|---|---|---|
| `schema` | number | required | Must be 1 |
| `disc.path` | string | none | The disc image (`.cue` or `.bin`). The launcher stores it absolute |
| `disc.sha1` | string | none | The launcher's record of the SHA-1 it verified. The game does not trust it: it checks the disc itself (`disc.c`, with its stamp cache) |
| `video.window` | bool | true | false runs headless (tests, a check run) |
| `video.scale` | integer 1-16 | 2 | The window is 320*scale x 240*scale |
| `video.fullscreen` | bool | false | |
| `video.refresh` | 50 or 60 | 50 | PAL, or the game's own 60 Hz mode ("50/60 Hz") |
| `audio.mute` | bool | false | No audio device |
| `memcard1`, `memcard2` | string or `null` | `"card1.mcd"`, `"card2.mcd"` | A `.mcd` image, created formatted when missing; `null`: no card in that slot. An empty string is an error |
| `watchdog` | integer 0-3600 | 0 | Seconds without a vsync before the game exits 4; 0 is off. The bare binary (no `--config`) keeps its 10 s |
| `input` | object | the game's | "Input bindings" |
| `mods` | object | every mod off | "Mods section" |
| `launcher` | object | none | The launcher's own state (e.g. `last_dir`, where its file dialog opens). The game never reads it and `--print-settings` prints it back unchanged |

### Input bindings

`input` has three members; `--print-settings` prints it back as it was given.

- **`keyboard`**: PS1 button -> one key name or a list of them. **`gamepad`**: PS1 button -> one gamepad input name
  or a list. A button named replaces all of that button's entries of that kind; a button that is absent keeps its
  defaults; `""` or `[]` unbinds it. Buttons: `up down left right cross circle square triangle start select l1 r1 l2
  r2`. An unknown button is logged and ignored.
- **Key names are SDL3's scancode names** (`SDL_GetScancodeName`/`SDL_GetScancodeFromName`: `"X"`, `"Return"`,
  `"Keypad Enter"`, `"Right Shift"`, `"F11"`, `"Tab"`). Scancodes name the key's place, whatever the layout. A
  headless build (no SDL) checks only that a name is non-empty.
- **Gamepad input names are ours** (SDL's positional buttons): `south east west north back guide start leftstick
  rightstick leftshoulder rightshoulder dpup dpdown dpleft dpright misc1 paddle1 paddle2 paddle3 paddle4 touchpad`,
  and the axes past half way: `lefttrigger righttrigger leftx- leftx+ lefty- lefty+ rightx- rightx+ righty- righty+`
  (`lefty-` is the left stick up). Every connected gamepad is ORed into pad 1.
- **Default map** (`port/src/input.c`): arrows the D-pad, `X` cross, `C` circle, `Z` square, `S` triangle, `Return`
  and `Keypad Enter` START, `Backspace` and `Right Shift` SELECT, `Q`/`E` L1/R1, `1`/`3` L2/R2; gamepads: south
  cross, east circle, west square, north triangle, back SELECT, start START, the shoulders L1/R1, the triggers L2/R2,
  the D-pad and the left stick the D-pad. The launcher mirrors these defaults for display (`launcher/src/input.cpp`).
- **`hotkeys`**: the port's own actions -> a binding. Actions: `pause` (default `"P"`), `fullscreen` (default
  `"F11"`). A mod's bindings are its options, under `mods.<id>`, not here.
- **A binding** is a string or a list; each element is one trigger, any of which fires the action (at most 8). A
  trigger is one input (a string) or a chord (a list of 1 to 4 inputs, all held). An input is a key name, or `"pad:"`
  + a gamepad input name. `""` or `[]`: unbound. Examples: `"F2"`; `["F2", "pad:guide"]`;
  `[["pad:guide", "pad:south"], "Tab"]`.
- **Hotkeys never reach the pad.** A trigger that completes latches until all its inputs are released, and its inputs
  are masked out of the pad while latched: a single-key hotkey is never seen by the game. The first input of a chord
  made only of game buttons does reach the pad until the chord is complete, so chords should start with an input the
  pad map does not use (`pad:guide`, the stick clicks).
- **The pause** (`port/src/pump.c`) takes effect at the end of the vsync where its key is pressed: the window keeps
  polling and presenting the last image, the audio device is paused, the watchdog is re-armed, and no vsync runs (so
  nothing reaches the game, the log or the record). The key again resumes, with the pace's schedule started over.
- With `--script` the script owns the pad: the keyboard and gamepads are read but never sent to the game (the hotkeys
  still act). `--input-test` tests the active map: the defaults without `--config`, the settings' with it.

### Mods section

`mods`: `<id>` -> `{ "enabled": bool, "<option id>": value }`, with the option ids and types of the mod's manifest
("Mod manifest").

- **A mod that is absent, or has no `enabled`, is off.** An absent option keeps the manifest's default.
- An unknown mod id ("no such mod in this build") is logged and ignored, an unknown option too. A value of the wrong
  type or range ends the game with status 64, as elsewhere.
- `--print-settings` prints `mods` resolved: every mod of the game with `enabled` and every option (defaults filled
  in), then the unknown mods as they were given.
- **Under `--script` every mod is off** unless `--script-mods` is given ("Mod runtime").
- The launcher writes `"enabled": true` for a mod switched on, writes an option only when it differs from the
  manifest's default, keeps a switched-off mod's changed options with `"enabled": false`, and keeps unknown mods and
  options.

## Mod manifest

Each mod has a manifest, `port/mods/<id>/mod.json`. The port's CMake copies them beside the binary
(`build/port*/mods/<id>/mod.json`), where the launcher lists them; later the launcher will also list the settings
directory's `mods/`. The user-facing text (names, descriptions, labels) lives in the manifests only; the user's values
live in the settings file, never in the manifest.

**Top level:**

| Key | Required | Meaning |
|---|---|---|
| `schema` | yes | 1 |
| `id` | yes | Must equal the mod's directory name; the key under the settings' `mods` |
| `name` | yes | Shown in the launcher |
| `version` | no | A string |
| `description` | no | Shown on the mod's page |
| `kind` | yes | `builtin` (the only kind supported; `data` is reserved for data-override mods) |
| `requires_port` | no | Integer: the game's mod interface the mod needs (`PORT_MODS_API` in `port/src/mods.c`, 1 now) |
| `options` | no | A list of options |
| `presets` | no | A list of presets: `{ "id", "name", "description" (optional), "values": { "<option id>": value } }`. The launcher shows a button per preset (pressed while all its values are in place) that sets those values; the game never sees a preset, only the values |

**An option:** `id` (not `enabled`, unique in the mod), `name`, `description` (the launcher's tooltip), `type`,
`default`, and optionally `group` (a heading on the mod's page; ungrouped options come first) and `applies` (`live` or
`restart`; `restart` is marked in the launcher). Types:

| Type | Extra keys | Value in the settings |
|---|---|---|
| `bool` | | `true`/`false` |
| `int`, `float` | `min`, `max`, `step`; `slider_max` with `input_toggle` | A number in range (an integer for `int`) |
| `enum` | `values`: `[{ "id", "label" }]` | One of the value ids |
| `binding` | | The binding grammar of "Input bindings"; the default written the same way |

There is no `string` or `path` type. An `int` or `float` with `min` and `max` is a slider, without them a number field.
**`slider_max` and `input_toggle`** (together, on an `int` or `float` with `min` and `max`): `input_toggle` names a
`bool` option of the same mod; while it is off the option is a slider from `min` to `slider_max` (above `min`, at most
`max`), and a larger value in the settings counts as `slider_max` (the game logs it and uses `slider_max`; the file
keeps the value); while it is on the option is a typed number from `min` to `max`. A preset's value above
`slider_max` must come with the toggle set to `true` in the same preset. Example:

```json
{
  "schema": 1, "id": "skip_dialogues", "name": "Skip dialogues", "version": "0.1", "kind": "builtin",
  "requires_port": 1,
  "description": "Text appears at once and advances by itself. Choices, menus and name entry still wait for you.",
  "options": [
    { "id": "toggle", "name": "Toggle", "type": "binding", "default": "F2", "group": "Controls", "applies": "live" },
    { "id": "hold",   "name": "Hold",   "type": "binding", "default": "",   "group": "Controls", "applies": "live" },
    { "id": "fast_forward_waits", "name": "Also fast-forward cutscene waits", "type": "bool", "default": false,
      "applies": "live" }
  ]
}
```

The built-in mods use the same schema as later data mods, so one renderer in the launcher serves both. A manifest the
launcher cannot use (not JSON, another schema, an `id` that is not its directory's name, a bad option or preset, a
`kind` other than `builtin`) is listed with the reason and cannot be switched on.

## Mod runtime

`port/src/mods.c` holds the registry of built-in mods (ids, versions, option types, defaults, ranges), reads their
values from `mods.<id>`, registers each enabled mod's `binding` options as hotkey actions (named `<id>.<option>`), and
runs `port_mods_frame()` from `port_frame` at every vsync. `dw2003 --print-mods` prints the registry as JSON;
`tests/port/settings.py` requires every `port/mods/<id>/mod.json` to agree with it.

**Rules for a mod that changes the game's behaviour:**

1. Its hook in the game's C sits in an `#ifdef PC_PORT` block that tests a `port_mod_*` flag (declared in
   `include/port.h`). Never `if (MACRO_THAT_IS_0)` or `|| MACRO`: an expression that is constant on the PS1 can change
   the old GCC's code. `scripts/build.sh --check` gates every hook.
2. **Mods are off under `--script`** unless the run passes `--script-mods`, so the replays, goldens and records stay
   the bare binary's.
3. A hotkey never reaches the pad: no `I` line in the frame log, no change to a record.
4. A mod's own state lives in the runtime's variables. A game global a mod must set is set before
   `port_overlay_init()` (the reset's snapshot); a later write breaks the reset check and is undone by a reset.
5. A run with a mod on needs its own expected results: the random generator steps once a frame, so any skipping
   shifts later rolls (as a faster player would). `tests/port/mods.py` holds those runs.

**The pace split** (`port/src/pump.c`): `port_rate` is the nominal rate (50 or 60: the vsyncs per second the game is
made for, which sets the audio's samples per vsync); the pace (`port_pace_set`; 0 = unthrottled) is the wall clock's
vsyncs per second. Every change of the pace starts the schedule over, so lowering it does not stall the game.
`--fps N` sets both.

## Launcher

`dw2003-launcher` (`launcher/`) is C++17 with Dear ImGui on SDL3 + `SDL_Renderer`; it is its own CMake project and the
game stays C. It finds the settings directory, edits `settings.json` (keeping every member it does not edit, unknown
ones included), and starts the game. Screens: **Disc** (file dialog, drag-and-drop, or a typed path; only the EU disc's
SHA-1 is accepted), **Play** (starts the game; on an error shows the exit status and the last lines of output),
**Settings** (scale, fullscreen, 50/60 Hz, mute, memory cards), **Controls** (keyboard, gamepad, hotkeys), **Mods**
(an on/off switch per mod and a page generated from its manifest). Build, run, details and the self-test:
`launcher/README.md`.

## Built-in mods

### Fast-forward

`fast_forward` (runtime only, no game C). Options: `hold` (binding, default `Tab`), `toggle` (binding, unbound),
`speed` (enum `2x 3x 4x 6x 8x unlimited`, default `4x`), `mute` (bool, default true).

While it is on (the hold binding held, or after the toggle) the pace is the nominal rate times the speed (`4x`: 200
vsyncs a second at 50 Hz, 240 at 60; `unlimited`: no pace), the window presents at most 60 images a second (every vsync
is still drawn), and with `mute` the audio device's queue is cleared and nothing is queued (the SPU keeps rendering,
since LIBSND reads its envelopes; a `--wav` is unchanged). The window's title says it is on. The game, its log and its
record are those of an unthrottled run, which are byte-identical to a paced run's.

Test hook: `DW3_PORT_FAST_FORWARD=ON:OFF` (only while the mod is enabled) turns it on for ON vsyncs and off for OFF,
repeating (`tests/port/settings.py`).

### Skip dialogues

`skip_dialogues`. Options: `toggle` (binding, default `F2`), `hold` (binding, unbound), `fast_forward_waits` (bool,
default false). While it is on, `port_mod_skip_dialogues` is 1 and three hooks act:

- `message_window_update` (`src/main/message.c`), RUN case: a revealing window (`speed > 0`, not done) shows its whole
  page at once (`message_find_page_end`). This covers every message window, the overlays' too.
- Its DONE step 1: a wait for button 13 (confirm) goes on by itself. Every wait in the disc's text tables uses that
  button; any other wait is left to the player.
- `fightstg_message_step` step 3 (`src/fightstg/fightstg_8008D3B4.c`): the battle's message wait goes on as a press
  would (without the press's sound).

No pad press is synthesised, so choices, menus, name entry, shops, the inn and the save prompts still wait, and a
closed dialogue does not talk to the NPC again. Not covered: the code-driven prompts of the card game, the status
screen and other overlays.

With `fast_forward_waits`, the mod also asks for fast-forward (with `fast_forward`'s speed and mute, or their defaults
when that mod is off) while FIELDSTG is the current overlay and `fieldstg_stage.event_running` is set: a cutscene's
scripted waits, walks and bubble animations. Free walking is not sped up.

Test hook: `DW3_PORT_SKIP_DIALOGUES=1` (only while the mod is enabled) starts it on. `tests/port/mods.py` runs
`first_battle_save`'s route with no press through the scenes.

### Disable battle animations

`battle_animations`. Option: `hit_reaction` (bool, default true). No hotkey: it acts whenever enabled
(`port_mod_battle_animations`).

The battle's rules (every roll, damage, HP) run before an action's animation script starts; the scripts only present
them. The hook sits in `fightstg_script_update`'s INIT step 0 (`src/fightstg/fightstg_8008B630.c`), after the script's
stream is found and before script 12's sound-bank load. For scripts 5 and up (attacks, techniques, items) it asks
`port_battle_cut` (`port/src/mods.c`), which scans the stream (`port/src/battle_scan.c`) and returns:

- **-1, run it:** a stream that does not scan to its end (none on the disc).
- **1-4, the target's reaction** (`results[3] + 1`: flinch, heavy hit, KO, dodge) for a script with a child command
  when `hit_reaction` is on. The hook restarts the same object as that reaction (side flipped; results, stage, effect
  and hit sound zeroed; step 0 again), as the child command would have.
- **0, end it at once:** a script with no child command, or every script when `hit_reaction` is off, except that **a
  knock-out keeps its KO reaction** so the model ends on its KO pose.

In each case it plays the script's first hit sound (`fightstg_script_run_sound`'s choice). Reaction scripts 1-4 created
directly (the defeat camera, `fightstg_player_reaction`) and the scenes outside the scripts (entrances,
digivolutions, the intro and defeat cameras) are left alone: they carry functional writes. `tests/port/battle.py`
checks the scanner against every script on the disc; `tests/port/mods.py` runs the first story battle with the mod on.

### Save anywhere

`global_save`. Option: `restore_map_state` (bool, default true). No hotkey: it acts whenever enabled
(`port_mod_global_save`).

The game only saves at an inn: the inn's event script changes the map to STGMCARD's 0xC01..0xC19 (the inn's name for
the slot summary) with entry -1 (save mode), and Back returns to the previous map. With the mod on, the field menu
(START; `src/main/fieldmenu.c`) gets a last entry, **SAVE** (the save screen's title, `?SMEMCRD` entry 1, so it is in
every language), in the field's menu only (not in the status screen's copy of the menu). Chosen, it makes the same map
change (`port_global_save_open`: the inn's 0xC map when the map is an inn's, else 0xC00), so STGMCARD, its card
prompts and its Back work as at an inn; the slot summary shows the area (`stgmcard_map_areas` covers every field map)
and, off an inn, no place name. Six entries use the bank's 6-entry panel; the seventh (with the card case) stretches
it by one row.

A load returns to the saved map at the saved position but as a fresh entry (`map_is_new`): the attribute layer, the
player's depth and height, the per-visit flags and a few more fields after the slot's 0x26C4 bytes are reset, which is
right at an inn but can put the player on the wrong floor of a multi-layer map. So the save also writes those
(`GsRecord`, `port/src/mods.c`) into the slot's unused tail (slot offset 0x26C4, 0x28 bytes, written to the card with
the slot and outside the game's checksum: a PS1 or an emulator ignores it and loads the save as a fresh entry), and a
load with `restore_map_state` puts them back and marks the map as revisited (`field_last_map`), so FIELDSTG resumes it
as after a Back from the save screen. A save without the record (an inn save of the plain game) loads as before. With
the mod off, the binary is the plain game: the menu has no SAVE and a load restores nothing.

`tests/port/mods.py` saves from the lab (map 0x206), returns, reboots and loads, with `restore_map_state` on and off,
and with the mod off; `tests/saves/run.py` loads the card it writes in the emulator.

### Preset language

`preset_language`. Option: `language` (enum `english french german italian spanish usa japanese`, default `english`,
`applies: restart`). No hotkey: it acts at boot whenever enabled (`port_mod_preset_language`).

The game boots into CNTY_SEL (map 0x1600), whose menu sets `records_language` (the offset of every localized text
file ID: 0 JPN, 1 USA, 2 ENG, 3 FRA, 4 ITA, 5 GER, 6 SPN) and goes on to the opening, map 0xE02 (0xE01 for Japanese).
With the mod on, CNTY_SEL's root object (`src/cnty_sel/cnty_sel_80082CE8.c`), once the screen's sound bank is in, sets
`records_language` to the option's code and makes that map change itself: the screen's image, menu, music and fade
are never loaded. Nothing else on the EU disc reads the choice; the title's New Game builds the new game's records
(`gamestate_init_records`) in that language. A reset goes through CNTY_SEL again, so the language stays. The screen
offers only the five EU languages, but the USA and Japanese text sets are on the disc too (`docs/DISC_LAYOUT.md`) and
the EU code handles them: Japanese takes the opening's 0xE01 path, with the Japanese release's intro art (both
play-tested to the memory card screen).

`tests/port/mods.py` boots each language (the opening's map and `records_language`, and the mod off still at the
screen) and runs `new_game`'s route from the opening in French, whose new game has the French deck names.

### Party experience

`party_xp`. Options: `share` (int 0-100, step 5, default 50), `knocked_out` (bool, default false), `catch_up` (bool,
default false). No hotkey: it acts whenever enabled (`port_mod_party_xp`).

The game splits a won battle's experience among the party members that took part (one fighter: all of it, two: 60%
each, three: a third each) and gives the others nothing, knocked-out members included (`docs/MECHANICS.md` section 6,
"Who gets a battle's experience"). With the mod on, STFGTREP's report (`stfgtrep_main_update`, after it creates the
members' panels) asks `port_party_xp_share` (`port/src/mods.c`) for each member's experience:

- **A member that took part** keeps the game's share.
- **A member that did not** gets `share` percent of one fighter's share (at least 1 when both are above 0). A member at
  0 HP when the battle ends (WFIGHTMN tells `port_party_xp_knocked_out` before it clears the member's `took_part`)
  gets it only with `knocked_out`.
- **`catch_up`:** a member below the party's highest level gets 10% more per level below it, at most twice as much;
  fighters included.

The report then shows, counts and adds that experience as it does a fighter's: its panel, the level-ups and their stat
draws, a new form, and item 0x141's fifth more. Form experience still goes only to the forms that fought, and only
the three party members gain (the Digimon at the lab have no panel). The money, the item and the battle are
unchanged. `tests/port/mods.py` runs the first story battle (one fighter of three) with the mod on and off; catch-up
and knocked-out members are not reached by a scripted run.

### XP boost

`xp_boost`. Options: `exp`, `form_exp`, `bits` (float 1-10, step 0.1, default 2; sliders up to 5, `slider_max` with
`input_toggle: manual`), `manual` (bool, default false: values above 5x count as 5x). Presets (launcher only): Boost
(all 2x, the defaults), Turbo (all 3x), Ultra (all 5x). No hotkey: it acts whenever enabled (`port_mod_xp_boost`).

A won battle's rewards come from one row of `stfgtrep_rewards` (`docs/MECHANICS.md` section 6, "Who gets a battle's
experience"). With the mod on, STFGTREP's report passes each through `port_xp_boost` (`port/src/mods.c`), which
multiplies it by the option's value (in tenths, rounded down, at most 9,999,999):

- **`exp`**: one fighter's split share, in `stfgtrep_main_update` before the members' panels are created; party_xp's
  shares for the members that did not fight come from the boosted share, and item 0x141's fifth comes on top.
- **`form_exp`**: a chosen form's experience, at the end of `stfgtrep_get_technique_exp`, after the game's cap (10 a
  battle below the form's threshold level, 50 past it), so a boosted form still gains more than the cap.
- **`bits`**: the money in `stfgtrep_main_run`'s step 20, item 0x142's fifth included; `gamestate_data.money` still
  stops at 9,999,999.

The report shows, counts and adds the boosted amounts as the game's own (the panels, the level-ups, the message).
The item won and the battle are unchanged. `tests/port/mods.py` runs the first story battle (4 experience, 50 bits)
with the mod on, with `manual` and party_xp, and capped without `manual`; that battle gives no form experience, so
`form_exp` is not reached by a scripted run.

## 50/60 Hz

A setting, not a mod: `video.refresh: 60` or `--refresh 60` (default 50, PAL). It sets, before `port_overlay_init()`
(so the reset's snapshot holds it):

- `records_60hz = 1`, the flag the NTSC patch sets (`0x8005CCAC`). The game then runs its own 60 Hz mode: the video
  mode, the time step per vsync, actor speeds, the sound tick and the fade rate are compensated, as on a patched PS1.
- `port_rate` and the pace 60 (735 audio samples a vsync).
- `psyq_cd_set_vsync_hz(60)`: the drive's rate stays per second (2.5 sectors and 735 XA frames a tick; seeks keep their
  milliseconds).

`main_screen_pos` (the patch's second word, `0x8005CCB0`) stays 1 (PAL): it only adds the PAL screen offset, which the
port's video ignores, and picks the card game's PAL layout. `--fps 60` alone is not the 60 Hz mode: it speeds
everything up, music and the play-time clock included.

`tests/port/hz60.py` compares the port at 60 Hz with the patched game in the emulator (`tests/port/ntsc_patch.lua`
writes the two words at `main`'s entry; references in `tests/port/hz60/`).
