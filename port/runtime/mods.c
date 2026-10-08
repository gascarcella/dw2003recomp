/* The built-in mods' engine (docs/LAUNCHER.md "Mod runtime"; GAME_CONTRACT.md "4. The adapter units": the mods): the
 * registry (the runtime's own fast_forward, then the game's PortMod records, psxstack/game.h game_mods), their
 * settings (`mods.<id>`, "Mods section"), their hotkeys, their callbacks every vsync (port_mods_frame), the window's
 * status text, and the registry's JSON (--print-mods), which tests/port/settings.py checks against the manifests
 * (mods/<id>/mod.json). The mods that change the game live in the game adapter (port/game/game_mods.c) and reach the
 * game's C through its include/port.h hooks; this file names no game. */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"
#include "platform.h"
#include "port_harness.h"
#include "port_runtime.h"
#include "psxstack/mods.h"
#include "settings.h"

static const char *const mod_type_names[] = { "bool", "int", "float", "enum", "binding" };

/* ---- fast_forward (docs/LAUNCHER.md "Fast-forward"): runtime only, no game C. While it is on (the hold binding held, or the toggle pressed once)
 * the pace is the nominal rate times the speed (unlimited: no pace; the schedule starts over at each change, pump.c),
 * the window presents at most 60 images a second (every vsync is still drawn), and with `mute` the audio device's
 * queue is cleared and nothing is queued (the SPU renders on: LIBSND reads its envelopes). The game, its log and its
 * record are the unpaced run's, which they are already byte for byte (DECISIONS "The settings file").
 *
 * DW3_PORT_FAST_FORWARD=ON:OFF (a test hook, used by tests/port/settings.py; only while the mod is enabled): on for ON
 * vsyncs, off for OFF vsyncs, repeating, as if the hold key were pressed so; each change is logged with the wall
 * clock's time. */
static const char *const ff_speeds[] = { "2x", "3x", "4x", "6x", "8x", "unlimited", NULL };
static const int ff_multiples[] = { 2, 3, 4, 6, 8, 0 };
enum { FF_HOLD, FF_TOGGLE, FF_SPEED, FF_MUTE };
static const PortModOption ff_options[] = {
    { .id = "hold", .type = PORT_MOD_BINDING, .def = "\"Tab\"", .applies = "live" },
    { .id = "toggle", .type = PORT_MOD_BINDING, .def = "\"\"", .applies = "live" },
    { .id = "speed", .type = PORT_MOD_ENUM, .def = "\"4x\"", .values = ff_speeds, .applies = "live" },
    { .id = "mute", .type = PORT_MOD_BOOL, .def = "true", .applies = "live" },
};

static struct {
    int toggled;            /* the toggle binding */
    int wanted;             /* the mod's own bindings (or the test pattern) ask for it */
    int requested;          /* a game mod asks for it (port_fast_forward_request: skip_dialogues' fast_forward_waits) */
    int active;             /* applied */
    long base_pace;         /* the pace when it is off */
    long test_on, test_off; /* DW3_PORT_FAST_FORWARD */
    long long t0;           /* port_clock_ns at the start */
} ff;

static double ff_seconds(void) {
    return (double)(port_clock_ns() - ff.t0) / 1e9;
}

static void ff_start(struct PortMod *mod) {
    const char *test = getenv("DW3_PORT_FAST_FORWARD");
    (void)mod;
    if (test != NULL && sscanf(test, "%ld:%ld", &ff.test_on, &ff.test_off) == 2 && ff.test_on > 0 && ff.test_off > 0) {
        port_log("fast-forward: test pattern %ld vsyncs on, %ld off", ff.test_on, ff.test_off);
    } else {
        ff.test_on = ff.test_off = 0;
    }
}

static void ff_frame(struct PortMod *mod) {
    if (port_input_pressed(mod->values[FF_TOGGLE].action)) {
        ff.toggled = !ff.toggled;
    }
    ff.wanted = ff.toggled || port_input_held(mod->values[FF_HOLD].action);
    if (ff.test_on > 0) {
        ff.wanted |= port_frames % (ff.test_on + ff.test_off) < ff.test_on;
    }
}

void port_fast_forward_request(int on) {
    ff.requested = on;
}

static PortMod ff_mod = { .id = "fast_forward", .version = "0.1", .options = ff_options,
                          .option_count = (int)(sizeof(ff_options) / sizeof(ff_options[0])), .start = ff_start,
                          .frame = ff_frame };

/* ---- The registry: the runtime's fast_forward, then the game's mods (psxstack/game.h game_mods), in that order for
 * the settings, --print-mods and the manifests. */
#define MODS_MAX 32
static PortMod *mods_all[MODS_MAX];
static int mods_count;

static void mods_init(void) {
    int i, n = game_mod_count();
    if (mods_count != 0) {
        return;
    }
    if (n + 1 > MODS_MAX) {
        port_fatal("mods: %d game mods, at most %d", n, MODS_MAX - 1);
    }
    mods_all[0] = &ff_mod;
    for (i = 0; i < n; i++) {
        mods_all[1 + i] = &game_mods()[i];
    }
    mods_count = 1 + n;
}

static int mod_enum_index(const PortModOption *o, const char *id) {
    int i;
    for (i = 0; o->values[i] != NULL; i++) {
        if (strcmp(o->values[i], id) == 0) {
            return i;
        }
    }
    return -1;
}

/* A value of option `o` into *v; `where` names it in messages. */
static void mod_value(const PortModOption *o, const PortJson *j, const char *where, PortModValue *v) {
    switch (o->type) {
    case PORT_MOD_BOOL:
        if (j->type != PORT_JSON_BOOL) {
            port_settings_fail(where, "true or false");
        }
        v->number = j->boolean;
        break;
    case PORT_MOD_INT:
    case PORT_MOD_FLOAT:
        if (j->type != PORT_JSON_NUMBER || j->number < o->min || j->number > o->max ||
            (o->type == PORT_MOD_INT && j->number != (double)(long)j->number)) {
            port_settings_fail(where, "%s from %g to %g", o->type == PORT_MOD_INT ? "an integer" : "a number", o->min,
                               o->max);
        }
        v->number = j->number;
        break;
    case PORT_MOD_ENUM: {
        int i = j->type == PORT_JSON_STRING ? mod_enum_index(o, j->string) : -1;
        if (i < 0) {
            char list[256] = "";
            int k;
            for (k = 0; o->values[k] != NULL; k++) {
                snprintf(list + strlen(list), sizeof(list) - strlen(list), "%s\"%s\"", k ? ", " : "", o->values[k]);
            }
            port_settings_fail(where, "one of %s", list);
        }
        v->number = i;
        break;
    }
    case PORT_MOD_BINDING:
        if (j->type != PORT_JSON_STRING && j->type != PORT_JSON_ARRAY) {
            port_settings_fail(where, "a binding (a string or a list: docs/LAUNCHER.md \"Input bindings\")");
        }
        port_input_check_binding(j, where);
        v->json = j;
        break;
    }
}

/* An option with an input_toggle that is off is capped at its slider's top ("PortMod manifest": `slider_max`). */
static void mod_resolve(PortMod *mod) {
    int k, t;
    for (k = 0; k < mod->option_count; k++) {
        const PortModOption *o = &mod->options[k];
        if (o->input_toggle == NULL) {
            continue;
        }
        for (t = 0; t < mod->option_count && strcmp(mod->options[t].id, o->input_toggle) != 0; t++) {
        }
        if (t == mod->option_count || mod->options[t].type != PORT_MOD_BOOL) {
            port_fatal("mods: %s.%s: input_toggle %s is not a bool option", mod->id, o->id, o->input_toggle);
        }
        if (mod->values[t].number == 0 && mod->values[k].number > o->slider_max) {
            port_log("settings: mods.%s.%s: %g is above %g without %s, %g used", mod->id, o->id, mod->values[k].number,
                     o->slider_max, o->input_toggle, o->slider_max);
            mod->values[k].number = o->slider_max;
        }
    }
}

void port_mods_settings(const PortJson *settings) {
    int m, k;
    size_t i;
    mods_init();
    for (m = 0; m < mods_count; m++) {
        PortMod *mod = mods_all[m];
        const PortJson *s = port_json_get(settings, mod->id);
        char where[128];
        mod->enabled = 0;
        for (k = 0; k < mod->option_count; k++) {
            char err[128];
            PortJson *d = port_json_parse(mod->options[k].def, strlen(mod->options[k].def), err, sizeof(err));
            if (d == NULL) {
                port_fatal("mods: %s.%s: the default %s: %s", mod->id, mod->options[k].id, mod->options[k].def, err);
            }
            snprintf(where, sizeof(where), "(default) mods.%s.%s", mod->id, mod->options[k].id);
            memset(&mod->values[k], 0, sizeof(mod->values[k]));
            mod->values[k].action = -1;
            mod_value(&mod->options[k], d, where, &mod->values[k]);
            mod->values[k].json = NULL; /* a binding's default stays the text */
            port_json_free(d);
        }
        if (s == NULL) {
            continue;
        }
        snprintf(where, sizeof(where), "mods.%s", mod->id);
        if (s->type != PORT_JSON_OBJECT) {
            port_settings_fail(where, "an object ({ \"enabled\": ..., options })");
        }
        for (i = 0; i < s->count; i++) {
            snprintf(where, sizeof(where), "mods.%s.%s", mod->id, s->keys[i]);
            if (strcmp(s->keys[i], "enabled") == 0) {
                if (s->items[i].type != PORT_JSON_BOOL) {
                    port_settings_fail(where, "true or false");
                }
                mod->enabled = s->items[i].boolean;
                continue;
            }
            for (k = 0; k < mod->option_count && strcmp(mod->options[k].id, s->keys[i]) != 0; k++) {
            }
            if (k == mod->option_count) {
                port_log("settings: %s: unknown option, ignored", where);
                continue;
            }
            mod_value(&mod->options[k], &s->items[i], where, &mod->values[k]);
        }
        mod_resolve(mod);
    }
    for (i = 0; settings != NULL && i < settings->count; i++) {
        for (m = 0; m < mods_count && strcmp(mods_all[m]->id, settings->keys[i]) != 0; m++) {
        }
        if (m == mods_count) {
            port_log("settings: mods.%s: no such mod in this build, ignored", settings->keys[i]);
        }
    }
}

void port_mods_start(int active) {
    int m, k;
    mods_init();
    ff.base_pace = port_pace_get();
    ff.t0 = port_clock_ns();
    for (m = 0; m < mods_count; m++) {
        PortMod *mod = mods_all[m];
        if (!active) {
            mod->enabled = 0;
        }
        if (!mod->enabled) {
            continue;
        }
        for (k = 0; k < mod->option_count; k++) {
            if (mod->options[k].type == PORT_MOD_BINDING) {
                char where[128], *name;
                snprintf(where, sizeof(where), "mods.%s.%s", mod->id, mod->options[k].id);
                name = strdup(where + 5);
                if (name == NULL) {
                    port_fatal("mods: out of memory");
                }
                mod->values[k].action = port_input_action(name, mod->values[k].json, where, mod->options[k].def);
            }
        }
        port_log("mods: %s on", mod->id);
        if (mod->start != NULL) {
            mod->start(mod);
        }
    }
}

/* The window's title: the mods that are on (fast-forward, then every enabled mod's status text). Set when it changes. */
static void mods_status(void) {
    static char last[128];
    char status[128];
    int m, n;
    n = snprintf(status, sizeof(status), "%s",
                 ff.active ? (ff_multiples[(int)ff_mod.values[FF_SPEED].number] > 0 ? "fast-forward" : "fast-forward, unlimited")
                           : "");
    for (m = 1; m < mods_count; m++) {
        const char *text = mods_all[m]->enabled && mods_all[m]->status != NULL ? mods_all[m]->status(mods_all[m]) : NULL;
        if (text != NULL && n < (int)sizeof(status)) {
            n += snprintf(status + n, sizeof(status) - (size_t)n, "%s%s", n ? ", " : "", text);
        }
    }
    if (strcmp(status, last) != 0) {
        snprintf(last, sizeof(last), "%s", status);
        port_video_set_status(status);
    }
}

/* Fast-forward on or off: the mod's bindings, or skip_dialogues' request (with fast_forward's speed and mute). */
static void ff_apply(void) {
    const PortMod *f = &ff_mod;
    int on = (f->enabled && ff.wanted) || ff.requested;
    int mult = ff_multiples[(int)f->values[FF_SPEED].number];
    if (on == ff.active) {
        return;
    }
    ff.active = on;
    port_pace_set(on ? (mult > 0 ? port_rate * mult : 0) : ff.base_pace);
    port_video_set_present_cap(on ? 60 : 0);
    port_audio_set_mute(on && f->values[FF_MUTE].number != 0);
    port_log("fast-forward: %s at frame %ld, %.3f s (pace %ld)%s", on ? "on" : "off", port_frames, ff_seconds(),
             port_pace_get(), ff.requested && !(f->enabled && ff.wanted) ? " (a cutscene's waits)" : "");
}

void port_mods_frame(void) {
    int m;
    for (m = 0; m < mods_count; m++) {
        if (mods_all[m]->enabled && mods_all[m]->frame != NULL) {
            mods_all[m]->frame(mods_all[m]);
        }
    }
    ff_apply();
    mods_status();
}

static void mod_print_value(FILE *f, const PortModOption *o, const PortModValue *v) {
    switch (o->type) {
    case PORT_MOD_BOOL:
        fputs(v->number != 0 ? "true" : "false", f);
        break;
    case PORT_MOD_INT:
        fprintf(f, "%lld", (long long)v->number);
        break;
    case PORT_MOD_FLOAT:
        fprintf(f, "%.17g", v->number);
        break;
    case PORT_MOD_ENUM:
        port_json_write_string(f, o->values[(int)v->number]);
        break;
    case PORT_MOD_BINDING:
        if (v->json != NULL) {
            port_json_write(f, v->json, 0, 0);
        } else {
            fputs(o->def, f);
        }
        break;
    }
}

/* The resolved `mods` object: every registered mod with `enabled` and every option, then the settings' unknown mods
 * as they were given (`settings`: the settings' `mods`, or NULL). */
static void mods_print(FILE *f, int level, const PortJson *settings) {
    int m, k, first = 1;
    size_t i;
    int in = 2 * (level + 1);
    fputs("{", f);
    for (m = 0; m < mods_count; m++) {
        const PortMod *mod = mods_all[m];
        fprintf(f, "%s\n%*s\"%s\": {\n%*s\"enabled\": %s", first ? "" : ",", in, "", mod->id, in + 2, "",
                mod->enabled ? "true" : "false");
        first = 0;
        for (k = 0; k < mod->option_count; k++) {
            fprintf(f, ",\n%*s\"%s\": ", in + 2, "", mod->options[k].id);
            mod_print_value(f, &mod->options[k], &mod->values[k]);
        }
        fprintf(f, "\n%*s}", in, "");
    }
    for (i = 0; settings != NULL && i < settings->count; i++) {
        for (m = 0; m < mods_count && strcmp(mods_all[m]->id, settings->keys[i]) != 0; m++) {
        }
        if (m == mods_count) {
            fprintf(f, ",\n%*s", in, "");
            port_json_write_string(f, settings->keys[i]);
            fputs(": ", f);
            port_json_write(f, &settings->items[i], 2, level + 1);
        }
    }
    fprintf(f, "\n%*s}", 2 * level, "");
}

void port_mods_print(FILE *f, int level) {
    mods_print(f, level, port_settings.mods);
}

void port_mods_print_registry(FILE *f) {
    int m, k, v;
    mods_init();
    fputs("[", f);
    for (m = 0; m < mods_count; m++) {
        const PortMod *mod = mods_all[m];
        fprintf(f, "%s\n  { \"id\": \"%s\", \"version\": \"%s\", \"kind\": \"builtin\", \"requires_port\": %d, "
                   "\"options\": [", m ? "," : "", mod->id, mod->version, PSXSTACK_API);
        for (k = 0; k < mod->option_count; k++) {
            const PortModOption *o = &mod->options[k];
            fprintf(f, "%s\n    { \"id\": \"%s\", \"type\": \"%s\", \"default\": %s, \"applies\": \"%s\"", k ? "," : "",
                    o->id, mod_type_names[o->type], o->def, o->applies);
            if (o->type == PORT_MOD_INT || o->type == PORT_MOD_FLOAT) {
                fprintf(f, ", \"min\": %.17g, \"max\": %.17g, \"step\": %.17g", o->min, o->max, o->step);
            }
            if (o->input_toggle != NULL) {
                fprintf(f, ", \"slider_max\": %.17g, \"input_toggle\": \"%s\"", o->slider_max, o->input_toggle);
            }
            if (o->type == PORT_MOD_ENUM) {
                fputs(", \"values\": [", f);
                for (v = 0; o->values[v] != NULL; v++) {
                    fprintf(f, "%s\"%s\"", v ? ", " : "", o->values[v]);
                }
                fputs("]", f);
            }
            fputs(" }", f);
        }
        fputs("\n  ] }", f);
    }
    fputs("\n]\n", f);
}
