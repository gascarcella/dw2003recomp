#ifndef PSYQ_LIBMCRD_H
#define PSYQ_LIBMCRD_H

/* Our own declarations of the Psy-Q 4.7 LIBMCRD interface, added as the game needs them. */

#include "common.h"

/* A memory card directory entry (MemCardGetDirentry; Sony declares it in KERNEL.H). */
typedef struct DIRENTRY {
    /* 0x00 */ char name[20];
    /* 0x14 */ s32 attr;
    /* 0x18 */ s32 size;
    /* 0x1C */ struct DIRENTRY *next;
    /* 0x20 */ s32 head;
    /* 0x24 */ char system[4];
} DIRENTRY; /* size 0x28 */

void MemCardInit(long val);
void MemCardStart(void);
s32 MemCardExist(s32 chan);
s32 MemCardAccept(s32 chan);
s32 MemCardReadFile(s32 chan, char *file, u32 *addr, s32 offset, s32 bytes);
s32 MemCardWriteFile(s32 chan, char *file, u32 *addr, s32 offset, s32 bytes);
s32 MemCardCreateFile(s32 chan, char *file, s32 blocks);
s32 MemCardFormat(s32 chan);
s32 MemCardUnformat(s32 chan);
s32 MemCardSync(s32 mode, s32 *cmds, s32 *result);
s32 MemCardGetDirentry(s32 chan, char *name, DIRENTRY *dir, s32 *files, s32 offset, s32 max);

#endif /* PSYQ_LIBMCRD_H */
