/* The interrupt pump (include/port.h PLATFORM_WAIT/PLATFORM_HALT): the game's busy-waits call port_wait(), which runs
 * what the PS1's interrupts would have run, deterministically: one vsync (psyq_vsync_tick: the VSyncCallback handler,
 * then port_frame below, which runs the CD "interrupt" and the harness). A frame is one vsync tick, whether the game's
 * VSync() or port_wait() ran it; the frame cap ends the run with status 0.
 *
 * The watchdog: a loop that no PLATFORM_WAIT reaches (cdload_load_file's `do cdload_update() while (loading)`, which
 * only a CD interrupt ends on the PS1) would spin forever once the shim cannot complete a read; SIGALRM ends it with
 * status 4 and names the last Psy-Q call, instead of hanging the acceptance run.
 *
 * The window (video.c, input.c; `--window`): each vsync also polls SDL's events (the pad, unless a script owns it),
 * presents the display, and waits for the vsync's time against CLOCK_MONOTONIC. Two rates (docs/LAUNCHER.md
 * "Fast-forward"): the **nominal rate** port_rate (50, PAL; `--fps N` sets it to N): the vsyncs per second the game is made for,
 * which the audio's samples per vsync follow; and the **pace** (port_pace_set; `--fps`, fast-forward): the vsyncs per
 * second of the wall clock (0: as fast as it runs). Every change of the pace starts the schedule over, so going back
 * from a fast pace to a slow one never waits for the vsyncs "owed". Only the wall-clock time between vsyncs depends on
 * them; the headless run is never paced.
 *
 * The pause (window mode; the `pause` hotkey, input.c): at the end of the vsync on which it is pressed the game stops
 * between two vsyncs: the window keeps polling its events (the hotkeys, fullscreen, the close) and presenting the last
 * image, the audio device is paused, the watchdog re-armed; the pause key again goes on with the next vsync, the
 * schedule started over. Nothing of it reaches the game, the log or the record. */
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "port_harness.h"
#include "port_runtime.h"
#include "psyq.h"

long port_max_frames = 600;
long port_frames;
int port_watchdog_sec = 10;
int port_script_active;
long port_rate = 50;        /* the nominal rate (vsyncs per second): the audio's samples per vsync */
static long pump_pace = 50; /* the pace (vsyncs per second of the wall clock); 0: unthrottled */
static int pump_pace_restart;
static volatile sig_atomic_t port_watchdog_armed;
static void port_frame(void);
static void port_pace(void);
static void port_pause(void);

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
    psyq_set_vsync_pre_hook(port_audio_frame); /* the SPU's samples of the frame, before the game's handler */
    psyq_set_vsync_hook(port_frame);
    if (port_watchdog_sec > 0) {
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = port_watchdog;
        sigaction(SIGALRM, &sa, NULL);
        alarm((unsigned)port_watchdog_sec);
        port_watchdog_armed = 1;
    }
    /* the last call before game_main: the game's data must still be as snapshotted (nothing in the setup wrote it),
     * which is what a reset restores */
    port_reset_check("startup");
}

/* The console's reset (port/src/reset.c): the frame count goes on; the watchdog starts over (the longjmp left the
 * tick that re-armed it), the per-frame hook stays the runtime's. */
void port_pump_reset(void) {
    psyq_set_vsync_pre_hook(port_audio_frame);
    psyq_set_vsync_hook(port_frame);
    if (port_watchdog_armed) {
        alarm((unsigned)port_watchdog_sec);
    }
}

/* The per-frame work, run by the shim at the end of every vsync tick (psyq_set_vsync_hook), whether the tick came
 * from the game's VSync() or from port_wait(): the CD "interrupt" (psyq_cd_tick), the frame count, the per-frame
 * log, the input script, the frame cap. */
static void port_frame(void) {
    int cd;
    if (port_watchdog_armed) {
        alarm((unsigned)port_watchdog_sec); /* re-arm: progress */
    }
    cd = psyq_cd_tick();
    port_frames++;
    if (port_trace) {
        port_log("tick: frame %ld%s", port_frames, cd ? " (CD handler ran)" : "");
    }
    port_framelog_frame();
    if (port_window) {
        port_input_frame();
    }
    if (port_script_active) {
        port_script_frame();
    }
    port_video_frame();
    if (port_max_frames > 0 && port_frames >= port_max_frames) {
        port_exit(0, "frame cap");
    }
    port_mods_frame();
    if (port_window) {
        port_pace();
        if (port_input_pressed(port_action_pause)) {
            port_pause();
        }
    }
}

void port_pace_set(long fps) {
    if (fps != pump_pace) {
        pump_pace = fps;
        pump_pace_restart = 1;
    }
}

long port_pace_get(void) {
    return pump_pace;
}

/* The pause: between two vsyncs, until the pause key again (or the window's close, which exits). */
static void port_pause(void) {
    struct timespec tick = { 0, 20000000L }; /* 50 polls a second */
    port_log("pause at frame %ld", port_frames);
    port_video_set_paused(1);
    port_audio_pause(1);
    for (;;) {
        port_input_poll_paused();
        if (port_input_pressed(port_action_pause)) {
            break;
        }
        port_video_refresh();
        if (port_watchdog_armed) {
            alarm((unsigned)port_watchdog_sec);
        }
        nanosleep(&tick, NULL);
    }
    port_audio_pause(0);
    port_video_set_paused(0);
    pump_pace_restart = 1;
    port_log("resume at frame %ld", port_frames);
}

/* Real-time pacing (window mode): vsync n is due at start + n / pace seconds; a run more than 0.1 s late (a
 * breakpoint, a slow host) starts over from now instead of hurrying to catch up, and so does a change of the pace (or
 * the end of a pause). */
static void port_pace(void) {
    static struct timespec start;
    static long long n;
    struct timespec now, due;
    long long t, ns;
    if (pump_pace_restart) {
        pump_pace_restart = 0;
        n = 0;
    }
    if (pump_pace <= 0) {
        return;
    }
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (n == 0) {
        start = now;
    }
    n++;
    ns = n * 1000000000LL / pump_pace;
    t = (long long)(now.tv_sec - start.tv_sec) * 1000000000LL + (now.tv_nsec - start.tv_nsec);
    if (t > ns + 100000000LL) {
        start = now;
        n = 0;
        return;
    }
    due.tv_sec = start.tv_sec + (time_t)((start.tv_nsec + ns) / 1000000000LL);
    due.tv_nsec = (long)((start.tv_nsec + ns) % 1000000000LL);
    while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &due, NULL) == EINTR) {
        /* the watchdog's SIGALRM, or another signal: wait on */
    }
}

void port_wait(void) {
    psyq_vsync_tick();
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
    static int exiting;
    if (!exiting) {
        exiting = 1;
        port_framelog_close(status, reason);
        port_video_close();
        port_audio_close();
        port_spu_trace_close();
        port_video_quit();
    }
    port_log("exit %d after %ld frame(s): %s", status, port_frames, reason);
    fflush(NULL);
    exit(status);
}
