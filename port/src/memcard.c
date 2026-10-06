/* The memory cards (port_harness.h port_memcard_open): raw 128 KB .mcd images (PC_PORT_PLAN 2.8; DECISIONS "PC port
 * decisions (session 15)" item 7), the format PCSX-Redux reads and writes, so a card written here loads in the
 * emulator and the other way round. This file owns the images and their files; the card's file system and LIBMCRD's
 * commands are the shim's (port/psyq/libmcrd.c), which calls port_memcard_written after each command that changed
 * an image. A slot without port_memcard_open has no card. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "port_harness.h"
#include "port_runtime.h"
#include "psyq.h"

#define PORT_MEMCARD_SIZE 0x20000


typedef struct PortMemcard {
    u8 image[PORT_MEMCARD_SIZE];
    char *path; /* the .mcd file written back after each change; NULL: in memory only */
} PortMemcard;

static PortMemcard *port_memcards[2];

/* Writes the image to its file (whole: 128 KB through a temporary file and a rename, so that a crash leaves the old
 * card or the new one, never half of each). */
static void port_memcard_written(int slot) {
    PortMemcard *card = port_memcards[slot];
    char *tmp;
    FILE *f;

    if (card == NULL || card->path == NULL) {
        return;
    }
    tmp = malloc(strlen(card->path) + 5);
    if (tmp == NULL) {
        port_fatal("memcard: out of memory");
    }
    sprintf(tmp, "%s.tmp", card->path);
    f = fopen(tmp, "wb");
    if (f == NULL || fwrite(card->image, 1, PORT_MEMCARD_SIZE, f) != PORT_MEMCARD_SIZE || fclose(f) != 0) {
        port_fatal("memcard %d: cannot write %s: %s", slot + 1, tmp, strerror(errno));
    }
    if (rename(tmp, card->path) != 0) {
        port_fatal("memcard %d: cannot rename %s to %s: %s", slot + 1, tmp, card->path, strerror(errno));
    }
    free(tmp);
}

void port_memcard_open(int slot, const char *path) {
    PortMemcard *card;
    FILE *f;

    if (slot < 0 || slot > 1) {
        port_fatal("memcard: no slot %d", slot + 1);
    }
    card = port_memcards[slot];
    if (card == NULL) {
        card = calloc(1, sizeof(*card));
        if (card == NULL) {
            port_fatal("memcard: out of memory");
        }
        port_memcards[slot] = card;
    }
    free(card->path);
    card->path = NULL;
    memset(card->image, 0, sizeof(card->image));
    if (path == NULL) {
        psyq_mcrd_format_image(card->image);
        port_log("memcard %d: a new formatted card (in memory)", slot + 1);
    } else {
        card->path = strdup(path);
        if (card->path == NULL) {
            port_fatal("memcard: out of memory");
        }
        f = fopen(path, "rb");
        if (f == NULL && errno == ENOENT) {
            psyq_mcrd_format_image(card->image);
            port_memcard_written(slot);
            port_log("memcard %d: %s: a new formatted card", slot + 1, path);
        } else if (f == NULL) {
            port_fatal("memcard %d: cannot open %s: %s", slot + 1, path, strerror(errno));
        } else {
            size_t n = fread(card->image, 1, sizeof(card->image), f);
            int extra = fgetc(f);
            fclose(f);
            if (n != sizeof(card->image) || extra != EOF) {
                port_fatal("memcard %d: %s is not a raw 128 KB card image (.mcd)", slot + 1, path);
            }
            port_log("memcard %d: %s", slot + 1, path);
        }
    }
    psyq_mcrd_set_card(slot, card->image, port_memcard_written);
}
