// The present's pixel shader (port/runtime/render_gpu.c): the image (src.xy pixels) nearest-scaled into the rectangle
// dst (x, y, w, h) of the target, in integers so that the result is defined exactly: target pixel p (its centre
// p + 0.5) reads image pixel floor((p - dst.xy + 0.5) * src.xy / dst.zw). Read with Load: no filtering, no sampler
// state involved. SDL_GPU's binding layout (SDL_gpu.h, SDL_CreateGPUShader): fragment textures t[n] in space2,
// fragment uniform buffers b[n] in space3.
Texture2D<float4> image : register(t0, space2);
cbuffer Present : register(b0, space3) {
    int4 dst; // the rectangle in the target: x, y, w, h
    int4 src; // the image's size: w, h, unused, unused
};

float4 main(float4 pos : SV_Position) : SV_Target0 {
    int2 p = int2(pos.xy) - dst.xy;
    int2 t = ((2 * p + 1) * src.xy) / (2 * dst.zw);
    return float4(image.Load(int3(t, 0)).rgb, 1.0);
}
