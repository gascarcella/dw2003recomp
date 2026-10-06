/* The SPU core (port/include/spu.h), our own, from psx-spx "Sound Processing Unit (SPU)": the register file, 512 KB
 * of SPU RAM written by DMA and the data FIFO, 24 voices (ADPCM, pitch counter, 4-point interpolation, ADSR, volume
 * sweeps, noise, pitch modulation), the stereo mix, the reverb at 22,050 Hz over its work area, the CD input and the
 * capture buffers. The DSP pieces are in spu_dsp.c (spu_internal.h).
 *
 * Time: spu_render advances the SPU by whole 44,100 Hz samples and nothing else does; a register write takes effect at
 * once, between two samples (the hardware applies it at its next 44,100 Hz cycle, which is the same place). Several
 * writes between two renders therefore act in their order, all at the same sample: a key-off then a key-on of one voice
 * (LIBSND's per-tick flush writes them in that order) restarts it; a key-on uses the start address, pitch and ADSR
 * written before it. Every integer path is fixed-width and deterministic. docs/SOUND.md section 6 lists what psx-spx
 * leaves open and the reading taken here. */
#include <string.h>

#include "spu_internal.h"

/* Register offsets from 0x1F801C00 (psx-spx names in comments). */
enum {
    REG_MVOL_L = 0x180, REG_MVOL_R = 0x182, /* MVOLL/R */
    REG_EVOL_L = 0x184, REG_EVOL_R = 0x186, /* the reverb's output volume */
    REG_KON = 0x188, REG_KOFF = 0x18C, REG_PMON = 0x190, REG_NON = 0x194, REG_EON = 0x198, REG_ENDX = 0x19C,
    REG_ESA = 0x1A2,  /* reverb work area start / 8 */
    REG_IRQA = 0x1A4, /* stored only: no SPU interrupt is modelled */
    REG_TSA = 0x1A6, REG_FIFO = 0x1A8, REG_ATTR = 0x1AA, REG_RAMCTRL = 0x1AC, REG_STATX = 0x1AE,
    REG_CDVOL_L = 0x1B0, REG_CDVOL_R = 0x1B2, REG_EXTVOL_L = 0x1B4, REG_EXTVOL_R = 0x1B6,
    REG_MVOLX_L = 0x1B8, REG_MVOLX_R = 0x1BA, /* the current main volume (read-only here) */
    REG_REVERB = 0x1C0,                       /* 32 registers, dAPF1 .. vRIN */
};
/* The reverb registers, by index from 0x1C0. */
enum {
    RV_DAPF1, RV_DAPF2, RV_VIIR, RV_VCOMB1, RV_VCOMB2, RV_VCOMB3, RV_VCOMB4, RV_VWALL,
    RV_VAPF1, RV_VAPF2, RV_MLSAME, RV_MRSAME, RV_MLCOMB1, RV_MRCOMB1, RV_MLCOMB2, RV_MRCOMB2,
    RV_DLSAME, RV_DRSAME, RV_MLDIFF, RV_MRDIFF, RV_MLCOMB3, RV_MRCOMB3, RV_MLCOMB4, RV_MRCOMB4,
    RV_DLDIFF, RV_DRDIFF, RV_MLAPF1, RV_MRAPF1, RV_MLAPF2, RV_MRAPF2, RV_VLIN, RV_VRIN,
};
/* ATTR (SPUCNT) bits. */
enum {
    ATTR_CD_ENABLE = 0x0001, ATTR_CD_REVERB = 0x0004, ATTR_MODE = 0x0030, ATTR_REVERB = 0x0080,
    ATTR_UNMUTE = 0x4000, ATTR_ENABLE = 0x8000,
};
enum { MODE_STOP, MODE_MANUAL_WRITE, MODE_DMA_WRITE, MODE_DMA_READ };

#define SPU_FIFO_SIZE 32
#define SPU_CD_QUEUE 16384 /* frames of CD audio waiting to be mixed */
#define SPU_BLOCK_END (SPU_BLOCK_SAMPLES << 12)

typedef struct SpuVoice {
    SpuEnvelope env; /* the ADSR level (ENVX) */
    int phase;
    SpuEnvelope vol[2]; /* the current volumes, as levels the sweep moves */
    uint32_t addr;      /* the current block (bytes) */
    uint32_t repeat;    /* the repeat address (bytes) */
    uint32_t counter;   /* the pitch counter */
    uint8_t flags;      /* the current block's flag byte */
    int32_t hist[2];    /* the ADPCM decoder's last two samples */
    int16_t buf[3 + SPU_BLOCK_SAMPLES]; /* the previous block's last three samples, then the current block's */
    int32_t out;                        /* the last sample after the envelope */
    uint32_t keyons, loops, mutes;
} SpuVoice;

static struct {
    uint8_t ram[SPU_RAM_SIZE];
    uint16_t reg[0x100]; /* every register as last written, by offset / 2 */
    SpuVoice voice[SPU_VOICES];
    SpuEnvelope mvol[2];
    uint32_t endx;
    uint32_t xfer_addr; /* the transfer's current address (bytes; TSA * 8 when written) */
    uint16_t fifo[SPU_FIFO_SIZE];
    int fifo_n;
    uint16_t noise_level;
    int32_t noise_timer;
    uint32_t rev_addr;                 /* the reverb's current buffer address (bytes) */
    /* The reverb's input at 44,100 Hz and its output zero-stuffed to 44,100 Hz: the last 39 samples of each, stored
     * twice (at fir_pos and fir_pos + 39) so that a filter reads them in one run from fir_pos + 1. */
    int16_t rev_in[2][2 * SPU_FIR_TAPS];
    int16_t rev_out[2][2 * SPU_FIR_TAPS];
    int fir_pos;
    uint32_t capture_pos; /* the capture buffers' sample index, 0..511 */
    int16_t cd[SPU_CD_QUEUE][2];
    uint32_t cd_head, cd_count;
    uint64_t samples;
} spu;

static SpuWriteHook spu_hook;

static uint16_t spu_ram16(uint32_t addr) {
    addr &= SPU_RAM_SIZE - 2;
    return (uint16_t)(spu.ram[addr] | (spu.ram[addr + 1] << 8));
}

static void spu_ram_write16(uint32_t addr, uint16_t value) {
    addr &= SPU_RAM_SIZE - 2;
    spu.ram[addr] = (uint8_t)value;
    spu.ram[addr + 1] = (uint8_t)(value >> 8);
}

static uint16_t spu_reg(uint32_t offset) {
    return spu.reg[offset >> 1];
}

void spu_set_write_hook(SpuWriteHook hook) {
    spu_hook = hook;
}

/* Everything zero, every voice idle: in release at level 0 (psx-spx gives no power-on state). */
void spu_reset(void) {
    int v;

    memset(&spu, 0, sizeof(spu));
    for (v = 0; v < SPU_VOICES; v++) {
        spu.voice[v].phase = SPU_RELEASE;
    }
}

void spu_init(void) {
    spu_reset();
}

/* ---- Voices */

/* Reads the 16-byte block at the voice's address (wrapping in SPU RAM) and decodes it after the previous one. A loop
 * start flag (bit 2) copies the block's address to the repeat address (psx-spx "Flag Bits"). */
static void spu_voice_fetch(SpuVoice *vo) {
    uint8_t block[16];
    int i;

    for (i = 0; i < 16; i++) {
        block[i] = spu.ram[(vo->addr + (uint32_t)i) & (SPU_RAM_SIZE - 1)];
    }
    memcpy(vo->buf, vo->buf + SPU_BLOCK_SAMPLES, 3 * sizeof(vo->buf[0]));
    vo->flags = block[1];
    if (vo->flags & 4) {
        vo->repeat = vo->addr;
    }
    spu_adpcm_decode(block, vo->hist, vo->buf + 3);
}

/* Key on (psx-spx "Voice Flags", "SSA"): the start address becomes the current address, the envelope starts its attack
 * from 0, ENDX clears. The pitch counter, the decoder's and the interpolation's history start at 0 (psx-spx does not
 * say; the samples' first block is silent on this disc anyway). The repeat address is not touched. */
static void spu_key_on(int v) {
    SpuVoice *vo = &spu.voice[v];

    vo->addr = (uint32_t)spu_reg((uint32_t)v * 16 + 6) * 8 & (SPU_RAM_SIZE - 1);
    vo->counter = 0;
    vo->hist[0] = vo->hist[1] = 0;
    memset(vo->buf, 0, sizeof(vo->buf));
    spu_voice_fetch(vo);
    vo->env.level = 0;
    vo->env.counter = 0;
    vo->phase = SPU_ATTACK;
    spu.endx &= ~(1u << v);
    vo->keyons++;
}

static void spu_key_off(int v) {
    spu.voice[v].phase = SPU_RELEASE;
    spu.voice[v].env.counter = 0;
}

/* The end of a block (psx-spx "Flag Bits"): loop end (bit 0) sets ENDX and jumps to the repeat address; without loop
 * repeat (bit 1) the voice is also released with its envelope at 0 (code 1, "End+Mute"). Else the next block. */
static void spu_voice_block_end(int v) {
    SpuVoice *vo = &spu.voice[v];

    if (vo->flags & 1) {
        spu.endx |= 1u << v;
        vo->addr = vo->repeat;
        if (vo->flags & 2) {
            vo->loops++;
        } else {
            vo->env.level = 0;
            vo->env.counter = 0;
            vo->phase = SPU_RELEASE;
            vo->mutes++;
        }
    } else {
        vo->addr = (vo->addr + 16) & (SPU_RAM_SIZE - 1);
    }
    spu_voice_fetch(vo);
}

/* One ADSR tick (psx-spx "ADSR1", "ADSR2"): attack (increase, its mode, shift and step) until 7FFFh, decay
 * (exponential decrease, step -8) until the sustain level (N+1)*800h, sustain (its mode, direction, shift, step) until
 * key off, release (decrease, its mode and shift, step -8) to 0. A phase's end is checked after its step; the next
 * phase starts on the next tick with its counter at 0. */
static void spu_adsr_tick(SpuVoice *vo, uint16_t adsr1, uint16_t adsr2) {
    switch (vo->phase) {
    case SPU_ATTACK:
        spu_envelope_tick(&vo->env, (adsr1 >> 8) & 0x7F, adsr1 >> 15, 0, 0);
        if (vo->env.level >= 0x7FFF) {
            vo->phase = SPU_DECAY;
            vo->env.counter = 0;
        }
        break;
    case SPU_DECAY:
        spu_envelope_tick(&vo->env, ((adsr1 >> 4) & 0x0F) << 2, 1, 1, 0);
        if (vo->env.level <= (int32_t)((adsr1 & 0x0F) + 1) * 0x800) {
            vo->phase = SPU_SUSTAIN;
            vo->env.counter = 0;
        }
        break;
    case SPU_SUSTAIN:
        spu_envelope_tick(&vo->env, (adsr2 >> 6) & 0x7F, adsr2 >> 15, (adsr2 >> 14) & 1, 0);
        break;
    default:
        spu_envelope_tick(&vo->env, (adsr2 & 0x1F) << 2, (adsr2 >> 5) & 1, 1, 0);
        break;
    }
}

/* A volume register (voice L/R, main L/R; psx-spx "Volume"): bit 15 clear sets the volume (value * 2) when written;
 * bit 15 set sweeps it from where it is, one envelope tick per sample (mode bit 14, direction bit 13, phase bit 12,
 * shift bits 6-2, step bits 1-0). */
static void spu_volume_write(SpuEnvelope *vol, uint16_t value) {
    if (!(value & 0x8000)) {
        vol->level = (int16_t)(uint16_t)(value << 1);
        vol->counter = 0;
    }
}

static void spu_volume_tick(SpuEnvelope *vol, uint16_t value) {
    if (value & 0x8000) {
        spu_envelope_tick(vol, value & 0x7F, (value >> 14) & 1, (value >> 13) & 1, (value >> 12) & 1);
    }
}

/* psx-spx "SPU Noise Generator", one 44,100 Hz tick: the shift and step in ATTR bits 13-10 and 9-8. */
static void spu_noise_tick(void) {
    uint16_t attr = spu_reg(REG_ATTR);
    int32_t reload = 0x20000 >> ((attr >> 10) & 0x0F);
    unsigned l = spu.noise_level;
    unsigned parity = ((l >> 15) ^ (l >> 12) ^ (l >> 11) ^ (l >> 10) ^ 1) & 1;

    spu.noise_timer -= (int32_t)((attr >> 8) & 3) + 4;
    if (spu.noise_timer < 0) {
        spu.noise_level = (uint16_t)(l * 2 + parity);
        spu.noise_timer += reload;
        if (spu.noise_timer < 0) {
            spu.noise_timer += reload;
        }
    }
}

/* One voice, one sample: the interpolated ADPCM sample (or the noise) times the envelope, then the pitch counter's step
 * (psx-spx "Pitch Counter", with PMON from the previous voice's output) and the block ends it crosses. Returns the
 * sample after the envelope; *l and *r get it after the voice's volumes. `pmon` and `non` are the PMON and NON bits. */
static int32_t spu_voice_sample(int v, uint32_t pmon, uint32_t non, int32_t *l, int32_t *r) {
    SpuVoice *vo = &spu.voice[v];
    const uint16_t *reg = &spu.reg[v * 8];
    uint32_t bit = 1u << v;
    int32_t s, out = 0;
    uint32_t step;

    if (vo->env.level != 0) { /* (at level 0 the product is 0 whatever the sample) */
        if (non & bit) {
            s = (int16_t)spu.noise_level;
        } else {
            s = spu_gauss_interp(vo->buf + (vo->counter >> 12), (vo->counter >> 4) & 0xFF);
        }
        out = spu_clamp16((s * vo->env.level) >> 15);
    }
    vo->out = out;
    *l = (out * vo->vol[0].level) >> 15;
    *r = (out * vo->vol[1].level) >> 15;

    spu_adsr_tick(vo, reg[4], reg[5]);
    spu_volume_tick(&vo->vol[0], reg[0]);
    spu_volume_tick(&vo->vol[1], reg[1]);

    step = reg[2];
    if (v > 0 && (pmon & bit)) {
        int32_t factor = spu.voice[v - 1].out + 0x8000;
        step = (uint32_t)(((int32_t)(int16_t)step * factor) >> 15) & 0xFFFF;
    }
    if (step > 0x3FFF) {
        step = 0x4000;
    }
    vo->counter += step;
    while (vo->counter >= SPU_BLOCK_END) {
        vo->counter -= SPU_BLOCK_END;
        spu_voice_block_end(v);
    }
    return out;
}

/* ---- Reverb (psx-spx "SPU Reverb Formula") */

uint32_t spu_reverb_current(void) {
    return spu.rev_addr;
}

/* A work-area address: `offset` bytes from the current buffer address, wrapped into ESA..7FFFEh. */
uint32_t spu_reverb_address(int32_t offset) {
    uint32_t esa = (uint32_t)spu_reg(REG_ESA) * 8;
    int32_t size = (int32_t)(SPU_RAM_SIZE - esa);
    int32_t rel = (int32_t)(spu.rev_addr - esa) + offset;

    if (rel < 0 || rel >= size) { /* (the usual offsets are inside the area: no division) */
        rel %= size;
        if (rel < 0) {
            rel += size;
        }
    }
    return (esa + (uint32_t)rel) & (SPU_RAM_SIZE - 2);
}

/* The reverb registers: an address (register * 8 bytes) and a volume (signed). */
static int32_t rv_addr(int i) {
    return (int32_t)spu.reg[(REG_REVERB >> 1) + i] * 8;
}

static int32_t rv_vol(int i) {
    return (int16_t)spu.reg[(REG_REVERB >> 1) + i];
}

static int32_t rv_read(int32_t offset) {
    return (int16_t)spu_ram16(spu_reverb_address(offset));
}

static void rv_write(int32_t offset, int32_t value, int enabled) {
    if (enabled) {
        spu_ram_write16(spu_reverb_address(offset), (uint16_t)(int16_t)value);
    }
}

static int32_t rv_mul(int32_t a, int32_t b) {
    return (a * b) >> 15;
}

/* A reflection: [m] = (in + [d] * vWALL - [m-2]) * vIIR + [m-2], saturated at each step. */
static int32_t rv_reflect(int32_t in, int32_t d, int32_t m_prev) {
    int32_t t = spu_clamp16(in + rv_mul(rv_read(d), rv_vol(RV_VWALL)));

    t = spu_clamp16(t - m_prev);
    return spu_clamp16(rv_mul(t, rv_vol(RV_VIIR)) + m_prev);
}

/* One 22,050 Hz reverb tick on the resampled input; returns the output (after EVOL) in out[2]. Left and right are
 * computed together (the hardware alternates them on the two 44,100 Hz cycles). Reads and writes follow psx-spx's
 * "Reverb Computation Order" (the SAME write before the DIFF and COMB reads, the APF reads before the APF writes);
 * with ATTR bit 7 clear nothing is written, everything is still read. */
static void spu_reverb_tick(const int32_t in[2], int32_t out[2]) {
    int enabled = (spu_reg(REG_ATTR) & ATTR_REVERB) != 0;
    int32_t dapf1 = rv_addr(RV_DAPF1), dapf2 = rv_addr(RV_DAPF2);
    int ch;

    for (ch = 0; ch < 2; ch++) {
        int32_t lin = rv_mul(in[ch], rv_vol(RV_VLIN + ch));
        int32_t msame = rv_addr(RV_MLSAME + ch), mdiff = rv_addr(RV_MLDIFF + ch);
        int32_t mapf1 = rv_addr(RV_MLAPF1 + ch), mapf2 = rv_addr(RV_MLAPF2 + ch);
        int32_t same, diff, acc, apf1, apf2;

        same = rv_reflect(lin, rv_addr(RV_DLSAME + ch), rv_read(msame - 2));
        rv_write(msame, same, enabled);
        /* the other side's d: dRDIFF for the left, dLDIFF for the right */
        diff = rv_reflect(lin, rv_addr(RV_DRDIFF - ch), rv_read(mdiff - 2));
        acc = rv_mul(rv_vol(RV_VCOMB1), rv_read(rv_addr(RV_MLCOMB1 + ch)));
        rv_write(mdiff, diff, enabled);
        acc += rv_mul(rv_vol(RV_VCOMB2), rv_read(rv_addr(RV_MLCOMB2 + ch)));
        acc += rv_mul(rv_vol(RV_VCOMB3), rv_read(rv_addr(RV_MLCOMB3 + ch)));
        acc += rv_mul(rv_vol(RV_VCOMB4), rv_read(rv_addr(RV_MLCOMB4 + ch)));
        acc = spu_clamp16(acc);
        apf1 = rv_read(mapf1 - dapf1);
        apf2 = rv_read(mapf2 - dapf2);
        acc = spu_clamp16(acc - rv_mul(rv_vol(RV_VAPF1), apf1));
        rv_write(mapf1, acc, enabled);
        acc = spu_clamp16(rv_mul(acc, rv_vol(RV_VAPF1)) + apf1);
        acc = spu_clamp16(acc - rv_mul(rv_vol(RV_VAPF2), apf2));
        rv_write(mapf2, acc, enabled);
        acc = spu_clamp16(rv_mul(acc, rv_vol(RV_VAPF2)) + apf2);
        out[ch] = spu_clamp16(rv_mul(acc, (int16_t)spu_reg(REG_EVOL_L + 2 * ch)));
    }
    {
        uint32_t esa = (uint32_t)spu_reg(REG_ESA) * 8;
        uint32_t next = (spu.rev_addr + 2) & (SPU_RAM_SIZE - 2);
        spu.rev_addr = next < esa ? esa : next;
    }
}

/* The reverb's resampling (psx-spx "Reverb Buffer Resampling"): the input goes through the 39-tap filter and every other
 * sample feeds a reverb tick; the ticks' outputs, zero-stuffed back to 44,100 Hz, go through the same filter at twice
 * the gain (the zeros halve it). Together 19 + 19 = 38 samples of delay, as psx-spx measured. */
static void spu_reverb_sample(const int32_t in[2], int32_t out[2]) {
    int tick = (spu.samples & 1) == 0;
    int ch, k;

    int pos = spu.fir_pos = (spu.fir_pos + 1) % SPU_FIR_TAPS;

    for (ch = 0; ch < 2; ch++) {
        spu.rev_in[ch][pos] = spu.rev_in[ch][pos + SPU_FIR_TAPS] = (int16_t)spu_clamp16(in[ch]);
    }
    if (tick) {
        int32_t down[2], wet[2];
        for (ch = 0; ch < 2; ch++) {
            const int16_t *x = &spu.rev_in[ch][pos + 1];
            int32_t acc = 0;
            for (k = 0; k < SPU_FIR_TAPS; k++) {
                acc += spu_fir[k] * x[k];
            }
            down[ch] = spu_clamp16(acc >> 15);
        }
        spu_reverb_tick(down, wet);
        for (ch = 0; ch < 2; ch++) {
            spu.rev_out[ch][pos] = spu.rev_out[ch][pos + SPU_FIR_TAPS] = (int16_t)wet[ch];
        }
    } else {
        for (ch = 0; ch < 2; ch++) {
            spu.rev_out[ch][pos] = spu.rev_out[ch][pos + SPU_FIR_TAPS] = 0;
        }
    }
    for (ch = 0; ch < 2; ch++) {
        const int16_t *x = &spu.rev_out[ch][pos + 1];
        int32_t acc = 0;
        for (k = 0; k < SPU_FIR_TAPS; k++) {
            acc += spu_fir[k] * x[k];
        }
        out[ch] = spu_clamp16(acc >> 14);
    }
}

/* ---- The mix */

/* One output sample: noise, the 24 voices, the CD input, the capture buffers, the reverb, the main volume. The voices'
 * sum is saturated, the reverb's output and the CD input (by CD volume, with ATTR bit 0) added and saturated, then the
 * main volume applied and saturated. ATTR bits 15 (enable) and 14 (unmute) clear silence the voices and the reverb but
 * not the CD input ("Don't care for CD Audio"). */
static void spu_sample(int16_t *out) {
    uint16_t attr = spu_reg(REG_ATTR);
    uint32_t eon = (uint32_t)spu_reg(REG_EON) | (uint32_t)spu_reg(REG_EON + 2) << 16;
    uint32_t pmon = (uint32_t)spu_reg(REG_PMON) | (uint32_t)spu_reg(REG_PMON + 2) << 16;
    uint32_t non = (uint32_t)spu_reg(REG_NON) | (uint32_t)spu_reg(REG_NON + 2) << 16;
    int32_t dry[2] = {0, 0}, wet_in[2] = {0, 0}, wet[2], cd[2] = {0, 0}, mix[2];
    uint32_t cap = spu.capture_pos * 2;
    int v, ch;

    spu_noise_tick();
    for (v = 0; v < SPU_VOICES; v++) {
        int32_t l, r, o = spu_voice_sample(v, pmon, non, &l, &r);

        dry[0] += l;
        dry[1] += r;
        if (eon & (1u << v)) {
            wet_in[0] += l;
            wet_in[1] += r;
        }
        if (v == 1 || v == 3) {
            spu_ram_write16((v == 1 ? 0x800 : 0xC00) + cap, (uint16_t)(int16_t)o);
        }
    }
    if (spu.cd_count > 0) {
        cd[0] = spu.cd[spu.cd_head][0];
        cd[1] = spu.cd[spu.cd_head][1];
        spu.cd_head = (spu.cd_head + 1) % SPU_CD_QUEUE;
        spu.cd_count--;
    }
    spu_ram_write16(0x000 + cap, (uint16_t)(int16_t)cd[0]);
    spu_ram_write16(0x400 + cap, (uint16_t)(int16_t)cd[1]);
    spu.capture_pos = (spu.capture_pos + 1) & 0x1FF;
    for (ch = 0; ch < 2; ch++) {
        cd[ch] = (cd[ch] * (int16_t)spu_reg(REG_CDVOL_L + 2 * ch)) >> 15;
        if (attr & ATTR_CD_REVERB) {
            wet_in[ch] += cd[ch];
        }
        if (!(attr & ATTR_CD_ENABLE)) {
            cd[ch] = 0;
        }
    }
    spu_reverb_sample(wet_in, wet);
    for (ch = 0; ch < 2; ch++) {
        int32_t s = 0;
        if ((attr & (ATTR_ENABLE | ATTR_UNMUTE)) == (ATTR_ENABLE | ATTR_UNMUTE)) {
            s = spu_clamp16(dry[ch]) + wet[ch];
        }
        s = spu_clamp16(s + cd[ch]);
        mix[ch] = spu_clamp16((s * spu.mvol[ch].level) >> 15);
        spu_volume_tick(&spu.mvol[ch], spu_reg(REG_MVOL_L + 2 * ch));
    }
    if (out != NULL) {
        out[0] = (int16_t)mix[0];
        out[1] = (int16_t)mix[1];
    }
    spu.samples++;
}

void spu_render(int16_t *out, int frames) {
    int i;

    for (i = 0; i < frames; i++) {
        spu_sample(out != NULL ? out + 2 * i : NULL);
    }
}

void spu_cd_input(const int16_t *samples, int frames) {
    int i;

    /* Beyond the queue's capacity the newest frames are dropped (the producer runs ahead of the mix). */
    for (i = 0; i < frames && spu.cd_count < SPU_CD_QUEUE; i++) {
        uint32_t tail = (spu.cd_head + spu.cd_count) % SPU_CD_QUEUE;
        spu.cd[tail][0] = samples[2 * i];
        spu.cd[tail][1] = samples[2 * i + 1];
        spu.cd_count++;
    }
}

/* ---- Transfers (psx-spx "SPU Memory Access") */

static void spu_fifo_flush(void) {
    int i;

    for (i = 0; i < spu.fifo_n; i++) {
        spu_ram_write16(spu.xfer_addr, spu.fifo[i]);
        spu.xfer_addr = (spu.xfer_addr + 2) & (SPU_RAM_SIZE - 1);
    }
    spu.fifo_n = 0;
}

/* DMA4 into SPU RAM at the transfer address, which advances. The transfer mode is not checked (LIBSPU sets DMA write
 * first; the port has no DMA controller to stall). */
void spu_dma_write(const uint16_t *data, uint32_t halfwords) {
    uint32_t i;

    if (spu_hook != NULL) {
        spu_hook(spu.xfer_addr, 0, data, halfwords);
    }
    for (i = 0; i < halfwords; i++) {
        spu_ram_write16(spu.xfer_addr, data[i]);
        spu.xfer_addr = (spu.xfer_addr + 2) & (SPU_RAM_SIZE - 1);
    }
}

/* ---- Registers */

/* STATX: ATTR's bits 5-0 at once (the hardware applies them a little later), bit 7 = ATTR bit 5, bits 8/9 the DMA
 * write/read request in those modes, bit 10 (busy) never (every transfer completes when it is made), bit 11 the
 * capture buffers' half. No interrupt flag. */
static uint16_t spu_statx(void) {
    uint16_t attr = spu_reg(REG_ATTR);
    unsigned mode = (attr & ATTR_MODE) >> 4;
    uint16_t s = attr & 0x3F;

    s |= (uint16_t)((attr & 0x20) << 2);
    s |= mode == MODE_DMA_WRITE ? 0x100 : mode == MODE_DMA_READ ? 0x200 : 0;
    s |= (spu.capture_pos & 0x100) ? 0x800 : 0;
    return s;
}

uint16_t spu_read16(uint32_t offset) {
    offset &= 0x1FE;
    if (offset < 0x180) {
        const SpuVoice *vo = &spu.voice[offset >> 4];
        switch (offset & 0x0F) {
        case 0x0C:
            return (uint16_t)(int16_t)vo->env.level;
        case 0x0E:
            return (uint16_t)(vo->repeat >> 3);
        default:
            return spu_reg(offset);
        }
    }
    switch (offset) {
    case REG_ENDX:
        return (uint16_t)spu.endx;
    case REG_ENDX + 2:
        return (uint16_t)(spu.endx >> 16);
    case REG_STATX:
        return spu_statx();
    case REG_MVOLX_L:
    case REG_MVOLX_R:
        return (uint16_t)(int16_t)spu.mvol[(offset - REG_MVOLX_L) >> 1].level;
    default:
        return spu_reg(offset);
    }
}

void spu_write16(uint32_t offset, uint16_t value) {
    int v;

    if (spu_hook != NULL) {
        spu_hook(offset, value, NULL, 0);
    }
    if (offset > 0x1FF) {
        return;
    }
    offset &= 0x1FE;
    if (offset == REG_ENDX || offset == REG_ENDX + 2 || offset == REG_STATX || offset == REG_MVOLX_L ||
        offset == REG_MVOLX_R) {
        return; /* read-only: the hardware overwrites them */
    }
    spu.reg[offset >> 1] = value;
    if (offset < 0x180) {
        SpuVoice *vo = &spu.voice[offset >> 4];
        switch (offset & 0x0F) {
        case 0x00:
        case 0x02:
            spu_volume_write(&vo->vol[(offset >> 1) & 1], value);
            break;
        case 0x0C:
            vo->env.level = (int16_t)value; /* ENVX: the envelope jumps there */
            break;
        case 0x0E:
            vo->repeat = (uint32_t)value * 8 & (SPU_RAM_SIZE - 1);
            break;
        default:
            break; /* pitch, start address, ADSR: read when used */
        }
        return;
    }
    switch (offset) {
    case REG_MVOL_L:
    case REG_MVOL_R:
        spu_volume_write(&spu.mvol[(offset - REG_MVOL_L) >> 1], value);
        break;
    case REG_KON:
    case REG_KON + 2:
        for (v = 0; v < 16; v++) {
            if ((value >> v) & 1) {
                int voice = v + (offset == REG_KON ? 0 : 16);
                if (voice < SPU_VOICES) {
                    spu_key_on(voice);
                }
            }
        }
        break;
    case REG_KOFF:
    case REG_KOFF + 2:
        for (v = 0; v < 16; v++) {
            if ((value >> v) & 1) {
                int voice = v + (offset == REG_KOFF ? 0 : 16);
                if (voice < SPU_VOICES) {
                    spu_key_off(voice);
                }
            }
        }
        break;
    case REG_ESA:
        spu.rev_addr = (uint32_t)value * 8 & (SPU_RAM_SIZE - 2);
        break;
    case REG_TSA:
        spu.xfer_addr = (uint32_t)value * 8 & (SPU_RAM_SIZE - 1);
        break;
    case REG_FIFO:
        if (spu.fifo_n < SPU_FIFO_SIZE) {
            spu.fifo[spu.fifo_n++] = value;
        }
        if (((spu_reg(REG_ATTR) & ATTR_MODE) >> 4) == MODE_MANUAL_WRITE) {
            spu_fifo_flush();
        }
        break;
    case REG_ATTR:
        /* Manual write (psx-spx "SPU RAM Manual Write"): the FIFO goes to RAM when the mode is set, and at once while
         * it stays set. */
        if (((value & ATTR_MODE) >> 4) == MODE_MANUAL_WRITE) {
            spu_fifo_flush();
        }
        break;
    default:
        break; /* stored only: volumes, PMON/NON/EON, IRQA, RAM_CTRL, CD/external volume, reverb */
    }
}

/* ---- The internals' view (spu_internal.h) */

void spu_voice_info(int voice, SpuVoiceInfo *info) {
    const SpuVoice *vo = &spu.voice[voice];

    info->addr = vo->addr;
    info->repeat = vo->repeat;
    info->counter = vo->counter;
    info->level = vo->env.level;
    info->phase = vo->phase;
    info->out = vo->out;
    info->vol_l = (int16_t)vo->vol[0].level;
    info->vol_r = (int16_t)vo->vol[1].level;
    info->keyons = vo->keyons;
    info->loops = vo->loops;
    info->mutes = vo->mutes;
}

const uint8_t *spu_ram(void) {
    return spu.ram;
}

uint64_t spu_samples(void) {
    return spu.samples;
}
