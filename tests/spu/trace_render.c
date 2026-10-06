/* Renders an SPU write trace (tests/sound's format, prepared by tests/spu/render_trace.py) through the port's SPU core
 * into a WAV file, and reports what can be checked without listening: RMS and peak per second, clipped samples, every
 * key-on (did the voice start; did its sample loop or stop at its end as the sample's flags say, and when), the DMA
 * blocks landing where the trace says (through the write hook), and the render speed.
 *
 *   trace_render CMDS BLOB OUT.wav [--frames-per-tick N]   (N may be fractional)
 *
 * CMDS is text: "tick N" (the stores that follow belong to tick N), "w OFF VAL" (a register store), "d ADDR LEN POS"
 * (a DMA block of LEN bytes, at BLOB offset POS, which the trace says lands at SPU address ADDR). Each tick's stores are
 * applied, then N frames are rendered (882 = 44,100 / 50, a PAL tick). The rendering is deterministic (the WAV is the
 * same bytes every run); only the speed line reads the clock. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "spu_internal.h"

#define MAX_EVENTS 8192

/* A key-on and what became of it. */
typedef struct KeyOn {
    long tick;
    int voice;
    int looping;          /* the sample's first loop-end block has the repeat flag */
    double end_samples;   /* output samples to that block's end, at the pitch the voice had at its key-on */
    int off_at_once;      /* keyed off in the same tick (SsInit does that to all 24 voices) */
    int started;          /* ENVX > 0 at the end of the tick */
    int first_end;        /* 0 none yet, 1 a loop (end + repeat), 2 a mute (end without repeat) */
    long first_end_at;    /* samples from the key-on */
    long age;             /* samples since the key-on, while it is the voice's latest */
} KeyOn;

static KeyOn keyons[MAX_EVENTS];
static int nkeyons;
static int active[SPU_VOICES];
static uint32_t prev_loops[SPU_VOICES], prev_mutes[SPU_VOICES];
static uint32_t expect_dma_addr;
static long dma_blocks, dma_misplaced;

static void hook(uint32_t offset, uint16_t value, const uint16_t *dma, uint32_t halfwords) {
    (void)value;
    (void)halfwords;
    if (dma != NULL) {
        dma_blocks++;
        if (offset != expect_dma_addr) {
            dma_misplaced++;
            fprintf(stderr, "DMA block at %05x, the trace says %05x\n", offset, expect_dma_addr);
        }
    }
}

/* Walks a sample from `start` to its first loop-end block: the number of blocks and whether that block repeats. */
static void walk_sample(uint32_t start, int *blocks, int *looping) {
    const uint8_t *ram = spu_ram();
    uint32_t a = start;
    int n;

    for (n = 1; n < 0x8000; n++) {
        uint8_t flags = ram[(a + 1) & (SPU_RAM_SIZE - 1)];
        if (flags & 1) {
            break;
        }
        a = (a + 16) & (SPU_RAM_SIZE - 1);
    }
    *blocks = n;
    *looping = (ram[(a + 1) & (SPU_RAM_SIZE - 1)] & 2) != 0;
}

static void key_on_event(int voice, long tick) {
    KeyOn *e;
    int blocks;
    uint16_t pitch = spu_read16((uint32_t)voice * 16 + 4);

    if (nkeyons == MAX_EVENTS) {
        return;
    }
    e = &keyons[nkeyons];
    memset(e, 0, sizeof(*e));
    e->tick = tick;
    e->voice = voice;
    walk_sample((uint32_t)spu_read16((uint32_t)voice * 16 + 6) * 8, &blocks, &e->looping);
    e->end_samples = pitch ? blocks * 28.0 * 4096.0 / (pitch > 0x4000 ? 0x4000 : pitch) : 1e30;
    active[voice] = nkeyons++;
}

/* After each frame: the first loop or mute of each voice's latest key-on. */
static void watch_voices(void) {
    int v;

    for (v = 0; v < SPU_VOICES; v++) {
        SpuVoiceInfo info;
        KeyOn *e;
        if (active[v] < 0) {
            continue;
        }
        e = &keyons[active[v]];
        spu_voice_info(v, &info);
        e->age++;
        if (e->first_end == 0 && (info.loops != prev_loops[v] || info.mutes != prev_mutes[v])) {
            e->first_end = info.mutes != prev_mutes[v] ? 2 : 1;
            e->first_end_at = e->age;
        }
        prev_loops[v] = info.loops;
        prev_mutes[v] = info.mutes;
    }
}

static void put32le(FILE *f, uint32_t v) {
    uint8_t b[4] = {(uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24)};
    fwrite(b, 1, 4, f);
}

static void wav_header(FILE *f, uint32_t frames) {
    fwrite("RIFF", 1, 4, f);
    put32le(f, 36 + frames * 4);
    fwrite("WAVEfmt ", 1, 8, f);
    put32le(f, 16);
    put32le(f, 1 | 2u << 16); /* PCM, 2 channels */
    put32le(f, SPU_RATE);
    put32le(f, SPU_RATE * 4);
    put32le(f, 4 | 16u << 16); /* block align 4, 16 bits */
    fwrite("data", 1, 4, f);
    put32le(f, frames * 4);
}

typedef struct Cmd {
    char kind;
    long a, b, c;
} Cmd;

static void summary(long last_tick) {
    int k, n = 0, off = 0, started = 0, loop_n = 0, loop_ok = 0, loop_end = 0, shot_n = 0, shot_end = 0, shot_ok = 0;
    double worst = 0;

    (void)last_tick;
    for (k = 0; k < nkeyons; k++) {
        KeyOn *e = &keyons[k];
        n++;
        if (e->off_at_once) {
            off++;
            continue;
        }
        started += e->started;
        if (e->looping) {
            loop_n++;
            loop_end += e->first_end != 0;
            loop_ok += e->first_end != 2;
        } else {
            shot_n++;
            if (e->first_end) {
                double ratio = e->first_end_at / e->end_samples;
                shot_end++;
                shot_ok += e->first_end == 2;
                ratio = ratio > 1 ? ratio - 1 : 1 - ratio;
                worst = ratio > worst ? ratio : worst;
            }
        }
    }
    printf("key-ons %d: %d keyed off in the same tick; of the other %d, %d started (ENVX > 0 at the tick's end)\n", n,
           off, n - off, started);
    printf("  looping samples %d: %d reached their loop end, %d never muted\n", loop_n, loop_end, loop_ok);
    printf("  one-shot samples %d: %d reached their end, %d muted there (no loop first); the end came at most %.1f%% "
           "off the time its block count and key-on pitch give\n",
           shot_n, shot_end, shot_ok, worst * 100);
}

int main(int argc, char **argv) {
    FILE *f, *blobf, *wav;
    Cmd *cmds = NULL;
    size_t ncmds = 0, capcmds = 0, i;
    uint8_t *blob, *wavbuf;
    long blob_size, first_tick = -1, last_tick = -1, tick, ft;
    double frames_per_tick = 882;
    char line[256];
    int16_t *buf;
    double sum[2] = {0, 0};
    long peak = 0, clipped = 0, total_clipped = 0, second = 0, frames_in_second = 0, total_frames = 0;
    double cpu = 0;
    int a, v;

    if (argc < 4) {
        fprintf(stderr, "usage: trace_render CMDS BLOB OUT.wav [--frames-per-tick N]\n");
        return 2;
    }
    for (a = 4; a + 1 < argc; a++) {
        if (strcmp(argv[a], "--frames-per-tick") == 0) {
            frames_per_tick = atof(argv[++a]);
        }
    }
    f = fopen(argv[1], "r");
    blobf = fopen(argv[2], "rb");
    wav = fopen(argv[3], "wb");
    if (f == NULL || blobf == NULL || wav == NULL) {
        perror("trace_render");
        return 2;
    }
    fseek(blobf, 0, SEEK_END);
    blob_size = ftell(blobf);
    fseek(blobf, 0, SEEK_SET);
    blob = malloc((size_t)blob_size + 1);
    if (fread(blob, 1, (size_t)blob_size, blobf) != (size_t)blob_size) {
        perror("blob");
        return 2;
    }
    fclose(blobf);
    while (fgets(line, sizeof(line), f) != NULL) {
        Cmd c = {0, 0, 0, 0};
        if (sscanf(line, "tick %ld", &c.a) == 1) {
            c.kind = 't';
            first_tick = first_tick < 0 ? c.a : first_tick;
            last_tick = c.a;
        } else if (sscanf(line, "w %lx %lx", &c.a, &c.b) == 2) {
            c.kind = 'w';
        } else if (sscanf(line, "d %lx %ld %ld", &c.a, &c.b, &c.c) == 3) {
            c.kind = 'd';
        } else {
            continue;
        }
        if (ncmds == capcmds) {
            capcmds = capcmds ? capcmds * 2 : 4096;
            cmds = realloc(cmds, capcmds * sizeof(*cmds));
        }
        cmds[ncmds++] = c;
    }
    fclose(f);

    spu_init();
    spu_set_write_hook(hook);
    for (v = 0; v < SPU_VOICES; v++) {
        active[v] = -1;
    }
    buf = malloc((size_t)frames_per_tick * 4 + 8);
    wavbuf = malloc((size_t)frames_per_tick * 4 + 8);
    wav_header(wav, (uint32_t)((double)(last_tick - first_tick + 1) * frames_per_tick));
    printf("second  rms.l   rms.r   peak  clipped  voices sounding at its end\n");
    i = 0;
    for (tick = first_tick; tick <= last_tick; tick++) {
        int k0 = nkeyons, k;
        long n;

        if (i < ncmds && cmds[i].kind == 't' && cmds[i].a == tick) {
            for (i++; i < ncmds && cmds[i].kind != 't'; i++) {
                Cmd *c = &cmds[i];
                if (c->kind == 'w') {
                    uint32_t off = (uint32_t)c->a;
                    uint16_t val = (uint16_t)c->b;
                    for (v = 0; v < 16; v++) {
                        int voice = v + ((off & 2) ? 16 : 0);
                        if (!(val >> v & 1) || voice >= SPU_VOICES) {
                            continue;
                        }
                        if (off == 0x188 || off == 0x18A) {
                            key_on_event(voice, tick);
                        } else if ((off == 0x18C || off == 0x18E) && active[voice] >= k0) {
                            keyons[active[voice]].off_at_once = 1;
                        }
                    }
                    spu_write16(off, val);
                } else {
                    uint32_t h, halfwords = (uint32_t)(c->b / 2);
                    uint16_t *data = malloc((size_t)halfwords * 2 + 2);
                    for (h = 0; h < halfwords; h++) {
                        data[h] = (uint16_t)(blob[c->c + 2 * h] | blob[c->c + 2 * h + 1] << 8);
                    }
                    expect_dma_addr = (uint32_t)c->a;
                    spu_dma_write(data, halfwords);
                    free(data);
                }
            }
        }
        /* this tick's frames: a fractional rate alternates (877.4: 877 and 878) */
        ft = (long)((double)(tick - first_tick + 1) * frames_per_tick) - (long)((double)(tick - first_tick) * frames_per_tick);
        for (n = 0; n < ft; n++) {
            spu_render(buf + 2 * n, 1);
            watch_voices();
        }
        for (n = 0; n < 2 * ft; n++) {
            wavbuf[2 * n] = (uint8_t)buf[n];
            wavbuf[2 * n + 1] = (uint8_t)((uint16_t)buf[n] >> 8);
        }
        fwrite(wavbuf, 4, (size_t)ft, wav);
        for (k = k0; k < nkeyons; k++) {
            SpuVoiceInfo info;
            spu_voice_info(keyons[k].voice, &info);
            keyons[k].started = info.level > 0;
        }
        for (n = 0; n < 2 * ft; n++) {
            long s = buf[n], m = s < 0 ? -s : s;
            sum[n & 1] += (double)s * (double)s;
            peak = m > peak ? m : peak;
            if (s >= 32767 || s <= -32768) {
                clipped++;
            }
        }
        frames_in_second += ft;
        total_frames += ft;
        if (frames_in_second >= SPU_RATE || tick == last_tick) {
            int sounding = 0;
            for (v = 0; v < SPU_VOICES; v++) {
                SpuVoiceInfo info;
                spu_voice_info(v, &info);
                sounding += info.level > 0 && (info.vol_l != 0 || info.vol_r != 0);
            }
            printf("%6ld  %6.1f  %6.1f  %5ld  %7ld  %d\n", second, sqrt(sum[0] / frames_in_second),
                   sqrt(sum[1] / frames_in_second), peak, clipped, sounding);
            total_clipped += clipped;
            second++;
            frames_in_second = 0;
            sum[0] = sum[1] = 0;
            peak = clipped = 0;
        }
    }
    fclose(wav);
    printf("ticks %ld..%ld (%ld frames, %.2f s), %ld DMA blocks (%ld not where the trace says)\n", first_tick,
           last_tick, total_frames, (double)total_frames / SPU_RATE, dma_blocks, dma_misplaced);
    summary(last_tick);
    printf("clipped samples %ld\n", total_clipped);
    /* The speed: the same replay again, a tick's frames per spu_render call, nothing watched. */
    {
        clock_t t0 = clock();
        spu_init();
        spu_set_write_hook(NULL);
        i = 0;
        for (tick = first_tick; tick <= last_tick; tick++) {
            if (i < ncmds && cmds[i].kind == 't' && cmds[i].a == tick) {
                for (i++; i < ncmds && cmds[i].kind != 't'; i++) {
                    Cmd *c = &cmds[i];
                    if (c->kind == 'w') {
                        spu_write16((uint32_t)c->a, (uint16_t)c->b);
                    } else {
                        uint32_t h, halfwords = (uint32_t)(c->b / 2);
                        uint16_t *data = malloc((size_t)halfwords * 2 + 2);
                        for (h = 0; h < halfwords; h++) {
                            data[h] = (uint16_t)(blob[c->c + 2 * h] | blob[c->c + 2 * h + 1] << 8);
                        }
                        spu_dma_write(data, halfwords);
                        free(data);
                    }
                }
            }
            ft = (long)((double)(tick - first_tick + 1) * frames_per_tick) -
                 (long)((double)(tick - first_tick) * frames_per_tick);
            spu_render(buf, (int)ft);
        }
        cpu = (double)(clock() - t0) / CLOCKS_PER_SEC;
    }
    printf("render time %.3f s for %.2f s of audio: %.0fx real time\n", cpu, (double)total_frames / SPU_RATE,
           cpu > 0 ? (double)total_frames / SPU_RATE / cpu : 0.0);
    free(buf);
    free(wavbuf);
    free(blob);
    free(cmds);
    return dma_misplaced ? 1 : 0;
}
