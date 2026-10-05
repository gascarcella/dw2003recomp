#!/usr/bin/env bash
# Compile one C file with Sony's real Psy-Q chain (CC1PSX.EXE + ASPSX.EXE through wibo; scripts/setup.sh psyq) and
# with ours (old-gcc cc1 + maspsx + GNU as), then compare one function's instructions. Relocation addends are
# masked (psyq-obj-parser and GNU as store them differently). DECISIONS "Psy-Q check".
#
# Usage: tools/psyq_compare.sh [-v 4.3|4.4] [-G N] [-D DEF ...] src/<t>/<unit>.c <function>
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
T="$ROOT/tools"; PQ="$T/psyq"
ver=4.4 g=0 defs=()
while [[ $# -gt 2 ]]; do
    case "$1" in
        -v) ver="$2"; shift 2 ;;
        -G) g="$2"; shift 2 ;;
        -D) defs+=("-D$2"); shift 2 ;;
        *) echo "unknown option $1" >&2; exit 2 ;;
    esac
done
[[ $# -eq 2 ]] || { sed -n '2,7p' "$0"; exit 2; }
src="$1" func="$2"
[[ -x "$PQ/wibo" ]] || { echo "Psy-Q missing: run scripts/setup.sh psyq" >&2; exit 1; }
tmp="$(mktemp -d)"; trap 'rm -rf "$tmp"' EXIT

"$T/gcc/2.8.1/cpp" -nostdinc -lang-c -D_LANGUAGE_C -D__GNUC__=2 "${defs[@]}" -I"$ROOT/include" \
    -I"$ROOT/include/asm_generated" -I"$ROOT" "$src" "$tmp/in.i"
# INCLUDE_ASM bodies are other functions: drop them (both chains see the same input).
"$PQ/wibo" "$PQ/psyq$ver/CC1PSX.EXE" -quiet -O2 -G"$g" "$tmp/in.i" -o "$tmp/sony.s" </dev/null
grep -v '\.include' "$tmp/sony.s" > "$tmp/sony2.s"
(cd "$tmp" && "$PQ/wibo" "$PQ/psyq$ver/ASPSX.EXE" -quiet sony2.s -o sony.obj </dev/null >/dev/null)
"$PQ/psyq-obj-parser" "$tmp/sony.obj" -o "$tmp/sony.o" >/dev/null

"$T/gcc/2.8.1/cc1" -quiet -O2 -mips1 -mcpu=3000 -mgas -msoft-float -G"$g" "$tmp/in.i" -o "$tmp/ours.s"
grep -v '\.include' "$tmp/ours.s" | "$T/venv/bin/python" "$T/ext/maspsx/maspsx.py" --aspsx-version=2.80 -G"$g" \
    > "$tmp/ours.m.s"
"$T/binutils/bin/mipsel-linux-gnu-as" -EL -march=r3000 -mtune=r3000 -no-pad-sections -G0 -o "$tmp/ours.o" "$tmp/ours.m.s"

dis() {
    "$T/binutils/bin/mipsel-linux-gnu-objdump" -d -m mips:3000 "$1" | awk -v f="<$func>:" '$2 == f {on=1; next} on && /^$/ {exit} on && NF > 2 {print $3, $4}' |
        sed -E 's/-?(0x)?[0-9a-f]+\(/(/; s/,-?(0x)?[0-9a-f]+$/,N/'
}
dis "$tmp/sony.o" > "$tmp/sony.dis"; dis "$tmp/ours.o" > "$tmp/ours.dis"
[[ -s "$tmp/sony.dis" ]] || { echo "$func not found in the Psy-Q output" >&2; exit 1; }
if diff -u "$tmp/sony.dis" "$tmp/ours.dis" > "$tmp/diff"; then
    echo "$func: identical ($(wc -l < "$tmp/sony.dis") instructions; Psy-Q $ver vs old-gcc 2.8.1 + maspsx)"
else
    echo "$func: DIFFERENT (Psy-Q $ver = '-', ours = '+')"; cat "$tmp/diff"
fi
