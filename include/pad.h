#ifndef PAD_H
#define PAD_H

#include "common.h"

/* One controller; pad_state has 2 ports x 4 multitap slots. */
typedef struct PadSlot {
    /* 0x00 */ u16 pressed;    /* buttons pressed this frame (pad_read_buttons) */
    /* 0x02 */ u16 repeat;     /* buttons repeating this frame */
    /* 0x04 */ u16 held_before; /* held of the previous frame */
    /* 0x06 */ u16 held;       /* buttons held (active high) */
    /* 0x08 */ u8 analog[4];   /* analog sticks (bytes 4..7 of the controller's reply) */
    /* 0x0C */ s32 repeat_times[16]; /* per button: time of the last repeat (gfx_module.funcs.get_time) */
    /* 0x4C */ u8 repeat_speed[16]; /* per button: repeat speed-up, 0, then 10..12 */
    /* 0x5C */ u8 button_map[16]; /* button remap, reset from pad_default_button_map (0..15) */
    /* 0x6C */ s16 actuator_times[2]; /* per actuator: time left (pad_set_actuator, pad_update_actuators) */
} PadSlot; /* size 0x70 */

/* pad_state: the pad module's state and its function table, which game code calls through (main
 * calls init at start-up and update every frame). Callers must go through the struct: GCC 2.8
 * assumes a struct field and a scalar global never alias, which changes instruction order. */
typedef struct PadState {
    /* 0x000 */ s32 flags;
    /* 0x004 */ u8 buffers[2][0x22]; /* PadInitMtap/PadInitDirect receive buffers */
    /* 0x048 */ PadSlot slots[2][4];
    /* 0x3C8 */ u8 actuators[2][6]; /* per port: actuator data for PadSetAct/PadSetActAlign */
    /* 0x3D4 */ s16 repeat_interval; /* button repeat interval (pad_init, default 0x10) */
    /* 0x3D6 */ s16 playback_port; /* port of the recorded input (pad_start_playback) */
    /* 0x3D8 */ u8 *playback; /* recorded input: a receive buffer (0x22 bytes) per frame (playback_frame) */
    /* 0x3DC */ s16 playback_frame; /* frame of playback; playback ends at 0x707 */
    /* 0x3E0 */ void (*init)(s32, s32);                      /* pad_init */
    /* 0x3E4 */ void (*stop)(void);                          /* pad_stop */
    /* 0x3E8 */ void (*update)();                            /* pad_update */
    /* 0x3EC */ void (*set_actuator)(s32, s32, s32, s32);    /* pad_set_actuator */
    /* 0x3F0 */ void (*lock_analog)();                       /* pad_lock_analog */
    /* 0x3F4 */ s32 (*get_pressed)(s32 port);                /* pad_get_pressed: slot 0's pressed */
    /* 0x3F8 */ s32 (*get_held)(s32 port);                   /* pad_get_held: slot 0's held */
    /* 0x3FC */ s32 (*get_repeat)(s32 port);                 /* pad_get_repeat: slot 0's repeat */
    /* 0x400 */ void (*reset_button_map)(u8 id);             /* pad_reset_button_map */
    /* 0x404 */ void (*swap_buttons)(u8 id, s32 a, s32 b);   /* pad_swap_buttons */
    /* 0x408 */ s32 (*get_button_map)(s32 port, s32 button); /* pad_get_button_map (defined returning u8; callers use the result as an int) */
    /* 0x40C */ s32 (*start_recording)(void);                /* pad_start_recording: a stub (returns 0) */
    /* 0x410 */ void (*stop_recording)(void);                /* pad_stop_recording: a stub; pad_update calls it at the
                                                           * buffer's end (0x707 frames) in the recording mode (flag
                                                           * 0x800000), which nothing enters */
    /* 0x414 */ s32 (*check_playback)(s32);                  /* pad_check_playback */
    /* 0x418 */ s32 (*start_playback)(s16, u8 *);            /* pad_start_playback */
    /* 0x41C */ void (*stop_playback)(void);                 /* pad_stop_playback */
    /* 0x420 */ s32 (*is_playback_port)(s32);                /* pad_is_playback_port */
} PadState; /* size 0x424 */

extern PadState pad_state;

/* `button` (through the button map) was pressed this frame / is repeating on port 0. The table's functions return
 * s32: callers use their u16 results unmasked. */
#define PAD_PRESSED(button) ((pad_state.get_pressed(0) >> pad_state.get_button_map(0, button)) & 1)
#define PAD_REPEAT(button) ((pad_state.get_repeat(0) >> pad_state.get_button_map(0, button)) & 1)
/* `button` is held on port 0. */
#define PAD_HELD(button) ((pad_state.get_held(0) >> pad_state.get_button_map(0, button)) & 1)

/* pad_random: the random-number module (end of pad.c): the index into the 4096-entry table
 * pad_random_table, and its two functions.
 * next: pad_random_next is defined returning u16, but the overlays use the call's result as a full word
 * (no masking), so the entry returns s32 (as message.h's decode_char for message_decode_char). */
typedef struct PadRandom {
    /* 0x0 */ s32 index;
    /* 0x4 */ void (*seed)(s32 seed);  /* pad_random_seed (main: seed 0) */
    /* 0x8 */ s32 (*next)(void);       /* pad_random_next: next random value */
} PadRandom; /* size 0xC */

extern PadRandom pad_random;

#endif /* PAD_H */
