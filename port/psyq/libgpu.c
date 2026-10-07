/* port/psyq/libgpu.c: LIBGPU over the software GPU (gpu.c). The environment setters and the ordering-table builders
 * compute what the PS1's do (word for word: golden family gpu); DrawOTag/ContinueDraw walk the list they are given,
 * hash the primitives (psyq_gpu_take_hash: the "primitive stream per frame" of the M1 test) and feed their words to
 * the GPU; LoadImage, MoveImage, ClearImage(2) draw into its VRAM; psyq_gpu_vram/psyq_gpu_display are the video
 * output (psyq.h).
 *
 * Tags (docs/PORT.md "Ordering tables on 64-bit"): a tag's low 24 bits are a pointer's offset in the arena (PTR_TO_U32),
 * so the walk resolves `tag & 0xFFFFFF` against the arena's base (port_arena) and follows it only inside the window
 * psyq_set_arena gave (by default the heap, port_heap_start..port_heap_end). */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "psyq_internal.h"
#include "psyq/libgpu.h"

#ifndef PC_PORT
#error "port/psyq is the host shim: compile it with -DPC_PORT"
#endif

/* ---- the walkable window and the stream recorder ---- */

static const u8 *psyq_gpu_lo;
static const u8 *psyq_gpu_hi;
static u32 psyq_gpu_hash = 0x811C9DC5u; /* FNV-1a offset basis */
static u32 psyq_gpu_count;
/* DW3_PORT_PRIM_DUMP=N: the words of every primitive hashed in frame N (the N-th psyq_gpu_take_hash period, counted
 * from 1 like the frame log), to stderr: to find what makes two builds' primitive hashes differ. */
static long psyq_gpu_dump_frame = -1, psyq_gpu_frame = 1;
static u32 psyq_gpu_terminator = 0xFFFFFF; /* what BreakDraw hands out: an empty list */
static PsyqDisplay psyq_gpu_disp;          /* the video output (psyq.h): PutDispEnv's area, SetDispMask */

void psyq_set_arena(const void *base, unsigned long size) {
    psyq_gpu_lo = (const u8 *)base;
    psyq_gpu_hi = (const u8 *)base + size;
}

u32 psyq_gpu_take_hash(u32 *count) {
    u32 h = psyq_gpu_hash;

    psyq_gpu_frame++;
    if (count != NULL) {
        *count = psyq_gpu_count;
    }
    psyq_gpu_hash = 0x811C9DC5u;
    psyq_gpu_count = 0;
    return h;
}

/* The console's reset (psyq.c psyq_reset): the recorder empty (what the interrupted frame had walked is dropped, as the
 * emulator's GPU drops it), BreakDraw's empty list intact. The walk window (psyq_set_arena) is the runtime's: kept. */
void psyq_gpu_reset(void) {
    psyq_gpu_hash = 0x811C9DC5u;
    psyq_gpu_count = 0;
    psyq_gpu_terminator = 0xFFFFFF;
    gpu_power_on();
    memset(&psyq_gpu_disp, 0, sizeof(psyq_gpu_disp));
}

static void psyq_gpu_window(const u8 **lo, const u8 **hi) {
    if (psyq_gpu_lo == NULL) {
        *lo = port_heap_start;
        *hi = port_heap_end;
    } else {
        *lo = psyq_gpu_lo;
        *hi = psyq_gpu_hi;
    }
}

static void psyq_gpu_hash_words(const u32 *w, u32 n) {
    u32 i;
    u32 h = psyq_gpu_hash;

    if (psyq_gpu_dump_frame < 0) {
        const char *env = getenv("DW3_PORT_PRIM_DUMP");
        psyq_gpu_dump_frame = env != NULL ? strtol(env, NULL, 0) : 0;
    }
    if (psyq_gpu_dump_frame == psyq_gpu_frame) {
        fprintf(stderr, "prim frame %ld:", psyq_gpu_frame);
        for (i = 0; i < n; i++) {
            fprintf(stderr, " %08x", w[i]);
        }
        fputc('\n', stderr);
    }
    for (i = 0; i < n; i++) {
        u32 x = w[i];
        int b;

        for (b = 0; b < 4; b++) {
            h ^= (x >> (8 * b)) & 0xFF;
            h *= 0x01000193u;
        }
    }
    psyq_gpu_hash = h;
}

/* One primitive's words as the GPU reads them. A textured polygon's (POLY_FT3/FT4/GT3/GT4) third and fourth texture
 * coordinate words carry padding in their high half (pad1/pad2), which the GPU ignores and the game never writes: it
 * holds whatever the packet buffer held before, which differs between the -m32 and -m64 builds (their heaps differ;
 * session 16, FIELDSTG's actor sprites in new_game), so it is hashed as 0. Words: the command and color, then per
 * vertex [its color, gouraud only, not the first] its position and its texture coordinates. */
static void psyq_gpu_hash_prim(const u32 *w, u32 len) {
    u32 code = w[0] >> 24;
    u32 copy[16];
    u32 per, verts, i;

    if (code < 0x20 || code >= 0x40 || !(code & 0x04) || len > 16) {
        psyq_gpu_hash_words(w, len); /* not a textured polygon */
        return;
    }
    per = (code & 0x10) ? 3 : 2; /* words per vertex after the first */
    verts = (code & 0x08) ? 4 : 3;
    if (len < 1 + 2 + (verts - 1) * per) {
        psyq_gpu_hash_words(w, len);
        return;
    }
    for (i = 0; i < len; i++) {
        copy[i] = w[i];
    }
    for (i = 2; i < verts; i++) {
        copy[2 + i * per] &= 0xFFFF;
    }
    psyq_gpu_hash_words(copy, len);
}

/* Follows the list from `p` (an OT entry or a primitive) to the 0xFFFFFF terminator, hashing every primitive's
 * words (the `len` words after its tag). */
static void psyq_gpu_walk(const u32 *p, const char *who) {
    const u32 *start = p;
    const u8 *base = port_arena;
    const u8 *lo;
    const u8 *hi;
    u32 prims = 0;
    u32 steps = 0;

    psyq_gpu_window(&lo, &hi);
    for (;;) {
        u32 tag = *p;
        u32 len = tag >> 24;
        u32 next = tag & 0xFFFFFF;
        const u32 *q;

        if (len != 0) {
            psyq_gpu_hash_prim(p + 1, len);
            gpu_gp0_words(p + 1, len);
            prims++;
        }
        if (next == 0xFFFFFF) {
            break;
        }
        q = (const u32 *)(base + next);
        if ((const u8 *)q < lo || (const u8 *)q + 4 > hi) {
            PSYQ_TRACE("%s: tag %06x at %u points outside the walkable window; stopped", who, next, PSYQ_PTR(p));
            break;
        }
        if (++steps > (1u << 22)) {
            PSYQ_TRACE("%s: more than 4M links from %u; a cycle? stopped", who, PSYQ_PTR(p));
            break;
        }
        p = q;
    }
    psyq_gpu_count += prims;
    PSYQ_TRACE("%s %u: %u primitives, hash %08x", who, PSYQ_PTR(start), prims, psyq_gpu_hash);
}

/* ---- environments (real: pure data) ---- */

/* Fills `env` as Psy-Q documents SetDefDispEnv: disp = the rectangle, screen 0 (the default raster), not
 * interlaced, not 24-bit. */
DISPENV *SetDefDispEnv(DISPENV *env, s32 x, s32 y, s32 w, s32 h) {
    PSYQ_TRACE("SetDefDispEnv %d,%d %dx%d", x, y, w, h);
    env->disp.x = (s16)x;
    env->disp.y = (s16)y;
    env->disp.w = (s16)w;
    env->disp.h = (s16)h;
    env->screen.x = 0;
    env->screen.y = 0;
    env->screen.w = 0;
    env->screen.h = 0;
    env->isinter = 0;
    env->isrgb24 = 0;
    env->pad0 = 0;
    env->pad1 = 0;
    return env;
}

/* Fills `env` as Psy-Q documents SetDefDrawEnv: clip = the rectangle, offset = its corner, no texture window,
 * tpage of VRAM (640, 0) (= 10), dithering on, drawing to the displayed area allowed unless the area is taller
 * than 288 lines (an interlaced 480-line screen), no background clear; dr_env is left as found (Psy-Q builds it in
 * SetDrawEnv). Checked against the PS1's (golden family gpu, api_setdefdrawenv_*: 256, 257, 288 lines: dfe 1;
 * 289, 480: 0). */
DRAWENV *SetDefDrawEnv(DRAWENV *env, s32 x, s32 y, s32 w, s32 h) {
    PSYQ_TRACE("SetDefDrawEnv %d,%d %dx%d", x, y, w, h);
    env->clip.x = (s16)x;
    env->clip.y = (s16)y;
    env->clip.w = (s16)w;
    env->clip.h = (s16)h;
    env->ofs[0] = (s16)x;
    env->ofs[1] = (s16)y;
    env->tw.x = 0;
    env->tw.y = 0;
    env->tw.w = 0;
    env->tw.h = 0;
    env->tpage = GetTPage(0, 0, 640, 0);
    env->dtd = 1;
    env->dfe = (h < 289) ? 1 : 0;
    env->isbg = 0;
    env->r0 = 0;
    env->g0 = 0;
    env->b0 = 0;
    return env;
}

/* The GP0 word that sets a texture window (E2): no window when tw is empty. */
static u32 psyq_gpu_tw_word(const RECT *tw) {
    if (tw->w == 0 && tw->h == 0) {
        return 0xE2000000u;
    }
    return 0xE2000000u | ((u32)((-tw->w) >> 3) & 0x1F) | (((u32)((-tw->h) >> 3) & 0x1F) << 5)
           | (((u32)(tw->x >> 3) & 0x1F) << 10) | (((u32)(tw->y >> 3) & 0x1F) << 15);
}

/* A coordinate clamped to the VRAM (0..1023 or 0..511), as LIBGPU's packet builders clamp them. */
static s32 psyq_gpu_clamp(s32 v, s32 max) {
    return v < 0 ? 0 : v > max ? max : v;
}

/* Real: the DR_ENV packet for `env`, word for word as the PS1's SetDrawEnv writes it (golden family gpu,
 * api_setdrawenv_*): E3 (the clip's corner) and E4 (its far corner), both clamped to the VRAM, E5 (the offset), E1
 * (tpage, dither, dfe), E2 (the texture window), E6 0 (no mask bits); with isbg (any non-zero value) a TILE (GP0 60h,
 * opaque) of the clip rectangle (its size clamped to 0..1023 x 0..511) in the colour (r0, g0, b0), placed relative to
 * the offset (clip - ofs). */
void SetDrawEnv(DR_ENV *dr_env, DRAWENV *env) {
    u32 *c = dr_env->code;
    u32 n = 6;
    s32 x2 = psyq_gpu_clamp(env->clip.x + env->clip.w - 1, 1023);
    s32 y2 = psyq_gpu_clamp(env->clip.y + env->clip.h - 1, 511);

    PSYQ_TRACE("SetDrawEnv clip %d,%d %dx%d ofs %d,%d tpage %x isbg %d", env->clip.x, env->clip.y, env->clip.w,
               env->clip.h, env->ofs[0], env->ofs[1], env->tpage, env->isbg);
    c[0] = 0xE3000000u | ((u32)psyq_gpu_clamp(env->clip.y, 511) << 10) | (u32)psyq_gpu_clamp(env->clip.x, 1023);
    c[1] = 0xE4000000u | ((u32)y2 << 10) | (u32)x2;
    c[2] = 0xE5000000u | (((u32)env->ofs[1] & 0x7FF) << 11) | ((u32)env->ofs[0] & 0x7FF);
    c[3] = _get_mode(env->dfe, env->dtd, env->tpage);
    c[4] = psyq_gpu_tw_word(&env->tw);
    c[5] = 0xE6000000u;
    if (env->isbg) {
        c[6] = 0x60000000u | ((u32)env->b0 << 16) | ((u32)env->g0 << 8) | env->r0;
        c[7] = ((u32)(u16)(env->clip.y - env->ofs[1]) << 16) | (u16)(env->clip.x - env->ofs[0]);
        c[8] = ((u32)psyq_gpu_clamp(env->clip.h, 511) << 16) | (u32)psyq_gpu_clamp(env->clip.w, 1023);
        n = 9;
    }
    setlen(dr_env, n);
}

/* The video output (psyq.h): the software GPU's VRAM (gpu.c) and what PutDispEnv/SetDispMask set. */
const u16 *psyq_gpu_vram(void) {
    return gpu_vram_pixels();
}

void psyq_gpu_display(PsyqDisplay *out) {
    *out = psyq_gpu_disp;
}

DISPENV *PutDispEnv(DISPENV *env) {
    PSYQ_TRACE("PutDispEnv disp %d,%d %dx%d inter %d", env->disp.x, env->disp.y, env->disp.w, env->disp.h,
               env->isinter);
    psyq_gpu_disp.x = env->disp.x;
    psyq_gpu_disp.y = env->disp.y;
    psyq_gpu_disp.w = env->disp.w;
    psyq_gpu_disp.h = env->disp.h;
    psyq_gpu_disp.rgb24 = env->isrgb24;
    psyq_gpu_disp.interlace = env->isinter;
    return env;
}

/* ---- primitive setters (real: pure data) ---- */

void SetDrawTPage(DR_TPAGE *p, s32 dfe, s32 dtd, s32 tpage) {
    setDrawTPage(p, dfe, dtd, tpage);
}

/* The VRAM-to-VRAM copy packet: a cache flush (0x01), the copy command (0x80), source, destination, size.
 * The PS1 builds the same five words (golden family gpu, api_setdrawmove_*). */
void SetDrawMove(DR_MOVE *p, RECT *rect, s32 x, s32 y) {
    setlen(p, 5);
    p->code[0] = 0x01000000u;
    p->code[1] = 0x80000000u;
    p->code[2] = ((u32)(u16)rect->y << 16) | (u16)rect->x;
    p->code[3] = ((u32)(u16)y << 16) | (u16)x;
    p->code[4] = ((u32)(u16)rect->h << 16) | (u16)rect->w;
}

u16 GetTPage(s32 tp, s32 abr, s32 x, s32 y) {
    return (u16)getTPage(tp, abr, x, y);
}

u16 GetClut(s32 x, s32 y) {
    return (u16)getClut(x, y);
}

void SetSemiTrans(void *p, s32 abe) {
    setSemiTrans(p, abe);
}

void SetSprt(SPRT *p) {
    setSprt(p);
}

/* ---- ordering tables (real) ---- */

/* ot[n-1] -> ... -> ot[0] -> end: drawing starts at ot[n-1] (the far end). */
u32 *ClearOTagR(u32 *ot, s32 n) {
    s32 i;

    if (n > 0) {
        for (i = 1; i < n; i++) {
            ot[i] = PTR_TO_U32(&ot[i - 1]) & 0xFFFFFF;
        }
        ot[0] = 0xFFFFFF;
    }
    return ot;
}

/* ot[0] -> ot[1] -> ... -> ot[n-1] -> end. */
u32 *ClearOTag(u32 *ot, s32 n) {
    s32 i;

    if (n > 0) {
        for (i = 0; i < n - 1; i++) {
            ot[i] = PTR_TO_U32(&ot[i + 1]) & 0xFFFFFF;
        }
        ot[n - 1] = 0xFFFFFF;
    }
    return ot;
}

/* Draws (and records) the list from `p` (gfx_draw_layer passes the far end of a reversed table). */
void DrawOTag(u32 *p) {
    psyq_gpu_walk(p, "DrawOTag");
}

/* ---- the GPU itself (gpu.c) ---- */

/* Drawing completes when it is queued, so the queue is always empty (0) in both modes. */
s32 DrawSync(s32 mode) {
    PSYQ_TRACE("DrawSync %d", mode);
    return 0;
}

/* Mode 0 resets the GPU (GP1(00h): the drawing state E1..E6 zero, the display off), 3 resets the drawing state and
 * keeps the display, 1 cancels the drawing in progress (none here: drawing completes when it is queued). */
int ResetGraph(int mode) {
    PSYQ_TRACE("ResetGraph %d", mode);
    if (mode == 0 || mode == 3) {
        gpu_reset_state();
    }
    if (mode == 0) {
        psyq_gpu_disp.enabled = 0;
    }
    return 0;
}

/* Returns the previous level, always 0 here. */
int SetGraphDebug(int level) {
    PSYQ_TRACE("SetGraphDebug %d", level);
    return 0;
}

void SetDispMask(int mask) {
    PSYQ_TRACE("SetDispMask %d", mask);
    psyq_gpu_disp.enabled = mask != 0;
}

/* ClearImage and ClearImage2, as LIBGPU does them (golden family gpu, api_clearimage*): the size clamped to the VRAM
 * (0..1023 x 0..511: a 1024x512 clear leaves the last column and row); a rectangle whose x and width are multiples of
 * 64 is a fill (GP0 02h), any other an opaque tile drawn with the whole VRAM as the draw area and no offset, after which
 * the draw area and offset are put back. Both reset the mask setting (E6 0) and rewrite E1 with the current texpage,
 * dither and dfe, ClearImage2 forcing dfe on. */
static int psyq_gpu_clear(RECT *rect, u8 r, u8 g, u8 b, int dfe) {
    u32 e1, e3, e4, e5;
    u32 w[16];
    u32 rgb = ((u32)b << 16) | ((u32)g << 8) | r;
    s32 cw = psyq_gpu_clamp(rect->w, 1023), ch = psyq_gpu_clamp(rect->h, 511);
    u32 n = 0;

    gpu_draw_state(&e1, &e3, &e4, &e5);
    e1 = (e1 & 0xFF0007FFu) | ((u32)dfe << 10);
    w[n++] = 0xE6000000u;
    w[n++] = e1;
    if ((rect->x & 0x3F) != 0 || (cw & 0x3F) != 0) {
        w[n++] = 0xE3000000u;
        w[n++] = 0xE4FFFFFFu;
        w[n++] = 0xE5000000u;
        w[n++] = 0x60000000u | rgb;
        w[n++] = ((u32)(u16)rect->y << 16) | (u16)rect->x;
        w[n++] = ((u32)ch << 16) | (u32)cw;
        w[n++] = e3;
        w[n++] = e4;
        w[n++] = e5;
    } else {
        w[n++] = 0x02000000u | rgb;
        w[n++] = ((u32)(u16)rect->y << 16) | (u16)rect->x;
        w[n++] = ((u32)ch << 16) | (u32)cw;
    }
    gpu_gp0_words(w, n);
    return 0;
}

int ClearImage(RECT *rect, u8 r, u8 g, u8 b) {
    PSYQ_TRACE("ClearImage %d,%d %dx%d rgb %u,%u,%u", rect->x, rect->y, rect->w, rect->h, r, g, b);
    return psyq_gpu_clear(rect, r, g, b, 0);
}

int ClearImage2(RECT *rect, u8 r, u8 g, u8 b) {
    PSYQ_TRACE("ClearImage2 %d,%d %dx%d rgb %u,%u,%u", rect->x, rect->y, rect->w, rect->h, r, g, b);
    return psyq_gpu_clear(rect, r, g, b, 1);
}

/* CPU to VRAM: the rectangle's pixels, row by row, from `p` (two per word). */
void LoadImage(RECT *rect, u32 *p) {
    PSYQ_TRACE("LoadImage %d,%d %dx%d from %u", rect->x, rect->y, rect->w, rect->h, PSYQ_PTR(p));
    gpu_load_image(rect->x, rect->y, rect->w, rect->h, (const u16 *)p);
}

/* VRAM to VRAM (GP0 80h). An empty rectangle (w or h 0) does nothing and returns -1, as on the PS1 (golden family
 * gpu, api_moveimage_4/5). */
s32 MoveImage(RECT *rect, s32 x, s32 y) {
    u32 w[4];

    PSYQ_TRACE("MoveImage %d,%d %dx%d -> %d,%d", rect->x, rect->y, rect->w, rect->h, x, y);
    if (rect->w == 0 || rect->h == 0) {
        return -1;
    }
    w[0] = 0x80000000u;
    w[1] = ((u32)(u16)rect->y << 16) | (u16)rect->x;
    w[2] = ((u32)(u16)y << 16) | (u16)x;
    w[3] = ((u32)(u16)rect->h << 16) | (u16)rect->w;
    gpu_gp0_words(w, 4);
    return 0;
}

/* Drawing finishes inside DrawOTag, so the GPU is always idle here, and the PS1's BreakDraw returns NULL while it is
 * idle (golden family gpu, api_breakdraw_idle): FIGHTSTG's cursor (which skips its VRAM copies on -1) draws them and
 * ContinueDraw has nothing to resume. */
u32 *BreakDraw(void) {
    PSYQ_TRACE("BreakDraw");
    return NULL;
}

/* 0 = idle (the PS1 returns -1 after max_count failed polls). */
s32 IsIdleGPU(s32 max_count) {
    PSYQ_TRACE("IsIdleGPU %d", max_count);
    return 0;
}

/* Draws (and records) `insaddr`'s list, then resumes `contaddr`'s (BreakDraw's result; NULL: none). */
void ContinueDraw(u32 *insaddr, u32 *contaddr) {
    psyq_gpu_walk(insaddr, "ContinueDraw");
    if (contaddr != NULL && contaddr != &psyq_gpu_terminator) {
        psyq_gpu_walk(contaddr, "ContinueDraw (resumed)");
    }
}
