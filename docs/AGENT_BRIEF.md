# Decompilation agent brief

Rules for a decompilation agent that the session's orchestrator starts. Each agent owns a fixed set of
files (its prompt lists them) and works in its own git worktree. Read `CLAUDE.md` first, then this file, then the
DECISIONS entries named below.

## Setup (once, in your worktree)
**Worktree base:** an `Agent` worktree may start from `main`, not the session branch: if `git log --oneline -1` is not
the branch your prompt names, `git reset --hard <that branch>` first (your worktree branch has no commits yet).
```sh
scripts/worktree_init.sh          # links tools/, iso/, extracted/ from the main checkout (~1 s)
export DW3_JOBS=1                 # several agents share one machine (16 GB, 4 cores): one job each, unless your brief says otherwise
scripts/build.sh                  # splat + ninja; must end "build passed" (makes build/expected/ for unit_diff)
```
**Memory:** a from-scratch `scripts/build.sh --check` splits ~315 targets; run it with `DW3_JOBS=1`, only when
configs or symbol files changed and once before your final report. For C-only changes the incremental
`scripts/build.sh` is enough: ninja's SHA-1 rules still check every output ("build passed"). Trust only the
script's exit code; never pipe it into `head` before `&&`.
Scratch files go in `$SCRATCH/<your agent name>/`. The orchestrator gives you `$SCRATCH`, and you never write into
another agent's directory. Never use a broad `pkill`/`killall`; stop only PIDs you started.

## Loop per function
1. Draft: `tools/venv/bin/python tools/ext/m2c/m2c.py --target mipsel-gcc-c --context <ctx.h> asm/<t>/nonmatchings/<unit>/<func>.s`
   (or the drafts in `build/compiler_id/<target>/` from `tools/compiler_id.py --target <t> --rodata-end <addr>`).
   Replace the `INCLUDE_ASM` in `src/<t>/<unit>.c` with the C.
2. Score: `tools/venv/bin/python tools/unit_diff.py <unit> [func] -t <target>` (parallel-safe, no ninja).
   It also checks `.rodata`/`.data` words. `unit_diff.py <unit> <func>` prints a side-by-side
   instruction diff; asm-differ (`tools/ext/asm-differ/diff.py -mw <func>`) works for EXE functions only.
3. 100% → keep. Otherwise iterate on natural C: types, struct fields, statement order, separate temporaries.
   After roughly 30–45 minutes of effort on one function, put it under `#ifdef NON_MATCHING` with the
   `INCLUDE_ASM` in the `#else` branch, add a one-line comment with the % and the remaining difference, and move on.
4. Permuter (`tools/ext/decomp-permuter`): only as a hint, at most 10 minutes per function, `-j1`. Keep its output
   only if it reads as natural C (DECISIONS "Permuter output as a hint"). Never commit `do {} while (0)`, dummy
   temporaries or other shapes that only exist to please the register allocator.
5. Every ~10 matched functions: `scripts/build.sh` must succeed ("build passed"; `--check` after config or symbol
   changes): the whole EXE and every overlay byte-identical. Then commit on your worktree branch. Never push, never
   touch `main`. Every WIP must still compile with `-DNON_MATCHING` (`tools/unit_diff.py <unit> -t <t> -D NON_MATCHING`).

## C conventions (DECISIONS: "C patterns learned in session 5", "All game units are C files", "Module state structs")
- GCC 2.8.1 `-O2`. EXE units are `-G8`, overlays `-G0` (`configure.py` `G_OVERRIDES`/`DEFAULT_G`).
- Function-pointer tables and module state are sized structs. Splat labels inside a struct give label-only
  mismatches; fix them with a `size:` entry in the symbol file, never with scalar externs per label.
- Types used by one file stay in that file. Types used by more than one file go in the target's header
  (`include/<target>.h`, or `include/<module>.h` for the EXE). Unknown fields are `unk_1C`, unknown types `Unk<addr>`.
- Name only what you understand: `<module>_<verb>_<noun>` snake_case. EXE names go in `config/symbol_addrs.txt`,
  overlay names in `config/<overlay>.symbols.txt` (`name = 0x80012345; // type:func`). The prefix is the C file
  name; for unnamed overlay files `<overlay>_…` (e.g. `fieldstg_update_camera`). Renaming means changing every use
  (`grep -rn` in `src/ include/ config/`).
- Never decompile Psy-Q functions (the `psyq/` units).
- Shared files (`include/common.h`, `include/object.h`, `include/gfx.h`, other EXE headers, `configure.py`): add only
  what you need, append rather than reorder, and list every change in your report. The orchestrator merges the agents.

## Splitting a new overlay into files (when your prompt asks for it)
Evidence, as for FIELDSTG/FIGHTSTG (DECISIONS "Overlay file split: FIELDSTG 5 files, FIGHTSTG 7"):
- `tools/venv/bin/python tools/overlay_xref.py <target>`: lines starting `|` are rodata-parity breaks (a jump table at
  a different address mod 8 starts a new object). Data order, create/update pairs, static helpers placed before
  their only caller, and duplicated helpers (`=DUP`) refine the exact positions.
- In `config/<target>.yaml`, replace `[.., rodata, rodata]` and `[.., asm, text]` with per-file `.rodata` and `c`
  subsegments named `<target>_<vram>`, as `config/fieldstg.yaml` does. Comment each boundary with its evidence,
  marking weak positions as such. With no evidence for a break, the overlay is one file `<target>_<vram>`.
- Run `tools/venv/bin/python configure.py`; splat writes `src/<target>/<unit>.c` stubs with `INCLUDE_ASM`. The build
  must stay byte-identical. Commit the split before decompiling.
- `tools/compiler_id.py --target <t> --rodata-end <.text start>` gives the m2c drafts that already match: do those first.

## Report (your final message)
- Functions matched (count and bytes per unit), functions left as `NON_MATCHING` with their %, functions not tried.
- Names added; changes to shared files; new facts or decisions for `docs/DECISIONS.md` (one line each); file-split
  evidence if you split; anything that looked wrong.
- The final `scripts/build.sh` result and your branch's last commit hash.
