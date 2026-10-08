/* The port runtime's internal interface (port/runtime/): the arena, the overlay manager, the interrupt pump, logging.
 * The game sees only psxstack/hooks.h's port_* declarations (through its include/port.h); the Psy-Q shim (port/psyq/)
 * sees psyq.h; the game adapter (port/game/) implements psxstack/game.h and may use this file. The runtime includes no
 * game header: its types are psxstack/types.h's, its game facts port_game_gen.h's (psxstack/hooks.h). */
#ifndef PORT_RUNTIME_H
#define PORT_RUNTIME_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "psxstack/desc.h"
#include "psxstack/game.h"
#include "psxstack/hooks.h"
#include "psxstack/types.h"

/* ---- The build (port_version.c, generated at every build by port/cmake/version.cmake from git describe) */
extern const char port_version[]; /* "0.2.2", "0.2.2-3-gabcdef1", "dev-abcdef1" (-dirty when the tree has changes) */
extern const char port_commit[];  /* the commit's hash, "unknown" without git */

/* ---- Logging (stderr; the last 64 lines also go to the crash report, crash.c) */
extern int port_trace; /* --trace: every overlay resolve, every tick */
void port_log(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void port_fatal(const char *fmt, ...) __attribute__((format(printf, 1, 2), noreturn));
/* A function the game needs that the skeleton does not implement (port/psyq/psyq.h): prints `fn`, exits with 3. */
void port_unimplemented(const char *fn) __attribute__((noreturn));

/* ---- The arena (arena.c; the layout comes from port_game_gen.h: tools/port_gen.py game-header) */
void port_arena_init(void);
/* The arena's base (port_arena): a 24-bit ordering-table tag (PTR_TO_U32 & 0xFFFFFF) is an offset from it. */
void *port_arena_base(void);
int port_arena_contains(const void *p, size_t size);

/* ---- The overlay manager (overlay.c; the tables come from tools/port_gen.py tables -> overlay_tables.c) */
typedef struct PortOverlayFunc {
    u32 addr;  /* the function's address in the PS1 build (a tag, port.h) */
    PortFn fn; /* the host function */
} PortOverlayFunc;
typedef struct PortOverlay {
    int tier;                     /* the slot's 1-based index (PORT_SLOT_COUNT of them) */
    s32 file;                     /* the overlay's file ID (the game's cdload IDs) */
    const char *name;             /* FIELDSTG, WSTAG200, ... */
    const PortOverlayFunc *funcs; /* sorted by addr, terminated by { 0, NULL } */
    int func_count;
    char *data_start, *data_stop; /* the overlay's .data on the host (the ld script's __start/__stop symbols) */
    char *bss_start, *bss_stop;   /* its .bss */
} PortOverlay;
extern const PortOverlay port_overlays[];
extern const int port_overlay_count;
void port_overlay_init(void); /* snapshots every overlay's .data (before game_main) */
const PortOverlay *port_overlay_current(int tier);
/* The first word (little-endian) of the file last loaded into the tier's slot, 0 before any load: what the PS1's slot
 * starts with (run.lua's wait_stage reads 0x80082CB0); on the host a code overlay's slot holds no code. */
u32 port_overlay_word0(int tier);

/* ---- The checkpoint image and its hashes (framelog.c): the game's state bytes (psxstack/game.h game_state_image,
 * game_state_image_size() of them) and their SHA-1s (40 hex digits): whole, and with the game's volatile ranges zeroed
 * (the record's gamestate_sha1_stable). port_state_image: the image in a buffer the runtime keeps. */
const u8 *port_state_image(void);
void port_state_sha1(char full[41], char stable[41]);

/* ---- The per-frame log's events (framelog.c), from the runtime and the script */
/* A file copied into a slot (port_overlay_load): `name` is the overlay's, or NULL for a data file. */
void port_framelog_overlay_load(int tier, s32 file, const char *name, u32 word0, u32 size);
/* The pad the script holds this frame (psyq_pad_set's bits, active high): a change goes to the log and to the record's
 * `inputs` ({frame, buttons: [names]}, as run.lua's apply_pad); the record has `inputs` once this has been called. */
void port_framelog_input(u16 buttons);
u16 port_framelog_last_input(void); /* the buttons last given (0 before any): the debug channel's status */

/* ---- The interrupt pump (pump.c) */
extern long port_max_frames;  /* --max-frames: port_wait() exits 0 after this many vsync ticks (0: no cap) */
extern long port_frames;      /* vsync ticks so far (frames) */
extern int port_script_active; /* --script given: port_script_frame runs every frame */
extern int port_watchdog_sec; /* --watchdog: exit 4 after this many wall-clock seconds without a port_wait() */
void port_pump_init(void);
/* ---- The console's reset (reset.c; port_harness.h): each part back to power-on */
void port_overlay_reset(void);                 /* overlay.c: every game section from its startup snapshot */
size_t port_overlay_check(size_t *checked);    /* overlay.c: bytes that differ from the snapshot (debug check) */
void port_arena_reset(void);                   /* arena.c: the PS1 RAM cleared */
void port_framelog_reset(void);                /* framelog.c: the log's R line */
void port_pump_reset(void);                    /* pump.c: the watchdog re-armed */
void port_reset_check(const char *when);       /* reset.c: DW3_PORT_RESET_CHECK=1 or --trace */
void port_exit(int status, const char *reason) __attribute__((noreturn)); /* logs the frame count and the reason */

/* ---- The crash report (crash.c; docs/PORT.md "Crash report") */
void port_crash_init(const char *dir);        /* the handlers; the report goes to `dir` (NULL: the current directory) */
void port_crash_watchdog_install(void);       /* POSIX: SIGALRM reports too; Windows: records the main thread */
void port_crash_watchdog_fire(void);          /* Windows: the watchdog thread's call: the report, then exit 4 */
void port_crash_log_line(const char *line);   /* port_log's lines, for the report's tail */
/* A report for a stop that is not a signal (port_fatal, port_halt, port_unimplemented): `kind` and `detail` go in it. */
void port_crash_report(const char *kind, int status, const char *detail);
const char *port_crash_last_path(void);       /* the file last written, NULL before any */
void port_crash_frame(long frame);            /* the DW3_PORT_CRASH_AT test hook, once per vsync */

#endif /* PORT_RUNTIME_H */
