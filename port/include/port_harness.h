/* The M1 harness's interfaces (session 16): the disc, the per-frame log and the game-state probes, the input script.
 * main.c parses the options and calls these; pump.c runs the per-frame ones on every vsync. Each group is owned by
 * one file (one agent), the header by the orchestrator: change a declaration here only with every caller.
 *
 * A "frame" is one vsync tick (psyq_vsync_tick, from VSync() or from the pump's port_wait), as in the emulator's
 * replay runner (tests/replay/run.lua, which runs its listener at every GPU vsync). port_frames counts them. */
#ifndef PORT_HARNESS_H
#define PORT_HARNESS_H

#include <setjmp.h>

#include "common.h"

/* ---- disc.c: LIBCD's sector source over the user's BIN/CUE (PC_PORT_PLAN 2.8; DECISIONS item 7) ----
 * port_disc_open: `path` is the .cue (its first FILE line names the BIN, relative to the cue) or the .bin itself
 * (raw 2352-byte sectors, MODE2/2352). With `check_sha1`, the whole BIN's SHA-1 must be the unpatched EU disc's
 * (457cb233..., scripts/setup.sh DISC_SHA1): anything else is fatal. Registers the reader with psyq_cd_set_reader.
 * port_disc_set_speed: "instant" or "realistic" (the default: the drive's double speed, seek time); 0 = unknown. */
void port_disc_open(const char *path, int check_sha1);
int port_disc_set_speed(const char *speed);

/* ---- framelog.c / state.c: the per-frame log, checkpoints, the record (tests/replay's shape) ----
 * port_framelog_open: either path may be NULL (no log / no record). The log is text, one line per frame and one per
 * event (overlay load, checkpoint); the record is JSON written at exit (port_framelog_close), with the checkpoints,
 * the overlay sequence and the map sequence as tests/replay/replay.py writes them. */
void port_framelog_open(const char *log_path, const char *record_path);
void port_framelog_frame(void);                 /* once per vsync, before the script's step */
void port_framelog_checkpoint(const char *name); /* a script checkpoint: hashes gamestate_data's PS1 image */
void port_framelog_close(int status, const char *reason); /* from port_exit: flushes the log, writes the record */

/* Game-state probes for the script (state.c): what run.lua reads from PS1 RAM, read from the host's objects. */
s32 port_state_stage(void);        /* overlay_module.stage */
s32 port_state_file(void);         /* overlay_module.file */
s32 port_state_map(void);          /* gamestate_data.map */
u32 port_state_slot1_word0(void);  /* the first word of the file last loaded into the tier-1 slot */
/* A `size`-byte (1, 2, 4) read at PS1 address `addr`, sign-extended if `is_signed`, into *out: 1, or 0 when the
 * address is not one the port maps (fatal to the caller). */
int port_state_read(u32 addr, int size, int is_signed, s32 *out);

/* ---- script.c: the input script (tests/replay/scripts/<name>.json, the layer-2 format) ----
 * port_script_load: parses the script (fatal on error). port_script_frame: once per vsync: advances the steps, sets
 * the pad (psyq_pad_set), records checkpoints; ends the run with port_exit(0) when the script is done, 5 when a step
 * times out or the script's max_frames is reached. */
void port_script_load(const char *path);
void port_script_frame(void);

/* ---- memcard.c: the memory cards (LIBMCRD's backing store; PC_PORT_PLAN 2.8: raw 128 KB .mcd images) ----
 * port_memcard_open: card `slot` (0 or 1) is the .mcd image at `path` (created formatted if it does not exist,
 * written back as the game writes); NULL: a fresh formatted card in memory only, as the emulator's replay runner
 * starts with (tests/replay/replay.py "Fresh (empty) memory cards per run"). Without a call a slot has no card. */
void port_memcard_open(int slot, const char *path);

/* ---- reset.c: the console's reset (the layer-2 script's `reset` step) ----
 * port_reset_request: from anywhere in the game (the script's step runs inside a vsync tick): unwinds to main()
 * (longjmp to port_reset_jmp), which calls port_reset_state() and runs game_main() again. port_reset_state: every
 * game global, the arena (the PS1's RAM: cleared) and the shim back to their power-on state; the memory cards keep
 * their contents (PCSX-Redux hardResetEmulator keeps them); the frame count goes on. */
extern jmp_buf port_reset_jmp;
void port_reset_request(void) __attribute__((noreturn));
void port_reset_state(void);

/* ---- video.c, input.c: the window and the screenshots (M2; the window only with -DDW3_PORT_SDL=ON: SDL3) ----
 * video.c converts the display area of the VRAM (psyq.h "The video output") into 32-bit pixels at the display's size;
 * port_video_frame (pump.c, once per vsync) writes the screenshots due at this frame (`--screenshot FRAME:PATH`: a
 * binary PPM; any build) and, with a window, presents the image (4:3, integer-scaled). port_video_open: the window
 * (`--scale N`: 320*N x 240*N, `--fullscreen`), fatal without SDL (port_video_available() 0) or when SDL fails.
 * port_input_frame (pump.c, once per vsync before the script's step, window mode only): SDL's events: the keyboard
 * and the gamepads to psyq_pad_set unless a script owns the pad, the window's close (port_exit 0); `test`
 * (`--input-test`) runs input.c's injection self-test. port_video_close: from port_exit (the present count). */
extern int port_window; /* a window is open: pump.c paces the vsyncs to real time */
extern long port_fps;   /* pump.c: the pace in window mode (`--fps`; 50, PAL; 0: unthrottled); headless: unthrottled */
int port_video_available(void);
int port_video_screenshot_add(const char *spec); /* "FRAME:PATH"; 0 when malformed (or too many) */
void port_video_open(int scale, int fullscreen);
void port_video_frame(void);
void port_video_toggle_fullscreen(void);
void port_video_close(void);
void port_input_init(int test);
void port_input_frame(void);

#endif /* PORT_HARNESS_H */
