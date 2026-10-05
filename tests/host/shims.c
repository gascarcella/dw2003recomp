/* Host-side replay: what the game's units need from the rest of the game and from Psy-Q, replaced for a Linux process.
 * Nothing here changes a result: the goldens call pure functions; these shims supply memory, files and time. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "cardgame.h"
#include "cdload.h"
#include "gfx.h"
#include "heap.h"
#include "psyq/libgpu.h"
#include "psyq/libmcrd.h"
#include "psyq/libpad.h"
#include "sound.h"

/* ---- cdload: files by ID, registered by the harness (F command) */
static struct { s32 id; u8 *data; } host_files[64];
static int host_file_count;

void host_register_file(s32 id, const char *path) {
    FILE *f = fopen(path, "rb");
    long n;
    if (!f) { fprintf(stderr, "host: cannot open %s\n", path); exit(2); }
    fseek(f, 0, SEEK_END); n = ftell(f); fseek(f, 0, SEEK_SET);
    host_files[host_file_count].id = id;
    host_files[host_file_count].data = malloc(n);
    if (fread(host_files[host_file_count].data, 1, n, f) != (size_t)n) { fprintf(stderr, "host: short read %s\n", path); exit(2); }
    fclose(f);
    host_file_count++;
}

static u8 *host_get_file(s32 id) {
    int i;
    for (i = 0; i < host_file_count; i++) if (host_files[i].id == id) return host_files[i].data;
    fprintf(stderr, "host: file %#x was not registered (F command)\n", id);
    exit(2);
}
static void host_free_file(s32 id) { (void)id; }
static void host_free_all(void) {}
static void host_free_above(void) {}
CdloadModule cdload_module = { .files = { host_get_file, host_free_file, host_free_all, host_free_above } };

/* ---- heap: libc */
static void host_bzero(void *dst, s32 size) { memset(dst, 0, size); }
static void host_memset(u8 *dst, u8 value, s32 count) { memset(dst, value, count); }
static void *host_alloc(s32 size, s32 state) { (void)state; return malloc(size); }
static void *host_alloc_zero(s32 size, s32 state) { (void)state; return calloc(1, size); }
static void host_free(void *ptr) { free(ptr); }
static void host_nop(void) {}
HeapFuncs heap_funcs = { .bzero = host_bzero, .memset = host_memset, .alloc = host_alloc, .alloc_top = host_alloc,
                         .alloc_zero = host_alloc_zero, .free = host_free, .nop = host_nop };
/* heap_objects (the object list and its search) is heap.c's own (tests/host/build.sh). */

/* ---- gfx: a frame counter; the packet pointer (gfx_get_packet/gfx_set_packet: sprite_draw writes its primitives
 * where gfx_module.packet points, a scratch buffer of the goldens) */
static s32 host_time;
static s32 host_get_time(void) { return host_time++; }
static void *host_get_packet(void) { return gfx_module.packet; }
static void host_set_packet(void *p) { gfx_module.packet = p; }
GfxModule gfx_module = { .funcs = { .get_packet = host_get_packet, .set_packet = host_set_packet, .get_time = host_get_time } };
/* Psy-Q's SetDrawTPage is libgpu's setDrawTPage macro as a function. */
void SetDrawTPage(DR_TPAGE *p, s32 dfe, s32 dtd, s32 tpage) { setDrawTPage(p, dfe, dtd, tpage); }

/* ---- main.c's screen position flag (as initialised there): the card game's board positions index tables with it */
s32 main_screen_pos = 1;

/* ---- units not built: font, tim, the overlay callbacks gamestate.c declares (CARDGAME's helpers are built) */
void font_init(Font *obj) { memset(obj, 0, sizeof(*obj)); }
void tim_init(Tim *obj) { memset(obj, 0, sizeof(*obj)); }
void func_8008B770(s32 a, s32 b, s32 c, s32 d, s32 e) { (void)a; (void)b; (void)c; (void)d; (void)e; }
void func_8008BFA4(s32 a) { (void)a; }
void func_8008C000(void) {}
void (*D_8009B6A4)(s32);

/* ---- Psy-Q pad and memory card: never reached by the goldens */
void PadInitDirect(u8 *a, u8 *b) { (void)a; (void)b; }
void PadInitMtap(u8 *a, u8 *b) { (void)a; (void)b; }
int PadStartCom(void) { return 0; }
void PadStopCom(void) {}
int PadChkVsync(void) { return 0; }
int PadGetState(int port) { (void)port; return 0; }
int PadInfoMode(int port, int term, int offs) { (void)port; (void)term; (void)offs; return 0; }
int PadInfoAct(int port, int actno, int term) { (void)port; (void)actno; (void)term; return 0; }
void PadSetAct(int port, u8 *data, int len) { (void)port; (void)data; (void)len; }
int PadSetActAlign(int port, u8 *data) { (void)port; (void)data; return 0; }
int PadSetMainMode(int socket, int offs, int lock) { (void)socket; (void)offs; (void)lock; return 0; }
void MemCardInit(long val) { (void)val; }
void MemCardStart(void) {}
s32 MemCardExist(s32 chan) { (void)chan; return 0; }
s32 MemCardAccept(s32 chan) { (void)chan; return 0; }
s32 MemCardReadFile(s32 chan, char *file, u32 *addr, s32 offset, s32 bytes) { (void)chan; (void)file; (void)addr; (void)offset; (void)bytes; return 0; }
s32 MemCardWriteFile(s32 chan, char *file, u32 *addr, s32 offset, s32 bytes) { (void)chan; (void)file; (void)addr; (void)offset; (void)bytes; return 0; }
s32 MemCardCreateFile(s32 chan, char *file, s32 blocks) { (void)chan; (void)file; (void)blocks; return 0; }
s32 MemCardFormat(s32 chan) { (void)chan; return 0; }
s32 MemCardUnformat(s32 chan) { (void)chan; return 0; }
s32 MemCardSync(s32 mode, s32 *cmds, s32 *result) { (void)mode; (void)cmds; (void)result; return 0; }
s32 MemCardGetDirentry(s32 chan, char *name, DIRENTRY *dir, s32 *files, s32 offset, s32 max) { (void)chan; (void)name; (void)dir; (void)files; (void)offset; (void)max; return 0; }

/* ---- sound: STSTATUS plays a sound after using an item or a technique (sound_module.play); nothing to hear here */
static s32 host_sound_play(s32 id) { (void)id; return 0; }
SoundModule sound_module = { .play = host_sound_play };
