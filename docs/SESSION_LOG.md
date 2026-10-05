# Session log

Newest first. One short entry per session: goal, result, next steps.

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
- **Next (user):** checklist steps 2 and 6 (rename, create, fix remotes, push the snapshot, trim this repo); then the
  port plan's section 4 and M0 in `port/`.

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
