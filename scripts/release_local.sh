#!/usr/bin/env bash
# A release without waiting for release.yml (docs/RELEASE.md "A local release"): builds the AppImage and the Windows
# package in a Docker ubuntu:24.04 container (the CI runner's base, so the same glibc floor), with release.yml's own
# steps (the tools, the disc, the game's Release build through the port's M1 test, package_appimage.sh --test,
# package_windows.sh --test under the container's Wine), then pushes the tag, cancels the release.yml run the tag
# starts and creates the same DRAFT release with the local files. Publishing stays the maintainer's decision
# (DECISIONS "Releases: tagged drafts, published by hand").
#
# Usage: scripts/release_local.sh vX.Y.Z [--commit REF] [--build-only]
#   --commit REF   what to release (default origin/main; it must be pushed: the tag goes to origin)
#   --build-only   build and test the packages only: no tag, no release
# Needs: docker, gh (logged in, with push rights), the disc (iso/dw2003.bin here or in the main checkout, or
# DW3_DISC_BIN in tools/local.env). The container's tools stay in build/release-local/ between runs (~10 min cold).
# Output: build/release-local/src/build/release/: dw2003-<version>-x86_64.AppImage, the .debug file, SHA256SUMS,
# dw2003-<version>-windows-x86_64.zip, its -symbols.zip and SHA256SUMS-windows.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MAIN="$(dirname "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir)")"
WORK="$ROOT/build/release-local"
IMAGE=dw2003-release
log() { printf '\033[1;34m[release]\033[0m %s\n' "$*"; }
die() { printf '\033[1;31m[release]\033[0m %s\n' "$*" >&2; exit 1; }

tag="" ref=origin/main build_only=0
while [[ $# -gt 0 ]]; do
    case "$1" in
        --commit) ref="$2"; shift 2 ;;
        --build-only) build_only=1; shift ;;
        -h|--help) sed -n '2,14p' "$0"; exit 0 ;;
        v*) tag="$1"; shift ;;
        *) die "unknown argument: $1" ;;
    esac
done
[[ "$tag" =~ ^v[0-9]+\.[0-9]+\.[0-9]+ ]] || die "a release tag vX.Y.Z is required (e.g. v0.2.0)"
command -v docker >/dev/null || die "docker is required"
command -v gh >/dev/null || die "gh is required"

# ---- The disc: this checkout's, the main checkout's, else DW3_DISC_BIN.
disc=""
for d in "$ROOT/iso/dw2003.bin" "$MAIN/iso/dw2003.bin"; do [[ -e "$d" ]] && { disc="$(readlink -f "$d")"; break; }; done
if [[ -z "$disc" && -f "$MAIN/tools/local.env" ]]; then
    # shellcheck disable=SC1091
    source "$MAIN/tools/local.env"; disc="${DW3_DISC_BIN:-}"
fi
[[ -f "$disc" ]] || die "no disc: set DW3_DISC_BIN in tools/local.env (see local.env.example)"

# ---- What to release.
git -C "$ROOT" fetch -q origin --tags
commit="$(git -C "$ROOT" rev-parse --verify "$ref^{commit}")" || die "no such commit: $ref"
git -C "$ROOT" branch -r --contains "$commit" | grep -q 'origin/' || die "$ref ($commit) is not pushed"
if [[ $build_only == 0 ]] && git -C "$ROOT" ls-remote --exit-code --tags origin "refs/tags/$tag" >/dev/null; then
    die "$tag already exists on origin (docs/RELEASE.md: delete it to retry)"
fi
log "$tag = $(git -C "$ROOT" log --oneline -1 "$commit")"

# ---- A clean checkout of that commit (its own main checkout, so setup.sh installs the tools into it).
if [[ ! -d "$WORK/src/.git" ]]; then
    mkdir -p "$WORK"
    git clone -q "$(git -C "$ROOT" remote get-url origin)" "$WORK/src"
fi
git -C "$WORK/src" fetch -q origin
git -C "$WORK/src" checkout -q --detach "$commit"
# The psxstack submodule at the commit's pin (fetched here, with this machine's access to the repository; the
# container never talks to GitHub). --reference: this checkout's copy, when it has one.
ref=(); [[ -e "$ROOT/psxstack/.git" ]] && ref=(--reference "$ROOT/psxstack")
git -C "$WORK/src" submodule update --init "${ref[@]}" psxstack
git -C "$WORK/src" clean -qfdx -e tools/ -e .home/   # the container's tools are kept between runs
echo 'DW3_DISC_BIN="/disc/dw2003.bin"' > "$WORK/src/tools/local.env"

# ---- The image: ubuntu:24.04 with release.yml's packages (cached by Docker); wine64 for the Windows package's test.
log "image $IMAGE (ubuntu:24.04)"
docker build -q -t "$IMAGE" - >/dev/null <<EOF
FROM ubuntu:24.04
RUN apt-get update -q && DEBIAN_FRONTEND=noninteractive apt-get install -y -q --no-install-recommends \
    build-essential git curl wget ca-certificates python3 python3-venv python3-pip ninja-build cmake xz-utils file \
    pkg-config binutils wine wine64 $("$ROOT/scripts/setup.sh" --sdl3-desktop-apt)
EOF

# ---- release.yml's appimage and windows jobs, in the container (the Windows package's test runs under the
# container's Wine, headless, as on the runner).
log "building and testing (log: $WORK/build.log)"
docker run --rm --user "$(id -u):$(id -g)" -e HOME=/work/.home -e "DW3_VERSION=$tag" -e WINEDEBUG=-all \
    -v "$WORK/src:/work" -v "$disc:/disc/dw2003.bin:ro" -w /work "$IMAGE" bash -c '
set -euxo pipefail
mkdir -p "$HOME"; git config --global --add safe.directory "*"
scripts/setup.sh venv mkpsxiso imgui sdl3-desktop appimage llvm-mingw sdl3-windows dxc
scripts/worktree_init.sh
test -f iso/dw2003.cue
cmake -S port -B build/port -G Ninja -DCMAKE_BUILD_TYPE=Release
tools/venv/bin/python tests/port/run.py
scripts/package_appimage.sh --test
mkdir -p build/wine-prefix
WINEPREFIX=/work/build/wine-prefix wineboot --init
scripts/package_windows.sh --test
' > "$WORK/build.log" 2>&1 || { tail -30 "$WORK/build.log"; die "the build or a test failed (log: $WORK/build.log)"; }
out="$WORK/src/build/release"
(cd "$out" && sha256sum -c SHA256SUMS && sha256sum -c SHA256SUMS-windows) || die "SHA256SUMS does not match"
appimage="$(ls "$out"/dw2003-*-x86_64.AppImage)"
debug="$(ls "$out"/dw2003-*-x86_64.debug)"
winzip="$(ls "$out"/dw2003-*-windows-x86_64.zip)"
winsyms="$(ls "$out"/dw2003-*-windows-x86_64-symbols.zip)"
log "built $appimage"
log "built $winzip"
[[ $build_only == 1 ]] && exit 0

# ---- The tag, without release.yml's run (it would build the same thing in ~30 min).
git -C "$ROOT" tag -a "$tag" "$commit" -m "dw2003recomp ${tag#v}"
git -C "$ROOT" push -q origin "$tag"
log "pushed $tag; cancelling its release.yml run"
run=""
for _ in $(seq 60); do
    run="$(gh run list --workflow release.yml --limit 10 --json databaseId,headBranch \
        -q ".[] | select(.headBranch == \"$tag\") | .databaseId" | head -1)"
    [[ -n "$run" ]] && break
    sleep 3
done
if [[ -n "$run" ]]; then gh run cancel "$run" || true; else log "no release.yml run found to cancel (check: gh run list)"; fi

# ---- The draft release, as release.yml's release job makes it (keep the notes in step with release.yml).
cd "$out"
gh release create "$tag" --draft --verify-tag --title "dw2003recomp $tag" \
    --generate-notes --notes "The launcher, the PC port and its mods, for Linux and for Windows.
Bring your own disc image of Digimon World 2003 (PS1, Europe; the launcher checks its SHA-1).

**Linux x86_64**: \`dw2003-*-x86_64.AppImage\`: \`chmod +x\` it, then run it. Needs glibc 2.39 or newer (Ubuntu 24.04, Fedora 40, Debian 13 or later).
**Windows x86_64**: \`dw2003-*-windows-x86_64.zip\`: unzip, run \`dw2003-launcher.exe\` (unsigned: SmartScreen's \"More info\", \"Run anyway\"). Windows 10 or newer. Tested on Linux under Wine and Proton only, not yet on real Windows: reports are welcome in the issues (#37).
The .debug file and the -symbols.zip are the programs' debug info for crash reports (not needed to play)." \
    "./$(basename "$appimage")" "./$(basename "$debug")" "./$(basename "$winzip")" "./$(basename "$winsyms")" \
    SHA256SUMS SHA256SUMS-windows
log "draft release $tag created: test the AppImage and the Windows zip, then Publish it on GitHub (gh release edit $tag --draft=false)"
