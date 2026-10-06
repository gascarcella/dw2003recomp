/* port/psyq/libgte.c: LIBGTE. The functions compute (the game uses their results for positions and sprites);
 * the GTE register setters record their value for M2. Fixed point is the PS1's: 4096 = 1.0, a full turn = 4096.
 *
 * Precision assumptions, to verify in a later milestone against the emulator (the layer-1 oracle can call rsin,
 * RotMatrixYXZ_gte, ... and dump the words):
 *  - rsin/rcos: Psy-Q's table is reproduced as round-to-nearest of 4096 * sin(2 pi a / 4096). Sony's table
 *    may differ by 1 in some entries (truncation, or a different generator).
 *  - the matrix products are 64-bit exact and shifted right by 12 with sign (floor), as the GTE does with its
 *    44-bit accumulators and sf = 1; the _gte variants round the same way (they use MulMatrix on the GTE).
 *  - RotMatrixYXZ composes M = Ry * Rx * Rz and RotMatrixZYX M = Rz * Ry * Rx, the order the name gives, with
 *    Psy-Q's right-handed matrices (Rz = [c -s 0; s c 0; 0 0 1], Rx = [1 0 0; 0 c -s; 0 s c],
 *    Ry = [c 0 s; 0 1 0; -s 0 c]).
 *  - ScaleMatrix scales column j by v_j; ApplyMatrixSV saturates each component to s16 (IR with lm = 0). */
#include "psyq_internal.h"
#include "psyq/libgte.h"

s32 rcos(s32 a); /* the game declares it in src/wstag/wstag460.c; not in our libgte.h */

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

static s16 psyq_fix_mul(s64 acc) {
    return (s16)(acc >> 12);
}

/* dst = a * b (3x3 only; dst may alias neither). */
static void psyq_mat_mul(const s16 a[3][3], const s16 b[3][3], s16 dst[3][3]) {
    int i;
    int j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            dst[i][j] = psyq_fix_mul((s64)a[i][0] * b[0][j] + (s64)a[i][1] * b[1][j] + (s64)a[i][2] * b[2][j]);
        }
    }
}

static void psyq_rot_x(s32 a, s16 m[3][3]) {
    s16 c = (s16)rcos(a);
    s16 s = (s16)rsin(a);

    m[0][0] = 4096; m[0][1] = 0;  m[0][2] = 0;
    m[1][0] = 0;    m[1][1] = c;  m[1][2] = (s16)-s;
    m[2][0] = 0;    m[2][1] = s;  m[2][2] = c;
}

static void psyq_rot_y(s32 a, s16 m[3][3]) {
    s16 c = (s16)rcos(a);
    s16 s = (s16)rsin(a);

    m[0][0] = c;        m[0][1] = 0;    m[0][2] = s;
    m[1][0] = 0;        m[1][1] = 4096; m[1][2] = 0;
    m[2][0] = (s16)-s;  m[2][1] = 0;    m[2][2] = c;
}

static void psyq_rot_z(s32 a, s16 m[3][3]) {
    s16 c = (s16)rcos(a);
    s16 s = (s16)rsin(a);

    m[0][0] = c;  m[0][1] = (s16)-s; m[0][2] = 0;
    m[1][0] = s;  m[1][1] = c;       m[1][2] = 0;
    m[2][0] = 0;  m[2][1] = 0;       m[2][2] = 4096;
}

/* m (rotation part only) = Ry(r.vy) * Rx(r.vx) * Rz(r.vz). */
MATRIX *RotMatrixYXZ_gte(SVECTOR *r, MATRIX *m) {
    s16 rx[3][3];
    s16 ry[3][3];
    s16 rz[3][3];
    s16 t[3][3];

    psyq_rot_x(r->vx, rx);
    psyq_rot_y(r->vy, ry);
    psyq_rot_z(r->vz, rz);
    psyq_mat_mul(ry, rx, t);
    psyq_mat_mul(t, rz, m->m);
    return m;
}

/* m (rotation part only) = Rz(r.vz) * Ry(r.vy) * Rx(r.vx). */
MATRIX *RotMatrixZYX_gte(SVECTOR *r, MATRIX *m) {
    s16 rx[3][3];
    s16 ry[3][3];
    s16 rz[3][3];
    s16 t[3][3];

    psyq_rot_x(r->vx, rx);
    psyq_rot_y(r->vy, ry);
    psyq_rot_z(r->vz, rz);
    psyq_mat_mul(rz, ry, t);
    psyq_mat_mul(t, rx, m->m);
    return m;
}

/* Column j of m scaled by v's component j (m = m * diag(v)); the translation is untouched. */
MATRIX *ScaleMatrix(MATRIX *m, VECTOR *v) {
    int i;

    for (i = 0; i < 3; i++) {
        m->m[i][0] = psyq_fix_mul((s64)m->m[i][0] * v->vx);
        m->m[i][1] = psyq_fix_mul((s64)m->m[i][1] * v->vy);
        m->m[i][2] = psyq_fix_mul((s64)m->m[i][2] * v->vz);
    }
    return m;
}

static s16 psyq_sat16(s64 x) {
    if (x > 0x7FFF) {
        return 0x7FFF;
    }
    if (x < -0x8000) {
        return -0x8000;
    }
    return (s16)x;
}

/* v1 = (m * v0) >> 12, each component saturated to s16; no translation. v1 may be v0. */
SVECTOR *ApplyMatrixSV(MATRIX *m, SVECTOR *v0, SVECTOR *v1) {
    s64 x = (s64)m->m[0][0] * v0->vx + (s64)m->m[0][1] * v0->vy + (s64)m->m[0][2] * v0->vz;
    s64 y = (s64)m->m[1][0] * v0->vx + (s64)m->m[1][1] * v0->vy + (s64)m->m[1][2] * v0->vz;
    s64 z = (s64)m->m[2][0] * v0->vx + (s64)m->m[2][1] * v0->vy + (s64)m->m[2][2] * v0->vz;

    v1->vx = psyq_sat16(x >> 12);
    v1->vy = psyq_sat16(y >> 12);
    v1->vz = psyq_sat16(z >> 12);
    return v1;
}

/* ---- GTE control registers (recorded for M2; the gte_* macros are the only readers and they are not run) ---- */

static struct {
    s32 ofx, ofy;    /* SetGeomOffset */
    s32 rbk, gbk, bbk; /* SetBackColor */
    int inited;
} psyq_gte;

void InitGeom(void) {
    PSYQ_TRACE("InitGeom");
    psyq_gte.ofx = 0;
    psyq_gte.ofy = 0;
    psyq_gte.rbk = 0;
    psyq_gte.gbk = 0;
    psyq_gte.bbk = 0;
    psyq_gte.inited = 1;
}

void SetGeomOffset(s32 ofx, s32 ofy) {
    PSYQ_TRACE("SetGeomOffset %d,%d", ofx, ofy);
    psyq_gte.ofx = ofx;
    psyq_gte.ofy = ofy;
}

void SetBackColor(s32 rbk, s32 gbk, s32 bbk) {
    PSYQ_TRACE("SetBackColor %d,%d,%d", rbk, gbk, bbk);
    psyq_gte.rbk = rbk;
    psyq_gte.gbk = gbk;
    psyq_gte.bbk = bbk;
}
