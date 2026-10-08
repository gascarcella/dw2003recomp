/* tests/host/libgs_harness.c: runs the libgs_view golden family's cases (tests/golden/families/libgs_view.py) through
 * the port's LIBGS and LIBGTE (psxstack/psyq/libgs.c, libgte.c, gte.c); tests/host/libgs_replay.py builds it and drives it
 * over stdin, one command per line, one answer line each (hex bytes, space-separated fields):
 *   S                                       the state after GsInitGraph(320, 240, ...), as the boot leaves it:
 *                                           -> D_80081398 (32 bytes), PSDCNT (4)
 *   V <fn> <regs> <view> <a0> <a1>          GsSetRefView2(a0) or GsGetLw(a0, a1) on the view buffer (the family's
 *                                           VIEW_LAYOUT at the PS1 address VIEW_ADDR; a0/a1 are PS1 addresses in it),
 *                                           the 64 GTE registers loaded from <regs> first ->
 *                                           GsWSMATRIX, D_80081338, the view buffer, the 64 registers, v0
 *   F <fn> <regs> <a0> <a1> <a2>            a LIBGTE function; each argument is a hex buffer, or "=N" (the same buffer
 *                                           as argument N), or "#V" (the number V) -> each distinct buffer argument
 *                                           after the call, the 64 registers, v0
 *   L <regs> <id> <light> <first>           GsSetFlatLight(id, &light), the 16-byte GsF_LIGHT given in hex; first = 1
 *                                           starts a case: both light matrices back to GsInitGraph's zero (the oracle
 *                                           restores them after each case) -> GsLIGHTWSMATRIX, D_80081318, the 64
 *                                           registers, v0
 * The view buffer is unpacked into host structs (its PS1 pointers become host pointers when they point into it) and
 * packed back in the PS1's layout, so the host's pointer size does not matter. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psyq_internal.h"
#include "psyq/libgs.h"

/* psxstack/psyq/libgs.c and libgte.c: what the game does not call (no header declares them). */
extern MATRIX GsWSMATRIX, D_80081338, D_80081398, GsLIGHTWSMATRIX, D_80081318;
extern u32 D_800812D8;
void GsGetLw(GsCOORDINATE2 *coord, MATRIX *m);
MATRIX *MulMatrix(MATRIX *m0, MATRIX *m1);
MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
VECTOR *ApplyMatrixLV(MATRIX *m, VECTOR *v0, VECTOR *v1);
MATRIX *TransposeMatrix(MATRIX *m0, MATRIX *m1);
s32 SquareRoot0(s32 a);

/* The shim's tracing, off (psyq.c is not linked). */
int psyq_trace_state = 0;
int psyq_trace_decide(void) {
    return 0;
}
void psyq_trace_printf(const char *fmt, ...) {
    (void)fmt;
}

/* tests/golden/families/libgs_view.py's layout of the view buffer. */
#define VIEW_ADDR 0x80180000u
#define COORD_OFF 0x20
#define COORD_SIZE 0x50
#define COORDS 4
#define MATRIX_OFF 0x160
#define VIEW_SIZE 0x180

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

static void put_hex(const void *p, size_t n, const char *end) {
    const u8 *b = p;
    size_t i;

    for (i = 0; i < n; i++) {
        printf("%02x", b[i]);
    }
    printf("%s", end);
}

static u32 word_at(const u8 *b, int off) {
    return b[off] | (b[off + 1] << 8) | (b[off + 2] << 16) | ((u32)b[off + 3] << 24);
}

static void load_regs(const u8 *in) {
    int i;

    for (i = 0; i < 32; i++) {
        psyq_gte_ctc2(i, word_at(in, 4 * (32 + i)));
    }
    for (i = 0; i < 32; i++) {
        if (i != 15 && i != 28 && i != 29 && i != 31) {
            psyq_gte_mtc2(i, word_at(in, 4 * i));
        }
    }
}

static void put_regs(void) {
    u8 regs[256];
    int i;

    for (i = 0; i < 64; i++) {
        u32 w = i < 32 ? psyq_gte_mfc2(i) : psyq_gte_cfc2(i - 32);

        memcpy(regs + 4 * i, &w, 4);
    }
    put_hex(regs, sizeof(regs), " ");
}

/* ---- the view buffer ---- */

static GsRVIEW2 view;
static GsCOORDINATE2 coords[COORDS];
static MATRIX view_matrix;

/* A PS1 address in the view buffer -> its host object (NULL for 0; anything else is not followed: the dummy param). */
static void *host_ptr(u32 addr) {
    int i;

    if (addr == VIEW_ADDR) {
        return &view;
    }
    for (i = 0; i < COORDS; i++) {
        if (addr == VIEW_ADDR + COORD_OFF + COORD_SIZE * i) {
            return &coords[i];
        }
    }
    if (addr == VIEW_ADDR + MATRIX_OFF) {
        return &view_matrix;
    }
    return NULL;
}

static void unpack_view(const u8 *b) {
    int i;

    memcpy(&view, b, 0x1C);
    view.super = host_ptr(word_at(b, 0x1C));
    for (i = 0; i < COORDS; i++) {
        const u8 *c = b + COORD_OFF + COORD_SIZE * i;

        memcpy(&coords[i], c, 0x44); /* flg, coord, workm */
        coords[i].param = NULL;
        coords[i].super = host_ptr(word_at(c, 0x48));
        coords[i].sub = host_ptr(word_at(c, 0x4C));
    }
    memcpy(&view_matrix, b + MATRIX_OFF, sizeof(MATRIX));
}

/* The buffer after the call: the PS1 bytes with what the functions write (flg, coord, workm, the MATRIX) put back;
 * the pointer words are the input's (nothing writes them). */
static void pack_view(u8 *b) {
    int i;

    memcpy(b, &view, 0x1C);
    for (i = 0; i < COORDS; i++) {
        memcpy(b + COORD_OFF + COORD_SIZE * i, &coords[i], 0x44);
    }
    memcpy(b + MATRIX_OFF, &view_matrix, sizeof(MATRIX));
}

int main(void) {
    static char line[1 << 16];
    static u8 bufs[3][0x200];

    psyq_gs_reset();
    GsInitGraph(320, 240, 1, 1, 0);
    while (fgets(line, sizeof(line), stdin) != NULL) {
        char *argv[8];
        int argc = 0;
        char *tok = strtok(line, " \r\n");
        u8 regs[256];

        while (tok != NULL && argc < 8) {
            argv[argc++] = tok;
            tok = strtok(NULL, " \r\n");
        }
        if (argc == 0) {
            continue;
        }
        if (strcmp(argv[0], "S") == 0) {
            put_hex(&D_80081398, sizeof(MATRIX), " ");
            put_hex(&D_800812D8, 4, "\n");
        } else if (strcmp(argv[0], "V") == 0 && argc == 6 && hex_bytes(argv[2], regs, sizeof(regs)) == 256 &&
                   hex_bytes(argv[3], bufs[0], sizeof(bufs[0])) == VIEW_SIZE) {
            u32 a0 = (u32)strtoul(argv[4], NULL, 0), a1 = (u32)strtoul(argv[5], NULL, 0);
            s32 v0 = 0;

            /* the oracle restores both matrices after each case: every case starts from the boot's (zero: the game has
             * not called GsSetRefView2 yet when the oracle runs), which GsSetRefView2's return 1 leaves in the copy */
            memset(&GsWSMATRIX, 0, sizeof(MATRIX));
            memset(&D_80081338, 0, sizeof(MATRIX));
            unpack_view(bufs[0]);
            load_regs(regs);
            if (strcmp(argv[1], "GsSetRefView2") == 0) {
                v0 = GsSetRefView2(host_ptr(a0));
            } else {
                GsGetLw(host_ptr(a0), host_ptr(a1));
            }
            pack_view(bufs[0]);
            put_hex(&GsWSMATRIX, sizeof(MATRIX), " ");
            put_hex(&D_80081338, sizeof(MATRIX), " ");
            put_hex(bufs[0], VIEW_SIZE, " ");
            put_regs();
            printf("%08x\n", (u32)v0);
        } else if (strcmp(argv[0], "F") == 0 && argc == 6 && hex_bytes(argv[2], regs, sizeof(regs)) == 256) {
            void *arg[3];
            int size[3];
            s32 num[3];
            s32 v0 = 0;
            int i;

            for (i = 0; i < 3; i++) {
                const char *a = argv[3 + i];

                size[i] = 0;
                num[i] = 0;
                arg[i] = NULL;
                if (a[0] == '=') {
                    arg[i] = arg[atoi(a + 1)];
                } else if (a[0] == '#') {
                    num[i] = (s32)strtol(a + 1, NULL, 0);
                } else {
                    size[i] = hex_bytes(a, bufs[i], sizeof(bufs[i]));
                    arg[i] = bufs[i];
                }
            }
            load_regs(regs);
            if (strcmp(argv[1], "MulMatrix") == 0) {
                MulMatrix(arg[0], arg[1]);
            } else if (strcmp(argv[1], "MulMatrix2") == 0) {
                MulMatrix2(arg[0], arg[1]);
            } else if (strcmp(argv[1], "ApplyMatrixLV") == 0) {
                ApplyMatrixLV(arg[0], arg[1], arg[2]);
            } else if (strcmp(argv[1], "TransposeMatrix") == 0) {
                TransposeMatrix(arg[0], arg[1]);
            } else if (strcmp(argv[1], "SquareRoot0") == 0) {
                v0 = SquareRoot0(num[0]);
            } else {
                printf("?\n");
                continue;
            }
            for (i = 0; i < 3; i++) {
                if (size[i] > 0) {
                    put_hex(bufs[i], (size_t)size[i], " ");
                }
            }
            put_regs();
            printf("%08x\n", (u32)v0);
        } else if (strcmp(argv[0], "L") == 0 && argc == 5 && hex_bytes(argv[1], regs, sizeof(regs)) == 256 &&
                   hex_bytes(argv[3], bufs[0], sizeof(bufs[0])) == 16) {
            GsF_LIGHT light;
            s32 v0;

            if (atoi(argv[4]) != 0) {
                memset(&GsLIGHTWSMATRIX, 0, sizeof(MATRIX));
                memset(&D_80081318, 0, sizeof(MATRIX));
            }
            memcpy(&light, bufs[0], 16); /* s32 vx, vy, vz; u8 r, g, b: the PS1's layout is the host's */
            load_regs(regs);
            v0 = GsSetFlatLight((s32)(u32)strtoul(argv[2], NULL, 0), &light);
            put_hex(&GsLIGHTWSMATRIX, sizeof(MATRIX), " ");
            put_hex(&D_80081318, sizeof(MATRIX), " ");
            put_regs();
            printf("%08x\n", (u32)v0);
        } else {
            printf("?\n");
        }
        fflush(stdout);
    }
    return 0;
}
