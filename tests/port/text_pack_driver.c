/* tests/port/mods.py's driver for port/game/text_pack.c, compiled with it alone (its runtime and game calls stubbed
 * here): text_pack_driver PACK SECTORS DIR ID...: opens PACK with the file table's sector counts from SECTORS (u16
 * per file ID), then for each ID loads DIR/<ID>.in (the file's sectors as the disc gives them) into a buffer laid out
 * as cdload's heap block of port_text_sectors(ID) sectors, runs port_text_loaded on it and writes the buffer to
 * DIR/<ID>.out. Prints each ID's port_text_sectors. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filetable.h"
#include "heap.h"
#include "port_runtime.h"
#include "psxstack/platform.h"
#include "text_pack.h"

s32 port_text_sectors(s32 id);
void port_text_loaded(s32 id, void *buffer);

static u16 drv_sectors[FILETABLE_COUNT];
long port_frames;

static s32 drv_get_sectors(s32 id) {
    return id >= 0 && id < FILETABLE_COUNT ? drv_sectors[id] : 0;
}

FiletableFuncs filetable_funcs = { NULL, drv_get_sectors, NULL, NULL };

void port_log(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fputs("port: ", stderr);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
}

int port_exe_path(char *out, size_t out_size) {
    (void)out, (void)out_size;
    return 0; /* the driver names its pack through DW3_PORT_TEXT_PACK */
}

const char *port_path_last_sep(const char *path) {
    return strrchr(path, '/');
}

static int drv_file(const char *path, void *buf, size_t n, int write) {
    FILE *f = fopen(path, write ? "wb" : "rb");
    size_t done;
    if (f == NULL) {
        return 0;
    }
    done = write ? fwrite(buf, 1, n, f) : fread(buf, 1, n, f);
    fclose(f);
    return done == n;
}

int main(int argc, char **argv) {
    char path[4096];
    int i;
    if (argc < 4) {
        fprintf(stderr, "usage: text_pack_driver PACK SECTORS DIR ID...\n");
        return 2;
    }
    if (!drv_file(argv[2], drv_sectors, sizeof(drv_sectors), 0)) {
        fprintf(stderr, "text_pack_driver: cannot read %s\n", argv[2]);
        return 2;
    }
    setenv("DW3_PORT_TEXT_PACK", argv[1], 1);
    if (!port_text_pack_open("unused")) {
        return 1;
    }
    for (i = 4; i < argc; i++) {
        s32 id = (s32)strtol(argv[i], NULL, 0);
        size_t in = (size_t)drv_sectors[id] << 11, cap = (size_t)port_text_sectors(id) << 11;
        /* a heap block: its header, the data, the next block's header (where the data's room ends) */
        HeapBlock *block = calloc(1, sizeof(HeapBlock) * 2 + cap);
        u8 *data = (u8 *)(block + 1);
        if (block == NULL) {
            return 2;
        }
        block->next = (HeapBlock *)(data + cap);
        memset(data, 0xAA, cap); /* what the heap held: the rebuild must clear what it does not write */
        snprintf(path, sizeof(path), "%s/%d.in", argv[3], (int)id);
        if (!drv_file(path, data, in, 0)) {
            fprintf(stderr, "text_pack_driver: cannot read %s\n", path);
            return 2;
        }
        port_text_loaded(id, data);
        snprintf(path, sizeof(path), "%s/%d.out", argv[3], (int)id);
        if (!drv_file(path, data, cap, 1)) {
            return 2;
        }
        printf("%d %d\n", (int)id, (int)port_text_sectors(id));
        free(block);
    }
    return 0;
}
