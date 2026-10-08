// The rasteriser's pixel shader (port/src/render_gpu.c; issue #31): one pixel of a unit of gpu.c's command stream into
// the VRAM target, computed as port/psyq/gpu.c's gpu_pixel computes it, in integers: the target holds each 5-bit
// channel as c << 3 and the mask bit in alpha, so every write is exact. Blending, dithering and the mask test read
// the background from a copy of the target (`background`, refreshed by the renderer before every unit that reads it),
// never from the fixed-function blender (DECISIONS "The hardware renderer"). Texels and CLUTs come from the copy of
// the software VRAM (`vram`, R16_UINT), uploaded in stream order.
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
    int4 s2;   // v2.x, v2.y; a copy's source offset (dx, dy)
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

// An attribute's value at pixel (x, y): a + floor((2 * plane + den) / (2 * den)), gpu.c's interpolation.
int attr(int base, int n0, int n1, int dx, int dy) {
    return base + floordiv(2 * (dx * n0 + dy * n1) + col.w, 2 * col.w);
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

// The target's pixel as gpu.c's 16-bit value (the background copy).
uint target_at(int2 p) {
    float4 c = background.Load(int3(p, 0));
    uint3 c8 = uint3(c.rgb * 255.0 + 0.5);
    return (c8.r >> 3) | ((c8.g >> 3) << 5) | ((c8.b >> 3) << 10) | (c.a > 0.5 ? 0x8000u : 0u);
}

float4 pixel_out(uint p) {
    uint3 c = uint3(p & 31u, (p >> 5) & 31u, (p >> 10) & 31u) << 3;
    return float4(float3(c) / 255.0, (p & 0x8000u) ? 1.0 : 0.0);
}

int blend(int abr, int b, int f, int hi) {
    int v = abr == 0 ? (b + f) >> 1 : abr == 1 ? b + f : abr == 2 ? b - f : b + (f >> 2);
    return clamp(v, 0, hi);
}

float4 main(float4 pos : SV_Position) : SV_Target0 {
    int2 p = int2(pos.xy);
    int op = kind.x, flags = kind.y, abr = kind.z;
    uint set_mask = (flags & F_SET_MASK) ? 0x8000u : 0u;
    uint bg = (flags & F_BACKGROUND) ? target_at(p) : 0u;

    if ((flags & F_CHECK_MASK) && (bg & 0x8000u)) {
        discard;
    }
    if (op == OP_FILL) {
        return pixel_out(uint(a.x));
    }
    if (op == OP_COPY) {
        return pixel_out(target_at(int2((p.x + s2.z) & 1023, (p.y + s2.w) & 511)) | set_mask);
    }
    if (op == OP_LOAD) {
        return pixel_out(vram.Load(int3(p, 0)));
    }

    int3 c = col.rgb;
    int u = 0, v = 0;
    if (op == OP_TRIANGLE) {
        int dx = p.x - a.x, dy = p.y - a.y;
        if (flags & F_GOURAUD) {
            // Gouraud-textured primitives of the plain pipeline take their colour per pair of pixels from the span's
            // start (gpu.c gpu_span_gouraud_tri).
            int cdx = (flags & F_PAIRS) ? dx - ((p.x - span_start(p.y)) & 1) : dx;
            c = int3(attr(col.r, nrg.x, nrg.y, cdx, dy), attr(col.g, nrg.z, nrg.w, cdx, dy),
                     attr(col.b, nbu.x, nbu.y, cdx, dy));
        }
        if (kind.w != 0) {
            u = attr(a.z, nbu.z, nbu.w, dx, dy);
            v = attr(a.w, nva.x, nva.y, dx, dy);
        }
    } else {
        u = a.z + (p.x - a.x);
        v = a.w + (p.y - a.y);
    }

    bool semi = (flags & F_SEMI) != 0;
    bool eight = (flags & F_DITHER) != 0;
    uint stp = 0;
    int3 f;
    if (kind.w != 0) {
        uint t = texel((u & win.x) | win.y, (v & win.z) | win.w);
        if (t == 0) {
            discard;
        }
        int3 tc = int3(t & 31u, (t >> 5) & 31u, (t >> 10) & 31u);
        stp = t & 0x8000u;
        semi = semi && stp != 0;
        if (eight) {
            f = (flags & F_RAW) ? tc << 3 : (tc * c) >> 4;
        } else if (flags & F_RAW) {
            f = tc;
        } else if (semi && abr == 3) {
            // Mode 3: the quarter of the modulated texel, added to the background.
            f = (flags & F_GOURAUD) ? (tc * c) >> 9 : ((tc >> 2) * c) >> 7;
            int3 b3 = int3(bg & 31u, (bg >> 5) & 31u, (bg >> 10) & 31u);
            int3 r3 = clamp(b3 + f, 0, 31);
            return pixel_out(uint(r3.r) | (uint(r3.g) << 5) | (uint(r3.b) << 10) | stp | set_mask);
        } else {
            f = (tc * c) >> 7;
        }
    } else {
        f = eight ? c : c >> 3;
    }
    int3 b5 = int3(bg & 31u, (bg >> 5) & 31u, (bg >> 10) & 31u);
    if (eight) {
        int d = dither_table[(p.y & 3) * 4 + (p.x & 3)];
        if (semi) {
            f = int3(blend(abr, b5.r << 3, f.r, 255), blend(abr, b5.g << 3, f.g, 255), blend(abr, b5.b << 3, f.b, 255));
        }
        f = clamp(f + d, 0, 255) >> 3;
    } else if (semi) {
        f = int3(blend(abr, b5.r, f.r, 31), blend(abr, b5.g, f.g, 31), blend(abr, b5.b, f.b, 31));
    } else {
        f = min(f, 31);
    }
    return pixel_out(uint(f.r) | (uint(f.g) << 5) | (uint(f.b) << 10) | stp | set_mask);
}
