/* tests/host/gpu_harness.c: the port's LIBGPU and software GPU (port/psyq/libgpu.c, gpu.c) driven line by line from
 * stdin by tests/host/gpu_replay.py, which mirrors the calls of the gpu golden family (tests/golden/gpu.json). One
 * command per line, one answer line each:
 *   R                       the console's reset (psyq_gpu_reset: VRAM zero)               -> ok
 *   L x y w h HEX           LoadImage of the pixels HEX (little-endian halfwords)         -> ok
 *   O ADDR HEX              the bytes HEX at the PS1 address ADDR in the arena, DrawOTag(ADDR) -> ok
 *   S x y w h               the VRAM rectangle (wrapping at the edges)                    -> HEX
 *   M x y w h dx dy         MoveImage                                                     -> its return value
 *   C x y w h r g b         ClearImage (C2: ClearImage2)                                  -> its return value
 *   E HEX_DRENV HEX_ENV     SetDrawEnv(DR_ENV with these bytes, DRAWENV with these bytes) -> the DR_ENV's bytes
 *   D HEX_ENV x y w h       SetDefDrawEnv(DRAWENV with these bytes, x, y, w, h)           -> the DRAWENV's bytes
 *   V HEX_DRMOVE x y w h dx dy  SetDrawMove(DR_MOVE with these bytes, RECT, dx, dy)       -> the DR_MOVE's bytes
 *   B                       BreakDraw                                                     -> 0 for NULL, else 1
 * Built with -DGPU_HW (tests/host/gpu_hw_replay.py: with port/runtime/render_gpu.c and SDL3) the hardware renderer's
 * rasteriser listens to the software GPU from the start, and:
 *   H                       its whole VRAM target against the software VRAM -> "ok", or "diff N x y SW HW" (the
 *                           count, the first differing pixel, both values), after which the target is set to the
 *                           software VRAM again (each case starts equal)
 * The ordering-table walk resolves 24-bit tags as PS1-style byte offsets in the arena (psyq_set_arena): a list at the
 * PS1 address A sits at port_arena + (A & 0xFFFFFF), so the golden's tags work unchanged (docs/PORT.md "Ordering
 * tables on 64-bit" has the port's own scheme, word offsets in the tag window). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psyq_internal.h"
#include "psyq/libgpu.h"

#ifdef GPU_HW
#include <stdarg.h>

#include "render_gpu.h"

void port_log(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fputs("harness: ", stderr);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
}

void port_fatal(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fputs("harness: fatal: ", stderr);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(1);
}
#endif

#define ARENA_SIZE PORT_ARENA_SIZE

/* The arena is the whole tag window here (include/port.h), and tags are PS1-style byte offsets in it (the goldens'
 * lists are placed as they are), which psyq_set_arena below tells the shim; the port's own tags are word offsets. */
u8 port_arena[PORT_ARENA_SIZE];
const u8 *port_tag_base = port_arena;
u32 port_tag_span = PORT_ARENA_SIZE;

u32 port_ptr_to_u32(const void *p) {
    return (u32)((const u8 *)p - port_arena);
}

void port_unimplemented(const char *fn) {
    fprintf(stderr, "unimplemented: %s\n", fn);
    exit(3);
}

static u8 *arena = port_arena;

static int unhex(const char *s, u8 *out, int max) {
    int n = 0;

    while (s[0] && s[1] && s[0] != ' ' && s[0] != '\n' && n < max) {
        unsigned v;

        sscanf(s, "%2x", &v);
        out[n++] = (u8)v;
        s += 2;
    }
    return n;
}

static void puthex(const u8 *p, int n) {
    int i;

    for (i = 0; i < n; i++) {
        printf("%02x", p[i]);
    }
    putchar('\n');
}

static const char *word(const char *s, int k) {
    while (k-- > 0) {
        s = strchr(s, ' ');
        if (s == NULL) {
            return "";
        }
        s++;
    }
    return s;
}

int main(void) {
    static char line[1 << 22];
    static u8 buf[1 << 21];

    memset(arena, 0, ARENA_SIZE);
    psyq_set_arena(arena, ARENA_SIZE);
    psyq_gpu_reset();
#ifdef GPU_HW
    {
        char why[256];
        if (!SDL_Init(SDL_INIT_VIDEO) || !render_gpu_open(NULL, why, sizeof(why)) ||
            !render_gpu_raster_start(1, why, sizeof(why))) {
            printf("nodevice %s\n", SDL_WasInit(SDL_INIT_VIDEO) ? why : SDL_GetError());
            return 2;
        }
        printf("device %s\n", render_gpu_describe());
        fflush(stdout);
    }
#endif
    while (fgets(line, sizeof(line), stdin) != NULL) {
        int x, y, w, h, a, b, c, dx, dy;
        char cmd[4];

        if (sscanf(line, "%3s", cmd) != 1) {
            continue;
        }
        if (strcmp(cmd, "R") == 0) {
            psyq_gpu_reset();
            puts("ok");
        } else if (strcmp(cmd, "L") == 0) {
            RECT r;

            sscanf(line + 2, "%d %d %d %d", &x, &y, &w, &h);
            unhex(word(line, 5), buf, sizeof(buf));
            r.x = (s16)x, r.y = (s16)y, r.w = (s16)w, r.h = (s16)h;
            LoadImage(&r, (u32 *)buf);
            puts("ok");
        } else if (strcmp(cmd, "O") == 0) {
            unsigned addr;
            int n;

            sscanf(line + 2, "%x", &addr);
            n = unhex(word(line, 2), arena + (addr & 0xFFFFFF), ARENA_SIZE - (addr & 0xFFFFFF));
            (void)n;
            DrawOTag((u32 *)(arena + (addr & 0xFFFFFF)));
            puts("ok");
        } else if (strcmp(cmd, "S") == 0) {
            const u16 *v = psyq_gpu_vram();
            int i, j, n = 0;

            sscanf(line + 2, "%d %d %d %d", &x, &y, &w, &h);
            for (j = 0; j < h; j++) {
                for (i = 0; i < w; i++) {
                    u16 p = v[((y + j) & 511) * 1024 + ((x + i) & 1023)];

                    buf[n++] = (u8)p;
                    buf[n++] = (u8)(p >> 8);
                }
            }
            puthex(buf, n);
        } else if (strcmp(cmd, "M") == 0) {
            RECT r;

            sscanf(line + 2, "%d %d %d %d %d %d", &x, &y, &w, &h, &dx, &dy);
            r.x = (s16)x, r.y = (s16)y, r.w = (s16)w, r.h = (s16)h;
            printf("%d\n", MoveImage(&r, dx, dy));
        } else if (strcmp(cmd, "C") == 0 || strcmp(cmd, "C2") == 0) {
            RECT r;

            sscanf(word(line, 1), "%d %d %d %d %d %d %d", &x, &y, &w, &h, &a, &b, &c);
            r.x = (s16)x, r.y = (s16)y, r.w = (s16)w, r.h = (s16)h;
            printf("%d\n", cmd[1] == '2' ? ClearImage2(&r, (u8)a, (u8)b, (u8)c) : ClearImage(&r, (u8)a, (u8)b, (u8)c));
        } else if (strcmp(cmd, "E") == 0) {
            DR_ENV dr;
            DRAWENV env;

            unhex(word(line, 1), (u8 *)&dr, sizeof(dr));
            unhex(word(line, 2), (u8 *)&env, sizeof(env));
            SetDrawEnv(&dr, &env);
            puthex((const u8 *)&dr, sizeof(dr));
        } else if (strcmp(cmd, "D") == 0) {
            DRAWENV env;

            unhex(word(line, 1), (u8 *)&env, sizeof(env));
            sscanf(word(line, 2), "%d %d %d %d", &x, &y, &w, &h);
            SetDefDrawEnv(&env, x, y, w, h);
            puthex((const u8 *)&env, sizeof(env));
        } else if (strcmp(cmd, "V") == 0) {
            DR_MOVE mv;
            RECT r;

            unhex(word(line, 1), (u8 *)&mv, sizeof(mv));
            sscanf(word(line, 2), "%d %d %d %d %d %d", &x, &y, &w, &h, &dx, &dy);
            r.x = (s16)x, r.y = (s16)y, r.w = (s16)w, r.h = (s16)h;
            SetDrawMove(&mv, &r, dx, dy);
            puthex((const u8 *)&mv, sizeof(mv));
        } else if (strcmp(cmd, "B") == 0) {
            printf("%d\n", BreakDraw() == NULL ? 0 : 1);
#ifdef GPU_HW
        } else if (strcmp(cmd, "H") == 0) {
            static u16 hw[1024 * 512];
            const u16 *sw = psyq_gpu_vram();
            int i, n = 0, first = -1;

            if (!render_gpu_read_vram(hw)) {
                puts("readback failed");
            } else {
                for (i = 0; i < 1024 * 512; i++) {
                    if (hw[i] != sw[i]) {
                        first = first < 0 ? i : first;
                        n++;
                    }
                }
                if (n == 0) {
                    puts("ok");
                } else {
                    printf("diff %d %d %d %04x %04x\n", n, first % 1024, first / 1024, sw[first], hw[first]);
                    render_gpu_raster_resync();
                }
            }
#endif
        } else {
            printf("? %s\n", cmd);
        }
        fflush(stdout);
    }
#ifdef GPU_HW
    render_gpu_close();
    SDL_Quit();
#endif
    return 0;
}
