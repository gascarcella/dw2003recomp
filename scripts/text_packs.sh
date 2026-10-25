#!/usr/bin/env bash
# Builds the translations' text packs into a package's mods/ (docs/PORT.md "Text packs"; issue #100), so that a release
# carries them: each translation the port knows becomes <MODS_DIR>/preset_language/<lang>.pack, built by
# tools/text_tools.py from its work files and the disc (iso/, extracted/). The work files are the owner's, in the data
# checkout's translations/<lang>/ (scripts/gamedata_dir.sh), never in this repository (DECISIONS "Code only");
# $DW3_TRANSLATIONS names another translations/ directory (release_local.sh's container mounts it there).
#
# Usage: scripts/text_packs.sh MODS_DIR
# Fails when a translation is missing or does not build with 0 errors, so that a package never ships without one by
# accident; the packages' --no-text-packs is the way to build one without them (no data checkout).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MAIN="$(dirname "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir)")"
log() { printf '\033[1;34m[text-packs]\033[0m %s\n' "$*"; }
die() { printf '\033[1;31m[text-packs]\033[0m %s\n' "$*" >&2; exit 1; }

# The languages preset_language opens a pack for (port/game/game_mods.c: portuguese_br -> pt_br.pack).
LANGS=(pt_br)

[[ $# -eq 1 ]] || die "usage: scripts/text_packs.sh MODS_DIR"
mods="$1"
[[ -f "$mods/preset_language/mod.json" ]] || die "$mods is not a build's mods/ (no preset_language/mod.json)"
python="$ROOT/tools/venv/bin/python"
[[ -x "$python" ]] || python="$MAIN/tools/venv/bin/python"
[[ -x "$python" ]] || die "no tools/venv/bin/python: scripts/setup.sh venv"

dir="${DW3_TRANSLATIONS:-}"
if [[ -z "$dir" ]]; then
    gd="$("$ROOT/scripts/gamedata_dir.sh")" ||
        die "no data checkout (scripts/gamedata_dir.sh) for the translations: --no-text-packs packages without them"
    dir="$gd/translations"
fi
for lang in "${LANGS[@]}"; do
    src="$dir/$lang"
    [[ -d "$src" ]] || die "no translation $src: --no-text-packs packages without them"
    pack="$mods/preset_language/$lang.pack"
    out="$("$python" "$ROOT/tools/text_tools.py" build "$src" "$pack" 2>&1)" ||
        { printf '%s\n' "$out" | tail -n 20 >&2; die "$lang: the pack did not build"; }
    # read back as the port's writer wrote it (the format's checks: tools/text_pack.py --dump)
    files="$("$python" "$ROOT/tools/text_pack.py" --dump "$pack" | grep -c '^0x')" || die "$lang: $pack does not read back"
    log "$lang: $(tail -n 1 <<<"$out" | sed "s#^$pack: ##") in $files text files -> $pack"
done
