/* The overlay manager (include/port.h OVERLAY_COPY/OVERLAY_FN/LATE_CALL; docs/PORT.md "Overlays").
 *
 * Every overlay is linked into the binary. A load (port_overlay_load, the game's two memcpy sites) makes the file the
 * tier's current overlay and puts its .data/.bss back as they were at startup, which is what the PS1's copy of the
 * file did; the tables (overlay_tables.c, generated) turn a tag (a PS1 address of a function in a slot) into the host
 * function of the tier's current overlay (port_overlay_resolve). A file that is not an overlay (no table: WSTAG260,
 * the data files FIELDSTG loads into the tier-2 slot) is copied into the slot buffer, as on the PS1. Every load keeps
 * the file's first word (port_overlay_word0: the replay scripts' wait_stage checks the slot's) and goes to the frame
 * log.
 *
 * The EXE's units (src/main/) have their .data/.bss in their own sections too (tools/port_gen.py ldscript:
 * .dw3.data.main, .dw3.bss.main), snapshotted at startup with the overlays': the console's reset (port/src/reset.c)
 * puts every game global back with port_overlay_reset, and port_overlay_check proves it (DW3_PORT_RESET_CHECK). The
 * link map check (port_gen.py sections, after every link) proves that no game object's writable data is outside
 * these sections. */
#include <stdlib.h>
#include <string.h>

#include "port_runtime.h"


/* The EXE's units' .data and .bss (the ld script's symbols, as the overlays' in overlay_tables.c). */
extern char __start_dw3_data_main[], __stop_dw3_data_main[], __start_dw3_bss_main[], __stop_dw3_bss_main[];

/* A section of the game's writable data and its contents at startup: .data's initial values, .bss's zeros (under
 * AddressSanitizer .bss also holds its ODR indicators, which ASan sets before main: the snapshot keeps them). */
typedef struct PortRegion {
    const char *name; /* for the check's log: "EXE .data", "FIELDSTG .bss", ... */
    char *start, *stop;
    u8 *snapshot;
} PortRegion;

static const PortOverlay *port_current[3]; /* per tier (index 1, 2) */
static u32 port_word0[3];                  /* per tier: the first word of the file last loaded */
/* [0] the EXE's .data, [1] its .bss, then [2 + 2 * i] overlay i's .data and [3 + 2 * i] its .bss */
static PortRegion *port_regions;
static int port_region_count;

/* Copies with plain byte accesses: under AddressSanitizer a memcpy over a whole section would trip on the redzones
 * between the globals (the bytes are untouched; copying them is harmless). */
__attribute__((no_sanitize_address)) static void port_copy_raw(void *dst, const void *src, size_t n) {
    volatile u8 *d = dst;
    const volatile u8 *s = src;
    while (n--) {
        *d++ = *s++;
    }
}

static void port_region_init(PortRegion *r, const char *name, char *start, char *stop) {
    size_t n = (size_t)(stop - start);
    if (stop < start) {
        port_fatal("overlay: %s: bad section bounds", name);
    }
    r->name = name;
    r->start = start;
    r->stop = stop;
    r->snapshot = malloc(n ? n : 1);
    if (r->snapshot == NULL) {
        port_fatal("overlay: out of memory");
    }
    port_copy_raw(r->snapshot, start, n);
}

static void port_region_restore(const PortRegion *r) {
    port_copy_raw(r->start, r->snapshot, (size_t)(r->stop - r->start));
}

void port_overlay_init(void) {
    static char names[2][16];
    size_t total = 0;
    int i;
    port_region_count = 2 + 2 * port_overlay_count;
    port_regions = calloc((size_t)port_region_count, sizeof(*port_regions));
    if (port_regions == NULL) {
        port_fatal("overlay: out of memory");
    }
    snprintf(names[0], sizeof(names[0]), "EXE .data");
    snprintf(names[1], sizeof(names[1]), "EXE .bss");
    port_region_init(&port_regions[0], names[0], __start_dw3_data_main, __stop_dw3_data_main);
    port_region_init(&port_regions[1], names[1], __start_dw3_bss_main, __stop_dw3_bss_main);
    for (i = 0; i < port_overlay_count; i++) {
        const PortOverlay *o = &port_overlays[i];
        port_region_init(&port_regions[2 + 2 * i], o->name, o->data_start, o->data_stop);
        port_region_init(&port_regions[3 + 2 * i], o->name, o->bss_start, o->bss_stop);
    }
    for (i = 0; i < port_region_count; i++) {
        total += (size_t)(port_regions[i].stop - port_regions[i].start);
    }
    if (port_trace) {
        port_log("overlay: the EXE's and %d overlays' .data/.bss snapshotted (%zu bytes)", port_overlay_count, total);
    }
}

/* The console's reset (port/src/reset.c): every game unit's .data and .bss back to their startup contents, the EXE's
 * and every overlay's (static locals included: they are in the same sections), and no overlay current in either
 * tier, as after power-on (the slots' first words are 0 again: the arena is cleared too). */
void port_overlay_reset(void) {
    int i;
    for (i = 0; i < port_region_count; i++) {
        port_region_restore(&port_regions[i]);
    }
    for (i = 0; i < 3; i++) {
        port_current[i] = NULL;
        port_word0[i] = 0;
    }
}

/* The bytes of a region that differ from its snapshot: their count; each run goes to the log as `<region>+offset` (the
 * link map, build/port/dw2003.map, names the object there), the first 8 runs of a check only. */
__attribute__((no_sanitize_address)) static size_t port_region_diff(const PortRegion *r, int is_bss, int *runs) {
    const u8 *p = (const u8 *)r->start;
    size_t n = (size_t)(r->stop - r->start), i = 0, count = 0;
    while (i < n) {
        size_t start;
        if (p[i] == r->snapshot[i]) {
            i++;
            continue;
        }
        start = i;
        while (i < n && p[i] != r->snapshot[i]) {
            i++;
        }
        count += i - start;
        if ((*runs)++ < 8) {
            port_log("overlay: check: %s%s+0x%zX: 0x%zX byte(s) differ from startup", r->name,
                     r == &port_regions[0] || r == &port_regions[1] ? "" : is_bss ? " .bss" : " .data", start,
                     i - start);
        }
    }
    return count;
}

/* The proof that a reset restored every game global (port/src/reset.c, DW3_PORT_RESET_CHECK): compares every game
 * unit's .data and .bss with their startup contents, the EXE's and every overlay's. Returns the number of bytes that
 * differ (0: all as at startup); *checked gets the number of bytes compared. */
size_t port_overlay_check(size_t *checked) {
    size_t bad = 0, total = 0;
    int runs = 0, i;
    for (i = 0; i < port_region_count; i++) {
        total += (size_t)(port_regions[i].stop - port_regions[i].start);
        bad += port_region_diff(&port_regions[i], i & 1, &runs);
    }
    if (checked != NULL) {
        *checked = total;
    }
    return bad;
}


static const PortOverlay *port_overlay_find(int tier, s32 file) {
    int i;
    for (i = 0; i < port_overlay_count; i++) {
        if (port_overlays[i].tier == tier && port_overlays[i].file == file) {
            return &port_overlays[i];
        }
    }
    return NULL;
}

const PortOverlay *port_overlay_current(int tier) {
    return (tier == 1 || tier == 2) ? port_current[tier] : NULL;
}

u32 port_overlay_word0(int tier) {
    return (tier == 1 || tier == 2) ? port_word0[tier] : 0;
}

/* The copy's CPU time on the PS1 (session 16, found by tests/port's new_game run): the game copies a file into its slot
 * with LIBC2's memcpy (0x8002514C), a byte loop of 6 instructions, about 12 cycles a byte (2 per instruction, as
 * PCSX-Redux counts them, and about what a load from main RAM costs); a PAL frame is 33,868,800 / 50 = 677,376
 * cycles. So FIELDSTG's 0x19000 bytes take 1.8 frames, and the vsync interrupt runs during the copy: the emulator's
 * replays see FIELDSTG's stage with no stage file yet (file -1) for two frames, and new_game's `new_game_field`
 * checkpoint is taken there, before FIELDSTG's start-up writes gamestate_data. The port's copy is instant, so it runs
 * the whole frames the copy would have taken (rounded down) after it, through the pump: the interrupt sees the new
 * stage, the new overlay in place and the game's state as the copy left it. Smaller copies (CNTY_SEL 0x1800,
 * STDWTITL 0x6800, a WSTAG file) take no frame. */
#define PORT_COPY_CYCLES_PER_BYTE 12
#define PORT_CYCLES_PER_FRAME 677376 /* PAL */

static void port_overlay_copy_time(u32 size) {
    unsigned long long frames = (unsigned long long)size * PORT_COPY_CYCLES_PER_BYTE / PORT_CYCLES_PER_FRAME;
    while (frames-- > 0) {
        port_wait();
    }
}

/* The source's first word (the PS1 is little-endian, as the hosts are; read byte-wise: the buffer may be unaligned). */
static u32 port_first_word(const void *src, u32 size) {
    const u8 *s = src;
    if (s == NULL || size < 4) {
        return 0;
    }
    return (u32)s[0] | (u32)s[1] << 8 | (u32)s[2] << 16 | (u32)s[3] << 24;
}

void *port_overlay_load(int tier, s32 file, void *dst, const void *src, u32 size) {
    const PortOverlay *o;
    if (tier != 1 && tier != 2) {
        port_fatal("overlay: load tier %d", tier);
    }
    o = port_overlay_find(tier, file);
    port_word0[tier] = port_first_word(src, size);
    port_framelog_overlay_load(tier, file, o != NULL ? o->name : NULL, port_word0[tier], size);
    if (o != NULL) {
        size_t data = (size_t)(o->data_stop - o->data_start), bss = (size_t)(o->bss_stop - o->bss_start);
        port_region_restore(&port_regions[2 + 2 * (o - port_overlays)]); /* .data */
        port_region_restore(&port_regions[3 + 2 * (o - port_overlays)]); /* .bss: zero (but ASan's ODR indicators) */
        port_current[tier] = o;
        port_log("overlay: tier %d, file 0x%X, %s (%d functions; .data %zu, .bss %zu bytes restored)", tier, file,
                 o->name, o->func_count, data, bss);
        port_overlay_copy_time(size);
        return dst;
    }
    /* Not code: a data file into the slot buffer, exactly the PS1's memcpy (a sector-rounded size may run past the
     * slot's end into the next region, as on the PS1; never past the arena). */
    if (!port_arena_contains(dst, size)) {
        port_fatal("overlay: tier %d, file 0x%X: %u bytes at %p are outside the arena", tier, file, size, dst);
    }
    if (src == NULL) {
        port_fatal("overlay: tier %d, file 0x%X: no source buffer", tier, file);
    }
    memcpy(dst, src, size);
    port_current[tier] = NULL;
    port_log("overlay: tier %d, file 0x%X, data (%u bytes into the slot)", tier, file, size);
    port_overlay_copy_time(size);
    return dst;
}

void (*port_overlay_resolve(int tier, uintptr_t addr))(void) {
    const PortOverlay *o;
    const PortOverlayFunc *f;
    if (addr < 0x80000000u || addr >= 0x80200000u) {
        return (PortFn)addr; /* a host function pointer already */
    }
    if (tier != 1 && tier != 2) {
        port_fatal("overlay: resolve tier %d", tier);
    }
    o = port_current[tier];
    if (o == NULL) {
        port_fatal("overlay: 0x%08X of tier %d: no overlay is loaded there", (u32)addr, tier);
    }
    for (f = o->funcs; f->fn != NULL; f++) {
        if (f->addr == addr) {
            if (port_trace) {
                port_log("overlay: 0x%08X -> %s", (u32)addr, o->name);
            }
            return f->fn;
        }
        if (f->addr > addr) {
            break; /* sorted */
        }
    }
    port_fatal("overlay: 0x%08X is not a host function of %s (tier %d): static, or still INCLUDE_ASM, or a data file",
               (u32)addr, o->name, tier);
}
