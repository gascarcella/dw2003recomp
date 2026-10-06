/* tests/host/gte_harness.c: runs the gte golden family's cases (tests/golden/families/gte.py) through the port's
 * software GTE and LIBGTE (port/psyq/gte.c, libgte.c); tests/host/gte_replay.py builds it and drives it over stdin,
 * one command per line, one answer line each (hex bytes):
 *   G <word|-> <in: 64 words, 512 hex digits>   the PS1 routine: ctc2 every control register, mtc2 the data registers
 *                                                but 15, 28, 29, 31, the command (- none) -> the 64 words read back
 *   W <in> <count> <step>                        RTPS sweep: TRZ = in's TRZ + i * step, RTPS sf 1 -> MAC0, FLAG pairs
 *   S <rsin|rcos> <first> <count>                -> the s16 results for first .. first + count - 1
 *   F <function> <in> <hex buffers...>           the 64 registers loaded as for G, then RotMatrixYXZ_gte r m,
 *                                                RotMatrixZYX_gte r m, ScaleMatrix m v or ApplyMatrixSV m v0 v1 ->
 *                                                the written buffer (m, m, m, v1), then the 64 registers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psyq_internal.h"
#include "psyq/libgte.h"

s32 rcos(s32 a);

/* The shim's tracing, off (psyq.c is not linked). */
int psyq_trace_state = 0;
int psyq_trace_decide(void) {
    return 0;
}
void psyq_trace_printf(const char *fmt, ...) {
    (void)fmt;
}

static int hex_bytes(const char *hex, u8 *out, size_t max) {
    size_t n = strlen(hex) / 2;
    size_t i;

    if (n > max) {
        return -1;
    }
    for (i = 0; i < n; i++) {
        unsigned b;

        if (sscanf(hex + 2 * i, "%2x", &b) != 1) {
            return -1;
        }
        out[i] = (u8)b;
    }
    return (int)n;
}

static void put_hex(const void *p, size_t n) {
    const u8 *b = p;
    size_t i;

    for (i = 0; i < n; i++) {
        printf("%02x", b[i]);
    }
    printf("\n");
}

static u32 word_at(const u8 *b, int i) {
    return b[4 * i] | (b[4 * i + 1] << 8) | (b[4 * i + 2] << 16) | ((u32)b[4 * i + 3] << 24);
}

static void load_regs(const u8 *in) {
    int i;

    for (i = 0; i < 32; i++) {
        psyq_gte_ctc2(i, word_at(in, 32 + i));
    }
    for (i = 0; i < 32; i++) {
        if (i != 15 && i != 28 && i != 29 && i != 31) {
            psyq_gte_mtc2(i, word_at(in, i));
        }
    }
}

static void store_regs(u8 *out) {
    int i;

    for (i = 0; i < 64; i++) {
        u32 w = i < 32 ? psyq_gte_mfc2(i) : psyq_gte_cfc2(i - 32);

        out[4 * i] = (u8)w;
        out[4 * i + 1] = (u8)(w >> 8);
        out[4 * i + 2] = (u8)(w >> 16);
        out[4 * i + 3] = (u8)(w >> 24);
    }
}

/* A buffer then the 64 registers, on one line. */
static void put_hex_regs(const void *p, size_t n) {
    u8 regs[256];
    const u8 *b = p;
    size_t i;

    store_regs(regs);
    for (i = 0; i < n; i++) {
        printf("%02x", b[i]);
    }
    put_hex(regs, sizeof(regs));
}

int main(void) {
    static char line[1 << 16];
    static u8 out[0x4000];

    while (fgets(line, sizeof(line), stdin) != NULL) {
        char *argv[8];
        int argc = 0;
        char *tok = strtok(line, " \r\n");
        u8 in[256];

        while (tok != NULL && argc < 8) {
            argv[argc++] = tok;
            tok = strtok(NULL, " \r\n");
        }
        if (argc == 0) {
            continue;
        }
        if (strcmp(argv[0], "G") == 0 && argc == 3 && hex_bytes(argv[2], in, sizeof(in)) == 256) {
            load_regs(in);
            if (strcmp(argv[1], "-") != 0) {
                psyq_gte_cmd((u32)strtoul(argv[1], NULL, 0));
            }
            store_regs(out);
            put_hex(out, 256);
        } else if (strcmp(argv[0], "W") == 0 && argc == 4 && hex_bytes(argv[1], in, sizeof(in)) == 256) {
            long count = strtol(argv[2], NULL, 0);
            u32 step = (u32)strtoul(argv[3], NULL, 0);
            u32 trz;
            long i;

            load_regs(in);
            trz = word_at(in, 32 + 7);
            for (i = 0; i < count && i < (long)sizeof(out) / 8; i++) {
                u32 mac0;
                u32 flag;

                psyq_gte_ctc2(7, trz);
                psyq_gte_cmd(0x80001); /* RTPS sf 1 */
                mac0 = psyq_gte_mfc2(24);
                flag = psyq_gte_cfc2(31);
                memcpy(out + 8 * i, &mac0, 4);
                memcpy(out + 8 * i + 4, &flag, 4);
                trz += step;
            }
            put_hex(out, 8 * (size_t)i);
        } else if (strcmp(argv[0], "S") == 0 && argc == 4) {
            s32 a = (s32)strtoul(argv[2], NULL, 0);
            long count = strtol(argv[3], NULL, 0);
            long i;

            for (i = 0; i < count && i < (long)sizeof(out) / 2; i++) {
                s16 v = (s16)(strcmp(argv[1], "rsin") == 0 ? rsin(a) : rcos(a));

                memcpy(out + 2 * i, &v, 2);
                a = (s32)((u32)a + 1);
            }
            put_hex(out, 2 * (size_t)i);
        } else if (strcmp(argv[0], "F") == 0 && argc >= 5 && hex_bytes(argv[2], in, sizeof(in)) == 256) {
            u32 words[3][8]; /* word-aligned, as the PS1's buffers */
            u8 *b[3];
            int k;

            memset(words, 0, sizeof(words));
            for (k = 0; k < 3; k++) {
                b[k] = (u8 *)words[k];
            }
            for (k = 0; k < argc - 3 && k < 3; k++) {
                hex_bytes(argv[3 + k], b[k], sizeof(words[k]));
            }
            load_regs(in);
            if (strcmp(argv[1], "RotMatrixYXZ_gte") == 0) {
                RotMatrixYXZ_gte((SVECTOR *)b[0], (MATRIX *)b[1]);
                put_hex_regs(b[1], 32);
            } else if (strcmp(argv[1], "RotMatrixZYX_gte") == 0) {
                RotMatrixZYX_gte((SVECTOR *)b[0], (MATRIX *)b[1]);
                put_hex_regs(b[1], 32);
            } else if (strcmp(argv[1], "ScaleMatrix") == 0) {
                ScaleMatrix((MATRIX *)b[0], (VECTOR *)b[1]);
                put_hex_regs(b[0], 32);
            } else if (strcmp(argv[1], "ApplyMatrixSV") == 0 && argc == 6) {
                ApplyMatrixSV((MATRIX *)b[0], (SVECTOR *)b[1], (SVECTOR *)b[2]);
                put_hex_regs(b[2], 8);
            } else {
                printf("error unknown function %s\n", argv[1]);
            }
        } else {
            printf("error bad command %s\n", argv[0]);
        }
        fflush(stdout);
    }
    return 0;
}
