#!/usr/bin/env python3
"""Sub-pixel precision (issue #68; psxstack docs/PORT.md "Sub-pixel precision"): the battle's vertex jitter at an
internal scale, and how many of its polygon vertices the GTE shadow finds.

Usage: tests/port/subpixel_jitter.py run [--exe build/port/dw2003] [--keep LOG] [--scale N ...]
       tests/port/subpixel_jitter.py analyze LOG [--scale N ...]

`run` replays first_battle_save to the end of its first battle (FIGHTSTG, frames BATTLE) with the shadow's log on
(DW3_PORT_SUBPIXEL_LOG, psxstack psyq/gte_shadow.c: it turns the shadow on from boot), then analyzes the log and
checks the result (exit 0 pass, 1 fail, 2 no build or no disc; ~40 s):
  - the polygon vertices gpu.c drew in the battle: at least MIN_PRECISE of them found their precise value;
  - no vertex's 16.16 sum disagrees with the integer SX, SY the game got (floor);
  - the motion error of the shadow's positions (below) at scale 4: at most MAX_JUMPS of the samples off by a whole
    target pixel or more (the integer SXY: about 20 %).

The jitter. Every RTPS vertex of the battle is in the log: its frame, its place in the frame's RTPS sequence, the
model-space vector, the integer SX, SY, the 16.16 sums they were cut from, the float projection OFX + H * MAC / MAC3
(the reference) and z. A vertex is tracked from frame f to f + 1 when the same place in the sequence projects the
same vector (the battle draws its models in the same order every frame); it counts when on screen and unclamped in
both frames. Its motion error on an axis is |(p1 - p0) - (q1 - q0)| x N in target pixels: p its drawn position, q the
reference. Drawn positions compared:
  - integer: SX, SY as the game stores them (the renderer without the shadow);
  - shadow: the 16.16 sum less half a pixel where the vertex's word is unique within its batch (one mesh's vertex
    cache: a run of consecutive SXY stores), else the mean of the batch's candidates for the word, less half a pixel
    (what the renderer gets; the half pixel centres it on the integer: the integer is the sum's floor);
  - shadow, integer fallback: as shadow, but an ambiguous word at the integer (for comparison).
The drawn polygon vertices' counts (precise, ambiguous, unmatched) are gpu.c's own, per frame, from the log.
"""
import argparse
import collections
import math
import os
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "tests/replay/scripts/first_battle_save.json"
DISC = ROOT / "iso/dw2003.cue"
BATTLE = (19760, 20952)  # first_battle_save's battle_start .. battle_won checkpoints in the port (FIGHTSTG)
MIN_PRECISE = 0.90
MAX_JUMPS = 0.05

REC = struct.Struct("<iI6h5f")  # frame, n, vx, vy, vz, sx, sy, kind, a, b, c, d, z
U32 = struct.Struct("<I")


def load(path):
    frames = collections.defaultdict(list)
    counts = [0, 0, 0, 0]  # drawn, precise, ambiguous, unmatched
    batch_of = {}
    batch = 0
    data = Path(path).read_bytes()
    for off in range(0, len(data), REC.size):
        r = REC.unpack_from(data, off)
        f = frames[r[0]]
        if r[7] == 1:
            # The store that starts a batch follows its own vertex's RTPS: the last record opens the new batch.
            batch += 1
            if f:
                batch_of[(r[0], len(f) - 1)] = batch
        elif r[7] == 2:
            counts[0] += r[1]
            for k in range(3):
                counts[1 + k] += U32.unpack(struct.pack("<f", r[8 + k]))[0]
        else:
            batch_of[(r[0], len(f))] = batch
            f.append(r)
    return {k: v for k, v in frames.items() if v}, batch_of, counts


def ok(r):
    """On screen (the game keeps OFX = OFY = 0: the drawing offset centres it), unclamped, in front."""
    return r[12] > 0 and abs(r[5]) <= 200 and abs(r[6]) <= 160 and abs(r[10] - r[5]) < 2 and abs(r[11] - r[6]) < 2


def drawn(frames, batch_of):
    """Each vertex's position under the shadow (mean for an ambiguous word) and with the integer fallback."""
    out = {}
    for f, recs in frames.items():
        groups = collections.defaultdict(list)
        for i, r in enumerate(recs):
            groups[(batch_of[(f, i)], r[5], r[6])].append(i)
        for i, r in enumerate(recs):
            g = groups[(batch_of[(f, i)], r[5], r[6])]
            cands = [(recs[j][8], recs[j][9]) for j in g]
            if len(set(cands)) == 1:
                pos = (r[8] - 0.5, r[9] - 0.5)
                out[(f, i)] = (pos, pos, False)
            else:
                mean = (sum(c[0] for c in cands) / len(cands) - 0.5, sum(c[1] for c in cands) / len(cands) - 0.5)
                out[(f, i)] = (mean, (float(r[5]), float(r[6])), True)
    return out


def pct(a, q):
    return a[min(len(a) - 1, int(q * len(a)))]


def analyze(path, scales):
    """Prints the counts and the jitter; returns (precise share, floor mismatches, shadow's >= 1 px share at 4)."""
    frames, batch_of, counts = load(path)
    fs = sorted(frames)
    if not fs:
        print("subpixel: the log has no RTPS vertex")
        return 0.0, 0, 1.0
    n = sum(len(frames[f]) for f in fs)
    pos = drawn(frames, batch_of)
    amb = sum(1 for v in pos.values() if v[2])
    bad = sum(1 for f in fs for r in frames[f] if r[10] != 0 and abs(r[5]) < 0x3FF and abs(r[6]) < 0x3FF and
              (math.floor(r[8]) != r[5] or math.floor(r[9]) != r[6]))
    share = counts[1] / counts[0] if counts[0] else 0.0
    print(f"subpixel: frames {fs[0]}..{fs[-1]} ({len(fs)} with RTPS), {n} RTPS vertices ({n // len(fs)} a frame), "
          f"{amb} ({100 * amb / n:.1f} %) ambiguous within their batch; floor(16.16 sum) != SXY: {bad}")
    if counts[0]:
        print(f"subpixel: polygon vertices drawn {counts[0]}: precise {counts[1]} ({100 * share:.1f} %; at an "
              f"ambiguous word's mean {counts[2]}, {100 * counts[2] / counts[0]:.1f} %), without {counts[3]} "
              f"({100 * counts[3] / counts[0]:.1f} %)")
    kinds = {
        "integer": lambda f, i, r, k: r[5 + k],
        "shadow": lambda f, i, r, k: pos[(f, i)][0][k],
        "shadow, integer fallback": lambda f, i, r, k: pos[(f, i)][1][k],
    }
    jumps = 1.0
    for scale in scales:
        for name, at in kinds.items():
            js = []
            for f0, f1 in zip(fs, fs[1:]):
                if f1 != f0 + 1:
                    continue
                for i, (ra, rb) in enumerate(zip(frames[f0], frames[f1])):
                    if ra[1] != rb[1] or ra[2:5] != rb[2:5] or not ok(ra) or not ok(rb):
                        continue
                    for k in (0, 1):
                        d = (at(f1, i, rb, k) - at(f0, i, ra, k)) - (rb[10 + k] - ra[10 + k])
                        js.append(abs(d) * scale)
            if not js:
                continue
            js.sort()
            over = sum(j >= 1 for j in js) / len(js)
            if scale == 4 and name == "shadow":
                jumps = over
            print(f"subpixel: scale {scale}, {name:24}: {len(js)} samples, motion error (target px) mean "
                  f"{sum(js) / len(js):.3f}, p50 {pct(js, .5):.3f}, p90 {pct(js, .9):.3f}, p99 {pct(js, .99):.3f}, max "
                  f"{js[-1]:.3f}; >= 1 px {100 * over:.2f} %, >= 2 px {100 * sum(j >= 2 for j in js) / len(js):.2f} %")
    return share, bad, jumps


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("run")
    r.add_argument("--exe", default=str(ROOT / "build/port/dw2003"))
    r.add_argument("--keep", help="keep the log at this path")
    r.add_argument("--scale", type=int, nargs="+", default=[4])
    a = sub.add_parser("analyze")
    a.add_argument("log")
    a.add_argument("--scale", type=int, nargs="+", default=[4])
    args = ap.parse_args()
    if args.cmd == "analyze":
        analyze(args.log, args.scale)
        return 0
    if not Path(args.exe).exists():
        print(f"subpixel: no {args.exe} (cmake -S port -B build/port -G Ninja && cmake --build build/port)")
        return 2
    if not DISC.exists():
        print(f"subpixel: no disc ({DISC}); skipped")
        return 2
    with tempfile.TemporaryDirectory() as tmp:
        log = args.keep or os.path.join(tmp, "subpixel.bin")
        env = dict(os.environ, DW3_PORT_SUBPIXEL_LOG=log, DW3_PORT_SUBPIXEL_LOG_FROM=str(BATTLE[0]),
                   DW3_PORT_SUBPIXEL_LOG_TO=str(BATTLE[1]))
        p = subprocess.run([args.exe, "--disc", str(DISC), "--script", str(SCRIPT), "--max-frames", str(BATTLE[1] + 1)],
                           env=env, cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True, timeout=600)
        if p.returncode != 0:
            print(f"subpixel: FAIL: the run exited {p.returncode}\n{p.stderr[-2000:]}")
            return 1
        share, bad, jumps = analyze(log, sorted(set(args.scale) | {4}))
    fails = []
    if share < MIN_PRECISE:
        fails.append(f"precise {100 * share:.1f} % < {100 * MIN_PRECISE:.0f} %")
    if bad:
        fails.append(f"{bad} floor mismatches")
    if jumps > MAX_JUMPS:
        fails.append(f"the shadow's jumps >= 1 px at scale 4: {100 * jumps:.2f} % > {100 * MAX_JUMPS:.0f} %")
    print("subpixel: " + ("FAIL: " + "; ".join(fails) if fails else "pass"))
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
