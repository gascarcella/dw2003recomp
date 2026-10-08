"""The GTE (the PS1's geometry coprocessor, COP2) and LIBGTE's matrix functions, for the port's software GTE
(psxstack/psyq/gte.c, psxstack/psyq/libgte.c; tests/host/gte_replay.py replays these cases through it).

The game's code is not called for the GTE cases: small MIPS routines written into scratch RAM (this module assembles
them) load every GTE register from a case's `in` buffer (32 data words, then 32 control words: ctc2 for all 32 control
registers, lwc2 for the data registers but 15 SXYP, 28 IRGB, 29 ORGB, 31 LZCR, which are a FIFO push, a conversion
input and two read-only registers), issue one command, and store every register back (swc2, cfc2) at OUT_ADDR: the
golden is the 64 words the PS1 left. Each routine keeps the GTE state it found (saved before, reloaded after), so the
families that run later see the GTE as the game left it.

Cases (fixed seed): every command with sf 0/1 and lm 0/1 on random registers, half in the ranges the game uses and half
anywhere (with garbage in the unused high halves of the 16-bit registers); MVMVA with every mx/v/cv; the game's own
command words (include/psyq/gtemac.h); no command at all (the registers' read/write semantics); edges: the 44-bit MAC
overflow (also one that overflows midway and comes back), IR/colour saturation, NCLIP and AVSZ MAC0 overflow and OTZ
limits, SXY clamps; and RTPS sweeps over SZ3 that store MAC0 (= the division's result n with DQA 1, DQB 0) and FLAG per
step, covering every entry of the UNR table, H >= 2 * SZ3 (overflow) and SZ3 saturation.
LIBGTE: rsin/rcos over a whole turn and outside it, RotMatrixYXZ_gte/ZYX_gte, ScaleMatrix and ApplyMatrixSV on random
inputs (the functions in the EXE, called through a wrapper that loads given GTE registers first and stores them
after: the GTE state each function leaves is compared too).
"""
import random
import struct

from oracle import Call, Case, Read, Write

COMMENT = ("The GTE: each command (sf/lm, MVMVA's mx/v/cv) on random and edge register sets, run by MIPS routines in "
           "scratch RAM that load all 64 registers, issue the command and store them back; RTPS division sweeps; "
           "LIBGTE's rsin/rcos, RotMatrixYXZ_gte/ZYX_gte, ScaleMatrix, ApplyMatrixSV.")
RESIDENT = "CNTY_SEL loaded; the GTE routines are this family's own code in scratch RAM"

# Scratch RAM (tests/golden/oracle.py SCRATCH_BASE..+0x8000): the case's buffers from 0x80180000, then fixed areas.
OUT_ADDR = 0x80180800    # the 64 registers after the command
SAVE_ADDR = 0x80180900   # the GTE state the routine found (reloaded before it returns)
GTE_IN_ADDR = 0x80180A00 # the registers "wrap" loads before it calls a LIBGTE function
CODE_ADDR = 0x80181000   # the routines (a fixture write, the same bytes in every case)
CODE_END = 0x80184000
SWEEP_ADDR = 0x80184000  # sweep outputs (up to 0x4000 bytes)
SEED = 0x6E7E2003

# Commands: name -> opcode (bits 0-5). Bits 19 (sf), 10 (lm), 17-18 (mx), 15-16 (v), 13-14 (cv) select the variant.
COMMANDS = {"RTPS": 0x01, "NCLIP": 0x06, "OP": 0x0C, "DPCS": 0x10, "INTPL": 0x11, "MVMVA": 0x12, "NCDS": 0x13,
            "CDP": 0x14, "NCDT": 0x16, "NCCS": 0x1B, "CC": 0x1C, "NCS": 0x1E, "NCT": 0x20, "SQR": 0x28, "DCPL": 0x29,
            "DPCT": 0x2A, "AVSZ3": 0x2D, "AVSZ4": 0x2E, "RTPT": 0x30, "GPF": 0x3D, "GPL": 0x3E, "NCCT": 0x3F}
# The words include/psyq/gtemac.h issues (with Psy-Q's command-number bits 20-24, which the GTE ignores).
GAME_WORDS = {"gpf12": 0x4B98003D, "rtir": 0x4A49E012, "rt": 0x4A480012, "ncs": 0x4AC8041E, "nclip": 0x4B400006,
              "avsz3": 0x4B58002D, "avsz4": 0x4B68002E, "rtps": 0x4A180001, "rtv0": 0x4A486012}
# Data registers the load routine writes (lwc2): not 15 (SXYP: a FIFO push), 28 (IRGB: sets IR1-3), 29 (ORGB) and
# 31 (LZCR), both read-only.
DATA_LOADED = [r for r in range(32) if r not in (15, 28, 29, 31)]

# ---------------------------------------------------------------------------------------------------- the assembler
ZERO, V0, A0, A1, A2, A3, T0, T1, T2, T3, T7, T8, T9, SP, RA = 0, 2, 4, 5, 6, 7, 8, 9, 10, 11, 15, 24, 25, 29, 31
S0, S1, S2, S3 = 16, 17, 18, 19
NOP = 0


def i_type(op, rs, rt, imm):
    return (op << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)


def lw(rt, off, base): return i_type(0x23, base, rt, off)
def sw(rt, off, base): return i_type(0x2B, base, rt, off)
def sh(rt, off, base): return i_type(0x29, base, rt, off)
def lwc2(rt, off, base): return i_type(0x32, base, rt, off)
def swc2(rt, off, base): return i_type(0x3A, base, rt, off)
def addiu(rt, rs, imm): return i_type(0x09, rs, rt, imm)
def lui(rt, imm): return i_type(0x0F, 0, rt, imm)
def bne(rs, rt, off): return i_type(0x05, rs, rt, off)          # off: instructions from the delay slot
def addu(rd, rs, rt): return (rs << 21) | (rt << 16) | (rd << 11) | 0x21
def jr(rs): return (rs << 21) | 0x08
def jalr(rs): return (rs << 21) | (RA << 11) | 0x09
def j(addr): return (0x02 << 26) | ((addr >> 2) & 0x3FFFFFF)
def jal(addr): return (0x03 << 26) | ((addr >> 2) & 0x3FFFFFF)
def mfc2(rt, rd): return (0x12 << 26) | (0x00 << 21) | (rt << 16) | (rd << 11)
def cfc2(rt, rd): return (0x12 << 26) | (0x02 << 21) | (rt << 16) | (rd << 11)
def mtc2(rt, rd): return (0x12 << 26) | (0x04 << 21) | (rt << 16) | (rd << 11)
def ctc2(rt, rd): return (0x12 << 26) | (0x06 << 21) | (rt << 16) | (rd << 11)
def cop2(word): return 0x4A000000 | (word & 0x1FFFFFF)
def li(rt, value): return [lui(rt, ((value + 0x8000) >> 16) & 0xFFFF), addiu(rt, rt, value & 0xFFFF)]


def command_word(name, sf=1, lm=0, mx=0, v=0, cv=0):
    return COMMANDS[name] | (sf << 19) | (lm << 10) | (mx << 17) | (v << 15) | (cv << 13)


def stub_list():
    """Every command routine: (label, command word or None for "no command"), in code order."""
    stubs = [("none", None)]
    for name in COMMANDS:
        if name == "MVMVA":
            continue
        for sf in (0, 1):
            for lm in (0, 1):
                stubs.append((f"{name}_sf{sf}_lm{lm}", command_word(name, sf, lm)))
    for sf, lm in ((1, 0), (0, 1)):
        for mx in range(4):
            for v in range(4):
                for cv in range(4):
                    stubs.append((f"MVMVA_sf{sf}_lm{lm}_mx{mx}_v{v}_cv{cv}", command_word("MVMVA", sf, lm, mx, v, cv)))
    for name, word in GAME_WORDS.items():
        stubs.append((f"game_{name}", word))
    return stubs


def assemble():
    """The routines: returns (code bytes, {label: address}). The command routines and the RTPS sweep return v0 = 0
    (the golden records v0, which would otherwise hold whatever the previous job left). Labels: "load" (a0 = 64 words in), "store" (a1 = out),
    "cmd:<stub>" (a0 = in, a1 = out), "wrap" (a0..a2 = arguments, a3 = function: the registers at GTE_IN_ADDR loaded before the call,
    stored at OUT_ADDR after it, and the GTE state kept around it),
    "sweep" (a0 = function, a1 = out, a2 = first argument, a3 = count: out[i] = (s16)function(a2 + i)),
    "rtps_sweep" (a0 = in, a1 = out, a2 = count, a3 = step: TRZ = in's TRZ + i * step, RTPS sf 1, out[i] = MAC0, FLAG)."""
    code, labels = [], {}

    def here():
        return CODE_ADDR + 4 * len(code)

    labels["load"] = here()
    for i in range(32):
        code += [lw(T0, 4 * (32 + i), A0), NOP, ctc2(T0, i)]
    for i in DATA_LOADED:
        code.append(lwc2(i, 4 * i, A0))
    code += [jr(RA), NOP]

    labels["store"] = here()
    for i in range(32):
        code.append(swc2(i, 4 * i, A1))
    for i in range(32):
        code += [cfc2(T0, i), NOP, sw(T0, 4 * (32 + i), A1)]
    code += [jr(RA), NOP]

    def save_state():   # a1 is clobbered
        return li(A1, SAVE_ADDR) + [jal(labels["store"]), NOP]

    def restore_state():   # a0 is clobbered
        return li(A0, SAVE_ADDR) + [jal(labels["load"]), NOP]

    # A command routine: t9 = its return address, "pre" keeps a1 in t8, saves the state and loads a0's registers; the
    # command; "post" stores the registers at t8, reloads the saved state and returns to t9.
    labels["pre"] = here()
    code += [addu(T7, RA, ZERO), addu(T8, A1, ZERO)] + save_state() + [jal(labels["load"]), NOP, jr(T7), NOP]
    labels["post"] = here()
    code += [addu(A1, T8, ZERO), jal(labels["store"]), NOP] + restore_state() + [jr(T9), addu(V0, ZERO, ZERO)]
    for label, word in stub_list():
        labels["cmd:" + label] = here()
        code += [addu(T9, RA, ZERO), jal(labels["pre"]), NOP, cop2(word) if word is not None else NOP,
                 j(labels["post"]), NOP]

    labels["wrap"] = here()
    # Frames keep 0..15(sp) free: a callee may store its arguments there (o32).
    code += [addiu(SP, SP, -48), sw(RA, 44, SP), sw(A0, 16, SP), sw(A1, 20, SP), sw(A2, 24, SP), sw(A3, 28, SP)]
    code += save_state() + li(A0, GTE_IN_ADDR) + [jal(labels["load"]), NOP]
    code += [lw(A0, 16, SP), lw(A1, 20, SP), lw(A2, 24, SP), lw(T0, 28, SP), NOP, jalr(T0), NOP]
    code += li(A1, OUT_ADDR) + [jal(labels["store"]), NOP]
    code += restore_state()
    code += [lw(RA, 44, SP), NOP, jr(RA), addiu(SP, SP, 48)]

    labels["sweep"] = here()
    code += [addiu(SP, SP, -48), sw(RA, 44, SP), sw(S0, 16, SP), sw(S1, 20, SP), sw(S2, 24, SP), sw(S3, 28, SP),
             addu(S0, A0, ZERO), addu(S1, A1, ZERO), addu(S2, A2, ZERO), addu(S3, A3, ZERO)]
    loop = len(code)
    code += [jalr(S0), addu(A0, S2, ZERO), sh(2, 0, S1), addiu(S1, S1, 2), addiu(S3, S3, -1)]
    code += [bne(S3, ZERO, loop - (len(code) + 1)), addiu(S2, S2, 1)]
    code += [lw(RA, 44, SP), lw(S0, 16, SP), lw(S1, 20, SP), lw(S2, 24, SP), lw(S3, 28, SP), NOP, jr(RA),
             addiu(SP, SP, 48)]

    labels["rtps_sweep"] = here()
    code += [addiu(SP, SP, -48), sw(RA, 44, SP), sw(A0, 16, SP), sw(A1, 20, SP)]
    code += save_state()
    code += [lw(A0, 16, SP), lw(A1, 20, SP), jal(labels["load"]), NOP, lw(T1, 4 * (32 + 7), A0), NOP]
    loop = len(code)
    code += [ctc2(T1, 7), NOP, NOP, cop2(command_word("RTPS", 1, 0)), mfc2(T2, 24), cfc2(T3, 31), sw(T2, 0, A1),
             sw(T3, 4, A1), addiu(A2, A2, -1), addiu(A1, A1, 8)]
    code += [bne(A2, ZERO, loop - (len(code) + 1)), addu(T1, T1, A3)]
    code += restore_state()
    code += [lw(RA, 44, SP), addu(V0, ZERO, ZERO), jr(RA), addiu(SP, SP, 48)]

    data = b"".join(struct.pack("<I", w) for w in code)
    assert CODE_ADDR + len(data) <= CODE_END, hex(len(data))
    return data, labels


# ------------------------------------------------------------------------------------------------- register sets
def u32(v):
    return v & 0xFFFFFFFF


def pair(lo, hi):
    return (lo & 0xFFFF) | ((hi & 0xFFFF) << 16)


class Gen:
    EDGE16 = [0, 1, -1, 0x7FFF, -0x8000, 0x1000, -0x1000, 0x7FFE, -0x7FFF, 0x0FFF, 0x8000 - 0x1000]
    EDGE32 = [0, 1, -1, 0x7FFFFFFF, -0x80000000, 0x10000, -0x10000, 0x7FFF, -0x8000, 0x3FFFFFFF, 0x1000000]

    def __init__(self, seed):
        self.r = random.Random(seed)

    def s16(self):
        k = self.r.random()
        if k < 0.4:
            return self.r.randint(-0x8000, 0x7FFF)
        if k < 0.7:
            return self.r.randint(-0x1000, 0x1000)
        if k < 0.85:
            return self.r.randint(-0x100, 0x100)
        return self.r.choice(self.EDGE16)

    def s32(self):
        k = self.r.random()
        if k < 0.3:
            return self.r.randint(-0x80000000, 0x7FFFFFFF)
        if k < 0.6:
            return self.r.randint(-0x8000, 0x7FFF)
        if k < 0.8:
            return self.r.randint(-0x100000, 0x100000)
        return self.r.choice(self.EDGE32)

    def garbage16(self, v):
        """A 16-bit register's word with random high bits (the GTE keeps the low 16)."""
        return (v & 0xFFFF) | (self.r.getrandbits(16) << 16) if self.r.random() < 0.5 else u32(v)

    def wild(self):
        r, d, c = self.r, [0] * 32, [0] * 32
        for i in (0, 2, 4):
            d[i] = pair(self.s16(), self.s16())
            d[i + 1] = self.garbage16(self.s16())
        d[6] = r.getrandbits(32)
        d[7] = self.garbage16(r.getrandbits(16))
        for i in (8, 9, 10, 11):
            d[i] = self.garbage16(self.s16())
        for i in (12, 13, 14):
            d[i] = pair(self.s16(), self.s16())
        for i in (16, 17, 18, 19):
            d[i] = self.garbage16(r.getrandbits(16) if r.random() < 0.7 else r.choice([0, 0xFFFF, 0x8000, 1]))
        for i in (20, 21, 22, 23):
            d[i] = r.getrandbits(32)
        for i in (24, 25, 26, 27):
            d[i] = u32(self.s32())
        d[30] = u32(self.s32())
        for base in (0, 8, 16):   # RT, LLM, LCM
            for i in range(4):
                c[base + i] = pair(self.s16(), self.s16())
            c[base + 4] = self.garbage16(self.s16())
        for i in (5, 6, 7, 13, 14, 15, 21, 22, 23, 24, 25, 28):
            c[i] = u32(self.s32())
        c[26] = self.garbage16(r.getrandbits(16) if r.random() < 0.7 else r.choice([0, 1, 0xFFFF, 0x8000, 0x7FFF]))
        for i in (27, 29, 30):
            c[i] = self.garbage16(self.s16())
        c[31] = r.getrandbits(32)
        return d, c

    def typical(self):
        """Ranges the game uses: unit-scale matrices, model coordinates, a camera in front, colours, InitGeom's
        depth cue and Z scales."""
        r, d, c = self.r, [0] * 32, [0] * 32
        for i in (0, 2, 4):
            d[i] = pair(r.randint(-0x400, 0x400), r.randint(-0x400, 0x400))
            d[i + 1] = self.garbage16(r.randint(-0x400, 0x400))
        d[6] = r.getrandbits(32)
        d[7] = r.getrandbits(16)
        d[8] = r.randint(0, 0x1000)
        for i in (9, 10, 11):
            d[i] = self.garbage16(r.randint(-0x1000, 0x1000))
        for i in (12, 13, 14):
            d[i] = pair(r.randint(-0x200, 0x200), r.randint(-0x200, 0x200))
        for i in (16, 17, 18, 19):
            d[i] = r.randint(0, 0x4000)
        for i in (20, 21, 22, 23):
            d[i] = r.getrandbits(32)
        for i in (24, 25, 26, 27):
            d[i] = u32(r.randint(-0x100000, 0x100000))
        d[30] = u32(self.s32())
        for base in (0, 8):   # RT, LLM: unit-scale
            for i in range(4):
                c[base + i] = pair(r.randint(-4096, 4096), r.randint(-4096, 4096))
            c[base + 4] = u32(r.randint(-4096, 4096))
        for i in range(4):    # LCM: light colours, 4096 = 1
            c[16 + i] = pair(r.randint(0, 4096), r.randint(0, 4096))
        c[20] = r.randint(0, 4096)
        c[5], c[6], c[7] = u32(r.randint(-0x800, 0x800)), u32(r.randint(-0x800, 0x800)), r.randint(0x200, 0x2000)
        for i in (13, 14, 15):
            c[i] = r.randint(0, 0xFF0)
        for i in (21, 22, 23):
            c[i] = r.randint(0, 0xFF0)
        c[24], c[25] = u32((160 << 16) + r.randint(-0x8000, 0x8000)), u32((120 << 16) + r.randint(-0x8000, 0x8000))
        c[26] = r.randint(0x100, 0x400)
        c[27] = u32(-0x1062 + r.randint(-0x100, 0x100))
        c[28] = 0x1400000 + r.randint(-0x100000, 0x100000)
        c[29], c[30] = 0x155, 0x100
        c[31] = r.getrandbits(32)
        return d, c


def regs_bytes(d, c):
    return struct.pack("<64I", *[u32(x) for x in d + c])


# ------------------------------------------------------------------------------------------------------------ cases
def gte_case(labels, name, d, c, stub, comment=""):
    return Case(name, [Call(f"0x{labels['cmd:' + stub]:08X}", [("buf", "in"), OUT_ADDR], "void",
                            [Read(f"0x{OUT_ADDR:08X}", 0, 256, "the 64 GTE registers after the command")],
                            comment=comment or stub)],
                buffers={"in": regs_bytes(d, c)})


def edge_cases(gen, labels):
    out = []
    ident = [pair(4096, 0), pair(0, 0), pair(4096, 0), pair(0, 0), 4096]

    def base():
        d, c = gen.typical()
        c[0:5] = ident
        return d, c

    # MAC1 overflowing 44 bits: TRX * 0x1000 near 2^43 plus products; one passes 2^43 at the first product and comes
    # back under it with the next two (a check after every step flags it, a check of the sum does not).
    for k, (trx, row, vec) in enumerate([
            (0x7FFFFFFF, (0x7FFF, -0x8000, -0x8000), (0x7FFF, 0x7FFF, 0x7FFF)),
            (0x7FFFFFFF, (0x7FFF, 0x7FFF, 0x7FFF), (0x7FFF, 0x7FFF, 0x7FFF)),
            (-0x80000000, (0x7FFF, 0x7FFF, 0x7FFF), (-0x8000, -0x8000, -0x8000)),
            (-0x80000000, (-0x8000, 0x7FFF, 0x7FFF), (0x7FFF, 0x7FFF, 0x7FFF)),
            (0x7FFFF000, (0x7FFF, 0, 0), (0x7FFF, 0, 0)),
            (-0x7FFFF000, (-0x8000, 0, 0), (0x7FFF, 0, 0))]):
        for stub in ("MVMVA_sf1_lm0_mx0_v0_cv0", "MVMVA_sf0_lm1_mx0_v0_cv0", "RTPS_sf1_lm0"):
            d, c = base()
            c[0], c[1] = pair(row[0], row[1]), pair(row[2], 0)
            c[5] = u32(trx)
            d[0], d[1] = pair(vec[0], vec[1]), u32(vec[2])
            out.append(gte_case(labels, f"edge_mac44_{k}_{stub}", d, c, stub, f"MAC1 near 2^43: TRX {trx:#x}"))
    # NCLIP: MAC0 at its limits.
    for k, sxy in enumerate([((0x7FFF, -0x8000), (-0x8000, 0x7FFF), (0x7FFF, 0x7FFF)),
                             ((-0x8000, -0x8000), (0x7FFF, -0x8000), (-0x8000, 0x7FFF)),
                             ((0x7FFF, 0), (0, 0x7FFF), (-0x8000, -0x8000)),
                             ((10, 10), (20, 10), (10, 20)), ((10, 10), (10, 20), (20, 10)), ((5, 5), (5, 5), (5, 5))]):
        d, c = base()
        d[12], d[13], d[14] = (pair(*p) for p in sxy)
        out.append(gte_case(labels, f"edge_nclip_{k}", d, c, "NCLIP_sf1_lm0"))
    # AVSZ3/4: OTZ saturation both ways, MAC0 overflow.
    for k, (sz, zsf3, zsf4) in enumerate([((0xFFFF,) * 4, 0x155, 0x100), ((0xFFFF,) * 4, 0x7FFF, 0x7FFF),
                                          ((0xFFFF,) * 4, -0x8000, -0x8000), ((0x1000, 0x2000, 0x3000, 0x4000), -1, -1),
                                          ((100, 200, 300, 400), 0x155, 0x100), ((0, 0, 0, 0), 0x7FFF, 0x7FFF),
                                          ((0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF), 0x5555, 0x4000),
                                          ((0x8000, 0x8000, 0x8000, 0x8000), 0x3000, 0x2000)]):
        d, c = base()
        d[16:20] = list(sz)
        c[29], c[30] = u32(zsf3), u32(zsf4)
        out.append(gte_case(labels, f"edge_avsz3_{k}", d, c, "AVSZ3_sf1_lm0"))
        out.append(gte_case(labels, f"edge_avsz4_{k}", d, c, "AVSZ4_sf1_lm0"))
    # RTPS: SX/SY clamps, MAC0 overflow of the projection and of the depth cue, IR0 limits, H/SZ3 corners.
    for k in range(24):
        d, c = base()
        d[0], d[1] = 0, 0
        c[5] = u32(gen.r.choice([0x7FFF, -0x8000, 0x400, -0x400, 0x3FF, 0x2000]))
        c[6] = u32(gen.r.choice([0x7FFF, -0x8000, 0x200, -0x201, 0]))
        c[7] = gen.r.choice([0, 1, 2, 0x100, 0x200, 0x3E8, 0x7FFF, 0xFFFF, 0x10000])
        c[24] = u32(gen.r.choice([0, 0x7FFFFFFF, -0x80000000, 160 << 16]))
        c[25] = u32(gen.r.choice([0, 0x7FFFFFFF, -0x80000000, 120 << 16]))
        c[26] = gen.r.choice([0, 1, 0x3E8, 0x7FFF, 0x8000, 0xFFFF])
        c[27] = u32(gen.r.choice([-0x8000, 0x7FFF, -0x1062, 1]))
        c[28] = u32(gen.r.choice([0x7FFFFFFF, -0x80000000, 0x1400000, 0]))
        out.append(gte_case(labels, f"edge_rtps_{k}", d, c, gen.r.choice(["RTPS_sf1_lm0", "RTPS_sf0_lm0", "RTPS_sf1_lm1"])))
    # The registers' write/read semantics with edge values (no command).
    for k, (h, lzcs, flag) in enumerate([(0x8000, 0, 0xFFFFFFFF), (0xFFFF, -1, 0x80000000), (0x7FFF, 1, 0x00001000),
                                         (0x12345, 0x80000000, 0x00800000), (1, 0x7FFFFFFF, 0x00040000),
                                         (0, 0x00010000, 0x00002000), (0x3E8, -0x10000, 0x7FFFFFFF)]):
        d, c = gen.wild()
        c[26], d[30], c[31] = h, u32(lzcs), flag
        out.append(gte_case(labels, f"edge_regs_{k}", d, c, "none", "register semantics"))
    return out


def rtps_sweeps(gen, labels):
    """MAC0 = n (DQA 1, DQB 0) and FLAG for SZ3 = TRZ (V0 0, identity, sf 1) stepping through the division."""
    out = []
    # (H, first TRZ, step, count): every UNR table index (SZ3 0x8000..0xFFFF in steps of 0x40, H 0x7FFF and 0xFFFF);
    # small SZ3 against H (the H < 2 * SZ3 limit); SZ3 past 0xFFFF and negative.
    for k, (h, start, step, count) in enumerate([(0x7FFF, 0x8000, 0x40, 512), (0xFFFF, 0x8000, 0x40, 512),
                                                 (0x3E8, 0, 1, 1024), (0xFFFF, 0, 0x81, 1024),
                                                 (0x8000, 0x3FF0, 1, 64), (0x1234, 0xFF00, 0x10, 64),
                                                 (0x3E8, -32, 1, 64), (0, 0, 1, 16)]):
        d, c = gen.typical()
        d[0], d[1] = 0, 0
        c[0:5] = [pair(4096, 0), pair(0, 0), pair(4096, 0), pair(0, 0), 4096]
        c[5], c[6], c[7] = 0x100, u32(-0x80), u32(start)
        c[24], c[25] = 0, 0
        c[26], c[27], c[28] = h, 1, 0
        size = 8 * count
        reads = [Read(f"0x{SWEEP_ADDR:08X}", off, min(256, size - off), f"MAC0, FLAG from step {off // 8}")
                 for off in range(0, size, 256)]
        out.append(Case(f"rtps_sweep_{k}", [Call(f"0x{labels['rtps_sweep']:08X}", [("buf", "in"), SWEEP_ADDR, count, step],
                                                 "void", reads, comment=f"H {h:#x}, SZ3 from {start:#x} step {step:#x}")],
                        buffers={"in": regs_bytes(d, c)}, saves=[(f"0x{SWEEP_ADDR:08X}", size)]))
    return out


def svec(v):
    return struct.pack("<4h", *v)


def matrix(m, t):
    return struct.pack("<9h", *[x for row in m for x in row]) + struct.pack("<h", 0x5A5A) + struct.pack("<3i", *t)


def library_cases(gen, labels, sym):
    out, r = [], gen.r

    def wrap(args, fn, read):
        """A LIBGTE function on GTE registers from `typical` (it reads none of them; it leaves some changed)."""
        d, c = gen.typical()
        return [Call(f"0x{labels['wrap']:08X}", args + [sym[fn]], "void",
                     [read, Read(f"0x{OUT_ADDR:08X}", 0, 256, "the 64 GTE registers after the call")],
                     writes=[Write(f"0x{GTE_IN_ADDR:08X}", 0, regs_bytes(d, c), "the GTE registers before the call")])]

    sweep = f"0x{labels['sweep']:08X}"
    for fn in ("rsin", "rcos"):
        for k, (start, count) in enumerate([(0, 4096), (-4096 - 8, 64), (4096 - 32, 64), (0x7FFFFFF0, 32),
                                            (-0x80000000, 16), (0x12345678, 16)]):
            size = 2 * count
            reads = [Read(f"0x{SWEEP_ADDR:08X}", off, min(256, size - off), f"{fn}({start} + {off // 2} ..)")
                     for off in range(0, size, 256)]
            out.append(Case(f"{fn}_sweep_{k}", [Call(sweep, [sym[fn], SWEEP_ADDR, u32(start), count], "void", reads,
                                                     comment=f"{fn}(a) for a = {start} .. {start + count - 1}")],
                            saves=[(f"0x{SWEEP_ADDR:08X}", size)]))

    def angle():
        k = r.random()
        if k < 0.6:
            return r.randint(0, 4095)
        if k < 0.8:
            return r.randint(-0x8000, 0x7FFF)
        return r.choice([0, 1024, 2048, 3072, 4096, -1024, -1, 512, 0x7FFF, -0x8000])

    junk = matrix([[0x1111, 0x2222, 0x3333], [0x4444, 0x5555, 0x6666], [0x7777, -0x7777, -0x6666]], [0x11223344, -5, 7])
    for fn in ("RotMatrixYXZ_gte", "RotMatrixZYX_gte"):
        for k in range(48):
            rot = svec((angle(), angle(), angle(), 0x2468))
            out.append(Case(f"{fn}_{k}", wrap([("buf", "r"), ("buf", "m"), 0], fn,
                                              Read("buf:m", 0, 32, "the MATRIX after the call")),
                            buffers={"r": rot, "m": junk}))

    def mat_entry():
        k = r.random()
        if k < 0.6:
            return r.randint(-4096, 4096)
        return gen.s16()

    for k in range(40):
        m = matrix([[mat_entry() for _ in range(3)] for _ in range(3)], [gen.s32(), gen.s32(), gen.s32()])
        scale = [r.choice([r.randint(0, 0x2000), r.randint(-0x10000, 0x10000), gen.s32(), 4096]) for _ in range(3)]
        v = struct.pack("<4i", *scale, 0x13579)
        out.append(Case(f"ScaleMatrix_{k}", wrap([("buf", "m"), ("buf", "v"), 0], "ScaleMatrix",
                                                 Read("buf:m", 0, 32, "the MATRIX after the call")),
                        buffers={"m": m, "v": v}))
    for k in range(40):
        m = matrix([[mat_entry() for _ in range(3)] for _ in range(3)], [gen.s32(), gen.s32(), gen.s32()])
        v0 = svec((gen.s16(), gen.s16(), gen.s16(), 0x1357))
        out.append(Case(f"ApplyMatrixSV_{k}", wrap([("buf", "m"), ("buf", "v0"), ("buf", "v1")], "ApplyMatrixSV",
                                                   Read("buf:v1", 0, 8, "the SVECTOR after the call")),
                        buffers={"m": m, "v0": v0, "v1": svec((0x1111, 0x2222, 0x3333, 0x4444))}))
    return out


def cases(sym):
    code, labels = assemble()
    gen = Gen(SEED)
    out = []
    for label, word in stub_list():
        if label == "none":
            n = 16
        elif label.startswith("MVMVA"):
            n = 2 if "sf1_lm0" in label else 1
        elif label.startswith("game_"):
            n = 6
        else:
            n = 4
        for i in range(n):
            d, c = gen.typical() if i % 2 == 0 else gen.wild()
            out.append(gte_case(labels, f"{label}_{i}", d, c, label, f"{label}: {'typical' if i % 2 == 0 else 'wild'}"
                                + (f", word {word:#010x}" if word is not None else "")))
    out += edge_cases(gen, labels)
    out += rtps_sweeps(gen, labels)
    out += library_cases(gen, labels, sym)
    fixture = [Write(f"0x{CODE_ADDR:08X}", 0, code, "the GTE routines (tests/golden/families/gte.py assemble)")]
    saves = [(f"0x{OUT_ADDR:08X}", 0x200)]
    for case in out:
        case.fixture = fixture
        case.saves = saves + case.saves
    return out
