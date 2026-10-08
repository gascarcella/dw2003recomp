/* The M1 harness's interfaces (session 16): the disc, the per-frame log and the game-state probes, the input script.
 * main.c parses the options and calls these; pump.c runs the per-frame ones on every vsync. Each group is owned by
 * one file (one agent), the header by the orchestrator: change a declaration here only with every caller.
 *
 * A "frame" is one vsync tick (psyq_vsync_tick, from VSync() or from the pump's port_wait), as in the emulator's
 * replay runner (tests/replay/run.lua, which runs its listener at every GPU vsync). port_frames counts them. */
#ifndef PORT_HARNESS_H
#define PORT_HARNESS_H

#include <setjmp.h>
#include <stdio.h>

#include "psxstack/types.h"

/* ---- disc.c: LIBCD's sector source over the user's BIN/CUE (docs/PORT.md "Disc, memory cards and movies"; DECISIONS "PC port architecture") ----
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

/* The game-state probes for the script are the game adapter's (psxstack/game.h game_state_*: what run.lua reads from
 * PS1 RAM, read from the host's objects); this one is the runtime's. */
u32 port_state_slot1_word0(void);  /* the first word of the file last loaded into slot 1 (overlay.c) */

/* ---- script.c: the input script (tests/replay/scripts/<name>.json, the layer-2 format) ----
 * port_script_load: parses the script (fatal on error). port_script_frame: once per vsync: advances the steps, sets
 * the pad (psyq_pad_set), records checkpoints; ends the run with port_exit(0) when the script is done, 5 when a step
 * times out or the script's max_frames is reached. */
void port_script_load(const char *path);
void port_script_frame(void);

/* ---- memcard.c: the memory cards (LIBMCRD's backing store: raw 128 KB .mcd images; docs/PORT.md "Disc, memory cards and movies") ----
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
/* port_setjmp: setjmp, except on mingw-w64 x64, where setjmp(buf) is _setjmp(buf, <frame>) and the longjmp then
 * unwinds the frames in between through SEH; with a NULL frame it restores the registers and jumps, as glibc's does,
 * which is what the reset wants (nothing on the host stack needs unwinding: reset.c). */
#if defined(_WIN32) && defined(__x86_64__)
#define port_setjmp(buf) _setjmp((buf), NULL)
#else
#define port_setjmp(buf) setjmp(buf)
#endif

/* ---- video.c, input.c: the window and the screenshots (M2; the window only with -DDW3_PORT_SDL=ON: SDL3) ----
 * video.c converts the display area of the VRAM (psyq.h "The video output") into 32-bit pixels at the display's size;
 * port_video_frame (pump.c, once per vsync) writes the screenshots due at this frame (`--screenshot FRAME:PATH`: a
 * binary PPM; any build) and, with a window, presents the image (4:3, integer-scaled). port_video_open: the window
 * (`--scale N`: 320*N x 240*N, `--fullscreen`), fatal without SDL (port_video_available() 0) or when SDL fails.
 * port_input_frame (pump.c, once per vsync before the script's step, window mode only): SDL's events: the keyboard
 * and the gamepads to psyq_pad_set unless a script owns the pad, the window's close (port_exit 0); `test`
 * (`--input-test`) runs input.c's injection self-test. port_video_close: from port_exit (the present count).
 * port_video_quit: from port_exit after the audio's close, SDL torn down before exit() (NVIDIA's EGL on Wayland crashed
 * in an SDL_Quit inside exit(): video.c port_video_quit). */
extern int port_window; /* a window is open: pump.c paces the vsyncs to real time */
/* pump.c: the nominal rate (vsyncs per second the game is made for: 50, PAL; `--fps N`: N), which the audio's samples
 * per vsync follow, and the pace (vsyncs per second of the wall clock in window mode; 0: unthrottled; headless:
 * never paced). port_pace_set starts the schedule over when the pace changes (fast-forward and back). */
extern long port_rate;
void port_pace_set(long fps);
/* mods.c: a game mod asks for fast-forward (skip_dialogues' fast_forward_waits while a cutscene runs): on or off,
 * with fast_forward's speed and mute. */
void port_fast_forward_request(int on);
long port_pace_get(void);
int port_video_available(void);
int port_video_screenshot_add(const char *spec); /* "FRAME:PATH"; 0 when malformed (or too many) */
/* The renderer: "software" or "gpu" (--renderer, video.renderer; video.c); 0 for another name. port_video_open uses it.
 * --gpu-screenshot "FRAME[@WxH]:PATH" (SDL build): the hardware renderer's picture; 0 when malformed (or too many). */
int port_video_set_renderer(const char *name);
void port_video_set_internal_scale(int scale); /* --internal-scale / video.internal_scale: the rasteriser's, 1..8 */
int port_video_gpu_screenshot_add(const char *spec);
/* A run without a window that has --gpu-screenshot: the GPU device and the rasteriser opened now, before the game
 * draws (SDL build; logged and skipped without a device). */
void port_video_gpu_headless(void);
/* The hardware renderer's picture now (the display at its internal scale) as a PPM: the debug channel's screenshot
 * with "renderer": "gpu". 1 written, 0 the file cannot be written, -1 no GPU renderer in this run. */
int port_video_gpu_screenshot_now(const char *path, int *w, int *h);
/* The current display image (converted at this frame already, or now) to `path` as --screenshot writes it; its size
 * in *w, *h; 0 when the file cannot be written (the debug channel's screenshot). */
int port_video_screenshot_now(const char *path, int *w, int *h);
void port_video_open(int scale, int fullscreen);
void port_video_frame(void);
void port_video_toggle_fullscreen(void);
void port_video_refresh(void);          /* the last image presented again (the pause) */
void port_video_set_paused(int paused); /* the window's title says so */
void port_video_set_status(const char *status); /* "" or e.g. "fast-forward 4x": in the window's title */
void port_video_set_present_cap(int hz); /* at most hz presents a second (0: every vsync; fast-forward: 60) */
void port_video_close(void);
void port_video_quit(void);
void port_input_init(int test);
void port_input_frame(void);

/* ---- input.c: the settings' input section and the hotkey actions (docs/LAUNCHER.md "Input bindings") ----
 * port_input_settings: the pad map (the defaults, then `input.keyboard`/`input.gamepad`) and the port's own actions
 * (`input.hotkeys`: pause, fullscreen); `input` may be NULL (the defaults: the bare binary). Before port_input_init.
 * port_input_action: registers an action with its `binding` (a PortJson in the settings' binding grammar; NULL: `default_json`, the
 * same grammar as JSON text) and returns its id; a bad binding fails the settings naming `where`. pressed: a trigger
 * completed at this vsync's poll; held: a trigger is complete. Both 0 headless and during --input-test.
 * port_input_poll_paused: the events and the actions only, nothing to the pad (pump.c's pause). */
struct PortJson;
extern int port_action_pause, port_action_fullscreen;
void port_input_settings(const struct PortJson *input);
int port_input_action(const char *name, const struct PortJson *binding, const char *where, const char *default_json);
void port_input_check_binding(const struct PortJson *binding, const char *where); /* fails the settings if bad */
int port_input_pressed(int action);
int port_input_held(int action);
void port_input_poll_paused(void);

/* ---- spu_trace.c: the SPU write trace (M3; T-libsnd; tests/sound's text format, docs/SOUND.md section 4) ----
 * port_spu_trace_open: from now on every SPU register write and DMA block (spu.h's write hook) goes to `path`, with the
 * frame (vsync) as its tick, comparable with the emulator's trace (tests/sound/spu_trace.py diff). Closed by port_exit. */
void port_spu_trace_open(const char *path);
void port_spu_trace_close(void);

/* ---- audio.c: the audio output (M3; T-audio) ----
 * port_audio_open: `device` 1 opens SDL3's audio device (window mode, unless --mute), `wav_path` (any build, headless
 * too) captures the output as a 44.1 kHz stereo 16-bit WAV; either may be off. port_audio_frame (pump.c's vsync pre-hook:
 * at the start of every vsync, before the game's VSyncCallback handler; always, headless and with no device or WAV too,
 * because LIBSND's flush reads the voices' envelopes): renders that vsync's samples with spu_render (exactly SPU_RATE / 50 = 882 per PAL vsync; deterministic) into the
 * device's queue and the WAV. port_audio_close: from port_exit (finishes the WAV header). */
void port_audio_open(int device, const char *wav_path);
void port_audio_frame(void);
void port_audio_close(void);
void port_audio_pause(int paused); /* the device paused (the pump's pause); nothing rendered meanwhile */
/* The device muted (fast-forward): its queue cleared, then the samples rendered but not queued; unmuted, the queue
 * starts again from its target. The WAV and the SPU never change. */
void port_audio_set_mute(int mute);

/* ---- mods.c: the built-in mods (docs/LAUNCHER.md "Mod manifest", "Mod runtime") ----
 * port_mods_settings: the settings' `mods` section (NULL: every mod off) read against the registry; a bad value fails
 * the settings. port_mods_start: `active` 0 keeps every mod off (a --script run that did not ask for them); else the
 * enabled mods register their hotkeys (after port_input_settings). port_mods_frame: every vsync (pump.c), the enabled
 * mods' per-vsync work. port_mods_print: the resolved `mods` object for --print-settings; port_mods_print_registry:
 * the registry as JSON (--print-mods), what tests/port/settings.py compares with port/mods/<id>/mod.json. */
void port_mods_settings(const struct PortJson *mods);
void port_mods_start(int active);
void port_mods_frame(void);
void port_mods_print(FILE *f, int level);
void port_mods_print_registry(FILE *f);

/* ---- debug.c: the debug channel (`--debug SOCKET`; the protocol is the file's header comment) ----
 * port_debug_open: the Unix socket at `path` (fatal when it cannot listen); sets port_debug_active. port_debug_frame
 * (pump.c, once per vsync after the script's step, before the video): the pad the channel holds, its deferred ops
 * (step, wait, a synced pad), then the socket's requests; a reset it was asked for is requested at its end.
 * port_debug_poll_paused: the requests only (pump.c's pause loop, every poll). port_debug_resumed: after the pause
 * loop (a reset asked for while paused runs here). port_debug_close: from port_exit (the socket file unlinked).
 * port_debug_pad_owned: the channel owns the pad (a pad op until pad_free): input.c sends nothing to the pad. */
extern int port_debug_active;
extern int port_debug_pad_owned;
void port_debug_open(const char *path);
void port_debug_frame(void);
void port_debug_poll_paused(void);
void port_debug_resumed(void);
void port_debug_close(void);
/* pump.c: the pause the channel and the pause key share: `wanted` holds the game between two vsyncs (from the end of
 * the current one) until a resume; port_pump_paused is the state (1 inside the pause loop, or about to enter it). */
void port_pump_pause_request(void);
void port_pump_resume_request(void);
int port_pump_paused(void);

#endif /* PORT_HARNESS_H */
