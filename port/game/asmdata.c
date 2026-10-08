/* Data the matching build keeps in split assembly (no C unit defines it) and the game's C references: zero blocks
 * (docs/PORT.md "Overlays"). The definitions are weak so that a unit or the
 * shim that defines one of them wins (port/psyq/libgs.c and libcd.c own the three Psy-Q ones today).
 *  - GsLIGHTWSMATRIX, GsWSMATRIX: LIBGS's matrices (psyq/libgs/bss; gfx.c saves/restores them);
 *  - StCdIntrFlag: LIBCD's StCdIntrFlag (psyq/libcd/bss; STDWTITL's movie player polls and clears it);
 *  - FIELDSTG's .bss at 0x8009B8EC..0x8009BA04, which stays asm on the PS1 because of psylink's fill
 *    (config/fieldstg.yaml), in FIELDSTG's host .data (as zeros; the section tools/port_gen.py `rename` gives its
 *    units' .data), so a reload of FIELDSTG resets them as the PS1's copy of the file did. */
#include "port_runtime.h"

#include "fieldstg.h"
#include "psyq/libgpu.h"
#include "psyq/libgte.h"

#define WEAK __attribute__((weak))
/* FIELDSTG's .data section (the names tools/port_gen.py `rename` gives the units' sections: game_section), not its
 * .bss: GCC gives a zero-initialised variable in a section not named .bss* a PROGBITS section, and GNU ld would then
 * make a second, PROGBITS dw3_bss_fieldstg beside the units' NOBITS one. 280 bytes of zeros in the file. */
#ifdef _WIN32
#define FIELDSTG_BSS __attribute__((section(".dw3data$fieldstg_1")))
#else
#define FIELDSTG_BSS __attribute__((section("dw3_data_fieldstg")))
#endif

WEAK MATRIX GsLIGHTWSMATRIX;
WEAK MATRIX GsWSMATRIX;
WEAK u8 StCdIntrFlag;

/* The types are the units' own externs (src/fieldstg/fieldstg_80083784.c, _80085590.c, _80087DB0.c). */
WEAK FIELDSTG_BSS s16 fieldstg_effects_voice;
WEAK FIELDSTG_BSS s32 fieldstg_background_size[2];
WEAK FIELDSTG_BSS void *fieldstg_sprites_search_next;
WEAK FIELDSTG_BSS s32 fieldstg_sprites_search_key;
WEAK FIELDSTG_BSS FieldstgPos fieldstg_shatter_tiles[5][6];
WEAK FIELDSTG_BSS s32 fieldstg_shatter_state;
WEAK FIELDSTG_BSS s16 fieldstg_carry_voice;
WEAK FIELDSTG_BSS RECT fieldstg_shatter_rect;
