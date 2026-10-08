// The rasteriser's pixel shader (port/runtime/render_gpu.c; issue #31): one pixel of a unit of gpu.c's command stream into
// the VRAM target, computed as port/psyq/gpu.c's gpu_pixel computes it, in integers. The target is the VRAM at the
// internal scale N (res.x): a VRAM pixel is N x N target pixels. Each channel is held in 8-bit units, the mask bit in
// alpha: at scale 1 exactly gpu.c's 5-bit value << 3, so every write is exact; above it gpu.c's 8-bit pipeline without
// dithering, the low bits kept. Blending, dithering and the mask test read the background from a copy of the target
// (`background`, refreshed by the renderer before every unit that reads it), never from the fixed-function blender
// (DECISIONS "The hardware renderer"). Texels and CLUTs come from the copy of the software VRAM (`vram`, R16_UINT, at
// scale 1), uploaded in stream order.
Texture2D<uint> vram : register(t0, space2);
Texture2D<float4> background : register(t1, space2);
cbuffer Unit : register(b0, space3) {
    int4 kind; // x: the operation (OP_*), y: flags (F_*), z: the blend mode (0..3), w: the texture depth (0, 1..3)
    int4 a;    // triangle: a.x, a.y, a.u, a.v; rectangle: x0, y0, u0, v0; fill: the colour (x)
    int4 col;  // r, g, b, den (a triangle's doubled-area determinant, made positive)
    int4 nrg;  // the planes' numerators: r (n0, n1), g (n0, n1)
    int4 nbu;  // b (n0, n1), u (n0, n1)
    int4 nva;  // v (n0, n1), the drawing area's x0, unused
    int4 tex;  // the texture page's x, y; the CLUT's x, y
    int4 win;  // u_and, u_or, v_and, v_or
    int4 s01;  // the triangle's vertices sorted by y (gpu.c's order): v0.x, v0.y, v1.x, v1.y
    int4 s2;   // v2.x, v2.y; a copy's source offset in VRAM pixels (dx, dy)
    int4 res;  // the internal scale N, the target's width and height (1024 N, 512 N), unused
    int4 uvr;  // the triangle's vertices' u range (min, max) and v range (min, max)
    int4 cmin; // their colours' minimum (r, g, b), unused
    int4 cmax; // and maximum
};

#define OP_TRIANGLE 0
#define OP_RECT 1
#define OP_FILL 2
#define OP_COPY 3
#define OP_LOAD 4

#define F_RAW 1
#define F_SEMI 2
#define F_GOURAUD 4
#define F_DITHER 8
#define F_SET_MASK 16
#define F_CHECK_MASK 32
#define F_PAIRS 64
#define F_BACKGROUND 128

static const int dither_table[16] = { -4, 0, -3, 1, 2, -2, 3, -1, -3, 1, -4, 0, 3, -1, 2, -2 };

// floor(n / d) for d > 0 (the shader's integer division truncates).
int floordiv(int n, int d) {
    int q = n / d;
    return q * d > n ? q - 1 : q;
}

int ceildiv(int n, int d) {
    return -floordiv(-n, d);
}

// The target pixel's place relative to the triangle's vertex a, in VRAM pixels: I + R / (2N), with 0 <= R < 2N. The
// target pixel P samples the VRAM at (P + 0.5) / N - 0.5 (at scale 1: the VRAM pixel itself).
static int2 rel_i, rel_r;

void relative(int2 P, int N) {
    int2 X = 2 * P + 1 - N - 2 * N * a.xy;
    rel_i = int2(floordiv(X.x, 2 * N), floordiv(X.y, 2 * N));
    rel_r = X - 2 * N * rel_i;
}

// An attribute at the sample point: a + floor(plane / den + 1/2), gpu.c's interpolation (at scale 1 exactly its
// a + floor((2 * plane + den) / (2 * den))), in 32-bit integers at any scale up to 8: the plane's whole-pixel part is
// gpu.c's own (it fits, by gpu.c's bound), split into quotient and remainder by den before the fraction is added.
int attr(int base, int n0, int n1, int N) {
    int t = rel_i.x * n0 + rel_i.y * n1;
    int qi = floordiv(t, col.w);
    int ri = t - qi * col.w;
    return base + qi + floordiv(2 * N * ri + N * col.w + rel_r.x * n0 + rel_r.y * n1, 2 * N * col.w);
}

// The first pixel gpu.c draws on row y of the triangle (its span start: the left edge's ceiling, the drawing area).
int span_start(int y) {
    int lden = s2.y - s01.y, ldx = s2.x - s01.x;
    int lnum = s01.x * lden + (y - s01.y) * ldx;
    bool upper = y < s01.w;
    int2 e0 = upper ? s01.xy : s01.zw;
    int2 e1 = upper ? s01.zw : s2.xy;
    int sden = e1.y - e0.y, sdx = e1.x - e0.x;
    int snum = e0.x * sden + (y - e0.y) * sdx;
    int xl = lnum * sden < snum * lden ? ceildiv(lnum, lden) : ceildiv(snum, sden);
    return max(xl, nva.z);
}

uint texel(int u, int v) {
    int row = (tex.y + v) & 511;
    if (kind.w == 1) {
        uint px = vram.Load(int3((tex.x + (u >> 2)) & 1023, row, 0));
        return vram.Load(int3((tex.z + int((px >> ((u & 3) * 4)) & 0xF)) & 1023, tex.w, 0));
    }
    if (kind.w == 2) {
        uint px = vram.Load(int3((tex.x + (u >> 1)) & 1023, row, 0));
        return vram.Load(int3((tex.z + int((px >> ((u & 1) * 8)) & 0xFF)) & 1023, tex.w, 0));
    }
    return vram.Load(int3((tex.x + u) & 1023, row, 0));
}

// The target's pixel (the background copy): the channels in 8-bit units, the mask bit.
static int3 bg8;
static bool bg_mask;

void read_background(int2 P) {
    float4 c = background.Load(int3(P, 0));
    bg8 = int3(c.rgb * 255.0 + 0.5);
    bg_mask = c.a > 0.5;
}

float4 out8(int3 c8, bool mask) {
    return float4(float3(clamp(c8, 0, 255)) / 255.0, mask ? 1.0 : 0.0);
}

// A 16-bit VRAM pixel into the target (each 5-bit channel << 3).
float4 out16(uint p) {
    return out8(int3(int(p & 31u), int((p >> 5) & 31u), int((p >> 10) & 31u)) << 3, (p & 0x8000u) != 0);
}

int blend(int abr, int b, int f, int hi) {
    int v = abr == 0 ? (b + f) >> 1 : abr == 1 ? b + f : abr == 2 ? b - f : b + (f >> 2);
    return clamp(v, 0, hi);
}

float4 main(float4 pos : SV_Position) : SV_Target0 {
    int N = res.x;
    int2 P = int2(pos.xy);
    int2 p = P / N; // the VRAM pixel
    int op = kind.x, flags = kind.y, abr = kind.z;
    bool hires = N > 1;
    bool set_mask = (flags & F_SET_MASK) != 0;

    bg8 = int3(0, 0, 0);
    bg_mask = false;
    if (flags & F_BACKGROUND) {
        read_background(P);
    }
    if ((flags & F_CHECK_MASK) && bg_mask) {
        discard;
    }
    if (op == OP_FILL) {
        return out16(uint(a.x));
    }
    if (op == OP_COPY) {
        read_background(int2((P.x + s2.z * N) % res.y, (P.y + s2.w * N) % res.z));
        return out8(bg8, bg_mask || set_mask);
    }
    if (op == OP_LOAD) {
        return out16(vram.Load(int3(p, 0)));
    }

    int3 c = col.rgb;
    int u = 0, v = 0;
    if (op == OP_TRIANGLE) {
        relative(P, N);
        if (kind.w != 0) {
            u = attr(a.z, nbu.z, nbu.w, N);
            v = attr(a.w, nva.x, nva.y, N);
            if (hires) {
                // A sample point near an edge may lie outside the triangle (at scale 1 never: gpu.c's values stay in
                // the vertices' range): kept in range, no texel from beside the primitive's own.
                u = clamp(u, uvr.x, uvr.y);
                v = clamp(v, uvr.z, uvr.w);
            }
        }
        if (flags & F_GOURAUD) {
            // Gouraud-textured primitives of the plain pipeline take their colour per pair of pixels from the span's
            // start (gpu.c gpu_span_gouraud_tri; F_PAIRS is set at scale 1 only).
            if ((flags & F_PAIRS) && ((p.x - span_start(p.y)) & 1)) {
                relative(P - int2(1, 0), N);
            }
            c = int3(attr(col.r, nrg.x, nrg.y, N), attr(col.g, nrg.z, nrg.w, N), attr(col.b, nbu.x, nbu.y, N));
            if (hires) {
                c = clamp(c, cmin.rgb, cmax.rgb);
            }
        }
    } else {
        u = a.z + (p.x - a.x);
        v = a.w + (p.y - a.y);
    }

    bool semi = (flags & F_SEMI) != 0;
    // gpu.c's 8-bit pipeline: the dithered primitives at scale 1, every primitive above it (no dithering there).
    bool eight = (flags & F_DITHER) != 0 || hires;
    bool stp = false;
    int3 f;
    if (kind.w != 0) {
        uint t = texel((u & win.x) | win.y, (v & win.z) | win.w);
        if (t == 0) {
            discard;
        }
        int3 tc = int3(int(t & 31u), int((t >> 5) & 31u), int((t >> 10) & 31u));
        stp = (t & 0x8000u) != 0;
        semi = semi && stp;
        if (eight) {
            f = (flags & F_RAW) ? tc << 3 : (tc * c) >> 4;
        } else if (flags & F_RAW) {
            f = tc;
        } else if (semi && abr == 3) {
            // Mode 3: the quarter of the modulated texel, added to the background.
            f = (flags & F_GOURAUD) ? (tc * c) >> 9 : ((tc >> 2) * c) >> 7;
            return out8(clamp((bg8 >> 3) + f, 0, 31) << 3, stp || set_mask);
        } else {
            f = (tc * c) >> 7;
        }
    } else {
        f = eight ? c : c >> 3;
    }
    if (eight) {
        int d = hires ? 0 : dither_table[(p.y & 3) * 4 + (p.x & 3)];
        if (semi) {
            f = int3(blend(abr, bg8.r, f.r, 255), blend(abr, bg8.g, f.g, 255), blend(abr, bg8.b, f.b, 255));
        }
        f = clamp(f + d, 0, 255);
        return out8(hires ? f : (f >> 3) << 3, stp || set_mask);
    }
    if (semi) {
        int3 b5 = bg8 >> 3;
        f = int3(blend(abr, b5.r, f.r, 31), blend(abr, b5.g, f.g, 31), blend(abr, b5.b, f.b, 31));
    } else {
        f = min(f, 31);
    }
    return out8(f << 3, stp || set_mask);
}
