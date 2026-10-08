/* port/psyq/psyq_internal.h: what the shim's C files share (tracing). Not for the port runtime: psyq.h is. */
#ifndef PORT_PSYQ_INTERNAL_H
#define PORT_PSYQ_INTERNAL_H

#include "psyq.h"

/* Tracing (DW3_PORT_TRACE=1 or psyq_set_trace). psyq_trace_state: -1 not decided yet, 0 off, 1 on. */
extern int psyq_trace_state;
int psyq_trace_decide(void);
void psyq_trace_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#define PSYQ_TRACE_ON() (psyq_trace_state > 0 || (psyq_trace_state < 0 && psyq_trace_decide()))

/* PSYQ_TRACE("CdControlF %u", com): one line per stub call when tracing is on; one predictable branch when off. */
#define PSYQ_TRACE(...)                      \
    do {                                     \
        if (PSYQ_TRACE_ON()) {               \
            psyq_trace_printf(__VA_ARGS__);  \
        }                                    \
    } while (0)

/* Each library's part of the console's reset (psyq_reset in psyq.c; psyq_mcrd_reset is in psyq.h). */
void psyq_etc_reset(void);
void psyq_cd_reset(void);
void psyq_pad_reset(void);
void psyq_gpu_reset(void);
void psyq_gs_reset(void);
void psyq_gte_reset(void);
void psyq_press_reset(void);
void psyq_snd_reset(void);

/* gte.c: the GTE (COP2). The generated gtemac.h (tools/port_gen.py overrides) calls these with the registers and
 * command words of include/psyq/gtemac.h's MIPS sequences, and declares them itself (the game's units do not see
 * this header): mtc2/lwc2 write data register `reg` (0..31), mfc2/swc2 read it, ctc2/cfc2 the control registers,
 * psyq_gte_cmd runs a command (the cop2 word's low 25 bits). psyq_gte_clear zeroes every register. */
void psyq_gte_mtc2(int reg, u32 v);
u32 psyq_gte_mfc2(int reg);
void psyq_gte_ctc2(int reg, u32 v);
u32 psyq_gte_cfc2(int reg);
void psyq_gte_cmd(u32 op);
void psyq_gte_clear(void);

/* gpu.c: the GPU. gpu_power_on zeroes the VRAM and resets the drawing state; gpu_reset_state resets the drawing state
 * only (GP1(00h)); gpu_gp0_write takes one GP0 word (a command, its parameters, a transfer's pixels), gpu_gp0_words a
 * run of them (a DMA packet); gpu_load_image is a whole CPU-to-VRAM transfer (LoadImage); gpu_vram_pixels is the
 * VRAM, 1024 pixels per row; gpu_draw_state the current E1/E3/E4/E5 words. */
void gpu_power_on(void);
void gpu_reset_state(void);
void gpu_gp0_write(u32 word);
void gpu_gp0_words(const u32 *w, u32 n);
void gpu_load_image(int x, int y, int w, int h, const u16 *pixels);
const u16 *gpu_vram_pixels(void);
void gpu_draw_state(u32 *e1, u32 *e3, u32 *e4, u32 *e5);

/* gpu.c's decoded commands for the hardware renderer (port/src/render_gpu.c; issue #31): an optional listener, called
 * synchronously for every triangle (a quad is two: (1, 2, 3) then (0, 1, 2)), rectangle, line segment, fill, VRAM copy
 * and finished CPU-to-VRAM transfer gpu.c executes, and at the power-on. A triangle, rectangle or segment is reported
 * before its pixels are drawn (the VRAM is then what its texels come from), only when gpu.c draws it (a triangle past
 * the size limit or of zero area is not); a fill before it is done; a copy after it (an overlapping copy reads pixels
 * it wrote: the listener may take its result from the VRAM); a transfer after its last pixel; the power-on after the
 * VRAM is cleared. With no listener (the default) gpu.c's work and outputs are unchanged. */
typedef struct {
    int x, y;    /* the drawing offset applied, wrapped to 11 bits */
    int r, g, b; /* 0..255 (128 for raw textures: the same pixels) */
    int u, v;    /* 0..255 */
} GpuVertex;
typedef enum {
    GPU_EV_TRIANGLE,
    GPU_EV_RECT,
    GPU_EV_SEGMENT,
    GPU_EV_FILL,
    GPU_EV_COPY,
    GPU_EV_LOAD,
    GPU_EV_POWER_ON
} GpuEventKind;
typedef struct {
    GpuEventKind kind;
    GpuVertex v[3];  /* a triangle's a, b, c as gpu.c's triangle takes them (a: the attribute planes' base); a
                      * segment's two ends; a rectangle's corner (with u, v and the colour) */
    int textured;    /* 0, or the texture depth + 1 (1 4-bit, 2 8-bit, 3 15-bit) */
    int raw, semi, abr, gouraud, dither;
    int tex_x, tex_y, clut_x, clut_y;
    int u_and, u_or, v_and, v_or;             /* the texture window: u = (u & u_and) | u_or, the same for v */
    int area_x0, area_y0, area_x1, area_y1;   /* the drawing area, inclusive */
    int set_mask;    /* 0 or 0x8000 */
    int check_mask;
    int x, y, w, h;  /* rectangle: its size in w, h; fill, copy, transfer: the destination (x rounded down to 16 and
                      * the width up for a fill), wrapping at the VRAM's edges */
    int sx, sy;      /* copy: the source */
    u16 color;       /* fill: the 15-bit colour */
} GpuEvent;
void gpu_set_listener(void (*listener)(const GpuEvent *ev));
/* A segment's pixels as gpu.c draws them (the measured stepping, the drawing area), in drawing order: for a listener,
 * from inside its call (the drawing area is the current one). */
void gpu_segment_walk(const GpuVertex *a, const GpuVertex *b, int gouraud,
                      void (*pixel)(void *ctx, int x, int y, int r, int g, int b), void *ctx);
/* The write stamps (gpu.c "Write stamps"): every write stamps the 64-pixel blocks it touches (16 per row) with the
 * current stamp. gpu_stamp_bump makes later writes carry a newer stamp than every block has now and returns the stamp
 * the blocks have at most; gpu_stamp_generation changes when the stamps start over (every block then reads 0). */
const u32 *gpu_block_stamps(void);
u32 gpu_stamp_bump(void);
u32 gpu_stamp_generation(void);

/* mdec.c: the movie decoder (LIBPRESS's bit-stream decoder and the MDEC). mdec_vlc_decode expands a .STR version 2
 * frame (its 8-byte header, then the bit stream) into the MDEC's run-level words at out (the command word 0x3800xxxx,
 * then xxxx words) and returns 0. mdec_reset loads the default quantisation and IDCT tables and ends any decode;
 * mdec_decode_start takes the run-level words (the command word's bits 27-25 select the output: 24-bit or 15-bit,
 * signed, bit 15; NULL ends the decode); mdec_decode_out writes up to `words` words of pixels and returns how many it
 * wrote (fewer when the run-level words ran out). */
int mdec_vlc_decode(const u8 *frame, u32 *out);
void mdec_reset(void);
void mdec_decode_start(const u32 *rl);
u32 mdec_decode_out(u32 *dst, u32 words);

/* libpad.c: the controllers were polled again (run by the vsync tick). */
void psyq_pad_vsync(void);

/* libsnd.c, for the SPU write trace (port/src/spu_trace.c): psyq_snd_in_vsync is 1 while the vsync handler's
 * sequencer tick runs (its stores belong to the vsync being run, which the runtime's frame count counts only after the
 * handler); the call hook gets one line per LIBSND call the game makes (the oracle's `--calls` comments). */
int psyq_snd_in_vsync(void);
void psyq_snd_set_call_hook(void (*hook)(const char *line));

/* A pointer's bits for a trace line (the low 32 bits: the arena offset lives there, docs/PORT.md "Memory arena"). */
#define PSYQ_PTR(p) ((unsigned)(uintptr_t)(p))

#endif /* PORT_PSYQ_INTERNAL_H */
