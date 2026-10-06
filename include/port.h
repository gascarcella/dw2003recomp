#ifndef PORT_H
#define PORT_H

/* Hooks for the PC port (docs/PC_PORT_PLAN.md "M0"; DECISIONS "PC port decisions (session 15)").
 *
 * Every macro here has two sides:
 *  - without PC_PORT (the PS1 matching build) it expands to exactly the code the game unit had before, so no byte of
 *    the EXE or of an overlay changes;
 *  - with -DPC_PORT (the 64-bit host build) it expands to a form that uses the port_* functions and objects declared
 *    at the end of this file. Those are implemented under port/ (overlay manager, memory arena, interrupt pump).
 *
 * common.h includes this file after its typedefs: no unit includes it directly.
 *
 * Addresses: the host keeps the PS1's addresses as the *names* of things that live in the overlay slots.
 *  - tier 1 slot: 0x80082CB0 (main_overlay_base), the 19 stage overlays;
 *  - tier 2 slot: 0x800A5DE0 (main_file_base), WFIGHTMN/WFIGHTTS, the 293 WSTAG files and data files;
 *  - heap: 0x800AB800..0x801FF000 (heap.c).
 * A function in a slot is a *tag* on the host: the PS1 address as an integer in a function-pointer variable
 * (SLOT_FUNC), which is a constant expression and so works in the static tables. A tag is never called directly:
 * OVERLAY_FN turns it into the host function at the call. Data in a slot is slot-relative (SLOT_PTR). */

#define PORT_SLOT1_BASE 0x80082CB0
#define PORT_SLOT2_BASE 0x800A5DE0
#define PORT_HEAP_START_ADDR 0x800AB800
#define PORT_HEAP_END_ADDR 0x801FF000

#ifndef PC_PORT
/* ===================================================== PS1 ===================================================== */

/* --- Waits ---
 * PLATFORM_WAIT(): the body of a busy-wait that only an interrupt (vsync, CD, GPU) can end:
 *     while (gfx_frame_pending != 0) { PLATFORM_WAIT(); }
 * PS1: nothing (an empty statement). Host: port_wait() runs the pending "interrupts" (the vsync callback, the CD
 * callbacks, the GPU going idle) so that the condition can change. */
#define PLATFORM_WAIT()

/* PLATFORM_HALT(): the body of a deliberate endless loop (FIELDSTG's fieldstg_find_stage, when the map has no stage):
 *     while (1) { PLATFORM_HALT(); }
 * PS1: nothing. Host: port_halt() reports the place and stops; it does not return. */
#define PLATFORM_HALT()

/* --- The scratchpad stack (heap_run_object) ---
 * PORT_SCRATCHPAD_STACK_ENTER(top): saves $sp at `top` and sets $sp 16 bytes below it (the original's inline asm);
 * PORT_SCRATCHPAD_STACK_LEAVE(): returns to the saved stack. PS1: heap.c's SET_SCRATCHPAD_STACK/RESTORE_STACK, text
 * for text. Host: nothing, the object runs on the normal stack. */
#define PORT_SCRATCHPAD_STACK_ENTER(top)                                                           \
    __asm__ volatile("addu $8, %0, $0\n\tsw $29, 0($8)\n\taddiu $8, $8, -16\n\taddu $29, $8, $0" \
                     :                                                                             \
                     : "r"(top)                                                                    \
                     : "$8", "memory")
#define PORT_SCRATCHPAD_STACK_LEAVE() __asm__ volatile("addiu $29, $29, 16\n\tlw $29, 0($29)" : : : "memory")

/* --- Overlay loads (overlay_load_stage, overlay_load_file) ---
 * OVERLAY_COPY(tier, file, dst, src, size): copies the file `file` (its file ID; `src`, `size` bytes) into the slot
 * of `tier` (1 or 2) at `dst`. PS1: memcpy(dst, src, size) (GCC's built-in, as before). Host: port_overlay_load():
 * the overlay manager makes `file` the tier's current overlay and restores its .data/.bss to the load-time contents;
 * a file that is not code (WSTAG260, ...) is copied into the slot buffer. All five arguments are evaluated once on
 * both sides. */
#define OVERLAY_COPY(tier, file, dst, src, size) memcpy((dst), (src), (size))

/* --- Pointers and integers ---
 * PTR_ADD(type, ofs, base): an offset-table resolve, `ofs` bytes from the pointer `base`, as `type`.
 * PS1: ((type)((ofs) + (s32)(base))): the integer addition the original has, with the offset on the left.
 * Host: pointer arithmetic. */
#define PTR_ADD(type, ofs, base) ((type)((ofs) + (s32)(base)))

/* PTR_TO_S32(p) / S32_TO_PTR(type, v): a pointer kept in an s32 (a struct field, an s32 argument) and back.
 * PS1: plain casts. Host: port_ptr_to_s32()/port_s32_to_ptr(): NULL <-> 0, otherwise a pointer into the port's
 * memory arena <-> its PS1-style address (anything else is a fatal error there). Retyping the field to a pointer is
 * the better fix where it keeps the bytes. */
#define PTR_TO_S32(p) ((s32)(p))
#define S32_TO_PTR(type, v) ((type)(v))

/* PTR_TO_U32(p): the pointer's bits, for the 24-bit ordering-table tags (gfx_compact_ot's `(u32)q & mask`, libgpu.h's
 * setaddr). PS1: (u32)(p). Host: the low 32 bits; the arena is 16 MB-aligned, so the low 24 bits are the offset. */
#define PTR_TO_U32(p) ((u32)(p))

/* --- Things at fixed addresses ---
 * SLOT_FUNC(type, addr): a function of whatever overlay is in a slot, as the function-pointer type `type`.
 * PS1: ((type)(addr)). Host: a tag (see the top of the file); call it through OVERLAY_FN. Valid in static
 * initializers on both sides. */
#define SLOT_FUNC(type, addr) ((type)(addr))

/* WSTAG_ENTRY(addr): a map's entry in the tier 2 slot (fieldstg_stages, fieldstg_stages_2d).
 * OVERLAY_ENTRY(addr): a stage overlay's entry in the tier 1 slot (overlay_entries). */
#define WSTAG_ENTRY(addr) ((void *(*)())(addr))
#define OVERLAY_ENTRY(addr) ((s32 (*)(void))(addr))

/* OVERLAY_FN(tier, fn): the function to call for `fn`, a function pointer that may hold a SLOT_FUNC of `tier`:
 *     data->stage = OVERLAY_FN(2, fieldstg_stage.entry)(obj);
 * PS1: (fn). Host: port_overlay_resolve() looks a tag up in the tier's current overlay; a pointer that is not a tag
 * (a real function: the tables mix both) comes back unchanged. `fn` is evaluated once. */
#define OVERLAY_FN(tier, fn) (fn)

/* LATE_FUNC(tier, addr, ret, name, params): declares `name`, a function at `addr` of whatever overlay is in the slot
 * of `tier`, that this unit calls by name (it is resolved by the PS1 linker through undefined_syms):
 *     LATE_FUNC(1, 0x8008B770, void, func_8008B770, (s32, s32, s32, s32, s32));
 * LATE_CALL(name): the function to call:
 *     LATE_CALL(func_8008B770)(0x700, index, 0, 0, 0);
 * PS1: the plain prototype and the plain name (the same jal). Host: no symbol `name` exists; the call goes through
 * port_overlay_resolve() by `addr`. */
#define LATE_FUNC(tier, addr, ret, name, params) ret name params
#define LATE_CALL(name) name

/* SLOT_PTR(tier, type, addr): data at `addr` in the slot of `tier` (the literal 1 or 2), as the pointer type `type`:
 * the event scripts of FIELDSTG's built-in stage, main_overlay_base, main_file_base.
 * PS1: ((type)(addr)). Host: the same offset into the port's slot buffer. Valid in static initializers on both
 * sides. */
#define SLOT_PTR(tier, type, addr) ((type)(addr))

/* The heap's bounds (heap_init, D_8005CB50).
 * HEAP_START(type) / HEAP_END(type): 0x800AB800 / 0x801FF000 as the pointer type `type`. Host: the arena's heap
 * region; valid in static initializers on both sides.
 * HEAP_SIZE_FROM(first): the bytes from the pointer `first` to the heap's end. PS1: (0x801FF000 - (u32)(first)).
 * HEAP_ADDR(addr): a constant address inside the heap, passed as a pointer (FIELDSTG's free_above(0x8015C674)).
 * PS1: the integer constant, as written. Host: the same offset from the heap's start, as a u8 pointer. */
#define HEAP_START(type) ((type)0x800AB800)
#define HEAP_END(type) ((type)0x801FF000)
#define HEAP_SIZE_FROM(first) (0x801FF000 - (u32)(first))
#define HEAP_ADDR(addr) (addr)

/* BIOS_PTR(type, addr): a pointer into the BIOS ROM (STAGSLCT's version string at 0x1FC0012C).
 * PS1: ((type)(addr)). Host: port_bios_ptr(), the same offset into the port's BIOS image (or a stand-in). */
#define BIOS_PTR(type, addr) ((type)(addr))

#else /* PC_PORT */
/* ===================================================== Host ==================================================== */

#include <stdint.h>

#define PLATFORM_WAIT() port_wait()
#define PLATFORM_HALT() port_halt(__FILE__, __LINE__)

#define PORT_SCRATCHPAD_STACK_ENTER(top) ((void)0)
#define PORT_SCRATCHPAD_STACK_LEAVE() ((void)0)

#define OVERLAY_COPY(tier, file, dst, src, size) port_overlay_load((tier), (file), (dst), (src), (size))

#define PTR_ADD(type, ofs, base) ((type)((char *)(base) + (ofs)))
#define PTR_TO_S32(p) port_ptr_to_s32(p)
#define S32_TO_PTR(type, v) ((type)port_s32_to_ptr(v))
#define PTR_TO_U32(p) ((u32)(uintptr_t)(p))

#define SLOT_FUNC(type, addr) ((type)(uintptr_t)(addr))
#define WSTAG_ENTRY(addr) SLOT_FUNC(void *(*)(), addr)
#define OVERLAY_ENTRY(addr) SLOT_FUNC(s32 (*)(void), addr)
#define OVERLAY_FN(tier, fn) ((__typeof__(fn))port_overlay_resolve((tier), (uintptr_t)(fn)))

/* The enum keeps the tier and the address (less 0x80000000, to stay an int) under the function's name. */
#define LATE_FUNC(tier, addr, ret, name, params) \
    typedef ret name##_late_fn params;           \
    enum { name##_late_tier = (tier), name##_late_ofs = (int)((addr) - 0x80000000u) }
#define LATE_CALL(name) \
    ((name##_late_fn *)port_overlay_resolve(name##_late_tier, (uintptr_t)0x80000000u + name##_late_ofs))

#define SLOT_PTR(tier, type, addr) ((type)(port_slot##tier + ((addr) - PORT_SLOT##tier##_BASE)))

#define HEAP_START(type) ((type)port_heap_start)
#define HEAP_END(type) ((type)port_heap_end)
#define HEAP_SIZE_FROM(first) ((u32)(port_heap_end - (u8 *)(first)))
#define HEAP_ADDR(addr) (port_heap_start + ((addr) - PORT_HEAP_START_ADDR))

#define BIOS_PTR(type, addr) ((type)port_bios_ptr(addr))

/* --- What the port implements (port/) --- */

/* Runs the pending vsync/CD/GPU "interrupts"; may sleep until the next one is due. */
void port_wait(void);
/* Reports an endless loop the game entered on purpose and stops. */
void port_halt(const char *file, int line) __attribute__((noreturn));

/* The overlay manager. `tier` is 1 or 2.
 * port_overlay_load: `file` (a file ID) becomes the tier's current overlay, with its .data/.bss as at load time; the
 * caller has already checked that it is not the resident one. `dst`/`src`/`size` are the PS1's memcpy arguments, for
 * files that are data. Returns `dst`, as memcpy does.
 * port_overlay_resolve: the host function for `addr` in the tier's current overlay. An `addr` outside the PS1's RAM
 * (0x80000000..0x80200000) is a host function pointer already and is returned unchanged; an address the current
 * overlay does not define is a fatal error. */
void *port_overlay_load(int tier, s32 file, void *dst, const void *src, u32 size);
void (*port_overlay_resolve(int tier, uintptr_t addr))(void);

/* The memory arena (16 MB-aligned, PC_PORT_PLAN 2.4). These are link-time symbols (arrays), so that their addresses
 * are constant expressions: records.c's D_8005CB50 and FIELDSTG's script table are static initializers.
 * port_slot1/port_slot2: the buffers that stand for the two slots' data (SLOT_PTR).
 * port_heap_start/port_heap_end: the heap region's bounds (0x800AB800/0x801FF000 on the PS1; it may be larger). */
extern u8 port_slot1[];
extern u8 port_slot2[];
extern u8 port_heap_start[];
extern u8 port_heap_end[];

/* NULL <-> 0; an arena pointer <-> its PS1-style address; anything else is a fatal error. */
s32 port_ptr_to_s32(const void *p);
void *port_s32_to_ptr(s32 v);

/* The byte at the PS1 address `addr` (0x1FC00000..0x1FC80000) of the BIOS ROM. */
void *port_bios_ptr(u32 addr);

/* ---- The mods' flags (docs/LAUNCHER_MODS_PLAN.md 4.5; port/src/mods.c sets them, 0 while a mod is off or under
 * --script): read only inside `#ifdef PC_PORT` blocks of the game's C, never in an expression the PS1 build sees.
 * port_mod_skip_dialogues: skip_dialogues is on (5.2): a revealing message window shows its page at once
 * (src/main/message.c), a wait for the confirm button goes on by itself, and so does the battle's message wait
 * (src/fightstg/fightstg_8008D3B4.c). No pad press is made: choices and code-driven prompts still wait. */
extern int port_mod_skip_dialogues;

#endif /* PC_PORT */

#endif /* PORT_H */
