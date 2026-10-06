/* The overlay manager (include/port.h OVERLAY_COPY/OVERLAY_FN/LATE_CALL; docs/PC_PORT_PLAN.md 2.5).
 *
 * Every overlay is linked into the binary. A load (port_overlay_load, the game's two memcpy sites) makes the file the
 * tier's current overlay and puts its .data/.bss back as they were at startup, which is what the PS1's copy of the
 * file did; the tables (overlay_tables.c, generated) turn a tag (a PS1 address of a function in a slot) into the host
 * function of the tier's current overlay (port_overlay_resolve). A file that is not an overlay (no table: WSTAG260,
 * the data files FIELDSTG loads into the tier-2 slot) is copied into the slot buffer, as on the PS1. */
#include <stdlib.h>
#include <string.h>

#include "port_runtime.h"

static const PortOverlay *port_current[3]; /* per tier (index 1, 2) */
static u8 **port_snapshots;                /* per overlay: its .data at startup (.bss was zero) */

/* Copies with plain byte accesses: under AddressSanitizer a memcpy over a whole section would trip on the redzones
 * between the globals (the bytes are untouched; copying them is harmless). */
__attribute__((no_sanitize_address)) static void port_copy_raw(void *dst, const void *src, size_t n) {
    volatile u8 *d = dst;
    const volatile u8 *s = src;
    while (n--) {
        *d++ = *s++;
    }
}

__attribute__((no_sanitize_address)) static void port_zero_raw(void *dst, size_t n) {
    volatile u8 *d = dst;
    while (n--) {
        *d++ = 0;
    }
}

void port_overlay_init(void) {
    int i;
    port_snapshots = calloc((size_t)port_overlay_count, sizeof(*port_snapshots));
    if (port_snapshots == NULL) {
        port_fatal("overlay: out of memory");
    }
    for (i = 0; i < port_overlay_count; i++) {
        const PortOverlay *o = &port_overlays[i];
        size_t n = (size_t)(o->data_stop - o->data_start);
        if (o->data_stop < o->data_start || o->bss_stop < o->bss_start) {
            port_fatal("overlay %s: bad section bounds", o->name);
        }
        port_snapshots[i] = malloc(n ? n : 1);
        if (port_snapshots[i] == NULL) {
            port_fatal("overlay: out of memory");
        }
        port_copy_raw(port_snapshots[i], o->data_start, n);
    }
    if (port_trace) {
        port_log("overlay: %d overlays snapshotted", port_overlay_count);
    }
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

void *port_overlay_load(int tier, s32 file, void *dst, const void *src, u32 size) {
    const PortOverlay *o;
    if (tier != 1 && tier != 2) {
        port_fatal("overlay: load tier %d", tier);
    }
    o = port_overlay_find(tier, file);
    if (o != NULL) {
        size_t data = (size_t)(o->data_stop - o->data_start), bss = (size_t)(o->bss_stop - o->bss_start);
        port_copy_raw(o->data_start, port_snapshots[o - port_overlays], data);
        port_zero_raw(o->bss_start, bss);
        port_current[tier] = o;
        port_log("overlay: tier %d, file 0x%X, %s (%d functions; .data %zu, .bss %zu bytes restored)", tier, file,
                 o->name, o->func_count, data, bss);
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
