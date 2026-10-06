/* The port runtime's internal interface (port/src/): the arena, the overlay manager, the interrupt pump, logging.
 * The game sees only include/port.h's port_* declarations; the Psy-Q shim (port/psyq/) sees psyq.h. */
#ifndef PORT_RUNTIME_H
#define PORT_RUNTIME_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "common.h" /* the game's types and, with PC_PORT, include/port.h */

/* ---- Logging (stderr) */
extern int port_trace; /* --trace: every overlay resolve, every tick */
void port_log(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void port_fatal(const char *fmt, ...) __attribute__((format(printf, 1, 2), noreturn));
/* A function the game needs that the skeleton does not implement (port/psyq/psyq.h): prints `fn`, exits with 3. */
void port_unimplemented(const char *fn) __attribute__((noreturn));

/* ---- The game's entry (src/main/main.c, compiled with -Dmain=game_main) */
int game_main(void);

/* ---- The arena (arena.c; the layout comes from tools/port_gen.py arena-header -> port_arena_gen.h) */
void port_arena_init(void);
/* The 16 MB-aligned base: a 24-bit ordering-table tag (PTR_TO_U32 & 0xFFFFFF) is an offset from it. */
void *port_arena_base(void);
int port_arena_contains(const void *p, size_t size);

/* ---- The overlay manager (overlay.c; the tables come from tools/port_gen.py tables -> overlay_tables.c) */
typedef void (*PortFn)(void);
typedef struct PortOverlayFunc {
    u32 addr;  /* the function's address in the PS1 build (a tag, port.h) */
    PortFn fn; /* the host function */
} PortOverlayFunc;
typedef struct PortOverlay {
    int tier;                     /* 1 or 2 */
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

/* ---- The interrupt pump (pump.c) */
extern long port_max_frames;  /* --max-frames: port_wait() exits 0 after this many vsync ticks (0: no cap) */
extern long port_frames;      /* vsync ticks so far (frames) */
extern int port_script_active; /* --script given: port_script_frame runs every frame */
extern int port_watchdog_sec; /* --watchdog: exit 4 after this many wall-clock seconds without a port_wait() */
void port_pump_init(void);
void port_exit(int status, const char *reason) __attribute__((noreturn)); /* logs the frame count and the reason */

#endif /* PORT_RUNTIME_H */
