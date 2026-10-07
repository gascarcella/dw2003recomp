/* The hardware renderer (render_gpu.c; issue #31, docs/PORT.md "Rendering"): SDL_GPU, only in the DW3_PORT_SDL build,
 * driven by video.c. Phase 1 presents the software image (video.c's video_pixels) through it; the rasteriser of the
 * software GPU's command stream comes later. Every function but render_gpu_open is a no-op until a device is open. */
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
/* `pixels` (w x h, 0xFFRRGGBB) into the window's swapchain at dest's rectangle, on black. 0 when the frame could not
 * be submitted (logged once). */
int render_gpu_present(const u32 *pixels, int w, int h, RenderDestFn dest);
/* The same present into an offscreen ow x oh target, read back into `out` (ow x oh, 0xFFRRGGBB); dest NULL: the whole
 * target (ow x oh = w x h gives the image itself). 0 on failure. */
int render_gpu_readback(const u32 *pixels, int w, int h, int ow, int oh, RenderDestFn dest, u32 *out);
/* Releases everything, the window's claim, then the device (before SDL_Quit: video.c port_video_quit). */
void render_gpu_close(void);
#endif

#endif /* PORT_RENDER_GPU_H */
