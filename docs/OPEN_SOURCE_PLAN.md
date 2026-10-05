# Going public: repository layout, private material, open questions (proposal)

**Status: decided by the user on 2026-10-05 (session 12), DECISIONS "Going public".** Section 4 records the choices
next to each item; section 5 is the migration checklist as adjusted by them (a clean snapshot, not a filtered history).

Context: the matching phase is closed (98.6% of game code in C, every file byte-identical), the reference tests are
in good shape (`scripts/test.sh` green), and the PC port is waiting on `docs/PC_PORT_PLAN.md` section 4. The user
wants an open-source release and to start the port. This file compares the repository layouts, lists everything that
must not go public and how it keeps working, and collects the decisions that have to be made first.

## 1. What the repository holds today

Measured on this tree (`2316cb7`, the cloud clone is shallow, so counts are of files, not commits).

| Part | Tracked files | Size | Public? |
|---|---|---|---|
| `src/`, `include/`, `config/`, `tools/` (scripts), `scripts/`, `tests/`, `docs/`, root files | 924 | 22 MB | yes: this is the project |
| `gamedata/` (the owner's game files for building and testing, and a console BIOS for cross-checks) | 328 | 467 MB | **never** (DECISIONS "Game data in the owner's data checkout", "A real PAL BIOS in the owner's data checkout") |
| `tools/prebuilt/` (the pinned PCSX-Redux Linux build, MIT) | 3 | 86 MB | legal to publish, but `tools/prebuilt/README.md` already plans a download step instead |

Both private parts are in history: `gamedata/` since session 6 (2026-10-02), `tools/prebuilt/` since 2026-10-04. Every
commit after those dates carries them, so a public release means rewriting history or starting a new one (section 5).

What refers to them, and so has to keep working once they move:
- `scripts/setup.sh` step `gamedata` (prepares `iso/dw2003.bin`, SHA-1 checked), `scripts/worktree_init.sh`
  (`extracted/disc -> gamedata/disc` fallback), `.claude/hooks/session-start.sh` (cloud sessions), `scripts/setup.sh redux`
  (unpacks `tools/prebuilt/`).
- `scripts/check_emulator.sh --bios retail`, `tests/golden/oracle.py` and `tests/replay/replay.py` (`RETAIL_BIOS` path).
- `.gitignore` (`!/gamedata/` exception), `CLAUDE.md` (the "one exception" rule), `gamedata/README.md`, the two DECISIONS entries.

What is public-safe by design, worth keeping that way:
- The goldens store fixtures as synthetic bytes and anything over 256 bytes as a SHA-1; the replay expectations are
  hashes; the families reference disc files by path under the user's `extracted/`. No disc bytes are committed.
- OpenBIOS is the default oracle; the retail BIOS is a cross-check only (`tests/README.md` rules). Nothing changes.
- `include/psyq/*.h` are our own declarations; no SDK file is tracked (`tools/psyq`, `sdk/` ignored).
- `docs/THIRD_PARTY.md` records the MIT borrowings from juandav/dw3_decomp (ideas, no text copied): a public release
  only needs the notices, as DECISIONS "Independent EU-only project" intended.

What is in the tree and should be looked at before publishing:
- **Game data as C initializers.** `src/` carries the game's `.data` as C (`src/main/records.c` is 145 kB of stat tables;
  every WSTAG stage table; `tools/data_to_c.py`). This is how every decomp project ships its data segment, and the
  US decomp does the same, but it is game data in source form and the one gray area a takedown could cite. See decision 4.
- **Two sentences in `docs/DECISIONS.md`** (line 69: the user's contacts for original Psy-Q releases; lines 1159-1160: the
  user's own BIOS dumps). Personal, not needed; remove or generalise.
- **No `LICENSE` file.** Recommended since session 8 (STATUS "Review follow-ups"); a hard prerequisite now.
- **Commit trailers.** Every agent commit ends with `Co-Authored-By` and a `Claude-Session:` link. The links open only
  for the owner. Harmless, but they are in every message; stripping them is one `--message-callback` during the filter
  (decision 5).
- **Agent-process notes** in `docs/SESSION_LOG.md`, `docs/MECHANICS.md` (token budgets, orchestration): fine to publish,
  slightly unusual for readers. Decision 7.

## 2. Repository layouts

### Layout A: private superset repo, public filtered mirror (the proposal in the question)
Keep working in the private repo (code + `gamedata/` + `tools/prebuilt/`); publish a copy with those paths filtered out,
re-run the filter to sync.

- **Works for a one-shot release.** `git filter-repo` is deterministic, so a re-run produces the same rewritten commits,
  and incremental syncs are possible.
- **Poor for an ongoing public project.** Two `main` branches to keep aligned; every outside pull request lands in the
  public repo and has to be ported into the private one by hand; CI has to run in the public repo anyway, because that
  is the tree outsiders build; the public commit hashes differ from the ones the docs cite. This is the
  "fork with secrets" pattern, and it costs a little on every single commit, forever.

### Layout B: the public repo is the working repo; a small private data repo beside it *(recommended)*
Invert the proposal. The code repo (this repo minus `gamedata/` and `tools/prebuilt/`) is public and is where every
branch, agent worktree and merge happens, exactly as today. A second, private repository, `dw2003-gamedata`
(name to decide), holds `gamedata/` and `tools/prebuilt/` with no history worth keeping. Nothing else is private.

- **One source of truth.** No sync, no porting of contributions, the docs' hashes stay valid from the first public commit on.
- **Local work is unchanged.** `tools/local.env` already points at the user's own disc; `setup.sh gamedata` becomes
  "use `$DW3_GAMEDATA` (default: `../dw2003-gamedata`) when it exists", with the same SHA-1 check.
- **Cloud sessions:** a session's repositories are chosen when it starts (the environment docs), so a cloud environment
  with both repos selected clones both into the container; the SessionStart hook looks for the sibling checkout and runs
  the same `setup.sh gamedata` step. The one thing to verify before cutting over (section 5, step 0) is that the hook sees
  the second checkout, or can clone it through the session's git proxy.
- **CI:** a read-only deploy key or fine-grained token for the data repo in the public repo's Actions secrets gives
  `build.sh --check` and `scripts/test.sh` on `main` the real disc (GitHub's Ubuntu runner has the same glibc as the cloud
  container, so the pinned Redux sysroot path applies). Pull requests from forks get no secrets, so they only get the
  checks that need no disc; maintainers re-run on a branch. Standard for decomps.
- **Cost:** one migration (section 5) and a one-session test of the cloud plumbing.

### Where the PC port lives
`docs/PC_PORT_PLAN.md` decision 9 recommends a `port/` directory in this repo (shim, platform layer, CMake, hook macros in
`include/`, a few `#ifdef PC_PORT` in `src/`, each verified byte-identical); `tests/README.md` still says "a separate
repo that vendors this directory". They have to agree.

| | Monorepo: `port/` here *(recommended)* | Separate port repo (Repo C) |
|---|---|---|
| The hooks (`include/`, `#ifdef PC_PORT`) | one commit, one `build.sh --check` | live here anyway; every port change that needs one is two repos and a submodule bump |
| `tests/` (the goldens the port replays) | shared in place | vendored or a submodule; drifts |
| Byte-identical guarantee | the same CI run proves the hooks are inert | proven here, consumed there |
| Players vs decomp contributors | one issue tracker, labels | separate trackers, cleaner story |
| Binary releases | GitHub Releases of the monorepo | natural |
| "Private at first" | a public repo nobody has been told about is private in practice; or a long-lived branch | a private repo, public later (but then its history has to be clean from day one, too) |

The separate repo buys a cleaner story for players and costs coordination on every hook. Recommendation: `port/` in
the public repo, unannounced until the port boots to the title screen (M1 or M2 of the port plan), then a release.

## 3. The private material, piece by piece

| Item | Where it goes | How it keeps working |
|---|---|---|
| `gamedata/` (the game files `setup.sh gamedata` and `worktree_init.sh` read) | the private data repo | `setup.sh gamedata` reads `$DW3_GAMEDATA` (default `../dw2003-gamedata`); `worktree_init.sh` unchanged otherwise |
| the console BIOS for cross-checks | the private data repo | `check_emulator.sh --bios retail`, `oracle.py`, `replay.py`: `RETAIL_BIOS = $DW3_GAMEDATA/bios/...`; OpenBIOS stays the default, so public `test.sh` is unaffected |
| `tools/prebuilt/` (PCSX-Redux zip, MIT) | the private data repo **and** a `setup.sh redux` download from distrib.app with the pinned SHA-256 (the public path; cloud sessions keep using the copy) | `tools/prebuilt/README.md` already documents the URL, build and hashes |
| `.claude/hooks/session-start.sh`, `.claude/settings.json` | stay public (they do nothing outside cloud sessions) | the hook adds "find the data checkout" |
| `tools/local.env` | untracked, unchanged | |
| The two DECISIONS sentences | removed from the public history (edit before the filter, or `--replace-text`) | |

Everything the public repo needs from the user is what the README already says: their own disc as a raw `.bin`.

## 4. Decisions needed from the user
1. **Layout:** B, the public repo as the working repo plus a private data repo *(recommended)*; or A, the private
   superset with a filtered public mirror.
    **Decided: B.**
2. **Port placement:** `port/` in the public repo, unannounced until it boots *(recommended)*; or a separate port repo
   (then `tests/` is vendored and the hooks still live here). Supersedes `PC_PORT_PLAN.md` decision 9; fixes
   `tests/README.md`'s wording either way.
    **Decided: `port/` in the public repo, unannounced until it boots.**
3. **Licence:** MIT for everything we wrote (code, tools, tests, docs), with the usual decomp notice in the README (not
   affiliated with Bandai; no game assets; a disc is required; the recompiled executable remains the publisher's)
   *(recommended, as `PC_PORT_PLAN.md` decision 8)*. Under MIT the port admits only MIT/BSD/zlib code.
    **Decided: MIT with the notice.**
4. **Game data as C initializers:** accept, as every decomp does *(recommended)*; or move the `.data` segments back to
   extracted binaries, which undoes session 6's data split and makes the port's data access worse.
    **Decided: accept.**
5. **History:** a fresh public repo from a `git filter-repo` export of this one (paths `gamedata/`, `tools/prebuilt/`
   removed; this repo kept as a private read-only archive) *(recommended, as session 8 advised)*; or force-push the
   rewrite over this repo. Sub-choices: keep or strip the `Claude-Session:` trailers (recommended: strip the links, keep
   `Co-Authored-By`); the two personal sentences in DECISIONS removed in the same pass.
    **Decided: a clean snapshot** (one initial commit of the current tree, no history, so no trailer rewrite) into a new repo the user creates; this repo is trimmed at its tip to `gamedata/` + `tools/prebuilt/` (history kept) and becomes the `DW3_GAMEDATA` checkout and the archive.
6. **Timing:** publish now, before the port *(recommended: the matching milestone is a clean, finished state to publish;
   it also makes the tag `v0.1-matching-closed` the first release)*; or after the port's M0/M1.
    **Decided: now.**
7. **Docs:** publish `SESSION_LOG.md`, `DECISIONS.md`, `MECHANICS.md`, `AGENT_BRIEF.md` as they are (minus the two
   sentences) *(recommended: they are the project's record and its clean-room evidence)*; or trim the agent-process
   content first.
    **Decided: as they are, minus the two sentences;** the public docs keep neutral mentions of an optional `DW3_GAMEDATA` checkout, no details of its contents or origin.
8. **CI:** GitHub Actions on `main` and on branches of the repo with the data repo through a secret, running
   `build.sh --check` and `scripts/test.sh` (~5 min; the emulator runs headless as in cloud sessions); fork PRs get the
   disc-free checks only, maintainers re-run on a branch *(recommended)*. Or no CI until contributors appear.
    **Decided: Actions with the data repo through a secret.**
9. **Names and account:** the public repo's name (`dw2003recomp` as now?) and the data repo's (`dw2003-gamedata`?),
   both under the user's account unless an organisation is wanted.
    **Decided: `dw2003recomp` for the public repo**, under the user's account; this repo is renamed first (its name is free for the new one after that; see the sequence in section 5).
10. **Contribution rules:** a `CONTRIBUTING.md` from `CLAUDE.md`'s rules (never commit game data, matching status, naming,
    `THIRD_PARTY.md` on borrowing, `build.sh --check` before a PR), and whether the `claude/*` agent-branch and
    `docs/SESSION_LOG.md` protocol apply to outside contributors (recommended: no; they send PRs, the log stays ours).
    **Decided: public read-only for now;** no CONTRIBUTING.md, the README says PRs are closed by policy until the port exists (GitHub cannot disable PRs on a public repo).
11. **Takedown posture (not legal advice):** publishing a decompilation carries a takedown risk whatever the layout; the
    mitigations are the ones already in place (no assets, user-supplied disc, no SDK files, documented clean-room
    process). Decide whether that risk is acceptable; it is the same risk the US decomp has carried publicly.

Still open:  `PC_PORT_PLAN.md` section 4 is still undecided (11 items); decision 2 above
replaces its item 9, the other ten remain.
    **Decided: accepted with the existing mitigations.**

## 5. Migration checklist (as decided: layout B, clean snapshot)
0. **Done (session 14, cloud).** Prove the cloud plumbing: a cloud environment selecting `dw2003recomp` and `dw2003-gamedata`
   clones both into `/home/user/` side by side; `scripts/gamedata_dir.sh` finds `../dw2003-gamedata`, the disc is rebuilt from
   its xz parts, `build.sh --check` and `scripts/test.sh` pass. Caveat: the session's project directory is the parent of the
   two checkouts, so the repo's `.claude/settings.json` is not loaded and the SessionStart hook does not fire; it is run by
   hand (`CLAUDE_PROJECT_DIR=$PWD .claude/hooks/session-start.sh`, CLAUDE.md step 0).
1. **Done (session 12, branch `claude/gallant-feynman-3mcny7`).** Prepare in this repo (normal commits): `LICENSE` (MIT), README notice and the disc requirement, `CONTRIBUTING.md`;
   `setup.sh gamedata`/`redux`, `worktree_init.sh`, the hook, `check_emulator.sh`, `oracle.py`, `replay.py` read
   `$DW3_GAMEDATA`; `.gitignore` loses the `!/gamedata/` lines; `CLAUDE.md` loses the exception; the two DECISIONS
   sentences go; `tests/README.md` says where the port lives; `scripts/test.sh` green and `build.sh --check` byte-identical
   with the data in the sibling checkout.
2. **Done 2026-10-05.** Rename, then create (user, GitHub settings): push everything from the current session first. Rename this repo
   (`dw2003recomp` -> `dw2003-gamedata`): GitHub redirects the old URL, so existing clones keep working. Then create the
   new empty public `dw2003recomp`: from that moment the old name points at the **new** repo, so any clone still using the
   old URL (this cloud session's `origin`, local checkouts, worktrees) must `git remote set-url origin
   git@github.com:<user>/dw2003-gamedata.git` before its next push, or it pushes game data into the public repo. Install
   the Claude GitHub App on the new repo; the next cloud session selects both repos.
3. **Done 2026-10-05 (`c1ca6f2`, from `e446f0e`).** Snapshot: on the prepared tree of step 1, `git ls-files` minus `gamedata/` and `tools/prebuilt/` copied into a fresh
   `git init`, one commit ("Initial public snapshot of the matching milestone"), its README naming the private commit it
   was taken from. Prove it: `git rev-list --objects --all` shows no `gamedata/` or `tools/prebuilt/` path and no blob
   over 1 MB (the largest public file is `tests/golden/fightstg_rules.json`, 2.1 MB: whitelist it).
4. **Done (session 12: cold setup, 2,100 outputs byte-identical, 4 test layers green).** Verify the snapshot: clone it, put the data checkout beside it, run `scripts/setup.sh && scripts/build.sh --check &&
   scripts/test.sh`; the SHA-1s are the proof.
5. **Hash references:** the docs cite `a424d38` and `0804d5b`, which exist only in the private history; the public docs
   say so once (section 1 of the README's history note) and keep them, since the private repo stays as the archive.
6. **Done 2026-10-05 (session 13), except the environment:** `main` pushed, the private repository trimmed; the tag `v0.1-matching-closed` on `c1ca6f2` with a pre-release "v0.1 — matching milestone"; the read-only deploy key on `dw2003-gamedata` and the secret `GAMEDATA_DEPLOY_KEY` here, CI green (build `--check` and `test.sh` included); description, topics, Issues on, Wiki/Projects/Discussions off; the README notice is in. **User:** the cloud environment with both repositories (step 0); pull requests are closed by hand. Publish: push to the new repo, tag `v0.1-matching-closed` on the initial commit, enable CI with the data-repo
   secret, README notice "pull requests closed by policy until the port exists". Trim this repo at its tip to
   `gamedata/` + `tools/prebuilt/` + READMEs (one commit, history kept). Switch the cloud environment to both repos.
7. **Port:** decide `PC_PORT_PLAN.md` section 4, then M0 in `port/`.
