#!/usr/bin/env bash
# The XA decoder's host tests (docs/SOUND.md "CD audio"): builds tests/xa/xa_test from port/psyq/xa.c and
# port/src/sha1.c with the port's flags into build/xa_test/<variant>/, and replays tests/xa/goldens.txt through it
# (tests/xa/xa_ref.py, the Python model, generates them). Under a second per variant; needs only the host gcc.
# Usage: tests/xa/run.sh [--m32] [--sanitize] [--all] [-v]
#   --m32       a 32-bit build (gcc-multilib), --sanitize ASan + UBSan, --all the three builds one after the other
# The movies' real sectors (needs the disc): tools/venv/bin/python tests/xa/xa_ref.py disc [MOVIE...] after this.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
VARIANTS=()
ARGS=()
for a in "$@"; do
    case "$a" in
        --m32) VARIANTS+=(m32) ;;
        --sanitize) VARIANTS+=(san) ;;
        --all) VARIANTS+=(m64 m32 san) ;;
        -h|--help) sed -n '2,7p' "$0"; exit 0 ;;
        *) ARGS+=("$a") ;;
    esac
done
[[ ${#VARIANTS[@]} -eq 0 ]] && VARIANTS=(m64)
SRCS=("$ROOT/port/psyq/xa.c" "$ROOT/port/src/sha1.c")
CFLAGS=(-std=gnu99 -O2 -fwrapv -fsigned-char -fno-strict-aliasing -Wall -Wextra -Werror
        -I"$ROOT/port/psyq" -I"$ROOT/port/src")
for v in "${VARIANTS[@]}"; do
    out="$ROOT/build/xa_test/$v"
    mkdir -p "$out"
    case "$v" in
        m64) extra=(-m64) ;;
        m32) extra=(-m32) ;;
        san) extra=(-m64 -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer) ;;
    esac
    gcc "${CFLAGS[@]}" "${extra[@]}" "${SRCS[@]}" "$ROOT/tests/xa/xa_test.c" -o "$out/xa_test"
    echo -n "[$v] "
    "$out/xa_test" "$ROOT/tests/xa/goldens.txt" ${ARGS[@]+"${ARGS[@]}"}
done
