/* The hardware renderer (render_gpu.h; issue #31, docs/PORT.md "Rendering"): SDL_GPU on Vulkan, only in the
 * DW3_PORT_SDL build. Phase 1: the device, the window's swapchain, and the present of the software image: video.c's
 * 32-bit display image is uploaded into a texture and drawn into the swapchain by one shader pair
 * (port/shaders/present.*.hlsl, compiled to SPIR-V at build time), nearest-scaled with integer arithmetic into
 * video.c's 4:3 rectangle, so the picture is the SDL_Renderer path's pixel for pixel. The same draw into an offscreen
 * target, read back, is the hardware screenshot (`--gpu-screenshot`).
 *
 * Measured on SDL 3.4.18 (issue #31, the plan's "What was verified"): on the offscreen video driver NVIDIA's Vulkan
 * driver gives a device but no presentable surface (claiming the window fails: video.c falls back), while Mesa's
 * lavapipe presents to it (VK_EXT_headless_surface: tests/port/render_gpu.py runs the window path headless there);
 * a window that had an OpenGL SDL_Renderer cannot
 * be claimed by Vulkan afterwards on Wayland, while a released claim leaves the window usable by SDL_Renderer (so
 * render_gpu_open comes first and releases everything on failure); destroying the device before SDL_Quit exits
 * cleanly on NVIDIA (the EGL teardown crash video.c avoids). The swapchain is SDR (8-bit, not sRGB-encoded: the
 * bytes pass through), presented in mailbox mode when the window supports it, else immediate, else vsync: the
 * SDL_Renderer path presents without vsync and pump.c paces the frames. */
#ifdef DW3_PORT_SDL
#include <stdio.h>
#include <string.h>

#include "port_harness.h"
#include "render_gpu.h"

#include "present_frag_spv.h"
#include "present_vert_spv.h"

#define RENDER_IMAGE_W 640 /* video.c's VIDEO_MAX_W, VIDEO_MAX_H: the largest display image */
#define RENDER_IMAGE_H 576

static struct {
    SDL_GPUDevice *device;
    SDL_Window *window;                    /* claimed; NULL headless */
    SDL_GPUTextureFormat swap_format;      /* the swapchain's (SDR: B8G8R8A8 or R8G8B8A8 UNORM) */
    SDL_GPUShader *vert, *frag;
    SDL_GPUGraphicsPipeline *pipe_swap;    /* the present into the swapchain's format */
    SDL_GPUGraphicsPipeline *pipe_off;     /* into B8G8R8A8_UNORM (the readback target) */
    SDL_GPUSampler *sampler;               /* bound with the image (the shader reads it with Load) */
    SDL_GPUTexture *image;                 /* the software image, RENDER_IMAGE_W x RENDER_IMAGE_H */
    SDL_GPUTransferBuffer *upload;         /* its pixels on the way */
    SDL_GPUTexture *target;                /* the readback's offscreen target and its download buffer */
    SDL_GPUTransferBuffer *download;
    int target_w, target_h;
    int submit_failed;
    char describe[160];
} r;

static void render_release(void) {
    if (r.device != NULL) {
        if (r.download != NULL) {
            SDL_ReleaseGPUTransferBuffer(r.device, r.download);
        }
        if (r.target != NULL) {
            SDL_ReleaseGPUTexture(r.device, r.target);
        }
        if (r.upload != NULL) {
            SDL_ReleaseGPUTransferBuffer(r.device, r.upload);
        }
        if (r.image != NULL) {
            SDL_ReleaseGPUTexture(r.device, r.image);
        }
        if (r.sampler != NULL) {
            SDL_ReleaseGPUSampler(r.device, r.sampler);
        }
        if (r.pipe_off != NULL) {
            SDL_ReleaseGPUGraphicsPipeline(r.device, r.pipe_off);
        }
        if (r.pipe_swap != NULL) {
            SDL_ReleaseGPUGraphicsPipeline(r.device, r.pipe_swap);
        }
        if (r.frag != NULL) {
            SDL_ReleaseGPUShader(r.device, r.frag);
        }
        if (r.vert != NULL) {
            SDL_ReleaseGPUShader(r.device, r.vert);
        }
        if (r.window != NULL) {
            SDL_ReleaseWindowFromGPUDevice(r.device, r.window);
        }
        SDL_DestroyGPUDevice(r.device);
    }
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

static SDL_GPUGraphicsPipeline *render_pipeline(SDL_GPUTextureFormat format) {
    SDL_GPUGraphicsPipelineCreateInfo ci;
    SDL_GPUColorTargetDescription target;
    memset(&ci, 0, sizeof(ci));
    memset(&target, 0, sizeof(target));
    target.format = format;
    ci.vertex_shader = r.vert;
    ci.fragment_shader = r.frag;
    ci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    ci.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    ci.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    ci.target_info.color_target_descriptions = &target;
    ci.target_info.num_color_targets = 1;
    return SDL_CreateGPUGraphicsPipeline(r.device, &ci);
}

static int render_fail(char *why, size_t why_size, const char *what) {
    snprintf(why, why_size, "%s: %s", what, SDL_GetError());
    render_release();
    return 0;
}

int render_gpu_open(SDL_Window *window, char *why, size_t why_size) {
    SDL_GPUTextureCreateInfo ti;
    SDL_GPUTransferBufferCreateInfo bi;
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
    if (r.vert == NULL || r.frag == NULL) {
        return render_fail(why, why_size, "SDL_CreateGPUShader");
    }
    r.pipe_off = render_pipeline(SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM);
    if (r.pipe_off == NULL) {
        return render_fail(why, why_size, "SDL_CreateGPUGraphicsPipeline");
    }
    if (window != NULL) {
        r.pipe_swap = r.swap_format == SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM ? NULL : render_pipeline(r.swap_format);
        if (r.swap_format != SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM && r.pipe_swap == NULL) {
            return render_fail(why, why_size, "SDL_CreateGPUGraphicsPipeline (swapchain)");
        }
    }
    memset(&si, 0, sizeof(si));
    si.min_filter = SDL_GPU_FILTER_NEAREST;
    si.mag_filter = SDL_GPU_FILTER_NEAREST;
    si.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    si.address_mode_u = si.address_mode_v = si.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    r.sampler = SDL_CreateGPUSampler(r.device, &si);
    memset(&ti, 0, sizeof(ti));
    ti.type = SDL_GPU_TEXTURETYPE_2D;
    ti.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM; /* 0xFFRRGGBB in memory: B, G, R, A */
    ti.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    ti.width = RENDER_IMAGE_W;
    ti.height = RENDER_IMAGE_H;
    ti.layer_count_or_depth = 1;
    ti.num_levels = 1;
    r.image = SDL_CreateGPUTexture(r.device, &ti);
    memset(&bi, 0, sizeof(bi));
    bi.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    bi.size = RENDER_IMAGE_W * RENDER_IMAGE_H * 4;
    r.upload = SDL_CreateGPUTransferBuffer(r.device, &bi);
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

/* The present's render pass into `target` (tw x th): black, then the image at `rect`. */
static void render_draw(SDL_GPUCommandBuffer *cb, SDL_GPUTexture *target, SDL_GPUGraphicsPipeline *pipe, int w, int h,
                        const int rect[4]) {
    SDL_GPUColorTargetInfo ct;
    SDL_GPURenderPass *pass;
    SDL_GPUTextureSamplerBinding bind;
    SDL_Rect scissor;
    struct {
        Sint32 dst[4];
        Sint32 src[4];
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
        bind.texture = r.image;
        bind.sampler = r.sampler;
        SDL_BindGPUFragmentSamplers(pass, 0, &bind, 1);
        u.dst[0] = rect[0];
        u.dst[1] = rect[1];
        u.dst[2] = rect[2];
        u.dst[3] = rect[3];
        u.src[0] = w;
        u.src[1] = h;
        u.src[2] = u.src[3] = 0;
        SDL_PushGPUFragmentUniformData(cb, 0, &u, sizeof(u));
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

int render_gpu_present(const u32 *pixels, int w, int h, RenderDestFn dest) {
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
    if (!render_upload(cb, pixels, w, h)) {
        SDL_CancelGPUCommandBuffer(cb);
        goto failed;
    }
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cb, r.window, &swap, &sw, &sh)) {
        SDL_CancelGPUCommandBuffer(cb);
        goto failed;
    }
    if (swap != NULL) { /* NULL: the window is minimised or occluded; nothing to draw this frame */
        dest((int)sw, (int)sh, rect);
        render_clip(rect, (int)sw, (int)sh);
        render_draw(cb, swap, r.pipe_swap != NULL ? r.pipe_swap : r.pipe_off, w, h, rect);
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

int render_gpu_readback(const u32 *pixels, int w, int h, int ow, int oh, RenderDestFn dest, u32 *out) {
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
        SDL_GPUTextureCreateInfo ti;
        SDL_GPUTransferBufferCreateInfo bi;
        if (r.target != NULL) {
            SDL_ReleaseGPUTexture(r.device, r.target);
            SDL_ReleaseGPUTransferBuffer(r.device, r.download);
        }
        memset(&ti, 0, sizeof(ti));
        ti.type = SDL_GPU_TEXTURETYPE_2D;
        ti.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
        ti.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        ti.width = (Uint32)ow;
        ti.height = (Uint32)oh;
        ti.layer_count_or_depth = 1;
        ti.num_levels = 1;
        r.target = SDL_CreateGPUTexture(r.device, &ti);
        memset(&bi, 0, sizeof(bi));
        bi.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
        bi.size = (Uint32)ow * (Uint32)oh * 4;
        r.download = SDL_CreateGPUTransferBuffer(r.device, &bi);
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
    if (!render_upload(cb, pixels, w, h)) {
        SDL_CancelGPUCommandBuffer(cb);
        return 0;
    }
    if (dest != NULL) {
        dest(ow, oh, rect);
        render_clip(rect, ow, oh);
    }
    render_draw(cb, r.target, r.pipe_off, w, h, rect);
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
