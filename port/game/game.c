/* The game adapter's entry points that are not probes or mods (psxstack/game.h; GAME_CONTRACT.md "4. The adapter
 * units"). The rest of the adapter: state.c (the probes and the checkpoint image), game_mods.c (the mods), asmdata.c
 * (the asm-only data's stand-ins), game.json (the description). */
#include "port_runtime.h"

#include "records.h"

/* The game's own 60 Hz mode (the NTSC patch's records_60hz; docs/LAUNCHER.md "50/60 Hz"): set before the runtime
 * snapshots the game's data, so that the reset restores it and the reset check holds; main_screen_pos stays 1 (the PAL
 * screen offset, which the port's video ignores, and the card game's PAL layout). */
void game_apply_rate(long rate) {
    if (rate == 60) {
        records_60hz = 1;
    }
}
