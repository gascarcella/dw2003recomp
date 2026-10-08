/* The hardware renderer (render_gpu.h; issue #31, docs/PORT.md "Rendering"): SDL_GPU on Vulkan, only in the
 * DW3_PORT_SDL build. Two parts:
 *
 * The present. A picture goes into the window's swapchain (or, read back, into a screenshot: `--gpu-screenshot`)
 * through one pair of shaders (port/shaders/present*.hlsl, compiled to SPIR-V at build time), nearest-scaled with
 * integer arithmetic into video.c's 4:3 rectangle: either video.c's 32-bit image (the software path's picture, pixel
 * for pixel the SDL_Renderer path's) or a 15-bit display cut from the rasteriser's target.
 *
 * The rasteriser (phase 2: internal scale 1). gpu.c's listener reports every triangle, rectangle, line segment, fill,
 * VRAM copy and CPU-to-VRAM transfer as gpu.c decodes it (port/psyq/psyq_internal.h); this file records them as units
 * of one frame and runs them at the vsync into a 1024x512 RGBA8 target, a VRAM of its own: each 5-bit channel as
 * c << 3, the mask bit in alpha. Its pixel shader (raster.frag.hlsl) is gpu.c's gpu_pixel in integers: coverage is the
 * GPU's (vertices shifted by half a pixel: the top-left rule then is gpu.c's span rule), attributes are gpu.c's plane
 * equations evaluated per pixel, and blending, dithering and the mask test read the target from a background copy
 * (no fixed-function blending: DECISIONS "The hardware renderer"). Two copies keep the stream's order:
 *  - the software VRAM's copy (R16_UINT, the texels and CLUTs): before a unit samples a block that gpu.c wrote since
 *    the last upload (gpu.c's write stamps), the block is copied now and uploaded at that point of the frame; so is a
 *    transfer's rectangle before the transfer is drawn;
 *  - the background (a copy of the target): before a unit that reads the background (semi-transparent, mask-checked;
 *    a VRAM copy), the 16x16 tiles of its rectangle that something drew into since their last copy are copied. A unit
 *    is one triangle or one line segment (a quad's two triangles may overlap, a polyline's segments share a pixel).
 * At internal scale 1 the target is the software VRAM, pixel for pixel, but for what tests/host/gpu_hw_mismatches.json
 * lists (tests/host/gpu_hw_replay.py checks it against the gpu golden family).
 *
 * Measured on SDL 3.4.18 (issue #31, the plan's "What was verified"): on the offscreen video driver NVIDIA's Vulkan
 * driver gives a device but no presentable surface (claiming the window fails: video.c falls back), while Mesa's
 * lavapipe presents to it (VK_EXT_headless_surface: tests/port/render_gpu.py runs the window path headless there); a
 * window that had an OpenGL SDL_Renderer cannot be claimed by Vulkan afterwards on Wayland, while a released claim
 * leaves the window usable by SDL_Renderer (so render_gpu_open comes first and releases everything on failure);
 * destroying the device before SDL_Quit exits cleanly on NVIDIA (the EGL teardown crash video.c avoids). The swapchain
 * is SDR (8-bit, not sRGB-encoded: the bytes pass through), presented in mailbox mode when the window supports it,
 * else immediate, else vsync: the SDL_Renderer path presents without vsync and pump.c paces the frames. */
#ifdef DW3_PORT_SDL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "port_harness.h"
#include "psyq_internal.h"
#include "render_gpu.h"

#include "present_frag_spv.h"
#include "present_vert_spv.h"
#include "present_vram_frag_spv.h"
#include "raster_frag_spv.h"
#include "raster_vert_spv.h"

#define RENDER_IMAGE_W 640 /* video.c's VIDEO_MAX_W, VIDEO_MAX_H: the largest display image */
#define RENDER_IMAGE_H 576
#define VRAM_W 1024
#define VRAM_H 512
#define TILE 16 /* the background's dirty tiles */
#define TILES_X (VRAM_W / TILE)
#define TILES_Y (VRAM_H / TILE)
#define BLOCKS 16 /* gpu.c's write-stamp blocks per row (64 pixels each) */
#define STAMP_NONE 0xFFFFFFFFu
#define RENDER_MAX_SCALE 8

/* ---- The units (raster.vert.hlsl's and raster.frag.hlsl's uniforms) ---- */

enum { OP_TRIANGLE, OP_RECT, OP_FILL, OP_COPY, OP_LOAD };
enum {
    F_RAW = 1,
    F_SEMI = 2,
    F_GOURAUD = 4,
    F_DITHER = 8,
    F_SET_MASK = 16,
    F_CHECK_MASK = 32,
    F_PAIRS = 64,
    F_BACKGROUND = 128
};

typedef struct {
    float v01[4], v2[4], rect[4], target[4];
    Sint32 mode[4];
} RasterGeometry;

typedef struct {
    Sint32 kind[4], a[4], col[4], nrg[4], nbu[4], nva[4], tex[4], win[4], s01[4], s2[4], res[4], uvr[4], cmin[4],
        cmax[4];
} RasterUnit;

/* An entry of the frame's list, in stream order. */
enum { REC_DRAW, REC_UPLOAD, REC_BACKGROUND };
typedef struct {
    int kind;
    int x, y, w, h;   /* the upload's or the background copy's rectangle; the draw's scissor */
    size_t offset;    /* the upload's pixels in the staging buffer (bytes) */
    int vertices;     /* the draw's: 3 (a triangle) or 6 (a rectangle) */
    RasterGeometry geometry;
    RasterUnit unit;
} Record;

static struct {
    SDL_GPUDevice *device;
    SDL_Window *window;                    /* claimed; NULL headless */
    SDL_GPUTextureFormat swap_format;      /* the swapchain's (SDR: B8G8R8A8 or R8G8B8A8 UNORM) */
    SDL_GPUShader *vert, *frag, *frag_vram;
    SDL_GPUGraphicsPipeline *pipe_swap, *pipe_vram_swap; /* the presents into the swapchain's format */
    SDL_GPUGraphicsPipeline *pipe_off, *pipe_vram_off;   /* into B8G8R8A8_UNORM (the readback target) */
    SDL_GPUSampler *sampler;               /* bound with the textures (the shaders read them with Load) */
    SDL_GPUTexture *image;                 /* the software image, RENDER_IMAGE_W x RENDER_IMAGE_H */
    SDL_GPUTransferBuffer *upload;         /* its pixels on the way */
    SDL_GPUTexture *target;                /* the readback's offscreen target and its download buffer */
    SDL_GPUTransferBuffer *download;
    int target_w, target_h;
    int submit_failed;
    char describe[160];

    /* The rasteriser. */
    int raster;
    int scale;                             /* the internal scale: the target is 1024 scale x 512 scale */
    SDL_GPUShader *raster_vert, *raster_frag;
    SDL_GPUGraphicsPipeline *pipe_raster;
    SDL_GPUTexture *vram_target;           /* RGBA8 1024 scale x 512 scale: the rasteriser's VRAM */
    SDL_GPUTexture *background;            /* its copy, what units read */
    SDL_GPUTexture *mirror;                /* R16_UINT 1024x512: the software VRAM's copy */
    SDL_GPUTransferBuffer *staging_buf;    /* the frame's uploads */
    Uint32 staging_buf_size;
    SDL_GPUTransferBuffer *vram_download;  /* render_gpu_read_vram */
    Record *recs;
    size_t nrecs, caprecs;
    u8 *staging;                           /* the frame's upload pixels */
    size_t nstaging, capstaging;
    u32 uploaded[VRAM_H][BLOCKS];          /* the stamp each block had at its last upload; STAMP_NONE: never */
    u32 generation;                        /* gpu.c's stamp generation at the last check */
    u8 dirty[TILES_Y][TILES_X];            /* drawn into since the tile's last background copy */
} r;

/* ---- Recording ---- */

static Record *rec_add(int kind) {
    Record *e;
    if (r.nrecs == r.caprecs) {
        size_t cap = r.caprecs ? r.caprecs * 2 : 1024;
        Record *n = realloc(r.recs, cap * sizeof(*n));
        if (n == NULL) {
            port_fatal("renderer: gpu: out of memory (%zu units)", cap);
        }
        r.recs = n;
        r.caprecs = cap;
    }
    e = &r.recs[r.nrecs++];
    memset(e, 0, sizeof(*e));
    e->kind = kind;
    return e;
}

static size_t staging_add(const void *data, size_t size) {
    size_t at = r.nstaging;
    if (r.nstaging + size > r.capstaging) {
        size_t cap = r.capstaging ? r.capstaging : 1 << 20;
        u8 *n;
        while (cap < r.nstaging + size) {
            cap *= 2;
        }
        n = realloc(r.staging, cap);
        if (n == NULL) {
            port_fatal("renderer: gpu: out of memory (%zu bytes of uploads)", cap);
        }
        r.staging = n;
        r.capstaging = cap;
    }
    memcpy(r.staging + at, data, size);
    r.nstaging += size;
    return at;
}

/* Brings the VRAM copy up to date over the rows y0 .. y0 + h - 1 and the columns x0 .. x0 + w - 1 (both wrapping):
 * every block among them that gpu.c wrote since its last upload is copied now and uploaded at this point of the frame.
 * Consecutive rows with the same span of blocks become one upload. */
static void sync_vram(int x0, int y0, int w, int h) {
    const u32 *stamps = gpu_block_stamps();
    const u16 *vram = gpu_vram_pixels();
    unsigned need = 0;
    int j;
    Record *run = NULL;
    u32 now;

    if (w <= 0 || h <= 0) {
        return;
    }
    if (gpu_stamp_generation() != r.generation) {
        r.generation = gpu_stamp_generation();
        memset(r.uploaded, 0xFF, sizeof(r.uploaded)); /* the stamps started over: every block again */
    }
    if (w >= VRAM_W) {
        need = 0xFFFF;
    } else {
        int i;
        for (i = 0; i < w; i += 64 - ((x0 + i) & 63)) {
            need |= 1u << (((x0 + i) & 1023) >> 6);
        }
        need |= 1u << (((x0 + w - 1) & 1023) >> 6);
    }
    now = gpu_stamp_bump(); /* the blocks' stamps are at most `now`; later writes carry newer ones */
    for (j = 0; j < (h < VRAM_H ? h : VRAM_H); j++) {
        int y = (y0 + j) & 511, b, lo = -1, hi = -1;
        for (b = 0; b < BLOCKS; b++) {
            if ((need >> b & 1) && (r.uploaded[y][b] == STAMP_NONE || stamps[y * BLOCKS + b] > r.uploaded[y][b])) {
                lo = lo < 0 ? b : lo;
                hi = b;
            }
        }
        if (lo < 0) {
            run = NULL;
            continue;
        }
        for (b = lo; b <= hi; b++) {
            r.uploaded[y][b] = now;
        }
        if (run != NULL && run->y + run->h == y && run->x == lo * 64 && run->w == (hi - lo + 1) * 64) {
            run->h++;
        } else {
            run = rec_add(REC_UPLOAD);
            run->x = lo * 64;
            run->y = y;
            run->w = (hi - lo + 1) * 64;
            run->h = 1;
            run->offset = r.nstaging;
        }
        staging_add(vram + y * VRAM_W + lo * 64, (size_t)(hi - lo + 1) * 64 * 2);
    }
}

/* The rectangle clipped to the VRAM; 0 when empty. */
static int clip(int *x, int *y, int *w, int *h) {
    if (*x < 0) {
        *w += *x;
        *x = 0;
    }
    if (*y < 0) {
        *h += *y;
        *y = 0;
    }
    if (*x + *w > VRAM_W) {
        *w = VRAM_W - *x;
    }
    if (*y + *h > VRAM_H) {
        *h = VRAM_H - *y;
    }
    return *w > 0 && *h > 0;
}

/* Before a unit reads the target over this rectangle: the dirty tiles in it are copied to the background. */
static void sync_background(int x, int y, int w, int h) {
    int tx0, ty0, tx1, ty1, tx, ty, dirty = 0;
    Record *e;
    if (!clip(&x, &y, &w, &h)) {
        return;
    }
    tx0 = x / TILE, ty0 = y / TILE, tx1 = (x + w - 1) / TILE, ty1 = (y + h - 1) / TILE;
    for (ty = ty0; ty <= ty1; ty++) {
        for (tx = tx0; tx <= tx1; tx++) {
            dirty |= r.dirty[ty][tx];
            r.dirty[ty][tx] = 0;
        }
    }
    if (!dirty) {
        return;
    }
    e = rec_add(REC_BACKGROUND);
    e->x = tx0 * TILE;
    e->y = ty0 * TILE;
    e->w = (tx1 - tx0 + 1) * TILE;
    e->h = (ty1 - ty0 + 1) * TILE;
}

static void mark_dirty(int x, int y, int w, int h) {
    int tx, ty;
    if (!clip(&x, &y, &w, &h)) {
        return;
    }
    for (ty = y / TILE; ty <= (y + h - 1) / TILE; ty++) {
        for (tx = x / TILE; tx <= (x + w - 1) / TILE; tx++) {
            r.dirty[ty][tx] = 1;
        }
    }
}

/* A draw of the unit u over [x0, x1) x [y0, y1) (a rectangle, or a triangle's bounding box) within the scissor. */
static Record *draw_add(const RasterUnit *u, int vertices, int sx, int sy, int sw, int sh) {
    Record *e = rec_add(REC_DRAW);
    e->unit = *u;
    e->vertices = vertices;
    e->x = sx;
    e->y = sy;
    e->w = sw;
    e->h = sh;
    e->unit.res[0] = r.scale;
    e->unit.res[1] = VRAM_W * r.scale;
    e->unit.res[2] = VRAM_H * r.scale;
    e->geometry.target[0] = (float)(VRAM_W * r.scale);
    e->geometry.target[1] = (float)(VRAM_H * r.scale);
    e->geometry.target[2] = (float)r.scale;
    e->geometry.target[3] = vertices == 3 ? 0.5f : 0.0f;
    e->geometry.mode[0] = vertices == 3 ? 0 : 1;
    return e;
}

static void rect_geometry(Record *e, int x, int y, int w, int h) {
    e->geometry.rect[0] = (float)x;
    e->geometry.rect[1] = (float)y;
    e->geometry.rect[2] = (float)(x + w);
    e->geometry.rect[3] = (float)(y + h);
}

/* The flags, blend mode, depth, texture page, CLUT and window of a drawing event. */
static void unit_mode(RasterUnit *u, int op, const GpuEvent *ev) {
    memset(u, 0, sizeof(*u));
    u->kind[0] = op;
    u->kind[1] = (ev->raw ? F_RAW : 0) | (ev->semi ? F_SEMI : 0) | (ev->gouraud ? F_GOURAUD : 0) |
                 (ev->dither ? F_DITHER : 0) | (ev->set_mask ? F_SET_MASK : 0) | (ev->check_mask ? F_CHECK_MASK : 0) |
                 (ev->semi || ev->check_mask ? F_BACKGROUND : 0);
    u->kind[2] = ev->abr;
    u->kind[3] = ev->textured;
    u->tex[0] = ev->tex_x;
    u->tex[1] = ev->tex_y;
    u->tex[2] = ev->clut_x;
    u->tex[3] = ev->clut_y;
    u->win[0] = ev->u_and;
    u->win[1] = ev->u_or;
    u->win[2] = ev->v_and;
    u->win[3] = ev->v_or;
    u->nva[2] = ev->area_x0;
}

/* The drawing area as a scissor clipped to the VRAM; 0 when empty. */
static int area(const GpuEvent *ev, int *x, int *y, int *w, int *h) {
    *x = ev->area_x0;
    *y = ev->area_y0;
    *w = ev->area_x1 - ev->area_x0 + 1;
    *h = ev->area_y1 - ev->area_y0 + 1;
    return clip(x, y, w, h);
}

/* The texels and the CLUT a textured unit may read: texture rows v_lo .. v_hi of its page (all 256 under a texture
 * window or when the range wraps), the page's width in VRAM words, the CLUT's entries. */
static void sync_texture(const GpuEvent *ev, int v_lo, int v_hi) {
    static const int page_words[4] = { 0, 64, 128, 256 };
    if (!ev->textured) {
        return;
    }
    if (ev->v_and != 0xFF || v_lo < 0 || v_hi > 255 || v_lo > v_hi) {
        v_lo = 0;
        v_hi = 255;
    }
    sync_vram(ev->tex_x, ev->tex_y + v_lo, page_words[ev->textured], v_hi - v_lo + 1);
    if (ev->textured < 3) {
        sync_vram(ev->clut_x, ev->clut_y, ev->textured == 1 ? 16 : 256, 1);
    }
}

static void raster_triangle(const GpuEvent *ev) {
    const GpuVertex *a = &ev->v[0], *b = &ev->v[1], *c = &ev->v[2];
    const GpuVertex *s[3] = { a, b, c }, *t;
    RasterUnit u;
    Record *e;
    int dx1 = b->x - a->x, dy1 = b->y - a->y, dx2 = c->x - a->x, dy2 = c->y - a->y;
    int den = dx1 * dy2 - dx2 * dy1, sign = den < 0 ? -1 : 1;
    int sx, sy, sw, sh, x0, y0, x1, y1;
    int vmin = a->v, vmax = a->v;

    if (!area(ev, &sx, &sy, &sw, &sh)) {
        return;
    }
    /* The bounding box within the scissor. */
    x0 = SDL_min(a->x, SDL_min(b->x, c->x));
    y0 = SDL_min(a->y, SDL_min(b->y, c->y));
    x1 = SDL_max(a->x, SDL_max(b->x, c->x));
    y1 = SDL_max(a->y, SDL_max(b->y, c->y));
    x0 = SDL_max(x0, sx);
    y0 = SDL_max(y0, sy);
    x1 = SDL_min(x1, sx + sw - 1);
    y1 = SDL_min(y1, sy + sh - 1);
    if (x0 > x1 || y0 > y1) {
        return;
    }
    unit_mode(&u, OP_TRIANGLE, ev);
    if (ev->gouraud && ev->textured && !ev->dither && !ev->semi && !ev->check_mask && r.scale == 1) {
        u.kind[1] |= F_PAIRS; /* gpu.c's colour per pair of pixels: a scale-1 rule (smooth above it) */
    }
    u.a[0] = a->x;
    u.a[1] = a->y;
    u.a[2] = a->u;
    u.a[3] = a->v;
    u.col[0] = a->r;
    u.col[1] = a->g;
    u.col[2] = a->b;
    u.col[3] = den * sign;
#define PLANE(dst0, dst1, f)                                                                                          \
    do {                                                                                                             \
        int e1 = b->f - a->f, e2 = c->f - a->f;                                                                      \
        dst0 = (e1 * dy2 - e2 * dy1) * sign;                                                                         \
        dst1 = (e2 * dx1 - e1 * dx2) * sign;                                                                         \
    } while (0)
    PLANE(u.nrg[0], u.nrg[1], r);
    PLANE(u.nrg[2], u.nrg[3], g);
    PLANE(u.nbu[0], u.nbu[1], b);
    PLANE(u.nbu[2], u.nbu[3], u);
    PLANE(u.nva[0], u.nva[1], v);
#undef PLANE
    /* Sorted by y as gpu.c sorts them (the per-pair rule's span start). */
    if (s[1]->y < s[0]->y) {
        t = s[0], s[0] = s[1], s[1] = t;
    }
    if (s[2]->y < s[1]->y) {
        t = s[1], s[1] = s[2], s[2] = t;
    }
    if (s[1]->y < s[0]->y) {
        t = s[0], s[0] = s[1], s[1] = t;
    }
    u.s01[0] = s[0]->x;
    u.s01[1] = s[0]->y;
    u.s01[2] = s[1]->x;
    u.s01[3] = s[1]->y;
    u.s2[0] = s[2]->x;
    u.s2[1] = s[2]->y;
    vmin = SDL_min(vmin, SDL_min(b->v, c->v));
    vmax = SDL_max(vmax, SDL_max(b->v, c->v));
    /* The vertices' ranges: above scale 1 a sample point may lie just outside the triangle (raster.frag.hlsl) */
    u.uvr[0] = SDL_min(a->u, SDL_min(b->u, c->u));
    u.uvr[1] = SDL_max(a->u, SDL_max(b->u, c->u));
    u.uvr[2] = vmin;
    u.uvr[3] = vmax;
    u.cmin[0] = SDL_min(a->r, SDL_min(b->r, c->r));
    u.cmin[1] = SDL_min(a->g, SDL_min(b->g, c->g));
    u.cmin[2] = SDL_min(a->b, SDL_min(b->b, c->b));
    u.cmax[0] = SDL_max(a->r, SDL_max(b->r, c->r));
    u.cmax[1] = SDL_max(a->g, SDL_max(b->g, c->g));
    u.cmax[2] = SDL_max(a->b, SDL_max(b->b, c->b));
    sync_texture(ev, vmin, vmax);
    if (u.kind[1] & F_BACKGROUND) {
        sync_background(x0, y0, x1 - x0 + 1, y1 - y0 + 1);
    }
    e = draw_add(&u, 3, sx, sy, sw, sh);
    e->geometry.v01[0] = (float)a->x;
    e->geometry.v01[1] = (float)a->y;
    e->geometry.v01[2] = (float)b->x;
    e->geometry.v01[3] = (float)b->y;
    e->geometry.v2[0] = (float)c->x;
    e->geometry.v2[1] = (float)c->y;
    mark_dirty(x0, y0, x1 - x0 + 1, y1 - y0 + 1);
}

static void raster_rect(const GpuEvent *ev) {
    RasterUnit u;
    Record *e;
    int sx, sy, sw, sh, x = ev->x, y = ev->y, w = ev->w, h = ev->h;

    if (!area(ev, &sx, &sy, &sw, &sh)) {
        return;
    }
    /* The rectangle within the scissor. */
    if (x < sx) {
        w -= sx - x;
        x = sx;
    }
    if (y < sy) {
        h -= sy - y;
        y = sy;
    }
    w = SDL_min(w, sx + sw - x);
    h = SDL_min(h, sy + sh - y);
    if (w <= 0 || h <= 0) {
        return;
    }
    unit_mode(&u, OP_RECT, ev);
    u.a[0] = ev->v[0].x;
    u.a[1] = ev->v[0].y;
    u.a[2] = ev->v[0].u;
    u.a[3] = ev->v[0].v;
    u.col[0] = ev->v[0].r;
    u.col[1] = ev->v[0].g;
    u.col[2] = ev->v[0].b;
    sync_texture(ev, ev->v[0].v + (y - ev->y), ev->v[0].v + (y - ev->y) + h - 1);
    if (u.kind[1] & F_BACKGROUND) {
        sync_background(x, y, w, h);
    }
    e = draw_add(&u, 6, x, y, w, h);
    rect_geometry(e, x, y, w, h);
    mark_dirty(x, y, w, h);
}

/* A segment's pixels, collected by gpu_segment_walk. */
typedef struct {
    int n, cap;
    int (*px)[5];
} Pixels;

static void collect_pixel(void *ctx, int x, int y, int cr, int cg, int cb) {
    Pixels *p = ctx;
    if (p->n == p->cap) {
        int cap = p->cap ? p->cap * 2 : 1024;
        void *n = realloc(p->px, (size_t)cap * sizeof(*p->px));
        if (n == NULL) {
            port_fatal("renderer: gpu: out of memory (a line of %d pixels)", cap);
        }
        p->px = n;
        p->cap = cap;
    }
    p->px[p->n][0] = x;
    p->px[p->n][1] = y;
    p->px[p->n][2] = cr;
    p->px[p->n][3] = cg;
    p->px[p->n][4] = cb;
    p->n++;
}

static void raster_segment(const GpuEvent *ev) {
    static Pixels pixels;
    RasterUnit u;
    int i, x0 = VRAM_W, y0 = VRAM_H, x1 = -1, y1 = -1;

    pixels.n = 0;
    gpu_segment_walk(&ev->v[0], &ev->v[1], ev->gouraud, collect_pixel, &pixels);
    if (pixels.n == 0) {
        return;
    }
    for (i = 0; i < pixels.n; i++) {
        x0 = SDL_min(x0, pixels.px[i][0]);
        y0 = SDL_min(y0, pixels.px[i][1]);
        x1 = SDL_max(x1, pixels.px[i][0]);
        y1 = SDL_max(y1, pixels.px[i][1]);
    }
    unit_mode(&u, OP_RECT, ev);
    u.kind[1] &= ~F_GOURAUD; /* the walk gives each pixel's colour */
    if (u.kind[1] & F_BACKGROUND) {
        sync_background(x0, y0, x1 - x0 + 1, y1 - y0 + 1);
    }
    for (i = 0; i < pixels.n; i++) {
        Record *e;
        u.a[0] = pixels.px[i][0];
        u.a[1] = pixels.px[i][1];
        u.col[0] = pixels.px[i][2];
        u.col[1] = pixels.px[i][3];
        u.col[2] = pixels.px[i][4];
        e = draw_add(&u, 6, pixels.px[i][0], pixels.px[i][1], 1, 1);
        rect_geometry(e, pixels.px[i][0], pixels.px[i][1], 1, 1);
    }
    mark_dirty(x0, y0, x1 - x0 + 1, y1 - y0 + 1);
}

/* A rectangle at (x, y) wrapping at the VRAM's edges, as up to four rectangles inside it: each piece's corner and size
 * in piece[k][0..3], the piece's offset from (x, y) in piece[k][4..5]. */
static int split_wrap(int x, int y, int w, int h, int piece[4][6]) {
    int n = 0, i, j;
    int xs[2][3], ys[2][3], nx = 0, ny = 0;
    x &= 1023;
    y &= 511;
    w = SDL_min(w, VRAM_W);
    h = SDL_min(h, VRAM_H);
    xs[nx][0] = x, xs[nx][1] = SDL_min(w, VRAM_W - x), xs[nx++][2] = 0;
    if (x + w > VRAM_W) {
        xs[nx][0] = 0, xs[nx][1] = x + w - VRAM_W, xs[nx++][2] = VRAM_W - x;
    }
    ys[ny][0] = y, ys[ny][1] = SDL_min(h, VRAM_H - y), ys[ny++][2] = 0;
    if (y + h > VRAM_H) {
        ys[ny][0] = 0, ys[ny][1] = y + h - VRAM_H, ys[ny++][2] = VRAM_H - y;
    }
    for (j = 0; j < ny; j++) {
        for (i = 0; i < nx; i++) {
            piece[n][0] = xs[i][0];
            piece[n][1] = ys[j][0];
            piece[n][2] = xs[i][1];
            piece[n][3] = ys[j][1];
            piece[n][4] = xs[i][2];
            piece[n][5] = ys[j][2];
            n++;
        }
    }
    return n;
}

static void raster_fill(const GpuEvent *ev) {
    int piece[4][6], n, k;
    RasterUnit u;
    memset(&u, 0, sizeof(u));
    u.kind[0] = OP_FILL;
    u.a[0] = ev->color;
    n = split_wrap(ev->x, ev->y, ev->w, ev->h, piece);
    for (k = 0; k < n; k++) {
        Record *e = draw_add(&u, 6, piece[k][0], piece[k][1], piece[k][2], piece[k][3]);
        rect_geometry(e, piece[k][0], piece[k][1], piece[k][2], piece[k][3]);
        mark_dirty(piece[k][0], piece[k][1], piece[k][2], piece[k][3]);
    }
}

/* A transfer's rectangle: the VRAM copy brought up to date there, then drawn from it into the target. */
static void raster_load(int x, int y, int w, int h, int check_mask) {
    int piece[4][6], n, k;
    RasterUnit u;
    memset(&u, 0, sizeof(u));
    u.kind[0] = OP_LOAD;
    u.kind[1] = check_mask ? F_CHECK_MASK | F_BACKGROUND : 0;
    sync_vram(x, y, w, h);
    n = split_wrap(x, y, w, h, piece);
    for (k = 0; k < n; k++) {
        Record *e;
        if (check_mask) {
            sync_background(piece[k][0], piece[k][1], piece[k][2], piece[k][3]);
        }
        e = draw_add(&u, 6, piece[k][0], piece[k][1], piece[k][2], piece[k][3]);
        rect_geometry(e, piece[k][0], piece[k][1], piece[k][2], piece[k][3]);
        mark_dirty(piece[k][0], piece[k][1], piece[k][2], piece[k][3]);
    }
}

/* The cuts of [0, n) where a or b + offset reaches `size`: 0, the wraps inside, n (sorted, unique). */
static int cuts(int a, int b, int n, int size, int out[4]) {
    int k = 0, c1 = size - a, c2 = size - b, i, j;
    out[k++] = 0;
    if (c1 > 0 && c1 < n) {
        out[k++] = c1;
    }
    if (c2 > 0 && c2 < n && c2 != c1) {
        out[k++] = c2;
    }
    out[k++] = n;
    for (i = 1; i < k; i++) {
        for (j = i; j > 0 && out[j] < out[j - 1]; j--) {
            int t = out[j];
            out[j] = out[j - 1];
            out[j - 1] = t;
        }
    }
    return k;
}

/* A VRAM copy, in pieces where neither the source nor the destination wraps; each reads the background (the source,
 * and the destination's mask bits), but for a copy that smears (below). */
static void raster_copy(const GpuEvent *ev) {
    int xs[4], ys[4], nx, ny, i, j;
    int ddx = ((ev->sx - ev->x + 512) & 1023) - 512, ddy = ((ev->sy - ev->y + 256) & 511) - 256;
    RasterUnit u;
    /* gpu.c copies pixel by pixel, rows from the top, each from the left, reading the VRAM as it writes: a copy whose
     * destination overlaps its source further on re-reads pixels it wrote (it smears). The event comes after the copy:
     * such a copy's destination is then taken from the VRAM, as a transfer is. */
    if (abs(ddx) < ev->w && abs(ddy) < ev->h && (ddy < 0 || (ddy == 0 && ddx < 0))) {
        raster_load(ev->x, ev->y, ev->w, ev->h, 0);
        return;
    }
    memset(&u, 0, sizeof(u));
    u.kind[0] = OP_COPY;
    u.kind[1] = F_BACKGROUND | (ev->set_mask ? F_SET_MASK : 0) | (ev->check_mask ? F_CHECK_MASK : 0);
    nx = cuts(ev->sx, ev->x, ev->w, VRAM_W, xs);
    ny = cuts(ev->sy, ev->y, ev->h, VRAM_H, ys);
    for (j = 0; j + 1 < ny; j++) {
        for (i = 0; i + 1 < nx; i++) {
            int pw = xs[i + 1] - xs[i], ph = ys[j + 1] - ys[j];
            int dx = (ev->x + xs[i]) & 1023, dy = (ev->y + ys[j]) & 511;
            int sx = (ev->sx + xs[i]) & 1023, sy = (ev->sy + ys[j]) & 511;
            Record *e;
            sync_background(sx, sy, pw, ph);
            sync_background(dx, dy, pw, ph);
            u.s2[2] = sx - dx;
            u.s2[3] = sy - dy;
            e = draw_add(&u, 6, dx, dy, pw, ph);
            rect_geometry(e, dx, dy, pw, ph);
            mark_dirty(dx, dy, pw, ph);
        }
    }
}

static void raster_event(const GpuEvent *ev) {
    switch (ev->kind) {
    case GPU_EV_TRIANGLE:
        raster_triangle(ev);
        break;
    case GPU_EV_RECT:
        raster_rect(ev);
        break;
    case GPU_EV_SEGMENT:
        raster_segment(ev);
        break;
    case GPU_EV_FILL:
        raster_fill(ev);
        break;
    case GPU_EV_COPY:
        raster_copy(ev);
        break;
    case GPU_EV_LOAD:
        raster_load(ev->x, ev->y, ev->w, ev->h, ev->check_mask);
        break;
    case GPU_EV_POWER_ON:
        memset(r.uploaded, 0xFF, sizeof(r.uploaded));
        raster_load(0, 0, VRAM_W, VRAM_H, 0);
        break;
    }
}

/* ---- The device ---- */

static void render_release(void) {
    if (r.raster) {
        gpu_set_listener(NULL);
    }
    if (r.device != NULL) {
        SDL_GPUTexture *textures[] = { r.target, r.image, r.vram_target, r.background, r.mirror };
        SDL_GPUTransferBuffer *buffers[] = { r.download, r.upload, r.staging_buf, r.vram_download };
        SDL_GPUGraphicsPipeline *pipes[] = { r.pipe_off, r.pipe_swap, r.pipe_vram_off, r.pipe_vram_swap,
                                             r.pipe_raster };
        SDL_GPUShader *shaders[] = { r.frag, r.vert, r.frag_vram, r.raster_vert, r.raster_frag };
        size_t i;
        for (i = 0; i < SDL_arraysize(textures); i++) {
            if (textures[i] != NULL) {
                SDL_ReleaseGPUTexture(r.device, textures[i]);
            }
        }
        for (i = 0; i < SDL_arraysize(buffers); i++) {
            if (buffers[i] != NULL) {
                SDL_ReleaseGPUTransferBuffer(r.device, buffers[i]);
            }
        }
        if (r.sampler != NULL) {
            SDL_ReleaseGPUSampler(r.device, r.sampler);
        }
        for (i = 0; i < SDL_arraysize(pipes); i++) {
            if (pipes[i] != NULL) {
                SDL_ReleaseGPUGraphicsPipeline(r.device, pipes[i]);
            }
        }
        for (i = 0; i < SDL_arraysize(shaders); i++) {
            if (shaders[i] != NULL) {
                SDL_ReleaseGPUShader(r.device, shaders[i]);
            }
        }
        if (r.window != NULL) {
            SDL_ReleaseWindowFromGPUDevice(r.device, r.window);
        }
        SDL_DestroyGPUDevice(r.device);
    }
    free(r.recs);
    free(r.staging);
    memset(&r, 0, sizeof(r));
}

static SDL_GPUShader *render_shader(const unsigned char *code, size_t size, SDL_GPUShaderStage stage, int samplers,
                                    int uniforms) {
    SDL_GPUShaderCreateInfo ci;
    memset(&ci, 0, sizeof(ci));
    ci.code = code;
    ci.code_size = size;
    ci.entrypoint = "main";
    ci.format = SDL_GPU_SHADERFORMAT_SPIRV;
    ci.stage = stage;
    ci.num_samplers = (Uint32)samplers;
    ci.num_uniform_buffers = (Uint32)uniforms;
    return SDL_CreateGPUShader(r.device, &ci);
}

static SDL_GPUGraphicsPipeline *render_pipeline(SDL_GPUShader *vert, SDL_GPUShader *frag,
                                                SDL_GPUTextureFormat format) {
    SDL_GPUGraphicsPipelineCreateInfo ci;
    SDL_GPUColorTargetDescription target;
    memset(&ci, 0, sizeof(ci));
    memset(&target, 0, sizeof(target));
    target.format = format;
    ci.vertex_shader = vert;
    ci.fragment_shader = frag;
    ci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    ci.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    ci.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    ci.target_info.color_target_descriptions = &target;
    ci.target_info.num_color_targets = 1;
    return SDL_CreateGPUGraphicsPipeline(r.device, &ci);
}

static SDL_GPUTexture *render_texture(SDL_GPUTextureFormat format, int w, int h, SDL_GPUTextureUsageFlags usage) {
    SDL_GPUTextureCreateInfo ti;
    memset(&ti, 0, sizeof(ti));
    ti.type = SDL_GPU_TEXTURETYPE_2D;
    ti.format = format;
    ti.usage = usage;
    ti.width = (Uint32)w;
    ti.height = (Uint32)h;
    ti.layer_count_or_depth = 1;
    ti.num_levels = 1;
    return SDL_CreateGPUTexture(r.device, &ti);
}

static SDL_GPUTransferBuffer *render_buffer(SDL_GPUTransferBufferUsage usage, Uint32 size) {
    SDL_GPUTransferBufferCreateInfo bi;
    memset(&bi, 0, sizeof(bi));
    bi.usage = usage;
    bi.size = size;
    return SDL_CreateGPUTransferBuffer(r.device, &bi);
}

static int render_fail(char *why, size_t why_size, const char *what) {
    snprintf(why, why_size, "%s: %s", what, SDL_GetError());
    render_release();
    return 0;
}

int render_gpu_open(SDL_Window *window, char *why, size_t why_size) {
    SDL_GPUSamplerCreateInfo si;
    SDL_PropertiesID props;

    if (r.device != NULL) {
        return 1;
    }
    r.device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
    if (r.device == NULL) {
        return render_fail(why, why_size, "SDL_CreateGPUDevice");
    }
    props = SDL_GetGPUDeviceProperties(r.device);
    snprintf(r.describe, sizeof(r.describe), "%s, %s", SDL_GetGPUDeviceDriver(r.device),
             SDL_GetStringProperty(props, SDL_PROP_GPU_DEVICE_NAME_STRING, "unnamed device"));
    if (window != NULL) {
        SDL_GPUPresentMode mode = SDL_GPU_PRESENTMODE_VSYNC;
        if (!SDL_ClaimWindowForGPUDevice(r.device, window)) {
            return render_fail(why, why_size, "SDL_ClaimWindowForGPUDevice");
        }
        r.window = window;
        if (SDL_WindowSupportsGPUPresentMode(r.device, window, SDL_GPU_PRESENTMODE_MAILBOX)) {
            mode = SDL_GPU_PRESENTMODE_MAILBOX;
        } else if (SDL_WindowSupportsGPUPresentMode(r.device, window, SDL_GPU_PRESENTMODE_IMMEDIATE)) {
            mode = SDL_GPU_PRESENTMODE_IMMEDIATE;
        }
        if (!SDL_SetGPUSwapchainParameters(r.device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, mode)) {
            return render_fail(why, why_size, "SDL_SetGPUSwapchainParameters");
        }
        r.swap_format = SDL_GetGPUSwapchainTextureFormat(r.device, window);
        snprintf(r.describe + strlen(r.describe), sizeof(r.describe) - strlen(r.describe), ", %s",
                 mode == SDL_GPU_PRESENTMODE_MAILBOX     ? "mailbox"
                 : mode == SDL_GPU_PRESENTMODE_IMMEDIATE ? "immediate"
                                                         : "vsync");
    }
    r.vert = render_shader(present_vert_spv, sizeof(present_vert_spv), SDL_GPU_SHADERSTAGE_VERTEX, 0, 0);
    r.frag = render_shader(present_frag_spv, sizeof(present_frag_spv), SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 1);
    r.frag_vram =
        render_shader(present_vram_frag_spv, sizeof(present_vram_frag_spv), SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 1);
    if (r.vert == NULL || r.frag == NULL || r.frag_vram == NULL) {
        return render_fail(why, why_size, "SDL_CreateGPUShader");
    }
    r.pipe_off = render_pipeline(r.vert, r.frag, SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM);
    r.pipe_vram_off = render_pipeline(r.vert, r.frag_vram, SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM);
    if (r.pipe_off == NULL || r.pipe_vram_off == NULL) {
        return render_fail(why, why_size, "SDL_CreateGPUGraphicsPipeline");
    }
    if (window != NULL && r.swap_format != SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM) {
        r.pipe_swap = render_pipeline(r.vert, r.frag, r.swap_format);
        r.pipe_vram_swap = render_pipeline(r.vert, r.frag_vram, r.swap_format);
        if (r.pipe_swap == NULL || r.pipe_vram_swap == NULL) {
            return render_fail(why, why_size, "SDL_CreateGPUGraphicsPipeline (swapchain)");
        }
    }
    memset(&si, 0, sizeof(si));
    si.min_filter = SDL_GPU_FILTER_NEAREST;
    si.mag_filter = SDL_GPU_FILTER_NEAREST;
    si.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    si.address_mode_u = si.address_mode_v = si.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    r.sampler = SDL_CreateGPUSampler(r.device, &si);
    /* B8G8R8A8: 0xFFRRGGBB in memory is B, G, R, A */
    r.image = render_texture(SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM, RENDER_IMAGE_W, RENDER_IMAGE_H,
                             SDL_GPU_TEXTUREUSAGE_SAMPLER);
    r.upload = render_buffer(SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, RENDER_IMAGE_W * RENDER_IMAGE_H * 4);
    if (r.sampler == NULL || r.image == NULL || r.upload == NULL) {
        return render_fail(why, why_size, "the present's texture");
    }
    return 1;
}

int render_gpu_active(void) {
    return r.device != NULL;
}

const char *render_gpu_describe(void) {
    return r.device != NULL ? r.describe : "";
}

int render_gpu_raster_start(int scale, char *why, size_t why_size) {
    if (r.device == NULL) {
        snprintf(why, why_size, "no device");
        return 0;
    }
    if (r.raster) {
        return 1;
    }
    r.raster_vert = render_shader(raster_vert_spv, sizeof(raster_vert_spv), SDL_GPU_SHADERSTAGE_VERTEX, 0, 1);
    r.raster_frag = render_shader(raster_frag_spv, sizeof(raster_frag_spv), SDL_GPU_SHADERSTAGE_FRAGMENT, 2, 1);
    if (r.raster_vert == NULL || r.raster_frag == NULL) {
        snprintf(why, why_size, "the rasteriser's shaders: %s", SDL_GetError());
        return 0;
    }
    r.pipe_raster = render_pipeline(r.raster_vert, r.raster_frag, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM);
    r.mirror = render_texture(SDL_GPU_TEXTUREFORMAT_R16_UINT, VRAM_W, VRAM_H, SDL_GPU_TEXTUREUSAGE_SAMPLER);
    r.vram_download = render_buffer(SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, VRAM_W * VRAM_H * 4);
    if (r.pipe_raster == NULL || r.mirror == NULL || r.vram_download == NULL) {
        snprintf(why, why_size, "the rasteriser's pipeline and textures: %s", SDL_GetError());
        return 0;
    }
    /* The target and its background copy at the scale asked for (2 x 4 bytes x 1024 x 512 per scale squared: 256 MB
     * at 8), or at the largest scale below it that the device can allocate (logged). */
    scale = SDL_clamp(scale, 1, RENDER_MAX_SCALE);
    for (r.scale = scale; r.scale >= 1; r.scale--) {
        int w = VRAM_W * r.scale, h = VRAM_H * r.scale;
        r.vram_target = render_texture(SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, w, h,
                                       SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER);
        r.background = r.vram_target != NULL
                           ? render_texture(SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, w, h, SDL_GPU_TEXTUREUSAGE_SAMPLER)
                           : NULL;
        if (r.background != NULL) {
            break;
        }
        port_log("renderer: gpu: no %dx%d target at internal scale %d (%s)", w, h, r.scale, SDL_GetError());
        if (r.vram_target != NULL) {
            SDL_ReleaseGPUTexture(r.device, r.vram_target);
            r.vram_target = NULL;
        }
    }
    if (r.vram_target == NULL) {
        snprintf(why, why_size, "the rasteriser's target: %s", SDL_GetError());
        return 0;
    }
    if (r.scale != scale) {
        port_log("renderer: gpu: internal scale %d instead of %d", r.scale, scale);
    }
    r.generation = gpu_stamp_generation();
    memset(r.uploaded, 0xFF, sizeof(r.uploaded));
    memset(r.dirty, 1, sizeof(r.dirty));
    raster_load(0, 0, VRAM_W, VRAM_H, 0); /* the target starts as the VRAM is now */
    r.raster = 1;
    gpu_set_listener(raster_event);
    return 1;
}

int render_gpu_scale(void) {
    return r.raster ? r.scale : 1;
}

int render_gpu_rasterising(void) {
    return r.raster;
}

void render_gpu_raster_resync(void) {
    if (r.raster) {
        memset(r.uploaded, 0xFF, sizeof(r.uploaded));
        raster_load(0, 0, VRAM_W, VRAM_H, 0);
    }
}

/* ---- Running the frame's units ---- */

/* The recorded units into cb, in order, then the list starts over. */
static void raster_run(SDL_GPUCommandBuffer *cb) {
    SDL_GPURenderPass *pass = NULL;
    SDL_GPUCopyPass *copy = NULL;
    size_t i;

    if (r.nstaging > 0) {
        void *map;
        if (r.staging_buf == NULL || r.staging_buf_size < r.nstaging) {
            Uint32 size = r.staging_buf_size ? r.staging_buf_size : 1 << 20;
            while (size < r.nstaging) {
                size *= 2;
            }
            if (r.staging_buf != NULL) {
                SDL_ReleaseGPUTransferBuffer(r.device, r.staging_buf);
            }
            r.staging_buf = render_buffer(SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, size);
            r.staging_buf_size = r.staging_buf != NULL ? size : 0;
            if (r.staging_buf == NULL) {
                port_fatal("renderer: gpu: no upload buffer of %u bytes: %s", size, SDL_GetError());
            }
        }
        map = SDL_MapGPUTransferBuffer(r.device, r.staging_buf, true);
        if (map == NULL) {
            port_fatal("renderer: gpu: SDL_MapGPUTransferBuffer: %s", SDL_GetError());
        }
        memcpy(map, r.staging, r.nstaging);
        SDL_UnmapGPUTransferBuffer(r.device, r.staging_buf);
    }
    for (i = 0; i < r.nrecs; i++) {
        const Record *e = &r.recs[i];
        if (e->kind == REC_DRAW) {
            SDL_Rect scissor;
            if (copy != NULL) {
                SDL_EndGPUCopyPass(copy);
                copy = NULL;
            }
            if (pass == NULL) {
                SDL_GPUColorTargetInfo ct;
                SDL_GPUTextureSamplerBinding bind[2];
                memset(&ct, 0, sizeof(ct));
                ct.texture = r.vram_target;
                ct.load_op = SDL_GPU_LOADOP_LOAD;
                ct.store_op = SDL_GPU_STOREOP_STORE;
                pass = SDL_BeginGPURenderPass(cb, &ct, 1, NULL);
                SDL_BindGPUGraphicsPipeline(pass, r.pipe_raster);
                bind[0].texture = r.mirror;
                bind[0].sampler = r.sampler;
                bind[1].texture = r.background;
                bind[1].sampler = r.sampler;
                SDL_BindGPUFragmentSamplers(pass, 0, bind, 2);
            }
            scissor.x = e->x * r.scale;
            scissor.y = e->y * r.scale;
            scissor.w = e->w * r.scale;
            scissor.h = e->h * r.scale;
            SDL_SetGPUScissor(pass, &scissor);
            SDL_PushGPUVertexUniformData(cb, 0, &e->geometry, sizeof(e->geometry));
            SDL_PushGPUFragmentUniformData(cb, 0, &e->unit, sizeof(e->unit));
            SDL_DrawGPUPrimitives(pass, (Uint32)e->vertices, 1, 0, 0);
            continue;
        }
        if (pass != NULL) {
            SDL_EndGPURenderPass(pass);
            pass = NULL;
        }
        if (copy == NULL) {
            copy = SDL_BeginGPUCopyPass(cb);
        }
        if (e->kind == REC_UPLOAD) {
            SDL_GPUTextureTransferInfo src;
            SDL_GPUTextureRegion dst;
            memset(&src, 0, sizeof(src));
            src.transfer_buffer = r.staging_buf;
            src.offset = (Uint32)e->offset;
            src.pixels_per_row = (Uint32)e->w;
            src.rows_per_layer = (Uint32)e->h;
            memset(&dst, 0, sizeof(dst));
            dst.texture = r.mirror;
            dst.x = (Uint32)e->x;
            dst.y = (Uint32)e->y;
            dst.w = (Uint32)e->w;
            dst.h = (Uint32)e->h;
            dst.d = 1;
            SDL_UploadToGPUTexture(copy, &src, &dst, false);
        } else {
            SDL_GPUTextureLocation src, dst;
            memset(&src, 0, sizeof(src));
            memset(&dst, 0, sizeof(dst));
            src.texture = r.vram_target;
            dst.texture = r.background;
            src.x = dst.x = (Uint32)(e->x * r.scale);
            src.y = dst.y = (Uint32)(e->y * r.scale);
            SDL_CopyGPUTextureToTexture(copy, &src, &dst, (Uint32)(e->w * r.scale), (Uint32)(e->h * r.scale), 1,
                                        false);
        }
    }
    if (pass != NULL) {
        SDL_EndGPURenderPass(pass);
    }
    if (copy != NULL) {
        SDL_EndGPUCopyPass(copy);
    }
    r.nrecs = 0;
    r.nstaging = 0;
}

void render_gpu_frame(void) {
    SDL_GPUCommandBuffer *cb;
    if (!r.raster || r.nrecs == 0) {
        return;
    }
    cb = SDL_AcquireGPUCommandBuffer(r.device);
    if (cb == NULL) {
        port_fatal("renderer: gpu: SDL_AcquireGPUCommandBuffer: %s", SDL_GetError());
    }
    raster_run(cb);
    if (!SDL_SubmitGPUCommandBuffer(cb)) {
        port_fatal("renderer: gpu: SDL_SubmitGPUCommandBuffer: %s", SDL_GetError());
    }
}

int render_gpu_read_vram(u16 *out) {
    SDL_GPUCommandBuffer *cb;
    SDL_GPUCopyPass *copy;
    SDL_GPUTextureRegion region;
    SDL_GPUTextureTransferInfo info;
    SDL_GPUFence *fence;
    const u8 *map;
    int i;

    if (!r.raster || r.scale != 1) {
        return 0;
    }
    cb = SDL_AcquireGPUCommandBuffer(r.device);
    if (cb == NULL) {
        return 0;
    }
    raster_run(cb);
    memset(&region, 0, sizeof(region));
    region.texture = r.vram_target;
    region.w = VRAM_W;
    region.h = VRAM_H;
    region.d = 1;
    memset(&info, 0, sizeof(info));
    info.transfer_buffer = r.vram_download;
    info.pixels_per_row = VRAM_W;
    info.rows_per_layer = VRAM_H;
    copy = SDL_BeginGPUCopyPass(cb);
    SDL_DownloadFromGPUTexture(copy, &region, &info);
    SDL_EndGPUCopyPass(copy);
    fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cb);
    if (fence == NULL) {
        return 0;
    }
    SDL_WaitForGPUFences(r.device, true, &fence, 1);
    SDL_ReleaseGPUFence(r.device, fence);
    map = SDL_MapGPUTransferBuffer(r.device, r.vram_download, false);
    if (map == NULL) {
        return 0;
    }
    for (i = 0; i < VRAM_W * VRAM_H; i++) {
        const u8 *p = map + i * 4; /* R, G, B, A: each channel c << 3, the mask bit in A */
        out[i] = (u16)((p[0] >> 3) | ((p[1] >> 3) << 5) | ((p[2] >> 3) << 10) | (p[3] >= 128 ? 0x8000 : 0));
    }
    SDL_UnmapGPUTransferBuffer(r.device, r.vram_download);
    return 1;
}

/* ---- The present ---- */

/* The image's upload into r.image (a copy pass on cb). */
static int render_upload(SDL_GPUCommandBuffer *cb, const u32 *pixels, int w, int h) {
    SDL_GPUTextureTransferInfo src;
    SDL_GPUTextureRegion dst;
    SDL_GPUCopyPass *copy;
    void *map;

    if (w <= 0 || h <= 0 || w > RENDER_IMAGE_W || h > RENDER_IMAGE_H) {
        return 0;
    }
    map = SDL_MapGPUTransferBuffer(r.device, r.upload, true);
    if (map == NULL) {
        return 0;
    }
    memcpy(map, pixels, (size_t)w * (size_t)h * 4);
    SDL_UnmapGPUTransferBuffer(r.device, r.upload);
    memset(&src, 0, sizeof(src));
    src.transfer_buffer = r.upload;
    src.pixels_per_row = (Uint32)w;
    src.rows_per_layer = (Uint32)h;
    memset(&dst, 0, sizeof(dst));
    dst.texture = r.image;
    dst.w = (Uint32)w;
    dst.h = (Uint32)h;
    dst.d = 1;
    copy = SDL_BeginGPUCopyPass(cb);
    SDL_UploadToGPUTexture(copy, &src, &dst, true);
    SDL_EndGPUCopyPass(copy);
    return 1;
}

/* The present's render pass into `target`: black, then the picture at `rect`: the w x h image, or (vram_xy) the w x h
 * display at that corner of the VRAM target, at the internal scale. */
static void render_draw(SDL_GPUCommandBuffer *cb, SDL_GPUTexture *target, SDL_GPUGraphicsPipeline *pipe, int w, int h,
                        const int *vram_xy, const int rect[4]) {
    SDL_GPUColorTargetInfo ct;
    SDL_GPURenderPass *pass;
    SDL_GPUTextureSamplerBinding bind;
    SDL_Rect scissor;
    struct {
        Sint32 dst[4];
        Sint32 src[4];
        Sint32 disp[4];
    } u;

    memset(&ct, 0, sizeof(ct));
    ct.texture = target;
    ct.load_op = SDL_GPU_LOADOP_CLEAR;
    ct.store_op = SDL_GPU_STOREOP_STORE;
    ct.clear_color.a = 1.0f;
    pass = SDL_BeginGPURenderPass(cb, &ct, 1, NULL);
    if (rect[2] > 0 && rect[3] > 0) {
        SDL_BindGPUGraphicsPipeline(pass, pipe);
        scissor.x = rect[0];
        scissor.y = rect[1];
        scissor.w = rect[2];
        scissor.h = rect[3];
        SDL_SetGPUScissor(pass, &scissor);
        bind.texture = vram_xy != NULL ? r.vram_target : r.image;
        bind.sampler = r.sampler;
        SDL_BindGPUFragmentSamplers(pass, 0, &bind, 1);
        memset(&u, 0, sizeof(u));
        u.dst[0] = rect[0];
        u.dst[1] = rect[1];
        u.dst[2] = rect[2];
        u.dst[3] = rect[3];
        u.src[0] = w;
        u.src[1] = h;
        if (vram_xy != NULL) { /* the display at the internal scale, in the 1024 N x 512 N target */
            u.src[0] = w * r.scale;
            u.src[1] = h * r.scale;
            u.src[2] = VRAM_W * r.scale;
            u.src[3] = VRAM_H * r.scale;
            u.disp[0] = vram_xy[0] & 1023;
            u.disp[1] = vram_xy[1] & 511;
            u.disp[2] = r.scale;
        }
        SDL_PushGPUFragmentUniformData(cb, 0, &u, vram_xy != NULL ? sizeof(u) : sizeof(Sint32) * 8);
        SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
    }
    SDL_EndGPURenderPass(pass);
}

/* The rectangle clipped to the output (a window smaller than the image's lines gives video.c's scaled-down rectangle,
 * whose position may round outside). */
static void render_clip(int rect[4], int ow, int oh) {
    if (rect[0] < 0) {
        rect[2] += rect[0];
        rect[0] = 0;
    }
    if (rect[1] < 0) {
        rect[3] += rect[1];
        rect[1] = 0;
    }
    if (rect[0] + rect[2] > ow) {
        rect[2] = ow - rect[0];
    }
    if (rect[1] + rect[3] > oh) {
        rect[3] = oh - rect[1];
    }
}

/* The picture's source on cb: the VRAM target (the units run first) or the uploaded image. 0 on failure. */
static int render_source(SDL_GPUCommandBuffer *cb, const u32 *pixels, int w, int h, const int **vram_xy) {
    if (*vram_xy != NULL && r.raster) {
        raster_run(cb);
        return 1;
    }
    *vram_xy = NULL;
    return render_upload(cb, pixels, w, h);
}

int render_gpu_present(const u32 *pixels, int w, int h, const int *vram_xy, RenderDestFn dest) {
    SDL_GPUCommandBuffer *cb;
    SDL_GPUTexture *swap = NULL;
    Uint32 sw = 0, sh = 0;
    int rect[4] = { 0, 0, 0, 0 };

    if (r.device == NULL || r.window == NULL) {
        return 0;
    }
    cb = SDL_AcquireGPUCommandBuffer(r.device);
    if (cb == NULL) {
        goto failed;
    }
    if (!render_source(cb, pixels, w, h, &vram_xy)) {
        SDL_CancelGPUCommandBuffer(cb);
        goto failed;
    }
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cb, r.window, &swap, &sw, &sh)) {
        SDL_SubmitGPUCommandBuffer(cb); /* the units recorded into it still run */
        goto failed;
    }
    if (swap != NULL) { /* NULL: the window is minimised or occluded; nothing to draw this frame */
        SDL_GPUGraphicsPipeline *pipe = vram_xy != NULL ? (r.pipe_vram_swap ? r.pipe_vram_swap : r.pipe_vram_off)
                                                        : (r.pipe_swap ? r.pipe_swap : r.pipe_off);
        dest((int)sw, (int)sh, rect);
        render_clip(rect, (int)sw, (int)sh);
        render_draw(cb, swap, pipe, w, h, vram_xy, rect);
    }
    if (!SDL_SubmitGPUCommandBuffer(cb)) {
        goto failed;
    }
    return 1;
failed:
    if (!r.submit_failed) {
        port_log("renderer: gpu: a frame was not presented: %s", SDL_GetError());
        r.submit_failed = 1;
    }
    return 0;
}

int render_gpu_readback(const u32 *pixels, int w, int h, const int *vram_xy, int ow, int oh, RenderDestFn dest,
                        u32 *out) {
    SDL_GPUCommandBuffer *cb;
    SDL_GPUCopyPass *copy;
    SDL_GPUTextureRegion region;
    SDL_GPUTextureTransferInfo info;
    SDL_GPUFence *fence;
    const void *map;
    int rect[4] = { 0, 0, ow, oh };

    if (r.device == NULL || ow <= 0 || oh <= 0) {
        return 0;
    }
    if (r.target == NULL || r.target_w != ow || r.target_h != oh) {
        if (r.target != NULL) {
            SDL_ReleaseGPUTexture(r.device, r.target);
            SDL_ReleaseGPUTransferBuffer(r.device, r.download);
        }
        r.target = render_texture(SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM, ow, oh,
                                  SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER);
        r.download = render_buffer(SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, (Uint32)ow * (Uint32)oh * 4);
        r.target_w = ow;
        r.target_h = oh;
        if (r.target == NULL || r.download == NULL) {
            port_log("renderer: gpu: no %dx%d readback target: %s", ow, oh, SDL_GetError());
            return 0;
        }
    }
    cb = SDL_AcquireGPUCommandBuffer(r.device);
    if (cb == NULL) {
        return 0;
    }
    if (!render_source(cb, pixels, w, h, &vram_xy)) {
        SDL_CancelGPUCommandBuffer(cb);
        return 0;
    }
    if (dest != NULL) {
        dest(ow, oh, rect);
        render_clip(rect, ow, oh);
    }
    render_draw(cb, r.target, vram_xy != NULL ? r.pipe_vram_off : r.pipe_off, w, h, vram_xy, rect);
    memset(&region, 0, sizeof(region));
    region.texture = r.target;
    region.w = (Uint32)ow;
    region.h = (Uint32)oh;
    region.d = 1;
    memset(&info, 0, sizeof(info));
    info.transfer_buffer = r.download;
    info.pixels_per_row = (Uint32)ow;
    info.rows_per_layer = (Uint32)oh;
    copy = SDL_BeginGPUCopyPass(cb);
    SDL_DownloadFromGPUTexture(copy, &region, &info);
    SDL_EndGPUCopyPass(copy);
    fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cb);
    if (fence == NULL) {
        return 0;
    }
    SDL_WaitForGPUFences(r.device, true, &fence, 1);
    SDL_ReleaseGPUFence(r.device, fence);
    map = SDL_MapGPUTransferBuffer(r.device, r.download, false);
    if (map == NULL) {
        return 0;
    }
    memcpy(out, map, (size_t)ow * (size_t)oh * 4); /* B, G, R, A bytes: 0xAARRGGBB words, A = 255 */
    SDL_UnmapGPUTransferBuffer(r.device, r.download);
    return 1;
}

void render_gpu_close(void) {
    if (r.device != NULL) {
        SDL_WaitForGPUIdle(r.device);
    }
    render_release();
}
#endif
