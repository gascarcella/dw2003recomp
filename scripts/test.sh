#!/usr/bin/env bash
# Runs the host-compile probe, then every reference-test layer that is available here (tests/README.md), and exits
# non-zero on the first failure. Headless; DW3_JOBS=1 by default (cloud sessions, shared machines).
# Usage: scripts/test.sh [--build] [--layer 1|2|3|port]... [--no-probe]
#   --build     run scripts/build.sh first (the matching build must stay byte-identical; tests never change it)
#   --layer N   run only that layer (repeatable); the probe still runs. "port": the PC port's M1 test (tests/port)
#   --no-probe  skip the host-compile probe (tools/port_inventory.py probe + link: every unit compiles at -m64 with no
#               pointer/int cast, implicit declaration or incompatible pointer type, and no global is defined twice)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export DW3_JOBS="${DW3_JOBS:-1}"
BUILD=0
PROBE=1
LAYERS=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build) BUILD=1; shift ;;
        --layer) LAYERS="$LAYERS $2"; shift 2 ;;
        --no-probe) PROBE=0; shift ;;
        -h|--help) sed -n '2,8p' "$0"; exit 0 ;;
        *) echo "unknown argument: $1" >&2; exit 2 ;;
    esac
done
[[ -z "$LAYERS" ]] && LAYERS="1 2 3 port"
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

# The host-compile gate (docs/PC_PORT_PLAN.md section 1): needs only the host gcc and nm (the probe writes its own
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
    port)
        layer port "the PC port replays the layer-2 scripts like the emulator (tests/port)"
        if ! { command -v cmake || [[ -x "$ROOT/tools/venv/bin/cmake" ]]; } >/dev/null; then
            skip "no cmake (on PATH or in tools/venv: scripts/setup.sh cmake)"
        elif ! { command -v ninja || [[ -x "$ROOT/tools/venv/bin/ninja" ]]; } >/dev/null; then
            skip "no ninja (on PATH or in tools/venv: scripts/setup.sh cmake)"
        elif ! command -v gcc >/dev/null; then
            skip "no gcc for the port"
        elif [[ ! -f "$ROOT/iso/dw2003.cue" ]]; then
            skip "no disc image (iso/dw2003.cue; scripts/setup.sh disc or gamedata)"
        else
            "$PY" "$ROOT/tests/port/run.py"; ran=$((ran + 1))
            "$PY" "$ROOT/tests/port/settings.py"   # --config, the launcher's contract (docs/LAUNCHER_MODS_PLAN.md 4.3)
        fi ;;
    *) echo "unknown layer $L" >&2; exit 2 ;;
    esac
done
echo
echo "test.sh: probe $probe, $ran layer(s) passed, $skipped skipped"
[[ $ran -gt 0 ]] || { echo "test.sh: no layer ran" >&2; exit 1; }
