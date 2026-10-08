/* The hardware renderer (render_gpu.c; issue #31, docs/PORT.md "Rendering"): SDL_GPU, only in the DW3_PORT_SDL build,
 * driven by video.c. It presents either video.c's 32-bit image (the software path's picture: 24-bit displays, the
 * disabled display) or, once the rasteriser runs, the 15-bit display cut from its own VRAM target, which it draws from
 * the software GPU's decoded command stream (port/psyq/gpu.c's listener). Every function but render_gpu_open is a
 * no-op until a device is open. */
#ifndef PORT_RENDER_GPU_H
#define PORT_RENDER_GPU_H

#ifdef DW3_PORT_SDL
#include <SDL3/SDL.h>

#include "port_runtime.h"

/* The image's rectangle (x, y, w, h) in an output of ow x oh pixels: video.c's 4:3 integer-scaled placement. */
typedef void (*RenderDestFn)(int ow, int oh, int rect[4]);

/* Opens a GPU device, claims `window` for it when not NULL (headless: offscreen work only, the screenshots), and
 * creates the present's shaders and pipelines. 0 on any failure, with the reason in `why` and everything released:
 * the caller falls back to the software path. A window that had an SDL_Renderer cannot be claimed afterwards
 * (Wayland), so video.c calls this first. */
int render_gpu_open(SDL_Window *window, char *why, size_t why_size);
int render_gpu_active(void);
/* "vulkan, NVIDIA GeForce ..." for the log; "" when closed. */
const char *render_gpu_describe(void);
/* The rasteriser: its VRAM target (1024 x 512 at the internal scale 1..8: a VRAM pixel is scale x scale target
 * pixels; a lower scale when the device cannot allocate it, logged), the copy of the software VRAM it samples, its
 * pipeline, and gpu.c's listener installed; the target starts as the VRAM is now. 0 on failure (why says it). */
int render_gpu_raster_start(int scale, char *why, size_t why_size);
int render_gpu_rasterising(void);
/* The internal scale in use (1 when not rasterising). */
int render_gpu_scale(void);
/* Runs the units recorded since the last call (once per vsync: the target is state, so every vsync's units run, also
 * when no picture is presented). */
void render_gpu_frame(void);
/* Into the window's swapchain, at dest's rectangle on black: `pixels` (w x h, 0xFFRRGGBB), or with `vram_xy` (and the
 * rasteriser running) the w x h display at that corner of the target. 0 when the frame could not be submitted
 * (logged once). */
int render_gpu_present(const u32 *pixels, int w, int h, const int *vram_xy, RenderDestFn dest);
/* The same present into an offscreen ow x oh target, read back into `out` (ow x oh, 0xFFRRGGBB); dest NULL: the whole
 * target (ow x oh = w x h gives the image itself). 0 on failure. */
int render_gpu_readback(const u32 *pixels, int w, int h, const int *vram_xy, int ow, int oh, RenderDestFn dest,
                        u32 *out);
/* The whole target as gpu.c's 16-bit pixels (1024x512: the exactness test's comparison with the software VRAM), after
 * the recorded units ran. 0 when not rasterising, at an internal scale above 1, or on failure. */
int render_gpu_read_vram(u16 *out);
/* The target set to the VRAM as it is now (tests/host/gpu_hw_replay.py, after a case that differed). */
void render_gpu_raster_resync(void);
/* Releases everything, the window's claim, then the device (before SDL_Quit: video.c port_video_quit). */
void render_gpu_close(void);
#endif

#endif /* PORT_RENDER_GPU_H */
