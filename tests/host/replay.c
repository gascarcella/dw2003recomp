/* Host-side replay harness: the game's units compiled for this machine, driven over stdin by tests/host/replay.py.
 * One command per line; answers on stdout:
 *   F <id> <path>            register the file cdload_module.files.get_file(id) returns
 *   B <name> <hex>           a scratch buffer (freed at X)
 *   P <buf> <offset> <target>   store the host address of buffer <target> at <buf> + <offset> (a pointer field);
 *                            target @nop: the address of an empty function (a stubbed method of a UI object);
 *                            target @<symbol>: that symbol's host address (a method, e.g. @object_set_step)
 *   G <symbol> <offset> <target>   the same into a global (a pointer field of heap_objects, gfx_module, ...)
 *   W <symbol> <offset> <hex>   write bytes into a global
 *   S <symbol> <size>        save a region, restored at X
 *   X                        end of a case: restore the saved regions, free the buffers
 *   C <func> <a0> <a1> <a2> <a3>   call; an argument is i<int>, b<buffer name> or s<symbol>+<offset>; answers R <hex rax>,
 *                            or R trap when the call raised SIGFPE (an x86 division by zero the R3000A does not trap on:
 *                            the harness jumps back out of the call, so the case's reads still run; FINDINGS 7)
 *   D <symbol> <offset> <size>  read bytes (symbol, or buf:<name> for a scratch buffer); answers H <hex>
 *   L                        list the layout facts (O <name> <value>), then "."
 *   Q                        quit */
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct host_symbol { const char *name; void *addr; };
extern struct host_symbol host_symbols[];
struct host_offset { const char *name; long value; };
extern struct host_offset host_offsets[];
void host_register_file(int id, const char *path);

/* Zeroes the stack below the caller's frame before each call, so a callee's uninitialised local reads 0 whatever ran
 * before (cardgame_cpu_get_score reads list[0] uninitialised on an empty side and indexes with it: on the host, stack
 * garbage there crashed the harness once another unit was linked in). The original reads its own stack's leftovers; no
 * golden depends on them (the value is discarded). */
static __attribute__((noinline)) void scrub_stack(void) {
    volatile unsigned char pad[0x10000];
    memset((void *)pad, 0, sizeof(pad));
}

static void *sym(const char *name) {
    int i;
    for (i = 0; host_symbols[i].name; i++) if (!strcmp(host_symbols[i].name, name)) return host_symbols[i].addr;
    printf("E unknown symbol %s\n", name);
    fflush(stdout);
    exit(2);
}

static struct { char name[64]; unsigned char *data; } bufs[32];
static int nbufs;
static struct { unsigned char *addr; unsigned char *copy; long size; } saves[32];
static int nsaves;

static unsigned char *buf(const char *name) {
    int i;
    for (i = 0; i < nbufs; i++) if (!strcmp(bufs[i].name, name)) return bufs[i].data;
    printf("E unknown buffer %s\n", name);
    fflush(stdout);
    exit(2);
}

static long unhex(const char *hex, unsigned char *out) {
    long n = 0;
    for (; hex[0] && hex[1] && hex[0] != '\n'; hex += 2, n++) {
        unsigned v;
        sscanf(hex, "%2x", &v);
        out[n] = (unsigned char)v;
    }
    return n;
}

static uintptr_t arg(const char *a) {
    if (a[0] == 'i') return (uintptr_t)(intptr_t)strtoll(a + 1, 0, 0);
    if (a[0] == 'b') return (uintptr_t)buf(a + 1);
    if (a[0] == 's') {
        char name[128]; long off = 0;
        const char *plus = strchr(a, '+');
        if (plus) { memcpy(name, a + 1, plus - a - 1); name[plus - a - 1] = 0; off = strtol(plus + 1, 0, 0); }
        else strcpy(name, a + 1);
        return (uintptr_t)((unsigned char *)sym(name) + off);
    }
    printf("E bad argument %s\n", a);
    fflush(stdout);
    exit(2);
}

static void host_stub(void) {}

static sigjmp_buf trap_return;
static void on_trap(int sig) { siglongjmp(trap_return, sig); }

typedef uintptr_t (*fn4)(uintptr_t, uintptr_t, uintptr_t, uintptr_t);

int main(void) {
    static char line[1 << 20];
    static unsigned char bytes[1 << 19];
    signal(SIGFPE, on_trap);
    while (fgets(line, sizeof line, stdin)) {
        char cmd = line[0];
        if (cmd == 'Q') break;
        if (cmd == 'F') {
            int id; char path[1024];
            sscanf(line + 2, "%i %1023s", &id, path);
            host_register_file(id, path);
        } else if (cmd == 'B') {
            char name[64]; char *hex = strchr(line + 2, ' ');
            sscanf(line + 2, "%63s", name);
            long n = hex ? unhex(hex + 1, bytes) : 0;
            strcpy(bufs[nbufs].name, name);
            bufs[nbufs].data = malloc(n ? n : 1);
            memcpy(bufs[nbufs].data, bytes, n);
            nbufs++;
        } else if (cmd == 'P' || cmd == 'G') {
            char name[128], target[128]; long off;
            void *p;
            sscanf(line + 2, "%127s %li %127s", name, &off, target);
            p = !strcmp(target, "@nop") ? (void *)host_stub : target[0] == '@' ? sym(target + 1) : (void *)buf(target);
            memcpy((cmd == 'P' ? buf(name) : (unsigned char *)sym(name)) + off, &p, sizeof p);
        } else if (cmd == 'W') {
            char name[128]; long off; char *hex = line;
            int i;
            sscanf(line + 2, "%127s %li", name, &off);
            for (i = 0; i < 3; i++) hex = strchr(hex, ' ') + 1;
            long n = unhex(hex, bytes);
            memcpy((unsigned char *)sym(name) + off, bytes, n);
        } else if (cmd == 'S') {
            char name[128]; long size;
            sscanf(line + 2, "%127s %li", name, &size);
            saves[nsaves].addr = sym(name);
            saves[nsaves].size = size;
            saves[nsaves].copy = malloc(size);
            memcpy(saves[nsaves].copy, saves[nsaves].addr, size);
            nsaves++;
        } else if (cmd == 'X') {
            int i;
            for (i = nsaves - 1; i >= 0; i--) { memcpy(saves[i].addr, saves[i].copy, saves[i].size); free(saves[i].copy); }
            nsaves = 0;
            for (i = 0; i < nbufs; i++) free(bufs[i].data);
            nbufs = 0;
        } else if (cmd == 'C') {
            char fname[128], a0[160], a1[160], a2[160], a3[160];
            sscanf(line + 2, "%127s %159s %159s %159s %159s", fname, a0, a1, a2, a3);
            fn4 f = (fn4)sym(fname);
            uintptr_t x0 = arg(a0), x1 = arg(a1), x2 = arg(a2), x3 = arg(a3);
            if (sigsetjmp(trap_return, 1) == 0) {
                scrub_stack();
                uintptr_t r = f(x0, x1, x2, x3);
                printf("R %lx\n", (unsigned long)r);
            } else {
                printf("R trap\n");
            }
        } else if (cmd == 'D') {
            char name[128]; long off, size, i;
            sscanf(line + 2, "%127s %li %li", name, &off, &size);
            unsigned char *p = (strncmp(name, "buf:", 4) ? (unsigned char *)sym(name) : buf(name + 4)) + off;
            printf("H ");
            for (i = 0; i < size; i++) printf("%02x", p[i]);
            printf("\n");
        } else if (cmd == 'L') {
            int i;
            for (i = 0; host_offsets[i].name; i++) printf("O %s %ld\n", host_offsets[i].name, host_offsets[i].value);
            printf(".\n");
        } else if (cmd != '\n' && cmd != '#') {
            printf("E unknown command %c\n", cmd);
        }
        fflush(stdout);
    }
    return 0;
}
