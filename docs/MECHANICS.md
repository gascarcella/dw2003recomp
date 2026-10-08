# Game mechanics reference for the reference tests (Track A)

Purpose: give the golden tests (layer 1), record/replay tests (layer 2) and save round trips (layer 3) of
DECISIONS "Reference tests: three layers" names, fixtures and expected behaviours.
Date: 2026-10-04 (first pass, research agent, read-only over the repo; the `[T]` marks were added by later sessions).

**Budget used:** 2 web fetches (both blocked), about 140k tokens (almost all of it reading the C and headers).

**Marking convention.** Each claim carries a tag. `[source]` means taken from a written guide (cite the URL once per
section); `[C]` means inferred from the decompiled C in this repo; `[T]` (added from session 9 on) means pinned by a
reference test, named where it is used: a layer-1 golden (`tests/golden/<family>.json`, the original's code run in the
emulator, replayed on the host) or a layer-2 checkpoint (`tests/replay/`). **No `[source]` claim exists in this file**:
gamefaqs.gamespot.com and strategywiki.org were both refused by the network egress proxy (EGRESS_BLOCKED), so by
the rules of the task I stopped after two blocked hosts. Everything not `[T]` is `[C]`, and the "Sources wanted" list at
the end says what to fetch from an unblocked machine. Where a guide and the C disagree, the C wins (it is the game).
"Names" (Power, Guard, Spirit, Wisdom, Boost, Charisma, element names) come from item-name comments in
`include/records.h` and are `[C]` too.

Provenance of the golden values: goldens come from the emulator-run original (the oracle runs the disc's own code), never
from this document's hand arithmetic; the hand-computed numbers below check that a harness is wired right.
`fightstg_rules_get_stats` (`src/fightstg/fightstg_8008D3B4.c`) is matching C since 2026-10-07, so the host replays
the original's code too.

## 0. Shared conventions

- Stat order [C]: `GamestateStats.values[]` = level, (a second value, +5 per level up, cap 99), HP, max HP, MP,
  max MP; `.stats[0..5]` = Power, Guard, Spirit, Wisdom, Boost (speed), Charisma; `.resists[0..6]` = Fire, Water, Ice,
  Wind, Thunder, Machine, Dark. In battle `FightstgStats.stats[0..4]` are the first five (no Charisma), `.resists[0..6]` the
  same seven, `.resists[7..11]` = poison, paralysis, confusion, sleep, knockout resistances (from the Digimon's
  `unk_2A[2..6]` and ring items 0x11..0x15).
- Element numbers [C]: an element `e >= 2` indexes `resists[e - 2]`: 2 Fire, 3 Water, 4 Ice, 5 Wind, 6 Thunder, 7 Machine,
  8 Dark. `RecordsTechnique.defense_stat >= 2` is that same number (the stat/resist the technique is "against").
  `element < 2` means none.
- Probabilities [C]: almost every roll is `(pad_random.next() & 0x7F) < chance`, i.e. `chance / 128` (clamped by the
  code, or not: see per mechanic). Integer division throughout, C truncation toward zero, `s32` intermediates.
- Sides [C]: `side == 0` is the party attacking; the enemy path is `side != 0` (the callers pass 0x10 for the
  enemy in `get_stats`). `fightstg_battle.state.current[2]` = acting member per side.
- Test seam [C]: `fightstg_rules` is a struct of function pointers (28) next to the two `FightstgStats`. Fixtures set
  `records_state`, `fightstg_battle.state.members[][]`, `gamestate_data.digimon[]`, then `pad_random.seed(n)`.

## 1. RNG (`pad_random`)

Facts [C]; the table, `seed` and `next` [T] (`pad_random`: the table's SHA-1 below, 11 seeds from 0 to -1 x 32 draws):
- `pad_random_table[0x1000]` (u16) in `src/main/pad.c` (line 437) is **a permutation of 0..4095** (verified: 4096 distinct
  values, min 0, max 4095). `PadRandom { s32 index; seed; next }` is the module `pad_random`.
- `pad_random_seed(s)`: `index = s & 0xFFF`. `pad_random_next()`: `index = (index + 1) & 0xFFF; return table[index]`
  (increment first, so after `seed(0)` the first value is `table[1] = 0x0C60`, not `table[0] = 0x0C93`).
  Main seeds 0 at boot (`src/main/main.c:77`); nothing else calls `pad_random.seed` in `src/`, so the stream is a
  pure function of the number of draws since boot (every overlay's rolls advance the same index, including cosmetic
  ones: effects and the field's meter patterns, ~47 call sites outside FIGHTSTG/STFGTREP).
- Consequence: a `& 0x7F` roll over one full cycle is exactly uniform (every residue appears 32 times), so
  `P(roll < c) = c / 128` exactly over 4096 draws; `% 9` and `% 5` rolls are near-uniform (4096 is not a multiple).
- SHA-1 of the table as 4096 little-endian u16: `76e59f411a4dac378941003222c1495685c1fe1e`.
- Some call sites use `next() % n` (poison/regen/steal `% 1024`, level-up gains `% 9`, `% 5`, `% 4`) and a few `& 0x3F`.
  `pad_random.next` returns u16 but callers use the full word; values are 0..0xFFF.

Code map: `pad_random_seed`, `pad_random_next`, `pad_random_table` (`src/main/pad.c`; `include/pad.h` `PadRandom`).

What a test should check:
- `seed(0)`; first 6 `next()` = `0x0C60, 0x047F, 0x0C08, 0x0248, 0x00FE, 0x0B5A` (table[1..6]; table[0] = `0x0C93`).
- `seed(0xFFF)`: first `next()` = `table[0]` = `0x0C93`; second = `0x0C60`. `table[0xFFF] = 0x07BD`, so `seed(0xFFE)` then
  `next()` = `0x07BD`. Wrap: 4096 draws return to the start state.
- `seed(0x1000 + k)` equals `seed(k)` (mask), `seed(-1)` = index 0xFFF.
- Permutation property and the table hash above. Exact `chance/128` frequency of `(x & 0x7F) < c` over a cycle for c = 0, 1, 4, 64, 127, 128.
- Replay layer: the draw count since boot is a checkpoint value worth logging (cheap, catches any missed or extra roll).
  But it is a function of timing, not only of the inputs [T]: in the field the index advances about once per frame (map 0x201:
  323 draws in 325 frames), so `first_battle_save`'s `random_index` agrees between the dynarec and the interpreter core only
  up to `battle_start` and differs from `battle_won` on (the battle itself: 706 draws in 1,411 frames on one core, 634 in 1,469
  on the other, with the same outcome). Compare it within one core only; a port compares the stable hash, which zeroes the
  fields drawn in the field (`tests/README.md` "The stable hash and the RNG").
- Random encounters [C] (`fieldstg_encounter_step`, FIELDSTG): while the player moves, every 8th frame (`fieldstg_actor_play_steps`:
  actor timer `& 7 == 0`) on a cell of attribute layer 4 (area 1..3) takes `fieldstg_encounter_rates[list->rate]` = {2000, 3, 4, 6,
  9, 18} off `gamestate_data.encounter_timer`; at <= 0 a battle from the area's list (`next() & 7`) starts if
  `records_state.encounters` (STAGSLCT's debug switch), and the timer is redrawn (`fieldstg_encounter_reset`): `r = next() % 2304`,
  `r < 256 ? r : (r + 256) / 2` (0..1279; the branches agree at 255 and 256) [T] (`fieldstg_encounter`: draws 254 -> 254, 257 -> 256,
  2303 -> 1279, 2304 -> 0, 4095 -> 1023, one draw each).
  A new game sets `(next() & 0x1FF) + 0x200`. Entering a map keeps the timer. [T] by replay `first_battle_save`: 926 from the new game
  to map 0x21D (no encounter area before), 809 after the walk to its trainer (~20 steps at 6 in area 1 and 3 in area 2; both cores agree, the timer is still in the
  stable hash's volatile set since a redraw would depend on the RNG index).

## 2. Battle damage (physical): `fightstg_rules_get_damage` and `fightstg_rules_scale_damage`

Facts [C]:
- Entry: `get_damage(side, id)` (`id` = technique ID, record `records_techniques[id - 1]`). It refreshes both stat blocks
  (`get_stats` into `fightstg_rules.stats[0]` attacker and `[1]` target) then computes
  `value = power * atk.stats[0] / target.stats[1]`. Enemy side first scales the power:
  `power * enemies[member].stat_scale / 16 * atk.stats[0] / target.stats[1]` (the scale is applied to the power before the
  multiply-divide, with its own truncation).
- `scale_damage(side, id, base)` pipeline, in this exact order, all integer truncating:
  1. `value += field_bonus(value, rec.defense_stat)`: no effect if `field.element < 2`; same element as the field
     `+ value * field.power / 128`; if `opposite_elements[e] == field.element`, `- value * field.power / 256`.
     `opposite_elements = {0,0,4,2,5,3,7,8,6}`: a cycle 2->4->5->3->2 (Fire, Ice, Wind, Water) and 6->7->8->6
     (Thunder, Machine, Dark); it is not symmetric.
  2. Strong type: if `rec.strong_type >= 2` and equals `target.type`, `value += value / 2`; else if any of the attacker's
     up to three `strong_types[]` (weapons, each >= 2) equals `target.type`, `+= value / 2` once.
  3. `power_up` (attacker's buff, `FightstgStats.power_up`): `value += value * power_up / 64`.
  4. Element: if `rec.defense_stat >= 2`: `value += value * rec.element_power * 2 / target.resists[defense_stat - 2]`;
     else if the technique's `anim_script` is not 0xB/0xC and the attacker has an elemental crest
     (`attack_element != 0`): `value += value * attack_element_power * 2 / target.resists[attack_element - 2]`.
     **Higher target resist = smaller bonus; the bonus is additive (never reduces below `value`).**
  5. Multi-hit crest (`multi_hit`, item 0x13C): if `rec.kind < 2` and script not 0xB/0xC, `value = value * 4 / 10`.
  6. Critical (a roll, section 3): `value += value * ((next() & 0x3F) + 0x20) / 64` (adds +50 % to about +98 %).
  7. Guard crest: `value -= target.guard` (flat), clamped at 0. (Items 0x145/0x146.)
  8. Cap: `value <= base * 5`, then `value >= 10000` becomes 9999.
- Note `get_damage` passes a 4th argument `target` to `scale_damage` that the old-style K&R definition ignores.
  Draw order matters for replays: the critical roll (one draw), then the crit magnitude (one more draw only when it crits).
- Special (spirit/magic) damage `get_special_damage`: `power = rec.power` (enemy: `* stat_scale / 16`);
  `base = power * (atkSpirit * 50 / tgtSpirit + 50) / 100` clamped into `[power / 2, 2 * power]`; field bonus on it;
  then if `defense_stat >= 2` with `d = target.resists[defense_stat - 2]`: `d < 100`: `value * (400 - 3d) / 100`;
  `d >= 300`: `value * (65 - d / 20) / 100`; else `value * (125 - d / 4) / 100` (100 % at d = 100, 51 % at 299, 50 % at 300);
  strong type `+ value / 2`; special critical (draw `next() % 65 + 0x20`, `/ 64`); guard flat; cap `base * 5` and 9999.
- Poison tick `get_poison_damage(args)`: 0 unless `args->member == current[side]`;
  `(amount / 2 - (resists[7] + resists[1]) / 10) * max_hp / 100`, min 1, max `max_hp / 2`.
- Counter damage, heal, regen: `get_counter_damage` (own basic technique `techniques[0]`: plain `scale_damage(amount)` with multi-hit
  zeroed; else `amount * effect_power / 64`, or `/ 32` when the member is `boosted`); `get_heal`:
  `(power << 6) + Wisdom * power / 8`, cap 9999; `get_regen(side, member, flag)`: `max_hp * (next() % 9 + 8) / 128`
  when `flag`, else `max_hp * (next() % 5 + 4) / 128`, cap 9999 (300 max HP at RNG index 0: 18 and 16).

[T] The pipeline above, in that order, is pinned by `fightstg_rules` (layer 1; the host's C matches every case): both damage
functions for 9 techniques (basic, elemental, strong-type, anim script 0xB, kinds 2/9/0xB) x both sides x 4 RNG indexes, and
with power_up, an Ice and a Fire field, the multi-hit, guard, Fire and critical crests, a sleeping/paralysed/confused target,
the scaled second enemy, the second member, a boss and Power/Guard modifiers; `get_heal`, `get_regen`, `get_poison_damage`,
`get_counter_damage` (own basic technique, boosted), `get_field_bonus` over every element. Hand checks (family comment):
technique 1, Power 80 against Guard 42: 60 * 80 / 42 = 114; power_up 32: 171; technique 150 (Ice, element_power 32) on an
Ice resist of 90: 380 + 380 * 64 / 90 = 650, on an Ice field 570 + 405 = 975 (the field bonus comes before the element);
special damage of 150 (Spirit 70 against 40, resist 90): 274 * 130 / 100 = 356.

Code map: `src/fightstg/fightstg_8008D3B4.c`: `fightstg_rules_get_damage` (7402), `fightstg_rules_scale_damage` (7343),
`fightstg_rules_get_field_bonus` (7323), `fightstg_rules_get_special_damage`, `_get_poison_damage`, `_get_counter_damage`,
`_get_heal`, `_get_regen`; data `fightstg_rules_opposite_elements`, `records_techniques` (`include/records.h`
`RecordsTechnique`: `power`, `defense_stat`, `element_power`, `strong_type`, `kind`, `anim_script`, `effect_power`),
`fightstg_battle.state.field` (`FightstgField`), `records_state.enemies[].stat_scale`.

What a test should check (hand-computed with crit forced off, all other modifiers neutral: set `stats->critical = 0`,
`strong_type = 0`, resist 100):
- power 100, Power 50, Guard 25: base 200; with no extras result 200 only if the crit roll misses; sweep seeds to find a miss.
  Result is always in `[200, ...]` minus guard, never above `5 * base = 1000`.
- Strong type: base 200 -> 300 (`+ value/2`); odd values truncate: base 201 -> 301.
- `power_up = 32`: 200 -> 300. Element: `element_power = 10`, target resist 100, value 200 -> `200 + 200*10*2/100 = 240`;
  resist 1 would add 4000 (clamped later by the `base * 5` cap); resist 0 divides by zero (see edge cases).
- Multi-hit crest on `kind < 2`: 200 -> 80; on a script 0xB/0xC technique unchanged.
- Guard crest 30 against 200: 170; guard larger than value gives 0, never negative.
- Cap: `power 9999, Power 999, Guard 1` -> base 9,980,001 (fits s32), `5*base` cap irrelevant, result 9999. A base of 1 can reach at most 5.
- Field element equals the technique element with `field.power = 64`: +50 %; opposite element: -25 %.
- Special damage: Spirit equal on both sides gives `power * 100/100 = power`; resist 100 -> exactly `power`; resist 99 -> 103 %, 299 -> 51 %.
- Poison: `amount 20`, resists 0, max HP 1000 -> `10 * 1000 / 100 = 100`; huge resists floors at 1; never above `max_hp/2`.
- Heal: power 10, Wisdom 80 -> `640 + 100 = 740`.
- Edge cases: enemy stats are not clamped like the party's (`enemy->stats * stat_scale / 16`), and the original has no
  division-by-zero trap (no `break` after its `div`s), but the game's data never makes a divisor 0 (section 4: enemy stats
  >= 12 after scaling, element resists >= 60). `id = 0` reads `records_techniques[-1]` (out of range; not a case).

## 3. Hit and critical rolls

Facts [C]:
- `roll_hit(side, id)`: returns `(next() & 0x7F) < chance`. `rec.accuracy` is the base (/128).
  Party attacking: `diff = atk.Boost + atk.accuracy(weapon+crest) - target.Boost`, `level = atk.level - target.level`;
  `kind < 2`: `chance = acc + acc * (diff / 8 + (level - atk.critical)) / 128`; `kind >= 2`: `acc + acc * (diff / 8 + level) / 128`.
  No lower clamp for the party (chance <= 0 never hits; >= 128 always hits). Against a boss (`state.type == 6`) the party's hit
  ignores everything: `(next() & 0x7F) >= 0x2B` (85/128 = 66.4 %).
  Enemy attacking: `diff = atk.Boost - target.Boost - target.evasion`; `chance = acc + acc * (diff / 8 + level) / 128`,
  **floored at 0x20** (25 %).
- `roll_special_hit`: same with `Wisdom` difference `diff = atk.stats[3] - target.stats[3]`, no clamp, boss rule for the party.
- `roll_critical(side, id)`: `chance = 4` (3.1 %). `kind < 2` and attacker `critical != 0`: `critical + 4` (`* 2 + 4` when the acting member is `boosted`).
  `kind >= 2`: only `kind == 0xB` changes it: `effect_power + 4`. Then technique `strong_type == target.type`: `+60`,
  else a weapon strong type matching: `+16`. Then the **target's** statuses add: paralysis (bit 2) `paralysis_power >> 3`,
  sleep (bit 8) `sleep_power >> 3`, confusion (bit 4) `confusion_power >> 1`. Roll `(next() & 0x7F) < chance`.
- `roll_special_critical`: `ratio = element_power * 100 / target.resists[defense_stat - 2]` capped at 64; `chance = ratio + 4`
  (`ratio + 64` with a strong-type match), plus the same target status terms. (Divides by the resist: never 0 in the game's data, section 4; `defense_stat < 2`
  would index `resists[-2..]`, so it is only meant for element techniques.)

[T] `fightstg_rules` pins the four rolls over the same techniques, sides, RNG indexes and modifiers (boss included), and
(appended 2026-10-05) the enemy's floor at both sides of 32 (residue 31 hits, 32 misses, from a chance of -61) and the party's
missing floor (a chance of -33 misses even on residue 0).

Code map: `fightstg_rules_roll_hit`, `_roll_special_hit`, `_roll_critical`, `_roll_special_critical` (`fightstg_8008D3B4.c` 7602-7765);
`RecordsTechnique.accuracy/kind/effect_power/strong_type`; `FightstgStats.critical/accuracy/evasion/strong_types`;
`FightstgBattleState.type` (6 boss).

What a test should check:
- Frequency over the 4096-cycle equals the chance exactly: e.g. technique accuracy 128 (the usual max), level and Boost equal -> chance 128, always hits.
  accuracy 100, equal stats: 100/128 hits, i.e. 3200 of 4096 draws.
- Enemy floor: choose inputs giving chance 5 -> effective 32 (1024 of 4096 hit).
- Boss: party hit probability is the rolls with residue >= 43: 85 of 128 residues, 2720 of 4096 (the 0x2B comparison is `>=`, so not 84).
- Critical base: weaponless, no statuses: 4/128 = 128 crits per cycle. Strong-type match: 64/128. A paralysed target (`paralysis_power 80`): `4 + 10`.
- Crit damage adds `value * (r & 0x3F + 0x20) / 64`: fixed `r` -> multiplier from 1.5 to 1.984. Ensure the crit draw happens only after a successful crit roll.
- Draw counts per action: hit roll 1 draw, crit roll 1, crit magnitude 1 more only if it crits.

## 4. Element and type effectiveness, resists

Facts [C]:
- Two mechanisms. (a) Types: each Digimon has `RecordsDigimon.type` (u8, `FightstgStats.type`); a technique's `strong_type` (>= 2)
  or a weapon's `strong_type` equal to the target's type gives x1.5 damage (`+ value/2`) and bigger crit chance (+60 / +16).
  There is **no table of which type beats which**: the match is by equality with data in the technique/weapon record.
  (b) Elements: 7 resists (Fire..Dark). A technique with `defense_stat = e >= 2` and `element_power` gets a bonus
  `value * element_power * 2 / resist[e-2]`; higher resist, smaller bonus. Resists are raised by training and equipment
  (`RECORDS_BONUS_FIRE..DARK`, items add via `gamestate_add_stat_bonus`) and by level up (to level 40).
- Field element (`FightstgField {element, power}`): boosts same element techniques `+power/128`, weakens the "opposite" ones `-power/256`.
- Resist floor: party resists and stats are clamped to >= 1 in `get_stats`; enemies are not.
- Status resists: `resists[7..11]` feed the status rolls (section 5). Crest/ring items 0x11..0x15 set `resists[7..11]` from `value`.

[T] `fightstg_rules`: `get_field_bonus` for fields {Ice 64, Fire 128, none} x the 9 elements x values 200/201 (the opposite table and
the truncations), the strong-type technique 131 (type 8) against enemy 0x20 (type 8), the Fire crest's element on basic techniques.
**No zero divisor in the game's data:** the original has no division-by-zero trap (GCC 2.8.1 emitted a plain `div`; on the R3000
a division by zero gives a defined result, on x86 it traps), but every divisor here is >= 1: the party's stats and resists are
clamped, and the enemies' raw stats are >= 40 (`SDIGIEDT`, 193 records), their element resists >= 60, `stat_scale` >= 5
(`fieldstg_battle_enemies`), so a scaled stat is >= 12 (>= 6 under a lowering modifier, which keeps half). A port may
still guard the divisions.

Code map: `fightstg_rules_scale_damage`, `fightstg_rules_get_field_bonus`, `fightstg_rules_get_stats` (item/Digimon resist fill),
`fightstg_rules_opposite_elements`; `RecordsDigimon.resists/type/unk_2A`, `RecordsWeapon.strong_type`, `RecordsBonusStat`.

What a test should check: the `opposite_elements` table verbatim; field bonus signs (+ for equal, - for opposite, 0 for none or
`field.element < 2`); weapon strong type applies once even when two weapons match; element crest ranges map 0x153-0x155 -> element 2
(Fire), 0x156-0x158 -> 3, ... 0x165-0x167 -> 8 (Dark) with `attack_element_power = item.value`.

## 5. Status effects, escape, modifiers (FIGHTSTG rules)

Facts [C], all [T] by `fightstg_rules` (every roll and helper below over techniques, both sides, RNG indexes, each `blocked[]`
slot, statuses, crests; all `(next() & 0x7F) < chance` unless noted; `blocked[]` entries only stop the **party's** attempt, before any draw):
- Status rolls `roll_poison/_paralysis/_confusion/_sleep/_knockout`: `chance = base + Wisdom_atk / 8 - (status_resist + element_resist
  + Wisdom_tgt / 2) / 8`, clamped to [1, 127]; `base` = the attacker's weapon chance (`poison_chance`, `paralysis_chance`,
  `confusion_chance`, `knockout_chance`) for a technique of kind < 2, else the technique's `effect_chance`; sleep always takes
  `effect_chance` (there is no weapon sleep chance). The resist pairs: poison `resists[7]` + Water `[1]`, paralysis `[8]` + Thunder
  `[4]`, confusion `[9]` + Wind `[3]`, sleep `[10]` + Ice `[2]`, knockout `[11]` + Dark `[6]`; blocked slots 0..4 in that order.
  Poison returns its power on success (the weapon's `poison_power` or the technique's `effect_power`; the tick is section 2), the others 1.
- Drain: `chance = drain_chance` (kind < 2) or `effect_chance`, no clamp; `blocked[5]` stops it.
- Steal (party only; the enemy's call returns 0): 0 without a draw if `blocked[7]` or the target holds no item; `ratio = Boost_a * 100 /
  Boost_t` capped 200, `percent = (effect_chance + steal) * 100 / 64` (`steal` = the Hack crests' 0x147/0x148 value),
  `chance = drop_rate * ratio * percent / 10000`, success when `next() % 1024 < chance` (successes, the cap and the crest pinned at both
  sides of their thresholds by the appended cases: chances 68, 286, 99).
- Revert / lower-stat / seals: `effect_chance` (lower-stat adds `diff_wisdom / 8`). Revert returns 0 for the eight starter Digimon.
- Counter `roll_counter(side, damage)`: `chance = (damage << 6) / max_hp_of_party_current + 0x20`.
- Escape: party: 0 without a draw if `blocked[11]` or the acting member is asleep (status 8) or knocked out (0x10);
  `(escapes + 1) * 8`, halved if paralysed (status 2), `+ (level_a - level_t)` if positive, `+ (Boost_a - Boost_t) / 10` if positive, `+ escape item`;
  enemy fleeing: 0 without a draw if it is asleep, else 64 (32 if the party wears the no-escape crest 0x13F), halved if paralysed,
  `+ (Boost_a - Boost_t) / 10` (negative too).
- Wake `roll_wake(side, amount)`: `chance = (amount << 7) / Guard + 64 - sleep_power / 2 - delay / 100`, the sleeper's Guard and
  `sleep_power`, `delay` of its sleep event (type 12); without one the original reads 0 there (`tests/host/FINDINGS.md` 6).
  `roll_confused`: `confusion_power - (resists[9] + resists[3]) / 8` of the member itself, no clamp; `roll_paralyzed`:
  `paralysis_power - (resists[8] + resists[4]) / 8`, floored at 32.
- Modifiers `change_modifier(side, member, kind, amount)` (nothing for an empty or 0 HP member): kinds 0..3 act on stats
  `{0 Power, 1 Guard, 4 Boost, 0 Power}`; new modifier `+= stat * amount / 128`, clamped to `[-stat/2, +stat]`; the old modifier
  is removed from the stat first.
- Gauge gain (blast gauge, max 1000): `percent = dmg * 100 / max_hp`, `value = percent^2 / 20`, crest 0x149 +20 %, 0x14A +40 %
  (equipment slots 4 and 5, per copy), cap 1000.
- Gauge accumulation (`wfightmn_add_gauge(side, value)`, WFIGHTMN, on every hit) [T] (`fightstg_rules`, appended by sweep2): only a hit
  on the party (`side != 0`) with `value != 0`, on an acting member with HP and not blasted, adds `get_gauge_gain(value)` to
  `records_state.gauges[the member's Digimon index]` (not its party slot); at >= 1000 the gauge is set to 1000 and a gauge-full event
  (type 0x12, delay 0) is queued (874 + 125 = 999: none; 875 + 125: 1000 and the event). The DV Plug's cap is 999 (section 9).
- MP cost `get_tech_cost(side, id)`: `records_techniques[(id & 0x1FFF) - 1].mp_cost`; party only: MP-saving crest 0x143/0x144 subtracts
  `bonus_value` (min 1); `id & 0x4000` (boosted use) adds `cost / 5` (at least +1). Enemies pay the plain cost.
- First strike (`wfightmn_roll_first_strike`, WFIGHTMN, at the battle's start) [T]: none and no draw when the battle's
  `records_state.first_strike_chance` is 0; else `chance = first_strike_chance * (32 - leader's level - enemy 0's level) / 32`
  and `r = next() % 128`: **`r == 0` always strikes first**, else `r < chance` (so from a level sum of 32 on only residue 0 does).
- Damage cap (`wfightmn_cap_damage(side, value, count)`, applied to every hit) [T]: in battle types 1 and 2 the party's damage
  leaves the enemy at `limit = (s16)(max_hp / 11)` or above (`hp - limit`, or `(hp - limit) / count` per hit of a multi-hit; 0
  when the enemy is already at or below it); type 3: the party deals 0; the enemy's damage and the other types pass unchanged.

- Event queue and delays (`fightstg_events`, 99 entries `{type, delay, args[6]}`) [T] (`wfightmn_events`, sweep3):
  `fightstg_events_get_delay(side, kind)`: kinds 0 (a turn), 1 (escape) and 3..7 (blast) = `base * other.Boost / root` with
  `root` = 10 integer Newton steps `(r + p / r) / 2` from 999 on `p = own.Boost * other.Boost` (at `p = k*k - 1` it ends on `k`,
  where an exact isqrt gives `k - 1`: Boost 50 against 48 -> 979, not 1000); kind 2 (regen ends) `next() % 2001 + Spirit * 10`;
  kind 8 (field, modifier by an item) `next() % 8001`; kinds 9/10/11 (paralysis, confusion, sleep) `next() % 500 + (Spirit +
  status_power) * 8 + 3000` (sleep `+ 1000`) `- (status + element resist of the target) * 8` (pairs [8]+[4], [9]+[3], [10]+[2]);
  kind 12 (modifier by a technique) `next() % 2001 + 2000 + Spirit * 10`; then `fightstg_events_delay_formulas[kind]` = {base,
  min, max} bounds it (0 = none): kind 0 [707, 1414], 1 [176, 353], 2 >= 1001, 8 [2000, 6000] (draws are < 4096: the cap never
  applies), 9/10 >= 1000, 11 >= 500. A turn (`get_delay(0, 0)`, halved for the first turns) is shorter for the faster side.
  `fightstg_events_take_next`: the smallest delay, the first on a tie, but a turn event (types 2, 3) loses a tie to any later
  event (two turns tied: the later one); every delay drops by the taken one; types 4, 6, 9 stay queued (take mode -1), the
  others are freed. `add_first` (end, knock-out, boost end, revert, boss entrance): the queue's delays + 2 and the event at 1.
  `add` with 99 events queued drops the new one. Boss final phase (`start_final_phase`): event 0x18 at 3000, enemy 0's
  modifiers [1] and [3] set to `-stat >> 1` of record 0x1D3 (750, 730: even, so `>> 1` and `/ 2` agree on the game's data).
- Copied techniques (`wfightmn_note_copied_tech(side, id)`, battle type 4) [T]: the party's technique becomes
  `state.copied_tech` when its `anim_script` is not 5 or 12 and its kind is 2..8, 11 or >= 13, or its `defense_stat` >= 2 (`wfightmn_events`).
- The enemy's choice (`fightstg_enemy_turn_update`, FIGHTSTG) [T] (`wfightmn_enemy_ai`, sweep4): in battle type 1 an enemy under
  `(s16)(max_hp / 10)` HP flees (ends the battle, result 0) [C]; asleep (status 8), paralysed (status 2 and `roll_paralyzed`) or
  confused (status 4: a random message, `next() % 8`) it loses the turn [C]; else the first of its record's `actions[3]` whose
  condition holds is taken, and when none holds the 4 bytes after them (record + 0x3E, `{1 or 2, 0, 0}` in every record) are the
  default action (FINDINGS 5). Conditions `fightstg_enemy_check_condition(type, value)` [T]: 0 always; 1 `next() % 128 < value`
  (one draw; the conditions are evaluated in order, so a later roll is drawn only when the earlier ones fail); 2/3 own HP
  `(s16)(max_hp / 100) * (value * 100 / 128)` `>` hp / `<=` hp, both divisions truncated (max_hp 450, value 26: 80, not 90 or
  91; max_hp < 100: 0, never under); 4/5 own MP `<` / `>=` value; 6/7 as 2/3 on the party's current member; 8 the party member's
  Digimon is `records_digimon[0..7].id` (the partners); 9 the party member asleep; 10 another enemy slot alive with Digimon
  `value` (0: any), never the current slot; 11 field element; 12 `records_state.battle`; 13 `records_state.unk_3D`; 14 own
  `power_up`; 15..17 own `modifiers[0..2] < 0`; 18 own `turns % value == 0` (turn 0 holds; values 2..4 in the data, never 0);
  > 18: 1. Actions `fightstg_enemy_get_action(type)` [T]: 1 attack (`fightstg_attack_create`), 2/3 `tech_2`/`tech_3` of the current
  enemy's record, 4 flee (`fightstg_events_add_escape(0x10)`), 5..8 call member 0/1/2/any (-2..-5), else 0. A technique costs its
  `mp_cost` and fails with a message when MP is short [C]. A call (`fightstg_enemy_turn_find_target`) [T]: -2..-4 name a slot,
  refused when it is dead or has the current member's Digimon (an ID compare: a second 0x20 cannot be called by a 0x20);
  -5 any other live slot, two candidates by `next() & 1`, one without a draw. The record's counter action (`counter_*`,
  `fightstg_counter_update`) uses the same two functions [C].
- Battle end (`wfightmn_battle_end`, WFIGHTMN) [T] (`wfightmn_spoils`, sweep5): on a result != 0 (won 1, lost 2: a loss runs
  the same drop, which STFGTREP then never reads) `results.battle = records_state.battle`, `results.member = current[0]`;
  `count` = the enemies with a Digimon and `item != 0` (a stolen item is -1 and counts; an empty slot never does), `pick =
  next() % count` (count 0 when none holds one: a division by zero, harmless on the R3000A but a trap on x86, FINDINGS 7; the
  story's first battle does it), and the item and record read are `enemies[k]` with k the holder's **rank**, not its slot (one
  holder in slot 1 reads `enemies[0]`: no item); `item > 0` drops when `next() % 1024 < drop_rate + 1` (0xD9, rate 128:
  residue 128 drops, 129 not), else (or `item <= 0`: stolen) item 0 with no second draw; `records_state.has_prize == 1`
  replaces the item with `prize_item`. Every party slot with a Digimon writes HP and MP back to its record; HP <= 0 comes
  back as 1 and clears `took_part` and `forms[]`. Result 1 goes to the report (0x1400; battle type 6: 0xE0B), 2 to 0xE00,
  0 (escape: no item logic, the write-back only) back to the field [C]. Layer 2's first battle pins the no-holder path (no
  item, HP 150 -> 93, `battle_won`/`battle_end`).
- Battle start (`fieldstg_start_battle(battle)`, FIELDSTG) [T] (`wfightmn_spoils`): needs the field manager (kind 7) on the
  object list, else nothing; copies `fieldstg_battles[battle]`'s enemies, first strike, `unk_3D` and blocked effects into
  `records_state`, sets the manager's next map (0x600; 0xE0A at progress 0x2B) and `has_prize` = enemy 0 in 0x1C9..0x1D0
  (only battles 327..334). Then the prize: an odd enemy draws `next() & 0x1F`, an even one `next() & 0xF` (a draw of residue
  16 mod 32 is the rare item for an even enemy only), residue 0 the rarer item, by map (`switch`, default 0x177/0x186) and on
  maps 0x28C..0x299 by progress 0x2D (0x1C9 on 0x28C: 0x17F/0x188 at 0x2D, else 0x17C/0x187).

Code map: `fightstg_rules_roll_poison/_paralysis/_confusion/_sleep/_knockout/_steal/_drain/_revert/_lower_stat/_switch_seal/_digivolve_seal/_counter/_escape/_wake/_confused/_paralyzed`,
`_change_modifier`, `_get_gauge_gain`, `_get_tech_cost`, `wfightmn_roll_first_strike`, `wfightmn_cap_damage`, `wfightmn_add_gauge`, `wfightmn_battle_end` (`src/wfightmn/wfightmn_800A6440.c`), `fightstg_enemy_check_condition`, `_get_action`, `_turn_find_target`, `_turn_update` (`src/fightstg/fightstg_80086A00.c`); data `fightstg_rules_modifier_stats`, `records_state.blocked[12]`, `FightstgMember` status fields.

What a test should check: clamps (chance <= 0 becomes 1, so a roll of residue 0 still succeeds; chance >= 128 becomes 127);
`blocked[]` short-circuit only for `side == 0` and consumes no RNG draw (compare draw count); escape fixture: equal levels and Boost,
`escapes = 0`, `escape = 0` -> chance 8 (8/128); `escapes = 3` -> 32; steal with an empty enemy item returns 0 without a draw;
modifier clamp (amount 128 twice on Power 100: second add capped at +100; amount -256: capped at -50);
tech cost fixture: cost 10, crest bonus 3 -> 7; bonus 20 -> 1; boosted flag on cost 10 -> 12, cost 3 -> 4.

## 6. Stat growth, experience, training

Facts [C] (`stfgtrep_add_exp` and `stfgtrep_raise_stats`, `src/stfgtrep/`; tables are in STFGTREP `.data`):
- Experience `rec->exp += exp`, capped 999999 (the form's `GamestateForm.exp` comment says 9999999; the member's cap here is 999999).
- Level up while the next level `lv` (`current + 1`) satisfies `(lv^3 + 5*lv - 6) * exp_curve / 10 + bonus[k] < exp` where
  `bonus = {0, 50, 800, 3000}` and `k` = band of `lv` (`< 5`: 0, `< 20`: 1, `< 40`: 2, `< 100`: 3; `lv >= 100` stops). Level caps at 99.
  Each level also does `values[1] += 5` (cap 99) and calls `stfgtrep_raise_stats`. Several levels can happen in one call (the loop repeats).
- HP/MP: `gain = hp_mp_gains[0 or 1] - cut[k] + random[next() % 9]` with `cut = {0, 5, 10, 15}`, `random = {-4..4}`; max HP and max MP
  capped 9999 (current HP/MP are not changed here).
- Six stats (Power..Charisma): `gain = stat_gains[band][class + next() % 5]` with band by level (< 5, < 20, < 40, < 60, < 80, else) and
  `class = RecordsDigimon.stat_gains[i]`. Table rows (9 entries each) fall from `{2,3,4,6,8,10,12,13,14}` (band 0) to `{0,0,1,1,1,1,1,2,2}`
  (band 5). Cap 999.
- Resists (only when `lv <= 40`): `resist_gains[class + next() % 4]`, `{0,0,0,1,1,1,2,2}`; cap 999. Class 5 (Kotemon's Water,
  Digimon 7's Dark) with residue 3 indexes entry 8, past the table and past the overlay file: the original reads 0 from `.bss`
  (`tests/host/FINDINGS.md` 2). Likewise `stat_gains[k][class + next() % 5]` with class 5 and residue 4 reads the next row's
  first entry (`resist_gains[0]`, 0, after the last row).
- Draw order per level: HP gain (1 draw), MP gain (1), six stats (6), resists (7 when `lv <= 40`).
- Setters `gamestate_set_stat/_add_stat`: stat index 0..18 on `values[]`: `< 0` -> 0; indices 0-1 capped 99, 2-5 capped 9999, the rest 999; out of range
  index (>= 19) ignored.
- Item-boosted stats: `gamestate_get_stats` copies the record, adds equipment (weapon power into Power, armour guard into Guard, charisma of every piece
  into Charisma, plus the two bonus stat/value pairs via `gamestate_add_stat_bonus`: type 7 all six, 1-6 one stat, 8-14 resists; each capped 999),
  subtracts `penalties[0,1,2]` from Power, Guard, Boost (floor 0), and adds a set bonus when the four first equipment slots equal `gamestate_item_sets[i]`.
- `GamestateStats.values[1]` is the **TP** (training points) [C]: +5 per level-up (cap 99, `stfgtrep_add_exp`), spent by training
  (`stgtrain_tp_costs` = {1, 5, 10} for trainer levels 0, 1, 2; `stgtrain_level_run` refuses a level the TP don't cover: "Not enough
  TP.", nothing spent [T] by replay `first_battle_save` (`train_left`: TP 0, level 0 refused; the party has no level-up yet)).
- Training (STGTRAIN, `stgtrain_rules` golden) [T]. A session trains one party Digimon at a menu entry (`training`, 1..24) with trainer
  level `level` (0..2). Each won round calls `stgtrain_session_apply_round`, which looks the entry up in the menu
  (`stgtrain_find_menu_entry(map_entry, training)`, a map outside 1..14 reads menu 0: `stat` 1-5 raises a stat, 8-14 a resist; a resist
  entry's `stat_2` also lowers a stat 1-5 or raises max HP/MP 15/16, kept in `losses[]`; a round result of 0 applies nothing, any other
  value, -1 "not played" included, applies) [T]:
  - Stats (type 1..5 = Power, Guard, Spirit, Wisdom, Boost; Charisma is never trained): `+ stgtrain_stat_gains[row]`, row =
    `(training >= 13 ? 6 : 0) + bonus * 3 + level`, gain `base + next() % rand` (no draw when `rand` is 0): rows
    {1+r2, 7+r2, 14+r3, 2, 10, 20} for trainings below 13 and {1+r2, 6+r3, 11+r5, 4, 22, 48} from 13 on (`rN` = 0..N-1). Cap 999.
  - Loss (type 1..5, the entry's `stat_2`): one draw; odd: no loss; even: `- (base + next() % rand)` from
    `stgtrain_stat_losses[level]` = {1, 2+r3, 4+r3}; floor 0.
  - Resists (type 8..14): table `stgtrain_resist_gain_tables[(class - 1) * 3 + size]` with `class` =
    `RecordsDigimon.resist_gains[type - 8]` (1..5) and `size` 0 below 100, 1 below 300, else 2; the 15 slots map to three arrays
    {0,1,1, 0,1,1, 1,1,1, 2,2,1, 2,2,1} (classes 4 and 5 gain *less* from 300 on); row = `level * 3 + (bonus ? (training < 13 ? 1 : 2) : 0)`
    (training matters only with the bonus here). Cap 999.
  - Max HP/MP (15/16): `stgtrain_max_gains[same row]` = {1+r2, 2, 4, 6+r4, 10, 20, 13+r5, 20, 40}; cap 9999.
  - Every raise/loss function returns the full roll even when the cap or floor cut it (the result screen recomputes what is shown from
    the stats saved before the session).
  - Success chance [C] (`stgtrain_trainee_update`, not a pure function): a round succeeds when `chance >= next() % 100`, so chance 75
    succeeds 76 times in 100; chance = 75 + `stgtrain_session_get_bonus` [T] (3 with item 0x151 in equipment slot 4 or 5, else 6 with
    0x152, else 0; 0x151 wins when both are worn). Rounds [C] (`stgtrain_session_run`): trainings below 13 have 3 rounds, and when all 3
    succeed a bonus round follows at once with the same chance (the last-bonus rule is not checked); from 13 on there are 5 rounds,
    and when the first 3 all succeed the player is asked whether to play the bonus round instead of rounds 4-5: its chance is 50, or 0
    when this training won the previous bonus (`GamestateRecord.last_bonus_training`). Either way a won bonus round sets
    `last_bonus_training` to the training and a lost one clears it; winning sets `bonus = 1` before the round's gain, so that gain
    already uses the bonus rows.
  - Menus [T]: `stgtrain_get_menu(menu)` keeps 1..14 and maps anything else to 0; `stgtrain_menus` has rows 0..13 only, so menu 14
    reads 16 entries past the table (the start of `stgtrain_trainings`; the original counts 12 IDs there, and the host build agrees
    only because the linker put the same table next). Menu 14 is never reached: STGTRAIN's `map_entry` is a trainer talk flag's index
    (`0x94xx`, indexes 0..13 in the census), STAGSLCT's (0..7) or -1 (field map changes); `tests/host/FINDINGS.md` 3.
- New game values [C]: `gamestate_init_records` sets level 1, HP/MP from `start_hp/start_mp`, six stats and seven resists from `RecordsDigimon`
  [T] as a whole by replay `new_game` (it runs during the boot, before `cnty_sel`; every checkpoint hashes the records).

Code map: `stfgtrep_add_exp`, `stfgtrep_raise_stats` (`src/stfgtrep/*`), tables `stfgtrep_level_exp_bonus`, `stfgtrep_hp_mp_gain_cut`,
`stfgtrep_hp_mp_gain_random`, `stfgtrep_stat_gains`, `stfgtrep_resist_gains`; `gamestate_set_stat`, `gamestate_add_stat`, `gamestate_get_stats`,
`gamestate_add_stat_bonus`, `gamestate_init_records` (`src/main/gamestate.c`).

What a test should check: threshold fixture with `exp_curve = 10`, band 0 (bonus 0): `lv^3 + 5lv - 6` is 12 for lv 2 and 36 for lv 3, so level 1 -> 2 needs `exp > 12` and
2 -> 3 needs `exp > 36`; exp exactly at the threshold does not level (strict `<`);
a big exp award gives several levels in one call and returns 1; cap level 99; with `seed(0)` and a fixed record, the full list of gains is deterministic
(this is a good golden: dump the record after N level ups); `set_stat(…, -5)` -> 0, `set_stat(0, 0, 150)` -> 99, `set_stat(i, 2, 20000)` -> 9999, `set_stat(i, 6, 2000)` -> 999;
`add_stat` overflow of an s16 (`*p += value` as s16) is the edge to record on the PS1 golden.

- [T] The story's first battle (scripted, Asuka City at progress 3; `first_battle_save`, checkpoints `battle_start`,
  `battle_won`, `battle_end`): of the save slot's bytes (the playtime aside) the battle changes only
  `digimon[0].record.stats.values[2]` (HP, `gamestate_data` + 0x788: 150 -> 93);
  the report (STFGTREP) then adds the prize money (`money`, + 0x6C) and `digimon[0].record.exp` (+ 0x780), no level-up. The outcome and the stable hashes are the same on both CPU cores although
  the battle takes a different number of frames.
- **Who gets a battle's experience** [C] (`stfgtrep_main_update`, step 1 of its init; the party mod of issue #28 hooks here, and the XP boost mod multiplies the three rewards):
  - The amount is per battle, not per enemy: `stfgtrep_rewards[records_battle_results.battle]` = `{tech_exp, exp, money}`
    (335 rows, STFGTREP `.data`), `battle` copied from `records_state.battle` (set by `fieldstg_start_battle`) when the battle is
    won (`wfightmn` result step). Enemy levels and the party's levels play no part.
  - `n` = party members with `records_battle_results.members[i].took_part`: set when the member acts (`wfightmn_mark_took_part`,
    the side's `current[0]`) or is chosen in the command menu (`wfightmn_mark_chosen_took_part`), and cleared again with its `forms[]`
    at the end for a member at 0 HP (which is left at 1 HP). The battle has one acting member per side; the others wait to be switched in.
  - It is split, not copied: n = 1 (or 0) -> `exp` each, 2 -> `exp * 6 / 10` each (120% in all), 3 -> `exp / 3` each.
    A member that did not take part is created with 0; the report skips its panel's experience step (`member_exp == 0`), so it gets no
    experience, no form experience and no new-form check (`learn_technique`) this battle.
  - Item 0x141 (equipment slot 4 or 5) then adds a fifth to the member's share (`stfgtrep_member_get_exp`, through `get_exp`).
  - Base Digimon versus forms: the share goes to the party Digimon's own record (`digimon[id].record.exp` and its level, through
    `stfgtrep_add_exp` in the panel's step 40) whichever form it fought as. A form keeps its own level and experience
    (`GamestateForm`, section 7) and gains only `tech_exp`, for the chosen forms marked in `forms[]`; the Digimon's own form gains none.
  - Only the three party members have panels; the five other Digimon (`gamestate_data.digimon[8]`, not in `party[3]`) get nothing.

## 7. Digivolution (forms)

Forms ("Digivolutions") are records_digimon rows 9..52 (1-based; IDs such as 386, 387, 5, 259...); a party Digimon owns up to 44 of them
(`GamestateRecord.forms[44]`, `GamestateForm {id, level (s8), exp, techniques[6]}`), fights as up to three (`chosen_forms[3]`) and shows one
(`shown_form`). `gamestate_add_form`, `_find_form`, `_get_chosen_forms`, `_set_chosen_forms`, `_list_forms`, `_get_form`, `_put_form` are
covered by `gamestate_records`. The C of STFGTREP calls a form a "technique" (`stfgtrep_learn_technique`, `StfgtrepLearn.technique`).

Rules established by the `stfgtrep_forms` golden [T] (the battle report runs them for each party member: form experience for the chosen
forms that fought, then skills, then the Digimon's experience, then a new form):
- **Gaining a form** (`stfgtrep_learn_technique(d)`): `stfgtrep_learn_lists[d]` lists all 44 forms in a per-Digimon order with
  requirements: up to two other forms at a minimum form level (`requires[i] = {row, level}`), and one stat minimum (`stat` 1-6 the six
  stats, 7 the Digimon's level, 8-14 the seven resists; `min`). The first form in list order that is not owned and meets all of them is
  gained: `add_form` (form level 1), and the first free chosen slot gets it after `get_chosen_forms` packed the slots
  ({386, -1, 367} becomes {386, 367, new}); with three chosen it is not chosen. One form per call; 0 when none qualifies. Example:
  Digimon 0 gains 386 at level 5 (and Digimon 7 gains 20 at level 5: each list differs), 389 at level 20, 387 once 367 is form level 20,
  388 with 367 at 30 and Boost >= 280, 367 with 375 at 50 and Fire resist >= 200, 254 only with both 259 and 260 at 5. Edge: with all 44
  form slots taken by IDs the game never finds (below 3), `add_form` fails but the call still returns the ID and `set_chosen_forms`
  writes -1 for it (unreachable: 44 slots hold at most the 44 list forms).
- **Form level** (`stfgtrep_add_technique_exp(d, form, exp)`): exp capped 9,999,999; while the next level `L + 1 <= t5`
  (`level_thresholds[5]`, 100 for the early forms, 95, 90, 80 or 60 for later ones) level L needs total exp >= `L * 10`; past it
  `x * 10 + (L - x) * 50` with `x = t5 - 1` (form 150, t5 60: level 60 -> 61 needs 640). Several levels per call; level cap 99. A form already
  at 99 returns 0 **without storing the exp**.
- **Form experience from a battle** (`stfgtrep_get_technique_exp(d, form, exp, n)`): `a = exp * 10 / level` (Digimon level < 51) or
  `exp / 5` (the two agree below overflow); n users: 0 or 1 -> a, 2 -> `a * 6 / 10`, more -> `a / n`; then at least 1, and at most 10
  while the form's level < t5, else at most 50. `exp` is `stfgtrep_rewards[battle].tech_exp`; a chosen form (ID >= 4) earns it when
  `records_battle_results.members[slot].forms[i]` was set in battle (`wfightmn_mark_took_part`: `took_part` for the acting member, and
  the form it was fighting as when it acted, digivolved or switched, by its place `i` in `get_chosen_forms`' packed list: chosen
  {386, -1, 367} fighting as 367 marks `forms[1]`; its own form or an unchosen one marks none [T] (`fightstg_rules`, appended by sweep2)).
- **Skills** (`stfgtrep_learn_skill(d, form)`): the first empty slot i (0..5) whose `RecordsDigimon.techniques[i + 1]` exists and whose
  `technique_levels[i] <= form level` is filled (slot 5 with flag 0x8000); one per call; a later slot can fill first when the levels are out
  of order (form 375 learns slot 2 at level 10 before slot 1 at 25), and a missing technique leaves its slot empty for good.
  `stfgtrep_mark_skill`: the first slot i < 5 with a positive technique (0x8000 makes it negative: never) without 0x2000 and
  `level_thresholds[i] <= form level` gets 0x2000; returns the technique & 0x1FFF. 0x4000 does not prevent it.
- **Experience bonus item** (`stfgtrep_member_get_exp`): `exp + exp / 5` once when item 0x141 is in equipment slot 4 or 5 (not 0..3);
  the money bonus item 0x142 is the same rule in `stfgtrep_main_run` [C].
- **Blast form** [C] (`wfightmn_blast`, a battle scene, not testable alone): `RecordsDigimon.blast_forms[band]` (records_digimon index + 1)
  by the Digimon's level band `< 4, < 19, < 39, < 70, more`; the member's HP is refilled when the form appears; `wfightmn_end_blast`
  zeroes `records_state.gauges[id]`. The table itself is in the `records` golden's `records_digimon` hash.
- **In-battle form change** [C] (FIGHTSTG's command menu and `wfightmn_digivolve`/`wfightmn_switch_member`/`wfightmn_revert_form`):
  UI objects; they set `FightstgMember.digimon` to a chosen form and call `wfightmn_mark_took_part`. Not covered by a golden.
- **The lab's chart** (STGDGLAB, `stgdglab_chart_*`) only displays: `stgdglab_funcs.charts[d]` has 4 pages of 4 lines of up to 5 form IDs
  with a count; a line is complete when `count` of its IDs are owned. It changes no state; not covered.
- **Entering the lab** compacts the party (`stgdglab_main_pack_party`): members keep their order, the empty slots (any negative
  value) go last as -1 [T] (`stgdglab_party`).

Not covered and why: the functions above with a form the Digimon does not own (their `GamestateForm` stays unset: undefined, and the
report only passes chosen forms that levelled up), a Digimon level of 0 in `get_technique_exp` (division by zero, unreachable).

## 8. Techniques

Facts [C]: `records_techniques[id - 1]` (443 entries, `RecordsTechnique`, 0x12 bytes): `mp_cost`, `power`, `element` (icon only), `target` (3 = one member),
`accuracy` (/128), `defense_stat` (0/1 physical-ish, >= 2 element), `element_power`, `strong_type`, `kind` (< 2 weapon-driven; >= 2 own `effect_chance/power`;
0xB boosts crit chance), `effect_chance` (/128), `effect_power`, animation fields, `anim_script` (5, 6, 0xB, 0xC change hit behaviour), `hit_count` (kind >= 2, else 3).
The basic attack of a Digimon is `RecordsDigimon.techniques[0]` (counter damage tests `id == techniques[0]` for "own" attack); `[1..6]` learnt at `technique_levels[i]`;
`[6]` is read by FIGHTSTG's status update. Technique ID flags in battle: `id & 0x1FFF` the number, `0x4000` boosted use, `0x2000`/`0x8000` form flags.
Out of battle [T] (`ststatus_items`): STSTATUS heals with `power * 64 + power * Wisdom / 8` (`ststatus_calc_heal`, Wisdom with equipment), same shape as `get_heal`; only the techniques 0xB8..0xBC (power 8, 32, 127 on one member; 32, 64 on the party) are offered, collected from the member's chosen forms (ID >= 3), each once, at most 5. One-member use: only if the target's HP < max (capped), else refused with no MP taken; party use: every member below max HP (capped), MP taken once if anyone was healed. `ststatus_tech_use` does not check the MP (it can go negative); the menu checks it first (`ststatus_tech_get_usable`: cost <= MP).

Code map: `records_techniques` (`src/main/records.c`), `fightstg_rules_*`, `ststatus_tech_use`, `ststatus_calc_heal`.

What a test should check: data-table golden (hash of the 443 x 0x12 bytes from the disc EXE; a regression of the table); `get_tech_cost` strips flags; fixtures: a
technique with `kind >= 2` uses `effect_chance` not weapon chance; `hit_count` 3 default for kind < 2.

## 9. Items

Facts [C]: `records_items[id - 1]` (402 entries, `RecordsItem {data, price, sell_price, category, type}`), `records_get_item(id)` (`records_funcs.get_item`).
Types: 2-14 weapons, 15-20 armour, 21-24 accessories (rings and crests), 25-28 usable, 29 key. Weapon: `power` -> Power, `accuracy`, `status_chance/power`, `strong_type`, two bonus stat pairs.
Armour: `guard` -> Guard, `evasion` (the code in `get_stats` reads byte 0x10 as `status_chance` for non-weapons and adds it to evasion), two bonus pairs.
Accessory: one bonus pair; crest IDs 0x13C-0x167 trigger battle effects in `get_stats` (list in section 5); ring IDs raise a stat or a status resist (types 17-21).
Status weapons by ID (all in `get_stats`, equipment slots 0, 2, 3 only: `i != 1`): 0x97 poison, 0xD2 paralysis, 0xB4/0xC2 confusion, 0x6D/0xBA knockout,
0x5E/0x93/0xAD drain, 0x96/0xBF critical (`status_power`). Usable (`RecordsUsable`): `flags` bit 0 menu, bit 1 battle; `effect` 1 heal HP by `amount`, 17 raise `values[1]`,
others a stat raise table `ststatus_stat_raises`. Money: capped 9,999,999, floor 0 (`gamestate_change_money`). Item counts: `items[0x193]` / `items_equipped[0x193]` as s8
(`gamestate_change_item`), cards 0..9.
Battle item cures (`fightstg_item_cure_*`) are small `{id, status mask, text}` tables for poison (mask 1), paralysis (2), confusion (4), all (63).

- [T] **Item use from the status menu** (`ststatus_items_use`, `tests/golden/families/ststatus_items.py`): by `RecordsUsable.effect`.
  1 (the Charges 43-46: 500, 2000, 5000, 9999 HP): only if HP < max HP, then HP += amount capped at max HP (HP 0 heals like any
  other value: no knockout outside battle; HP above max is left alone). 17 (Train Chips 62-65: +1, +2, +3, +5): only if the TP
  (`values[1]`) < 99, a result >= 100 becomes 99. 2..16 (the Chips 47-61) through `ststatus_stat_raises`: 2 max HP and 3 max MP
  (cap 9999), 4..9 Power..Charisma, 10..16 the seven resists (cap 999); only if the stat is below its cap: one draw,
  `+= next() % amount + 1` (amount 30 for HP/MP, else 10), capped; the HP Chip raises max HP only. Anything else (0: the 25
  battle-only items 66-90, the boosters) does nothing. A use that did something costs one of the item (`items[id]--`, an s8
  with no floor: a count of 0 becomes -1); a refused use costs nothing and draws nothing. There is no MP-healing or reviving
  item outside battle.
- [T] **Shops' helpers** (`shop_rules`): `stitshop_get_items(shop)` (31 shops 0..30, goods counts 2..32, shop 30 = all 52, the
  STAGSLCT debug shop; shop 31 reads `stitshop_funcs` past the table, FINDINGS 4); `stitshop_can_equip` = bit `digimon` of
  `RecordsEquip.members`; `stitshop_get_slot`: head -> 0, body -> 1, shield -> 3, two hands -> 2; one hand: the empty one of 2/3,
  else 2 if slot 2 holds a two-handed weapon or slot 3 a shield (type 20), -1 if either is not a weapon, 2 if slot 2's
  weapon has less power, else 3 (ties too); ring: an empty one of 4/5, else the one with the smaller `bonus_value` (ties: 5,
  rings and crests compared alike); crest: empty, else the slot holding a crest of its group (4 first), else the smaller
  `bonus_value`. `stitshop_equip(digimon, slot, item, take)`: a two-handed item takes out the other hand's item, removing one
  empties both hand slots, a crest takes out the crests of its group, rings have no group rule; with `take` each item taken
  out goes back to the bag (`items_equipped--`, `items++`) and the new one leaves it (a two-handed weapon counts once).
  `stitshop_info_get_stats` (the "with the item" preview) is `gamestate_get_stats` without the item-set bonus.
  Card shop: `stcrdshp_get_price(card)` from `stcrdshp_prices` (44 cards, 500..13,000; every stocked card has one), 1 for any
  other card; `stcrdshp_find_shop(id)`: shops 49..67 and 70..74, an unknown ID gives shop 0; 6 cards each.
  STSTATUS's equipment page (`ststatus_items`) uses the same rules: `ststatus_equip_item` is `stitshop_equip` with the bag
  counts always moved; `ststatus_can_equip` is the members bit plus a shield (kind 2) never in slot 2.
- [C] **Buying and selling** (UI state machines, not in layer 1: `stitshop_buy_run`, `stitshop_sell_run`, `stcrdshp_run_buy`;
  layer 2 is the place): buying needs `money >= price` and fewer than 99 of the item; the count's maximum is
  `money / max(price, 1)`, lowered so the bag stays <= 99 (`99 - items`); up/down by 1 and 10 (down by 10 below 10 gives 1),
  re-capped to `money / price`; the bag is capped at 99 and `money -= price * count`. Selling lists only items with
  `sell_price != 0` (`stitshop_list_collect`), gives `sell_price * count` capped at 9,999,999, the bag floored at 0.
  **The sell price is its own field**: for all 234 items with a price it is `price / 2` (truncated; 5 odd prices), 60
  unbuyable items can still be sold (price 0, sell price > 0), 108 can be neither (`src/main/records.c`). Cards are bought
  one at a time: refused below the price or when 9 are owned (`gamestate_data.cards[id] == 9`), else `add_card(id, 1)` and
  `money -= price`. A booster pack (`stcrdshp_run_booster`, still asm) draws six cards, card i =
  `stcrdshp_booster_contents[k].choices[i][next() % 16]` (k the last entry for that item, 0 if none), adds each, and uses one pack.
- [C] **Battle items** (`fightstg_item_update`, a message-driven object; not callable as a pure function, so not in layer 1):
  43-46 heal the acting member by `amount`, capped ("no effect" at full HP); 66-69 clear status bits 1/2/4/63 (poison,
  paralysis, confusion, all) through `fightstg_item_cure_*`, "no effect" if none is set; 70 Life Disk revives every party
  member at 0 HP to full HP (the only revive); 71 Life Plug heals half max HP and half max MP (capped; "no effect" only when
  both are full); 72 Mach Plug: Boost modifier `+= Boost * amount / 128` capped at +Boost; 73/74 Power/Guard Plug: that
  modifier `+= stat * 64 / 128`, the other `-= stat * 64 / 512` (floor -stat/2); 75 Aura Plug: the next technique is boosted;
  76 DV Plug: blast gauge `+= 200`, capped at 999; 77-83 a field of element 2..8 at power 0x40, 84 the battle's own field
  element at 0x7F; 85 Crimson Cable drains 1/5 of the enemy's max HP into the user (capped; blocked by `records_state.blocked[5]`); 86 Chaos Wave
  confuses the enemy on an odd draw, then the user too one draw in four; 87 Charm Gas raises the enemy's Power by 30% and
  halves its Guard; 88 Cursed Puppet lowers its Power or Guard (`obj->lowered`, set in `fightstg_item_run_script`); 89 Spider Web lowers its Boost by `amount / 128`; 90 TNT Ball deals technique 0x89's special damage.
  The bag loses one after the effect. A replay through a battle that uses items is the place to pin these.
Code map: `records_get_item`, `records_is_item_category`, `gamestate_change_item`, `gamestate_unequip_item`, `gamestate_get_stats` (`src/main/*.c`),
`ststatus_items_use` (`src/ststatus/ststatus_8008E94C.c`), `fightstg_item_cure_*`, STITSHOP (price/sell/equip rules: `stitshop_can_equip`).

Shops [C]: a talk that sets flag `0x7A00 + n` (n < 30) opens STITSHOP (map 0xF00, `map_entry` n = `stitshop_shops[n]`); the first shop
reachable from boot is the Asuka Item Shop (map 0x20D, WSTAG245, after the first battle): counters for shops 0 (weapons and armour), 1
(accessories) and 2 (usable items), each opened by the shopkeeper's second talk (the first sets a `0x1Axx` greeting flag). Buying
(`stitshop_buy_run`): refused with "Not enough BIT." when `money < price` and with a message at 99 owned; the quantity starts at 1 and is capped
at `min(money / price, 99 - owned)`; Yes adds the quantity to `items[id]` (capped 99) and takes `price * quantity` from `money`.
- [T] `first_battle_save`, checkpoints `shop_open`, `shop_bought`, `shop_left` (after the load): shop 2, Power Charge (item 43, 12 BIT) with 50 BIT:
  the quantity stops at 4 = 50 / 12, so `money` (`gamestate_data` + 0x6C) 50 -> 2 and `items[43]` (+ 0xA7) 0 -> 4; asking again with 2 BIT is refused
  and changes nothing.

What a test should check: record counts and table hashes; `get_stats` fixtures per item type (a weapon with power 30 on Power 40 gives 70, capped 999; charisma sums across six
slots); two set items; set bonus only when slots 0-3 match exactly; `gamestate_change_money` clamps (add past 9,999,999; subtract below 0); `ststatus_items_use` heal clamps via `gamestate_add_stat` (HP above max HP is not clamped to max HP in that code path; verify).

## 10. Card game (CARDGAME, `card.c`)

Facts [C]:
- Rules shape (header comments): a match is best of rounds, two won rounds win. Phases: choose deck (2), decide who starts (3), deal (4), per round: first play phase (5),
  slot placement (6), second play phase (7), round end (8). Cards are Digimon cards (record byte 3 kind 16, 43 per colour 1-6) or option cards; effects are
  scripts (60 effect scripts, `cardgame_card_data`), run by `cardgame_run_effect`. Each board slot holds a card with `attack` and `hp` fields. Decks: 40 cards
  (`GamestateDeck.cards[40]`, 3 saved decks); cards owned `gamestate_data.cards[0x13D]`, counts 0..9 (`gamestate_change_card`).
- CPU score `cardgame_cpu_get_score(game, side, mask)`: sort the side's slot cards by card ID; sum `attack + hp` of cards **not** in `mask` (bit i = slot i excluded);
  plus a combination term: runs of the same card ID count when the card's record field at +10 (s16) is non-zero ("combinable"); a pair alone adds nothing; a triple adds the three
  cards' attack+hp **again**; a run of four or more adds 20 to both attack and hp sums (so +40 total). Result `rest_a + rest_b + sum_a + sum_b`.
  The combination logic has an `i`/`skip` quirk for masked cards (read before asserting masked cases).
- CPU choice `cardgame_cpu_choose_card(game, board)` marks at most one hand card (`game->marked[i] = 1`) and returns 1/0:
  - Own turn (turn 0), phase 5: first hand card of CPU "kind" 1 that is playable (`cardgame_is_card_playable`), usable (`cardgame_cpu_can_use_effect`) and has a target
    (`cardgame_cpu_choose_target`). Phase 7: kind 3 cards only, and only if `swap_count` is even and the player's score (side 0) >= the CPU's (side 1) (the CPU does not use a
    swap card while it is ahead).
  - Reply turn: if the player's card ID is in `cpu_counter_ids` (0xFF ends the list), try kind-4 cards; else if the player's card record kind (byte 3) is 3 or 9, answer with a
    CPU card whose record kind is the opposite (3 vs 9), via `cardgame_cpu_choose_counter_target`.
  - `cardgame_cpu_get_card_damage`: card IDs 0x12 -> 60, 0x14 -> 15, 0x38 -> 10, 0x15 and 0x2E -> 30, else 0; `cardgame_cpu_get_card_heal`: 0x2 -> 15, 0xC -> 50, 0xF -> 20,
    0x3 and 0x28 -> 30, 0x3A -> 10, else 0.
  - Targeting helpers: `cardgame_cpu_set_target`, `_set_target_kind_3/_4`, `_keep_best_target`, `_filter_targets`, `_set_cheapest_target`, `_is_target_within`, `_find_kind`.
- Opponent data: `CARD_NPC` (170 x 0xD0): 40 deck entries `{card, stage, kind}`, counter list, level, prize index; the CPU may use a card from stage `round * 2 + 1/2` on.
- Win flag: `gamestate_flags.card_game_won` becomes flag 0x10 when leaving map 0x700 (`gamestate_update_map_flags`).
- Who plays [C]: a talk sets flag 0x76xx/0x78xx (`gamestate_start_card_game`: map 0x700, opponent `index * 2 + 1 or 2`). Every opponent
  in the early towns (Asuka City 0x200, the card arena 0x211, 0x21D, ...) is gated by flag `0x7200` = party Charisma total >= 60
  (`gamestate_party_stat_levels[0]`); the party has 1 + 1 + 1 = 3 after the first battle (level 1), so no card match is reachable from boot without
  grinding levels. The player starts with 40 cards in three identical saved decks (`gamestate_init_cards`); the first card shop (STCRDSHP, arena
  0x211) sells nothing under 1,000 BIT. Not replay-tested (tests/README.md, layer 2).
- **The card case** (item 0x192) [T] (first_battle_save `card_case`, `status_open`, `album_open`, `card_shop_open`): the arena's
  counter (actor 220, a talk redirect to the clerk 206) without the case starts event 1415 (flag `0x905C`, indexed event 0x5C),
  whose end gives `items[0x192] = 1` and sets `0x1A1C`, `0x40A1` (and the talk `0x1C48`). With the case the field menu gets its
  sixth option (`fieldmenu_update`: `items[0x192] != 0`), STSTATUS's party page, whose menu opens the card album (STCRDABM, map
  0x1200) and the deck editor (STCRDDEK, 0x400; both return to the previous map); and the clerk is placed in its shop form
  (flags `0x8192` and condition 9 = progress 4..99), whose second talk sets `0x7A31` (STCRDSHP shop 0x31). Actors are placed when
  a map is entered: the shop form appears only after leaving and re-entering the arena.

Tested [T] (layer 1: the `cardgame_cpu_choice` and `cardgame_rules` goldens, 2026-10-05; every case is replayed on the host
and matches). "Colour" is card record byte 0 (1-6, `kind` in the C), "record kind" byte 3 (0x10 Digimon, 1-14 options),
"class" `card_classes[record kind]`; card indexes 0..39 are the player's cards, 40..79 the CPU's (`card_ids[index]` = card ID).
- **Classes and playability:** class 0 for Digimon (record kind 0x10) and record kind 0/15, 2 for record kinds 4, 7, 8, 11, 14,
  else 1; `card_select(n <= 0)` selects card 0. Phase 5 plays class 2 only, phase 7 classes 1 and 2, phase 6 (placement)
  only Digimon with `points[colour - 1] >= level` (byte 5), any other phase nothing.
- **Points** (after the deal, `cardgame_player_add_card_point` for hand cards 0..5 of each side): a card of colour 1-5 (option
  cards too) gives its owner one point of that colour, `points[colour - 1]`, capped at 99; a colour-6 card gives none (no
  card has colour 0) [T] (`cardgame_rules`, appended 2026-10-05).
- **Slot cards go back to their owner** (`cardgame_slot_to_discard`, `_to_hand`, under the discard/return effects): a slot
  card goes onto the discard pile or into the hand of `CardgameSlot.owner`, which can differ from the side it sits on [T]
  (`cardgame_rules`).
- **Conditions that void a card** (`cardgame_check_condition`, card data byte 1; "side" played the card): 1 the other
  side's hand is empty; 2 it holds no colour-6 Digimon; 3 all its cards are colour 5 (an empty hand too); 4/5 side's
  discard pile/deck is empty; 6 the other's deck is empty; 7/8 side's deck from `deck_pos` to 40 has no Digimon / only
  Digimon; 9 the turn's target slot is gone (clears `marked` up to it); 10 no slot in the target area (kinds 1-3; 4-8 ask
  the board); 11 the target card has left side's hand; anything else 0.
- **Script control** (`cardgame_run_effect`): 0x02 skip 2; 0x03/0x04/0x05 repeat count 2/3/5 from the current entry, 0x06
  decrements and jumps back while it stays > 0; 0x07/0x08 skip 1/2 when the CPU played the card; 0x09/0x0B skip 9 when
  side's hand <= 10 / <= 3, 0x0A/0x0C back 9 when > 10 / > 3, 0x0D skip 4 when > 2; 0x0E skip 2 when the other's deck
  is not empty; 0x0F back 4 when the other's hand has a card not of colour 5; 0x10 skip 5 when side has < 6 slots; 0x11
  hands the card to the other side. Every control effect sets `effect_state` 2 (back to the script).
- **The interpreter** (`cardgame_game_resolve_effect`): bonuses first (0x5D, then 0x4C close gaps, 0x5A recount), the
  condition check (step 6: a failed condition shows the card's message instead of the script), one script entry per
  step 7 (the entry becomes `new_effect`), and at step 12 the played card goes on its owner's discard pile, except card
  13; the turn counter goes back by 1, by 2 when the card cancelled the one before it.
- **Draw marking** (0x45 two, 0x46 `3 - hand` floored at 0, 0x47 six): the player draws from `deck_pos` and stops at the
  deck's end (40), flagging the shortfall in `target_rows`; the CPU takes cards from the front of its deck while their
  stage equals `round * 2 + 2`, otherwise from the back (index 39 down), at most `deck_count`.
- **Bonus cards** (`cardgame_bonus_cards`; attack and hp both, plus the slot's own bonus, clamped to 0..99): 68 = side's
  slots × 20; 111 = side's hand × 10 + 10; 154 = side's discard × 20 + 10; 197 = all slots × 10; 240 = both discard
  piles × 10 + 10.
- **Combinations** (`cardgame_game_find_combo`): the side's slots sorted by card ID; a run of 3 or more equal IDs whose
  record s16 at +10 is non-zero is a combination (a pair is nothing); the search returns the position after the run, so
  a second run is found by the next call. The field at +10 is the combination's result: the card ID + 1 of the Digimon the
  combo shows (`cardgame_combo_start` looks it up in `card_ids`, 87 and 315 when absent) [C]; the combo adds the run's
  attack and hp (each sum capped at 99), +20 each for 4 or more (capped at 99) [C, the board animates it].
- **Round winner** (`cardgame_game_end_round`): the side with the higher total hp wins the round; **a tie goes to the
  CPU**. When either row is empty the attacks are skipped; if the winner's row is the empty one, the loser's slots are
  still discarded (0x17/0x18). Swaps queued by 0x2A run one per step before the result. Two round wins end the match [C].
- **Shuffle** (`cardgame_shuffle_deck`): only the player's deck (`players[0]`, whatever side is meant), 40 swaps of
  `deck[next() % count + start]` and `deck[next() % count + start]`, so 80 draws; `count < 2` draws nothing. The CPU's
  deck is never shuffled: it is ordered by stage (`cardgame_game_reset_cpu_deck` moves the cards whose stage is due to
  `cpu_deck_last`; `cardgame_game_set_cpu_deck_limits`, `cardgame_restore_cpu_deck`). Dealing (`cardgame_deal_cards`, effect 0x14)
  takes 6 cards from `deck_pos` of each deck into `hand[0..5]` (from 0 whatever the hand held), `hand_count += 6`, `deck_count -= 6`,
  with no check of the deck's end (past 40 it reads `hand[]`, which it has just written) [T] (`cardgame_rules`).
- **Sorting** (`cardgame_sort_cards`, a FAKE match): a swapping selection sort by card ID, not stable (equal IDs can swap);
  flags 1/2 swap `cpu_deck_info`/`selectable` along. The CPU's hand is sorted by its play kind, then its order key.
- **Placement** (`cardgame_game_place_cards`): at most `selected` marked hand cards become slots (attack and hp from the
  record, slot ids from `next_slot_id`), but every marked card leaves the hand: a marked card past the limit is lost.
- **The CPU's play** (`cardgame_cpu_choose_card`): phase 5 tries its kind-1 cards, phase 7 its kind-3 cards only when
  `swap_count` is even and its score is not above the player's (equal proceeds); each candidate must be playable, its
  use effect usable (`cardgame_cpu_can_use_effect`: the masks of `cardgame_mark_selectable_cards/_slots`, 0x83 any reply
  turn, 0x84 a reply to a colour-6 card, 0x70 and 0x73-0x7E always) and have a target. In a reply turn, if the player's
  card ID is in the opponent's counter list, every card from the first kind-4 one on is tried, **whatever its kind**;
  otherwise a record-kind 3 card is answered with a flagged kind-3/4 card of record kind 9, and 9 with 3.
- **Counter targets** (`cardgame_cpu_choose_counter_target`) take the damage and heal from `get_card_damage/_heal` of
  the card **indexes**, not IDs: the CPU's cards (40..79) can only do 30 (index 46) or 10 (56) and heal 30 (40) or 10
  (58). The CPU's "heal my slot" answer (record kind 9 played on its slot, target kind 0) reads `selectable[slot]`, the
  player's row, while use effect 0x85 marks only the CPU's row: through `choose_card` that answer never happens.
- **Targets:** `keep_best_target` keeps the slot whose removal leaves the lowest score, ties to the later slot;
  `filter_targets` drops slots with hp > max and, when none is left, re-flags the last one (with nothing flagged at all,
  slot 0); `set_cheapest_target` the flagged hand card with the lowest record value (+8). Inside effects the CPU takes the
  other's lowest-value hand card (0xA9, ties to the first), its own highest (0xAA, ties to the later), the first selectable card of
  its own deck before `cpu_deck_end`, else the last selectable one (0xAC) and the player's lowest-value deck card (0xAD).
- **The CPU's slot pick** (effect 0xAE, `cardgame_mark_best_cpu_slot`): for its slots 0..5 the score of its side without that slot;
  the strictly highest wins (first of a tie), `selectable` is not looked at, so it picks its least valuable slot; with fewer than six
  slots, leaving out a missing slot costs nothing and the pick is slot `count` (an empty slot: the effect acts on nothing); with no slot
  nothing is marked [T] (`cardgame_cpu_choice`).
- **The CPU's placement choice** (effect 0x9D, `cardgame_cpu_select_cards`, phase 6): its hand in order, each playable card (a Digimon
  whose colour's points cover its level, against the points left after the cards already chosen) not of play kind 5 is chosen and its
  level paid, until 6 are selected [T] (`cardgame_cpu_choice`).
- `cardgame_cpu_get_score` of an empty side is 0; the sort is skipped and the uninitialised `list[0]` indexes the slots once
  (the value is discarded). The CPU calls it so whenever a side placed nothing; a port may guard that read.

Code map: `src/cardgame/cardgame_cpu.c` (`cardgame_cpu_get_score` 145, `_choose_card` 594, `_get_card_damage` 399, `_get_card_heal` 420); `include/cardgame.h`
(`CardgameGame`, `CardgameOpponent`); `card.c` (`card_select`, `card_get_class`); FORMATS "Card game".

What a test should check: score fixtures on a 4-slot board: attack/hp (5,5), (4,6), (3,3), (2,2) no mask -> 30 (no combos); masking slot 0 removes 10; three equal combinable cards (3,3) each:
rest 18 + sum 18 = 36 (+ the triple counted again); four equal: `4*6 + 24 + 20 + 20`. Cases: empty slots (`count = 0` reads `list[0]`, an edge to document); `mask` all ones;
`get_card_damage/heal` tables verbatim; choose_card golden: a board/hand/seed table -> marked index (the function has no RNG, so it is a pure function of the `CardgameGame`).
Whole-game RNG: the dealing/shuffling in `cardgame_game_*` uses `pad_random`; record/replay is the right layer for that.

## 11. Gamestate flags, conditions and progress

Facts [C] (`src/main/gamestate.c`, `include/gamestate.h`); the reading side is [T] by `gamestate_flags` (every flag type x 11
indexes x value 0/1, the flag lists, every condition id but money and object), money conditions by `gamestate_records`, the
writing side by `gamestate_actions` (marked below):
- A flag word is u16: `type = (flag >> 8) & ~1`, `index = flag & 0x1FF`. `gamestate_get_flag(flag, value)` returns true if the bit equals `value` (`gamestate_test_bit(bits, index, set)`:
  set -> bit is 1, clear -> bit is 0). Bit arrays: type 0x00 map flags (`gamestate_flags.map_flags[3]`, cleared on entering a new map), 0x02 `flags[0x12]`, 0x04..0x40 the `flags_NN` arrays
  (sizes in the header: 2, 1, 1, 4, 8, 12, 4, 2, 9, 11, 30, 26 bytes). **Indexes past an array's size are not bounds-checked** (they read the next array or beyond, a
  documented edge, not a feature). The game itself never goes past one (census below).
- Computed flag types (get): 0x60 progress equality (`gamestate_check_progress(index, value)`: `progress == index` if value else `!=`), 0x70 condition table (`gamestate_check_condition`),
  0x72 party Charisma total vs `gamestate_party_stat_levels = {60,150,210,285,378,492,630,795,990,1218,1482,1785,2049,2277,2472}`, 0x7E route/room
  (`index < 30`: `route == index + 1`; else `room == index - 29`), 0x80-0x8E item owned (`items[id] != 0 || items_equipped[id] != 0`), 0x92 card owned (`cards[id] != 0`). **Any other type returns 1** (true).
- Setting (`gamestate_set_flag`): bit types write the bit (any non-zero value sets, 0 clears; no bound check: `0x0608` sets `flags_08` bit 0); 0x70 runs the condition with
  value 1 whatever the value passed (side effects: joins a Digimon, heals the party); 0x80-0x8E `gamestate_change_item(index, value)` (all eight types are the same item
  index); 0x92 `gamestate_change_card`; 0x60 (progress), 0x7E (route), 0x72 and unknown types write nothing [T] (`gamestate_actions`); 0x74, 0x76, 0x78, 0x90, 0x94, 0x7A, 0x7C
  start events or card games (calls into FIELDSTG, not pure; layer 2: `first_battle_save` sets 0x7A02 and 0x7A31 (STITSHOP shop 2,
  STCRDSHP shop 0x31), 0x7C00 (STGDGLAB) and 0x9400 (STGTRAIN) [T]). The odd type bit is index bit 8 (`0x0113`: type 0x00, index 0x113, past `map_flags`); the game never uses it.
- `gamestate_update_map_flags` (on entering a field map): `map_is_new` clears `map_flags` (the 24 type-0 flags); back from a card game (`prev_map == 0x700`) sets flags 0x11 and
  0x12, sets flag 0x10 to `card_game_won` and clears `card_game_won` [T] (`gamestate_actions`).
- `gamestate_check_flags(list)`: list of `(flag, value)` u16 pairs ending in 0xFFFF; true when every pair matches (empty list true). `gamestate_set_flags` same shape.
- Where flag words come from (census [C], `tools/flag_census.py`; DECISIONS "Reference tests: three layers"): the field's talk lists (`FieldstgTalk.flags_required`, `flags_set`), placed actors'
  `flags_required`, `FieldstgMapEvent.flag`/`flag_2`, `fieldstg_flag_events` and constants in FIELDSTG/STSTATUS/WSTAG code; nothing else reads them (no event or card script, text or
  battle data). 16,169 uses (12,658 get, 3,511 set) of 31 types. Computed: `set_flag(get_map() + 0x1E00, 1)` on entering a field map sets the visited-map bit (type 0x20, index
  `map - 0x200`, maps 0x200..0x2EE), which STSTATUS's map reads back as `(i & 0xFF) | 0x2000`. Every bit-array index used is inside its array [T] (layer 3, `flag_census.py --check`; the tightest: type 0x04 index 15 of 16 bits, 0x06 7 of 8), and **type 0x72 is used only with
  indexes 0..14** (1,498 reads, 45 WSTAG files; 14 = the 2472 threshold, `wstag725.c` only), so `gamestate_party_stat_levels`' overrun (FINDINGS 1) is never reached by the game.
- Condition table `gamestate_conditions[]` ({id, type, arg}, terminated by id 0xFF): `type & 0xF0` selects the family, `type & 0xF` the sub-case:
  0x00 item sets, 0x10 Digimon (party set, joined, in party, join (side effect `joined = arg + 3`), level >= 45, party level sum >= `arg * 15 + 30`, not joined, all joined Digimon >= 45,
  full heal (side effect), all eight joined), 0x20 money (check `>= gamestate_money_check[arg]` / add `gamestate_money_add` / subtract `gamestate_money_sub`),
  0x30 progress ranges `gamestate_progress_ranges[arg] = [min, max]` inclusive (e.g. `{2, 18}`, `{4, 99}`), 0x40 bit combinations, 0x50 an object step (condition 0x13, the only one: `gamestate_cond_object` ignores its arguments, `heap_objects.find(0x16, -1, -1)` takes the
  first object of kind 0x16, FIELDSTG's actor effect, in slot order, keys and state not looked at, calls `set_step(obj, 3)` (step 3, substep and timer 0) and returns 1 [T]
  (`gamestate_actions`); with no such object the original calls through a NULL object, a crash). An unknown condition ID returns 0 and
  `check_condition(id, value)` returns `value == ret`.
- Progress: `gamestate_data.progress` (s32, set by the stage list and by events); money 0..9,999,999.
- Maps and modes [C]: `map >> 8` picks the tier-1 overlay (`overlay_files`). The debug overlays are unreachable without a
  memory poke: STAGSLCT (0x15xx) is entered only from itself, SOUNDTST, SHOCKTST and a CARDGAME match it started; no map exit,
  event script or constant map change targets 0x15xx, 0x8xx (SOUNDTST) or 0x9xx (SHOCKTST), and nothing at all, not even
  STAGSLCT's stage list, goes to 0x8xx/0x9xx; WFIGHTTS needs map 0x600 with a non-zero entry, which only STAGSLCT sets
  (tests/README.md, layer 2, "The overlay tour").
- Playtime `gamestate_tick_playtime`: 24.8 fixed frames (+0x100 at 60 Hz, +0x133 at 50 Hz), hours <= 999.

Code map: functions above, `gamestate_data` (`GamestateData`), `gamestate_flags` (`GamestateFlags`), tables `gamestate_conditions`, `gamestate_progress_ranges`, `gamestate_money_*`,
`gamestate_party_stat_levels`, `gamestate_item_sets`.

What a test should check:
- Bit math: flag `0x0213`: type 0x02, index 0x13 -> byte 2, mask 8; `set_flag(0x0213, 1)` then `get_flag(0x0213, 1) == 1`, `get_flag(0x0213, 0) == 0`; neighbours untouched; flag 0x0113 maps to
  type 0x00 (the odd type bit is masked).
- `check_flags` on `{0x0201, 1, 0x0202, 0, 0xFFFF}` with bit1 set and bit2 clear -> 1; flip either -> 0; list `{0xFFFF}` -> 1.
- Progress ranges: progress 2 -> condition 0x06 (`{2,18}`) true, 1 and 19 false; 99 and 100 for `{4,99}`.
- `check_progress(7, 1)` iff `progress == 7`; `(7, 0)` iff `!= 7`. Unknown flag type 0x30 or 0x66 returns 1.
- Money: `change_money(1, i)` with money 9,999,000 and add 3000 -> 9,999,999; `change_money(2, i)` with money 100 and sub 800 -> 0; check (type 0, arg 0) true at 800, false at 799.
- Item cond 0 true only when items 7, 89 and 167 are each owned or equipped; party level sum rule at `arg = 0`: >= 30, `arg = 3`: >= 75.
- Route flag 0x7E: `route = 5`: index 4 true, index 29 uses room.

## 12. Save data and checksum

Facts [C] (`src/main/memcard.c` 451-470, `docs/FORMATS.md` "Save data"); the two checksum functions [T] (`memcard_checksum`: 9 byte
patterns x the right sum, sum ^ 1, 0, 0xFF):
- `memcard_get_checksum(data, size)` = XOR of the bytes (u8). `memcard_check_checksum(data, size, sum)` = `XOR == sum` (exact). **STGMCARD does not call the exact check**: loads use
  `checksum & ~stored == 0` (lenient: a stored byte with extra bits set passes).
- Slot = first 0x26C4 bytes of `gamestate_data` (byte 0 = checksum of bytes 4..0x26C3, byte 2 = version 4, name at 0x54, money 0x6C, Digimon records from 0x75C, progress 0x263C, flags 0x2644).
  Header (0xD4 bytes at file 0x200): checksum of bytes 4..0xD3, `"DMW3"` at 4, three slot summaries. File `BESLES-03936DMW3-EUR`, 4 blocks (0x8000), slot offsets `0x300`, `0x2A00`, `0x5100`.
- Load rejects a slot whose version byte is not 4.
- [T] Round trip in the game (`tests/replay/scripts/first_battle_save.json`, checkpoints `saved` and `loaded`): a save at
  the Asuka Inn (STGMCARD map 0xC01) then a reboot and Continue (map 0xC00) gives back bytes 4..0x26C3 of `gamestate_data`
  exactly, except the playtime, which goes on counting; the load writes the slot's checksum and version into bytes 0 and 2
  (0 before), and the bytes from 0x26C4 on (map, next/prev map, entry, `map_is_new`, `field_last_map`, depth, meter count)
  are not saved. The load returns to the saved field map (`field_map`, 0x20A), not to 0xC01. A new file on an empty card
  is created after a "Create new data?" prompt; after "Saved." STGMCARD waits for CROSS, and Back (TRIANGLE) leaves it.
- [C] A loaded map is a fresh entry (`fieldstg_update_main`: `field_last_map` differs, so `map_is_new` = 1): the type-0 map
  flags (`gamestate_flags.map_flags`) are cleared, `player_depth` is 4, `attr_layer` is the stage's initial layer, `player_height`
  0x3000, `meter_random_count` 16, and the spot target is re-drawn; a return from STGMCARD to the same map keeps them all. The
  port's save-anywhere mod carries them in the slot's unused tail (file bytes 0x26C4..0x26EB of the slot, outside the checksum;
  `docs/LAUNCHER.md` "Save anywhere"). [T] by `tests/port/mods.py` and the emulator's load of such a card in `tests/saves/run.py`.

Code map: `memcard_get_checksum`, `memcard_check_checksum`, `memcard_funcs`, `memcard_read/_write`, STGMCARD (`src/stgmcard/stgmcard_80082CD0.c` `StgmcardSaveData`, `StgmcardSaveHeader`),
`GamestateData` first 0x26C4 bytes.

What a test should check: checksum of all-zero data = 0; `{0x01}` -> 1; `{0xAA, 0xAA}` -> 0; size 0 -> 0; XOR of a repeated byte depends on parity; order independence; single bit flip changes the sum.
`check_checksum(data, n, sum)` exact. Lenient rule: stored `0xFF` accepts any computed sum; stored 0 accepts only 0. Round trip: pack `gamestate_data` (0x26C4 bytes) -> write byte 0 =
`get_checksum(buf + 4, 0x26C0)` -> `.mcd` -> reload -> byte equality and `gamestate_data` hash equality; version byte != 4 rejected; a corrupted flag bit changes the checksum. Cross-check
with emulator-written cards and with the port.

## 13. Open questions

Answered and removed (2026-10-05): the goldens' validity while `fightstg_rules_get_stats` was NON_MATCHING (they run the
original's code; the WIP C reproduces every `fightstg_rules` case on the host and the battle replay, `tests/holdouts/`);
`values[1]` (the TP, section 6); division by zero (no trap, and no zero divisor in the game's data: section 4); where the
digivolution and training rules are (sections 6, 7); the card rules (section 10); the menu item effects and the shops' helpers
(section 9); the exact fields of the status rolls (section 5); which training menus and shops the field opens (sections 6 and 9,
FINDINGS 3 and 4: never menu 14 nor shop 31); the special enemies' prize item and the battle-end drop (section 5, `wfightmn_spoils`, sweep5).

- DNA digivolution: no code found under that idea in the units read.
- Card game: the animated effects' exact results (points, attacks, stat changes, the combo's sums) are read from the C only;
  they need the board display and belong to a layer-2 card-game script, which needs party Charisma >= 60 (section 10).
- Battle item effects, buying/selling counts and the booster draw (section 9) and the encounter step (section 1; its redraw is
  `[T]`) are `[C]`: UI or field objects, and any outcome drawn from `pad_random` after the first battle is core-dependent in layer 2.
- Field and card-game flow left `[C]` by sweep5 (each a first-match walk or a one-line rule inside an object's update, over
  functions that are `[T]`): the talk chosen (`placed->talks`, the first with no `flags_required` or `check_flags == 1`), the
  placed actors shown and the player's look (ids 1/0x6A/0x146/0x147 by flags, else 2 with the party followers from progress 3),
  map events (`fieldstg_map_events_enter`: two `get_flag` gates, then the type's action; transitions are in layer 2's map
  sequence), stage 528's story event (`fieldstg_flag_events`: progress and two flags), the dig spots (`fieldstg_spots_find`'s
  inclusive +-10 box, `fieldstg_spots_pick_target`'s `next() % count`, the found target's battle 3 at `(next() & 0x7F) < 0x66`,
  else 6), the card match's result (`items[prize]++` below 99 and `card_game_won`, read by `gamestate_update_map_flags` [T]),
  the opponent's load (`cardgame_game_load_opponent`: kinds 6/4/2/1/3/5/7 sort at 100..700 + the deck index, the counter IDs - 1 followed
  by a fixed default list of 15 bytes ending in 0xFF; the data's longest list is 11, so the 27-byte array is never overrun).
- Playtime at 50 Hz (`+0x133` per vsync) is read from the vsync handler, not tested (`gamestate_tick_playtime`'s carries are).
- Guide-versus-C differences: none known (no guide was reachable).

## 14. Sources wanted (blocked this run) and sources used

Used: none outside the repo. Blocked by the proxy (EGRESS_BLOCKED): `gamefaqs.gamespot.com`, `strategywiki.org`. Not tried (budget rule: stop after two blocked hosts).
URLs to fetch from an unblocked machine, in priority order:
1. https://gamefaqs.gamespot.com/ps/562335-digimon-world-3/faqs (index; pick the longest battle/stats guide; append `?print=1`)
2. https://gamefaqs.gamespot.com/ps/562335-digimon-world-3/faqs/ (the "Digimon World 3 Battle Mechanics / Damage Formula" and "Stat Growth" FAQs listed there)
3. GameFAQs "Digimon World 3 Card Game Guide" and "Digivolution Guide" from the same index
4. https://strategywiki.org/wiki/Digimon_World_3 (and its Battle, Stats, Digivolution subpages if present)
5. https://digimon.fandom.com/wiki/Digimon_World_3 and its "Battle", "Card Battle" pages
6. https://www.neoseeker.com/digimon-world-3/ (FAQs tab) and an IGN "Digimon World 3" guide
What to extract: the damage formula and the order of modifiers (compare with section 2), critical and hit chances (section 3), level-up/EXP table (section 6, the formula
`(lv^3 + 5lv - 6) * curve / 10 + bonus`), element wheel (section 4), digivolution conditions (section 7), card game rules (section 10), item effect values (section 9).
