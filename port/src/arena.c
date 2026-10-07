/* The memory arena (include/port.h; docs/PORT.md "Memory arena"): one block that stands for the PS1 RAM from the tier-1
 * slot up: slot 1 (0x80082CB0), slot 2 (0x800A5DE0) and the heap (0x800AB800..) at the PS1's distances, so that a
 * pointer's PS1-style address is PORT_SLOT1_BASE + its offset. The heap is larger than the PS1's (PORT_HEAP_SIZE).
 * The block needs no alignment: an ordering-table tag's 24 bits are a pointer's offset in the arena (PTR_TO_U32,
 * port_ptr_to_u32), which the shim's DrawOTag resolves against the arena's base (port/psyq/libgpu.c). The arena is
 * smaller than 16 MB, so every offset fits a tag.
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
_Static_assert(PORT_ARENA_SIZE < (1u << 24), "the arena must stay under 16 MB: a tag's offset has 24 bits");

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

u32 port_ptr_to_u32(const void *p) {
    if (!port_arena_contains(p, 0)) {
        port_fatal("PTR_TO_U32(%p): not an arena pointer", p);
    }
    return (u32)((const u8 *)p - port_arena);
}

void *port_bios_ptr(u32 addr) {
    if (addr < BIOS_STANDIN_BASE || addr >= BIOS_STANDIN_BASE + sizeof(port_bios_standin)) {
        port_fatal("BIOS_PTR(0x%08X): outside the stand-in BIOS region (0x%08X..0x%08X)", addr, BIOS_STANDIN_BASE,
                   BIOS_STANDIN_BASE + (u32)sizeof(port_bios_standin));
    }
    return port_bios_standin + (addr - BIOS_STANDIN_BASE);
}
