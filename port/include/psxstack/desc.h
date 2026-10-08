/* psxstack/desc.h: the types of the game description's generated initializers (port_game_gen.h, from the game's
 * game.json: tools/port_gen.py game-header; GAME_CONTRACT.md "1. game.json"). PSXSTACK_GAME_DISCS is a PortGameDisc
 * initializer list, PSXSTACK_GAME_BIOS_STANDINS a PortGameBiosStandin one. */
#ifndef PSXSTACK_DESC_H
#define PSXSTACK_DESC_H

#include <stdint.h>

typedef struct PortGameDisc {
    const char *label;  /* for messages: "Europe (SLES-03936), unpatched" */
    const char *serial; /* "SLES-03936" */
    const char *sha1;   /* of the whole BIN, 40 hex digits */
    uint64_t size;      /* the BIN's size in bytes */
    const char *region; /* "PAL", "NTSC-U", "NTSC-J" */
    const char *cue;    /* the expected .cue name, or "" */
} PortGameDisc;

typedef struct PortGameBiosStandin {
    uint32_t address; /* 0x1FC0xxxx: where the game reads the BIOS ROM */
    const char *text; /* what the stand-in holds there (NUL-terminated) */
} PortGameBiosStandin;

#endif /* PSXSTACK_DESC_H */
