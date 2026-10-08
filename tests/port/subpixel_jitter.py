#!/usr/bin/env python3
"""The battle's vertex jitter at an internal scale (issue #68): how far each projected vertex's motion from one frame
to the next departs from the motion of its exact projection, in target pixels.

Usage: tests/port/subpixel_jitter.py LOG [--scale N ...]

LOG is the GTE log of a run with the measurement patch of issue #68 applied (port/psyq/gte.c logs every RTPS/RTPT of
frames DW3_GTE_LOG_FROM..DW3_GTE_LOG_TO into DW3_GTE_LOG; the shadow's batch starts are marker records):

    DW3_GTE_LOG=gte.bin DW3_GTE_LOG_FROM=19760 DW3_GTE_LOG_TO=20952 build/port/dw2003 --disc iso/dw2003.cue \\
        --script tests/replay/scripts/first_battle_save.json --max-frames 20953

(frames 19760..20952 of first_battle_save are its first battle, FIGHTSTG). A record is one vertex: the frame, its
place in the frame's RTPS sequence, the model-space vector, the integer SX, SY the game gets, the 16.16 sums SX and SY
are cut from (gte.c: OFX + IR1 * n before >> 16), the float projection OFX + H * MAC1 / MAC3 and the camera-space z.

A vertex is tracked from frame f to f + 1 when the same place in the sequence projects the same vector (the battle
draws its models in the same order every frame); it counts when it is on screen and unclamped in both frames. Its
motion error on an axis is |(p1 - p0) - (q1 - q0)| x N: p its drawn position, q the float projection (the reference).
Drawn positions compared:
  - integer: SX, SY as the game stores them (what the renderer draws today);
  - shadow: the 16.16 sum less half a pixel where the vertex's word is unique within its batch (one mesh's vertex
    cache: the words a run of consecutive SXY stores wrote), the integer where another vertex of the batch has the
    same word (ambiguous: the renderer cannot tell them apart in a packet);
  - shadow, mean: as shadow, but an ambiguous word takes the mean of its candidates.
"""
import argparse
import collections
import math
import struct
import sys

REC = struct.Struct("<iI6h5f")  # frame, ordinal, vx, vy, vz, sx, sy, pad, p16x, p16y, pfx, pfy, z
MARK = 0xFFFFFFFF


def load(path):
    frames = collections.defaultdict(list)
    data = open(path, "rb").read()
    batch_of = {}
    batch = 0
    for off in range(0, len(data), REC.size):
        r = REC.unpack_from(data, off)
        f = frames[r[0]]
        if r[1] == MARK:
            # The store that starts a batch follows its own vertex's RTPS: the last record opens the new batch.
            batch += 1
            if f:
                batch_of[(r[0], len(f) - 1)] = batch
            continue
        batch_of[(r[0], len(f))] = batch
        f.append(r)
    return frames, batch_of


def ok(r):
    """On screen (the game keeps OFX = OFY = 0: the drawing offset centres it), unclamped, in front."""
    return r[12] > 0 and abs(r[5]) <= 200 and abs(r[6]) <= 160 and abs(r[10] - r[5]) < 2 and abs(r[11] - r[6]) < 2


def drawn(frames, batch_of):
    """Each vertex's drawn position under the shadow (centred, integer fallback) and its mean variant."""
    out = {}
    for f, recs in frames.items():
        groups = collections.defaultdict(list)
        for i, r in enumerate(recs):
            groups[(batch_of[(f, i)], r[5], r[6])].append(i)
        for i, r in enumerate(recs):
            g = groups[(batch_of[(f, i)], r[5], r[6])]
            cands = {(recs[j][8], recs[j][9]) for j in g}
            if len(cands) == 1:
                pos = (r[8] - 0.5, r[9] - 0.5)
                out[(f, i)] = (pos, pos, False)
            else:
                mx = sum(c[0] for c in cands) / len(cands) - 0.5
                my = sum(c[1] for c in cands) / len(cands) - 0.5
                out[(f, i)] = ((float(r[5]), float(r[6])), (mx, my), True)
    return out


def pct(a, q):
    return a[min(len(a) - 1, int(q * len(a)))]


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("log")
    ap.add_argument("--scale", type=int, nargs="+", default=[4])
    args = ap.parse_args()
    frames, batch_of = load(args.log)
    fs = sorted(frames)
    n = sum(len(frames[f]) for f in fs)
    pos = drawn(frames, batch_of)
    amb = sum(1 for v in pos.values() if v[2])
    bad = sum(1 for f in fs for r in frames[f] if ok(r) and (math.floor(r[8]) != r[5] or math.floor(r[9]) != r[6]))
    print(f"frames {fs[0]}..{fs[-1]} ({len(fs)}), {n} vertices ({n // len(fs)} a frame); floor(16.16 sum) != SXY: {bad}; "
          f"ambiguous within their batch: {amb} ({100 * amb / n:.1f} %)")
    kinds = {
        "integer": lambda f, i, r, k: r[5 + k],
        "shadow": lambda f, i, r, k: pos[(f, i)][0][k],
        "shadow, mean": lambda f, i, r, k: pos[(f, i)][1][k],
    }
    for scale in args.scale:
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
            js.sort()
            print(f"scale {scale}, {name:12}: {len(js)} samples; motion error (target px) mean {sum(js) / len(js):.3f}, "
                  f"p50 {pct(js, .5):.3f}, p90 {pct(js, .9):.3f}, p99 {pct(js, .99):.3f}, max {js[-1]:.3f}; "
                  f">= 1 px {100 * sum(j >= 1 for j in js) / len(js):.2f} %, >= 2 px {100 * sum(j >= 2 for j in js) / len(js):.2f} %")
    return 0


if __name__ == "__main__":
    sys.exit(main())
