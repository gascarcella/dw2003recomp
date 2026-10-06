#!/usr/bin/env python3
"""Render an SPU write trace (tests/sound/spu_trace.py's format) through the port's SPU core into a WAV file.

  tools/venv/bin/python tests/spu/render_trace.py [TRACE] [--out FILE.wav] [--frames-per-tick N]

TRACE defaults to the committed tests/sound/expected/cnty_sel.trace; the WAV goes to build/spu_test/<name>.wav (never
into git). The trace has every SPU register store per tick but only the SHA-1 of each DMA block: the blocks are found on
the disc (tools/disc_files.py) among the 71 sound banks' VAB bodies, each cut or zero-padded to the block's length (a
body is sent with the bytes after it in RAM up to the 64-byte DMA block: the sector's zero padding), and blocks of
zeros (SsInit clearing the reverb work area). A block that matches nothing is replaced by zeros and counted.

The renderer (tests/spu/trace_render.c, built by tests/spu/run.sh) applies each tick's stores, then renders 882 frames
(44,100 / 50: a PAL vsync tick), and reports per second the RMS and peak, the clipped samples, every key-on (started;
looped or stopped as its sample's flags say) and the render speed. Needs the disc (iso/dw2003.bin) and the extracted EXE.
"""
import argparse
import hashlib
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
sys.path.insert(0, str(ROOT / "tests/sound"))
import disc_files  # noqa: E402
import sound_formats  # noqa: E402

DEFAULT_TRACE = ROOT / "tests/sound/expected/cnty_sel.trace"


def candidates():
    """Every sound bank's VAB body: (name, bytes)."""
    exe = (ROOT / "extracted/disc/SLES_039.36").read_bytes()
    out = []
    for bank_id, b in sound_formats.banks(exe):
        body = sound_formats.sub(disc_files.read(b["body_file"]), b["vab_body"])
        out.append((f"bank {bank_id} ({disc_files.files()[b['body_file']].path})", body))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("trace", nargs="?", default=str(DEFAULT_TRACE))
    ap.add_argument("--out")
    ap.add_argument("--frames-per-tick", type=float, default=882, help="samples per tick (fractional: alternating)")
    args = ap.parse_args()
    trace = Path(args.trace)
    build = ROOT / "build/spu_test"
    exe = build / "m64/trace_render"
    if not exe.exists():
        subprocess.run([str(ROOT / "tests/spu/run.sh")], check=True, stdout=subprocess.DEVNULL)
    wav = Path(args.out) if args.out else build / (trace.stem + ".wav")

    dma = []      # (tick, addr, length, sha1)
    stores = []   # (tick, kind, ...)
    for line in trace.read_text().splitlines():
        if not line or line.startswith("#"):
            continue
        f = line.split()
        tick = int(f[0])
        if f[1] == "dma4":
            kv = dict(x.split("=") for x in f[2:])
            entry = (tick, int(kv["spu"], 16), int(kv["len"]), kv["sha1"])
            dma.append(entry)
            stores.append((tick, "d", entry))
        else:
            # the trace names a register by its address's low 12 bits (d80 = 0x1F801D80): spu.h offsets are from C00
            stores.append((tick, "w", int(f[1], 16) - 0xC00, int(f[3], 16)))

    # Match the DMA blocks by SHA-1.
    wanted = {(length, sha) for _, _, length, sha in dma}
    found = {}
    for length, sha in wanted:
        if hashlib.sha1(bytes(length)).hexdigest() == sha:
            found[(length, sha)] = ("zeros", bytes(length))
    if len(found) < len(wanted):
        for name, body in candidates():
            for length, sha in wanted - set(found):
                data = body[:length].ljust(length, b"\0")
                if hashlib.sha1(data).hexdigest() == sha:
                    found[(length, sha)] = (name, data)
    blob, where = bytearray(), {}
    for key, (name, data) in sorted(found.items()):
        where[key] = len(blob)
        blob += data
    missing = len(blob)
    blob += bytes(max(length for _, _, length, _ in dma) if dma else 0)

    build.mkdir(parents=True, exist_ok=True)
    cmds = build / (trace.stem + ".cmds")
    blob_path = build / (trace.stem + ".blob")
    lines, last = [], None
    for s in stores:
        if s[0] != last:
            lines.append(f"tick {s[0]}")
            last = s[0]
        if s[1] == "w":
            lines.append(f"w {s[2]:x} {s[3]:x}")
        else:
            _, addr, length, sha = s[2]
            lines.append(f"d {addr:x} {length} {where.get((length, sha), missing)}")
    cmds.write_text("\n".join(lines) + "\n")
    blob_path.write_bytes(bytes(blob))

    matched = sum(1 for _, _, length, sha in dma if (length, sha) in found)
    print(f"{trace.name}: {len(stores) - len(dma)} stores, {len(dma)} DMA blocks: {matched} matched by SHA-1 "
          f"({len(dma) - matched} replaced by zeros)")
    for key, (name, data) in sorted(found.items(), key=lambda kv: -kv[0][0]):
        n = sum(1 for _, _, length, sha in dma if (length, sha) == key)
        print(f"  {n:3d} x {key[0]:6d} bytes {key[1][:12]}: {name}")
    r = subprocess.run([str(exe), str(cmds), str(blob_path), str(wav), "--frames-per-tick",
                        str(args.frames_per_tick)])
    print(f"wrote {wav}")
    return r.returncode or (1 if matched < len(dma) else 0)


if __name__ == "__main__":
    sys.exit(main())
