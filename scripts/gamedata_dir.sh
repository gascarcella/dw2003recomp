#!/usr/bin/env bash
# Prints the data checkout: a directory holding `gamedata/` (the game's files for building and testing) and
# `tools/prebuilt/` (pinned binaries that can't be downloaded everywhere). It is optional and never part of this
# repository: the public setup path is your own disc (tools/local.env DW3_DISC_BIN). Lookup order:
#   1. $DW3_GAMEDATA (environment)   2. DW3_GAMEDATA in tools/local.env   3. this checkout, if it has gamedata/
#   4. the sibling directory ../dw2003-gamedata (how cloud sessions and CI lay the two checkouts side by side)
# Prints nothing (exit 1) when none exists. Usage: gd="$(scripts/gamedata_dir.sh)" || gd=
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
dir="${DW3_GAMEDATA:-}"
if [[ -z "$dir" && -f "$ROOT/tools/local.env" ]]; then
    dir="$(sed -n 's/^DW3_GAMEDATA=["'"'"']\{0,1\}\([^"'"'"']*\).*/\1/p' "$ROOT/tools/local.env" | head -1)"
fi
[[ -n "$dir" || ! -d "$ROOT/gamedata" ]] || dir="$ROOT"
[[ -n "$dir" || ! -d "$ROOT/../dw2003-gamedata/gamedata" ]] || dir="$(cd "$ROOT/../dw2003-gamedata" && pwd)"
[[ -n "$dir" && -d "$dir" ]] || exit 1
echo "$dir"
