#include "common.h"

#include "cdload.h"
#include "filetable.h"
#include "gfx.h"
#include "heap.h"

u32 cdload_sector_header[3]; /* header of the last sector read */

u8 cdload_mode; /* CdControlF parameter (the CdlSetmode mode byte) */

void cdload_sync_callback(s32 status);
u8 *cdload_get_file(s32);

s32 cdload_check_sector(void);
s32 cdload_is_busy(void);
void cdload_start_read(void);
CdloadEntry *cdload_find_entry(s32 id);
CdloadEntry *cdload_find_free_entry(void);
s32 cdload_is_loading(s32);
CdloadEntry *cdload_find_oldest_entry();
void cdload_queue_file(s32 id);
void cdload_update();

/* Checks that the sector just read is the expected one (expected_sector) and advances it; -1 if not. */
s32 cdload_check_sector(void) {
    s32 sector;

    CdGetSector(cdload_sector_header, 3);
    sector = CdPosToInt((CdlLOC *)cdload_sector_header);
    if (sector == cdload_reader.expected_sector) {
        cdload_reader.expected_sector = sector + 1;
        return 0;
    }
    return -1;
}

/* CdReadyCallback handler: copies each sector into dest, pauses after the last one or on error. */
void cdload_ready_callback(s32 intr) {
    if (intr == 1 && cdload_check_sector() == 0) {
        CdGetSector(cdload_reader.dest, 0x200);
        cdload_reader.dest += 0x800;
        if (--cdload_reader.sectors_left != 0) {
            return;
        }
    } else {
        cdload_reader.sectors_left = -1;
    }
    CdReadyCallback(NULL);
    CdControlF(9, NULL);
}

/* CdSyncCallback handler: steps the read through set mode, read, wait and finish. */
void cdload_sync_callback(s32 status) {
    switch (status) {
    case 5:
        if (cdload_reader.busy == 4) {
            CdControlF(9, NULL);
        } else {
            cdload_start_read();
        }
        break;
    case 2:
        switch (cdload_reader.busy) {
        case 1:
            cdload_mode = 0xA0;
            CdControlF(0xE, &cdload_mode);
            cdload_reader.busy++;
            break;
        case 2:
            CdReadyCallback(cdload_ready_callback);
            CdControlF(6, NULL);
            cdload_reader.busy++;
            break;
        case 3:
            cdload_reader.busy = 4;
            break;
        case 4:
            CdSyncCallback(NULL);
            if (cdload_reader.sectors_left == 0) {
                cdload_reader.busy = 0;
                if (cdload_reader.done != NULL) {
                    *cdload_reader.done = 1;
                }
            } else {
                cdload_start_read();
            }
            break;
        }
        break;
    }
}

s32 cdload_is_busy(void) {
    return cdload_reader.busy != 0;
}

void cdload_start_read(void) {
    cdload_reader.busy = 1;
    cdload_reader.expected_sector = cdload_reader.first_sector;
    cdload_reader.dest = cdload_reader.buffer;
    cdload_reader.sectors_left = cdload_reader.sectors;
    CdSyncCallback(cdload_sync_callback);
    CdControlF(2, (u8 *)&cdload_reader.loc);
}

/* Starts reading `sectors` sectors (0 = the whole file) of file `id` from sector `offset` into `buf`. */
void cdload_read(s32 id, s32 offset, s32 sectors, void *buf, s32 *done) {
    if (cdload_is_busy() == 0) {
        cdload_reader.id = id;
        cdload_reader.offset = offset;
        cdload_reader.buffer = buf;
        cdload_reader.done = done;
        if (done != NULL) {
            *done = 0;
        }
        if (sectors == 0) {
            cdload_reader.sectors = filetable_funcs.get_sectors(id);
        } else {
            cdload_reader.sectors = sectors;
        }
        filetable_funcs.get_cdloc(id, offset, &cdload_reader.loc);
        cdload_reader.first_sector = filetable_funcs.get_lba(id) + offset;
        cdload_start_read();
    }
}

CdloadEntry *cdload_find_entry(s32 id) {
    CdloadEntry *entry = cdload_module.entries;
    s32 i;

    for (i = 0; i < 64; i++, entry++) {
        if (entry->id == id) {
            return entry;
        }
    }
    return NULL;
}

CdloadEntry *cdload_find_free_entry(void) {
    CdloadEntry *entry = cdload_module.entries;
    s32 i;

    for (i = 0; i < 64; i++, entry++) {
        if (entry->id == 0) {
            return entry;
        }
    }
    return NULL;
}

/* Returns 1 while file `id` is still loading (starts the load if it isn't queued yet), 0 once done. */
s32 cdload_is_loading(s32 id) {
    CdloadEntry *entry = cdload_find_entry(id);

    if (entry != NULL) {
        entry->last_use = gfx_module.funcs.get_time();
        if (entry->state == 3) {
            return 0;
        }
    } else {
        cdload_queue_file(id);
    }
    return 1;
}

/* Picks the loaded entry used least recently (smallest last_use), the largest file on a tie. */
CdloadEntry *cdload_find_oldest_entry(void) {
    CdloadEntry *best = NULL;
    s32 best_sectors = 0;
    s32 i;
    s32 oldest = gfx_module.funcs.get_time();
    CdloadEntry *entry = cdload_module.entries;

    for (i = 0; i < 64; i++, entry++) {
        if (entry->id != 0 && entry->state == 3 && entry->last_use <= oldest) {
            if (oldest != entry->last_use || filetable_funcs.get_sectors(entry->id) >= best_sectors) {
                best_sectors = filetable_funcs.get_sectors(entry->id);
                oldest = entry->last_use;
                best = entry;
            }
        }
    }
    return best;
}

void cdload_free_oldest(void) {
    CdloadEntry *entry = cdload_find_oldest_entry();

    heap_funcs.free(entry->buffer);
    entry->id = 0;
    entry->buffer = NULL;
    entry->last_use = 0;
    entry->marked = 0;
    entry->state = 0;
}

void cdload_queue_file(s32 id) {
    CdloadEntry *entry = cdload_find_entry(id);

    if (entry != NULL) {
        entry->last_use = gfx_module.funcs.get_time();
        return;
    }
    entry = cdload_find_free_entry();
#ifdef PC_PORT
    /* PC_PORT: the PS1 frees cached files only when the heap runs out (heap.c calls free_oldest), which keeps fewer
     * than 64 in its 1.3 MB; the port's heap is larger, so the table can fill: free the least recent file then (on the
     * PS1 a full table would write through NULL). Only which files stay cached changes, so only the CD timing. */
    if (entry == NULL) {
        cdload_free_oldest();
        entry = cdload_find_free_entry();
    }
#endif
    entry->id = id;
    entry->buffer = heap_funcs.alloc(filetable_funcs.get_sectors(id) << 11, 3);
    entry->state = 1;
    entry->last_use = 0;
    entry->marked = 0;
    cdload_module.queued = 1;
}

/* Starts reading the first queued entry (state 1 -> 2) once the reader is idle, marks the one read
 * before as loaded (2 -> 3); clears the "work queued" flag when nothing is left. */
void cdload_update(void) {
    CdloadEntry *entry;
    s32 i;
    s32 pending;
    s32 started;

    if (cdload_module.queued == 0) {
        return;
    }
    if (cdload_reader.is_busy() == 1) {
        return;
    }
    entry = cdload_module.entries;
    started = 0;
    pending = 0;
    for (i = 0; i < 64; i++, entry++) {
        if (entry->id != 0) {
            switch (entry->state) {
            case 2:
                entry->state = 3;
                pending = 1;
                entry->last_use = gfx_module.funcs.get_time();
                break;
            case 1:
                pending = 1;
                if (!started) {
                    cdload_reader.read(entry->id, 0, 0, entry->buffer, NULL);
                    entry->state = 2;
                    started = 1;
                    entry->last_use = gfx_module.funcs.get_time();
                }
                break;
            }
        }
    }
    if (!pending) {
        cdload_module.queued = 0;
    }
}

void cdload_load_file(s32 id) {
    cdload_queue_file(id);
    do {
        cdload_update();
        PLATFORM_WAIT(); /* the CD interrupt ends this loop */
    } while (cdload_is_loading(id) != 0);
}

/* Returns file `id`'s buffer, loading it now (blocking) if it isn't loaded yet. */
u8 *cdload_get_file(s32 id) {
    CdloadEntry *entry = cdload_find_entry(id);

    if (entry != NULL && entry->state == 3) {
        entry->last_use = gfx_module.funcs.get_time();
        return entry->buffer;
    }
    while (cdload_reader.is_busy() != 0) {
        PLATFORM_WAIT();
    }
    cdload_load_file(id);
    return cdload_find_entry(id)->buffer;
}

void cdload_free_file(s32 id) {
    CdloadEntry *entry = cdload_find_entry(id);

    if (entry != NULL && entry->state == 3) {
        heap_funcs.free(entry->buffer);
        entry->id = 0;
        entry->buffer = NULL;
        entry->last_use = 0;
        entry->marked = 0;
        entry->state = 0;
    }
}

/* Frees every entry. */
void cdload_free_all(void) {
    CdloadEntry *entry = cdload_module.entries;
    s32 i;

    for (i = 0; i < 64; i++, entry++) {
        if (entry->id != 0) {
            heap_funcs.free(entry->buffer);
            entry->id = 0;
            entry->buffer = NULL;
            entry->last_use = 0;
            entry->marked = 0;
            entry->state = 0;
        }
    }
}

/* Frees every entry whose buffer ends at or above `addr`. */
void cdload_free_above(u8 *addr) {
    CdloadEntry *entry = cdload_module.entries;
    s32 i;

    for (i = 0; i < 64; i++, entry++) {
        if (entry->id != 0) {
            u8 *end = (u8 *)entry->buffer + 0x20;

            end += filetable_funcs.get_sectors(entry->id) << 11;
            if (end >= addr) {
                heap_funcs.free(entry->buffer);
                entry->id = 0;
                entry->buffer = NULL;
                entry->last_use = 0;
                entry->marked = 0;
                entry->state = 0;
            }
        }
    }
}

void *cdload_get_subfile_by_id(u32 id) {
    s32 index = id & 0xFFFF;
    s32 *data = (s32 *)cdload_get_file(id >> 16);

    return PTR_ADD(void *, data[index], data);
}

void *cdload_get_subfile(u16 index, s32 *data) {
    return PTR_ADD(void *, data[index], data);
}

void cdload_mark_loaded(void) {
    s32 i;
    CdloadEntry *entry = cdload_module.entries;

    for (i = 0; i < 64; i++, entry++) {
        if (entry->id == 0) {
            entry->marked = 0;
        } else {
            entry->marked = 1;
        }
    }
}

/* Entries marked by cdload_mark_loaded (marked) get an old timestamp, so cdload_find_oldest_entry picks them first. */
void cdload_age_marked(void) {
    CdloadEntry *entry = cdload_module.entries;
    s32 time = gfx_module.funcs.get_time() - 10;
    s32 i;

    for (i = 0; i < 64; i++, entry++) {
        if (entry->id != 0 && entry->marked != 0) {
            entry->last_use = time;
            entry->marked = 0;
        }
    }
}

CdloadReader cdload_reader = {
    .is_busy = cdload_is_busy,
    .read = cdload_read,
};

/* cdload_get_subfile takes a u16 index; its callers pass an s32. */
CdloadModule cdload_module = {
    .is_loading = cdload_is_loading,
    .free_oldest = cdload_free_oldest,
    .queue_file = cdload_queue_file,
    .update = cdload_update,
    .files = { cdload_get_file, cdload_free_file, cdload_free_all, cdload_free_above },
    .get_subfile_by_id = cdload_get_subfile_by_id,
    .get_subfile = (void *(*)(s32, s32 *))cdload_get_subfile,
    .mark_loaded = cdload_mark_loaded,
    .age_marked = cdload_age_marked,
};
