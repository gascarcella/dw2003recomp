#ifndef FILETABLE_H
#define FILETABLE_H

#include "common.h"
#include "psyq/libcd.h"

/* The game addresses disc files by file ID: one entry per file under AAA/ (docs/DISC_LAYOUT.md). */
#define FILETABLE_COUNT 2382

extern u32 filetable_lba[FILETABLE_COUNT];     /* start sector of each file */
extern u16 filetable_sectors[FILETABLE_COUNT]; /* size in sectors (2048 B; 2336 B for .STR) */

/* filetable_funcs (0x80048740): the accessors as game code calls them, through this table. */
typedef struct FiletableFuncs {
    s32 (*exists)(s32 id);
    s32 (*get_sectors)(s32 id);
    u32 (*get_lba)(s32 id);
    void (*get_cdloc)(s32 id, s32 offset, CdlLOC *loc);
} FiletableFuncs;

extern FiletableFuncs filetable_funcs;

s32 filetable_exists(s32 id);
s32 filetable_get_sectors(s32 id);
u32 filetable_get_lba(s32 id);
void filetable_get_cdloc(s32 id, s32 offset, CdlLOC *loc);

#endif /* FILETABLE_H */
