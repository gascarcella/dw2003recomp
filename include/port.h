#ifndef PORT_H
#define PORT_H

/* Hooks for the PC port (docs/PORT.md "Hook macros (`include/port.h`)"; DECISIONS "PC port architecture"; the
 * contract with the port stack: psxstack's GAME_CONTRACT.md "2. The hook header").
 *
 * Every macro here has two sides:
 *  - without PC_PORT (the PS1 matching build) it expands to exactly the code the game unit had before, so no byte of
 *    the EXE or of an overlay changes; this side is all here, and needs nothing from the port;
 *  - with -DPC_PORT (the 64-bit host build) the generic macros come from the stack's port/include/psxstack/hooks.h
 *    (included below), which expands them to forms that use the port_* functions and objects it declares, implemented
 *    under port/runtime/ (overlay manager, memory arena, interrupt pump). This file adds the game's own hooks: the
 *    typed slot-function macros (WSTAG_ENTRY, OVERLAY_ENTRY) and the mods' flags.
 *
 * common.h includes this file after its typedefs: no unit includes it directly.
 *
 * Addresses (port/game/game.json, the game's description): the host keeps the PS1's addresses as the *names* of things
 * that live in the overlay slots.
 *  - slot 1 (tier 1): 0x80082CB0 (main_overlay_base), the 19 stage overlays;
 *  - slot 2 (tier 2): 0x800A5DE0 (main_file_base), WFIGHTMN/WFIGHTTS, the 293 WSTAG files and data files;
 *  - heap: 0x800AB800..0x801FF000 (heap.c).
 * A function in a slot is a *tag* on the host: the PS1 address as an integer in a function-pointer variable
 * (SLOT_FUNC), which is a constant expression and so works in the static tables. A tag is never called directly:
 * OVERLAY_FN turns it into the host function at the call. Data in a slot is slot-relative (SLOT_PTR). */

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
 * setaddr). PS1: (u32)(p). Host: the pointer's word offset in the tag window (port_ptr_to_u32: the game's static data
 * and the arena), which the shim's DrawOTag resolves against the window's base; anything else is a fatal error. */
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

#include "psxstack/hooks.h" /* the generic macros and the port_* interface (the stack's) */

/* The game's typed slot functions: a map's entry in slot 2 (fieldstg_stages, fieldstg_stages_2d), a stage overlay's
 * entry in slot 1 (overlay_entries). */
#define WSTAG_ENTRY(addr) SLOT_FUNC(void *(*)(), addr)
#define OVERLAY_ENTRY(addr) SLOT_FUNC(s32 (*)(void), addr)

/* ---- The mods' flags (docs/LAUNCHER.md "Mod runtime"; port/game/game_mods.c sets them, 0 while a mod is off or under
 * --script): read only inside `#ifdef PC_PORT` blocks of the game's C, never in an expression the PS1 build sees.
 * port_mod_skip_dialogues: skip_dialogues is on (docs/LAUNCHER.md "Skip dialogues"): a revealing message window shows its page at once
 * (src/main/message.c), a wait for the confirm button goes on by itself, and so does the battle's message wait
 * (src/fightstg/fightstg_8008D3B4.c). No pad press is made: choices and code-driven prompts still wait. */
extern int port_mod_skip_dialogues;
/* port_mod_battle_animations: battle_animations is on ("Disable battle animations"): fightstg_script_update asks port_battle_cut, at a script's
 * INIT, what script 5 or above becomes: -1 run it as it is, 0 end it at once, 1..4 the target's reaction (the script
 * its child command would have started, results[3] + 1); *sound_id and *sound_arg: the hit sound it would have played
 * (0: none), for fightstg_sound_play. The rules ran before the script started. */
extern int port_mod_battle_animations;
s32 port_battle_cut(const s16 *stream, s32 stage, const s32 *results, s32 hit_sound, s32 *sound_id, s32 *sound_arg);
/* port_mod_global_save: global_save is on ("Save anywhere"): the field menu (src/main/fieldmenu.c) gets a last entry, SAVE (the
 * save screen's title, ?SMEMCRD entry 1), in the field's menu only; chosen, port_global_save_open sends the game to
 * STGMCARD in save mode (the inn's 0xC map for an inn's map, 0xC00 elsewhere). STGMCARD then asks
 * port_global_save_map_name for the slot summary's place name (0, none, off an inn), port_global_save_record puts the
 * map's state that a load loses (the attribute layer, depth and height, the per-visit flags) into the slot's unused tail
 * (slot offset 0x26C4, outside the checksum: a PS1 ignores it), and port_global_save_restore reads it back after a load
 * so FIELDSTG resumes the map as after a Back from the save screen (map_is_new 0). */
extern int port_mod_global_save;
void port_global_save_open(void);
s32 port_global_save_map_name(s32 map_name);
void port_global_save_record(void *slot);
void port_global_save_restore(const void *slot);
/* port_mod_preset_language: preset_language is on ("Preset language"): CNTY_SEL's root object (src/cnty_sel), once the
 * screen's sound bank is in, sets records_language to port_preset_language (0 JPN, 1 USA, 2-6 Europe) and goes on to
 * the opening as the menu would, without drawing the screen. */
extern int port_mod_preset_language;
extern s32 port_preset_language;
/* port_mod_party_xp: party_xp is on ("Party experience"): at the end of every battle WFIGHTMN tells
 * port_party_xp_knocked_out which party slots are at 0 HP (before it clears their took_part), and STFGTREP's report,
 * once it has created the members' panels, sets each panel's experience to port_party_xp_share(slot, took_part, exp)
 * (`exp`: one fighter's split share; the result is before item 0x141's fifth, which get_exp then adds). */
extern int port_mod_party_xp;
void port_party_xp_knocked_out(s32 slot, s32 knocked_out);
s32 port_party_xp_share(s32 slot, s32 took_part, s32 exp);
/* port_mod_xp_boost: xp_boost is on ("XP boost"): STFGTREP's report passes each of a won battle's rewards through
 * port_xp_boost: the experience once it is split among the fighters (before party_xp's shares and item 0x141's
 * fifth), a form's experience once stfgtrep_get_technique_exp has capped it, and the money with item 0x142's fifth. */
extern int port_mod_xp_boost;
enum { PORT_XP_BOOST_EXP, PORT_XP_BOOST_FORM_EXP, PORT_XP_BOOST_BITS };
s32 port_xp_boost(s32 kind, s32 amount);

#endif /* PC_PORT */

#endif /* PORT_H */
