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
/* The first word (little-endian) of the file last loaded into the tier's slot, 0 before any load: what the PS1's slot
 * starts with (run.lua's wait_stage reads 0x80082CB0); on the host a code overlay's slot holds no code. */
u32 port_overlay_word0(int tier);

/* ---- The game-state probes (state.c; the tables come from tools/port_gen.py state -> port_state_tables.c) */
typedef struct PortExeFunc {
    u32 addr;  /* the EXE function's PS1 address (config/symbol_addrs.txt) */
    PortFn fn; /* the host function */
} PortExeFunc;
typedef struct PortExeData {
    u32 addr;         /* the PS1 address */
    u32 size;         /* the PS1 size (the symbol's size: in config/symbol_addrs.txt) */
    const char *name;
    const void *host; /* the host object */
    u32 identical;    /* bytes from the start that have the PS1 layout by the default rule: size, or 0 (port_gen) */
} PortExeData;
typedef struct PortRange {
    u32 lo, hi; /* [lo, hi) */
} PortRange;
extern const PortExeFunc port_exe_funcs[];
extern const int port_exe_func_count;
extern const PortExeData port_exe_data[];
extern const int port_exe_data_count;
extern const PortRange port_gamestate_volatile[]; /* tests/replay/replay.py VOLATILE_RANGES */
extern const int port_gamestate_volatile_count;

#define PORT_GAMESTATE_PS1_SIZE 0x275C /* gamestate_data on the PS1: what a checkpoint hashes */
/* gamestate_data's PS1 image: the pointer-free bytes before funcs as they are, then funcs as the PS1 addresses of the
 * host functions it holds (fatal if one is not an EXE function). */
void port_state_gamestate_image(u8 out[PORT_GAMESTATE_PS1_SIZE]);
/* The SHA-1s of the image (40 hex digits): whole, and with the volatile ranges zeroed (gamestate_sha1_stable). */
void port_state_gamestate_sha1(char full[41], char stable[41]);
s32 port_state_random_index(void); /* pad_random.index */

/* ---- The per-frame log's events (framelog.c), from the runtime and the script */
/* A file copied into a slot (port_overlay_load): `name` is the overlay's, or NULL for a data file. */
void port_framelog_overlay_load(int tier, s32 file, const char *name, u32 word0, u32 size);
/* The pad the script holds this frame (psyq_pad_set's bits, active high): a change goes to the log and to the record's
 * `inputs` ({frame, buttons: [names]}, as run.lua's apply_pad); the record has `inputs` once this has been called. */
void port_framelog_input(u16 buttons);

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

#endif /* PORT_RUNTIME_H */
