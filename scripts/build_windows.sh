#!/usr/bin/env bash
# Cross-builds the PC port and the launcher for Windows x86_64 from Linux (cmake/windows-x86_64.cmake: llvm-mingw's
# clang and lld, SDL3 for Windows; DECISIONS "Windows: cross-built from Linux").
#
# Usage: scripts/build_windows.sh [--launcher] [--configure-only] [--test] [--jobs N]
#   (default)         the game into build/port-win (the SDL build, Release): every object (ninja -k 0), then the link
#   --launcher        also the launcher into build/launcher-win (dw2003-launcher.exe and its PDB)
#   --configure-only  stop after configuring
#   --test            after the launcher: its --self-test under Wine (wine on PATH; SDL's offscreen video) in
#                     build/launcher-win/selftest/, the prefix in build/wine-prefix/; the game's crash report under
#                     Wine (tests/port/crash.py --wine); and when the disc is there (iso/dw2003.cue) the game's
#                     layer-2 replays under Wine (tests/port/run.py --exe ... --wine: the log and the record must
#                     equal the Linux build's). The same gate as CI's windows job; each Wine run under `timeout`.
# Needs: scripts/setup.sh llvm-mingw sdl3-windows (and imgui for the launcher); cmake and ninja (tools/venv's when the
# system has none). Exit 0 when everything asked for built (and the test passed), else 1 with a summary: which objects
# did not compile and whether the link failed. The game links since the overlay sections stopped needing a linker
# script (DECISIONS "Overlay sections by renaming"); tests/port/run.py --exe build/port-win/dw2003.exe --wine replays
# the layer-2 scripts with it under Wine.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MAIN="$(dirname "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir)")"
JOBS="${DW3_JOBS:-$(nproc)}"
log() { printf '\033[1;34m[windows]\033[0m %s\n' "$*"; }
die() { printf '\033[1;31m[windows]\033[0m %s\n' "$*" >&2; exit 1; }

launcher=0 configure_only=0 test=0
while [[ $# -gt 0 ]]; do
    case "$1" in
        --launcher) launcher=1; shift ;;
        --configure-only) configure_only=1; shift ;;
        --test) test=1; launcher=1; shift ;;
        --jobs) JOBS="$2"; shift 2 ;;
        -h|--help) sed -n '2,17p' "$0"; exit 0 ;;
        *) die "unknown argument: $1" ;;
    esac
done

# A built tool: this checkout's tools/<name>, else the main checkout's (scripts/setup.sh installs there).
tool() {
    if [[ -e "$ROOT/tools/$1" ]]; then echo "$ROOT/tools/$1"; else echo "$MAIN/tools/$1"; fi
}
llvm="$(tool llvm-mingw)"
[[ -x "$llvm/bin/x86_64-w64-mingw32-clang" ]] || die "no llvm-mingw at $llvm: scripts/setup.sh llvm-mingw"
sdl="$(tool sdl3-windows)"
[[ -f "$sdl/lib/cmake/SDL3/SDL3Config.cmake" ]] || die "no SDL3 for Windows at $sdl: scripts/setup.sh sdl3-windows"
PATH="$PATH:$(tool venv)/bin"
for t in cmake ninja; do
    command -v "$t" >/dev/null || die "no $t on PATH (scripts/setup.sh cmake)"
done
toolchain="$ROOT/cmake/windows-x86_64.cmake"
status=0
mkdir -p "$ROOT/build"   # a fresh checkout has none: the logs below go beside the build directories

# ---- The game.
port_dir="$ROOT/build/port-win"
log "game: configuring $port_dir"
cmake -S "$ROOT/port" -B "$port_dir" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$toolchain" -DCMAKE_BUILD_TYPE=Release \
    -DDW3_PORT_SDL=ON >"$port_dir.configure.log" 2>&1 || die "game: configure failed (see $port_dir.configure.log)"
if [[ $configure_only -eq 0 ]]; then
    log "game: building every object with $JOBS jobs (ninja -k 0; log: $port_dir.build.log)"
    if ninja -C "$port_dir" -k 0 -j"$JOBS" >"$port_dir.build.log" 2>&1; then
        log "game: $port_dir/dw2003.exe linked"
    else
        status=1
        failed="$(grep '^FAILED: ' "$port_dir.build.log" | sed 's/^FAILED: //; s/^\[code=[0-9]*\] //; s/ *$//' || true)"
        units_failed="$(grep -c 'dw3_game.dir' <<<"$failed" || true)"
        runtime_failed="$(grep 'dw2003.dir' <<<"$failed" | sed 's|.*dw2003.dir/||; s|\.obj$||' | tr '\n' ' ' || true)"
        objs="$(find "$port_dir/CMakeFiles" -name '*.obj' | wc -l)"
        units="$(find "$port_dir/CMakeFiles/dw3_game.dir" -name '*.obj' 2>/dev/null | wc -l)"
        log "game: did not link. $units of 388 units compiled ($units_failed failed); $objs objects in all"
        [[ -z "$runtime_failed" ]] || log "game: runtime and shim files that do not compile: $runtime_failed"
        if grep -q '^FAILED: dw2003.exe' "$port_dir.build.log"; then
            log "game: the link itself failed"
        fi
        log "game: the errors by kind:"
        grep -oE 'error: [^[]*' "$port_dir.build.log" | sed -E 's/ \(aka[^)]*\)//; s/ +$//' | sort | uniq -c | sort -rn |
            head -12 | sed 's/^/    /'
    fi
fi

# ---- The launcher.
if [[ $launcher -eq 1 ]]; then
    imgui="$(tool imgui)"
    [[ -f "$imgui/imgui.cpp" ]] || die "no Dear ImGui: scripts/setup.sh imgui"
    launcher_dir="$ROOT/build/launcher-win"
    log "launcher: configuring $launcher_dir"
    cmake -S "$ROOT/launcher" -B "$launcher_dir" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
        -DCMAKE_BUILD_TYPE=Release -DDW3_LAUNCHER_STATIC_RUNTIME=ON -DDW3_IMGUI_DIR="$imgui" \
        >"$launcher_dir.configure.log" 2>&1 || die "launcher: configure failed (see $launcher_dir.configure.log)"
    if [[ $configure_only -eq 0 ]]; then
        log "launcher: building"
        if ninja -C "$launcher_dir" -j"$JOBS" >"$launcher_dir.build.log" 2>&1; then
            log "launcher: $launcher_dir/dw2003-launcher.exe linked ($(du -h "$launcher_dir/dw2003-launcher.exe" | cut -f1), PDB beside it)"
        else
            status=1
            log "launcher: build failed (see $launcher_dir.build.log)"
            test=0
        fi
    fi
    if [[ $test -eq 1 && $configure_only -eq 0 ]]; then
        command -v wine >/dev/null || die "no wine on PATH for --test"
        selftest="$launcher_dir/selftest"
        rm -rf "$selftest"
        mkdir -p "$selftest" "$ROOT/build/wine-prefix"
        log "launcher: --self-test under wine ($(wine --version 2>/dev/null); log: $selftest.log)"
        # Wine sees the Unix tree as drive Z:; forward slashes are fine for the Windows API.
        # Wine drops SDL_VIDEO_DRIVER from the Windows environment (Proton, wine-staging): the self-test takes the
        # driver from DW3_SELFTEST_VIDEO_DRIVER (SDL's hint). `timeout`: a hang (a window waiting for a display) fails
        # instead of holding the runner.
        if WINEPREFIX="$ROOT/build/wine-prefix" WINEDEBUG=-all DW3_SELFTEST_VIDEO_DRIVER=offscreen \
            SDL_VIDEO_DRIVER=offscreen SDL_AUDIO_DRIVER=dummy \
            timeout 600 wine "$launcher_dir/dw2003-launcher.exe" --self-test "Z:$selftest" >"$selftest.log" 2>&1; then
            log "launcher: self-test passed under wine: $(grep -o '[0-9]* of [0-9]* checks passed' "$selftest.log" || true)"
        else
            status=1
            log "launcher: self-test FAILED under wine: $(grep -o '[0-9]* of [0-9]* checks passed' "$selftest.log" || true)"
            grep 'FAILED' "$selftest.log" | sed 's/^/    /' | head -20
        fi
    fi
fi

# ---- The game's crash report under Wine (no disc needed: tests/port/crash.py --wine).
if [[ $test -eq 1 && $configure_only -eq 0 && -x "$port_dir/dw2003.exe" ]]; then
    log "game: the crash report under wine (tests/port/crash.py --wine)"
    if timeout 600 "$(tool venv)/bin/python" "$ROOT/tests/port/crash.py" --wine --exe "$port_dir/dw2003.exe"; then
        log "game: the crash report under wine passed"
    else
        status=1
        log "game: the crash report under wine FAILED"
    fi
fi

# ---- The game's replays under Wine (the disc needed).
if [[ $test -eq 1 && $configure_only -eq 0 && -x "$port_dir/dw2003.exe" ]]; then
    if [[ -f "$ROOT/iso/dw2003.cue" ]]; then
        log "game: the layer-2 replays under wine (tests/port/run.py --exe $port_dir/dw2003.exe --wine)"
        if timeout 1800 "$(tool venv)/bin/python" "$ROOT/tests/port/run.py" --exe "$port_dir/dw2003.exe" --wine -j "$JOBS"; then
            log "game: the replays under wine match the Linux build's"
        else
            status=1
            log "game: the replays under wine FAILED"
        fi
    else
        log "game: no disc (iso/dw2003.cue): the replays under wine skipped"
    fi
fi

if [[ $status -eq 0 ]]; then
    log "done"
else
    log "done with failures (see above)"
fi
exit $status
