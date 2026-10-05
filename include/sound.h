#ifndef SOUND_H
#define SOUND_H

#include "common.h"

/* A sound bank: what sound_update_loading loads into an entry (sound_banks[id] points to one). */
typedef struct SoundBank {
    /* 0x00 */ s32 body_file; /* file ID: VAB body */
    /* 0x04 */ s32 header_file; /* file ID: VAB header, copied to the entry's buffer */
    /* 0x08 */ s32 vab_header; /* sub-file of the header file: the VAB header */
    /* 0x0C */ s32 vab_body;  /* sub-file ID of the body file: the VAB body */
    /* 0x10 */ s32 seps[2];   /* sub-files of the header file: SEPs, 0-terminated */
} SoundBank; /* size 0x18 */

/* sound_module.loading (0x80055C3C): the bank being loaded. A struct of its own: sound_load_bank and
 * sound_update_loading take its address and reach the entries from it (base - 0x50). */
typedef struct SoundBankLoad {
    /* 0x0 */ SoundBank *bank;  /* the bank (sound_banks[id]) */
    /* 0x4 */ s16 step;           /* step of sound_update_loading: 0 = idle, 1..3 */
    /* 0x6 */ s16 entry;          /* entry it goes to */
} SoundBankLoad; /* size 0x8 */

typedef struct SoundEntry {
    /* 0x00 */ s32 id; /* searched by sound_find_entry */
    /* 0x04 */ s16 vab_id;
    /* 0x06 */ s16 sep_count; /* number of seps used */
    /* 0x08 */ s16 seps[4];   /* SEP access numbers (SsSepStop/SsSepClose in sound_load_bank) */
    /* 0x10 */ s32 *header_buffer; /* buffer for the header file (sound_buffers) */
    /* 0x14 */ s32 spu_addr; /* SPU address of the VAB body (sound_spu_addrs) */
} SoundEntry; /* size 0x18 */

/* sound_module: the sound module's state and its function tables, which game code calls through.
 * One object: the code reaches the entries as sound_module + 0x4200 and the tables as base + 0x427C.
 * (libgte's sin_1 addresses its own table D_80056088 with folded negative constants that land
 * inside seq_table; nothing of libgte's lives here.) Callers must go through the struct: GCC 2.8
 * assumes a struct field and a scalar global never alias, which changes instruction order. */
typedef struct SoundModule {
    /* 0x0000 */ u8 seq_table[0x4200]; /* libsnd's SEQ/SEP attribute table: SsSetTableSize(seq_table, 6, 16), 6 x 16 x
                                     * SS_SEQ_TABSIZ (176) bytes (sound_init) */
    /* 0x4200 */ SoundEntry entries[3];
    /* 0x4248 */ s32 current;
    /* 0x424C */ s32 extra_entry; /* entry (1 or 2) that sound_load_extra_bank loaded last */
    /* 0x4250 */ SoundBankLoad loading;
    /* 0x4258 */ void (*init)(void);                             /* sound_init (main, at start-up) */
    /* 0x425C */ s32 (*play)(s32 id);                            /* sound_play: plays sound `id`, returns its voice */
    /* 0x4260 */ s16 (*key_on)(s32 index, s16 prog, s16 note);   /* sound_key_on */
    /* 0x4264 */ void (*key_off)(s32 key, s16 voice);            /* sound_key_off */
    /* 0x4268 */ void (*load_extra_bank)(s32 id);                /* sound_load_extra_bank */
    /* 0x426C */ void (*load_bank)(s32 index, s32 id);           /* sound_load_bank */
    /* 0x4270 */ void (*update_loading)();                       /* sound_update_loading (main, every frame) */
    /* 0x4274 */ s32 (*is_loading)(void);                        /* sound_is_loading */
    /* 0x4278 */ void (*stop_all)();                             /* sound_stop_all */
    /* 0x427C */ void (*stop)();                                 /* sound_stop */
    /* 0x4280 */ void (*fade_out)();                             /* sound_fade_out */
} SoundModule; /* size 0x4284 */

extern SoundModule sound_module;

#endif /* SOUND_H */
