/* port/psyq/xa.h: the CD-ROM drive's XA-ADPCM decoder and its 37,800 -> 44,100 Hz resampler (port/psyq/xa.c; M5),
 * what the drive does with an audio sector before its samples reach the SPU's CD input. Plain integer code over
 * caller-owned state, no Psy-Q or port dependency: libcd.c drives it, tests/xa/ checks it against a Python model. */
#ifndef PORT_PSYQ_XA_H
#define PORT_PSYQ_XA_H

#include <stdint.h>

#define XA_GROUPS 18          /* 128-byte sound groups per sector (0x900 bytes) */
#define XA_GROUP_BYTES 128
#define XA_UNIT_SAMPLES 28    /* samples per sound unit */
#define XA_RING 32            /* the resampler's ring of 37,800 Hz samples, per channel */
#define XA_TAPS 29            /* the zigzag tables' points */
#define XA_PHASES 7           /* 44,100 Hz samples out per six 37,800 Hz samples in */

/* Most samples a sector gives: 4-bit mono = 18 x 8 x 28 = 4032 decoded samples; at 18,900 Hz each is played twice
 * (8064 at 37,800 Hz); x 7/6 = 9408 at 44,100 Hz. A 37,800 Hz stereo sector (the movies') gives 2016 frames, 2352
 * at 44,100 Hz. */
#define XA_MAX_DECODED 4032
#define XA_MAX_OUT 9408

/* The coding info byte (the subheader's 4th byte), the readings xa.c takes of it. */
#define XA_CI_STEREO(ci) (((ci) & 0x03) == 1)
#define XA_CI_HALF_RATE(ci) ((((ci) >> 2) & 0x03) == 1) /* 18,900 Hz */
#define XA_CI_8BIT(ci) ((((ci) >> 4) & 0x03) == 1)

typedef struct XaDecoder {
    int32_t old[2], older[2];      /* the ADPCM filter's last two samples, per channel (left/mono, right) */
    int16_t ring[2][XA_RING];      /* the resampler's last 32 input samples, per channel */
    uint32_t pos;                  /* the ring's write position (counts up, used modulo 32) */
    int sixstep;                   /* 37,800 Hz samples left before the next seven outputs (6..1) */
} XaDecoder;

/* The decoder as after a reset: no history, an empty ring, the six-step counter at 6. */
void xa_reset(XaDecoder *d);

/* Decodes one 128-byte sound group `g`, as xa_decode_groups below does each of its 18: returns the samples per channel
 * (112 or 224 at 4 bits, stereo or mono; 56 or 112 at 8 bits). */
int xa_decode_group(XaDecoder *d, uint8_t ci, const uint8_t *g, int16_t *left, int16_t *right);

/* Decodes the 18 sound groups at `groups` (0x900 bytes: the Form 2 data after the 8-byte subheader) with coding info
 * `ci` into 16-bit samples: stereo into left[] and right[], mono into left[] only. Returns the samples per channel
 * (2016 or 4032 at 4 bits, 1008 or 2016 at 8 bits); the ADPCM history in `d` carries over to the next sector. */
int xa_decode_groups(XaDecoder *d, uint8_t ci, const uint8_t *groups, int16_t *left, int16_t *right);

/* One 37,800 Hz frame into the resampler; writes 0 or 7 stereo frames at 44,100 Hz to out (interleaved L, R) and
 * returns how many. */
int xa_resample_frame(XaDecoder *d, int16_t left, int16_t right, int16_t *out);

/* A whole audio sector: `sector` is the raw 2352-byte sector (sync, header, subheader, data). Decodes it, plays an
 * 18,900 Hz sample twice and a mono sample on both sides, and resamples: writes the 44,100 Hz stereo frames to out
 * (room for XA_MAX_OUT frames) and returns how many. */
int xa_decode_sector(XaDecoder *d, const uint8_t *sector, int16_t *out);

/* The documentation's tables (tests/xa checks them): the filters' coefficients and the zigzag tables, xa_zigzag[k]
 * being psx-spx's Table(k+1), entry i (0..28) its index i+1. */
extern const int16_t xa_filter_pos[4], xa_filter_neg[4];
extern const int16_t xa_zigzag[XA_PHASES][XA_TAPS];

#endif /* PORT_PSYQ_XA_H */
