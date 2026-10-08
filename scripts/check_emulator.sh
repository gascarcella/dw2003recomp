#!/usr/bin/env bash
# Smoke test for the emulator installed by `scripts/setup.sh redux`: boots the disc headlessly in PCSX-Redux and
# waits for the EXE to load CNTY_SEL, the country-select overlay (tests/replay/probes.lua's booted(), through
# psxstack's boot check: tests/replay/replay.py boot). ~30 s.
# Usage: scripts/check_emulator.sh [--bios openbios|retail|<file>] [--iso <cue>] [--frames N]
#   --bios openbios   the OpenBIOS shipped with the emulator (default)
#   --bios retail     <data checkout>/gamedata/bios/scph7502.bin (scripts/gamedata_dir.sh), else PSX_BIOS from tools/local.env
#   --iso <cue>       default iso/dw2003.cue (a cue sheet over the raw MODE2/2352 image)
#   --frames N        give up after N vsyncs (default 3000)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIOS=openbios
ISO="$ROOT/iso/dw2003.cue"
FRAMES=3000
while [[ $# -gt 0 ]]; do
    case "$1" in
        --bios) BIOS="$2"; shift 2 ;;
        --iso) ISO="$2"; shift 2 ;;
        --frames) FRAMES="$2"; shift 2 ;;
        -h|--help) sed -n '2,9p' "$0"; exit 0 ;;
        *) echo "unknown argument: $1" >&2; exit 1 ;;
    esac
done
[[ -x "$ROOT/tools/redux/pcsx-redux" ]] || { echo "missing $ROOT/tools/redux/pcsx-redux; run scripts/setup.sh redux" >&2; exit 1; }
[[ -f "$ISO" ]] || { echo "missing $ISO; run scripts/setup.sh gamedata (or disc)" >&2; exit 1; }
if [[ "$BIOS" == retail ]]; then
    if GD="$("$ROOT/scripts/gamedata_dir.sh" 2>/dev/null)" && [[ -f "$GD/gamedata/bios/scph7502.bin" ]]; then
        BIOS="$GD/gamedata/bios/scph7502.bin"
    else
        # shellcheck disable=SC1091
        [[ -f "$ROOT/tools/local.env" ]] && source "$ROOT/tools/local.env"
        BIOS="${PSX_BIOS:-}"
        [[ -f "$BIOS" ]] || { echo "no retail BIOS: set PSX_BIOS in tools/local.env" >&2; exit 1; }
    fi
fi
PY="$ROOT/tools/venv/bin/python"
[[ -x "$PY" ]] || PY=python3
exec "$PY" "$ROOT/tests/replay/replay.py" boot --bios "$BIOS" --iso "$ISO" --frames "$FRAMES"
