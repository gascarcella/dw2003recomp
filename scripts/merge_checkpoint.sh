#!/usr/bin/env bash
# Merge a (decompilation agent's) branch into the current branch, prove it, and update the README progress table.
#
# Usage: scripts/merge_checkpoint.sh <branch> "<merge commit message>"
#   exit 0: merged, `scripts/build.sh --check` passed (every output byte-identical), README progress committed
#   exit 2: merge conflict (the merge is left in progress: resolve, build, commit, or `git merge --abort`)
#   exit 3: build failed after the merge (the merge commit is left in place, not pushed: fix or `git reset --hard HEAD~1`)
# The build log is build/merge_checkpoint.log (outside build/ while --check deletes it, then moved).
# Commit trailers come from $MERGE_TRAILERS (e.g. Co-Authored-By lines), if set.
set -uo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
[[ $# -eq 2 ]] || { sed -n '2,10p' "$0"; exit 1; }
branch="$1"; msg="$2"
trailers="${MERGE_TRAILERS:+

$MERGE_TRAILERS}"
log="$(mktemp)"

if ! git merge --no-ff -q -m "$msg$trailers" "$branch" >/dev/null 2>&1; then
    echo "CONFLICT in: $(git diff --name-only --diff-filter=U | tr '\n' ' ')"
    exit 2
fi
if ! scripts/build.sh --check >"$log" 2>&1; then
    mkdir -p build && mv "$log" build/merge_checkpoint.log
    echo "BUILD FAILED (build/merge_checkpoint.log):"
    grep -E "error|FAILED" build/merge_checkpoint.log | head -8
    exit 3
fi
mv "$log" build/merge_checkpoint.log
echo "merged + check passed: $branch"

# README progress table (tools/progress.py), committed separately when it changed.
tools/venv/bin/python tools/progress.py --readme >/dev/null
if ! git diff --quiet README.md; then
    git commit -q -m "README: progress after merging $branch$trailers" README.md
    echo "README progress updated"
fi
