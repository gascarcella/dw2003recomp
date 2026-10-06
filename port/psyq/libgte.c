/* port/psyq/libgte.c: LIBGTE. Fixed point is the PS1's: 4096 = 1.0, a full turn = 4096. The functions that use the
 * GTE on the PS1 use the software GTE here (gte.c), with LIBGTE's command sequence, so they leave the GTE in the
 * console's state; the register setters write the GTE's control registers.
 *
 * Checked against the PS1 (session 16, the layer-1 family tests/golden/families/gte.py: LIBGTE's own functions in the
 * EXE, called in PCSX-Redux, replayed here by tests/host/gte_replay.py):
 *  - rsin/rcos: round-to-nearest of 4096 * sin(2 pi a / 4096) is Psy-Q's table, for a whole turn and outside it.
 *  - RotMatrixYXZ_gte is M = Ry * Rx * Rz and RotMatrixZYX_gte M = Rz * Ry * Rx with Psy-Q's right-handed matrices,
 *    but each product is floored on its own (see RotMatrixYXZ_gte): the exact product shifted once was off by one in
 *    about 80 of 96 cases.
 *  - ScaleMatrix multiplies on the CPU (32-bit products) and writes m[2][2]'s whole word: the pad halfword after it
 *    gets the product's high half (the shim used to keep it).
 *  - ApplyMatrixSV is MVMVA (sf 1, lm 0): IR saturated to s16.
 * LIBGS's helpers (MulMatrix, MulMatrix2, ApplyMatrixLV, TransposeMatrix, SquareRoot0) are checked by the family
 * tests/golden/families/libgs_view.py (tests/host/libgs_replay.py), with the GTE state each leaves. */
#include <string.h>

#include "psyq_internal.h"
#include "psyq/libgte.h"

s32 rcos(s32 a); /* the game declares it in src/wstag/wstag460.c; not in our libgte.h */
/* Not called by the game (LIBGS calls them on the PS1); our libgte.h does not declare them. */
void SetGeomScreen(s32 h);
void SetFarColor(s32 rfc, s32 gfc, s32 bfc);
void SetColorMatrix(MATRIX *m);
MATRIX *MulMatrix(MATRIX *m0, MATRIX *m1);
MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
VECTOR *ApplyMatrixLV(MATRIX *m, VECTOR *v0, VECTOR *v1);
MATRIX *TransposeMatrix(MATRIX *m0, MATRIX *m1);
s32 SquareRoot0(s32 a);

/* ---- the sine table ---- */

static s16 psyq_sin_table[4096];
static int psyq_sin_ready;

/* sin(x) for 0 <= x <= pi/2 by its Taylor series (no libm: the shim must not add a link dependency). */
static double psyq_sin_series(double x) {
    double term = x;
    double sum = x;
    double x2 = x * x;
    int k;

    for (k = 1; k < 14; k++) {
        term *= -x2 / ((2.0 * k) * (2.0 * k + 1.0));
        sum += term;
    }
    return sum;
}

static void psyq_sin_init(void) {
    const double pi = 3.14159265358979323846;
    int i;

    for (i = 0; i <= 1024; i++) {
        double v = 4096.0 * psyq_sin_series(i * pi / 2048.0);
        s32 q = (s32)(v + 0.5); /* v >= 0 */

        if (q > 4096) {
            q = 4096;
        }
        psyq_sin_table[i] = (s16)q;                 /* 0 .. 90 degrees */
        psyq_sin_table[2048 - i] = (s16)q;          /* 180 - a */
        psyq_sin_table[(2048 + i) & 4095] = (s16)-q; /* 180 + a */
        psyq_sin_table[(4096 - i) & 4095] = (s16)-q; /* 360 - a */
    }
    psyq_sin_ready = 1;
}

/* Real: 4096 * sin(a), a in 1/4096 of a turn (any s32: it wraps). */
s32 rsin(s32 a) {
    if (!psyq_sin_ready) {
        psyq_sin_init();
    }
    return psyq_sin_table[a & 4095];
}

s32 rcos(s32 a) {
    if (!psyq_sin_ready) {
        psyq_sin_init();
    }
    return psyq_sin_table[(a + 1024) & 4095];
}

/* ---- matrices ---- */

/* Word i of a MATRIX (0..4: the rotation; word 4 is m[2][2] and the pad halfword after it), as lw reads it. */
static u32 psyq_mat_word(const MATRIX *m, int i) {
    u32 w;

    memcpy(&w, (const u8 *)m + 4 * i, 4);
    return w;
}

static void psyq_mat_set_word(MATRIX *m, int i, u32 w) {
    memcpy((u8 *)m + 4 * i, &w, 4);
}

static u32 psyq_pair(s32 lo, s32 hi) {
    return ((u32)lo & 0xFFFF) | ((u32)hi << 16);
}

/* IR0 = a, IR1-3 = b[0..2]; GPF (sf 1, lm 0); out = IR1-3 = (a * b) >> 12. Products of sines and cosines never
 * saturate. */
static void psyq_gpf3(s32 a, const s32 b[3], s32 out[3]) {
    int i;

    psyq_gte_mtc2(8, (u32)a);
    for (i = 0; i < 3; i++) {
        psyq_gte_mtc2(9 + i, (u32)b[i]);
    }
    psyq_gte_cmd(0x0198003D);
    for (i = 0; i < 3; i++) {
        out[i] = (s32)psyq_gte_mfc2(9 + i);
    }
}

/* RotMatrixYXZ_gte and RotMatrixZYX_gte: the rotation part of m from the angles, with LIBGTE's arithmetic: every
 * product of two sines/cosines is floored (>> 12: four GPF commands of three products each, as on the PS1, and two
 * CPU multiplies), a product of three is a floored product of a floored one, and the sums are taken last, in 16 bits.
 * The GPFs leave the GTE as LIBGTE leaves it. Words 0..3 of m are written whole and m[2][2] alone (the pad after it
 * and the translation are kept).
 * YXZ: m = Ry(vy) * Rx(vx) * Rz(vz), right-handed (Rz = [c -s 0; s c 0; 0 0 1], Rx = [1 0 0; 0 c -s; 0 s c],
 * Ry = [c 0 s; 0 1 0; -s 0 c]). */
MATRIX *RotMatrixYXZ_gte(SVECTOR *r, MATRIX *m) {
    s32 sx = rsin(r->vx), cx = rcos(r->vx);
    s32 sy = rsin(r->vy), cy = rcos(r->vy);
    s32 sz = rsin(r->vz), cz = rcos(r->vz);
    s32 in[3];
    s32 a[3]; /* cy * (sx, sz, cz) */
    s32 b[3]; /* sy * (sx, sz, cz) */
    s32 c[3]; /* cz * (cx, sy sx, cy sx) */
    s32 d[3]; /* sz * (cx, sy sx, cy sx) */

    in[0] = sx;
    in[1] = sz;
    in[2] = cz;
    psyq_gpf3(cy, in, a);
    psyq_gpf3(sy, in, b);
    in[0] = cx;
    in[1] = b[0];
    in[2] = a[0];
    psyq_gpf3(cz, in, c);
    psyq_gpf3(sz, in, d);
    m->m[2][2] = (s16)((cx * cy) >> 12);
    psyq_mat_set_word(m, 2, psyq_pair(c[0], -sx));                  /* m11, m12 */
    psyq_mat_set_word(m, 0, psyq_pair(a[2] + d[1], c[1] - a[1]));    /* m00, m01 */
    psyq_mat_set_word(m, 1, psyq_pair((cx * sy) >> 12, d[0]));       /* m02, m10 */
    psyq_mat_set_word(m, 3, psyq_pair(d[2] - b[2], c[2] + b[1]));    /* m20, m21 */
    return m;
}

/* ZYX: m = Rz(vz) * Ry(vy) * Rx(vx), the same arithmetic. */
MATRIX *RotMatrixZYX_gte(SVECTOR *r, MATRIX *m) {
    s32 sx = rsin(r->vx), cx = rcos(r->vx);
    s32 sy = rsin(r->vy), cy = rcos(r->vy);
    s32 sz = rsin(r->vz), cz = rcos(r->vz);
    s32 in[3];
    s32 a[3]; /* cx * (sy, sz, cz) */
    s32 b[3]; /* sx * (sy, sz, cz) */
    s32 c[3]; /* cz * (cy, sx sy, cx sy) */
    s32 d[3]; /* sz * (cy, sx sy, cx sy) */

    in[0] = sy;
    in[1] = sz;
    in[2] = cz;
    psyq_gpf3(cx, in, a);
    psyq_gpf3(sx, in, b);
    in[0] = cy;
    in[1] = b[0];
    in[2] = a[0];
    psyq_gpf3(cz, in, c);
    m->m[2][2] = (s16)((cy * cx) >> 12);
    psyq_mat_set_word(m, 3, psyq_pair(-sy, (cy * sx) >> 12));        /* m20, m21 */
    psyq_gpf3(sz, in, d);
    psyq_mat_set_word(m, 0, psyq_pair(c[0], c[1] - a[1]));           /* m00, m01 */
    psyq_mat_set_word(m, 1, psyq_pair(c[2] + b[1], d[0]));           /* m02, m10 */
    psyq_mat_set_word(m, 2, psyq_pair(d[1] + a[2], d[2] - b[2]));    /* m11, m12 */
    return m;
}

/* Column j of m scaled by v_j (m = m * diag(v)) on the CPU: each product is the low 32 bits of the 32 x 32 multiply,
 * shifted right by 12 with sign. Word 4 is written whole: the pad halfword after m[2][2] gets the shifted product's
 * high half. The translation is untouched. */
MATRIX *ScaleMatrix(MATRIX *m, VECTOR *v) {
    const s32 s[3] = { v->vx, v->vy, v->vz };
    int i;

    for (i = 0; i < 5; i++) {
        u32 w = psyq_mat_word(m, i);
        s32 lo = (s32)((u32)(s16)(w & 0xFFFF) * (u32)s[(2 * i) % 3]) >> 12;

        if (i == 4) {
            psyq_mat_set_word(m, 4, (u32)lo);
        } else {
            s32 hi = (s32)((u32)(s16)(w >> 16) * (u32)s[(2 * i + 1) % 3]) >> 12;

            psyq_mat_set_word(m, i, psyq_pair(lo, hi));
        }
    }
    return m;
}

/* v1 = (m * v0) >> 12 on the GTE, as LIBGTE does it: m's rotation becomes RT, v0 V0, MVMVA (sf 1, RT, V0, no
 * translation, lm 0), and IR1-3 (saturated to s16) go to v1's three components (its pad is kept). v1 may be v0. */
SVECTOR *ApplyMatrixSV(MATRIX *m, SVECTOR *v0, SVECTOR *v1) {
    u32 w;
    int i;

    for (i = 0; i < 5; i++) {
        psyq_gte_ctc2(i, psyq_mat_word(m, i));
    }
    memcpy(&w, v0, 4);
    psyq_gte_mtc2(0, w);
    memcpy(&w, (const u8 *)v0 + 4, 4);
    psyq_gte_mtc2(1, w);
    psyq_gte_cmd(0x00486012); /* MVMVA sf 1, mx RT, v V0, cv none, lm 0 */
    v1->vx = (s16)psyq_gte_mfc2(9);
    v1->vy = (s16)psyq_gte_mfc2(10);
    v1->vz = (s16)psyq_gte_mfc2(11);
    return v1;
}

/* ---- what LIBGS calls (GsSetRefView2, GsGetLw, GsMulCoord2/3), not the game ---- */

/* The rotation of m becomes RT (ctc2 0-4), as LIBGTE's matrix functions load it. */
static void psyq_load_rt(const MATRIX *m) {
    int i;

    for (i = 0; i < 5; i++) {
        psyq_gte_ctc2(i, psyq_mat_word(m, i));
    }
}

/* MulMatrix and MulMatrix2 (mtx_03.s, mtx_04.s): RT = m0's rotation, then each column of m1 through MVMVA (sf 1, RT,
 * V0, no translation, lm 0); IR1-3 (s16-saturated) are the product's column. Every word of m1 is read before the
 * first store, so out may be m0 or m1. The words are written as the PS1 writes them: word 4 is IR3's whole register
 * (swc2), so the pad halfword after m[2][2] gets IR3's sign. The translations are untouched. */
static void psyq_mul_matrix(const MATRIX *m0, const MATRIX *m1, MATRIX *out) {
    u32 w1 = psyq_mat_word(m1, 1), w2 = psyq_mat_word(m1, 2), w3 = psyq_mat_word(m1, 3), w4 = psyq_mat_word(m1, 4);
    u32 col[3][2];
    u32 ir[3][3];
    int c;
    int i;

    col[0][0] = (u16)m1->m[0][0] | (w1 & 0xFFFF0000u);
    col[0][1] = w3;
    col[1][0] = (u16)m1->m[0][1] | (w2 << 16);
    col[1][1] = (u32)(s32)m1->m[2][1];
    col[2][0] = (u16)m1->m[0][2] | (w2 & 0xFFFF0000u);
    col[2][1] = w4;
    psyq_load_rt(m0);
    for (c = 0; c < 3; c++) {
        psyq_gte_mtc2(0, col[c][0]);
        psyq_gte_mtc2(1, col[c][1]);
        psyq_gte_cmd(0x00486012); /* MVMVA sf 1, mx RT, v V0, cv none, lm 0 */
        for (i = 0; i < 3; i++) {
            ir[c][i] = psyq_gte_mfc2(9 + i);
        }
    }
    psyq_mat_set_word(out, 0, psyq_pair((s32)ir[0][0], (s32)ir[1][0]));
    psyq_mat_set_word(out, 3, psyq_pair((s32)ir[0][2], (s32)ir[1][2]));
    psyq_mat_set_word(out, 1, psyq_pair((s32)ir[2][0], (s32)ir[0][1]));
    psyq_mat_set_word(out, 2, psyq_pair((s32)ir[1][1], (s32)ir[2][1]));
    psyq_mat_set_word(out, 4, ir[2][2]);
}

/* m0 = m0 * m1 (rotations); returns m0. */
MATRIX *MulMatrix(MATRIX *m0, MATRIX *m1) {
    psyq_mul_matrix(m0, m1, m0);
    return m0;
}

/* m1 = m0 * m1 (rotations); returns m1. */
MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1) {
    psyq_mul_matrix(m0, m1, m1);
    return m1;
}

/* v1 = m * v0 for a 32-bit vector (mtx_004.s): each component is split by its magnitude into a high part (|v| >> 15)
 * and a low one (|v| & 0x7FFF), both with v's sign; MVMVA (sf 0) of the high parts, times 8 (<< 15 >> 12), plus MVMVA
 * (sf 1) of the low parts. MAC1-3 are read, not IR (no saturation). RT = m's rotation; v1 may be v0. */
VECTOR *ApplyMatrixLV(MATRIX *m, VECTOR *v0, VECTOR *v1) {
    const s32 in[3] = { v0->vx, v0->vy, v0->vz };
    u32 hi[3], lo[3], mac[3];
    int i;

    psyq_load_rt(m);
    for (i = 0; i < 3; i++) {
        u32 v = (u32)in[i];

        if (in[i] >= 0) {
            hi[i] = (u32)((s32)v >> 15);
            lo[i] = v & 0x7FFF;
        } else {
            v = 0u - v;
            hi[i] = 0u - (u32)((s32)v >> 15);
            lo[i] = 0u - (v & 0x7FFF);
        }
    }
    for (i = 0; i < 3; i++) {
        psyq_gte_mtc2(9 + i, hi[i]);
    }
    psyq_gte_cmd(0x0041E012); /* MVMVA sf 0, mx RT, v IR, cv none, lm 0 */
    for (i = 0; i < 3; i++) {
        mac[i] = psyq_gte_mfc2(25 + i) << 3;
    }
    for (i = 0; i < 3; i++) {
        psyq_gte_mtc2(9 + i, lo[i]);
    }
    psyq_gte_cmd(0x0049E012); /* MVMVA sf 1, mx RT, v IR, cv none, lm 0 */
    v1->vx = (s32)(psyq_gte_mfc2(25) + mac[0]);
    v1->vy = (s32)(psyq_gte_mfc2(26) + mac[1]);
    v1->vz = (s32)(psyq_gte_mfc2(27) + mac[2]);
    return v1;
}

/* m1's rotation = m0's transposed (fgo_00.s), with the PS1's word and halfword stores in their order, so m1 may be
 * m0. m1's translation is untouched. */
MATRIX *TransposeMatrix(MATRIX *m0, MATRIX *m1) {
    u8 *d = (u8 *)m1;
    const u8 *s = (const u8 *)m0;
    u32 t1, t2, t3;
    s16 h;

    memcpy(&t1, s + 0, 4);
    memcpy(&t2, s + 4, 4);
    memcpy(d + 4, &t1, 4);
    memcpy(d + 0, &t2, 4);
    memcpy(d + 0, &t1, 2); /* little-endian: the low halfword */
    memcpy(&t3, s + 8, 4);
    memcpy(&t1, s + 12, 4);
    memcpy(d + 12, &t3, 4);
    memcpy(d + 8, &t1, 4);
    memcpy(d + 12, &t2, 2);
    memcpy(d + 8, &t3, 2);
    memcpy(&h, s + 16, 2);
    memcpy(d + 4, &t1, 2);
    memcpy(d + 16, &h, 2);
    return m1;
}

/* LIBGTE's square-root table (D_800568A8): sqrt(i) * 512 rounded down, i = 64..255 (checked against the EXE's
 * table: all 192 entries). */
static u16 psyq_sqrt_table[192];

/* sqrt(a) in integers (msc01.s): the GTE's leading-zero count (LZCS/LZCR) normalises a to 8 bits by an even shift,
 * the table gives its square root and the half shift scales it back. 0 for a = 0. The table index is kept in range
 * (the PS1 reads past the table for a negative a; the callers' sums of squares are not negative). */
s32 SquareRoot0(s32 a) {
    s32 lz, e, t, n;

    if (psyq_sqrt_table[0] == 0) {
        u32 i;

        for (i = 64; i < 256; i++) {
            u32 r = 0;

            while ((r + 1) * (r + 1) <= (i << 18)) {
                r++;
            }
            psyq_sqrt_table[i - 64] = (u16)r;
        }
    }
    psyq_gte_mtc2(30, (u32)a);
    lz = (s32)psyq_gte_mfc2(31);
    if (lz == 32) {
        return 0;
    }
    e = lz & ~1;
    t = (31 - e) >> 1;
    n = e - 24;
    n = n >= 0 ? (s32)((u32)a << n) : a >> -n;
    return (s32)(((u32)psyq_sqrt_table[(u32)(n - 64) % 192] << t) >> 12);
}

/* ---- the GTE's control registers: LIBGTE's setters write them as the PS1's do ---- */

/* The console's reset (psyq.c psyq_reset): every GTE register zero. The sine table is a cache of a constant: kept. */
void psyq_gte_reset(void) {
    psyq_gte_clear();
}

/* ZSF3 = 0x155 and ZSF4 = 0x100 (1/3 and 1/4 for AVSZ3/4), H = 1000, DQA = -0x1062, DQB = 0x1400000, OFX = OFY = 0
 * (the PS1's also enables COP2: nothing to do here). */
void InitGeom(void) {
    PSYQ_TRACE("InitGeom");
    psyq_gte_ctc2(29, 0x155);
    psyq_gte_ctc2(30, 0x100);
    psyq_gte_ctc2(26, 0x3E8);
    psyq_gte_ctc2(27, (u32)-0x1062);
    psyq_gte_ctc2(28, 0x1400000);
    psyq_gte_ctc2(24, 0);
    psyq_gte_ctc2(25, 0);
}

/* OFX/OFY: the screen offset in 16.16. */
void SetGeomOffset(s32 ofx, s32 ofy) {
    PSYQ_TRACE("SetGeomOffset %d,%d", ofx, ofy);
    psyq_gte_ctc2(24, (u32)ofx << 16);
    psyq_gte_ctc2(25, (u32)ofy << 16);
}

/* H: the projection plane's distance (LIBGS's GsSetProjection calls it on the PS1). */
void SetGeomScreen(s32 h) {
    PSYQ_TRACE("SetGeomScreen %d", h);
    psyq_gte_ctc2(26, (u32)h);
}

/* BK: the back colour x 16. */
void SetBackColor(s32 rbk, s32 gbk, s32 bbk) {
    PSYQ_TRACE("SetBackColor %d,%d,%d", rbk, gbk, bbk);
    psyq_gte_ctc2(13, (u32)rbk << 4);
    psyq_gte_ctc2(14, (u32)gbk << 4);
    psyq_gte_ctc2(15, (u32)bbk << 4);
}

/* FC: the far colour x 16 (LIBGS's start-up, gte_init, calls it on the PS1). */
void SetFarColor(s32 rfc, s32 gfc, s32 bfc) {
    PSYQ_TRACE("SetFarColor %d,%d,%d", rfc, gfc, bfc);
    psyq_gte_ctc2(21, (u32)rfc << 4);
    psyq_gte_ctc2(22, (u32)gfc << 4);
    psyq_gte_ctc2(23, (u32)bfc << 4);
}

/* LCM: the light colour matrix, m's rotation words (LIBGS's GsSetFlatLight calls it on the PS1). */
void SetColorMatrix(MATRIX *m) {
    int i;

    PSYQ_TRACE("SetColorMatrix");
    for (i = 0; i < 5; i++) {
        psyq_gte_ctc2(16 + i, psyq_mat_word(m, i));
    }
}
