#!/usr/bin/env bash
# Project-local toolchain setup. Idempotent: re-running skips finished steps.
# Nothing here needs sudo or installs outside the repo.
#
# Usage: scripts/setup.sh [--disc /path/to/disc.bin] [step...]
#   steps: binutils mkpsxiso venv gcc objdiff ext redux link gamedata disc  (default: all); optional: psyq
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
JOBS="$(nproc)"

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
        curl -sSfL -o "$tarball.part" "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.xz" ||
            curl -sSfL -o "$tarball.part" \
                "http://archive.ubuntu.com/ubuntu/pool/main/b/binutils/binutils_$BINUTILS_VER.orig.tar.xz"
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
            curl -sSfL -o "$tarball.part" \
                "https://github.com/decompals/old-gcc/releases/download/$OLDGCC_TAG/gcc-$ver-psx.tar.gz"
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
            curl -sSfL -o "$dir/$name.part" "$url"
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
    curl -sSfL -o "$bin.part" \
        "https://github.com/encounter/objdiff/releases/download/v$OBJDIFF_VER/objdiff-cli-linux-x86_64"
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
    elif mkdir -p "$SRC" && log "redux: downloading $REDUX_ZIP (86 MB)" && curl -sSfL -o "$SRC/$REDUX_ZIP.part" "$REDUX_URL"; then
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
                curl -sSfL -o "$deb.part" "$REDUX_UBUNTU/$file"
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
        if git -C "$MAIN" check-ignore -q "tools/$name"; then
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
        -h|--help) sed -n '2,11p' "$0"; exit 0 ;;
        *) steps+=("$1"); shift ;;
    esac
done
[[ ${#steps[@]} -gt 0 ]] || steps=(binutils mkpsxiso venv gcc objdiff ext redux link gamedata disc)

for s in "${steps[@]}"; do
    declare -F "step_$s" >/dev/null || die "unknown step: $s"
    "step_$s"
done
# Anything just installed into the main checkout becomes visible in this worktree.
step_link
log "done"
