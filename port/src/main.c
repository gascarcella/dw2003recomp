/* The port's entry point: options, the runtime's setup, then the game's main() (src/main/main.c, compiled as
 * game_main). The game never returns; the run ends in port_exit (the frame cap, status 0; a stub that cannot fake
 * its result, 3; PLATFORM_HALT, 2; the watchdog, 4). */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "port_harness.h"
#include "port_runtime.h"
#include "psyq.h"

int port_trace;

void port_log(const char *fmt, ...) {
    va_list ap;
    fputs("port: ", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

void port_fatal(const char *fmt, ...) {
    va_list ap;
    fputs("port: fatal: ", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    port_exit(1, "fatal error");
}

static void usage(const char *argv0) {
    fprintf(stderr,
            "usage: %s [--disc CUE|BIN] [--no-disc-check] [--cd-speed instant|realistic] [--script JSON]\n"
            "          [--log FILE] [--record FILE] [--max-frames N] [--watchdog SEC] [--trace]\n"
            "  --disc PATH      the user's disc (.cue or .bin; SHA-1 checked); without it reads find no data\n"
            "  --no-disc-check  skip the disc's SHA-1 check (experiments with another image)\n"
            "  --cd-speed S     the CD's timing: realistic (default: double speed and seeks) or instant\n"
            "  --script JSON    an input script (tests/replay/scripts/*.json); the run ends when it does\n"
            "  --log FILE       the per-frame log (frame, overlay, map, primitive-stream hash; events)\n"
            "  --record FILE    the run's record at exit (JSON: checkpoints, overlay and map sequences)\n"
            "  --max-frames N   stop with status 0 after N vsync ticks (default 600, none with --script; 0: no cap)\n"
            "  --watchdog SEC   stop with status 4 after SEC seconds without a vsync tick (default 10; 0: none)\n"
            "  --trace          log every tick, overlay resolve and Psy-Q stub call\n",
            argv0);
}

static long number(const char *s, const char *opt) {
    char *end;
    long v;
    errno = 0;
    v = strtol(s, &end, 0);
    if (errno != 0 || *end != '\0' || v < 0) {
        fprintf(stderr, "port: %s: not a number: %s\n", opt, s);
        exit(64);
    }
    return v;
}

int main(int argc, char **argv) {
    const char *disc = NULL, *script = NULL, *log = NULL, *record = NULL, *speed = NULL;
    int disc_check = 1, max_frames_given = 0;
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--max-frames") == 0 && i + 1 < argc) {
            port_max_frames = number(argv[i + 1], argv[i]);
            max_frames_given = 1;
            i++;
        } else if (strcmp(argv[i], "--disc") == 0 && i + 1 < argc) {
            disc = argv[++i];
        } else if (strcmp(argv[i], "--no-disc-check") == 0) {
            disc_check = 0;
        } else if (strcmp(argv[i], "--cd-speed") == 0 && i + 1 < argc) {
            speed = argv[++i];
        } else if (strcmp(argv[i], "--script") == 0 && i + 1 < argc) {
            script = argv[++i];
        } else if (strcmp(argv[i], "--log") == 0 && i + 1 < argc) {
            log = argv[++i];
        } else if (strcmp(argv[i], "--record") == 0 && i + 1 < argc) {
            record = argv[++i];
        } else if (strcmp(argv[i], "--watchdog") == 0 && i + 1 < argc) {
            port_watchdog_sec = (int)number(argv[i + 1], argv[i]);
            i++;
        } else if (strcmp(argv[i], "--trace") == 0) {
            port_trace = 1;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            return 0;
        } else {
            usage(argv[0]);
            return 64;
        }
    }
    setvbuf(stderr, NULL, _IOLBF, 0);
    psyq_set_trace(port_trace, stderr);
    port_arena_init();
    port_overlay_init();
    if (speed != NULL && !port_disc_set_speed(speed)) {
        fprintf(stderr, "port: --cd-speed: unknown speed %s\n", speed);
        return 64;
    }
    if (disc != NULL) {
        port_disc_open(disc, disc_check);
    }
    port_framelog_open(log, record);
    if (script != NULL) {
        port_script_load(script);
        port_script_active = 1;
        if (!max_frames_given) {
            port_max_frames = 0; /* the script's own max_frames ends the run */
        }
    }
    port_pump_init();
    port_log("start: max-frames %ld, watchdog %d s", port_max_frames, port_watchdog_sec);
    game_main();
    port_exit(0, "game_main returned");
}
