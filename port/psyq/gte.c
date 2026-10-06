/* port/psyq/gte.c: the GTE (the PS1's geometry coprocessor, COP2) in software. The game's gte_* macros
 * (include/psyq/gtemac.h) become calls of psyq_gte_mtc2/mfc2/ctc2/cfc2/cmd with the same registers and command words
 * (tools/port_gen.py overrides translates each macro's MIPS sequence), and LIBGTE's functions that use the GTE on
 * the PS1 (libgte.c) go through it too, so the state the game sees between calls is the console's.
 *
 * Written from the public description of the hardware (psx-spx, "Geometry Transformation Engine (GTE)"): its
 * registers, the commands and their formulas, the FLAG bits, the saturations and the RTPS/RTPT division (the
 * Newton-Raphson "UNR" reciprocal with its 257-entry table). No emulator code is used. Every command, every sf/lm and
 * MVMVA mx/v/cv combination and the registers' read/write rules are checked against the PS1 in PCSX-Redux: the
 * layer-1 family tests/golden/families/gte.py, replayed through this file by tests/host/gte_replay.py.
 *
 * Registers are kept as the 32-bit words a read returns (16-bit registers already sign- or zero-extended), so mfc2
 * and cfc2 are a load except for SXYP (15), IRGB/ORGB (28, 29) and FLAG's bit 31, which are computed.
 *
 * Arithmetic, as the PS1 does it (each line checked by the goldens):
 *  - MAC1-3 accumulate in a 44-bit register: every partial sum (the translation, then each product added) sets the
 *    FLAG overflow bits (30-25) when it leaves 44 bits signed, and wraps to 44 bits (a sum that passes 2^43 and comes
 *    back sets both bits: tests/golden/families/gte.py "edge_mac44"). The MAC register is the sum shifted right by 12
 *    when sf = 1, in 32 bits; SZ3 (RTPS/RTPT) is the 44-bit sum >> 12 whatever sf is.
 *  - IR1-3 = the 32-bit MAC saturated to -0x8000..0x7FFF (lm = 0) or 0..0x7FFF (lm = 1), FLAG bits 24-22; in the
 *    depth-cue interpolation the first IR goes through the 32-bit MAC too, saturated with lm = 0. RTPS/RTPT set IR3's
 *    FLAG bit from MAC3 >> 12 (not from the value saturated) when sf = 0.
 *  - MAC0 is tested against 32 bits (FLAG 16, 15); the colour FIFO takes MAC1-3 / 16 saturated to 0..0xFF (FLAG
 *    21-19); SZ3 and OTZ saturate to 0..0xFFFF (FLAG 18); SX2/SY2 to -0x400..0x3FF (FLAG 14, 13); IR0 to 0..0x1000
 *    (FLAG 12); H >= 2 * SZ3 is a division overflow (FLAG 17, the quotient 0x1FFFF).
 *  - MVMVA with mx = 3 multiplies by [-R * 16, R * 16, IR0; RT13 x 3; RT22 x 3] (R: RGBC's red), and with cv = 2
 *    (FC) the first column's product only sets flags, as psx-spx describes; both agree with the PS1. */
#include "psyq_internal.h"

/* The 32 data registers (cop2r0-31) and the 32 control registers (cop2r32-63), as words. */
static u32 gte_d[32];
static u32 gte_c[32];
static u32 gte_flag; /* FLAG (control 31) bits 12-30 of the command being computed */

/* Data register numbers. */
enum {
    GTE_VXY0 = 0, GTE_VZ0 = 1, GTE_RGBC = 6, GTE_OTZ = 7, GTE_IR0 = 8, GTE_IR1 = 9, GTE_SXY0 = 12, GTE_SXY2 = 14,
    GTE_SXYP = 15, GTE_SZ0 = 16, GTE_SZ3 = 19, GTE_RGB0 = 20, GTE_RGB2 = 22, GTE_MAC0 = 24, GTE_MAC1 = 25,
    GTE_IRGB = 28, GTE_ORGB = 29, GTE_LZCS = 30, GTE_LZCR = 31
};
/* Control register numbers: the three matrices start at RT, LLM and LCM; the vectors TR, BK, FC. */
enum {
    GTE_RT = 0, GTE_TR = 5, GTE_LLM = 8, GTE_BK = 13, GTE_LCM = 16, GTE_FC = 21, GTE_OFX = 24, GTE_OFY = 25,
    GTE_H = 26, GTE_DQA = 27, GTE_DQB = 28, GTE_ZSF3 = 29, GTE_ZSF4 = 30, GTE_FLAG = 31
};

/* FLAG bits. */
#define GTE_F_MAC_POS(i) (1u << (31 - (i))) /* MAC1-3 above 2^43 - 1: bits 30, 29, 28 */
#define GTE_F_MAC_NEG(i) (1u << (28 - (i))) /* below -2^43: bits 27, 26, 25 */
#define GTE_F_IR(i) (1u << (25 - (i)))      /* IR1-3 saturated: bits 24, 23, 22 */
#define GTE_F_COLOR(i) (1u << (21 - (i)))   /* colour FIFO R, G, B saturated (i = 0..2): bits 21, 20, 19 */
#define GTE_F_SZ 0x00040000u                /* SZ3 or OTZ saturated */
#define GTE_F_DIV 0x00020000u               /* the division overflowed */
#define GTE_F_MAC0_POS 0x00010000u
#define GTE_F_MAC0_NEG 0x00008000u
#define GTE_F_SX 0x00004000u
#define GTE_F_SY 0x00002000u
#define GTE_F_IR0 0x00001000u
#define GTE_F_ERROR_BITS 0x7F87E000u /* the bits whose OR is bit 31 (30-23, 18-13) */

static s32 gte_lo(u32 w) {
    return (s16)(w & 0xFFFF);
}

static s32 gte_hi(u32 w) {
    return (s16)(w >> 16);
}

/* ---- register access (mtc2/lwc2, mfc2/swc2, ctc2, cfc2) ---- */

/* Leading zeros of a non-negative value, leading ones of a negative one (1..32). */
static u32 gte_lzc(u32 v) {
    u32 n = 0;

    if (v & 0x80000000u) {
        v = ~v;
    }
    while (n < 32 && !(v & 0x80000000u)) {
        v <<= 1;
        n++;
    }
    return n;
}

void psyq_gte_mtc2(int reg, u32 v) {
    switch (reg & 31) {
    case 1: case 3: case 5: case 8: case 9: case 10: case 11: /* VZ0-2, IR0-3: 16 bits signed */
        gte_d[reg] = (u32)gte_lo(v);
        break;
    case GTE_OTZ: case 16: case 17: case 18: case 19: /* OTZ, SZ0-3: 16 bits unsigned */
        gte_d[reg] = v & 0xFFFF;
        break;
    case GTE_SXYP: /* pushes the screen XY FIFO */
        gte_d[GTE_SXY0] = gte_d[GTE_SXY0 + 1];
        gte_d[GTE_SXY0 + 1] = gte_d[GTE_SXY2];
        gte_d[GTE_SXY2] = v;
        break;
    case GTE_IRGB: /* 5:5:5 colour into IR1-3 (x 0x80) */
        gte_d[GTE_IR1] = (v & 0x1F) << 7;
        gte_d[GTE_IR1 + 1] = ((v >> 5) & 0x1F) << 7;
        gte_d[GTE_IR1 + 2] = ((v >> 10) & 0x1F) << 7;
        break;
    case GTE_ORGB: case GTE_LZCR: /* read-only */
        break;
    case GTE_LZCS:
        gte_d[GTE_LZCS] = v;
        gte_d[GTE_LZCR] = gte_lzc(v);
        break;
    default:
        gte_d[reg] = v;
        break;
    }
}

/* IR1-3 / 0x80 saturated to 0..0x1F, as a 5:5:5 colour (what IRGB and ORGB read). */
static u32 gte_orgb(void) {
    u32 out = 0;
    int i;

    for (i = 0; i < 3; i++) {
        s32 c = (s32)gte_d[GTE_IR1 + i] >> 7;

        out |= (u32)(c < 0 ? 0 : c > 0x1F ? 0x1F : c) << (5 * i);
    }
    return out;
}

u32 psyq_gte_mfc2(int reg) {
    switch (reg & 31) {
    case GTE_SXYP:
        return gte_d[GTE_SXY2];
    case GTE_IRGB: case GTE_ORGB:
        return gte_orgb();
    default:
        return gte_d[reg & 31];
    }
}

void psyq_gte_ctc2(int reg, u32 v) {
    switch (reg & 31) {
    case 4: case 12: case 20: case GTE_DQA: case GTE_ZSF3: case GTE_ZSF4: /* RT33, L33, LB3, DQA, ZSF3/4: s16 */
        gte_c[reg] = (u32)gte_lo(v);
        break;
    case GTE_H: /* 16 bits unsigned, but reads sign-extended (a hardware quirk): kept as written, see cfc2 */
        gte_c[GTE_H] = v & 0xFFFF;
        break;
    case GTE_FLAG: /* only bits 12-30 are writable; 31 is computed */
        gte_c[GTE_FLAG] = v & 0x7FFFF000u;
        break;
    default:
        gte_c[reg & 31] = v;
        break;
    }
}

u32 psyq_gte_cfc2(int reg) {
    switch (reg & 31) {
    case GTE_H:
        return (u32)gte_lo(gte_c[GTE_H]);
    case GTE_FLAG:
        return gte_c[GTE_FLAG] | ((gte_c[GTE_FLAG] & GTE_F_ERROR_BITS) ? 0x80000000u : 0);
    default:
        return gte_c[reg & 31];
    }
}

void psyq_gte_clear(void) {
    int i;

    for (i = 0; i < 32; i++) {
        gte_d[i] = 0;
        gte_c[i] = 0;
    }
    gte_d[GTE_LZCR] = 32; /* LZCS 0 */
}

/* ---- the arithmetic ---- */

/* Element (row, col) of the matrix whose first word is control register `base` (RT, LLM or LCM). */
static s32 gte_mat(int base, int row, int col) {
    int k = row * 3 + col;
    u32 w = gte_c[base + (k >> 1)];

    return (k & 1) ? gte_hi(w) : gte_lo(w);
}

static s32 gte_ir(int i) {
    return (s32)gte_d[GTE_IR0 + i];
}

/* A partial sum of MAC1-3 (i = 1..3): FLAG's overflow bits when it leaves 44 bits signed, and the value wrapped to
 * 44 bits (the accumulator's width). */
static s64 gte_acc(int i, s64 v) {
    if (v > (((s64)1 << 43) - 1)) {
        gte_flag |= GTE_F_MAC_POS(i);
    } else if (v < -((s64)1 << 43)) {
        gte_flag |= GTE_F_MAC_NEG(i);
    }
    return (s64)((u64)v << 20) >> 20;
}

/* IR1-3 (i = 1..3) from a value: saturated by lm, FLAG's bit when it was. */
static s32 gte_sat_ir(int i, s64 v, int lm) {
    s32 lo = lm ? 0 : -0x8000;

    if (v < lo) {
        gte_flag |= GTE_F_IR(i);
        return lo;
    }
    if (v > 0x7FFF) {
        gte_flag |= GTE_F_IR(i);
        return 0x7FFF;
    }
    return (s32)v;
}

/* MAC_i = v >> shift; IR_i = MAC_i saturated by lm. */
static void gte_set_mac_ir(int i, s64 v, int shift, int lm) {
    s32 mac = (s32)(v >> shift);

    gte_d[GTE_MAC0 + i] = (u32)mac;
    gte_d[GTE_IR0 + i] = (u32)gte_sat_ir(i, mac, lm);
}

/* MAC0 from a value: FLAG's bits when it leaves 32 bits signed; returns the value (not truncated). */
static s64 gte_mac0(s64 v) {
    if (v > 0x7FFFFFFFLL) {
        gte_flag |= GTE_F_MAC0_POS;
    } else if (v < -0x80000000LL) {
        gte_flag |= GTE_F_MAC0_NEG;
    }
    gte_d[GTE_MAC0] = (u32)(s32)v;
    return v;
}

static u32 gte_sat_sz(s64 v) {
    if (v < 0) {
        gte_flag |= GTE_F_SZ;
        return 0;
    }
    if (v > 0xFFFF) {
        gte_flag |= GTE_F_SZ;
        return 0xFFFF;
    }
    return (u32)v;
}

/* Pushes MAC1-3 / 16 (saturated to a byte) and RGBC's code byte onto the colour FIFO. */
static void gte_push_color(void) {
    u32 rgb = gte_d[GTE_RGBC] & 0xFF000000u;
    int i;

    for (i = 0; i < 3; i++) {
        s32 c = (s32)gte_d[GTE_MAC1 + i] >> 4;

        if (c < 0) {
            gte_flag |= GTE_F_COLOR(i);
            c = 0;
        } else if (c > 0xFF) {
            gte_flag |= GTE_F_COLOR(i);
            c = 0xFF;
        }
        rgb |= (u32)c << (8 * i);
    }
    gte_d[GTE_RGB0] = gte_d[GTE_RGB0 + 1];
    gte_d[GTE_RGB0 + 1] = gte_d[GTE_RGB2];
    gte_d[GTE_RGB2] = rgb;
}

/* MAC_i = (t_i * 0x1000 + M_i * v) >> shift for i = 1..3, IR = MAC saturated by lm. `base` selects the matrix
 * (RT, LLM, LCM, or -1: MVMVA's mx = 3 matrix), `t` the translation (NULL: none). */
static void gte_mul_matrix(int base, const s32 v[3], const s64 t[3], int shift, int lm) {
    int i;

    for (i = 0; i < 3; i++) {
        s32 m[3];
        s64 acc = t != NULL ? t[i] * 0x1000 : 0;
        int j;

        for (j = 0; j < 3; j++) {
            if (base >= 0) {
                m[j] = gte_mat(base, i, j);
            } else if (i == 0) { /* mx = 3: -R * 16, R * 16, IR0 */
                m[j] = j == 0 ? -(s32)((gte_d[GTE_RGBC] & 0xFF) << 4) : j == 1 ? (s32)((gte_d[GTE_RGBC] & 0xFF) << 4)
                                                                             : gte_ir(0);
            } else { /* then RT13 x 3, RT22 x 3 */
                m[j] = i == 1 ? gte_mat(GTE_RT, 0, 2) : gte_mat(GTE_RT, 1, 1);
            }
        }
        for (j = 0; j < 3; j++) {
            acc = gte_acc(i + 1, acc + (s64)m[j] * v[j]);
        }
        gte_set_mac_ir(i + 1, acc, shift, lm);
    }
}

/* MVMVA with cv = 2 (FC), the hardware's bug: the first column's product goes to the flags only (with the
 * translation), and MAC = the other two columns' products. */
static void gte_mul_matrix_fc_bug(int base, const s32 v[3], int shift, int lm) {
    int i;

    for (i = 0; i < 3; i++) {
        s32 m[3];
        s64 first;
        s64 acc;
        int j;

        for (j = 0; j < 3; j++) {
            if (base >= 0) {
                m[j] = gte_mat(base, i, j);
            } else if (i == 0) {
                m[j] = j == 0 ? -(s32)((gte_d[GTE_RGBC] & 0xFF) << 4) : j == 1 ? (s32)((gte_d[GTE_RGBC] & 0xFF) << 4)
                                                                             : gte_ir(0);
            } else {
                m[j] = i == 1 ? gte_mat(GTE_RT, 0, 2) : gte_mat(GTE_RT, 1, 1);
            }
        }
        first = gte_acc(i + 1, (s64)(s32)gte_c[GTE_FC + i] * 0x1000 + (s64)m[0] * v[0]);
        (void)gte_sat_ir(i + 1, first >> shift, 0);
        acc = gte_acc(i + 1, (s64)m[1] * v[1]);
        acc = gte_acc(i + 1, acc + (s64)m[2] * v[2]);
        gte_set_mac_ir(i + 1, acc, shift, lm);
    }
}

/* The perspective division: H / SZ3 as a 1.16 fixed-point value (0..0x1FFFF), by the GTE's reciprocal: a table
 * lookup and one Newton-Raphson step (psx-spx "GTE Division Inaccuracy"). */
static u32 gte_divide(u32 h, u32 sz3) {
    static u8 unr_table[0x101];
    static int unr_ready;
    u32 z;
    u64 n;
    u64 d;
    u64 u;

    if (!unr_ready) {
        int i;

        for (i = 0; i <= 0x100; i++) {
            s32 v = (0x40000 / (i + 0x100) + 1) / 2 - 0x101;

            unr_table[i] = (u8)(v < 0 ? 0 : v);
        }
        unr_ready = 1;
    }
    if (h >= sz3 * 2) {
        gte_flag |= GTE_F_DIV;
        return 0x1FFFF;
    }
    z = 0; /* leading zeros of sz3 as a 16-bit value (sz3 > h / 2 >= 0, so sz3 != 0) */
    while (!(sz3 & (0x8000u >> z))) {
        z++;
    }
    n = (u64)h << z;
    d = (u64)sz3 << z;
    u = (u64)unr_table[(d - 0x7FC0) >> 7] + 0x101;
    d = (0x2000080 - d * u) >> 8;
    d = (0x0000080 + d * u) >> 8;
    n = (n * d + 0x8000) >> 16;
    if (n > 0x1FFFF) {
        gte_flag |= GTE_F_DIV;
        n = 0x1FFFF;
    }
    return (u32)n;
}

/* RTPS for vector `k` (the second half: SZ, SXY, and with `last` the depth cue). */
static void gte_rtp(int k, int shift, int lm, int last) {
    s32 v[3];
    s64 mac3;
    s32 x;
    s32 y;
    s64 sx;
    s64 sy;
    u32 n;
    int i;

    v[0] = gte_lo(gte_d[2 * k]);
    v[1] = gte_hi(gte_d[2 * k]);
    v[2] = (s32)gte_d[2 * k + 1];
    mac3 = 0;
    for (i = 0; i < 3; i++) {
        s64 acc = (s64)(s32)gte_c[GTE_TR + i] * 0x1000;
        int j;

        for (j = 0; j < 3; j++) {
            acc = gte_acc(i + 1, acc + (s64)gte_mat(GTE_RT, i, j) * v[j]);
        }
        if (i < 2) {
            gte_set_mac_ir(i + 1, acc, shift, lm);
        } else {
            /* IR3 is saturated from MAC3, but its FLAG bit tests MAC3 >> 12 whatever sf is. */
            s32 m3 = (s32)(acc >> shift);
            s64 f = acc >> 12;
            s32 lo = lm ? 0 : -0x8000;

            gte_d[GTE_MAC1 + 2] = (u32)m3;
            gte_d[GTE_IR0 + 3] = (u32)(m3 < lo ? lo : m3 > 0x7FFF ? 0x7FFF : m3);
            if (f < -0x8000 || f > 0x7FFF) {
                gte_flag |= GTE_F_IR(3);
            }
            mac3 = acc;
        }
    }
    /* SZ3 = MAC3 >> ((1 - sf) * 12), i.e. the sum >> 12 */
    for (i = 0; i < 3; i++) {
        gte_d[GTE_SZ0 + i] = gte_d[GTE_SZ0 + i + 1];
    }
    gte_d[GTE_SZ3] = gte_sat_sz(mac3 >> 12);
    n = gte_divide(gte_c[GTE_H] & 0xFFFF, gte_d[GTE_SZ3]);
    sx = gte_mac0((s64)(s32)gte_c[GTE_OFX] + (s64)gte_ir(1) * n) >> 16;
    sy = gte_mac0((s64)(s32)gte_c[GTE_OFY] + (s64)gte_ir(2) * n) >> 16;
    x = sx < -0x400 ? -0x400 : sx > 0x3FF ? 0x3FF : (s32)sx;
    y = sy < -0x400 ? -0x400 : sy > 0x3FF ? 0x3FF : (s32)sy;
    if (x != sx) {
        gte_flag |= GTE_F_SX;
    }
    if (y != sy) {
        gte_flag |= GTE_F_SY;
    }
    gte_d[GTE_SXY0] = gte_d[GTE_SXY0 + 1];
    gte_d[GTE_SXY0 + 1] = gte_d[GTE_SXY2];
    gte_d[GTE_SXY2] = ((u32)x & 0xFFFF) | ((u32)y << 16);
    if (last) {
        s64 dq = gte_mac0((s64)gte_lo(gte_c[GTE_DQA]) * n + (s32)gte_c[GTE_DQB]) >> 12;

        if (dq < 0 || dq > 0x1000) {
            gte_flag |= GTE_F_IR0;
            dq = dq < 0 ? 0 : 0x1000;
        }
        gte_d[GTE_IR0] = (u32)dq;
    }
}

/* The depth-cue interpolation towards the far colour: IR = (FC * 0x1000 - MAC) >> shift (lm = 0), then
 * MAC = (IR * IR0 + MAC) >> shift. `mac` holds the three MACs before it (unshifted). */
static void gte_interpolate(const s64 mac[3], int shift, int lm) {
    int i;

    for (i = 0; i < 3; i++) {
        s64 v = gte_acc(i + 1, (s64)(s32)gte_c[GTE_FC + i] * 0x1000 - mac[i]);

        gte_d[GTE_IR0 + 1 + i] = (u32)gte_sat_ir(i + 1, (s32)(v >> shift), 0); /* through the 32-bit MAC */
    }
    for (i = 0; i < 3; i++) {
        s64 v = gte_acc(i + 1, (s64)gte_ir(i + 1) * gte_ir(0) + mac[i]);

        gte_set_mac_ir(i + 1, v, shift, lm);
    }
}

/* MAC = [R * IR1, G * IR2, B * IR3] << 4 of colour word `rgb` (unshifted, for the next step). */
static void gte_color_times_ir(u32 rgb, s64 mac[3]) {
    int i;

    for (i = 0; i < 3; i++) {
        mac[i] = gte_acc(i + 1, (s64)((rgb >> (8 * i)) & 0xFF) * gte_ir(i + 1) * 16);
    }
}

/* The light steps of NCS/NCC/NCD: IR = LLM * V_k, then IR = BK + LCM * IR. */
static void gte_light(int k, int shift, int lm) {
    s32 v[3];
    s64 bk[3];
    int i;

    v[0] = gte_lo(gte_d[2 * k]);
    v[1] = gte_hi(gte_d[2 * k]);
    v[2] = (s32)gte_d[2 * k + 1];
    gte_mul_matrix(GTE_LLM, v, NULL, shift, lm);
    for (i = 0; i < 3; i++) {
        v[i] = gte_ir(i + 1);
        bk[i] = (s32)gte_c[GTE_BK + i];
    }
    gte_mul_matrix(GTE_LCM, v, bk, shift, lm);
}

static void gte_set_mac3(const s64 mac[3], int shift, int lm) {
    int i;

    for (i = 0; i < 3; i++) {
        gte_set_mac_ir(i + 1, mac[i], shift, lm);
    }
}

/* Runs one GTE command (the cop2 instruction's low 25 bits). */
void psyq_gte_cmd(u32 op) {
    int shift = (op & (1u << 19)) ? 12 : 0;
    int lm = (op >> 10) & 1;
    s64 mac[3];
    s32 v[3];
    int i;
    int k;

    gte_flag = 0;
    switch (op & 0x3F) {
    case 0x01: /* RTPS */
        gte_rtp(0, shift, lm, 1);
        break;
    case 0x30: /* RTPT */
        gte_rtp(0, shift, lm, 0);
        gte_rtp(1, shift, lm, 0);
        gte_rtp(2, shift, lm, 1);
        break;
    case 0x06: { /* NCLIP */
        s64 x0 = gte_lo(gte_d[GTE_SXY0]), y0 = gte_hi(gte_d[GTE_SXY0]);
        s64 x1 = gte_lo(gte_d[GTE_SXY0 + 1]), y1 = gte_hi(gte_d[GTE_SXY0 + 1]);
        s64 x2 = gte_lo(gte_d[GTE_SXY2]), y2 = gte_hi(gte_d[GTE_SXY2]);

        gte_mac0(x0 * y1 + x1 * y2 + x2 * y0 - x0 * y2 - x1 * y0 - x2 * y1);
        break;
    }
    case 0x0C: { /* OP: the cross product of RT's diagonal and IR */
        s64 d1 = gte_mat(GTE_RT, 0, 0), d2 = gte_mat(GTE_RT, 1, 1), d3 = gte_mat(GTE_RT, 2, 2);
        s64 ir1 = gte_ir(1), ir2 = gte_ir(2), ir3 = gte_ir(3);

        mac[0] = gte_acc(1, ir3 * d2 - ir2 * d3);
        mac[1] = gte_acc(2, ir1 * d3 - ir3 * d1);
        mac[2] = gte_acc(3, ir2 * d1 - ir1 * d2);
        gte_set_mac3(mac, shift, lm);
        break;
    }
    case 0x10: /* DPCS */
    case 0x2A: /* DPCT */
        for (k = 0; k < ((op & 0x3F) == 0x2A ? 3 : 1); k++) {
            u32 rgb = (op & 0x3F) == 0x2A ? gte_d[GTE_RGB0] : gte_d[GTE_RGBC];

            for (i = 0; i < 3; i++) {
                mac[i] = (s64)((rgb >> (8 * i)) & 0xFF) << 16;
            }
            gte_interpolate(mac, shift, lm);
            gte_push_color();
        }
        break;
    case 0x11: /* INTPL */
        for (i = 0; i < 3; i++) {
            mac[i] = (s64)gte_ir(i + 1) * 0x1000;
        }
        gte_interpolate(mac, shift, lm);
        gte_push_color();
        break;
    case 0x29: /* DCPL */
        gte_color_times_ir(gte_d[GTE_RGBC], mac);
        gte_interpolate(mac, shift, lm);
        gte_push_color();
        break;
    case 0x12: { /* MVMVA */
        int mx = (op >> 17) & 3;
        int vs = (op >> 15) & 3;
        int cv = (op >> 13) & 3;
        int base = mx == 0 ? GTE_RT : mx == 1 ? GTE_LLM : mx == 2 ? GTE_LCM : -1;
        s64 t[3];

        if (vs == 3) {
            for (i = 0; i < 3; i++) {
                v[i] = gte_ir(i + 1);
            }
        } else {
            v[0] = gte_lo(gte_d[2 * vs]);
            v[1] = gte_hi(gte_d[2 * vs]);
            v[2] = (s32)gte_d[2 * vs + 1];
        }
        if (cv == 2) {
            gte_mul_matrix_fc_bug(base, v, shift, lm);
            break;
        }
        for (i = 0; i < 3; i++) {
            t[i] = cv == 0 ? (s32)gte_c[GTE_TR + i] : cv == 1 ? (s32)gte_c[GTE_BK + i] : 0;
        }
        gte_mul_matrix(base, v, t, shift, lm);
        break;
    }
    case 0x13: /* NCDS */
    case 0x16: /* NCDT */
        for (k = 0; k < ((op & 0x3F) == 0x16 ? 3 : 1); k++) {
            gte_light(k, shift, lm);
            gte_color_times_ir(gte_d[GTE_RGBC], mac);
            gte_interpolate(mac, shift, lm);
            gte_push_color();
        }
        break;
    case 0x14: /* CDP */
    case 0x1C: /* CC */
        for (i = 0; i < 3; i++) {
            v[i] = gte_ir(i + 1);
            mac[i] = (s32)gte_c[GTE_BK + i];
        }
        gte_mul_matrix(GTE_LCM, v, mac, shift, lm);
        gte_color_times_ir(gte_d[GTE_RGBC], mac);
        if ((op & 0x3F) == 0x14) {
            gte_interpolate(mac, shift, lm);
        } else {
            gte_set_mac3(mac, shift, lm);
        }
        gte_push_color();
        break;
    case 0x1B: /* NCCS */
    case 0x3F: /* NCCT */
        for (k = 0; k < ((op & 0x3F) == 0x3F ? 3 : 1); k++) {
            gte_light(k, shift, lm);
            gte_color_times_ir(gte_d[GTE_RGBC], mac);
            gte_set_mac3(mac, shift, lm);
            gte_push_color();
        }
        break;
    case 0x1E: /* NCS */
    case 0x20: /* NCT */
        for (k = 0; k < ((op & 0x3F) == 0x20 ? 3 : 1); k++) {
            gte_light(k, shift, lm);
            gte_push_color();
        }
        break;
    case 0x28: /* SQR */
        for (i = 0; i < 3; i++) {
            mac[i] = gte_acc(i + 1, (s64)gte_ir(i + 1) * gte_ir(i + 1));
        }
        gte_set_mac3(mac, shift, lm);
        break;
    case 0x2D: /* AVSZ3 */
        gte_d[GTE_OTZ] = gte_sat_sz(gte_mac0((s64)gte_lo(gte_c[GTE_ZSF3]) *
                                             (gte_d[GTE_SZ0 + 1] + gte_d[GTE_SZ0 + 2] + gte_d[GTE_SZ3])) >> 12);
        break;
    case 0x2E: /* AVSZ4 */
        gte_d[GTE_OTZ] = gte_sat_sz(gte_mac0((s64)gte_lo(gte_c[GTE_ZSF4]) *
                                             (gte_d[GTE_SZ0] + gte_d[GTE_SZ0 + 1] + gte_d[GTE_SZ0 + 2] +
                                              gte_d[GTE_SZ3])) >> 12);
        break;
    case 0x3D: /* GPF */
        for (i = 0; i < 3; i++) {
            mac[i] = gte_acc(i + 1, (s64)gte_ir(0) * gte_ir(i + 1));
        }
        gte_set_mac3(mac, shift, lm);
        gte_push_color();
        break;
    case 0x3E: /* GPL */
        for (i = 0; i < 3; i++) {
            mac[i] = gte_acc(i + 1, (s64)(s32)gte_d[GTE_MAC1 + i] * ((s64)1 << shift) + (s64)gte_ir(0) * gte_ir(i + 1));
        }
        gte_set_mac3(mac, shift, lm);
        gte_push_color();
        break;
    default:
        /* Not a command (the game issues none of the others). */
        break;
    }
    gte_c[GTE_FLAG] = gte_flag;
}
