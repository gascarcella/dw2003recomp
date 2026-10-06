/* The built-in mods (docs/LAUNCHER_MODS_PLAN.md 4.4, 4.5; port_harness.h "mods.c"): the registry, their option values
 * from the settings (`mods.<id>`), their hotkeys, and port_mods_frame, called by pump.c at every vsync.
 *
 * Each mod has a manifest, port/mods/<id>/mod.json, which the launcher reads to list the mods and draw their screens
 * (names, descriptions, labels: the user-facing text lives there only). The registry below holds what the game needs:
 * the ids, the option types, defaults and ranges; `dw2003 --print-mods` prints it and tests/port/settings.py requires
 * it to equal the manifests. A mod that the settings do not enable is off; under --script every mod is off unless the
 * run asks for them (--script-mods), so the replays, the goldens and the records stay the bare binary's.
 *
 * Rules for a mod that changes the game's behaviour (4.5): its hook in the game's C sits in an `#ifdef PC_PORT` block
 * testing a port_mod_* flag (never an expression that is constant on the PS1), its state lives here, and a game
 * global it sets is set before port_overlay_init() (the reset's snapshot). */
#include <stdlib.h>
#include <string.h>

#include "json.h"
#include "port_harness.h"
#include "port_runtime.h"
#include "settings.h"

/* The game's mod interface: a manifest's `requires_port` (4.4) must not be higher. */
#define PORT_MODS_API 1

typedef enum ModType { MOD_BOOL, MOD_INT, MOD_FLOAT, MOD_ENUM, MOD_BINDING } ModType;
static const char *const mod_type_names[] = { "bool", "int", "float", "enum", "binding" };

typedef struct ModOption {
    const char *id;
    ModType type;
    const char *def;            /* the default, as JSON text (4.3's grammar for a binding) */
    double min, max, step;      /* int, float */
    const char *const *values;  /* enum: the value ids, NULL-terminated */
    const char *applies;        /* "live" or "restart" */
} ModOption;

#define MOD_MAX_OPTIONS 8

typedef struct ModValue {
    double number;          /* bool (0/1), int, float, enum (the value's index) */
    const PortJson *json;   /* binding: the settings' value; NULL: the default */
    int action;             /* binding: the hotkey's action id once started (-1: none) */
} ModValue;

typedef struct Mod {
    const char *id;
    const char *version;
    const ModOption *options;
    int option_count;
    void (*frame)(struct Mod *mod); /* every vsync while enabled; NULL: none */
    int enabled;
    ModValue values[MOD_MAX_OPTIONS];
} Mod;

/* ---- fast_forward (5.1; its frame work is phase 2's) */
static const char *const ff_speeds[] = { "2x", "3x", "4x", "6x", "8x", "unlimited", NULL };
static const ModOption ff_options[] = {
    { .id = "hold", .type = MOD_BINDING, .def = "\"Tab\"", .applies = "live" },
    { .id = "toggle", .type = MOD_BINDING, .def = "\"\"", .applies = "live" },
    { .id = "speed", .type = MOD_ENUM, .def = "\"4x\"", .values = ff_speeds, .applies = "live" },
    { .id = "mute", .type = MOD_BOOL, .def = "true", .applies = "live" },
};

static Mod mods[] = {
    { .id = "fast_forward", .version = "0.1", .options = ff_options,
      .option_count = (int)(sizeof(ff_options) / sizeof(ff_options[0])) },
};
#define MOD_COUNT ((int)(sizeof(mods) / sizeof(mods[0])))

static int mod_enum_index(const ModOption *o, const char *id) {
    int i;
    for (i = 0; o->values[i] != NULL; i++) {
        if (strcmp(o->values[i], id) == 0) {
            return i;
        }
    }
    return -1;
}

/* A value of option `o` into *v; `where` names it in messages. */
static void mod_value(const ModOption *o, const PortJson *j, const char *where, ModValue *v) {
    switch (o->type) {
    case MOD_BOOL:
        if (j->type != PORT_JSON_BOOL) {
            port_settings_fail(where, "true or false");
        }
        v->number = j->boolean;
        break;
    case MOD_INT:
    case MOD_FLOAT:
        if (j->type != PORT_JSON_NUMBER || j->number < o->min || j->number > o->max ||
            (o->type == MOD_INT && j->number != (double)(long)j->number)) {
            port_settings_fail(where, "%s from %g to %g", o->type == MOD_INT ? "an integer" : "a number", o->min,
                               o->max);
        }
        v->number = j->number;
        break;
    case MOD_ENUM: {
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
    case MOD_BINDING:
        if (j->type != PORT_JSON_STRING && j->type != PORT_JSON_ARRAY) {
            port_settings_fail(where, "a binding (a string or a list: docs/LAUNCHER_MODS_PLAN.md 4.3)");
        }
        port_input_check_binding(j, where);
        v->json = j;
        break;
    }
}

void port_mods_settings(const PortJson *settings) {
    int m, k;
    size_t i;
    for (m = 0; m < MOD_COUNT; m++) {
        Mod *mod = &mods[m];
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
    }
    for (i = 0; settings != NULL && i < settings->count; i++) {
        for (m = 0; m < MOD_COUNT && strcmp(mods[m].id, settings->keys[i]) != 0; m++) {
        }
        if (m == MOD_COUNT) {
            port_log("settings: mods.%s: no such mod in this build, ignored", settings->keys[i]);
        }
    }
}

void port_mods_start(int active) {
    int m, k;
    for (m = 0; m < MOD_COUNT; m++) {
        Mod *mod = &mods[m];
        if (!active) {
            mod->enabled = 0;
        }
        if (!mod->enabled) {
            continue;
        }
        for (k = 0; k < mod->option_count; k++) {
            if (mod->options[k].type == MOD_BINDING) {
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
    }
}

void port_mods_frame(void) {
    int m;
    for (m = 0; m < MOD_COUNT; m++) {
        if (mods[m].enabled && mods[m].frame != NULL) {
            mods[m].frame(&mods[m]);
        }
    }
}

static void mod_print_value(FILE *f, const ModOption *o, const ModValue *v) {
    switch (o->type) {
    case MOD_BOOL:
        fputs(v->number != 0 ? "true" : "false", f);
        break;
    case MOD_INT:
        fprintf(f, "%lld", (long long)v->number);
        break;
    case MOD_FLOAT:
        fprintf(f, "%.17g", v->number);
        break;
    case MOD_ENUM:
        port_json_write_string(f, o->values[(int)v->number]);
        break;
    case MOD_BINDING:
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
    for (m = 0; m < MOD_COUNT; m++) {
        fprintf(f, "%s\n%*s\"%s\": {\n%*s\"enabled\": %s", first ? "" : ",", in, "", mods[m].id, in + 2, "",
                mods[m].enabled ? "true" : "false");
        first = 0;
        for (k = 0; k < mods[m].option_count; k++) {
            fprintf(f, ",\n%*s\"%s\": ", in + 2, "", mods[m].options[k].id);
            mod_print_value(f, &mods[m].options[k], &mods[m].values[k]);
        }
        fprintf(f, "\n%*s}", in, "");
    }
    for (i = 0; settings != NULL && i < settings->count; i++) {
        for (m = 0; m < MOD_COUNT && strcmp(mods[m].id, settings->keys[i]) != 0; m++) {
        }
        if (m == MOD_COUNT) {
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
    fputs("[", f);
    for (m = 0; m < MOD_COUNT; m++) {
        fprintf(f, "%s\n  { \"id\": \"%s\", \"version\": \"%s\", \"kind\": \"builtin\", \"requires_port\": %d, "
                   "\"options\": [", m ? "," : "", mods[m].id, mods[m].version, PORT_MODS_API);
        for (k = 0; k < mods[m].option_count; k++) {
            const ModOption *o = &mods[m].options[k];
            fprintf(f, "%s\n    { \"id\": \"%s\", \"type\": \"%s\", \"default\": %s, \"applies\": \"%s\"", k ? "," : "",
                    o->id, mod_type_names[o->type], o->def, o->applies);
            if (o->type == MOD_INT || o->type == MOD_FLOAT) {
                fprintf(f, ", \"min\": %.17g, \"max\": %.17g, \"step\": %.17g", o->min, o->max, o->step);
            }
            if (o->type == MOD_ENUM) {
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
