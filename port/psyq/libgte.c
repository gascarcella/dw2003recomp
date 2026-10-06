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
 *  - ApplyMatrixSV is MVMVA (sf 1, lm 0): IR saturated to s16. */
#include <string.h>

#include "psyq_internal.h"
#include "psyq/libgte.h"

s32 rcos(s32 a); /* the game declares it in src/wstag/wstag460.c; not in our libgte.h */
/* Not called by the game (LIBGS calls them on the PS1); our libgte.h does not declare them. */
void SetGeomScreen(s32 h);
void SetFarColor(s32 rfc, s32 gfc, s32 bfc);
void SetColorMatrix(MATRIX *m);

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
