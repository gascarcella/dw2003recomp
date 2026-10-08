// The present of a 15-bit display from the rasteriser's VRAM target (port/src/render_gpu.c): the display area
// (disp.xy, wrapping at the VRAM's edges) nearest-scaled into the rectangle dst as present.frag.hlsl maps it, each
// 5-bit channel widened as video.c widens it ((c << 3) | (c >> 2)): the picture is the software path's when the
// target equals the VRAM. The target holds c << 3 per channel (raster.frag.hlsl).
Texture2D<float4> target : register(t0, space2);
cbuffer Present : register(b0, space3) {
    int4 dst;  // the rectangle in the output: x, y, w, h
    int4 src;  // the display's size: w, h, unused, unused
    int4 disp; // the display area's corner in the VRAM: x, y, unused, unused
};

float4 main(float4 pos : SV_Position) : SV_Target0 {
    int2 p = int2(pos.xy) - dst.xy;
    int2 t = ((2 * p + 1) * src.xy) / (2 * dst.zw);
    float4 c = target.Load(int3((disp.x + t.x) & 1023, (disp.y + t.y) & 511, 0));
    uint3 c5 = uint3(c.rgb * 255.0 + 0.5) >> 3;
    return float4(float3((c5 << 3) | (c5 >> 2)) / 255.0, 1.0);
}
