#!/usr/bin/env bash
# Prepare a fresh git worktree. A checkout only has tracked files, so the gitignored ones
# (built tools, tools/local.env, the disc link, extracted/) are missing. This script:
#   1. symlinks the main checkout's built tools and local.env into this worktree (setup.sh link)
#   2. links the disc (setup.sh disc) and extracts it (extract.sh); with a data checkout
#      (scripts/gamedata_dir.sh) the disc comes from there instead (setup.sh gamedata)
# If the main checkout already has extracted/, a worktree links its iso/ and extracted/ instead.
# Idempotent, ~1 s. To install a NEW tool from a worktree, run scripts/setup.sh <step>:
# it installs into the main checkout's tools/ and links it here.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MAIN="$(dirname "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir)")"
# A worktree of a checkout that already has the disc extracted shares it (read-only inputs).
if [[ "$MAIN" != "$ROOT" && -f "$MAIN/extracted/disc/SLES_039.36" && ! -e "$ROOT/extracted" ]]; then
    "$ROOT/scripts/setup.sh" link
    ln -s "$MAIN/iso" "$ROOT/iso"
    ln -s "$MAIN/extracted" "$ROOT/extracted"
    echo "iso/, extracted/ -> $MAIN (shared with the main checkout)"
    echo "worktree ready: $ROOT"
    exit 0
fi
"$ROOT/scripts/setup.sh" link gamedata disc
if [[ -f "$ROOT/extracted/disc/SLES_039.36" ]]; then
    # Already extracted (re-extracting would pull files from under worktrees that link to it).
    echo "extracted/ already present; rm -rf extracted and rerun to re-extract"
elif [[ -e "$ROOT/iso/dw2003.bin" ]]; then
    "$ROOT/scripts/extract.sh"
elif GD="$("$ROOT/scripts/gamedata_dir.sh" 2>/dev/null)" && [[ -f "$GD/gamedata/disc/SLES_039.36" ]]; then
    # A data checkout without the disc image: its EXE + overlays are enough to build.
    mkdir -p "$ROOT/extracted"
    ln -sfn "$GD/gamedata/disc" "$ROOT/extracted/disc"
    echo "extracted/disc -> $GD/gamedata/disc (no disc image; EXE and overlays only)"
else
    echo "no disc: set DW3_DISC_BIN (or DW3_GAMEDATA) in tools/local.env (see local.env.example)" >&2
    exit 1
fi
echo "worktree ready: $ROOT"
