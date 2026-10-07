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
