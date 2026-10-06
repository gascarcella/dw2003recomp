/* Data the matching build keeps in split assembly (no C unit defines it) and the game's C references: zero blocks
 * (docs/PC_PORT_PLAN.md 1.5 "Asm-only data still referenced by C"). The definitions are weak so that a unit or the
 * shim that defines one of them wins (port/psyq/libgs.c and libcd.c own the three Psy-Q ones today).
 *  - D_800812F8, D_80081358: LIBGS's matrices (psyq/libgs/bss; gfx.c saves/restores them);
 *  - D_80081454: LIBCD's StCdIntrFlag (psyq/libcd/bss; STDWTITL's movie player polls and clears it);
 *  - FIELDSTG's .bss at 0x8009B8EC..0x8009BA04, which stays asm on the PS1 because of psylink's fill
 *    (config/fieldstg.yaml), in FIELDSTG's host .bss (the section name is what tools/port_gen.py ldscript collects),
 *    so a reload of FIELDSTG resets them as the PS1's copy of the file did. */
#include "port_runtime.h"

#include "fieldstg.h"
#include "psyq/libgpu.h"
#include "psyq/libgte.h"

#define WEAK __attribute__((weak))
#define FIELDSTG_BSS __attribute__((section(".bss.dw3.fieldstg")))

WEAK MATRIX D_800812F8;
WEAK MATRIX D_80081358;
WEAK u8 D_80081454;

/* The types are the units' own externs (src/fieldstg/fieldstg_80083784.c, _80085590.c, _80087DB0.c). */
WEAK FIELDSTG_BSS s16 fieldstg_effects_voice;
WEAK FIELDSTG_BSS s32 fieldstg_background_size[2];
WEAK FIELDSTG_BSS void *fieldstg_sprites_search_next;
WEAK FIELDSTG_BSS s32 fieldstg_sprites_search_key;
WEAK FIELDSTG_BSS FieldstgPos fieldstg_shatter_tiles[5][6];
WEAK FIELDSTG_BSS s32 fieldstg_shatter_state;
WEAK FIELDSTG_BSS s16 fieldstg_carry_voice;
WEAK FIELDSTG_BSS RECT fieldstg_shatter_rect;
