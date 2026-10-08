/* port/psyq/gpu.c: the GPU in software: a 1024x512 VRAM of 16-bit pixels and the GP0 commands that draw into it.
 * Written from public hardware documentation (psx-spx "GPU"), then measured against the emulator: the layer-1 family
 * `gpu` (tests/golden/families/gpu.py) runs command lists on the PS1 in PCSX-Redux (its software GPU) and reads the
 * VRAM back, tests/host/gpu_replay.py replays them here. Where the two disagree the emulator's pixels win (the M2 test is
 * a pixel comparison with it), except where it contradicts documented hardware in ways the game never meets; each
 * such rule is listed in tests/host/known_mismatches.json:
 *  - the fill (GP0 02h) rounds x down to 16 pixels and wraps at the VRAM's edges (the emulator keeps x and clips);
 *  - VRAM copies (80h) obey the mask bits and wrap at the edges (the emulator ignores the mask bits);
 *  - a 1x1 draw area draws its pixel (the emulator draws nothing).
 * The emulator's rules taken over (each measured by the family's probes, named in the comments below): triangle
 * coverage, attribute rounding, the order of a quad's two triangles, the line stepping, the colour pipeline (modulation,
 * blending, dithering in 8 bits), Gouraud-textured colour per pair of pixels, vertex + offset wrapping at 11 bits.
 * Not reproduced (rare 1-step differences): exact .5 interpolation ties, and modes 2 and 3 of modulated
 * semi-transparent textures, where the emulator computes two pixels at once and lets carries cross between them.
 *
 * GP0 arrives as a word stream (gpu_gp0_write: DrawOTag's packets, LoadImage's transfer); commands may span packets.
 * GP1 is not modelled beyond what LIBGPU needs (gpu_reset_state: GP1(00h)'s drawing state; the display is libgpu.c's).
 * Not modelled: timing (a command completes when written), the texture cache (a texel is what the VRAM holds when its
 * pixel is drawn; the decoded segments below only remember what decoding would give again), GPUSTAT,
 * VRAM-to-CPU transfers (the game never reads VRAM back), the interlaced field skip of `dfe` (only interlaced
 * 480-line drawing would see it; the game's 480-line screens are MDEC frames loaded with LoadImage). */
#include <stdlib.h>
#include <string.h>

#include "psyq_internal.h"

#ifndef PC_PORT
#error "port/psyq is the host shim: compile it with -DPC_PORT"
#endif

/* Speed (the rasteriser runs every frame, tens of thousands of frames per test): port/CMakeLists.txt builds this file at
 * -O3 inside the -O0 port (the span loops rely on inlining and on the compiler's vectorisation of the select loops; the
 * pragma is for other -O0 builds). What makes it fast, every step byte-identical (the first_battle_save script: 22 s ->
 * about 9 s, against about 2 s without drawing; port/psyq/README.md):
 *  - span loops instantiated per primitive kind (texture depth, blend mode, dithering) instead of a per-pixel
 *    pipeline that tests the mode (gpu_pixel stays the reference, for lines and mask-checked primitives);
 *  - triangle edges walked a row at a time and attributes stepped per pixel, with no division per row or pixel;
 *  - sprite rows from texels read ahead: decoded segments kept while their VRAM is unchanged (write stamps), drawn by
 *    select loops that compilers vectorise (copies; semi-transparency on whole 15-bit pixels when the colour is 128);
 *  - fills with memset where both bytes of the colour are equal. */
#if defined(__GNUC__) && !defined(__clang__) && !defined(__OPTIMIZE__)
#pragma GCC optimize("O2")
#endif

#define VRAM_W 1024
#define VRAM_H 512

static u16 gpu_vram[VRAM_W * VRAM_H];

/* Write stamps: every write to the VRAM stamps the 64-pixel blocks it touches with the current stamp (gpu_touch), which
 * advances whenever a texture segment is decoded (gpu_decoded): a decoded segment stays valid while none of the blocks
 * it was made from (its texture words, its CLUT) carries a stamp newer than its own. */
#define GPU_BLOCKS 16 /* per VRAM row */
static u32 gpu_stamp = 1;
static u32 gpu_block_stamp[VRAM_H][GPU_BLOCKS];

/* Pixels x0 .. x1 - 1 of row y were (or may have been) written; 0 <= x0 < x1 <= 1024. */
static inline void gpu_touch(int y, int x0, int x1) {
    u32 *b = gpu_block_stamp[y];
    int i;

    for (i = x0 >> 6; i <= (x1 - 1) >> 6; i++) {
        b[i] = gpu_stamp;
    }
}

static void gpu_segs_reset(void);
static u32 gpu_generation; /* gpu_stamp_generation: the stamps started over this many times */

/* The listener (psyq_internal.h "gpu.c's decoded commands"): the hardware renderer's, or NULL. */
static void (*gpu_listener)(const GpuEvent *ev);

void gpu_set_listener(void (*listener)(const GpuEvent *ev)) {
    gpu_listener = listener;
}

const u32 *gpu_block_stamps(void) {
    return &gpu_block_stamp[0][0];
}

u32 gpu_stamp_bump(void) {
    if (gpu_stamp == 0xFFFFFFFFu) {
        gpu_segs_reset();
    }
    return gpu_stamp++;
}

u32 gpu_stamp_generation(void) {
    return gpu_generation;
}

static struct {
    /* E1: texpage bits 0-8 (base x 0-3, base y 4, semi-transparency 5-6, depth 7-8), dither 9, dfe 10. */
    u32 texpage;
    int dither;
    int dfe;
    /* E2: the texture window in texels (mask and offset already multiplied by 8). */
    int tw_mask_x, tw_mask_y, tw_off_x, tw_off_y;
    /* E3/E4: the drawing area (inclusive); E5: the drawing offset (signed 11-bit). */
    int area_x0, area_y0, area_x1, area_y1;
    int ofs_x, ofs_y;
    /* E6: the mask bit OR-ed into every pixel drawn, and whether pixels with the mask bit set are kept. */
    u16 set_mask;
    int check_mask;
    /* The command being assembled. */
    u32 cmd[16];
    int n, need;
    /* A polyline in progress: its words so far are the last vertex (and colour). */
    int polyline;
    /* A CPU-to-VRAM transfer in progress: the rectangle and the next pixel. */
    int load_left;          /* pixels still to come */
    int load_x, load_y, load_w, load_h, load_i;
} g;

/* The drawing state as the E1, E3, E4 and E5 words that set it (LIBGPU's ClearImage puts them back after its tile;
 * on the PS1 it reads them from the GPU's status and info registers). */
void gpu_draw_state(u32 *e1, u32 *e3, u32 *e4, u32 *e5) {
    *e1 = 0xE1000000u | g.texpage | ((u32)g.dither << 9) | ((u32)g.dfe << 10);
    *e3 = 0xE3000000u | ((u32)g.area_y0 << 10) | (u32)g.area_x0;
    *e4 = 0xE4000000u | ((u32)g.area_y1 << 10) | (u32)g.area_x1;
    *e5 = 0xE5000000u | (((u32)g.ofs_y & 0x7FF) << 11) | ((u32)g.ofs_x & 0x7FF);
}

const u16 *gpu_vram_pixels(void) {
    return gpu_vram;
}

void gpu_reset_state(void) {
    g.texpage = 0;
    g.dither = 0;
    g.dfe = 0;
    g.tw_mask_x = g.tw_mask_y = g.tw_off_x = g.tw_off_y = 0;
    g.area_x0 = g.area_y0 = g.area_x1 = g.area_y1 = 0;
    g.ofs_x = g.ofs_y = 0;
    g.set_mask = 0;
    g.check_mask = 0;
    g.n = g.need = 0;
    g.polyline = 0;
    g.load_left = 0;
}

void gpu_power_on(void) {
    int y;

    memset(gpu_vram, 0, sizeof(gpu_vram));
    for (y = 0; y < VRAM_H; y++) {
        gpu_touch(y, 0, VRAM_W);
    }
    gpu_reset_state();
    if (gpu_listener != NULL) {
        GpuEvent ev;
        memset(&ev, 0, sizeof(ev));
        ev.kind = GPU_EV_POWER_ON;
        gpu_listener(&ev);
    }
}

static inline int sext11(u32 v) {
    return (s32)(v << 21) >> 21;
}

/* ---- the pixel pipeline ---- */

#define GPU_INLINE static inline __attribute__((always_inline))

/* psx-spx "Dithering": the offset added to an 8-bit colour before it is cut to 5 bits, by (y & 3, x & 3). */
static const s8 gpu_dither[4][4] = {
    { -4, 0, -3, 1 },
    { 2, -2, 3, -1 },
    { -3, 1, -4, 0 },
    { 3, -1, 2, -2 },
};

typedef struct {
    int semi;           /* semi-transparent primitive */
    int abr;            /* its mode */
    int textured;       /* 0, or the texture depth + 1 (1 4-bit, 2 8-bit, 3 15-bit) */
    int raw;            /* textured, not modulated */
    int dither;         /* dithering applies */
    int gouraud;        /* a Gouraud primitive */
    int eight;          /* the 8-bit colour pipeline (the dithered primitives) */
    int tex_x, tex_y;   /* the texture page's corner */
    int clut_x, clut_y; /* the CLUT's first entry */
    /* The texture window (E2) as u = (u & u_and) | u_or, the same for v: (u & 0xFF & ~mask) | (offset & mask). */
    int u_and, u_or, v_and, v_or;
} Mode;

/* A texel of a texture row (v applied) at the windowed u: DEPTH (1 4-bit, 2 8-bit, 3 15-bit) is a constant in the span
 * loops below, so each of them gets its own fetch. */
GPU_INLINE u16 gpu_fetch(const u16 *row, const u16 *clut, int tex_x, int clut_x, int u, const int depth) {
    u16 px;

    if (depth == 1) {
        px = row[(tex_x + (u >> 2)) & 1023];
        return clut[(clut_x + ((px >> ((u & 3) * 4)) & 0xF)) & 1023];
    }
    if (depth == 2) {
        px = row[(tex_x + (u >> 1)) & 1023];
        return clut[(clut_x + ((px >> ((u & 1) * 8)) & 0xFF)) & 1023];
    }
    return row[(tex_x + u) & 1023];
}

static inline u16 gpu_texel(const Mode *m, int u, int v) {
    const u16 *row = &gpu_vram[((m->tex_y + ((v & m->v_and) | m->v_or)) & 511) * VRAM_W];
    const u16 *clut = &gpu_vram[m->clut_y * VRAM_W];

    u = (u & m->u_and) | m->u_or;
    switch (m->textured) {
    case 1:
        return gpu_fetch(row, clut, m->tex_x, m->clut_x, u, 1);
    case 2:
        return gpu_fetch(row, clut, m->tex_x, m->clut_x, u, 2);
    default:
        return gpu_fetch(row, clut, m->tex_x, m->clut_x, u, 3);
    }
}

static inline int gpu_clamp(int v, int hi) {
    return v < 0 ? 0 : v > hi ? hi : v;
}

static inline int gpu_min31(int v) {
    return v > 31 ? 31 : v;
}

/* Semi-transparency of one channel: the background b and the foreground f in the same units (5 or 8 bits), the
 * result clamped to 0..hi. */
static inline int gpu_blend(int abr, int b, int f, int hi) {
    switch (abr) {
    case 0:
        return gpu_clamp((b + f) >> 1, hi);
    case 1:
        return gpu_clamp(b + f, hi);
    case 2:
        return gpu_clamp(b - f, hi);
    default:
        return gpu_clamp(b + (f >> 2), hi);
    }
}

/* One pixel: colour (r, g, b) 8-bit (the vertex colour, interpolated), the texel at (u, v) if textured.
 * The colour pipeline, as the emulator computes it (golden family gpu: dither_*, edge_modulate_*, rect_sprt*,
 * semi_probe_*):
 *  - without dithering, in 5-bit units: the texel modulated as (t * c) >> 7 (raw: t itself; mode 3 quarters it:
 *    ((t >> 2) * c) >> 7, Gouraud-textured (t * c) >> 9), not clamped before the blend; the blend with the background,
 *    then the clamp to 31; an untextured colour is c >> 3;
 *  - with dithering (Gouraud primitives only), in 8-bit units: the foreground (t * c) >> 4 (raw: t << 3; untextured:
 *    c), blended with the background << 3 and clamped to 0..255, then the dither offset added, clamped, >> 3.
 * A textured pixel whose texel is 0x0000 is not drawn; the texel's bit 15 decides whether a semi-transparent
 * primitive blends there, and is the pixel's mask bit (with E6's set-mask bit).
 * This is the reference: lines and mask-checked primitives draw with it; the span loops below specialise it. */
static inline __attribute__((always_inline)) void gpu_pixel(const Mode *m, int x, int y, int r, int gg, int b, int u,
                                                            int v) {
    u16 *dst = &gpu_vram[y * VRAM_W + x];
    u16 stp = 0;
    int semi = m->semi;
    int fr, fg, fb;

    if (g.check_mask && (*dst & 0x8000)) {
        return;
    }
    if (m->textured) {
        u16 t = gpu_texel(m, u, v);
        int tr = t & 31, tg = (t >> 5) & 31, tb = (t >> 10) & 31;

        if (t == 0) {
            return;
        }
        stp = t & 0x8000;
        semi = semi && stp;
        if (m->eight) {
            if (m->raw) {
                fr = tr << 3, fg = tg << 3, fb = tb << 3;
            } else {
                fr = (tr * r) >> 4, fg = (tg * gg) >> 4, fb = (tb * b) >> 4;
            }
        } else if (m->raw) {
            fr = tr, fg = tg, fb = tb;
        } else if (semi && m->abr == 3) {
            /* Mode 3: the quarter of the modulated texel, added to the background. */
            if (m->gouraud) {
                fr = (tr * r) >> 9, fg = (tg * gg) >> 9, fb = (tb * b) >> 9;
            } else {
                fr = ((tr >> 2) * r) >> 7, fg = ((tg >> 2) * gg) >> 7, fb = ((tb >> 2) * b) >> 7;
            }
            *dst = (u16)(gpu_clamp((*dst & 31) + fr, 31) | (gpu_clamp(((*dst >> 5) & 31) + fg, 31) << 5)
                         | (gpu_clamp(((*dst >> 10) & 31) + fb, 31) << 10) | stp | g.set_mask);
            return;
        } else {
            fr = (tr * r) >> 7, fg = (tg * gg) >> 7, fb = (tb * b) >> 7;
        }
    } else if (m->eight) {
        fr = r, fg = gg, fb = b;
    } else {
        fr = r >> 3, fg = gg >> 3, fb = b >> 3;
    }
    if (m->eight) {
        int d = m->dither ? gpu_dither[y & 3][x & 3] : 0;

        if (semi) {
            fr = gpu_blend(m->abr, (*dst & 31) << 3, fr, 255);
            fg = gpu_blend(m->abr, ((*dst >> 5) & 31) << 3, fg, 255);
            fb = gpu_blend(m->abr, ((*dst >> 10) & 31) << 3, fb, 255);
        }
        fr = gpu_clamp(fr + d, 255) >> 3;
        fg = gpu_clamp(fg + d, 255) >> 3;
        fb = gpu_clamp(fb + d, 255) >> 3;
    } else if (semi) {
        fr = gpu_blend(m->abr, *dst & 31, fr, 31);
        fg = gpu_blend(m->abr, (*dst >> 5) & 31, fg, 31);
        fb = gpu_blend(m->abr, (*dst >> 10) & 31, fb, 31);
    } else {
        fr = fr > 31 ? 31 : fr;
        fg = fg > 31 ? 31 : fg;
        fb = fb > 31 ? 31 : fb;
    }
    *dst = (u16)(fr | (fg << 5) | (fb << 10) | stp | g.set_mask);
}

/* ---- the span loops ----
 * gpu_pixel specialised: each loop below is instantiated per texture depth and blend mode (constants after inlining),
 * for primitives without mask checks. Their pixels are gpu_pixel's, in the same order, each texel read when its pixel
 * is drawn (a primitive may draw over its own texture or CLUT). Raw textures arrive with the colour 128 in every
 * channel, which gives the same pixels as raw everywhere: (t * 128) >> 7 = t, ((t >> 2) * 128) >> 7 = (t * 128) >> 9
 * = t >> 2 (mode 3), (t * 128) >> 4 = t << 3 (8-bit pipeline). The loops copy what they read of the drawing state
 * into locals first: the VRAM stores could otherwise alias it (-fno-strict-aliasing). */

#define GPU_COPY (-2)   /* BLEND: opaque, texels drawn as they are (raw, or modulated by 128) */
#define GPU_OPAQUE (-1) /* BLEND: not semi-transparent; 0..3 a semi-transparent primitive's mode */

/* A pixel of a primitive with one colour (no Gouraud shading, hence no dithering): t the texel if TEX (not 0). */
GPU_INLINE void gpu_shade_flat(u16 *dst, u16 t, int r, int gg, int b, u16 set_mask, const int tex, const int blend) {
    u16 stp = 0;
    int fr, fg, fb, bg;

    if (tex) {
        int tr = t & 31, tg = (t >> 5) & 31, tb = (t >> 10) & 31;

        stp = t & 0x8000;
        if (blend == GPU_COPY) {
            *dst = t | set_mask;
            return;
        }
        if (blend == GPU_OPAQUE || !stp) {
            *dst = (u16)(gpu_min31((tr * r) >> 7) | (gpu_min31((tg * gg) >> 7) << 5) | (gpu_min31((tb * b) >> 7) << 10)
                         | stp | set_mask);
            return;
        }
        if (blend == 3) {
            fr = ((tr >> 2) * r) >> 7, fg = ((tg >> 2) * gg) >> 7, fb = ((tb >> 2) * b) >> 7;
        } else {
            fr = (tr * r) >> 7, fg = (tg * gg) >> 7, fb = (tb * b) >> 7;
        }
    } else {
        fr = r >> 3, fg = gg >> 3, fb = b >> 3;
        if (blend < 0) {
            *dst = (u16)(fr | (fg << 5) | (fb << 10) | set_mask);
            return;
        }
        if (blend == 3) {
            fr >>= 2, fg >>= 2, fb >>= 2;
        }
    }
    /* The blend (mode 3's quarter taken above); every value here is >= 0. */
    bg = *dst;
    switch (blend) {
    case 0:
        fr = gpu_min31(((bg & 31) + fr) >> 1);
        fg = gpu_min31((((bg >> 5) & 31) + fg) >> 1);
        fb = gpu_min31((((bg >> 10) & 31) + fb) >> 1);
        break;
    case 2:
        fr = gpu_clamp((bg & 31) - fr, 31);
        fg = gpu_clamp(((bg >> 5) & 31) - fg, 31);
        fb = gpu_clamp(((bg >> 10) & 31) - fb, 31);
        break;
    default:
        fr = gpu_min31((bg & 31) + fr);
        fg = gpu_min31(((bg >> 5) & 31) + fg);
        fb = gpu_min31(((bg >> 10) & 31) + fb);
        break;
    }
    *dst = (u16)(fr | (fg << 5) | (fb << 10) | stp | set_mask);
}

/* A pixel of a Gouraud primitive (gpu_pixel with gouraud set): EIGHT the dithered 8-bit pipeline (d the dither
 * offset), SEMI whether the primitive is semi-transparent (mode abr), t the texel if TEX (not 0). */
GPU_INLINE void gpu_shade_gouraud(u16 *dst, int d, u16 t, int r, int gg, int b, int abr, u16 set_mask, const int tex,
                                  const int eight, const int semi_prim) {
    u16 stp = 0;
    int semi = semi_prim;
    int fr, fg, fb;

    if (tex) {
        int tr = t & 31, tg = (t >> 5) & 31, tb = (t >> 10) & 31;

        stp = t & 0x8000;
        semi = semi && stp;
        if (eight) {
            fr = (tr * r) >> 4, fg = (tg * gg) >> 4, fb = (tb * b) >> 4;
        } else if (semi && abr == 3) {
            fr = (tr * r) >> 9, fg = (tg * gg) >> 9, fb = (tb * b) >> 9;
            *dst = (u16)(gpu_clamp((*dst & 31) + fr, 31) | (gpu_clamp(((*dst >> 5) & 31) + fg, 31) << 5)
                         | (gpu_clamp(((*dst >> 10) & 31) + fb, 31) << 10) | stp | set_mask);
            return;
        } else {
            fr = (tr * r) >> 7, fg = (tg * gg) >> 7, fb = (tb * b) >> 7;
        }
    } else if (eight) {
        fr = r, fg = gg, fb = b;
    } else {
        fr = r >> 3, fg = gg >> 3, fb = b >> 3;
    }
    if (eight) {
        if (semi) {
            fr = gpu_blend(abr, (*dst & 31) << 3, fr, 255);
            fg = gpu_blend(abr, ((*dst >> 5) & 31) << 3, fg, 255);
            fb = gpu_blend(abr, ((*dst >> 10) & 31) << 3, fb, 255);
        }
        fr = gpu_clamp(fr + d, 255) >> 3;
        fg = gpu_clamp(fg + d, 255) >> 3;
        fb = gpu_clamp(fb + d, 255) >> 3;
    } else if (semi) {
        fr = gpu_blend(abr, *dst & 31, fr, 31);
        fg = gpu_blend(abr, (*dst >> 5) & 31, fg, 31);
        fb = gpu_blend(abr, (*dst >> 10) & 31, fb, 31);
    } else {
        fr = gpu_min31(fr);
        fg = gpu_min31(fg);
        fb = gpu_min31(fb);
    }
    *dst = (u16)(fr | (fg << 5) | (fb << 10) | stp | set_mask);
}

/* ---- primitives ---- */

/* The fast path of an opaque, untextured, undithered primitive without mask checks: one colour for every pixel. */
static inline u16 gpu_flat_colour(u32 rgb) {
    return (u16)(((rgb >> 3) & 0x1F) | (((rgb >> 11) & 0x1F) << 5) | (((rgb >> 19) & 0x1F) << 10) | g.set_mask);
}

/* n pixels of the colour c from p (memset when both bytes of c are equal, as for black). */
static inline void gpu_fill_pixels(u16 *p, int n, u16 c) {
    if ((c >> 8) == (c & 0xFF)) {
        memset(p, c & 0xFF, (size_t)n * 2);
        return;
    }
    while (n-- > 0) {
        *p++ = c;
    }
}

static inline void gpu_span_fill(int y, int xs, int xe, u16 c) {
    gpu_fill_pixels(&gpu_vram[y * VRAM_W + xs], xe - xs, c);
    gpu_touch(y, xs, xe);
}

typedef GpuVertex Vertex;

/* The event of a drawing primitive with mode m (the drawing state's area and mask settings are g's). */
static void gpu_event_mode(GpuEvent *ev, GpuEventKind kind, const Mode *m) {
    memset(ev, 0, sizeof(*ev));
    ev->kind = kind;
    ev->textured = m->textured;
    ev->raw = m->raw;
    ev->semi = m->semi;
    ev->abr = m->abr;
    ev->gouraud = m->gouraud;
    ev->dither = m->dither;
    ev->tex_x = m->tex_x;
    ev->tex_y = m->tex_y;
    ev->clut_x = m->clut_x;
    ev->clut_y = m->clut_y;
    ev->u_and = m->u_and;
    ev->u_or = m->u_or;
    ev->v_and = m->v_and;
    ev->v_or = m->v_or;
    ev->area_x0 = g.area_x0;
    ev->area_y0 = g.area_y0;
    ev->area_x1 = g.area_x1;
    ev->area_y1 = g.area_y1;
    ev->set_mask = g.set_mask;
    ev->check_mask = g.check_mask;
}

/* The texture page attribute of a textured polygon (also E1's bits 0-8): sets the mode's page, depth and blend. */
static void gpu_apply_texpage(u32 page) {
    g.texpage = (g.texpage & ~0x1FFu) | (page & 0x1FF);
}

static void gpu_mode_from_texpage(Mode *m) {
    m->tex_x = (g.texpage & 0xF) * 64;
    m->tex_y = ((g.texpage >> 4) & 1) * 256;
    m->abr = (g.texpage >> 5) & 3;
    if (m->textured) {
        int depth = (g.texpage >> 7) & 3;

        m->textured = depth == 0 ? 1 : depth == 1 ? 2 : 3;
    }
    m->u_and = 0xFF & ~g.tw_mask_x;
    m->u_or = g.tw_off_x & g.tw_mask_x;
    m->v_and = 0xFF & ~g.tw_mask_y;
    m->v_or = g.tw_off_y & g.tw_mask_y;
}

static inline s64 gpu_floor_div(s64 a, s64 b) {
    s64 q = a / b;

    if ((a % b != 0) && ((a < 0) != (b < 0))) {
        q--;
    }
    return q;
}

/* An attribute along a span (or an edge's x down the rows): value = base + floor(n / d), n stepping by a constant;
 * kept as a quotient q and a remainder 0 <= rem < d, so a step is two additions and a compare (no division per
 * pixel). Every quantity fits 32 bits: d <= 2 * 1023 * 511, |n| <= 2 * (1023 * 2 * 255 * 511 + 511 * 2 * 255 * 1023)
 * + d < 2^31 (the size limit bounds the vertex distances, attributes are 0..255); the setup multiplies in 64 bits. */
typedef struct {
    int q, rem;   /* floor(n / d), n - q * d */
    int sq, srem; /* the same for the step */
} Interp;

/* q and rem for n (d > 0): a 32-bit division when n fits (always, by the bound above), else a 64-bit one. */
static inline void gpu_interp_at(Interp *it, s64 n, int d) {
    if (n == (s32)n) {
        int q = (s32)n / d;
        int rem = (s32)n - q * d;

        if (rem < 0) {
            q--;
            rem += d;
        }
        it->q = q;
        it->rem = rem;
    } else {
        s64 q = gpu_floor_div(n, d);

        it->q = (int)q;
        it->rem = (int)(n - q * d);
    }
}

static inline void gpu_interp_start(Interp *it, s64 n, s64 step, int d) {
    Interp s;

    gpu_interp_at(it, n, d);
    gpu_interp_at(&s, step, d);
    it->sq = s.q;
    it->srem = s.rem;
}

GPU_INLINE void gpu_interp_step(Interp *it, int d) {
    it->q += it->sq;
    it->rem += it->srem;
    if (it->rem >= d) {
        it->rem -= d;
        it->q++;
    }
}

/* A span of a one-colour triangle: n pixels from dst, texture coordinates (au, av) + the interpolators iu, iv. */
GPU_INLINE void gpu_span_flat_tri(const Mode *m, u16 *dst, int n, Interp iu, Interp iv, int au, int av, int d, int r,
                                  int gg, int b, const int depth, const int blend) {
    const u16 *clut = &gpu_vram[m->clut_y * VRAM_W];
    const int tex_x = m->tex_x, tex_y = m->tex_y, clut_x = m->clut_x;
    const int u_and = m->u_and, u_or = m->u_or, v_and = m->v_and, v_or = m->v_or;
    const u16 set_mask = g.set_mask;
    int i;

    for (i = 0; i < n; i++) {
        u16 t = 0;

        if (depth) {
            int u = ((au + iu.q) & u_and) | u_or, v = ((av + iv.q) & v_and) | v_or;

            t = gpu_fetch(&gpu_vram[((tex_y + v) & 511) * VRAM_W], clut, tex_x, clut_x, u, depth);
            gpu_interp_step(&iu, d);
            gpu_interp_step(&iv, d);
            if (t == 0) {
                continue;
            }
        }
        gpu_shade_flat(&dst[i], t, r, gg, b, set_mask, depth != 0, blend);
    }
}

/* A span of a Gouraud triangle starting at (x, y): it[0..2] the colour's interpolators, it[3..4] the texture's.
 * Textured spans of the plain pipeline (not dithered, not semi-transparent) take their colour per pair of pixels. */
GPU_INLINE void gpu_span_gouraud_tri(const Mode *m, u16 *dst, int x, int y, int n, const Interp *it, const Vertex *a,
                                     int d, const int depth, const int eight, const int semi) {
    const u16 *clut = &gpu_vram[m->clut_y * VRAM_W];
    const int tex_x = m->tex_x, tex_y = m->tex_y, clut_x = m->clut_x;
    const int u_and = m->u_and, u_or = m->u_or, v_and = m->v_and, v_or = m->v_or;
    const int abr = m->abr;
    const u16 set_mask = g.set_mask;
    const s8 *dither = gpu_dither[y & 3];
    const int ar = a->r, ag = a->g, ab = a->b, au = a->u, av = a->v;
    Interp ir = it[0], ig = it[1], ib = it[2], iu = it[3], iv = it[4];
    int cr = ar, cg = ag, cb = ab;
    int i;

    for (i = 0; i < n; i++) {
        u16 t = 0;

        if (!(depth && !eight && !semi) || (i & 1) == 0) {
            cr = ar + ir.q;
            cg = ag + ig.q;
            cb = ab + ib.q;
        }
        gpu_interp_step(&ir, d);
        gpu_interp_step(&ig, d);
        gpu_interp_step(&ib, d);
        if (depth) {
            int u = ((au + iu.q) & u_and) | u_or, v = ((av + iv.q) & v_and) | v_or;

            t = gpu_fetch(&gpu_vram[((tex_y + v) & 511) * VRAM_W], clut, tex_x, clut_x, u, depth);
            gpu_interp_step(&iu, d);
            gpu_interp_step(&iv, d);
            if (t == 0) {
                continue;
            }
        }
        gpu_shade_gouraud(&dst[i], eight ? dither[(x + i) & 3] : 0, t, cr, cg, cb, abr, set_mask, depth != 0, eight,
                          semi);
    }
}

/* A row of a one-colour rectangle: n pixels from dst, the texture row `row` (v applied) from u (not windowed). */
GPU_INLINE void gpu_span_sprite(const Mode *m, u16 *dst, int n, const u16 *row, int u, int r, int gg, int b,
                                const int depth, const int blend) {
    const u16 *clut = &gpu_vram[m->clut_y * VRAM_W];
    const int tex_x = m->tex_x, clut_x = m->clut_x;
    const int u_and = m->u_and, u_or = m->u_or;
    const u16 set_mask = g.set_mask;
    int i;

    for (i = 0; i < n; i++) {
        u16 t = 0;

        if (depth) {
            t = gpu_fetch(row, clut, tex_x, clut_x, ((u + i) & u_and) | u_or, depth);
            if (t == 0) {
                continue;
            }
        }
        gpu_shade_flat(&dst[i], t, r, gg, b, set_mask, depth != 0, blend);
    }
}

/* The instances: one case per (depth, blend) of a one-colour primitive, per (depth, eight, semi) of a Gouraud one. */
#define GPU_FLAT_KIND(depth, blend) ((depth) * 8 + (blend) + 2)
#define GPU_FLAT_CASES(CASE)                                                                                         \
    CASE(0, 0) CASE(0, 1) CASE(0, 2) CASE(0, 3)                                                                      \
    CASE(1, GPU_COPY) CASE(1, GPU_OPAQUE) CASE(1, 0) CASE(1, 1) CASE(1, 2) CASE(1, 3)                                \
    CASE(2, GPU_COPY) CASE(2, GPU_OPAQUE) CASE(2, 0) CASE(2, 1) CASE(2, 2) CASE(2, 3)                                \
    CASE(3, GPU_COPY) CASE(3, GPU_OPAQUE) CASE(3, 0) CASE(3, 1) CASE(3, 2) CASE(3, 3)

static void gpu_flat_tri(int kind, const Mode *m, u16 *dst, int n, const Interp *it, int au, int av, int d, int r,
                         int gg, int b) {
    switch (kind) {
#define CASE(depth, blend)                                                                                           \
    case GPU_FLAT_KIND(depth, blend):                                                                                \
        gpu_span_flat_tri(m, dst, n, it[3], it[4], au, av, d, r, gg, b, depth, blend);                               \
        return;
        GPU_FLAT_CASES(CASE)
#undef CASE
    }
}

static void gpu_sprite(int kind, const Mode *m, u16 *dst, int n, const u16 *row, int u, int r, int gg, int b) {
    switch (kind) {
#define CASE(depth, blend)                                                                                           \
    case GPU_FLAT_KIND(depth, blend):                                                                                \
        gpu_span_sprite(m, dst, n, row, u, r, gg, b, depth, blend);                                                  \
        return;
        GPU_FLAT_CASES(CASE)
#undef CASE
    }
}

/* Decoded texture segments: 64 texels of a 4- or 8-bit texture row through a CLUT, kept while the VRAM they come from
 * is unchanged (the write stamps). A segment has its place by texture row, page column and position in the row (the
 * texture's place in the VRAM), and holds the depth and CLUT it was decoded with. Sprites draw from them: most of the
 * game's sprites are the same texels every frame (the field's backgrounds, the fonts). Invisible: a segment is used
 * only where decoding it again would give the same texels. */
#define GPU_SEG 64
typedef struct {
    u32 key; /* 0: empty; else 1 | depth << 1 | CLUT column / 16 << 3 | CLUT row << 9 */
    u32 stamp;
    u16 texel[GPU_SEG];
} Segment;
static Segment gpu_segs[VRAM_H * 16 * (256 / GPU_SEG)];

/* Forgets every decoded segment (when the stamps wrap around). */
static void gpu_segs_reset(void) {
    size_t i;

    for (i = 0; i < sizeof(gpu_segs) / sizeof(gpu_segs[0]); i++) {
        gpu_segs[i].key = 0;
    }
    memset(gpu_block_stamp, 0, sizeof(gpu_block_stamp));
    gpu_stamp = 1;
    gpu_generation++;
}

/* Whether pixels x .. x + n - 1 of row y are unchanged since `stamp`. */
static inline int gpu_unchanged(int y, int x, int n, u32 stamp) {
    const u32 *b = gpu_block_stamp[y];
    int i;

    for (i = x >> 6; i <= (x + n - 1) >> 6; i++) {
        if (b[i] > stamp) {
            return 0;
        }
    }
    return 1;
}

static int gpu_decode_row(u16 *out, const Mode *m, const u16 *row, int u, int n);

/* Texels GPU_SEG * k .. GPU_SEG * (k + 1) - 1 of texture row tv of the mode's page (4- or 8-bit; the caller checks that
 * neither these texture words nor the CLUT wrap at the VRAM's edge). */
static const u16 *gpu_decoded(const Mode *m, int tv, int k) {
    int words = m->textured == 1 ? GPU_SEG / 4 : GPU_SEG / 2;
    int entries = m->textured == 1 ? 16 : 256;
    u32 key = 1u | ((u32)m->textured << 1) | ((u32)(m->clut_x >> 4) << 3) | ((u32)m->clut_y << 9);
    Segment *e = &gpu_segs[(tv * 16 + (m->tex_x >> 6)) * (256 / GPU_SEG) + k];

    if (e->key == key && gpu_unchanged(tv, m->tex_x + k * words, words, e->stamp)
        && gpu_unchanged(m->clut_y, m->clut_x, entries, e->stamp)) {
        return e->texel;
    }
    if (gpu_stamp == 0xFFFFFFFFu) {
        gpu_segs_reset();
    }
    gpu_decode_row(e->texel, m, &gpu_vram[tv * VRAM_W], k * GPU_SEG, GPU_SEG);
    e->key = key;
    e->stamp = gpu_stamp++;
    return e->texel;
}

/* A sprite row's texels decoded ahead into `out` (gpu_rectangle checks that the row it draws is neither the texture row
 * nor the CLUT's, so that drawing them cannot change what is read): only for the common case, a plain texture window
 * and no wrapping (u, the texture's VRAM columns and the CLUT stay in range), whole texture words at a time. Returns 0
 * when the row is not that case. */
static int gpu_decode_row(u16 *out, const Mode *m, const u16 *row, int u, int n) {
    const u16 *src = row + m->tex_x;
    const u16 *clut = &gpu_vram[m->clut_y * VRAM_W + m->clut_x];
    int i = 0;

    if (m->u_and != 0xFF || m->u_or != 0 || u + n > 256) {
        return 0;
    }
    switch (m->textured) {
    case 1:
        if (m->tex_x + ((u + n - 1) >> 2) > 1023) {
            return 0;
        }
        for (; i < n && ((u + i) & 3) != 0; i++) {
            out[i] = clut[(src[(u + i) >> 2] >> (((u + i) & 3) * 4)) & 0xF];
        }
        for (; i + 4 <= n; i += 4) {
            u16 w = src[(u + i) >> 2];

            out[i] = clut[w & 0xF];
            out[i + 1] = clut[(w >> 4) & 0xF];
            out[i + 2] = clut[(w >> 8) & 0xF];
            out[i + 3] = clut[w >> 12];
        }
        for (; i < n; i++) {
            out[i] = clut[(src[(u + i) >> 2] >> (((u + i) & 3) * 4)) & 0xF];
        }
        return 1;
    case 2:
        if (m->tex_x + ((u + n - 1) >> 1) > 1023 || m->clut_x + 255 > 1023) {
            return 0;
        }
        if (u & 1) {
            out[i++] = clut[src[u >> 1] >> 8];
        }
        for (; i + 2 <= n; i += 2) {
            u16 w = src[(u + i) >> 1];

            out[i] = clut[w & 0xFF];
            out[i + 1] = clut[w >> 8];
        }
        if (i < n) {
            out[i] = clut[src[(u + i) >> 1] & 0xFF];
        }
        return 1;
    default:
        if (m->tex_x + u + n - 1 > 1023) {
            return 0;
        }
        memcpy(out, src + u, (size_t)n * 2);
        return 1;
    }
}

/* The four blend modes on whole 15-bit pixels (a the background, f the foreground, bit 15 clear in both), for a
 * foreground that is a texel as it is (raw, or modulated by 128, so every channel is 0..31): the same as gpu_blend per
 * channel, five bits at a time in one integer. avg is floor((a + f) / 2) per channel (the shift masked so that no bit
 * crosses into the channel below); a channel's sum overflows 31 exactly when its average has bit 4 set, and a - f is
 * negative exactly when the average of a and 31 - f has bit 4 clear; such channels become 31 (add) or 0 (subtract). */
GPU_INLINE u32 gpu_avg15(u32 a, u32 f) {
    return (a & f) + (((a ^ f) & 0x7BDE) >> 1);
}

GPU_INLINE u32 gpu_add15(u32 a, u32 f) {
    u32 over = gpu_avg15(a, f) & 0x4210;

    return ((a + f) - (over << 1)) | ((over << 1) - (over >> 4));
}

GPU_INLINE u32 gpu_blend15(u32 a, u32 f, const int abr) {
    switch (abr) {
    case 0:
        return gpu_avg15(a, f);
    case 1:
        return gpu_add15(a, f);
    case 2: {
        u32 under = ~gpu_avg15(a, f ^ 0x7FFF) & 0x4210;
        u32 keep = ~((under << 1) - (under >> 4));

        return (a & keep) - (f & keep);
    }
    default:
        return gpu_add15(a, (f >> 2) & 0x1CE7);
    }
}

/* A sprite row from decoded texels, as they are (the colour 128), semi-transparent in mode abr: written as a select,
 * which compilers vectorise. */
GPU_INLINE void gpu_span_texels_plain(u16 *dst, int n, const u16 *t, const int abr) {
    const u16 set_mask = g.set_mask;
    int i;

    for (i = 0; i < n; i++) {
        u32 tx = t[i], d = dst[i];
        u32 out = (tx & 0x8000) ? gpu_blend15(d & 0x7FFF, tx & 0x7FFF, abr) | 0x8000 : tx;

        dst[i] = tx != 0 ? (u16)(out | set_mask) : (u16)d;
    }
}

/* A sprite row from decoded texels. */
GPU_INLINE void gpu_span_texels(u16 *dst, int n, const u16 *t, int r, int gg, int b, const int blend) {
    const u16 set_mask = g.set_mask;
    int i;

    if (blend == GPU_COPY) {
        /* Written as a select (the same value back where the texel is 0): compilers vectorise it. */
        for (i = 0; i < n; i++) {
            dst[i] = t[i] != 0 ? (u16)(t[i] | set_mask) : dst[i];
        }
        return;
    }
    for (i = 0; i < n; i++) {
        if (t[i] != 0) {
            gpu_shade_flat(&dst[i], t[i], r, gg, b, set_mask, 1, blend);
        }
    }
}

static void gpu_sprite_texels(int kind, u16 *dst, int n, const u16 *t, int r, int gg, int b) {
    if (r == 128 && gg == 128 && b == 128) {
        switch ((kind & 7) - 2) {
        case 0:
            gpu_span_texels_plain(dst, n, t, 0);
            return;
        case 1:
            gpu_span_texels_plain(dst, n, t, 1);
            return;
        case 2:
            gpu_span_texels_plain(dst, n, t, 2);
            return;
        case 3:
            gpu_span_texels_plain(dst, n, t, 3);
            return;
        }
    }
    switch ((kind & 7) - 2) {
    case GPU_COPY:
        gpu_span_texels(dst, n, t, r, gg, b, GPU_COPY);
        return;
    case GPU_OPAQUE:
        gpu_span_texels(dst, n, t, r, gg, b, GPU_OPAQUE);
        return;
    case 0:
        gpu_span_texels(dst, n, t, r, gg, b, 0);
        return;
    case 1:
        gpu_span_texels(dst, n, t, r, gg, b, 1);
        return;
    case 2:
        gpu_span_texels(dst, n, t, r, gg, b, 2);
        return;
    default:
        gpu_span_texels(dst, n, t, r, gg, b, 3);
        return;
    }
}

/* A sprite row of n pixels from u (texture row tv) drawn from texels read ahead: decoded segments, the VRAM row itself
 * (15-bit), or decoded into buf. Only the plain case (gpu_decode_row); returns 0, having drawn nothing, otherwise.
 * gpu_rectangle calls it only for rows that are neither tv nor the CLUT's, so that drawing the row cannot change what
 * it reads. */
static int gpu_sprite_ahead(int kind, const Mode *m, u16 *dst, int tv, int u, int n, int r, int gg, int b, u16 *buf) {
    const u16 *row = &gpu_vram[tv * VRAM_W];
    int shift = m->textured == 1 ? 2 : 1;
    int i, len;

    if (m->u_and != 0xFF || m->u_or != 0 || u + n > 256) {
        return 0;
    }
    if (m->textured == 3) {
        if (m->tex_x + u + n > VRAM_W) {
            return 0;
        }
        gpu_sprite_texels(kind, dst, n, row + m->tex_x + u, r, gg, b);
        return 1;
    }
    if (m->clut_x + (m->textured == 1 ? 16 : 256) <= VRAM_W
        && m->tex_x + ((((u + n - 1) / GPU_SEG) + 1) * GPU_SEG >> shift) <= VRAM_W) {
        for (i = 0; i < n; i += len) {
            const u16 *t = gpu_decoded(m, tv, (u + i) / GPU_SEG);
            int at = (u + i) % GPU_SEG;

            len = GPU_SEG - at < n - i ? GPU_SEG - at : n - i;
            gpu_sprite_texels(kind, dst + i, len, t + at, r, gg, b);
        }
        return 1;
    }
    if (!gpu_decode_row(buf, m, row, u, n)) {
        return 0;
    }
    gpu_sprite_texels(kind, dst, n, buf, r, gg, b);
    return 1;
}

/* The kind of a one-colour primitive's spans (untextured opaque ones are fills, mask-checked ones gpu_pixel's). */
static int gpu_flat_kind(const Mode *m, int r, int gg, int b) {
    int blend = m->semi ? m->abr : (m->textured && r == 128 && gg == 128 && b == 128) ? GPU_COPY : GPU_OPAQUE;

    return GPU_FLAT_KIND(m->textured, blend);
}

#define GPU_GOURAUD_KIND(depth, eight, semi) ((depth) * 4 + (eight) * 2 + (semi))
static void gpu_gouraud_tri(int kind, const Mode *m, u16 *dst, int x, int y, int n, const Interp *it, const Vertex *a,
                            int d) {
    switch (kind) {
#define CASE(depth, eight, semi)                                                                                     \
    case GPU_GOURAUD_KIND(depth, eight, semi):                                                                       \
        gpu_span_gouraud_tri(m, dst, x, y, n, it, a, d, depth, eight, semi);                                         \
        return;
#define CASES(depth) CASE(depth, 0, 0) CASE(depth, 0, 1) CASE(depth, 1, 0) CASE(depth, 1, 1)
        CASES(0) CASES(1) CASES(2) CASES(3)
#undef CASES
#undef CASE
    }
}

/* One triangle: every pixel (x, y) inside the edges, the left edges and the top edge included (sampled at integer
 * coordinates; the emulator's coverage exactly), each attribute from the plane through the three vertices, rounded to
 * the nearest integer. An exact .5 rounds up here; the emulator's fixed-point steps round it either way (the one rule
 * not reproduced: golden family gpu, uv_ramp_* and uv_axis_*). Gouraud-textured spans that are not dithered, not
 * semi-transparent and not mask-checked take their colour per pair of pixels from the span's start, the first pixel's
 * for both (the emulator draws them two at a time).
 * The edges are walked a row at a time (x = ceil(num / dy), num growing by dx per row: no division per row); each span
 * starts its attributes with one division and steps them per pixel. */
static void gpu_triangle(const Mode *m, const Vertex *a, const Vertex *b, const Vertex *c, int gouraud) {
    const Vertex *v[3] = { a, b, c };
    const Vertex *t;
    const Vertex *s0 = NULL;
    s64 den, n[5][2], lnum, snum = 0;
    int d2, lden, ldx, sden = 0, sdx = 0;
    int nattr, pairs, first, kind, seg;
    int y, ymin, ymax, i;
    int dx1, dy1, dx2, dy2;
    Interp it[5], el, es;

    /* The size limit: no edge longer than 1023 horizontally or 511 vertically. */
    if (abs(a->x - b->x) > 1023 || abs(a->x - c->x) > 1023 || abs(b->x - c->x) > 1023 || abs(a->y - b->y) > 511
        || abs(a->y - c->y) > 511 || abs(b->y - c->y) > 511) {
        return;
    }
    /* Sort by y. */
    if (v[1]->y < v[0]->y) {
        t = v[0], v[0] = v[1], v[1] = t;
    }
    if (v[2]->y < v[1]->y) {
        t = v[1], v[1] = v[2], v[2] = t;
    }
    if (v[1]->y < v[0]->y) {
        t = v[0], v[0] = v[1], v[1] = t;
    }
    dx1 = b->x - a->x;
    dy1 = b->y - a->y;
    dx2 = c->x - a->x;
    dy2 = c->y - a->y;
    den = (s64)dx1 * dy2 - (s64)dx2 * dy1;
    if (den == 0) {
        return;
    }
    if (gpu_listener != NULL) {
        GpuEvent ev;
        gpu_event_mode(&ev, GPU_EV_TRIANGLE, m);
        ev.gouraud = gouraud;
        ev.v[0] = *a;
        ev.v[1] = *b;
        ev.v[2] = *c;
        gpu_listener(&ev);
    }
    /* The planes: attribute(x, y) = attribute(a) + ((x - a.x) * n[0] + (y - a.y) * n[1]) / den; attributes 0..2
     * the colour (Gouraud), 3..4 the texture coordinates. */
#define PLANE(k, f)                                                                    \
    do {                                                                               \
        s64 e1 = b->f - a->f, e2 = c->f - a->f;                                        \
        n[k][0] = e1 * dy2 - e2 * dy1;                                                 \
        n[k][1] = e2 * dx1 - e1 * dx2;                                                 \
    } while (0)
    PLANE(0, r);
    PLANE(1, g);
    PLANE(2, b);
    PLANE(3, u);
    PLANE(4, v);
#undef PLANE
    if (den < 0) {
        den = -den;
        for (i = 0; i < 5; i++) {
            n[i][0] = -n[i][0];
            n[i][1] = -n[i][1];
        }
    }
    d2 = (int)(2 * den);
    nattr = gouraud ? (m->textured ? 5 : 3) : (m->textured ? 5 : 0);
    first = gouraud ? 0 : 3;
    pairs = gouraud && m->textured && !m->dither && !m->semi && !g.check_mask;
    memset(it, 0, sizeof(it));
    for (i = first; i < nattr; i++) {
        gpu_interp_start(&it[i], 0, 2 * n[i][0], d2);
    }
    kind = gouraud ? GPU_GOURAUD_KIND(m->textured, m->eight, m->semi) : gpu_flat_kind(m, a->r, a->g, a->b);
    ymin = v[0]->y;
    ymax = v[2]->y;
    if (ymin < g.area_y0) {
        ymin = g.area_y0;
    }
    if (ymax > g.area_y1 + 1) {
        ymax = g.area_y1 + 1;
    }
    if (ymin >= ymax) {
        return;
    }
    /* The long edge v0-v2 and the short edge (v0-v1 above v1, v1-v2 from it), stepped from ymin. */
    lden = v[2]->y - v[0]->y;
    ldx = v[2]->x - v[0]->x;
    lnum = (s64)v[0]->x * lden + (s64)(ymin - v[0]->y) * ldx;
    gpu_interp_start(&el, -lnum, -ldx, lden);
    seg = -1;
    for (y = ymin; y < ymax; y++) {
        int upper = y < v[1]->y;
        s64 xl, xr, x0, x1;
        int x, xs, xe;
        int cr = a->r, cg = a->g, cb = a->b;

        if (upper != seg) {
            const Vertex *s1 = upper ? v[1] : v[2];

            s0 = upper ? v[0] : v[1];
            sden = s1->y - s0->y;
            sdx = s1->x - s0->x;
            snum = (s64)s0->x * sden + (s64)(y - s0->y) * sdx;
            if (sden != 0) {
                gpu_interp_start(&es, -snum, -sdx, sden);
            }
            seg = upper;
        }
        if (sden == 0) {
            goto next;
        }
        x0 = -el.q; /* ceil(lnum / lden) */
        x1 = -es.q; /* ceil(snum / sden) */
        /* Which side the long edge is on: compare the edges' positions (as fractions) at this row. */
        if (lnum * sden < snum * lden) {
            xl = x0, xr = x1;
        } else {
            xl = x1, xr = x0;
        }
        xs = xl < g.area_x0 ? g.area_x0 : (int)xl;
        xe = xr > g.area_x1 + 1 ? g.area_x1 + 1 : (int)xr;
        if (xs >= xe) {
            goto next;
        }
        if (nattr == 0 && !m->semi && !g.check_mask) {
            gpu_span_fill(y, xs, xe, gpu_flat_colour((u32)a->r | ((u32)a->g << 8) | ((u32)a->b << 16)));
            goto next;
        }
        for (i = first; i < nattr; i++) {
            gpu_interp_at(&it[i], 2 * ((s64)(xs - a->x) * n[i][0] + (s64)(y - a->y) * n[i][1]) + den, d2);
        }
        gpu_touch(y, xs, xe);
        if (!g.check_mask) {
            if (gouraud) {
                gpu_gouraud_tri(kind, m, &gpu_vram[y * VRAM_W + xs], xs, y, xe - xs, it, a, d2);
            } else {
                gpu_flat_tri(kind, m, &gpu_vram[y * VRAM_W + xs], xe - xs, it, a->u, a->v, d2, a->r, a->g, a->b);
            }
            goto next;
        }
        for (x = xs; x < xe; x++) {
            int u = 0, vv = 0;

            if (gouraud && (!pairs || ((x - xs) & 1) == 0)) {
                cr = a->r + it[0].q;
                cg = a->g + it[1].q;
                cb = a->b + it[2].q;
            }
            if (m->textured) {
                u = a->u + it[3].q;
                vv = a->v + it[4].q;
            }
            gpu_pixel(m, x, y, cr, cg, cb, u, vv);
            for (i = first; i < nattr; i++) {
                gpu_interp_step(&it[i], d2);
            }
        }
    next:
        lnum += ldx;
        snum += sdx;
        gpu_interp_step(&el, lden);
        if (sden != 0) {
            gpu_interp_step(&es, sden);
        }
    }
}

/* GP0 20h-3Fh. */
static void gpu_polygon(const u32 *w) {
    u32 code = w[0] >> 24;
    int gouraud = (code & 0x10) != 0;
    int quad = (code & 0x08) != 0;
    int textured = (code & 0x04) != 0;
    int nverts = quad ? 4 : 3;
    Vertex vx[4];
    Mode m;
    int i, k = 0;

    memset(&m, 0, sizeof(m));
    m.semi = (code & 2) != 0;
    m.raw = (code & 1) != 0;
    m.textured = textured;
    for (i = 0; i < nverts; i++) {
        u32 col = (i == 0 || gouraud) ? w[k++] : w[0];

        vx[i].r = col & 0xFF;
        vx[i].g = (col >> 8) & 0xFF;
        vx[i].b = (col >> 16) & 0xFF;
        if (textured && m.raw) {
            /* Raw texels: the colour 128, which gives the same pixels (the span loops' comment). */
            vx[i].r = vx[i].g = vx[i].b = 128;
        }
        vx[i].x = sext11((u32)(sext11(w[k]) + g.ofs_x));
        vx[i].y = sext11((u32)(sext11(w[k] >> 16) + g.ofs_y));
        k++;
        vx[i].u = vx[i].v = 0;
        if (textured) {
            vx[i].u = w[k] & 0xFF;
            vx[i].v = (w[k] >> 8) & 0xFF;
            if (i == 0) {
                m.clut_x = ((w[k] >> 16) & 0x3F) * 16;
                m.clut_y = (w[k] >> 22) & 0x1FF;
            } else if (i == 1) {
                gpu_apply_texpage(w[k] >> 16);
            }
            k++;
        }
    }
    gpu_mode_from_texpage(&m);
    /* Dithering applies to Gouraud primitives only (golden family gpu: dither_tex_*, dither_gouraud_*, line_g2_*). */
    m.dither = g.dither && gouraud;
    m.eight = m.dither;
    m.gouraud = gouraud;
    /* A quad is the triangles (1, 2, 3) and (0, 1, 2), in that order: where they overlap the second one shows (the
     * emulator's; golden family gpu, uv_ramp_* quads). */
    if (quad) {
        gpu_triangle(&m, &vx[1], &vx[2], &vx[3], gouraud);
    }
    gpu_triangle(&m, &vx[0], &vx[1], &vx[2], gouraud);
}

/* GP0 60h-7Fh: a rectangle (a sprite when textured): the size from the command or a fixed 1, 8 or 16. */
static void gpu_rectangle(const u32 *w) {
    u32 code = w[0] >> 24;
    int textured = (code & 4) != 0;
    int size = (code >> 3) & 3;
    int x0 = sext11((u32)(sext11(w[1]) + g.ofs_x));
    int y0 = sext11((u32)(sext11(w[1] >> 16) + g.ofs_y));
    int u0 = 0, v0 = 0;
    int r = w[0] & 0xFF, gg = (w[0] >> 8) & 0xFF, b = (w[0] >> 16) & 0xFF;
    int width, height, kind;
    int x, y, xs, xe, ys, ye;
    u16 texels[1024];
    Mode m;

    memset(&m, 0, sizeof(m));
    m.semi = (code & 2) != 0;
    m.raw = (code & 1) != 0;
    m.textured = textured;
    if (textured) {
        u0 = w[2] & 0xFF;
        v0 = (w[2] >> 8) & 0xFF;
        m.clut_x = ((w[2] >> 16) & 0x3F) * 16;
        m.clut_y = (w[2] >> 22) & 0x1FF;
        if (m.raw) {
            r = gg = b = 128; /* the same pixels as raw (the span loops' comment) */
        }
    }
    if (size == 0) {
        u32 wh = w[textured ? 3 : 2];

        width = wh & 0x3FF;
        height = (wh >> 16) & 0x1FF;
    } else {
        width = height = size == 1 ? 1 : size == 2 ? 8 : 16;
    }
    gpu_mode_from_texpage(&m);
    m.dither = 0;
    if (gpu_listener != NULL) {
        GpuEvent ev;
        gpu_event_mode(&ev, GPU_EV_RECT, &m);
        ev.v[0].x = x0;
        ev.v[0].y = y0;
        ev.v[0].r = r;
        ev.v[0].g = gg;
        ev.v[0].b = b;
        ev.v[0].u = u0;
        ev.v[0].v = v0;
        ev.x = x0;
        ev.y = y0;
        ev.w = width;
        ev.h = height;
        gpu_listener(&ev);
    }
    xs = x0 < g.area_x0 ? g.area_x0 : x0;
    ys = y0 < g.area_y0 ? g.area_y0 : y0;
    xe = x0 + width > g.area_x1 + 1 ? g.area_x1 + 1 : x0 + width;
    ye = y0 + height > g.area_y1 + 1 ? g.area_y1 + 1 : y0 + height;
    if (xs >= xe) {
        return;
    }
    if (!textured && !m.semi && !g.check_mask) {
        u16 c = gpu_flat_colour(w[0]);

        for (y = ys; y < ye; y++) {
            gpu_span_fill(y, xs, xe, c);
        }
        return;
    }
    if (g.check_mask) {
        for (y = ys; y < ye; y++) {
            for (x = xs; x < xe; x++) {
                gpu_pixel(&m, x, y, r, gg, b, u0 + (x - x0), v0 + (y - y0));
            }
            gpu_touch(y, xs, xe);
        }
        return;
    }
    kind = gpu_flat_kind(&m, r, gg, b);
    for (y = ys; y < ye; y++) {
        int tv = (m.tex_y + (((v0 + (y - y0)) & m.v_and) | m.v_or)) & 511;
        const u16 *row = &gpu_vram[tv * VRAM_W];
        u16 *dst = &gpu_vram[y * VRAM_W + xs];
        int u = u0 + (xs - x0);

        if (!textured || y == tv || y == m.clut_y
            || !gpu_sprite_ahead(kind, &m, dst, tv, u, xe - xs, r, gg, b, texels)) {
            gpu_sprite(kind, &m, dst, xe - xs, row, u, r, gg, b);
        }
        gpu_touch(y, xs, xe);
    }
}

/* One line segment (flat or Gouraud), both ends included. The emulator's stepping, measured (golden family gpu,
 * line_probe_*): along the major axis from the left end (x-major, |dx| >= |dy|) or the top end (y-major), the minor
 * coordinate moves by floor((|minor| * i + c) / major) after i steps, with c = (major + |minor| - 1) / 2 for x-major
 * segments, (major - 1) / 2 for y-major ones going right and major / 2 going left. A sloped segment leaves out the
 * draw area's last column and row (x < x1, y < y1); a horizontal or vertical one reaches them. */
void gpu_segment_walk(const GpuVertex *a, const GpuVertex *b, int gouraud,
                      void (*pixel)(void *ctx, int x, int y, int r, int g, int b), void *ctx) {
    const Vertex *a0 = a, *b0 = b;
    int dx = b->x - a->x, dy = b->y - a->y;
    int adx = abs(dx), ady = abs(dy);
    int xmajor = adx >= ady;
    int major, minor, c, i, sx, sy;
    int xlim = g.area_x1, ylim = g.area_y1;

    if (adx > 1023 || ady > 511) {
        return;
    }
    const Vertex *ca = a, *cb = b;

    if ((xmajor && dx < 0) || (!xmajor && dy < 0)) {
        const Vertex *t = a;

        a = b;
        b = t;
        dx = -dx;
        dy = -dy;
    }
    if (dx != 0 && dy != 0) {
        xlim--;
        ylim--;
    }
    /* The colour at the drawing's start (measured, line_gprobe_*): the first vertex's for x-major segments and for
     * y-major ones going right (or straight down), the second vertex's for y-major ones going left; it runs to the
     * other vertex's colour at the end, whichever end is which. */
    if (!xmajor && dx < 0) {
        ca = b0;
        cb = a0;
    } else {
        ca = a0;
        cb = b0;
    }
    sx = dx < 0 ? -1 : 1;
    sy = dy < 0 ? -1 : 1;
    if (xmajor) {
        major = adx;
        minor = ady;
        c = (major + minor - 1) >> 1;
    } else {
        major = ady;
        minor = adx;
        c = dx > 0 ? (major - 1) >> 1 : major >> 1;
    }
    for (i = 0; i <= major; i++) {
        int off = major == 0 ? 0 : (minor * i + c) / major;
        int x = xmajor ? a->x + i : a->x + sx * off;
        int y = xmajor ? a->y + sy * off : a->y + i;
        int r = ca->r, gg = ca->g, bb = ca->b;

        if (gouraud && major != 0) {
            r += (int)gpu_floor_div((s64)(cb->r - ca->r) * i, major);
            gg += (int)gpu_floor_div((s64)(cb->g - ca->g) * i, major);
            bb += (int)gpu_floor_div((s64)(cb->b - ca->b) * i, major);
        }
        if (x < g.area_x0 || x > xlim || y < g.area_y0 || y > ylim) {
            continue;
        }
        pixel(ctx, x, y, r, gg, bb);
    }
}

static void gpu_segment_pixel(void *ctx, int x, int y, int r, int gg, int b) {
    gpu_pixel((const Mode *)ctx, x, y, r, gg, b, 0, 0);
    gpu_touch(y, x, x + 1);
}

static void gpu_segment(const Mode *m, const Vertex *a, const Vertex *b, int gouraud) {
    if (gpu_listener != NULL) {
        GpuEvent ev;
        gpu_event_mode(&ev, GPU_EV_SEGMENT, m);
        ev.gouraud = gouraud;
        ev.v[0] = *a;
        ev.v[1] = *b;
        gpu_listener(&ev);
    }
    gpu_segment_walk(a, b, gouraud, gpu_segment_pixel, (void *)m);
}

static void gpu_line_vertex(Vertex *v, u32 col, u32 pos) {
    v->r = col & 0xFF;
    v->g = (col >> 8) & 0xFF;
    v->b = (col >> 16) & 0xFF;
    v->x = sext11((u32)(sext11(pos) + g.ofs_x));
    v->y = sext11((u32)(sext11(pos >> 16) + g.ofs_y));
    v->u = v->v = 0;
}

static void gpu_line_mode(Mode *m, u32 code) {
    memset(m, 0, sizeof(*m));
    m->semi = (code & 2) != 0;
    gpu_mode_from_texpage(m);
    m->dither = 0;
}

/* GP0 02h: fills a rectangle with a colour: x rounded down and the width up to 16 pixels, no mask, no area. */
static void gpu_fill(const u32 *w) {
    u16 c = (u16)(((w[0] >> 3) & 0x1F) | (((w[0] >> 11) & 0x1F) << 5) | (((w[0] >> 19) & 0x1F) << 10));
    int x0 = w[1] & 0x3F0;
    int y0 = (w[1] >> 16) & 0x1FF;
    int width = ((w[2] & 0x3FF) + 0xF) & ~0xF;
    int height = (w[2] >> 16) & 0x1FF;
    int x, y;

    if (gpu_listener != NULL) {
        GpuEvent ev;
        memset(&ev, 0, sizeof(ev));
        ev.kind = GPU_EV_FILL;
        ev.x = x0;
        ev.y = y0;
        ev.w = width;
        ev.h = height;
        ev.color = c;
        gpu_listener(&ev);
    }
    for (y = 0; y < height; y++) {
        u16 *row = &gpu_vram[((y0 + y) & 511) * VRAM_W];

        for (x = 0; x < width; x++) {
            row[(x0 + x) & 1023] = c;
        }
        gpu_touch((y0 + y) & 511, 0, VRAM_W);
    }
}

/* GP0 80h: VRAM to VRAM, pixel by pixel, wrapping at the edges, with the mask rules. */
static void gpu_copy(const u32 *w) {
    int sx = w[1] & 0x3FF, sy = (w[1] >> 16) & 0x1FF;
    int dx = w[2] & 0x3FF, dy = (w[2] >> 16) & 0x1FF;
    int width = (((w[3] & 0xFFFF) - 1) & 0x3FF) + 1;
    int height = ((((w[3] >> 16) & 0xFFFF) - 1) & 0x1FF) + 1;
    int x, y;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            u16 p = gpu_vram[((sy + y) & 511) * VRAM_W + ((sx + x) & 1023)];
            u16 *d = &gpu_vram[((dy + y) & 511) * VRAM_W + ((dx + x) & 1023)];

            if (g.check_mask && (*d & 0x8000)) {
                continue;
            }
            *d = p | g.set_mask;
        }
        gpu_touch((dy + y) & 511, 0, VRAM_W);
    }
    if (gpu_listener != NULL) {
        GpuEvent ev;
        memset(&ev, 0, sizeof(ev));
        ev.kind = GPU_EV_COPY;
        ev.sx = sx;
        ev.sy = sy;
        ev.x = dx;
        ev.y = dy;
        ev.w = width;
        ev.h = height;
        ev.set_mask = g.set_mask;
        ev.check_mask = g.check_mask;
        gpu_listener(&ev);
    }
}

/* A CPU-to-VRAM transfer finished: the listener's event. */
static void gpu_load_done(void) {
    if (gpu_listener != NULL) {
        GpuEvent ev;
        memset(&ev, 0, sizeof(ev));
        ev.kind = GPU_EV_LOAD;
        ev.x = g.load_x;
        ev.y = g.load_y;
        ev.w = g.load_w;
        ev.h = g.load_h;
        ev.set_mask = g.set_mask;
        ev.check_mask = g.check_mask;
        gpu_listener(&ev);
    }
}

static void gpu_load_pixel(u16 p) {
    int x = (g.load_x + g.load_i % g.load_w) & 1023;
    int y = (g.load_y + g.load_i / g.load_w) & 511;
    u16 *d = &gpu_vram[y * VRAM_W + x];

    g.load_i++;
    g.load_left--;
    if (!g.check_mask || !(*d & 0x8000)) {
        *d = p | g.set_mask;
        gpu_touch(y, x, x + 1);
    }
    if (g.load_left == 0) {
        gpu_load_done();
    }
}

/* The words a command takes (its first word included); 0 for a variable-length one handled apart. */
static int gpu_command_length(u32 code) {
    switch (code >> 5) {
    case 1: { /* polygons */
        int verts = (code & 8) ? 4 : 3;
        int per = 1 + ((code & 4) ? 1 : 0) + ((code & 0x10) ? 1 : 0);

        return 1 + verts * per - ((code & 0x10) ? 1 : 0);
    }
    case 2: /* lines: a polyline is open-ended */
        return (code & 0x10) ? 4 : 3;
    case 3: { /* rectangles */
        int n = 2 + ((code & 4) ? 1 : 0);

        return ((code >> 3) & 3) == 0 ? n + 1 : n;
    }
    case 4: /* VRAM to VRAM */
        return 4;
    case 5: /* CPU to VRAM */
    case 6: /* VRAM to CPU */
        return 3;
    default:
        return code == 0x02 ? 3 : 1;
    }
}

static void gpu_execute(void) {
    const u32 *w = g.cmd;
    u32 code = w[0] >> 24;

    switch (code >> 5) {
    case 1:
        gpu_polygon(w);
        return;
    case 2: {
        Mode m;
        Vertex a, b;
        int gour = (code & 0x10) != 0;

        gpu_line_mode(&m, code);
        gpu_line_vertex(&a, w[0], w[1]);
        gpu_line_vertex(&b, gour ? w[2] : w[0], gour ? w[3] : w[2]);
        gpu_segment(&m, &a, &b, gour);
        if (code & 8) {
            /* A polyline: keep the last vertex (and its colour) and wait for more. */
            g.polyline = 1;
            if (gour) {
                g.cmd[0] = (w[0] & 0xFF000000u) | (w[2] & 0xFFFFFF);
                g.cmd[1] = w[3];
            } else {
                g.cmd[1] = w[2];
            }
            g.n = 2;
        }
        return;
    }
    case 3:
        gpu_rectangle(w);
        return;
    case 4:
        gpu_copy(w);
        return;
    case 5:
        g.load_x = w[1] & 0x3FF;
        g.load_y = (w[1] >> 16) & 0x1FF;
        g.load_w = (((w[2] & 0xFFFF) - 1) & 0x3FF) + 1;
        g.load_h = ((((w[2] >> 16) & 0xFFFF) - 1) & 0x1FF) + 1;
        g.load_i = 0;
        g.load_left = g.load_w * g.load_h;
        return;
    case 6:
        return; /* VRAM to CPU: nobody reads */
    case 7:
        switch (code) {
        case 0xE1:
            g.texpage = w[0] & 0x1FF;
            g.dither = (w[0] >> 9) & 1;
            g.dfe = (w[0] >> 10) & 1;
            return;
        case 0xE2:
            g.tw_mask_x = (w[0] & 0x1F) * 8;
            g.tw_mask_y = ((w[0] >> 5) & 0x1F) * 8;
            g.tw_off_x = ((w[0] >> 10) & 0x1F) * 8;
            g.tw_off_y = ((w[0] >> 15) & 0x1F) * 8;
            return;
        case 0xE3:
            g.area_x0 = w[0] & 0x3FF;
            g.area_y0 = (w[0] >> 10) & 0x1FF;
            return;
        case 0xE4:
            g.area_x1 = w[0] & 0x3FF;
            g.area_y1 = (w[0] >> 10) & 0x1FF;
            return;
        case 0xE5:
            g.ofs_x = sext11(w[0]);
            g.ofs_y = sext11(w[0] >> 11);
            return;
        case 0xE6:
            g.set_mask = (w[0] & 1) ? 0x8000 : 0;
            g.check_mask = (w[0] >> 1) & 1;
            return;
        default:
            return;
        }
    default:
        if (code == 0x02) {
            gpu_fill(w);
        }
        return;
    }
}

void gpu_gp0_write(u32 word) {
    if (g.load_left > 0) {
        gpu_load_pixel((u16)word);
        if (g.load_left > 0) {
            gpu_load_pixel((u16)(word >> 16));
        }
        return;
    }
    if (g.polyline) {
        u32 code = g.cmd[0] >> 24;
        int gour = (code & 0x10) != 0;

        if ((word & 0xF000F000u) == 0x50005000u && (!gour || g.n == 2)) {
            g.polyline = 0;
            g.n = 0;
            return;
        }
        g.cmd[g.n++] = word;
        if (g.n == (gour ? 4 : 3)) {
            Mode m;
            Vertex a, b;

            gpu_line_mode(&m, code);
            gpu_line_vertex(&a, g.cmd[0], g.cmd[1]);
            gpu_line_vertex(&b, gour ? g.cmd[2] : g.cmd[0], gour ? g.cmd[3] : g.cmd[2]);
            gpu_segment(&m, &a, &b, gour);
            if (gour) {
                g.cmd[0] = (g.cmd[0] & 0xFF000000u) | (g.cmd[2] & 0xFFFFFF);
                g.cmd[1] = g.cmd[3];
            } else {
                g.cmd[1] = g.cmd[2];
            }
            g.n = 2;
        }
        return;
    }
    if (g.n == 0) {
        g.need = gpu_command_length(word >> 24);
    }
    g.cmd[g.n++] = word;
    if (g.n >= g.need) {
        g.n = 0;
        gpu_execute();
    }
}

void gpu_gp0_words(const u32 *w, u32 n) {
    u32 i;

    for (i = 0; i < n; i++) {
        gpu_gp0_write(w[i]);
    }
}

/* CPU to VRAM in one go (LoadImage): GP0 A0h, the rectangle, then the pixels (two per word, the last word's high
 * half unused when the count is odd). */
void gpu_load_image(int x, int y, int w, int h, const u16 *pixels) {
    int i, n;

    gpu_gp0_write(0xA0000000u);
    gpu_gp0_write(((u32)(y & 0xFFFF) << 16) | (u32)(x & 0xFFFF));
    gpu_gp0_write(((u32)(h & 0xFFFF) << 16) | (u32)(w & 0xFFFF));
    n = g.load_left;
    if (!g.check_mask && g.load_i == 0 && n == g.load_w * g.load_h) {
        /* The transfer just started (the usual case): gpu_load_pixel's pixels, a row at a time. */
        int row, x;

        for (row = 0, i = 0; row < g.load_h; row++) {
            int y = (g.load_y + row) & 511;
            u16 *line = &gpu_vram[y * VRAM_W];

            for (x = 0; x < g.load_w; x++) {
                line[(g.load_x + x) & 1023] = pixels[i++] | g.set_mask;
            }
            if (g.load_x + g.load_w <= VRAM_W) {
                gpu_touch(y, g.load_x, g.load_x + g.load_w);
            } else {
                gpu_touch(y, 0, VRAM_W);
            }
        }
        g.load_i = n;
        g.load_left = 0;
        gpu_load_done();
        return;
    }
    for (i = 0; i < n; i++) {
        gpu_load_pixel(pixels[i]);
    }
}
