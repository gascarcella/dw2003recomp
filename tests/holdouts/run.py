#!/usr/bin/env python3
"""Holdout validation by run: the -DNON_MATCHING build replayed through the layer-2 scripts.

  tests/holdouts/run.sh [--out DIR] [--scripts NAME ...] [--control] [--no-replay] [--no-probe]

DECISIONS "Reference tests: three layers": "The -DNON_MATCHING build replayed through the holdouts' scenes validates the
WIP C before the port." The holdouts are the functions whose C is under `#ifdef NON_MATCHING` (the `#else` keeps the
original's asm). This script:

1. compiles every C file with a holdout again with -DNON_MATCHING (tools/cc_psx.sh, the build's flags) into DIR;
2. relinks the overlays that hold them with the matching build's linker scripts, the new objects swapped in. A WIP
   function of another size moves everything after it in its overlay, so what points into a moved overlay is
   relinked or patched too: a tier-1 parent's tier-2 children (FIELDSTG -> the 293 WSTAG files, FIGHTSTG ->
   WFIGHTMN/WFIGHTTS) against the parent's new symbols; the parent against a changed child's new addresses
   (configure.py's tier2_calls); the EXE, whose absolute overlay symbols (gamestate.c's calls into FIELDSTG) are
   relinked with the new addresses and whose `overlay_entries` (plain entry addresses) are patched. Guards: every
   word/lui/jal of a relinked file that holds an address of a moved range carries a relocation (else the relink could
   not move it: refused), and no other EXE word holds the old address of a moved symbol (reported);
3. rebuilds a disc image in DIR with mkpsxiso from extracted/dw2003.xml, the changed files swapped in. The game finds
   files by the EXE's file table (LBA, sectors): if a file needs more or fewer sectors, the files after it move, so the
   table in the EXE copy is patched from the new image's directories and the image is built again;
4. replays every recorded layer-2 script (tests/replay/scripts with an expected file) against that image twice: on the
   default dynarec core, compared with the expected record in full (frames, hashes, inputs) and in the cross-core view
   (checkpoint names, stages, maps, stable hashes; overlay and map sequences), and on -debugger -interpreter with the
   holdout probe (tools/coverage.lua armed on the eight functions only, in the NON_MATCHING files), compared in the
   cross-core view and recording which holdouts ran;
5. reports per holdout: compiled (size against the original), which scripts ran it, and validated (every script that
   ran it reproduced its checkpoints) / diverged / not reached. DIR/report.json has the details.

--control runs steps 1-3 without -DNON_MATCHING first and requires the original image back, bit for bit (a check of
the procedure itself). Needs the matching build (scripts/build.sh: build/<target>/ linker scripts and objects), the
emulator (scripts/setup.sh redux) and the disc. Writes nothing outside DIR (default $TMPDIR/dw3_holdouts, ~0.7 GB: the
image). Not part of scripts/test.sh (tests/README.md "Holdout validation": its cost).
Exit: 0 when every replay reproduces its checkpoints (cross-core view), 1 otherwise, 2 on a missing tool or build.
"""
import argparse
import filecmp
import hashlib
import importlib.util
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BINUTILS = ROOT / "tools/binutils/bin/mipsel-linux-gnu-"
MKPSXISO = ROOT / "tools/mkpsxiso/bin/mkpsxiso"
CC = ROOT / "tools/cc_psx.sh"
XML = ROOT / "extracted/dw2003.xml"
DISC = ROOT / "extracted/disc"


def load_module(name, path):
    if name in sys.modules:
        return sys.modules[name]
    sys.path.insert(0, str(path.parent))
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    sys.modules[name] = mod
    spec.loader.exec_module(mod)
    return mod


configure = load_module("configure", ROOT / "configure.py")
disc_files = load_module("disc_files", ROOT / "tools/disc_files.py")


def run(cmd, **kw):
    return subprocess.run([str(c) for c in cmd], check=True, cwd=ROOT, **kw)


# ---------------------------------------------------------------------------------------------------------- holdouts

def find_holdouts():
    """[(c file relative to ROOT, function name, line)] for every `#ifdef NON_MATCHING` block in src/."""
    out = []
    for c in sorted((ROOT / "src").rglob("*.c")):
        lines = c.read_text(errors="replace").splitlines()
        for i, line in enumerate(lines):
            if line.startswith("#ifdef NON_MATCHING"):
                for j in range(i + 1, len(lines)):
                    m = re.match(r'INCLUDE_ASM\("[^"]*",\s*(\w+)\)', lines[j])
                    if m:
                        out.append((str(c.relative_to(ROOT)), m.group(1), i + 1))
                        break
    return out


def target_of(src):
    """The link target (configure.TARGETS) of a C file src/<dir>/<stem>.c."""
    parts = Path(src).parts
    if parts[1] == "wstag":
        return Path(src).stem.split("_")[0]
    return parts[1]


TARGETS = {t.name: t for t in configure.TARGETS}
CHILDREN = {}
for _child, _parent in configure.TIER2_PARENT.items():
    CHILDREN.setdefault(_parent, []).append(_child.lower())


def obj_of(src, target):
    return f"build/{target}/{src}.o"


def nm_globals(elf, lo, hi):
    """{name: addr} of the ELF's defined global symbols in [lo, hi)."""
    out = {}
    text = run([f"{BINUTILS}nm", "--defined-only", "-g", elf], capture_output=True, text=True).stdout
    for line in text.splitlines():
        p = line.split()
        if len(p) == 3 and lo <= int(p[0], 16) < hi:
            out[p[2]] = int(p[0], 16)
    return out


def elf_symbols(elf):
    """{name: (addr, size)} of every symbol of an ELF (functions and objects, local ones too)."""
    out = {}
    text = run([f"{BINUTILS}readelf", "-sW", elf], capture_output=True, text=True).stdout
    for line in text.splitlines():
        p = line.split()
        if len(p) >= 8 and p[3] in ("FUNC", "OBJECT", "NOTYPE") and p[6] not in ("UND", "ABS"):
            out.setdefault(p[7], (int(p[1], 16), int(p[2])))
    return out


def section_size(elf):
    """Size of the ELF's one loadable output section (splat's .<target> section)."""
    text = run([f"{BINUTILS}readelf", "-SW", elf], capture_output=True, text=True).stdout
    sizes = [int(m.group(1), 16) for m in re.finditer(r"PROGBITS\s+[0-9a-f]{8}\s+[0-9a-f]+\s+([0-9a-f]+)", text)]
    if len(sizes) != 1:
        raise RuntimeError(f"{elf}: expected one PROGBITS section, found {len(sizes)}")
    return sizes[0]


def syms_ld(symbols):
    return "".join(f"{n} = 0x{a:08x};\n" for n, a in sorted(symbols.items(), key=lambda x: (x[1], x[0])))


class Builder:
    def __init__(self, out, defines=("NON_MATCHING",)):
        self.out = out
        self.defines = defines
        self.objs = {}       # matching object path -> NON_MATCHING object path
        self.elfs = {}       # target -> relinked ELF
        self.parent_ld = {}  # parent -> its new global symbols as a linker script
        self.calls_ld = {}   # parent -> its new tier2_calls linker script

    def compile(self, src):
        target = target_of(src)
        obj = self.out / "obj" / f"{src}.o"
        obj.parent.mkdir(parents=True, exist_ok=True)
        flags = configure.cc_flags(src)
        cmd = [CC, "-V", configure.CC_OVERRIDES.get(src, configure.DEFAULT_CC), "-G", str(configure.g_for(src)), *flags,
               *(f"-D{d}" for d in self.defines), *configure.CC_INCLUDES.split(), src, "-o", obj]
        proc = subprocess.run([str(c) for c in cmd], cwd=ROOT, capture_output=True, text=True)
        if proc.returncode:  # the build's warnings are the matching build's too: shown only on a failure
            sys.stderr.write(proc.stdout + proc.stderr)
            raise RuntimeError(f"{src}: does not compile with -DNON_MATCHING")
        self.objs[obj_of(src, target)] = obj
        return target

    def link(self, name, overrides=None):
        """Relinks a target with the NON_MATCHING objects, the new parent symbols / tier-2 calls where there are, and
        `overrides` ({name: address}) for symbols of splat's undefined_*_auto files (the EXE's calls into FIELDSTG)."""
        t = TARGETS[name]
        p = configure.splat_paths(t)
        ld_text = (ROOT / p["ld_script_path"]).read_text()
        for old, new in self.objs.items():
            if old.startswith(f"build/{name}/"):
                ld_text = ld_text.replace(f"{old}(", f"{new}(")
        d = self.out / "build" / name
        d.mkdir(parents=True, exist_ok=True)
        ld_script = d / Path(p["ld_script_path"]).name
        ld_script.write_text(ld_text)
        extra = []
        for e in t.ld_extra:
            parent = next((q for q in CHILDREN if e == configure.parent_syms_ld(q)), None)
            if parent and parent in self.parent_ld:
                e = self.parent_ld[parent]
            elif e == configure.tier2_calls_ld(name) and name in self.calls_ld:
                e = self.calls_ld[name]
            extra.append(e)
        autos = []
        for a in (p["undefined_syms_auto_path"], p["undefined_funcs_auto_path"]):
            if overrides:
                lines = (ROOT / a).read_text().splitlines()
                for i, line in enumerate(lines):
                    m = re.match(r"(\w+) = 0x[0-9A-Fa-f]+;", line)
                    if m and m.group(1) in overrides:
                        lines[i] = f"{m.group(1)} = 0x{overrides[m.group(1)]:08X};"
                a = d / Path(a).name
                a.write_text("".join(l + "\n" for l in lines))
            autos.append(a)
        scripts = [ld_script, *autos, *extra]
        elf = d / Path(p["elf_path"]).name
        args = [f"{BINUTILS}ld", "-EL", "-nostdlib", "-q"]  # -q: keep the relocations (stale_refs)
        for s in scripts:
            args += ["-T", s]
        run(args + ["-Map", elf.with_suffix(".map"), "-o", elf])
        self.elfs[name] = elf
        return elf

    def output(self, name):
        """The relinked file's bytes: objcopy, cut to the original file's size plus the section's growth."""
        t = TARGETS[name]
        elf = self.elfs[name]
        old_elf = ROOT / configure.splat_paths(t)["elf_path"]
        size = configure.yaml.safe_load((ROOT / t.yaml).read_text())["segments"][-1][0]
        if name != "main":  # the EXE is only relinked with moved overlay addresses: same size
            size += section_size(elf) - section_size(old_elf)
        binf = self.out / "build" / Path(t.output).name
        run([f"{BINUTILS}objcopy", "-O", "binary", elf, binf])
        os.truncate(binf, size)
        return binf

    def new_parent_syms(self, parent):
        syms = nm_globals(self.elfs[parent], configure.OVERLAY_BASE, configure.TIER2_BASE)
        path = self.out / "build" / parent / f"{parent.upper()}.PRO.syms.ld"
        path.write_text(syms_ld(syms))
        self.parent_ld[parent] = path
        return syms

    def new_tier2_calls(self, parent, children):
        """The parent's tier2_calls.ld with the addresses the relinked children give its names."""
        new = {}
        for c in children:
            new.update(nm_globals(self.elfs[c], configure.TIER2_BASE, 0x80200000))
        lines = []
        for line in (ROOT / configure.tier2_calls_ld(parent)).read_text().splitlines():
            m = re.match(r"(\w+) = 0x([0-9A-Fa-f]+);", line)
            if m and m.group(1) in new:
                line = f"{m.group(1)} = 0x{new[m.group(1)]:08X};"
            lines.append(line)
        path = self.out / "build" / parent / "tier2_calls.ld"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("".join(l + "\n" for l in lines))
        self.calls_ld[parent] = path


def own_symbols(elf):
    """{addr: [names]} of an ELF's own defined symbols (not the ABS ones a tier-2 file gets from its parent)."""
    out = {}
    text = run([f"{BINUTILS}nm", "--defined-only", elf], capture_output=True, text=True).stdout
    for line in text.splitlines():
        p = line.split()
        if len(p) == 3 and p[1] not in "Aa" and not p[2].endswith(".NON_MATCHING"):
            out.setdefault(int(p[0], 16), []).append(p[2])
    return out


def exe_remap(b, relinked):
    """{EXE symbol: (old, new address, overlay)} for the EXE's absolute symbols that point at a symbol of a relinked
    overlay that moved. The overlays share addresses, so a symbol found in two relinked overlays that move it
    differently is refused."""
    exe = ROOT / configure.splat_paths(TARGETS["main"])["elf_path"]
    text = run([f"{BINUTILS}nm", "--defined-only", exe], capture_output=True, text=True).stdout
    absolute = {p[2]: int(p[0], 16) for p in (l.split() for l in text.splitlines())
                if len(p) == 3 and p[1] in "Aa" and configure.OVERLAY_BASE < int(p[0], 16) < 0x80200000}
    remap = {}
    for name, addr in sorted(absolute.items()):
        cands = {}
        for t in sorted(relinked):
            old = own_symbols(ROOT / configure.splat_paths(TARGETS[t])["elf_path"]).get(addr)
            if old:
                new = {a for a, names in own_symbols(b.elfs[t]).items() if set(names) & set(old)}
                if len(new) != 1:
                    raise RuntimeError(f"EXE symbol {name}: {t}'s {old} not found once in the relink")
                cands[t] = new.pop()
        moved = {t: a for t, a in cands.items() if a != addr}
        if moved and len(set(cands.values())) > 1:
            raise RuntimeError(f"EXE symbol {name} (0x{addr:08X}) is in several relinked overlays: {cands}")
        if moved:
            t, a = next(iter(moved.items()))
            remap[name] = (addr, a, t)
    return remap


def moved_range(b, name):
    """(first old address whose symbol moved, the old end) of a relinked file, or None if nothing in it moved."""
    old_elf = ROOT / configure.splat_paths(TARGETS[name])["elf_path"]
    old, new = own_symbols(old_elf), own_symbols(b.elfs[name])
    new_addr = {n: a for a, names in new.items() for n in names}
    moved = [a for a, names in old.items() if any(new_addr.get(n, a) != a for n in names)]
    if not moved:
        return None
    text = run([f"{BINUTILS}readelf", "-SW", old_elf], capture_output=True, text=True).stdout
    base, size = (int(x, 16) for x in re.search(r"PROGBITS\s+([0-9a-f]{8})\s+[0-9a-f]+\s+([0-9a-f]+)", text).groups())
    return min(moved), base + size


def moved_symbols(b, relinked):
    """{target: {old address: new address}} of the own symbols that moved in each relinked overlay."""
    out = {}
    for t in sorted(relinked - {"main"}):
        old = own_symbols(ROOT / configure.splat_paths(TARGETS[t])["elf_path"])
        new_addr = {n: a for a, names in own_symbols(b.elfs[t]).items() for n in names}
        m = {a: new_addr[names[0]] for a, names in old.items() if names[0] in new_addr and new_addr[names[0]] != a}
        if m:
            out[t] = m
    return out


def exe_entry_patches(b, relinked):
    """[(EXE address, old word, new word, note)] for overlay_entries (src/main/overlay.c): the tier-1 overlays'
    entry points are plain addresses in the EXE's data, one per stage, the stage's file in overlay_files."""
    exe = elf_symbols(ROOT / configure.splat_paths(TARGETS["main"])["elf_path"])
    data = (DISC / "SLES_039.36").read_bytes()
    word = lambda addr: struct.unpack_from("<I", data, addr - disc_files.EXE_OFF)[0]
    entries, files = exe["overlay_entries"][0], exe["overlay_files"][0]
    esize = files - entries  # the two tables are adjacent, one word per stage (the ELF gives them no size)
    paths = {e.id: e.path for e in disc_files.files()}
    moved = moved_symbols(b, relinked)
    out = []
    for i in range(esize // 4):
        entry, fid = word(entries + 4 * i), word(files + 4 * i)
        t = Path(paths.get(fid) or "-").stem.lower() if fid else None
        if entry and t in moved and entry in moved[t]:
            out.append((entries + 4 * i, entry, moved[t][entry], f"overlay_entries[{i}] ({t})"))
    return out


def exe_stale(binf, b, relinked, handled):
    """Words of the new EXE that still hold the old address of a symbol that moved in a relinked overlay (not one
    the relink or the entry patches changed): [(EXE address, word, overlays)]. The overlays share addresses, so a hit
    may be another overlay's address that only coincides; each one needs a look."""
    moved = moved_symbols(b, relinked)
    data = binf.read_bytes()
    out = []
    for off in range(0x800, len(data) - 3, 4):
        w = struct.unpack_from("<I", data, off)[0]
        hit = [t for t, m in moved.items() if w in m]
        if hit and off + disc_files.EXE_OFF not in handled:
            out.append((off + disc_files.EXE_OFF, w, hit))
    return out


def stale_refs(b, name):
    """Words, lui and jal in a relinked file that hold an address of the part of its own file or its parent's that
    moved, without a relocation: the relink cannot move those. [(address, word, kind)]"""
    ranges = [r for r in (moved_range(b, name),
                          moved_range(b, configure.TIER2_PARENT[name.upper()])
                          if name.upper() in configure.TIER2_PARENT else None) if r]
    if not ranges:
        return []
    elf = b.elfs[name]
    relocs = {}
    for line in run([f"{BINUTILS}readelf", "-rW", elf], capture_output=True, text=True).stdout.splitlines():
        p = line.split()
        if len(p) >= 3 and p[2].startswith("R_MIPS"):
            relocs.setdefault(int(p[0], 16), set()).add(p[2])
    text = run([f"{BINUTILS}readelf", "-SW", elf], capture_output=True, text=True).stdout
    base, off, size = (int(x, 16) for x in
                       re.search(r"PROGBITS\s+([0-9a-f]{8})\s+([0-9a-f]+)\s+([0-9a-f]+)", text).groups())
    data = Path(elf).read_bytes()[off:off + size]
    bad = []
    for i in range(0, len(data) - 3, 4):
        a, w = base + i, struct.unpack_from("<I", data, i)[0]
        k = relocs.get(a, set())
        for lo, hi in ranges:
            if lo <= w < hi and "R_MIPS_32" not in k:
                bad.append((a, w, "word"))
            elif w >> 26 == 0x0F and lo >> 16 <= (w & 0xFFFF) <= (hi - 1) >> 16 and "R_MIPS_HI16" not in k:
                bad.append((a, w, "lui"))
            elif w >> 26 == 3 and lo <= (0x80000000 | (w & 0x3FFFFFF) << 2) < hi and "R_MIPS_26" not in k:
                bad.append((a, w, "jal"))
    return bad


def build_overlays(out, holdouts, defines=("NON_MATCHING",)):
    """Compiles and relinks; returns ({disc path: (new file, original file)} for the files that differ, the holdouts'
    sizes {function: ((addr, size) original, (addr, size) new)}, the relinked targets, the EXE's remapped symbols)."""
    b = Builder(out, defines)
    changed = sorted({b.compile(src) for src, _, _ in holdouts})
    parents = sorted({configure.TIER2_PARENT.get(c.upper(), c) for c in changed if c.upper() in configure.TIER2_PARENT}
                     | {c for c in changed if c in CHILDREN})
    # Tier-2 children first (their layout does not depend on the parent's addresses), then the parents with the
    # children's new addresses, then every child of a parent whose symbols moved, against the parent's new symbols.
    for c in changed:
        if c.upper() in configure.TIER2_PARENT:
            b.link(c)
    relinked = set(changed)
    for p in parents:
        kids = [c for c in CHILDREN[p] if c in b.elfs]
        if kids:
            b.new_tier2_calls(p, kids)
        b.link(p)
        relinked.add(p)
        old = nm_globals(ROOT / configure.splat_paths(TARGETS[p])["elf_path"], configure.OVERLAY_BASE,
                         configure.TIER2_BASE)
        new = b.new_parent_syms(p)
        moved = sorted(n for n in new if old.get(n) != new[n])
        for c in CHILDREN[p] if moved or p in changed else []:
            b.link(c)
            relinked.add(c)
        # The children's addresses do not depend on the parent's: the tier-2 calls used above must still hold.
        if kids:
            prev = b.calls_ld[p].read_text()
            b.new_tier2_calls(p, [c for c in CHILDREN[p] if c in changed])
            assert b.calls_ld[p].read_text() == prev, p
    for c in changed:  # tier-1 files that load no tier-2 file
        if c not in b.elfs:
            b.link(c)
    # The EXE calls into FIELDSTG at fixed addresses (gamestate.c: func_8008B770 = fieldstg_goto_map, ...): relink it
    # with the overlays' new addresses of those symbols.
    remap = exe_remap(b, relinked)
    patches = exe_entry_patches(b, relinked)
    if remap:
        b.link("main", {n: new for n, (old, new, where) in remap.items()})
    else:
        b.elfs["main"] = ROOT / configure.splat_paths(TARGETS["main"])["elf_path"]
    exe_bin = b.output("main")
    data = bytearray(exe_bin.read_bytes())
    for addr, old, new, _ in patches:
        assert struct.unpack_from("<I", data, addr - disc_files.EXE_OFF)[0] == old
        struct.pack_into("<I", data, addr - disc_files.EXE_OFF, new)
    exe_bin.write_bytes(data)
    relinked.add("main")
    # Jal/lui/lw sites the relink changed, and the patched entries, are handled; any other old address is reported.
    orig = (DISC / "SLES_039.36").read_bytes()
    handled = {disc_files.EXE_OFF + o for o in range(0x800, len(data), 4) if data[o:o + 4] != orig[o:o + 4]}
    unhandled = exe_stale(exe_bin, b, relinked, handled)
    stale = {name: stale_refs(b, name) for name in sorted(relinked) if name != "main"}
    stale = {k: v for k, v in stale.items() if v}
    if stale:
        raise RuntimeError("addresses into a moved overlay without a relocation (the relink cannot move them): "
                           + "; ".join(f"{k}: " + ", ".join(f"{a:08X}={w:08X} {kind}" for a, w, kind in v[:5])
                                       for k, v in stale.items()))
    files = {}
    for name in sorted(relinked):
        binf = exe_bin if name == "main" else b.output(name)
        orig = DISC / (TARGETS[name].output[len("build/"):] if name == "main" else
                       "AAA/PRO/" + Path(TARGETS[name].output).name)
        if binf.read_bytes() != orig.read_bytes():
            files[str(orig.relative_to(DISC))] = (binf, orig)
    # Per holdout: the function's size in the original and in the NON_MATCHING build.
    sizes = {}
    for src, func, _ in holdouts:
        t = target_of(src)
        old = elf_symbols(ROOT / configure.splat_paths(TARGETS[t])["elf_path"]).get(func)
        new = elf_symbols(b.elfs[t]).get(func)
        sizes[func] = (old, new)
    return files, sizes, sorted(relinked), remap, patches, unhandled


# ---------------------------------------------------------------------------------------------------------- disc

def mkpsxiso(work, out_bin):
    cue = out_bin.with_suffix(".cue")
    run([MKPSXISO, "-y", "-q", "-o", out_bin, "-c", cue, work / XML.name], stdout=subprocess.DEVNULL)
    return cue


def iso_paths(image):
    """{path: (lba, size)} of every file of an image (disc_files' directory walk, pointed at another image)."""
    disc_files.ISO = image
    disc_files._iso.cache_clear()
    disc_files.iso_files.cache_clear()
    out = {path: (lba, size) for lba, (path, size) in disc_files.iso_files().items()}
    disc_files.ISO = ROOT / "iso/dw2003.bin"
    disc_files._iso.cache_clear()
    disc_files.iso_files.cache_clear()
    return out


def build_disc(out, files):
    """Rebuilds the image with the changed files; patches the EXE's file table if a file moved. Returns the cue."""
    work = out / "disc_src"
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)
    shutil.copy(XML, work / XML.name)
    # A tree of symlinks to extracted/disc (mkpsxiso reads through them), with the changed files as copies.
    run(["cp", "-as", DISC.resolve(), work / "disc"])
    for rel, (binf, _) in files.items():
        dst = work / "disc" / rel
        dst.unlink()
        shutil.copy(binf, dst)
    image = out / "dw2003.bin"
    cue = mkpsxiso(work, image)
    entries = disc_files.files()  # the original table: (id, lba, sectors, path)
    layout = iso_paths(image)
    moved = [(e, layout[e.path]) for e in entries if e.path and
             (layout[e.path][0], (layout[e.path][1] + 2047) // 2048) != (e.lba, e.sectors)]
    if moved:
        exe = work / "disc/SLES_039.36"
        data = bytearray(exe.read_bytes())
        exe.unlink()
        for e, (lba, size) in moved:
            struct.pack_into("<I", data, disc_files.FILETABLE_LBA - disc_files.EXE_OFF + 4 * e.id, lba)
            struct.pack_into("<H", data, disc_files.FILETABLE_SECTORS - disc_files.EXE_OFF + 2 * e.id,
                             (size + 2047) // 2048)  # u16 per file (disc_files.files)
        exe.write_bytes(data)
        cue = mkpsxiso(work, image)
        if iso_paths(image) != layout:
            raise RuntimeError("the patched EXE moved the files again")
    return cue, moved


# ---------------------------------------------------------------------------------------------------------- probe

def probe_spec(out, holdouts, sizes):
    """A tools/coverage.lua spec that arms only the holdouts, in the NON_MATCHING files (their .text from the relink is
    what the slot holds when they are resident): {"path": spec file, "funcs": {(key, addr): name}}."""
    cov = load_module("coverage", ROOT / "tools/coverage.py")
    slots, names = {"t1": {}, "t2": {}}, {}
    for src, func, _ in holdouts:
        t = TARGETS[target_of(src)]
        n = Path(t.output).name
        elf = out / "build" / t.name / f"{n}.elf"
        syms = {p[2]: int(p[0], 16) for p in (l.split() for l in run([f"{BINUTILS}nm", elf], capture_output=True,
                                                                     text=True).stdout.splitlines()) if len(p) == 3}
        base = configure.TIER2_BASE if t.name.upper() in configure.TIER2_PARENT else configure.OVERLAY_BASE
        slot = slots[cov.SLOTS[base]]
        c = slot.setdefault(n, {"key": n, "path": str(out / "build" / n), "base": base, "lo": syms[f"{t.name}_TEXT_START"],
                                "hi": syms[f"{t.name}_TEXT_END"], "funcs": []})
        addr = sizes[func][1][0]
        c["funcs"].append(addr)
        names[(n, addr)] = func
    exe = elf_symbols(ROOT / configure.splat_paths(TARGETS["main"])["elf_path"])
    spec = {"exe": [], "slots": [{"name": k, "cands": list(v.values())} for k, v in slots.items()],
            "sync": cov.sync_points(), "copy": [exe["memcpy"][0], exe["memcpy"][0] + exe["memcpy"][1]]}
    path = out / "probe_spec.lua"
    path.write_text("return " + cov.lua_literal(spec) + "\n")
    return {"path": path, "funcs": names}


def probe_run(replay, probe, script_path, script, bios, run_dir, cue):
    """The script under -debugger -interpreter with tools/coverage.lua armed on the holdouts only. Returns the record and
    {function: {frame, stage}} of the holdouts that ran."""
    run_dir.mkdir(parents=True, exist_ok=True)
    hits_path = run_dir / "probe.txt"
    if hits_path.exists():
        hits_path.unlink()
    wrapper = run_dir / "probe_wrapper.lua"
    wrapper.write_text(f"dofile({json.dumps(str(ROOT / 'tools/coverage.lua'))})\n"
                       f"dofile({json.dumps(str(replay.RUN_LUA))})\n")
    os.environ["DW3_COVERAGE_SPEC"], os.environ["DW3_COVERAGE_OUT"] = str(probe["path"]), str(hits_path)
    try:
        rec = replay.run_once(script_path, script, bios, run_dir, lua=wrapper, emu_args=("-debugger", "-interpreter"),
                              iso=cue)
    finally:
        del os.environ["DW3_COVERAGE_SPEC"], os.environ["DW3_COVERAGE_OUT"]
    hits = {}
    for line in hits_path.read_text().splitlines() if hits_path.exists() else []:
        p = line.split()
        if p and p[0] == "hit":
            func = probe["funcs"].get((p[1], int(p[2], 16)), "?")
            hits.setdefault(func, {"frame": int(p[3]), "stage": int(p[4])})
    return rec, hits


# ---------------------------------------------------------------------------------------------------------- report

def coverage_hits():
    """{function name: [sources]} from build/coverage/*.json (tools/coverage.py), or None if there is none."""
    runs = sorted((ROOT / "build/coverage").glob("*.json"))
    runs = [p for p in runs if p.name != "coverage.json"]
    if not runs:
        return None
    out = {}
    for p in runs:
        rec = json.loads(p.read_text())
        for h in rec.get("hits", []):
            out.setdefault(h["name"], set()).add(rec["source"])
    return {k: sorted(v) for k, v in out.items()}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", help="scratch directory (default: $TMPDIR/dw3_holdouts)")
    ap.add_argument("--scripts", nargs="*", help="replay scripts by name (default: every one with an expected file)")
    ap.add_argument("--no-replay", action="store_true", help="build the image only")
    ap.add_argument("--control", action="store_true",
                    help="check the procedure itself: the same steps without -DNON_MATCHING must give the original image")
    ap.add_argument("--no-probe", action="store_true",
                    help="skip the interpreter run with the holdout probe (which holdouts each script runs)")
    args = ap.parse_args()
    out = Path(args.out or Path(tempfile.gettempdir()) / "dw3_holdouts").resolve()
    out.mkdir(parents=True, exist_ok=True)
    for p in (MKPSXISO, CC, XML, ROOT / configure.splat_paths(TARGETS["main"])["elf_path"]):
        if not p.exists():
            print(f"holdouts: missing {p} (scripts/build.sh, scripts/setup.sh, scripts/extract.sh)", file=sys.stderr)
            return 2

    t0 = time.time()
    holdouts = find_holdouts()
    if args.control:
        files, _, relinked, remap, patches, _ = build_overlays(out / "control", holdouts, defines=())
        cue, moved = build_disc(out / "control", files)
        same = not files and not remap and not patches and not moved and filecmp.cmp(cue.with_suffix(".bin"), ROOT / "iso/dw2003.bin",
                                                                     shallow=False)
        print(f"control (no -DNON_MATCHING, {len(relinked)} targets relinked): "
              + ("the image is bit-identical to iso/dw2003.bin" if same else
                 f"DIFFERS: {len(files)} files, {len(remap)} EXE symbols, {len(moved)} moved") + f" ({time.time() - t0:.0f} s)")
        shutil.rmtree(out / "control")
        if not same:
            return 1
    print(f"holdouts: {len(holdouts)} NON_MATCHING functions in {len({h[0] for h in holdouts})} files")
    files, sizes, relinked, remap, patches, unhandled = build_overlays(out, holdouts)
    print(f"  relinked {len(relinked)} targets; {len(files)} files differ from the disc's:")
    for n, (old, new, t) in remap.items():
        print(f"    EXE: {n} 0x{old:08X} -> 0x{new:08X} ({t})")
    for addr, old, new, note in patches:
        print(f"    EXE: {note} at 0x{addr:08X}: 0x{old:08X} -> 0x{new:08X}")
    for addr, w, ts in unhandled:
        print(f"    EXE: word 0x{w:08X} at 0x{addr:08X} is the old address of a moved symbol of {', '.join(ts)}: "
              "not changed (another overlay's address that coincides, or a reference to fix)")
    quiet = [rel for rel in files if Path(rel).name.startswith("WSTAG") and
             files[rel][0].stat().st_size == files[rel][1].stat().st_size]
    for rel, (binf, orig) in files.items():
        n, o = binf.stat().st_size, orig.stat().st_size
        if rel not in quiet:
            print(f"    {rel}: 0x{o:X} -> 0x{n:X} bytes ({(o + 2047) // 2048} -> {(n + 2047) // 2048} sectors)")
    if quiet:
        print(f"    {len(quiet)} WSTAG files: same size, relinked against FIELDSTG's moved symbols")
    t_build = time.time() - t0
    cue, moved = build_disc(out, files)
    t_disc = time.time() - t0 - t_build
    print(f"  image {cue.with_suffix('.bin')} ({'file table patched: ' + str(len(moved)) + ' files moved' if moved else 'no file moved'}); "
          f"build {t_build:.0f} s, image {t_disc:.0f} s")
    report = {"holdouts": [], "files": {k: [v[1].stat().st_size, v[0].stat().st_size] for k, v in files.items()},
              "moved": len(moved), "replays": {}}
    if args.no_replay:
        return 0

    replay = load_module("replay", ROOT / "tests/replay/replay.py")
    probe = None if args.no_probe else probe_spec(out, holdouts, sizes)
    status = 0
    scripts = [replay.SCRIPTS / f"{s}.json" for s in args.scripts] if args.scripts else sorted(replay.SCRIPTS.glob("*.json"))
    reached = {}  # function -> [scripts]
    for path in scripts:
        script = replay.load_script(path)
        expected_path = replay.EXPECTED / f"{script['name']}.json"
        if not expected_path.exists():
            continue
        expected = json.loads(expected_path.read_text())
        bios = replay.bios_path(expected["bios"]["name"])
        entry = report["replays"][script["name"]] = {}
        for core in ("dynarec", "interpreter") if probe else ("dynarec",):
            print(f"replay {script['name']} on the NON_MATCHING image ({core}" + (", holdout probe)" if core != "dynarec" else ")"))
            t1 = time.time()
            run_dir = out / "replay" / f"{script['name']}_{core}"
            try:
                if core == "dynarec":
                    rec = replay.run_once(path, script, bios, run_dir, iso=cue)
                else:
                    rec, hits = probe_run(replay, probe, path, script, bios, run_dir, cue)
            except (RuntimeError, subprocess.TimeoutExpired) as e:
                print(f"  FAIL: {e}")
                entry[core] = {"status": "fail", "error": str(e)[:500]}
                status = 1
                continue
            full = replay.compare(expected, rec) if core == "dynarec" else None
            cross = replay.compare(replay.cross_core_view(expected), replay.cross_core_view(rec))
            first = next((cp["name"] for e_cp, cp in zip(expected["checkpoints"], rec["checkpoints"])
                          if e_cp["gamestate_sha1_stable"] != cp["gamestate_sha1_stable"]), None)
            verdict = ("identical" if full == [] else "same checkpoints (cross-core view)") if not cross else "DIVERGED"
            if cross:
                status = 1
            print(f"  {verdict}" + (f": {len(full)} fields differ in full, first: {full[0][:160]}" if full else "")
                  + (f"; first differing checkpoint: {first}" if first else "")
                  + (f"; {cross[0][:200]}" if cross else "") + f" ({time.time() - t1:.0f} s)")
            entry[core] = {"status": verdict, "frames": rec["frames"], "full_diffs": (full or [])[:20],
                           "cross_diffs": cross[:20], "first_checkpoint": first}
            if core != "dynarec":
                for func, h in sorted(hits.items(), key=lambda x: x[1]["frame"]):
                    print(f"  ran {func} (first at frame {h['frame']}, stage {h['stage']})")
                    reached.setdefault(func, []).append(script["name"])
                entry[core]["holdouts_run"] = {f: h for f, h in hits.items()}

    if probe is None:  # no probe: what tools/coverage.py recorded on the matching build, if anything
        cov = coverage_hits()
        reached = None if cov is None else {f: [s[len("replay_"):] for s in v if s.startswith("replay_")]
                                            for f, v in cov.items()}
    print("\nper holdout" + (" (reached: build/coverage/*.json)" if probe is None else ""))
    for src, func, line in holdouts:
        old, new = sizes[func]
        ran_scripts = sorted(set((reached or {}).get(func, [])))
        results = [r.get("status") for s in ran_scripts for r in report["replays"].get(s, {}).values()]
        if reached is None:
            verdict = "unknown (no probe, no coverage data)"
        elif not ran_scripts:
            verdict = "not reached"
        elif all(r in ("identical", "same checkpoints (cross-core view)") for r in results):
            verdict = "validated"
        else:
            verdict = "DIVERGED or failed"
        size = f"0x{old[1]:X} -> 0x{new[1]:X}" if old and new else "?"
        print(f"  {func:38s} {src}:{line}  compiled ({size} bytes)  run by: {', '.join(ran_scripts) or '-'}  => {verdict}")
        report["holdouts"].append({"function": func, "where": f"{src}:{line}", "size": [old and old[1], new and new[1]],
                                   "run_by": ran_scripts, "verdict": verdict})
    (out / "report.json").write_text(json.dumps(report, indent=1) + "\n")
    print(f"\nreport: {out / 'report.json'}; total {time.time() - t0:.0f} s")
    return status

if __name__ == "__main__":
    sys.exit(main())
