#!/usr/bin/env python3
"""A reference model of the PS1 SPU in Python, written from psx-spx "Sound Processing Unit (SPU)" (and its CD-ROM page
for the ADPCM filter tables) independently of the port's C core (psxstack/runtime/spu.c, spu_dsp.c), plus the generator of the
unit goldens that the C core must reproduce (tests/spu/goldens.txt, run by tests/spu/run.sh).

  tools/venv/bin/python tests/spu/spu_ref.py gen      # regenerate tests/spu/goldens.txt (~1 min)
  tools/venv/bin/python tests/spu/spu_ref.py check    # regenerate in memory, compare with the committed file
  tools/venv/bin/python tests/spu/spu_ref.py dump CASE [N]   # the CASE's first trace/render values, one per line

Two implementations of the same documentation agreeing is the check: a reading of psx-spx that one of them gets wrong
shows up as a mismatch. Where psx-spx is silent both take the readings docs/SOUND.md section 6 lists; those are
assumptions, not checked here (the envelope oracle, tests/spu/envelope_oracle.py, compares the ADSR with the emulator).

Golden file format (text, one op per line; '#' comments; every case starts from a reset SPU):
  case NAME
  w OFF VAL               spu_write16(OFF, VAL)
  r OFF VAL               spu_read16(OFF) must be VAL
  dma TSA HEX             TSA written, then the bytes (little-endian halfwords) through spu_dma_write
  cd N HEX                spu_cd_input: N stereo frames (s16 LE, interleaved)
  render N SHA1 [S...]    N frames rendered: the SHA-1 of the s16 LE output (and the samples themselves when short)
  trace N OFF SHA1 LAST   N single-frame renders, spu_read16(OFF) after each: the SHA-1 of the u16 LE values, the last
  ram ADDR LEN SHA1       SPU RAM bytes ADDR..ADDR+LEN
  revaddr OFF ADDR        the reverb's address of a work-area offset (bytes, signed) from the current buffer address
  adpcm HEX H0 H1 S0..S27 H0' H1'   one block decoded with history H0 (last), H1 (the one before)
  gauss I S0 S1 S2 S3 OUT the interpolation at index I over oldest..newest
  env RATE EXP DEC NEG LEVEL N SHA1 LEVEL' COUNTER'   N envelope ticks from LEVEL, counter 0: the levels' SHA-1 (s32 LE)
  end
"""
import hashlib
import random
import struct
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
GOLDENS = HERE / "goldens.txt"

# psx-spx "4-Point Gaussian Interpolation" (entries 000h..1FFh, as printed there).
GAUSS = [
    -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001,
    -0x001, -0x001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002,
    0x0002, 0x0002, 0x0003, 0x0003, 0x0003, 0x0004, 0x0004, 0x0005, 0x0005, 0x0006, 0x0007, 0x0007, 0x0008, 0x0009,
    0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F, 0x0010, 0x0011, 0x0012, 0x0013, 0x0015, 0x0016, 0x0018,
    0x0019, 0x001B, 0x001C, 0x001E, 0x0020, 0x0021, 0x0023, 0x0025, 0x0027, 0x0029, 0x002C, 0x002E, 0x0030, 0x0033,
    0x0035, 0x0038, 0x003A, 0x003D, 0x0040, 0x0043, 0x0046, 0x0049, 0x004D, 0x0050, 0x0054, 0x0057, 0x005B, 0x005F,
    0x0063, 0x0067, 0x006B, 0x006F, 0x0074, 0x0078, 0x007D, 0x0082, 0x0087, 0x008C, 0x0091, 0x0096, 0x009C, 0x00A1,
    0x00A7, 0x00AD, 0x00B3, 0x00BA, 0x00C0, 0x00C7, 0x00CD, 0x00D4, 0x00DB, 0x00E3, 0x00EA, 0x00F2, 0x00FA, 0x0101,
    0x010A, 0x0112, 0x011B, 0x0123, 0x012C, 0x0135, 0x013F, 0x0148, 0x0152, 0x015C, 0x0166, 0x0171, 0x017B, 0x0186,
    0x0191, 0x019C, 0x01A8, 0x01B4, 0x01C0, 0x01CC, 0x01D9, 0x01E5, 0x01F2, 0x0200, 0x020D, 0x021B, 0x0229, 0x0237,
    0x0246, 0x0255, 0x0264, 0x0273, 0x0283, 0x0293, 0x02A3, 0x02B4, 0x02C4, 0x02D6, 0x02E7, 0x02F9, 0x030B, 0x031D,
    0x0330, 0x0343, 0x0356, 0x036A, 0x037E, 0x0392, 0x03A7, 0x03BC, 0x03D1, 0x03E7, 0x03FC, 0x0413, 0x042A, 0x0441,
    0x0458, 0x0470, 0x0488, 0x04A0, 0x04B9, 0x04D2, 0x04EC, 0x0506, 0x0520, 0x053B, 0x0556, 0x0572, 0x058E, 0x05AA,
    0x05C7, 0x05E4, 0x0601, 0x061F, 0x063E, 0x065C, 0x067C, 0x069B, 0x06BB, 0x06DC, 0x06FD, 0x071E, 0x0740, 0x0762,
    0x0784, 0x07A7, 0x07CB, 0x07EF, 0x0813, 0x0838, 0x085D, 0x0883, 0x08A9, 0x08D0, 0x08F7, 0x091E, 0x0946, 0x096F,
    0x0998, 0x09C1, 0x09EB, 0x0A16, 0x0A40, 0x0A6C, 0x0A98, 0x0AC4, 0x0AF1, 0x0B1E, 0x0B4C, 0x0B7A, 0x0BA9, 0x0BD8,
    0x0C07, 0x0C38, 0x0C68, 0x0C99, 0x0CCB, 0x0CFD, 0x0D30, 0x0D63, 0x0D97, 0x0DCB, 0x0E00, 0x0E35, 0x0E6B, 0x0EA1,
    0x0ED7, 0x0F0F, 0x0F46, 0x0F7F, 0x0FB7, 0x0FF1, 0x102A, 0x1065, 0x109F, 0x10DB, 0x1116, 0x1153, 0x118F, 0x11CD,
    0x120B, 0x1249, 0x1288, 0x12C7, 0x1307, 0x1347, 0x1388, 0x13C9, 0x140B, 0x144D, 0x1490, 0x14D4, 0x1517, 0x155C,
    0x15A0, 0x15E6, 0x162C, 0x1672, 0x16B9, 0x1700, 0x1747, 0x1790, 0x17D8, 0x1821, 0x186B, 0x18B5, 0x1900, 0x194B,
    0x1996, 0x19E2, 0x1A2E, 0x1A7B, 0x1AC8, 0x1B16, 0x1B64, 0x1BB3, 0x1C02, 0x1C51, 0x1CA1, 0x1CF1, 0x1D42, 0x1D93,
    0x1DE5, 0x1E37, 0x1E89, 0x1EDC, 0x1F2F, 0x1F82, 0x1FD6, 0x202A, 0x207F, 0x20D4, 0x2129, 0x217F, 0x21D5, 0x222C,
    0x2282, 0x22DA, 0x2331, 0x2389, 0x23E1, 0x2439, 0x2492, 0x24EB, 0x2545, 0x259E, 0x25F8, 0x2653, 0x26AD, 0x2708,
    0x2763, 0x27BE, 0x281A, 0x2876, 0x28D2, 0x292E, 0x298B, 0x29E7, 0x2A44, 0x2AA1, 0x2AFF, 0x2B5C, 0x2BBA, 0x2C18,
    0x2C76, 0x2CD4, 0x2D33, 0x2D91, 0x2DF0, 0x2E4F, 0x2EAE, 0x2F0D, 0x2F6C, 0x2FCC, 0x302B, 0x308B, 0x30EA, 0x314A,
    0x31AA, 0x3209, 0x3269, 0x32C9, 0x3329, 0x3389, 0x33E9, 0x3449, 0x34A9, 0x3509, 0x3569, 0x35C9, 0x3629, 0x3689,
    0x36E8, 0x3748, 0x37A8, 0x3807, 0x3867, 0x38C6, 0x3926, 0x3985, 0x39E4, 0x3A43, 0x3AA2, 0x3B00, 0x3B5F, 0x3BBD,
    0x3C1B, 0x3C79, 0x3CD7, 0x3D35, 0x3D92, 0x3DEF, 0x3E4C, 0x3EA9, 0x3F05, 0x3F62, 0x3FBD, 0x4019, 0x4074, 0x40D0,
    0x412A, 0x4185, 0x41DF, 0x4239, 0x4292, 0x42EB, 0x4344, 0x439C, 0x43F4, 0x444C, 0x44A3, 0x44FA, 0x4550, 0x45A6,
    0x45FC, 0x4651, 0x46A6, 0x46FA, 0x474E, 0x47A1, 0x47F4, 0x4846, 0x4898, 0x48E9, 0x493A, 0x498A, 0x49D9, 0x4A29,
    0x4A77, 0x4AC5, 0x4B13, 0x4B5F, 0x4BAC, 0x4BF7, 0x4C42, 0x4C8D, 0x4CD7, 0x4D20, 0x4D68, 0x4DB0, 0x4DF7, 0x4E3E,
    0x4E84, 0x4EC9, 0x4F0E, 0x4F52, 0x4F95, 0x4FD7, 0x5019, 0x505A, 0x509A, 0x50DA, 0x5118, 0x5156, 0x5194, 0x51D0,
    0x520C, 0x5247, 0x5281, 0x52BA, 0x52F3, 0x532A, 0x5361, 0x5397, 0x53CC, 0x5401, 0x5434, 0x5467, 0x5499, 0x54CA,
    0x54FA, 0x5529, 0x5558, 0x5585, 0x55B2, 0x55DE, 0x5609, 0x5632, 0x565B, 0x5684, 0x56AB, 0x56D1, 0x56F6, 0x571B,
    0x573E, 0x5761, 0x5782, 0x57A3, 0x57C3, 0x57E2, 0x57FF, 0x581C, 0x5838, 0x5853, 0x586D, 0x5886, 0x589E, 0x58B5,
    0x58CB, 0x58E0, 0x58F4, 0x5907, 0x5919, 0x592A, 0x593A, 0x5949, 0x5958, 0x5965, 0x5971, 0x597C, 0x5986, 0x598F,
    0x5997, 0x599E, 0x59A4, 0x59A9, 0x59AD, 0x59B0, 0x59B2, 0x59B3,
]
assert len(GAUSS) == 512
# psx-spx CD-ROM "Pos/neg Tables" (SPU-ADPCM has five filters).
POS = (0, 60, 115, 98, 122)
NEG = (0, 0, -52, -55, -60)
# psx-spx "Reverb Buffer Resampling".
FIR = [-1, 0, 2, 0, -10, 0, 35, 0, -103, 0, 266, 0, -616, 0, 1332, 0, -2960, 0, 10246, 16384,
       10246, 0, -2960, 0, 1332, 0, -616, 0, 266, 0, -103, 0, 35, 0, -10, 0, 2, 0, -1]
RAM_SIZE = 0x80000


def sat(x):
    return max(-32768, min(32767, x))


def s16(x):
    x &= 0xFFFF
    return x - 0x10000 if x & 0x8000 else x


# ------------------------------------------------------------------------------------------------ the DSP pieces
def adpcm_block(block, last, before):
    """psx-spx: header byte 0 = shift (bits 0-3; 13..15 = 9) | filter (bits 4-6); 28 nibbles low first. Each sample:
    the nibble as bits 12-15 of a signed 16-bit value shifted right by the shift, plus (last * pos + before * neg + 32)
    / 64 taken as a floor (an arithmetic shift), clamped. Filters 5..7 (undocumented) as 0."""
    shift = block[0] & 15
    if shift > 12:
        shift = 9
    flt = (block[0] >> 4) & 7
    if flt > 4:
        flt = 0
    out = []
    for k in range(28):
        nib = (block[2 + k // 2] >> (4 * (k % 2))) & 15
        if nib >= 8:
            nib -= 16
        value = (nib * 4096) >> shift
        value += (last * POS[flt] + before * NEG[flt] + 32) // 64
        value = sat(value)
        out.append(value)
        before, last = last, value
    return out, last, before


def interpolate(i, oldest, older, old, new):
    return ((GAUSS[0xFF - i] * oldest) >> 15) + ((GAUSS[0x1FF - i] * older) >> 15) + \
           ((GAUSS[0x100 + i] * old) >> 15) + ((GAUSS[i] * new) >> 15)


class Envelope:
    """psx-spx "Envelope Operation": a level and a step counter, one tick per 44,100 Hz cycle. rate = shift << 2 | step.
    The counter loses 8000h when it steps (psx-spx's earlier "wait 1 SHL (shift-11) cycles" reading)."""

    def __init__(self, level=0):
        self.level = level
        self.counter = 0

    def tick(self, rate, exponential, decrease, negative):
        shift, stepval = (rate >> 2) & 31, rate & 3
        adsr_step = 7 - stepval
        if bool(decrease) != bool(negative):
            adsr_step = -adsr_step - 1   # NOT
        adsr_step *= 2 ** max(0, 11 - shift)
        inc = 0x8000 >> max(0, shift - 11)
        if exponential and not decrease and self.level > 0x6000:
            if shift < 10:
                adsr_step >>= 2
            elif shift >= 11:
                inc >>= 2
            else:
                adsr_step >>= 1
                inc >>= 1
        elif exponential and decrease:
            adsr_step = (adsr_step * self.level) >> 15
        if (rate & 0x7F) != 0x7F:
            inc = max(inc, 1)
        self.counter += inc
        if not self.counter & 0x8000:
            return
        self.counter -= 0x8000
        lvl = self.level + adsr_step
        if not decrease:
            lvl = sat(lvl)
        elif negative:
            lvl = max(-0x8000, min(0, lvl))
        else:
            lvl = max(0, lvl)
        self.level = lvl


# ------------------------------------------------------------------------------------------------------- the SPU
ATTACK, DECAY, SUSTAIN, RELEASE = range(4)


class Voice:
    def __init__(self):
        self.vol = [0, 0, 0, 0, 0, 0]  # registers 0..5: vol.l, vol.r, pitch, ssa, adsr1, adsr2
        self.env = Envelope()
        self.phase = RELEASE
        self.cur_vol = [Envelope(), Envelope()]
        self.addr = 0
        self.repeat = 0
        self.counter = 0
        self.flags = 0
        self.last = self.before = 0
        self.samples = [0] * 31   # three samples of the previous block, then the current block's 28
        self.out = 0


class Spu:
    def __init__(self):
        self.ram = bytearray(RAM_SIZE)
        self.regs = {}            # offset -> value as written (outside the voices)
        self.voices = [Voice() for _ in range(24)]
        self.main_vol = [Envelope(), Envelope()]
        self.endx = 0
        self.transfer = 0
        self.fifo = []
        self.noise = 0
        self.noise_timer = 0
        self.rev_cur = 0
        self.fir_in = [[0] * 39, [0] * 39]   # newest last
        self.fir_out = [[0] * 39, [0] * 39]
        self.capture = 0
        self.cd_queue = []
        self.n = 0

    # -- registers
    def reg(self, off):
        return self.regs.get(off, 0)

    def ram16(self, addr):
        addr &= RAM_SIZE - 2
        return self.ram[addr] | self.ram[addr + 1] << 8

    def set_ram16(self, addr, value):
        addr &= RAM_SIZE - 2
        self.ram[addr] = value & 0xFF
        self.ram[addr + 1] = (value >> 8) & 0xFF

    def mode(self):
        return (self.reg(0x1AA) >> 4) & 3

    def write(self, off, val):
        off &= 0x1FE
        if off in (0x19C, 0x19E, 0x1AE, 0x1B8, 0x1BA):
            return
        if off < 0x180:
            vo, r = self.voices[off >> 4], (off & 15) >> 1
            if r < 6:
                vo.vol[r] = val
                if r < 2 and not val & 0x8000:
                    vo.cur_vol[r].level = s16(val << 1)
                    vo.cur_vol[r].counter = 0
            elif r == 6:
                vo.env.level = s16(val)
            else:
                vo.repeat = (val * 8) % RAM_SIZE
            return
        self.regs[off] = val
        if off in (0x180, 0x182) and not val & 0x8000:
            self.main_vol[(off - 0x180) // 2].level = s16(val << 1)
            self.main_vol[(off - 0x180) // 2].counter = 0
        elif off in (0x188, 0x18A, 0x18C, 0x18E):
            base = 0 if off in (0x188, 0x18C) else 16
            for b in range(16):
                if val >> b & 1 and base + b < 24:
                    (self.key_on if off < 0x18C else self.key_off)(base + b)
        elif off == 0x1A2:
            self.rev_cur = (val * 8) % RAM_SIZE
        elif off == 0x1A6:
            self.transfer = (val * 8) % RAM_SIZE
        elif off == 0x1A8:
            if len(self.fifo) < 32:
                self.fifo.append(val)
            if self.mode() == 1:
                self.flush()
        elif off == 0x1AA and (val >> 4) & 3 == 1:
            self.flush()

    def flush(self):
        for h in self.fifo:
            self.set_ram16(self.transfer, h)
            self.transfer = (self.transfer + 2) % RAM_SIZE
        self.fifo = []

    def dma(self, data):
        for k in range(0, len(data), 2):
            self.set_ram16(self.transfer, data[k] | data[k + 1] << 8)
            self.transfer = (self.transfer + 2) % RAM_SIZE

    def read(self, off):
        off &= 0x1FE
        if off < 0x180:
            vo, r = self.voices[off >> 4], (off & 15) >> 1
            return vo.vol[r] if r < 6 else (vo.env.level & 0xFFFF if r == 6 else vo.repeat // 8)
        if off == 0x19C:
            return self.endx & 0xFFFF
        if off == 0x19E:
            return self.endx >> 16
        if off == 0x1AE:
            attr = self.reg(0x1AA)
            m = (attr >> 4) & 3
            return (attr & 0x3F) | (0x80 if attr & 0x20 else 0) | {2: 0x100, 3: 0x200}.get(m, 0) | \
                (0x800 if self.capture >= 256 else 0)
        if off in (0x1B8, 0x1BA):
            return self.main_vol[(off - 0x1B8) // 2].level & 0xFFFF
        return self.reg(off)

    def cd(self, frames):
        room = 16384 - len(self.cd_queue)
        self.cd_queue.extend(frames[:max(0, room)])

    # -- voices
    def load_block(self, vo):
        blk = bytes(self.ram[(vo.addr + k) % RAM_SIZE] for k in range(16))
        vo.flags = blk[1]
        if vo.flags & 4:
            vo.repeat = vo.addr
        out, vo.last, vo.before = adpcm_block(blk, vo.last, vo.before)
        vo.samples = vo.samples[28:31] + out

    def key_on(self, v):
        vo = self.voices[v]
        vo.addr = (vo.vol[3] * 8) % RAM_SIZE
        vo.counter = 0
        vo.last = vo.before = 0
        vo.samples = [0] * 31
        self.load_block(vo)
        vo.env = Envelope(0)
        vo.phase = ATTACK
        self.endx &= ~(1 << v)

    def key_off(self, v):
        self.voices[v].phase = RELEASE
        self.voices[v].env.counter = 0

    def adsr(self, vo):
        a1, a2 = vo.vol[4], vo.vol[5]
        e = vo.env
        if vo.phase == ATTACK:
            e.tick((a1 >> 8) & 0x7F, a1 >> 15, False, False)
            if e.level >= 0x7FFF:
                vo.phase, e.counter = DECAY, 0
        elif vo.phase == DECAY:
            e.tick(((a1 >> 4) & 15) * 4, True, True, False)
            if e.level <= ((a1 & 15) + 1) * 0x800:
                vo.phase, e.counter = SUSTAIN, 0
        elif vo.phase == SUSTAIN:
            e.tick((a2 >> 6) & 0x7F, a2 >> 15, (a2 >> 14) & 1, False)
        else:
            e.tick((a2 & 31) * 4, (a2 >> 5) & 1, True, False)

    def voice(self, v, pmon, non):
        vo = self.voices[v]
        idx = vo.counter >> 12
        if non >> v & 1:
            raw = s16(self.noise)
        else:
            raw = interpolate((vo.counter >> 4) & 255, *vo.samples[idx:idx + 4])
        out = sat((raw * vo.env.level) >> 15)
        vo.out = out
        left = (out * vo.cur_vol[0].level) >> 15
        right = (out * vo.cur_vol[1].level) >> 15
        self.adsr(vo)
        for k in (0, 1):
            if vo.vol[k] & 0x8000:
                vo.cur_vol[k].tick(vo.vol[k] & 0x7F, (vo.vol[k] >> 14) & 1, (vo.vol[k] >> 13) & 1,
                                   (vo.vol[k] >> 12) & 1)
        step = vo.vol[2]
        if v and pmon >> v & 1:
            step = ((s16(step) * (self.voices[v - 1].out + 0x8000)) >> 15) & 0xFFFF
        step = min(step, 0x4000)
        vo.counter += step
        while vo.counter >= 28 << 12:
            vo.counter -= 28 << 12
            if vo.flags & 1:
                self.endx |= 1 << v
                vo.addr = vo.repeat
                if not vo.flags & 2:
                    vo.env.level, vo.env.counter, vo.phase = 0, 0, RELEASE
            else:
                vo.addr = (vo.addr + 16) % RAM_SIZE
            self.load_block(vo)
        return out, left, right

    def idle(self, v, pmon):
        """A voice that cannot change anything but its envelope counter this sample (the fast path of the reference:
        released at 0, no pitch, no modulation, fixed volumes)."""
        vo = self.voices[v]
        return (vo.phase == RELEASE and vo.env.level == 0 and vo.vol[2] == 0 and not (pmon >> v & 1)
                and not vo.vol[0] & 0x8000 and not vo.vol[1] & 0x8000)

    # -- reverb
    def rev_addr(self, off):
        esa = self.reg(0x1A2) * 8
        size = RAM_SIZE - esa
        return (esa + (self.rev_cur - esa + off) % size) & (RAM_SIZE - 2)

    def rv(self, off):
        return s16(self.ram16(self.rev_addr(off)))

    def reverb(self, lin_raw, rin_raw):
        r = [self.reg(0x1C0 + 2 * k) for k in range(32)]
        vol = [s16(x) for x in r]
        adr = [x * 8 for x in r]
        on = bool(self.reg(0x1AA) & 0x80)
        mul = lambda a, b: (a * b) >> 15   # noqa: E731
        out = []
        for side in (0, 1):
            inp = mul((lin_raw, rin_raw)[side], vol[30 + side])
            m_same, d_same = adr[10 + side], adr[16 + side]
            m_diff, d_diff = adr[18 + side], adr[25 - side]
            prev = self.rv(m_same - 2)
            x = sat(inp + mul(self.rv(d_same), vol[7]))
            x = sat(x - prev)
            same = sat(mul(x, vol[2]) + prev)
            if on:
                self.set_ram16(self.rev_addr(m_same), same & 0xFFFF)
            prev = self.rv(m_diff - 2)
            x = sat(inp + mul(self.rv(d_diff), vol[7]))
            x = sat(x - prev)
            diff = sat(mul(x, vol[2]) + prev)
            comb = mul(vol[3], self.rv(adr[12 + side]))
            if on:
                self.set_ram16(self.rev_addr(m_diff), diff & 0xFFFF)
            comb += mul(vol[4], self.rv(adr[14 + side]))
            comb += mul(vol[5], self.rv(adr[20 + side]))
            comb += mul(vol[6], self.rv(adr[22 + side]))
            acc = sat(comb)
            a1 = self.rv(adr[26 + side] - adr[0])
            a2 = self.rv(adr[28 + side] - adr[1])
            acc = sat(acc - mul(vol[8], a1))
            if on:
                self.set_ram16(self.rev_addr(adr[26 + side]), acc & 0xFFFF)
            acc = sat(mul(acc, vol[8]) + a1)
            acc = sat(acc - mul(vol[9], a2))
            if on:
                self.set_ram16(self.rev_addr(adr[28 + side]), acc & 0xFFFF)
            acc = sat(mul(acc, vol[9]) + a2)
            out.append(sat(mul(acc, s16(self.reg(0x184 + 2 * side)))))
        esa = self.reg(0x1A2) * 8
        self.rev_cur = max(esa, (self.rev_cur + 2) & 0x7FFFE)
        return out

    # -- one sample
    def sample(self):
        attr = self.reg(0x1AA)
        # noise (psx-spx "SPU Noise Generator")
        self.noise_timer -= ((attr >> 8) & 3) + 4
        bit = ((self.noise >> 15) ^ (self.noise >> 12) ^ (self.noise >> 11) ^ (self.noise >> 10) ^ 1) & 1
        if self.noise_timer < 0:
            self.noise = (self.noise * 2 + bit) & 0xFFFF
            for _ in range(2):
                if self.noise_timer < 0:
                    self.noise_timer += 0x20000 >> ((attr >> 10) & 15)
        pmon = self.reg(0x190) | self.reg(0x192) << 16
        non = self.reg(0x194) | self.reg(0x196) << 16
        eon = self.reg(0x198) | self.reg(0x19A) << 16
        dry, wet_in = [0, 0], [0, 0]
        cap = self.capture * 2
        for v in range(24):
            if self.idle(v, pmon):
                self.adsr(self.voices[v])
                self.voices[v].out = out = left = right = 0
            else:
                out, left, right = self.voice(v, pmon, non)
            dry[0] += left
            dry[1] += right
            if eon >> v & 1:
                wet_in[0] += left
                wet_in[1] += right
            if v in (1, 3):
                self.set_ram16((0x800 if v == 1 else 0xC00) + cap, out & 0xFFFF)
        cd = list(self.cd_queue.pop(0)) if self.cd_queue else [0, 0]
        self.set_ram16(cap, cd[0] & 0xFFFF)
        self.set_ram16(0x400 + cap, cd[1] & 0xFFFF)
        self.capture = (self.capture + 1) % 512
        for c in (0, 1):
            cd[c] = (cd[c] * s16(self.reg(0x1B0 + 2 * c))) >> 15
            if attr & 4:
                wet_in[c] += cd[c]
            if not attr & 1:
                cd[c] = 0
        # the reverb at 22,050 Hz between the two resampling filters
        for c in (0, 1):
            self.fir_in[c] = self.fir_in[c][1:] + [sat(wet_in[c])]
        if self.n % 2 == 0:
            down = [sat(sum(a * b for a, b in zip(FIR, self.fir_in[c])) >> 15) for c in (0, 1)]
            wet = self.reverb(*down)
        else:
            wet = [0, 0]
        rev = []
        for c in (0, 1):
            self.fir_out[c] = self.fir_out[c][1:] + [wet[c]]
            rev.append(sat(sum(a * b for a, b in zip(FIR, self.fir_out[c])) >> 14))
        result = []
        for c in (0, 1):
            s = sat(dry[c]) + rev[c] if (attr & 0xC000) == 0xC000 else 0
            s = sat(s + cd[c])
            result.append(sat((s * self.main_vol[c].level) >> 15))
            mv = self.reg(0x180 + 2 * c)
            if mv & 0x8000:
                self.main_vol[c].tick(mv & 0x7F, (mv >> 14) & 1, (mv >> 13) & 1, (mv >> 12) & 1)
        self.n += 1
        return result

    def render(self, n):
        out = []
        for _ in range(n):
            out += self.sample()
        return out


# --------------------------------------------------------------------------------------------- the golden cases
def sha(data):
    return hashlib.sha1(data).hexdigest()


def pack16(values):
    return struct.pack(f"<{len(values)}h", *values)


class Case:
    """A case's ops, run on the reference as they are added; text() is the golden."""

    def __init__(self, name, comment=""):
        self.spu = Spu()
        self.name = name
        self.lines = [f"case {name}"] + ([f"# {comment}"] if comment else [])
        self.values = []   # (op line, values) of the renders and traces: `dump`

    def w(self, off, val):
        self.spu.write(off, val)
        self.lines.append(f"w {off:03x} {val:04x}")

    def r(self, off):
        self.lines.append(f"r {off:03x} {self.spu.read(off):04x}")

    def dma(self, tsa, data):
        self.spu.write(0x1A6, tsa)
        self.spu.dma(data)
        self.lines.append(f"dma {tsa:04x} {data.hex()}")

    def cd(self, frames):
        self.spu.cd(frames)
        self.lines.append(f"cd {len(frames)} {pack16([x for f in frames for x in f]).hex()}")

    def render(self, n, show=None):
        out = self.spu.render(n)
        show = n <= 16 if show is None else show
        extra = (" " + " ".join(str(x) for x in out)) if show else ""
        self.lines.append(f"render {n} {sha(pack16(out))}{extra}")
        self.values.append((self.lines[-1][:60], out))
        return out

    def trace(self, n, off):
        vals = []
        for _ in range(n):
            self.spu.sample()
            vals.append(self.spu.read(off))
        self.lines.append(f"trace {n} {off:03x} {sha(struct.pack(f'<{n}H', *vals))} {vals[-1]:04x}")
        self.values.append((self.lines[-1], vals))
        return vals

    def ram(self, addr, length):
        self.lines.append(f"ram {addr:05x} {length:x} {sha(bytes(self.spu.ram[addr:addr + length]))}")

    def revaddr(self, off):
        self.lines.append(f"revaddr {off} {self.spu.rev_addr(off):05x}")

    def comment(self, text):
        self.lines.append(f"# {text}")

    def text(self):
        return "\n".join(self.lines + ["end"]) + "\n"


def block(shift, flt, flags, nibbles):
    data = bytes([(flt << 4) | shift, flags])
    return data + bytes((nibbles[2 * k] & 15) | (nibbles[2 * k + 1] & 15) << 4 for k in range(14))


def dc_block(level_nibble, flags, shift=0):
    return block(shift, 0, flags, [level_nibble] * 28)


def unit_cases(rnd):
    """The pure pieces: ADPCM blocks, the interpolation, the envelope generator."""
    lines = ["case tables",
             "# the table is psx-spx's: its SHA-1 (s16 LE) and the documented sums of each four entries (7F7Fh..7F81h)",
             f"gausstable {sha(pack16(GAUSS))} "
             f"{min(GAUSS[i] + GAUSS[0xFF - i] + GAUSS[0x100 + i] + GAUSS[0x1FF - i] for i in range(256)):x} "
             f"{max(GAUSS[i] + GAUSS[0xFF - i] + GAUSS[0x100 + i] + GAUSS[0x1FF - i] for i in range(256)):x}",
             "# ADPCM: every filter (0..4, and 5..7 taken as 0) with every shift (13..15 act as 9), random nibbles and"
             " history"]
    for flt in range(8):
        for shift in range(16):
            for _ in range(2 if flt < 5 else 1):
                blk = block(shift, flt, rnd.randrange(8), [rnd.randrange(16) for _ in range(28)])
                h0, h1 = rnd.randint(-32768, 32767), rnd.randint(-32768, 32767)
                out, n0, n1 = adpcm_block(blk, h0, h1)
                lines.append(f"adpcm {blk.hex()} {h0} {h1} {' '.join(map(str, out))} {n0} {n1}")
    lines.append("# ADPCM clamps: extreme history and nibbles, shift 0, every filter")
    for flt in range(5):
        for h0, h1, nib in ((32767, -32768, 7), (-32768, 32767, 8), (32767, 32767, 7), (-32768, -32768, 8),
                            (20000, -20000, 0)):
            blk = block(0, flt, 0, [nib] * 28)
            out, n0, n1 = adpcm_block(blk, h0, h1)
            lines.append(f"adpcm {blk.hex()} {h0} {h1} {' '.join(map(str, out))} {n0} {n1}")
    lines.append("# the interpolation at known phases (0, 1, 7Fh, 80h, FFh) and random ones; extreme and random samples")
    sets = [(32767,) * 4, (-32768,) * 4, (32767, -32768, 32767, -32768), (0, 0, 0, 32767), (32767, 0, 0, 0)]
    sets += [tuple(rnd.randint(-32768, 32767) for _ in range(4)) for _ in range(6)]
    for s in sets:
        for i in (0, 1, 0x7F, 0x80, 0xFF, rnd.randrange(256), rnd.randrange(256)):
            lines.append(f"gauss {i} {' '.join(map(str, s))} {interpolate(i, *s)}")
    lines.append("# the envelope generator: every rate in each mode (lin/exp, increase/decrease, phase), 3000 ticks")
    for rate in range(128):
        for exp, dec, neg, level in ((0, 0, 0, 0), (1, 0, 0, 0x5000), (0, 1, 0, 0x7FFF), (1, 1, 0, 0x7FFF),
                                     (0, 0, 1, 0x1000), (0, 1, 1, -0x7000)):
            if neg and rate % 8:
                continue
            env, levels = Envelope(level), []
            for _ in range(3000):
                env.tick(rate, exp, dec, neg)
                levels.append(env.level)
            lines.append(f"env {rate:02x} {exp} {dec} {neg} {level} 3000 "
                         f"{sha(struct.pack('<3000i', *levels))} {env.level} {env.counter}")
    return "\n".join(lines + ["end"]) + "\n"


# The voice used by the scenarios: a silent self-looping block at 0x1000 (SsInit's), samples from 0x2000 on.
SILENT = 0x1000


def setup(c, attr=0xC000, mvol=0x3FFF):
    c.dma(SILENT // 8, dc_block(0, 7))
    c.w(0x1AA, attr)
    c.w(0x180, mvol)
    c.w(0x182, mvol)


def voice(c, v, pitch=0x1000, addr=SILENT, adsr1=0x00FF, adsr2=0x0000, vl=0x3FFF, vr=0x3FFF):
    base = 16 * v
    c.w(base + 0, vl)
    c.w(base + 2, vr)
    c.w(base + 4, pitch)
    c.w(base + 6, addr // 8)
    c.w(base + 8, adsr1)
    c.w(base + 10, adsr2)


def kon(c, mask):
    if mask & 0xFFFF:
        c.w(0x188, mask & 0xFFFF)
    if mask >> 16:
        c.w(0x18A, mask >> 16)


def koff(c, mask):
    if mask & 0xFFFF:
        c.w(0x18C, mask & 0xFFFF)
    if mask >> 16:
        c.w(0x18E, mask >> 16)


# ADSR words: the game's (from the CNTY_SEL trace) and a spread of modes and rates.
ADSRS = [(0x80FF, 0x5FCF), (0x87FF, 0x5FCF), (0x87BA, 0x500A), (0x9BBC, 0x500A), (0x87FF, 0x5FC7), (0x80FF, 0x5FCD),
         (0x87FF, 0x5FCA),
         (0x0000, 0x0000), (0x3F00, 0x0000), (0x2800, 0x00C0), (0x3C00, 0x0000),
         (0xBC00, 0x0000), (0xA400, 0x0000), (0x2C0A, 0x4000), (0x00F5, 0xC0C3), (0x0043, 0x8E40), (0x00A0, 0x8014),
         (0x000F, 0x0025), (0x1F7F, 0x9FFF), (0x7F00, 0x0000), (0x0000, 0x7FC0), (0x0080, 0x4A85), (0x0D3F, 0xC4A6)]


def scenario_cases(rnd):
    cases = []

    # -- ADSR through the registers: key on, hold, key off, ENVX after every sample.
    for k, (a1, a2) in enumerate(ADSRS):
        c = Case(f"adsr_{k:02d}_{a1:04x}_{a2:04x}", "ENVX (0x0C) after each sample: key on at 0, key off at 6000")
        setup(c)
        voice(c, 0, adsr1=a1, adsr2=a2)
        kon(c, 1)
        levels = c.trace(6000, 0x00C)
        koff(c, 1)
        levels += c.trace(6000, 0x00C)
        marks = []
        if 0x7FFF in levels:
            marks.append(f"attack reaches 7FFFh after {levels.index(0x7FFF) + 1} samples")
        zero = next((i for i in range(6000, 12000) if levels[i] == 0), None)
        marks.append(f"level {levels[5999]:04x} at key off" + (f", 0 after {zero - 5999} samples of release"
                                                              if zero is not None else ", not 0 after 6000 samples"))
        c.comment("; ".join(marks))
        cases.append(c)

    # -- ADPCM through a voice at pitch 1000h: a random sample of 12 blocks, every filter and shift on the way.
    c = Case("voice_adpcm", "12 random blocks (every filter), pitch 1000h, full volume, ADSR at 7FFFh at once")
    setup(c)
    data = b"".join(block(rnd.randrange(13), k % 5, 0, [rnd.randrange(16) for _ in range(28)]) for k in range(11))
    data += dc_block(0, 7)
    c.dma(0x2000 // 8, data)
    voice(c, 0, addr=0x2000, adsr1=0x0000, adsr2=0x0000, vl=0x3FFF, vr=0x2000)
    kon(c, 1)
    c.render(400)
    cases.append(c)

    # -- pitch: stepping and the interpolation phases, the 4000h clip, 0 (stopped).
    for pitch in (0x1000, 0x0800, 0x0123, 0x1FFF, 0x3FFF, 0x4000, 0x7000, 0xFFFF, 0x0000):
        c = Case(f"pitch_{pitch:04x}", "a random 4-block loop played at this pitch")
        setup(c)
        data = block(4, 1, 4, [rnd.randrange(16) for _ in range(28)])
        data += b"".join(block(4, 1, 0, [rnd.randrange(16) for _ in range(28)]) for _ in range(2))
        data += block(4, 1, 3, [rnd.randrange(16) for _ in range(28)])
        c.dma(0x3000 // 8, data)
        voice(c, 5, pitch=pitch, addr=0x3000, adsr1=0x0000, adsr2=0x0000)
        kon(c, 1 << 5)
        c.render(500)
        c.r(0x19C)
        cases.append(c)

    # -- loops and ENDX: one-shot (end+mute), a loop over all blocks, a loop from a middle block, no start flag.
    layouts = {
        "oneshot": [(0x1, 0), (0x2, 0), (0x3, 1), (0x0, 7)],     # (nibble, flags) per block: ends with code 1
        "loop_all": [(0x1, 4), (0x2, 0), (0x3, 3)],
        "loop_mid": [(0x1, 0), (0x2, 6), (0x4, 0), (0x5, 3)],
        "loop_nostart": [(0x1, 0), (0x2, 0), (0x3, 3)],         # repeats to whatever the repeat address holds
        "code2": [(0x1, 0), (0x2, 2), (0x3, 0), (0x4, 1), (0x0, 7)],   # code 2 is code 0
    }
    for name, blocks in layouts.items():
        c = Case(f"flags_{name}", "DC blocks (nibble = level) with these flags; pitch 4000h; ENDX, LSAX, ENVX per sample")
        setup(c)
        c.dma(0x4000 // 8, b"".join(dc_block(n, f, shift=4) for n, f in blocks))
        voice(c, 2, pitch=0x4000, addr=0x4000, adsr1=0x0000, adsr2=0x0000)
        c.w(0x02E, 0x5000 // 8)   # repeat address before key on: where "loop_nostart" goes
        kon(c, 1 << 2)
        c.trace(40, 0x19C)
        c.trace(40, 0x02E)
        c.trace(40, 0x02C)
        c.render(64, show=True)
        kon(c, 1 << 2)             # key on again: ENDX clears
        c.r(0x19C)
        c.render(8)
        cases.append(c)

    # -- key on/off within one tick: off then on (restart), on then off (released at once), on twice, off of an idle
    # voice; registers written after the key on (no effect until the next one).
    c = Case("keys", "key-on/key-off orders between two renders")
    setup(c)
    c.dma(0x4000 // 8, dc_block(0x3, 4) + dc_block(0x5, 3))
    for v in range(4):
        voice(c, v, pitch=0x1000, addr=0x4000, adsr1=0x0A00, adsr2=0x0000)
    kon(c, 0xF)
    c.trace(300, 0x00C)
    koff(c, 1)
    kon(c, 1)
    kon(c, 2)
    koff(c, 2)
    c.w(0x024, 0x0800)       # voice 2's pitch, used at once
    c.w(0x036, SILENT // 8)  # voice 3's start: used at its next key on only
    c.trace(200, 0x00C)
    c.trace(200, 0x01C)
    c.trace(200, 0x03C)
    c.render(300)
    kon(c, 8)
    c.render(300)
    koff(c, 0xFFFFFF)
    kon(c, 1 << 23)
    c.r(0x19C)
    c.r(0x19E)
    c.r(0x188)
    c.r(0x18C)
    c.trace(100, 0x17C)
    cases.append(c)

    # -- ENVX and LSAX written directly.
    c = Case("envx_write", "ENVX and the repeat address written by the CPU")
    setup(c)
    c.dma(0x4000 // 8, dc_block(0x6, 4) + dc_block(0x2, 3) + dc_block(0x7, 3))
    voice(c, 0, addr=0x4000, adsr1=0x7F00, adsr2=0x0000)   # attack rate 7Fh: never steps
    kon(c, 1)
    c.w(0x00C, 0x4000)
    c.trace(100, 0x00C)
    c.w(0x00E, 0x4020 // 8)
    c.render(200)
    c.r(0x00E)
    cases.append(c)

    # -- volumes: direct (positive, negative), every sweep mode, the main volume's sweep.
    sweeps = [0x3FFF, 0x4000, 0x2000, 0x8000 | 0x0000, 0x8000 | 0x007F, 0x8000 | 0x0013, 0x8000 | 0x4013,
              0x8000 | 0x2013, 0x8000 | 0x6013, 0x8000 | 0x1013, 0x8000 | 0x3013, 0x8000 | 0x5013, 0x8000 | 0x7013,
              0x8000 | 0x2050, 0x8000 | 0x0030]
    for k, sv in enumerate(sweeps):
        c = Case(f"volume_{k:02d}_{sv:04x}", "a DC voice: direct volume 1000h, then this volume word (L) and its negation"
                 " pattern (R); main volume sweeping down")
        setup(c)
        c.dma(0x4000 // 8, dc_block(0x4, 6) + dc_block(0x4, 3))
        voice(c, 0, addr=0x4000, adsr1=0x0000, adsr2=0x0000, vl=0x1000, vr=0x7000)
        kon(c, 1)
        c.render(50)
        c.w(0x000, sv)
        c.w(0x002, sv ^ 0x1000 if sv & 0x8000 else (-sv) & 0x7FFF)
        c.trace(1500, 0x000)
        c.render(3000)
        c.w(0x180, 0x8000 | 0x2000 | 0x0044)
        c.render(4000)
        c.r(0x1B8)
        c.r(0x1BA)
        cases.append(c)

    # -- the mix's clamps: 24 loud voices, a full main volume.
    c = Case("clamp", "24 voices of a loud DC sample at full volume, main volume 3FFFh then -4000h")
    setup(c)
    c.dma(0x4000 // 8, dc_block(0x7, 6, shift=0) + dc_block(0x7, 3, shift=0))
    for v in range(24):
        voice(c, v, addr=0x4000, adsr1=0x0000, adsr2=0x0000, vl=0x3FFF, vr=0x4000 if v % 2 else 0x3FFF)
    kon(c, 0xFFFFFF)
    c.render(200)
    c.w(0x180, 0x4000)
    c.render(100)
    c.w(0x1AA, 0x8000)     # muted
    c.render(10)
    cases.append(c)

    # -- noise: two voices on the noise generator at several clocks.
    for k, attr in enumerate((0xC000, 0xC100 | 0x3C00, 0xC300 | 0x2000, 0xC200 | 0x1400)):
        c = Case(f"noise_{k}", f"NON on voices 0 and 1, ATTR {attr:04x}")
        setup(c, attr=attr)
        voice(c, 0, adsr1=0x0000, adsr2=0x0000)
        voice(c, 1, adsr1=0x0F00, adsr2=0x0000, vl=0x0800, vr=0x3000)
        c.w(0x194, 0x0003)
        kon(c, 3)
        c.render(2000)
        cases.append(c)

    # -- pitch modulation: voice 0 a slow loud wave (silent: volume 0), voice 1 modulated by it; 8000h+ pitches.
    for pitch in (0x1000, 0x0400, 0x7000, 0x9000):
        c = Case(f"pmon_{pitch:04x}", "voice 1 pitch-modulated by voice 0")
        setup(c)
        c.dma(0x4000 // 8, block(0, 0, 4, [7] * 14 + [8] * 14) + block(0, 0, 3, [8] * 14 + [7] * 14))
        c.dma(0x5000 // 8, block(2, 1, 4, [rnd.randrange(16) for _ in range(28)]) +
              block(2, 1, 3, [rnd.randrange(16) for _ in range(28)]))
        voice(c, 0, pitch=0x0100, addr=0x4000, adsr1=0x0000, adsr2=0x0000, vl=0, vr=0)
        voice(c, 1, pitch=pitch, addr=0x5000, adsr1=0x0000, adsr2=0x0000)
        c.w(0x190, 0x0003)   # bit 0 (voice 0) is unused
        kon(c, 3)
        c.render(3000)
        cases.append(c)

    # -- transfers: the FIFO (manual write), DMA, STATX in each mode, the transfer address wrapping.
    c = Case("transfer", "FIFO writes, manual-write mode, DMA, STATX")
    c.w(0x1AC, 0x0004)
    c.w(0x1A6, 0x0200)
    for k in range(8):
        c.w(0x1A8, 0x0707)
    c.ram(0x1000, 16)
    c.w(0x1AA, 0x0010)
    c.r(0x1AE)
    c.ram(0x1000, 16)
    c.w(0x1A8, 0x1234)       # written at once while in manual-write mode
    c.ram(0x1000, 32)
    c.w(0x1AA, 0x0000)
    for k in range(40):     # more than the FIFO holds
        c.w(0x1A8, 0x1000 + k)
    c.w(0x1AA, 0xC010)
    c.ram(0x1010, 0x60)
    for attr in (0xC020, 0xC030, 0xC0A1, 0xC081):
        c.w(0x1AA, attr)
        c.r(0x1AE)
        c.r(0x1AA)
    c.dma(0xFFFF, bytes(range(32)))   # wraps past the end of SPU RAM
    c.ram(0x7FFF8, 8)
    c.ram(0x00000, 24)
    for off in (0x1A6, 0x1A2, 0x1A4, 0x1AC, 0x1B0, 0x1B4, 0x1A0, 0x1BC):
        c.w(off, 0x1357 + off)
        c.r(off)
    c.render(1000)
    c.r(0x1AE)
    cases.append(c)

    # -- CD input: mixed by CD volume with ATTR bit 0, into the reverb with bit 2, into the capture buffers always.
    c = Case("cd_input", "CD frames through the CD volume, ATTR bit 0 off then on; the capture buffers")
    setup(c, attr=0xC000)
    frames = [(rnd.randint(-32768, 32767), rnd.randint(-32768, 32767)) for _ in range(700)]
    c.cd(frames)
    c.w(0x1B0, 0x7FFE)
    c.w(0x1B2, 0x4000)
    c.render(100)
    c.w(0x1AA, 0xC001)
    c.render(300)
    c.w(0x1AA, 0x0001)     # SPU off and muted: the CD still plays
    c.render(300)
    c.render(20)           # the queue ran dry: silence
    c.ram(0x0000, 0x800)
    c.r(0x1AE)
    c.comment("then into the reverb (ATTR bit 2; psx-spx's Room preset, reverb volume 3000h), not to the mix")
    for k, value in enumerate([0x007D, 0x005B, 0x6D80, 0x54B8, 0xBED0, 0, 0, 0xBA80, 0x5800, 0x5300, 0x04D6, 0x0333,
                               0x03F0, 0x0227, 0x0374, 0x01EF, 0x0334, 0x01B5, 0, 0, 0, 0, 0, 0, 0, 0, 0x01B4, 0x0136,
                               0x00B8, 0x005C, 0x8000, 0x8000]):
        c.w(0x1C0 + 2 * k, value)
    c.w(0x1A2, (0x80000 - 0x26C0) // 8)
    c.w(0x184, 0x3000)
    c.w(0x186, 0x3000)
    c.w(0x1AA, 0xC084)
    c.cd(frames[:400])
    c.render(3000)
    cases.append(c)

    # -- the capture of voices 1 and 3.
    c = Case("capture", "voices 1 and 3 after their envelope in SPU RAM 800h..FFFh")
    setup(c)
    c.dma(0x4000 // 8, block(1, 2, 4, [rnd.randrange(16) for _ in range(28)]) +
          block(1, 2, 3, [rnd.randrange(16) for _ in range(28)]))
    for v in range(4):
        voice(c, v, pitch=0x0C00 + v * 0x100, addr=0x4000, adsr1=0x0400, adsr2=0x0000)
    kon(c, 0xF)
    c.render(700)
    c.ram(0x0800, 0x800)
    cases.append(c)

    # -- reverb: the address arithmetic, then impulse responses of the game's preset (psx-spx "Studio Medium",
    # SsUtSetReverbType(3)'s registers in the trace) with reverb writes on, off, and the "Room" preset's -8000h vLIN.
    studio_medium = [0x00B1, 0x007F, 0x70F0, 0x4FA8, 0xBCE0, 0x4510, 0xBEF0, 0xB4C0, 0x5280, 0x4EC0, 0x0904, 0x076B,
                     0x0824, 0x065F, 0x07A2, 0x0616, 0x076C, 0x05ED, 0x05EC, 0x042E, 0x050F, 0x0305, 0x0462, 0x02B7,
                     0x042F, 0x0265, 0x0264, 0x01B2, 0x0100, 0x0080, 0x8000, 0x8000]
    room = [0x007D, 0x005B, 0x6D80, 0x54B8, 0xBED0, 0x0000, 0x0000, 0xBA80, 0x5800, 0x5300, 0x04D6, 0x0333, 0x03F0,
            0x0227, 0x0374, 0x01EF, 0x0334, 0x01B5, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
            0x01B4, 0x0136, 0x00B8, 0x005C, 0x8000, 0x8000]
    c = Case("reverb_address", "ESA F6F8h (the game's): offsets around the work area's ends, after 0, 3 and 37000 ticks")
    c.w(0x1A2, 0xF6F8)
    for ticks in (0, 6, 74000):
        if ticks:
            c.render(ticks)
        for off in (0, 2, -2, 0x4838, 0x483E, 0x4840, -0x4840, 0x9000, -0x9002, 0x80000, 0x7FFFE):
            c.revaddr(off)
    c.w(0x1A2, 0xFFFF)        # an 8-byte area
    c.render(9)
    for off in (0, 2, 6, 8, -2):
        c.revaddr(off)
    c.w(0x1A2, 0x0000)        # the whole RAM
    c.render(5)
    for off in (0, -2, 0x7FFFE, 0x80000):
        c.revaddr(off)
    cases.append(c)
    for name, preset, attr, esa in (("studio_medium", studio_medium, 0xC080, 0xF6F8),
                                    ("studio_medium_nowrite", studio_medium, 0xC000, 0xF6F8),
                                    ("room", room, 0xC080, (0x80000 - 0x26C0) // 8)):
        c = Case(f"reverb_{name}", "an impulse-like click on voice 0 (EON), reverb volume 3870h (the game's depth): the"
                 " dry click in the first frames, then only the reverb's tail")
        setup(c, attr=attr)
        click = [0] * 28
        click[3], click[4] = 7, 8
        c.dma(0x4000 // 8, block(0, 0, 0, click) + dc_block(0, 7))
        for k, value in enumerate(preset):
            c.w(0x1C0 + 2 * k, value)
        c.w(0x1A2, esa)
        c.w(0x184, 0x3870)
        c.w(0x186, 0x3870)
        voice(c, 0, addr=0x4000, adsr1=0x0000, adsr2=0x0000)
        c.w(0x198, 0x0001)
        kon(c, 1)
        out = c.render(4)
        c.w(0x180, 0x3FFF)
        out = c.render(20000)
        energy = [sum(x * x for x in out[k:k + 4000]) // 4000 for k in range(0, 40000, 4000)]
        c.comment("the response's mean square per 2000 frames (L and R interleaved): " + " ".join(map(str, energy)))
        c.ram(esa * 8, 0x80000 - esa * 8)
        cases.append(c)
    return cases


def generate():
    rnd = random.Random(0x5B0_2003)
    text = ("# tests/spu/goldens.txt: generated by tests/spu/spu_ref.py gen (the Python reference of psx-spx's SPU);\n"
            "# replayed through the port's C core by tests/spu/run.sh. Do not edit by hand.\n")
    text += unit_cases(rnd)
    for c in scenario_cases(rnd):
        text += c.text()
    return text


def main():
    if len(sys.argv) < 2 or sys.argv[1] not in ("gen", "check", "dump"):
        print(__doc__)
        return 2
    if sys.argv[1] == "dump":
        name, limit = sys.argv[2], int(sys.argv[3]) if len(sys.argv) > 3 else 10 ** 9
        rnd = random.Random(0x5B0_2003)
        unit_cases(rnd)
        for c in scenario_cases(rnd):
            if c.name == name:
                for line, vals in c.values:
                    print("#", line)
                    print("\n".join(str(v) for v in vals[:limit]))
        return 0
    text = generate()
    if sys.argv[1] == "gen":
        GOLDENS.write_text(text)
        print(f"wrote {GOLDENS} ({text.count(chr(10))} lines, {text.count(chr(10) + 'case ')} cases)")
        return 0
    old = GOLDENS.read_text() if GOLDENS.exists() else ""
    if old != text:
        print("spu_ref.py check: tests/spu/goldens.txt differs from the reference's output")
        return 1
    print(f"spu_ref.py check: goldens.txt matches the reference ({text.count(chr(10) + 'case ')} cases)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
