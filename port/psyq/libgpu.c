/* port/psyq/libgpu.c: LIBGPU. No VRAM and no drawing in the M1 skeleton: the environment setters and the
 * ordering-table builders are real (they only compute), the transfers are recorded, DrawOTag/ContinueDraw walk the
 * list they are given and hash the primitives (psyq_gpu_take_hash: the "primitive stream per frame" of the M1 test).
 *
 * Tags (PC_PORT_PLAN 2.4): a tag's low 24 bits are an offset inside the 16 MB window the ordering table lives in, so
 * the walk resolves `tag & 0xFFFFFF` against `ot & ~0xFFFFFF` and follows it only inside the window psyq_set_arena
 * gave (by default the heap, port_heap_start..port_heap_end). */
#include <stdint.h>
#include <stdlib.h>

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
    uintptr_t base = (uintptr_t)p & ~(uintptr_t)0xFFFFFF;
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
 * than 256 lines (an interlaced 480-line screen), no background clear.
 * Assumption to verify (M2, against the emulator's DRAWENV): the dfe rule is "h <= 256", and dr_env is left as
 * found (Psy-Q builds it in SetDrawEnv). */
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
    env->dfe = (h <= 256) ? 1 : 0;
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

/* Real: the DR_ENV packet for `env`: E1 (tpage, dither, dfe), E2 (texture window), E3/E4 (clip), E5 (offset),
 * and with isbg a fill of the clip rectangle (GP0 0x02).
 * Assumption to verify (M2, against a GPU-command trace): Psy-Q 4.7's exact word order and whether it emits more
 * (an E6 mask word). The M1 recorder only hashes these words. */
void SetDrawEnv(DR_ENV *dr_env, DRAWENV *env) {
    u32 *c = dr_env->code;
    u32 n = 5;
    u32 x2 = (u32)(env->clip.x + env->clip.w - 1);
    u32 y2 = (u32)(env->clip.y + env->clip.h - 1);

    PSYQ_TRACE("SetDrawEnv clip %d,%d %dx%d ofs %d,%d tpage %x isbg %d", env->clip.x, env->clip.y, env->clip.w,
               env->clip.h, env->ofs[0], env->ofs[1], env->tpage, env->isbg);
    c[0] = _get_mode(env->dfe, env->dtd, env->tpage);
    c[1] = psyq_gpu_tw_word(&env->tw);
    c[2] = 0xE3000000u | (((u32)env->clip.y & 0x3FF) << 10) | ((u32)env->clip.x & 0x3FF);
    c[3] = 0xE4000000u | ((y2 & 0x3FF) << 10) | (x2 & 0x3FF);
    c[4] = 0xE5000000u | (((u32)env->ofs[1] & 0x7FF) << 11) | ((u32)env->ofs[0] & 0x7FF);
    if (env->isbg) {
        c[5] = 0x02000000u | ((u32)env->b0 << 16) | ((u32)env->g0 << 8) | env->r0;
        c[6] = ((u32)(u16)env->clip.y << 16) | (u16)env->clip.x;
        c[7] = ((u32)(u16)env->clip.h << 16) | (u16)env->clip.w;
        n = 8;
    }
    setlen(dr_env, n);
}

/* Stub: the display environment would go to the GPU; recorded only. Returns env (Psy-Q returns it). */
/* The video output (psyq.h). M2 step-0 stub: T9 ("gpu") replaces the VRAM with the software GPU's. */
static u16 psyq_gpu_vram_pixels[1024 * 512];
static PsyqDisplay psyq_gpu_disp;

const u16 *psyq_gpu_vram(void) {
    return psyq_gpu_vram_pixels;
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
 * Assumption to verify (M2): Psy-Q's DR_MOVE word layout. */
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

/* Recorded: walks the list from `p` (gfx_draw_layer passes the far end of a reversed table). */
void DrawOTag(u32 *p) {
    psyq_gpu_walk(p, "DrawOTag");
}

/* ---- the GPU itself (stubs: the M1 skeleton has none) ---- */

/* Stub: nothing is ever queued, so the queue is empty (0) in both modes. */
s32 DrawSync(s32 mode) {
    PSYQ_TRACE("DrawSync %d", mode);
    return 0;
}

int ResetGraph(int mode) {
    PSYQ_TRACE("ResetGraph %d", mode);
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

int ClearImage(RECT *rect, u8 r, u8 g, u8 b) {
    PSYQ_TRACE("ClearImage %d,%d %dx%d rgb %u,%u,%u", rect->x, rect->y, rect->w, rect->h, r, g, b);
    return 0;
}

int ClearImage2(RECT *rect, u8 r, u8 g, u8 b) {
    PSYQ_TRACE("ClearImage2 %d,%d %dx%d rgb %u,%u,%u", rect->x, rect->y, rect->w, rect->h, r, g, b);
    return 0;
}

/* Stub: the pixels stay where they are (no VRAM); only the rectangle and the source are recorded. */
void LoadImage(RECT *rect, u32 *p) {
    PSYQ_TRACE("LoadImage %d,%d %dx%d from %u", rect->x, rect->y, rect->w, rect->h, PSYQ_PTR(p));
}

s32 MoveImage(RECT *rect, s32 x, s32 y) {
    PSYQ_TRACE("MoveImage %d,%d %dx%d -> %d,%d", rect->x, rect->y, rect->w, rect->h, x, y);
    return 0;
}

/* Stub: drawing finishes inside DrawOTag, so there is nothing to interrupt. The PS1's BreakDraw returns the
 * primitive at which the DMA stopped, and FIGHTSTG's cursor skips its VRAM copies when it gets -1 (nothing in
 * progress); to record those copies the stub hands out an empty list for ContinueDraw to resume at.
 * Assumption to verify (M2/M5): what the PS1 returns while the GPU is idle, and whether the cursor copies are
 * meant to be skipped in that case. */
u32 *BreakDraw(void) {
    PSYQ_TRACE("BreakDraw");
    return &psyq_gpu_terminator;
}

/* 0 = idle (the PS1 returns -1 after max_count failed polls). */
s32 IsIdleGPU(s32 max_count) {
    PSYQ_TRACE("IsIdleGPU %d", max_count);
    return 0;
}

/* Recorded: draws `insaddr`'s list, then resumes `contaddr`'s (BreakDraw's result). */
void ContinueDraw(u32 *insaddr, u32 *contaddr) {
    psyq_gpu_walk(insaddr, "ContinueDraw");
    if (contaddr != NULL && contaddr != &psyq_gpu_terminator) {
        psyq_gpu_walk(contaddr, "ContinueDraw (resumed)");
    }
}
