#ifndef MEMCARD_H
#define MEMCARD_H

#include "common.h"
#include "psyq/libmcrd.h"

/* The save icon's 16-colour palette. */
typedef struct MemcardClut {
    u16 colors[16];
} MemcardClut; /* size 0x20 */

/* The 0x80-byte header at the start of a save file (memory card file format). */
typedef struct MemcardHeader {
    /* 0x00 */ char magic[2]; /* "SC" */
    /* 0x02 */ u8 type;       /* 0x10 + icon frames */
    /* 0x03 */ u8 blocks;
    /* 0x04 */ char title[64];
    /* 0x44 */ u8 pad[28];
    /* 0x60 */ MemcardClut clut;
} MemcardHeader; /* size 0x80 */

/* memcard_state: the memory card module's state (src/main/memcard.c; STGMCARD reaches it through its own view,
 * StgmcardMemcard). */
typedef struct MemcardState {
    /* 0x000 */ s32 state; /* state: 0 idle, 1 MemCardExist, 2 MemCardAccept, 3 reading, 4 writing, 5 command */
    /* 0x004 */ s32 unk_004;
    /* 0x008 */ s32 unk_008;
    /* 0x00C */ char *file_name; /* save file name (memcard_set_file_name) */
    /* 0x010 */ MemcardHeader header;
    /* 0x090 */ s32 command; /* MemCardSync command */
    /* 0x094 */ s32 result; /* MemCardSync result */
    /* 0x098 */ s32 retries; /* retries */
    /* 0x09C */ s32 max_retries; /* retry limit */
    /* 0x0A0 */ s32 retry; /* set when a command must be retried */
    /* 0x0A4 */ s32 file_count; /* files in files */
    /* 0x0A8 */ DIRENTRY files[15];
    /* 0x300 */ s32 done; /* bytes done */
    /* 0x304 */ s32 offset; /* file offset */
    /* 0x308 */ s32 unk_308;
    /* 0x30C */ s32 slot_size;  /* size of a save slot's part: 0x2700 (docs/FORMATS.md "Save data") */
    /* 0x310 */ s32 part1_size; /* size of part 1 (the load screen's summary): 0x100 */
    /* 0x314 */ s32 icon_frames; /* icon frames (1-3) */
    /* 0x318 */ void *icons[4]; /* icon images */
} MemcardState; /* size 0x328 */

/* memcard_funcs: the memory card module's function table, which game code calls through (main calls
 * init at start-up; the rest are called from the overlays). */
typedef struct MemcardFuncs {
    /* 0x00 */ void (*init)(void);                                            /* memcard_init */
    /* 0x04 */ void (*set_file_name)(void);                                   /* memcard_set_file_name */
    /* 0x08 */ void (*set_header)(char *title, MemcardClut *clut, s32 frames, void **icons); /* memcard_set_header */
    /* 0x0C */ s32 (*wait_exist)(s32 chan);                                   /* memcard_wait_exist */
    /* 0x10 */ s32 (*wait_accept)(s32 chan);                                  /* memcard_wait_accept */
    /* 0x14 */ s32 (*read)(s32 chan, void *buf, s32 size, s32 part);          /* memcard_read */
    /* 0x18 */ s32 (*write)(s32 chan, void *buf, s32 size, s32 part);         /* memcard_write */
    /* 0x1C */ s32 (*list_files)(s32 chan);                                   /* memcard_list_files */
    /* 0x20 */ s32 (*create_file)(s32 chan);                                  /* memcard_create_file */
    /* 0x24 */ s32 (*format)(s32 chan);                                       /* memcard_format */
    /* 0x28 */ s32 (*unk_28)(void);                                           /* func_80015528 */
    /* 0x2C */ s32 (*check_checksum)(u8 *data, s32 size, u8 sum);             /* memcard_check_checksum */
    /* 0x30 */ u8 (*get_checksum)(u8 *data, s32 size);                        /* memcard_get_checksum */
} MemcardFuncs; /* size 0x34 */

extern MemcardFuncs memcard_funcs;

#endif /* MEMCARD_H */
