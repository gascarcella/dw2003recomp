/* port/psyq/libsnd_spu.c: LIBSND's SPU side, the part LIBSPU does on the PS1 (the game never calls LIBSPU itself):
 * the start-up writes, the key and voice-mask registers, the voice registers in LIBSPU's order, the main/CD volumes,
 * the reverb presets and the body transfer by DMA with its completion. Every register write goes through spu.h, so
 * the trace sees what the PS1's LIBSPU would have stored, in the same order (docs/SOUND.md "LIBSND").
 *
 * The DMA's timing: on the PS1 a body transfer runs in the background and its interrupt (which clears the transfer
 * mode in SPUCNT and signals the event SsVabTransCompleted tests) comes later. The oracle shows two cases: a bank
 * body of up to 82 KB transferred by sound_update_loading completes in the same frame, after the first completion
 * poll; COMMON's 297 KB body (sound_init's loop) completes only after the next vsync. The port models a transfer as
 * taking SND_DMA_BYTES_PER_FRAME bytes per frame from the frame's start: shorter ones complete right after the first
 * poll, longer ones at the next vsync (snd_spu_vsync, before the sequencer's flush). */
#include <string.h>

#include "libsnd_internal.h"

/* A DMA longer than this does not complete in the frame it starts (see above; between the oracle's 82 KB and
 * 297 KB). */
#define SND_DMA_BYTES_PER_FRAME 0x30000

/* LIBSPU's reverb presets: the 32 registers dAPF1..vRIN of each type 0..9 (SsUtSetReverbType's argument), and the
 * work area's start / 8. The numbers are LIBSPU's table in the EXE (SLES-03936: 0x8005C840, 10 x 0x44 bytes, a mask
 * word and the 32 halfwords; the start addresses at 0x8005C810); type 3, the one the game uses, is psx-spx's
 * "Studio Medium" word for word (docs/SOUND.md section 6). */
static const u16 snd_reverb_presets[10][32] = {
    { 0 },
    { 0x007D, 0x005B, 0x6D80, 0x54B8, 0xBED0, 0x0000, 0x0000, 0xBA80, 0x5800, 0x5300, 0x04D6, 0x0333, 0x03F0, 0x0227,
      0x0374, 0x01EF, 0x0334, 0x01B5, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x01B4, 0x0136,
      0x00B8, 0x005C, 0x8000, 0x8000 },
    { 0x0033, 0x0025, 0x70F0, 0x4FA8, 0xBCE0, 0x4410, 0xC0F0, 0x9C00, 0x5280, 0x4EC0, 0x03E4, 0x031B, 0x03A4, 0x02AF,
      0x0372, 0x0266, 0x031C, 0x025D, 0x025C, 0x018E, 0x022F, 0x0135, 0x01D2, 0x00B7, 0x018F, 0x00B5, 0x00B4, 0x0080,
      0x004C, 0x0026, 0x8000, 0x8000 },
    { 0x00B1, 0x007F, 0x70F0, 0x4FA8, 0xBCE0, 0x4510, 0xBEF0, 0xB4C0, 0x5280, 0x4EC0, 0x0904, 0x076B, 0x0824, 0x065F,
      0x07A2, 0x0616, 0x076C, 0x05ED, 0x05EC, 0x042E, 0x050F, 0x0305, 0x0462, 0x02B7, 0x042F, 0x0265, 0x0264, 0x01B2,
      0x0100, 0x0080, 0x8000, 0x8000 },
    { 0x00E3, 0x00A9, 0x6F60, 0x4FA8, 0xBCE0, 0x4510, 0xBEF0, 0xA680, 0x5680, 0x52C0, 0x0DFB, 0x0B58, 0x0D09, 0x0A3C,
      0x0BD9, 0x0973, 0x0B59, 0x08DA, 0x08D9, 0x05E9, 0x07EC, 0x04B0, 0x06EF, 0x03D2, 0x05EA, 0x031D, 0x031C, 0x0238,
      0x0154, 0x00AA, 0x8000, 0x8000 },
    { 0x01A5, 0x0139, 0x6000, 0x5000, 0x4C00, 0xB800, 0xBC00, 0xC000, 0x6000, 0x5C00, 0x15BA, 0x11BB, 0x14C2, 0x10BD,
      0x11BC, 0x0DC1, 0x11C0, 0x0DC3, 0x0DC0, 0x09C1, 0x0BC4, 0x07C1, 0x0A00, 0x06CD, 0x09C2, 0x05C1, 0x05C0, 0x041A,
      0x0274, 0x013A, 0x8000, 0x8000 },
    { 0x033D, 0x0231, 0x7E00, 0x5000, 0xB400, 0xB000, 0x4C00, 0xB000, 0x6000, 0x5400, 0x1ED6, 0x1A31, 0x1D14, 0x183B,
      0x1BC2, 0x16B2, 0x1A32, 0x15EF, 0x15EE, 0x1055, 0x1334, 0x0F2D, 0x11F6, 0x0C5D, 0x1056, 0x0AE1, 0x0AE0, 0x07A2,
      0x0464, 0x0232, 0x8000, 0x8000 },
    { 0x0001, 0x0001, 0x7FFF, 0x7FFF, 0x0000, 0x0000, 0x0000, 0x8100, 0x0000, 0x0000, 0x1FFF, 0x0FFF, 0x1005, 0x0005,
      0x0000, 0x0000, 0x1005, 0x0005, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1004, 0x1002,
      0x0004, 0x0002, 0x8000, 0x8000 },
    { 0x0001, 0x0001, 0x7FFF, 0x7FFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1FFF, 0x0FFF, 0x1005, 0x0005,
      0x0000, 0x0000, 0x1005, 0x0005, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1004, 0x1002,
      0x0004, 0x0002, 0x8000, 0x8000 },
    { 0x0017, 0x0013, 0x70F0, 0x4FA8, 0xBCE0, 0x4510, 0xBEF0, 0x8500, 0x5F80, 0x54C0, 0x0371, 0x02AF, 0x02E5, 0x01DF,
      0x02B0, 0x01D7, 0x0358, 0x026A, 0x01D6, 0x011E, 0x012D, 0x00B1, 0x011F, 0x0059, 0x01A0, 0x00E3, 0x0058, 0x0040,
      0x0028, 0x0014, 0x8000, 0x8000 },
};
static const u16 snd_reverb_start[10] = { 0xFFFE, 0xFB28, 0xFC18, 0xF6F8, 0xF204, 0xEA44, 0xE128, 0xCFF8, 0xCFF8,
                                          0xF880 };

static struct {
    u16 xfer_addr8;     /* the transfer address / 8 LIBSPU last set */
    int transfer_done;  /* no transfer in flight (LIBSPU's "not in transfer"; also the VAB open lock) */
    int dma_event;      /* the DMA interrupt has signalled completion, not yet tested */
    int irq_pending;    /* a body transfer's interrupt is still to come: 1 after the next poll, 2 at the next vsync */
    u16 reverb_base;
} sspu;

/* 1 KB of zeros: the source of the reverb work area's clearing. */
static const u16 snd_zero_block[512];
/* A body transfer's copy, in whole 64-byte DMA blocks. */
static u16 snd_dma_buf[(0x7EFF0 + 64) / 2];

static void w(u32 reg, u16 value) {
    spu_write16(reg, value);
}

static u16 r(u32 reg) {
    return spu_read16(reg);
}

void snd_spu_reset(void) {
    memset(&sspu, 0, sizeof(sspu));
}

/* A DMA block transfer to `addr8` * 8, of `bytes` rounded up to 64 bytes (the DMA's 16-word blocks): the transfer
 * address, SPUCNT's transfer mode set to DMA write, the block. The completion (snd_dma_irq) clears the mode again. */
static void snd_dma(const u16 *data, u32 bytes, u16 addr8) {
    u32 blocks = (bytes >> 6) + ((bytes & 0x3F) != 0);

    sspu.xfer_addr8 = addr8;
    w(SND_REG_TSA, addr8);
    w(SND_REG_ATTR, (u16)((r(SND_REG_ATTR) & ~0x30) | 0x20));
    spu_dma_write(data, blocks * 32);
}

static void snd_dma_irq(void) {
    w(SND_REG_ATTR, (u16)(r(SND_REG_ATTR) & ~0x30));
    sspu.dma_event = 1;
}

/* SsInit's hardware set-up (LIBSPU's initialisation, in its order): volumes and SPUCNT off, every voice silenced and
 * keyed off, PMON/NON/EON and the CD/external inputs off, the 16-byte silent block at 0x1000 (written by FIFO: flags 7,
 * a block that loops onto itself) that every idle voice points at, all voices keyed on and off onto it, SPUCNT on,
 * the reverb work area at the top of RAM. */
void snd_spu_init(void) {
    int i;

    w(SND_REG_MVOL_L, 0);
    w(SND_REG_MVOL_R, 0);
    w(SND_REG_ATTR, 0);
    w(SND_REG_MVOL_L, 0);
    w(SND_REG_MVOL_R, 0);
    w(SND_REG_XFER_CTRL, 4);
    w(SND_REG_RVOL_L, 0);
    w(SND_REG_RVOL_R, 0);
    w(SND_REG_KOFF, 0xFFFF);
    w(SND_REG_KOFF + 2, 0xFFFF);
    w(SND_REG_EON, 0);
    w(SND_REG_EON + 2, 0);
    w(SND_REG_PMON, 0);
    w(SND_REG_PMON + 2, 0);
    w(SND_REG_NON, 0);
    w(SND_REG_NON + 2, 0);
    w(SND_REG_CDVOL_L, 0);
    w(SND_REG_CDVOL_R, 0);
    w(SND_REG_EXTVOL_L, 0);
    w(SND_REG_EXTVOL_R, 0);
    sspu.xfer_addr8 = 0x1000 >> 3;
    w(SND_REG_TSA, sspu.xfer_addr8);
    for (i = 0; i < 8; i++) {
        w(SND_REG_FIFO, 0x0707);
    }
    w(SND_REG_ATTR, (u16)((r(SND_REG_ATTR) & ~0x30) | 0x10)); /* manual write: the FIFO goes to RAM */
    w(SND_REG_ATTR, (u16)(r(SND_REG_ATTR) & ~0x30));
    for (i = 0; i < SND_VOICES; i++) {
        w((u32)i * 16 + 0, 0);
        w((u32)i * 16 + 2, 0);
        w((u32)i * 16 + 4, 0x3FFF);
        w((u32)i * 16 + 6, 0x1000 >> 3);
        w((u32)i * 16 + 8, 0);
        w((u32)i * 16 + 10, 0);
    }
    w(SND_REG_KON, 0xFFFF);
    w(SND_REG_KON + 2, 0xFF);
    w(SND_REG_KOFF, 0xFFFF);
    w(SND_REG_KOFF + 2, 0xFF);
    w(SND_REG_ATTR, 0xC000);
    sspu.transfer_done = 1;
    sspu.dma_event = 0;
    sspu.irq_pending = 0;
    sspu.reverb_base = snd_reverb_start[0];
    w(SND_REG_ESA, sspu.reverb_base);
}

/* The work area of reverb type `type` zeroed by DMA in 1 KB blocks, LIBSPU waiting for each one. */
static void snd_spu_clear_area(int type) {
    u32 addr, left, chunk;
    int last = 0;

    if (type == 0) {
        addr = 0xFFF0 << 3;
        left = 0x10 << 3;
    } else {
        addr = (u32)snd_reverb_start[type] << 3;
        left = (0x10000 - (u32)snd_reverb_start[type]) << 3;
    }
    do {
        chunk = 0x400;
        if (left <= 0x400) {
            chunk = left;
            last = 1;
        }
        snd_dma(snd_zero_block, chunk, (u16)(addr >> 3));
        snd_dma_irq();
        sspu.dma_event = 0; /* LIBSPU waits for it */
        left -= 0x400;
        addr += 0x400;
    } while (!last);
}

void snd_spu_clear_reverb_area(void) {
    snd_spu_clear_area(7); /* the largest area (types 7 and 8: 0x18040 bytes) */
}

void snd_spu_set_key(int on, u32 voices) {
    u32 reg = on ? SND_REG_KON : SND_REG_KOFF;

    w(reg, (u16)(voices & 0xFFFF));
    w(reg + 2, (u16)((voices >> 16) & 0xFF));
}

void snd_spu_set_voice_mask(int reg, u32 bits, u32 keep) {
    u32 value = bits | (((u32)r((u32)reg) | ((u32)(r((u32)reg + 2) & 0xFF) << 16)) & keep);

    w((u32)reg, (u16)(value & 0xFFFF));
    w((u32)reg + 2, (u16)((value >> 16) & 0xFF));
}

u16 snd_spu_envelope(int voice) {
    return r((u32)voice * 16 + 0x0C);
}

void snd_spu_voice_attr(int voice, int what, u16 vol_l, u16 vol_r, u16 pitch, u16 addr8, u16 adsr1, u16 adsr2) {
    u32 base = (u32)voice * 16;

    if (what & SND_ATTR_PITCH) {
        w(base + 4, pitch);
    }
    if (what & SND_ATTR_VOL) {
        w(base + 0, vol_l & 0x7FFF); /* a fixed volume: bit 15 clear */
        w(base + 2, vol_r & 0x7FFF);
    }
    if (what & SND_ATTR_ADDR) {
        w(base + 6, addr8);
    }
    if (what & SND_ATTR_ADSR) {
        w(base + 8, adsr1);
        w(base + 10, adsr2);
    }
}

void snd_spu_set_main_volume(u16 left, u16 right) {
    w(SND_REG_MVOL_L, left & 0x7FFF);
    w(SND_REG_MVOL_R, right & 0x7FFF);
}

void snd_spu_set_cd_volume(u16 left, u16 right) {
    w(SND_REG_CDVOL_L, left);
    w(SND_REG_CDVOL_R, right);
}

void snd_spu_set_cd_mix(int on) {
    u16 attr = r(SND_REG_ATTR);

    w(SND_REG_ATTR, (u16)(on ? (attr | 1) : (attr & ~1)));
}

/* A reverb type: the output silenced (SPUCNT's reverb bit cleared while the registers change, and the depth 0), the
 * 32 registers, the work area's start; with bit 8 of `type` (SsUtSetReverbType's negative types) the area is also
 * cleared first. */
int snd_spu_set_reverb_type(int type) {
    int clear = (type & 0x100) != 0;
    int was_on;
    int i;

    type &= ~0x100;
    if (type < 0 || type >= 10) {
        return -1;
    }
    sspu.reverb_base = snd_reverb_start[type];
    was_on = (r(SND_REG_ATTR) >> 7) & 1;
    if (was_on) {
        w(SND_REG_ATTR, (u16)(r(SND_REG_ATTR) & ~0x80));
    }
    w(SND_REG_RVOL_L, 0);
    w(SND_REG_RVOL_R, 0);
    for (i = 0; i < 32; i++) {
        w(SND_REG_REVERB + 2 * (u32)i, snd_reverb_presets[type][i]);
    }
    if (clear) {
        snd_spu_clear_area(type);
    }
    w(SND_REG_ESA, sspu.reverb_base);
    if (was_on) {
        w(SND_REG_ATTR, (u16)(r(SND_REG_ATTR) | 0x80));
    }
    return 0;
}

void snd_spu_set_reverb_depth(s16 left, s16 right) {
    w(SND_REG_RVOL_L, (u16)left);
    w(SND_REG_RVOL_R, (u16)right);
}

/* On: SPUCNT's reverb bit (the work area is never one LIBSPU's allocator handed out: the game allocates none). Off:
 * the bit cleared and the depth set to 0. */
void snd_spu_reverb_enable(int on) {
    if (on) {
        w(SND_REG_ATTR, (u16)(r(SND_REG_ATTR) | 0x80));
    } else {
        w(SND_REG_ATTR, (u16)(r(SND_REG_ATTR) & ~0x80));
        w(SND_REG_RVOL_L, 0);
        w(SND_REG_RVOL_R, 0);
    }
}

/* SsVabTransBody's transfer: `bytes` (at most 0x7EFF0) from `data` to SPU address `addr` (rounded up to 8). */
void snd_spu_transfer(const u8 *data, u32 bytes, u32 addr) {
    u32 padded;

    if (bytes > 0x7EFF0) {
        bytes = 0x7EFF0;
    }
    padded = (bytes + 63) & ~63u;
    /* The DMA sends whole 64-byte blocks: on the PS1 the bytes after the body are what follows it in RAM, which for
     * 70 of the 71 banks is the body file's zero padding; bank 64's last block reaches 4 bytes past its file (the PS1
     * sends 4 bytes of whatever follows the file in RAM). The port sends zeros there: the same for 70 banks, and
     * no read past the file (a host heap's bytes would differ between builds). */
    memcpy(snd_dma_buf, data, bytes);
    memset((u8 *)snd_dma_buf + bytes, 0, padded - bytes);
    if (addr % 8 != 0) {
        addr = (addr + 8) & ~7u;
    }
    snd_dma(snd_dma_buf, bytes, (u16)(addr >> 3));
    sspu.transfer_done = 0;
    sspu.dma_event = 0;
    sspu.irq_pending = padded > SND_DMA_BYTES_PER_FRAME ? 2 : 1;
}

/* SsVabTransCompleted: 1 when no transfer is in flight, or when the DMA's event has come (which ends the transfer);
 * `wait` waits for it. */
int snd_spu_transfer_done(int wait) {
    int ev;

    if (sspu.transfer_done) {
        return 1;
    }
    if (wait && sspu.irq_pending) {
        sspu.irq_pending = 0;
        snd_dma_irq();
    }
    ev = sspu.dma_event;
    sspu.dma_event = 0;
    if (ev) {
        sspu.transfer_done = 1;
    }
    if (sspu.irq_pending == 1) {
        sspu.irq_pending = 0;
        snd_dma_irq(); /* the rest of the transfer runs while the game goes on */
    }
    return ev;
}

void snd_spu_vsync(void) {
    if (sspu.irq_pending != 0) {
        sspu.irq_pending = 0;
        snd_dma_irq();
    }
}

int snd_spu_in_transfer(void) {
    return !sspu.transfer_done;
}

void snd_spu_set_in_transfer(int on) {
    sspu.transfer_done = !on;
}
