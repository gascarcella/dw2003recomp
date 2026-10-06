#!/usr/bin/env bash
# Which parts of CI a change needs (.github/workflows/ci.yml; DECISIONS "CI per area"). Prints game=, port= and
# launcher= (true/false) for $GITHUB_OUTPUT, and each file's area on stderr.
#
# Usage: scripts/ci_areas.sh FILE...            the areas of these paths (as `git diff --name-only` prints them)
#        scripts/ci_areas.sh --diff BASE [HEAD]  the areas of what changed from BASE to HEAD (default HEAD)
#        scripts/ci_areas.sh --all               every area (manual runs, release tags, unknown history)
#
# The areas and what each runs (ci.yml has the steps):
#   game      the byte-identical rebuild (build.sh --check), the toolchain smoke test, all of scripts/test.sh
#   port      the PC port: its SDL build and input self-tests, the probe, scripts/test.sh's layers 1, 3 and port (every
#             test that compiles or runs port/: the host replays of the gpu/gte/mdec goldens, tests/spu, tests/xa, the
#             save round trips, tests/port), the -m32 build's M1 test. Layer 2 (the emulator's replays and SPU trace)
#             reads nothing of port/.
#   launcher  the launcher's build and self-test, and its run with the disc and the SDL game
# Each implies the next: game => port (the port compiles the game's C, and its tests replay the game's scripts) =>
# launcher (the launcher compiles port/src/json.c and sha1.c, lists port/mods/*/mod.json, and its self-test starts the
# game with --config: the settings contract). A path no rule below names counts as game, so a new kind of file runs
# everything until it gets a rule (a too-narrow filter that skips a needed test is worse than a broad one).
set -euo pipefail

# The area of one path: all, game, port, launcher, or none (no CI step reads it).
area_of() {
    case "$1" in
        # CI itself, and this mapping: everything.
        .github/workflows/ci.yml|scripts/ci_areas.sh) echo all ;;
        # Documentation (ci.yml's paths-ignore too): no step reads it.
        docs/*|*.md|LICENSE|.gitignore|.github/ISSUE_TEMPLATE/*) echo none ;;
        # The release's own files: release.yml tests them (a manual run); here only `bash -n`, which always runs.
        .github/workflows/release.yml|scripts/package_appimage.sh|packaging/*) echo none ;;
        # The launcher's own tree.
        launcher/*) echo launcher ;;
        # The port: its sources (port/src/json.c, sha1.c are the launcher's too: port => launcher), its generator, and
        # the tests that build or run its code.
        port/*|tools/port_gen.py|tests/port/*|tests/spu/*|tests/xa/*|tests/saves/*|tests/host/*) echo port ;;
        # Everything else: the game's C and headers, config/, configure.py, tools/, the other tests, scripts/ (setup.sh
        # pins every tool), .claude/hooks/, and anything new.
        *) echo game ;;
    esac
}

game=false port=false launcher=false
mark() {
    case "$1" in
        all|game) game=true; port=true; launcher=true ;;
        port) port=true; launcher=true ;;
        launcher) launcher=true ;;
    esac
}

files=()
case "${1:-}" in
    --all) mark all; echo "ci_areas: every area" >&2 ;;
    --diff)
        [[ $# -ge 2 ]] || { echo "usage: $0 --diff BASE [HEAD]" >&2; exit 2; }
        mapfile -t files < <(git diff --name-only "$2" "${3:-HEAD}")
        echo "ci_areas: ${#files[@]} file(s) changed from $2 to ${3:-HEAD}" >&2 ;;
    -h|--help) sed -n '2,22p' "$0"; exit 0 ;;
    *) files=("$@") ;;
esac
for f in "${files[@]}"; do
    a="$(area_of "$f")"
    printf '  %-9s %s\n' "$a" "$f" >&2
    mark "$a"
done
echo "game=$game"
echo "port=$port"
echo "launcher=$launcher"
