#!/usr/bin/env bash
# Extract the disc into extracted/disc/ and write the mkpsxiso layout to
# extracted/dw2003.xml (paths relative to the XML, so the tree is relocatable).
#
# -pt walks the ISO path table: DW3's directory records are lightly obfuscated, and
# a plain directory walk finds only 3 of the 2385 files.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DISC="$ROOT/iso/dw2003.bin"
OUT="$ROOT/extracted"

[[ -e "$DISC" ]] || { echo "missing $DISC; run scripts/setup.sh disc" >&2; exit 1; }

rm -rf "$OUT/disc" "$OUT/dw2003.xml"
mkdir -p "$OUT"
"$ROOT/tools/mkpsxiso/bin/dumpsxiso" -q -pt -x "$OUT/disc" -s "$OUT/dw2003.xml" "$DISC"

# Make source paths relative to the XML's directory.
sed -i "s|\"$OUT/|\"|g" "$OUT/dw2003.xml"

echo "extracted $(find "$OUT/disc" -type f | wc -l) files to extracted/disc, layout in extracted/dw2003.xml"
