#!/usr/bin/env bash
# Build everything and check it against config/*.sha1.
#
# Usage: scripts/build.sh            configure (splat + build.ninja) if needed, then ninja
#        scripts/build.sh --check    from scratch: delete asm/ and build/, re-split, build;
#                                    exits non-zero unless every output matches its SHA-1
#        scripts/build.sh -- ARGS    pass ARGS to ninja (e.g. -- -v)
#        DW3_JOBS=N scripts/build.sh ...   limit parallel jobs (ninja and configure.py's WSTAG split)
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

[[ -x tools/venv/bin/python && -x tools/binutils/bin/mipsel-linux-gnu-as ]] ||
    { echo "tools missing: run scripts/setup.sh (or scripts/worktree_init.sh in a worktree)" >&2; exit 1; }
[[ -f extracted/disc/SLES_039.36 ]] ||
    { echo "extracted/ missing: run scripts/extract.sh" >&2; exit 1; }

check=0
ninja_args=()
while [[ $# -gt 0 ]]; do
    case "$1" in
        --check) check=1; shift ;;
        --) shift; ninja_args=("$@"); break ;;
        -h|--help) sed -n '2,8p' "$0"; exit 0 ;;
        *) echo "unknown argument: $1" >&2; exit 1 ;;
    esac
done

if [[ $check == 1 ]]; then
    rm -rf asm build include/asm_generated build.ninja objdiff.json
fi
[[ -f build.ninja ]] || tools/venv/bin/python configure.py
# DW3_JOBS limits ninja's and configure.py's parallelism (several agents on one machine).
ninja ${DW3_JOBS:+-j "$DW3_JOBS"} "${ninja_args[@]}"
if [[ $check == 1 ]]; then
    echo "check passed: all outputs match their SHA-1"
else
    echo "build passed: all outputs match their SHA-1"
fi
