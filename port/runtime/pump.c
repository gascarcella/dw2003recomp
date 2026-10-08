/* The interrupt pump (include/port.h PLATFORM_WAIT/PLATFORM_HALT): the game's busy-waits call port_wait(), which runs
 * what the PS1's interrupts would have run, deterministically: one vsync (psyq_vsync_tick: the VSyncCallback handler,
 * then port_frame below, which runs the CD "interrupt" and the harness). A frame is one vsync tick, whether the game's
 * VSync() or port_wait() ran it; the frame cap ends the run with status 0.
 *
 * The watchdog: a loop that no PLATFORM_WAIT reaches (cdload_load_file's `do cdload_update() while (loading)`, which
 * only a CD interrupt ends on the PS1) would spin forever once the shim cannot complete a read; the watchdog
 * (platform.c: SIGALRM on POSIX, a thread on Windows) ends it with status 4 after a crash report (crash.c: the
 * registers say where it spins), instead of hanging the acceptance run.
 *
 * The window (video.c, input.c; `--window`): each vsync also polls SDL's events (the pad, unless a script owns it),
 * presents the display, and waits for the vsync's time against the monotonic clock (platform.c). Two rates (docs/LAUNCHER.md
 * "Fast-forward"): the **nominal rate** port_rate (50, PAL; `--fps N` sets it to N): the vsyncs per second the game is made for,
 * which the audio's samples per vsync follow; and the **pace** (port_pace_set; `--fps`, fast-forward): the vsyncs per
 * second of the wall clock (0: as fast as it runs). Every change of the pace starts the schedule over, so going back
 * from a fast pace to a slow one never waits for the vsyncs "owed". Only the wall-clock time between vsyncs depends on
 * them; the headless run is never paced.
 *
 * The pause (the `pause` hotkey, input.c, in window mode; the debug channel's pause/step/wait, debug.c, in any mode):
 * at the end of the vsync on which it is asked for (pump_pause_wanted) the game stops between two vsyncs: the window
 * keeps polling its events (the hotkeys, fullscreen, the close) and presenting the last image, the debug channel its
 * socket, the audio device is paused, the watchdog re-armed; the pause key again (or the channel's resume/step) goes
 * on with the next vsync, the schedule started over. Nothing of it reaches the game, the log or the record. */
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "port_harness.h"
#include "port_runtime.h"
#include "psyq.h"

long port_max_frames = 600;
long port_frames;
int port_watchdog_sec = 10;
int port_script_active;
long port_rate = PSXSTACK_GAME_RATE;        /* the nominal rate (vsyncs per second): the audio's samples per vsync */
static long pump_pace = PSXSTACK_GAME_RATE; /* the pace (vsyncs per second of the wall clock); 0: unthrottled */
static int pump_pace_restart;
static int pump_pause_wanted; /* the game is to be held (is held) between two vsyncs: the pause key, the channel */
static int port_watchdog_armed;
static void port_frame(void);
static void port_pace(void);
static void port_pause(void);

void port_pump_init(void) {
    psyq_set_vsync_pre_hook(port_audio_frame); /* the SPU's samples of the frame, before the game's handler */
    psyq_set_vsync_hook(port_frame);
    if (port_watchdog_sec > 0) {
        port_watchdog_start(port_watchdog_sec);
        port_watchdog_armed = 1;
    }
    /* the last call before game_main: the game's data must still be as snapshotted (nothing in the setup wrote it),
     * which is what a reset restores */
    port_reset_check("startup");
}

/* The console's reset (port/runtime/reset.c): the frame count goes on; the watchdog starts over (the longjmp left the
 * tick that re-armed it), the per-frame hook stays the runtime's. */
void port_pump_reset(void) {
    psyq_set_vsync_pre_hook(port_audio_frame);
    psyq_set_vsync_hook(port_frame);
    if (port_watchdog_armed) {
        port_watchdog_kick();
    }
}

/* The per-frame work, run by the shim at the end of every vsync tick (psyq_set_vsync_hook), whether the tick came
 * from the game's VSync() or from port_wait(): the CD "interrupt" (psyq_cd_tick), the frame count, the per-frame
 * log, the input script, the frame cap. */
static void port_frame(void) {
    int cd;
    if (port_watchdog_armed) {
        port_watchdog_kick(); /* re-arm: progress */
    }
    cd = psyq_cd_tick();
    port_frames++;
    port_crash_frame(port_frames); /* DW3_PORT_CRASH_AT: the crash report's test hook */
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
    if (port_debug_active) {
        port_debug_frame(); /* the channel's requests and its deferred ops, between the script and the video */
    }
    port_video_frame();
    if (port_max_frames > 0 && port_frames >= port_max_frames) {
        port_exit(0, "frame cap");
    }
    port_mods_frame();
    if (port_window) {
        port_pace();
        if (port_input_pressed(port_action_pause)) {
            pump_pause_wanted = 1;
        }
    }
    if (pump_pause_wanted) {
        port_pause();
    }
}

void port_pump_pause_request(void) {
    pump_pause_wanted = 1;
}

void port_pump_resume_request(void) {
    pump_pause_wanted = 0;
}

int port_pump_paused(void) {
    return pump_pause_wanted;
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

/* The pause: between two vsyncs, until the pause key again, the channel's resume (or the window's close, which
 * exits). The channel's reset, asked for while paused, runs once the loop is left (port_debug_resumed): the longjmp
 * must not skip the audio's and the window's un-pause. */
static void port_pause(void) {
    port_log("pause at frame %ld", port_frames);
    port_video_set_paused(1);
    port_audio_pause(1);
    while (pump_pause_wanted) {
        if (port_window) {
            port_input_poll_paused();
            if (port_input_pressed(port_action_pause)) {
                pump_pause_wanted = 0;
                break;
            }
            port_video_refresh();
        }
        if (port_debug_active) {
            port_debug_poll_paused();
            if (!pump_pause_wanted) {
                break;
            }
        }
        if (port_watchdog_armed) {
            port_watchdog_kick();
        }
        port_sleep_ms(20); /* 50 polls a second */
    }
    port_audio_pause(0);
    port_video_set_paused(0);
    pump_pace_restart = 1;
    port_log("resume at frame %ld", port_frames);
    if (port_debug_active) {
        port_debug_resumed();
    }
}

/* Real-time pacing (window mode): vsync n is due at start + n / pace seconds; a run more than 0.1 s late (a
 * breakpoint, a slow host) starts over from now instead of hurrying to catch up, and so does a change of the pace (or
 * the end of a pause). */
static void port_pace(void) {
    static long long start;
    static long long n;
    long long now, t, ns;
    if (pump_pace_restart) {
        pump_pace_restart = 0;
        n = 0;
    }
    if (pump_pace <= 0) {
        return;
    }
    now = port_clock_ns();
    if (n == 0) {
        start = now;
    }
    n++;
    ns = n * 1000000000LL / pump_pace;
    t = now - start;
    if (t > ns + 100000000LL) {
        start = now;
        n = 0;
        return;
    }
    port_sleep_until_ns(start + ns);
}

void port_wait(void) {
    psyq_vsync_tick();
}

void port_halt(const char *file, int line) {
    char where[256];
    port_log("halt: the game entered its endless loop at %s:%d", file, line);
    snprintf(where, sizeof(where), "PLATFORM_HALT at %s:%d", file, line);
    port_crash_report("halt", 2, where);
    port_exit(2, "PLATFORM_HALT");
}

void port_cdload_wait_read(s32 id) {
    port_log("cdload: frame %ld: file 0x%X freed while the CD reads into it: waiting for the read", port_frames, id);
}

void port_unimplemented(const char *fn) {
    port_log("unimplemented: %s", fn);
    port_crash_report("unimplemented", 3, fn);
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
        if (port_debug_active) {
            port_debug_close();
        }
    }
    port_log("exit %d after %ld frame(s): %s", status, port_frames, reason);
    fflush(NULL);
    exit(status);
}
