/* The port's entry point: options, the runtime's setup, then the game's main() (src/main/main.c, compiled as
 * game_main). The game never returns; the run ends in port_exit (the frame cap, status 0; a stub that cannot fake
 * its result, 3; PLATFORM_HALT, 2; the watchdog, 4; the window closed, 0; --input-test failed, 6). */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "port_harness.h"
#include "port_runtime.h"
#include "spu.h"
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
            "usage: %s [--disc CUE|BIN] [--no-disc-check] [--cd-speed instant|realistic] [--memcard1|2 MCD|none]\n          [--script JSON]\n"
            "          [--log FILE] [--record FILE] [--max-frames N] [--watchdog SEC] [--trace]\n"
            "          [--window] [--scale N] [--fullscreen] [--fps N] [--input-test] [--screenshot FRAME:PATH]\n"
            "          [--spu-trace FILE] [--wav FILE] [--mute]\n"
            "  --disc PATH      the user's disc (.cue or .bin; SHA-1 checked); without it reads find no data\n"
            "  --no-disc-check  skip the disc's SHA-1 check (experiments with another image)\n"
            "  --cd-speed S     the CD's timing: realistic (default: double speed and seeks) or instant\n"
            "  --memcard1 P     memory card 1: a .mcd image (created if missing), or none; default: a fresh card in\n"
            "                   memory (--memcard2 likewise)\n"
            "  --script JSON    an input script (tests/replay/scripts/*.json); the run ends when it does\n"
            "  --log FILE       the per-frame log (frame, overlay, map, primitive-stream hash; events)\n"
            "  --record FILE    the run's record at exit (JSON: checkpoints, overlay and map sequences)\n"
            "  --max-frames N   stop with status 0 after N vsync ticks (default 600; none with --script or --window;\n"
            "                   0: none)\n"
            "  --watchdog SEC   stop with status 4 after SEC seconds without a vsync tick (default 10; 0: none)\n"
            "  --trace          log every tick, overlay resolve and Psy-Q stub call\n"
            "  --window         show the display in a window (SDL3; a build with -DDW3_PORT_SDL=ON), real time;\n"
            "                   keys: arrows, X cross, C circle, Z square, S triangle, Enter START, Backspace SELECT,\n"
            "                   Q/E L1/R1, 1/3 L2/R2, F11 fullscreen; gamepads too (port/src/input.c); with --script\n"
            "                   the script owns the pad\n"
            "  --scale N        the window's size: 320*N x 240*N (default 2; implies --window)\n"
            "  --fullscreen     a fullscreen window (implies --window)\n"
            "  --fps N          the window's pace: N vsyncs per second (default 50, PAL; 0: unthrottled)\n"
            "  --input-test     the window's input self-test: injected key and gamepad events (implies --window);\n"
            "                   exit 0 = passed, 6 = failed\n"
            "  --screenshot F:P write the display at vsync F to P (binary PPM); repeatable; any build\n"
            "  --spu-trace FILE every SPU write and DMA block, per vsync (tests/sound's trace format)\n"
            "  --wav FILE       the audio output as a 44.1 kHz stereo WAV (any build, headless too)\n"
            "  --mute           no audio device in window mode\n",
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
    const char *memcard[2] = { NULL, NULL };
    const char *spu_trace = NULL, *wav = NULL;
    int mute = 0;
    int memcard_given[2] = { 0, 0 };
    int disc_check = 1, max_frames_given = 0;
    int window = 0, scale = 2, fullscreen = 0, input_test = 0;
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
        } else if ((strcmp(argv[i], "--memcard1") == 0 || strcmp(argv[i], "--memcard2") == 0) && i + 1 < argc) {
            int slot = argv[i][9] - '1';
            memcard[slot] = strcmp(argv[i + 1], "none") == 0 ? NULL : argv[i + 1];
            memcard_given[slot] = strcmp(argv[i + 1], "none") == 0 ? -1 : 1;
            i++;
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
        } else if (strcmp(argv[i], "--window") == 0) {
            window = 1;
        } else if (strcmp(argv[i], "--scale") == 0 && i + 1 < argc) {
            scale = (int)number(argv[i + 1], argv[i]);
            window = 1;
            i++;
            if (scale < 1 || scale > 16) {
                fprintf(stderr, "port: --scale: 1 to 16\n");
                return 64;
            }
        } else if (strcmp(argv[i], "--fullscreen") == 0) {
            fullscreen = 1;
            window = 1;
        } else if (strcmp(argv[i], "--fps") == 0 && i + 1 < argc) {
            port_fps = number(argv[i + 1], argv[i]);
            i++;
        } else if (strcmp(argv[i], "--input-test") == 0) {
            input_test = 1;
            window = 1;
        } else if (strcmp(argv[i], "--spu-trace") == 0 && i + 1 < argc) {
            spu_trace = argv[++i];
        } else if (strcmp(argv[i], "--wav") == 0 && i + 1 < argc) {
            wav = argv[++i];
        } else if (strcmp(argv[i], "--mute") == 0) {
            mute = 1;
        } else if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) {
            if (!port_video_screenshot_add(argv[++i])) {
                fprintf(stderr, "port: --screenshot: FRAME:PATH (FRAME >= 1; at most 64): %s\n", argv[i]);
                return 64;
            }
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            return 0;
        } else {
            usage(argv[0]);
            return 64;
        }
    }
    if (window && !port_video_available()) {
        fprintf(stderr, "port: --window: this build has no window: configure with -DDW3_PORT_SDL=ON "
                        "(port/README.md \"The window\")\n");
        return 64;
    }
    setvbuf(stderr, NULL, _IOLBF, 0);
    if (port_trace) {
        psyq_set_trace(1, stderr); /* else the shim decides by DW3_PORT_TRACE at its first call */
    }
    port_arena_init();
    spu_init();
    port_overlay_init();
    if (speed != NULL && !port_disc_set_speed(speed)) {
        fprintf(stderr, "port: --cd-speed: unknown speed %s\n", speed);
        return 64;
    }
    if (disc != NULL) {
        port_disc_open(disc, disc_check);
    }
    for (i = 0; i < 2; i++) {
        if (memcard_given[i] >= 0) {
            port_memcard_open(i, memcard[i]); /* default: a fresh formatted card in memory, as the replay runner */
        }
    }
    port_framelog_open(log, record);
    if (spu_trace != NULL) {
        port_spu_trace_open(spu_trace);
    }
    port_audio_open(window && !mute, wav);
    if (script != NULL) {
        port_script_load(script);
        port_script_active = 1;
        if (!max_frames_given) {
            port_max_frames = 0; /* the script's own max_frames ends the run */
        }
    }
    if (window) {
        if (!max_frames_given) {
            port_max_frames = 0; /* a window runs until it is closed */
        }
        port_video_open(scale, fullscreen);
        port_input_init(input_test);
    }
    port_pump_init();
    port_log("start: max-frames %ld, watchdog %d s", port_max_frames, port_watchdog_sec);
    if (setjmp(port_reset_jmp) != 0) {
        port_reset_state(); /* the script's reset step (port_reset_request) */
    }
    game_main();
    port_exit(0, "game_main returned");
}
