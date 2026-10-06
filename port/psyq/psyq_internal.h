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

/* libpad.c: the controllers were polled again (run by the vsync tick). */
void psyq_pad_vsync(void);

/* A pointer's bits for a trace line (the low 32 bits: the arena offset lives there, PC_PORT_PLAN 2.4). */
#define PSYQ_PTR(p) ((unsigned)(unsigned long)(p))

#endif /* PORT_PSYQ_INTERNAL_H */
