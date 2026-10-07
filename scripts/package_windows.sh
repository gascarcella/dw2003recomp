#!/usr/bin/env bash
# Builds the Windows release package (DECISIONS "Releases: tagged drafts, published by hand"; docs/RELEASE.md): the game
# (the SDL build) and the launcher cross-compiled for Windows x86_64 in Release (scripts/build_windows.sh: llvm-mingw,
# static, a PDB beside each executable), zipped as one folder with the mods' manifests, LICENSES/ and a README, plus a
# second zip with the two PDBs (the symbols of crash minidumps and reports: docs/PORT.md "Crash report"). Players
# supply their own disc in the launcher.
#
# Usage: scripts/package_windows.sh [--version V] [--out DIR] [--test] [--jobs N]
#   --version V  the version in the file names (default: the release tag being built, vX.Y.Z..., else the last such
#                tag by `git describe`, else dev-<commit>; a leading v dropped)
#   --out DIR    where the zips and SHA256SUMS-windows go (default build/release/)
#   --test       then test the package itself, unzipped, under Wine (wine on PATH; build/wine-prefix/): the launcher's
#                --self-test with DW3_SELFTEST_GAME=beside (the bundled game and mods, found by the launcher's own
#                lookup) and, when iso/dw2003.cue exists, the disc (the bundled game runs 300 frames from the
#                launcher's command) and a forced crash of the bundled game (tests/port/crash.py --wine) whose report
#                symbolizes with the symbols zip's PDB
#   --jobs N     parallel compile jobs (default: every core)
# Needs: scripts/setup.sh llvm-mingw sdl3-windows imgui; cmake, ninja, python3 (tools/venv), the host's objdump.
# Outputs: <out>/dw2003-<version>-windows-x86_64.zip, <out>/dw2003-<version>-windows-x86_64-symbols.zip,
# <out>/SHA256SUMS-windows; the build trees in build/port-win and build/launcher-win, the staging under
# build/release/windows/.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MAIN="$(dirname "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir)")"
JOBS="${DW3_JOBS:-$(nproc)}"
log() { printf '\033[1;34m[windows-package]\033[0m %s\n' "$*"; }
die() { printf '\033[1;31m[windows-package]\033[0m %s\n' "$*" >&2; exit 1; }

version="${DW3_VERSION:-}" out="$ROOT/build/release" test=0
while [[ $# -gt 0 ]]; do
    case "$1" in
        --version) version="$2"; shift 2 ;;
        --out) out="$2"; shift 2 ;;
        --test) test=1; shift ;;
        --jobs) JOBS="$2"; shift 2 ;;
        -h|--help) sed -n '2,22p' "$0"; exit 0 ;;
        *) die "unknown argument: $1" ;;
    esac
done

# A built tool: this checkout's tools/<name>, else the main checkout's (scripts/setup.sh installs there).
tool() {
    if [[ -e "$ROOT/tools/$1" ]]; then echo "$ROOT/tools/$1"; else echo "$MAIN/tools/$1"; fi
}

# ---- The version, as scripts/package_appimage.sh names its file.
if [[ -z "$version" ]]; then
    if [[ "${GITHUB_REF:-}" == refs/tags/v* ]]; then
        version="${GITHUB_REF#refs/tags/}"
    else
        version="$(git -C "$ROOT" describe --tags --match 'v[0-9]*.[0-9]*.[0-9]*' --always --dirty)"
        [[ "$version" == v* ]] || version="dev-$version"
    fi
fi
version="${version#v}"
[[ "$version" =~ ^[A-Za-z0-9._+-]+$ ]] || die "a version of letters, digits and ._+- only: '$version'"
base="dw2003-$version-windows-x86_64"
name="$base.zip"
symbols="$base-symbols.zip"

# ---- The tools.
llvm="$(tool llvm-mingw)"
[[ -x "$llvm/bin/x86_64-w64-mingw32-clang" ]] || die "no llvm-mingw at $llvm: scripts/setup.sh llvm-mingw"
sdl="$(tool sdl3-windows)"
[[ -f "$sdl/lib/libSDL3.a" && -f "$sdl/share/licenses/SDL3/LICENSE.txt" ]] || die "no SDL3 for Windows at $sdl: scripts/setup.sh sdl3-windows"
imgui="$(tool imgui)"
[[ -f "$imgui/imgui.cpp" && -f "$imgui/LICENSE.txt" ]] || die "no Dear ImGui: scripts/setup.sh imgui"
python="$(tool venv)/bin/python"
[[ -x "$python" ]] || die "no tools/venv/bin/python: scripts/setup.sh venv"
command -v objdump >/dev/null || die "no objdump on PATH (the host's binutils: the import check)"
pkg="$ROOT/packaging/windows"
[[ -f "$pkg/README.txt" && -f "$pkg/LICENSES/README.txt" ]] || die "no packaging/windows/"
log "dw2003 $version for Windows: llvm-mingw at $llvm, SDL3 at $sdl, Dear ImGui at $imgui"

# ---- The two programs, in Release (build_windows.sh's own build directories: build/port-win, build/launcher-win),
# the version stamped into both (port/cmake/version.cmake reads DW3_VERSION).
work="$ROOT/build/release/windows"
mkdir -p "$work" "$out"
log "building the game and the launcher (scripts/build_windows.sh --launcher; log: $work/build.log)"
DW3_VERSION="$version" "$ROOT/scripts/build_windows.sh" --launcher --jobs "$JOBS" >"$work/build.log" 2>&1 ||
    { tail -n 30 "$work/build.log" >&2; die "the Windows build failed (the whole output: $work/build.log)"; }
game_dir="$ROOT/build/port-win"
launcher_dir="$ROOT/build/launcher-win"
for f in "$game_dir/dw2003.exe" "$game_dir/dw2003.pdb" "$launcher_dir/dw2003-launcher.exe" \
         "$launcher_dir/dw2003-launcher.pdb"; do
    [[ -f "$f" ]] || die "the build made no $f"
done
[[ -d "$game_dir/mods" ]] || die "the game's build has no mods/ (port/CMakeLists.txt copies port/mods there)"

# ---- Checks on the executables: GUI subsystem (no console window), the manifest resource, static (no DLL of the
# toolchain's or SDL's: only Windows' own libraries are imported).
for exe in "$game_dir/dw2003.exe" "$launcher_dir/dw2003-launcher.exe"; do
    headers="$(objdump -p "$exe")"
    subsystem="$(grep -E '^Subsystem' <<<"$headers" | head -n 1 | sed 's/^Subsystem[[:space:]]*//')"
    [[ "$subsystem" == 00000002* || "$subsystem" == *"Windows GUI"* ]] ||
        die "$(basename "$exe") is not a Windows GUI program (Subsystem: $subsystem)"
    grep -aq 'activeCodePage' "$exe" || die "$(basename "$exe") has no application manifest (the UTF-8 code page)"
    dlls="$(grep -E '^\s*DLL Name:' <<<"$headers" | sed 's/.*DLL Name: //' | tr '\n' ' ')"
    bad="$(tr ' ' '\n' <<<"$dlls" | grep -iE '^(libc\+\+|libunwind|libwinpthread|libgcc|SDL3)' || true)"
    [[ -z "$bad" ]] || die "$(basename "$exe") imports a DLL the package does not carry: $bad"
    log "$(basename "$exe"): $(du -h "$exe" | cut -f1), GUI subsystem, manifest; imports: $dlls"
done

# ---- The package folder and the symbols folder.
stage="$work/$base"
syms="$work/$base-symbols"
rm -rf "$stage" "$syms"
mkdir -p "$stage/LICENSES" "$syms"
cp "$game_dir/dw2003.exe" "$launcher_dir/dw2003-launcher.exe" "$stage/"
cp -r "$game_dir/mods" "$stage/mods"
sed "s/@VERSION@/$version/g" "$pkg/README.txt" > "$stage/README.txt"
cp "$ROOT/LICENSE" "$stage/LICENSES/dw2003recomp.txt"
cp "$sdl/share/licenses/SDL3/LICENSE.txt" "$stage/LICENSES/SDL3.txt"
cp "$imgui/LICENSE.txt" "$stage/LICENSES/imgui.txt"
cp "$ROOT/packaging/appimage/LICENSES/ProggyClean.txt" "$ROOT/packaging/appimage/LICENSES/ProggyForever.txt" \
    "$stage/LICENSES/"
cp "$llvm/LICENSE.TXT" "$stage/LICENSES/llvm.txt"
cp "$pkg/LICENSES/"*.txt "$stage/LICENSES/"
# Windows line endings for the text files a player opens in Notepad (the licences as their authors wrote them).
sed -i 's/\r\?$/\r/' "$stage/README.txt" "$stage/LICENSES/README.txt"
cp "$game_dir/dw2003.pdb" "$launcher_dir/dw2003-launcher.pdb" "$syms/"
printf 'The PDB files of dw2003 %s for Windows (dw2003-%s-windows-x86_64.zip): the symbols of its crash reports and\r\nminidumps (docs/PORT.md "Crash report"; scripts/symbolize.py REPORT --binary dw2003.exe --pdb dw2003.pdb).\r\nNot needed to play.\r\n' "$version" "$version" > "$syms/README.txt"

# ---- The zips (one folder each at the root, so that unzipping spills nothing) and the checksums.
rm -f "$out/$name" "$out/$symbols"
(cd "$work" && "$python" -m zipfile -c "$out/$name" "$base")
(cd "$work" && "$python" -m zipfile -c "$out/$symbols" "$base-symbols")
(cd "$out" && sha256sum "$name" "$symbols" > SHA256SUMS-windows)
log "$(head -n 1 "$out/SHA256SUMS-windows") ($(du -h "$out/$name" | cut -f1))"
log "$(sed -n 2p "$out/SHA256SUMS-windows") ($(du -h "$out/$symbols" | cut -f1))"

# ---- The test: the package itself, unzipped, under Wine.
if [[ $test -eq 1 ]]; then
    command -v wine >/dev/null || die "no wine on PATH for --test"
    unz="$work/test"
    rm -rf "$unz"
    mkdir -p "$unz" "$ROOT/build/wine-prefix"
    "$python" -m zipfile -e "$out/$name" "$unz"
    "$python" -m zipfile -e "$out/$symbols" "$unz"
    [[ -f "$unz/$base/dw2003-launcher.exe" && -f "$unz/$base/dw2003.exe" && -f "$unz/$base/mods/fast_forward/mod.json" &&
       -f "$unz/$base-symbols/dw2003.pdb" ]] || die "the zips do not unpack to $base/ and $base-symbols/"
    export WINEPREFIX="$ROOT/build/wine-prefix" WINEDEBUG=-all
    # The launcher's self-test from the unzipped folder: the game and the mods beside it, found by its own lookup.
    # Wine sees the Unix tree as drive Z:. DW3_SELFTEST_VIDEO_DRIVER: SDL's hint, since Wine may drop SDL_VIDEO_DRIVER
    # from the program's environment (launcher/README.md). `timeout`: a hang fails instead of waiting for a display.
    scratch="$unz/selftest"
    mkdir -p "$scratch"
    vars=(DW3_SELFTEST_VIDEO_DRIVER=offscreen SDL_VIDEO_DRIVER=offscreen SDL_AUDIO_DRIVER=dummy DW3_SELFTEST_GAME=beside)
    if [[ -f "$ROOT/iso/dw2003.cue" ]]; then
        vars+=("DW3_SELFTEST_DISC=Z:$ROOT/iso/dw2003.cue")
        log "test: the launcher's self-test under wine ($(wine --version)) with the bundled game, the mods and the disc"
    else
        log "test: the launcher's self-test under wine ($(wine --version)) with the bundled game and the mods (no iso/dw2003.cue: no disc run)"
    fi
    if env -u DW3_GAME -u DW3_CONFIG_DIR "${vars[@]}" timeout 900 wine "$unz/$base/dw2003-launcher.exe" \
        --self-test "Z:$scratch" >"$scratch.log" 2>&1; then
        log "test: self-test passed: $(grep -o '[0-9]* of [0-9]* checks passed' "$scratch.log" || true)"
    else
        grep 'FAILED' "$scratch.log" | sed 's/^/    /' | head -20 >&2
        die "the package's self-test under wine failed (log: $scratch.log)"
    fi
    # A forced crash of the bundled game, its report symbolized with the symbols zip's PDB (tests/port/crash.py wants
    # the PDB beside the executable: a copy of both in a scratch folder).
    if [[ -f "$ROOT/iso/dw2003.cue" ]]; then
        crash="$unz/crash"
        mkdir -p "$crash"
        cp "$unz/$base/dw2003.exe" "$unz/$base-symbols/dw2003.pdb" "$crash/"
        log "test: a forced crash of the bundled game under wine, symbolized with the symbols zip's PDB"
        timeout 600 "$python" "$ROOT/tests/port/crash.py" --wine --exe "$crash/dw2003.exe" --out "$crash/out" ||
            die "the package's crash report test under wine failed"
    fi
    log "test passed"
fi
