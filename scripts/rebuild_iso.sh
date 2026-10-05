#!/usr/bin/env bash
# Rebuild the disc image from extracted/ with mkpsxiso and compare it to the original.
# Usage: scripts/rebuild_iso.sh [xml]   (default: extracted/dw2003.xml)
# Output: build/dw2003.bin + .cue. Exits non-zero if the image differs.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
XML="${1:-$ROOT/extracted/dw2003.xml}"
mkdir -p "$ROOT/build"

# Run from the XML directory so its relative source paths resolve.
(cd "$(dirname "$XML")" && "$ROOT/tools/mkpsxiso/bin/mkpsxiso" -y -q \
    -o "$ROOT/build/dw2003.bin" -c "$ROOT/build/dw2003.cue" "$(basename "$XML")")

if cmp -s "$ROOT/build/dw2003.bin" "$ROOT/iso/dw2003.bin"; then
    echo "OK: build/dw2003.bin is bit-identical to iso/dw2003.bin"
else
    echo "DIFF: build/dw2003.bin differs from iso/dw2003.bin ($(cmp -l "$ROOT/build/dw2003.bin" "$ROOT/iso/dw2003.bin" 2>/dev/null | wc -l) bytes)" >&2
    exit 1
fi
