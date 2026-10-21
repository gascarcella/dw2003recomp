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
- Texture packs are data mods in the settings directory's `mods/` (psxstack `docs/LAUNCHER.md` "Data mods"; the
  launcher passes `--mods-dir`); `tests/port/settings.py` checks `mod_order` and the data mods' settings, and
  `tests/port/textures.py` the packs found that way.
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

### Faster loading

`fast_loading` (runtime only, no game C; psxstack's `docs/RUNTIME.md` "Faster loading"). Option: `speed` (enum
`2x 4x 8x instant`, default `4x`).

The emulated drive reads the game's files (`cdload`'s reads: the overlays, the maps, the sound banks) at that multiple
of a double-speed drive with its seeks shortened as much, or at once. The movies (`stdwtitl`'s stream, `CdRead2(0x1E0)`,
XA audio on) keep the drive's rate, so they and their sound play as without the mod; the music plays from sound RAM
and is never affected. A load ends in fewer frames, and the game's random generator steps once a frame, so later rolls
differ from a run without the mod; every read still completes through the game's callbacks.

On the global_save route (new game, the first battle, a save, a reset and Continue; 25,471 frames) the run ends 244,
1,080, 1,239 and 1,505 frames earlier at 2x, 4x, 8x and instant (boot to the language screen: 85 frames, then 51, 35,
27, 19). `--cd-speed instant` (the debug tools' default) times the data reads the same way and the movies at the
drive's rate too.

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

### Recover on level up

`level_up_recover`. No options, no hotkey: it acts whenever enabled (`port_mod_level_up_recover`; issue #50).

The game's level up (`stfgtrep_add_exp`, in the member's panel of STFGTREP's report: `docs/MECHANICS.md` section 6)
raises only max HP and max MP; the member keeps the HP and MP it ended the battle with. With the mod on, the panel
calls `port_level_up_recover` (`port/game/game_mods.c`) when `stfgtrep_add_exp` says the member went up (one level or
several in one battle), which sets the record's HP and MP to the new maximums. WFIGHTMN has already written the
battle's HP and MP back to the records, and nothing after the report writes them again, so the field gets the full
values. A member that did not fight levels up only with party_xp's share, and recovers then too. The level up's stat
draws, the report's text and the battle are unchanged.

`tests/port/mods.py` runs the first story battle with xp_boost's 10x (40 experience: member 0 goes from level 1 to 3)
with the mod on (HP 320/320, MP 207/207) and off (HP 93/320, MP 100/207).

### Widescreen battles

`widescreen`. No options, no hotkey: while enabled, the battle (FIGHTSTG) is shown 16:9 through the GPU renderer's wide
canvas (psxstack `runtime/render_gpu_wide.c`; issue #71). The game is unchanged: FIGHTSTG already sends the 3D a 16:9
frame needs (its only screen-space cull, `fightstg_model_mesh_is_visible`, keeps a mesh with a bounding point within
the clip plus 64 pixels: 224 from the centre, against the 213.5 of a 16:9 frame), and the GPU clipped it to the
320-wide drawing area. The mod's frame callback (`port/game/game_mods.c`) calls `port_video_widescreen(1)` while FIGHTSTG
is the tier-1 overlay; its start asks for the canvas and a 16:9 window (`port_video_widescreen_enable`).

- **What widens:** the battle's stage and models (more of the arena at both sides); the background clear, fades and
  flashes cover the whole width. The HUD, the menus and the portrait window stay as drawn, in the middle 4:3.
- **What stays 4:3:** the field, the menus and every other overlay (320-wide 2D whose art ends at the screen's edge;
  the field's widening is issue #78), the movies, and everything with the software renderer (logged once).
- **The window** keeps its size; the picture switches between 4:3 and 16:9 inside it. A new window opens 16:9
  (`video.scale` x 427 x 240) when the mod is on and `video.renderer` is `gpu`.
- **Known limits:** the digivolution's 2D effects are 320-wide art and may show their edges at the sides; a stage
  modelled only at the front (stage kind 4) may show its back at the sides. Both are for the play-test.

`tests/port/render_gpu.py` (a GPU device): a battle frame's middle 320 columns equal the software image byte for byte,
its sides are drawn, the present is 16:9, the field stays 4:3, the frame log equals the run without the mod.
`tests/port/mods.py` (headless): the frame log unchanged and the software path's 4:3 logged once.

### Disable random encounters

`random_encounters`. Option: `toggle` (binding, default none). While enabled it is on from the start; the toggle
turns random encounters back on (for grinding) and off again (`port_mod_random_encounters`; the window's title says
"no encounters" while it is on).

FIELDSTG rolls a random battle while the player moves on a map's encounter area: every step takes the area's rate off
`encounter_timer`, and when it runs out `fieldstg_encounter_step` starts a battle from the area's list and redraws
the timer (`docs/MECHANICS.md` section 1, "Random encounters"). With the mod on, the step logs the skip
(`port_random_encounter_skipped`, `port/game/game_mods.c`) and only redraws the timer, which is what the game does with
its debug switch `records_state.encounters` off. The timer still runs and its redraw still takes its draw, so the
mod changes nothing until the roll. The other battles do not go through that step and still start: the story's and
the trainers' (event scripts), the spots' and the meters' (`fieldstg_start_listed_battle`) and the map events' of
`fieldstg_start_battle_5`.

`tests/port/mods.py` walks map 0x21D (first_battle_save's route, then back and forth across its plaza) until the
timer runs out: without the mod a random battle starts; with it the walk completes, each skip is logged on map 0x21D,
and the frame log equals the mod-off run's up to the first skip; the route's first story battle (an event's) still
starts with the mod on. The toggle is a key and not reached by a script.

### Faster transitions

`fast_transitions`. Option: `speed` (enum `normal fast instant`, default `normal`). No hotkey; the speed is read every
vsync (`port_mod_fast_transitions`; the window's title says "fast transitions" or "instant transitions").

The game's own transition and save-screen animations take fewer frames: "fast" about half, "instant" about one frame
each, "normal" the game's own (nothing changes: the frame log and the record are the mod-off run's). It is not
fast-forward: the game runs at its normal pace and the music plays on untouched; only the animations' frame counts or
per-frame steps change, and each still reaches its end state through its own update and reports done the way the code
waits for. What it covers (`docs/MECHANICS.md` section 15 has the timings and the functions):

- every full-screen `Fade` (the copy in each overlay's `*_fade_start`, and the EXE's `inn_fade_start`): its `frames`
  through `port_transition_frames` (half rounded up, or 1). This is the fade out of the menus (status, shops, card
  menus, the Digimon lab, naming, training, the battle report, the save screen), the battle's exit (WFIGHTMN) and the
  inn's and the warp's fades;
- FIELDSTG's map change (`fieldstg_manager_leave_map`): more of the closing window's 10 x 7-pixel steps and the black
  window's 8-pixel steps a frame (`port_transition_extra`), landing on the same last values (window 0, fade 0x8000, 168);
- the map's title (`fieldstg_map_title_update`): the rectangles' speed doubled (or the whole screen), the texts typed
  every 3 frames ("instant": the page at once, as skip_dialogues shows it), the 60-tick hold halved (or none), the
  closing clip's steps as the map change's; the warp's 60-tick hold the same;
- the field menu's and the inn's windows (`include/window_anim.h`) and the field menu's picture before STATUS or SAVE
  (a palette step every tick instead of every 2; "instant": all of them in one frame);
- STGMCARD: its tweens (`stgmcard_start_value`: the title, the slots bar, the icons, the cursor), the contents panel
  (`stgmcard_start_fade`) and the bar's finish (`stgmcard_frame_open` mode 2). The bar's mode 1, which creeps while the
  card works, is left alone.

Never shortened: anything that waits on the memory card, the CD or a button (the card's commands, the loads, the
prompts), the inn's 240-tick hold (the inn's jingle plays through it), the warp's picture, the battle's shatter
transition and the battles' own scenes, CARDGAME, the movies, and the other overlays' and the event scripts' windows.
Saves are the game's own saves. As with skip_dialogues and battle_animations, the random generator steps once a frame,
so fewer frames move the later rolls (a battle can go differently); a run with the mod is deterministic with itself.

`tests/port/mods.py` runs `global_save.json` (a field-menu SAVE, STGMCARD's save, Back, a reset and Continue's load)
with global_save alone and with the mod at each speed: fast and instant complete with the slot's bytes unchanged from
the menu to the load; from the menu's opening to back on the field takes 795 frames off, 707 fast, 608 instant, and
map 0x203 to 0x204 754, 696, 640; normal's frame log and record equal the mod-off run's; instant twice gives the same
log and record.

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
