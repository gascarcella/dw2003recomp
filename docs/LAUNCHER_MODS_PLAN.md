# Launcher and mods plan

_Planned in session 18 (2026-10-06, a discovery and planning session). **Implemented so far (session 19, path B):**
`--config FILE` and the settings file, 4.3 (settled there). The user's
decisions are in section 2 and in `docs/DECISIONS.md` "Launcher and mods (session 18)". The facts in section 3 come from
five research agents: the measurements were run on the dev machine; everything about the game's code is from reading it
(nothing in the game was changed or run with a mod); the Windows findings are small scratch experiments, not a build of
the port. Each fact says which it is._

## 1. Goals and scope

- **A launcher:** its own executable with a cross-platform GUI (Linux and Windows first). It finds the user's settings,
  asks for the disc image when it is unset or missing, edits settings (keybinds now; graphics options and filters
  later; a 50/60 Hz toggle), enables and disables mods, shows each mod's configuration screen, and starts the game.
- **Mods:** basic support plus four first mods: **fast-forward**, **skip dialogues**, **skip intro** (the idea only for
  now), **disable battle animations**.
  Added later as an idea: **Global Saves** (5.6: a Save entry in the field menu).
- **Out of scope here:** the battle camera bug (issue #7), a hardware renderer, third-party code mods.

## 2. Decisions (the user, 2026-10-06)

| # | Question | Decision |
|---|---|---|
| 1 | Launcher toolkit | **Dear ImGui** on SDL3 + `SDL_Renderer`. C++ is accepted in the launcher only; the game stays C |
| 2 | What "mods" means in v1 | **Built-in features with manifests** (stage 0); **data overrides** next (stage 1); no third-party code mods for now |
| 3 | File format | **JSON** for the settings file and the mod manifests |
| 4 | Windows | **After** the launcher works on Linux (the Linux-side refactors of section 6 are not started in parallel) |
| 5 | Skip dialogues | **Toggle by default; hold offered too.** Fast-forwarding the cutscenes' scripted waits is **off by default**, offered as a setting |
| 6 | Battle animations | **Both offered as a setting; the short hit reaction is kept by default** |

Carried over from the user's request: the launcher is a separate executable; unpacking the disc image is not required;
fast-forward unlocks the frame rate to a number (200 or 300 fps were named) or to unlimited; skip intro is planned as an
idea only.

Proposed in the session's summary and not objected to (treat as defaults to confirm when each is built): the settings
lookup order (4.2), fast-forward's audio muted while it is active, fast-forward with both a hold and a toggle binding,
mods forced off under `--script`, the phase order (section 7).

## 3. What the research found

### 3.1 Speed (measured: Ryzen 9 5900XT, one core, session 18)

| Run | Vsyncs | Wall | Vsyncs/s | vs 50 Hz |
|---|---|---|---|---|
| Headless `new_game` (realistic CD) | 1,834 | 0.59 s | ~3,100 | 62x |
| Headless `new_game --cd-speed instant` | 1,623 | 0.56 s | ~2,900 | |
| Headless `first_battle_save` (realistic) | 41,826 | 14.05 s | ~2,980 | 60x |
| Headless `first_battle_save --cd-speed instant` | 38,678 | 13.20 s | ~2,930 | |
| SDL build, no window, `first_battle_save` | 41,826 | 14.08 s | ~2,970 | |
| SDL offscreen `--window --fps 0`, dummy audio, `new_game` | 1,834 | 1.83 s | 1,267 presented/s | 25x |
| The same, `first_battle_save` | 41,826 | 31.64 s | 1,337 presented/s | 27x |

- Presenting costs about 0.41 ms per vsync (convert, texture upload, present) on the offscreen driver with SDL's
  `opengl` renderer: more than the rest of the frame. A real compositor may be slower or may throttle.
- A gprof build of `first_battle_save` (instrumentation inflates small functions): the SPU about 58%, the software GPU
  about 33%, the game's logic and the GTE under 5%. The game's C is built at `-O0`; only `gpu.c` (`-O3`) and `spu*.c`
  (`-O2`) are optimised (`port/CMakeLists.txt:137-141`).
- The records of the window `--fps 0` runs are byte-identical to the headless runs' (both scripts).
- **So:** 200-300 fps has 4-10x headroom even when every vsync is presented. No rasterisation or SPU skipping is needed.
  The battle-only rate was not measured separately.

### 3.2 Pacing, audio, the present (read)

- A vsync runs `port_frame` (`port/src/pump.c:73-97`): the CD tick, the frame log, the input, the script,
  `port_video_frame`, then `port_pace`. Audio is the vsync pre-hook (`pump.c:45-46`, `port/psyq/libetc.c:22-34`).
- `port_pace` (`pump.c:101-126`): vsync n is due at `start + n/port_fps` (`clock_nanosleep`); `port_fps <= 0` returns at
  once; more than 0.1 s late starts the schedule over. `start` and `n` are statics: **lowering `port_fps` mid-run
  (250 back to 50) makes the game sleep for seconds** unless the schedule is reset.
- Audio: `audio_rate` is copied from `port_fps` once, at open (`port/src/audio.c:268`), and sets the samples per vsync
  (`audio.c:290`: 882 at 50, 735 at 60). When the queue passes `audio_max`, whole vsyncs of samples are dropped
  (`audio.c:206-208`); the offscreen `--fps 0` run dropped 40,245 of 41,825. Unthrottled, that is 20 ms shreds of sound.
- The present happens on every vsync (`port/src/video.c:264-281`, `187-212`); there is no `SDL_SetRenderVSync` call.
- The GPU always draws, headless too (`port/psyq/libgpu.c:128,340`). No `StoreImage` in `src/`, but `MoveImage` at
  `src/fieldstg/fieldstg_80087DB0.c:1248` (the shatter effect) reads the framebuffer: skipping rasterisation is not
  worth it.
- The CD, the XA audio and the movies count vsync ticks (`port/psyq/libcd.c:6`, `:99-106`; `StGetNext` ticks once per
  5,000 empty polls, `libcd.c:777`), so they speed up with the pace. `psyq_cd_set_timing` is a setter (`libcd.c:175`).

### 3.3 Input (read)

- Two `static const` tables: `input_keymap[16]` (scancode to pad bits, `port/src/input.c:53-63`) and `input_padmap[12]`
  (`:66-76`); the triggers and the left stick are hard-coded in `input_buttons()` (`:168-173`).
- F11 is hard-coded in the event switch (`:315-318`); quit and window-close call `port_exit(0)` (`:309-311`). **No
  pause, reset or Escape key.**
- The pad reaches the game a frame late on purpose (`port/psyq/libpad.c:41-55`, `109-115`): the emulator's timing.
- Only changes of the pad are logged (`port/src/framelog.c:156-165`); with `--script` the window never calls
  `psyq_pad_set` (`input.c:336`).

### 3.4 Options and configuration (read)

- Options (`port/src/main.c:89-153`): `--disc`, `--no-disc-check`, `--cd-speed`, `--memcard1|2 PATH|none`, `--script`,
  `--log`, `--record`, `--max-frames`, `--watchdog`, `--trace`, `--window`, `--scale`, `--fullscreen`, `--fps`,
  `--input-test`, `--screenshot`, `--spu-trace`, `--wav`, `--mute`, `--help`. Stored in `main()`'s locals and a few
  globals (`port_max_frames`, `port_fps`, `port_watchdog_sec`, `port_trace`). **No struct, no config file.**
- Environment: `DW3_PORT_RESET_CHECK`, `DW3_PORT_CHECKPOINT_DIR`, `DW3_PORT_TRACE`, `DW3_PORT_PRIM_DUMP`;
  `XDG_CACHE_HOME`/`HOME` only for the disc-hash stamp (`~/.cache/dw2003-port/disc-stamps`, `disc.c:115-142`).
- **Memory cards default to a fresh card in memory** (`main.c:173-177`, `memcard.c:70-72`): without `--memcard1 PATH`
  a save is lost at exit. There is no user data directory.
- **The watchdog** (10 s without a `port_wait`, `SIGALRM`, exit 4; `pump.c:33-41`) would kill a paused game or one
  behind a modal dialog.

### 3.5 50/60 Hz (read; 60 Hz has never been traced against the emulator, `docs/SOUND.md:249`)

- The NTSC patch's two bytes: `0x8005CCAC` is `records_60hz` (`config/symbol_addrs.txt:57`, `src/main/records.c:1419`,
  default 0); `0x8005CCB0` is `main_screen_pos` (`symbol_addrs.txt:473`, `src/main/main.c:24`, default 1).
- `records_60hz` drives: `SetVideoMode` (`main.c:37-41`); the time step per vsync, `+0x100` instead of `+0x133`
  (`src/main/gfx.c:28-36`, `86-90`: 60 ticks a second in both modes); actor speeds
  (`fieldstg_80087DB0.c:3900`, `4399`); the sound tick mode (`src/main/sound.c:344-348`; the shim then ticks at the
  video mode's rate, `port/psyq/libsnd.c:157-158`); the fade rate (`sound.c:220`).
- `main_screen_pos` drives `dispenv.screen.y = 24` (`gfx.c:181`; the port's video ignores it) and card-game layouts
  (`src/cardgame/cardgame_80096950.c:702`).
- So the game carries its own 60 Hz mode: with the flag and a 60 pace, the compensated systems (time, play time,
  walking, music tempo) keep their real speed and anything counted per vsync runs 20% faster than PAL, as on the patched
  PS1. `--fps 60` alone speeds everything up (music and the play-time clock too); the flag without the pace makes
  everything 17% slow.
- "`--fps 60` with XA" (STATUS): `CD_VSYNC_HZ` is a compile-time 50 (`libcd.c:99,106`); at 60 the drive feeds 882 XA
  frames a tick while the SPU takes 735, and the movies run 20% fast (`libcd.c:66-67`).
- A game global must be set **before** `port_overlay_init()` (`main.c:165`) so it is in the reset's snapshot: a later
  write is fatal under `DW3_PORT_RESET_CHECK`/`--trace` (`pump.c:57`) and undone by any reset.

### 3.6 Hooks in the game's C (read)

- 15 `#if(n)def PC_PORT` directives in 12 files under `src/`; `include/port.h` has 25 macros with 482 uses. Shapes: a
  macro that is empty on the PS1 (`PLATFORM_WAIT()`, e.g. `src/main/cdload.c:269`), an added block (`cdload.c:206-214`,
  `src/fightstg/fightstg_model.c:529-533`), an `#ifndef PC_PORT ... #else` alternative (`src/main/object.c:162-168`).
- `port_frame` runs on every vsync tick from wherever the game is (a `VSync`, a wait loop, `StGetNext`, an overlay
  copy's ticks): it is not a game-frame boundary.
- The runtime reads game globals directly (`port/src/state.c:17-23,51-55`). An overlay's globals are valid only while
  that overlay is current and are restored from the snapshot at every load.

### 3.7 File loading (read)

- File ID to `filetable_lba[id]`/`filetable_sectors[id]` (`src/main/filetable.c:4,207,366-383`; 2,382 entries, writable
  game data); `cdload_read` (`cdload.c:108-126`) issues Setloc/ReadN and checks each sector's header position
  (`cdload.c:26-36`); `disc_read` serves raw 2352-byte sectors (`port/src/disc.c:40-48`, registered at `disc.c:279`).
- Buffers are `get_sectors(id) << 11` (`cdload.c:216`); the overlay copies and the sound use the same table
  (`src/main/overlay.c:109,120`, `sound.c:290`). Partial reads at `src/fieldstg/fieldstg_80085590.c:729,818`; the movies
  stream by `get_cdloc` (`src/stdwtitl/stdwtitl_80082D70.c:370`).
- **A loose-file override is feasible** as a wrapper around the sector reader in `disc.c` (LBA to file ID, a raw sector
  synthesised from the loose file): no game C changes, the disc hash check still holds. Same size or smaller works as it
  is; a larger file needs a virtual LBA past the disc's end, `filetable_lba`/`filetable_sectors` patched before the
  snapshot, and heap room (the host heap is 4 MB). Excluded: `.STR` movies (raw interleaved XA sectors) and code
  overlays (their file contents are ignored: the code is linked in).

### 3.8 Dialogue (read; nothing run)

- **The window** (`MessageWindow`, `include/message.h:74`; `message_window_update`, `src/main/message.c:872`): in
  `OBJECT_STATE_RUN` one more character is shown once `speed_count` exceeds `speed` (`message.c:884`); field dialogs set
  speed 6, one character per 7 frames (`message.c:1336`, `1706`). The control code `02 02 n` runs
  `message_code_wait_button` (`message.c:769`); the window leaves the wait at `message.c:904` on
  `PAD_PRESSED(message_wait_buttons[substep])` (`{13,12,13,14,15}`, `message.c:1813`). `is_waiting` is `base.step == 1`
  (`message.c:671`). The timed pause (code 6, `message.c:852`) never occurs in the files (`docs/FORMATS.md:175`).
- **No text speed setting.** Pressing confirm during the reveal shows the whole page (`find_page_end`: `message.c:1608`
  dialog, `1305` box; the overlays copy the pattern, e.g. `src/wstag/wstag935.c:178`). The presses are edges
  (`src/main/pad.c:332`): a page takes two presses, reveal then advance. Confirm is bit 13 (physical cross is rotated
  onto it when `records_language != 0`, `pad.c:274-291`).
- **Containers:** `message_create_dialog`/`message_dialog_update` (`message.c:1664`, `1595`: the speech bubble, with an
  open and a close animation), `message_box_create`/`message_box_update` (`message.c:1328`, `1283`: the bottom box).
- **The field's event VM** (`fieldstg_event_update`, `src/fieldstg/fieldstg_event.c:68`): opcode `0x02xx` creates a
  dialog (`:145`), `0x0301` waits until it is gone (`:166`); its own waits are N frames (`:156`), a walk (`:173`), an
  animation (`:181`), and C-coded child objects (`:216`). NPC talk: the player's raw `0x2000` press
  (`src/fieldstg/fieldstg_80087DB0.c:3119`) reaches `fieldstg_actor_talk_to` (`:3071`).
- **Opcode census** over `src/wstag` and `src/fieldstg`: DIALOG 1,519; WAIT(n) 3,150 (134,028 frames in all; 2,272 are
  WAIT(30)); WAIT_WALK 598; WAIT_ANIM 79; WALK 823. About two scripted waits (0.6 s each at 50 Hz) per dialog, plus the
  walks and about 25 frames of bubble open/close: **a text-only skip leaves a large share of a cutscene untouched.**
- **Other confirm systems (no single choke point):** the battle's `fightstg_message_step`
  (`src/fightstg/fightstg_8008D3B4.c:4934`; step 3 waits on raw `0x2000` at `:4961`, step 4 auto-closes after 0x15 ticks
  or a press, `:4974`); code-driven `PAD_PRESSED(13)` sites in cardgame (31), ststatus (24), wstag (23), stgtrain (9),
  stgmcard (7) and others.
- **Choices are code, not control codes:** `fieldstg_choice_update` (`src/fieldstg/fieldstg_80083D08.c:246`, the press
  at `:301`, the first answer the default); the same shape in `wstag210/220/270/280/935.c`, `src/main/inn.c:144`,
  `src/main/fieldmenu.c:244`. None goes through `message.c:904`.
- **Predicates:** `fieldstg_stage.event_running` (`include/fieldstg.h:161`), `fieldstg_stage.actor_busy` (`:163`),
  `FieldstgEventData.dialogs[]` (`fieldstg_event.c:13`), `MessageWindow.is_waiting()`, the battle's
  `FightstgMessage.waiting` (`fightstg_8008D3B4.c:301`). **There is no global "a dialogue is open" flag** and none for
  a pending choice.
- **Softlocks:** none found: no script waits on a voice or a sound; the battle's timed close also takes a press.
- Not checked: whether wait-button arguments 1, 3, 4 occur in any text; the callers of the battle's messages.

### 3.9 Battle (read; nothing run)

Files: **WM** `src/wfightmn/wfightmn_800A6440.c`, **F8** `src/fightstg/fightstg_8008B630.c`, **FD**
`src/fightstg/fightstg_8008D3B4.c`, **F0** `src/fightstg/fightstg_80086A00.c`.

- **The sequencer:** `wfightmn_run_events` (WM:1459): step 0 takes the next event (`fightstg_events.take_next()`,
  FD:5908), step 1 maps its type to a handler (`wfightmn_handlers`, WM:1908), step 2 waits for `data->wait.obj == NULL`
  (WM:1554). A party turn (`wfightmn_party_turn`, WM:472) opens the menu, waits on `fightstg_command_is_open()`, then
  creates `fightstg_attack_create`, `fightstg_tech_create` or `fightstg_item_create` (WM:510-545). An enemy turn:
  `fightstg_enemy_turn_create` (WM:1489-1500, F0:889); battle types 5 and 6 use the scripted and boss turn objects.
- **An action** (`fightstg_attack_update`, F8:713; `fightstg_tech_update` FD:1362 and `fightstg_counter_update` FD:1131
  have the same shape): INIT shows the "X attacks" message and calls `fightstg_action.run()`, **which makes every roll
  and computes the damage** (F8:746, FD:6807). Step 0 creates the animation script once the message closes
  (`wfightmn_tech_script_create`, F8:761). Step 1, when the script is gone, shows the damage message and writes the HP; a
  kill queues a knockout event (F8:766-812). Step 2 applies status effects, steal and drain, a message each
  (`fightstg_results_create`, FD:2219). Steps 4-5: wake-up, the counter-attack, the gauge.
- **The rules run up front.** The scripts never touch HP, status or the random generator; they read `script->results[]`
  to pick the target's reaction. The script creator has rule side effects (WM:1806-1816: the copied technique, the boss
  hit count, the idle animation, the final-phase check): **a cut must come after the creator, not replace it.**
- **The random stream depends on time:** `pad_random.next()` is drawn once per main-loop frame (`src/main/main.c:95`),
  plus cosmetic draws (blink timers, `fightstg_8008B118.c:86`; the idle camera, F0:448); `docs/MECHANICS.md:66-69`.
  Skipping frames leaves the rules unchanged and shifts later rolls, as a faster player would.
- **The animations:** per-model disc data (`FightstgModelRecord.script_file`, s16 command streams) interpreted by
  `fightstg_script_update` (F8:552). Commands: child reaction (F8:101), model animation/jump/move/show (F8:144), effect
  (F8:292), stage (F8:468), camera (F8:389), prop, flash, sound (F8:520), wait (F8:445). Durations come from the model's
  animation tables, explicit wait words, effect sprite tables and on-demand CD loads (F8:314-341).
- **A scan of the disc** (a scratch decoder): all 245 script files x 18 scripts parse to END. Scripts 1-4 are the
  target's reactions (flinch, heavy hit, KO, which always ends on animation 10, dodge). Scripts 5-17 are attacks,
  techniques and items; every one with reactions ends with `child 0`, which plays `results[3] + 1`. Explicit waits alone:
  a median of 65 ticks for the basic attack, about 1,500 ticks (~25 s) for script 12.
- **Scenes outside the scripts carry functional writes:** `fightstg_entrance_update` (F0:233; slot swap, stage and
  music changes at F0:316-333), `fightstg_digivolve_update` (F0:1518), the defeat camera (F8:663, 240 ticks), the intro
  camera and the victory wait (WM:1342).
- **Predicates:** `fightstg_command_is_open()` (FD:2955; step 2 the menu, 3 the confused menu, 4 the forced switch,
  FD:2689); `FightstgMessage.waiting` (FD:301, set at FD:4953). No global says "an action is being presented": script,
  entrance and digivolve objects have kind 0. The one runner call `obj->update(obj, obj->children)`
  (`src/main/heap.c:299`) is where a port macro could compare `obj->update` with the presentation functions.
- **No retail option.** `fightstg_battle.speed.mode` (FD:6887: 0 normal, 1 paused, 2 slow, 3 double) is cycled only in
  the debug overlay WFIGHTTS (`src/wfightts/wfightts_800A67B8.c:404-409`) and set to slow for the defeat scene (F8:683).
  It scales model, sprite, jump and move steps, not script waits, cameras or scene timers.
- **Softlocks:** none found for a cut at the script's INIT: every parent waits on its slot becoming NULL; multi-hit,
  counters, poison/regen ticks and items create scripts the same way (FD:1211, WM:858, WM:918, FD:431). **Cutting a
  script mid-run is unsafe** (delayed key-off sound objects FD:5806, stage restores, added models).

### 3.10 Precedents (web research; items marked [memory] were not re-checked)

| Project | Launcher | Toolkit | Mods | Mod options |
|---|---|---|---|---|
| Zelda 64: Recompiled | In-process | RmlUi | `.nrm` (zip: manifest, MIPS code, textures) | Declarative `config_options` in `mod.toml`, rendered by the host |
| Ship of Harkinian / 2S2H | In-game menu | Dear ImGui | `.otr`/`.o2r` archives; built-in "enhancements" as cvars | None declared |
| sm64coopdx | In-game | Own [memory] | Lua + DynOS packs | Imperative hooks (`hook_mod_menu_checkbox`, ...) |
| OpenGOAL | Separate | Tauri | Installed from mod-source URLs | Not found |
| OpenMW | Separate `openmw-launcher` | Qt | Data dirs + content files, ordered | Lua `I.Settings` pages |
| Unleashed Recompiled | In-process wizard | Dear ImGui | Hedge Mod Manager format; no code mods yet | None |
| devilutionX | In-game | Own | `mods/<name>/` (Lua, data) | Not found |
| Daggerfall Unity | In-game | Unity | `.dfmod` | Declarative `settings.json` (checkbox, choice, sliders, ...) |
| RetroArch | In-app | Own | Cores | Core options v2 (key, values[], default, categories) |

- **The trend** is an in-process UI drawn by the game's renderer and navigable with a controller: one binary, one
  config path, and Steam Deck's Game Mode handles extra windows badly. Separate launchers survive where a desktop
  toolkit or an install flow justifies them. Hence the rule in 4.1: a thin launcher, screens that could later be
  embedded.
- **Toolkits:** Dear ImGui (MIT, a handful of files, SDL3 + SDL_Renderer backends, gamepad navigation by a flag, C++;
  the default look needs a font and a style) ranked first, and it is the one whose screens can later run inside the
  game's window. Nuklear (pure C; its SDL3 backends are recent and demo-grade; no built-in gamepad focus navigation
  [memory]) second. RmlUi (MIT, C++, FreeType; the best theming; the heaviest build) third. Rejected [memory]: Qt
  (LGPL, static builds), GTK, wxWidgets/FLTK (no in-game reuse), Tauri/Avalonia/Slint/egui (a second toolchain).
- **`SDL_ShowOpenFileDialog`** (SDL >= 3.2) is asynchronous; on Linux it tries the XDG portal, then `zenity`, and
  reports an error when neither is there; on Windows it uses the system dialog. It can fail (minimal distributions, Game
  Mode), so the launcher also needs drag-and-drop and a typed path.
- **Option schemas:** Zelda64Recomp's `mod.toml` (`id`, `name`, `description`, `type` Enum/Number/String, `min`/`max`/
  `step`, `default`); Factorio's `settings.lua` adds *when a change takes effect* (`startup`/`runtime`), worth copying.
- **Settings locations:** `SDL_GetPrefPath` gives `%APPDATA%\org\app\` on Windows and `~/.local/share/app/` on Linux.
  An empty `portable.txt` beside the executable selects portable mode in DuckStation, PCSX2, Dolphin, Zelda64Recomp and
  UnleashedRecomp.
- **Mod stages:** 0 built-in features; 1 data overrides (no code runs; only the game's own formats); 2 scripting (a
  versioned event API kept forever); 3 native code (ABI versions, per-OS builds, arbitrary code). N64Recomp's
  hook-any-function model depends on mods being MIPS code recompiled at load and does not transfer to a native C port:
  hooks here are explicit call sites.
- **Fast-forward elsewhere:** RetroArch has separate hold and toggle binds and a ratio cap; emulators mute, pitch-shift
  or time-stretch the audio (time-stretching needs SoundTouch: C++, LGPL [memory], outside our licence rule).

### 3.11 Windows (scratch experiments with clang `--target=x86_64-w64-mingw32`, GNU ld `i386pep`, lld, Wine 11.17)

No MinGW GCC, headers or CRT are on the dev machine: the experiments were freestanding objects, and **the port itself
was not built for Windows**.

| Item | Where | Why it is not portable | Fix |
|---|---|---|---|
| Per-overlay output sections | `tools/port_gen.py:368-403` | 314 overlays give 630 output sections; the Windows loader allows 96; names over 8 characters are truncated; COFF's `-fdata-sections` names are `.data$x` | One section with subranges, or the script-free scheme below |
| `-T` / `INSERT` | `port/CMakeLists.txt:145`, `port_gen.py:395-396` | GNU ld only; lld rejects `-T` (verified) | The script-free scheme |
| Arena symbols | `port_gen.py:398-401`, `include/port.h:186-189` | Defined by the script; must be constants for static initialisers (`src/main/main.c:108,110`, `fieldstg_80087DB0.c:5694+`) | Macros on the array (`port_arena + 0x23130` is an address constant: verified on PE and ELF) |
| The 16 MB-aligned `.bss` arena, `-no-pie` | `port/src/arena.c:15,25`, `CMakeLists.txt:67,72` | COFF's maximum alignment is 8,192 (clang rejects `aligned(1<<24)`: verified); ASLR moves the image | Drop the requirement: tags relative to the arena's base |
| Low-24-bit tags | `include/port.h:142`, `port/psyq/libgpu.c:130,150,318,331`, `src/main/gfx.c:310-314` | Depend on that alignment | `PTR_TO_U32` relative to the arena |
| `nm -S` sizes, the `.size` probe | `port_gen.py:634,671,703-705` | COFF has neither (verified for `nm -S`) | Probe with the host ELF compiler, or emit `sizeof` as data |
| The POST_BUILD map check | `CMakeLists.txt:146-148`, `port_gen.py:409-472` | Parses a GNU ld map | A per-object check before the link |
| The watchdog | `port/src/pump.c:48-52,66,76` | `sigaction`/`alarm` | A thread or a timer |
| Pacing | `pump.c:109,123` | `clock_nanosleep(TIMER_ABSTIME)` | `SDL_GetTicksNS` and SDL's delay |
| Disc I/O | `port/src/disc.c:46,227,261,265` | `pread`, `O_CLOEXEC`, no `O_BINARY` | `_open(O_BINARY)` + `_lseeki64`, or `SDL_IOFromFile` |
| The stamp cache | `disc.c:116-147,186,218` | `XDG_CACHE_HOME`, two-argument `mkdir`, `st_mtim`, `st_ino`, `realpath` | The user directory, size + mtime |
| Paths | `disc.c:92,95,256` | `/` only, `strcasecmp`; `fopen` takes the ANSI code page (non-ASCII disc paths fail) | Accept `\` and drive letters; UTF-8 through SDL's I/O or `_wfopen` |
| `rename()` onto an existing file | `port/src/memcard.c:44`, `disc.c:204` | Fails on Windows: **every save after the first would be fatal** | `MoveFileExW(MOVEFILE_REPLACE_EXISTING)` |
| Text-mode `fopen(..., "w")` | `framelog.c:86,94,206`, `spu_trace.c:85` | CRLF breaks byte-identical logs | `"wb"` |
| `setvbuf(f, NULL, _IOLBF, 0)` | `framelog.c:90`, `main.c:159` | UCRT wants a size >= 2; `_IOLBF` is full buffering there | `_IONBF` or `fflush` |
| `longjmp` (the reset) | `port/src/reset.c:37`, `main.c:199` | Win64 `longjmp` unwinds through SEH: untested | `__builtin_setjmp`/`longjmp` as the fallback |

- **LLP64 is harmless:** the game's C has three `(unsigned long)` offset casts and two Psy-Q prototypes taking `long`;
  nothing else in `src/` or `include/` uses `long`.
- **The Wine crash seen in the session** (`out_bfd.exe`): the first PE adaptation of `overlays.ld` (`INSERT BEFORE
  .data`, separate `.dw3.data.<ovl>` output sections, no `BLOCK(__section_alignment__)`) linked silently into an invalid
  PE: the data sections sat in `.text`'s page, so the entry point held data. **GNU ld does not validate PE section
  alignment.** A reshaped script (one page-aligned `.dw3data` and one `.dw3bss` output section, the start/stop symbols
  inside) passed.
- **The script-free scheme (verified on test objects, ELF and PE):** the arena a plain static array with tags relative
  to its base (tags become pointers only in `psyq_gpu_walk`; the primitive hash excludes tag words, so the frame logs
  should not change); the game units built without `-fdata-sections`, then `objcopy --rename-section` on each object's
  `.data`/`.bss`: on ELF `dw3d_<ovl>`/`dw3b_<ovl>` (the linker supplies `__start_`/`__stop_`; passed with and without
  PIE), on PE `.dw3d$<ovl>.1` between generated marker objects `.0` and `.2` (passed with GNU ld, lld's MinGW driver
  and `lld-link`). Caveats: the bss markers must be emitted as bss; `llvm-objcopy` 20 cannot rename COFF sections (GNU
  objcopy is needed; the repo already builds binutils). The guarantee "no game data outside the ranges" becomes "no game
  object has a writable section other than the renamed ones", checked per object.
- **Toolchain:** llvm-mingw publishes Linux-hosted UCRT cross tarballs (with ASan/UBSan), which fits the `setup.sh`
  pattern; it is lld-only, so it needs the script-free scheme. SDL3 ships a MinGW CMake toolchain file (a static
  cross-build of the pinned tarball was not tried).
- **For the launcher and mods:** `SDL_CreateProcess` exists since SDL 3.2 (the repo pins 3.4.18); a PE DLL cannot have
  undefined symbols, so a native mod would get the game's functions through a table passed to its init (the same on both
  systems), not through `-rdynamic`.

## 4. Design

### 4.1 The contract between the launcher and the game

- The game gets **one new option, `--config FILE`** (JSON). Command-line options override the file.
- **The game never loads a configuration on its own.** Without `--config` it behaves exactly as today: the tests and CI
  run the bare binary and must not depend on the machine.
- The launcher only edits that file and starts `dw2003 --config <dir>/settings.json` (`SDL_CreateProcess`). The game
  stays startable without the launcher (`--config`, or plain options).
- A launcher-started game always has: the disc, **a memory card file in the user directory**, a window, the watchdog
  off.
- The disc's SHA-1 check stays the game's (`port/src/disc.c`, `sha1.c`); the launcher reuses that code (linked, or
  through a check mode of the game: to pick when it is built).
- The launcher's screens are written so they could later be drawn inside the game's window (the same SDL3 and
  `SDL_Renderer`), for Steam Deck. Not planned now.

### 4.2 The settings directory (lookup order)

1. `--config-dir DIR` (or an environment variable).
2. An empty `portable.txt` beside the executable: the executable's directory.
3. A settings file that already exists in the current directory (the local-testing fallback; never created there
   implicitly).
4. The per-user directory (`SDL_GetPrefPath`: `%APPDATA%\...` on Windows, `~/.local/share/...` on Linux).

One directory holds `settings.json`, the memory cards and (later) `mods/`. The launcher shows which directory it chose.
The disc path is stored with its verified SHA-1.

### 4.3 The settings file (schema 1: settled by path B, session 19; the contract path A builds against)

_Settled when `--config` was built (`port/src/settings.c`; the sketch of session 18 changed in four places, marked
**changed**). `dw2003 --config FILE --print-settings` prints the effective settings with every key and absolute paths
and exits 0, or exits 64 naming the bad key: the launcher can use it to validate a file. `tests/port/settings.py`
checks the round trip, the defaults, the overrides and the errors._

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

- **Every key but `schema` is optional;** an absent key keeps its default (the values above, except `disc.path`: none,
  and `input`/`mods`, below). `schema` must be 1; a higher one is refused (exit 64, "a newer launcher's").
- **Paths** (`disc.path`, `memcard1`, `memcard2`) are relative to the settings file's directory, or absolute. The
  launcher owns the lookup of that directory (4.2); the game never looks for a settings file.
- **Errors:** a value of the wrong type or out of range ends the game with status 64 and a message naming the key
  (`port: settings FILE: video.scale: an integer from 1 to 16, not 17`), as a bad option does. **An unknown key is
  logged and ignored**, so an older game runs a newer launcher's file.
- **The command line overrides the file** (`--disc`, `--scale`, `--fullscreen`, `--window`, `--mute`, `--memcard1|2
  PATH|none`, `--watchdog`, `--fps`).
- `disc.sha1`: the launcher's record of what it verified. The game does not trust it: it checks the disc itself
  (`disc.c`, with its stamp cache).
- `video.window` (**changed: added**, default true): false runs headless (tests, a check run). `video.scale` 1-16,
  `video.refresh` 50 or 60 (60 is phase 2: until then the game logs it and runs at 50).
- `memcard1`/`memcard2` (**changed:** both slots, default `card1.mcd`/`card2.mcd` beside the file, created formatted
  when missing): a string is a `.mcd` image, **`null` means no card in that slot**.
- `watchdog` (**changed: added**): seconds without a vsync before the game exits 4; **0 (the default under
  `--config`) is off**. The bare binary keeps its 10 s.
- `launcher` (**changed: added**): the launcher's own state (window geometry, last directory, ...). The game never
  reads it and prints it back unchanged.
- **`input`** (applied from phase 1's second pull request; until then read, checked as an object and printed back):
  - `keyboard`: PS1 button -> one key name or a list of them. `gamepad`: PS1 button -> one gamepad input name or a list.
    A button that is absent keeps its default; `""` or `[]` unbinds it. Buttons: `up down left right cross circle square
    triangle start select l1 r1 l2 r2`.
  - **Key names are SDL3's scancode names** (`SDL_GetScancodeName`/`SDL_GetScancodeFromName`: `"X"`, `"Return"`,
    `"Keypad Enter"`, `"Right Shift"`, `"F11"`, `"Tab"`); scancodes are the key's place, whatever the layout.
  - **Gamepad input names are ours** (SDL's positional buttons): `south east west north back guide start leftstick
    rightstick leftshoulder rightshoulder dpup dpdown dpleft dpright misc1 paddle1 paddle2 paddle3 paddle4 touchpad`,
    and the axes past half way: `lefttrigger righttrigger leftx- leftx+ lefty- lefty+ rightx- rightx+ righty- righty+`
    (`lefty-` is the left stick up).
  - `hotkeys`: the port's own actions -> a **binding**. Actions: `pause` (default `"P"`), `fullscreen` (default
    `"F11"`). A mod's bindings are its options, under `mods.<id>` (below), not here.
  - **A binding** is a string or a list; each element is one trigger, any of which fires the action. A trigger is one
    input (a string) or a chord (a list of inputs, all held). An input is a key name, or `"pad:"` + a gamepad input
    name. `""` or `[]`: unbound. Examples: `"F2"`; `["F2", "pad:guide"]`; `[["pad:guide", "pad:south"], "Tab"]`.
    The inputs of a chord that fired are masked out of the pad while held; a single-key hotkey never reaches the pad.
    The first input of a chord made only of game buttons does reach the pad until the chord is complete: chords
    should start with an input the pad map does not use (`pad:guide`, the stick clicks).
- **`mods`**: `<id>` -> `{ "enabled": bool, "<option id>": value }`, the option ids and types from the mod's manifest
  (4.4). **A mod that is absent, or has no `enabled`, is off.** An absent option keeps the manifest's default. An
  unknown mod id is logged and ignored (applied from phase 1's second pull request; until then read and printed back).
  Under `--script` every mod is off unless the run asks for them (4.5).

### 4.4 The mod manifest (`mod.json`)

Top level: `schema`, `id`, `name`, `version`, `description`, `kind` (`builtin` now; `data` at stage 1),
`requires_port`, `options[]`. An option: `id`, `name`, `description`, `type`, `default`, and optionally `group` and
`applies` (`live` or `restart`). Types: `bool`; `int` and `float` with `min`/`max`/`step`; `enum` with
`values[{id, label}]`; `binding` (4.3's grammar: a key, a pad input or a chord, or a list of them; the default
written the same way). No `string` or `path` type for now.

```json
{
  "schema": 1, "id": "skip_dialogues", "name": "Skip dialogues", "version": "1.0", "kind": "builtin",
  "description": "Text appears at once and advances by itself. Choices still wait for you.",
  "options": [
    { "id": "toggle", "name": "Toggle", "type": "binding", "default": "F2" },
    { "id": "hold",   "name": "Hold",   "type": "binding", "default": "" },
    { "id": "fast_forward_waits", "name": "Also fast-forward cutscene waits", "type": "bool", "default": false,
      "applies": "live" }
  ]
}
```

- The user's values live in the settings file under `mods.<id>`, never in the manifest.
- The built-in mods use the same schema as later data mods: one renderer in the launcher serves both.
- The manifests are files (`port/mods/<id>/mod.json`, installed beside the game); the launcher lists what it finds there
  and, later, in the user directory's `mods/`.

### 4.5 The mod runtime (game side)

- `port/src/mods.c`: the registry of built-in mods, their option values from the settings, `port_mods_frame()` called
  from `port_frame`, and the hotkey actions.
- **Input:** the two map tables become mutable and filled from the settings (SDL's name-to-scancode and
  name-to-button lookups); a second table maps a key or a pad chord to an action id. A chord's buttons are masked out
  of the pad before `psyq_pad_set` and `port_framelog_input`. `--input-test` keeps testing the defaults.
- **Rules for a mod that changes the game's behaviour:**
  1. The change sits in an `#ifdef PC_PORT` block that tests a `port_mod_*` flag. Not `if (MACRO_THAT_IS_0)` or
     `|| MACRO`: an expression that is constant on the PS1 can change the old GCC's code. `scripts/build.sh --check`
     gates every hook.
  2. **Mods are off under `--script`** unless the run asks for them, so the replays, goldens and records are unchanged.
  3. A hotkey never reaches the pad: no `I` line, no change to a record.
  4. A mod's own state lives in the runtime's variables. A game global a mod must set is set before
     `port_overlay_init()` (the snapshot), or the reset check fails.
  5. A run with a mod on needs its own expected results: the random generator steps once a frame, so any skipping
     shifts later rolls (as a faster player would).

### 4.6 The launcher

- Its own target (`launcher/` or `port/launcher/`, to pick), C++ with Dear ImGui, the SDL3 backends and `SDL_Renderer`;
  Dear ImGui pinned (version and hash) by a `setup.sh` step, recorded in `docs/THIRD_PARTY.md` (MIT: inside the port's
  licence rule).
- Screens: **first run / disc** (the file dialog, drag-and-drop, a typed path; the SHA-1 check's result), **play**,
  **settings** (video scale/fullscreen, 50/60 Hz, audio), **controls** (keyboard and gamepad bindings, the hotkeys),
  **mods** (the list with enable switches; each mod's screen generated from its manifest).
- It writes `settings.json` and starts the game; it shows the game's exit status and its last log lines when the game
  ends with an error.

## 5. The mods

### 5.1 Fast-forward (runtime only: no game C)

- `port_fps` splits into the **nominal rate** (50 or 60: the audio's samples per vsync, the CD) and the **pace** (the
  wall clock; changed by the hotkey, with the schedule reset on every change).
- Presents are capped near 60 a second while it is active; every vsync is still drawn by the software GPU.
- Audio is cleared and muted while it is active (`spu_render` keeps running: LIBSND reads the envelopes). A pitched-up
  mode can come later. No time-stretching.
- Bindings: hold and toggle. Options: the speed (a multiple of the nominal rate, or a frame rate, or unlimited: the
  user named 200 or 300 fps; **the default value is still to pick**), mute.
- Tests: the record of a run with fast-forward held is the unthrottled run's (already byte-identical at `--fps 0`);
  pace changes up and down without a stall.

### 5.2 Skip dialogues

- **The hook:** a port flag at the message window's wait (`message.c:904`) and in its RUN case (`:884`, calling
  `find_page_end`: instant text for every window, the overlays' too), plus the battle's message wait
  (`fightstg_8008D3B4.c:4961`). No pad press is synthesised.
- **Choices stop by themselves:** they are separate code that the hook does not touch. Name entry, shops, the inn and
  the save prompts likewise.
- **Rejected: turbo presses of confirm from the input layer.** It picks the default answer at every prompt and talks
  to the NPC again when control returns (the same button).
- **Bindings (decided):** toggle by default, hold offered too.
- **Option (decided): "also fast-forward cutscene waits", off by default.** When on, the mod turns fast-forward on while
  `fieldstg_stage.event_running` holds (the scripted waits, walks and bubble animations are a large share of a
  cutscene). Free walking is not sped up.
- **Not covered in v1:** the card game's, the status screen's and other overlays' code-driven prompts.
- **To verify first:** the PS1 build byte-identical; a dialog closes cleanly and the NPC is not triggered again; a
  choice reached with the mod on waits (stage 528's events, an inn); the replays unchanged with the mod off. Map titles
  print at once too (`fieldstg_80085590.c:1009`: harmless).

### 5.3 Disable battle animations

- **v1, the cut:** at `fightstg_script_update`'s INIT (F8:562), before script 12's sound-bank load (F8:575), for scripts
  5 and up (attacks, techniques, items): with the **hit reaction kept (the default, decided)**, a script that has a
  child command becomes the target's reaction (`side` flipped, `script = results[3] + 1`) and one without ends at
  once; the hook plays the hit sound the parent script would have played (F8:527). An action is then its two messages
  and about half a second of reaction.
- **The setting "hit reaction: off" (offered, decided):** the script ends at once, no reaction. To verify: a knocked-out
  model must still end in its KO pose (reaction script 3 ends on animation 10), or the option keeps the KO reaction.
- Scripts 1-4 created directly (the defeat camera's script 3, `fightstg_player_reaction`) are left alone.
- **v1.1, the scenes:** entrances, digivolutions, the intro camera and the defeat scene carry functional writes (slot
  swaps, stage and music changes), so they are not cut: an option fast-forwards them, with the predicate "a
  presentation object's update ran this frame" (`heap.c:299`), no message waiting and the menu closed. The sequenced
  music advances with the frames and would jump after each burst (inferred, not heard).
- **Not chosen:** the game's own speed mode (FD:6887): partial (waits and cameras ignore it), and large steps can jump
  over loop markers in `fightstg_model_step_anim` (`fightstg_model.c:346-357`).
- **Before judging the picture: the battle camera fix (issue #7).**
- **To verify first:** reaction scripts 1-4 run correctly as a top-level script (zeroed `effect`, `stage`,
  `hit_sound`); the KO pose and the victory/defeat flow in the first story battle (`first_battle_save` with the mod
  on, new expectations); a multi-hit technique, a counter, poison ticks, an item, script 12, the type 4/5/6 final boss;
  the "has a child command" scan against all 245 script files.

### 5.4 Skip intro (the idea only; nothing investigated, by the user's instruction)

- **What the player gets:** START at the title leads to the name entry and the team selection; then the game continues
  in the main lobby with the event that gives the first Digimon triggered automatically.
- **Two candidate routes, to compare later:**
  1. **A state transplant:** after the registration, put the game state where a reference run has it on arrival at the
     lobby (the map, the position, the story flags the skipped events set). The replay tests already hash
     `gamestate_data` at checkpoints on this path (`new_game`, `first_battle_save`), which gives the reference and the
     check.
  2. **An invisible auto-play:** run the opening with fast-forward and skip-dialogues on and the picture held. No state
     risk; it needs the opening to require no player movement, or a pad script for it.
- **The first things to find out:** what the opening sets (flags, items, the party), whether it needs movement, and
  where the registration's choices (the name, the team) land in the state. The boot's logos and movies could be skipped
  by the same mod (an option).

### 5.5 The 50/60 Hz toggle (a setting, not a mod)

Three things change together: the pace (60), `records_60hz = 1` set before the snapshot, and the CD/XA rate
(`CD_VSYNC_HZ`) made a run-time value. `main_screen_pos` stays 1 until the card game is looked at in both layouts.
Needs its own check against the emulator (the 2-byte patch applied there): 60 Hz has never been traced.

### 5.6 Global Saves (the idea only; added 2026-10-06 from a reading of the save path, nothing run or built)

- **What the player gets:** a **Save** entry in the field menu (`src/main/fieldmenu.c`) that opens the game's own save
  screen from any field map. Today the save screen is reached only from the inns' events (18
  `FIELDSTG_EVENT_GOTO_MAP(0xC0x, …)` sites in `src/wstag/`) and the load only from the title's Continue.
- **Why it looks cheap: a save already holds the map and the exact position.** What the reading found:
  - **The slot** is the first 0x26C4 bytes of `gamestate_data`, copied whole (`include/gamestate.h`, FORMATS "Save
    data"). In it: `field_map` (`0x34`, the map ID), `player_pos` (`0x38`, x and y as 24.8 fixed point), `player_dir`
    (`0x40`), `route`/`room` (`0x44`, the maze position). There is no spawn table and no saved entry index.
  - **Not in the slot** (they start at `0x26C4`): `map`, `next_map`, `prev_map`, `map_entry`, `countdown`,
    `map_is_new`, `field_last_map`, `attr_layer`, `unk_26E4`, `player_depth`, `spot_target`, `player_height`,
    `meter_random_count`. Also outside
    it: `gamestate_flags.map_flags` (the type-0 flags) and everything FIELDSTG and the stage overlay keep themselves
    (actors, a running event).
  - **Who writes the position:** FIELDSTG copies `get_map()` and the player actor's `pos`/`dir` into those fields when
    the field menu opens (`src/fieldstg/fieldstg_80087DB0.c:1681`), when a battle starts (`:1253`) and when the
    window-close transition to another map ends (`:1449`, the inn's path to the save screen). **Opening the menu
    already takes the snapshot a save needs.**
  - **The save screen** is STGMCARD, map `0xC00 + i`. It saves when the map entry is negative and loads otherwise
    (`stgmcard_main_create`: `loading = entry >= 0`; the inn's event passes -1, the title 0; STAGSLCT lists
    `0xC00`/`0x80000000` as save and `0xC00`/`0` as load). `i` picks the place name stored in the slot summary
    (`stgmcard_map_names[26]`, `?SHPNAM` entries); the area name comes from the previous map
    (`stgmcard_map_areas`: 239 maps in `0x200`-`0x2FF`, plus `0x1500`; a map that is not listed leaves the area 0).
  - **The way back** is the same after a save, a cancelled save and a load: `set_next_map(field_map, 0)` (or the
    previous map on a cancel), then FIELDSTG's loader sees a previous map that is not a field map (`0x2xx`/`0x3xx`) nor
    `0xE00`/`0x1500`/`0x500`, forces `map_entry = -1` and copies `player_pos`/`player_dir` to
    `fieldstg_stage.return_pos`/`return_dir` (`:1532`), where the player actor is created (`:4421`). With any other
    entry the actor starts at the stage's single `start_pos` (`src/wstag/wstag230.c:39`); exits and warps carry their
    own destination x, y and direction (`FieldstgMapEvent`, `fieldstg_goto_map(map, -1, x, y, dir)`).
  - **Tested already** (MECHANICS section 12, replay `first_battle_save`): a save at the Asuka Inn (`0xC01`) and a load
    after a reboot (`0xC00`) give back bytes 4..0x26C3 exactly (the playtime aside) and return to `field_map`
    (`0x20A`). The position after the load is not among that test's stated facts.
- **The sketch:** a port mod (4.5's rules: an `#ifdef PC_PORT` block testing a `port_mod_*` flag, the PS1 build
  byte-identical, off under `--script`). The menu gets one more option; choosing it leaves the field as the other
  options do but with `set_next_map(0xC00 + i, -1)`. The saves stay the game's own format: a card saved this way loads
  in the original game.
- **The first things to find out:**
  1. **What a load loses on a map that is not an inn.** After a boot `field_last_map` is 0, so the map counts as new:
     the map flags are cleared, `player_depth` is 4, `player_height` is `0x3000`, `meter_random_count` is 16, and
     `attr_layer` and `countdown` are whatever the title left. A save taken on a second attribute layer (WSTAG810),
     during the countdown (WSTAG795/800), at a height or mid-maze may load into a wrong or stuck state. Either refuse
     the option where it is unsafe, or carry those fields in the bytes the slot does not use (`0x26C4`-`0x26FF` of each
     0x2700-byte part are stale RAM today), which the original game would ignore.
  2. **Where the menu can be opened:** it already needs `progress >= 4` and no running event or busy actor (`:1676`);
     whether that is enough to keep a save out of every scripted state.
  3. **The menu itself:** five options, six with the card case (item `0x192`), drawn 14 pixels apart from
     `fieldmenu_option_messages` (messages of file `records_language + 0xB0`); the room for a seventh, and where the
     word "Save" comes from in the five languages (an existing string of the game's text, or the port's own).
  4. **The slot summary's names:** which `i` to pass (a neutral `?SHPNAM` entry, or none), and the area shown for the
     `0x3xx` maps.
  5. **The menu on map `0x1000`** (STSTATUS shows the same menu): the option must work or be hidden there, since
     `prev_map` is then `0x1000`, not the field map.
- **Tests when built:** a layer-3 round trip (`tests/saves/run.py`) from a map that is not an inn, with the position
  and direction compared before the save and after the load; the same card loaded in the emulator; the replays
  unchanged with the mod off.

## 6. The Windows track (decided: after the launcher works on Linux)

| # | Step | Effort |
|---|---|---|
| 1 | Linux: the arena symbols as macros, tags relative to the arena, no 16 MB alignment, no `-no-pie` requirement | S |
| 2 | Linux: script-free sections (objcopy rename, a per-object check, `overlays.ld` deleted) | M |
| 3 | `port_gen.py state`: sizes without `nm -S`/`.size` | S |
| 4 | The platform layer: disc I/O, paths, replace-rename, binary logs, `setvbuf`, the watchdog, pacing | M |
| 5 | `setup.sh` steps for llvm-mingw and a Windows SDL3; a CMake toolchain file; the PE markers | M |
| 6 | CI: the cross-build, then `new_game` under Wine with its log compared with Linux's | M |
| 7 | A test on real Windows: ASLR, Defender, audio, non-ASCII paths | S-M |

Steps 1-3 are refactors on Linux that `tests/port/run.py` and the `-m32`/`-m64`/sanitizer log comparison guard; after
them Windows is a platform-layer port, not a linker port. Risks: ASan's redzones with renamed sections; `longjmp` over
SEH frames; Wine in CI is not Windows; the scheme has not been tried on the real 388-unit build. The launcher (Dear
ImGui + SDL3) cross-builds the same way.

## 7. Phases

Each phase ends with a summary and waits for the user's go-ahead (CLAUDE.md).

| # | Phase | Contents | Test |
|---|---|---|---|
| 1 | Foundations in the game | `--config FILE` and the settings reader; the settings directory; memory cards in it by default (launcher runs); rebindable input tables and hotkey actions; `port/src/mods.c` and the manifests; the pace split from the nominal rate; a pause key and the watchdog off in play | The bare binary's logs and records unchanged (`tests/port/run.py`, `scripts/test.sh`); `--input-test`; a config round trip |
| 2 | Fast-forward; the 50/60 toggle | 5.1, 5.5 | 5.1's tests; a 60 Hz comparison with the patched game in the emulator |
| 3 | The launcher | 4.6 | Its screens on Linux; a first run from an empty directory to the game; a missing disc; the file dialog's fallbacks |
| 4 | Skip dialogues, then battle animations | 5.2, 5.3 (after the camera fix, issue #7) | `build.sh --check`; the "verify first" lists; replays with the mod on and their own expectations |
| 5 | Windows | Section 6 | Section 6, steps 6-7 |
| 6 | Skip intro; Global Saves; data-override mods | 5.4; 5.6; 3.7's sector-reader layer, `kind: data` manifests | To plan then |

## 8. Risks and open points

- Lowering the pace mid-run stalls the game unless the schedule is reset (3.2).
- A game global written after the snapshot breaks the reset check and is undone by a reset (3.5, 4.5).
- An auto-loaded configuration would make the tests depend on the machine (4.1).
- Saves are lost today without `--memcard1` (3.4): phase 1 fixes it for launcher runs.
- The watchdog kills a paused game (3.4).
- 60 Hz is untested end to end (3.5).
- Every statement about the dialogue and the battle is from reading the code: the hooks' sites and the "verify first"
  lists are the first work of phase 4.
- The Windows scheme is verified on test objects only (3.11).
- **Still to pick:** fast-forward's default speed (200 or 300 fps, or a multiple of the nominal rate); where the
  launcher's source lives; how the launcher reuses the disc check (linked code or a check mode of the game); the
  per-user directory's name.

## 9. Sources (web, fetched in session 18)

- Zelda64Recomp: [mod.toml template](https://github.com/Zelda64Recomp/MMRecompModTemplate/blob/main/mod.toml),
  [modding doc](https://hackmd.io/fMDiGEJ9TBSjomuZZOgzNg),
  [README](https://github.com/Zelda64Recomp/Zelda64Recomp/blob/dev/README.md)
- [UnleashedRecomp README](https://github.com/hedge-dev/UnleashedRecomp/blob/main/README.md) ·
  [Perfect Dark port README](https://github.com/fgsfdsfgs/perfect_dark/blob/port/README.md) ·
  [OpenGOAL progress report](https://opengoal.dev/blog/progress-report-aug-2024) ·
  [devilutionX modding wiki](https://github-wiki-see.page/m/diasurgical/DevilutionX/wiki/Modding) ·
  [Daggerfall Unity mod features](https://www.dfworkshop.net/projects/daggerfall-unity/modding/features/) ·
  [Factorio mod settings](https://wiki.factorio.com/Tutorial:Mod_settings)
- SDL: [SDL_ShowOpenFileDialog](https://wiki.libsdl.org/SDL3/SDL_ShowOpenFileDialog),
  [SDL_GetPrefPath](https://wiki.libsdl.org/SDL3/SDL_GetPrefPath),
  [SDL_CreateProcess](https://wiki.libsdl.org/SDL3/SDL_CreateProcess),
  [the MinGW toolchain file](https://github.com/libsdl-org/SDL/blob/main/docs/README-windows.md)
- [Dear ImGui's SDL3 backend](https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_sdl3.cpp) ·
  Nuklear's SDL3 backends ([issue 760](https://github.com/Immediate-Mode-UI/Nuklear/issues/760),
  [PR 825](https://github.com/Immediate-Mode-UI/Nuklear/pull/825)) · [RmlUi](https://github.com/mikke89/RmlUi)
- [PCSX2's portable mode](https://pcsx2.net/docs/configuration/general) ·
  [Dolphin's user directory](https://wiki.dolphin-emu.org/index.php?title=Controlling_the_Global_User_Directory)
- Windows: [the PE format](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format) (96 sections, 8,192
  alignment, 8-character names, `$` grouping) · [llvm-mingw](https://github.com/mstorsjo/llvm-mingw) ("LLD doesn't
  support linker script") · [binutils 2.36: dynamicbase by default](https://lists.gnu.org/r/info-gnu/2021-01/msg00015.html)
  · [`setvbuf`](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setvbuf) ·
  [`rename`](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/rename-wrename)
