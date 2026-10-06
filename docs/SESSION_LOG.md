# Session log

Newest first. One short entry per session: goal, result, next steps.

## 2026-10-06: Session 16: PC port M1, then both layer-2 scripts in the port (cloud branch `claude/peaceful-allen-q3t9f9`, PR to `main`)
- **Asked:** M1 proper with dedicated sub-agents owning disjoint files: LIBCD over the BIN, the scripted pad, the per-frame
  log, the headless runner and its test against `tests/replay/expected/new_game.json`, the findings on the boot path;
  delivered as a pull request (the user later: merge it after CI is green, keep going).
- **Phase 1:** base verified (`build.sh --check` 55 s, `test.sh` green, the skeleton builds and runs). This machine: 4 cores,
  15 GB: 3 agents at once, `DW3_JOBS=1`. ASan/UBSan present; `gcc-multilib` installed with the user's approval.
- **Step 0 (orchestrator):** `port/include/port_harness.h` (the tasks' interfaces), the per-vsync frame hook
  (`psyq_set_vsync_hook`; a frame is a vsync from any source), `--disc/--cd-speed/--script/--log/--record`, our own SHA-1.
- **Agents (each merged with `merge_checkpoint.sh`, byte-identical every time):**
  - T1 cd: `port/src/disc.c` (CUE/BIN, whole-BIN SHA-1 with a stamp cache), `libcd.c` (realistic/instant timing in vsync
    ticks, movie streaming into the game's ring: the opening movie plays at exactly 15 fps).
  - T2 log: `framelog.c` (per-frame log, record), `state.c` (probes, `port_state_read`, `gamestate_data`'s PS1-image hash,
    equal to the emulator's five dumps), `port_gen.py state`, `-DDW3_PORT_M32`; its `-m32`/`-m64` log comparison found the
    truncated overlay root object.
  - T4 findings: FINDINGS 7 (R3000A `% 0`), 8 (36 creators return their object, `OBJECT_V0`), 9a-c (`object_destroy` stops
    exactly the live objects), new 9d.
  - T5 sizes: FINDINGS 9d: 122 object data sizes in `sizeof` units, `port_inventory.py object-sizes`, overlay entries
    return `Object *`, the 8-aligned host heap, FIELDSTG's NULL map events and `FieldstgBackgroundView`.
  - T3 runner: `script.c` (run.lua's engine in C; `walk` too, not `reset`), `json.c`, `tests/port/run.py`, the `port` layer
    of `test.sh`, CI (`gcc-multilib`, `run.py --m32`), `setup.sh cmake` (venv cmake/ninja when missing). Integration: the
    overlay copy's CPU time (the checkpoint `new_game_field` sees the PS1 mid-copy), the primitive hash ignores texture
    padding, the `-m32` heap assert.
  - Orchestrator: `DW3_PORT_TRACE` fixed, `OFFSETOF` for `fieldstg_find_stage`'s clear, FINDINGS 10, docs.
- **Result:** `tests/port/run.py --m32 --sanitize`: all 5 checkpoints (stage, map, stable hash), both sequences, two runs
  identical, `-m32` == `-m64`, no sanitizer report; `scripts/test.sh`: probe + 5 layers passed.
- **Merged as PR #2** (CI green; the user had asked to merge after completion and go on).
- **Second part (the user: keep going), branch restarted from `main`:**
  - The pad a frame late as in the emulator (found from the emulator's per-frame trajectory of a `walk`): `first_battle_save`
    then matched through the first battle.
  - T6 memcard (`.mcd` images, LIBMCRD, `state.c`'s field table), T7 reset (every game section from a startup snapshot,
    the link map checked), T8 GTE (`port/psyq/gte.c`, golden family `gte`, 865 cases on the PS1).
  - Orchestrator: STGMCARD's view split (T6's patch), the full file cache freed on the host, `object_destroy` at `-m32`,
    `fightstg_model_new`'s parts zeroed, `tests/port/run.py` over every script, `ubsan.supp`, FINDINGS 5 additions,
    `DW3_PORT_PRIM_DUMP`.
  - Result: `tests/port/run.py --m32 --sanitize` passes both scripts (all 25 checkpoints); `scripts/test.sh` green.
    Found and documented: FIELDSTG's `free_above` makes the file cache, hence frames, depend on the heap layout.
- **Next:** M2 (the software GPU against the emulator's VRAM, SDL3; LIBGS's GTE set-up checked like the GTE).

## 2026-10-05: Session 15: PC port decisions, M0 and the M1 skeleton (worktree branch `gascarcella/PC-Port-Kickoff`, PR to `main`)
- **Asked:** decide `docs/PC_PORT_PLAN.md` section 4, then M0 (and, the user's choice in phase 1, an M1 skeleton) with
  dedicated sub-agents owning disjoint files, delivered as a pull request.
- **Decisions:** every recommendation taken (DECISIONS "PC port decisions (session 15)"); the `-m32` oracle wanted for M1
  (the user installs `glibc-devel.i686`).
- **Done, in waves of worktree agents** (32 cores: 5 at once with `DW3_JOBS=6`; each merged with `merge_checkpoint.sh`,
  byte-identical every time):
  - Wave 0: T0 `include/port.h` (the hook macros, both sides, proven at every site) + the `MoveImage` prototype;
    T6 `tools/port_inventory.py`. Baseline under GCC 16: 31 of 388 units failed the probe (93 errors, 60 of them
    `incompatible-pointer-types`, warnings under the plan's GCC 13).
  - Wave 1, by file owner (EXE, FIELDSTG, battle, WSTAG, the 17 small overlays): every transform of the plan's M0; the
    probe clean on the whole tree. Extras the plan missed: 67 `SLOT_FUNC` script objects, 17 more literal sizes, the
    `D_8009B6A4` data pointer, two CD-ended loops without a wait hook (`cdload_load_file`, `sound_init`).
  - Wave 2: T7 `tests/host/build.sh` without `-fpermissive`/the prototype headers; the probe gates `scripts/test.sh` and
    a disc-free CI step; FINDINGS 9. T7 also found the host replay broken by the `opponents` retype (fixed in `replay.py`).
  - Wave 3: T9 `port/psyq/` (113 stubs, real where pure) and T8 `port/` (CMake, arena, overlay manager with generated
    ld script and tables, pump, `tools/port_gen.py`). The real link has no unresolved symbol; `build/port/dw2003
    --max-frames 60` exits 0 at the cap.
  - Orchestrator: `setaddr` through `PTR_TO_U32`, `object.c`'s `child_count` in pointer units on the host, the two
    `PLATFORM_WAIT`s, the `D_8009B6A4` binding.
- **Left as is:** FIELDSTG's zero block (asm; psylink fill), 23 literal byte counts (annotated), the open items in
  STATUS "Not done". The sanitizer link needs `libasan`/`libubsan` (sudo).
- **Next:** M1 proper (STATUS "Next" 2).

## 2026-10-05: Session 14: the two-repository cloud session proven (cloud, branch `claude/optimistic-heisenberg-3b2i6d`)
- **Asked:** check that building and testing still work after the split into `dw2003recomp` (public, code) and
  `dw2003-gamedata` (private, data), from a cloud environment that selects both.
- **Done (docs only, no code change):**
  - Both repositories were cloned side by side under `/home/user/`; `scripts/gamedata_dir.sh` resolves the sibling. The
    data checkout's `SHA1SUMS` (disc files, BIOS) and `SHA256SUMS` (the emulator zip) verify.
  - **The SessionStart hook did not fire:** with two repositories the session's project directory is `/home/user`, the
    parent of both checkouts, so the repo's `.claude/settings.json` is not loaded (no tools, no `iso/` at the start).
    Run by hand with `CLAUDE_PROJECT_DIR=$PWD`, it completed (exit 0, ~3 min): the toolchain (one binutils mirror 403,
    the fallback worked), the pinned emulator with the glibc sysroot, the disc decompressed from the five xz parts and
    SHA-1 verified, 2,386 files extracted. Recorded in CLAUDE.md step 0, STATUS, OPEN_SOURCE_PLAN step 0.
  - `scripts/build.sh --check`: 2,100 outputs byte-identical (45 s). `scripts/check_toolchain.sh`: OK.
    `scripts/check_emulator.sh --bios retail`: CNTY_SEL at frame 1011. `scripts/test.sh`: 4 layers passed, 0 skipped
    (110 s; the host replay's known mismatches as listed in `tests/host/FINDINGS.md`).
  - CI on the public `main` is green (run 5, with the data steps).
- **Next:** the user closes pull requests by hand; then `docs/PC_PORT_PLAN.md` section 4 (the user decides first).

## 2026-10-05: Session 13: the public repository's GitHub steps (local, on `main`)
- **Asked:** prove the local setup with the data checkout beside the repo, then finish what the cloud session could not:
  the milestone tag and release, CI's data access and first green run, the repository settings.
- **Done:**
  - Local setup proof (GCC 16.2 host): `scripts/setup.sh` (1.5 min cold, the disc and the emulator from
    `../dw2003-gamedata`), `worktree_init.sh`, `build.sh --check` (2,100 outputs byte-identical) and, after the fix
    below, `scripts/test.sh` (4 layers passed, 0 skipped).
  - **Fix: the host replay on GCC 14+.** GCC 14 and later make implicit declarations and incompatible pointer assignments
    errors in C (`-w` does not hide them): `cardgame_80096950.c` assigns `cardgame_board_open_dialog` (`s8`/`s16`
    parameters) and `cardgame_board_set_dialog_answer` (`s16`) to `s32`-typed method slots, and `fieldstg_80087DB0.c`
    calls `MoveImage` undeclared. `tests/host/build.sh` adds `-fpermissive` (older GCC ignores it for C). The port will
    need those prototypes fixed instead.
  - Tag `v0.1-matching-closed` (annotated) on `c1ca6f2`, pushed; GitHub pre-release "v0.1 — matching milestone", no assets.
  - CI: an ed25519 read-only deploy key on `dw2003-gamedata`, its private half as the Actions secret `GAMEDATA_DEPLOY_KEY`
    (key files shredded). The two runs from the cloud pushes had failed: the snapshot tracked `tools/__pycache__`, a
    symlink to the cloud machine's checkout made by `setup.sh`'s worktree link step (`__pycache__/` matches only
    directories), which dangles on a clone and broke `py_compile`. Untracked; `.gitignore` now says `__pycache__`. Next run
    green in ~5 min with every step, the data ones included. Host-replay difference noticed: finding 1's six
    `gamestate_flags` cases agree by accident with GCC 16's layout and mismatch (as listed) with CI's GCC 13.
  - Settings: description, topics (decompilation, playstation, psx, digimon, reverse-engineering), Issues on, Wiki,
    Projects and Discussions off (`gh repo edit`).
- **Next (user):** a Claude Code cloud environment selecting `dw2003recomp` and `dw2003-gamedata` (OPEN_SOURCE_PLAN
  section 5 step 0); pull requests closed by hand. Then `docs/PC_PORT_PLAN.md` section 4 (the user decides first).

## 2026-10-05: Session 12: going-public proposal (cloud session, branch `claude/gallant-feynman-3mcny7`)
- **Asked:** the best repository setup for an open-source release (private repo A, public monorepo B or a separate port
  repo C), how the private material is handled, and the open questions before moving on to the port.
- **Done:** `docs/OPEN_SOURCE_PLAN.md`: an inventory of the tree (924 public-safe files, 22 MB; `gamedata/` 467 MB and
  `tools/prebuilt/` 86 MB in history since 2026-10-02/04; what refers to them; the two personal sentences in DECISIONS;
  no LICENSE), the layouts compared (recommended: the public repo as the working repo plus a private data repo, the port
  as `port/` in it, unannounced until it boots), the private items one by one, 11 decisions with recommendations, and the
  migration checklist. **Decided by the user** in the same session (DECISIONS "Going public"): public working repo +
  this repo trimmed to the data, `port/` in it, MIT, data-as-C accepted, a clean one-commit snapshot into a new repo
  named `dw2003recomp` after renaming this one, publish now, docs as they are minus two sentences, CI through a
  deploy-key secret, public read-only, risk accepted.
- **Step 1 done (this branch):** `scripts/gamedata_dir.sh` (the optional data checkout: `$DW3_GAMEDATA`, `tools/local.env`,
  this checkout's `gamedata/`, or `../dw2003-gamedata`) used by `setup.sh gamedata`/`redux` (with a pinned distrib.app
  download as the public fallback), `worktree_init.sh`, `check_emulator.sh --bios retail`, `oracle.py`, `replay.py`;
  LICENSE (MIT), the README notice and data-checkout section, CLAUDE.md's rule, `.gitignore`, the two DECISIONS sentences,
  `tests/README.md` (the port is `port/`), `scripts/publish_snapshot.sh` (one commit + proofs), `.github/workflows/ci.yml`
  (unrun). Verified: `check_emulator.sh --bios retail` through the lookup; the snapshot (929 files, 19.8 MB, clean) built
  beside a simulated data checkout (this repo's `gamedata/` and `tools/prebuilt/` linked under `../dw2003-gamedata`): a cold `scripts/setup.sh` (binutils built from source, the disc and the emulator taken from the data checkout), `worktree_init.sh`, `build.sh --check` (all 2,100 outputs match their SHA-1) and `scripts/test.sh` (4 layers passed, 0 skipped) in 5 min 50 s.
- **Published (same session):** the user renamed the private repository to `dw2003-gamedata` and created the empty public
  `dw2003recomp`; the two data-contents DECISIONS entries were generalised (user decision); `scripts/publish_snapshot.sh`
  took the snapshot of `main` at `e446f0e` and it was pushed as this repository's initial commit `c1ca6f2`. The tag could
  not be pushed from the cloud session (the proxy refuses tag pushes and ref writes: HTTP 403), so the user creates it.
  The private repository was trimmed at its tip to `gamedata/` + `tools/prebuilt/` + a README (history kept). From here
  on, this public repository is the working repository and its commits carry no session links (user decision).
- **Next (user):** the tag, the deploy-key secret for CI, a cloud environment with both repositories; then the port plan's
  section 4 and M0 in `port/`.

## 2026-10-05: Session 11 (branch claude/vigilant-lovelace-oed6n1): the reference tests to "good shape"
- **Goal:** a reference suite that pins most of what the game does for the port (the user's unattended brief: coverage
  tool, replay scripts, holdout validation, layer-1 families, finding 1 from data, layer 3, docs, a sweep that finds
  nothing worth adding). Orchestrated: every item went to a worktree agent, merged with a full `scripts/test.sh`.
- **Result:**
  - Layer 1: 8 → 21 families, 2,619 → 3,437 cases, 4,458 → 5,333 calls, one boot ~30 s, all replayed on the host.
    New: `stfgtrep_forms` 62, `stgtrain_rules` 108, `cardgame_cpu_choice` 155, `cardgame_rules` 193 (225 calls),
    `ststatus_items` 67 (75), `shop_rules` 49 (66), `gamestate_actions` 50, `sprite_draw` 4, `stgdglab_party` 7,
    `fieldstg_encounter` 5, `wfightmn_events` 34, `wfightmn_enemy_ai` 21, `wfightmn_spoils` 24; `fightstg_rules` +39.
    Two oracle bugs fixed (icache flush after a file write; file writes fill whole sectors).
  - Layer 2: unthrottled runner (new_game 62 s → 5 s), state-driven steps that hold on both CPU cores, `first_battle_save`
    (46,597 frames, 20 checkpoints, ~80 s: the first battle, save + reset + load, a shop purchase and refusal, STGDGLAB,
    STSTATUS, STCRDABM, STCRDDEK, STCRDSHP, STGTRAIN); 14 of 19 tier-1 overlays reached, the rest shown unreachable by pad
    or gated. A Lua stack leak in the pinned emulator (~32,700 frames) is worked around in `run.lua`.
  - Layer 3: four checks (file table, text tables, WSTAG stage table, flag ranges). `tools/flag_census.py` closes finding 1.
  - Coverage (`tools/coverage.py`): game functions run 296 → 1,203 of 3,608 (oracle 166 → 294, its jobs 97 → 231);
    focus modules (union): gamestate 49/50, records 7/7, pad_random 2/2, fightstg_rules 32/32, cardgame_cpu 15/15,
    memcard 13/15, stfgtrep 34/36, card 10/12, stgtrain 37/94, ststatus 34/123, stgdglab 20/71 (the rest UI, justified).
    Holdouts and FAKE matches run: 3 → 9 of 17. `tests/holdouts/run.sh`: 5 holdouts validated by replay, none diverged.
  - Findings 3-8 added (8 in total); 7 matters most: the first battle's end computes `next() % 0` (SIGFPE on x86).
  - `scripts/test.sh` green in ~2m06; `scripts/build.sh --check` byte-identical.
- **Next:** the "For review" points in STATUS (stable-hash additions, splitting `gamestate_flags`' whole-struct read),
  save round trips with the port, a card match replay if a core-independent route exists.

## 2026-10-05: Session 10 (branch claude/charming-ride-j1p627): more reference tests
- **Goal:** find and add new test cases to the reference suite (the user's request), keeping `scripts/test.sh` green.
- **Result:**
  - Three new layer-1 families (361 cases, 1,812 calls; all 8 families run in one 23 s boot and reproduce exactly):
    `gamestate_records` (every stat setter clamp and the `add_stat` s16 wrap, `add_stat_bonus` for every type,
    `get_stats` with weapon/armour/accessory/two-handed/key/usable items, penalties and the item sets, money and the
    money conditions the flags family skipped, items including the equipped-unequip branches, cards, all form functions,
    parties, playtime carries, `init_cards`, `records_list_items`), `records` (table hashes, every Digimon id, every
    item's icon, categories), `stfgtrep_exp` (`STFGTREP.PRO` in the slot: experience thresholds 12/36 as MECHANICS
    predicts, levels to 99, `raise_stats` at every band edge x 3 RNG indexes, caps).
  - Oracle: `Read("buf:<name>")` reads a scratch buffer back (host harness `D buf:<name>`); a kept setup case writes only
    the overlay. Lesson: a call that spans a vblank sees the vsync handler add to `gamestate_data.playtime_frames`, so
    the whole-struct hashes skip that word (DECISIONS "Session 10 oracle lessons").
  - Host replay: `gamestate_records` and `records` match the original everywhere; `stfgtrep_exp` differs in 17 cases, one
    finding (`tests/host/FINDINGS.md` 2): `stfgtrep_resist_gains[class + residue]` reaches entry 8 of an 8-entry table
    that ends the overlay file, so the original's gain is 0 (the overlay's `.bss`) and the host's is garbage. Listed in
    `known_mismatches.json`; the suite is green.
  - Layer 3 exists: `tests/formats/run.sh` runs `tools/dump_text.py --check` (91,059 entries). `scripts/test.sh`: 3 layers, ~1.5 min.
- **Next:** STATUS "Next" 0: replay scripts for a battle, a card match and a save; `cardgame_cpu_choose_card`;
  save round trips in layer 3.

## 2026-10-05: Session 9 (branch claude/tests-1), phase 3: battle and card goldens, host-side replay, findings
- **Goal:** golden families for the battle rules and the card CPU (overlay code), the host-side replay of every golden
  (B5), and the findings list.
- **Result:**
  - Oracle: kept setup cases load an overlay into the slot and data files into RAM (registered as loaded cdload
    entries), per-call writes vary one shared fixture, overlay symbol files are read (DECISIONS "Overlay goldens").
  - `fightstg_rules` (1,358 cases) and `cardgame_cpu` (182) in `tests/golden/`; hand checks agree (basic attack 114,
    +50% power_up, Ice field +50%, Multi Crest ×0.4, gauge 125/150, MP cost 48/57/43; card scores 30, 36, 88).
    All five families (2,646 calls) reproduce exactly in one 23 s boot.
  - `tests/host/`: the units compiled with gcc `-m64` (GTE stubbed, externals resolved to 0, shims for heap/files/time),
    a stdin-driven harness and `replay.py`; `scripts/test.sh` runs it in layer 1 (~2 s). Result: 2,640 of 2,646 calls
    match; the 6 that differ are `gamestate_check_party_stat` reading past its 15-entry table (`tests/host/FINDINGS.md`),
    listed in `known_mismatches.json` so the suite is green and fails only on a new mismatch.
  - Lesson: the first host run differed everywhere until the gamestate fixture covered the whole pointer-free struct.
- **Next:** layer 3 (`tests/formats/`), more replay scripts (battle, card match, save), `cardgame_cpu_choose_card`,
  the skipped condition types, a 32-bit host build if `gcc-multilib` is installed.

## 2026-10-05: Session 9 (branch claude/tests-1), phase 2: layer-1 oracle and the first goldens
- **Goal:** decide and build the layer-1 oracle (B3), generate the first goldens (B4: `pad_random`, memcard checksum,
  `gamestate_*`), prove they reproduce, keep `scripts/test.sh` green.
- **Result:**
  - Oracle = the game itself: `tests/golden/oracle.lua` hijacks the main loop at `pad_update` (exec breakpoint, needs
    `-debugger -interpreter`), sets registers, runs the function to a sentinel `ra`, reads back memory, restores. The PS1
    test-EXE alternative was costed (14–41 undefined symbols per unit) and rejected (DECISIONS "Layer-1 oracle").
  - `tests/golden/oracle.py gen|check|list`, families in `tests/golden/families/*.py`, goldens `tests/golden/*.json`
    (1,106 calls in 718 cases), `tests/golden/run.sh` for layer 1. One boot per run, ~20 s; `scripts/test.sh` ~80 s.
  - Proven: `gen` twice gives identical files; `check --bios retail` reproduces the OpenBIOS goldens; every
    `gamestate_data` SHA-1 after a call equals the fixture's except the five `check_condition` ids of type `0x13`
    (which mark a Digimon joined, as the C says), and the restore keeps every case independent.
  - Spot-checked against the C by hand: flag type `0x02` bits of the patterned byte 0 (`0x0B`); the unknown flag types
    (`0x50`, `0xA0`) always return 1; an unknown condition id leaves `ret` at 0, so `value == 0` is true.
- **Next:** phase 3: battle (`fightstg_rules_*`) and card (`cardgame_cpu_*`) families with the overlay loaded into the
  slot (`spec.overlays`), the host-side replay `tests/host/` and its findings list.

## 2026-10-04: Session 9 (branch claude/tests-1), phase 1: test foundation, replay runner, mechanics notes
- **Goal:** start the reference tests (STATUS "Next" 0): layout and runner (B1), the layer-2 replay runner with a first
  script and a determinism proof (B2), plus a background research pass over the game's mechanics (Track A).
- **Result:**
  - `tests/README.md` (the three layers, the never-change-the-build rule), `scripts/test.sh` (runs the available layers).
  - Layer 2: `tests/replay/run.lua` (inside PCSX-Redux) + `tests/replay/replay.py` (driver, hashes, record/check).
    First script `tests/replay/scripts/new_game.json`: boot to the first field map in 3,124 frames (about 60 s wall),
    5 checkpoints; `tests/replay/expected/new_game.json` recorded from two identical runs (DECISIONS "Replay runner").
  - Found on the way: Redux drops GC'd Lua listeners; the opening movie never ends under Redux (START skips it); the
    game rotates the face buttons for Western languages (CROSS confirms); English is CNTY_SEL's default choice.
  - Track A: `docs/MECHANICS.md` from the C only (the FAQ hosts are blocked here): RNG, damage pipeline, hit/critical,
    level-up formula, gamestate flags, checksum and card CPU scoring mapped to functions, tables and test ideas.
    Spend: 2 blocked fetches, ~160k tokens.
- **Next:** phase 2: the layer-1 oracle decision (Lua-driven calls vs a PS1 test EXE) and the first goldens
  (`pad_random`, memcard checksum, `gamestate_check_*`); then battle/card fixtures and the host-side replay (phase 3).

## 2026-10-04: Session 8: project review; PCSX-Redux runs headlessly in cloud sessions; test plan
- **Review (cloud session):** read every doc, rebuilt everything (`build.sh`, 1 min 17 s, byte-identical) and gave
  recommendations (STATUS "Review follow-ups"): licence now, CI while private, public release as a fresh repo, a tag
  for the matching milestone, split the GCC journal out of DECISIONS, run the `NON_MATCHING` build in an emulator
  before the port, check the two implicit-declaration warnings, name WSTAG data by generator rule.
- **Test plan (agreed):** three layers (STATUS "Next" 0): golden tests of pure logic with goldens from the same C
  compiled by GCC 2.8.1 and run on the emulated R3000; record/replay traces hashing `gamestate_data` at checkpoints
  (deterministic: the RNG is a 4,096-entry table); save/format round trips. The emulator was the missing tool.
- **Emulator:** cloud sessions can't download PCSX-Redux (egress policy: `distrib.app` 403 on CONNECT, no prompt), so
  the owner keeps the pinned Linux build (`tools/prebuilt/`) and a PAL BIOS in the data checkout (owner's decision). The AppImage needs glibc 2.43 (the container has 2.39): `setup.sh redux` unpacks a 14 MB sysroot of
  pinned Ubuntu 26.04 packages and runs the binary through its loader. `scripts/check_emulator.sh` boots the disc to
  CNTY_SEL in ~16 s with OpenBIOS and the retail BIOS; the SessionStart hook installs the step (+10 s).
- **Next:** the user's choice among the review follow-ups; then `tests/` (layer 1: the `fightstg_rules` PS1 harness).

## 2026-10-03/04: Session 7: matching phase closed; 98.6% of game code in C, EXE 100%
- **Method:** the same orchestration (worktree agents, `scripts/merge_checkpoint.sh`, `main` fast-forwarded after each
  merge). Each WIP-retry round fed its lessons (DECISIONS "wip-7" … "last-rest lessons") into the next agents' briefs.
- **Result:** all game code 91.9% → **98.6%** (3,598 / 3,606 functions); EXE **100%**; tier-1 88.1% → 98.1% (CARDGAME,
  STGDGLAB, STSTATUS, STGTRAIN and 11 more at 100%); tier-2 87.9% → 95.7% (WFIGHTMN 100%); WSTAG 100%.
- **Key steps:** FightstgBattle's per-battle state became a sub-struct; a LOOP_BLOCK audit classified every use (A/B/C);
  the US decomp (juandav/dw3_decomp, MIT) proved a useful reference once it built EU too (shapes ported in our names,
  user-approved, `docs/THIRD_PARTY.md`); the user reviewed every judgement call and set an endgame rule: forced
  matches only after a last pass, marked `/* FAKE: */` (9 now); the 8 holdouts stay INCLUDE_ASM with notes.
- **Also:** `docs/FORMATS.md` rewritten from the C and verified against every disc file of each kind
  (`tools/disc_files.py`, `tools/dump_text.py`); `tools/cc_psx.sh` drops intermediates for /tmp outputs (the permuter
  filled the disk with 23 GB of them).
- **Naming phase (after the checkpoint):** names-7 … names-11 and decode-1 named ~2,400 fields, ~2,700 functions and
  ~2,900 data symbols (every WSTAG function), the object header (`Object`) and typed the item data, event scripts and
  card scripts: `unk_` uses 15,646 → 1,097, no `Unk<addr>` types left. Concurrent renames were merged with a
  token-level three-way merge (scratch `tokmerge.py`), each merge proven by `build.sh --check`.
- **PC port:** `docs/PC_PORT_PLAN.md` proposes a native C port with our own Psy-Q layer (385/388 C files already
  compile on a modern host); 11 decisions in its section 4 wait for the user.
- **Next:** the user's port decisions, then M0. Holdouts/FAKEs can be revisited any time (list in STATUS).

## 2026-10-02/03: Session 6: cloud session, orchestrated agents; 80% of game code in C
- **Setup:** first cloud session. The owner keeps the game data in a private data checkout (`gamedata/`); `setup.sh
  gamedata`, a binutils mirror fallback and a SessionStart hook make a fresh cloud session build in ~4 min.
- **Method:** the orchestrator ran 4–7 decompilation agents at a time in git worktrees (`docs/AGENT_BRIEF.md`), merged
  each branch with `scripts/merge_checkpoint.sh` (full byte-identical check, README progress table) and fast-forwarded
  `main` after every merge (user's instruction); integration agents reconciled shared headers.
- **Result (code compiled from matching C):** all game code 80.2%; EXE 94.2% → 98.9%; tier-1 overlays 7.9% → 85.4%
  (every one split into C files); tier-2 0% → 86.9%; WSTAG 0% → 62.3% (all 293 build, 292 are C units). Data per file
  and in C for the EXE and every tier-1/tier-2 overlay. EXE, FIELDSTG, FIGHTSTG, CARDGAME largely named. Dozens of
  GCC 2.8.1 matching patterns recorded in DECISIONS.
- **Incidents:** several API usage limits and two container restarts (likely out of memory from parallel full
  builds: `DW3_JOBS`, incremental builds and at most 4 agents since). Agents resumed from their transcripts; committed
  work survived every time (checkpoint merges). Agents can't merge the orchestrator branch (permission classifier),
  so stale headers in agent worktrees were reconciled at merge time.
- **Needs the user:** the stage setups (DECISIONS "Stage setups don't match"); the review items in STATUS.
- **Next:** see STATUS "Next".

## 2026-10-01: Session 5: EXE game code 94% in C, all tier-1 overlays build, first overlay C
- **Setup:** this worktree started at `main` (session 3); fast-forwarded to the unmerged `Session-4` first.
- **Goal:** finish the EXE's game code (A), settle the open boundary questions and plan the data split (B),
  start the overlays (C).
- **Result:**
  - **A:** the last EXE function tables sized as module structs. Sound is one object (libgte's "sine tables"
    were folded table offsets). Six agents decompiled 93 of the 102 remaining functions; an integration agent
    moved local types into headers and applied the build changes they found evidence for (ASPSX 2.80, no
    `-fno-builtin`, more `-G8`, struct sizes). **244 → 337 / 346 (94.2%)**. 80 names applied; `inn`, `overlay`.
  - **B:** `-G8` for every EXE game file; gfx split into `gfx`, `game_8001E7E0`, `sprite`, `font`, `tim`; the
    other open boundaries kept (no evidence); `.data`/`.bss` split plan approved, not applied.
  - **C:** overlay code is GCC 2.8.1 `-G0`. Configs for the other 17 tier-1 overlays (`overlay_layout.py`;
    LIBPRESS in STDWTITL; CNTY_SEL's odd size). FIELDSTG/FIGHTSTG split at jump-table parity breaks into 5 + 7 C
    files; four agents + an integration pass: **FIELDSTG 182 / 222, FIGHTSTG 164 / 301**. Overlay `.bss` is in
    the file.
- **Incidents:** decompilation agents shared one scratch directory at first and overwrote each other's helper
  files (now: one subdirectory per agent). A usage limit stopped four agents mid-edit; resumed from their
  transcripts, every file re-checked.
- **Next:** see STATUS "Next".

## 2026-10-01: Session 4: 244 functions in C, module structs, -G8 settled, first overlays
- **Goal:** decompile the easy EXE functions (A), turn the heap table into a struct and settle the session-3 file
  boundaries (B), build FIELDSTG/FIGHTSTG (C).
- **Result:**
  - **A:** `compiler_id.py` rerun over all functions; the 33 tiny ones and 57 near-misses (2.8.1 >= 90%) plus ~75
    neighbours went to C, by 4 parallel agents (one per group of files) checking with the new `tools/unit_diff.py`.
    75 → 235 functions; no permuter leftovers kept.
  - **B:** `heap_funcs` (`HeapFuncs`) and 7 more module structs (state + function table) got sizes, so splat writes
    base + offset; that fixed 7 label-only near-misses (241). `message` and `memcard` are `-G8` (string literals in
    `.sdata`); new `-G8` unit `game_800134AC`; `game_80014274` → `object`, `game_80015590` → `gamestate`; 54 functions
    named. 244 / 346 (28.8%). Shared types moved to headers (`include/{cdload,gfx,heap}.h`, `include/psyq/lib*.h`).
  - **C:** FIELDSTG and FIGHTSTG build byte-identical, linked against the EXE's symbols; no Psy-Q code in them.
    Per-overlay symbol files; unnamed overlay symbols are `func_FIELDSTG_<addr>` (user decision).
    `tools/split_case_sizes.py` merges switch-case pieces spimdisasm split off.
- **Incident:** one agent ran `pkill -f decomp-permuter/permuter.py`, which could have stopped another agent's
  permuter run (none was lost; permuter results are hints only). Future agent prompts should forbid broad `pkill`.
- **Next:** see STATUS "Next".

## 2026-10-01: Session 3: file split, first quick wins, Psy-Q objects settled
- **Goal:** split the EXE's game code into source files (A), decompile the m2c quick wins (B), settle the
  ambiguous Psy-Q objects and split the libraries per object (C).
- **Result:**
  - **A:** 14 game units with per-file `.rodata` (DECISIONS "Game code file split"). Evidence came from rodata
    jump-table alignment, `.data`/`.sdata`/`.sbss` order, `$gp` owners, module function tables, and duplicated
    static helpers. The US decomp's split was a cross-check only; it agrees on 6 boundaries, and we differ at
    `0x80014854` (memcard's init, missed as a function start) and `0x8001764C`. `tools/game_xref.py` regenerates
    the evidence table. Unsure boundaries are listed in DECISIONS.
  - **B:** every game unit is a C file (splat stubs). The 69 functions m2c matches unedited are C, plus 2 empty ones
    splat wrote: game 3,492 / 65,088 B (5.4%), 75 / 348 functions. `cc_psx.sh` uses maspsx `--use-comm-section`, so
    `-G8` files can define their `$gp` variables while the data stays in asm. Lessons: small externs in `-G8` files
    need their real (large or incomplete) type, function-pointer tables must be structs (GCC 2.8 aliasing), and
    unknown types are named `Unk<addr>`. Named: `memcard_init`, `heap_free`, `pad_start_com`, `pad_stop_com`,
    `pad_state`.
  - **C:** all 19 ambiguous Psy-Q objects named from call targets, shared variables and tables (TOOLCHAIN.md);
    `func_8002E10C` is `CdIntToPos`. The libraries are split into 283 units `psyq/<lib>/<obj>` with `.rodata` for 22
    of them. objdiff units are named by path.
- **Next:** see STATUS "Next".

## 2026-10-01: Session 2: matching toolchain, Psy-Q identification, Milestone 0
- **Goal:** session 2 goals 1–3 (toolchain, Psy-Q signatures, asm-only rebuild of the EXE).
- **Result:**
  - `setup.sh` installs old-gcc 0.17 (5 versions), objdiff-cli 3.8.2 and pinned clones of maspsx, m2c,
    asm-differ, decomp-permuter and psx_psyq_signatures. `scripts/check_toolchain.sh` proves the chain;
    only 2.8.0/2.8.1/2.95.2 reproduce `func_800141E4`'s exact instructions.
  - `tools/psyq_match.py` (own matcher): **Psy-Q 4.7**, 284 objects, SDK `0x80020D8C`–`0x8003EDCC` with no gaps,
    game code `0x80010F4C`–`0x80020D8C` (65 KB), crt0 = `2MBYTE.OBJ`. ghidra_psx_ldr (headless) agrees on
    the version and 403 names. `config/symbol_addrs.txt`: 431 SDK names + `main`; 19 ambiguous left unnamed.
  - **Milestone 0:** `config/main.yaml` + `configure.py` (ninja, objdiff.json) + `scripts/build.sh --check`
    rebuild `SLES_039.36` byte-identical. Key settings: `.bss` linked as loaded zeros, `subalign: 4`.
    objdiff report and asm-differ work on the asm build.
  - **Milestone 1:** `func_800141E4` from C in `src/main/filetable.c`: objdiff 100%, EXE still byte-identical;
    GCC 2.91.66 fails the check (negative test). Added `tools/cc_psx.sh` (one compile chain for ninja, the smoke
    test and the permuter), depfiles, and permuter settings. The permuter forced a 2.91.66 match only with
    nonsense temporaries, so the compiler-ID experiment must score natural C.
  - Found the game's file table: LBA and sector-size arrays for all 2,382 `AAA/` files, by file ID.
  - Naming convention decided (snake_case, module prefix = file name) and applied: `filetable.c` complete (4/4 in C),
    with the file-table data and the `filetable_funcs` pointer table named.
  - Compiler-ID experiment (`tools/compiler_id.py`): GCC 2.8.x confirmed (68 of 259 unedited m2c drafts match exactly;
    next best 36; 2.8.0 = 2.8.1). `-G0` by default; `$gp` code (`main`, a cluster at `0x8001D3F4`–`0x8001FF9C`) is
    `-G8` with its variables defined in the same file, since ASPSX never uses `$gp` for `.extern`s.
- **Next:** file boundaries in the game code, then decompiling (starting from the m2c quick wins).

## 2026-09-30 → 2026-10-01: Session 1: setup, discovery, planning
- **Goal:** environment audit, repo scaffold, disc extraction and survey, toolchain plan.
- **Result:**
  - Local toolchain (binutils 2.47, mkpsxiso 2.30, splat venv) built by `scripts/setup.sh` and tested.
  - Disc verified against Redump #3160, extracted with dumpsxiso `-pt` (the directory records are
    obfuscated), and rebuilt **bit-identically**.
  - EXE memory map recovered from crt0. 318 overlays found, with two load addresses checked for every overlay.
  - Toolchain fingerprints recorded.
  - Found an active US-version decomp (juandav/dw3_decomp). The user decided to stay an independent,
    EU-only project and use it as a reference.
  - Toolchain/workflow plan approved (`WORKFLOW.md`).
  - The user's 60fps patch is 2 data bytes; the user's Ghidra project uses the patched EXE.
  - Checked from a fresh clone: setup → extract → rebuild → survey → checksums all pass (~1.5 min).
  - Decided: no Psy-Q SDK and no decomp.me for now (both fallbacks). Added worktree support for Orca
    (`scripts/worktree_init.sh`; tools built once in the main checkout and symlinked).
- **Next:** session 2 goals 1–3 in `STATUS.md` (install the matching toolchain, Psy-Q signature pass,
  Milestone 0 asm-only rebuild).
