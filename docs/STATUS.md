# Status

_Last updated: 2026-10-05 (session 15, worktree branch `gascarcella/PC-Port-Kickoff`, delivered as a pull request):
the PC port's decisions taken (DECISIONS "PC port decisions (session 15)"), **M0 done** (every PS1 byte unchanged: hook
macros in `include/port.h`, every `INCLUDE_RODATA` and the EXE matrix block in C, literal sizes to `sizeof`, all 388 units
host-clean under `tools/port_inventory.py probe`, which now gates `scripts/test.sh` and CI), and the **M1 skeleton**:
`port/` builds all 388 units plus the stub shim into one 64-bit binary that runs the game's `main()` to a frame cap.
Code state: every game file rebuilds byte-identical, 98.6% of game code compiles from matching C, 8 holdouts stay asm)._

The README's progress table (`tools/progress.py --readme`) has the current numbers per part and per overlay.

## Exists and verified
- **Byte-identical builds of every game file:** `scripts/build.sh --check` (from scratch: splat → ninja, ~2 min) rebuilds
  `SLES_039.36`, the 19 tier-1 overlays, WFIGHTMN/WFIGHTTS and all 293 `WSTAG###` files and checks each SHA-1.
  `scripts/build.sh` (incremental) also checks every output ("build passed").
- **Code compiled from matching C** (objdiff, Psy-Q excluded): all game code **98.6%** (3,598 / 3,606 functions):
  EXE 100% (346/346), tier-1 overlays 98.1% (1,607/1,614), tier-2 95.7% (55/56), WSTAG 100% (1,590/1,590).
  8 functions are WIP C under `NON_MATCHING` (most at 97–99.9%: register allocation or scheduling); every WIP
  compiles with `-DNON_MATCHING`.
- **EXE:** 19 game units (`inn`, `fieldmenu`, `records`, `cdload`, `filetable`, `object`, `main`, `memcard`,
  `gamestate`, `heap`, `pad`, `message`, `gfx`, `card`, `sprite`, `font`, `tim`, `sound`, `overlay`), all `-G8`.
  Every EXE game function is C (`message_dialog_draw_arrow` is a marked FAKE match).
  `.data`/`.sdata`/`.sbss`/`.bss` split per object (`tools/data_owners.py`); every game unit's data is in C except
  crt0's `.sbss` and the matrix block `0x8004DC10` (owner unknown). Psy-Q code/data stay split asm (sdk category).
- **Tier-1 overlays:** all 19 split into C files with evidence in their configs (jump-table parity, data order, copies
  of known files, define-before-use); all their `.data`/`.bss` per file and in C (except a FIELDSTG zero block with
  non-zero psylink padding, and STDWTITL's LIBPRESS data). Per overlay: README table.
- **Tier-2:** WFIGHTMN (2 files) and WFIGHTTS (6 files) link against the EXE and their parent FIGHTSTG
  (`TIER2_PARENT`); data in C.
- **WSTAG (293 stage overlays):** generated configs (`config/wstag.txt`, `config/wstag_c.txt` for the 292 C units,
  `config/wstag/<n>.symbols.txt`); every function is matching C (stage setups: DECISIONS "Stage setups match").
  `tools/wstag_groups.py` groups shared functions and propagates a matched one to its copies.
- **Names:** EXE 230+ symbols; FIELDSTG 212/222 functions, FIGHTSTG 272/301, CARDGAME 181/306, all small overlays and
  STGDGLAB/STSTATUS/STGTRAIN/STITSHOP/... mostly named; shared overlay types in `include/overlay_common.h`.
  Structure notes for the field, the battle and the card game: DECISIONS session-6 entries.
- **Infrastructure:** cloud sessions (SessionStart hook, the data checkout beside the repo; with both repositories
  selected the hook is run by hand, CLAUDE.md step 0, verified session 14), worktree-isolated agents
  (`docs/AGENT_BRIEF.md`), `scripts/merge_checkpoint.sh`, `DW3_JOBS`, README progress table.
- **PC port, M0 (session 15):** `include/port.h` holds the hook macros, each the plain PS1 code without `PC_PORT` and a
  `port_*` call or constant with it: `PLATFORM_WAIT`/`PLATFORM_HALT` (the 9 interrupt-ended loops and FIELDSTG's hang),
  the scratchpad stack, `OVERLAY_COPY`, `PTR_ADD`/`PTR_TO_S32`/`S32_TO_PTR`/`PTR_TO_U32` (`setaddr` too), `SLOT_FUNC`/
  `WSTAG_ENTRY`/`OVERLAY_ENTRY`/`OVERLAY_FN` (293 stage entries, 67 script objects, 20 overlay entries), `SLOT_PTR` (50
  script pointers, `main_overlay_base`/`main_file_base`), `HEAP_*`, `BIOS_PTR`, `LATE_FUNC`/`LATE_CALL` (the 4 late-bound
  calls; the fifth, `D_8009B6A4`, binds by name). `INCLUDE_RODATA` 106 → 0 (incl. the 92 WSTAG colours; the EXE matrix block
  `0x8004DC10` is C in `message.c`); literal sizes 88 → 23 (the rest are true byte counts, each marked `PC_PORT:`);
  prototypes for `fieldstg_spots_create`/`fightstg_enemy_check_condition`, explicit casts at the CARDGAME/STDWTITL
  method slots, `CardgameGame.opponents` a pointer. `tools/port_inventory.py` (counts, probe, link, structs): **388 of
  388 units compile at `-m64 -Werror`, no duplicate global**; it runs first in `scripts/test.sh` and as a disc-free CI
  step. `tests/host/build.sh` no longer needs `-fpermissive` or the prototype headers. Left as asm: FIELDSTG's zero
  block (psylink's "IN" fill inside its `.bss` run: needs a FILL facility in `configure.py`).
- **PC port, M1 skeleton (session 15):** `port/CMakeLists.txt` (`cmake -S port -B build/port -G Ninja && cmake --build
  build/port`, 30 s) compiles the 388 units with `-DPC_PORT -DNON_MATCHING` and links them with `port/src/` (the 16 MB-aligned
  arena mirroring the PS1 from `0x80082CB0`, the overlay manager with per-overlay `.data`/`.bss` sections from a generated
  ld script and address tables from `tools/port_gen.py`: 1612 tier-1 and 1646 tier-2 functions, every tag site checked at
  build time; the interrupt pump `port_wait`; a watchdog) and `port/psyq/` (113 Psy-Q functions: the pure ones real, the
  rest fixed answers or recorders, `DW3_PORT_TRACE`; LIBC2/LIBAPI are the host libc). `build/port/dw2003 --max-frames 60`
  runs the game's `main()` to the frame cap (exit 0); with a 20-line sector reader over the BIN (an experiment, not
  committed) it loads the sound banks and CNTY_SEL and runs 6000 frames. No SDL, no drawing, no sound, no disc yet.
- **CI (session 13):** `.github/workflows/ci.yml` on every push: toolchain, script/Python checks, `check_toolchain.sh`, and
  with the secret `GAMEDATA_DEPLOY_KEY` (a read-only deploy key of `dw2003-gamedata`) `build.sh --check` and
  `scripts/test.sh`; first green run 2026-10-05, ~5 min. Fork pull requests get only the disc-free steps.
- **Emulator (session 8):** `scripts/setup.sh redux` installs the pinned PCSX-Redux build from `tools/prebuilt/` into
  `tools/redux/` (through a pinned glibc sysroot where the host is too old, as in cloud containers) and the SessionStart
  hook runs it. `scripts/check_emulator.sh` boots `iso/dw2003.cue` headlessly to CNTY_SEL in ~16 s, with OpenBIOS
  (frame 807) or `--bios retail` (the data checkout's BIOS, frame 1011); verified in this cloud session.
  Headless Lua has RAM access, savestates, a per-frame event and pad injection (DECISIONS "PCSX-Redux pinned").
- **Reference tests, layer 2 (session 9):** `scripts/test.sh` runs every available layer. `tests/replay/replay.py check`
  replays `tests/replay/scripts/*.json` in the emulator and compares with `tests/replay/expected/` (checkpoint hashes of
  `gamestate_data`, overlay and map sequences, the input trace). `new_game` (boot → English → movie skipped → title → New
  Game → FIELDSTG `0x2D7`) is recorded and reproduces exactly across runs (DECISIONS "Replay runner"). `docs/MECHANICS.md`
  maps the mechanics to functions, tables and test ideas (from the C; the FAQ hosts are blocked in cloud sessions).
- **Reference tests, layer 1 (session 9, phase 2):** `tests/golden/oracle.py gen|check` calls the game's own functions
  inside the running game (exec breakpoint on `pad_update`, sentinel return; `-debugger -interpreter`) and writes/verifies
  `tests/golden/<family>.json`; one boot for every family, ~20 s. Families: `pad_random` (364 calls), `memcard_checksum`
  (45), `gamestate_flags` (697: `get_flag` grid, `check_flags` lists, every side-effect-free `check_condition` id). All
  reproduce exactly on regeneration and with the retail BIOS (DECISIONS "Layer-1 oracle").
- **Reference tests, layer 1 overlays and host replay (session 9, phase 3):** `fightstg_rules` (1,358 cases: damage,
  special, heal, hit/critical, status rolls, escape, steal, modifiers, gauge, cost) and `cardgame_cpu` (182) load their
  overlay and data files from a kept setup case (DECISIONS "Overlay goldens"). `tests/host/replay.py` compiles the
  units with gcc `-m64` and replays all 2,646 calls: everything matches but `gamestate_check_party_stat`'s out-of-range
  indexes (`tests/host/FINDINGS.md`). `scripts/test.sh` is green (layers 1 and 2, ~90 s; layer 3 not written yet).
- **Reference tests, session 10:** `gamestate_records` (163 cases: stat setters and clamps, the `add_stat` s16 wrap,
  equipment bonuses and item sets, money and the money conditions `gamestate_flags` skipped, items, cards, forms, parties,
  playtime, item lists), `records` (29: table SHA-1s, every Digimon id, every item's icon, categories), `stfgtrep_exp`
  (169: experience thresholds, level-ups to 99, stat gains at every band edge; `STFGTREP.PRO` in the slot). Reads of
  scratch buffers (`buf:<name>`), `gamestate_data` hashed around the vsync-counted `playtime_frames`. Host replay: all
  match except finding 2 (`stfgtrep_resist_gains[8]`, past the overlay file: 0 on the PS1). Layer 3: `tests/formats/run.sh`
  runs `tools/dump_text.py --check`. `scripts/test.sh` green on all three layers (~1.5 min).

- **Reference tests, session 11** (DECISIONS session-11 entries; `tests/README.md` has every family and script):
  - Layer 1: new families `stfgtrep_forms` (digivolution: the rules live in STFGTREP), `stgtrain_rules`,
    `cardgame_cpu_choice` (`cardgame_cpu_choose_card` and its target helpers), `cardgame_rules` (classes, playability,
    effect script steps, round scoring, shuffle, deal), `ststatus_items`, `shop_rules`, `gamestate_actions` (`set_flag`,
    `gamestate_cond_object` with scratch objects), `sprite_draw`, `stgdglab_party`, `fieldstg_encounter`,
    `wfightmn_events` (the battle's event queue: turn delays with the game's 10-step Newton root, ties),
    `wfightmn_enemy_ai` (enemy technique and target choice), `wfightmn_spoils` (battle-end drop and HP write-back, the
    field battle setup and prize); `fightstg_rules` +39 boundary cases. One boot, ~30 s; all replayed on the host.
  - Oracle fixes: the instruction cache is flushed after a file write, and file writes fill whole sectors as the loader
    does (both had made goldens depend on family order). Every whole-`gamestate_data` read skips the vsync-counted
    playtime word, so family order no longer matters (user decision, DECISIONS "User decisions on session 11").
  - Layer 2: the runner runs unthrottled (~500 frames/s), scripts wait on state and hold on both CPU cores (`--interpreter`),
    new steps `press` with `repeat`/`until`, `walk`, `reset`. `first_battle_save`: registration, the first (scripted) battle,
    a save at the Asuka Inn and a reload after a reset, a STITSHOP purchase and refusal, STGDGLAB, STSTATUS, STCRDABM,
    STCRDDEK, STCRDSHP, STGTRAIN (TP refusal). 14 of 19 tier-1 overlays are reached; STAGSLCT/SOUNDTST/SHOCKTST and
    WFIGHTTS have no pad route (rp-tour's evidence), CARDGAME needs party Charisma >= 60, STDGNAME later story.
  - Layer 3: file table, text tables, WSTAG stage table, flag ranges (the tools' `--check` modes).
  - Coverage (`tools/coverage.py`, not in test.sh): game functions run, union of oracle and replays: 296 -> 1,203 of 3,608;
    holdouts and FAKE matches run: 3 -> 9 of 17. `tests/holdouts/run.sh`: the `-DNON_MATCHING` image replayed: 5 holdouts
    validated, 3 not reachable (card booster, WFIGHTTS, SHOCKTST), no divergence.
  - Findings (`tests/host/FINDINGS.md`): 1 closed from data (no type-0x72 flag index >= 15: safe to bounds-check;
    `tools/flag_census.py`), 3/4 safe to bounds-check, 5 overruns inside one struct, 6 `roll_wake` reads `events[-1]`,
    **7 the first battle's end computes `next() % 0`** (no trap on the R3000A, SIGFPE on x86), 8 `void` object creators whose
    `v0` is used.

## For review
- Session 11's review points were decided by the user on 2026-10-05 (DECISIONS "User decisions on session 11"): the
  stable-hash additions stay, the whole-`gamestate_data` reads skip `playtime_frames`, no memory-poke replay step.
- Reviewed by the user on 2026-10-04 (DECISIONS "User decisions on the review items"): every judgement call listed there
  is accepted; LOOP_BLOCK class B uses kept; forced ("FAKE:") matches allowed only as an endgame step, marked and
  listed; data decoding deferred to the naming / PC-port phase.
- New since that review (all marked FAKE under the endgame rule, for a look when convenient): `cardgame_sort_cards`
  (a local copy of its parameter), `stgmcard_screen_run` (a repeated store reorg deletes), `stgtrain_anim_upload`
  (a constant in a variable before a `LOOP_BARRIER`), `stcrdshp_update_buy` (`q - -count`). `grep -rn "FAKE:" src`.

## Blocked / needs user
- **Going public: done 2026-10-05** (`docs/OPEN_SOURCE_PLAN.md` section 5; DECISIONS "Going public"): the clean
  snapshot `c1ca6f2` (from the private `e446f0e`), the tag `v0.1-matching-closed` on it with a pre-release, CI green with
  the deploy-key secret, description/topics/features set (Issues on; Wiki, Projects, Discussions off). The cloud
  environment with both repositories works (session 14: section 5 step 0 done; the hook is run by hand there). **Left for
  the user:** closing pull requests by hand (GitHub cannot disable them).
- **The port's sanitizer build:** `-DDW3_PORT_SANITIZE=ON` needs `libasan`/`libubsan` installed (sudo); the `-m32`
  oracle needs `glibc-devel.i686` (DECISIONS "PC port decisions (session 15)", item 4). Both are the user's installs.
- **Mechanics sources:** `docs/MECHANICS.md` "Sources wanted" lists the GameFAQs/StrategyWiki URLs the cloud session
  cannot reach; fetched text (or a local copy) would let the tests get names and expected behaviours from written sources.
- **Review follow-ups (session 8, recommended, not decided):** done since: the LICENSE (MIT), CI, the public release as a
  fresh repo, the milestone tag (`v0.1-matching-closed`, on the public `c1ca6f2` rather than the private `0804d5b`). Left: splitting
  the GCC-lessons journal out of `docs/DECISIONS.md`.

## Not done / open
- **8 holdouts** stay `INCLUDE_ASM` with WIP C and final notes (DECISIONS "last-rest lessons; the matching phase
  closes"; `grep -rn "^#ifdef NON_MATCHING" -A1 src`). Revisit any time; each comment says what was tried.
- **9 forced (FAKE) matches** (`grep -rn "FAKE:" src`), allowed by the user's endgame rule; revisit if natural C turns up.
- Unnamed: `unk_` fields of the other overlays (naming phase in progress), `u8` record arrays. The object
  header is `Object` and every WSTAG function and stage table is named (DECISIONS "Naming pass names-8"); FIELDSTG's
  and WSTAG's own types have their understood fields named (DECISIONS "Naming pass names-10"), and so have the EXE's
  and CARDGAME's (DECISIONS "Naming pass names-9": what is left there is unread, padding or per-effect scratch).
- Kept as is: object list in `heap`, random module in `pad`, gamestate one file vs two, `object` vs `main`.
- PC port: decided and started (`docs/PC_PORT_PLAN.md`: M0 done, M1 skeleton); `docs/PC_PORT_RESEARCH.md` stays background.
  Open from the agents' reports: `object_destroy` scans mixed data blocks as pointers (FINDINGS 9c), `gfx.h`'s
  `vsync_arg` could be a pointer field, `GfxLayer.add_callback` wants a public typedef, an `offsetof` macro for the
  three partial `bzero`s, `include/psyq/` prototype nits listed in `port/psyq/README.md`.

## Next: candidate goals (priority order)
0. **Reference tests** (direction agreed in session 8; DECISIONS "Reference tests for the port"): (1) golden tests of the
   pure logic (`fightstg_rules_*`, `cardgame_cpu_*`, `gamestate_*`, memcard checksum, `pad_random`) with goldens from a
   PS1 test EXE run in the emulator; (2) record/replay traces with per-checkpoint hashes of `gamestate_data`; (3) save and
   format round trips. **Done (session 9, phase 1):** the `tests/` layout, `scripts/test.sh`, the replay runner and its first
   script (boot → first field map, deterministic). **Done (phase 2):** the layer-1 oracle (Lua-driven calls on the
   real game; DECISIONS "Layer-1 oracle") and goldens for `pad_random`, the memcard checksum and `gamestate_*`.
   **Done (phase 3):** battle and card fixtures, the host-side replay (`tests/host/`) and its findings list. **Done
   (session 10):** `gamestate_records`, `records`, `stfgtrep_exp`, layer 3's first check. **Done (session 11):** 13 more
   families, the first-battle/save/shop/menus replay, coverage, holdout validation, layer-3 checks, findings 1-8. **Next:**
   save round trips in layer 3 (with the port), a card match replay (needs a route to party Charisma >= 60 that does not
   depend on core-dependent random battles), the user's decisions in "For review", a 32-bit host build if
   `gcc-multilib` is installed.
1. Naming/readability: names-7 (fields/types from `docs/FORMATS.md`), names-8 (the object header `Object`, every
   WSTAG function), names-10 (FIELDSTG/WSTAG type fields), names-9 (EXE and CARDGAME fields, EXE data symbols), names-11
   (FIGHTSTG, tier-2, small overlays) and decode-1 (item data, event scripts, card scripts) are done: `unk_` uses
   15,646 → 1,097 (mostly never-read or not yet understood), 12 `func_` names left, no `Unk<addr>` types.
2. PC port, **M1 proper** (`docs/PC_PORT_PLAN.md`): LIBCD over the BIN (hash-checked; `psyq_cd_set_reader` exists),
   the scripted pad, the per-frame primitive-stream hash (`psyq_gpu_take_hash`) and overlay log, the headless boot to
   STDWTITL's menu with two identical runs, `-m32` against `-m64` once `glibc-devel.i686` is installed, ASan/UBSan once
   `libasan`/`libubsan` are. Then M2 (the software GPU, SDL3 via a `setup.sh` step).
3. Holdouts/FAKEs: opportunistic retries with new techniques.
