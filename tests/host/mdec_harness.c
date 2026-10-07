/* tests/host/mdec_harness.c: the port's LIBPRESS and MDEC (port/psyq/libpress.c, mdec.c) driven line by line from
 * stdin by tests/host/mdec_replay.py, which mirrors the calls of the mdec golden family (tests/golden/mdec.json). One
 * command per line, one answer line each:
 *   V HEX_FRAME N           DecDCTvlc2 on the frame HEX_FRAME into a buffer filled with 0xA5 -> the return value and
 *                           the buffer's first N bytes (HEX)
 *   R MODE                  DecDCTReset(MODE)                                                -> ok
 *   I MODE HEX_WORDS        DecDCTin(the run-level words HEX_WORDS, MODE)                    -> the command word (HEX)
 *   O WORDS                 DecDCTout(a buffer, WORDS)                                       -> the buffer (HEX)
 * The run-level words stay in a static buffer between I and O, as the MDEC reads them during the DMAs. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psyq_internal.h"
#include "psyq/libpress.h"

#define BUF_SIZE 0x40000

u8 port_arena[PORT_ARENA_SIZE]; /* the arena the shim's window and tags refer to (include/port.h) */

void port_unimplemented(const char *fn) {
    fprintf(stderr, "unimplemented: %s\n", fn);
    exit(3);
}

static u32 frame_buf[BUF_SIZE / 4 + 16]; /* + a margin: the bit reader fetches ahead */
static u32 rl_buf[BUF_SIZE / 4];
static u32 out_buf[BUF_SIZE / 4];
static u16 vlc_table[0x11000 / 2];

static u32 unhex(const char *s, u8 *out, u32 max) {
    u32 n = 0;

    while (s[0] && s[1] && s[0] != ' ' && s[0] != '\n' && n < max) {
        unsigned v;

        sscanf(s, "%2x", &v);
        out[n++] = (u8)v;
        s += 2;
    }
    return n;
}

static void puthex(const void *p, u32 n) {
    const u8 *b = p;
    u32 i;

    for (i = 0; i < n; i++) {
        printf("%02x", b[i]);
    }
}

int main(void) {
    static char line[2 * BUF_SIZE + 64];

    psyq_press_reset();
    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (line[0] == 'V') {
            u32 n = 0;
            char *hex = line + 2, *sp = strchr(hex, ' ');
            int ret;

            if (sp == NULL || sscanf(sp + 1, "%u", &n) != 1 || n > BUF_SIZE) {
                return 2;
            }
            memset(frame_buf, 0, sizeof(frame_buf));
            unhex(hex, (u8 *)frame_buf, BUF_SIZE);
            memset(rl_buf, 0xA5, sizeof(rl_buf));
            DecDCTvlcBuild(vlc_table);
            ret = DecDCTvlc2(frame_buf, rl_buf, vlc_table);
            printf("%d ", ret);
            puthex(rl_buf, n);
            putchar('\n');
        } else if (line[0] == 'R') {
            DecDCTReset(atoi(line + 2));
            printf("ok\n");
        } else if (line[0] == 'I') {
            int mode = atoi(line + 2);
            char *hex = strchr(line + 2, ' ');

            if (hex == NULL) {
                return 2;
            }
            memset(rl_buf, 0, sizeof(rl_buf));
            unhex(hex + 1, (u8 *)rl_buf, BUF_SIZE);
            DecDCTin(rl_buf, mode);
            puthex(rl_buf, 4);
            putchar('\n');
        } else if (line[0] == 'O') {
            u32 words = (u32)atoi(line + 2);

            if (words > BUF_SIZE / 4) {
                return 2;
            }
            memset(out_buf, 0, words * 4);
            DecDCTout(out_buf, (int)words);
            puthex(out_buf, words * 4);
            putchar('\n');
        } else if (line[0] == 'Q') {
            break;
        } else {
            return 2;
        }
        fflush(stdout);
    }
    return 0;
}
