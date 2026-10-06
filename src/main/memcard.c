#include "common.h"

#include "records.h"
#include "psyq/libc2.h"
#include "psyq/libmcrd.h"
#include "heap.h"
#include "memcard.h"

MemcardState memcard_state = { 0 };

extern s32 memcard_sync_commands[]; /* MemCardSync command per memcard_run_command mode */

void memcard_set_file_name(void);
s32 memcard_sync(void);
s32 memcard_run_command(s32 arg0, s32 arg1);

/* First entry of the memcard function table memcard_funcs (memcard.h). */
void memcard_init(void) {
    MemCardInit(0);
    MemCardStart();
    heap_funcs.bzero(&memcard_state, sizeof(memcard_state));
    memcard_state.max_retries = 3;
    memcard_state.icon_frames = -1;
    memcard_set_file_name();
    memcard_state.part1_size = 0x100;
    memcard_state.slot_size = 0x2700;
}

/* Sets the save file name (file_name) for the disc's region records_language: 0 Japan, 1 USA, 2-6 Europe. */
void memcard_set_file_name(void) {
    switch (records_language) {
    default:
        memcard_state.file_name = "BISLPS-03446DMW3-JPN";
        break;
    case 0:
        memcard_state.file_name = "BISLPS-03446DMW3-JPN";
        break;
    case 1:
        memcard_state.file_name = "BASLUS-01436DMW3-USA";
        break;
    case 2:
        memcard_state.file_name = "BESLES-03936DMW3-EUR";
        break;
    case 3:
        memcard_state.file_name = "BESLES-03936DMW3-EUR";
        break;
    case 4:
        memcard_state.file_name = "BESLES-03936DMW3-EUR";
        break;
    case 5:
        memcard_state.file_name = "BESLES-03936DMW3-EUR";
        break;
    case 6:
        memcard_state.file_name = "BESLES-03936DMW3-EUR";
        break;
    }
}

/* Sets up the save file header: `title`, the icon palette and `frames` (1-3) icon images. */
void memcard_set_header(char *title, MemcardClut *clut, s32 frames, void **icons) {
    s32 i;

    if (frames < 1 || frames > 3) {
        return;
    }
    if (strlen(title) > sizeof(memcard_state.header.title)) {
        return;
    }
    memcard_state.icon_frames = frames;
    heap_funcs.bzero(&memcard_state.header, sizeof(memcard_state.header));
    memcard_state.header.magic[0] = 'S';
    memcard_state.header.magic[1] = 'C';
    memcard_state.header.type = memcard_state.icon_frames | 0x10;
    memcard_state.header.blocks = 4;
    strcpy(memcard_state.header.title, title);
    memcard_state.header.clut = *clut;
    for (i = 0; i < memcard_state.icon_frames; i++) {
        memcard_state.icons[i] = icons[i];
    }
}

/* Polls the card (MemCardSync); on an error other than 1 (no card) or 3 (new card) asks for a retry
 * (retry) up to max_retries times. Returns 1 when a command has finished, else 0 (or -1). */
s32 memcard_sync(void) {
    s32 cmds;
    s32 result;
    s32 ret = MemCardSync(1, &cmds, &result);

    if (ret != 1) {
        return ret;
    }
    memcard_state.command = cmds;
    memcard_state.result = result;
    if (result == 0 || result == 1 || result == 3) {
        memcard_state.retries = 0;
    } else if (++memcard_state.retries < memcard_state.max_retries) {
        memcard_state.retry = ret;
        return 0;
    } else {
        memcard_state.retries = 0;
        memcard_state.retry = 0;
    }
    return ret;
}

/* Waits for MemCardExist on `chan` (state 1). Call until non-zero: 1 = card there, else result + 1. */
s32 memcard_wait_exist(s32 chan) {
    switch (memcard_state.state) {
    case 0:
    default:
        while (MemCardExist(chan << 4) == 0) {
            memcard_sync();
        }
        memcard_state.state = 1;
        break;
    case 1:
        if (memcard_sync() != 0) {
            memcard_state.state = 0;
            if (memcard_state.result == 0) {
                return 1;
            }
            return memcard_state.result + 1;
        }
        if (memcard_state.retry != 0) {
            memcard_state.retry = 0;
            while (MemCardExist(chan << 4) == 0) {
                memcard_sync();
            }
        }
        break;
    }
    return 0;
}

/* Same with MemCardAccept (state 2). */
s32 memcard_wait_accept(s32 chan) {
    switch (memcard_state.state) {
    case 0:
    default:
        while (MemCardAccept(chan << 4) == 0) {
            memcard_sync();
        }
        memcard_state.state = 2;
        break;
    case 2:
        if (memcard_sync() != 0) {
            memcard_state.state = 0;
            if (memcard_state.result == 0) {
                return 1;
            }
            return memcard_state.result + 1;
        }
        if (memcard_state.retry != 0) {
            memcard_state.retry = 0;
            while (MemCardAccept(chan << 4) == 0) {
                memcard_sync();
            }
        }
        break;
    }
    return 0;
}

/* Reads `size` bytes of the save file into `buf`, 0x80 bytes per call; `part` picks the start: 0 the header,
 * 1 after the icons, 2 after the part1_size bytes that follow, 3 and 4 after one or two more slot_size blocks.
 * Call until it returns non-zero: 1 = done (or bad arguments), 2+ = MemCardSync error + 1. */
s32 memcard_read(s32 chan, void *buf, s32 size, s32 part) {
    u8 *data;

    if (buf == NULL || size == 0) {
        return 1;
    }
    if (memcard_state.icon_frames < 1 || memcard_state.icon_frames > 3) {
        return 1;
    }
    data = buf;
    switch (memcard_state.state) {
    case 0:
    default:
        if (memcard_wait_exist(chan) == 0) {
            return 0;
        }
        switch (memcard_state.result) {
        case 0:
            memcard_state.done = 0;
            switch (part) {
            case 0:
            default:
                memcard_state.offset = 0;
                break;
            case 1:
                memcard_state.offset = memcard_state.icon_frames * 0x80 + 0x80;
                break;
            case 2:
                memcard_state.offset = memcard_state.icon_frames * 0x80 + 0x80 + memcard_state.part1_size;
                break;
            case 3:
                memcard_state.offset = memcard_state.icon_frames * 0x80 + 0x80 + memcard_state.part1_size + memcard_state.slot_size;
                break;
            case 4:
                memcard_state.offset =
                    memcard_state.icon_frames * 0x80 + 0x80 + memcard_state.part1_size + memcard_state.slot_size * 2;
                break;
            }
            while (MemCardReadFile(chan << 4, memcard_state.file_name, (u32 *)data, memcard_state.offset, 0x80) == 0) {
                memcard_sync();
            }
            memcard_state.state = 3;
            break;
        default:
            memcard_state.state = 0;
            return memcard_state.result + 1;
        }
        break;
    case 3:
        if (memcard_sync() != 0) {
            switch (memcard_state.result) {
            case 0:
                memcard_state.done += 0x80;
                if (memcard_state.done >= size) {
                    memcard_state.state = 0;
                    return 1;
                }
                while (MemCardReadFile(chan << 4, memcard_state.file_name, (u32 *)(data + memcard_state.done),
                                       memcard_state.offset + memcard_state.done, 0x80) == 0) {
                    memcard_sync();
                }
                break;
            default:
                memcard_state.state = 0;
                return memcard_state.result + 1;
            }
        } else if (memcard_state.retry != 0) {
            memcard_state.retry = 0;
            memcard_state.done = 0;
            while (MemCardReadFile(chan << 4, memcard_state.file_name, (u32 *)data, memcard_state.offset, 0x80) == 0) {
                memcard_sync();
            }
        }
        break;
    }
    return 0;
}

/* memcard_read's twin for writing; part 0 writes at offset `part >> 8`. */
s32 memcard_write(s32 chan, void *buf, s32 size, s32 part) {
    u8 *data;

    if (buf == NULL || size == 0) {
        return 1;
    }
    if (memcard_state.icon_frames < 1 || memcard_state.icon_frames > 3) {
        return 1;
    }
    data = buf;
    switch (memcard_state.state) {
    case 0:
    default:
        if (memcard_wait_exist(chan) == 0) {
            return 0;
        }
        switch (memcard_state.result) {
        case 0:
            memcard_state.done = 0;
            switch (part & 0xFF) {
            case 0:
            default:
                memcard_state.offset = part >> 8;
                break;
            case 1:
                memcard_state.offset = memcard_state.icon_frames * 0x80 + 0x80;
                break;
            case 2:
                memcard_state.offset = memcard_state.icon_frames * 0x80 + 0x80 + memcard_state.part1_size;
                break;
            case 3:
                memcard_state.offset = memcard_state.icon_frames * 0x80 + 0x80 + memcard_state.part1_size + memcard_state.slot_size;
                break;
            case 4:
                memcard_state.offset =
                    memcard_state.icon_frames * 0x80 + 0x80 + memcard_state.part1_size + memcard_state.slot_size * 2;
                break;
            }
            while (MemCardWriteFile(chan << 4, memcard_state.file_name, (u32 *)data, memcard_state.offset, 0x80) == 0) {
                memcard_sync();
            }
            memcard_state.state = 4;
            break;
        default:
            memcard_state.state = 0;
            return memcard_state.result + 1;
        }
        break;
    case 4:
        if (memcard_sync() != 0) {
            switch (memcard_state.result) {
            case 0:
                memcard_state.done += 0x80;
                if (memcard_state.done >= size) {
                    memcard_state.state = 0;
                    return 1;
                }
                while (MemCardWriteFile(chan << 4, memcard_state.file_name, (u32 *)(data + memcard_state.done),
                                       memcard_state.offset + memcard_state.done, 0x80) == 0) {
                    memcard_sync();
                }
                break;
            default:
                memcard_state.state = 0;
                return memcard_state.result + 1;
            }
        } else if (memcard_state.retry != 0) {
            memcard_state.retry = 0;
            memcard_state.done = 0;
            while (MemCardWriteFile(chan << 4, memcard_state.file_name, (u32 *)data, memcard_state.offset, 0x80) == 0) {
                memcard_sync();
            }
        }
        break;
    }
    return 0;
}

/* Runs a card command once the card is accepted: mode 0 lists the files (file_count/files), 1 creates the save
 * file, 2 formats (only on an unformatted card, result 4), 3 unformats. Call until non-zero: 1 = done,
 * 5 = card not formatted, else MemCardSync error + 1. */
s32 memcard_run_command(s32 chan, s32 mode) {
    switch (memcard_state.state) {
    case 0:
    default:
        if (memcard_wait_accept(chan) == 0) {
            return 0;
        }
        switch (memcard_state.result) {
        case 0:
            memcard_state.state = 5;
            break;
        case 4:
            if (mode == 2) {
                memcard_state.state = 5;
                break;
            }
            memcard_state.state = 0;
            return 5;
        default:
            memcard_state.state = 0;
            return memcard_state.result + 1;
        }
        break;
    case 5:
        if ((memcard_state.result == 0 && (mode == 0 || mode == 1 || mode == 3))
            || (memcard_state.result == 4 && mode == 2)) {
            heap_funcs.bzero(&memcard_state.file_count, sizeof(memcard_state.file_count) + sizeof(memcard_state.files));
            memcard_state.command = memcard_sync_commands[mode];
            switch (mode) {
            case 0:
            default:
                memcard_state.result = MemCardGetDirentry(chan << 4, "*", memcard_state.files,
                                                        &memcard_state.file_count, 0, 15);
                break;
            case 1:
                memcard_state.result = MemCardCreateFile(chan << 4, memcard_state.file_name, 4);
                break;
            case 2:
                memcard_state.result = MemCardFormat(chan << 4);
                break;
            case 3:
                memcard_state.result = MemCardUnformat(chan << 4);
                break;
            }
            if (memcard_state.result == -1) {
                memcard_state.result = 8;
            }
        }
        memcard_state.state = 0;
        if (memcard_state.result == 0) {
            return 1;
        }
        return memcard_state.result + 1;
    }
    return 0;
}

/* The three wrappers below call memcard_run_command with modes 0-2. The `ret == 1` test changes nothing,
 * but the original has it. */
s32 memcard_list_files(s32 arg0) {
    s32 ret = memcard_run_command(arg0, 0);

    if (ret == 1) {
        return 1;
    }
    return ret;
}

s32 memcard_create_file(s32 arg0) {
    s32 ret = memcard_run_command(arg0, 1);

    if (ret == 1) {
        return 1;
    }
    return ret;
}

s32 memcard_format(s32 arg0) {
    s32 ret = memcard_run_command(arg0, 2);

    if (ret == 1) {
        return 1;
    }
    return ret;
}

s32 func_80015528(void) {
    return 2;
}

/* True if the XOR of `size` bytes equals `sum`. */
s32 memcard_check_checksum(u8 *data, s32 size, u8 sum) {
    u8 x = 0;
    s32 i;

    for (i = 0; i < size; i++) {
        x ^= *data++;
    }
    return x == sum;
}

/* XOR of `size` bytes. */
u8 memcard_get_checksum(u8 *data, s32 size) {
    u8 x = 0;
    s32 i;

    for (i = 0; i < size; i++) {
        x ^= *data++;
    }
    return x;
}

MemcardFuncs memcard_funcs = {
    memcard_init,
    memcard_set_file_name,
    memcard_set_header,
    memcard_wait_exist,
    memcard_wait_accept,
    memcard_read,
    memcard_write,
    memcard_list_files,
    memcard_create_file,
    memcard_format,
    func_80015528,
    memcard_check_checksum,
    memcard_get_checksum,
};

s32 memcard_sync_commands[] = { 7, 8, 9, 10 };

/* Unreferenced .sdata after the "*" literal (-G8 puts small literals in .sdata). */
s32 D_8005CCC4[2] = { 0, 0 };
u8 D_8005CCCC[4] = { 0x80, 0x80, 0x80, 0 }; /* a grey RGB colour? */
