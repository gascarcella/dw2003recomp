/* The console's reset (port_harness.h): the layer-2 script's `reset` step, PCSX-Redux's hardResetEmulator in
 * tests/replay/run.lua: the RAM cleared, the machine back through the BIOS to the game's start, the memory cards and
 * the disc kept, the frame count going on.
 *
 * port_reset_request (from the script's step, inside a vsync tick, deep in the game's stack) longjmps to main(), which
 * calls port_reset_state and runs game_main() again. Nothing on the host stack needs unwinding: the game's frames hold
 * no host resources, and no "inside a tick" flag is kept anywhere (the pump, the shim's VSync and StGetNext keep none).
 * port_reset_state puts back what the PS1's power-on state is:
 *  - every game global: the EXE's and every overlay's .data and .bss back to their startup snapshot (.bss is zero
 *    there, but for AddressSanitizer's ODR indicators in a sanitizer build), static locals included, and no overlay
 *    current (port_overlay_reset; tools/port_gen.py `sections` proves at every link that no game object's writable
 *    data lies outside those sections);
 *  - the arena (the PS1's RAM above the EXE: the slots and the heap) to zero (port_arena_reset);
 *  - the Psy-Q shim: every library's state the game can observe (psyq_reset; the CD's sector source and timing, the
 *    memory cards' contents and the trace setting are kept);
 *  - the runtime: the frame log gets an `R` line and its sequences go on; the watchdog starts over (port_pump_reset).
 * The script's own state (its step, the input trace) is outside the game: it goes on.
 *
 * DW3_PORT_RESET_CHECK=1 (or --trace) proves the restore: at startup (the last setup call before game_main) and after
 * every reset, every game section is compared with its startup snapshot and the arena with zero; a difference is
 * fatal. The startup check proves that nothing between the snapshot (port_overlay_init) and game_main wrote the game's
 * data, so the snapshot is the state the first game_main started from. */
#include <stdlib.h>
#include <string.h>

#include "port_harness.h"
#include "port_runtime.h"
#include "spu.h"
#include "psyq.h"

jmp_buf port_reset_jmp;
static long port_reset_count;

void port_reset_request(void) {
    port_log("reset: frame %ld: back to the game's start", port_frames);
    longjmp(port_reset_jmp, 1);
}

void port_reset_state(void) {
    port_reset_count++;
    port_log("reset %ld: the game's data, the arena and the shim back to power-on; game_main again", port_reset_count);
    port_framelog_reset();
    port_overlay_reset();
    port_arena_reset();
    psyq_reset();
    spu_reset(); /* the SPU's registers, voices and RAM (the console's reset clears them) */
    port_pump_reset();
    port_reset_check("reset");
}

static int port_reset_check_on(void) {
    const char *env = getenv("DW3_PORT_RESET_CHECK");
    return port_trace || (env != NULL && env[0] != '\0' && strcmp(env, "0") != 0);
}

/* The game's data as at startup, and the arena zero (port/runtime/overlay.c port_overlay_check); fatal otherwise. */
void port_reset_check(const char *when) {
    const u8 *arena = port_arena_base();
    size_t checked, bad, i, arena_bad = 0;
    if (!port_reset_check_on()) {
        return;
    }
    bad = port_overlay_check(&checked);
    for (i = 0; i < PORT_ARENA_SIZE; i++) {
        arena_bad += arena[i] != 0;
    }
    if (bad != 0 || arena_bad != 0) {
        port_fatal("reset: check (%s, frame %ld): %zu byte(s) of the game's data differ from startup, %zu byte(s) of "
                   "the arena are not zero", when, port_frames, bad, arena_bad);
    }
    port_log("reset: check (%s, frame %ld): the game's data (%zu bytes of .data/.bss, as at startup) and the "
             "arena (%u bytes, zero) as at power-on", when, port_frames, checked,
             (unsigned)PORT_ARENA_SIZE);
}
