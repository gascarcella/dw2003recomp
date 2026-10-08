#!/usr/bin/env bash
# port/psyq/check.sh: compiles the Psy-Q shim on its own with the port's flags (-Wall -Wextra -Werror) and checks
# that it covers every Psy-Q symbol the game's host objects need (tools/port_inventory.py's probe objects).
# Needs no CMake or SDL. Run from the repo root after `tools/venv/bin/python tools/port_inventory.py probe`
# (which writes build/port_inventory/include, the override headers: an empty INCLUDE_ASM and no-op GTE macros).
#   port/psyq/check.sh            # compile + coverage
#   port/psyq/check.sh --compile  # compile only
#   EXTRA="-O2 -fsanitize=address,undefined" port/psyq/check.sh --compile   # with more flags
set -euo pipefail
cd "$(dirname "$0")/../.."

CC=${CC:-gcc}
EXTRA=${EXTRA:-}   # extra compiler flags, e.g. EXTRA="-O2 -fsanitize=address,undefined"
OUT=build/port_psyq
CFLAGS=(-m64 -std=gnu99 -fsigned-char -fwrapv -fno-strict-aliasing -DPC_PORT -DNON_MATCHING -Wall -Wextra -Werror
        -Ibuild/port_inventory/include -Iinclude -Iinclude/asm_generated -I. -Iport/psyq -Iport/include -Iport/runtime)

if [ ! -d build/port_inventory/include ]; then
    echo "check: build/port_inventory/include is missing: run tools/venv/bin/python tools/port_inventory.py probe" >&2
    exit 2
fi
mkdir -p "$OUT"
rm -f "$OUT"/*.o
for src in port/psyq/*.c; do
    "$CC" ${EXTRA} "${CFLAGS[@]}" -c "$src" -o "$OUT/$(basename "${src%.c}").o"
done
ar rcs "$OUT/libpsyq.a" "$OUT"/*.o
echo "compile: $(ls "$OUT"/*.o | wc -l) objects -> $OUT/libpsyq.a"
[ "${1:-}" = "--compile" ] && exit 0

# Coverage: the 123 Psy-Q functions (port_inventory.py counts) and the asm-only Psy-Q data (link -v) against what the
# shim defines; LIBC2 and LIBAPI are the host libc (psyq.h), so they are checked against libc's exports instead.
PY=tools/venv/bin/python
if [ ! -d build/port_inventory/m64/obj ]; then
    echo "check: build/port_inventory/m64/obj is missing: run $PY tools/port_inventory.py probe" >&2
    exit 2
fi
game_objs=$(find build/port_inventory/m64/obj -name '*.o')
need=$($PY tools/port_inventory.py link -v 2>/dev/null | awk '/Psy-Q function:/{f=1;next} /asm-only data/{f=0} f && $2 ~ /^0x/ {print $1}' | sort -u)
need_data="D_800812F8 D_80081358 D_80081454"
have=$(nm -g --defined-only "$OUT"/*.o | awk '$2 ~ /^[TDBR]$/ {print $3}' | sort -u)
libc=$(nm -D --defined-only "$(gcc -print-file-name=libc.so.6)" | awk '{print $3}' | sed 's/@.*//' | sort -u)
missing=0; via_libc=0; via_shim=0
for s in $need $need_data; do
    if grep -qx "$s" <<<"$have"; then via_shim=$((via_shim+1))
    elif grep -qx "$s" <<<"$libc"; then via_libc=$((via_libc+1)); echo "  host libc: $s"
    else missing=$((missing+1)); echo "  MISSING: $s"; fi
done
echo "coverage: $via_shim symbols from the shim, $via_libc from the host libc, $missing missing (of $(wc -w <<<"$need $need_data"))"

# Link check: the probe's game objects + the shim; what stays undefined must be the port runtime's (port_*,
# port_unimplemented), the game's own asm-only data (not Psy-Q) and libc.
undef=$(nm -u $game_objs "$OUT"/*.o | awk '{print $2}' | sort -u)
defined=$( (nm -g --defined-only $game_objs "$OUT"/*.o | awk '$2 ~ /^[TDBRCW]$/ {print $3}'; echo "$libc") | sort -u)
left=$(comm -23 <(echo "$undef") <(echo "$defined") | grep -v -e '^port_' -e '^$' || true)
dups=$(nm -g --defined-only $game_objs "$OUT"/*.o | awk '$2 ~ /^[TDBR]$/ {print $3}' | sort | uniq -d || true)
echo "link: undefined after the shim and libc (should be the game's asm-only data only):"
echo "$left" | sed 's/^/  /'
if [ -n "$dups" ]; then echo "link: DUPLICATE globals:"; echo "$dups" | sed 's/^/  /'; exit 1; fi
[ "$missing" -eq 0 ]
