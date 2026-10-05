#!/usr/bin/env python3
"""Survey the extracted disc: EXE header, file types, and overlay load addresses.

Reproduces the findings in docs/DISC_LAYOUT.md. Run after scripts/extract.sh:
    tools/venv/bin/python tools/disc_survey.py [--checksums]

--checksums rewrites config/SLES_039.36.sha1 and config/overlays.sha1.
"""
import argparse
import collections
import hashlib
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DISC = ROOT / "extracted" / "disc"
EXE = DISC / "SLES_039.36"
PRO = DISC / "AAA" / "PRO"

# Load addresses (see docs/DISC_LAYOUT.md). Tier 1 starts at the end of .bss;
# fight and stage overlays start right after the largest tier-1 overlay (CARDGAME).
TIER1_BASE = 0x80082CB0
TIER2_BASE = 0x800A5DE0
DATA_ONLY = {"SDIGIEDT.PRO", "SFSTDATA.PRO", "SMDLDATA.PRO", "WSTAG260.PRO"}
LOADS_STORES = (0x09, 0x20, 0x21, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2B)  # addiu, lb..sw


def words(data):
    return struct.unpack_from(f"<{len(data) // 4}I", data)


def overlay_base(name):
    if name in DATA_ONLY:
        return None
    if name.startswith("WSTAG") or name.startswith("WFIGHT"):
        return TIER2_BASE
    return TIER1_BASE


def hilo_addresses(ws):
    """Absolute addresses built by lui + (addiu | load | store) pairs."""
    out = []
    for i, x in enumerate(ws):
        if x >> 26 != 0x0F:
            continue
        rt, hi = (x >> 16) & 31, (x & 0xFFFF) << 16
        for y in ws[i + 1 : i + 8]:
            if (y >> 21) & 31 == rt and y >> 26 in LOADS_STORES:
                lo = y & 0xFFFF
                out.append((hi + lo - (0x10000 if lo & 0x8000 else 0)) & 0xFFFFFFFF)
                break
    return out


def exe_header():
    d = EXE.read_bytes()
    assert d[:8] == b"PS-X EXE", "not a PS-X EXE"
    f = struct.unpack_from("<10I", d, 0x10)
    names = ["pc0", "gp0", "t_addr", "t_size", "d_addr", "d_size", "b_addr", "b_size", "s_addr", "s_size"]
    print(f"== {EXE.name}: {len(d)} bytes, SHA-1 {hashlib.sha1(d).hexdigest()}")
    for n, v in zip(names, f):
        print(f"  {n:7} 0x{v:08x}")
    marker = d[0x4C:0x80].split(b"\0")[0].decode()  # no backslash inside the f-string (Python < 3.12)
    print(f"  marker  {marker}")


def file_types():
    print("== files by extension")
    stats = collections.defaultdict(lambda: [0, 0])
    for p in DISC.rglob("*"):
        if p.is_file():
            ext = p.suffix.upper().lstrip(".") or "(none)"
            stats[ext][0] += 1
            stats[ext][1] += p.stat().st_size
    for ext, (n, size) in sorted(stats.items(), key=lambda kv: -kv[1][1]):
        print(f"  {ext:8} {n:5} files {size / 2**20:9.2f} MiB")


def check_overlays():
    """Check every overlay's absolute references against its assumed base."""
    print("== overlay base check (lui/lo references landing inside the overlay)")
    bad = 0
    per_base = collections.Counter()
    for p in sorted(PRO.glob("*.PRO")):
        base = overlay_base(p.name)
        if base is None:
            continue
        d = p.read_bytes()
        refs = [a for a in hilo_addresses(words(d)) if 0x80060000 <= a < 0x80100000]
        inside = sum(base <= a < base + len(d) for a in refs)
        per_base[base] += 1
        if refs and inside == 0:
            bad += 1
            print(f"  ?? {p.name}: {len(refs)} refs, none inside 0x{base:08x}")
    for base, n in sorted(per_base.items()):
        print(f"  0x{base:08x}: {n} overlays")
    print(f"  {bad} overlays without supporting references")
    return bad == 0


def write_checksums():
    cfg = ROOT / "config"
    (cfg / "SLES_039.36.sha1").write_text(f"{hashlib.sha1(EXE.read_bytes()).hexdigest()}  SLES_039.36\n")
    lines = [f"{hashlib.sha1(p.read_bytes()).hexdigest()}  AAA/PRO/{p.name}" for p in sorted(PRO.glob("*.PRO"))]
    (cfg / "overlays.sha1").write_text("\n".join(lines) + "\n")
    print(f"== wrote config/SLES_039.36.sha1 and config/overlays.sha1 ({len(lines)} overlays)")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--checksums", action="store_true", help="rewrite config/*.sha1")
    args = ap.parse_args()
    if not EXE.exists():
        sys.exit("extracted/disc missing; run scripts/extract.sh first")
    exe_header()
    file_types()
    ok = check_overlays()
    if args.checksums:
        write_checksums()
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
