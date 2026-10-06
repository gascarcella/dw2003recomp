/* port/psyq/libetc.c: LIBETC (vsync, the vsync callback, the video mode). The vsync "interrupt" is psyq_vsync_tick,
 * called by the port's interrupt pump (port_wait). */
#include "psyq_internal.h"
#include "psyq/libetc.h"

static void (*psyq_vsync_handler)(void);
static void (*psyq_vsync_hook)(void);
static void (*psyq_vsync_pre_hook)(void); /* the runtime's work before the game's handler (psyq_set_vsync_pre_hook) */ /* the port runtime's per-frame work (psyq_set_vsync_hook) */
static int psyq_vsync_count;      /* frames since boot: what VSync(0) returns */
static int psyq_video_mode = MODE_PAL;

/* Real: stores the handler and returns the previous one (as an int, the way our header declares it). */
int VSyncCallback(void (*f)(void)) {
    void (*prev)(void) = psyq_vsync_handler;

    PSYQ_TRACE("VSyncCallback %u", PSYQ_PTR(f));
    psyq_vsync_handler = f;
    return (int)(unsigned long)prev;
}

/* The interrupt: one vsync has happened. */
void psyq_vsync_tick(void) {
    psyq_vsync_count++;
    psyq_pad_vsync();
    if (psyq_vsync_pre_hook != NULL) {
        psyq_vsync_pre_hook();
    }
    if (psyq_vsync_handler != NULL) {
        psyq_vsync_handler();
    }
    if (psyq_vsync_hook != NULL) {
        psyq_vsync_hook();
    }
}

void psyq_set_vsync_hook(void (*hook)(void)) {
    psyq_vsync_hook = hook;
}

void psyq_set_vsync_pre_hook(void (*hook)(void)) {
    psyq_vsync_pre_hook = hook;
}

/* The console's reset (psyq.c psyq_reset): no handler, the vsync count from 0, PAL. The runtime's hook stays. */
void psyq_etc_reset(void) {
    psyq_vsync_handler = NULL;
    psyq_vsync_count = 0;
    psyq_video_mode = MODE_PAL;
}

/* Stub that keeps time moving: the PS1's VSync(0) waits for the next vsync and VSync(n > 1) for n of them, with the
 * vsync interrupt (and so the game's handler) firing each time; here the wait is instant and the ticks are run
 * inline, so that frame-counted state advances as it would. Returns the vsync count since boot. VSync(1) and
 * VSync(< 0) only read a counter on the PS1 (the hsyncs since the previous vsync): 0 here, no tick. */
int VSync(int mode) {
    int i;

    PSYQ_TRACE("VSync %d", mode);
    if (mode == 0 || mode > 1) {
        for (i = 0; i < (mode == 0 ? 1 : mode); i++) {
            psyq_vsync_tick();
        }
        return psyq_vsync_count;
    }
    return 0;
}

/* Stub: no interrupt system to reset. */
int ResetCallback(void) {
    PSYQ_TRACE("ResetCallback");
    return 0;
}

/* Real: records the mode and returns the previous one. */
int SetVideoMode(int mode) {
    int prev = psyq_video_mode;

    PSYQ_TRACE("SetVideoMode %d", mode);
    psyq_video_mode = mode;
    return prev;
}
