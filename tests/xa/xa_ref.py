#!/usr/bin/env python3
"""A reference model of the CD-ROM drive's XA-ADPCM decoder and 44,100 Hz resampler in Python, written from psx-spx
"CDROM XA Audio ADPCM Compression" (and "CDROM XA Subheader, File, Channel, Interleave") independently of the port's C
(port/psyq/xa.c), plus the generator of the unit goldens the C must reproduce (tests/xa/goldens.txt, run by
tests/xa/run.sh), and the check of the movies' real sectors.

  tools/venv/bin/python tests/xa/xa_ref.py gen      # regenerate tests/xa/goldens.txt (~2 s)
  tools/venv/bin/python tests/xa/xa_ref.py check    # regenerate in memory, compare with the committed file
  tools/venv/bin/python tests/xa/xa_ref.py disc [MOVIE...] [--xa-test BIN] [--rms]
      # every XA audio sector of the movies (default MOVIEOPN) through this model and through the C (build/xa_test/
      # m64/xa_test, made by tests/xa/run.sh): the decoded 37,800 Hz and the resampled 44,100 Hz PCM must be identical
      # (SHA-1 over the whole movie); --rms prints the RMS per second of the 44,100 Hz output. Needs the user's disc;
      # ~1 min per 2,000 sectors.

Two implementations of the same documentation agreeing is the check: a reading of psx-spx that one of them gets wrong
shows up as a mismatch. Where psx-spx is ambiguous both take the readings port/psyq/xa.c's header lists (docs/SOUND.md
"CD audio"): "/64" and "/8000h" as arithmetic shifts (the latter per term, where the pseudo-code puts it), a
reserved coding field as its 0 value, no emphasis, an 18,900 Hz sample played twice, a mono sample on both sides, the
six-step counter starting at 6. This model follows the pseudo-code's shape (decode_28_nibbles with
"shift = 12 - range", the Output37800Hz / ZigZagInterpolate loop over the table as printed, row by row); the C uses
its own (unit-wise decoding, the tables by column).

Golden file format (text, one op per line; '#' comments; every case starts from a reset decoder):
  case NAME
  tables ZIGZAG FILTERS        the SHA-1s of the zigzag tables (Table1..7, index 1..29, s16 LE) and of the filters'
                               coefficients (pos 0..3, neg 0..3)
  hist OL OOL OR OOR           set the ADPCM history: old/older of the left (mono) and of the right channel
  group CI HEX N SHA1 L0..L3 R0..R3   one 128-byte sound group decoded with coding info CI: N samples per channel,
                               the SHA-1 of the left samples then the right ones (s16 LE; mono: left only), the first
                               four of each side (R = 0 when mono)
  decode CI SEED N SHA1        a sector's 18 groups, made by gen_sector(CI, SEED), decoded likewise from a copy of
                               the decoder (its state does not change; no resampling)
  sector CI SEED N SHA1        the same sector decoded and resampled (xa_decode_sector): N frames at 44,100 Hz, the
                               SHA-1 of the interleaved s16 LE output
  frames HEX N SHA1 [S...]     stereo 37,800 Hz frames (s16 LE, interleaved) into the resampler: N frames out, their
                               SHA-1, and the output samples themselves when short
  state OL OOL OR OOR POS SIX  the decoder's history, the ring position and the six-step counter
  end
"""
import argparse
import hashlib
import random
import struct
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
GOLDENS = HERE / "goldens.txt"

# psx-spx "Pos/neg Tables" (five entries as printed; XA-ADPCM uses filters 0..3 only).
POS_XA_ADPCM_TABLE = (0, +60, +115, +98, +122)
NEG_XA_ADPCM_TABLE = (0, 0, -52, -55, -60)

# psx-spx "25-point Zigzag Interpolation": the rows as printed, index 1..29, columns Table1..Table7.
ZIGZAG_ROWS = [
    (0, 0, 0, 0, -0x0001, +0x0002, -0x0005),
    (0, 0, 0, -0x0001, +0x0003, -0x0008, +0x0011),
    (0, 0, -0x0001, +0x0003, -0x0008, +0x0010, -0x0023),
    (0, -0x0002, +0x0003, -0x0008, +0x0011, -0x0023, +0x0046),
    (0, 0, -0x0002, +0x0006, -0x0010, +0x002B, -0x0017),
    (-0x0002, +0x0003, -0x0005, +0x0005, +0x000A, +0x001A, -0x0044),
    (+0x000A, -0x0013, +0x001F, -0x001B, +0x006B, -0x00EB, +0x015B),
    (-0x0022, +0x003C, -0x004A, +0x00A6, -0x016D, +0x027B, -0x0347),
    (+0x0041, -0x004B, +0x00B3, -0x01A8, +0x0350, -0x0548, +0x080E),
    (-0x0054, +0x00A2, -0x0192, +0x0372, -0x0623, +0x0AFA, -0x1249),
    (+0x0034, -0x00E3, +0x02B1, -0x05BF, +0x0BCD, -0x16FA, +0x3C07),
    (+0x0009, +0x0132, -0x039E, +0x09B8, -0x1780, +0x53E0, +0x53E0),
    (-0x010A, -0x0043, +0x04F8, -0x11B4, +0x6794, +0x3C07, -0x16FA),
    (+0x0400, -0x0267, -0x05A6, +0x74BB, +0x234C, -0x1249, +0x0AFA),
    (-0x0A78, +0x0C9D, +0x7939, +0x0C9D, -0x0A78, +0x080E, -0x0548),
    (+0x234C, +0x74BB, -0x05A6, -0x0267, +0x0400, -0x0347, +0x027B),
    (+0x6794, -0x11B4, +0x04F8, -0x0043, -0x010A, +0x015B, -0x00EB),
    (-0x1780, +0x09B8, -0x039E, +0x0132, +0x0009, -0x0044, +0x001A),
    (+0x0BCD, -0x05BF, +0x02B1, -0x00E3, +0x0034, -0x0017, +0x002B),
    (-0x0623, +0x0372, -0x0192, +0x00A2, -0x0054, +0x0046, -0x0023),
    (+0x0350, -0x01A8, +0x00B3, -0x004B, +0x0041, -0x0023, +0x0010),
    (-0x016D, +0x00A6, -0x004A, +0x003C, -0x0022, +0x0011, -0x0008),
    (+0x006B, -0x001B, +0x001F, -0x0013, +0x000A, -0x0005, +0x0002),
    (+0x000A, +0x0005, -0x0005, +0x0003, -0x0001, 0, 0),
    (-0x0010, +0x0006, -0x0002, 0, 0, 0, 0),
    (+0x0011, -0x0008, +0x0003, -0x0002, +0x0001, 0, 0),
    (-0x0008, +0x0003, -0x0001, 0, 0, 0, 0),
    (+0x0003, -0x0001, 0, 0, 0, 0, 0),
    (-0x0001, 0, 0, 0, 0, 0, 0),
]
assert len(ZIGZAG_ROWS) == 29 and all(len(r) == 7 for r in ZIGZAG_ROWS)
# TableX[i] for X = 1..7, i = 1..29 (index 0 unused, as in the pseudo-code).
TABLES = [None] + [[None] + [ZIGZAG_ROWS[i - 1][x - 1] for i in range(1, 30)] for x in range(1, 8)]


def min_max(v, lo, hi):
    return lo if v < lo else hi if v > hi else v


def signed4bit(v):
    return v - 16 if v & 8 else v


def signed8bit(v):
    return v - 256 if v & 0x80 else v


def coding(ci):
    """(stereo, 18,900 Hz, 8-bit) from the coding info byte; a reserved field value (2, 3) reads as 0."""
    return (ci & 3) == 1, ((ci >> 2) & 3) == 1, ((ci >> 4) & 3) == 1


class Xa:
    def __init__(self):
        self.old = {"L": 0, "R": 0}  # psx-spx "Old/Older Values": per channel, carried over
        self.older = {"L": 0, "R": 0}
        self.ringbuf = {"L": [0] * 32, "R": [0] * 32}
        self.p = 0
        self.sixstep = 6

    # decode_28_nibbles(src, blk, nibble, dst, old, older), for one 128-byte portion `src`.
    def decode_28_nibbles(self, src, blk, nibble, side):
        hdr = src[4 + blk * 2 + nibble]
        rng = hdr & 0x0F
        if rng > 12:  # "reserved shift values 13..15 will act same as shift=9"
            rng = 9
        shift = 12 - rng
        filt = (hdr & 0x30) >> 4
        f0, f1 = POS_XA_ADPCM_TABLE[filt], NEG_XA_ADPCM_TABLE[filt]
        old, older = self.old[side], self.older[side]
        out = []
        for j in range(28):
            t = signed4bit((src[16 + blk + j * 4] >> (nibble * 4)) & 0x0F)
            s = (t << shift) + ((old * f0 + older * f1 + 32) >> 6)
            s = min_max(s, -0x8000, +0x7FFF)
            out.append(s)
            older, old = old, s
        self.old[side], self.older[side] = old, older
        return out

    # The 8-bit format: "Byte for 1st Block/Mono, or 1st Block/Left" etc., "expanded to 16bit by left-shifting by 8,
    # then right-shifted by the selected shift amount"; the headers at 04h..07h.
    def decode_28_bytes(self, src, blk, side):
        hdr = src[4 + blk]
        rng = hdr & 0x0F
        if rng > 12:
            rng = 9
        filt = (hdr & 0x30) >> 4
        f0, f1 = POS_XA_ADPCM_TABLE[filt], NEG_XA_ADPCM_TABLE[filt]
        old, older = self.old[side], self.older[side]
        out = []
        for j in range(28):
            t = signed8bit(src[16 + blk + j * 4])
            s = ((t << 8) >> rng) + ((old * f0 + older * f1 + 32) >> 6)
            s = min_max(s, -0x8000, +0x7FFF)
            out.append(s)
            older, old = old, s
        self.old[side], self.older[side] = old, older
        return out

    def decode_portion(self, ci, src):
        """One 128-byte portion: (left or mono samples, right samples)."""
        stereo, _, eight = coding(ci)
        left, right = [], []
        if eight:
            if stereo:
                for blk in (0, 2):
                    left += self.decode_28_bytes(src, blk, "L")
                    right += self.decode_28_bytes(src, blk + 1, "R")
            else:
                for blk in range(4):
                    left += self.decode_28_bytes(src, blk, "L")
        else:
            for blk in range(4):
                if stereo:  # left-samples (LO-nibbles), plus right-samples (HI-nibbles)
                    left += self.decode_28_nibbles(src, blk, 0, "L")
                    right += self.decode_28_nibbles(src, blk, 1, "R")
                else:  # first 28 samples (LO-nibbles), plus next 28 samples (HI-nibbles)
                    left += self.decode_28_nibbles(src, blk, 0, "L")
                    left += self.decode_28_nibbles(src, blk, 1, "L")
        return left, right

    def decode_sector_data(self, ci, data):
        """decode_sector: the 12h portions of the data (after the subheader)."""
        left, right = [], []
        for i in range(0x12):
            l, r = self.decode_portion(ci, data[i * 128:(i + 1) * 128])
            left += l
            right += r
        return left, right

    def zigzag_interpolate(self, side, table):
        ring = self.ringbuf[side]
        total = 0
        for i in range(1, 30):
            total += (ring[(self.p - i) & 0x1F] * table[i]) >> 15
        return min_max(total, -0x8000, +0x7FFF)

    def output37800(self, sample_l, sample_r, out):
        """Output37800Hz(sample), both channels: appends the (L, R) frames it outputs at 44,100 Hz to `out`."""
        self.ringbuf["L"][self.p & 0x1F] = sample_l
        self.ringbuf["R"][self.p & 0x1F] = sample_r
        self.p += 1
        self.sixstep -= 1
        if self.sixstep == 0:
            self.sixstep = 6
            for x in range(1, 8):
                out.append((self.zigzag_interpolate("L", TABLES[x]), self.zigzag_interpolate("R", TABLES[x])))

    def play_sector(self, raw):
        """A raw 2352-byte audio sector: decoded, an 18,900 Hz sample twice, mono on both sides, resampled."""
        ci = raw[19]
        stereo, half, _ = coding(ci)
        left, right = self.decode_sector_data(ci, raw[24:24 + 0x900])
        if not stereo:
            right = left
        out = []
        for a, b in zip(left, right):
            for _ in range(2 if half else 1):
                self.output37800(a, b, out)
        return out

    def state(self):
        return (self.old["L"], self.older["L"], self.old["R"], self.older["R"], self.p & 0xFFFFFFFF, self.sixstep)


def xorshift_bytes(seed, n):
    """n bytes from xorshift32 (x ^= x << 13; x ^= x >> 17; x ^= x << 5), the low byte of each state; seed != 0.
    tests/xa/xa_test.c has the same generator."""
    x, out = seed & 0xFFFFFFFF, bytearray()
    for _ in range(n):
        x ^= (x << 13) & 0xFFFFFFFF
        x ^= x >> 17
        x ^= (x << 5) & 0xFFFFFFFF
        out.append(x & 0xFF)
    return bytes(out)


def gen_sector(ci, seed):
    """A raw sector: mode 2, subheader file 1 channel 1 submode 64h (audio, Form 2, real time) coding CI (twice), the
    0x900 bytes of sound groups from xorshift_bytes(seed), the rest 0."""
    raw = bytearray(2352)
    raw[15] = 2
    raw[16:24] = bytes((1, 1, 0x64, ci)) * 2
    raw[24:24 + 0x900] = xorshift_bytes(seed, 0x900)
    return bytes(raw)


def s16le(values):
    return struct.pack("<%dh" % len(values), *values)


def sha1(b):
    return hashlib.sha1(b).hexdigest()


def frames_bytes(frames):
    return s16le([v for f in frames for v in f])


# ---- the goldens

CODINGS = [0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15]           # every mono/stereo x rate x 4/8-bit
RESERVED = [0x02, 0x03, 0x08, 0x0C, 0x20, 0x30, 0x40, 0x41, 0xFF]   # reserved fields, emphasis


def gen_lines():
    rng = random.Random(0x5A17)
    out = ["# tests/xa/goldens.txt: generated by tests/xa/xa_ref.py gen (the Python reference of psx-spx's XA-ADPCM",
           "# decoder and zigzag resampler); replayed through port/psyq/xa.c by tests/xa/run.sh. Do not edit by hand."]

    def case(name):
        out.append("case " + name)
        return Xa()

    def group_op(x, ci, g):
        left, right = x.decode_portion(ci, g)
        stereo = coding(ci)[0]
        r4 = right[:4] if stereo else [0, 0, 0, 0]
        out.append("group %02x %s %d %s %s" % (ci, g.hex(), len(left), sha1(s16le(left) + s16le(right)),
                                                " ".join(str(v) for v in left[:4] + r4)))

    def state_op(x):
        out.append("state %d %d %d %d %d %d" % x.state())

    def frames_op(x, frames, show):
        res = []
        for a, b in frames:
            x.output37800(a, b, res)
        line = "frames %s %d %s" % (frames_bytes(frames).hex(), len(res), sha1(frames_bytes(res)))
        if show:
            line += " " + " ".join(str(v) for f in res for v in f)
        out.append(line)

    # The tables: the zigzag tables (Table1..7, index 1..29, s16 LE) and the filters' (0..3) SHA-1s.
    out.append("case tables")
    out.append("tables %s %s" % (sha1(s16le([TABLES[x][i] for x in range(1, 8) for i in range(1, 30)])),
                                 sha1(s16le(list(POS_XA_ADPCM_TABLE[:4]) + list(NEG_XA_ADPCM_TABLE[:4])))))
    out.append("end")

    # Every filter (0..3) with every range (0..15, 13..15 act as 9), at 4 and 8 bits, mono and stereo: every unit of
    # the group uses that parameter byte (bits 6-7 random: unused), random samples, random history.
    out.append("# every filter x range, 4 and 8 bits, mono and stereo, random samples and history")
    for eight in (0, 1):
        for stereo in (0, 1):
            ci = (0x10 if eight else 0) | stereo
            x = case("units_%s_%s" % ("8bit" if eight else "4bit", "stereo" if stereo else "mono"))
            for filt in range(4):
                for r in range(16):
                    hist = [rng.randint(-0x8000, 0x7FFF) for _ in range(4)]
                    x.old["L"], x.older["L"], x.old["R"], x.older["R"] = hist
                    out.append("hist %d %d %d %d" % tuple(hist))
                    g = bytearray(rng.getrandbits(8) for _ in range(128))
                    for k in range(16):
                        if 4 <= k < 12 or rng.random() < 0.5:
                            g[k] = (rng.getrandbits(2) << 6) | (filt << 4) | r
                    group_op(x, ci, bytes(g))
                    state_op(x)
            out.append("end")

    # Saturation: full-scale samples and history pulling the same way, every filter, range 0.
    x = case("clamp")
    for filt in range(4):
        for sign in (1, -1):
            for eight in (0, 1):
                ci = 0x10 if eight else 0x00
                h = 0x7FFF if sign > 0 else -0x8000
                x.old["L"], x.older["L"], x.old["R"], x.older["R"] = h, -h if filt >= 2 else h, 0, 0
                out.append("hist %d %d %d %d" % (x.old["L"], x.older["L"], 0, 0))
                fill = (0x77 if sign > 0 else 0x88) if not eight else (0x7F if sign > 0 else 0x80)
                g = bytes([filt << 4] * 16) + bytes([fill] * 112)
                group_op(x, ci, g)
                state_op(x)
    out.append("end")

    # Whole sectors of random groups, two in a row (the history and the ring carry over), every coding; then the
    # reserved fields and emphasis.
    for ci in CODINGS + RESERVED:
        x = case("sector_%02x" % ci)
        for seed in (0x1000 + ci, 0x2000 + ci):
            y = Xa()
            y.old, y.older = dict(x.old), dict(x.older)
            left, right = y.decode_sector_data(ci, gen_sector(ci, seed)[24:24 + 0x900])
            out.append("decode %02x %d %d %s" % (ci, seed, len(left), sha1(s16le(left) + s16le(right))))
            res = x.play_sector(gen_sector(ci, seed))
            out.append("sector %02x %d %d %s" % (ci, seed, len(res), sha1(frames_bytes(res))))
            state_op(x)
        out.append("end")

    # The resampler on known inputs: an impulse at each of the six phases (the tables' columns come out), a full-scale
    # square wave (the clamp), a step, random frames.
    for phase in range(6):
        x = case("impulse_%d" % phase)
        frames = [(0, 0)] * phase + [(0x7FFF, -0x8000)] + [(0, 0)] * (41 - phase)
        frames_op(x, frames, True)
        state_op(x)
        out.append("end")
    x = case("square")
    frames = [((0x7FFF, -0x8000) if (i // 3) % 2 == 0 else (-0x8000, 0x7FFF)) for i in range(96)]
    frames_op(x, frames, True)
    state_op(x)
    out.append("end")
    x = case("step")
    frames = [(0, 0)] * 30 + [(0x4000, -0x4000)] * 42
    frames_op(x, frames, True)
    state_op(x)
    out.append("end")
    x = case("random_frames")
    for n in (1, 5, 6, 7, 13, 36, 600):
        frames = [(rng.randint(-0x8000, 0x7FFF), rng.randint(-0x8000, 0x7FFF)) for _ in range(n)]
        frames_op(x, frames, n <= 13)
        state_op(x)
    out.append("end")
    return out


# ---- the movies' real sectors

def movie_audio_sectors(name):
    sys.path.insert(0, str(ROOT / "tools"))
    import disc_files  # noqa: E402
    iso = disc_files._iso()
    for e in disc_files.files():
        if e.path.endswith("/" + name + ".STR") or e.path.endswith("/" + name):
            lba, count = e.lba, e.sectors
            break
    else:
        raise SystemExit("xa_ref: no movie " + name)
    sectors = []
    for i in range(count):
        raw = iso[(lba + i) * 2352:(lba + i + 1) * 2352]
        # psx-spx "Data/ADPCM Sector Filtering/Delivery": MODE2, submode audio + real time (the game sets no filter).
        if raw[15] == 2 and (raw[18] & 0x44) == 0x44:
            sectors.append(bytes(raw))
    return lba, count, sectors


def disc(args):
    xa_test = Path(args.xa_test) if args.xa_test else ROOT / "build/xa_test/m64/xa_test"
    if not xa_test.exists():
        raise SystemExit("xa_ref: %s is missing: run tests/xa/run.sh first" % xa_test)
    iso = ROOT / "iso" / "dw2003.bin"
    ok = True
    for name in args.movies:
        lba, count, sectors = movie_audio_sectors(name)
        x = Xa()
        h37, h44 = hashlib.sha1(), hashlib.sha1()
        n37 = n44 = 0
        rms_acc, rms = [], []
        for raw in sectors:
            ci = raw[19]
            stereo = coding(ci)[0]
            y = Xa()
            y.old, y.older = dict(x.old), dict(x.older)
            left, right = y.decode_sector_data(ci, raw[24:24 + 0x900])
            h37.update(s16le([v for pair in zip(left, right if stereo else left) for v in pair]))
            n37 += len(left)
            res = x.play_sector(raw)
            h44.update(frames_bytes(res))
            n44 += len(res)
            if args.rms:
                rms_acc += res
                while len(rms_acc) >= 44100:
                    sec, rms_acc = rms_acc[:44100], rms_acc[44100:]
                    rms.append((sum(a * a + b * b for a, b in sec) / (2 * 44100)) ** 0.5)
        ours = "%d %s %d %s" % (n37, h37.hexdigest(), n44, h44.hexdigest())
        res = subprocess.run([str(xa_test), "--disc", str(iso), str(lba), str(count)], capture_output=True, text=True,
                             check=True)
        theirs = res.stdout.strip().split("\n")[-1]
        same = theirs == "%d sectors %s" % (len(sectors), ours)
        ok &= same
        print("%s: %d sectors from %d (%d audio): python %d sectors %s; C %s: %s" % (
            name, count, lba, len(sectors), len(sectors), ours, theirs, "identical" if same else "DIFFERENT"))
        print("  %.2f s of audio at 44,100 Hz (%d frames)" % (n44 / 44100, n44))
        if args.rms:
            print("  RMS per second: " + " ".join("%.0f" % v for v in rms))
    return 0 if ok else 1


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("cmd", choices=("gen", "check", "disc"))
    ap.add_argument("movies", nargs="*", default=None)
    ap.add_argument("--xa-test", help="the C test binary (default build/xa_test/m64/xa_test)")
    ap.add_argument("--rms", action="store_true", help="disc: the RMS per second of the 44,100 Hz output")
    args = ap.parse_args()
    if args.cmd == "disc":
        args.movies = args.movies or ["MOVIEOPN"]
        return disc(args)
    text = "\n".join(gen_lines()) + "\n"
    if args.cmd == "gen":
        GOLDENS.write_text(text)
        print("wrote %s (%d lines)" % (GOLDENS.relative_to(ROOT), text.count("\n")))
        return 0
    same = GOLDENS.exists() and GOLDENS.read_text() == text
    print("tests/xa/goldens.txt: %s" % ("up to date" if same else "DIFFERS from the model's"))
    return 0 if same else 1


if __name__ == "__main__":
    sys.exit(main())
