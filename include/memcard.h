#ifndef MEMCARD_H
#define MEMCARD_H

#include "common.h"

/* The save icon's 16-colour palette. */
typedef struct MemcardClut {
    u16 colors[16];
} MemcardClut; /* size 0x20 */

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
