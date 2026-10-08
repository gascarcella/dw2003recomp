/* The port's entry point: options, the runtime's setup, then the game's main() (src/main/main.c, compiled as
 * game_main). The game never returns; the run ends in port_exit (the frame cap, status 0; a stub that cannot fake
 * its result, 3; PLATFORM_HALT, 2; the watchdog, 4; the window closed, 0; --input-test failed, 6). */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "port_harness.h"
#include "port_runtime.h"
#include "records.h"
#include "settings.h"
#include "spu.h"
#include "psyq.h"

int port_trace;

void port_log(const char *fmt, ...) {
    char line[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    fputs("port: ", stderr);
    fputs(line, stderr);
    fputc('\n', stderr);
    port_crash_log_line(line);
}

void port_fatal(const char *fmt, ...) {
    char line[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    fputs("port: fatal: ", stderr);
    fputs(line, stderr);
    fputc('\n', stderr);
    port_crash_log_line(line);
    port_crash_report("fatal", 1, line);
    port_exit(1, "fatal error");
}

static void usage(const char *argv0) {
    fprintf(stderr,
            "usage: %s [--config JSON] [--print-settings] [--print-mods] [--script-mods] [--refresh 50|60]\n"
            "          [--disc CUE|BIN] [--no-disc-check] [--cd-speed instant|realistic] [--memcard1|2 MCD|none]\n"
            "          [--script JSON]\n"
            "          [--log FILE] [--record FILE] [--max-frames N] [--watchdog SEC] [--trace]\n"
            "          [--window] [--scale N] [--fullscreen] [--fps N] [--input-test] [--screenshot FRAME:PATH]\n"
            "          [--renderer software|gpu] [--gpu-screenshot FRAME[@WxH]:PATH]\n"
            "          [--spu-trace FILE] [--wav FILE] [--mute] [--debug SOCKET] [--debug-hold] [--crash-dir DIR]\n"
            "          [--version]\n"
            "  --config JSON    the settings file (docs/LAUNCHER.md; what the launcher starts the game\n"
            "                   with): the disc, the window, the memory cards (default card1.mcd and card2.mcd beside\n"
            "                   the file), the watchdog (default off); the options below override it\n"
            "  --print-settings with --config: print the effective settings (the file's, then the options) as a\n"
            "                   settings file with every key and absolute paths, and exit 0 (64: a bad file)\n"
            "  --print-mods     print the built-in mods' registry (ids, options, defaults) as JSON and exit\n"
            "  --script-mods    with --script: keep the settings' mods on (default: every mod off under a script)\n"
            "  --refresh HZ     50 (PAL, the default) or 60: the game's own 60 Hz mode (the NTSC patch's flag), the\n"
            "                   pace, the audio's and the CD's rate (overrides the settings' video.refresh)\n"
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
            "                   Q/E L1/R1, 1/3 L2/R2, F11 fullscreen, P pause; gamepads too (port/src/input.c;\n"
            "                   rebindable in the settings); with --script the script owns the pad\n"
            "  --scale N        the window's size: 320*N x 240*N (default 2; implies --window)\n"
            "  --fullscreen     a fullscreen window (implies --window)\n"
            "  --fps N          the window's pace and the nominal rate: N vsyncs per second (default 50, PAL;\n"
            "                   0: unthrottled at PAL's rate)\n"
            "  --input-test     the window's input self-test: injected key and gamepad events (implies --window);\n"
            "                   exit 0 = passed, 6 = failed\n"
            "  --screenshot F:P write the display at vsync F to P (binary PPM); repeatable; any build\n"
            "  --renderer R     the window's renderer: software (default: SDL_Renderer) or gpu (SDL_GPU, the\n"
            "                   hardware renderer; software when no device can present; overrides video.renderer)\n"
            "  --gpu-screenshot F[@WxH]:P  the hardware renderer's picture of vsync F to P (binary PPM): the image, or\n"
            "                   with @WxH its present into a W x H output; repeatable; a build with -DDW3_PORT_SDL=ON;\n"
            "                   skipped (logged) when no GPU device opens\n"
            "  --spu-trace FILE every SPU write and DMA block, per vsync (tests/sound's trace format)\n"
            "  --wav FILE       the audio output as a 44.1 kHz stereo WAV (any build, headless too)\n"
            "  --mute           no audio device in window mode\n"
            "  --debug SOCKET   the debug channel (port/src/debug.c): a Unix socket at SOCKET taking newline-delimited\n"
            "                   JSON requests (pause, step, wait, pad, peek/poke, screenshot, hash, reset, quit),\n"
            "                   polled once per vsync; turns the watchdog and the default frame cap off\n"
            "  --debug-hold     with --debug: hold the game paused at its first vsync until the client resumes it\n"
            "                   (a run reproducible from frame 1: the client connects before anything happened)\n"
            "  --crash-dir DIR  where a crash report goes (crash-<time>.txt; docs/PORT.md \"Crash report\"); default:\n"
            "                   the current directory\n"
            "  --version        print the build's version and commit, and exit\n",
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
    const char *spu_trace = NULL, *wav = NULL, *debug = NULL, *crash_dir = NULL;
    int mute = 0, debug_hold = 0;
    int memcard_given[2] = { 0, 0 };
    int disc_check = 1, max_frames_given = 0;
    int window = 0, scale = 2, fullscreen = 0, input_test = 0, gpu = 0, gpu_shots = 0;
    const char *config = NULL;
    int print_settings = 0, print_mods = 0, script_mods = 0;
    long fps = -1;
    int refresh = 0; /* --refresh, else the settings' video.refresh; 0: neither (PAL) */
    int i;
    /* --config first: its values are the defaults that the other options override */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            config = argv[++i];
        }
    }
    if (config != NULL) {
        port_settings_load(config);
        disc = port_settings.disc;
        window = port_settings.window;
        scale = port_settings.scale;
        fullscreen = port_settings.fullscreen;
        gpu = port_settings.gpu;
        mute = port_settings.mute;
        port_watchdog_sec = port_settings.watchdog;
        for (i = 0; i < 2; i++) {
            memcard[i] = port_settings.memcard[i];
            memcard_given[i] = memcard[i] != NULL ? 1 : -1;
        }
    }
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            i++; /* read above */
        } else if (strcmp(argv[i], "--print-settings") == 0) {
            print_settings = 1;
        } else if (strcmp(argv[i], "--print-mods") == 0) {
            print_mods = 1;
        } else if (strcmp(argv[i], "--refresh") == 0 && i + 1 < argc) {
            refresh = (int)number(argv[i + 1], argv[i]);
            i++;
            if (refresh != 50 && refresh != 60) {
                fprintf(stderr, "port: --refresh: 50 (PAL) or 60\n");
                return 64;
            }
        } else if (strcmp(argv[i], "--script-mods") == 0) {
            script_mods = 1;
        } else if (strcmp(argv[i], "--max-frames") == 0 && i + 1 < argc) {
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
            fps = number(argv[i + 1], argv[i]);
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
        } else if (strcmp(argv[i], "--debug") == 0 && i + 1 < argc) {
            debug = argv[++i];
        } else if (strcmp(argv[i], "--debug-hold") == 0) {
            debug_hold = 1;
        } else if (strcmp(argv[i], "--crash-dir") == 0 && i + 1 < argc) {
            crash_dir = argv[++i];
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("dw2003 %s (%s)\n", port_version, port_commit);
            return 0;
        } else if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) {
            if (!port_video_screenshot_add(argv[++i])) {
                fprintf(stderr, "port: --screenshot: FRAME:PATH (FRAME >= 1; at most 64): %s\n", argv[i]);
                return 64;
            }
        } else if (strcmp(argv[i], "--renderer") == 0 && i + 1 < argc) {
            if (strcmp(argv[i + 1], "software") != 0 && strcmp(argv[i + 1], "gpu") != 0) {
                fprintf(stderr, "port: --renderer: software or gpu\n");
                return 64;
            }
            gpu = argv[++i][0] == 'g';
        } else if (strcmp(argv[i], "--gpu-screenshot") == 0 && i + 1 < argc) {
            if (!port_video_gpu_screenshot_add(argv[++i])) {
                fprintf(stderr, "port: --gpu-screenshot: FRAME[@WxH]:PATH (FRAME >= 1; at most 64): %s\n", argv[i]);
                return 64;
            }
            gpu_shots = 1;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            return 0;
        } else {
            usage(argv[0]);
            return 64;
        }
    }
    if (refresh == 0 && config != NULL) {
        refresh = port_settings.refresh;
    }
    if (refresh != 0) {
        port_rate = refresh; /* the 60 Hz mode: the rate, the pace, the CD, records_60hz below */
        port_pace_set(refresh);
    }
    if (fps >= 0) {
        /* --fps N: the pace; without a refresh also the nominal rate (as before: 0 is unthrottled at PAL's rate) */
        if (refresh == 0) {
            port_rate = fps > 0 ? fps : 50;
        }
        port_pace_set(fps);
    }
    port_input_settings(config != NULL ? port_settings.input : NULL);
    port_mods_settings(config != NULL ? port_settings.mods : NULL);
    if (print_mods) {
        port_mods_print_registry(stdout);
        return 0;
    }
    if (print_settings) {
        PortSettings eff = port_settings;
        if (config == NULL) {
            fprintf(stderr, "port: --print-settings: needs --config FILE\n");
            return 64;
        }
        eff.disc = disc != NULL ? port_settings_abspath(disc) : NULL;
        eff.window = window;
        eff.scale = scale;
        eff.fullscreen = fullscreen;
        eff.mute = mute;
        eff.watchdog = port_watchdog_sec;
        eff.refresh = refresh;
        eff.gpu = gpu;
        for (i = 0; i < 2; i++) {
            eff.memcard[i] = memcard_given[i] > 0 && memcard[i] != NULL ? port_settings_abspath(memcard[i]) : NULL;
        }
        port_settings_print(stdout, &eff);
        return 0;
    }
    if (gpu_shots && !port_video_available()) {
        fprintf(stderr, "port: --gpu-screenshot: this build has no GPU renderer: configure with -DDW3_PORT_SDL=ON\n");
        return 64;
    }
    port_video_set_renderer(gpu ? "gpu" : "software");
    if (window && !port_video_available()) {
        fprintf(stderr, "port: --window: this build has no window: configure with -DDW3_PORT_SDL=ON "
                        "(port/README.md \"The window\")\n");
        return 64;
    }
    port_file_line_buffered(stderr); /* a line at a time to the launcher's pipe (unbuffered on Windows) */
    port_crash_init(crash_dir);
    port_log("version %s (%s)", port_version, port_commit);
    if (port_trace) {
        psyq_set_trace(1, stderr); /* else the shim decides by DW3_PORT_TRACE at its first call */
    }
    port_arena_init();
    spu_init();
    if (refresh == 60) {
        /* the game's own 60 Hz mode (the NTSC patch's records_60hz; docs/LAUNCHER.md "50/60 Hz"), set before the
         * snapshot (port_overlay_init) so that the reset restores it and the reset check holds; main_screen_pos stays
         * 1 (the PAL screen offset, which the port's video ignores, and the card game's PAL layout) */
        records_60hz = 1;
        psyq_cd_set_vsync_hz(60);
    }
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
    port_mods_start(script == NULL || script_mods); /* the mods' hotkeys: before --input-test plans its steps */
    if (window) {
        if (!max_frames_given) {
            port_max_frames = 0; /* a window runs until it is closed */
        }
        port_video_open(scale, fullscreen);
        port_input_init(input_test);
    } else {
        port_video_gpu_headless(); /* --gpu-screenshot: the rasteriser draws from the first frame */
    }
    if (debug != NULL) {
        /* a driven run: the tool decides when it ends, and may hold the game paused for as long as it likes */
        port_watchdog_sec = 0;
        if (!max_frames_given) {
            port_max_frames = 0;
        }
        port_debug_open(debug);
        if (debug_hold) {
            port_pump_pause_request(); /* the pump holds the game at the end of its first vsync; the channel's poll
                                        * accepts the client there (pump.c port_pause) */
        }
    }
    port_pump_init();
    port_log("start: max-frames %ld, watchdog %d s, %ld Hz", port_max_frames, port_watchdog_sec, port_rate);
    if (port_setjmp(port_reset_jmp) != 0) {
        port_reset_state(); /* the script's reset step (port_reset_request) */
    }
    game_main();
    port_exit(0, "game_main returned");
}
