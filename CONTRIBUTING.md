# Contributing

Thanks for wanting to help. This page covers what you need before you open a pull request. The project's background
is in the [README](README.md) and [`docs/STATUS.md`](docs/STATUS.md).

## Contents

- [Ground rules](#ground-rules)
- [What you can work on](#what-you-can-work-on)
- [Getting set up](#getting-set-up)
- [Making a change](#making-a-change)
  - [Decompilation (matching C)](#decompilation-matching-c)
  - [The PC port and the launcher](#the-pc-port-and-the-launcher)
  - [Documentation](#documentation)
- [Checks before a pull request](#checks-before-a-pull-request)
- [Opening a pull request](#opening-a-pull-request)
- [Reporting issues](#reporting-issues)
- [AI-assisted contributions](#ai-assisted-contributions)

## Ground rules

- **Never commit game data.** Nothing from the disc: no BIN/ISO, extracted files, split assembly (`asm/`), assets,
  textures, audio, BIOS, and no Psy-Q/SDK files. The repository is code only; everyone supplies their own disc. Check
  `git status` before you commit.
- **The matching build must stay byte-identical.** Every game file (`SLES_039.36`, the overlays) must still rebuild to
  the original SHA-1. A change that breaks it can't be merged.
- **Europe only.** The target is the unpatched EU disc (SLES-03936). The US decompilation is a reference, not a
  dependency: anything you borrow from another project must be recorded in
  [`docs/THIRD_PARTY.md`](docs/THIRD_PARTY.md) in the same pull request.
- **Don't decompile Psy-Q library functions.** They are identified by signature, named with Sony's names and stay as
  split assembly.
- **Tests never change the game's code.** Reference tests live in `tests/` and `tools/`, never in `src/`.
- By contributing you agree that your work is released under the project's [MIT licence](LICENSE).

## What you can work on

- **Open issues** labelled [`good first issue`](https://github.com/gascarcella/dw2003recomp/labels/good%20first%20issue)
  or [`help wanted`](https://github.com/gascarcella/dw2003recomp/labels/help%20wanted).
- **Planned work** on the [project board](https://github.com/users/gascarcella/projects/1).
- **The remaining non-matching functions** (the README's progress table; `#ifdef NON_MATCHING` in `src/`).
- **Names and types:** many overlay functions and fields are still `func_…`/`unk_…`.
- **Play-testing the PC port** against the original and reporting differences.

For anything bigger than a small fix, open an issue (or comment on an existing one) first, so we can agree on the
approach before you spend time on it.

## Getting set up

Follow the README's [Development](README.md#development) section: `tools/local.env` pointing at your disc,
`scripts/setup.sh`, `scripts/extract.sh`, then `scripts/build.sh`. Everything installs into `tools/`; no root access is
needed. If you work in a git worktree, run `scripts/worktree_init.sh` in it first.

## Making a change

### Decompilation (matching C)

The workflow, the tools (splat, objdiff, asm-differ, decomp-permuter, m2c) and the C patterns that make this compiler
match are in [`docs/MATCHING.md`](docs/MATCHING.md). In short:

- A function is **matching** when it compiles from C and the full build is byte-identical. Until then it stays
  `INCLUDE_ASM`; work-in-progress C goes under `#ifdef NON_MATCHING` and must still compile with `-DNON_MATCHING`.
- **Naming:** game symbols are `<module>_<verb>_<noun>` in snake_case, with the module prefix being the source file's
  name (`src/main/filetable.c` → `filetable_get_lba`); types are PascalCase, constants `MODULE_UPPER_SNAKE`, unknown
  fields `unk_1C`. Unnamed symbols keep splat's defaults (`func_8001xxxx`, `D_8001xxxx`). Name only what you
  understand.
- New names go in `config/symbol_addrs.txt` (the EXE) or `config/<overlay>.symbols.txt` (overlays).
- The C must also compile for the PC port: `tools/venv/bin/python tools/port_inventory.py probe` must pass. Use the
  hooks in `include/port.h` for anything PS1-specific (see [`docs/PORT.md`](docs/PORT.md)).

### The PC port and the launcher

The port's runtime is `port/` (C), the launcher `launcher/` (C++17, Dear ImGui on SDL3). Read
[`docs/PORT.md`](docs/PORT.md), [`docs/LAUNCHER.md`](docs/LAUNCHER.md), [`port/README.md`](port/README.md) and
[`launcher/README.md`](launcher/README.md). A change in behaviour should come with a test: a layer-1 golden, a layer-2
pad script, or the launcher's self-test ([`tests/README.md`](tests/README.md)).

### Documentation

`docs/` holds reference documentation only: how things are and why. Plans go in the project board, problems in
issues; there is no session log. Record a decision that changes how the project works in
[`docs/DECISIONS.md`](docs/DECISIONS.md) (a few lines: what and why).

## Checks before a pull request

CI on a pull request from a fork has **no disc**, so it can only build the toolchain, compile every unit for the host
and run the disc-free checks. Please run the rest yourself and say so in the pull request:

```sh
scripts/build.sh --check                                    # every game file byte-identical, from scratch
scripts/test.sh                                             # the reference tests (needs the emulator: scripts/setup.sh redux)
tools/venv/bin/python tools/port_inventory.py probe         # the game C still compiles for the host
```

For port or launcher changes also build them (`cmake` commands in the README) and run the relevant tests
(`tests/port/run.py`, the launcher's `--self-test`); CI also cross-builds them for Windows and runs them under Wine
(`scripts/build_windows.sh --test` is the same gate, needs `scripts/setup.sh llvm-mingw sdl3-windows` and `wine`;
a release change: `scripts/package_windows.sh --test` and `scripts/package_appimage.sh --test`, `docs/RELEASE.md`).
`scripts/ci_areas.sh --diff origin/main` tells you which areas your change touches.

## Opening a pull request

- Branch from `main`, keep the pull request focused on one thing, and write commit messages that say what changed and
  why.
- Fill in the [pull request template](.github/PULL_REQUEST_TEMPLATE.md): what changed, how you tested it, and the
  checklist.
- Update progress with `tools/venv/bin/python tools/progress.py --readme` if you matched functions.
- A maintainer reviews and merges. CI must be green.

## Reporting issues

Use the [issue templates](https://github.com/gascarcella/dw2003recomp/issues/new/choose): a **bug report** for
something that's wrong in the game, the port or the launcher, a **feature request** for an idea. Never attach game
files or a disc image. See the README's [Reporting an issue](README.md#reporting-an-issue).

## AI-assisted contributions

Most of this project was written with AI coding agents, and AI-assisted pull requests are welcome on the same terms as
any other: you are responsible for the change, you have run the checks above, and you can explain it. Say in the pull
request which parts were AI-generated.
