# The launcher and the mods: this game's part

The launcher is psxstack's (`psxstack/launcher/`), built for this game from `port/game/game.json`
(`cmake -S psxstack/launcher -B build/launcher -G Ninja -DPSXSTACK_GAME_JSON=$PWD/port/game/game.json
-DPSXSTACK_VERSION_ROOT=$PWD -DPSXSTACK_TOOLS_DIR=$PWD/tools`; `build/launcher/dw2003-launcher`). The contract between
the launcher and the game, the settings directory and file (schema 1), the mod manifest and runtime, the launcher's
screens and self-test, and the crash report are psxstack's `docs/LAUNCHER.md` and `launcher/README.md`. This page
keeps what is this game's: its built-in mods and its 60 Hz mode. The decisions behind the design: DECISIONS
"Launcher and mods (session 18)".

Code cites this file as `docs/LAUNCHER.md "<heading>"`; keep the headings stable.

## The game's side of the contract
- `dw2003 --config FILE` and `--print-settings` are psxstack's runtime (`psxstack/runtime/settings.c`, `input.c`,
  `mods.c`); `tests/port/settings.py` checks the round trip, the defaults, the overrides and the errors against this
  game's build, and that every manifest under `psxstack/mods/` and `port/mods/` equals the game's registry
  (`--print-mods`).
- The disc check is the game's description's: `port/game/game.json` names the unpatched EU disc (SLES-03936) by its
  SHA-1 and size; the launcher and the game both check against it.
- The environment variables keep their names (`DW3_CONFIG_DIR`, `DW3_GAME`, `DW3_SELFTEST_*`, `DW3_PORT_*`): the
  description's `env_prefix` is `DW3`.
- The self-test with the real disc and game: `DW3_SELFTEST_DISC=$PWD/iso/dw2003.cue
  DW3_SELFTEST_GAME=$PWD/build/port-sdl/dw2003 SDL_VIDEO_DRIVER=offscreen build/launcher/dw2003-launcher --self-test DIR`.

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
`port_battle_cut` (`port/game/game_mods.c`), which scans the stream (`port/game/battle_scan.c`) and returns:

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
(`GsRecord`, `port/game/game_mods.c`) into the slot's unused tail (slot offset 0x26C4, 0x28 bytes, written to the card with
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
members' panels) asks `port_party_xp_share` (`port/game/game_mods.c`) for each member's experience:

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
experience"). With the mod on, STFGTREP's report passes each through `port_xp_boost` (`port/game/game_mods.c`), which
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
