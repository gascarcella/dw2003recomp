/* The input script (port_harness.h): the layer-2 step engine of tests/replay/run.lua, in C, over the port's game-state
 * probes. A script is tests/replay/scripts/<name>.json (tests/README.md "Layer 2"); its steps run with run.lua's
 * semantics, frame for frame:
 *
 * - Every frame (port_script_frame, once per vsync tick after the frame log's sample, as run.lua's vsync listener runs
 *   after recording the sequences): past `max_frames` the run fails; then the current step runs, and steps that end
 *   "instantly" (a checkpoint, a wait already satisfied, a press whose `until` holds, a walk that arrived) let the next
 *   step run in the same frame, at most 100 in a row; then the pad is set to the held buttons (run.lua's apply_pad);
 *   then, past the last step, the run ends with status 0, "script complete".
 * - A step that times out (or `max_frames`) ends the run with status 5 and run.lua's message, naming the step.
 * - `reset` (a hard reset of the console: PCSX-Redux hardResetEmulator in run.lua) releases the pad, ends the frame and
 *   resets (port_reset_request, port/src/reset.c): the game starts over from main() with its data, the arena and the
 *   shim at power-on, the memory cards kept; the frame count, the record and the script's next step go on.
 *
 * The buttons are the physical pad's (PCSX.CONSTS.PAD.BUTTON numbers them as the PS1 pad's bits, which psyq_pad_set
 * takes, active high); the game itself rotates the face buttons for non-Japanese languages (pad_read_buttons), so they
 * pass through unchanged. Numbers in a script may be JSON numbers or hex strings ("0x2D7"). Anything a step does not
 * define (`comment`, ...) is ignored, as run.lua ignores it; everything it defines is checked when the script is
 * loaded (run.lua would fail only on reaching the step), except a wait_mem address, which is fatal (status 1) only when
 * its step runs if port_state_read does not map it, so the steps before it still run. A step's start goes to stderr. */
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"
#include "port_harness.h"
#include "port_runtime.h"
#include "psyq.h"

#include "fieldstg.h"
#include "heap.h"

#define SCRIPT_STEP_BUDGET 100 /* instant steps chained in one frame (run.lua's budget) */
#define SCRIPT_PLAYER_KIND 5   /* the field's player: the heap_objects entry of kind 5, key2 0 (run.lua player_pos) */

typedef enum ScriptStepType {
    SCRIPT_WAIT_STAGE,
    SCRIPT_WAIT_MAP,
    SCRIPT_WAIT_MEM,
    SCRIPT_WAIT_FRAMES,
    SCRIPT_PRESS,
    SCRIPT_WALK,
    SCRIPT_RESET,
    SCRIPT_CHECKPOINT,
    SCRIPT_VRAM,
} ScriptStepType;

static const char *const script_type_names[] = {
    "wait_stage", "wait_map", "wait_mem", "wait_frames", "press", "walk", "reset", "checkpoint", "vram",
};

/* The PS1 pad's button names by bit (PCSX.CONSTS.PAD.BUTTON; psyq.h psyq_pad_set). */
static const char *const script_button_names[16] = {
    "SELECT", "L3", "R3", "START", "UP", "RIGHT", "DOWN", "LEFT",
    "L2", "R2", "L1", "R1", "TRIANGLE", "CIRCLE", "CROSS", "SQUARE",
};
#define SCRIPT_BUTTON_UP (1u << 4)
#define SCRIPT_BUTTON_RIGHT (1u << 5)
#define SCRIPT_BUTTON_DOWN (1u << 6)
#define SCRIPT_BUTTON_LEFT (1u << 7)

/* A wait condition: a wait_* step itself, or a press step's `until`. */
typedef struct ScriptCond {
    ScriptStepType type; /* SCRIPT_WAIT_STAGE, _MAP or _MEM */
    long long stage;     /* wait_stage */
    int has_word0;
    long long word0;
    long long map; /* wait_map */
    u32 addr;      /* wait_mem: `size` bytes at `addr`, sign-extended if `is_signed`, equal to `value` */
    int size, is_signed;
    long long value;
} ScriptCond;

typedef struct ScriptStep {
    ScriptStepType type;
    ScriptCond cond; /* wait_stage/_map/_mem: the condition; press: `until` (if has_until) */
    int has_until;
    long timeout;  /* the step's timeout, or the script's default_timeout */
    long frames;   /* wait_frames; press: frames held (default 2) */
    long release;  /* press: frames released (default 2) */
    int has_repeat;
    long repeat;   /* press */
    u16 buttons;   /* press */
    double x, y, tol; /* walk */
    char *name;    /* checkpoint, vram (default "unnamed") */
} ScriptStep;

static char *script_name;
static ScriptStep *script_steps;
static int script_step_count;
static long script_max_frames;

static int script_index;               /* the current step (0-based; run.lua's step_index - 1) */
static long script_step_started = -1;  /* the frame the current step started, -1 before it runs */
static u16 script_held;                /* the buttons held (run.lua's `held`), applied every frame */
static int script_reset_pending;       /* a `reset` step ran this frame: the console resets at the frame's end */

/* ---- Loading */

static const char *script_path;

static void script_error(int step, const char *fmt, ...) __attribute__((format(printf, 2, 3), noreturn));
static void script_error(int step, const char *fmt, ...) {
    char msg[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    if (step > 0) {
        port_fatal("script %s: step %d: %s", script_path, step, msg);
    }
    port_fatal("script %s: %s", script_path, msg);
}

/* An integer field: a JSON number (an integer) or a hex string "0x..."; 0 when absent (fatal if `required`). */
static int script_int(const PortJson *obj, const char *key, long long *out, int required, int step) {
    const PortJson *v = port_json_get(obj, key);
    if (v == NULL) {
        if (required) {
            script_error(step, "`%s` missing", key);
        }
        return 0;
    }
    if (v->type == PORT_JSON_NUMBER) {
        if (v->number != floor(v->number) || fabs(v->number) > 9007199254740992.0) {
            script_error(step, "`%s`: not an integer", key);
        }
        *out = (long long)v->number;
        return 1;
    }
    if (v->type == PORT_JSON_STRING && strncmp(v->string, "0x", 2) == 0 && v->string[2] != '\0' &&
        strlen(v->string) <= 2 + 8 && strspn(v->string + 2, "0123456789abcdefABCDEF") == strlen(v->string + 2)) {
        *out = (long long)strtoull(v->string + 2, NULL, 16);
        return 1;
    }
    script_error(step, "`%s`: a number or a hex string (\"0x...\", at most 8 digits) expected", key);
}

static double script_number(const PortJson *obj, const char *key, double dflt, int required, int step) {
    const PortJson *v = port_json_get(obj, key);
    long long i;
    if (v == NULL) {
        if (required) {
            script_error(step, "`%s` missing", key);
        }
        return dflt;
    }
    if (v->type == PORT_JSON_NUMBER) {
        return v->number;
    }
    script_int(obj, key, &i, 1, step);
    return (double)i;
}

/* A count of frames: an integer >= `min`, `dflt` when absent. */
static long script_frames(const PortJson *obj, const char *key, long dflt, long min, int step) {
    long long v;
    if (!script_int(obj, key, &v, dflt < 0, step)) {
        return dflt;
    }
    if (v < min || v > 0x7FFFFFFF) {
        script_error(step, "`%s`: %lld out of range", key, v);
    }
    return (long)v;
}

static ScriptStepType script_type(const PortJson *obj, int step) {
    const PortJson *t = port_json_get(obj, "type");
    size_t i;
    if (t == NULL || t->type != PORT_JSON_STRING) {
        script_error(step, "`type` (a string) missing");
    }
    for (i = 0; i < sizeof(script_type_names) / sizeof(script_type_names[0]); i++) {
        if (strcmp(t->string, script_type_names[i]) == 0) {
            return (ScriptStepType)i;
        }
    }
    script_error(step, "unknown step type \"%s\"", t->string);
}

static void script_load_cond(const PortJson *obj, ScriptCond *c, int step) {
    c->type = script_type(obj, step);
    switch (c->type) {
    case SCRIPT_WAIT_STAGE:
        script_int(obj, "stage", &c->stage, 1, step);
        c->has_word0 = script_int(obj, "word0", &c->word0, 0, step);
        break;
    case SCRIPT_WAIT_MAP:
        script_int(obj, "map", &c->map, 1, step);
        break;
    case SCRIPT_WAIT_MEM: {
        const PortJson *sg = port_json_get(obj, "signed");
        long long addr, size = 4;
        script_int(obj, "addr", &addr, 1, step);
        script_int(obj, "size", &size, 0, step);
        script_int(obj, "value", &c->value, 1, step);
        if (size != 1 && size != 2 && size != 4) {
            script_error(step, "wait_mem: `size` must be 1, 2 or 4");
        }
        if (sg != NULL && sg->type != PORT_JSON_BOOL) {
            script_error(step, "wait_mem: `signed` must be true or false");
        }
        if (addr < 0 || addr > 0xFFFFFFFFLL) {
            script_error(step, "wait_mem: `addr` 0x%llX is not a 32-bit address", addr);
        }
        c->addr = (u32)addr;
        c->size = (int)size;
        c->is_signed = sg != NULL && sg->boolean;
        /* an address the port does not map (port/src/state.c maps only layout-identical ranges) is fatal when the step
         * runs, not here: a script's steps before it still run, as in run.lua */
        break;
    }
    default:
        script_error(step, "`until` must be a wait_stage, wait_map or wait_mem condition, not %s",
                     script_type_names[c->type]);
    }
}

static u16 script_buttons(const PortJson *obj, int step) {
    const PortJson *list = port_json_get(obj, "buttons");
    u16 bits = 0;
    size_t i;
    int b;
    if (list == NULL || list->type != PORT_JSON_ARRAY) {
        script_error(step, "press: `buttons` (an array of names) missing");
    }
    for (i = 0; i < list->count; i++) {
        const PortJson *name = &list->items[i];
        if (name->type != PORT_JSON_STRING) {
            script_error(step, "press: a button name is a string");
        }
        for (b = 0; b < 16; b++) {
            if (strcmp(name->string, script_button_names[b]) == 0) {
                break;
            }
        }
        if (b == 16) {
            script_error(step, "press: unknown button \"%s\"", name->string);
        }
        bits |= (u16)(1u << b);
    }
    return bits;
}

static void script_load_step(const PortJson *obj, ScriptStep *s, long default_timeout, int step) {
    const PortJson *until;
    if (obj->type != PORT_JSON_OBJECT) {
        script_error(step, "a step is an object");
    }
    memset(s, 0, sizeof(*s));
    s->type = script_type(obj, step);
    s->timeout = script_frames(obj, "timeout", default_timeout, 0, step);
    switch (s->type) {
    case SCRIPT_WAIT_STAGE:
    case SCRIPT_WAIT_MAP:
    case SCRIPT_WAIT_MEM:
        script_load_cond(obj, &s->cond, step);
        break;
    case SCRIPT_WAIT_FRAMES:
        s->frames = script_frames(obj, "frames", -1, 0, step);
        break;
    case SCRIPT_PRESS:
        s->buttons = script_buttons(obj, step);
        s->frames = script_frames(obj, "frames", 2, 0, step);
        s->release = script_frames(obj, "release", 2, 0, step);
        if (s->frames + s->release == 0) {
            script_error(step, "press: `frames` + `release` is 0");
        }
        if (port_json_get(obj, "repeat") != NULL) {
            s->has_repeat = 1;
            s->repeat = script_frames(obj, "repeat", 1, 0, step);
        }
        until = port_json_get(obj, "until");
        if (until != NULL) {
            if (until->type != PORT_JSON_OBJECT) {
                script_error(step, "press: `until` is an object (a wait condition)");
            }
            script_load_cond(until, &s->cond, step);
            s->has_until = 1;
        }
        break;
    case SCRIPT_WALK:
        s->x = script_number(obj, "x", 0, 1, step);
        s->y = script_number(obj, "y", 0, 1, step);
        s->tol = script_number(obj, "tol", 3, 0, step);
        break;
    case SCRIPT_RESET:
        break;
    case SCRIPT_VRAM:
        s->frames = script_frames(obj, "frames", 1, 0, step);
        if (s->frames == 0) {
            script_error(step, "vram: `frames` is 0");
        }
        /* and a name, as a checkpoint */
        /* fallthrough */
    case SCRIPT_CHECKPOINT: {
        const PortJson *name = port_json_get(obj, "name");
        if (name != NULL && name->type != PORT_JSON_STRING) {
            script_error(step, "%s: `name` is a string", script_type_names[s->type]);
        }
        s->name = strdup(name != NULL ? name->string : "unnamed");
        if (s->name == NULL) {
            port_fatal("script: out of memory");
        }
        break;
    }
    }
}

void port_script_load(const char *path) {
    FILE *f;
    char *text;
    long size;
    char err[256];
    PortJson *root;
    const PortJson *steps, *name;
    long long v;
    long default_timeout;
    size_t i;

    script_path = path;
    f = fopen(path, "rb");
    if (f == NULL || fseek(f, 0, SEEK_END) != 0 || (size = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) != 0) {
        port_fatal("script %s: cannot read", path);
    }
    text = malloc((size_t)size + 1);
    if (text == NULL || fread(text, 1, (size_t)size, f) != (size_t)size) {
        port_fatal("script %s: cannot read", path);
    }
    fclose(f);
    text[size] = '\0';
    root = port_json_parse(text, (size_t)size, err, sizeof(err));
    free(text);
    if (root == NULL) {
        port_fatal("script %s: not JSON: %s", path, err);
    }
    if (root->type != PORT_JSON_OBJECT) {
        script_error(0, "the script is an object");
    }
    name = port_json_get(root, "name");
    if (name != NULL && name->type == PORT_JSON_STRING) {
        script_name = strdup(name->string);
    } else {
        /* replay.py: the file's stem */
        const char *base = strrchr(path, '/') != NULL ? strrchr(path, '/') + 1 : path;
        const char *dot = strrchr(base, '.');
        script_name = strndup(base, dot != NULL && dot != base ? (size_t)(dot - base) : strlen(base));
    }
    script_max_frames = 20000; /* run.lua's defaults */
    default_timeout = 3000;
    if (script_int(root, "max_frames", &v, 0, 0)) {
        script_max_frames = (long)v;
    }
    if (script_int(root, "default_timeout", &v, 0, 0)) {
        default_timeout = (long)v;
    }
    steps = port_json_get(root, "steps");
    if (steps == NULL || steps->type != PORT_JSON_ARRAY) {
        script_error(0, "`steps` (an array) missing");
    }
    script_step_count = (int)steps->count;
    script_steps = calloc(steps->count ? steps->count : 1, sizeof(*script_steps));
    if (script_name == NULL || script_steps == NULL) {
        port_fatal("script: out of memory");
    }
    for (i = 0; i < steps->count; i++) {
        script_load_step(&steps->items[i], &script_steps[i], default_timeout, (int)i + 1);
    }
    port_json_free(root);
    port_log("script: %s (%d steps, max %ld frames)", script_name, script_step_count, script_max_frames);
}

/* ---- Running */

static void script_state(char *buf, size_t size) {
    snprintf(buf, size, "frame=%ld stage=%d file=%d map=0x%X rnd=%d held=0x%04X", port_frames, port_state_stage(),
             port_state_file(), (u32)port_state_map(), port_state_random_index(), script_held);
}

/* run.lua's fail(): the run ends with status 5 and the message. */
static void script_fail(const char *fmt, ...) __attribute__((format(printf, 1, 2), noreturn));
static void script_fail(const char *fmt, ...) {
    char msg[256], state[128];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    script_state(state, sizeof(state));
    port_log("script: fail: %s %s", msg, state);
    port_exit(5, msg);
}

static int script_cond_met(const ScriptCond *c) {
    s32 v;
    switch (c->type) {
    case SCRIPT_WAIT_STAGE:
        return port_state_stage() == c->stage && (!c->has_word0 || port_state_slot1_word0() == c->word0);
    case SCRIPT_WAIT_MAP:
        return (u32)port_state_map() == c->map;
    case SCRIPT_WAIT_MEM:
        if (!port_state_read(c->addr, c->size, c->is_signed, &v)) {
            port_fatal("script: step %d: wait_mem: 0x%08X (size %d) is not an address the port maps (port/src/state.c "
                       "maps only the layout-identical ranges)", script_index + 1, c->addr, c->size);
        }
        /* a 4-byte unsigned read is a u32 in run.lua (read_mem), the others fit an s32 either way */
        return (c->size == 4 && !c->is_signed ? (long long)(u32)v : (long long)v) == c->value;
    default:
        return 0;
    }
}

/* With DW3_PORT_CHECKPOINT_DIR set, a checkpoint also writes gamestate_data's PS1 image (the bytes it hashes) to
 * <dir>/cpNN_<name>.bin, as run.lua names its dumps: a stable-hash mismatch is then a byte diff away. */
static void script_dump_checkpoint(const char *name) {
    static int count;
    const char *dir = getenv("DW3_PORT_CHECKPOINT_DIR");
    u8 image[PORT_GAMESTATE_PS1_SIZE];
    char path[4096];
    FILE *f;
    count++;
    if (dir == NULL || dir[0] == '\0') {
        return;
    }
    port_state_gamestate_image(image);
    snprintf(path, sizeof(path), "%s/cp%02d_%s.bin", dir, count, name);
    f = fopen(path, "wb");
    if (f == NULL || fwrite(image, 1, sizeof(image), f) != sizeof(image) || fclose(f) != 0) {
        port_fatal("script: DW3_PORT_CHECKPOINT_DIR: cannot write %s", path);
    }
}

/* A vram step's frame: the whole VRAM (1024 x 512 pixels, 1 MB, little-endian rows) appended to
 * <DW3_PORT_CHECKPOINT_DIR>/vram_<name>.bin, as run.lua appends PCSX.GPU.getVRAM() to its dump (tests/port/vram.py
 * compares them). Nothing without the variable. */
static void script_dump_vram(const char *name, int first) {
    const char *dir = getenv("DW3_PORT_CHECKPOINT_DIR");
    char path[4096];
    FILE *f;
    if (dir == NULL || dir[0] == '\0') {
        return;
    }
    snprintf(path, sizeof(path), "%s/vram_%s.bin", dir, name);
    f = fopen(path, first ? "wb" : "ab");
    if (f == NULL || fwrite(psyq_gpu_vram(), 2, 1024 * 512, f) != 1024 * 512 || fclose(f) != 0) {
        port_fatal("script: DW3_PORT_CHECKPOINT_DIR: cannot write %s", path);
    }
}

/* The field player's position in pixels (run.lua player_pos): the heap_objects entry of kind 5, key2 0, a
 * FieldstgActor, its pos (24.8) / 256. 0 outside the field / before the actor exists. */
static int script_player_pos(double *x, double *y) {
    int i;
    for (i = 0; i < (int)(sizeof(heap_objects.objects) / sizeof(heap_objects.objects[0])); i++) {
        Object *obj = heap_objects.objects[i];
        if (obj != NULL && obj->kind == SCRIPT_PLAYER_KIND && obj->key2 == 0) {
            const FieldstgActor *actor = (const FieldstgActor *)obj;
            *x = actor->pos.x / 256.0;
            *y = actor->pos.y / 256.0;
            return 1;
        }
    }
    return 0;
}

/* Runs the current step for this frame (run.lua run_step): 1 when it is complete; *instant: the next step may run in
 * the same frame. */
static int script_run_step(const ScriptStep *s, int *instant) {
    long frame = port_frames, elapsed;
    int number = script_index + 1;
    *instant = 0;
    if (script_step_started < 0) {
        char state[128];
        script_step_started = frame;
        script_state(state, sizeof(state));
        port_log("script: step %d %s %s", number, script_type_names[s->type], state);
    }
    elapsed = frame - script_step_started;
    switch (s->type) {
    case SCRIPT_CHECKPOINT:
        port_framelog_checkpoint(s->name);
        script_dump_checkpoint(s->name);
        *instant = 1;
        return 1;
    case SCRIPT_RESET:
        /* run.lua: held = {}, PCSX.hardResetEmulator() (RAM cleared, the memory cards kept), done but not instant: the
         * frame ends as any other (the released pad, the input trace, the end of the script), then port_script_frame
         * resets the console (port_reset_request: back to main(), game_main again). */
        script_held = 0;
        script_reset_pending = 1;
        return 1;
    case SCRIPT_PRESS: {
        long cycle = s->frames + s->release;
        long phase = elapsed % cycle, count = elapsed / cycle;
        if (s->has_until) {
            if (script_cond_met(&s->cond)) {
                script_held = 0;
                *instant = 1;
                return 1;
            }
            if (phase == 0 && elapsed >= s->timeout) {
                script_fail("step %d (press until %s) timed out after %ld frames", number,
                            script_type_names[s->cond.type], elapsed);
            }
        }
        if (phase < s->frames) {
            script_held = s->buttons;
            return 0;
        }
        script_held = 0;
        if (s->has_until && !s->has_repeat) {
            return 0;
        }
        return phase == cycle - 1 && count >= (s->has_repeat ? s->repeat : 1) - 1;
    }
    case SCRIPT_WALK: {
        double x, y;
        int found = script_player_pos(&x, &y);
        script_held = 0;
        if (found && fabs(s->x - x) <= s->tol && fabs(s->y - y) <= s->tol) {
            *instant = 1;
            return 1;
        }
        if (elapsed >= s->timeout) {
            char at[64];
            if (found) {
                snprintf(at, sizeof(at), "(%.14g, %.14g)", x, y);
            } else {
                snprintf(at, sizeof(at), "(nil, nil)");
            }
            script_fail("step %d (walk to %.14g,%.14g) timed out after %ld frames at %s", number, s->x, s->y,
                        elapsed, at);
        }
        if (found) {
            if (s->x - x > s->tol) {
                script_held |= SCRIPT_BUTTON_RIGHT;
            } else if (x - s->x > s->tol) {
                script_held |= SCRIPT_BUTTON_LEFT;
            }
            if (s->y - y > s->tol) {
                script_held |= SCRIPT_BUTTON_DOWN;
            } else if (y - s->y > s->tol) {
                script_held |= SCRIPT_BUTTON_UP;
            }
        }
        return 0;
    }
    case SCRIPT_WAIT_FRAMES:
        return elapsed >= s->frames - 1;
    case SCRIPT_VRAM:
        /* as wait_frames, dumping the VRAM on each of its frames */
        script_dump_vram(s->name, elapsed == 0);
        return elapsed >= s->frames - 1;
    case SCRIPT_WAIT_STAGE:
    case SCRIPT_WAIT_MAP:
    case SCRIPT_WAIT_MEM:
        if (script_cond_met(&s->cond)) {
            *instant = 1;
            return 1;
        }
        if (elapsed >= s->timeout) {
            script_fail("step %d (%s) timed out after %ld frames", number, script_type_names[s->type], elapsed);
        }
        return 0;
    }
    return 0;
}

void port_script_frame(void) {
    int budget = SCRIPT_STEP_BUDGET;
    if (port_frames > script_max_frames) {
        script_fail("max_frames reached");
    }
    while (script_index < script_step_count) {
        int instant;
        if (!script_run_step(&script_steps[script_index], &instant)) {
            break;
        }
        script_index++;
        script_step_started = -1;
        budget--;
        if (!instant || budget <= 0) {
            break;
        }
    }
    /* run.lua apply_pad: the held buttons on pad 1, every frame; the record's input trace gets the changes */
    psyq_pad_set(0, 1, script_held);
    port_framelog_input(script_held);
    if (script_index >= script_step_count) {
        port_exit(0, "script complete");
    }
    if (script_reset_pending) {
        /* the script's state (the next step, the frame count, the input trace) is outside the game: it goes on */
        script_reset_pending = 0;
        port_reset_request();
    }
}
