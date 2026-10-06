# Matching functions

The practical guide to turning a function's assembly into C that rebuilds byte-identical, for contributors and
agents. The rules behind it are in `docs/DECISIONS.md` ("Match status and forced (FAKE) matches", "Naming
conventions", "LOOP_BLOCK"); the compiler evidence is in `docs/TOOLCHAIN.md`. Every game function is already C except a
few holdouts (`grep -rn "^#ifdef NON_MATCHING" src`), so this guide serves retries of holdouts and FAKE matches,
new splits, and anyone checking how a shape was found.

## Setup

```sh
scripts/worktree_init.sh          # in a fresh worktree: links tools/, iso/, extracted/ from the main checkout (~1 s)
export DW3_JOBS=1                 # when several agents share a machine: one job each
scripts/build.sh                  # splat + ninja; must end "build passed" (also makes build/expected/ for unit_diff)
```

- A from-scratch `scripts/build.sh --check` splits ~315 targets (~2 min, memory-hungry). Run it only after config or
  symbol-file changes, and once before you hand in work. For C-only changes the incremental `scripts/build.sh` is
  enough: ninja still checks every output's SHA-1.
- Trust only the build script's exit code; never pipe it into `head` before `&&`.
- Scratch files go in your own scratch directory. Never run a broad `pkill`/`killall`; stop only PIDs you started.
- An agent worktree may start from `main`: if `git log --oneline -1` is not the branch your brief names, reset onto it
  before your first commit.

## Tools

| Tool | Use |
|---|---|
| splat (`configure.py`) | Splits each target from `config/<target>.yaml` into `asm/<t>/` and `src/<t>/` stubs; `configure.py` also writes `build.ninja` and `objdiff.json` |
| `tools/cc_psx.sh` | The one compile path: cpp → cc1 (GCC 2.8.1 `-O2`) → maspsx (ASPSX 2.80) → GNU as. ninja, the permuter and the scripts all use it. `-V`, `-G`, `-D`, `--data-in-c` |
| m2c (`tools/ext/m2c`) | First draft: `m2c.py --target mipsel-gcc-c --context <ctx.h> asm/<t>/nonmatchings/<unit>/<func>.s` |
| `tools/compiler_id.py --target <t> --rodata-end <addr>` | m2c drafts of a whole target, scored; the exact ones are free matches |
| `tools/unit_diff.py <unit> [func] [-t <target>] [-D NON_MATCHING]` | Scores one C unit against the original per function and data section, no ninja, parallel-safe; with a function, a side-by-side instruction diff |
| objdiff (`tools/bin/objdiff-cli`) | Per-function diff and the progress report (`objdiff.json`) |
| asm-differ (`tools/ext/asm-differ/diff.py -mw <func>`) | Live terminal diff; EXE functions only |
| decomp-permuter (`tools/ext/decomp-permuter`) | Random source mutations scored against the target; a hint, never the answer |
| `tools/psyq_compare.sh [-D NON_MATCHING] <file.c> <func>` | One function through Sony's real Psy-Q chain and ours (needs the optional `setup.sh psyq`) |
| `tools/split_case_sizes.py asm/<t>/X.s` | `size:` lines that merge switch-case pieces spimdisasm split off as `jlabel` functions |
| `tools/overlay_layout.py`, `tools/overlay_xref.py` | An overlay's section boundaries; per-function evidence for file boundaries |
| `tools/data_owners.py`, `tools/data_to_c.py` | Which C file owns each data symbol; a symbol as a C initializer |
| `tools/wstag_groups.py --funcs/--add/--propagate/--rename` | Functions shared by WSTAG stage files: match one, copy to the rest |
| cc1 dump flags | `-dL` (loop.c hoisting decisions), `-dl`/`-dg` (local/global allocation priorities) |

GCC 2.8.1's sources, for reading why the compiler chose something:
`https://raw.githubusercontent.com/gcc-mirror/gcc/releases/gcc-2.8.1/gcc/<file>.c` (`sched.c`, `loop.c`, `local-alloc.c`,
`global.c`, `jump.c`, `reorg.c`, `cse.c`).

## How targets are configured

- One `config/<target>.yaml` per link unit; the EXE is `main`. Each has a `Target` in `configure.py` and a
  `config/<NAME>.sha1`. The 293 WSTAG configs are generated from a template and `config/wstag.txt`; a line in
  `config/wstag_c.txt` makes a stage a C unit (`data` moves its `.data` into C, `split=R:T:D` adds a second unit).
- `-G`: `configure.py` `DEFAULT_G` (EXE 8, overlays 0) and `G_OVERRIDES` per file. `DATA_IN_C` lists units whose data
  is defined in C. `TIER2_PARENT` names the resident parent a tier-2 overlay links against.
- Names and sizes: `config/symbol_addrs.txt` (EXE) and `config/<overlay>.symbols.txt` (`name = 0x80012345; //
  type:func`, `size:0x34` for structs). Overlay symbols the parent calls in a child use `absolute:True`.
- After a symbol-file change splat can keep stale `asm/<t>/nonmatchings/*.s`: delete `asm/<t>` or run `build.sh --check`.
- `permuter_settings.toml` compiles at `-G 0`; set `-G 8` there when permuting an EXE function.

## The loop per function

1. **Draft** with m2c (or take the `compiler_id.py` draft) and replace the `INCLUDE_ASM` in `src/<t>/<unit>.c`.
   m2c misreads arguments of calls through function pointers (check `a2`/`a3` and stack stores in the asm) and its
   store order is not source order.
2. **Score** with `tools/unit_diff.py <unit> <func> -t <t>`. A near-100% "label-only" difference can hide a real one
   (a missing `default:`), and unit_diff does not check function order within the file, which the link needs: only
   the build proves a match.
3. **Iterate on natural C**: types, struct fields, statement order, a variable per job (the patterns below). Look for
   an already matched sibling first.
4. **Stop** after about 30-45 minutes on one function: put the C under `#ifdef NON_MATCHING` with the `INCLUDE_ASM` in
   the `#else`, plus a comment with the score and the remaining difference. Every WIP must compile with
   `-DNON_MATCHING` (`unit_diff.py <unit> -t <t> -D NON_MATCHING`).
5. **Build and commit** every ~10 matches: `scripts/build.sh` must say "build passed" (`--check` after config or symbol
   changes).

## Conventions

- Function-pointer tables and module state are sized structs (DECISIONS "Module state structs"). A label-only mismatch
  inside a struct is fixed with a `size:` entry, never with scalar externs per label; don't add the `size:` while a
  WIP's asm still uses the inner label.
- Types used by one file stay in it; shared ones go in `include/<module>.h` (EXE) or `include/<target>.h`; types shared
  by overlays in `include/overlay_common.h`. Unknown fields `unk_<offset>`, unknown types `Unk<addr>`.
- One declaration per symbol, in a header. When callers see a wider type than the definition (a `u8` getter called as
  `s32` through a module table), the table entry gets the callers' type with a comment.
- Name only what you understand, everywhere at once (`grep -rn` over `src/ include/ config/`). Rename struct fields
  through a unique temporary name first, so the compiler reports every use (`clang -fsyntax-only` names the struct of
  each error); a field-rename map must never be applied tree-wide, since `unk_304` exists in many structs.
- Never decompile Psy-Q code (`psyq/` units).
- Shared files (`include/common.h`, EXE headers, `configure.py`): add what you need, append rather than reorder, and
  list every change when you hand in.

## Splitting a target into files

Evidence (DECISIONS "File boundaries come from evidence"):
- `tools/overlay_xref.py <target>`: lines starting `|` are jump-table parity breaks: a table whose `(address - base)
  mod 8` differs from the previous one starts a new object (objects are packed at 4, tables 8-aligned within their
  object). A zero pad word after an odd-length table ties the next table to the same object.
- Positions: create/update pairs, static helpers placed just before their only caller, define-before-use, `.data` and
  `.bss` in link order, each module's function table, `$gp` users, helpers duplicated byte for byte (`=DUP`; a copy of a
  known file proves both its boundaries; a small callback copied into several groups marks one file per copy).
- `overlay_layout.py` can place `.text` too late when an overlay starts with frameless setters: check the first
  `jr $ra` before the first stack frame.

Then replace the `rodata`/`asm` subsegments in `config/<target>.yaml` with per-file `.rodata` and `c` subsegments named
`<target>_<vram>`, a comment with each boundary's evidence (weak ones marked weak), run `configure.py`, check the build
stays byte-identical and commit the split before decompiling. No evidence: one file.

## C patterns

Each pattern below matched at least one function. Try them before searching blindly.

### Types and calls
- A missing argument load means the callee takes no arguments; a register the original never sets is not an argument;
  a parameter used without sign extension is an `int`.
- An `lh` where `lhu` was expected means a copy through an `s32` local; `x * -1` loads with `lh`.
- Unused locals and structs show in the frame size; a struct passed by value shows as `a0`/`a1` home-slot stores.
- A function that leaves its object in `v0` returns it (`return obj;` changes no code); constructors whose result
  callers use return their object.
- Method slots keep word-sized parameters: narrowing a slot's parameter type changes the callers' bytes.
- A gfx draw callback takes `void *` parameters converted to typed locals.

### Structs, globals and aliasing
- GCC 2.8 assumes a struct field and a scalar global never alias. A global reloaded after a struct-field store is a
  struct member: make it a one-member struct (also for a function-pointer global called right after such stores).
- Code that reaches `X + 0x68` and `X + 0x7C` with a hoisted `%hi` only matches as separate symbols.
- cse forgets every field of a struct pointer after any store through it: the original's field reloads show where its
  stores sit.
- A store through a computed pointer conflicts with any symbol-based load unless the store is into a struct or array
  and the load isn't. A local base (`s32 *sheet = D_...`) escapes the symbol-vs-pointer conflict.
- GCC builds `la sym` + offset for struct globals; combine folds it unless the base has several uses, the use is in
  another block, a call intervenes, or the MEM is volatile.
- A constant index folded into a load offset (`lbu 2(v0)`) reveals a real middle array dimension (`[4][2][2]`).
- Split an array where every use has a constant index; merge fields where a use indexes past an array's end.
- Pointer-linked record sets are one array with `&arr[i]` pointers. Struct layouts include tail padding.

### Operand order and arithmetic
- `x->arr[k]` emits `addu x,k`; `*(x->arr + k)` emits `addu k,x`. An array inside a struct puts the base after the
  index offset. Arrays of page structs (`chars[page].k[row][col]`) fix operand order where a 3-D array doesn't.
- When the original adds the offset to the struct base (`addu obj,k`) and every index form gives `addu k,obj`, a
  `static inline` accessor gives the original's order (its parameters are fresh pseudos).
- Indexing a table directly (`T[i].a … T[i].b`) loads the base first, a pointer local doesn't; use `Entry *e =
  &array[i]` when the original computes `i * size` before the base.
- fold reassociates `y + (t + c)` into `(y + c) + t`: write `y + c + t` to get `t + c` first. `v + c + i * k` keeps
  the original's order; `a - -b` keeps `a` first.
- The left operand of a comparison loads first, so `a > b` and `b < a` differ. Reversed branch operands plus a moved
  address computation point to a block-level record pointer as its own variable.
- GCC computes an assignment's left-hand array address before the right-hand side; only an inlined call on the right
  reverses it.
- `n = 3; n -= x;` and `n = 3 - x` differ. `1 - (x != 0)` folds to `x == 0`: if the original subtracts, the `!= 0`
  value was its own variable. `x += (c ? -o : o) / n` is one statement.
- `s16 / const` becomes a 16-bit divide plus sign extension; without the `sll/sra 16` the dividend went through an
  int (`v = max_hp; v /= 5;`).
- `if (p == NULL) return 1; return 0;` becomes `sltiu` unless the `return 0` is shared.
- Chained `a = b = v` stores `b` first.
- `addiu -a; sltiu n` then an `== k` test is `if (x >= a && x < a + n) … else if (x == k)`, not a switch.
- A value loaded before a branch on another variable can be a `?:` inside the comparison (jump threading).
- cse doesn't merge `(x + 1) * 8 - 1` with `x * 8`.

### Register allocation
- Global allocation priority is `floor_log2(refs) × refs / live_length` (`-dl`/`-dg` print it); references inside a
  loop count twice. Code repeated per case or branch and merged later by cross-jumping still counts.
- Give each job its own variable: one per loop, one per switch case (block-local), one per branch when branches swap
  fields (`sx`/`sy` instead of shared `x`/`y`), fresh block-locals for intermediates. Yet the original also reuses
  counters and flags across loops: try a rename search over both directions.
- A value used both before and after a call wants its own variable assigned once.
- local-alloc ties a block-local result to its dying first operand (and never to a pseudo set more than once in its
  block); global-alloc then prefers that operand's variable for the result.
- A value used in two blocks becomes a global pseudo, which changes local-alloc's `v0`/`v1` choices.
- A local initialised at its declaration has a longer live range. Statement order within store runs changes live
  lengths; declaration order rarely matters.
- cse makes the longer-lived copy canonical (after `tim = p`, later uses of `p` become `tim`). A local holding a
  `lui/ori` constant stays in one register. Constant-init order decides which register cse copies from.
- reload_cse has no cost check: `move vX,sY` at a constant store means the constant was stored before the variable
  was set.
- When the original copies a value just stored into a field, write the read through the field
  (`card->unk_20 = card->scale_x`).
- When the original advances one register through a packet (`POLY_FT4` → `SPRT` → `DR_TPAGE`), keep one running
  pointer (`poly = (POLY_FT4 *)(sprt + 1)`), and step it in place (`tpage++; f(tpage);`).
- `lw t7; addiu t7; addu t7; sw t7,sp` means a field loaded straight into a spilled variable and stepped in place.
- Repeated `lui`/`li` before each use inside a loop is reload rematerialising a hoisted constant that got no register.

### Scheduling and store order
- Independent stores keep source order through the scheduler; swap chains are ordered by when their fields are read.
- sched1 gives top priority to an insn setting a live, once-assigned pseudo, so a load into a fresh block-local sits
  right before its use; a variable assigned more than once keeps its statement position. A value read before a store
  it could alias is a block-local initialiser.
- Scheduling ties go to the later insn in RTL order: an unrelated statement between a copy and a test can decide.
- When every statement order gives the same schedule, look at dependence chains and alias rules: sched often places a
  load to feed a just-scheduled insn.
- A store fed by a load sinks below independent constant stores (load latency). A GNU constructor assignment into a
  struct field (`s.start_pos = (GamestatePos){ x, y };`) is a store barrier for that symbol that emits no code and still
  lets the last store fill a call's delay slot; a volatile view cannot.
- A statement placed after a call moves delay-slot fills; code after a branch can decide that branch's delay slot
  (`row = 0` in an `else`).
- Pass order: sched1 → … → sched2 → jump2 (cross-jumping) → reorg. Analyse tail merges in sched2's order.

### Switches and cross-jumping
- A jump table needs at least 5 case labels (5 sparse cases still gave a compare tree, 6 a table). `slti`+`bltz` range
  tests come from a switch with adjacent cases. Check the jump table before believing a fallthrough.
- Case bodies are emitted in source order: order cases as the original places them, but try numeric order first, since
  a case out of order can switch cross-jumping on or off.
- Hidden empty cases decide codegen: `case 0: break;` when the table starts at 0; `case 0: default:` in object
  `switch (state)`es; a `slti` in a `u8` switch tree means an explicit `case 0:` sharing the default; the compare tree's
  root can reveal a case equal to `default`. A switch on -1/-2 with `default` gives the -2-first test order.
- An out-of-line stub jumping into a shared tail means the source wrote the tail in each branch: write the final call,
  `return` or clear per branch when the original has them per branch. A variable per case stops tail merging.
- Cross-jumping is order dependent: a tail first merges into the code before its target label, and a jump to a label
  that already exists (`default:` just before `result = 1;`) is recorded and merged later. Put `case 0: default:` first
  to keep per-case tails separate. Branch sense decides which copies merge.
- GCC emits a self tail call as `j` only for `return f(...)` in an int function.
- spimdisasm may start a "function" at a jump-table target: merge it back with `split_case_sizes.py`'s `size:` line.

### Loops
- loop.c hoists an invariant when threshold × savings × lifetime ≥ the loop's insn count (threshold 1 + the number of
  non-fixed registers, 29 here, with calls; −3 per value already moved; a `%hi/%lo` pair has life 2). `-dL` prints each
  decision. If the original reloads inside the loop, its loop was bigger: one more use, a per-branch tail or a type
  change can flip it, never `-f` flags. Identical constants are combined (savings and lifetimes summed); a call's
  argument evaluated before the function-pointer load reorders the movables.
- GCC 2.8.1 reverses count-up loops into count-down pointer loops: write them count-up.
- Screen positions are `i * step + base` (loop.c reduces them), not `x += step` counters; a per-iteration row offset
  `y = row * step` can be shared by an outer and inner loop.
- Table scans: an index loop (`T[i].x`) when the asm sets the walking pointer in the preheader and hoists the sentinel
  separately; `for (i = 0; D[i].x != -1; i++) { t = &D[i]; … }` avoids an extra induction variable. Check each search:
  some really are pointer walks. A base kept in one register over two loops is one base variable plus index loops.
- `found[count++] = i` and `*p++ = i` differ; a pointer step in the `for` increment (`i++, p++`) gives
  increment-before-load.
- A loop entered at its test comes from code after the loop duplicated in each branch. A "first non-zero" search:
  `i = 0; while (a[i] == 0) { if (++i >= N) break; }`. Clamped searches: `do { if (--x < 0) { x = 0; break; } } while
  (…)`.
- A global pointer read as an array element is reloaded in loops, a scalar global hoisted; a loop bound left as a
  field lets the invariant pass load it as the original does.
- Every loop that crosses a call needs its own variable. A giv kept in a user variable pays a copy cost, so extra givs
  in the original were compiler temporaries.
- GCC shares stack slots only between variables of nested blocks: a frame bigger than the original's means the original
  nested its blocks (often macro blocks).

### Small data (`-G8`) and data in C
- ASPSX uses `$gp` only for symbols defined earlier in the same file, so a `$gp` user and its variable share a file, and
  that file defines it. Taking a `$gp` variable's address (`addiu rX, $gp, %gp_rel(x)`) needs ASPSX 2.80.
- In a `-G8` file every extern of 8 bytes or less is possible small data (an unsplit `lw sym`). Where the original uses
  a `%hi/%lo` pair, declare the symbol with its real larger type or an incomplete type.
- At `-G8`, literals and `const` objects of 8 bytes or less go to `.sdata` (strings included) unless the section is
  named. Overlays are `-G0` and have no `.sdata`.
- `.bss` order is the order of first declaration (an earlier `extern`, even in a header, fixes it): define `.bss`
  variables before any other declaration, or move the extern to the only user. Explicit zero initialisers stay in
  `.data`.
- Strings of a table's initialisers are emitted where the table is defined, last element first, and GCC shares an
  identical literal within one file only. A table with strings is defined where its strings sit in `.rodata`.
- A function's `.rodata` constants can be `static const` locals; a `const` word must follow the functions whose jump
  tables precede it. A C `.rodata` definition right before an `INCLUDE_ASM` whose asm carries `.rodata` makes GCC omit
  the next `.rdata` directive: avoid that order. psylink's non-zero fill after a string becomes a `const u8[N]` with the
  fill spelled out.
- Overlay data objects are 4-aligned; the EXE's small data and `.bss` start 8-aligned (`configure.py` inserts the
  `ALIGN(8)`).

### LOOP_BLOCK and LOOP_BARRIER
A loop's begin/end notes are scheduling barriers, and blocks leaving a loop are placed out of line. When a block matches
only inside `do { } while (0)`, use `LOOP_BLOCK(body)` or `LOOP_BARRIER()` (`include/common.h`) with a comment naming
the evidence class, found by compiling the macro form and the plain form:
- **A1:** identical with `-fno-schedule-insns` (a sched1 barrier). **A2:** identical only with
  `-fno-schedule-insns -fno-schedule-insns2` too (a post-reload reorder). Unfilled load-delay `nop`s and stores before a
  call's argument moves are typical.
- **C:** still different with scheduling off, in instructions or layout: the loop pass puts a block out of line (a
  branch inverted, an extra `j`) or reorg fills a delay slot differently. The first exit block of a loop stays inline,
  later ones go out of line.
- **B:** still different with scheduling off, but only in register names (references inside a loop count twice).
  The weakest evidence: first try the natural alternatives (case order, a duplicated tail, `return` for `break`, a
  fresh variable).
A bare `do { } while (0)` stays forbidden.

### Forced (FAKE) matches
Only after natural C has been given up for that function (DECISIONS "Match status and forced (FAKE) matches"). Mark the
forced line with `/* FAKE: <what is forced and why> */`. Forms used so far: a copy variable (`arrow = ofs`, a local
copy of a parameter), a repeated store reorg deletes, mixed spellings of one access (`(p + i)->f` and `p[i].f`), a cast
chain, a `volatile` cast that stops combine folding an address, a constant in a variable before a `LOOP_BARRIER`, one
variable reused for two jobs, `q - -count`.

## Search techniques

- **Siblings first:** copy the shape of an already matched function with the same logic (locals, re-fetched records,
  nested `if`s), also from other overlays. When porting a shape from the US decomp, port it whole (all cases, its
  locals) before judging it.
- **Hand searches that worked:** renaming variables across loops (one fresh variable per loop, declared first or last),
  case splitting and case order, hidden empty cases, duplicating tails per branch, statement-order hill-climbing,
  permuting declarations and types.
- **Mini reproductions:** a standalone C file with the function's shape (fake types), compiled with `tools/cc_psx.sh`
  and cut down until the choice flips, finds a scheduler or reorg cause in minutes.
- **Permuter:** a hint only, about 10 minutes per function, `-j1`; keep its output only if it reads as natural C (a
  shape that states intent, not a forced temporary). `import.py src/<t>/X.c asm/<t>/nonmatchings/X/func.s`. For GTE
  inline asm: `--preserve-macros 'gte_\w+'` and run `strip_other_fns` on `base.c` after importing. For allocation
  differences, raise the weights of `perm_temp_for_expr`, `perm_reorder_decls`, `perm_refer_to_var`; for a difference
  inside one basic block, permute a copy reduced to that block. Re-score its outputs with an instruction diff.
- **Psy-Q cross-check:** `tools/psyq_compare.sh` when the toolchain itself is in doubt (so far it never was).

## Working in parallel

- Several agents on one machine: `DW3_JOBS` limits `configure.py` and ninja; a from-scratch build per agent at once
  exhausts memory. Size the number of agents to the machine.
- Never trust git's line merge on a big C unit two agents edited: merge per function by address and compare each side's
  per-function objdump, also with `-DNON_MATCHING`. When a scalar field becomes an array, old uses silently become
  pointer compares in WIP code.
- Hand in: functions matched (count and bytes per unit), WIPs with their score, functions not tried; names added;
  every change to a shared file; new facts (one line each); split evidence; the final build result and the last
  commit hash.
