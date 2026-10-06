/* The port's entry point: options, the runtime's setup, then the game's main() (src/main/main.c, compiled as
 * game_main). The game never returns; the run ends in port_exit (the frame cap, status 0; a stub that cannot fake
 * its result, 3; PLATFORM_HALT, 2; the watchdog, 4). */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

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
            "usage: %s [--max-frames N] [--watchdog SEC] [--trace]\n"
            "  --max-frames N   stop with status 0 after N vsync ticks (default 600; 0: no cap)\n"
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
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--max-frames") == 0 && i + 1 < argc) {
            port_max_frames = number(argv[i + 1], argv[i]);
            i++;
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
    port_pump_init();
    port_log("start: max-frames %ld, watchdog %d s", port_max_frames, port_watchdog_sec);
    game_main();
    port_exit(0, "game_main returned");
}
