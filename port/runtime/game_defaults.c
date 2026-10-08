/* The adapter interface's weak defaults (psxstack/game.h): a game whose adapter does not implement one of them gets
 * the empty behaviour (no probes, nothing to hash, no mods), so the runtime links and runs with an empty adapter. The
 * adapter's own definition replaces a default at link time (weak symbols: ELF and COFF both). */
#include "psxstack/game.h"

#define WEAK __attribute__((weak))

WEAK int game_main(void) {
    return 0;
}

WEAK void game_apply_rate(long rate) {
    (void)rate;
}

WEAK int32_t game_state_stage(void) {
    return 0;
}

WEAK int32_t game_state_file(void) {
    return 0;
}

WEAK int32_t game_state_map(void) {
    return 0;
}

WEAK int32_t game_state_random_index(void) {
    return 0;
}

WEAK int game_state_player_pos(double *x, double *y) {
    (void)x;
    (void)y;
    return 0;
}

WEAK uint32_t game_state_image_size(void) {
    return 0;
}

WEAK void game_state_image(uint8_t *out) {
    (void)out;
}

WEAK int game_state_volatile_count(void) {
    return 0;
}

WEAK const PortRange *game_state_volatile(void) {
    return NULL;
}

WEAK int game_state_read(uint32_t addr, int size, int is_signed, int32_t *out) {
    (void)addr;
    (void)size;
    (void)is_signed;
    (void)out;
    return 0;
}

WEAK void *game_state_host(uint32_t addr, int size) {
    (void)addr;
    (void)size;
    return NULL;
}

WEAK int game_mod_count(void) {
    return 0;
}

WEAK PortMod *game_mods(void) {
    return NULL;
}
