/* The window's input (M2; docs/PORT.md "Input"; rebindable: docs/LAUNCHER.md "Input bindings"): the
 * keyboard and every SDL gamepad, ORed into pad 1 (a digital pad: SELECT, START, the D-pad, L1/L2/R1/R2 and the four
 * face buttons; psyq_pad_set, psyq.h's bit order), once per vsync from pump.c; and the hotkey actions (pause,
 * fullscreen, the mods' bindings). Only in a build configured with -DDW3_PORT_SDL=ON (DW3_PORT_SDL); the headless build
 * has no input but the script's (its actions never fire; the settings' names are checked for their shape only).
 *
 * Inputs: every keyboard key (an SDL scancode: the key's place, whatever the layout) and the gamepad inputs of
 * input_pad_names (SDL's positional buttons, then the triggers and the stick directions past half way, all gamepads
 * ORed). The pad map is a list of (input, PS1 button) entries; by default:
 *   arrows             D-pad              X  cross      C  circle     Z  square     S  triangle
 *   Enter, keypad Enter START             Backspace, right Shift  SELECT
 *   Q  L1      E  R1      1  L2      3  R2
 *   gamepads: south cross, east circle, west square, north triangle, back SELECT, start START, the shoulders L1/R1,
 *   the triggers L2/R2, the D-pad and the left stick the D-pad
 * The settings (`input.keyboard`, `input.gamepad`: a PS1 button -> a name or a list of names) replace a button's
 * keyboard or gamepad entries. Hotkeys (`input.hotkeys`, and the mods' binding options through port_input_action):
 * a binding is a list of triggers, a trigger one input or a chord of up to INPUT_CHORD inputs, all held
 * (docs/LAUNCHER.md "Input bindings"). Defaults: pause P, fullscreen F11.
 *
 * Hotkeys never reach the pad: a trigger that completes (all its inputs held) latches until every one of its inputs is
 * released, and a latched trigger's inputs are masked out of the pad (so a single-key hotkey is never seen by the game,
 * and a chord's buttons stop reaching it once it is complete; its first button reaches it until then).
 *
 * With `--script` the script owns the pad: the keyboard and the gamepads are read but never reach psyq_pad_set, so a
 * replay in a window runs exactly as it does headless (the hotkeys still act). Without one, every change of the pad
 * goes to the frame log's `I` lines and the record's `inputs` (port_framelog_input), as the script's do. Closing the
 * window ends the run: port_exit(0, "window closed").
 *
 * `--input-test` (a self-test of the mapping, for CI and after a change here): from frame 2 on, it injects the key
 * events of every keyboard input of the pad map with SDL_PushEvent (a press in one frame, the release in the next),
 * then attaches a virtual SDL gamepad and does the same with each gamepad input of the map, then one chord of two keys
 * and a gamepad input, then every hotkey trigger; after each step's poll the buttons sent to psyq_pad_set must be the
 * map's bits for the inputs held (none for a hotkey's: masked; with a script: nothing sent at all) and a hotkey step
 * must have fired its action (the actions do nothing during the test). Without --config it tests the defaults; with
 * one, the settings' map and hotkeys. Then (without a script) the pause: its key pressed, released and pressed again
 * while paused; the pause must poll without a vsync and give back the game. It ends the run with status 0 when every check passed, 6 when one failed; with
 * a script it only logs the result and the script runs on, so its log can be compared with a run without the test. */
#include <string.h>

#include "json.h"
#include "port_harness.h"
#include "port_runtime.h"
#include "psyq.h"
#include "settings.h"

#ifdef DW3_PORT_SDL
#include <SDL3/SDL.h>
#endif

/* The PS1 pad's bits (psyq.h psyq_pad_set; active high), by the settings' names. */
static const struct {
    const char *name;
    u16 bits;
} input_buttons_named[] = {
    { "select", 1u << 0 },  { "start", 1u << 3 },     { "up", 1u << 4 },     { "right", 1u << 5 },
    { "down", 1u << 6 },    { "left", 1u << 7 },      { "l2", 1u << 8 },     { "r2", 1u << 9 },
    { "l1", 1u << 10 },     { "r1", 1u << 11 },       { "triangle", 1u << 12 }, { "circle", 1u << 13 },
    { "cross", 1u << 14 },  { "square", 1u << 15 },
};
#define INPUT_BUTTONS ((int)(sizeof(input_buttons_named) / sizeof(input_buttons_named[0])))
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

/* The gamepad inputs: the buttons in SDL_GamepadButton order (index = the SDL button), then the axes past half way. */
static const char *const input_pad_names[] = {
    "south", "east", "west", "north", "back", "guide", "start", "leftstick", "rightstick", "leftshoulder",
    "rightshoulder", "dpup", "dpdown", "dpleft", "dpright", "misc1", "paddle1", "paddle2", "paddle3", "paddle4",
    "touchpad",
    "lefttrigger", "righttrigger", "leftx-", "leftx+", "lefty-", "lefty+", "rightx-", "rightx+", "righty-", "righty+",
};
#define INPUT_PAD_BUTTONS 21
#define INPUT_PAD_INPUTS ((int)(sizeof(input_pad_names) / sizeof(input_pad_names[0])))
#define INPUT_KEYS 512 /* SDL_SCANCODE_COUNT */
#define INPUT_PAD(n) (INPUT_KEYS + (n)) /* the input id of gamepad input n */
#define INPUT_IDS (INPUT_KEYS + INPUT_PAD_INPUTS)
enum { PAD_LT = INPUT_PAD_BUTTONS, PAD_RT, PAD_LX_NEG, PAD_LX_POS, PAD_LY_NEG, PAD_LY_POS };

#define INPUT_MAX_MAP 128
#define INPUT_MAX_ACTIONS 32
#define INPUT_MAX_TRIGGERS 8
#define INPUT_CHORD 4

static struct {
    int id;
    u16 bits;
} input_map[INPUT_MAX_MAP];
static int input_map_count;

typedef struct InputTrigger {
    int n;
    int ids[INPUT_CHORD];
    int complete; /* every input held at the last poll */
    int latched;  /* completed, and not every input released since: its inputs are masked out of the pad */
} InputTrigger;

static struct {
    const char *name;
    InputTrigger triggers[INPUT_MAX_TRIGGERS];
    int count;
    int held, pressed; /* at the last poll: a trigger complete; a trigger completed */
} input_actions[INPUT_MAX_ACTIONS];
static int input_action_count;
int port_action_pause = -1, port_action_fullscreen = -1;
static int input_test;      /* --input-test running */
static int input_test_mute; /* the actions do nothing for their consumers (the test's steps) */

/* ---- The names (the settings' grammar; docs/LAUNCHER.md "Input bindings") */

static int input_button_bits(const char *name) {
    int i;
    for (i = 0; i < INPUT_BUTTONS; i++) {
        if (strcmp(input_buttons_named[i].name, name) == 0) {
            return input_buttons_named[i].bits;
        }
    }
    return -1;
}

static int input_pad_index(const char *name) {
    int i;
    for (i = 0; i < INPUT_PAD_INPUTS; i++) {
        if (strcmp(input_pad_names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

/* A key name (SDL3's scancode names) -> its input id; -1 when unknown. Without SDL: 0 for any non-empty name (the
 * headless build checks the shape only: it has no keyboard). */
static int input_key_id(const char *name) {
#ifdef DW3_PORT_SDL
    SDL_Scancode sc = SDL_GetScancodeFromName(name);
    return sc != SDL_SCANCODE_UNKNOWN && (int)sc < INPUT_KEYS ? (int)sc : -1;
#else
    return name[0] != '\0' ? 0 : -1;
#endif
}

/* An input of a binding: a key name or "pad:" + a gamepad input name. */
static int input_binding_input(const char *name, const char *where) {
    int id;
    if (strncmp(name, "pad:", 4) == 0) {
        id = input_pad_index(name + 4);
        if (id < 0) {
            port_settings_fail(where, "\"%s\": not a gamepad input (docs/LAUNCHER.md \"Input bindings\")", name);
        }
        return INPUT_PAD(id);
    }
    id = input_key_id(name);
    if (id < 0) {
        port_settings_fail(where, "\"%s\": not a key name (SDL3's scancode names, e.g. \"Tab\", \"F2\")", name);
    }
    return id;
}

static void input_trigger_add(int action, const PortJson *t, const char *where) {
    InputTrigger *tr;
    size_t i;
    if (t->type == PORT_JSON_STRING && t->string[0] == '\0') {
        return; /* unbound */
    }
    if (input_actions[action].count == INPUT_MAX_TRIGGERS) {
        port_settings_fail(where, "at most %d triggers", INPUT_MAX_TRIGGERS);
    }
    tr = &input_actions[action].triggers[input_actions[action].count];
    memset(tr, 0, sizeof(*tr));
    if (t->type == PORT_JSON_STRING) {
        tr->ids[tr->n++] = input_binding_input(t->string, where);
    } else if (t->type == PORT_JSON_ARRAY && t->count >= 1 && t->count <= INPUT_CHORD) {
        for (i = 0; i < t->count; i++) {
            if (t->items[i].type != PORT_JSON_STRING) {
                port_settings_fail(where, "a chord is a list of input names");
            }
            tr->ids[tr->n++] = input_binding_input(t->items[i].string, where);
        }
    } else {
        port_settings_fail(where, "a trigger is an input name or a chord (a list of 1 to %d names)", INPUT_CHORD);
    }
    input_actions[action].count++;
}

int port_input_action(const char *name, const PortJson *binding, const char *where, const char *default_json) {
    PortJson *dflt = NULL;
    int a;
    size_t i;
    if (input_action_count == INPUT_MAX_ACTIONS) {
        port_fatal("input: more than %d actions", INPUT_MAX_ACTIONS);
    }
    a = input_action_count++;
    memset(&input_actions[a], 0, sizeof(input_actions[a]));
    input_actions[a].name = name;
    if (binding == NULL) {
        char err[128];
        dflt = port_json_parse(default_json, strlen(default_json), err, sizeof(err));
        if (dflt == NULL) {
            port_fatal("input: %s: the default binding %s: %s", name, default_json, err);
        }
        binding = dflt;
    }
    if (binding->type == PORT_JSON_STRING) {
        input_trigger_add(a, binding, where);
    } else if (binding->type == PORT_JSON_ARRAY) {
        for (i = 0; i < binding->count; i++) {
            input_trigger_add(a, &binding->items[i], where);
        }
    } else {
        port_settings_fail(where, "a binding is a string or a list, not %s", port_json_type_name(binding->type));
    }
    if (dflt != NULL) {
        port_json_free(dflt);
    }
    return a;
}

void port_input_check_binding(const PortJson *binding, const char *where) {
    int saved = input_action_count;
    port_input_action(where, binding, where, "\"\"");
    input_action_count = saved;
}

int port_input_pressed(int action) {
    return action >= 0 && action < input_action_count && !input_test_mute && input_actions[action].pressed;
}

int port_input_held(int action) {
    return action >= 0 && action < input_action_count && !input_test_mute && input_actions[action].held;
}

static void input_map_add(int id, u16 bits) {
    if (input_map_count == INPUT_MAX_MAP) {
        port_fatal("input: more than %d pad map entries", INPUT_MAX_MAP);
    }
    input_map[input_map_count].id = id;
    input_map[input_map_count].bits = bits;
    input_map_count++;
}

static void input_map_defaults(void) {
    static const struct {
        const char *name;
        u16 bits;
    } keys[] = {
        { "Up", PAD_UP },         { "Right", PAD_RIGHT },     { "Down", PAD_DOWN },      { "Left", PAD_LEFT },
        { "X", PAD_CROSS },       { "C", PAD_CIRCLE },        { "Z", PAD_SQUARE },       { "S", PAD_TRIANGLE },
        { "Return", PAD_START },  { "Keypad Enter", PAD_START }, { "Backspace", PAD_SELECT },
        { "Right Shift", PAD_SELECT }, { "Q", PAD_L1 },       { "E", PAD_R1 },           { "1", PAD_L2 },
        { "3", PAD_R2 },
    };
    static const struct {
        int pad;
        u16 bits;
    } pads[] = {
        { 0, PAD_CROSS },  { 1, PAD_CIRCLE }, { 2, PAD_SQUARE }, { 3, PAD_TRIANGLE }, { 4, PAD_SELECT },
        { 6, PAD_START },  { 9, PAD_L1 },     { 10, PAD_R1 },    { 11, PAD_UP },      { 12, PAD_DOWN },
        { 13, PAD_LEFT },  { 14, PAD_RIGHT }, { PAD_LT, PAD_L2 }, { PAD_RT, PAD_R2 },  { PAD_LX_NEG, PAD_LEFT },
        { PAD_LX_POS, PAD_RIGHT }, { PAD_LY_NEG, PAD_UP },       { PAD_LY_POS, PAD_DOWN },
    };
    size_t i;
    input_map_count = 0;
    for (i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        int id = input_key_id(keys[i].name);
        if (id < 0) {
            port_fatal("input: the default key %s is unknown to SDL", keys[i].name);
        }
        input_map_add(id, keys[i].bits);
    }
    for (i = 0; i < sizeof(pads) / sizeof(pads[0]); i++) {
        input_map_add(INPUT_PAD(pads[i].pad), pads[i].bits);
    }
}

/* input.keyboard or input.gamepad: each button named replaces that button's entries of that kind. */
static void input_map_section(const PortJson *obj, int pad) {
    const char *where = pad ? "input.gamepad" : "input.keyboard";
    size_t i, j;
    if (obj == NULL) {
        return;
    }
    if (obj->type != PORT_JSON_OBJECT) {
        port_settings_fail(where, "an object (a PS1 button -> a name or a list of names)");
    }
    for (i = 0; i < obj->count; i++) {
        int bits = input_button_bits(obj->keys[i]);
        const PortJson *v = &obj->items[i];
        char key[64];
        int k, n;
        snprintf(key, sizeof(key), "%s.%s", where, obj->keys[i]);
        if (bits < 0) {
            port_log("settings: %s: unknown PS1 button, ignored", key);
            continue;
        }
        for (k = n = 0; k < input_map_count; k++) {
            if (input_map[k].bits == bits && (input_map[k].id >= INPUT_KEYS) == pad) {
                continue; /* replaced */
            }
            input_map[n++] = input_map[k];
        }
        input_map_count = n;
        for (j = 0; j < (v->type == PORT_JSON_ARRAY ? v->count : 1); j++) {
            const PortJson *name = v->type == PORT_JSON_ARRAY ? &v->items[j] : v;
            int id;
            if (name->type != PORT_JSON_STRING) {
                port_settings_fail(key, "a %s name or a list of them", pad ? "gamepad input" : "key");
            }
            if (name->string[0] == '\0') {
                continue;
            }
            if (pad) {
                id = input_pad_index(name->string);
                if (id < 0) {
                    port_settings_fail(key, "\"%s\": not a gamepad input (docs/LAUNCHER.md \"Input bindings\")",
                                       name->string);
                }
                id = INPUT_PAD(id);
            } else {
                id = input_key_id(name->string);
                if (id < 0) {
                    port_settings_fail(key, "\"%s\": not a key name (SDL3's scancode names)", name->string);
                }
            }
            input_map_add(id, (u16)bits);
        }
    }
}

void port_input_settings(const PortJson *input) {
    static const char *const known[] = { "keyboard", "gamepad", "hotkeys", NULL };
    const PortJson *hotkeys = NULL;
    size_t i;
    input_map_defaults();
    input_action_count = 0;
    if (input != NULL) {
        for (i = 0; i < input->count; i++) {
            const char *const *k;
            for (k = known; *k != NULL && strcmp(*k, input->keys[i]) != 0; k++) {
            }
            if (*k == NULL) {
                port_log("settings: input.%s: unknown key, ignored", input->keys[i]);
            }
        }
        input_map_section(port_json_get(input, "keyboard"), 0);
        input_map_section(port_json_get(input, "gamepad"), 1);
        hotkeys = port_json_get(input, "hotkeys");
        if (hotkeys != NULL && hotkeys->type != PORT_JSON_OBJECT) {
            port_settings_fail("input.hotkeys", "an object (an action -> a binding)");
        }
        for (i = 0; hotkeys != NULL && i < hotkeys->count; i++) {
            if (strcmp(hotkeys->keys[i], "pause") != 0 && strcmp(hotkeys->keys[i], "fullscreen") != 0) {
                port_log("settings: input.hotkeys.%s: unknown action, ignored", hotkeys->keys[i]);
            }
        }
    }
    port_action_pause = port_input_action("pause", port_json_get(hotkeys, "pause"), "input.hotkeys.pause", "\"P\"");
    port_action_fullscreen = port_input_action("fullscreen", port_json_get(hotkeys, "fullscreen"),
                                               "input.hotkeys.fullscreen", "\"F11\"");
}

#ifdef DW3_PORT_SDL
#define INPUT_AXIS_PRESSED 16384 /* half way: a trigger or a stick direction counts as pressed beyond it */
#define INPUT_MAX_GAMEPADS 8

static u8 input_key_down[INPUT_KEYS];  /* from the key events */
static u8 input_down[INPUT_IDS];       /* every input, at the last poll */
static u8 input_masked[INPUT_IDS];     /* inputs of a latched trigger */
static SDL_Gamepad *input_gamepads[INPUT_MAX_GAMEPADS];
static u16 input_applied;              /* the buttons last sent to psyq_pad_set */
static int input_applied_any;          /* psyq_pad_set was called by this file */

/* ---- --input-test */
static int input_test_step;  /* the current step (input_test_steps: done) */
static int input_test_phase; /* 0: press now, 1: release now */
static int input_test_checks, input_test_failures;
static long input_test_wait;          /* frames spent waiting for the virtual gamepad */
static SDL_JoystickID input_test_pad; /* the virtual gamepad (0: not attached) */
static SDL_Joystick *input_test_joy;
/* The steps: each a set of inputs pressed together, and the action it must fire (-1: none). */
#define TEST_MAX_STEPS 192
static struct {
    int n;
    int ids[INPUT_CHORD];
    int action;
} input_test_steps[TEST_MAX_STEPS];
static int input_test_count, input_test_keys, input_test_pads, input_test_hotkeys;

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

/* Every input's state: the keys from their events, the gamepads read now (all of them ORed). */
static void input_read(void) {
    static const SDL_GamepadAxis axes[] = { SDL_GAMEPAD_AXIS_LEFT_TRIGGER, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
                                            SDL_GAMEPAD_AXIS_LEFTX, SDL_GAMEPAD_AXIS_LEFTX,
                                            SDL_GAMEPAD_AXIS_LEFTY, SDL_GAMEPAD_AXIS_LEFTY,
                                            SDL_GAMEPAD_AXIS_RIGHTX, SDL_GAMEPAD_AXIS_RIGHTX,
                                            SDL_GAMEPAD_AXIS_RIGHTY, SDL_GAMEPAD_AXIS_RIGHTY };
    int i, j;
    memcpy(input_down, input_key_down, INPUT_KEYS);
    memset(input_down + INPUT_KEYS, 0, INPUT_PAD_INPUTS);
    for (i = 0; i < INPUT_MAX_GAMEPADS; i++) {
        SDL_Gamepad *g = input_gamepads[i];
        if (g == NULL) {
            continue;
        }
        for (j = 0; j < INPUT_PAD_BUTTONS; j++) {
            input_down[INPUT_PAD(j)] |= SDL_GetGamepadButton(g, (SDL_GamepadButton)j) ? 1 : 0;
        }
        for (j = 0; j < INPUT_PAD_INPUTS - INPUT_PAD_BUTTONS; j++) {
            Sint16 v = SDL_GetGamepadAxis(g, axes[j]);
            /* the triggers and the + directions: above half way; the - directions: below minus half way */
            int on = (j < 2 || (j & 1)) ? v > INPUT_AXIS_PRESSED : v < -INPUT_AXIS_PRESSED;
            input_down[INPUT_PAD(INPUT_PAD_BUTTONS + j)] |= on ? 1 : 0;
        }
    }
}

/* The actions from the inputs: triggers completed (an action pressed), held, latched; the masked inputs. */
static void input_actions_update(void) {
    int a, t, k;
    memset(input_masked, 0, sizeof(input_masked));
    for (a = 0; a < input_action_count; a++) {
        input_actions[a].held = input_actions[a].pressed = 0;
        for (t = 0; t < input_actions[a].count; t++) {
            InputTrigger *tr = &input_actions[a].triggers[t];
            int all = 1, any = 0;
            for (k = 0; k < tr->n; k++) {
                all &= input_down[tr->ids[k]];
                any |= input_down[tr->ids[k]];
            }
            if (all && !tr->complete) {
                input_actions[a].pressed = 1;
                tr->latched = 1;
            }
            tr->complete = all;
            if (!any) {
                tr->latched = 0;
            }
            input_actions[a].held |= all;
            if (tr->latched) {
                for (k = 0; k < tr->n; k++) {
                    input_masked[tr->ids[k]] = 1;
                }
            }
        }
    }
}

static u16 input_buttons(void) {
    u16 bits = 0;
    int i;
    for (i = 0; i < input_map_count; i++) {
        if (input_down[input_map[i].id] && !input_masked[input_map[i].id]) {
            bits |= input_map[i].bits;
        }
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

/* The virtual gamepad: every button in SDL_GamepadButton order (south .. touchpad) and 6 axes in SDL_GamepadAxis
 * order (SDL's virtual driver maps them one to one when the masks are 0). */
static void input_test_attach(void) {
    SDL_VirtualJoystickDesc desc;
    SDL_INIT_INTERFACE(&desc);
    desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
    desc.nbuttons = INPUT_PAD_BUTTONS;
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

static void input_test_set(int id, int down) {
    static const SDL_GamepadAxis axes[] = { SDL_GAMEPAD_AXIS_LEFT_TRIGGER, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
                                            SDL_GAMEPAD_AXIS_LEFTX, SDL_GAMEPAD_AXIS_LEFTX,
                                            SDL_GAMEPAD_AXIS_LEFTY, SDL_GAMEPAD_AXIS_LEFTY,
                                            SDL_GAMEPAD_AXIS_RIGHTX, SDL_GAMEPAD_AXIS_RIGHTX,
                                            SDL_GAMEPAD_AXIS_RIGHTY, SDL_GAMEPAD_AXIS_RIGHTY };
    int p;
    if (id < INPUT_KEYS) {
        input_push_key((SDL_Scancode)id, down);
        return;
    }
    p = id - INPUT_KEYS;
    if (p < INPUT_PAD_BUTTONS) {
        SDL_SetJoystickVirtualButton(input_test_joy, p, down != 0);
    } else {
        int j = p - INPUT_PAD_BUTTONS;
        Sint16 rest = j < 2 ? -32768 : 0, on = (j < 2 || (j & 1)) ? 32767 : -32768;
        SDL_SetJoystickVirtualAxis(input_test_joy, axes[j], down ? on : rest);
    }
}

static int input_test_has(int id) {
    int i;
    for (i = 0; i < input_test_count; i++) {
        if (input_test_steps[i].n == 1 && input_test_steps[i].action < 0 && input_test_steps[i].ids[0] == id) {
            return 1;
        }
    }
    return 0;
}

static void input_test_add(int n, const int *ids, int action) {
    if (input_test_count == TEST_MAX_STEPS) {
        port_fatal("input test: more than %d steps", TEST_MAX_STEPS);
    }
    input_test_steps[input_test_count].n = n;
    memcpy(input_test_steps[input_test_count].ids, ids, sizeof(int) * (size_t)n);
    input_test_steps[input_test_count].action = action;
    input_test_count++;
}

/* The steps: every keyboard input of the map, every gamepad input, a chord (two keys and a gamepad input), every
 * hotkey trigger. */
static void input_test_plan(void) {
    int i, t, pass, chord[3], c = 0;
    for (pass = 0; pass < 2; pass++) {
        for (i = 0; i < input_map_count; i++) {
            int id = input_map[i].id;
            if ((id >= INPUT_KEYS) == pass && !input_test_has(id)) {
                input_test_add(1, &id, -1);
                if (pass == 0) {
                    input_test_keys++;
                    if (c < 2) {
                        chord[c++] = id;
                    }
                } else {
                    input_test_pads++;
                    if (c == 2) {
                        chord[c++] = id;
                    }
                }
            }
        }
    }
    if (c == 3) {
        input_test_add(3, chord, -1);
    }
    for (i = 0; i < input_action_count; i++) {
        for (t = 0; t < input_actions[i].count; t++) {
            input_test_add(input_actions[i].triggers[t].n, input_actions[i].triggers[t].ids, i);
            input_test_hotkeys++;
        }
    }
}

static u16 input_test_expected;
static int input_test_pending; /* a step's input was applied this frame: check after the poll */
static int input_test_pause;   /* the pause's round trip: 1 press now, 2 paused (then released) */
static int input_test_pause_polls;
static long input_test_pause_frame, input_test_pause_resumed;
static int input_test_pause_before(void);
static void input_test_paused(void);

static int input_test_needs_pad(int step) {
    int k;
    for (k = 0; k < input_test_steps[step].n; k++) {
        if (input_test_steps[step].ids[k] >= INPUT_KEYS) {
            return 1;
        }
    }
    return 0;
}

/* Before the poll: this frame's step (a press or its release). */
static void input_test_before(void) {
    int k, i, masked;
    if (input_test_pause_before()) {
        return;
    }
    if (port_frames < 2 || input_test_step >= input_test_count) {
        return;
    }
    if (input_test_phase == 0 && input_test_needs_pad(input_test_step)) {
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
    input_test_expected = 0;
    for (k = 0; k < input_test_steps[input_test_step].n; k++) {
        int id = input_test_steps[input_test_step].ids[k];
        input_test_set(id, input_test_phase == 0);
        /* the map's bits, unless a single-input hotkey or this step's own hotkey masks the input */
        masked = input_test_steps[input_test_step].action >= 0;
        for (i = 0; i < input_action_count && !masked; i++) {
            int t;
            for (t = 0; t < input_actions[i].count; t++) {
                masked |= input_actions[i].triggers[t].n == 1 && input_actions[i].triggers[t].ids[0] == id;
            }
        }
        for (i = 0; i < input_map_count && input_test_phase == 0 && !masked; i++) {
            if (input_map[i].id == id) {
                input_test_expected |= input_map[i].bits;
            }
        }
    }
    input_test_pending = 1;
}

/* After the poll: the pad must show the step's bits (nothing at all with a script), a hotkey must have fired. */
static void input_test_after(void) {
    int ok, action;
    if (!input_test_pending) {
        return;
    }
    input_test_pending = 0;
    input_test_checks++;
    action = input_test_steps[input_test_step].action;
    if (port_script_active) {
        ok = !input_applied_any && input_buttons() == input_test_expected;
    } else {
        ok = input_applied == input_test_expected;
    }
    if (action >= 0 && input_test_phase == 0) {
        ok &= input_actions[action].pressed;
    }
    if (!ok) {
        input_test_failures++;
        port_log("input test: frame %ld step %d %s: pad 0x%04X (computed 0x%04X), expected 0x%04X%s%s%s", port_frames,
                 input_test_step, input_test_phase == 0 ? "press" : "release", input_applied, input_buttons(),
                 input_test_expected, port_script_active ? " and nothing sent (a script owns the pad)" : "",
                 action >= 0 ? ", the action " : "", action >= 0 ? input_actions[action].name : "");
    }
    if (input_test_phase == 0) {
        input_test_phase = 1;
        return;
    }
    input_test_phase = 0;
    if (++input_test_step < input_test_count) {
        return;
    }
    port_log("input test: %d of %d checks passed (%d keys, %d gamepad inputs, 1 chord, %d hotkey triggers)%s",
             input_test_checks - input_test_failures, input_test_checks, input_test_keys, input_test_pads,
             input_test_hotkeys, port_script_active ? "; the script owned the pad: nothing was sent" : "");
    if (input_test_pad != 0) {
        SDL_CloseJoystick(input_test_joy);
        SDL_DetachVirtualJoystick(input_test_pad);
    }
    if (input_test_failures != 0) {
        port_exit(6, "input test failed");
    }
    if (!port_script_active && input_actions[port_action_pause].count > 0) {
        input_test_pause = 1; /* then the pause's round trip */
        return;
    }
    input_test = input_test_mute = 0;
    if (!port_script_active) {
        port_exit(0, "input test passed");
    }
}

/* The pause's round trip (after the steps, without a script): the pause key's first trigger pressed at a vsync (the
 * pump pauses at its end), released at the first poll of the pause and pressed again at the third (the pump goes
 * on), released at the next vsync: the pause must have polled three times without a vsync. */
static void input_test_pause_set(int down) {
    const InputTrigger *tr = &input_actions[port_action_pause].triggers[0];
    int k;
    for (k = 0; k < tr->n; k++) {
        input_test_set(tr->ids[k], down);
    }
}

static int input_test_pause_before(void) {
    if (input_test_pause == 1) {
        input_test_mute = 0;
        input_test_pause_set(1);
        input_test_pause_frame = port_frames;
        input_test_pause = 2;
        return 1;
    }
    if (input_test_pause == 2) {
        int ok = input_test_pause_polls == 3 && input_test_pause_resumed == input_test_pause_frame;
        input_test_pause_set(0);
        port_log("input test: the pause: %s (%d polls paused at frame %ld, resumed at frame %ld)",
                 ok ? "passed" : "FAILED", input_test_pause_polls, input_test_pause_frame, input_test_pause_resumed);
        port_exit(ok ? 0 : 6, ok ? "input test passed" : "input test failed");
    }
    return 0;
}

static void input_test_paused(void) {
    if (input_test_pause != 2) {
        return;
    }
    input_test_pause_polls++;
    if (input_test_pause_polls == 1) {
        input_test_pause_set(0);
    } else if (input_test_pause_polls == 3) {
        input_test_pause_set(1);
        input_test_pause_resumed = port_frames;
    }
}

void port_input_init(int test) {
    input_test = input_test_mute = test;
    if (test) {
        input_test_plan();
    }
}

/* SDL's events, the inputs and the actions; the fullscreen hotkey acts here. */
static void input_poll(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            port_exit(0, "window closed");
            break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            if ((int)e.key.scancode >= 0 && (int)e.key.scancode < INPUT_KEYS) {
                input_key_down[e.key.scancode] = e.key.down ? 1 : 0;
            }
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            memset(input_key_down, 0, sizeof(input_key_down)); /* no key stays held while another window has them */
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
    input_read();
    input_actions_update();
    if (port_input_pressed(port_action_fullscreen)) {
        port_video_toggle_fullscreen();
    }
}

void port_input_frame(void) {
    if (input_test) {
        input_test_before();
    }
    input_poll();
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

void port_input_poll_paused(void) {
    if (input_test) {
        input_test_paused();
    }
    input_poll();
}
#else
void port_input_init(int test) {
    input_test = input_test_mute = test;
}

void port_input_frame(void) {
}

void port_input_poll_paused(void) {
}
#endif
