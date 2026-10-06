/* The interrupt pump (include/port.h PLATFORM_WAIT/PLATFORM_HALT): the game's busy-waits call port_wait(), which runs
 * what the PS1's interrupts would have run, deterministically: one vsync (the VSyncCallback handler, the frame
 * counter) and the pending CD command's completion (the CdSync/CdReady handlers), both in the Psy-Q shim
 * (port/psyq/psyq.h). Each call is one frame; the frame cap ends the run with status 0.
 *
 * The watchdog: a loop that no PLATFORM_WAIT reaches (cdload_load_file's `do cdload_update() while (loading)`, which
 * only a CD interrupt ends on the PS1) would spin forever once the shim cannot complete a read; SIGALRM ends it with
 * status 4 and names the last Psy-Q call, instead of hanging the acceptance run. */
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "port_runtime.h"
#include "psyq.h"

long port_max_frames = 600;
long port_frames;
int port_watchdog_sec = 10;
static volatile sig_atomic_t port_watchdog_armed;

static void port_watchdog(int sig) {
    static const char msg[] = "port: watchdog: no port_wait() for the watchdog's time: the game spins in a loop without "
                              "a PLATFORM_WAIT hook (cdload_load_file?); exiting 4\n";
    (void)sig;
    if (write(2, msg, sizeof(msg) - 1) < 0) {
        /* nothing to do */
    }
    _exit(4);
}

void port_pump_init(void) {
    if (port_watchdog_sec > 0) {
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = port_watchdog;
        sigaction(SIGALRM, &sa, NULL);
        alarm((unsigned)port_watchdog_sec);
        port_watchdog_armed = 1;
    }
}

void port_wait(void) {
    int cd;
    if (port_watchdog_armed) {
        alarm((unsigned)port_watchdog_sec); /* re-arm: progress */
    }
    psyq_vsync_tick();
    cd = psyq_cd_tick();
    port_frames++;
    if (port_trace) {
        port_log("tick: frame %ld%s", port_frames, cd ? " (CD handler ran)" : "");
    }
    if (port_max_frames > 0 && port_frames >= port_max_frames) {
        port_exit(0, "frame cap");
    }
}

void port_halt(const char *file, int line) {
    port_log("halt: the game entered its endless loop at %s:%d", file, line);
    port_exit(2, "PLATFORM_HALT");
}

void port_unimplemented(const char *fn) {
    port_log("unimplemented: %s", fn);
    port_exit(3, fn);
}

void port_exit(int status, const char *reason) {
    port_log("exit %d after %ld frame(s): %s", status, port_frames, reason);
    fflush(NULL);
    exit(status);
}
