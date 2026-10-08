// The present of a 15-bit display from the rasteriser's VRAM target (port/src/render_gpu.c): the display area (disp.xy
// in VRAM pixels, wrapping at the VRAM's edges) at the internal scale N (disp.z; the target is 1024 N x 512 N, src.zw
// its size) into the rectangle dst. The target holds each channel in 8-bit units (raster.frag.hlsl: at
// scale 1 a 5-bit value << 3), shown as v + (v >> 5), which is video.c's (c << 3) | (c >> 2) for v = c << 3: at scale
// 1 the picture is the software path's. When the rectangle is at least the display's size at the internal scale (and
// always at scale 1, as the software path), each output pixel reads the one image pixel its centre falls on
// (present.frag.hlsl's integer mapping); when it is smaller, the average of the image pixels whose centres fall in it
// (at most 8 x 8): a higher internal scale shown in a smaller window is supersampled.
Texture2D<float4> target : register(t0, space2);
cbuffer Present : register(b0, space3) {
    int4 dst;  // the rectangle in the output: x, y, w, h
    int4 src;  // the image: the display's size at the internal scale (w, h); the target's size (w, h)
    int4 disp; // the display area's corner in the VRAM (x, y), the internal scale, unused
};

float3 image(int2 t) {
    int N = disp.z;
    float4 c = target.Load(int3((disp.x * N + t.x) % src.z, (disp.y * N + t.y) % src.w, 0));
    float3 v = floor(c.rgb * 255.0 + 0.5);
    return min(v + floor(v / 32.0), 255.0);
}

float4 main(float4 pos : SV_Position) : SV_Target0 {
    int2 p = int2(pos.xy) - dst.xy;
    if (disp.z == 1 || (dst.z >= src.x && dst.w >= src.y)) {
        int2 t = ((2 * p + 1) * src.xy) / (2 * dst.zw);
        return float4(image(t) / 255.0, 1.0);
    }
    // The image pixels whose centres fall in output pixel p: t with p <= (t + 0.5) * dst / src < p + 1.
    int2 lo = (2 * p * src.xy + dst.zw - 1) / (2 * dst.zw);
    int2 hi = ((2 * p + 2) * src.xy + dst.zw - 1) / (2 * dst.zw);
    hi = max(hi, lo + 1);
    float3 sum = float3(0.0, 0.0, 0.0);
    int n = 0;
    for (int y = lo.y; y < hi.y && y < lo.y + 8; y++) {
        for (int x = lo.x; x < hi.x && x < lo.x + 8; x++) {
            sum += image(int2(x, y));
            n++;
        }
    }
    return float4(floor(sum / n + 0.5) / 255.0, 1.0);
}
