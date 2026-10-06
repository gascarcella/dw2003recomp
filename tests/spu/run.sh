#!/usr/bin/env bash
# The SPU core's host tests (docs/SOUND.md section 6): builds tests/spu/spu_test (and the trace renderer, for
# render_trace.py) from port/src/spu.c, spu_dsp.c and sha1.c with the port's flags into build/spu_test/<variant>/, and
# replays tests/spu/goldens.txt through it. About a second per variant; needs only the host gcc.
# Usage: tests/spu/run.sh [--m32] [--sanitize] [--all] [-v]
#   --m32       a 32-bit build (gcc-multilib), --sanitize ASan + UBSan, --all the three builds one after the other
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
VARIANTS=()
ARGS=()
for a in "$@"; do
    case "$a" in
        --m32) VARIANTS+=(m32) ;;
        --sanitize) VARIANTS+=(san) ;;
        --all) VARIANTS+=(m64 m32 san) ;;
        -h|--help) sed -n '2,5p' "$0"; exit 0 ;;
        *) ARGS+=("$a") ;;
    esac
done
[[ ${#VARIANTS[@]} -eq 0 ]] && VARIANTS=(m64)
SRCS=("$ROOT/port/src/spu.c" "$ROOT/port/src/spu_dsp.c" "$ROOT/port/src/sha1.c")
CFLAGS=(-std=gnu99 -O2 -fwrapv -fsigned-char -fno-strict-aliasing -Wall -Wextra -Werror
        -I"$ROOT/port/include" -I"$ROOT/port/src")
for v in "${VARIANTS[@]}"; do
    out="$ROOT/build/spu_test/$v"
    mkdir -p "$out"
    case "$v" in
        m64) extra=(-m64) ;;
        m32) extra=(-m32) ;;
        san) extra=(-m64 -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer) ;;
    esac
    gcc "${CFLAGS[@]}" "${extra[@]}" "${SRCS[@]}" "$ROOT/tests/spu/spu_test.c" -o "$out/spu_test"
    gcc "${CFLAGS[@]}" "${extra[@]}" "${SRCS[@]}" "$ROOT/tests/spu/trace_render.c" -o "$out/trace_render" -lm
    echo -n "[$v] "
    "$out/spu_test" "$ROOT/tests/spu/goldens.txt" ${ARGS[@]+"${ARGS[@]}"}
done
