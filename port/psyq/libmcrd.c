/* port/psyq/libmcrd.c: LIBMCRD over raw 128 KB memory card images (docs/PORT.md "Disc, memory cards and movies"). The port runtime
 * (port/src/memcard.c) owns the images and their files and inserts them with psyq_mcrd_set_card; this file is the
 * card's file system (the layout of psx-spx "Memory Card Data Format") and LIBMCRD's command interface over it, as
 * the game's src/main/memcard.c and STGMCARD use it.
 *
 * The card: 16 blocks of 8 KB, each 64 frames of 128 bytes. Block 0 is the directory: frame 0 the header ("MC"),
 * frames 1-15 one entry per data block (allocation state, file size, next block, name, XOR checksum in byte 0x7F),
 * frames 16-35 the broken-frame list (none: FFFFFFFF), frame 63 a copy of frame 0. Blocks 1-15 hold the files, a file
 * being a chain of blocks (0x51 first, 0x52 middle, 0x53 last; 0xA0 free).
 *
 * The commands, as LIBMCRD documents them and as checked against the emulator (session 16, first_battle_save):
 * - MemCardExist, MemCardAccept, MemCardReadFile, MemCardWriteFile are asynchronous: they return 1 when the command
 *   is registered (0 while another one is running), and MemCardSync reports it finished with its number in *cmds
 *   (1 Exist, 2 Accept, 3 ReadFile, 4 WriteFile: the numbers the emulator's run stores in memcard_state.command) and
 *   its result: 0 McErrNone, 1 McErrCardNotExist, 2 McErrCardInvalid, 3 McErrNewCard, 4 McErrNotFormat,
 *   5 McErrFileNotExist (memcard.c retries every result but 0, 1 and 3 up to three times).
 * - MemCardCreateFile, MemCardFormat, MemCardUnformat, MemCardGetDirentry are synchronous (the result at once; 6
 *   McErrAlreadyExist, 7 McErrBlockFull besides the above).
 * - The new-card flag: a card answers its first access after insertion or after the console's reset with
 *   McErrNewCard, which clears the flag (the emulator: the first MemCardAccept of STGMCARD gives 3, the next one 0,
 *   after the boot and again after the script's reset). On the PS1 the BIOS clears it by writing frame 63 (the
 *   emulator's image then holds whatever buffer its BIOS wrote there); here the image is left as it is.
 *
 * Timing: deterministic, counted in MemCardSync polls (the game polls once per frame from its state machine, and
 * spins on MemCardSync when a command is refused): Exist completes at the 2nd poll, Accept at the 4th, a read or a
 * write at the (1 + bytes / 128)th. The work (the copy, the flag) is done at the completing poll, as the transfer
 * ends there. The emulator's card is slower (Exist ~4 frames, Accept ~80, a 128-byte write ~17); the scripts wait on
 * memcard_state, not on frames. MemCardSync(0, ...) (wait) completes the command at once. */
#include <string.h>

#include "psyq_internal.h"
#include "psyq/libmcrd.h"


#define MCRD_CARD_SIZE 0x20000
#define MCRD_FRAME 0x80
#define MCRD_BLOCK 0x2000
#define MCRD_BLOCKS 16 /* block 0 is the directory */

/* Results (LIBMCRD's McErr*). */
#define MCRD_OK 0
#define MCRD_NO_CARD 1
#define MCRD_INVALID 2
#define MCRD_NEW_CARD 3
#define MCRD_NOT_FORMAT 4
#define MCRD_FILE_NOT_EXIST 5
#define MCRD_ALREADY_EXIST 6
#define MCRD_BLOCK_FULL 7

/* Asynchronous command numbers (LIBMCRD's McFunc*). */
#define MCRD_FUNC_EXIST 1
#define MCRD_FUNC_ACCEPT 2
#define MCRD_FUNC_READ_FILE 3
#define MCRD_FUNC_WRITE_FILE 4

/* Directory entries' allocation states. */
#define MCRD_DIR_FIRST 0x51
#define MCRD_DIR_MIDDLE 0x52
#define MCRD_DIR_LAST 0x53
#define MCRD_DIR_FREE 0xA0

typedef struct PsyqCard {
    u8 *image;                  /* MCRD_CARD_SIZE bytes; NULL: no card in the slot */
    void (*written)(int slot);  /* called after a command changed the image (the runtime writes its file back) */
    int fresh;                  /* the new-card flag: set at insertion and at the console's reset */
} PsyqCard;

typedef struct PsyqMcrdCommand {
    int pending;  /* a command is registered and not yet reported */
    s32 func;     /* MCRD_FUNC_* */
    int slot;     /* 0 or 1; -1: a channel without a card (a multitap's) */
    char name[21];
    u8 *addr;
    s32 offset, bytes;
    int polls;    /* MemCardSync polls so far */
    int latency;  /* the poll that completes it */
} PsyqMcrdCommand;

static PsyqCard psyq_mcrd_cards[2];
static PsyqMcrdCommand psyq_mcrd_cmd;

/* ---- The card image */

static u8 *psyq_mcrd_frame(u8 *image, int block, int frame) {
    return image + block * MCRD_BLOCK + frame * MCRD_FRAME;
}

static u8 psyq_mcrd_xor(const u8 *p, int n) {
    u8 x = 0;
    int i;

    for (i = 0; i < n; i++) {
        x ^= p[i];
    }
    return x;
}

static void psyq_mcrd_put32(u8 *p, u32 v) {
    p[0] = (u8)v;
    p[1] = (u8)(v >> 8);
    p[2] = (u8)(v >> 16);
    p[3] = (u8)(v >> 24);
}

static u32 psyq_mcrd_get32(const u8 *p) {
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
}

/* A directory frame: state, size, next (0xFFFF: none), name (NULL: none), then the checksum. */
static void psyq_mcrd_dir_set(u8 *f, u32 state, u32 size, u16 next, const char *name) {
    memset(f, 0, MCRD_FRAME);
    psyq_mcrd_put32(f, state);
    psyq_mcrd_put32(f + 4, size);
    f[8] = (u8)next;
    f[9] = (u8)(next >> 8);
    if (name != NULL) {
        strncpy((char *)f + 0xA, name, 20);
    }
    f[0x7F] = psyq_mcrd_xor(f, 0x7F);
}

/* Writes a formatted directory (block 0) into `image`, the data blocks left as they are. The bytes are those of a new
 * card of PCSX-Redux (the emulator's replay runner starts with one): frame 0 "MC", frames 1-15 free entries, frames
 * 16-35 the empty broken-frame list, frames 36-62 zero, frame 63 a copy of frame 0. */
void psyq_mcrd_format_image(u8 *image) {
    u8 *f;
    int i;

    memset(image, 0, MCRD_BLOCK);
    f = psyq_mcrd_frame(image, 0, 0);
    f[0] = 'M';
    f[1] = 'C';
    f[0x7F] = psyq_mcrd_xor(f, 0x7F);
    for (i = 1; i < MCRD_BLOCKS; i++) {
        psyq_mcrd_dir_set(psyq_mcrd_frame(image, 0, i), MCRD_DIR_FREE, 0, 0xFFFF, NULL);
    }
    for (i = 16; i < 36; i++) {
        psyq_mcrd_dir_set(psyq_mcrd_frame(image, 0, i), 0xFFFFFFFF, 0, 0xFFFF, NULL);
    }
    memcpy(psyq_mcrd_frame(image, 0, 63), psyq_mcrd_frame(image, 0, 0), MCRD_FRAME);
}

/* Inserts the card `image` (MCRD_CARD_SIZE bytes, kept by the caller) into `slot`, NULL removes it. `written` is
 * called with the slot after each command that changed the image. A card inserted is a new card (its first access
 * answers McErrNewCard). */
void psyq_mcrd_set_card(int slot, u8 *image, void (*written)(int slot)) {
    psyq_mcrd_cards[slot].image = image;
    psyq_mcrd_cards[slot].written = written;
    psyq_mcrd_cards[slot].fresh = image != NULL;
}

static int psyq_mcrd_formatted(const u8 *image) {
    return image[0] == 'M' && image[1] == 'C';
}

/* The first block (1-15) of the file `name`, 0 if none. */
static int psyq_mcrd_find(u8 *image, const char *name) {
    int i;

    for (i = 1; i < MCRD_BLOCKS; i++) {
        const u8 *f = psyq_mcrd_frame(image, 0, i);
        if (f[0] == MCRD_DIR_FIRST && strncmp((const char *)f + 0xA, name, 21) == 0) {
            return i;
        }
    }
    return 0;
}

/* The block holding byte `offset` of the file starting at `block`, 0 if the chain ends before. */
static int psyq_mcrd_block_at(u8 *image, int block, s32 offset) {
    s32 n;

    for (n = offset / MCRD_BLOCK; n > 0; n--) {
        const u8 *f = psyq_mcrd_frame(image, 0, block);
        u32 next = f[8] | (f[9] << 8);
        if (next >= MCRD_BLOCKS - 1) {
            return 0;
        }
        block = (int)next + 1;
    }
    return block;
}

/* The channel's slot: 0x00 = port 1, 0x10 = port 2 (a multitap's other channels: -1, no card there). */
static int psyq_mcrd_slot(s32 chan) {
    if ((chan & 0xF) != 0 || (chan >> 4) > 1 || chan < 0) {
        return -1;
    }
    return chan >> 4;
}

/* The card in the channel, NULL if none. */
static PsyqCard *psyq_mcrd_card(int slot) {
    if (slot < 0 || psyq_mcrd_cards[slot].image == NULL) {
        return NULL;
    }
    return &psyq_mcrd_cards[slot];
}

/* A file transfer of the current command: copies, returns the result. */
static s32 psyq_mcrd_transfer(PsyqCard *card, const PsyqMcrdCommand *c) {
    int first = psyq_mcrd_find(card->image, c->name);
    s32 done;
    u32 size;

    if (first == 0) {
        return MCRD_FILE_NOT_EXIST;
    }
    size = psyq_mcrd_get32(psyq_mcrd_frame(card->image, 0, first) + 4);
    if (c->offset < 0 || c->bytes <= 0 || (c->offset | c->bytes) % MCRD_FRAME != 0 || (u32)(c->offset + c->bytes) > size) {
        return MCRD_INVALID;
    }
    for (done = 0; done < c->bytes;) {
        s32 ofs = c->offset + done;
        int block = psyq_mcrd_block_at(card->image, first, ofs);
        s32 n = MCRD_BLOCK - ofs % MCRD_BLOCK;
        u8 *p;
        if (block == 0) {
            return MCRD_INVALID;
        }
        if (n > c->bytes - done) {
            n = c->bytes - done;
        }
        p = card->image + block * MCRD_BLOCK + ofs % MCRD_BLOCK;
        if (c->func == MCRD_FUNC_READ_FILE) {
            memcpy(c->addr + done, p, (size_t)n);
        } else {
            memcpy(p, c->addr + done, (size_t)n);
        }
        done += n;
    }
    return MCRD_OK;
}

/* The current command's work and result, at its completion. */
static s32 psyq_mcrd_complete(void) {
    PsyqMcrdCommand *c = &psyq_mcrd_cmd;
    PsyqCard *card = psyq_mcrd_card(c->slot);
    s32 result;

    if (card == NULL) {
        return MCRD_NO_CARD;
    }
    if (card->fresh) {
        card->fresh = 0;
        return MCRD_NEW_CARD;
    }
    if (c->func == MCRD_FUNC_EXIST) {
        return MCRD_OK;
    }
    if (!psyq_mcrd_formatted(card->image)) {
        return MCRD_NOT_FORMAT;
    }
    if (c->func == MCRD_FUNC_ACCEPT) {
        return MCRD_OK;
    }
    result = psyq_mcrd_transfer(card, c);
    if (result == MCRD_OK && c->func == MCRD_FUNC_WRITE_FILE && card->written != NULL) {
        card->written(c->slot);
    }
    return result;
}

/* ---- LIBMCRD */

/* Registers an asynchronous command: 1, or 0 while another one is pending. */
static s32 psyq_mcrd_start(s32 func, s32 chan, const char *name, void *addr, s32 offset, s32 bytes, int latency) {
    PsyqMcrdCommand *c = &psyq_mcrd_cmd;

    if (c->pending) {
        PSYQ_TRACE("MemCard: command %d refused, %d pending", func, c->func);
        return 0;
    }
    memset(c, 0, sizeof(*c));
    c->pending = 1;
    c->func = func;
    c->slot = psyq_mcrd_slot(chan);
    if (name != NULL) {
        strncpy(c->name, name, 20);
    }
    c->addr = addr;
    c->offset = offset;
    c->bytes = bytes;
    c->latency = latency;
    return 1;
}

/* Real: initialises the driver (the argument: 0 = the pads' and the cards' interrupts shared). */
void MemCardInit(long val) {
    PSYQ_TRACE("MemCardInit %ld", val);
    psyq_mcrd_cmd.pending = 0;
}

void MemCardStart(void) {
    PSYQ_TRACE("MemCardStart");
}

s32 MemCardExist(s32 chan) {
    PSYQ_TRACE("MemCardExist chan %x", chan);
    return psyq_mcrd_start(MCRD_FUNC_EXIST, chan, NULL, NULL, 0, 0, 2);
}

s32 MemCardAccept(s32 chan) {
    PSYQ_TRACE("MemCardAccept chan %x", chan);
    return psyq_mcrd_start(MCRD_FUNC_ACCEPT, chan, NULL, NULL, 0, 0, 4);
}

s32 MemCardReadFile(s32 chan, char *file, u32 *addr, s32 offset, s32 bytes) {
    PSYQ_TRACE("MemCardReadFile chan %x %s ofs 0x%X bytes 0x%X to %u", chan, file, offset, bytes, PSYQ_PTR(addr));
    return psyq_mcrd_start(MCRD_FUNC_READ_FILE, chan, file, addr, offset, bytes, 1 + bytes / MCRD_FRAME);
}

s32 MemCardWriteFile(s32 chan, char *file, u32 *addr, s32 offset, s32 bytes) {
    PSYQ_TRACE("MemCardWriteFile chan %x %s ofs 0x%X bytes 0x%X from %u", chan, file, offset, bytes, PSYQ_PTR(addr));
    return psyq_mcrd_start(MCRD_FUNC_WRITE_FILE, chan, file, addr, offset, bytes, 1 + bytes / MCRD_FRAME);
}

/* mode 0 waits, 1 polls: 1 = the command finished (cmds/result filled), 0 = still running, -1 = none pending. */
s32 MemCardSync(s32 mode, s32 *cmds, s32 *result) {
    PsyqMcrdCommand *c = &psyq_mcrd_cmd;
    s32 r;

    if (!c->pending) {
        return -1;
    }
    if (mode != 0 && ++c->polls < c->latency) {
        return 0;
    }
    r = psyq_mcrd_complete();
    c->pending = 0;
    if (cmds != NULL) {
        *cmds = c->func;
    }
    if (result != NULL) {
        *result = r;
    }
    PSYQ_TRACE("MemCardSync: command %d done, result %d", c->func, r);
    return 1;
}

/* Synchronous: the card in `chan` for a directory command, or the error. */
static s32 psyq_mcrd_sync_card(s32 chan, PsyqCard **out) {
    PsyqCard *card = psyq_mcrd_card(psyq_mcrd_slot(chan));

    *out = card;
    if (card == NULL) {
        return MCRD_NO_CARD;
    }
    return psyq_mcrd_formatted(card->image) ? MCRD_OK : MCRD_NOT_FORMAT;
}

/* Creates the file `file` of `blocks` blocks in the first free ones (its data left as it was). */
s32 MemCardCreateFile(s32 chan, char *file, s32 blocks) {
    PsyqCard *card;
    s32 r = psyq_mcrd_sync_card(chan, &card);
    int list[MCRD_BLOCKS - 1];
    int n = 0, i;

    PSYQ_TRACE("MemCardCreateFile chan %x %s blocks %d", chan, file, blocks);
    if (r != MCRD_OK) {
        return r;
    }
    if (blocks < 1 || blocks > MCRD_BLOCKS - 1 || strlen(file) > 20) {
        return MCRD_INVALID;
    }
    if (psyq_mcrd_find(card->image, file) != 0) {
        return MCRD_ALREADY_EXIST;
    }
    for (i = 1; i < MCRD_BLOCKS && n < blocks; i++) {
        if ((psyq_mcrd_frame(card->image, 0, i)[0] & 0xF0) == MCRD_DIR_FREE) {
            list[n++] = i;
        }
    }
    if (n < blocks) {
        return MCRD_BLOCK_FULL;
    }
    for (i = 0; i < n; i++) {
        u16 next = i + 1 < n ? (u16)(list[i + 1] - 1) : 0xFFFF;
        u32 state = i == 0 ? MCRD_DIR_FIRST : (i + 1 < n ? MCRD_DIR_MIDDLE : MCRD_DIR_LAST);
        psyq_mcrd_dir_set(psyq_mcrd_frame(card->image, 0, list[i]), state, i == 0 ? (u32)blocks * MCRD_BLOCK : 0,
                          next, i == 0 ? file : NULL);
    }
    if (card->written != NULL) {
        card->written(psyq_mcrd_slot(chan));
    }
    return MCRD_OK;
}

s32 MemCardFormat(s32 chan) {
    PsyqCard *card = psyq_mcrd_card(psyq_mcrd_slot(chan));

    PSYQ_TRACE("MemCardFormat chan %x", chan);
    if (card == NULL) {
        return MCRD_NO_CARD;
    }
    psyq_mcrd_format_image(card->image);
    if (card->written != NULL) {
        card->written(psyq_mcrd_slot(chan));
    }
    return MCRD_OK;
}

/* The header frame cleared: the card reads as not formatted (McErrNotFormat). */
s32 MemCardUnformat(s32 chan) {
    PsyqCard *card = psyq_mcrd_card(psyq_mcrd_slot(chan));

    PSYQ_TRACE("MemCardUnformat chan %x", chan);
    if (card == NULL) {
        return MCRD_NO_CARD;
    }
    memset(card->image, 0, MCRD_FRAME);
    if (card->written != NULL) {
        card->written(psyq_mcrd_slot(chan));
    }
    return MCRD_OK;
}

/* The BIOS's file-name pattern: '?' any one character, '*' the rest of the name. */
static int psyq_mcrd_match(const char *pattern, const char *name) {
    for (; *pattern != '\0'; pattern++, name++) {
        if (*pattern == '*') {
            return 1;
        }
        if (*name == '\0' || (*pattern != '?' && *pattern != *name)) {
            return 0;
        }
    }
    return *name == '\0';
}

/* The files matching `name`, from the `offset`th match on, at most `max`: their names, sizes in bytes (the game reads
 * only size); attr is the first block's allocation state (0x51) and head its block number (1-15), next NULL. */
s32 MemCardGetDirentry(s32 chan, char *name, DIRENTRY *dir, s32 *files, s32 offset, s32 max) {
    PsyqCard *card;
    s32 r = psyq_mcrd_sync_card(chan, &card);
    s32 found = 0, n = 0;
    int i;

    PSYQ_TRACE("MemCardGetDirentry chan %x %s ofs %d max %d", chan, name, offset, max);
    if (files != NULL) {
        *files = 0;
    }
    if (r != MCRD_OK) {
        return r;
    }
    for (i = 1; i < MCRD_BLOCKS && n < max; i++) {
        const u8 *f = psyq_mcrd_frame(card->image, 0, i);
        char fname[21];
        if (f[0] != MCRD_DIR_FIRST) {
            continue;
        }
        memcpy(fname, f + 0xA, 20);
        fname[20] = '\0';
        if (!psyq_mcrd_match(name, fname) || found++ < offset) {
            continue;
        }
        memset(&dir[n], 0, sizeof(dir[n]));
        memcpy(dir[n].name, fname, sizeof(dir[n].name));
        dir[n].attr = f[0];
        dir[n].size = (s32)psyq_mcrd_get32(f + 4);
        dir[n].head = i;
        n++;
    }
    if (files != NULL) {
        *files = n;
    }
    return MCRD_OK;
}

/* LIBMCRD's part of the console's reset (psyq.h): no command pending, and every card in a slot is a new card again
 * (the emulator: STGMCARD's first MemCardAccept after the script's reset answers 3); the images are kept. */
void psyq_mcrd_reset(void) {
    int i;

    memset(&psyq_mcrd_cmd, 0, sizeof(psyq_mcrd_cmd));
    for (i = 0; i < 2; i++) {
        psyq_mcrd_cards[i].fresh = psyq_mcrd_cards[i].image != NULL;
    }
}
