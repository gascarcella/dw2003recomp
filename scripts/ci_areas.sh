#!/usr/bin/env bash
# Which parts of CI a change needs (.github/workflows/ci.yml; DECISIONS "CI per area"). Prints game=, port= and
# launcher= (true/false) for $GITHUB_OUTPUT, and each file's area on stderr.
#
# Usage: scripts/ci_areas.sh FILE...            the areas of these paths (as `git diff --name-only` prints them)
#        scripts/ci_areas.sh --diff BASE [HEAD]  the areas of what changed from BASE to HEAD (default HEAD)
#        scripts/ci_areas.sh --all               every area (manual runs, release tags, unknown history)
#
# The areas and the jobs each runs (ci.yml has the steps; the jobs run in parallel, each gated by its area):
#   game      `check`'s workaround census (tools/hacks.py --check: reads src/, include/, config/, docs/STATUS.md),
#             `game`: the toolchain smoke test, the byte-identical rebuild (build.sh --check), scripts/test.sh's
#             layer 2 (the emulator's replays and SPU trace: nothing of port/)
#   port      `check`'s probes (the host's gcc and llvm-mingw's clang), `port`: scripts/test.sh's layer 1 and port
#             layer (every test that compiles or runs port/: the host replays of the gpu/gte/libgs_view/mdec goldens,
#             tests/spu, tests/xa, tests/port, the -m32 build's M1 test), `port-mods`: layer 3 (the formats, the save
#             round trips) and the mods, `windows`: the Windows cross-build of the game and the launcher, the launcher's
#             self-test, the crash report and the layer-2 replays under Wine (scripts/build_windows.sh --test)
#   launcher  `launcher`: the SDL game's build and input self-tests, the launcher's build and self-test, and its run
#             with the disc and the SDL game
# Each implies the next: game => port (the port compiles the game's C, and its tests replay the game's scripts) =>
# launcher (the launcher compiles port/src/json.c and sha1.c, lists port/mods/*/mod.json, and its self-test starts the
# game with --config: the settings contract). A path no rule below names counts as game, so a new kind of file runs
# everything until it gets a rule (a too-narrow filter that skips a needed test is worse than a broad one).
set -euo pipefail

# The area of one path: all, game, port, launcher, or none (no CI step reads it).
area_of() {
    case "$1" in
        # CI itself (the workflow, its jobs' setup action), and this mapping: everything.
        .github/workflows/ci.yml|.github/actions/*|scripts/ci_areas.sh) echo all ;;
        # Documentation (ci.yml's paths-ignore too): no step reads it.
        docs/*|*.md|LICENSE|.gitignore|.github/ISSUE_TEMPLATE/*) echo none ;;
        # The release's own files: release.yml tests them (a manual run); here only `bash -n`, which always runs.
        .github/workflows/release.yml|scripts/package_appimage.sh|packaging/*) echo none ;;
        # The launcher's own tree.
        launcher/*) echo launcher ;;
        # The port: its sources (port/src/json.c, sha1.c are the launcher's too: port => launcher), its generator, the
        # MCP server that drives it (tools/mcp, .mcp.json; tests/port/debug.py runs its self-test), and the tests that
        # build or run its code.
        port/*|tools/port_gen.py|tools/mcp/*|.mcp.json|tests/port/*|tests/spu/*|tests/xa/*|tests/saves/*|tests/host/*) echo port ;;
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
    -h|--help) sed -n '2,23p' "$0"; exit 0 ;;
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
