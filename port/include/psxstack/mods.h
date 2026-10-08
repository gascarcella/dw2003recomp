/* psxstack/mods.h: the shape of a built-in mod (GAME_CONTRACT.md "4. The adapter units": the mods). The game's adapter
 * defines its mods as PortMod records (id, version, options, start/frame callbacks) and lists them through
 * game_mods()/game_mod_count() (psxstack/game.h); the runtime's engine (port/runtime/mods.c) reads their settings
 * (`mods.<id>`, docs/LAUNCHER.md "Mods section"), registers their hotkeys, runs their callbacks every vsync and prints
 * the registry (--print-mods), which must equal the manifests (mods/<id>/mod.json, "Mod manifest"). */
#ifndef PSXSTACK_MODS_H
#define PSXSTACK_MODS_H

#include "json.h"

typedef enum PortModType { PORT_MOD_BOOL, PORT_MOD_INT, PORT_MOD_FLOAT, PORT_MOD_ENUM, PORT_MOD_BINDING } PortModType;

typedef struct PortModOption {
    const char *id;
    PortModType type;
    const char *def;            /* the default, as JSON text (the settings' grammar for a binding) */
    double min, max, step;      /* int, float */
    double slider_max;          /* int, float with input_toggle: the slider's top (below max); 0: none */
    const char *input_toggle;   /* the bool option that allows values above slider_max; NULL: none */
    const char *const *values;  /* enum: the value ids, NULL-terminated */
    const char *applies;        /* "live" or "restart" */
} PortModOption;

#define PORT_MOD_MAX_OPTIONS 8

typedef struct PortModValue {
    double number;          /* bool (0/1), int, float, enum (the value's index) */
    const PortJson *json;   /* binding: the settings' value; NULL: the default */
    int action;             /* binding: the hotkey's action id once started (-1: none) */
} PortModValue;

typedef struct PortMod {
    const char *id;
    const char *version;
    const PortModOption *options;
    int option_count;
    void (*start)(struct PortMod *mod); /* once, when enabled, after its hotkeys are registered; NULL: none */
    void (*frame)(struct PortMod *mod); /* every vsync while enabled; NULL: none */
    const char *(*status)(struct PortMod *mod); /* the window title's text while on ("skip dialogues"), NULL for none */
    int enabled;
    PortModValue values[PORT_MOD_MAX_OPTIONS];
} PortMod;

#endif /* PSXSTACK_MODS_H */
