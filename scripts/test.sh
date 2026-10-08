#!/usr/bin/env bash
# Runs the workaround census (tools/hacks.py --check) and the host-compile probe, then every reference-test layer that
# is available here (tests/README.md), and exits non-zero on the first failure. Headless; DW3_JOBS=1 by default (cloud
# sessions, shared machines): the layers that can run their pieces at once (layer 2's replays, the mods) do so with
# DW3_JOBS > 1 (CI: 4, the runner's cores).
# Usage: scripts/test.sh [--build] [--layer 1|2|3|port|mods]... [--m32] [--no-probe]
#   --build     run scripts/build.sh first (the matching build must stay byte-identical; tests never change it)
#   --layer N   run only that layer (repeatable); the census and the probe still run. "port": the PC port's M1 test
#               and its checks (tests/port); "mods": the mods that change the game (tests/port/mods.py, the longest:
#               CI's own job)
#   --m32       the port layer's M1 test also on the -m32 build (tests/port/run.py --m32; needs gcc-multilib)
#   --no-probe  skip the host-compile probe (tools/port_inventory.py probe + link: every unit compiles at -m64 with no
#               pointer/int cast, implicit declaration or incompatible pointer type, and no global is defined twice)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export DW3_JOBS="${DW3_JOBS:-1}"
BUILD=0
PROBE=1
LAYERS=""
M32=()
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build) BUILD=1; shift ;;
        --layer) LAYERS="$LAYERS $2"; shift 2 ;;
        --m32) M32=(--m32); shift ;;
        --no-probe) PROBE=0; shift ;;
        -h|--help) sed -n '2,13p' "$0"; exit 0 ;;
        *) echo "unknown argument: $1" >&2; exit 2 ;;
    esac
done
[[ -z "$LAYERS" ]] && LAYERS="1 2 3 port mods"
PY="$ROOT/tools/venv/bin/python"
[[ -x "$PY" ]] || PY=python3

ran=0
skipped=0
probe="not run"
layer() { echo; echo "=== layer $1: $2"; }
skip() { echo "skipped: $1"; skipped=$((skipped + 1)); }

if [[ $BUILD -eq 1 ]]; then
    echo "=== build (must stay byte-identical)"
    "$ROOT/scripts/build.sh"
fi

# The workaround census (docs/MATCHING.md "The census: tools/hacks.py"): stdlib only, <1 s, always. The game code is all
# C, every FAKE: and LOOP_BLOCK carries its comment, and docs/STATUS.md quotes the counts.
echo "=== census: the matching workarounds (tools/hacks.py --check)"
"$PY" "$ROOT/tools/hacks.py" --check

# The host-compile gate (docs/PORT.md "Compiling the game C for the host"): needs only the host gcc and nm (the probe writes its own
# INCLUDE_ASM/gte override headers under build/port_inventory/; include/asm_generated/ is not needed), a few seconds.
if [[ $PROBE -eq 1 ]]; then
    echo "=== probe: every unit host-clean at -m64 (tools/port_inventory.py probe, link)"
    if command -v gcc >/dev/null && command -v nm >/dev/null; then
        "$PY" "$ROOT/tools/port_inventory.py" probe -j "$DW3_JOBS"
        mkdir -p "$ROOT/build/port_inventory"
        if ! "$PY" "$ROOT/tools/port_inventory.py" link -j "$DW3_JOBS" > "$ROOT/build/port_inventory/link.txt"; then
            cat "$ROOT/build/port_inventory/link.txt"; exit 1
        fi
        tail -n 1 "$ROOT/build/port_inventory/link.txt"    # the whole report: build/port_inventory/link.txt
        probe="passed"
    else
        skip "no gcc/nm for the host-compile probe (tools/port_inventory.py)"
        probe="skipped"
    fi
fi

for L in $LAYERS; do
    case "$L" in
    1)
        layer 1 "golden tests of the pure logic (tests/golden)"
        if [[ -x "$ROOT/tests/golden/run.sh" ]]; then
            "$ROOT/tests/golden/run.sh"; ran=$((ran + 1))
        else
            skip "tests/golden/run.sh does not exist yet (STATUS: Next)"
        fi
        if command -v gcc >/dev/null; then
            echo "--- host-side replay of the goldens (tests/host)"
            "$PY" "$ROOT/tests/host/replay.py"; ran=$((ran + 1))
            echo "--- the SPU core's unit goldens (tests/spu, ~2 s)"
            "$ROOT/tests/spu/run.sh"; ran=$((ran + 1))
            echo "--- the XA decoder's unit goldens (tests/xa, ~1 s)"
            "$ROOT/tests/xa/run.sh"; ran=$((ran + 1))
        else
            skip "no gcc for the host-side replay (tests/host)"
        fi ;;
    2)
        layer 2 "record/replay in the emulator (tests/replay)"
        if [[ ! -x "$ROOT/tools/redux/pcsx-redux" ]]; then
            skip "no emulator (scripts/setup.sh redux)"
        elif [[ ! -f "$ROOT/iso/dw2003.cue" ]]; then
            skip "no disc image (iso/dw2003.cue; scripts/setup.sh disc or gamedata)"
        else
            "$PY" "$ROOT/tests/replay/replay.py" check; ran=$((ran + 1))
            echo "--- SPU write trace of the boot and CNTY_SEL's music (tests/sound, ~25 s)"
            "$PY" "$ROOT/tests/sound/spu_trace.py" check
        fi ;;
    3)
        layer 3 "formats and save round trips (tests/formats)"
        if [[ -x "$ROOT/tests/formats/run.sh" ]]; then
            "$ROOT/tests/formats/run.sh"; ran=$((ran + 1))
        else
            skip "tests/formats/run.sh does not exist yet (STATUS: Next)"
        fi
        echo "--- save round trips: port -> emulator, emulator -> port, the two cards (tests/saves/run.py, ~1 min)"
        rc=0; "$PY" "$ROOT/tests/saves/run.py" || rc=$?
        if [[ $rc -eq 2 ]]; then skip "save round trips: needs the emulator, the disc, cmake/ninja and gcc (above)"
        elif [[ $rc -ne 0 ]]; then exit 1
        else ran=$((ran + 1)); fi ;;
    port|mods)
        if [[ "$L" == port ]]; then
            layer port "the PC port replays the layer-2 scripts like the emulator (tests/port)"
        else
            layer mods "the mods that change the game, run with the mod on (tests/port/mods.py)"
        fi
        if ! { command -v cmake || [[ -x "$ROOT/tools/venv/bin/cmake" ]]; } >/dev/null; then
            skip "no cmake (on PATH or in tools/venv: scripts/setup.sh cmake)"
        elif ! { command -v ninja || [[ -x "$ROOT/tools/venv/bin/ninja" ]]; } >/dev/null; then
            skip "no ninja (on PATH or in tools/venv: scripts/setup.sh cmake)"
        elif ! command -v gcc >/dev/null; then
            skip "no gcc for the port"
        elif [[ ! -f "$ROOT/iso/dw2003.cue" ]]; then
            skip "no disc image (iso/dw2003.cue; scripts/setup.sh disc or gamedata)"
        elif [[ "$L" == mods ]]; then
            "$PY" "$ROOT/tests/port/mods.py"; ran=$((ran + 1))   # DW3_JOBS mods at once, each its own process
        else
            "$PY" "$ROOT/tests/port/run.py" "${M32[@]}"; ran=$((ran + 1))   # --m32: the -m32 build's log and record too
            "$PY" "$ROOT/tests/port/settings.py"   # --config, the launcher's contract (docs/LAUNCHER.md "Settings file")
            "$PY" "$ROOT/tests/port/render_gpu.py" # the hardware renderer (pictures only with a GPU device)
            "$PY" "$ROOT/tests/host/gpu_hw_replay.py"   # its rasteriser on the gpu goldens (only with a GPU device)
            "$PY" "$ROOT/tests/port/hz60.py"       # the 60 Hz mode against the patched game's records
            "$PY" "$ROOT/tests/port/battle.py"     # the battle scripts on the disc, for battle_animations
            "$PY" "$ROOT/tests/port/vram.py"       # the first battle's textures in VRAM against the emulator's
            "$PY" "$ROOT/tests/port/subpixel_jitter.py" run   # the GTE shadow in the first battle (sub-pixel precision, ~40 s)
            "$PY" "$ROOT/tests/port/debug.py"      # the debug channel (--debug) and tools/mcp's offline self-test
            "$PY" "$ROOT/tests/port/savestate.py" "${M32[@]}"   # save states: resumed at battle_start, the same run
            "$PY" "$ROOT/tests/port/crash.py"      # the crash report (DW3_PORT_CRASH_AT, a fatal error, --version)
        fi ;;
    *) echo "unknown layer $L" >&2; exit 2 ;;
    esac
done
echo
echo "test.sh: probe $probe, $ran layer(s) passed, $skipped skipped"
[[ $ran -gt 0 ]] || { echo "test.sh: no layer ran" >&2; exit 1; }
