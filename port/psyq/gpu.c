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
 * Not modelled: timing (a command completes when written), the texture cache (VRAM is always read fresh), GPUSTAT,
 * VRAM-to-CPU transfers (the game never reads VRAM back), the interlaced field skip of `dfe` (only interlaced
 * 480-line drawing would see it; the game's 480-line screens are MDEC frames loaded with LoadImage). */
#include <stdlib.h>
#include <string.h>

#include "psyq_internal.h"

#ifndef PC_PORT
#error "port/psyq is the host shim: compile it with -DPC_PORT"
#endif

/* The rasteriser runs every frame (tens of thousands of frames per test): optimised even in the port's default
 * (-O0) build. Fast paths: opaque untextured spans and tiles are plain fills, opaque sprites drawn as they are (raw, or
 * modulated by 128) copy their texels. */
#if defined(__GNUC__) && !defined(__clang__) && !defined(__OPTIMIZE__)
#pragma GCC optimize("O2")
#endif

#define VRAM_W 1024
#define VRAM_H 512

static u16 gpu_vram[VRAM_W * VRAM_H];

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
    memset(gpu_vram, 0, sizeof(gpu_vram));
    gpu_reset_state();
}

static inline int sext11(u32 v) {
    return (s32)(v << 21) >> 21;
}

/* ---- the pixel pipeline ---- */

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
} Mode;

static inline u16 gpu_texel(const Mode *m, int u, int v) {
    u16 px;

    u = (u & 0xFF & ~g.tw_mask_x) | (g.tw_off_x & g.tw_mask_x);
    v = (v & 0xFF & ~g.tw_mask_y) | (g.tw_off_y & g.tw_mask_y);
    switch (m->textured) {
    case 1:
        px = gpu_vram[((m->tex_y + v) & 511) * VRAM_W + ((m->tex_x + (u >> 2)) & 1023)];
        px = (px >> ((u & 3) * 4)) & 0xF;
        return gpu_vram[m->clut_y * VRAM_W + ((m->clut_x + px) & 1023)];
    case 2:
        px = gpu_vram[((m->tex_y + v) & 511) * VRAM_W + ((m->tex_x + (u >> 1)) & 1023)];
        px = (px >> ((u & 1) * 8)) & 0xFF;
        return gpu_vram[m->clut_y * VRAM_W + ((m->clut_x + px) & 1023)];
    default:
        return gpu_vram[((m->tex_y + v) & 511) * VRAM_W + ((m->tex_x + u) & 1023)];
    }
}

static inline int gpu_clamp(int v, int hi) {
    return v < 0 ? 0 : v > hi ? hi : v;
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
 * primitive blends there, and is the pixel's mask bit (with E6's set-mask bit). */
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

/* ---- primitives ---- */

/* The fast path of an opaque, untextured, undithered primitive without mask checks: one colour for every pixel. */
static inline u16 gpu_flat_colour(u32 rgb) {
    return (u16)(((rgb >> 3) & 0x1F) | (((rgb >> 11) & 0x1F) << 5) | (((rgb >> 19) & 0x1F) << 10) | g.set_mask);
}

static inline void gpu_span_fill(int y, int xs, int xe, u16 c) {
    u16 *p = &gpu_vram[y * VRAM_W + xs];
    int n = xe - xs;

    while (n-- > 0) {
        *p++ = c;
    }
}

typedef struct {
    int x, y;
    int r, g, b;
    int u, v;
} Vertex;

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
}

static inline s64 gpu_floor_div(s64 a, s64 b) {
    s64 q = a / b;

    if ((a % b != 0) && ((a < 0) != (b < 0))) {
        q--;
    }
    return q;
}

static inline s64 gpu_ceil_div(s64 a, s64 b) {
    return -gpu_floor_div(-a, b);
}

/* An attribute along a span: value = base + floor(n / d), n stepping by a constant per pixel; kept as a quotient q and
 * a remainder 0 <= rem < d, so a step is two additions and a compare (no division per pixel). */
typedef struct {
    s64 q, rem;   /* floor(n / d), n - q * d */
    s64 sq, srem; /* the same for the step */
} Interp;

static inline void gpu_interp_start(Interp *it, s64 n, s64 step, s64 d) {
    it->q = gpu_floor_div(n, d);
    it->rem = n - it->q * d;
    it->sq = gpu_floor_div(step, d);
    it->srem = step - it->sq * d;
}

static inline void gpu_interp_step(Interp *it, s64 d) {
    it->q += it->sq;
    it->rem += it->srem;
    if (it->rem >= d) {
        it->rem -= d;
        it->q++;
    }
}

/* One triangle: every pixel (x, y) inside the edges, the left edges and the top edge included (sampled at integer
 * coordinates; the emulator's coverage exactly), each attribute from the plane through the three vertices, rounded to
 * the nearest integer. An exact .5 rounds up here; the emulator's fixed-point steps round it either way (the one rule
 * not reproduced: golden family gpu, uv_ramp_* and uv_axis_*). Gouraud-textured spans that are not dithered, not
 * semi-transparent and not mask-checked take their colour per pair of pixels from the span's start, the first pixel's
 * for both (the emulator draws them two at a time). */
static void gpu_triangle(const Mode *m, const Vertex *a, const Vertex *b, const Vertex *c, int gouraud) {
    const Vertex *v[3] = { a, b, c };
    const Vertex *t;
    s64 den, d2;
    s64 n[5][2];
    int nattr, pairs;
    int y, ymin, ymax, i;
    int dx1, dy1, dx2, dy2;

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
    d2 = 2 * den;
    nattr = gouraud ? (m->textured ? 5 : 3) : (m->textured ? 5 : 0);
    pairs = gouraud && m->textured && !m->dither && !m->semi && !g.check_mask;
    ymin = v[0]->y;
    ymax = v[2]->y;
    if (ymin < g.area_y0) {
        ymin = g.area_y0;
    }
    if (ymax > g.area_y1 + 1) {
        ymax = g.area_y1 + 1;
    }
    for (y = ymin; y < ymax; y++) {
        /* The long edge v0-v2 and the short edge (v0-v1 above v1, v1-v2 from it). */
        const Vertex *s0 = y < v[1]->y ? v[0] : v[1];
        const Vertex *s1 = y < v[1]->y ? v[1] : v[2];
        s64 xl, xr;
        s64 lnum = (s64)v[0]->x * (v[2]->y - v[0]->y) + (s64)(y - v[0]->y) * (v[2]->x - v[0]->x);
        s64 lden = v[2]->y - v[0]->y;
        s64 snum = (s64)s0->x * (s1->y - s0->y) + (s64)(y - s0->y) * (s1->x - s0->x);
        s64 sden = s1->y - s0->y;
        s64 x0, x1;
        int x, xs, xe;
        Interp it[5];
        int cr = a->r, cg = a->g, cb = a->b;

        if (sden == 0) {
            continue;
        }
        x0 = gpu_ceil_div(lnum, lden);
        x1 = gpu_ceil_div(snum, sden);
        /* Which side the long edge is on: compare the edges' positions (as fractions) at this row. */
        if (lnum * sden < snum * lden) {
            xl = x0, xr = x1;
        } else {
            xl = x1, xr = x0;
        }
        xs = xl < g.area_x0 ? g.area_x0 : (int)xl;
        xe = xr > g.area_x1 + 1 ? g.area_x1 + 1 : (int)xr;
        if (xs >= xe) {
            continue;
        }
        if (nattr == 0 && !m->semi && !g.check_mask) {
            gpu_span_fill(y, xs, xe, gpu_flat_colour((u32)a->r | ((u32)a->g << 8) | ((u32)a->b << 16)));
            continue;
        }
        for (i = (gouraud ? 0 : 3); i < nattr; i++) {
            gpu_interp_start(&it[i], 2 * ((s64)(xs - a->x) * n[i][0] + (s64)(y - a->y) * n[i][1]) + den,
                             2 * n[i][0], d2);
        }
        for (x = xs; x < xe; x++) {
            int u = 0, vv = 0;

            if (gouraud && (!pairs || ((x - xs) & 1) == 0)) {
                cr = a->r + (int)it[0].q;
                cg = a->g + (int)it[1].q;
                cb = a->b + (int)it[2].q;
            }
            if (m->textured) {
                u = a->u + (int)it[3].q;
                vv = a->v + (int)it[4].q;
            }
            gpu_pixel(m, x, y, cr, cg, cb, u, vv);
            for (i = (gouraud ? 0 : 3); i < nattr; i++) {
                gpu_interp_step(&it[i], d2);
            }
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
    int width, height;
    int x, y, xs, xe, ys, ye;
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
    xs = x0 < g.area_x0 ? g.area_x0 : x0;
    ys = y0 < g.area_y0 ? g.area_y0 : y0;
    xe = x0 + width > g.area_x1 + 1 ? g.area_x1 + 1 : x0 + width;
    ye = y0 + height > g.area_y1 + 1 ? g.area_y1 + 1 : y0 + height;
    if (!textured && !m.semi && !g.check_mask) {
        u16 c = gpu_flat_colour(w[0]);

        for (y = ys; y < ye; y++) {
            gpu_span_fill(y, xs, xe, c);
        }
        return;
    }
    if (textured && !m.semi && !g.check_mask && (m.raw || (w[0] & 0xFFFFFF) == 0x808080)) {
        /* An opaque sprite drawn as it is (raw, or modulated by 128: (t * 128) >> 7 = t): the texels themselves. */
        const u16 *clut = &gpu_vram[m.clut_y * VRAM_W];

        for (y = ys; y < ye; y++) {
            u16 *row = &gpu_vram[y * VRAM_W];
            int vv = ((v0 + (y - y0)) & 0xFF & ~g.tw_mask_y) | (g.tw_off_y & g.tw_mask_y);
            const u16 *trow = &gpu_vram[((m.tex_y + vv) & 511) * VRAM_W];

            for (x = xs; x < xe; x++) {
                int u = ((u0 + (x - x0)) & 0xFF & ~g.tw_mask_x) | (g.tw_off_x & g.tw_mask_x);
                u16 t;

                if (m.textured == 1) {
                    t = clut[(m.clut_x + ((trow[(m.tex_x + (u >> 2)) & 1023] >> ((u & 3) * 4)) & 0xF)) & 1023];
                } else if (m.textured == 2) {
                    t = clut[(m.clut_x + ((trow[(m.tex_x + (u >> 1)) & 1023] >> ((u & 1) * 8)) & 0xFF)) & 1023];
                } else {
                    t = trow[(m.tex_x + u) & 1023];
                }
                if (t != 0) {
                    row[x] = t | g.set_mask;
                }
            }
        }
        return;
    }
    for (y = ys; y < ye; y++) {
        for (x = xs; x < xe; x++) {
            gpu_pixel(&m, x, y, w[0] & 0xFF, (w[0] >> 8) & 0xFF, (w[0] >> 16) & 0xFF, u0 + (x - x0),
                      v0 + (y - y0));
        }
    }
}

/* One line segment (flat or Gouraud), both ends included. The emulator's stepping, measured (golden family gpu,
 * line_probe_*): along the major axis from the left end (x-major, |dx| >= |dy|) or the top end (y-major), the minor
 * coordinate moves by floor((|minor| * i + c) / major) after i steps, with c = (major + |minor| - 1) / 2 for x-major
 * segments, (major - 1) / 2 for y-major ones going right and major / 2 going left. A sloped segment leaves out the
 * draw area's last column and row (x < x1, y < y1); a horizontal or vertical one reaches them. */
static void gpu_segment(const Mode *m, const Vertex *a, const Vertex *b, int gouraud) {
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
        gpu_pixel(m, x, y, r, gg, bb, 0, 0);
    }
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

    for (y = 0; y < height; y++) {
        u16 *row = &gpu_vram[((y0 + y) & 511) * VRAM_W];

        for (x = 0; x < width; x++) {
            row[(x0 + x) & 1023] = c;
        }
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
    }
}

static void gpu_load_pixel(u16 p) {
    int x = (g.load_x + g.load_i % g.load_w) & 1023;
    int y = (g.load_y + g.load_i / g.load_w) & 511;
    u16 *d = &gpu_vram[y * VRAM_W + x];

    g.load_i++;
    g.load_left--;
    if (g.check_mask && (*d & 0x8000)) {
        return;
    }
    *d = p | g.set_mask;
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
    for (i = 0; i < n; i++) {
        gpu_load_pixel(pixels[i]);
    }
}
