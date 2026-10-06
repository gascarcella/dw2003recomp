#!/usr/bin/env bash
# Layer 3 (tests/README.md): the file formats, checked the way docs/FORMATS.md describes them, over every file of the
# disc. Needs extracted/ and iso/ (scripts/extract.sh or scripts/worktree_init.sh); nothing from a configured tree
# (asm/, build/). Each check prints one summary line with its counts; the run stops non-zero at the first failure.
#   file table: tools/disc_files.py --check: the EXE's 2,382 (LBA, sectors) entries against the ISO directories: every
#         AAA/ file exactly once, sectors = size rounded up, zero bytes after each file's end in its last sector (cdload
#         loads whole sectors), the 350 text files at JPN ID + records_language.
#   text: tools/dump_text.py --check decodes every text file of every language (91,059 entries in 3,005 tables) and
#         fails on an unknown byte, a file that is not a table, or a table whose entry 0 is not the empty string.
#   WSTAG stage table: tools/overlay_layout.py --wstag-table: every WSTAG### overlay has a record in FIELDSTG's stage
#         tables whose entry is a function start inside the overlay's .text; the table equals config/wstag.txt.
#   flag ranges: tools/flag_census.py --check (reads src/): every flag of a bit-array type (gamestate save data) that
#         the game's data or code reads or sets has an index inside its array.
#   sound banks: tests/sound/sound_formats.py --check: the 71 banks' VAB headers, VAB bodies (SPU ADPCM) and SEPs parse
#         as docs/FORMATS.md "Sound" describes them (sizes, tone records, block flags, 16 sequences per SEP).
# Save round trips (.mcd files exchanged with the port) are added here once the port writes saves.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PY="$ROOT/tools/venv/bin/python"
[[ -x "$PY" ]] || PY=python3
for need in extracted/disc iso/dw2003.bin; do
    if [[ ! -e "$ROOT/$need" ]]; then
        echo "formats: no $need (scripts/worktree_init.sh)" >&2
        exit 2
    fi
done

echo "--- file table: IDs against the ISO directories (tools/disc_files.py --check)"
"$PY" "$ROOT/tools/disc_files.py" --check

echo "--- text files: decode every one (tools/dump_text.py --check)"
"$PY" "$ROOT/tools/dump_text.py" --check

echo "--- WSTAG stage table: FIELDSTG's records against the overlays (tools/overlay_layout.py --wstag-table)"
table="$("$PY" "$ROOT/tools/overlay_layout.py" --wstag-table)"
if ! diff <(printf '%s\n' "$table") "$ROOT/config/wstag.txt" >/dev/null; then
    diff <(printf '%s\n' "$table") "$ROOT/config/wstag.txt" | head -20 || true
    echo "WSTAG stage table differs from config/wstag.txt" >&2
    exit 1
fi
echo "$(grep -vc '^#' <<<"$table") WSTAG overlays, each entry a function start in its .text, table == config/wstag.txt"

echo "--- flag ranges: bit-array flags inside their arrays (tools/flag_census.py --check)"
"$PY" "$ROOT/tools/flag_census.py" --check

echo "--- sound banks: VAB headers and bodies, SEPs (tests/sound/sound_formats.py --check)"
"$PY" "$ROOT/tests/sound/sound_formats.py" --check

echo "formats: ok"
