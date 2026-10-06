/* port/psyq/libgs.c: LIBGS. Owns the two LIBGS .bss matrices the game reads and writes (include/gfx.h):
 *  - D_80081358: the world-screen matrix. Evidence: GsSetRefView2 (asm/main/psyq/libgs/gs_131.s) builds it there,
 *    starting from the identity at 0x80081398, and copies the result to 0x80081338; FIGHTSTG composes its models
 *    with it and loads it as the GTE translation (fightstg_8008D3B4.c:6919). gfx.c calls it the camera.
 *  - D_800812F8: the flat-light matrix (the three light directions as rows). Evidence: GsSetFlatLight (gs_107.s)
 *    reads it, rewrites one row and stores it back; fightstg_model.c composes it with a model's matrix for
 *    gte_SetLightMatrix. gfx.c calls it the world-screen matrix (its save/restore pairs are what matters there).
 * GsGetTimInfo is real (it parses a TIM header, docs/FORMATS.md "TIM"); the rest records. */
#include "psyq_internal.h"
#include "psyq/libgs.h"
#include "psyq/libgpu.h"

MATRIX D_80081358; /* GsSetRefView2's world-screen matrix */
MATRIX D_800812F8; /* GsSetFlatLight's light matrix */

static const MATRIX psyq_gs_identity = { { { 4096, 0, 0 }, { 0, 4096, 0 }, { 0, 0, 4096 } }, { 0, 0, 0 } };

static struct {
    s32 projection;        /* GsSetProjection: the distance to the screen (h) */
    s32 light_mode;        /* GsSetLightMode */
    u8 light_rgb[3][3];    /* GsSetFlatLight's colours */
    u16 w, h, intl, dither, vram; /* GsInitGraph */
} psyq_gs;

/* Stub: records the screen size (the PS1 version also resets the GPU and sets up LIBGS's draw/display environments,
 * none of which the game reads back: it uses its own gfx module). */
void GsInitGraph(u16 w, u16 h, u16 intl, u16 dither, u16 vram) {
    PSYQ_TRACE("GsInitGraph %ux%u intl %u dither %u vram %u", w, h, intl, dither, vram);
    psyq_gs.w = w;
    psyq_gs.h = h;
    psyq_gs.intl = intl;
    psyq_gs.dither = dither;
    psyq_gs.vram = vram;
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

/* Recorded (the PS1 sets the GTE's screen distance: SetGeomScreen). */
void GsSetProjection(s32 h) {
    PSYQ_TRACE("GsSetProjection %d", h);
    psyq_gs.projection = h;
}

/* Stub: the PS1's GsInit3D resets the GTE and LIBGS's matrices; here both matrices become the identity.
 * Assumption to verify (M2): that the light matrix is reset too (LIBGS's .bss starts zeroed; what GsInit3D
 * writes beyond the world-screen matrix is unread). */
void GsInit3D(void) {
    PSYQ_TRACE("GsInit3D");
    D_80081358 = psyq_gs_identity;
    D_800812F8 = psyq_gs_identity;
    psyq_gs.projection = 0;
}

/* Recorded: row `id` of the light matrix becomes the light's direction and its colour is kept.
 * Assumption to verify (M2): the PS1 normalises the direction to 4096 before storing it (gs_107.s computes
 * with shifts and a multiply; this stub stores the raw vector, which the game already gives at 4096 scale for its
 * battle lights or does not: check FIGHTSTG's light tables). Returns 0 (ok). */
s32 GsSetFlatLight(s32 id, GsF_LIGHT *lt) {
    PSYQ_TRACE("GsSetFlatLight %d dir %d,%d,%d rgb %u,%u,%u", id, lt->vx, lt->vy, lt->vz, lt->r, lt->g, lt->b);
    if (id < 0 || id > 2) {
        return -1;
    }
    D_800812F8.m[id][0] = (s16)lt->vx;
    D_800812F8.m[id][1] = (s16)lt->vy;
    D_800812F8.m[id][2] = (s16)lt->vz;
    psyq_gs.light_rgb[id][0] = lt->r;
    psyq_gs.light_rgb[id][1] = lt->g;
    psyq_gs.light_rgb[id][2] = lt->b;
    return 0;
}

void GsSetLightMode(s32 mode) {
    PSYQ_TRACE("GsSetLightMode %d", mode);
    psyq_gs.light_mode = mode;
}

/* Stub: the world-screen matrix is left as it is (the identity after GsInit3D). The PS1 builds the view matrix
 * from the viewpoint, the reference point and the twist, in the coordinate system `pv->super`; M2 (battle
 * rendering) implements it and checks D_80081358's words against the emulator. Returns 0 (ok). */
s32 GsSetRefView2(GsRVIEW2 *pv) {
    PSYQ_TRACE("GsSetRefView2 vp %d,%d,%d vr %d,%d,%d rz %d super %u", pv->vpx, pv->vpy, pv->vpz, pv->vrx,
               pv->vry, pv->vrz, pv->rz, PSYQ_PTR(pv->super));
    return 0;
}
