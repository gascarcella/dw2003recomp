/* The video output (M2; docs/PORT.md "Rendering", DECISIONS "PC port architecture"): what the PS1's video
 * DAC shows, the display area of the VRAM (psyq.h "The video output": psyq_gpu_vram, psyq_gpu_display), converted once
 * per vsync into 32-bit pixels at the display's own size. Two consumers:
 *
 * - The window (`--window`; only in a build configured with -DDW3_PORT_SDL=ON, which defines DW3_PORT_SDL): an SDL3
 *   streaming texture updated and presented every vsync, drawn at 4:3 with nearest-neighbour filtering, as large as
 *   an integer multiple of the image's lines fits (`--scale N`: a 320*N x 240*N window; `--fullscreen`; F11 toggles):
 *   with an even N a 240-line and a 480-line display come out the same size. input.c reads the window's events;
 *   pump.c paces the vsyncs to real time (50 Hz) in window mode.
 * - Screenshots (`--screenshot FRAME:PATH`, any build): the image of vsync FRAME as a binary PPM (P6), exactly the
 *   pixels the window's texture gets. They depend on nothing but the VRAM and the display area (no host state), so
 *   two runs, or a window and a headless run, give the same bytes.
 *
 * The conversion (psx-spx "GPU Display Control", "24bit RGB"): a 15-bit display reads one VRAM pixel per screen pixel
 * (bits 0-4 red, 5-9 green, 10-14 blue; bit 15, the mask bit, is not shown); a 24-bit display (`rgb24`, the movies)
 * reads 3 bytes per screen pixel (red, green, blue) from the VRAM rows taken as byte strings, so a 320-pixel line
 * spans 480 VRAM pixels: DISPENV.disp.w counts screen pixels in both modes (gfx_init_display's 24-bit buffers are 320
 * wide at x 0 and x 480). 5-bit components become 8-bit as (c << 3) | (c >> 2). Coordinates wrap in the VRAM (1024 x
 * 512), as the GPU's reads do. An interlaced 480-line display (the title's 320x480) is shown as one frame of both
 * fields, its lines in VRAM order (the PS1 draws the whole frame into VRAM; the TV interleaves the two fields). The
 * display is black while SetDispMask(0) holds or before the first PutDispEnv (then 320x240). */
#include <stdlib.h>
#include <string.h>

#include "port_harness.h"
#include "port_runtime.h"
#include "psyq.h"

#ifdef DW3_PORT_SDL
#include <SDL3/SDL.h>
#endif

#define VIDEO_MAX_W 640 /* screen pixels: the GPU's widest mode */
#define VIDEO_MAX_H 576 /* lines: PAL, interlaced */
#define VIDEO_MAX_SHOTS 64

int port_window;
static u32 video_pixels[VIDEO_MAX_W * VIDEO_MAX_H]; /* 0xFFRRGGBB */
static int video_w, video_h;                         /* the converted image's size */
static struct {
    long frame;
    char *path;
} video_shots[VIDEO_MAX_SHOTS];
static int video_shot_count;

static u32 video_rgb15(u16 c) {
    u32 r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
    r = (r << 3) | (r >> 2);
    g = (g << 3) | (g >> 2);
    b = (b << 3) | (b >> 2);
    return 0xFF000000u | r << 16 | g << 8 | b;
}

/* Byte `x` (0..2047, wrapping) of VRAM row `row`: the low byte of a pixel first. */
static u32 video_byte(const u16 *row, int x) {
    u16 p = row[(x >> 1) & 1023];
    return (x & 1) ? p >> 8 : p & 0xFF;
}

/* Converts the current display area into video_pixels (video_w x video_h). */
static void video_convert(void) {
    PsyqDisplay d;
    const u16 *vram = psyq_gpu_vram();
    int w, h, x, y;

    psyq_gpu_display(&d);
    w = d.w > 0 ? d.w : 320;
    h = d.h > 0 ? d.h : 240;
    w = w < VIDEO_MAX_W ? w : VIDEO_MAX_W;
    h = h < VIDEO_MAX_H ? h : VIDEO_MAX_H;
    video_w = w;
    video_h = h;
    if (!d.enabled || d.w <= 0 || d.h <= 0) {
        for (x = 0; x < w * h; x++) {
            video_pixels[x] = 0xFF000000u;
        }
        return;
    }
    for (y = 0; y < h; y++) {
        const u16 *row = vram + (size_t)((d.y + y) & 511) * 1024;
        u32 *out = video_pixels + (size_t)y * w;
        if (d.rgb24) {
            int bx = (d.x & 1023) * 2;
            for (x = 0; x < w; x++, bx += 3) {
                out[x] = 0xFF000000u | video_byte(row, bx) << 16 | video_byte(row, bx + 1) << 8 |
                         video_byte(row, bx + 2);
            }
        } else {
            for (x = 0; x < w; x++) {
                out[x] = video_rgb15(row[(d.x + x) & 1023]);
            }
        }
    }
}

int port_video_screenshot_add(const char *spec) {
    char *end;
    long frame = strtol(spec, &end, 0);
    if (end == spec || *end != ':' || end[1] == '\0' || frame <= 0 || video_shot_count == VIDEO_MAX_SHOTS) {
        return 0;
    }
    video_shots[video_shot_count].frame = frame;
    video_shots[video_shot_count].path = strdup(end + 1);
    video_shot_count++;
    return 1;
}

static void video_write_ppm(const char *path) {
    FILE *f = fopen(path, "wb");
    int i;
    if (f == NULL) {
        port_fatal("screenshot: cannot write %s", path);
    }
    fprintf(f, "P6\n%d %d\n255\n", video_w, video_h);
    for (i = 0; i < video_w * video_h; i++) {
        u8 rgb[3] = { (u8)(video_pixels[i] >> 16), (u8)(video_pixels[i] >> 8), (u8)video_pixels[i] };
        fwrite(rgb, 1, 3, f);
    }
    if (fclose(f) != 0) {
        port_fatal("screenshot: cannot write %s", path);
    }
    port_log("screenshot: frame %ld, %dx%d -> %s", port_frames, video_w, video_h, path);
}

#ifdef DW3_PORT_SDL
static SDL_Window *video_window;
static SDL_Renderer *video_renderer;
static SDL_Texture *video_texture;
static int video_tex_w, video_tex_h;
static Uint64 video_start_ns;
static long video_presents;

int port_video_available(void) {
    return 1;
}

void port_video_open(int scale, int fullscreen) {
    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        port_fatal("SDL_Init: %s (a host without a display: SDL_VIDEO_DRIVER=offscreen)", SDL_GetError());
    }
    atexit(SDL_Quit);
    video_window = SDL_CreateWindow("dw2003", 320 * scale, 240 * scale, flags);
    if (video_window == NULL) {
        port_fatal("SDL_CreateWindow: %s", SDL_GetError());
    }
    video_renderer = SDL_CreateRenderer(video_window, NULL);
    if (video_renderer == NULL) {
        port_fatal("SDL_CreateRenderer: %s", SDL_GetError());
    }
    port_window = 1;
    video_start_ns = SDL_GetTicksNS();
    port_log("window: SDL %d.%d.%d, video driver %s, renderer %s, %dx%d%s", SDL_VERSIONNUM_MAJOR(SDL_GetVersion()),
             SDL_VERSIONNUM_MINOR(SDL_GetVersion()), SDL_VERSIONNUM_MICRO(SDL_GetVersion()),
             SDL_GetCurrentVideoDriver(), SDL_GetRendererName(video_renderer), 320 * scale, 240 * scale,
             fullscreen ? " (fullscreen)" : "");
}

void port_video_toggle_fullscreen(void) {
    if (video_window != NULL) {
        SDL_SetWindowFullscreen(video_window, (SDL_GetWindowFlags(video_window) & SDL_WINDOW_FULLSCREEN) == 0);
    }
}

/* The image's place in the renderer's output (ow x oh pixels): 4:3, as tall as an integer multiple of its lines
 * allows (every line the same height), centred; scaled to fit when the output has fewer pixel rows than it has lines.
 * The width is not an integer multiple for every mode (only 320 and 640 wide displays are 4:3 pixel for pixel). */
static void video_dest(int ow, int oh, SDL_FRect *dst) {
    int f = oh / video_h;
    float dw, dh;
    while (f > 0 && (video_h * f * 4 + 2) / 3 > ow) {
        f--;
    }
    if (f > 0) {
        dh = (float)(video_h * f);
        dw = (float)((video_h * f * 4 + 2) / 3);
    } else {
        dh = (float)oh < ow * 3.0f / 4.0f ? (float)oh : ow * 3.0f / 4.0f;
        dw = dh * 4.0f / 3.0f;
    }
    dst->w = dw;
    dst->h = dh;
    dst->x = (float)(int)((ow - dw) / 2);
    dst->y = (float)(int)((oh - dh) / 2);
}

static void video_present(void) {
    SDL_FRect dst;
    int ow, oh;
    if (video_texture == NULL || video_tex_w != video_w || video_tex_h != video_h) {
        if (video_texture != NULL) {
            SDL_DestroyTexture(video_texture);
        }
        video_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING,
                                          video_w, video_h);
        if (video_texture == NULL) {
            port_fatal("SDL_CreateTexture %dx%d: %s", video_w, video_h, SDL_GetError());
        }
        SDL_SetTextureScaleMode(video_texture, SDL_SCALEMODE_NEAREST);
        video_tex_w = video_w;
        video_tex_h = video_h;
    }
    SDL_UpdateTexture(video_texture, NULL, video_pixels, video_w * (int)sizeof(u32));
    SDL_SetRenderDrawColor(video_renderer, 0, 0, 0, 255);
    SDL_RenderClear(video_renderer);
    if (SDL_GetCurrentRenderOutputSize(video_renderer, &ow, &oh) && ow > 0 && oh > 0) {
        video_dest(ow, oh, &dst);
        SDL_RenderTexture(video_renderer, video_texture, NULL, &dst);
    }
    SDL_RenderPresent(video_renderer);
    video_presents++;
}

void port_video_refresh(void) {
    if (port_window) {
        video_present();
    }
}

static int video_paused;
static char video_status[64]; /* fast-forward's, or "" */

static void video_title(void) {
    char title[96];
    if (video_window != NULL) {
        snprintf(title, sizeof(title), "dw2003%s%s%s%s", video_paused ? " (paused)" : "",
                 video_status[0] ? " (" : "", video_status, video_status[0] ? ")" : "");
        SDL_SetWindowTitle(video_window, title);
    }
}

void port_video_set_paused(int paused) {
    video_paused = paused;
    video_title();
}

void port_video_set_status(const char *status) {
    snprintf(video_status, sizeof(video_status), "%s", status != NULL ? status : "");
    video_title();
}

void port_video_close(void) {
    if (port_window) {
        double s = (double)(SDL_GetTicksNS() - video_start_ns) / 1e9;
        port_log("window: %ld frames presented in %.2f s (%.2f per second)", video_presents, s,
                 s > 0 ? (double)video_presents / s : 0.0);
    }
}

/* SDL torn down before exit(), not only by the atexit(SDL_Quit) above: with NVIDIA's EGL (Wayland and offscreen
 * drivers; driver 595.104.02) an SDL_Quit inside exit() unloads libnvidia-eglcore and the process then jumps into the
 * unloaded code (SIGSEGV after the run's "exit" line, seen in play-testing). X11 (GLX) was not affected. */
void port_video_quit(void) {
    if (video_texture != NULL) {
        SDL_DestroyTexture(video_texture);
        video_texture = NULL;
    }
    if (video_renderer != NULL) {
        SDL_DestroyRenderer(video_renderer);
        video_renderer = NULL;
    }
    if (video_window != NULL) {
        SDL_DestroyWindow(video_window);
        video_window = NULL;
    }
    SDL_Quit();
}
#else
int port_video_available(void) {
    return 0;
}

void port_video_open(int scale, int fullscreen) {
    (void)scale;
    (void)fullscreen;
    port_fatal("this build has no window: configure with -DDW3_PORT_SDL=ON (port/README.md \"The window\")");
}

void port_video_toggle_fullscreen(void) {
}

static void video_present(void) {
}

void port_video_refresh(void) {
}

void port_video_set_paused(int paused) {
    (void)paused;
}

void port_video_set_status(const char *status) {
    (void)status;
}

void port_video_close(void) {
}

void port_video_quit(void) {
}
#endif

/* The present cap (fast-forward): at most `hz` presents a second (0: every vsync). Every vsync is still drawn into
 * the VRAM by the software GPU; only the conversion and the present are skipped. */
static int video_cap_hz;

void port_video_set_present_cap(int hz) {
    video_cap_hz = hz;
}

/* Whether this vsync is presented under the cap. */
static int video_due(void) {
#ifdef DW3_PORT_SDL
    static Uint64 last;
    Uint64 now;
    if (video_cap_hz <= 0) {
        return 1;
    }
    now = SDL_GetTicksNS();
    if (now - last < 1000000000ull / (Uint64)video_cap_hz) {
        return 0;
    }
    last = now;
#endif
    return 1;
}

void port_video_frame(void) {
    int i, shot = 0, present;
    for (i = 0; i < video_shot_count; i++) {
        shot |= video_shots[i].frame == port_frames;
    }
    present = port_window && video_due();
    if (!present && !shot) {
        return;
    }
    video_convert();
    for (i = 0; i < video_shot_count; i++) {
        if (video_shots[i].frame == port_frames) {
            video_write_ppm(video_shots[i].path);
        }
    }
    if (present) {
        video_present();
    }
}
