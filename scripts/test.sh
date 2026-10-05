#!/usr/bin/env bash
# Runs every reference-test layer that is available here (tests/README.md) and exits non-zero on the first failure.
# Headless; DW3_JOBS=1 by default (cloud sessions, shared machines).
# Usage: scripts/test.sh [--build] [--layer 1|2|3]...
#   --build     run scripts/build.sh first (the matching build must stay byte-identical; tests never change it)
#   --layer N   run only that layer (repeatable)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export DW3_JOBS="${DW3_JOBS:-1}"
BUILD=0
LAYERS=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build) BUILD=1; shift ;;
        --layer) LAYERS="$LAYERS $2"; shift 2 ;;
        -h|--help) sed -n '2,6p' "$0"; exit 0 ;;
        *) echo "unknown argument: $1" >&2; exit 2 ;;
    esac
done
[[ -z "$LAYERS" ]] && LAYERS="1 2 3"
PY="$ROOT/tools/venv/bin/python"
[[ -x "$PY" ]] || PY=python3

ran=0
skipped=0
layer() { echo; echo "=== layer $1: $2"; }
skip() { echo "skipped: $1"; skipped=$((skipped + 1)); }

if [[ $BUILD -eq 1 ]]; then
    echo "=== build (must stay byte-identical)"
    "$ROOT/scripts/build.sh"
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
        fi ;;
    3)
        layer 3 "formats and save round trips (tests/formats)"
        if [[ -x "$ROOT/tests/formats/run.sh" ]]; then
            "$ROOT/tests/formats/run.sh"; ran=$((ran + 1))
        else
            skip "tests/formats/run.sh does not exist yet (STATUS: Next)"
        fi ;;
    *) echo "unknown layer $L" >&2; exit 2 ;;
    esac
done
echo
echo "test.sh: $ran layer(s) passed, $skipped skipped"
[[ $ran -gt 0 ]] || { echo "test.sh: nothing ran" >&2; exit 1; }
