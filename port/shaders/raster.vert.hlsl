// The rasteriser's vertex shader (port/src/render_gpu.c): no vertex buffer, the corners come from the uniforms.
// mode.x 0: a triangle, three vertices (v01.xy, v01.zw, v2.xy) in VRAM pixels, shifted by half a pixel so that the
// rasteriser's pixel centres (x + 0.5) cover what gpu.c's span rule covers at x (the top-left rule is gpu.c's);
// mode.x 1: a rectangle, two triangles over [rect.x, rect.z) x [rect.y, rect.w) (whole pixels, no shift).
// target: the target's size in its own pixels, the internal scale (VRAM pixels to target pixels), the shift.
cbuffer Geometry : register(b0, space1) {
    float4 v01;
    float4 v2;
    float4 rect;
    float4 target; // xy: the target's size in pixels, z: the internal scale, w: the triangle's shift (0.5)
    int4 mode;
};

static const uint corner[6] = { 0, 1, 2, 2, 1, 3 };

float4 main(uint id : SV_VertexID) : SV_Position {
    float2 p;
    if (mode.x == 0) {
        p = (id == 0 ? v01.xy : id == 1 ? v01.zw : v2.xy) + target.w;
    } else {
        uint c = corner[id];
        p = float2((c & 1) ? rect.z : rect.x, (c & 2) ? rect.w : rect.y);
    }
    p *= target.z;
    return float4(p.x / target.x * 2.0 - 1.0, 1.0 - p.y / target.y * 2.0, 0.0, 1.0);
}
