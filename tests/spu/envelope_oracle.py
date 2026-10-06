#!/usr/bin/env python3
"""The ADSR envelope against the emulator: PCSX-Redux's SPU (an emulation too, so a second opinion, not the hardware)
programs voice 23 with an ADSR word, keys it on, and reads its envelope (ENVX, 0x1F801D7C) once per vsync; the same
ADSR in our SPU (tests/spu/spu_ref.py's model, which tests/spu/goldens.txt holds equal to the port's C core) gives the
envelope per 44,100 Hz sample. Each vsync reading must equal our envelope at sample t0 + k * S for one key-on offset t0
and one samples-per-vsync rate S per case, found by search: the report gives, per case, the share of readings that fit.

  tools/venv/bin/python tests/spu/envelope_oracle.py gen     # run the emulator (~40 s), write envelope_oracle.json
  tools/venv/bin/python tests/spu/envelope_oracle.py check   # compare the committed readings with our envelope

How it runs (tests/golden/oracle.py's machinery, one boot, CNTY_SEL resident): a MIPS routine written into scratch RAM
waits for a vsync (LIBETC's VSync(0) in the EXE), stores voice 23's registers directly (volume 0: silent; pitch 1000h;
start: a looping sample of COMMON, which sound_init loaded at 1010h; the ADSR words), keys it on (KON1 bit 7), then per
vsync: VSync(0), reads ENVX into the output buffer, keys the voice off after `hold` vsyncs. LIBSND's per-tick flush (the
game's vsync callback) writes KON/KOFF of its own voices only; voice 23 is never used by the game at CNTY_SEL (the
trace). Found on the way: on SsInit's silent block at 1000h (flags 7: start, end, repeat) Redux reads ENVX as 0 at every
vsync after a key-on, where psx-spx's code 3 (end + repeat) keeps the envelope going; hence a real looping sample.
"""
import json
import struct
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(ROOT / "tests/golden"))
sys.path.insert(0, str(HERE))
import oracle  # noqa: E402
import spu_ref  # noqa: E402

GOLDEN = HERE / "envelope_oracle.json"
CODE_ADDR = 0x80181000
VOICE = 23
VOLUME = 0       # the voice's volume (L and R): silent
START = 0x37C6   # the start address / 8: COMMON's VAG 36 (1BE30h), looping from its third block

# (adsr1, adsr2, hold vsyncs, total vsyncs): slow enough phases to span several vsyncs each; the game's own words.
CASES = [
    (0x3C00, 0x0000, 40, 60),    # linear attack shift 15 (+7 per 16 samples), then decay to the sustain level
    (0x4000, 0x0000, 60, 80),    # linear attack shift 16, step +7
    (0xBC00, 0x0000, 40, 60),    # exponential attack shift 15 (slower above 6000h)
    (0xC000, 0x0000, 60, 80),    # exponential attack shift 16
    (0x00F7, 0x5FC0, 40, 60),    # instant attack, decay shift 15 to the sustain level 4000h, sustain rate 7Fh (held)
    (0x00FF, 0x5FC0, 20, 40),    # decay shift 15 with the sustain level 8000h (no decay at all?)
    (0x00FF, 0x4D00, 50, 60),    # sustain: linear decrease, shift 13
    (0x00FF, 0xCD00, 50, 60),    # sustain: exponential decrease, shift 13
    (0x00FF, 0x0D40, 50, 60),    # sustain: linear increase from 7FFFh (stays)
    (0x00FF, 0x5FCF, 5, 60),     # release: linear, shift 15
    (0x00FF, 0x5FED, 5, 60),     # release: exponential, shift 13
    (0x80FF, 0x5FCF, 10, 40),    # the game's (CNTY_SEL's music)
    (0x87BA, 0x500A, 10, 40),
    (0x9BBC, 0x500A, 10, 40),
]


# ------------------------------------------------------------------------------------------- a minimal assembler
def i_type(op, rs, rt, imm):
    return (op << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)


ZERO, A0, A1, A2, T0, T1, T2, S0, S1, S2, S3, SP, RA = 0, 4, 5, 6, 8, 9, 10, 16, 17, 18, 19, 29, 31


def addiu(rt, rs, imm): return i_type(0x09, rs, rt, imm)
def lui(rt, imm): return i_type(0x0F, 0, rt, imm)
def lw(rt, off, base): return i_type(0x23, base, rt, off)
def lhu(rt, off, base): return i_type(0x25, base, rt, off)
def sw(rt, off, base): return i_type(0x2B, base, rt, off)
def sh(rt, off, base): return i_type(0x29, base, rt, off)
def bne(rs, rt, off): return i_type(0x05, rs, rt, off)
def addu(rd, rs, rt): return (rs << 21) | (rt << 16) | (rd << 11) | 0x21
def jr(rs): return (rs << 21) | 0x08
def jalr(rs): return (rs << 21) | (RA << 11) | 0x09


def probe_code():
    """probe(a0 = params {u16 adsr1, u16 adsr2, u32 hold, u32 total}, a1 = out (u16 per vsync), a2 = VSync)."""
    base = 0x1C00 + 16 * VOICE   # voice 23's registers, from t0 = 0x1F800000
    c = [addiu(SP, SP, -40), sw(RA, 36, SP), sw(S0, 16, SP), sw(S1, 20, SP), sw(S2, 24, SP), sw(S3, 28, SP),
         addu(S0, A0, ZERO), addu(S1, A1, ZERO), addu(S2, A2, ZERO), addu(S3, ZERO, ZERO),
         jalr(S2), addiu(A0, ZERO, 0),                       # VSync(0): start on a frame boundary
         lui(T0, 0x1F80),
         addiu(T1, ZERO, VOLUME), sh(T1, base + 0, T0), sh(T1, base + 2, T0),
         addiu(T1, ZERO, 0x1000), sh(T1, base + 4, T0),      # pitch 1000h
         addiu(T1, ZERO, START), sh(T1, base + 6, T0),
         lhu(T1, 0, S0), 0, sh(T1, base + 8, T0),
         lhu(T1, 2, S0), 0, sh(T1, base + 10, T0),
         addiu(T1, ZERO, 1 << (VOICE - 16)), sh(T1, 0x1D8A, T0)]   # KON1
    loop = len(c)
    c += [jalr(S2), addiu(A0, ZERO, 0),                       # VSync(0)
          lui(T0, 0x1F80), lhu(T1, base + 12, T0), addiu(S3, S3, 1), sh(T1, 0, S1), addiu(S1, S1, 2),
          lw(T2, 4, S0), 0]   # (nops: the R3000 load delay slot)
    skip = len(c) + 4
    c += [bne(S3, T2, skip - (len(c) + 1)), 0,
          addiu(T1, ZERO, 1 << (VOICE - 16)), sh(T1, 0x1D8E, T0)]   # KOFF1 after `hold` vsyncs
    assert len(c) == skip
    c += [lw(T2, 8, S0), 0]
    c += [bne(S3, T2, loop - (len(c) + 1)), 0]
    c += [lw(RA, 36, SP), lw(S0, 16, SP), lw(S1, 20, SP), lw(S2, 24, SP), lw(S3, 28, SP), 0, jr(RA), addiu(SP, SP, 40)]
    return b"".join(struct.pack("<I", w) for w in c)


def run_emulator():
    sym = oracle.Symbols()
    code = probe_code()
    cases = []
    for k, (a1, a2, hold, total) in enumerate(CASES):
        cases.append(oracle.Case(
            name=f"env_{k:02d}_{a1:04x}_{a2:04x}",
            fixture=[oracle.Write(CODE_ADDR, 0, code)],
            buffers={"params": struct.pack("<HHII", a1, a2, hold, total), "out": bytes(2 * total)},
            calls=[oracle.Call(CODE_ADDR, [("buf", "params"), ("buf", "out"), sym["VSync"]], "void",
                               reads=[oracle.Read("buf:out", 0, 2 * total)])]))
    jobs = [oracle.job_for(sym, "envelope", c) for c in cases]
    with tempfile.TemporaryDirectory(prefix="dw3_envelope_") as tmp:
        results = oracle.run_oracle(jobs, tmp)
    out = {"comment": "ENVX of voice 23 once per vsync after a key-on (tests/spu/envelope_oracle.py)",
           "oracle": oracle.oracle_meta("CNTY_SEL loaded; the probe is this script's own code in scratch RAM"),
           "cases": []}
    for c, (a1, a2, hold, total) in zip(cases, CASES):
        raw = bytes.fromhex(results[f"envelope/{c.name}"]["calls"][0]["reads"][0])
        out["cases"].append({"adsr1": a1, "adsr2": a2, "hold": hold, "envx": list(struct.unpack(f"<{total}H", raw))})
    return out


def envelope(a1, a2, n, release_from=None):
    """Our ENVX after each of n samples (the reference's ADSR): from a key-on (attack from 0), or with release_from =
    L a release from level L (a key-off: the phase's counter starts at 0)."""
    vo = spu_ref.Voice()
    vo.vol[4], vo.vol[5] = a1, a2
    if release_from is None:
        vo.env, vo.phase = spu_ref.Envelope(0), spu_ref.ATTACK
    else:
        vo.env, vo.phase = spu_ref.Envelope(release_from), spu_ref.RELEASE
    spu = spu_ref.Spu()
    levels = []
    for _ in range(n):
        spu.adsr(vo)
        levels.append(vo.env.level)
    return levels


def hull(levels, value):
    """The positions p (samples since the start, 1-based: levels[p - 1]) where our envelope shows `value`: the first
    and the last (the envelope is monotonic within a phase), or None."""
    try:
        first = levels.index(value)
    except ValueError:
        return None
    last = len(levels) - 1 - levels[::-1].index(value)
    return first + 1, last + 1


# A reading may sit this many samples off the fitted line: the emulator renders whole samples per vsync (877 or 878
# at 877.4 on average), so a vsync falls up to a sample either side of (k + 1) * S.
JITTER = 2


def stab(spans, s, lo, hi):
    """Interval stabbing: the offset t in [lo, hi] that puts the most readings k at a position (k + 1) * s + t inside
    their span; returns (count, t)."""
    events = []
    for k, span in enumerate(spans):
        if span:
            a = max(lo, span[0] - JITTER - (k + 1) * s)
            b = min(hi, span[1] + JITTER + 0.999 - (k + 1) * s)
            if a <= b:
                events += [(a, 0), (b, 1)]
    events.sort()
    depth, top, t = 0, 0, 0.0
    for x, kind in events:
        depth += 1 if kind == 0 else -1
        if kind == 0 and depth > top:
            top, t = depth, x
    return top, t


def fit(case):
    """Reading k (0-based) is taken at vsync k + 1 after the vsync that starts the key-on: sample (k + 1) * S + t0 of our
    envelope (the key-on follows that vsync at once: |t0| <= 50). The key-off follows reading hold - 1: reading k >= hold
    is (k + 1 - hold) * S + d samples into the release from our level at the key-off, d the key-off's offset (|d| <= 50:
    when the emulator applies a store relative to when a read sees the envelope). For each S (870.0 .. 890.0 by 0.1):
    the t0 that fits the most readings before the key-off, then the d that fits the most after it (interval stabbing).
    Returns (fitted, total, t0, d, S)."""
    envx, hold, a1, a2 = case["envx"], case["hold"], case["adsr1"], case["adsr2"]
    n = int((hold + 2) * 900)
    attack = envelope(a1, a2, n)
    spans = [hull(attack, v) for v in envx[:hold]]
    releases = {}
    best = (-1, 0.0, 0.0, 0.0)
    for s10 in range(8700, 8901):
        s = s10 / 10
        top, t0 = stab(spans, s, -50, 50)
        if top < best[0] - (len(envx) - hold):
            continue
        start = attack[int(hold * s + t0) - 1]
        if start not in releases:
            rel = envelope(a1, a2, (len(envx) - hold + 2) * 900, release_from=start)
            releases[start] = (rel, [hull(rel, v) for v in envx[hold:]])
        rel_top, d = stab(releases[start][1], s, -50, 50)
        if top + rel_top > best[0]:
            best = (top + rel_top, t0, d, s)
    return best[0], len(envx), best[1], best[2], best[3]

def check(golden):
    bad = 0
    for case in golden["cases"]:
        ok, total, t0, d, s = fit(case)
        envx = case["envx"]
        print(f"adsr {case['adsr1']:04x} {case['adsr2']:04x}: {ok:3d}/{total} readings fit; S {s:.1f} samples per vsync,"
              f" key-on offset {t0:+.0f}, key-off offset {d:+.0f}; readings {envx[0]:04x} {envx[1]:04x} .. "
              f"{envx[case['hold']]:04x} .. {envx[-1]:04x}")
        bad += ok < total
    print(f"envelope_oracle: {len(golden['cases']) - bad} of {len(golden['cases'])} cases fit every reading")
    return 1 if bad else 0


def write_golden(golden):
    """The readings, one case per line."""
    head = json.dumps({k: v for k, v in golden.items() if k != "cases"}, indent=1)
    cases = ",\n".join("  " + json.dumps(c) for c in golden["cases"])
    GOLDEN.write_text(head[:-2] + ',\n "cases": [\n' + cases + "\n ]\n}\n")


def main():
    if len(sys.argv) != 2 or sys.argv[1] not in ("gen", "check"):
        print(__doc__)
        return 2
    if sys.argv[1] == "gen":
        golden = run_emulator()
        write_golden(golden)
        print(f"wrote {GOLDEN}")
    else:
        golden = json.loads(GOLDEN.read_text())
    return check(golden)


if __name__ == "__main__":
    sys.exit(main())
