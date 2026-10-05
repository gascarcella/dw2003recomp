#!/usr/bin/env bash
# Build the public snapshot of this checkout: a new repository holding one commit with the tracked tree minus the
# paths that stay in the data checkout (gamedata/, tools/prebuilt/), and prove it clean. DECISIONS "Going public",
# docs/OPEN_SOURCE_PLAN.md section 5 step 3.
#
# Usage: scripts/publish_snapshot.sh <empty or absent dir> [--tag v0.1-matching-closed] [--remote <url>]
#   The tree must be clean (no modified tracked files). The commit message names the source commit.
#   --remote adds `origin` (nothing is pushed: review, then `git -C <dir> push -u origin main --tags`).
# Checks after the commit: no excluded path, no game-data-like file name, no blob over 1 MiB outside tests/golden/.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EXCLUDE=(gamedata tools/prebuilt)
BIG_OK='^tests/golden/.*\.json$'

DEST= TAG= REMOTE=
while [[ $# -gt 0 ]]; do
    case "$1" in
        --tag) TAG="$2"; shift 2 ;;
        --remote) REMOTE="$2"; shift 2 ;;
        -h|--help) sed -n '2,10p' "$0"; exit 0 ;;
        *) [[ -z "$DEST" ]] || { echo "unexpected argument: $1" >&2; exit 1; }; DEST="$1"; shift ;;
    esac
done
[[ -n "$DEST" ]] || { echo "usage: $0 <dir> [--tag T] [--remote URL]" >&2; exit 1; }
[[ -z "$(git -C "$ROOT" status --porcelain --untracked-files=no)" ]] || { echo "tree not clean: commit or stash first" >&2; exit 1; }
if [[ -e "$DEST" ]]; then
    [[ -d "$DEST" && -z "$(ls -A "$DEST")" ]] || { echo "$DEST exists and is not empty" >&2; exit 1; }
fi
mkdir -p "$DEST"
DEST="$(cd "$DEST" && pwd)"

SRC_COMMIT="$(git -C "$ROOT" rev-parse HEAD)"
SRC_DATE="$(git -C "$ROOT" log -1 --format=%cs HEAD)"
echo "snapshot of $SRC_COMMIT ($SRC_DATE) -> $DEST"
git -C "$ROOT" archive --format=tar HEAD | tar -x -C "$DEST"
for p in "${EXCLUDE[@]}"; do rm -rf "${DEST:?}/$p"; done

git -C "$DEST" init -q -b main
git -C "$DEST" add -A
git -C "$DEST" -c commit.gpgsign=false commit -q -F - <<MSG
Initial public snapshot of the matching milestone

Every game file rebuilds byte-identical from this tree (scripts/build.sh --check) and the reference tests pass
(scripts/test.sh). Taken from the private working history at $SRC_COMMIT ($SRC_DATE), which stays the archive
of the per-session record; docs/ cites commits of that history.
MSG
[[ -z "$TAG" ]] || git -C "$DEST" tag -a "$TAG" -m "$TAG: the matching milestone, as published"
[[ -z "$REMOTE" ]] || git -C "$DEST" remote add origin "$REMOTE"

# Proofs.
fail=0
for p in "${EXCLUDE[@]}"; do
    if git -C "$DEST" ls-files -- "$p" | grep -q .; then echo "FAIL: $p is in the snapshot" >&2; fail=1; fi
done
if git -C "$DEST" ls-files | grep -iE '\.(bin|iso|img|cue|chd|exe|pro|mcd|sstate|lib|obj|xz|part[0-9]+)$'; then
    echo "FAIL: game-data-like file names above" >&2; fail=1
fi
while IFS=$'\t' read -r size path; do
    [[ "$path" =~ $BIG_OK ]] && continue
    echo "FAIL: $path is $size bytes (over 1 MiB)" >&2; fail=1
done < <(git -C "$DEST" ls-files -z | (cd "$DEST" && xargs -0 stat -c $'%s\t%n') | awk -F'\t' '$1 > 1048576')
files="$(git -C "$DEST" ls-files | wc -l)"
size="$(du -sk --apparent-size "$DEST" --exclude=.git | cut -f1)"
echo "snapshot: $files files, ${size} kB, commit $(git -C "$DEST" rev-parse --short HEAD)${TAG:+, tag $TAG}"
[[ $fail -eq 0 ]] || { echo "snapshot FAILED its checks" >&2; exit 1; }
echo "clean. Next: review it, then:  git -C $DEST push -u origin main${TAG:+ --tags}"
