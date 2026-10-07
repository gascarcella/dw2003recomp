/* The memory arena (include/port.h; docs/PORT.md "Memory arena"): one block that stands for the PS1 RAM from the tier-1
 * slot up: slot 1 (0x80082CB0), slot 2 (0x800A5DE0) and the heap (0x800AB800..) at the PS1's distances, so that a
 * pointer's PS1-style address is PORT_SLOT1_BASE + its offset. The heap is larger than the PS1's (PORT_HEAP_SIZE).
 * The block needs no alignment: an ordering-table tag's 24 bits are a pointer's word offset in the tag window
 * (PTR_TO_U32, port_ptr_to_u32: the game's static data and the arena, one image's writable memory, measured at
 * startup; words, so that a sanitizer build's redzones (22 MB of game data) fit), which the shim's DrawOTag resolves
 * against the window's base (port/psyq/libgpu.c).
 *
 * port_slot1, port_slot2, port_heap_start and port_heap_end are macros on port_arena (include/port.h): the game needs
 * their addresses as constant expressions. tools/port_gen.py keeps the same numbers (port_arena_gen.h); the asserts
 * below tie the two. */
#include <stdlib.h>
#include <string.h>

#include "port_arena_gen.h"
#include "port_runtime.h"

_Static_assert(PORT_SLOT2_OFS == PORT_SLOT1_SIZE, "port_gen.py's slot 1 size differs from include/port.h's");
_Static_assert(PORT_HEAP_OFS == PORT_SLOT1_SIZE + PORT_SLOT2_SIZE, "port_gen.py's slot 2 size differs from include/port.h's");
_Static_assert(PORT_HEAP_SIZE == PORT_HEAP_GEN_SIZE, "port_gen.py's heap size differs from include/port.h's");
_Static_assert(PORT_ARENA_SIZE == PORT_ARENA_GEN_SIZE, "port_gen.py's arena size differs from include/port.h's");
_Static_assert(PORT_ARENA_SIZE < ((u32)0xFFFFFF << PORT_TAG_SHIFT), "the arena must fit the tag window: a 24-bit tag of words");

u8 port_arena[PORT_ARENA_SIZE] __attribute__((aligned(4096)));

/* A stand-in for the BIOS ROM's version string (STAGSLCT's version display reads 0x1FC0012C). */
#define BIOS_STANDIN_BASE 0x1FC00100u
static u8 port_bios_standin[0x100] = { [0x2C] = 'P', 'C', '-', 'P', 'O', 'R', 'T', '-', 'M', '1', 0 };

void port_arena_init(void) {
    if (port_trace) {
        port_log("arena: %p, %u KB (slot1 %#x, slot2 %#x, heap %u KB)", (void *)port_arena, PORT_ARENA_SIZE >> 10,
                 PORT_SLOT1_SIZE, PORT_SLOT2_SIZE, PORT_HEAP_SIZE >> 10);
    }
}

/* The console's reset (port/src/reset.c): the PS1's RAM is cleared (PCSX-Redux hardResetEmulator), so are the slots
 * and the heap; as at startup (.bss). The stand-in BIOS is ROM: it stays. */
void port_arena_reset(void) {
    memset(port_arena, 0, sizeof(port_arena));
}

void *port_arena_base(void) {
    return port_arena;
}

int port_arena_contains(const void *p, size_t size) {
    const u8 *q = p;
    return q >= port_arena && size <= PORT_ARENA_SIZE && q + size <= port_arena + PORT_ARENA_SIZE;
}

s32 port_ptr_to_s32(const void *p) {
    if (p == NULL) {
        return 0;
    }
    if (!port_arena_contains(p, 0)) {
        port_fatal("PTR_TO_S32(%p): not an arena pointer", p);
    }
    return (s32)(PORT_SLOT1_BASE + (u32)((const u8 *)p - port_arena));
}

void *port_s32_to_ptr(s32 v) {
    u32 addr = (u32)v;
    if (v == 0) {
        return NULL;
    }
    if (addr < PORT_SLOT1_BASE || addr - PORT_SLOT1_BASE >= PORT_ARENA_SIZE) {
        port_fatal("S32_TO_PTR(0x%08X): not an arena address", addr);
    }
    return port_arena + (addr - PORT_SLOT1_BASE);
}

/* The tag window (include/port.h): the game's static data and the arena, measured by port_overlay_init. */
const u8 *port_tag_base;
u32 port_tag_span;

void port_tag_window_set(const void *lo, const void *hi) {
    const u8 *a = lo, *b = hi;
    /* a tag is a word offset: 0xFFFFFF is the list terminator, so the window holds fewer words than that */
    if (b < a || (uintptr_t)(b - a) >= (uintptr_t)0xFFFFFF << PORT_TAG_SHIFT) {
        port_fatal("tag window: %p..%p does not fit a 24-bit ordering-table tag of words (the game's data and the "
                   "arena must lie within 64 MB of each other)", lo, hi);
    }
    port_tag_base = a;
    port_tag_span = (u32)(b - a);
    if (port_trace) {
        port_log("tag window: %p, %u KB", lo, port_tag_span >> 10);
    }
}

u32 port_ptr_to_u32(const void *p) {
    const u8 *q = p;
    if (port_tag_base == NULL || q < port_tag_base || q >= port_tag_base + port_tag_span) {
        port_fatal("PTR_TO_U32(%p): not in the tag window (%p, %u KB)", p, (const void *)port_tag_base,
                   port_tag_span >> 10);
    }
    if (((uintptr_t)q & ((1u << PORT_TAG_SHIFT) - 1)) != 0) {
        port_fatal("PTR_TO_U32(%p): not word-aligned (an ordering-table entry or a primitive always is)", p);
    }
    return (u32)((uintptr_t)(q - port_tag_base) >> PORT_TAG_SHIFT);
}

void *port_bios_ptr(u32 addr) {
    if (addr < BIOS_STANDIN_BASE || addr >= BIOS_STANDIN_BASE + sizeof(port_bios_standin)) {
        port_fatal("BIOS_PTR(0x%08X): outside the stand-in BIOS region (0x%08X..0x%08X)", addr, BIOS_STANDIN_BASE,
                   BIOS_STANDIN_BASE + (u32)sizeof(port_bios_standin));
    }
    return port_bios_standin + (addr - BIOS_STANDIN_BASE);
}
