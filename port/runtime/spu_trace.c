/* The SPU write trace (port_harness.h): `--spu-trace FILE` writes every SPU register store and DMA block the port
 * makes, in tests/sound's text format (tests/sound/spu_trace.py's docstring; docs/SOUND.md section 4), so that
 * `spu_trace.py diff` and tests/port/sound.py compare it with the emulator's:
 *   <tick> <reg> <name> <value>              reg = the address - 0x1F801000 (hex), name as spu_trace.py names it
 *   <tick> dma4 spu=<addr> len=<bytes> sha1=<hex>
 *   # <tick> call <LIBSND call>              the game's LIBSND calls (comments; the oracle's `--calls` lines)
 * The tick is the port's frame (port_frames): a store made while the vsync handler runs (LIBSND's sequencer tick)
 * belongs to the vsync being run, which port_frames counts only after the handler, so it gets port_frames + 1, as the
 * emulator's trace counts it (its tick changes at the vsync, before the handler). */
#include <stdio.h>
#include <string.h>

#include "port_harness.h"
#include "port_runtime.h"
#include "psyq_internal.h"
#include "sha1.h"
#include "spu.h"

static FILE *trace_file;
static long trace_writes, trace_dma, trace_dma_bytes;

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

static long trace_tick(void) {
    return port_frames + psyq_snd_in_vsync();
}

/* spu_trace.py's register names, by offset from 0x1F801C00. */
static void reg_name(uint32_t offset, char *out, size_t size) {
    offset &= 0x1FE;
    if (offset < 0x180) {
        snprintf(out, size, "v%02u.%s", (unsigned)(offset >> 4), voice_regs[(offset & 0xF) >> 1]);
    } else if (offset < 0x1C0) {
        snprintf(out, size, "%s", control_regs[(offset - 0x180) >> 1]);
    } else {
        snprintf(out, size, "rev.%s", reverb_regs[(offset - 0x1C0) >> 1]);
    }
}

static void trace_hook(uint32_t offset, uint16_t value, const uint16_t *dma, uint32_t halfwords) {
    long tick = trace_tick();

    if (dma == NULL) {
        char name[24];
        reg_name(offset, name, sizeof(name));
        fprintf(trace_file, "%ld %03x %s %04x\n", tick, (unsigned)(0xC00 + (offset & 0x1FE)), name, value);
        trace_writes++;
    } else {
        PortSha1 c;
        uint8_t digest[20], le[2];
        char hex[41];
        uint32_t i;

        port_sha1_init(&c);
        for (i = 0; i < halfwords; i++) {
            le[0] = (uint8_t)(dma[i] & 0xFF);
            le[1] = (uint8_t)(dma[i] >> 8);
            port_sha1_update(&c, le, 2);
        }
        port_sha1_final(&c, digest);
        port_sha1_hex(digest, hex);
        fprintf(trace_file, "%ld dma4 spu=%05x len=%u sha1=%s\n", tick, (unsigned)offset, (unsigned)(halfwords * 2), hex);
        trace_dma++;
        trace_dma_bytes += (long)halfwords * 2;
    }
}

static void trace_call(const char *line) {
    fprintf(trace_file, "# %ld call %s\n", trace_tick(), line);
}

void port_spu_trace_open(const char *path) {
    trace_file = fopen(path, "wb"); /* "b": the same bytes on Windows */
    if (trace_file == NULL) {
        port_fatal("--spu-trace %s: cannot write it", path);
    }
    fprintf(trace_file, "# " PSXSTACK_GAME_ID " spu trace v1\n# the PC port (port/), its frame as the tick\n");
    trace_writes = trace_dma = trace_dma_bytes = 0;
    spu_set_write_hook(trace_hook);
    psyq_snd_set_call_hook(trace_call);
}

void port_spu_trace_close(void) {
    if (trace_file == NULL) {
        return;
    }
    spu_set_write_hook(NULL);
    psyq_snd_set_call_hook(NULL);
    fprintf(trace_file, "# ticks %ld, writes %ld, dma blocks %ld (%ld bytes)\n", port_frames, trace_writes, trace_dma,
            trace_dma_bytes);
    fclose(trace_file);
    trace_file = NULL;
}
