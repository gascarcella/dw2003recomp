# Reference tests

Tests that pin down what the original game does, so that the PC port (`port/` in this repository; DECISIONS "Going public: the working repo is public")
can be checked against it. The direction is DECISIONS "Reference tests: three layers"; the status of
each layer is in `docs/STATUS.md`.

## The three layers

| Layer | What | Where | Oracle |
|---|---|---|---|
| 1 | **Golden tests of pure logic:** `fightstg_rules_*`, the card game's CPU and rules, `gamestate_*`, `records_*`, digivolution, experience and training (`stfgtrep_*`, `stgtrain_*`), items and shops, `memcard_get_checksum`, `pad_random`, the encounter redraw, the battle's event queue, the enemy's choice of action, the battle's spoils, `sprite_draw`, the GTE's commands, LIBGS's view and the GPU's drawing (25 families, table below). A fixture fills the globals a function reads, the golden records what it returned and wrote. | `tests/golden/` (fixtures and goldens, JSON, one file per function family), `tests/host/` (the host-side replay the port uses) | The game's own code run in the emulator (PCSX-Redux, Lua-driven): resolves the UB the way the original did. |
| 2 | **Record/replay:** a pad script run from boot; at named checkpoints the SHA-1 of `gamestate_data` (the pointer-free save struct, `0x80048D34`, `0x275C` bytes), the frame, map and `pad_random.index`; plus the overlay load sequence and the map sequence. | `tests/replay/` (`run.lua`, `replay.py`, `scripts/*.json`, `expected/*.json`) | The disc in PCSX-Redux with OpenBIOS. The port replays the same script through its pad shim and compares at checkpoints only (CD latency differs, so frames do not). |
| 3 | **Formats and save round trips:** the `docs/FORMATS.md` verification scripts; `.mcd` saves exchanged between emulator and port. | `tests/formats/` (`run.sh` over the verification tools in `tools/`), `tests/saves/` (`run.py`, `cards.py`) | The disc files and the emulator's memory cards. |

## Rules

- **Tests never change the matching build.** `scripts/build.sh --check` must stay byte-identical: tests live in
  `tests/` and `tools/`, never in `src/`, `include/` or `config/`. A test that needs a change in `src/` has found a bug;
  prove it with a golden and ask before touching the source.
- **One entry point:** `scripts/test.sh` runs every layer that is available on this machine (emulator and disc present,
  host compiler present) and exits non-zero on the first failure. It runs headless with `DW3_JOBS=1`, as cloud sessions do;
  with `DW3_JOBS=N` the layer-2 replays and the mods' tests run N at a time (CI: 4). `--layer` picks layers (`1`, `2`, `3`,
  `port`, `mods`), `--m32` adds the port's -m32 build to the M1 test.
  Before the layers it runs the PC port's host-compile gate (`tools/port_inventory.py probe` and `link`, ~5 s: every unit
  compiles at `-m64` with no pointer/int cast, implicit declaration or incompatible pointer type, and no global is defined
  twice; `--no-probe` skips it). The gate needs only the host gcc and nm (no disc, not even `include/asm_generated/`), so
  CI also runs it in its disc-free part (`.github/workflows/ci.yml`, Ubuntu's GCC 13).
- **Every golden and expected file is tagged** with the commit of the matching tree that produced it, the emulator build
  (`tools/redux/app/.../version.json`: `bf4c9ceb`, build 359) and the BIOS (name and SHA-1). Bumping the emulator is a
  decision (DECISIONS "Project-local, pinned toolchain"); OpenBIOS is the default and the retail BIOS a cross-check only, never the
  source of a committed file. A second generation run must reproduce a committed file exactly.
- **No emulator savestates, memory cards or disc data are committed.** Scripts start from boot; the driver gives each run
  fresh empty memory cards in its output directory. Scratch output goes to a temp dir (or `--out`).

## Layer 1: golden tests

`tests/golden/oracle.py gen [families]` runs every case of every family in **one emulator boot** (about 30 s) and writes
`tests/golden/<family>.json`; `oracle.py check` regenerates and requires the committed file to be reproduced exactly
(that is what `tests/golden/run.sh`, hence `scripts/test.sh`, runs); `oracle.py list` names the families. `--bios retail`
is a cross-check for `check` only; `gen` refuses it (DECISIONS "Layer-1 goldens: calls on the running game").

The oracle is the game itself (DECISIONS "Layer-1 goldens: calls on the running game"): `tests/golden/oracle.lua`
boots the disc with `-debugger -interpreter`, waits until CNTY_SEL is loaded (so the EXE's code and data are resident and
idle), then an exec breakpoint on `pad_update` hijacks the main loop: for each job it saves the regions a job may write,
applies the fixture and the scratch buffers, loads `a0..a3`, sets `ra` to a sentinel address (`0x80010000`, the EXE's
`.rodata`, never executed) and `pc` to the function; the sentinel breakpoint records `v0`/`v1` and reads back memory;
the saved regions are restored after the job. Scratch buffers live at `0x80180000` (free heap above the overlay slot,
restored byte for byte).

A family is a module in `tests/golden/families/<family>.py` with `cases(sym)` returning `Case` objects (`oracle.py`
docstring): a fixture (bytes written at `symbol + offset`, named by field), buffers, and calls (`func`, `args` as integers,
`("buf", name)` or `("symbol", name, offset)`, a `ret_type` of `u8 s8 u16 s16 u32 s32 void`, and `Read`s of memory after
the call; reads over 256 bytes are stored as SHA-1). Every case is independent: the game's state after a job is the state
before it. A golden file is `{family, comment, generator, oracle: {method, emulator, bios, tree_commit, resident},
fixtures: {name: [...]}, cases: [{name, comment, fixture, buffers, calls: [{func, args, ret_type, ret, v0, reads}]}]}`;
`check` compares everything but `oracle.tree_commit` and `oracle.bios`.

| Family | Functions | Cases |
|---|---|---|
| `pad_random` | `pad_random_seed`, `pad_random_next` | the table's SHA-1 (`0x2000` bytes) after `seed(0)`; 11 seeds (`0`, `1`, `2`, around `0xFFF`, large, `-1`) × 32 draws with the index after each (12 cases, 364 calls) |
| `memcard_checksum` | `memcard_get_checksum`, `memcard_check_checksum` | 9 byte patterns (empty, one byte, zeros, `0xFF`, incrementing, LCG, a save-slot-sized 0x26C0) × the right sum, sum ^ 1, 0, `0xFF` (9 cases, 45 calls) |
| `gamestate_flags` | `gamestate_get_flag`, `gamestate_check_flags`, `gamestate_check_condition` | one fixture (the whole pointer-free `gamestate_data` zeroed, then patterned flag arrays, progress 20, items, cards, route, party, three Digimon); every flag type × 11 indexes × value 0/1 (506), 7 flag lists, every condition id of `gamestate_conditions` except the money (0x20) and object (0x50) types × value 0/1 (184), one unknown id; `gamestate_data`'s SHA-1 (up to `.funcs`) after each call (697 cases) |
| `fightstg_rules` | `fightstg_rules_get_damage`, `_get_special_damage`, `_get_heal`, `_get_regen`, `_get_poison_damage`, `_get_counter_damage`, `_get_field_bonus`, `_roll_hit`, `_roll_special_hit`, `_roll_critical`, `_roll_special_critical`, the ten status rolls, `_roll_steal`, `_roll_escape`, `_roll_counter`, `_roll_wake`, `_roll_confused`, `_roll_paralyzed`, `_change_modifier`, `_get_gauge_gain`, `_get_tech_cost`; `wfightmn_roll_first_strike`, `wfightmn_cap_damage`, `wfightmn_add_gauge`, `wfightmn_mark_took_part` | a kept setup case puts `FIGHTSTG.PRO` in the overlay slot and `SDIGIEDT.PRO` (the enemy records) in RAM as a loaded cdload entry; one battle fixture (two party members, two enemies; `tests/golden/families/_battle.py`), cases over techniques × sides × RNG indexes, with per-call writes for buffs, fields, crests, statuses, boss, blocked effects; reads back `fightstg_rules.stats` and the RNG index; appended (2026-10-05): a successful steal, its ratio cap and the Hack crest, the enemy's hit floor 0x20 and the party's missing floor, each at both sides of its threshold, waking with a sleep event (`wait = delay / 100`), and two WFIGHTMN rules with `WFIGHTMN.PRO` in the tier-2 slot (restored after each case): `wfightmn_roll_first_strike` (no draw at chance 0, the threshold, residue 0 always first) and `wfightmn_cap_damage` (battle types 1/2 keep the enemy above `max_hp / 11`, per hit; type 3: the party deals 0); appended by sweep2: `wfightmn_add_gauge` (only a hit on the party with a value, on a live, unblasted acting member; into `records_state.gauges[Digimon index]`; at >= 1000 set to 1000 and a gauge-full event 0x12 queued: 999 / 1000 / capped) and `wfightmn_mark_took_part` (took_part, and the form's place in the packed chosen list: chosen {386, -1, 367} fighting as 367 marks forms[1]) (1,397 cases) |
| `cardgame_cpu` | `cardgame_cpu_get_score`, `_get_card_damage`, `_get_card_heal`, `_is_target_within`, `_find_kind` | a kept setup case puts `CARDGAME.PRO` in the slot and the card files `0x7F6`/`0x7F7` in RAM; a `CardgameGame` scratch buffer per case: boards of plain and combinable cards with masks, the damage/heal tables for IDs 0..64, hands (182 cases) |
| `gamestate_records` | `gamestate_set_stat`, `_add_stat`, `_add_stat_bonus`, `_get_stats`, `_change_money`, `_check_condition` (the money type `0x20`), `_change_item`, `_unequip_item`, `_change_card`, `_add_card`, `_init_cards`, `_find_form`, `_add_form`, `_list_forms`, `_get_chosen_forms`, `_set_chosen_forms`, `_get_form`, `_put_form`, `_get_party_member`, `_get_party_digimon`, `_set_party`, `_tick_playtime`, `_reset_playtime`, `records_list_items` | one fixture (pointer-free `gamestate_data` zeroed; party {0, 3, 5}; four records with stats, penalties, equipment and forms; items, cards, 5,000 money): every stat index × 12 values and the `add_stat` s16 wrap; every bonus type; `get_stats` with a sword, armour, a gem, a two-handed weapon in both slots, key/usable items, penalties past zero, the item set and two near misses; money × every table index; items (counts, the equipped branches), cards, forms (duplicates, the 44 limit, an id below 3), parties, playtime carries, item lists owned/equipped (163 cases, 1,004 calls) |
| `records` | `records_find_digimon`, `records_get_item_icon`, `records_is_item_category` | SHA-1 of `records_digimon`, `records_techniques`, `records_item_icons` in RAM (not `records_items`: pointers); every Digimon id and 6 unknown ones; the icon (type) of all 402 items; 25 items × categories 0..6 (29 cases, 639 calls) |
| `stfgtrep_exp` | `stfgtrep_add_exp`, `stfgtrep_raise_stats` | a kept setup case puts `STFGTREP.PRO` in the slot; Digimon 1 (exp curve 10): exp around the thresholds 12 and 36, several levels in one call, the 999,999 cap, levels 97-99; Digimon 0 and 7 at two RNG indexes; `raise_stats` for three Digimon × 13 band-edge levels × 3 RNG indexes, and at the caps. Reads the record, the RNG index (draws taken) and `gamestate_data` (169 cases) |
| `cardgame_cpu_choice` | `cardgame_cpu_choose_card`, `_can_use_effect`, `_choose_target`, `_choose_counter_target`, `_keep_best_target`, `_filter_targets`, `_set_target`, `_set_target_kind_3`/`_4`, `_set_cheapest_target`, `_get_score` (empty side), `cardgame_cpu_choose_card_by_value`, `_choose_own_deck_card`, `_choose_player_deck_card`, `cardgame_mark_best_cpu_slot`, `cardgame_cpu_select_cards` | its own kept setup (as `cardgame_cpu`); a `CardgameGame` per case with cards placed by ID (indexes 0..39 the player's, 40..79 the CPU's), board argument 0: every use effect 0x70-0x97 on an empty and a full board plus two colour-masked boards; each `choose_target` arm found/not found; the helpers' ties and fallbacks; `choose_counter_target` per target kind × side (damage/heal looked up by card index); `choose_card` per decision (phase 5 kind 1, phase 7 behind the swap and score gates, the counter list, the record-kind 3/9 answers); the CPU's picks inside effects; appended: `cardgame_mark_best_cpu_slot` (effect 0xAE: the slot whose loss costs least, an empty slot when the CPU has fewer than six) and `cardgame_cpu_select_cards` (effect 0x9D, phase 6: greedy against the points left, kind 5 skipped, the cap of 6; a stub board of `heap_nop` methods). Reads the written fields and a SHA-1 of the `CardgameGame` past its header (155 cases) |
| `cardgame_rules` | `card_init`/`card_select`/`card_get_class`, `cardgame_can_play_card`, `cardgame_get_card_data`, `cardgame_check_condition`, `cardgame_run_effect` (board 0), `cardgame_game_resolve_effect`, `_apply_bonuses`, `_sum_slots`, `_find_combo`, `_end_round`, `cardgame_shuffle_deck`, `cardgame_sort_cards`, `cardgame_game_sort_cpu_hand`, `_set_cpu_deck_limits`, `_reset_cpu_deck`, `cardgame_restore_cpu_deck`, `cardgame_game_place_cards`, `cardgame_deal_cards`, `cardgame_player_add_card_point`, `cardgame_slot_to_discard`, `cardgame_slot_to_hand` | kept setup with the card files `0x7F6`/`0x7F7`/`0x7FA` (the last for colour-6 Digimon); the class per record class byte; playability per phase × class and the phase-6 points; the card data fields; every condition kind true/false (kind 10 only for target kinds 1-3, the others call the board); the script control effects 0x02-0x11 at their bounds, 0x2A, the mark/choose effects and the draw marking; the interpreter's steps that need no board; the five bonus cards and the 0..99 clamp; combinations; the round winner steps; the shuffle at RNG indexes; the CPU deck's stage bookkeeping; appended: `cardgame_deal_cards` (a fresh deal; a held hand and a deck shorter than 6, read past its end) with a stub board, the deal's points (`cardgame_player_add_card_point`: a colour 1-5 card's point, the 99 cap, colour 6 none) and a slot card back to its owner's pile, not its side's (`cardgame_slot_to_discard`/`_to_hand`) (193 cases) |
| `stfgtrep_forms` | `stfgtrep_learn_technique`, `_add_technique_exp`, `_learn_skill`, `_mark_skill`, `_get_technique_exp`, `_member_get_exp` | the digivolution rules (a "technique" in the C is a form): a kept setup puts `STFGTREP.PRO` in the slot; Digimon 0's form list at each requirement boundary (Digimon level, form level, a stat, a resist, two forms), the first eligible entry, all owned, the chosen-slot packing, a full form table, Digimon 7's list; form exp at the x10/x50 thresholds (t5 60, 95, 100), several levels, level 99 (exp not stored), the cap; skill slot order and the 0x8000/0x2000/0x4000 flags; battle form exp over level bands, user counts, the 1/10/50 limits; the 0x141 exp item (62 cases) |
| `stgtrain_rules` | `stgtrain_session_raise_stat`, `_lower_stat`, `_raise_stat2`, `_raise_max`, `_get_bonus`, `stgtrain_get_menu`, `stgtrain_session_apply_round` | training: a kept setup puts `STGTRAIN.PRO` in the slot; a `train_session` buffer pointing at a `train_main` buffer (trainer level); every gain row of stats, resists and max HP/MP with RNG residues, all 15 resist-table slots (5 classes x 3 stat sizes) and the size edges, the 999/9999 caps, the loss's odd/even draw and floor, out-of-range types; the 0x151/0x152 accessories; the menu index rule (menu 14 reads past the table); appended: `stgtrain_session_apply_round`'s dispatch (a stat; a resist with a stat loss or a max HP/MP raise as its "loss"; a lost and an unplayed round; a map outside the menus) with stubbed result windows and the text file loaded by a second kept setup (108 cases) |
| `shop_rules` | `stitshop_get_items`, `_can_equip`, `_get_slot`, `_equip`, `_info_get_stats`, `_info_add_stat`; `stcrdshp_get_price`, `stcrdshp_find_shop` | the shops' pure helpers: kept setups put `STITSHOP.PRO`, then `STCRDSHP.PRO`, in the slot; shop goods counts (shop 31 reads past the table), the members bit, every slot kind and every hand/ring/crest branch of the slot choice, equipping with and without the bag counts (two-handed weapons, crest groups, unequip), the shop's stats preview (no set bonus, caps, penalties); card prices (table edges, absent cards -> 1) and the card shop lookup (unknown ID -> shop 0, the count) (49 cases). The buy/sell counts and money are UI state machines (layer 2) |
| `ststatus_items` | `ststatus_items_use`, `ststatus_calc_heal`, `ststatus_tech_get_usable`, `ststatus_tech_use`, `ststatus_tech_collect`, `ststatus_can_equip`, `ststatus_list_items`, `ststatus_equip_item` | items and healing techniques from the status menu: a kept setup puts `STSTATUS.PRO` in the slot and the menu text (`0xB2`) in RAM; the page's windows are stubs (`window`/`windows` buffers of `heap_nop` pointers); HP heal (partial, capped, exactly full, at/above max, from 0 HP), TP (+5, the 99 cap, at/above 99), all 15 stat-raise effects with mod-10/mod-30 residues and the 999/9999 caps (no draw at the cap), a battle-only item, the s8 bag count; the heal formula for 0xB8..0xBC with equipment; one-member and party heals (capped, all full, member count, a -1 slot, MP not checked); the healing-technique list (flags, duplicates, range edges, the 5 limit, a stale list); the equipment page's can-equip, slot lists and equip (67 cases) |
| `gamestate_actions` | `gamestate_set_flag`, `_set_flags`, `_update_map_flags`, `_cond_object` (via `gamestate_check_condition` too) | what the event scripts write, over `gamestate_flags`' fixture: every bit-array type set and cleared (the bit's old value the opposite), an index past its array (`0x0608` sets `flags_08` bit 0), value 2, the types that write nothing (0x60, 0x7E, 0xA0), a condition (0x70, value 1 whatever is passed), items via 0x80 and 0x8E, a card; a list and an empty list; map flags on a new map and back from a won/lost card game; condition 0x13 (type 0x50) on a scratch `heap_objects` list: the first kind-0x16 object gets `set_step(3)`, other kinds, empty slots, a second match and slot 99 (50 cases) |
| `stgdglab_party` | `stgdglab_main_pack_party` | entering the Digivolution Lab: a kept setup puts `STGDGLAB.PRO` in the slot; the party compacted (members keep their order, holes last) for a full party, holes at the front, middle and both ends, -2 as a hole, an empty party; `member_count` (7 cases) |
| `sprite_draw` | `sprite_draw` (no scale or rotation) | the SPRT path into a scratch packet buffer (`gfx_module.packet`, `sprite_current` pointed at scratch buffers): one opaque cell, the frame search by list position with three cells drawn last to first and the CLUT row sums, a texture-page change (4-bit u 0/256, 8-bit u 200) with its DR_TPAGEs, blend modes 0 and 2; the reads skip each primitive's link address (4 cases) |
| `fieldstg_encounter` | `fieldstg_encounter_reset` | the random-encounter timer's redraw (`FIELDSTG.PRO` copied into the slot per case, restored after it): `r = next() % 2304`, `r < 256 ? r : (r + 256) / 2`, at RNG indexes whose draw is 254 and 257 (the two branches agree at 255 and 256, so these pin the threshold), 2303 (the top, 1279), 2304 and 4095 (the `% 2304` wrap) (5 cases) |
| `wfightmn_enemy_ai` | `fightstg_enemy_check_condition`, `fightstg_enemy_get_action`, `fightstg_enemy_turn_find_target` | the `fightstg_rules` fixture (imported; its own kept setup case); named to run after `gamestate_flags` (sweep4, 2026-10-05): the enemy's action conditions at their boundaries (the roll `next() % 128 < value` at 31/32; the HP thresholds `(s16)(max_hp / 100) * (value * 100 / 128)`, both truncated: max_hp 450, value 26 -> 80, so hp 80 is not under (an exact 91 or 90 would be), max_hp 99 -> 0; the complement `<=`; the party's member; the party Digimon among `records_digimon[0..7]`, index 7 in, 8 out; another live enemy by Digimon, the current slot excluded, value 0 = any; `turns % value` at turn 0), the action types (2/3 the current enemy's record's techniques, 4..8 -> -1..-5), and the called member (refused when dead or the current member's Digimon: an ID compare; -5: one candidate without a draw, two by `next() & 1`); a `FightstgEnemyTurn` buffer (`enemy_turn`) carries the action (21 cases) |
| `wfightmn_events` | `fightstg_events_get_delay`, `_take_next`, `_add_knockout` (`_add_first`), `_add_party_turn`, `_start_final_phase`; `wfightmn_note_copied_tech` | the `fightstg_rules` fixture (imported; its own kept setup case); a family of its own (first made to run last because appending to `fightstg_rules` moved a vblank into `gamestate_flags`' whole-struct read; that read now skips `playtime_frames`, DECISIONS "Replay and golden contracts"): the delays (the turn `base * other.Boost / root` with its 10-step Newton root, which ends on k at k*k - 1 where an isqrt gives k - 1: Boost 50 against 48 -> 979; its bounds 707/1414; an unbounded blast kind; regen, kind 8 (its 6000 cap unreachable), paralysis/confusion/sleep with their resist pairs, a technique's modifier), `take_next` (smallest delay; a turn event loses a tie to any later event; types 4/6/9 stay queued; empty queue), `add_first` (+2 to the queue, itself at 1), a full queue drops the new event, the boss's final phase (event 0x18 at 3000, modifiers set to `-stat >> 1`), and battle type 4's copy rule (`WFIGHTMN.PRO` in the tier-2 slot: party only; kinds 2..8, 11, >= 13 or defense_stat >= 2; never anim scripts 5/12) (34 cases) |
| `gte` | the GTE (`psxstack/psyq/gte.c` on the host) | small MIPS routines written into scratch RAM (`0x80180800`..`0x80184000`, code at `0x80181000`) save the GTE's state, load all 64 registers, run one command, store every register back and restore the state: every command with sf/lm 0/1, all 128 MVMVA variants, the game's 9 command words, MAC overflow past 44 bits, NCLIP/AVSZ limits, SXY/IR0 clamps, RTPS sweeps over every UNR table entry, H >= 2*SZ3, SZ3 saturation; LIBGTE's own functions and the state they leave; replayed on the host by `tests/host/gte_replay.py` (865 cases) |
| `libgs_view` | LIBGS's `GsSetRefView2`, `GsGetLw` and `GsSetFlatLight`, LIBGTE's `MulMatrix`, `MulMatrix2`, `ApplyMatrixLV`, `TransposeMatrix`, `SquareRoot0` (`psxstack/psyq/libgs.c`, `libgte.c` on the host) | the battle camera (issue #7): each call through the `gte` family's wrapper (its routines are this family's fixture), so the GTE registers before and after are part of the golden; a scratch view buffer holds the `GsRVIEW2`, up to four `GsCOORDINATE2` and a `MATRIX`; reads the world-screen matrix `D_80081358`, its copy `D_80081338`, the coordinate systems' `flg`/`workm` (and once the boot's view base `D_80081398` and PSDCNT 1): FIGHTSTG's camera with `super` rotations taken along its swing and its model view, no `super`, twists, `vp = vr` (returns 1), straight up/down (no y turn), coordinates past 15 bits (scaled down), random views, hierarchies of 2..4 systems over every `flg` case (0, PSDCNT, other) with `GsGetLw` on its own; the LIBGTE functions on random and edge inputs, aliased arguments, `SquareRoot0` over powers of two and the table's edges (non-negative only: the PS1 reads past its table for a negative one); `GsSetFlatLight` (issue #19: the battle's lights) as runs of calls from GsInitGraph's zero matrices, reading the light matrix `D_800812F8` (row id = the direction normalised to 4096 and negated) and the light colour matrix `D_80081318` (column id = the colour, `(c << 12) / 255`, loaded as the GTE's LCM): every distinct lighting of FIGHTSTG's stage table (zero lights return -1 and leave their row alone), a zero light over a set row, ids outside 0..2, axis, tiny and large directions, every colour edge, random lights; replayed on the host by `tests/host/libgs_replay.py` (267 cases) |
| `gpu` | the GPU and LIBGPU (`psxstack/psyq/gpu.c`, `libgpu.c` on the host) | GP0 command lists drawn through the game's `DrawOTag` into a 64x64 target at VRAM (512, 0) and read back with `StoreImage` (MIPS routines at `0x80187800` fill textures with a xorshift pattern, `0x80187900` makes 5-argument calls): every polygon, line and rectangle type with random modes (blend mode, dither, mask set/check, depth, raw) and the texture window; probes that pin the rasteriser (UV/colour ramps, line stepping, the colour pipeline per blend mode, sprites); edges (shared edges, thin/degenerate/bowtie, negative and 11-bit coordinates, the 1023/511 limit, draw areas, texture pages); fills and copies; `LoadImage`, `MoveImage`, `ClearImage(2)` (aligned and not, the whole VRAM), the `SetDrawEnv`/`SetDefDrawEnv`/`SetDrawMove` packets, `BreakDraw` idle; replayed on the host by `tests/host/gpu_replay.py` with 133 known differences (`tests/host/known_mismatches.json`) (715 cases); the same lists through the hardware renderer's rasteriser by `tests/host/gpu_hw_replay.py` (its whole VRAM target against the software VRAM after each case; ledger `tests/host/gpu_hw_mismatches.json`, empty; needs a GPU device) |
| `mdec` | LIBPRESS and the MDEC (`psxstack/psyq/libpress.c`, `mdec.c` on the host) | a kept setup puts `STDWTITL.PRO` (LIBPRESS) in the slot; frames of the movies on the disc (MOVIEOPN 3, MOVIEE03 100/1383/1480, MOVIED01 397: the opening, the largest frames, the largest quantiser) are assembled from their sectors into RAM at `0x80112000` (the golden keeps only their `source`, size and SHA-1: the family's `frame_bitstream` rebuilds them from the disc), `DecDCTvlcBuild` + `DecDCTvlc2` expand them (the run-level words and the 0xA5 fill after them read back), then `DecDCTReset(0)`, `DecDCTin` (the command word) and per 16-pixel column `DecDCTout` + `DecDCToutSync` (each column's pixels: a SHA-1); one frame also in 15-bit modes 0 and 2; made-up run-level data (every quantiser, quantiser 0, DC only, 0xFE00 padding, runs past 63, 15-bit, saturation and out-of-range colours) kept as pixels; replayed on the host by `tests/host/mdec_replay.py`: the run-level words exact, the pixels within 4 per byte (PCSX-Redux's IDCT rounding), the movie columns (SHA-1s) and two emulator disagreements known (`known_mismatches.json`) (17 cases) |
| `wfightmn_spoils` | `wfightmn_battle_end` (substep 0), `fieldstg_start_battle` | the battle's spoils, named to run after `gamestate_flags` (sweep5, 2026-10-05): the `fightstg_rules` fixture (its own kept setup case), `WFIGHTMN.PRO` in the tier-2 slot per case; the fade object the function creates is allocated in a 0x100-byte scratch heap arena (`heap_funcs.first/end` pointed at it, restored after the case): the drop pick `next() % count` over the enemies with a Digimon and `item != 0` (a stolen -1 counts, an empty slot does not), the holder's rank used as the slot index (one holder in slot 1: `enemies[0]` is read, no item), `drop_rate + 1` out of 1024 at residues 128/129 (0xD9, rate 128), the picked enemy's own rate (0x20, rate 2), a stolen item (no second draw), the prize override, result 0 (nothing but the HP/MP write-back), 2 (the same drop), a knocked-out member back at 1 HP with its results cleared, and no holder (`% 0`: the draw, no item; the host traps: FINDINGS 7); `fieldstg_start_battle` with `FIELDSTG.PRO` in the slot and a scratch field manager (kind 7) on `heap_objects`: no manager, a plain battle copied (has_prize 0, next map 0x600; 0xE0A at progress 0x2B), the range edges 0x1C8/0x1D1, the prize by parity (odd `next() & 0x1F`, even `& 0xF`: a draw with residue 16 is the rare item only for an even enemy), map and progress 0x2D (24 cases) |

Counts are what `oracle.py check` prints (kept setup cases included): 24 families, 5,034 cases, 14,447 calls.

A **kept** case (`keep`) is a setup whose writes stay for the rest of the family: an overlay copied into the slot, a data
file placed in free RAM and registered in `cdload_module.entries` so `cdload_module.files.get_file(id)` finds it without a
CD read. Every other case restores what it touched. A write can carry a `file` instead of bytes; the golden records the
file's path, size and SHA-1.

A `Read` names a symbol, a raw address or one of the case's scratch buffers (`buf:<name>`): a function's output buffer is
read back after the call (`gamestate_get_stats`, `gamestate_list_forms`, `records_list_items`). Two oracle facts the
fixtures work around: `gamestate_data.playtime_frames` (`0x48`) is counted by the vsync handler, so a call that spans a
vblank (a level-up to 99) changes it; the families hash `gamestate_data` around it. A third: the interpreter emulates the
R3000's instruction cache, so a file write (an overlay copied into the slot) is followed by `PCSX.invalidateCache()`;
without it a family whose overlay replaces another one ran the previous overlay's cached instructions at the same
addresses (found when `stgtrain_rules` ran after `stfgtrep_forms`). And a kept case restores nothing, so
it writes only the overlay (and data files), never a fixture. A file write is padded with zeros to whole sectors, as the game
loads files (cdload reads sectors; `overlay_load_stage` copies `get_sectors << 11` bytes): a table at a file's end is
followed by the zeros of its last sector, not by the previous overlay's bytes (`stfgtrep_resist_gains`, found when
`shop_rules` ran before `stfgtrep_exp`).

### Host-side replay (`tests/host/`)

`tests/host/replay.py` compiles `src/main/{pad,memcard,gamestate,records,card}.c`, `src/fightstg/fightstg_8008D3B4.c`,
`src/cardgame/cardgame_cpu.c` (with the card game's other units `cardgame_80083E34.c`, `_80085DE8.c`, `_8009D6E0.c`, `_800954F8.c`, `_80096950.c` for the rules, the CPU's helpers and the board's data), `src/stfgtrep/stfgtrep_80082E70.c`, `src/stgtrain/stgtrain_{800861BC,80088100}.c`, `src/ststatus/ststatus_{8008E94C,800937A4,80099B6C}.c`, `src/stitshop/stitshop_800859C0.c`, `src/stcrdshp/stcrdshp_80088E24.c`, `src/main/{object,sprite}.c`, `src/stgdglab/stgdglab_8008EB30.c`, `src/wfightmn/wfightmn_800A6440.c`, `src/fieldstg/fieldstg_80083784.c` (FIELDSTG's battle table, data only), `src/fieldstg/fieldstg_80087DB0.c`, `src/fightstg/fightstg_80086A00.c` and `src/main/heap.c` (the MIPS inline asm of
`include/port.h`'s scratchpad-stack macros compiled away, its `heap_funcs` weak: the shims' libc heap wins) for this machine (`-m64 -std=gnu99 -fwrapv -fsigned-char -fno-strict-aliasing
-DNON_MATCHING`, `INCLUDE_ASM` empty, the GTE macros no-ops, `tests/host/shims.c` for the heap, files, time and sound, the rest of
the game's externals resolved to 0 by the linker; the stack zeroed before each call, so uninitialised locals read 0) into `build/host/replay`, then drives every golden case through it
over stdin and compares each return value and read with the original's. Mismatches are findings (`tests/host/FINDINGS.md`);
the explained ones are listed in `tests/host/known_mismatches.json` and keep the suite green. `scripts/test.sh` runs it in
layer 1 after the emulator check. `--findings` prints every mismatch and exits 0. Pointers in fixtures are rebuilt as host pointers: a buffer's pointer fields
(`BUFFER_LAYOUTS`, `POINTER_BUFFERS`), the words of `object_*` buffers (Object headers: a method word that holds an EXE
function's address becomes the host's own function, `P <buf> <off> @<symbol>`) and fixture writes into a global's pointer fields
(`POINTER_GLOBALS`: `heap_objects.objects`, `sprite_current`, `gfx_module.packet`; the harness's `G` command). An `object_*` buffer may carry a pointer-free tail after its 0x50-byte header (the type's own fields: `FieldstgManager`'s
up to `map_entry`); the host places it after its own header and moves the tail's reads. Writes to `heap_funcs` (the oracle's
scratch heap arena) are skipped: the host allocates with libc (`HOST_SKIPPED_GLOBALS`). A call that raises SIGFPE (an x86
division by zero the R3000A does not trap on) answers `trap` instead of a return value and the case's reads still run
(`wfightmn_spoils/no_holder`, FINDINGS 7). These three were added by sweep5 and change nothing for the existing goldens.

## Layer 2: replay scripts

`tests/replay/replay.py run tests/replay/scripts/<name>.json [--record] [--repeat 2] [--bios retail] [--interpreter]
[--speed S] [--iso CUE] [-v]` runs one script; `replay.py check [--interpreter] [--iso CUE]` runs every script that has an
expected file (what `scripts/test.sh` calls). `--iso` boots another image than `iso/dw2003.cue` (the holdout validation's). The emulator runs unthrottled (PCSX-Redux's `spu.Speed` = 0, set by `run.lua` from
`DW3_REPLAY_SPEED`; `--speed 1` is real time): about 500 game frames per wall second on the default dynarec core, 250 on
`-interpreter`. The speed paces the host only: new_game's record is identical at speed 1 and 0.

A script is JSON: `name`, `max_frames`, `default_timeout` (frames a wait may take) and `steps`, each one of

| Step | Fields | Meaning |
|---|---|---|
| `wait_stage` | `stage`, optional `word0`, `timeout` | until `overlay_module.stage` (`0x80055D28`, index into `overlay_files`) equals `stage` (and the first word at `main_overlay_base` is `word0`) |
| `wait_map` | `map`, `timeout` | until `gamestate_data.map` equals `map` (hex strings allowed: `"0x2D7"`) |
| `wait_mem` | `addr`, `size` (1, 2, 4), `signed`, `value`, `timeout` | until the word at `addr` equals `value` |
| `wait_frames` | `frames` | idle |
| `press` | `buttons` (names from `PCSX.CONSTS.PAD.BUTTON`: `START`, `CROSS`, `UP`, ...), `frames` (default 2), `release` (default 2), optional `repeat`, `until`, `timeout` | hold the buttons on port 1, then release; `repeat` N: N presses in a row; `until` (a `wait_stage` / `wait_map` / `wait_mem` step as an object): press again and again until the condition holds, checked every frame (the buttons are released and the step ends on the frame it holds; with `until`, `repeat` is a cap) or fail after `timeout` frames |
| `walk` | `x`, `y`, optional `tol` (default 3), `timeout` | in the field, hold the d-pad toward (`x`, `y`) (pixels; RIGHT = +x, DOWN = +y, both for a diagonal) until the player is within `tol` on both axes. The player is FIELDSTG's actor of kind 5, key2 0 in `heap_objects` (`0x8004B610`), position `FieldstgActor.pos` (24.8) |
| `reset` | | reboot the console (PCSX-Redux `hardResetEmulator`: RAM cleared, the memory cards kept); the frame count goes on |
| `checkpoint` | `name` | dump `gamestate_data`, record frame, stage, map and `pad_random.index` |
| `vram` | `name`, `frames` (default 1) | as `wait_frames`, appending the whole VRAM (1024x512 pixels, 1 MB) to `vram_<name>.bin` on each of its frames (the port: in `DW3_PORT_CHECKPOINT_DIR`); for `tests/port/vram.py`, not in the committed scripts |

Every field not listed (`comment`, ...) is ignored. Buttons are the controller's physical buttons, held through PCSX-Redux's pad override
(`PCSX.SIO0.slots[1].pads[1].setOverride(bit)`, source `src/core/pad.cc` at `bf4c9ceb`: a pressed button is a cleared
override bit; the bit numbers are the PS1 pad's). The game rotates the face buttons for every language but Japanese
(`pad_read_buttons`, `src/main/pad.c`): physical CROSS arrives as logical bit 13, the overlays' "confirm"
(`PAD_PRESSED(0xD)`), TRIANGLE as bit 14 ("cancel"), CIRCLE as bit 12. An English script therefore confirms with `CROSS`.
The runner applies the override at every vsync; the game's `pad_update` runs in the main loop (`src/main/main.c`),
so a 2-frame press is one edge while the game runs normally. While the game is busy (the opening movie spins in
`StGetNext` on the dynarec core), the pad is sampled rarely: hold the button long (the movie's START skip).

**Both CPU cores.** A script must reach the same checkpoints on the default dynarec core and on `-interpreter` (the core
`tools/coverage.py` needs), whose CD and MDEC timing differ (on the interpreter the opening movie decodes normally and
takes START at once). So a script waits on game state, not on frame counts: a stage, a map, a memory value, a player
position, or a button tapped `until` the game reacts. A fixed `wait_frames` is allowed only for a game-logic animation
right after such a state change. `run --interpreter` and `check --interpreter` compare the **cross-core view** of the
record with the expected file: per checkpoint its name, stage, map and stable hash, and the overlay and map sequences
without frames. Frames, RNG draw counts, the full hash and the input trace are core-dependent. Expected files always come
from the dynarec core (`--record --interpreter` is refused).

**The stable hash and the RNG.** The stable hash is the SHA-1 of `gamestate_data` with the fields zeroed that depend on
timing (`replay.py` `VOLATILE_RANGES`): byte 0 (the slot checksum), `encounter_timer` (+0x30), the playtime (+0x48..0x53) and
`spot_target` (+0x26EC). `pad_random.index` (a checkpoint's `random_index`) advances about once per frame in the field, so
every value drawn from it depends on the frame count: in `first_battle_save` the index is the same on both cores at
`login_movie`, `asuka_lobby` and `battle_start` (where even the frames agree), and differs at every checkpoint from
`battle_won` on (dynarec - interpreter, mod 4096: +72 through `back_on_field`, +71 at `saved`, -20 .. -18 after the reset,
-29 at `train_left`; the interpreter is 58 .. 80 frames later, and the battle alone draws 72 fewer numbers in 58 more frames). The port's timing differs again, so a port compares the stable hash and never `random_index`.
A field that holds a draw goes into `VOLATILE_RANGES` when it is still 0 at every recorded checkpoint before (so no earlier
hash changes), as `spot_target` did; an outcome drawn from it (a random battle's enemies, a training round) is not
replayed at all: layer 1 pins those rules with the index set by the case (`stgtrain_rules`, `fightstg_rules`).

The expected file records the whole run (checkpoints, overlay sequence, map sequence, the per-frame input trace as a
list of changes); `check` compares all of it except `tree_commit`. Since the frames are in there, a script's expected file
also proves the emulator is deterministic: `--repeat 2` runs twice and requires identical records.

| Script | Frames (dynarec) | Checkpoints | Path (overlays, maps) |
|---|---|---|---|
| `new_game` | 2,462 | `cnty_sel`, `opening_movie`, `title_screen`, `new_game_field`, `first_field_map` | CNTY_SEL (0x1600) → STDWTITL opening movie (0xE02), title (0xE00) → New Game → FIELDSTG intro map 0x2D7 (WSTAG780) |
| `first_battle_save` | 46,597 (80 s; 186 s on `-interpreter`) | `login_movie`, `asuka_lobby`, `battle_start`, `battle_won`, `battle_end`, `back_on_field`, `saved`, `loaded`, `shop_open`, `shop_bought`, `shop_left`, `lab_open`, `card_case`, `status_open`, `album_open`, `deck_edit_open`, `card_shop_open`, `card_shop_left`, `train_open`, `train_left` | new_game's boot → intro and registration (0x2D7-0x2D9, STPLNMET 0x500: the name typed with CROSS, the default partner set) → login movie (0xE03) → Asuka (0x207, Main Lobby 0x203) → talk to Tamer Service → admin room (0x204, progress 3) → Asuka City (0x200) → the story's first battle, scripted (FIGHTSTG 0x600 with WFIGHTMN, won with CROSS on every menu) → STFGTREP report (0x1400) → 0x200 → lab (0x206, progress 4) → 0x200 → Asuka Inn (0x20A) → save attendant → STGMCARD save (0xC01) → 0x20A → `reset` → title → Continue → STGMCARD load (0xC00) → 0x20A → (after the load) Asuka City (0x200) → Asuka Item Shop (0x20D) → shopkeeper's second talk → STITSHOP shop 2 (0xF00): buy, refused → 0x20D → (the tour) 0x200 → lab (0x206) → chart terminal → STGDGLAB (0xD00) → 0x206 → 0x200 → inn (0x20A), down its stairs → 0x20B → 0x201 → card arena (0x211): the counter's talk, event 1415 gives the card case (item 0x192) → field menu, sixth option → STSTATUS party page (0x1000) → STCRDABM (0x1200) → STSTATUS → STCRDDEK (0x400) → STSTATUS → 0x211 → 0x201 → 0x211 → the counter → STCRDSHP (0x1300, shop 0x31): Buy, nothing affordable → 0x211 → (STGTRAIN) 0x201 → 0x20B → 0x20A, up its stairs → 0x200 → 0x202 → 0x21D (encounter area, crossed without a battle) → the trainer → STGTRAIN (0xA00): Punch at TP 1 refused (TP 0) → 0x21D |

`first_battle_save` is one script because the save point is only reachable through the battle's story path (the lobby's
exit to the city turns the player back before progress 3, and the admin-room scene that sets progress 3 leads straight
into the battle). What it pins: the registration (name, party), the battle's HP loss (`battle_won`), the report's prize
money and experience (`battle_end`), and the save round trip. `saved` vs `loaded`: the slot (bytes 4..0x26C3 of
`gamestate_data`) comes back byte for byte except the playtime; byte 0 (the slot checksum) and byte 2 (version 4) are only
written into `gamestate_data` by the load, and the bytes from 0x26C4 on (map, next/prev map, map entry, `map_is_new`,
`field_last_map`, player depth, meter count) are runtime state that the save does not hold, so the two stable hashes
differ by design. The stable hash zeroes byte 0 as well: it is 0 until a load, and a loaded slot's checksum covers the
playtime, which differs between the cores.

The shop is appended to `first_battle_save` rather than given its own script: the Asuka Item Shop is the first shop reachable
from boot, but only after the first battle (the lobby's exit to the city opens at progress 3, which starts that battle), so a
separate script would replay the same 31,000 frames and add a second input trace of ~250 KB. Appended after `loaded`, it leaves every
earlier checkpoint, sequence entry and input unchanged (the expected file grew by 9 KB). What it pins: `shop_bought`, `money`
(`gamestate_data` + 0x6C) 50 -> 2 and `items[43]` (+ 0xA7) 0 -> 4, four Power Charges at 12 BIT. UP+CROSS pressed together until the item count
changes reaches the quantity's cap (`money / price` = 4) without waiting on a menu state: UP is harmless in the menu, the list (cursor at 0)
and the Yes/No box, and in the quantity box CROSS is taken only on a press where UP changed nothing. `shop_left`: the same bytes after
eight more CROSS presses on the item with 2 BIT ("Not enough BIT.", nothing bought).

**The overlay tour** (appended after `shop_left`, +10,261 frames; every earlier checkpoint, sequence entry and input of the
expected file is unchanged, checked against the previous file) visits the menus a boot can reach by pad input in the first
towns, one checkpoint per overlay load: STGDGLAB (the lab's chart terminal, flag `0x7C00`), the card case (event 1415 at the
arena's counter, ~3,700 frames of dialogue: `items[0x192]` 0 -> 1), STSTATUS, STCRDABM and STCRDDEK (the field menu's sixth
option, the party page, exists only with the card case: `fieldmenu_update`), STCRDSHP (the counter becomes the card shop only
with the card case, placed when the map is entered, so the script leaves and re-enters 0x211) and `card_shop_left` (Buy opened,
every card costs more than the 2 BIT left: nothing bought). Tier-1 overlays reached by the two scripts: CNTY_SEL, STDWTITL,
FIELDSTG, STPLNMET, FIGHTSTG (+ tier-2 WFIGHTMN), STFGTREP, STGMCARD, STITSHOP, STGDGLAB, STSTATUS, STCRDABM, STCRDDEK,
STCRDSHP (13 of 19). Coverage of the tour (`tools/coverage.py run --replay first_battle_save`, functions run / total):
STGDGLAB 19/71, STSTATUS 22/123, STCRDABM 25/29, STCRDDEK 17/55, STCRDSHP 33/45 (with the FAKE match `stcrdshp_update_buy`;
the holdout `stcrdshp_run_booster` needs a booster pack, which money cannot buy here); the script as a whole runs 1,025 of
3,608 game functions. Not reached:
- **STAGSLCT, SOUNDTST, SHOCKTST, WFIGHTTS: not reachable by pad input.** The only maps that load STAGSLCT (0x15xx) are set by
  STAGSLCT itself, SOUNDTST, SHOCKTST and CARDGAME (`get_map() & 0xF ? 0x1500 : field_map`, a match started from STAGSLCT's
  0x701); boot starts at CNTY_SEL (0x1600, `gamestate_new_game`), which goes only to the movies (0xE01/0xE02), and the title to
  0x2D7, 0xC00 or a movie. No field route leads there: every map exit (`FieldstgMapEvent` types 1, 14, 9, 10: 799 rows, all
  to field maps), every event script `GOTO_MAP` (144: field maps, 0xC00, 0xE0x, 0x500) and every constant `set_next_map`/`fieldstg_goto_map` in the C targets a field map or a
  game-mode map (0x400-0x1300, 0xE0x), never 0x8xx, 0x9xx or 0x15xx. Nothing sets 0x8xx/0x9xx at all, not even STAGSLCT's
  stage list (270 stages: 31 game modes from 0x400 to 0x1600 and 239 field maps, no 0x8xx or 0x9xx): those two overlays are dead
  code. WFIGHTTS loads when map 0x600 has a non-zero map entry (`fightstg_main_update`); every battle the field starts sets
  entry 0, only STAGSLCT's `{0x600, 1}` does not. Reaching them would need a memory poke (a test-contract change), not done.
- **STDGNAME** (flag `0x7C01`: maps 0x239, 0x295, later story) and **CARDGAME** (every opponent needs party Charisma >= 60,
  below) are past the first towns.

**STGTRAIN** (appended after `card_shop_left`, +4,129 frames; every earlier checkpoint, sequence entry and input of the expected
file is unchanged, checked against the previous file): the trainer (actor 27 at (923, 374) of map 0x21D, talk sets `0x9400`, type
0x94: map 0xA00) is reached through the inn's stairs (0x211 -> 0x201 -> 0x20B -> 0x20A -> 0x200 -> 0x202 -> 0x21D). Map 0x21D is
encounter area 1 almost everywhere, but a step of `encounter_timer` is every 8th moving frame (`fieldstg_actor_play_steps`), not
every frame: the timer goes 926 -> 809 on the walk, no random battle (one would draw its enemies at a core-dependent RNG index).
`train_open`: STGTRAIN loaded. No training session: the party has TP 0 (TP come with level-ups, +5 each, and the first battle
gives none), so Punch at TP 1 is refused ("Not enough TP.", `stgtrain_level_run` step 10) and `train_left` pins that nothing
changed (the dump differs from `train_open` only in the playtime and the map). A session would not be replayed anyway: its rounds
and gains are drawn from `pad_random`, whose index differs between the cores here (above); `stgtrain_rules` pins them in layer 1.
The menus take CROSS only once loaded (CD timing), so the script presses CROSS 14 times 62 frames apart (Digimon -> training ->
level -> refused -> closed -> refused ...), ending on the level menu or the warning depending on the core, then CROSS, TRIANGLE,
CROSS, TRIANGLE 92 frames apart, which ends on the training choice from either (the warning takes only CROSS, every other screen
goes back on TRIANGLE), and TRIANGLE until the field. Entering 0x21D (a map with dig spots) draws `spot_target` from `pad_random`:
that field joined the stable hash's volatile set (0 at every earlier checkpoint, so no earlier hash changed). STGTRAIN makes 14
of 19 tier-1 overlays reached. Coverage (`tools/coverage.py run --replay first_battle_save`): STGTRAIN 30/94 (the main screen, the
choice and level menus, the anim loader; not the session, the trainee or the result screen, so not the FAKE matches
`stgtrain_sprite_update` and `stgtrain_anim_upload`, which only a session runs: the trainee's animation); the script as a whole runs 1,065
of 3,608 game functions (1,025 before).

**Runner fix (backward compatible):** PCSX-Redux leaks two Lua stack slots per event-listener call; at ~32,720 vsyncs
the stack overflows inside the runner's listener (seen in this script, the first one past that length: a "stack
overflow" error at frame 32,727 that skipped that frame's step processing; a bare listener aborts the emulator there).
An error raised out of a listener makes Redux reset the stack and keep the listener, so `run.lua` raises one every 8,192
frames after the frame's work (the log shows the stack dump and "Lua stack reset"; not a failure). The emulated machine
does not see it: new_game's and first_battle_save's earlier records are unchanged.

**No card match.** Every card-game opponent of the early towns needs party Charisma total >= 60 (flag `0x7200`); the party has 3 after the
first battle, so a match needs grinding levels (random battles, RNG- and core-dependent), far past ~40,000 frames. The player does start with
cards (40, three saved decks), and the first card shop (arena 0x211) sells nothing under 1,000 BIT (MECHANICS section 10).

### SPU write traces (`tests/sound/`, part of layer 2)

`tests/sound/spu_trace.py` runs a layer-2 script under `-debugger -interpreter` with `tests/sound/spu_trace.lua` loaded
before `run.lua`: write breakpoints over the SPU's and DMA4's registers record every store with its vsync tick (DMA
blocks as SPU address, length, SHA-1), from the EXE's entry point on. `check` reproduces the committed
`tests/sound/expected/cnty_sel.trace` (the boot to CNTY_SEL + 300 frames, ~25 s; `scripts/test.sh` layer 2 runs it);
`run <script> [--until CHECKPOINT]` traces any script; `diff` compares two traces (the port's against the emulator's).
Format, costs and findings: `docs/SOUND.md`.
`tests/sound/key_trace.py` plays every sound of the game one at a time (the oracle's call mechanism stops the game in
CNTY_SEL's main loop and calls `sound_play` per key, with `sound_load_extra_bank` for each of the 71 banks: no pad
route) and records a trace per step: `tests/sound/expected/keys/` (the plan and one xz trace per bank, 818 KB); `check`
reproduces them (4 emulator boots of ~5 min, `--boots N` one; not in `test.sh`), `diff` compares a port trace of the
same driver step by step.

## Layer 3: formats

`tests/formats/run.sh` (what `scripts/test.sh` calls, ~5 s) checks format facts the port's loaders depend on, with the
`--check` modes of the existing tools. It needs `extracted/` and `iso/` only (no `asm/` or `build/`); each check prints
one summary line with its counts and fails non-zero on a violation:

| Check | Tool | Pins (docs/FORMATS.md) |
|---|---|---|
| file table | `tools/disc_files.py --check` | the EXE's 2,382 `(LBA, sectors)` entries cover every `AAA/` file of the ISO directories exactly once; sectors = size rounded up; the bytes after each file's end in its last sector are zero (cdload loads whole sectors); the 350 text files sit at JPN ID + `records_language` (IDs 1–0x15E) |
| text | `tools/dump_text.py --check` | all 91,059 entries of the 3,005 string tables (350 files, 7 languages) decode with no unknown byte; every file is a table or a container of tables; entry 0 of every table is the empty string |
| WSTAG stage table | `tools/overlay_layout.py --wstag-table` | each of the 293 code `WSTAG###` overlays has a record in FIELDSTG's stage tables (stage, file ID, entry) whose entry is a function start inside its `.text`; the result equals `config/wstag.txt` |
| flag ranges | `tools/flag_census.py --check` (reads `src/`) | every one of the 7,061 bit-array flag uses (types 0x00–0x40, part of the save data) has an index inside its `include/gamestate.h` array; the tightest are type 0x04 (index 15 of 16 bits) and 0x06 (7 of 8) |
| sound banks | `tests/sound/sound_formats.py --check` | the 71 banks: VAB header sizes and tone records, every ADPCM block's shift/filter and each VAG's end flag, the 72 SEPs' 16 sequences each ending exactly at their end-of-track (FORMATS "Sound") |

**Save round trips** (`tests/saves/run.py`, ~1 min; `scripts/test.sh` layer 3 runs it after `tests/formats/run.sh`, and
skips it when the emulator, the disc, CMake/Ninja or gcc is missing): the port and the emulator each save at the Asuka Inn
(a cut of `first_battle_save` to `saved`, on a new card: `--memcard1` / `-memcard1`), then each loads the other's card
(the script's steps after `reset` to `loaded`, from a fresh boot); both `saved` and both `loaded` checkpoints must have
`tests/replay/expected/first_battle_save.json`'s stable hashes. `tests/saves/cards.py` then checks both cards (directory
checksums, the save file's blocks, the Sony header, the game's header and slot with exact XOR sums) and requires them equal
byte for byte except 8 named fields: directory frame 63 (the emulator's BIOS), the play time and the encounter timer with
the two checksums that cover them, and the two stale buffer tails (`tests/saves/README.md`). No card is committed.

## The port's M1 test (`scripts/test.sh` layer `port`)

`tests/port/run.py [--m32] [--sanitize] [--cd-speed instant|realistic]` builds the PC port (`port/`), runs layer 2's
`new_game` script in it twice (`build/port/dw2003 --disc iso/dw2003.cue --script ...`: the port's step engine,
`psxstack/runtime/script.c`, has `run.lua`'s semantics) and compares the record's **cross-core view** with
`tests/replay/expected/new_game.json`, with `replay.py`'s own `cross_core_view` and `compare`: the checkpoints' names,
stages, maps and stable hashes, and the overlay and map sequences without frames. The two runs' logs and records must be
byte-identical; `--m32` adds the 32-bit build (same log and record: the layout check), `--sanitize` an ASan/UBSan build (no
report). Frames, `random_index` and the full hash are not compared, as between the emulator's two cores (`tests/port/README.md`).
It needs the disc, the host gcc, CMake and Ninja (on PATH, or `scripts/setup.sh cmake` puts them in the venv) and the
venv: no emulator. `scripts/test.sh` skips it when one is missing; CI runs it, and `--m32`, with the data checkout.
The mods' tests (`tests/port/mods.py`, the longest of the port's checks: ~4 min one after another) are their own layer,
`mods`: each mod's test runs in its own process (`--only MOD`), `DW3_JOBS` at a time, in CI's own `port-mods` job.

## Coverage (not a test)

`tools/coverage.py run` records which game functions the emulator tests execute, to choose the next tests and to
show what the suite reaches; `tools/coverage.py report [--module X]` summarises the recorded runs. It is not part of
`scripts/test.sh`. `run` boots once for the oracle (all families, the goldens are checked on the way) and once per
replay script (`--oracle`, `--replay NAME|path.json` pick sources), with `tools/coverage.lua` loaded before the runner
under `-debugger -interpreter`: an exec breakpoint on every function start of the build's ELFs, deleted at its first
hit. The overlay slots' files share addresses, so a slot's breakpoints are those of its resident file: the one whose
`.text` from the build equals RAM, re-checked after the game's two overlay `memcpy`s, at each vsync and at each oracle
call (`oracle.lua` calls `DW3_COVERAGE_SYNC` when the recorder defined it). The oracle source is also split into
`oracle_calls` (the jobs only, without the boot to CNTY_SEL). Output: `build/coverage/<source>.json` (every first hit
with its frame and stage, or oracle job), `build/coverage/report.md` (covered/total per EXE unit, overlay, C file,
WSTAG aggregated, Psy-Q apart; the focus modules' uncovered functions; which `NON_MATCHING` holdouts and `FAKE:`
matches ran) and `build/coverage/coverage.json` (the same, per function, for diffs between runs). `report` merges every
`build/coverage/<source>.json` present; delete one to drop that source. Cost: about +50% on the oracle (35 s) and
about 30 game frames per wall second for a replay.

Replays run under the interpreter here. Both scripts wait on game state, so they reach every checkpoint under it
(`new_game` reproduces its expected file; `first_battle_save` its checkpoints, with other frames). A full `run` takes
about 23 minutes on this machine (the oracle 36 s, `new_game` 72 s, `first_battle_save` 21 min at ~37 frames per second).

State (2026-10-05, end of session 11; functions run / total, game functions without Psy-Q):

| Source | Functions |
|---|---|
| oracle (layer 1, with the boot) / its jobs only | 279 / 210 (sweep2: `tools/coverage.py run --oracle`) |
| `first_battle_save` / `new_game` | 1,065 / 329 |
| union | 1,199 of 3,608 (the replays' runs of the end of session 11 plus the oracle's new `fieldstg_encounter_reset`, `fightstg_events_add_gauge_full`) |

The pure-logic modules: `fightstg_rules_*` 32/32, `cardgame_cpu` 15/15, `pad_random` 2/2 by the oracle; `gamestate` 46/50 by
the oracle and 49/50 in the union (only `gamestate_start_card_game`, no card match), `records` 6/7 and 7/7; the rest of each
overlay is mostly UI (object update/draw/run, windows, cursors) that the replays run: STFGTREP 34/36, STGTRAIN 37/94,
STGDGLAB 20/71, STSTATUS 34/123 in the union. Tier-1 overlays run by the replays: 14 of 19 (not CARDGAME, STDGNAME,
STAGSLCT, SOUNDTST, SHOCKTST); tier 2: WFIGHTMN and 15 of 293 WSTAG files, not WFIGHTTS. FAKE matches run: see
`build/coverage/report.md`.

## Holdout validation (retired)

`tests/holdouts/run.sh` replayed a `-DNON_MATCHING` PS1 image through the layer-2 scripts to validate the holdouts' WIP
C (the code the port compiled for them). Since #55 there are no holdouts, and `tools/hacks.py --check` keeps any new
`INCLUDE_ASM`/`NON_MATCHING` out of game code, so the script was removed; it is in the history (`git show
26bace0:tests/holdouts/run.py`) should a function ever have to go back to asm.
