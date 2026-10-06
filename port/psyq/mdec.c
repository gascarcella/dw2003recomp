/* port/psyq/mdec.c: the movie decoder. Two parts, as on the PS1:
 *  - the bit-stream decoder LIBPRESS runs on the CPU (DecDCTvlc2): a movie frame (the .STR "version 2" format) to the
 *    MDEC's run-level halfwords;
 *  - the MDEC (the macroblock decoder chip): run-level halfwords to pixels, a 16x16 macroblock at a time.
 * Written from public descriptions only: psx-spx "Macroblock Decoder (MDEC)" (the run-level format, the quantisation,
 * the zig-zag order, the IDCT built on the scale table, the YCbCr to RGB conversion, the output formats) and "CDROM
 * File Video STR Streaming and BS Picture Compression" (the frame header, the bit stream and its AC code table, which
 * is MPEG-1's DCT coefficient table, ISO 11172-2 table B.5). No emulator or Sony code is used: the Psy-Q functions'
 * results were checked against the PS1 in PCSX-Redux (the layer-1 family tests/golden/families/mdec.py, replayed
 * through this file by tests/host/mdec_replay.py).
 *
 * The frame (what StGetNext hands the player): a header of four halfwords (the run-level size in words, 0x3800, the
 * quantiser, the version: 2 in every movie of the disc), then the bit stream: 16-bit little-endian units read from bit
 * 15 down. Per block, the DC coefficient is 10 bits (signed) and becomes the halfword (quantiser << 10 | dc); then AC
 * codes, each (run, level) with a sign bit, the escape 000001 followed by the 16-bit halfword itself (run 6 bits, level
 * 10 bits), and the end of the block "10" (0xFE00). A DC of 0x1FF ends the frame's data. The output starts with the
 * MDEC command word 0x3800 << 16 | size (the header's first word) and has `size` words after it, 0xFE00 after the
 * last block (as LIBPRESS writes them: the golden family's DecDCTvlc2 cases are equal word for word).
 *
 * The MDEC (psx-spx): per block, 0xFE00 halfwords before the DC are skipped; DC = dc * table[0] (the quantiser does
 * not apply), AC k = (level * table[k] * quantiser + 4) >> 3, each saturated to -0x400..0x3FF and stored at the
 * zig-zag position of k; with quantiser 0 every value is level * 2, stored in index order (no table, no zig-zag). A run
 * taking k past 63 ends the block. The 8x8 IDCT is two passes of the scale table (the cosine matrix, 0x5A82 ..., the
 * last three bits dropped) with a rounded division by 0x2000 each, the colour blocks are Cr, Cb (8x8 for the whole
 * 16x16 macroblock), then Y top-left, top-right, bottom-left, bottom-right; RGB = Y + (1.402 Cr, -0.3437 Cb - 0.7143 Cr,
 * 1.772 Cb), clamped to -128..127, + 128 unless the command's signed bit (26) is set. 24-bit output is R, G, B bytes;
 * 15-bit output rounds each colour to 5 bits ((c + 4) >> 3, at most 31), with bit 15 from the command's bit 25. A
 * macroblock's 16 rows are output in order (16 x 3 bytes or 16 x 2 bytes each).
 *
 * Against PCSX-Redux's MDEC (the golden family): the run-level rules, the quantiser-0 form, the runs past 63, the
 * zig-zag order, the tables and the 15-bit rounding agree; its IDCT and colour conversion round differently, so about
 * 40% of the pixel bytes differ by 1 to 3 (4 at most); it does not skip 0xFE00 before a DC (psx-spx: the hardware
 * does; no movie has one) and treats out-of-range colours otherwise (2 bytes of the 5 movie frames checked are such).
 * The opening movie's VRAM matches the emulator's at the same movie frames within those differences. */
#include <string.h>

#include "psyq_internal.h"

/* ---- the bit stream (DecDCTvlc2) ---- */

/* MPEG-1's DCT coefficient codes (without the sign bit): the code, its run and level. "10" (end of block) and
 * "000001" (escape) are handled by the decoder. */
static const struct {
    const char *code;
    u8 run, level;
} mdec_ac_codes[] = {
    { "11", 0, 1 }, { "011", 1, 1 }, { "0100", 0, 2 }, { "0101", 2, 1 }, { "00101", 0, 3 }, { "00111", 3, 1 },
    { "00110", 4, 1 }, { "000110", 1, 2 }, { "000111", 5, 1 }, { "000101", 6, 1 }, { "000100", 7, 1 },
    { "0000110", 0, 4 }, { "0000100", 2, 2 }, { "0000111", 8, 1 }, { "0000101", 9, 1 },
    { "00100110", 0, 5 }, { "00100001", 0, 6 }, { "00100101", 1, 3 }, { "00100100", 3, 2 }, { "00100111", 10, 1 },
    { "00100011", 11, 1 }, { "00100010", 12, 1 }, { "00100000", 13, 1 },
    { "0000001010", 0, 7 }, { "0000001100", 1, 4 }, { "0000001011", 2, 3 }, { "0000001111", 4, 2 },
    { "0000001001", 5, 2 }, { "0000001110", 14, 1 }, { "0000001101", 15, 1 }, { "0000001000", 16, 1 },
    { "000000011101", 0, 8 }, { "000000011000", 0, 9 }, { "000000010011", 0, 10 }, { "000000010000", 0, 11 },
    { "000000011011", 1, 5 }, { "000000010100", 2, 4 }, { "000000011100", 3, 3 }, { "000000010010", 4, 3 },
    { "000000011110", 6, 2 }, { "000000010101", 7, 2 }, { "000000010001", 8, 2 }, { "000000011111", 17, 1 },
    { "000000011010", 18, 1 }, { "000000011001", 19, 1 }, { "000000010111", 20, 1 }, { "000000010110", 21, 1 },
    { "0000000011010", 0, 12 }, { "0000000011001", 0, 13 }, { "0000000011000", 0, 14 }, { "0000000010111", 0, 15 },
    { "0000000010110", 1, 6 }, { "0000000010101", 1, 7 }, { "0000000010100", 2, 5 }, { "0000000010011", 3, 4 },
    { "0000000010010", 5, 3 }, { "0000000010001", 9, 2 }, { "0000000010000", 10, 2 }, { "0000000011111", 22, 1 },
    { "0000000011110", 23, 1 }, { "0000000011101", 24, 1 }, { "0000000011100", 25, 1 }, { "0000000011011", 26, 1 },
    { "00000000011111", 0, 16 }, { "00000000011110", 0, 17 }, { "00000000011101", 0, 18 },
    { "00000000011100", 0, 19 }, { "00000000011011", 0, 20 }, { "00000000011010", 0, 21 },
    { "00000000011001", 0, 22 }, { "00000000011000", 0, 23 }, { "00000000010111", 0, 24 },
    { "00000000010110", 0, 25 }, { "00000000010101", 0, 26 }, { "00000000010100", 0, 27 },
    { "00000000010011", 0, 28 }, { "00000000010010", 0, 29 }, { "00000000010001", 0, 30 },
    { "00000000010000", 0, 31 },
    { "000000000011000", 0, 32 }, { "000000000010111", 0, 33 }, { "000000000010110", 0, 34 },
    { "000000000010101", 0, 35 }, { "000000000010100", 0, 36 }, { "000000000010011", 0, 37 },
    { "000000000010010", 0, 38 }, { "000000000010001", 0, 39 }, { "000000000010000", 0, 40 },
    { "000000000011111", 1, 8 }, { "000000000011110", 1, 9 }, { "000000000011101", 1, 10 },
    { "000000000011100", 1, 11 }, { "000000000011011", 1, 12 }, { "000000000011010", 1, 13 },
    { "000000000011001", 1, 14 },
    { "0000000000010011", 1, 15 }, { "0000000000010010", 1, 16 }, { "0000000000010001", 1, 17 },
    { "0000000000010000", 1, 18 }, { "0000000000010100", 6, 3 }, { "0000000000011010", 11, 2 },
    { "0000000000011001", 12, 2 }, { "0000000000011000", 13, 2 }, { "0000000000010111", 14, 2 },
    { "0000000000010110", 15, 2 }, { "0000000000010101", 16, 2 }, { "0000000000011111", 27, 1 },
    { "0000000000011110", 28, 1 }, { "0000000000011101", 29, 1 }, { "0000000000011100", 30, 1 },
    { "0000000000011011", 31, 1 },
};

/* Lookup by the next 16 bits: length of the code (0 = not a code), the halfword (run << 10 | level). Every code is at
 * most 16 bits long, so the table maps each 16-bit prefix to the one code it starts with. Built on first use. */
#define MDEC_VLC_EOB 0xFE
#define MDEC_VLC_ESCAPE 0xFF
#define MDEC_VLC_END 0x1FF /* a DC of 0x1FF: the end of the frame's data (checked against the PS1's LIBPRESS) */
static u8 mdec_vlc_len[1 << 16];
static u16 mdec_vlc_val[1 << 16];
static int mdec_vlc_ready;

static void mdec_vlc_fill(const char *code, u8 len_tag, u16 val) {
    u32 len = (u32)strlen(code), prefix = 0, i, n;

    for (i = 0; i < len; i++) {
        prefix = (prefix << 1) | (u32)(code[i] - '0');
    }
    n = 1u << (16 - len);
    for (i = 0; i < n; i++) {
        mdec_vlc_len[(prefix << (16 - len)) | i] = len_tag != 0 ? len_tag : (u8)len;
        mdec_vlc_val[(prefix << (16 - len)) | i] = val;
    }
}

static void mdec_vlc_init(void) {
    u32 i;

    if (mdec_vlc_ready) {
        return;
    }
    for (i = 0; i < sizeof(mdec_ac_codes) / sizeof(mdec_ac_codes[0]); i++) {
        mdec_vlc_fill(mdec_ac_codes[i].code, 0, (u16)((mdec_ac_codes[i].run << 10) | mdec_ac_codes[i].level));
    }
    mdec_vlc_fill("10", MDEC_VLC_EOB, 0xFE00);
    mdec_vlc_fill("000001", MDEC_VLC_ESCAPE, 0);
    mdec_vlc_ready = 1;
}

/* The bit reader: 16-bit little-endian units, most significant bit first. `bits` holds the next 32 bits at the top. */
typedef struct MdecBits {
    const u8 *p;
    u32 bits;  /* the next bits, left-aligned */
    int avail; /* how many of them are valid */
} MdecBits;

static void mdec_bits_refill(MdecBits *b) {
    while (b->avail <= 16) {
        u32 unit = (u32)b->p[0] | ((u32)b->p[1] << 8);

        b->p += 2;
        b->bits |= unit << (16 - b->avail);
        b->avail += 16;
    }
}

static u32 mdec_bits_peek16(MdecBits *b) {
    mdec_bits_refill(b);
    return b->bits >> 16;
}

static void mdec_bits_skip(MdecBits *b, int n) {
    b->bits <<= n;
    b->avail -= n;
}

static u32 mdec_bits_get(MdecBits *b, int n) {
    u32 v;

    mdec_bits_refill(b);
    v = b->bits >> (32 - n);
    mdec_bits_skip(b, n);
    return v;
}

int mdec_vlc_decode(const u8 *frame, u32 *out) {
    MdecBits b;
    u32 size = (u32)frame[0] | ((u32)frame[1] << 8);
    u32 quant = ((u32)frame[4] | ((u32)frame[5] << 8)) & 0x3F;
    u32 total = size * 2, n = 0; /* halfwords to write, written */
    u16 *hw = (u16 *)(out + 1);
    int in_block = 0;

    mdec_vlc_init();
    out[0] = (0x3800u << 16) | size;
    b.p = frame + 8;
    b.bits = 0;
    b.avail = 0;
    while (n < total) {
        u32 peek, len;

        if (!in_block) {
            u32 dc = mdec_bits_get(&b, 10);

            if (dc == MDEC_VLC_END) {
                /* The end of the frame's data: the rest is 0xFE00 (the encoder's padding is not decoded). */
                while (n < total) {
                    hw[n++] = 0xFE00;
                }
                break;
            }
            hw[n++] = (u16)((quant << 10) | dc);
            in_block = 1;
            continue;
        }
        peek = mdec_bits_peek16(&b);
        len = mdec_vlc_len[peek];
        if (len == MDEC_VLC_EOB) {
            mdec_bits_skip(&b, 2);
            hw[n++] = 0xFE00;
            in_block = 0;
        } else if (len == MDEC_VLC_ESCAPE) {
            mdec_bits_skip(&b, 6);
            hw[n++] = (u16)mdec_bits_get(&b, 16);
        } else if (len != 0) {
            u32 val = mdec_vlc_val[peek];

            mdec_bits_skip(&b, (int)len);
            if (mdec_bits_get(&b, 1)) {
                val = (val & 0xFC00) | ((0x400 - (val & 0x3FF)) & 0x3FF);
            }
            hw[n++] = (u16)val;
        } else {
            /* Not a code (a damaged stream): end the block. */
            PSYQ_TRACE("mdec: bad code %04x at halfword %u", peek, n);
            mdec_bits_skip(&b, 1);
            hw[n++] = 0xFE00;
            in_block = 0;
        }
    }
    return 0;
}

/* ---- the MDEC ---- */

/* psx-spx's zig-zag table: zagzig[k] = the position (row * 8 + column) of coefficient k. */
static const u8 mdec_zagzig[64] = {
    0,  1,  8,  16, 9,  2,  3,  10, 17, 24, 32, 25, 18, 11, 4,  5,  12, 19, 26, 33, 40, 48,
    41, 34, 27, 20, 13, 6,  7,  14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23,
    30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63,
};

/* LIBPRESS's quantisation table (the MPEG-1 intra matrix with 2 as its first value), in zig-zag order, for both
 * luminance and colour; DecDCTReset(0) loads it. */
static const u8 mdec_default_quant[64] = {
    2,  16, 16, 19, 16, 19, 22, 22, 22, 22, 22, 22, 26, 24, 26, 27, 27, 27, 26, 26, 26, 26,
    27, 27, 27, 29, 29, 29, 34, 34, 34, 29, 29, 29, 27, 27, 29, 29, 32, 32, 34, 34, 37, 38,
    37, 35, 35, 34, 35, 38, 38, 40, 40, 40, 48, 48, 46, 46, 56, 56, 58, 69, 69, 83,
};

/* The IDCT's scale table: row u, column x = floor(0x8000 * c(u) * cos((2x + 1) u pi / 16)), c(0) = 1 / sqrt(2),
 * c(u) = 1 otherwise; DecDCTReset(0) loads it. */
static const s16 mdec_default_scale[64] = {
    0x5A82,  0x5A82,  0x5A82,  0x5A82,  0x5A82,  0x5A82,  0x5A82,  0x5A82,
    0x7D8A,  0x6A6D,  0x471C,  0x18F8,  -0x18F9, -0x471D, -0x6A6E, -0x7D8B,
    0x7641,  0x30FB,  -0x30FC, -0x7642, -0x7642, -0x30FC, 0x30FB,  0x7641,
    0x6A6D,  -0x18F9, -0x7D8B, -0x471D, 0x471C,  0x7D8A,  0x18F8,  -0x6A6E,
    0x5A82,  -0x5A83, -0x5A83, 0x5A82,  0x5A82,  -0x5A83, -0x5A83, 0x5A82,
    0x471C,  -0x7D8B, 0x18F8,  0x6A6D,  -0x6A6E, -0x18F9, 0x7D8A,  -0x471D,
    0x30FB,  -0x7642, 0x7641,  -0x30FC, -0x30FC, 0x7641,  -0x7642, 0x30FB,
    0x18F8,  -0x471D, 0x6A6D,  -0x7D8B, 0x7D8A,  -0x6A6E, 0x471C,  -0x18F9,
};

static struct {
    u8 quant_y[64], quant_c[64];
    s16 scale[64];
    u32 cmd;            /* the decode command (DecDCTin's first word, with the mode bits) */
    const u16 *in;      /* the next run-level halfword */
    u32 in_left;        /* halfwords left */
    u8 mb[16 * 16 * 3]; /* the current macroblock's pixels, in output order */
    u32 mb_len, mb_pos; /* its size in bytes, bytes already output */
} mdec;

void mdec_reset(void) {
    memcpy(mdec.quant_y, mdec_default_quant, 64);
    memcpy(mdec.quant_c, mdec_default_quant, 64);
    memcpy(mdec.scale, mdec_default_scale, sizeof(mdec.scale));
    mdec.in = NULL;
    mdec.in_left = 0;
    mdec.mb_len = mdec.mb_pos = 0;
}

void mdec_decode_start(const u32 *rl) {
    mdec.cmd = rl != NULL ? rl[0] : 0;
    mdec.in = rl != NULL ? (const u16 *)(rl + 1) : NULL;
    mdec.in_left = rl != NULL ? (rl[0] & 0xFFFF) * 2 : 0;
    mdec.mb_len = mdec.mb_pos = 0;
}

static s32 mdec_s10(u32 v) {
    return (s32)((v & 0x3FF) ^ 0x200) - 0x200;
}

static s32 mdec_sat11(s32 v) {
    return v < -0x400 ? -0x400 : (v > 0x3FF ? 0x3FF : v);
}

/* One block's coefficients (psx-spx rl_decode_block). 0 = the input ended first. */
static int mdec_rl_block(s32 *blk, const u8 *quant) {
    u32 n, q, k;

    memset(blk, 0, 64 * sizeof(*blk));
    do {
        if (mdec.in_left == 0) {
            return 0;
        }
        n = *mdec.in++;
        mdec.in_left--;
    } while (n == 0xFE00);
    q = n >> 10;
    k = 0;
    blk[0] = mdec_sat11(q == 0 ? mdec_s10(n) * 2 : mdec_s10(n) * quant[0]);
    for (;;) {
        s32 v;

        if (mdec.in_left == 0) {
            return 1;
        }
        n = *mdec.in++;
        mdec.in_left--;
        k += (n >> 10) + 1;
        if (k > 63) {
            return 1;
        }
        if (q == 0) {
            blk[k] = mdec_sat11(mdec_s10(n) * 2);
        } else {
            v = mdec_s10(n) * (s32)quant[k] * (s32)q;
            blk[mdec_zagzig[k]] = mdec_sat11((v + 4) >> 3);
        }
    }
}

/* The 8x8 IDCT (psx-spx real_idct_core): two passes, each output = sum of 8 products with the scale table's entries
 * >> 3, rounded and divided by 0x2000; a pass reads columns and writes rows, so two passes are the whole transform. */
static void mdec_idct(s32 *blk) {
    s32 tmp[64];
    s32 *src = blk, *dst = tmp;
    int pass, x, y, z;

    for (pass = 0; pass < 2; pass++) {
        for (x = 0; x < 8; x++) {
            for (y = 0; y < 8; y++) {
                s32 sum = 0;

                for (z = 0; z < 8; z++) {
                    sum += src[y + z * 8] * (mdec.scale[x + z * 8] >> 3);
                }
                dst[x + y * 8] = (sum + 0x1000) >> 13;
            }
        }
        src = tmp;
        dst = blk;
    }
}

static s32 mdec_clamp8(s32 v) {
    return v < -128 ? -128 : (v > 127 ? 127 : v);
}

/* An 8-bit colour to 5 bits, rounded as the emulator's MDEC does (the 15-bit cases of tests/golden/families/mdec.py). */
static u32 mdec_to5(u32 v) {
    v = (v + 4) >> 3;
    return v > 31 ? 31 : v;
}

/* The colour of one 8x8 luminance block at (xx, yy) of the macroblock into mdec.mb. */
static void mdec_yuv_to_rgb(const s32 *cr, const s32 *cb, const s32 *yb, int xx, int yy) {
    int x, y;
    int bpp = (mdec.cmd >> 27 & 3) == 2 ? 3 : 2;
    u32 flip = (mdec.cmd & (1u << 26)) ? 0 : 0x80;
    u32 bit15 = (mdec.cmd & (1u << 25)) ? 0x8000 : 0;

    for (y = 0; y < 8; y++) {
        for (x = 0; x < 8; x++) {
            int c = ((x + xx) >> 1) + ((y + yy) >> 1) * 8;
            s32 r = cr[c], b = cb[c], yv = yb[x + y * 8];
            s32 gr = (-22527 * b - 46812 * r + 0x8000) >> 16;
            s32 rr = (91881 * r + 0x8000) >> 16;
            s32 br = (116130 * b + 0x8000) >> 16;
            u32 R = ((u32)mdec_clamp8(yv + rr) ^ flip) & 0xFF;
            u32 G = ((u32)mdec_clamp8(yv + gr) ^ flip) & 0xFF;
            u32 B = ((u32)mdec_clamp8(yv + br) ^ flip) & 0xFF;
            u8 *p = mdec.mb + ((y + yy) * 16 + x + xx) * bpp;

            if (bpp == 3) {
                p[0] = (u8)R;
                p[1] = (u8)G;
                p[2] = (u8)B;
            } else {
                u32 v = mdec_to5(R) | (mdec_to5(G) << 5) | (mdec_to5(B) << 10) | bit15;

                p[0] = (u8)v;
                p[1] = (u8)(v >> 8);
            }
        }
    }
}

/* Decodes the next macroblock into mdec.mb. 0 = no input left. */
static int mdec_macroblock(void) {
    static const int pos[4][2] = { { 0, 0 }, { 8, 0 }, { 0, 8 }, { 8, 8 } };
    s32 cr[64], cb[64], yb[64];
    int i;

    if (!mdec_rl_block(cr, mdec.quant_c) || !mdec_rl_block(cb, mdec.quant_c)) {
        return 0;
    }
    mdec_idct(cr);
    mdec_idct(cb);
    for (i = 0; i < 4; i++) {
        if (!mdec_rl_block(yb, mdec.quant_y)) {
            return 0;
        }
        mdec_idct(yb);
        mdec_yuv_to_rgb(cr, cb, yb, pos[i][0], pos[i][1]);
    }
    mdec.mb_len = 16 * 16 * ((mdec.cmd >> 27 & 3) == 2 ? 3 : 2);
    mdec.mb_pos = 0;
    return 1;
}

u32 mdec_decode_out(u32 *dst, u32 words) {
    u8 *out = (u8 *)dst;
    u32 bytes = words * 4, done = 0;

    while (done < bytes) {
        u32 n;

        if (mdec.mb_pos == mdec.mb_len && (mdec.in == NULL || !mdec_macroblock())) {
            break;
        }
        n = mdec.mb_len - mdec.mb_pos;
        if (n > bytes - done) {
            n = bytes - done;
        }
        memcpy(out + done, mdec.mb + mdec.mb_pos, n);
        mdec.mb_pos += n;
        done += n;
    }
    return done / 4;
}
