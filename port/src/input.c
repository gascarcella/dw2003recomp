/* The window's input (M2; PC_PORT_PLAN "LIBPAD over SDL3"): the keyboard and every SDL gamepad, ORed into pad 1 (a
 * digital pad: SELECT, START, the D-pad, L1/L2/R1/R2 and the four face buttons; psyq_pad_set, psyq.h's bit order),
 * once per vsync from pump.c. Only in a build configured with -DDW3_PORT_SDL=ON (DW3_PORT_SDL); the headless build has
 * no input but the script's.
 *
 * Default keys (scancodes: the key's place, whatever the layout):
 *   arrows             D-pad              X  cross      C  circle     Z  square     S  triangle
 *   Enter, keypad Enter START             Backspace, right Shift  SELECT
 *   Q  L1      E  R1      1  L2      3  R2           F11  fullscreen on/off
 * Gamepads (SDL's positional names, the PlayStation layout): south cross, east circle, west square, north triangle,
 * back SELECT, start START, the shoulders L1/R1, the triggers L2/R2 (past half way), the D-pad and the left stick
 * (past half way) the D-pad.
 *
 * With `--script` the script owns the pad: the keyboard and the gamepads are read but never reach psyq_pad_set, so a
 * replay in a window runs exactly as it does headless. Without one, every change of the pad goes to the frame log's
 * `I` lines and the record's `inputs` (port_framelog_input), as the script's do. Closing the window ends the run:
 * port_exit(0, "window closed").
 *
 * `--input-test` (a self-test of the mapping, for CI and after a change here): from frame 2 on, it injects the key
 * events of every entry of the key map with SDL_PushEvent (a press in one frame, the release in the next), then
 * attaches a virtual SDL gamepad and does the same with each of its buttons, triggers and stick directions, and a
 * chord of each; after each step's poll the buttons sent to psyq_pad_set must be the entry's bits (with a script:
 * nothing sent at all). It ends the run with status 0 when every check passed, 6 when one failed; with a script it
 * only logs the result and the script runs on, so its log can be compared with a run without the test. */
#include <string.h>

#include "port_harness.h"
#include "port_runtime.h"
#include "psyq.h"

#ifdef DW3_PORT_SDL
#include <SDL3/SDL.h>

/* The PS1 pad's bits (psyq.h psyq_pad_set; active high). */
#define PAD_SELECT (1u << 0)
#define PAD_START (1u << 3)
#define PAD_UP (1u << 4)
#define PAD_RIGHT (1u << 5)
#define PAD_DOWN (1u << 6)
#define PAD_LEFT (1u << 7)
#define PAD_L2 (1u << 8)
#define PAD_R2 (1u << 9)
#define PAD_L1 (1u << 10)
#define PAD_R1 (1u << 11)
#define PAD_TRIANGLE (1u << 12)
#define PAD_CIRCLE (1u << 13)
#define PAD_CROSS (1u << 14)
#define PAD_SQUARE (1u << 15)

#define INPUT_AXIS_PRESSED 16384 /* half way: a trigger or a stick direction counts as pressed beyond it */
#define INPUT_MAX_GAMEPADS 8

static const struct {
    SDL_Scancode key;
    u16 bits;
} input_keymap[] = {
    { SDL_SCANCODE_UP, PAD_UP },         { SDL_SCANCODE_RIGHT, PAD_RIGHT },     { SDL_SCANCODE_DOWN, PAD_DOWN },
    { SDL_SCANCODE_LEFT, PAD_LEFT },     { SDL_SCANCODE_X, PAD_CROSS },         { SDL_SCANCODE_C, PAD_CIRCLE },
    { SDL_SCANCODE_Z, PAD_SQUARE },      { SDL_SCANCODE_S, PAD_TRIANGLE },      { SDL_SCANCODE_RETURN, PAD_START },
    { SDL_SCANCODE_KP_ENTER, PAD_START }, { SDL_SCANCODE_BACKSPACE, PAD_SELECT }, { SDL_SCANCODE_RSHIFT, PAD_SELECT },
    { SDL_SCANCODE_Q, PAD_L1 },          { SDL_SCANCODE_E, PAD_R1 },            { SDL_SCANCODE_1, PAD_L2 },
    { SDL_SCANCODE_3, PAD_R2 },
};
#define INPUT_KEYS ((int)(sizeof(input_keymap) / sizeof(input_keymap[0])))

static const struct {
    SDL_GamepadButton button;
    u16 bits;
} input_padmap[] = {
    { SDL_GAMEPAD_BUTTON_SOUTH, PAD_CROSS },        { SDL_GAMEPAD_BUTTON_EAST, PAD_CIRCLE },
    { SDL_GAMEPAD_BUTTON_WEST, PAD_SQUARE },        { SDL_GAMEPAD_BUTTON_NORTH, PAD_TRIANGLE },
    { SDL_GAMEPAD_BUTTON_BACK, PAD_SELECT },        { SDL_GAMEPAD_BUTTON_START, PAD_START },
    { SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, PAD_L1 },   { SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, PAD_R1 },
    { SDL_GAMEPAD_BUTTON_DPAD_UP, PAD_UP },         { SDL_GAMEPAD_BUTTON_DPAD_DOWN, PAD_DOWN },
    { SDL_GAMEPAD_BUTTON_DPAD_LEFT, PAD_LEFT },     { SDL_GAMEPAD_BUTTON_DPAD_RIGHT, PAD_RIGHT },
};
#define INPUT_PADS ((int)(sizeof(input_padmap) / sizeof(input_padmap[0])))

static int input_down[INPUT_KEYS];                   /* the key map's entries held (from the key events) */
static SDL_Gamepad *input_gamepads[INPUT_MAX_GAMEPADS];
static u16 input_applied;                            /* the buttons last sent to psyq_pad_set */
static int input_applied_any;                        /* psyq_pad_set was called by this file */

/* ---- --input-test */
static int input_test;
static int input_test_step;  /* the current step (TEST_STEPS: done) */
static int input_test_phase; /* 0: press now, 1: release now */
static int input_test_checks, input_test_failures;
static long input_test_wait;          /* frames spent waiting for the virtual gamepad */
static SDL_JoystickID input_test_pad; /* the virtual gamepad (0: not attached) */
static SDL_Joystick *input_test_joy;
/* The axis steps: an axis set to a value (released: the rest value, -32768 for a trigger, else 0). */
enum { TEST_AXIS_STEPS = 6 };
static const struct {
    SDL_GamepadAxis axis;
    Sint16 value;
    u16 bits;
} input_test_axes[TEST_AXIS_STEPS] = {
    { SDL_GAMEPAD_AXIS_LEFT_TRIGGER, 32767, PAD_L2 }, { SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 32767, PAD_R2 },
    { SDL_GAMEPAD_AXIS_LEFTX, -32768, PAD_LEFT },     { SDL_GAMEPAD_AXIS_LEFTX, 32767, PAD_RIGHT },
    { SDL_GAMEPAD_AXIS_LEFTY, -32768, PAD_UP },       { SDL_GAMEPAD_AXIS_LEFTY, 32767, PAD_DOWN },
};
/* The steps: the key map's entries, the gamepad's buttons, its axes, then one chord (two keys and a button at once). */
#define TEST_STEPS (INPUT_KEYS + INPUT_PADS + TEST_AXIS_STEPS + 1)

static int input_key_index(SDL_Scancode key) {
    int i;
    for (i = 0; i < INPUT_KEYS; i++) {
        if (input_keymap[i].key == key) {
            return i;
        }
    }
    return -1;
}

static void input_gamepad_added(SDL_JoystickID id) {
    int i;
    for (i = 0; i < INPUT_MAX_GAMEPADS; i++) {
        if (input_gamepads[i] == NULL) {
            input_gamepads[i] = SDL_OpenGamepad(id);
            if (input_gamepads[i] != NULL) {
                port_log("input: gamepad %s", SDL_GetGamepadName(input_gamepads[i]));
            }
            return;
        }
    }
}

static void input_gamepad_removed(SDL_JoystickID id) {
    int i;
    for (i = 0; i < INPUT_MAX_GAMEPADS; i++) {
        if (input_gamepads[i] != NULL && SDL_GetGamepadID(input_gamepads[i]) == id) {
            SDL_CloseGamepad(input_gamepads[i]);
            input_gamepads[i] = NULL;
        }
    }
}

static int input_gamepad_open(SDL_JoystickID id) {
    int i;
    for (i = 0; i < INPUT_MAX_GAMEPADS; i++) {
        if (input_gamepads[i] != NULL && SDL_GetGamepadID(input_gamepads[i]) == id) {
            return 1;
        }
    }
    return 0;
}

static u16 input_buttons(void) {
    u16 bits = 0;
    int i, j;
    for (i = 0; i < INPUT_KEYS; i++) {
        if (input_down[i]) {
            bits |= input_keymap[i].bits;
        }
    }
    for (i = 0; i < INPUT_MAX_GAMEPADS; i++) {
        SDL_Gamepad *g = input_gamepads[i];
        Sint16 x, y;
        if (g == NULL) {
            continue;
        }
        for (j = 0; j < INPUT_PADS; j++) {
            if (SDL_GetGamepadButton(g, input_padmap[j].button)) {
                bits |= input_padmap[j].bits;
            }
        }
        bits |= SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > INPUT_AXIS_PRESSED ? PAD_L2 : 0;
        bits |= SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > INPUT_AXIS_PRESSED ? PAD_R2 : 0;
        x = SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_LEFTX);
        y = SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_LEFTY);
        bits |= x < -INPUT_AXIS_PRESSED ? PAD_LEFT : x > INPUT_AXIS_PRESSED ? PAD_RIGHT : 0;
        bits |= y < -INPUT_AXIS_PRESSED ? PAD_UP : y > INPUT_AXIS_PRESSED ? PAD_DOWN : 0;
    }
    return bits;
}

static void input_push_key(SDL_Scancode key, int down) {
    SDL_Event e;
    memset(&e, 0, sizeof(e));
    e.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    e.key.scancode = key;
    e.key.key = SDL_GetKeyFromScancode(key, SDL_KMOD_NONE, false);
    e.key.down = down != 0;
    if (!SDL_PushEvent(&e)) {
        port_fatal("input test: SDL_PushEvent: %s", SDL_GetError());
    }
}

/* The virtual gamepad: 15 buttons in SDL_GamepadButton order (south .. D-pad right) and 6 axes in SDL_GamepadAxis
 * order (SDL's virtual driver maps them one to one when the masks are 0). */
static void input_test_attach(void) {
    SDL_VirtualJoystickDesc desc;
    SDL_INIT_INTERFACE(&desc);
    desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
    desc.nbuttons = SDL_GAMEPAD_BUTTON_DPAD_RIGHT + 1;
    desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
    desc.name = "dw2003 input test";
    input_test_pad = SDL_AttachVirtualJoystick(&desc);
    if (input_test_pad == 0) {
        port_fatal("input test: SDL_AttachVirtualJoystick: %s", SDL_GetError());
    }
    input_test_joy = SDL_OpenJoystick(input_test_pad);
    if (input_test_joy == NULL) {
        port_fatal("input test: SDL_OpenJoystick: %s", SDL_GetError());
    }
}

/* Sets the step's input pressed or released; returns the bits the pad must then show. */
static u16 input_test_apply(int step, int down) {
    if (step < INPUT_KEYS) {
        input_push_key(input_keymap[step].key, down);
        return down ? input_keymap[step].bits : 0;
    }
    step -= INPUT_KEYS;
    if (step < INPUT_PADS) {
        SDL_SetJoystickVirtualButton(input_test_joy, input_padmap[step].button, down != 0);
        return down ? input_padmap[step].bits : 0;
    }
    step -= INPUT_PADS;
    if (step < TEST_AXIS_STEPS) {
        Sint16 rest = input_test_axes[step].axis >= SDL_GAMEPAD_AXIS_LEFT_TRIGGER ? -32768 : 0;
        SDL_SetJoystickVirtualAxis(input_test_joy, input_test_axes[step].axis,
                                   down ? input_test_axes[step].value : rest);
        return down ? input_test_axes[step].bits : 0;
    }
    /* the chord: X (cross) on the keyboard, up on the keyboard and R1 on the gamepad */
    input_push_key(SDL_SCANCODE_X, down);
    input_push_key(SDL_SCANCODE_UP, down);
    SDL_SetJoystickVirtualButton(input_test_joy, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, down != 0);
    return down ? PAD_CROSS | PAD_UP | PAD_R1 : 0;
}

static u16 input_test_expected;
static int input_test_pending; /* a step's input was applied this frame: check after the poll */

/* Before the poll: this frame's step (a press or its release). */
static void input_test_before(void) {
    if (port_frames < 2 || input_test_step >= TEST_STEPS) {
        return;
    }
    if (input_test_step == INPUT_KEYS && input_test_phase == 0) {
        if (input_test_pad == 0) {
            input_test_attach();
        }
        if (!input_gamepad_open(input_test_pad)) {
            if (++input_test_wait > 10) {
                port_fatal("input test: the virtual gamepad was not opened after 10 frames");
            }
            return; /* its SDL_EVENT_GAMEPAD_ADDED comes with a poll */
        }
    }
    input_test_expected = input_test_apply(input_test_step, input_test_phase == 0);
    input_test_pending = 1;
}

/* After the poll: the pad must show the step's bits (nothing at all with a script). */
static void input_test_after(void) {
    int ok;
    if (!input_test_pending) {
        return;
    }
    input_test_pending = 0;
    input_test_checks++;
    if (port_script_active) {
        ok = !input_applied_any && input_buttons() == input_test_expected;
    } else {
        ok = input_applied == input_test_expected;
    }
    if (!ok) {
        input_test_failures++;
        port_log("input test: frame %ld step %d %s: pad 0x%04X (computed 0x%04X), expected 0x%04X%s", port_frames,
                 input_test_step, input_test_phase == 0 ? "press" : "release", input_applied, input_buttons(),
                 input_test_expected, port_script_active ? " and nothing sent (a script owns the pad)" : "");
    }
    if (input_test_phase == 0) {
        input_test_phase = 1;
        return;
    }
    input_test_phase = 0;
    if (++input_test_step < TEST_STEPS) {
        return;
    }
    port_log("input test: %d of %d checks passed (%d keys, %d gamepad buttons, %d axes, 1 chord)%s",
             input_test_checks - input_test_failures, input_test_checks, INPUT_KEYS, INPUT_PADS, TEST_AXIS_STEPS,
             port_script_active ? "; the script owned the pad: nothing was sent" : "");
    SDL_CloseJoystick(input_test_joy);
    SDL_DetachVirtualJoystick(input_test_pad);
    if (input_test_failures != 0) {
        port_exit(6, "input test failed");
    }
    if (!port_script_active) {
        port_exit(0, "input test passed");
    }
}

void port_input_init(int test) {
    input_test = test;
}

void port_input_frame(void) {
    SDL_Event e;
    int i;
    if (input_test) {
        input_test_before();
    }
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            port_exit(0, "window closed");
            break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            if (e.key.scancode == SDL_SCANCODE_F11) {
                if (e.key.down && !e.key.repeat) {
                    port_video_toggle_fullscreen();
                }
            } else if ((i = input_key_index(e.key.scancode)) >= 0) {
                input_down[i] = e.key.down;
            }
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            memset(input_down, 0, sizeof(input_down)); /* no key stays held while another window has the keys */
            break;
        case SDL_EVENT_GAMEPAD_ADDED:
            input_gamepad_added(e.gdevice.which);
            break;
        case SDL_EVENT_GAMEPAD_REMOVED:
            input_gamepad_removed(e.gdevice.which);
            break;
        default:
            break;
        }
    }
    if (!port_script_active) {
        u16 bits = input_buttons();
        psyq_pad_set(0, 1, bits);
        port_framelog_input(bits);
        input_applied = bits;
        input_applied_any = 1;
    }
    if (input_test) {
        input_test_after();
    }
}
#else
void port_input_init(int test) {
    (void)test;
}

void port_input_frame(void) {
}
#endif
