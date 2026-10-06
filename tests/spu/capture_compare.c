/* Finds windows of our rendering in an emulator's audio capture (tests/spu/capture.py) and reports how well they match.
 *
 *   capture_compare OURS.wav CAPTURE.f32 START_FRAME [WINDOW_FRAMES]
 *
 * OURS: 16-bit stereo WAV at 44,100 Hz (a 44-byte header); CAPTURE: raw F32LE stereo (SDL3's disk driver). The stretch
 * of our rendering from START_FRAME on is first placed in the capture by its 10 ms RMS envelope (its first 3 s: a short
 * window alone may match a repeat of the music); then each window (default 4,410 frames, 0.1 s) is located by
 * cross-correlation of the mono mix within +-1,000 frames of the previous window's lag, or, when that finds no good
 * match (the capture has a gap there: it only gains frames), from 1,000 frames before to 30,000 after. Per window: the
 * lag, the normalized correlation of the mono mix there, and the capture's level over ours per channel. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENV_HOP 441

static float *ours, *cap;
static long nours, ncap;

static float *load(const char *path, long *frames, int wav) {
    FILE *f = fopen(path, "rb");
    long size, i;
    float *out;
    unsigned char *raw;

    if (f == NULL) {
        perror(path);
        exit(2);
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    raw = malloc((size_t)size);
    if (fread(raw, 1, (size_t)size, f) != (size_t)size) {
        perror(path);
        exit(2);
    }
    fclose(f);
    if (wav) {
        *frames = (size - 44) / 4;
        out = malloc((size_t)*frames * 2 * sizeof(float));
        for (i = 0; i < 2 * *frames; i++) {
            out[i] = (float)(short)(raw[44 + 2 * i] | raw[45 + 2 * i] << 8);
        }
    } else {
        *frames = size / 8;
        out = malloc((size_t)*frames * 2 * sizeof(float));
        for (i = 0; i < 2 * *frames; i++) {
            float v;
            memcpy(&v, raw + 4 * i, 4);
            out[i] = v * 32768.0f;
        }
    }
    free(raw);
    return out;
}

/* The 10 ms RMS envelope of the mono mix. */
static double *envelope(const float *x, long frames, long *n) {
    long i, k;
    double *e;

    *n = frames / ENV_HOP;
    e = calloc((size_t)*n + 1, sizeof(double));
    for (k = 0; k < *n; k++) {
        double s = 0;
        for (i = k * ENV_HOP; i < (k + 1) * ENV_HOP; i++) {
            double m = 0.5 * (x[2 * i] + x[2 * i + 1]);
            s += m * m;
        }
        e[k] = sqrt(s / ENV_HOP);
    }
    return e;
}

static double ncc(const double *a, const double *b, long n) {
    double ab = 0, aa = 0, bb = 0;
    long i;
    for (i = 0; i < n; i++) {
        ab += a[i] * b[i];
        aa += a[i] * a[i];
        bb += b[i] * b[i];
    }
    return aa > 0 && bb > 0 ? ab / sqrt(aa * bb) : 0;
}

/* The normalized correlation of the mono mixes: ours at w, the capture at w + lag, `n` frames (every `step`-th). */
static double corr(long w, long lag, long n, int step) {
    double s = 0, p = 0, q = 0;
    long i;
    if (w + lag < 0 || w + lag + n > ncap) {
        return -2;
    }
    for (i = 0; i < n; i += step) {
        double a = (double)ours[2 * (w + i)] + ours[2 * (w + i) + 1];
        double b = (double)cap[2 * (w + lag + i)] + cap[2 * (w + lag + i) + 1];
        s += a * b;
        p += a * a;
        q += b * b;
    }
    return p > 0 && q > 0 ? s / sqrt(p * q) : 0;
}

static long search(long w, long n, long lo, long hi, double *best) {
    long lag, best_lag = lo;
    *best = -2;
    for (lag = lo; lag <= hi; lag++) {
        double c = corr(w, lag, n, 2);
        if (c > *best) {
            *best = c;
            best_lag = lag;
        }
    }
    return best_lag;
}

int main(int argc, char **argv) {
    long start, window = 4410, ne_ours, ne_cap, prev, w;
    double *eo, *ec, sum_r = 0;
    int windows = 0, good = 0, gaps = 0;

    if (argc < 4) {
        fprintf(stderr, "usage: capture_compare OURS.wav CAPTURE.f32 START_FRAME [WINDOW_FRAMES]\n");
        return 2;
    }
    ours = load(argv[1], &nours, 1);
    cap = load(argv[2], &ncap, 0);
    start = atol(argv[3]);
    if (argc > 4) {
        window = atol(argv[4]);
    }
    eo = envelope(ours, nours, &ne_ours);
    ec = envelope(cap, ncap, &ne_cap);
    printf("ours %ld frames, capture %ld frames; windows of %ld frames from frame %ld\n", nours, ncap, window, start);
    {
        long ew = start / ENV_HOP, en = (nours - start) / ENV_HOP, k, best_k = 0;
        double best = -2;
        en = en > 300 ? 300 : en;
        for (k = 0; k + en <= ne_cap; k++) {
            double c = ncc(eo + ew, ec + k, en);
            if (c > best) {
                best = c;
                best_k = k;
            }
        }
        prev = best_k * ENV_HOP - start;
        printf("the stretch starts near capture frame %ld (envelope correlation %.3f over %ld ms)\n", best_k * ENV_HOP,
               best, en * 10);
        prev = search(start, window, prev - 1000, prev + 1000, &best);
    }
    printf("window  ours.at  capture.at  correlation  level.l  level.r  (capture / ours)\n");
    for (w = start; w + window <= nours; w += window) {
        double c, aa[2] = {0, 0}, bb[2] = {0, 0};
        long lag = search(w, window, prev - 1000, prev + 1000, &c), i;
        int ch;

        if (c < 0.8) {
            double c2;
            long lag2 = search(w, window, prev - 1000, prev + 30000, &c2);
            if (c2 > c + 0.1) {
                lag = lag2;
                c = c2;
                gaps++;
            }
        }
        c = corr(w, lag, window, 1);
        for (i = 0; i < window; i++) {
            for (ch = 0; ch < 2; ch++) {
                aa[ch] += (double)ours[2 * (w + i) + ch] * ours[2 * (w + i) + ch];
                bb[ch] += (double)cap[2 * (w + lag + i) + ch] * cap[2 * (w + lag + i) + ch];
            }
        }
        printf("%6d  %7ld  %10ld  %11.4f  %7.3f  %7.3f\n", windows, w, w + lag, c,
               aa[0] > 0 ? sqrt(bb[0] / aa[0]) : 0, aa[1] > 0 ? sqrt(bb[1] / aa[1]) : 0);
        sum_r += c;
        good += c >= 0.9;
        windows++;
        prev = lag;
    }
    if (windows) {
        printf("%d windows: correlation mean %.4f, %d at 0.9 or more; %d gaps in the capture skipped\n", windows,
               sum_r / windows, good, gaps);
    }
    free(ours);
    free(cap);
    free(eo);
    free(ec);
    return 0;
}
