/* port/psyq/xa.c: the CD-ROM drive's XA-ADPCM decoder and 37,800 -> 44,100 Hz resampler (xa.h), our own, from psx-spx
 * "CDROM XA Audio ADPCM Compression" and "CDROM XA Subheader, File, Channel, Interleave".
 *
 * A sector's data (Form 2) holds 18 sound groups of 128 bytes: 16 header bytes (the sound units' parameter bytes at
 * 4..11, copies at 0..3 and 12..15), then 28 little-endian data words, each holding the n-th sample of every unit.
 * At 4 bits a group has 8 units of 28 samples (unit u: parameter byte 4+u, nibble u of each word: the low nibble of
 * byte u/2), at 8 bits 4 units (unit u: parameter byte 4+u, byte u of each word). Stereo: the even units are left,
 * the odd ones right; mono: the units in order. A parameter byte is the shift (range, bits 0-3) and the filter (bits
 * 4-5): s = (sample << 12 (4-bit) or << 8 (8-bit)) >> range + (old * pos[f] + older * neg[f] + 32) >> 6, clamped to
 * 16 bits; ranges 13..15 act as 9. Each channel keeps its own old/older across units, groups and sectors.
 *
 * The resampler (psx-spx "25-point Zigzag Interpolation"): every 37,800 Hz sample goes into a 32-entry ring per
 * channel; after every sixth one, seven 44,100 Hz samples come out, output k (1..7) being the sum over i = 1..29 of
 * ring[(p - i) & 31] * TableK[i] >> 15 (p = the position after the newest sample), clamped to 16 bits. An 18,900 Hz
 * sample is put in twice and a mono sample on both sides (psx-spx does not give the hardware's 18,900 Hz path: this
 * is our reading). The six-step counter starts at 6 (the hardware's is uninitialised at power-up).
 *
 * Readings where psx-spx is ambiguous (docs/SOUND.md "CD audio"; tests/xa's Python model takes the same ones):
 *  - "/64" and "/8000h" are arithmetic shifts (the SPU core's reading of the same "/64", spu_dsp.c), the latter per
 *    term, where the pseudo-code places it;
 *  - a reserved coding field (value 2 or 3) is read as its 0 value: mono, 37,800 Hz, 4 bits;
 *  - emphasis (coding bit 6; "isn't used by any known PSX games", its formula unknown) is not applied. */
#include "xa.h"

#include <string.h>

/* psx-spx "Pos/neg Tables" (XA-ADPCM has four filters). */
const int16_t xa_filter_pos[4] = {0, 60, 115, 98};
const int16_t xa_filter_neg[4] = {0, 0, -52, -55};

/* psx-spx's Table1..Table7, each read down its column (index 1..29). */
const int16_t xa_zigzag[XA_PHASES][XA_TAPS] = {
    {0, 0, 0, 0, 0, -2, 10, -34, 65, -84, 52, 9, -266, 1024, -2680, 9036, 26516, -6016, 3021, -1571, 848, -365, 107,
     10, -16, 17, -8, 3, -1},
    {0, 0, 0, -2, 0, 3, -19, 60, -75, 162, -227, 306, -67, -615, 3229, 29883, -4532, 2488, -1471, 882, -424, 166, -27,
     5, 6, -8, 3, -1, 0},
    {0, 0, -1, 3, -2, -5, 31, -74, 179, -402, 689, -926, 1272, -1446, 31033, -1446, 1272, -926, 689, -402, 179, -74,
     31, -5, -2, 3, -1, 0, 0},
    {0, -1, 3, -8, 6, 5, -27, 166, -424, 882, -1471, 2488, -4532, 29883, 3229, -615, -67, 306, -227, 162, -75, 60, -19,
     3, 0, -2, 0, 0, 0},
    {-1, 3, -8, 17, -16, 10, 107, -365, 848, -1571, 3021, -6016, 26516, 9036, -2680, 1024, -266, 9, 52, -84, 65, -34,
     10, -1, 0, 1, 0, 0, 0},
    {2, -8, 16, -35, 43, 26, -235, 635, -1352, 2810, -5882, 21472, 15367, -4681, 2062, -839, 347, -68, -23, 70, -35, 17,
     -5, 0, 0, 0, 0, 0, 0},
    {-5, 17, -35, 70, -23, -68, 347, -839, 2062, -4681, 15367, 21472, -5882, 2810, -1352, 635, -235, 26, 43, -35, 16,
     -8, 2, 0, 0, 0, 0, 0, 0},
};

static int32_t xa_clamp16(int32_t v) {
    return v < -0x8000 ? -0x8000 : v > 0x7FFF ? 0x7FFF : v;
}

void xa_reset(XaDecoder *d) {
    memset(d, 0, sizeof(*d));
    d->sixstep = 6;
}

/* One sound unit: 28 samples of channel `ch` from the group `g`, unit `u`, into dst. */
static void xa_decode_unit(XaDecoder *d, const uint8_t *g, int u, int eight_bit, int ch, int16_t *dst) {
    uint8_t param = g[4 + u];
    int range = param & 0x0F, filter = (param >> 4) & 0x03, j;
    int32_t pos = xa_filter_pos[filter], neg = xa_filter_neg[filter];
    int32_t old = d->old[ch], older = d->older[ch];

    if (range > 12) {
        range = 9;
    }
    for (j = 0; j < XA_UNIT_SAMPLES; j++) {
        const uint8_t *word = g + 16 + 4 * j;
        int32_t t, s;

        if (eight_bit) {
            t = (int8_t)word[u] * 256;
        } else {
            t = ((int8_t)(word[u >> 1] << ((u & 1) ? 0 : 4)) >> 4) * 4096; /* the nibble, sign-extended */
        }
        s = xa_clamp16((t >> range) + ((old * pos + older * neg + 32) >> 6));
        dst[j] = (int16_t)s;
        older = old;
        old = s;
    }
    d->old[ch] = old;
    d->older[ch] = older;
}

int xa_decode_group(XaDecoder *d, uint8_t ci, const uint8_t *g, int16_t *left, int16_t *right) {
    int stereo = XA_CI_STEREO(ci), eight_bit = XA_CI_8BIT(ci);
    int units = eight_bit ? 4 : 8, u;

    for (u = 0; u < units; u++) {
        if (stereo) {
            xa_decode_unit(d, g, u, eight_bit, u & 1, ((u & 1) ? right : left) + (u >> 1) * XA_UNIT_SAMPLES);
        } else {
            xa_decode_unit(d, g, u, eight_bit, 0, left + u * XA_UNIT_SAMPLES);
        }
    }
    return (stereo ? units / 2 : units) * XA_UNIT_SAMPLES;
}

int xa_decode_groups(XaDecoder *d, uint8_t ci, const uint8_t *groups, int16_t *left, int16_t *right) {
    int n = 0, i;

    for (i = 0; i < XA_GROUPS; i++) {
        n += xa_decode_group(d, ci, groups + i * XA_GROUP_BYTES, left + n, right + n);
    }
    return n;
}

int xa_resample_frame(XaDecoder *d, int16_t left, int16_t right, int16_t *out) {
    int k, i, ch;

    d->ring[0][d->pos & (XA_RING - 1)] = left;
    d->ring[1][d->pos & (XA_RING - 1)] = right;
    d->pos++;
    if (--d->sixstep > 0) {
        return 0;
    }
    d->sixstep = 6;
    for (k = 0; k < XA_PHASES; k++) {
        for (ch = 0; ch < 2; ch++) {
            int32_t sum = 0;

            for (i = 1; i <= XA_TAPS; i++) {
                sum += (d->ring[ch][(d->pos - (uint32_t)i) & (XA_RING - 1)] * xa_zigzag[k][i - 1]) >> 15;
            }
            out[2 * k + ch] = (int16_t)xa_clamp16(sum);
        }
    }
    return XA_PHASES;
}

int xa_decode_sector(XaDecoder *d, const uint8_t *sector, int16_t *out) {
    int16_t left[XA_MAX_DECODED], right[XA_MAX_DECODED];
    uint8_t ci = sector[19];
    int stereo = XA_CI_STEREO(ci), repeat = XA_CI_HALF_RATE(ci) ? 2 : 1;
    int n = xa_decode_groups(d, ci, sector + 24, left, right), frames = 0, i, r;

    for (i = 0; i < n; i++) {
        for (r = 0; r < repeat; r++) {
            frames += xa_resample_frame(d, left[i], stereo ? right[i] : left[i], out + 2 * frames);
        }
    }
    return frames;
}
