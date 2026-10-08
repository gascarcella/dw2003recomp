// The present's vertex shader (port/runtime/render_gpu.c): one triangle that covers the whole target; the scissor keeps
// the image's rectangle. No vertex buffer: the corners come from the vertex index.
float4 main(uint id : SV_VertexID) : SV_Position {
    float2 t = float2((id << 1) & 2, id & 2);
    return float4(t * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
}
