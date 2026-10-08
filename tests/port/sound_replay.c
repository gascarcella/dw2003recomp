/* tests/port/sound_replay.c: the port's LIBSND and SPU core replayed on the emulator's timeline (tests/port/sound.py).
 *
 * The game calls LIBSND at frames that depend on CPU time the port does not model (the PS1 drops frames, the CD and
 * the movie take their own time), so a port run's trace and the emulator's interleave the game's calls with the
 * sequencer's ticks differently. This program takes the game out: sound.py turns an emulator trace (with its
 * `--calls` comments) into a replay script, and this program makes exactly those LIBSND calls at exactly those ticks,
 * with the vsync (the sequencer's tick, LIBSND's flush) where the emulator had it, and writes the SPU trace LIBSND
 * makes. sound.py then requires it to equal the emulator's trace, store for store and tick for tick.
 *
 * Script (one item per line; `#` comments):
 *   data <n> <path>                         a file (a bank's header or body file, as the game loads it) as buffer n
 *   vsync <index> <tick>                    the emulator's vsync of `tick`, which came after its first `index` events
 *   call <tick> <Function> <args...>        a LIBSND call; a pointer argument is `<n>+<offset>` into buffer n
 *   cdinit <tick>                           LIBCD's CdInit (its five SPU stores)
 *   call <tick> reset                       the console's reset (a port run's): LIBSND's and the SPU's state cleared
 * A vsync runs before any call of its tick or a later one; one that the emulator had in the middle of a call (SsInit's
 * work-area clearing outlasts its frame) runs when the call's store count reaches its index. Before each vsync the
 * SPU core renders a PAL frame (882 samples), as the port's audio output does between two vsyncs, so the envelopes
 * LIBSND reads at the flush are where the port's would be.
 *
 * Usage: sound_replay SCRIPT OUT.trace [SAMPLES_PER_VSYNC_X10] (default 8820: the port's 882) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libsnd_internal.h"
#include "psyq/libsnd.h"
#include "sha1.h"

/* ---- The shim pieces libsnd*.c use ---- */
int psyq_trace_state = 0;

int psyq_trace_decide(void) {
    psyq_trace_state = 0;
    return 0;
}

void psyq_trace_printf(const char *fmt, ...) {
    (void)fmt;
}

void port_unimplemented(const char *fn) {
    fprintf(stderr, "sound_replay: unimplemented: %s\n", fn);
    exit(3);
}

static int video_mode = 1; /* MODE_PAL; the 4th argument "ntsc": 0 (the 60 Hz game: SsSetTickMode(0x1000) ticks at 60) */

int SetVideoMode(int mode) {
    int prev = video_mode;
    video_mode = mode;
    return prev;
}

/* ---- The script ---- */
#define MAX_DATA 256
#define MAX_ARGS 8

typedef struct Item {
    char kind;          /* 'v' vsync, 'c' call, 'i' cdinit */
    long tick, index;
    char name[32];
    int nargs;
    long arg[MAX_ARGS];
    int ptr_buf[MAX_ARGS]; /* -1, or the buffer a pointer argument points into */
} Item;

static u8 *data_buf[MAX_DATA];
static Item *items;
static long n_items;
static long *vsync_item; /* indexes of the vsync items, in order */
static long n_vsyncs, next_vsync;
static long events, cur_tick;
static int in_vsync_run;
static FILE *out;

static void fatal(const char *msg, const char *detail) {
    fprintf(stderr, "sound_replay: %s%s\n", msg, detail);
    exit(2);
}

static u8 *load_file(const char *path) {
    FILE *f = fopen(path, "rb");
    long size;
    u8 *buf;

    if (f == NULL) {
        fatal("cannot read ", path);
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = calloc(1, (size_t)size + 0x10000); /* room for reads past the end (the DMA's last block) */
    if (buf == NULL || fread(buf, 1, (size_t)size, f) != (size_t)size) {
        fatal("cannot load ", path);
    }
    fclose(f);
    return buf;
}

static void read_script(const char *path) {
    FILE *f = fopen(path, "r");
    char line[512];
    long cap = 0;

    if (f == NULL) {
        fatal("cannot read ", path);
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        char *tok, *save = NULL;
        Item it;

        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        memset(&it, 0, sizeof(it));
        tok = strtok_r(line, " \n", &save);
        if (tok == NULL) {
            continue;
        }
        if (strcmp(tok, "data") == 0) {
            int n = atoi(strtok_r(NULL, " \n", &save));
            char *p = strtok_r(NULL, " \n", &save);
            if (n < 0 || n >= MAX_DATA || p == NULL) {
                fatal("bad data line", "");
            }
            data_buf[n] = load_file(p);
            continue;
        }
        if (strcmp(tok, "vsync") == 0) {
            it.kind = 'v';
            it.index = atol(strtok_r(NULL, " \n", &save));
            it.tick = atol(strtok_r(NULL, " \n", &save));
        } else if (strcmp(tok, "cdinit") == 0) {
            it.kind = 'i';
            it.tick = atol(strtok_r(NULL, " \n", &save));
        } else if (strcmp(tok, "call") == 0) {
            it.kind = 'c';
            it.tick = atol(strtok_r(NULL, " \n", &save));
            snprintf(it.name, sizeof(it.name), "%s", strtok_r(NULL, " \n", &save));
            while ((tok = strtok_r(NULL, " \n", &save)) != NULL && it.nargs < MAX_ARGS) {
                char *plus = strchr(tok, '+');
                it.ptr_buf[it.nargs] = -1;
                if (plus != NULL) {
                    it.ptr_buf[it.nargs] = atoi(tok);
                    it.arg[it.nargs] = strtol(plus + 1, NULL, 0);
                } else {
                    it.arg[it.nargs] = strtol(tok, NULL, 0);
                }
                it.nargs++;
            }
        } else {
            fatal("bad line: ", tok);
        }
        if (n_items == cap) {
            cap = cap ? cap * 2 : 4096;
            items = realloc(items, (size_t)cap * sizeof(Item));
        }
        items[n_items++] = it;
    }
    fclose(f);
    vsync_item = malloc((size_t)(n_items + 1) * sizeof(long));
    for (long i = 0; i < n_items; i++) {
        if (items[i].kind == 'v') {
            vsync_item[n_vsyncs++] = i;
        }
    }
}

static long rate_x10 = 8820; /* samples per vsync, x 10 */
static long rate_acc;

static void run_vsync(void) {
    static int16_t frame[1000 * 2];
    const Item *v = &items[vsync_item[next_vsync++]];
    long samples;

    in_vsync_run = 1;
    cur_tick = v->tick;
    rate_acc += rate_x10;
    samples = rate_acc / 10;
    rate_acc %= 10;
    spu_render(frame, (int)samples);
    SsSeqCalledTbyT();
    in_vsync_run = 0;
}

/* Every vsync of `tick` and before. */
static void vsyncs_until(long tick) {
    while (next_vsync < n_vsyncs && items[vsync_item[next_vsync]].tick <= tick) {
        run_vsync();
    }
}

/* ---- The trace (spu_trace.c's format) ---- */
static const char *const voice_regs[8] = { "vol.l", "vol.r", "pitch", "addr", "adsr.lo", "adsr.hi", "adsr.vol", "loop" };
static const char *const control_regs[32] = {
    "mvol.l", "mvol.r", "rvol.l", "rvol.r", "kon.lo", "kon.hi", "koff.lo", "koff.hi",
    "pmon.lo", "pmon.hi", "non.lo", "non.hi", "eon.lo", "eon.hi", "endx.lo", "endx.hi",
    "unk_da0", "rev.base", "irq.addr", "xfer.addr", "xfer.fifo", "spucnt", "xfer.ctrl", "spustat",
    "cdvol.l", "cdvol.r", "extvol.l", "extvol.r", "curvol.l", "curvol.r", "unk_dbc", "unk_dbe",
};
static const char *const reverb_regs[32] = {
    "dAPF1", "dAPF2", "vIIR", "vCOMB1", "vCOMB2", "vCOMB3", "vCOMB4", "vWALL",
    "vAPF1", "vAPF2", "mLSAME", "mRSAME", "mLCOMB1", "mRCOMB1", "mLCOMB2", "mRCOMB2",
    "dLSAME", "dRSAME", "mLDIFF", "mRDIFF", "mLCOMB3", "mRCOMB3", "mLCOMB4", "mRCOMB4",
    "dLDIFF", "dRDIFF", "mLAPF1", "mRAPF1", "mLAPF2", "mRAPF2", "vLIN", "vRIN",
};

static void hook(uint32_t offset, uint16_t value, const uint16_t *dma, uint32_t halfwords) {
    /* a vsync the emulator had inside a call: before this store */
    if (!in_vsync_run && next_vsync < n_vsyncs && items[vsync_item[next_vsync]].index == events) {
        run_vsync();
    }
    events++;
    if (dma == NULL) {
        char name[24];
        offset &= 0x1FE;
        if (offset < 0x180) {
            snprintf(name, sizeof(name), "v%02u.%s", (unsigned)(offset >> 4), voice_regs[(offset & 0xF) >> 1]);
        } else if (offset < 0x1C0) {
            snprintf(name, sizeof(name), "%s", control_regs[(offset - 0x180) >> 1]);
        } else {
            snprintf(name, sizeof(name), "rev.%s", reverb_regs[(offset - 0x1C0) >> 1]);
        }
        fprintf(out, "%ld %03x %s %04x\n", cur_tick, (unsigned)(0xC00 + offset), name, value);
    } else {
        PortSha1 c;
        uint8_t digest[20], le[2];
        char hex[41];
        port_sha1_init(&c);
        for (uint32_t i = 0; i < halfwords; i++) {
            le[0] = (uint8_t)(dma[i] & 0xFF);
            le[1] = (uint8_t)(dma[i] >> 8);
            port_sha1_update(&c, le, 2);
        }
        port_sha1_final(&c, digest);
        port_sha1_hex(digest, hex);
        fprintf(out, "%ld dma4 spu=%05x len=%u sha1=%s\n", cur_tick, (unsigned)offset, (unsigned)(halfwords * 2), hex);
    }
}

/* ---- The calls ---- */
static u8 *ptr_arg(const Item *it, int i) {
    if (it->ptr_buf[i] < 0 || data_buf[it->ptr_buf[i]] == NULL) {
        fatal("a pointer argument without its data: ", it->name);
    }
    return data_buf[it->ptr_buf[i]] + it->arg[i];
}

static void call(const Item *it) {
    const long *a = it->arg;
    const char *n = it->name;

    fprintf(out, "# %ld call %s\n", cur_tick, n);
    if (strcmp(n, "reset") == 0) { /* the console's reset: the shim's LIBSND, then the SPU (psxstack/runtime/reset.c) */
        psyq_snd_reset();
        spu_reset();
    } else if (strcmp(n, "SsInit") == 0) {
        SsInit();
    } else if (strcmp(n, "SsSetTableSize") == 0) {
        SsSetTableSize(NULL, (s16)a[0], (s16)a[1]);
    } else if (strcmp(n, "SsSetTickMode") == 0) {
        SsSetTickMode((s32)a[0]);
    } else if (strcmp(n, "SsStart2") == 0) {
        SsStart2();
    } else if (strcmp(n, "SsSetMVol") == 0) {
        SsSetMVol((s16)a[0], (s16)a[1]);
    } else if (strcmp(n, "SsSetSerialAttr") == 0) {
        SsSetSerialAttr((char)a[0], (char)a[1], (char)a[2]);
    } else if (strcmp(n, "SsSetSerialVol") == 0) {
        SsSetSerialVol((char)a[0], (s16)a[1], (s16)a[2]);
    } else if (strcmp(n, "SsUtSetReverbType") == 0) {
        SsUtSetReverbType((s16)a[0]);
    } else if (strcmp(n, "SsUtSetReverbDepth") == 0) {
        SsUtSetReverbDepth((s16)a[0], (s16)a[1]);
    } else if (strcmp(n, "SsUtReverbOn") == 0) {
        SsUtReverbOn();
    } else if (strcmp(n, "SsVabOpenHeadSticky") == 0) {
        SsVabOpenHeadSticky(ptr_arg(it, 0), (s16)a[1], (u32)a[2]);
    } else if (strcmp(n, "SsVabTransBody") == 0) {
        SsVabTransBody(ptr_arg(it, 0), (s16)a[1]);
    } else if (strcmp(n, "SsVabTransCompleted") == 0) {
        SsVabTransCompleted((s16)a[0]);
    } else if (strcmp(n, "SsVabClose") == 0) {
        SsVabClose((s16)a[0]);
    } else if (strcmp(n, "SsSepOpen") == 0) {
        SsSepOpen((u32 *)ptr_arg(it, 0), (s16)a[1], (s16)a[2]);
    } else if (strcmp(n, "SsSepPlay") == 0) {
        SsSepPlay((s16)a[0], (s16)a[1], (char)a[2], (s16)a[3]);
    } else if (strcmp(n, "SsSepStop") == 0) {
        SsSepStop((s16)a[0], (s16)a[1]);
    } else if (strcmp(n, "SsSepClose") == 0) {
        SsSepClose((s16)a[0]);
    } else if (strcmp(n, "SsSepSetVol") == 0) {
        SsSepSetVol((s16)a[0], (s16)a[1], (s16)a[2], (s16)a[3]);
    } else if (strcmp(n, "SsSepSetDecrescendo") == 0) {
        SsSepSetDecrescendo((s16)a[0], (s16)a[1], (s16)a[2], (s32)a[3]);
    } else if (strcmp(n, "SsUtAllKeyOff") == 0) {
        SsUtAllKeyOff((s16)a[0]);
    } else if (strcmp(n, "SsUtKeyOn") == 0) {
        SsUtKeyOn((s16)a[0], (s16)a[1], (s16)a[2], (s16)a[3], (s16)a[4], (s16)a[5], (s16)a[6]);
    } else if (strcmp(n, "SsUtKeyOff") == 0) {
        SsUtKeyOff((s16)a[0], (s16)a[1], (s16)a[2], (s16)a[3], (s16)a[4]);
    } else {
        fatal("unknown call ", n);
    }
}

int main(int argc, char **argv) {
    long i;

    if (argc < 3 || argc > 5 || (argc == 5 && strcmp(argv[4], "ntsc") != 0 && strcmp(argv[4], "pal") != 0)) {
        fprintf(stderr, "usage: sound_replay SCRIPT OUT.trace [SAMPLES_PER_VSYNC_X10 [pal|ntsc]]\n");
        return 2;
    }
    if (argc == 5) {
        video_mode = strcmp(argv[4], "ntsc") == 0 ? 0 : 1;
    }
    if (argc >= 4) {
        rate_x10 = atol(argv[3]);
        if (rate_x10 < 1000 || rate_x10 > 9990) {
            fatal("bad samples per vsync: ", argv[3]);
        }
    }
    read_script(argv[1]);
    out = fopen(argv[2], "w");
    if (out == NULL) {
        fatal("cannot write ", argv[2]);
    }
    fprintf(out, "# dw2003 spu trace v1\n# LIBSND replayed on the emulator's timeline (tests/port/sound_replay.c)\n");
    spu_init();
    spu_set_write_hook(hook);
    for (i = 0; i < n_items; i++) {
        const Item *it = &items[i];
        if (it->kind == 'v') {
            continue;
        }
        vsyncs_until(it->tick);
        if (cur_tick < it->tick) {
            cur_tick = it->tick;
        }
        if (it->kind == 'i') {
            /* LIBCD's CdInit: main volume (when the current one reads 0: the emulator's always does), CD volume, the
             * CD input on */
            spu_write16(0x180, 0x3FFF);
            spu_write16(0x182, 0x3FFF);
            spu_write16(0x1B0, 0x3FFF);
            spu_write16(0x1B2, 0x3FFF);
            spu_write16(0x1AA, 0xC001);
        } else {
            call(it);
        }
    }
    while (next_vsync < n_vsyncs) {
        run_vsync();
    }
    fprintf(out, "# events %ld\n", events);
    fclose(out);
    return 0;
}
