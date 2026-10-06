# Host-side replay: findings

What differs between the original (the goldens, recorded from the game in PCSX-Redux) and the same C compiled for a
64-bit Linux host with the port's flags (`tests/host/build.sh`). `tests/host/replay.py` lists every mismatch; the ones
explained here are in `tests/host/known_mismatches.json` so the suite stays green until the port decides what to do.
A finding is never a reason to change `src/`: the matching build is the reference. The port handles each one in its
own code (a bounds check, a shim, a documented divergence).

State on 2026-10-05 (21 families, 3,437 cases, 5,333 calls): everything matches except findings 1, 2 and 7 (41 mismatching calls,
6, 34 and 1, listed by the 21 entries of `known_mismatches.json`); findings 3, 4, 6 and 8 agree by accident, 5 by the same struct
layout; finding 9 comes from reading the C (session 15), no golden reaches it. Session 16: the port build (`-DPC_PORT`)
handles 7, 8 and 9 (a-c) in `src/` under `PC_PORT`; the harness builds without `PC_PORT`, so the replay is unchanged.

## 1. `gamestate_check_party_stat` reads past `gamestate_party_stat_levels` (flag type 0x72, index >= 15)

- **Where:** `src/main/gamestate.c` `gamestate_check_party_stat(i, flag)` indexes `gamestate_party_stat_levels[i]`, a
  15-entry table, with the flag's 9-bit index unchecked.
- **Original:** the table sits at `0x80048CF8`, right before `gamestate_data` (`0x80048D34`), so index 15 and above
  read `gamestate_data` itself (index 31 reads its bytes `0x40..0x43`, index 63 `0xC0..`, index 100 `0x154..`). With the
  golden's fixture those bytes are 0, so the comparison is against 0 and `value == 1` holds.
- **Host:** the bytes after the table are whatever the host linker placed there; the six cases
  `get_flag_721f_*`, `get_flag_723f_*`, `get_flag_7264_*` return the opposite.
- **For the port: safe to bounds-check.** No data or code of the game uses a type 0x72 flag with an index >= 15, so the
  overrun is reachable only from a hand-made flag (the goldens' edge cases); the port may clamp or return 0 there and
  keep these six cases as documented divergences. Index 20 agrees by accident (both sides compare against a positive
  number). Evidence (2026-10-05, `tools/flag_census.py --type 0x72`, which re-runs all of it in ~2 s):
  - **Every reader of a flag word is known.** `gamestate_check_party_stat` is called only by `gamestate_get_flag`
    (`gamestate_set_flag` has no 0x72 branch), which is called only by `gamestate_check_flags` and through the
    `gamestate_flags` table. The table's callers in all of `src/` (EXE, the 21 tier-1 overlays, the 293 WSTAG files) and
    in the residual INCLUDE_ASM bodies (only `fieldstg_manager_update` uses it: the placed actors' lists) take flags
    from `FieldstgTalk.flags_required`/`flags_set` and `FieldstgPlacedActor.flags_required` lists, `FieldstgMapEvent.flag`/
    `flag_2`, `fieldstg_flag_events`, constants in code, and two computed values (the visited-map bit
    `get_map() + 0x1E00`, set side; STSTATUS's `(i & 0xFF) | 0x2000`, type 0x20). Event scripts, card scripts, text,
    battle data and the save are read by none of them.
  - **All of that data is C** (the build is byte-identical), and every `fieldstg_stage.actors`/`map_events` root, placed
    actor, talk list and flag-list pointer resolves to a parsed definition. Census: 16,169 flag uses (12,658 get, 3,511
    set) of 31 types; the computed types show up as expected (0x60 progress 1,946 gets, 0x70 conditions 1,746, 0x7E
    route 502, 0x80-0x88 items 1,787, 0x92 cards 70), and every bit-array index is inside its array.
  - **Type 0x72: 1,498 uses, all reads, indexes 0..14 only** (flags `0x7200..0x720E`, no `0x73xx`): 1,489 in talk lists
    and 8 in placed-actor lists of 45 WSTAG files, and one in code (`wstag280.c`, `get_flag(0x7201, 0)`). Per index
    (value 0 / value 1): 0: 24/71, 1: 29/124, 2: 28/136, 3: 24/96, 4: 44/104, 5: 29/43, 6: 42/126, 7: 9/35, 8: 37/132,
    9: 39/75, 10: 36/76, 11: 34/55, 12: 7/7, 13: 17/17, 14: 1/1 (the last threshold, 2472, only in `wstag725.c`).
  - **Backstops.** Every integer literal (hex or decimal, or either half of a 32-bit word) with a type-0x72 value in
    `src/`, `include/` and the residual asm (116 INCLUDE_ASM/INCLUDE_RODATA bodies and `data` segments) is a counted
    use or something else (file sizes, CLUT colours, addresses, start positions, a Shift-JIS string). A raw scan of the
    disc's 319 PRO files and the EXE for a word `0x7200..0x73FF` followed by 0 or 1 finds 1,497 in range (the data uses)
    and 24 out of range, all text bytes in SOUNDTST, STAGSLCT and the EXE, none of which read flags.
  - Not scanned, because no flag reader takes data from them: the non-PRO disc files (maps, sprites, text, sound).

## 2. `stfgtrep_raise_stats` reads past `stfgtrep_resist_gains` (resist class 5, RNG residue 3)

- **Where:** `src/stfgtrep/stfgtrep_80082E70.c` `stfgtrep_raise_stats(digimon, lv)`, for `lv <= 40`:
  `stfgtrep_resist_gains[class + pad_random.next() % 4]` with `class = records_digimon[digimon].resist_gains[i]`. The table
  has 8 entries (`{0, 0, 0, 1, 1, 1, 2, 2}`) and the index reaches 8 when the class is 5 and the draw's residue is 3.
  Two party Digimon have a class-5 resist: Digimon 0 (Kotemon, Water, `resist_gains[1]`) and Digimon 7 (Dark, `[6]`).
- **Original:** the table ends the overlay file (`STFGTREP.PRO` is `0x65F8` bytes and `stfgtrep_resist_gains` ends at
  `0x80089288 + 0x20 = 0x800892A8`, the file's end), so entry 8 is the rest of the file's last sector, which is zeros
  on the disc and is loaded with it (`overlay_load_stage` copies whole sectors), and the gain is 0:
  one draw in four gives that resist nothing, on top of the three draws that give 1, 1, 2. The golden records it (the 17
  `stfgtrep_exp` cases with `add_exp` on Digimon 0 or 7 and `raise_stats` on Digimon 7 at RNG index 4000, levels <= 40).
  The oracle pads every file it writes to whole sectors for this reason (2026-10-05): before that it copied the file's
  bytes only, the word came from the overlay the previous family had left in the slot, and the golden held only because
  that was FIGHTSTG (a zero there); with `shop_rules`' STCRDSHP before it, the word was 999.
- **Host:** the word after the table is whatever the host linker placed there: the resist gets a garbage gain
  (`-6528` in this build).
- **Related, in bounds:** `stfgtrep_stat_gains[k][class + next() % 5]` with class 5 and residue 4 reads entry 9 of a
  9-entry row: the next row's first entry, and for the last row (levels >= 80) `stfgtrep_resist_gains[0]`, which follows
  it in both the original's and the host's layout, so the host agrees there (the Digimon 1 cases at levels >= 80). The
  port must keep the two tables adjacent or bounds-check both.
- **For the port:** emulate the original (a gain of 0 for the overrun) or bounds-check; either way the level-up draws must
  stay identical to the original's for the replay tests, so the draw count must not change.

## 3. `stgtrain_get_menu(14)` reads past `stgtrain_menus` (agrees on the host by accident)

- **Where:** `src/stgtrain/` `stgtrain_get_menu(menu)` keeps any menu 1..14, but `stgtrain_menus` has 14 rows (0..13),
  so menu 14 reads the 16 entries after the table, the start of `stgtrain_trainings`, and counts their non-zero IDs into
  `stgtrain_module.count`.
- **Original:** the golden `stgtrain_rules` case `menu_14` records what the original counts there (12 entries).
- **Host:** matches today only because the host linker also placed `stgtrain_trainings` right after `stgtrain_menus`; a
  different layout or compiler would read something else. Not in `known_mismatches.json` (no mismatch).
- **For the port: safe to bounds-check** (2026-10-05): menu 14 is never passed. STGTRAIN's `map_entry` comes from three
  places, all in C: a trainer's talk flag `0x94xx` (`gamestate_set_flag`: map 0xA00, entry = the index), whose 28 uses have
  indexes 0..13 (`tools/flag_census.py --type 0x94`); STAGSLCT's stage list (entries 0..7); and field map changes, which
  pass -1 (menu 0; `fieldstg_manager_update`'s WIP C too). No event script `GOTO_MAP` targets 0xA00 (tests/README.md,
  "The overlay tour").

## 4. `stitshop_get_items(31)` reads past `stitshop_shops` (agrees on the host by accident)

- **Where:** `src/stitshop/stitshop_800859C0.c` `stitshop_get_items(shop)` checks `shop < 0` and a NULL list only;
  `stitshop_shops` has 31 entries (0..30).
- **Original:** `stitshop_funcs` follows the table, so entry 31 is `stitshop_funcs` itself: its `count` (the field the
  function sets) and its first function pointer (`stitshop_load_files`), returned as the goods list; the count is set to
  itself. The golden `shop_rules` case `get_items_31` records the count (77 before and after).
- **Host:** the 16 bytes after the host's table are alignment padding (zeros), so the host returns NULL and leaves the
  count alone: the same count, a different list (the list is an address, not compared).
- **For the port:** the shop is the field's map entry (`stitshop_main_create`: `get_map_entry()`). Shop 30 (all items) is
  STAGSLCT's debug entry `{0xF00, 0x1E}`, and its title already reads `stitshop_shop_names[30]` past that 30-entry table
  (UI, not tested). **Safe to bounds-check** (2026-10-05): the field opens STITSHOP only through a talk flag `0x7Axx` with
  index < 30 (`gamestate_set_flag`'s branch; 30 and up go to STCRDSHP or elsewhere), so the field passes 0..29; STAGSLCT
  passes 0..4, 0x1C and 0x1E. Shop 31 is never passed, shop 30 only by the debug list.

## 5. Overruns inside one struct (agree on the host because the struct layout is the same)

Not mismatches: the original and the host read or write the next field of the same struct, so they agree as long as the
port keeps the struct layout (the save struct `gamestate_data` must keep it anyway). Recorded so the port does not
"fix" them into a different behaviour, nor reorder these fields:
- `gamestate_set_flag(0x0608, v)` (golden `gamestate_actions`): index 8 of the 8-bit `flags_06` sets `flags_08` bit 0.
  The game's data never does this (`tools/flag_census.py --check`: every bit-array use is in range).
- `cardgame_deal_cards` with fewer than 6 cards left in the deck (golden `cardgame_rules`): reads past the deck's end
  into `hand[]`, which it has just written.
- `cardgame_mark_best_cpu_slot` (effect 0xAE) with fewer than 6 CPU slots picks slot `count`, an empty slot
  (golden `cardgame_cpu_choice`; MECHANICS 10): a rule to keep, not an overrun.
- `fightstg_enemy_turn_update` (FIGHTSTG, the enemy's turn): when none of the record's three `actions[]` conditions holds,
  the loop ends at `i = 3` and reads `actions[3]`, the 4 bytes at record + 0x3E (`unk_3E`). Every record of file 0x1CF
  has `{1 or 2, 0, 0}` there (187 attack, 6 `tech_2`), and 58 of the 193 records have no unconditional action among the
  three, so this is the game's default action, used often. A port must read it (declare `actions[4]`), not stop at 3
  (sweep4; the conditions themselves are golden `wfightmn_enemy_ai`).
- `fightstg_rules_get_stats` (FIGHTSTG, every combatant's stats block) indexes `GamestateStats.values[6..18]`: the
  `s16 values[6]`, `stats[6]` and `resists[7]` arrays read as one array (`&stats.values[6]`, `&stats.values[12]`).
  Reported by UBSan in the port's sanitizer run of `first_battle_save` (session 16); `tests/port/ubsan.supp` names it.
- `fieldstg_tile_upload` (FIELDSTG, a map tile's sprite lists): a row of 6 sprites writes `sprites[0][5]` of the
  `[3][5]` array, i.e. `sprites[1][0]`, which row 1's own loop rewrites next (seen 3 times in `first_battle_save`, always
  row 0). Reported by UBSan in the same run; in `tests/port/ubsan.supp`.

## 6. `fightstg_rules_roll_wake` without a sleep event reads `fightstg_events.events[-1]` (agrees on the host by accident)

- **Where:** `src/fightstg/fightstg_8008D3B4.c` `fightstg_rules_roll_wake(side, amount)`: `i = fightstg_events.find_member(0xC,
  ...)` and then `fightstg_events.events[i].delay / 100` with no check of `i == -1` (no sleep event for the member).
- **Original:** `events[-1].delay` is the s16 at `fightstg_events - 0x1A`, inside `fightstg_events_take_modes` (the table
  defined just before it): 0, so the wait term is 0. The `fightstg_rules` cases `wake_s0_a*`, `wake_s1_a*` (a sleeping
  member with no event) record that; `wake_event_*` (appended 2026-10-05) have the event and pin `wait = delay / 100`.
- **Host:** agrees today by accident: there the s16 lands in the top two bytes of the pointer in `fightstg_enemy_records`
  (placed before `fightstg_events`), which are 0 in this non-PIE build.
- **For the port: treat -1 as wait 0** (what the original reads). Sleep is set only by `fightstg_events_start_sleep`, which
  adds or refreshes the member's event 12 at the same time, so a sleeper without an event needs a full queue (99 events)
  or an event removed while the status stays (`fightstg_events_remove_member`, WFIGHTMN's switch handler; not traced).
  A port that bounds-checks the index must not crash on -1.

## 7. `wfightmn_battle_end` divides by zero when no enemy holds an item (traps on x86)

- **Where:** `src/wfightmn/wfightmn_800A6440.c` `wfightmn_battle_end`, at the battle's end (won or lost, `fightstg_events.result
  != 0`): `count` = the enemy members with `item != 0`, then `pick = pad_random.next() % count`. An enemy's `item` is set
  only from its record (`wfightmn_init_members`, when the record's item is non-zero; else it stays 0, as the first
  battle below shows) or to -1 by a steal, so a battle whose enemies all have record item 0 (101 of the 193 records) has
  `count == 0`.
- **Original:** no trap: GCC 2.8 emitted a bare `div` (no `break 7` check), and the R3000A's division by zero leaves the
  dividend in HI, so `pick` is the draw; the second loop then finds no holder, `count` stays 0, and `enemies[0].item > 0`
  is false: no item, one draw taken. Seen in the emulator: the story's first battle (layer-2 `first_battle_save`) executes
  it with dividend 2729 and divisor 0 (an exec breakpoint on the `div` at `0x800A7CF4`, sweep4).
- **Host:** compiled for x86, `% 0` raises SIGFPE: a port built from the C as it is crashes at the end of the first battle.
  Replayed since sweep5 by `wfightmn_spoils/no_holder` (the golden: item 0, one draw, substep 1): `replay.c` catches the
  SIGFPE and answers `trap`, so the case's return and two of its reads (`records_battle_results` with the old item,
  substep 0) mismatch and are listed in `known_mismatches.json`; the other 10 battle-end cases (one or two holders) match.
- **For the port: `count == 0` must give no item and still take the one draw** (`pad_random.next()` is called before the
  `%`, so the RNG index advances either way, and `first_battle_save`'s hashes depend on it).
- **Port (session 16): done.** Under `PC_PORT`, `wfightmn_battle_end` takes the draw and reduces it only when `count != 0`,
  which is the R3000A's result (`div` by 0 leaves the dividend in HI, any sign; `pad_random_next` returns a `u16`, so the
  dividend is never negative): `pick` is the draw, no holder is found, no item. The objdump of the PS1 object shows the
  bare `div zero,v0,s0` / `mfhi` with no `break 7`. The harness above builds without `-DPC_PORT`, so `no_holder` still
  traps there; built with `-DPC_PORT` for `wfightmn_800A6440.c` (a local experiment) all 24 `wfightmn_spoils` cases
  match, `no_holder` included.

## 8. A function defined `void` whose callers use its return value (agrees on the host by accident)

- **Where:** FIGHTSTG's object creators: `fightstg_fade_create` (`src/fightstg/fightstg_80086A00.c`) is defined `void` and
  ends on a store through its object pointer; `wfightmn_battle_end` declares it as returning `Fade *` and calls
  `->start` on the result (the comments at `src/wfightmn/wfightmn_800A6440.c` 72 and `src/fightstg/fightstg_80086A00.c` 187,
  `fightstg_8008B630.c` 34/61, `fightstg_8008D3B4.c` 2682 list the others: the command menu, the message box, the scenes).
- **Original:** the object stays in `v0` from `object_new` through the field stores (GCC 2.8 needs no other register),
  so the caller gets the object.
- **Host:** agrees by accident: the harness is built at `-O0`, where `rax` still holds the object after the last store
  (`wfightmn_spoils`' 11 non-trapping battle-end cases pass). With optimisation, or another compiler, the caller gets garbage.
- **For the port: give these creators their real return type** (return the object). The declarations that disagree are
  marked in the C, which keeps them as they are (tests never change `src/`).
- **Port (session 16): done.** `include/object.h`'s `OBJECT_V0(type)` (the return type: `void` on the PS1, `type` on the
  host), `OBJECT_V0_RETURN(obj)` (nothing / `return (obj);`) and `OBJECT_V0_TAIL(call)` (`call;` / `return call;`) give
  36 creators their object on the host; the PS1 build is byte-identical. Found by comparing every definition's return
  type with every declaration, every function-pointer cast and every `SLOT_FUNC` tag in `src/` and `include/`, and each
  checked in the PS1 objects (no write to `v0` after the last call: the object, or for `fieldstg_sprites_find_first` the
  entry `fieldstg_sprites_find_next` returns): FIGHTSTG's `fightstg_idle_camera_create`, `_player_reaction_create`,
  `_enemy_turn_create`, `_digivolve_create`, `_fade_create`, `_lights_create` (80086A00), `_defeat_camera_create`,
  `_attack_create` (8008B630), `_item_create`, `_tech_create`, `_scripted_turn_create`, `_camera_create`,
  `_command_create`, `_jump_create` (8008D3B4), `fightstg_stage_create`, `fightstg_intro_camera_create` (800A1FE0), whose
  callers are WFIGHTMN, WFIGHTTS and FIGHTSTG's other files; and the same pattern in FIELDSTG, called through tables:
  `fieldstg_choice_start_0..15` (`FieldstgEventDef.start`, cast to `s32 (*)(void)`; their result is
  `FieldstgEventData.started`), `fieldstg_icon_start` and `fieldstg_effects_start` (`fieldstg_script_objects`' starts,
  kept in `FieldstgEventData.script_objects`), `fieldstg_sprites_find_first` (`wstag.h` declares it returning the
  sprite; WSTAG790 uses it) and `wstag925_sprite_anim_create` (script object 855). Not one: `ststatus_equip_item`, cast
  to `s32 (*)()` in `ststatus_module`, whose result no caller uses. Still open: `FieldstgEventDef.start` returns `s32`,
  so a start's object reaches `started` as the low half of the host pointer, whole only while the arena lies below
  4 GB (the non-PIE build puts it at `0x2000000`); typing `start` and `started` as `Object *` under `PC_PORT` would end that.

## 9. Objects created with size 0, and data blocks that are not arrays of pointers (from reading the C, session 15)

Not replayed by a golden (no case reaches these creators); found by the M0 agents while sizing every `object_new` /
`object_create` call in `sizeof` units, verified here against the C.

- **Where:** (a) `wfightts_main_create` (`src/wfightts/wfightts_800A67B8.c` 449) creates its object with
  `object_create(wfightts_main_update, 0, 0, 0)`, while `wfightts_main_update` uses it as a `WfighttsMain` (0x6C bytes:
  `obj->menu`, ...) and writes its data block as a `WfighttsMainData` (0x2C bytes: `data->command = ...`) through
  `children`, which stays NULL (`data_size == 0` allocates nothing). (b) `stagslct_create`
  (`src/stagslct/stagslct_800849CC.c` 714, the documented bug: `object_new(stagslct_update, 0, 0)`) does the same for a
  `StageSelect` (0x70) and its `StageSelectData` (0x9C: the windows). (c) `object_destroy` (`src/main/object.c`) scans a
  data block as `child_count` pointers and stops every non-NULL one (`heap_objects.stop`: `set_state(END)` through it).
  `object_create` sets `child_count = data_size / 4` on the PS1 and, since this session, `data_size / sizeof(void *)`
  under `PC_PORT`, with the creators passing `sizeof(<T>Data)` (so a pointer-only block, e.g. `FightstgCommandMenuData`,
  is scanned right at either width). But about a dozen of the 124 `*Data` blocks mix other fields with the pointers:
  `FieldstgEventData` (`s32 started` first, then 13 pointers), `FightstgDigivolveData` (`unk_4` between pointers),
  `Wstag800Data` (`unk_18`), `StgdglabFormsetData` (`unk_9C` last), and the byte pads `unk_7C[4]` of `StageSelectData`,
  `unk_4[4]` of `FightstgEnemyTurnData`, `unk_00[4]` of `FightstgStatusData`, `unk_5C[0x10]` of `StstatusItemListData`,
  `unk_9C[8]` of `StstatusEquipPageData`.
- **Original:** (a) and (b) work by accident. `heap_alloc(0)` returns a block of 0 bytes (`heap_try_alloc` rounds the size
  to 0 and splits the free block right behind the header), so the 0x50-byte `Object` header and the type's fields are
  written over the next block's header and whatever follows it, and the data fields go to `0x0 + offset` (KUSEG's low
  RAM: no fault on the R3000). WFIGHTTS is a debug overlay (DISC_LAYOUT: a test battle, reached from STAGSLCT's stage
  list, MECHANICS "WFIGHTTS needs map 0x600"), and STAGSLCT the debug stage select: neither runs in normal play. (c) On
  the PS1 every 4-byte field is one scanned word, so the mix is harmless as long as a non-pointer word is 0 or a real
  object at destroy time (`FieldstgEventData.started` is an object pointer typed `s32`; the `unk_` words are 0 or
  unknown).
- **Host:** (a) and (b) are a heap overflow of `malloc(0)` and a write through NULL: a crash as soon as either overlay
  runs. (c) At `-m64` the mixed blocks are no longer arrays of pointers: `FieldstgEventData` has 4 bytes of padding after
  `started` and its first scanned "pointer" is `started` plus the padding; `FightstgDigivolveData.unk_4` and its padding
  make one bogus word; the byte pads shift every pointer after them by 4 into the middle of a scanned word. Whatever
  `child_count` says, `object_destroy` calls `set_state` through garbage.
- **For the port:** (a) and (b) need the real sizes (`sizeof(WfighttsMain)`/`sizeof(WfighttsMainData)`,
  `sizeof(StageSelect)`/`sizeof(StageSelectData)`); until then the port must not run WFIGHTTS or STAGSLCT as they are.
  For (c), either give those objects a `destroy` that knows the block's layout (stop the pointer fields by name), or keep
  the data blocks pointer-only (move the odd field into the object, give the pads a pointer-sized type). The matching
  build keeps the C as it is (tests never change `src/`).
- **Port (session 16): (a), (b) and (c) done** (each under `PC_PORT`, the PS1 build byte-identical).
  (a) `wfightts_main_create` passes `sizeof(WfighttsMain)`, `sizeof(WfighttsMainData)`; (b) `stagslct_create` passes
  `sizeof(StageSelect)`, `sizeof(StageSelectData)`. Their windows and FIGHTSTG objects are then children that run, which
  is what the code is written for (what the original runs instead depends on the heap block its 0-byte object overlaps).
  (c) The host's `Object` keeps the block's byte size (`data_size`, in the padding after `child_count`, so the header
  stays 0x80 bytes and no offset moves), and `object_destroy` finds the objects in the block instead of reading it as an
  array: in byte order, at each 8-aligned offset it reads 8 bytes, and a live object there is stopped and covers both
  4-byte words; otherwise each 4-byte word is tried alone, as a PS1-style heap address (an object kept with
  `PTR_TO_S32`) or as the low half of a host pointer (an object that came back through an `s32`, like
  `FieldstgEventData.started`; whole while the arena lies below 4 GB). "Live" (`object_live`) is checked in the arena
  only, deterministically: inside the heap, 4-byte aligned, the heap block's header neither free nor the terminator and
  linked both ways, and `object_create`'s `set_state` and `destroy` in the object. Every word the PS1 stops is a live
  object (stopping anything else calls `set_state` through garbage there), so the host stops the same objects in the
  same order; it skips what the PS1 would crash on, and an object destroyed earlier whose pointer stayed in the block
  (the PS1 would destroy it a second time). A scratch test (object.c and heap.c with `-DPC_PORT`, ASan/UBSan) built
  blocks with the layouts of `FieldstgEventData` (an object in `started`), `FightstgDigivolveData`, a byte pad plus a
  PS1-style address, and non-object words, and got the PS1's children in the PS1's order, no report. The block is freed
  when `data_size != 0` (the PS1: when `child_count != 0`; a 1..3-byte block, which no creator passes, leaked there).
  Not changed: `heap_run_children` (`heap.c`) still runs the block as `child_count` 8-byte slots, which is right for
  these blocks as long as their non-pointer fields are 0 while the object runs (the PS1 runs every non-zero word as an
  object each frame, so they must be) and `started` is whole in its slot.
- **(d) Literal data sizes (session 16).** At least 122 `object_new`/`object_create` calls still pass the data
  block's size as a PS1 byte count (`grep -rnE "object_new\([^,]+,[^,]+, *(0x[0-9A-Fa-f]+|[1-9][0-9]*)\)|object_create\([^,]+,[^,]+, *(0x[0-9A-Fa-f]+|[1-9][0-9]*)," src`:
  72 in WSTAG files, 27 in FIELDSTG, every overlay's root object `object_new(..._update_root, sizeof(Object), 4)`,
  `message_create_cursor`, `fieldstg_dialog_create` (4), `fieldstg_manager_create` (0x7C), `fieldstg_choice_start_*`
  (0x14), ...). Where the block holds pointers, the host block is half the size the C writes (`*data = child` in a
  4-byte block overruns into the next heap block's header) and `child_count = data_size / 8` runs half of the children,
  none for a 4-byte block: CNTY_SEL's root (`cnty_sel_start`) never runs its menu. Each needs its host size under
  `PC_PORT` (`N * sizeof(void *)` for N pointers, or a `sizeof` of the block's type); `object_destroy` above scans only
  the bytes allocated, so it stays safe either way.
- **Port (session 16): (d) done**, in plain C that has the PS1's value (no `#ifdef`; the PS1 build is byte-identical).
  The rule: a data size is `sizeof(<T>Data)` where the block has a type, `sizeof(T *)` or `N * sizeof(T *)` for a block
  of N pointers (T: what the update's second parameter points at; `Object *` for a block nothing writes), and an
  object size is `sizeof(<T>)` or `sizeof(Object) + N` (N unused bytes); a literal that is a true byte count stays and
  is marked `PC_PORT: bytes` (none is). What each block holds was read from its update function and the code it calls.
  The 122 calls: 72 in WSTAG (45 `WstagEventData` updates whose block is only `event`, 4 bytes: `sizeof(FieldstgEvent *)`,
  not `sizeof(WstagEventData)`, which is 8, and one `FieldstgEvent **`; 24 of `Object **`/`WstagAnimObject **`/
  `WstagLiftObject **` or with no data parameter: `sizeof(T *)`; WSTAG355 0x50 and WSTAG934 8, unused: `20 *` / `2 * sizeof(Object *)`; the copies of a
  `--funcs` group all got the same expression), 27 in FIELDSTG (`sizeof` of the existing `FieldstgChoiceData` x16,
  `FieldstgStageData`, `FieldstgBackgroundData`, `FieldstgMapTitleData`, `FieldstgMapEventsData`, `FieldstgManagerData`,
  `FieldstgSpotsData`; `sizeof(MessageDialog *)` x2, `sizeof(FieldstgSpots *)`; `fieldstg_start`'s 0xC: 3 manager
  pointers of which only `[0]` is used; `fieldstg_actor_update`'s 0x10: `4 * sizeof(void *)`, slots 0-3 all used), and
  23 elsewhere: the 15 overlay roots (`sizeof(CntySelMenu *)`, `sizeof(StcrddekMain *)`, ...), `message_create_cursor`
  (`sizeof(MessageCursorData)`), CARDGAME's deck window, board and fade (`sizeof(CardgameDeckWindowData)`,
  `sizeof(CardgameBoardData)`, `sizeof(Object *)` unused), `stcrddek`'s editor (`sizeof(StcrddekEditorData)`),
  `stdgname_party_create` (0x28: `10 * sizeof(MessageWindow *)`, windows 0-5 used), `stgmcard_contents_create` (0x4C:
  `19 * sizeof(MessageWindow *)`, one per `stgmcard_contents_windows` entry), `fightstg_entrance_create` (0xC:
  `FightstgEntranceData` grows the two words nothing writes, `Object *unk_4[2]`, to its 0xC). Also: the object sizes
  0x54 (`fieldstg_loader_create`) and 0x58 (`stgmcard_create_root`) are `sizeof(Object) + 4` / `+ 8` (the host's
  0x54 bytes were smaller than its 0x80-byte `Object`), `fightstg_model_new`'s `(data[1] + 2) * 4` is
  `* sizeof(void *)` (`FightstgModelData`: `texture_anim`, then a mesh per part), and `overlay_create_object`'s
  `sizeof(s32)` is `sizeof(Object *)`: its block holds the stage's root object, which `overlay_run_object` now stores
  as an `Object *` (on the host it calls the entry as returning `Object *`: the table's `s32 (*)(void)` truncated it),
  and the five entries defined `void` (`fieldstg_start`, `fightstg_entry`, `cardgame_start`, `stgmcard_create_root`,
  `stdwtitl_create_root`) return it (`OBJECT_V0`, as in 8). The mixed blocks (pointers and other fields:
  `FieldstgEventData`, `FightstgDigivolveData`, `FightstgEnemyTurnData`, `FightstgStatusData`, `StageSelectData`,
  `StcrddekNameWindows` (`u8 unk_14[0xC]`, not in the list of (c): no code writes it), `StgdglabFormsetData`,
  `StstatusEquipPageData`, `StstatusItemListData`, `Wstag800Data`) keep their layout on both sides: the host block is
  `sizeof` of the host's struct, so large enough, `object_destroy` (c) finds their objects by scanning the bytes, and
  `heap_run_children` runs them as 8-byte slots, right while their non-pointer fields are 0 (as the PS1 needs them to
  be: it runs every non-zero word). The class stays closed: `tools/port_inventory.py object-sizes` lists every call
  with a bare literal object or data size (0 data and the PS1 side of `#ifndef PC_PORT` allowed, `PC_PORT: bytes`
  exempt) and exits 1 if any: 0 of 663 calls. Also on the host (`src/main/heap.c`): blocks are rounded to 8 bytes
  (`HEAP_ALIGN`; 4 on the PS1), so every block's data is 8-aligned behind the 24-byte header (UBSan's misaligned
  `FieldstgActor`/`MessageWindow` accesses on the new_game path are gone). With these, `build/port/dw2003 --disc`
  with START/UP/CROSS taps goes CNTY_SEL, 0xE02, 0xE00, New Game, FIELDSTG map 0x2D7 (WSTAG780) and runs there; two
  more host faults on that path were fixed in `fieldstg_80087DB0.c`: `fieldstg_loader_load_action_anims` walks
  `fieldstg_stage.map_events`, NULL for WSTAG780 (no map events; the PS1 reads the end marker's 0 from low RAM at
  0x00000008, 0 there in the emulator), and `FieldstgBackgroundView`'s byte pad put `get_size` at the PS1 offset, not
  the host's (`FieldstgBackground` has two pointers before it).

## 10. `stplnmet` trims the name from index 19 of a `u16[12]` (agrees on the host by layout; session 16)

- **Where:** `src/stplnmet/stplnmet_80083D70.c` (the name entry's confirm, key code 0x67): once the name has a
  non-space character, `for (j = 19; j >= 0; j--)` walks `StplnmetNameEntry.name` (`u16[12]`, `include/stplnmet.h`)
  from index 19 down, zeroing trailing spaces and stopping at the first non-space. Indexes 12..19 are the halves of
  the four `s32` after the array (`name_cursor`, `length`, `key_column`, `key_row`).
- **Original:** `name[19]` is the high half of `key_row` (0 in play), which is not a space: the loop ends at once with
  step 100, and nothing is written. The trimming of the name's own trailing spaces never runs.
- **Host:** the same fields follow `name` in the same order and sizes, so the host reads the same half-word and does the
  same. Reported by UBSan (index 19 out of bounds for `u16[12]`) in the port's sanitizer run of `first_battle_save`'s
  registration (T3, session 16); `new_game` does not reach it.
- **For the port:** nothing to change while the layout after `name` stays as it is; a port that reorders the struct must
  keep the PS1 behaviour (no trim, step 100).

## What the host does not replay (by design, not findings)

- Stack leftovers: `replay.c` zeroes 64 KB of stack before every call, so a local the C reads uninitialised is 0 on the
  host. The one known case is `cardgame_cpu_get_score` on an empty side (`list[0]` indexes the slots once, the value is
  discarded; MECHANICS 10): with leftovers there the harness crashed as soon as another unit (FIELDSTG's) was linked in.
  The original reads its own stack's leftovers; no golden depends on them. A port should not read them at all.

- Writes to raw addresses: the overlay in the slot (`FIGHTSTG.PRO`, `CARDGAME.PRO`) and the data files placed in RAM.
  The host links the same C natively and serves the files through its own `cdload_module.files.get_file` from the
  cdload entry writes (`replay.py` turns them into file registrations).
- Pointer-bearing structs are wider on an LP64 host. Fixture offsets are chosen in the pointer-free part of each struct
  (`gamestate_data` before `.funcs`, `fightstg_battle.state`, `fightstg_rules.stats`); the one fixture buffer of such a
  struct (`CardgameGame`, whose `Object` header holds pointers, and whose `opponents` at 0x2F0 is a struct pointer) is
  remapped with the offsets `layout.c` reports, and so are the reads of it (`buf:game` past the header; `replay.py`
  `BUFFER_LAYOUTS`: a pointer field in the tail splits it into segments).
  A 32-bit host build (`gcc -m32`, needs `gcc-multilib`) would need none of that.
- UI calls inside a rule (`ststatus_items_use`, `ststatus_tech_use`): the goldens give the page's windows methods that do
  nothing (every word of a `window` buffer is `heap_nop`'s address, the data block points every window at it); the host
  builds the same from host pointers (`replay.py` `POINTER_BUFFERS`, the `P ... @nop` command) and a `sound_module` whose
  `play` does nothing (`shims.c`). Only what the rule writes to `gamestate_data` and the RNG is compared.
