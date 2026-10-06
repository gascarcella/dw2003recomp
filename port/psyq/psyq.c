/* port/psyq/psyq.c: the shim's tracing (psyq_set_trace, DW3_PORT_TRACE). Everything else is per library. */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "psyq_internal.h"

int psyq_trace_state = -1;
static FILE *psyq_trace_stream;

/* First use without psyq_set_trace: DW3_PORT_TRACE=1 (or any value but 0/empty) turns tracing on. */
int psyq_trace_decide(void) {
    const char *env = getenv("DW3_PORT_TRACE");

    psyq_trace_state = (env != NULL && env[0] != '\0' && strcmp(env, "0") != 0) ? 1 : 0;
    if (psyq_trace_stream == NULL) {
        psyq_trace_stream = stderr;
    }
    return psyq_trace_state;
}

void psyq_set_trace(int on, FILE *stream) {
    psyq_trace_state = on ? 1 : 0;
    psyq_trace_stream = stream != NULL ? stream : stderr;
}

void psyq_trace_printf(const char *fmt, ...) {
    va_list ap;

    if (psyq_trace_stream == NULL) {
        psyq_trace_stream = stderr;
    }
    fputs("psyq: ", psyq_trace_stream);
    va_start(ap, fmt);
    vfprintf(psyq_trace_stream, fmt, ap);
    va_end(ap);
    fputc('\n', psyq_trace_stream);
}

/* The console's reset (psyq.h, port/src/reset.c): every library's state the game can observe back to power-on, as if
 * the shim had just been loaded. Kept: the runtime's settings (the vsync hook, the CD's sector source and timing model,
 * the GPU's walk window), the memory cards' contents (libmcrd.c) and the trace setting. */
void psyq_reset(void) {
    PSYQ_TRACE("reset");
    psyq_etc_reset();
    psyq_cd_reset();
    psyq_pad_reset();
    psyq_gpu_reset();
    psyq_gs_reset();
    psyq_gte_reset();
    psyq_press_reset();
    psyq_snd_reset();
    psyq_mcrd_reset();
}
