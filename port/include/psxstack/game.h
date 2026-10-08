/* psxstack/game.h: the adapter interface (GAME_CONTRACT.md "4. The adapter units"): what the runtime asks of the game.
 * The game's adapter (port/game/) implements these; port/runtime/game_defaults.c has a weak default for each, so a
 * game starts with an empty adapter and adds what its tests need. The runtime reaches the game's objects only through
 * this file and game_main; it includes no game header. Types are <stdint.h>'s. */
#ifndef PSXSTACK_GAME_H
#define PSXSTACK_GAME_H

#include <stddef.h>
#include <stdint.h>

#include "psxstack/mods.h"

/* The stack's interface version: counts incompatible changes to this contract (GAME_CONTRACT.md "Compatibility and
 * versions"). A mod manifest's `requires_port` must not be higher. */
#define PSXSTACK_API 1

/* ---- The game's entry: its main(), compiled as game_main (-Dmain=game_main on that unit). Never returns normally. */
int game_main(void);

/* ---- The rate: called once the options and settings are read, before the game's data is snapshotted and before
 * game_main, with the nominal vsyncs per second the run uses (PSXSTACK_GAME_RATE, or the other rate the launcher
 * offers): the game's own response to it (dw2003: records_60hz for 60). Default: nothing. */
void game_apply_rate(long rate);

/* ---- The state probes (the replay scripts, the frame log, the debug channel, the crash report): what the emulator's
 * runner reads from PS1 RAM, read from the game's host objects. Defaults: 0. */
int32_t game_state_stage(void);        /* the current stage overlay's number */
int32_t game_state_file(void);         /* the current stage overlay's file ID */
int32_t game_state_map(void);          /* the current map */
int32_t game_state_random_index(void); /* the pad RNG's index (the record's random_index) */
/* The field player's position in pixels: 1 and the values in x and y, or 0 when there is no player (default). */
int game_state_player_pos(double *x, double *y);

/* ---- The checkpoint image: the game-state bytes a checkpoint hashes, as the emulator dumps them from PS1 RAM:
 * game_state_image_size() bytes (0 by default: nothing to hash), written by game_state_image (pointers as the PS1
 * addresses of the host functions they hold). game_state_volatile: the [lo, hi) byte ranges of the image zeroed for
 * the stable hash (the emulator's VOLATILE_RANGES); default none. */
typedef struct PortRange {
    uint32_t lo, hi;
} PortRange;
uint32_t game_state_image_size(void);
void game_state_image(uint8_t *out);
int game_state_volatile_count(void);
const PortRange *game_state_volatile(void);

/* ---- PS1-address reads (the scripts' wait_mem, the debug channel's ps1: targets) outside the arena: a `size`-byte
 * (1, 2, 4) read at `addr`, sign-extended if `is_signed`, into *out: 1, or 0 when the game does not map the address
 * (default). game_state_host: the writable host bytes the same access maps to, NULL when unmapped (default). The
 * arena itself is the runtime's. */
int game_state_read(uint32_t addr, int size, int is_signed, int32_t *out);
void *game_state_host(uint32_t addr, int size);

/* ---- The mods: the game's built-in mods, game_mod_count() PortMod records at game_mods() (default: none). The
 * runtime's own fast_forward is not among them. */
int game_mod_count(void);
PortMod *game_mods(void);

/* ---- The state tables the build generates for the adapter from the game's symbol files (tools/port_gen.py state):
 * the EXE's functions ({PS1 address, host function}), its sized data symbols ({PS1 address, PS1 size, name, host
 * object, the layout-identical prefix}). The runtime does not read them; the adapter's probes do. */
typedef void (*PortFn)(void);
typedef struct PortExeFunc {
    uint32_t addr; /* the function's PS1 address */
    PortFn fn;     /* the host function */
} PortExeFunc;
typedef struct PortExeData {
    uint32_t addr;      /* the PS1 address */
    uint32_t size;      /* the PS1 size */
    const char *name;
    const void *host;   /* the host object */
    uint32_t identical; /* bytes from the start that have the PS1 layout by the default rule: size, or 0 */
} PortExeData;
extern const PortExeFunc port_exe_funcs[];
extern const int port_exe_func_count;
extern const PortExeData port_exe_data[];
extern const int port_exe_data_count;
extern const PortRange port_gamestate_volatile[]; /* the emulator runner's VOLATILE_RANGES, generated */
extern const int port_gamestate_volatile_count;

#endif /* PSXSTACK_GAME_H */
