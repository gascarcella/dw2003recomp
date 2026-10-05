#ifndef CDLOAD_H
#define CDLOAD_H

#include "common.h"
#include "psyq/libcd.h"

/* An entry handed out by cdload_find_entry/cdload_find_free_entry/cdload_find_oldest_entry. */
typedef struct CdloadEntry {
    /* 0x0 */ s16 state;  /* 0 free, 1 queued (cdload_queue_file), 2 reading (cdload_update), 3 loaded */
    /* 0x2 */ s16 marked; /* set by cdload_mark_loaded; cdload_age_marked makes it the oldest */
    /* 0x4 */ s32 id;    /* file ID */
    /* 0x8 */ s32 last_use; /* vsync count of the last use (cdload_find_oldest_entry frees the least recent) */
    /* 0xC */ void *buffer; /* buffer from heap_funcs.alloc */
} CdloadEntry;

/* Part of cdload's function table (cdload_module.files). */
typedef struct CdloadFileFuncs {
    /* 0x0 */ u8 *(*get_file)(s32); /* cdload_get_file */
    /* 0x4 */ void (*free_file)(s32); /* cdload_free_file */
    /* 0x8 */ void (*free_all)(); /* cdload_free_all */
    /* 0xC */ void (*free_above)(); /* cdload_free_above */
} CdloadFileFuncs;

/* cdload_module: the CD module's state, its 64 entries and its function table, which game code calls
 * through (GCC 2.8 assumes a struct field and a scalar global never alias). */
typedef struct CdloadModule {
    /* 0x000 */ s32 queued; /* work queued (cdload_queue_file sets it; cdload_update clears it when done) */
    /* 0x004 */ CdloadEntry entries[64];
    /* 0x404 */ s32 (*is_loading)(s32 id);              /* cdload_is_loading */
    /* 0x408 */ void (*free_oldest)(void);              /* cdload_free_oldest */
    /* 0x40C */ void (*queue_file)(s32 id);             /* cdload_queue_file */
    /* 0x410 */ void (*update)();                       /* cdload_update */
    /* 0x414 */ CdloadFileFuncs files;
    /* 0x424 */ void *(*get_subfile_by_id)(u32 id);     /* cdload_get_subfile_by_id */
    /* 0x428 */ void *(*get_subfile)(s32 index, s32 *data); /* cdload_get_subfile */
    /* 0x42C */ void (*mark_loaded)(void);              /* cdload_mark_loaded */
    /* 0x430 */ void (*age_marked)();                   /* cdload_age_marked */
} CdloadModule; /* size 0x434 */

extern CdloadModule cdload_module;

/* cdload_reader: the sector reader's state (cdload_read fills it, cdload_ready_callback runs per sector) and
 * its function table. */
typedef struct CdloadReader {
    /* 0x00 */ s32 busy;   /* 0 idle, else the step of cdload_sync_callback (cdload_is_busy) */
    /* 0x04 */ s32 id;     /* file ID */
    /* 0x08 */ s32 offset; /* sector offset in the file */
    /* 0x0C */ s32 sectors; /* sectors */
    /* 0x10 */ void *buffer; /* the caller's buffer */
    /* 0x14 */ s32 *done;   /* the caller's flag: cleared at the start, 1 when the read ends */
    /* 0x18 */ CdlLOC loc;
    /* 0x1C */ s32 first_sector; /* first sector */
    /* 0x20 */ s32 sectors_left; /* sectors left, -1 on error */
    /* 0x24 */ u8 *dest;     /* where the next sector goes */
    /* 0x28 */ s32 expected_sector; /* expected sector number (cdload_check_sector) */
    /* 0x2C */ s32 (*is_busy)(void); /* cdload_is_busy */
    /* 0x30 */ void (*read)(s32 id, s32 offset, s32 sectors, void *buf, s32 *done);   /* cdload_read */
} CdloadReader; /* size 0x34 */

extern CdloadReader cdload_reader;

#endif /* CDLOAD_H */
