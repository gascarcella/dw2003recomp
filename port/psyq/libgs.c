/* port/psyq/libgs.c: LIBGS. Owns the two LIBGS .bss matrices the game reads and writes (include/gfx.h):
 *  - D_80081358: the world-screen matrix. Evidence: GsSetRefView2 (asm/main/psyq/libgs/gs_131.s) builds it there,
 *    starting from the identity at 0x80081398, and copies the result to 0x80081338; FIGHTSTG composes its models
 *    with it and loads it as the GTE translation (fightstg_8008D3B4.c:6919). gfx.c calls it the camera.
 *  - D_800812F8: the flat-light matrix (the three light directions as rows). Evidence: GsSetFlatLight (gs_107.s)
 *    reads it, rewrites one row and stores it back; fightstg_model.c composes it with a model's matrix for
 *    gte_SetLightMatrix. gfx.c calls it the world-screen matrix (its save/restore pairs are what matters there).
 * Real: GsGetTimInfo (it parses a TIM header, docs/FORMATS.md "TIM"), GsSetProjection (the GTE's H), GsSetFlatLight
 * (the light and light colour matrices, the GTE's LCM), GsSetRefView2 and GsGetLw (with the PS1's LIBGTE calls through
 * the software GTE, so the matrices are the PS1's to the bit: the layer-1 family tests/golden/families/libgs_view.py,
 * replayed by tests/host/libgs_replay.py); GsInitGraph and GsInit3D do the GTE and matrix set-up of the PS1's and
 * record the rest (the game draws with its own environments). */
#include <string.h>
#include "psyq_internal.h"
#include "psyq/libgs.h"
#include "psyq/libgpu.h"

MATRIX D_80081358; /* GsSetRefView2's world-screen matrix */
MATRIX D_800812F8; /* GsSetFlatLight's light matrix */
/* LIBGS's view and light state (the PS1's .bss; the game does not read it, tests/host/libgs_harness.c does): */
MATRIX D_80081318; /* GsSetFlatLight's light colour matrix (the GTE's LCM): light `id`'s colour is column id */
MATRIX D_80081338; /* GsSetRefView2's result again (GsGetLs and the like read it on the PS1) */
MATRIX D_80081398; /* GsSetRefView2's start: GsInitGraph's identity with m[1][1] the screen's aspect */
u32 D_800812D8;    /* PSDCNT: 1 after GsInitGraph (GsSwapDispBuff counts it; the EXE has none): GsGetLw's "current" */

/* libgte.c: the LIBGTE functions LIBGS calls (not the game: our libgte.h does not declare them). */
void InitGeom(void);
void SetGeomOffset(s32 ofx, s32 ofy);
void SetGeomScreen(s32 h);
void SetFarColor(s32 rfc, s32 gfc, s32 bfc);
void SetColorMatrix(MATRIX *m);
MATRIX *MulMatrix(MATRIX *m0, MATRIX *m1);
MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
VECTOR *ApplyMatrixLV(MATRIX *m, VECTOR *v0, VECTOR *v1);
MATRIX *TransposeMatrix(MATRIX *m0, MATRIX *m1);
s32 SquareRoot0(s32 a);
s32 rcos(s32 a);

static const MATRIX psyq_gs_identity = { { { 4096, 0, 0 }, { 0, 4096, 0 }, { 0, 0, 4096 } }, { 0, 0, 0 } };

static struct {
    s32 projection;        /* GsSetProjection: the distance to the screen (h) */
    s32 light_mode;        /* GsSetLightMode (D_800812EC on the PS1; only LIBGS's own object drawing reads it) */
    u16 w, h, intl, dither, vram; /* GsInitGraph */
    GsCOORDINATE2 *lw_stack[100]; /* D_800813B8: GsGetLw's walk up the hierarchy */
} psyq_gs;

/* The console's reset (psyq.c psyq_reset): LIBGS's .bss (the matrices) and the recorded settings zero. */
void psyq_gs_reset(void) {
    memset(&D_80081358, 0, sizeof(D_80081358));
    memset(&D_800812F8, 0, sizeof(D_800812F8));
    memset(&D_80081318, 0, sizeof(D_80081318));
    memset(&D_80081338, 0, sizeof(D_80081338));
    memset(&D_80081398, 0, sizeof(D_80081398));
    D_800812D8 = 0;
    memset(&psyq_gs, 0, sizeof(psyq_gs));
}

/* Real where the game can see it (gs_001.s): the GTE and the matrices as the PS1 leaves them. Left out: the GPU
 * reset and LIBGS's draw/display environments (PutDrawEnv/PutDispEnv), which the game never reads back: it draws
 * with its own gfx module's. */
void GsInitGraph(u16 w, u16 h, u16 intl, u16 dither, u16 vram) {
    PSYQ_TRACE("GsInitGraph %ux%u intl %u dither %u vram %u", w, h, intl, dither, vram);
    psyq_gs.w = w;
    psyq_gs.h = h;
    psyq_gs.intl = intl;
    psyq_gs.dither = dither;
    psyq_gs.vram = vram;
    /* gs_121.s gte_init: the GTE's defaults (H 1000, the depth cue, ZSF3/4), no far colour, no screen offset */
    InitGeom();
    SetFarColor(0, 0, 0);
    SetGeomOffset(0, 0);
    /* gs_001.s func_800293E4: the view's base matrix scales y by the aspect, (h << 14) / w / 3 (4096 for 320x240,
     * the game's); the light matrix and the light colour matrix are zero; PSDCNT starts at 1 */
    D_80081398 = psyq_gs_identity;
    D_80081398.m[1][1] = (s16)(((s32)h << 14) / (s32)w / 3);
    memset(&D_800812F8, 0, sizeof(D_800812F8));
    memset(&D_80081318, 0, sizeof(D_80081318));
    D_800812D8 = 1;
}

/* Real. `tim` points at the TIM's flag word (the caller skips the 0x10 magic: main.c passes main_file_base + 1).
 * Flag bits 0-2 are the pixel mode, bit 3 says a CLUT block comes first. Each block is {u32 size, u16 x, y, w, h,
 * data}; `size` counts the whole block in bytes. pmode gets the flag word, as Psy-Q's GsIMAGE does. */
void GsGetTimInfo(u32 *tim, GsIMAGE *image) {
    u32 flags = tim[0];
    u32 *block = tim + 1;

    image->pmode = flags;
    if (flags & 8) {
        const u16 *h = (const u16 *)(block + 1);

        image->cx = (s16)h[0];
        image->cy = (s16)h[1];
        image->cw = h[2];
        image->ch = h[3];
        image->clut = block + 3;
        block = (u32 *)((u8 *)block + block[0]);
    } else {
        image->cx = 0;
        image->cy = 0;
        image->cw = 0;
        image->ch = 0;
        image->clut = NULL;
    }
    {
        const u16 *h = (const u16 *)(block + 1);

        image->px = (s16)h[0];
        image->py = (s16)h[1];
        image->pw = h[2];
        image->ph = h[3];
        image->pixel = block + 3;
    }
    PSYQ_TRACE("GsGetTimInfo pmode %x clut %d,%d %ux%u pixel %d,%d %ux%u", flags, image->cx, image->cy,
               image->cw, image->ch, image->px, image->py, image->pw, image->ph);
}

/* Real (gs_106.s): the GTE's screen distance H. */
void GsSetProjection(s32 h) {
    PSYQ_TRACE("GsSetProjection %d", h);
    psyq_gs.projection = h;
    SetGeomScreen(h);
}

/* Real where the game can see it (gs_104.s): LIBGS's draw offset becomes the screen's centre and GsSetDrawBuffOffset
 * (gs_0022.s) loads it into the GTE, SetGeomOffset(w / 2 + the draw buffer's x, h / 2 + its y) with the buffer at
 * 0,0 (the game's intl has no GsOFSGPU bit, so the GTE gets the offset; the game sets it back to 0,0 right after,
 * main.c's InitGeom and gfx.c's SetGeomOffset). Then the light mode is 0 and LIBGS's Z range 10..0x3FFF (only its
 * own object sorting reads it). No matrix is touched: the light matrix stays GsInitGraph's zero and the world-screen
 * matrix zero until GsSetRefView2. Left out: the PutDrawEnv of LIBGS's draw environment (the game draws with its
 * own). */
void GsInit3D(void) {
    PSYQ_TRACE("GsInit3D");
    SetGeomOffset(psyq_gs.w / 2, psyq_gs.h / 2);
    psyq_gs.light_mode = 0;
}

/* Real (gs_107.s): light `id`'s direction, normalised to 4096 and negated (the direction light travels becomes the
 * direction to the light, which the GTE's lighting wants), is row id of the light matrix D_800812F8, and its colour,
 * (c << 12) / 255, column id of the light colour matrix D_80081318, which is then loaded as the GTE's LCM. A zero
 * direction returns -1 and changes nothing (FIGHTSTG's stage table has such lights: the row and column stay as they
 * were). An id outside 0..2 changes neither matrix but still loads LCM and returns 0, as the PS1 does. The PS1 works
 * on copies of both matrices and stores them back whole, so the translations are untouched. */
s32 GsSetFlatLight(s32 id, GsF_LIGHT *lt) {
    s32 r;

    PSYQ_TRACE("GsSetFlatLight %d dir %d,%d,%d rgb %u,%u,%u", id, lt->vx, lt->vy, lt->vz, lt->r, lt->g, lt->b);
    r = SquareRoot0((s32)((u32)lt->vx * (u32)lt->vx + (u32)lt->vy * (u32)lt->vy + (u32)lt->vz * (u32)lt->vz));
    if (r == 0) {
        return -1;
    }
    if (id >= 0 && id <= 2) {
        D_800812F8.m[id][0] = (s16)((s32)((0u - (u32)lt->vx) << 12) / r);
        D_800812F8.m[id][1] = (s16)((s32)((0u - (u32)lt->vy) << 12) / r);
        D_800812F8.m[id][2] = (s16)((s32)((0u - (u32)lt->vz) << 12) / r);
        D_80081318.m[0][id] = (s16)(((s32)lt->r << 12) / 255);
        D_80081318.m[1][id] = (s16)(((s32)lt->g << 12) / 255);
        D_80081318.m[2][id] = (s16)(((s32)lt->b << 12) / 255);
    }
    SetColorMatrix(&D_80081318);
    return 0;
}

void GsSetLightMode(s32 mode) {
    PSYQ_TRACE("GsSetLightMode %d", mode);
    psyq_gs.light_mode = mode;
}

/* gs_123.s Gssub_make_matrix: m = the identity with the rotation about `axis` ('x'/'X', 'y'/'Y', 'z'/'Z') whose
 * sine is s and cosine c; any other axis leaves the identity. */
static void psyq_gs_axis_matrix(MATRIX *m, s16 s, s16 c, char axis) {
    *m = psyq_gs_identity;
    switch (axis) {
    case 'x':
    case 'X':
        m->m[1][1] = c;
        m->m[2][2] = c;
        m->m[1][2] = (s16)-s;
        m->m[2][1] = s;
        break;
    case 'y':
    case 'Y':
        m->m[0][0] = c;
        m->m[2][2] = c;
        m->m[0][2] = s;
        m->m[2][0] = (s16)-s;
        break;
    case 'z':
    case 'Z':
        m->m[0][0] = c;
        m->m[1][1] = c;
        m->m[0][1] = (s16)-s;
        m->m[1][0] = s;
        break;
    }
}

/* gs_119.s: m = m * Rz(rz), rz in 1/360 of a degree x 4096 (a full turn = 360 << 12): the angle in 1/4096 turn is
 * rz / 360 (truncated). Nothing happens for rz = 0. */
static void psyq_gs_rotate_z(MATRIX *m, s32 rz) {
    s32 a = rz / 360;
    s32 c = rcos(a), s = rsin(a);
    MATRIX r;

    if (rz != 0) {
        r = psyq_gs_identity;
        r.m[0][0] = (s16)c;
        r.m[0][1] = (s16)-s;
        r.m[1][0] = (s16)s;
        r.m[1][1] = (s16)c;
        MulMatrix(m, &r);
    }
}

/* matrix8.s GsMulCoord2: m1 = m0 * m1 (the translation too: m1.t = m0 * m1.t + m0.t). */
static void psyq_gs_mul_coord2(MATRIX *m0, MATRIX *m1) {
    VECTOR t;

    ApplyMatrixLV(m0, (VECTOR *)m1->t, &t);
    MulMatrix2(m0, m1);
    m1->t[0] = (s32)((u32)t.vx + (u32)m0->t[0]);
    m1->t[1] = (s32)((u32)t.vy + (u32)m0->t[1]);
    m1->t[2] = (s32)((u32)t.vz + (u32)m0->t[2]);
}

/* matrix9.s GsMulCoord3: m0 = m0 * m1 (m0.t = m0 * m1.t + m0.t). */
static void psyq_gs_mul_coord3(MATRIX *m0, MATRIX *m1) {
    VECTOR t;

    ApplyMatrixLV(m0, (VECTOR *)m1->t, &t);
    MulMatrix(m0, m1);
    m0->t[0] = (s32)((u32)t.vx + (u32)m0->t[0]);
    m0->t[1] = (s32)((u32)t.vy + (u32)m0->t[1]);
    m0->t[2] = (s32)((u32)t.vz + (u32)m0->t[2]);
}

/* Real (gs_133.s): m = coord's local-world matrix, the product of the coordinate systems from the root down to coord.
 * A node whose flg is PSDCNT has its workm up to date; flg 0 marks it changed. The walk goes up until the root or an
 * up-to-date node, then multiplies down again, storing each node's workm and setting its flg to PSDCNT. */
void GsGetLw(GsCOORDINATE2 *coord, MATRIX *m) {
    GsCOORDINATE2 **stack = psyq_gs.lw_stack;
    s32 n = 0;
    s32 changed = 100;

    for (;;) {
        stack[n] = coord;
        if (coord->super == NULL) {
            if (coord->flg == D_800812D8 || coord->flg == 0) {
                coord->workm = coord->coord;
                *m = coord->workm;
                coord->flg = D_800812D8;
            } else if (changed == 100) {
                *m = stack[0]->workm;
                n = 0;
            } else {
                n = changed + 1;
                *m = stack[n]->workm;
            }
            break;
        }
        if (coord->flg == D_800812D8) {
            *m = coord->workm;
            break;
        }
        if (coord->flg == 0) {
            changed = n;
        }
        coord = coord->super;
        n++;
    }
    for (; n > 0; n--) {
        GsCOORDINATE2 *c = stack[n - 1];

        psyq_gs_mul_coord3(m, &c->coord);
        c->workm = *m;
        c->flg = D_800812D8;
    }
}

/* gs_131.s func_8002A57C: the viewpoint and the reference point (pv's first six words), shifted right together until
 * the largest magnitude fits in 15 bits. */
static void psyq_gs_view_points(const GsRVIEW2 *pv, s32 out[6]) {
    const s32 in[6] = { pv->vpx, pv->vpy, pv->vpz, pv->vrx, pv->vry, pv->vrz };
    s32 max = 0, bits = 0;
    int i;

    for (i = 0; i < 6; i++) {
        s32 a = in[i] < 0 ? (s32)(0u - (u32)in[i]) : in[i];

        if (max < a) {
            max = a;
        }
    }
    for (; max > 0; max >>= 1) {
        bits++;
    }
    for (i = 0; i < 6; i++) {
        out[i] = bits >= 16 ? in[i] >> (bits - 15) : in[i];
    }
}

/* Real (gs_131.s): the world-screen matrix D_80081358 from the viewpoint vp, the reference point vr and the twist rz
 * (1/360 degree x 4096), all in the coordinate system pv->super (NULL: the world). It starts from the base matrix
 * (GsInitGraph's aspect), turns by -rz about z, pitches so the line of sight is level (x axis) and turns it onto z
 * (y axis), translates by -vp, and, with a super, multiplies by the inverse of super's local-world matrix (whose
 * rotation is orthonormal: its transpose). The result is also copied to D_80081338. Returns 1 (and leaves the matrix
 * half built, as the PS1 does) when vp = vr, else 0. */
s32 GsSetRefView2(GsRVIEW2 *pv) {
    MATRIX *ws = &D_80081358;
    MATRIX tmp, inv;
    VECTOR v;
    s32 p[6];
    s32 dx, dy, dz, r, rxz, s;

    PSYQ_TRACE("GsSetRefView2 vp %d,%d,%d vr %d,%d,%d rz %d super %u", pv->vpx, pv->vpy, pv->vpz, pv->vrx,
               pv->vry, pv->vrz, pv->rz, PSYQ_PTR(pv->super));
    *ws = D_80081398;
    psyq_gs_rotate_z(ws, -pv->rz);
    psyq_gs_view_points(pv, p);
    dx = p[3] - p[0];
    dy = p[4] - p[1];
    dz = p[5] - p[2];
    r = SquareRoot0((s32)((u32)dx * (u32)dx + (u32)dy * (u32)dy + (u32)dz * (u32)dz));
    if (r == 0) {
        return 1;
    }
    s = -((s32)((u32)(p[1] - p[4]) << 12) / r);
    rxz = SquareRoot0((s32)((u32)dx * (u32)dx + (u32)dz * (u32)dz));
    psyq_gs_axis_matrix(&tmp, (s16)s, (s16)((s32)((u32)rxz << 12) / r), 'x');
    MulMatrix(ws, &tmp);
    if (rxz != 0) {
        psyq_gs_axis_matrix(&tmp, (s16)-((s32)((u32)dx << 12) / rxz), (s16)((s32)((u32)dz << 12) / rxz), 'y');
        MulMatrix(ws, &tmp);
    }
    v.vx = -pv->vpx;
    v.vy = -pv->vpy;
    v.vz = -pv->vpz;
    ApplyMatrixLV(ws, &v, (VECTOR *)ws->t);
    if (pv->super != NULL) {
        GsGetLw(pv->super, &tmp);
        TransposeMatrix(&tmp, &inv);
        ApplyMatrixLV(&inv, (VECTOR *)tmp.t, &v);
        inv.t[0] = -v.vx;
        inv.t[1] = -v.vy;
        inv.t[2] = -v.vz;
        psyq_gs_mul_coord2(ws, &inv);
        *ws = inv;
    }
    D_80081338 = *ws;
    return 0;
}
