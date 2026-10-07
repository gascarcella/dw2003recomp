#!/usr/bin/env bash
# Project-local toolchain setup. Idempotent: re-running skips finished steps.
# Nothing here needs sudo or installs outside the repo.
#
# Usage: scripts/setup.sh [--disc /path/to/disc.bin] [step...]
#   steps: binutils venv cmake mkpsxiso gcc objdiff ext redux link gamedata disc  (default: all)
#          optional: psyq sdl3 imgui sdl3-desktop appimage llvm-mingw sdl3-windows
#   llvm-mingw: the pinned llvm-mingw release (clang + lld, UCRT) into tools/llvm-mingw: the Windows cross toolchain
#          (cmake/windows-x86_64.cmake, scripts/build_windows.sh; DECISIONS "Windows")
#   sdl3-windows: SDL3 cross-built static for Windows into tools/sdl3-windows (needs llvm-mingw)
#   imgui: Dear ImGui at its pinned tag into tools/imgui, for the launcher (launcher/README.md; needs sdl3 too)
#   sdl3:  SDL3 built from its pinned source tarball into tools/sdl3 (static), for the PC port's window
#          (cmake -DDW3_PORT_SDL=ON; port/README.md "The window"); its backends follow the -dev headers present
#   sdl3-desktop: the same SDL into tools/sdl3-desktop, with the desktop backends required (the release's);
#          `scripts/setup.sh --sdl3-desktop-apt` prints the Ubuntu -dev packages it needs
#   appimage: appimagetool and the static AppImage runtime, pinned, into tools/appimage (scripts/package_appimage.sh)
#   cmake: CMake and Ninja (mkpsxiso and the PC port build with them) pip-installed into tools/venv, only when either is
#          missing from PATH; later steps and tests/port/run.py find them there
#
# Worktrees: built tools always go into the MAIN checkout's tools/ (shared by every
# worktree), and the "link" step symlinks them into this worktree. Tracked inputs
# (requirements.txt, ...) are read from this checkout.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MAIN="$(dirname "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir)")"
TOOLS="$ROOT/tools"          # tracked files of this checkout
INSTALL="$MAIN/tools"        # built/installed tools, shared across worktrees
SRC="$INSTALL/src"
JOBS="${DW3_JOBS:-$(nproc)}"   # DW3_JOBS: fewer on a shared machine (as scripts/build.sh)

BINUTILS_VER=2.47
BINUTILS_SHA256=154ab23b60070e8f27013c22977f1129425d67d1e8acd6e13010e617811e4cff
MKPSXISO_TAG=v2.30

# decompals/old-gcc release: GNU GCC built with Psy-Q's PSX patches (static i386 binaries).
# Psy-Q 4.0/4.3/4.4/4.5/4.6 shipped 2.7.2/2.8.0/2.8.1/2.91.66/2.95.2. SHA-256 of each tarball.
OLDGCC_TAG=0.17
OLDGCC=(
    "2.7.2   500a459b3485e885a8d302cac23c2a4632f3900e03a09153f6190699fd723571"
    "2.8.0   1a3c956fe8aea5ebdb251749d95de2c84f023530584d7bd663744b5ec24050b7"
    "2.8.1   f6f6e883942d4d3289d048236c672e71ed410e546aaae8ff655952f1567e1be0"
    "2.91.66 f773a0a9659fa4ff74313ac4363d939312e8125675cd09ad9f8c1202f587f1fd"
    "2.95.2  932ed3669710a82b12570c29c46ff42989b6505b2af10db84d4551d55dfc0b1c"
)

OBJDIFF_VER=3.8.2
OBJDIFF_CLI_SHA256=e5445089a6f707e30cb5e02992451b604619cedfd2660ee2373a86bfefd9e718  # linux-x86_64

# Python tools used straight from git, pinned to a commit (no tags/PyPI releases worth using).
EXT=(
    "maspsx          https://github.com/mkst/maspsx.git                   7686f845a181700534c83c0419183e38aeb3e49c"
    "m2c             https://github.com/matt-kempster/m2c.git             708d2d2cb2698f091a92492b328f73b24209f72d"
    "asm-differ      https://github.com/simonlindholm/asm-differ.git      0dd09af8f8008f1f880327cf0aca3b26d2562ea2"
    "decomp-permuter https://github.com/simonlindholm/decomp-permuter.git 059609d4aec73eb0650726772954e1ad575825f8"
    "psyq-sigs       https://github.com/lab313ru/psx_psyq_signatures.git e9e46e7e133ef275a79bfce650924f98edb086bc"
)

# SHA-1 of the unpatched "Digimon World 2003 (Europe) (En,Fr,De,Es,It)" .bin
# (single MODE2/2352 track); matches Redump disc #3160.
DISC_SHA1=457cb233349ba841e03b33d8060f8fbcadd45cb3

# PCSX-Redux (MIT), the emulator the tests run the game in (headless: `-no-ui`). The pinned Linux build (dev channel,
# build 359, changeset bf4c9ceb) comes from the data checkout's tools/prebuilt/ when there is one (scripts/gamedata_dir.sh;
# some networks can't reach distrib.app), else it is downloaded from distrib.app and checked against REDUX_SHA256, else the
# AppImage named by PCSX_REDUX in tools/local.env is used. Its AppImage is built against glibc 2.43, so on an older host
# (Ubuntu 24.04: 2.39) the step unpacks a pinned runtime sysroot (Ubuntu 26.04 "resolute" packages, SHA-256 each) and
# runs the binary through that glibc's loader.
REDUX_ZIP=PCSX-Redux-bf4c9ceb-linux-x86_64.zip
REDUX_URL=https://distrib.app/storage/assets/d9b/f74/b48/50eac9b83609cd54f27b63a27b7155cf9c91d293b32673847315b52/$REDUX_ZIP
REDUX_SHA256=5c0138d8a948c021e67aaba62648924c0a6e05d9d933c945d2c5077b4c758980
REDUX_UBUNTU=http://archive.ubuntu.com/ubuntu
REDUX_SYSROOT_DEBS=(
    "pool/main/g/glibc/libc6_2.43-2ubuntu2_amd64.deb c13775dc0c984403f3fcad229d14507a9f387763bd07ace1e5f93897ee6b8434"
    "pool/main/g/gcc-16/libgcc-s1_16-20260322-1ubuntu1_amd64.deb 2fb4d81c14fdf34251639ae82f5181f9f98480ea16125d535571ac1be9db3065"
    "pool/main/g/gcc-16/libstdc++6_16-20260322-1ubuntu1_amd64.deb a32b9ad585e39bdc7bd15d1eae6293461e6d466859a1dd2f7862d1e40f89b9d8"
    "pool/main/f/fontconfig/libfontconfig1_2.17.1-3ubuntu1_amd64.deb 72ba4fc43155566b46442b741a6892d5d1f1581ba58cdfd037aabf47ad9080f7"
    "pool/main/f/freetype/libfreetype6_2.14.2+dfsg-1_amd64.deb a76ad9102039122ef72b01c9c364e7d8967331414cbe19151c01d1391ff9fe25"
    "pool/main/h/harfbuzz/libharfbuzz0b_12.3.2-2_amd64.deb ec93ea39ddeb5a98179b60eab4af234b6c676f1958106645d1f4b19280d4cd09"
    "pool/main/f/fribidi/libfribidi0_1.0.16-5_amd64.deb 47fb50e96401a46c06d5122cd04cc42316f65530b9f868d2d450576a379d5d34"
    "pool/main/e/expat/libexpat1_2.7.4-1_amd64.deb ef78089497946219cac03209d951bbfff93480104e791b770a7bf6429452e475"
    "pool/main/g/gmp/libgmp10_6.3.0+dfsg-5ubuntu2_amd64.deb a9bbe9d4a4bcd5875bbd0f53e67c379bc881d1c1a80522d51dfb4b107cc31819"
    "pool/main/z/zlib/zlib1g_1.3.dfsg+really1.3.1-1ubuntu3_amd64.deb c45bbbf9c87457d90b8ba38720c5f01b9388c380d40f303c26f5c4953932da26"
    "pool/main/libx/libx11/libx11-6_1.8.13-1_amd64.deb 7e643e76063b94df9159c8b8fd4b8bf53d2fbf119c9889a5cc6842b6be272e4f"
    "pool/main/libx/libx11/libx11-xcb1_1.8.13-1_amd64.deb e20ab288f2f756800ae25e7a827ae70aee64272f692dd3cb8f706764b6767de9"
    "pool/main/libx/libxcb/libxcb1_1.17.0-2ubuntu1_amd64.deb 069e64705e4a5721a223b4d22643bbaeaa63a6efbb56ebd3d7c4a31c4a26fa1c"
    "pool/main/libx/libxcb/libxcb-dri3-0_1.17.0-2ubuntu1_amd64.deb 2e0e3916c647a74e081621f385aa296dab3052b2582b0781d9fbea2f15696ef4"
    "pool/main/libd/libdrm/libdrm2_2.4.131-1_amd64.deb b87e6f715d0795a88b53fbe268004a1acbd0d728da0cc0ab9108aff5ab4e1929"
)

log() { printf '\033[1;34m[setup]\033[0m %s\n' "$*"; }
die() { printf '\033[1;31m[setup]\033[0m %s\n' "$*" >&2; exit 1; }
# fetch URL FILE: a download that survives a transient failure. CI's runners get connection resets from the mirrors
# now and then (archive.ubuntu.com, 2026-10-07: two runs in a row, then a hang); a cold tool cache must not die or
# stall on that: 5 retries on any error, no connect longer than 30 s, no transfer longer than 15 min.
fetch() { curl -sSfL --retry 5 --retry-all-errors --retry-delay 5 --connect-timeout 30 --max-time 900 -o "$2" "$1"; }

step_binutils() {
    local prefix="$INSTALL/binutils"
    if [[ -x "$prefix/bin/mipsel-linux-gnu-as" ]]; then
        log "binutils: already installed ($("$prefix/bin/mipsel-linux-gnu-as" --version | head -1))"
        return
    fi
    mkdir -p "$SRC"
    local tarball="$SRC/binutils-$BINUTILS_VER.tar.xz"
    if [[ ! -f "$tarball" ]]; then
        log "binutils: downloading $BINUTILS_VER"
        # Ubuntu's .orig tarball is the upstream file (same SHA-256); it is the fallback for
        # networks that block ftp.gnu.org (e.g. cloud sessions).
        fetch "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.xz" "$tarball.part" ||
            fetch "http://archive.ubuntu.com/ubuntu/pool/main/b/binutils/binutils_$BINUTILS_VER.orig.tar.xz" \
                "$tarball.part"
        mv "$tarball.part" "$tarball"
    fi
    echo "$BINUTILS_SHA256  $tarball" | sha256sum -c --quiet - || die "binutils: checksum mismatch"
    rm -rf "$SRC/binutils-$BINUTILS_VER" "$SRC/binutils-build"
    tar -C "$SRC" -xf "$tarball"
    mkdir -p "$SRC/binutils-build"
    log "binutils: configuring (target mipsel-linux-gnu)"
    (cd "$SRC/binutils-build" && "../binutils-$BINUTILS_VER/configure" \
        --prefix="$prefix" --target=mipsel-linux-gnu \
        --disable-nls --disable-werror --disable-gdb --disable-gdbserver \
        --disable-sim --disable-gprofng --disable-libdecnumber --disable-readline \
        MAKEINFO=true >/dev/null)
    log "binutils: building with $JOBS jobs"
    make -C "$SRC/binutils-build" -j"$JOBS" MAKEINFO=true >/dev/null
    make -C "$SRC/binutils-build" install MAKEINFO=true >/dev/null
    rm -rf "$SRC/binutils-build" "$SRC/binutils-$BINUTILS_VER"
    log "binutils: installed to $prefix"
}

step_mkpsxiso() {
    local prefix="$INSTALL/mkpsxiso"
    if [[ -x "$prefix/bin/dumpsxiso" && -x "$prefix/bin/mkpsxiso" ]]; then
        log "mkpsxiso: already installed"
        return
    fi
    command -v cmake >/dev/null && command -v ninja >/dev/null || step_cmake
    mkdir -p "$SRC"
    local dir="$SRC/mkpsxiso"
    if [[ ! -d "$dir/.git" ]]; then
        log "mkpsxiso: cloning $MKPSXISO_TAG"
        git -c advice.detachedHead=false clone -q --branch "$MKPSXISO_TAG" --depth 1 --recurse-submodules --shallow-submodules \
            https://github.com/Lameguy64/mkpsxiso.git "$dir"
    fi
    log "mkpsxiso: building"
    cmake -S "$dir" -B "$dir/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$prefix" >/dev/null
    cmake --build "$dir/build" -j"$JOBS" >/dev/null
    cmake --install "$dir/build" >/dev/null
    log "mkpsxiso: installed to $prefix"
}

# SDL3 (zlib licence) for the PC port's window (port/README.md "The window"; DECISIONS "PC port architecture"):
# the pinned release tarball (signed by Sam Lantinga; the SHA-256 pins it), built with CMake into
# tools/sdl3/ as a static library. Static: the port stays one binary that runs from anywhere (no rpath, no
# LD_LIBRARY_PATH), and SDL still loads its platform libraries (X11, Wayland, ALSA, PulseAudio, PipeWire, udev, ...)
# with dlopen at run time (SDL_DEPS_SHARED), so the binary needs none of them on a headless host. Which backends get
# compiled in depends on the -dev headers present at build time: with none, only the offscreen/dummy video and the
# dummy/disk audio drivers (enough for the tests); for a desktop window install e.g. libx11-dev libxext-dev
# (libwayland-dev libxkbcommon-dev wayland-protocols) libasound2-dev libpulse-dev libudev-dev first, then rebuild with
# `rm -rf tools/sdl3 && scripts/setup.sh sdl3`. Optional (not in the default steps): scripts/setup.sh sdl3
#
# sdl3-desktop: the same SDL into tools/sdl3-desktop/, for the release (scripts/package_appimage.sh; DECISIONS
# "Releases"). It REQUIRES the desktop backends (X11, Wayland, PipeWire, PulseAudio, ALSA; SDL3_DESKTOP_APT lists
# Ubuntu's -dev packages for them) and fails instead of turning one off, and its library is checked for each backend's
# driver (nm). Its own directory, so a headless tools/sdl3 (CI's cache) never stands in for it. Optional:
# scripts/setup.sh sdl3-desktop
SDL3_VER=3.4.18
SDL3_SHA256=9c75cf16330322c217dedd2e0609f1124f1b54b8633e763467b4684d0f4334a3
# The drivers a desktop SDL must have: each one's bootstrap symbol in libSDL3.a.
SDL3_DESKTOP_DRIVERS=(X11_bootstrap Wayland_bootstrap PIPEWIRE_bootstrap PULSEAUDIO_bootstrap ALSA_bootstrap)
# Ubuntu 24.04's -dev packages for them and SDL's other desktop features (XInput2, Xrandr, libdecor, IBus, udev, KMSDRM).
SDL3_DESKTOP_APT="libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev
    libxkbcommon-dev libwayland-dev wayland-protocols libdecor-0-dev libegl1-mesa-dev libgles2-mesa-dev libgl1-mesa-dev
    libdrm-dev libgbm-dev libasound2-dev libpulse-dev libpipewire-0.3-dev libdbus-1-dev libibus-1.0-dev libudev-dev"

# Prints the drivers of SDL3_DESKTOP_DRIVERS that a libSDL3.a lacks (nothing: a desktop build).
sdl3_missing_drivers() {
    local syms d
    syms="$(nm -g --defined-only "$1" 2>/dev/null)" || { printf '%s\n' "${SDL3_DESKTOP_DRIVERS[@]}"; return; }
    for d in "${SDL3_DESKTOP_DRIVERS[@]}"; do
        grep -q " $d\$" <<<"$syms" || echo "$d"
    done
}

# sdl3_fetch NAME DIR: the pinned SDL3 source tarball (downloaded into tools/src once, SHA-256 checked) unpacked into DIR.
sdl3_fetch() {
    local name="$1" dir="$2" tarball="$SRC/SDL3-$SDL3_VER.tar.gz"
    mkdir -p "$SRC"
    if [[ ! -f "$tarball" ]] || ! echo "$SDL3_SHA256  $tarball" | sha256sum -c --quiet - 2>/dev/null; then
        log "$name: downloading $SDL3_VER"
        fetch "https://github.com/libsdl-org/SDL/releases/download/release-$SDL3_VER/SDL3-$SDL3_VER.tar.gz" \
            "$tarball.part"
        mv "$tarball.part" "$tarball"
    fi
    echo "$SDL3_SHA256  $tarball" | sha256sum -c --quiet - || die "$name: checksum mismatch"
    rm -rf "$dir"
    mkdir -p "$dir"
    tar -C "$dir" --strip-components=1 -xzf "$tarball"
}

# sdl3_build NAME PREFIX DESKTOP(0|1)
sdl3_build() {
    local name="$1" prefix="$2" desktop="$3" tarball="$SRC/SDL3-$SDL3_VER.tar.gz" dir="$SRC/SDL3-$SDL3_VER-$1"
    if [[ -f "$prefix/lib/libSDL3.a" && -f "$prefix/.sha256" && "$(cat "$prefix/.sha256")" == "$SDL3_SHA256" ]] &&
        { [[ $desktop -eq 0 ]] || [[ -z "$(sdl3_missing_drivers "$prefix/lib/libSDL3.a")" ]]; }; then
        log "$name: $SDL3_VER already installed ($prefix)"
        return
    fi
    command -v cmake >/dev/null && command -v ninja >/dev/null || step_cmake
    sdl3_fetch "$name" "$dir"
    rm -rf "$prefix"
    log "$name: configuring (static; no tests, examples or camera)"
    # SDL stops at an optional dependency whose headers are missing (an X11 extension, ...) and names the option
    # that turns it off; so does a host with neither X11 nor Wayland headers (SDL_UNIX_CONSOLE_BUILD: offscreen and
    # dummy video only). Each such feature is turned off in turn and logged; the desktop build never turns off one of
    # its backends.
    local opts=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$prefix" -DCMAKE_INSTALL_LIBDIR=lib
                -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_DEPS_SHARED=ON -DSDL_TEST_LIBRARY=OFF -DSDL_TESTS=OFF
                -DSDL_EXAMPLES=OFF -DSDL_CAMERA=OFF) off tries=0
    if [[ $desktop -eq 1 ]]; then
        opts+=(-DSDL_X11=ON -DSDL_WAYLAND=ON -DSDL_PIPEWIRE=ON -DSDL_PULSEAUDIO=ON -DSDL_ALSA=ON)
    fi
    until cmake -S "$dir" -B "$dir/build" -G Ninja "${opts[@]}" >"$dir/configure.log" 2>&1; do
        off="$(grep -o -- '-DSDL_[A-Z0-9_]*=OFF' "$dir/configure.log" | head -1)" || off=
        if [[ -z "$off" ]] && grep -q 'X11 or Wayland' "$dir/configure.log"; then
            off=-DSDL_UNIX_CONSOLE_BUILD=ON
        fi
        if [[ $desktop -eq 1 && "$off" =~ ^-DSDL_(X11|WAYLAND|PIPEWIRE|PULSEAUDIO|ALSA|UNIX_CONSOLE_BUILD)= ]]; then
            die "$name: needs ${BASH_REMATCH[1]}'s headers (Ubuntu: apt-get install $(echo $SDL3_DESKTOP_APT));" \
                "see $dir/configure.log"
        fi
        tries=$((tries + 1))
        [[ -n "$off" && $tries -le 30 ]] || die "$name: configure failed (see $dir/configure.log)"
        log "$name: headers missing for an optional feature: $off"
        opts+=("$off")
    done
    # SDL's own summary of what it found: the video and audio drivers this build has.
    grep -E '^--   (Video|Audio|Joystick) drivers:' "$dir/configure.log" | sed "s/^-- */  $name: /" || true
    log "$name: building with $JOBS jobs"
    cmake --build "$dir/build" -j"$JOBS" >/dev/null
    cmake --install "$dir/build" >/dev/null
    rm -rf "$dir"
    [[ -f "$prefix/lib/libSDL3.a" && -f "$prefix/lib/cmake/SDL3/SDL3Config.cmake" ]] || die "$name: install incomplete"
    if [[ $desktop -eq 1 ]]; then
        local missing
        missing="$(sdl3_missing_drivers "$prefix/lib/libSDL3.a" | tr '\n' ' ')"
        [[ -z "$missing" ]] || die "$name: built without ${missing}(Ubuntu: apt-get install $(echo $SDL3_DESKTOP_APT))"
    fi
    echo "$SDL3_SHA256" > "$prefix/.sha256"
    log "$name: installed $SDL3_VER to $prefix"
}
step_sdl3() { sdl3_build sdl3 "$INSTALL/sdl3" 0; }
step_sdl3-desktop() { sdl3_build sdl3-desktop "$INSTALL/sdl3-desktop" 1; }

# llvm-mingw (Apache-2.0 with LLVM exceptions; mingw-w64 runtime under its own permissive licences): clang, lld and the
# UCRT mingw-w64 sysroot, the Windows cross toolchain (DECISIONS "Windows"; cmake/windows-x86_64.cmake is the CMake
# toolchain file, scripts/build_windows.sh builds the game and the launcher with it). One pinned release tarball for
# Linux x86_64, SHA-256 checked, unpacked into tools/llvm-mingw. Self-contained: no system package, the same build
# here and on CI. Optional (not in the default steps): scripts/setup.sh llvm-mingw
LLVM_MINGW_VER=20260922
LLVM_MINGW_NAME=llvm-mingw-$LLVM_MINGW_VER-ucrt-ubuntu-22.04-x86_64
LLVM_MINGW_SHA256=bb7bb7654b33d5aa8712acb837c963b2e0c56352560c76105270a3268c665c21
step_llvm-mingw() {
    local dir="$INSTALL/llvm-mingw" tarball="$SRC/$LLVM_MINGW_NAME.tar.xz" unpack="$SRC/$LLVM_MINGW_NAME"
    if [[ -x "$dir/bin/x86_64-w64-mingw32-clang" && -f "$dir/.sha256" && "$(cat "$dir/.sha256")" == "$LLVM_MINGW_SHA256" ]]; then
        log "llvm-mingw: $LLVM_MINGW_VER already installed ($dir)"
        return
    fi
    [[ "$(uname -s)-$(uname -m)" == Linux-x86_64 ]] || die "llvm-mingw: the pinned tarball is a Linux x86_64 build"
    mkdir -p "$SRC"
    if [[ ! -f "$tarball" ]] || ! echo "$LLVM_MINGW_SHA256  $tarball" | sha256sum -c --quiet - 2>/dev/null; then
        log "llvm-mingw: downloading $LLVM_MINGW_VER (~80 MB)"
        fetch "https://github.com/mstorsjo/llvm-mingw/releases/download/$LLVM_MINGW_VER/$LLVM_MINGW_NAME.tar.xz" \
            "$tarball.part"
        mv "$tarball.part" "$tarball"
    fi
    echo "$LLVM_MINGW_SHA256  $tarball" | sha256sum -c --quiet - || die "llvm-mingw: checksum mismatch"
    rm -rf "$dir" "$unpack"
    mkdir -p "$unpack"
    log "llvm-mingw: unpacking"
    tar -C "$unpack" --strip-components=1 -xJf "$tarball"
    [[ -x "$unpack/bin/x86_64-w64-mingw32-clang" ]] || die "llvm-mingw: no x86_64-w64-mingw32-clang in the tarball"
    mv "$unpack" "$dir"
    "$dir/bin/x86_64-w64-mingw32-clang" --version >/dev/null || die "llvm-mingw: the compiler does not run"
    echo "$LLVM_MINGW_SHA256" > "$dir/.sha256"
    log "llvm-mingw: installed $LLVM_MINGW_VER to $dir ($("$dir/bin/x86_64-w64-mingw32-clang" --version | head -1))"
}

# sdl3-windows: the same pinned SDL3, cross-built static for Windows x86_64 with llvm-mingw (the toolchain file) into
# tools/sdl3-windows, the drivers SDL picks for Windows by default (Windows video, WASAPI/DirectSound audio, XInput,
# raw input...). Optional: scripts/setup.sh llvm-mingw sdl3-windows
step_sdl3-windows() {
    local name=sdl3-windows prefix="$INSTALL/sdl3-windows" dir="$SRC/SDL3-$SDL3_VER-sdl3-windows"
    if [[ -f "$prefix/lib/libSDL3.a" && -f "$prefix/.sha256" && "$(cat "$prefix/.sha256")" == "$SDL3_SHA256" ]]; then
        log "$name: $SDL3_VER already installed ($prefix)"
        return
    fi
    [[ -x "$INSTALL/llvm-mingw/bin/x86_64-w64-mingw32-clang" ]] || step_llvm-mingw
    command -v cmake >/dev/null && command -v ninja >/dev/null || step_cmake
    sdl3_fetch "$name" "$dir"
    rm -rf "$prefix"
    log "$name: configuring (static, Windows x86_64; no tests, examples or camera)"
    DW3_LLVM_MINGW="$INSTALL/llvm-mingw" cmake -S "$dir" -B "$dir/build" -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/windows-x86_64.cmake" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$prefix" -DCMAKE_INSTALL_LIBDIR=lib \
        -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST_LIBRARY=OFF -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF -DSDL_CAMERA=OFF \
        >"$dir/configure.log" 2>&1 || die "$name: configure failed (see $dir/configure.log)"
    grep -E '^--   (Video|Audio|Joystick) drivers:' "$dir/configure.log" | sed "s/^-- */  $name: /" || true
    log "$name: building with $JOBS jobs"
    cmake --build "$dir/build" -j"$JOBS" >"$dir/build.log" 2>&1 || die "$name: build failed (see $dir/build.log)"
    cmake --install "$dir/build" >/dev/null
    rm -rf "$dir"
    [[ -f "$prefix/lib/libSDL3.a" && -f "$prefix/lib/cmake/SDL3/SDL3Config.cmake" ]] || die "$name: install incomplete"
    echo "$SDL3_SHA256" > "$prefix/.sha256"
    log "$name: installed $SDL3_VER to $prefix"
}

# The AppImage tools for the release (scripts/package_appimage.sh; DECISIONS "Releases: tagged drafts, published by hand"): appimagetool (MIT) and the
# static type-2 runtime (MIT, with musl, libfuse 3 (LGPL-2.1), squashfuse, zstd and zlib linked in: the AppImage needs
# no libfuse2 on the player's machine), each a pinned release asset checked by SHA-256, and the runtime's LICENSE at
# its tag's commit (bundled in the AppImage's LICENSES/). appimagetool is itself an AppImage: it is unpacked once
# (--appimage-extract needs no FUSE) and run as tools/appimage/appimagetool/AppRun. Optional: scripts/setup.sh appimage
APPIMAGETOOL_VER=1.9.1
APPIMAGETOOL_SHA256=ed4ce84f0d9caff66f50bcca6ff6f35aae54ce8135408b3fa33abfc3cb384eb0
APPIMAGE_RUNTIME_VER=20251108
APPIMAGE_RUNTIME_COMMIT=dd6cebedcbddde9c82f89b011e8e1d40b6e43868
APPIMAGE_RUNTIME_SHA256=2fca8b443c92510f1483a883f60061ad09b46b978b2631c807cd873a47ec260d
APPIMAGE_RUNTIME_LICENSE_SHA256=aa154fc9070614bbe7921f89db11efd1dba7a1f3a41685958110e2230f9c0ca1
appimage_get() { # URL FILE SHA256
    log "appimage: downloading $(basename "$2")"
    fetch "$1" "$2.part"
    echo "$3  $2.part" | sha256sum -c --quiet - || die "appimage: checksum mismatch: $1"
    mv "$2.part" "$2"
}
step_appimage() {
    local dir="$INSTALL/appimage" stamp="$APPIMAGETOOL_SHA256 $APPIMAGE_RUNTIME_SHA256 $APPIMAGE_RUNTIME_LICENSE_SHA256"
    if [[ -x "$dir/appimagetool/AppRun" && -f "$dir/runtime-x86_64" && -f "$dir/runtime-LICENSE" &&
          -f "$dir/.sha256" && "$(cat "$dir/.sha256")" == "$stamp" ]]; then
        log "appimage: appimagetool $APPIMAGETOOL_VER, runtime $APPIMAGE_RUNTIME_VER already installed ($dir)"
        return
    fi
    [[ "$(uname -m)" == x86_64 ]] || die "appimage: the pinned tools are x86_64 builds"
    rm -rf "$dir"
    mkdir -p "$dir"
    appimage_get "https://github.com/AppImage/appimagetool/releases/download/$APPIMAGETOOL_VER/appimagetool-x86_64.AppImage" \
        "$dir/appimagetool-x86_64.AppImage" "$APPIMAGETOOL_SHA256"
    appimage_get "https://github.com/AppImage/type2-runtime/releases/download/$APPIMAGE_RUNTIME_VER/runtime-x86_64" \
        "$dir/runtime-x86_64" "$APPIMAGE_RUNTIME_SHA256"
    appimage_get "https://raw.githubusercontent.com/AppImage/type2-runtime/$APPIMAGE_RUNTIME_COMMIT/LICENSE" \
        "$dir/runtime-LICENSE" "$APPIMAGE_RUNTIME_LICENSE_SHA256"
    chmod +x "$dir/appimagetool-x86_64.AppImage"
    (cd "$dir" && ./appimagetool-x86_64.AppImage --appimage-extract >/dev/null) || die "appimage: unpacking appimagetool failed"
    mv "$dir/squashfs-root" "$dir/appimagetool"
    rm "$dir/appimagetool-x86_64.AppImage"
    [[ -x "$dir/appimagetool/AppRun" ]] || die "appimage: appimagetool has no AppRun"
    echo "$stamp" > "$dir/.sha256"
    log "appimage: installed appimagetool $APPIMAGETOOL_VER and runtime $APPIMAGE_RUNTIME_VER to $dir"
}

# Dear ImGui (MIT) for the launcher (launcher/README.md; DECISIONS "Launcher and mods"): the pinned
# release tag cloned into tools/imgui/, its commit checked (the commit hash is the checksum, as for the ext step). The
# launcher's CMake compiles its core files and the SDL3 + SDL_Renderer backends from there. Optional (not in the
# default steps; the launcher needs sdl3 too): scripts/setup.sh sdl3 imgui
IMGUI_VER=1.92.9b
IMGUI_COMMIT=f1cc2ae15e53a861a874c3034aae6798fde194ab
step_imgui() {
    local dir="$INSTALL/imgui"
    if [[ -f "$dir/imgui.cpp" && -d "$dir/.git" && "$(git -C "$dir" rev-parse HEAD)" == "$IMGUI_COMMIT" ]]; then
        log "imgui: $IMGUI_VER already installed ($dir)"
        return
    fi
    rm -rf "$dir"
    log "imgui: cloning v$IMGUI_VER"
    git -c advice.detachedHead=false clone -q --depth 1 --branch "v$IMGUI_VER" https://github.com/ocornut/imgui.git "$dir"
    [[ "$(git -C "$dir" rev-parse HEAD)" == "$IMGUI_COMMIT" ]] || die "imgui: v$IMGUI_VER is not at $IMGUI_COMMIT"
    [[ -f "$dir/backends/imgui_impl_sdl3.cpp" && -f "$dir/backends/imgui_impl_sdlrenderer3.cpp" ]] ||
        die "imgui: the SDL3 backends are missing"
    log "imgui: installed $IMGUI_VER (${IMGUI_COMMIT:0:12}) to $dir"
}

# CMake and Ninja from PyPI (official wheels), pinned, into the venv: only when the system has none (no sudo).
CMAKE_PIP=cmake==3.31.10
NINJA_PIP=ninja==1.13.0
step_cmake() {
    local venv="$INSTALL/venv"
    if command -v cmake >/dev/null && command -v ninja >/dev/null; then
        log "cmake: cmake and ninja on PATH ($(command -v cmake), $(command -v ninja))"
        return
    fi
    [[ -x "$venv/bin/pip" ]] || step_venv
    if [[ ! -x "$venv/bin/cmake" || ! -x "$venv/bin/ninja" ]]; then
        log "cmake: installing $CMAKE_PIP $NINJA_PIP into tools/venv (missing from PATH)"
        "$venv/bin/pip" install -q "$CMAKE_PIP" "$NINJA_PIP"
    fi
    export PATH="$PATH:$venv/bin"   # after the system's: the venv's python stays out of the way
    log "cmake: using $(command -v cmake), $(command -v ninja) (tools/venv)"
}

step_venv() {
    local venv="$INSTALL/venv"
    if [[ ! -x "$venv/bin/python" ]]; then
        log "venv: creating with $(python3 --version)"
        python3 -m venv "$venv"
    fi
    log "venv: installing tools/requirements.txt"
    "$venv/bin/pip" install -q --upgrade pip
    "$venv/bin/pip" install -q -r "$TOOLS/requirements.txt"
}

# old-gcc PSX builds into tools/gcc/<version>/ (cpp, cc1, gcc, ...).
step_gcc() {
    local entry ver sha dir tarball
    mkdir -p "$SRC" "$INSTALL/gcc"
    for entry in "${OLDGCC[@]}"; do
        read -r ver sha <<<"$entry"
        dir="$INSTALL/gcc/$ver"
        if [[ -x "$dir/cc1" && -f "$dir/.sha256" && "$(cat "$dir/.sha256")" == "$sha" ]]; then
            log "gcc: $ver already installed"
            continue
        fi
        tarball="$SRC/gcc-$ver-psx.tar.gz"
        if [[ ! -f "$tarball" ]] || ! echo "$sha  $tarball" | sha256sum -c --quiet - 2>/dev/null; then
            log "gcc: downloading $ver (old-gcc $OLDGCC_TAG)"
            fetch "https://github.com/decompals/old-gcc/releases/download/$OLDGCC_TAG/gcc-$ver-psx.tar.gz" \
                "$tarball.part"
            mv "$tarball.part" "$tarball"
        fi
        echo "$sha  $tarball" | sha256sum -c --quiet - || die "gcc: checksum mismatch for $ver"
        rm -rf "$dir"
        mkdir -p "$dir"
        tar -C "$dir" -xzf "$tarball"
        echo "$sha" > "$dir/.sha256"
        log "gcc: installed $ver to $dir"
    done
}

# objdiff-cli (prebuilt, checksum-pinned) into tools/bin/.
# Optional (not in the default steps): Sony's Psy-Q 4.3/4.4 compilers and assembler (the binaries decomp.me uses,
# from mkst/esa's releases), run through wibo; for checking the open toolchain against the real one (DECISIONS
# "Psy-Q check"). Never committed (tools/psyq is gitignored). Usage: scripts/setup.sh psyq
PSYQ=(
    "psyq4.3.tar.gz https://github.com/mkst/esa/releases/download/psyq-binaries/psyq4.3.tar.gz 577038d66507d3aa5423de0ba3f540e121a4f60f637f4794aa4350135d4f9a46"
    "psyq4.4.tar.gz https://github.com/mkst/esa/releases/download/psyq-binaries/psyq4.4.tar.gz 72e73934bab0d51933eb95af514afb14f3d432f01530eac3ddb16dfbb57ab66c"
    "psyq-obj-parser.tar.gz https://github.com/decompme/compilers/releases/download/compilers/psyq-obj-parser.tar.gz 353495f13f6756773cd905ba3a0743843232f95719e306fc6ee56cbf7d731a3e"
    "wibo https://github.com/decompals/wibo/releases/download/0.6.16/wibo 8a8490a6172aa4f0f6ddcadb144ca96f51da6e90e6648ce9adaf4f6babb6e00b"
)
step_psyq() {
    local dir="$INSTALL/psyq" entry name url sha
    mkdir -p "$dir"
    for entry in "${PSYQ[@]}"; do
        read -r name url sha <<<"$entry"
        if [[ ! -f "$dir/$name" ]] || ! echo "$sha  $dir/$name" | sha256sum -c --quiet - 2>/dev/null; then
            log "psyq: downloading $name"
            fetch "$url" "$dir/$name.part"
            echo "$sha  $dir/$name.part" | sha256sum -c --quiet - || die "psyq: checksum mismatch for $name"
            mv "$dir/$name.part" "$dir/$name"
        fi
        [[ "$name" == *.tar.gz ]] && tar -C "$dir" -xzf "$dir/$name"
    done
    chmod +x "$dir/wibo" "$dir/psyq-obj-parser"
    log "psyq: installed to $dir (wibo $dir/psyq4.4/CC1PSX.EXE ...)"
}

step_objdiff() {
    local bin="$INSTALL/bin/objdiff-cli"
    if [[ -x "$bin" ]] && echo "$OBJDIFF_CLI_SHA256  $bin" | sha256sum -c --quiet - 2>/dev/null; then
        log "objdiff: already installed ($("$bin" --version))"
        return
    fi
    mkdir -p "$INSTALL/bin"
    log "objdiff: downloading objdiff-cli $OBJDIFF_VER"
    fetch "https://github.com/encounter/objdiff/releases/download/v$OBJDIFF_VER/objdiff-cli-linux-x86_64" "$bin.part"
    echo "$OBJDIFF_CLI_SHA256  $bin.part" | sha256sum -c --quiet - || die "objdiff: checksum mismatch"
    chmod +x "$bin.part"
    mv "$bin.part" "$bin"
    log "objdiff: installed $("$bin" --version)"
}

# Pinned git clones into tools/ext/<name>/. The commit hash is the checksum.
step_ext() {
    local entry name url commit dir
    mkdir -p "$INSTALL/ext"
    for entry in "${EXT[@]}"; do
        read -r name url commit <<<"$entry"
        dir="$INSTALL/ext/$name"
        if [[ -d "$dir/.git" && "$(git -C "$dir" rev-parse HEAD)" == "$commit" ]]; then
            log "ext: $name already at ${commit:0:12}"
            continue
        fi
        if [[ ! -d "$dir/.git" ]]; then
            log "ext: cloning $name"
            git clone -q "$url" "$dir"
        fi
        git -C "$dir" fetch -q origin "$commit" 2>/dev/null || git -C "$dir" fetch -q origin
        git -c advice.detachedHead=false -C "$dir" checkout -q "$commit"
        [[ "$(git -C "$dir" rev-parse HEAD)" == "$commit" ]] || die "ext: $name is not at $commit"
        log "ext: $name at ${commit:0:12}"
    done
}

# Symlink the user's own disc image into iso/ (never copied into git).
step_disc() {
    local bin="${DISC_PATH:-}"
    if [[ -z "$bin" && -f "$INSTALL/local.env" ]]; then
        # shellcheck disable=SC1091
        source "$INSTALL/local.env"
        bin="${DW3_DISC_BIN:-}"
    fi
    if [[ -z "$bin" ]]; then
        log "disc: no disc given (use --disc or set DW3_DISC_BIN in tools/local.env); skipping"
        return
    fi
    [[ -f "$bin" ]] || die "disc: not found: $bin"
    log "disc: verifying SHA-1 (this reads ~690 MB)"
    local sha
    sha="$(sha1sum "$bin" | cut -d' ' -f1)"
    if [[ "$sha" != "$DISC_SHA1" ]]; then
        die "disc: SHA-1 $sha does not match the expected image $DISC_SHA1"
    fi
    mkdir -p "$ROOT/iso"
    ln -sfn "$bin" "$ROOT/iso/dw2003.bin"
    cat > "$ROOT/iso/dw2003.cue" <<'EOF'
FILE "dw2003.bin" BINARY
  TRACK 01 MODE2/2352
    INDEX 01 00:00:00
EOF
    log "disc: linked iso/dw2003.bin -> $bin"
}

# Optional data checkout (scripts/gamedata_dir.sh: $DW3_GAMEDATA, tools/local.env, or a sibling dw2003-gamedata/):
# its gamedata/ may hold the disc image as xz parts; rebuild iso/dw2003.bin from them and check its SHA-1. Without
# parts, worktree_init.sh links its gamedata/disc. Public checkouts use their own disc instead (step disc).
step_gamedata() {
    local gd parts sha
    gd="$("$ROOT/scripts/gamedata_dir.sh" 2>/dev/null)" || gd=
    if [[ -z "$gd" || ! -d "$gd/gamedata" ]]; then
        log "gamedata: no data checkout (optional; DW3_GAMEDATA in tools/local.env); skipping"
        return
    fi
    gd="$gd/gamedata"
    if [[ -e "$ROOT/iso/dw2003.bin" ]]; then
        log "gamedata: iso/dw2003.bin already present"
        return
    fi
    shopt -s nullglob
    parts=("$gd"/dw2003.bin.xz.part*)
    shopt -u nullglob
    if [[ ${#parts[@]} -eq 0 ]]; then
        log "gamedata: no disc image parts in $gd; only its disc/ is available"
        return
    fi
    mkdir -p "$ROOT/iso"
    log "gamedata: decompressing ${#parts[@]} parts into iso/dw2003.bin"
    cat "${parts[@]}" | xz -dc -T0 > "$ROOT/iso/dw2003.bin.part"
    sha="$(sha1sum "$ROOT/iso/dw2003.bin.part" | cut -d' ' -f1)"
    [[ "$sha" == "$DISC_SHA1" ]] || die "gamedata: SHA-1 $sha does not match the expected image $DISC_SHA1"
    mv "$ROOT/iso/dw2003.bin.part" "$ROOT/iso/dw2003.bin"
    cat > "$ROOT/iso/dw2003.cue" <<'EOF'
FILE "dw2003.bin" BINARY
  TRACK 01 MODE2/2352
    INDEX 01 00:00:00
EOF
    log "gamedata: iso/dw2003.bin verified"
}

# PCSX-Redux into tools/redux/: app/ (the AppImage's contents), sysroot/ (only when the host glibc is too old) and the
# wrapper tools/redux/pcsx-redux. Source: tools/prebuilt/$REDUX_ZIP, or the AppImage named by PCSX_REDUX in
# tools/local.env when the zip is absent (public checkouts). Test: scripts/check_emulator.sh.
step_redux() {
    local dir="$INSTALL/redux" gd zip src sha appimage entry file want deb
    gd="$("$ROOT/scripts/gamedata_dir.sh" 2>/dev/null)" || gd=
    zip="${gd:+$gd/tools/prebuilt/$REDUX_ZIP}"
    if [[ -x "$dir/pcsx-redux" && -f "$dir/.sha256" && "$(cat "$dir/.sha256")" == "$REDUX_SHA256" ]]; then
        log "redux: already installed ($dir/pcsx-redux)"
        return
    fi
    if [[ -n "$zip" && -f "$zip" ]]; then
        src="$zip"; sha="$REDUX_SHA256"
    elif [[ -f "$SRC/$REDUX_ZIP" ]] && echo "$REDUX_SHA256  $SRC/$REDUX_ZIP" | sha256sum -c --quiet - 2>/dev/null; then
        src="$SRC/$REDUX_ZIP"; sha="$REDUX_SHA256"
    elif mkdir -p "$SRC" && log "redux: downloading $REDUX_ZIP (86 MB)" && fetch "$REDUX_URL" "$SRC/$REDUX_ZIP.part"; then
        mv "$SRC/$REDUX_ZIP.part" "$SRC/$REDUX_ZIP"
        src="$SRC/$REDUX_ZIP"; sha="$REDUX_SHA256"
    else
        rm -f "$SRC/$REDUX_ZIP.part"
        [[ -f "$INSTALL/local.env" ]] && source "$INSTALL/local.env"
        src="${PCSX_REDUX:-}"
        [[ -f "$src" ]] || { log "redux: no data checkout, no download and no PCSX_REDUX AppImage in tools/local.env; skipping"; return; }
        sha="$(sha256sum "$src" | cut -d' ' -f1)"
    fi
    if [[ -x "$dir/pcsx-redux" && -f "$dir/.sha256" && "$(cat "$dir/.sha256")" == "$sha" ]]; then
        log "redux: already installed ($dir/pcsx-redux)"
        return
    fi
    echo "$sha  $src" | sha256sum -c --quiet - || die "redux: checksum mismatch for $src"
    rm -rf "$dir" "$SRC/redux-unpack"
    mkdir -p "$dir" "$SRC/redux-unpack"
    if [[ "$src" == *.zip ]]; then
        log "redux: unpacking $(basename "$src")"
        unzip -q "$src" -d "$SRC/redux-unpack"
        appimage="$(find "$SRC/redux-unpack" -name '*.AppImage' | head -1)"
    else
        appimage="$src"
    fi
    [[ -n "$appimage" ]] || die "redux: no AppImage found in $src"
    chmod +x "$appimage"
    (cd "$SRC/redux-unpack" && "$appimage" --appimage-extract >/dev/null)   # no FUSE needed
    mv "$SRC/redux-unpack/squashfs-root" "$dir/app"
    rm -rf "$SRC/redux-unpack"
    [[ -x "$dir/app/usr/bin/pcsx-redux" ]] || die "redux: app/usr/bin/pcsx-redux missing after extraction"
    if (unset DISPLAY WAYLAND_DISPLAY; cd "$dir/app/usr/bin" && ./pcsx-redux -version >/dev/null 2>&1); then
        log "redux: the AppImage runs natively (host glibc $(ldd --version | head -1 | awk '{print $NF}'))"
    else
        log "redux: host glibc $(ldd --version | head -1 | awk '{print $NF}') is too old; unpacking the pinned runtime sysroot"
        mkdir -p "$SRC/redux-debs" "$dir/sysroot"
        for entry in "${REDUX_SYSROOT_DEBS[@]}"; do
            read -r file want <<<"$entry"
            deb="$SRC/redux-debs/$(basename "$file")"
            if [[ ! -f "$deb" ]] || ! echo "$want  $deb" | sha256sum -c --quiet - 2>/dev/null; then
                fetch "$REDUX_UBUNTU/$file" "$deb.part"
                mv "$deb.part" "$deb"
            fi
            echo "$want  $deb" | sha256sum -c --quiet - || die "redux: checksum mismatch for $(basename "$file")"
            dpkg-deb -x "$deb" "$dir/sysroot"
        done
        [[ -x "$dir/sysroot/usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2" ]] || die "redux: sysroot has no loader"
    fi
    cat > "$dir/pcsx-redux" <<'EOF'
#!/usr/bin/env bash
# Generated by scripts/setup.sh redux: runs the pinned PCSX-Redux (app/) through the pinned glibc sysroot when the
# host's glibc is too old for it. Headless use: pcsx-redux -no-ui -stdout -testmode -run -iso <cue> -bios <bin> -dofile <lua>
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ -d "$here/sysroot" ]]; then
    exec "$here/sysroot/usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2" \
        --library-path "$here/sysroot/usr/lib/x86_64-linux-gnu:$here/app/usr/lib" "$here/app/usr/bin/pcsx-redux" "$@"
fi
exec "$here/app/usr/bin/pcsx-redux" "$@"
EOF
    chmod +x "$dir/pcsx-redux"
    echo "$sha" > "$dir/.sha256"
    log "redux: installed to $dir (openbios: app/usr/share/pcsx-redux/resources/openbios.bin)"
}

# In a worktree: symlink every gitignored entry of the main checkout's tools/ that is
# missing here (built tools, local.env). No-op in the main checkout.
step_link() {
    [[ "$MAIN" != "$ROOT" ]] || return 0
    local src name dst
    for src in "$INSTALL"/* "$INSTALL"/.[!.]*; do
        [[ -e "$src" ]] || continue
        name="$(basename "$src")"
        dst="$TOOLS/$name"
        [[ -e "$dst" || -L "$dst" ]] && continue
        # this checkout's .gitignore: a branch that adds a tool knows its directory before main does
        if git -C "$ROOT" check-ignore -q "tools/$name"; then
            ln -s "$src" "$dst"
            log "link: tools/$name -> $src"
        fi
    done
}

DISC_PATH=""
steps=()
while [[ $# -gt 0 ]]; do
    case "$1" in
        --disc) DISC_PATH="$2"; shift 2 ;;
        --sdl3-desktop-apt) echo $SDL3_DESKTOP_APT; exit 0 ;;   # what release.yml installs before sdl3-desktop
        -h|--help) sed -n '2,19p' "$0"; exit 0 ;;
        *) steps+=("$1"); shift ;;
    esac
done
[[ ${#steps[@]} -gt 0 ]] || steps=(binutils venv cmake mkpsxiso gcc objdiff ext redux link gamedata disc)

for s in "${steps[@]}"; do
    declare -F "step_$s" >/dev/null || die "unknown step: $s"
    "step_$s"
done
# Anything just installed into the main checkout becomes visible in this worktree.
step_link
log "done"
