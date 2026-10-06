/* The memory arena (include/port.h; docs/PC_PORT_PLAN.md 2.4): one block that stands for the PS1 RAM from the tier-1
 * slot up: slot 1 (0x80082CB0), slot 2 (0x800A5DE0) and the heap (0x800AB800..) at the PS1's distances, so that a
 * pointer's PS1-style address is PORT_SLOT1_BASE + its offset. The heap is larger than the PS1's (PORT_HEAP_SIZE,
 * tools/port_gen.py). The block is 16 MB-aligned and smaller than 16 MB: the low 24 bits of any arena pointer are its
 * offset, which is what the game's 24-bit ordering-table tags keep (PTR_TO_U32).
 *
 * port_slot1, port_slot2, port_heap_start and port_heap_end are link-time symbols into port_arena, defined by the
 * generated ld script (tools/port_gen.py ldscript): the game needs their addresses as constant expressions. */
#include <stdlib.h>
#include <string.h>

#include "port_arena_gen.h"
#include "port_runtime.h"

u8 port_arena[PORT_ARENA_SIZE] __attribute__((aligned(PORT_ARENA_ALIGN)));

/* A stand-in for the BIOS ROM's version string (STAGSLCT's version display reads 0x1FC0012C). */
#define BIOS_STANDIN_BASE 0x1FC00100u
static u8 port_bios_standin[0x100] = { [0x2C] = 'P', 'C', '-', 'P', 'O', 'R', 'T', '-', 'M', '1', 0 };

void port_arena_init(void) {
    /* volatile: the compiler may otherwise fold a comparison of two distinct objects' addresses (they alias here) */
    volatile uintptr_t base = (uintptr_t)port_arena, s1 = (uintptr_t)port_slot1, s2 = (uintptr_t)port_slot2,
                       h0 = (uintptr_t)port_heap_start, h1 = (uintptr_t)port_heap_end;
    if (base & (PORT_ARENA_ALIGN - 1)) {
        port_fatal("arena: port_arena at %p is not %u MB-aligned (a PIE or a non-ELF loader?)", (void *)port_arena,
                   PORT_ARENA_ALIGN >> 20);
    }
    if (s1 != base || s2 != base + PORT_SLOT1_SIZE || h0 != base + PORT_SLOT1_SIZE + PORT_SLOT2_SIZE ||
        h1 != base + PORT_ARENA_SIZE) {
        port_fatal("arena: the ld script's symbols do not match port_arena_gen.h");
    }
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

void *port_bios_ptr(u32 addr) {
    if (addr < BIOS_STANDIN_BASE || addr >= BIOS_STANDIN_BASE + sizeof(port_bios_standin)) {
        port_fatal("BIOS_PTR(0x%08X): outside the stand-in BIOS region (0x%08X..0x%08X)", addr, BIOS_STANDIN_BASE,
                   BIOS_STANDIN_BASE + (u32)sizeof(port_bios_standin));
    }
    return port_bios_standin + (addr - BIOS_STANDIN_BASE);
}
