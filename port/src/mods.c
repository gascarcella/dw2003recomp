/* The built-in mods (docs/LAUNCHER.md "Mod manifest", "Mod runtime"; port_harness.h "mods.c"): the registry, their option values
 * from the settings (`mods.<id>`), their hotkeys, and port_mods_frame, called by pump.c at every vsync.
 *
 * Each mod has a manifest, port/mods/<id>/mod.json, which the launcher reads to list the mods and draw their screens
 * (names, descriptions, labels: the user-facing text lives there only). The registry below holds what the game needs:
 * the ids, the option types, defaults and ranges; `dw2003 --print-mods` prints it and tests/port/settings.py requires
 * it to equal the manifests. A mod that the settings do not enable is off; under --script every mod is off unless the
 * run asks for them (--script-mods), so the replays, the goldens and the records stay the bare binary's.
 *
 * Rules for a mod that changes the game's behaviour (docs/LAUNCHER.md "Mod runtime"): its hook in the game's C sits in an `#ifdef PC_PORT` block
 * testing a port_mod_* flag (never an expression that is constant on the PS1), its state lives here, and a game
 * global it sets is set before port_overlay_init() (the reset's snapshot). */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "battle_scan.h"
#include "fieldstg.h"
#include "gamestate.h"
#include "json.h"
#include "port_harness.h"
#include "port_runtime.h"
#include "settings.h"

#define PORT_BATTLE_SCAN_MAX 4096 /* words: the longest script on the disc has 567 */

/* The game's mod interface: a manifest's `requires_port` ("Mod manifest") must not be higher. */
#define PORT_MODS_API 1

typedef enum ModType { MOD_BOOL, MOD_INT, MOD_FLOAT, MOD_ENUM, MOD_BINDING } ModType;
static const char *const mod_type_names[] = { "bool", "int", "float", "enum", "binding" };

typedef struct ModOption {
    const char *id;
    ModType type;
    const char *def;            /* the default, as JSON text (the settings' grammar for a binding) */
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
    void (*start)(struct Mod *mod); /* once, when enabled, after its hotkeys are registered; NULL: none */
    void (*frame)(struct Mod *mod); /* every vsync while enabled; NULL: none */
    int enabled;
    ModValue values[MOD_MAX_OPTIONS];
} Mod;

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
static const ModOption ff_options[] = {
    { .id = "hold", .type = MOD_BINDING, .def = "\"Tab\"", .applies = "live" },
    { .id = "toggle", .type = MOD_BINDING, .def = "\"\"", .applies = "live" },
    { .id = "speed", .type = MOD_ENUM, .def = "\"4x\"", .values = ff_speeds, .applies = "live" },
    { .id = "mute", .type = MOD_BOOL, .def = "true", .applies = "live" },
};

static struct {
    int toggled;            /* the toggle binding */
    int wanted;             /* the mod's own bindings (or the test pattern) ask for it */
    int requested;          /* skip_dialogues' fast_forward_waits asks for it (a field event runs) */
    int active;             /* applied */
    long base_pace;         /* the pace when it is off */
    long test_on, test_off; /* DW3_PORT_FAST_FORWARD */
    struct timespec t0;
} ff;

static double ff_seconds(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double)(now.tv_sec - ff.t0.tv_sec) + (double)(now.tv_nsec - ff.t0.tv_nsec) / 1e9;
}

static void ff_start(struct Mod *mod) {
    const char *test = getenv("DW3_PORT_FAST_FORWARD");
    (void)mod;
    if (test != NULL && sscanf(test, "%ld:%ld", &ff.test_on, &ff.test_off) == 2 && ff.test_on > 0 && ff.test_off > 0) {
        port_log("fast-forward: test pattern %ld vsyncs on, %ld off", ff.test_on, ff.test_off);
    } else {
        ff.test_on = ff.test_off = 0;
    }
}

static void ff_frame(struct Mod *mod) {
    if (port_input_pressed(mod->values[FF_TOGGLE].action)) {
        ff.toggled = !ff.toggled;
    }
    ff.wanted = ff.toggled || port_input_held(mod->values[FF_HOLD].action);
    if (ff.test_on > 0) {
        ff.wanted |= port_frames % (ff.test_on + ff.test_off) < ff.test_on;
    }
}

/* ---- skip_dialogues (docs/LAUNCHER.md "Skip dialogues"): while it is on (the toggle pressed once, or the hold binding held) the game's C hooks
 * (port_mod_skip_dialogues, include/port.h) show every revealing message window's page at once and go on from its
 * confirm waits and the battle's message waits by themselves; choices, menus, name entry and the other overlays'
 * code-driven prompts are separate code and still wait for the player. With `fast_forward_waits` it also asks for
 * fast-forward (fast_forward's speed and mute, its defaults when that mod is off) while FIELDSTG's event VM runs
 * (fieldstg_stage.event_running: the scripted waits, walks and bubble animations); free walking is not sped up.
 *
 * DW3_PORT_SKIP_DIALOGUES=1 (a test hook, tests/port/mods.py; only while the mod is enabled): on from the start, as if
 * the toggle had been pressed. */
enum { SD_TOGGLE, SD_HOLD, SD_FF_WAITS };
static const ModOption sd_options[] = {
    { .id = "toggle", .type = MOD_BINDING, .def = "\"F2\"", .applies = "live" },
    { .id = "hold", .type = MOD_BINDING, .def = "\"\"", .applies = "live" },
    { .id = "fast_forward_waits", .type = MOD_BOOL, .def = "false", .applies = "live" },
};
int port_mod_skip_dialogues;
static int sd_toggled;

static void sd_start(struct Mod *mod) {
    const char *test = getenv("DW3_PORT_SKIP_DIALOGUES");
    (void)mod;
    if (test != NULL && strcmp(test, "1") == 0) {
        sd_toggled = 1;
        port_log("skip dialogues: on from the start (DW3_PORT_SKIP_DIALOGUES)");
    }
}

static void sd_frame(struct Mod *mod) {
    const PortOverlay *field = port_overlay_current(1);
    int on;
    if (port_input_pressed(mod->values[SD_TOGGLE].action)) {
        sd_toggled = !sd_toggled;
    }
    on = sd_toggled || port_input_held(mod->values[SD_HOLD].action);
    if (on != port_mod_skip_dialogues) {
        port_mod_skip_dialogues = on;
        port_log("skip dialogues: %s at frame %ld", on ? "on" : "off", port_frames);
    }
    /* fieldstg_stage is FIELDSTG's: valid only while FIELDSTG is the tier-1 overlay */
    ff.requested = on && mod->values[SD_FF_WAITS].number != 0 && field != NULL &&
                   strcmp(field->name, "FIELDSTG") == 0 && fieldstg_stage.event_running != 0;
}

/* ---- battle_animations (docs/LAUNCHER.md "Disable battle animations"): while enabled, port_mod_battle_animations makes fightstg_script_update ask
 * port_battle_cut at the INIT of every script 5 and up (the attacks', techniques' and items' animations; the rules
 * already ran): one with a child command becomes the target's reaction (results[3] + 1: flinch, heavy hit, KO, dodge)
 * with `hit_reaction` (the default), else ends at once; one without ends at once. A knock-out keeps its KO reaction in
 * both cases (it ends on the KO pose, animation 10). The hit sound the script would have played is played. A script
 * that does not scan to its end (none on the disc: tests/port/battle.py) is left to run. */
enum { BA_HIT_REACTION };
static const ModOption ba_options[] = {
    { .id = "hit_reaction", .type = MOD_BOOL, .def = "true", .applies = "live" },
};
int port_mod_battle_animations;
static int ba_hit_reaction;

static void ba_start(struct Mod *mod) {
    port_mod_battle_animations = 1;
    ba_hit_reaction = mod->values[BA_HIT_REACTION].number != 0;
}

s32 port_battle_cut(const s16 *stream, s32 stage, const s32 *results, s32 hit_sound, s32 *sound_id, s32 *sound_arg) {
    PortBattleScan scan;
    s32 reaction = results[3] + 1;
    *sound_id = *sound_arg = 0;
    port_battle_scan(stream, PORT_BATTLE_SCAN_MAX, stage, &scan);
    if (!scan.ok || reaction < 1 || reaction > 4) {
        return -1;
    }
    if (scan.sound != 0) {
        /* fightstg_script_run_sound's choice for its first hit-sound command */
        *sound_id = (scan.sound == 0x62 ? results[0] : results[3]) == 3 ? 0x38 : hit_sound;
        *sound_arg = scan.sound_arg;
    }
    if (scan.child && (ba_hit_reaction || reaction == 3)) {
        port_log("battle animations: frame %ld: a script of %d words cut%s; the target's reaction %d%s", port_frames,
                 scan.length, scan.multi ? " (multi-hit)" : "", reaction, *sound_id ? ", its hit sound" : "");
        return reaction;
    }
    port_log("battle animations: frame %ld: a script of %d words cut%s; ended%s", port_frames, scan.length,
             scan.child ? "" : " (no reaction in it)", *sound_id ? ", its hit sound" : "");
    return 0;
}

/* ---- global_save (docs/LAUNCHER.md "Save anywhere"): while enabled, port_mod_global_save gives the field's menu (fieldmenu.c) a last
 * entry, SAVE, which opens STGMCARD in save mode from any field map (port_global_save_open: the map change the inns'
 * event scripts make, with the inn's 0xC map when the map is an inn's, else 0xC00 and no place name in the slot
 * summary). The save then carries the map's state in the slot's unused tail (GsRecord at slot offset 0x26C4, outside
 * the game's checksum: a PS1 or the emulator ignores it), and a load with `restore_map_state` puts it back and marks
 * the map as revisited (field_last_map = field_map), so FIELDSTG resumes it as after a Back from the save screen
 * instead of as a fresh entry (which resets the attribute layer, the depth and height, the per-visit flags). */
enum { GS_RESTORE };
static const ModOption gs_options[] = {
    { .id = "restore_map_state", .type = MOD_BOOL, .def = "true", .applies = "live" },
};
int port_mod_global_save;
static int gs_restore;
static s32 gs_opened_from; /* the field map SAVE was chosen on, until STGMCARD asks; 0: STGMCARD came the game's way */

/* The inns' field maps and the 0xC map their event script goes to (each inn stage file of src/wstag: FIELDSTG_EVENT_GOTO_MAP(0xC..)):
 * STGMCARD names the place by the 0xC map's low byte (stgmcard_map_names). */
static const struct { s16 map, inn; } gs_inns[] = {
    { 0x20A, 0xC01 }, { 0x223, 0xC12 }, { 0x230, 0xC02 }, { 0x238, 0xC03 }, { 0x23F, 0xC04 }, { 0x247, 0xC05 },
    { 0x249, 0xC14 }, { 0x25B, 0xC13 }, { 0x25D, 0xC06 }, { 0x263, 0xC15 }, { 0x269, 0xC07 }, { 0x26D, 0xC08 },
    { 0x26F, 0xC09 }, { 0x279, 0xC0A }, { 0x292, 0xC0B }, { 0x29E, 0xC0C }, { 0x2A5, 0xC16 }, { 0x2AC, 0xC0D },
    { 0x2B1, 0xC0E }, { 0x2B3, 0xC18 }, { 0x2C4, 0xC17 }, { 0x2C6, 0xC0F }, { 0x2CB, 0xC19 }, { 0x2D1, 0xC10 },
    { 0x2D6, 0xC11 },
};

/* The map's state in the slot's tail. The slot is 0x26C4 bytes of a 0x2700-byte part; the game writes the whole part
 * from its buffer, so the tail (0x3C bytes) reaches the card and comes back with the slot. */
#define GS_RECORD_OFFSET 0x26C4
#define GS_RECORD_VERSION 1
typedef struct GsRecord {
    u8 magic[4];      /* "GSAV" */
    u8 version;       /* GS_RECORD_VERSION */
    u8 checksum;      /* XOR of the bytes from map_flags on */
    u8 pad[2];
    u8 map_flags[3];  /* gamestate_flags.map_flags: the type-0 (per-visit) flags, cleared on a fresh entry */
    u8 pad2;
    s32 attr_layer;   /* gamestate_data's fields after the slot that a fresh entry resets */
    s32 unk_26E4;
    s32 player_depth;
    s32 spot_target;
    s32 screen_white;
    s32 player_height;
    s32 meter_random_count;
} GsRecord; /* size 0x28 */

static s32 gs_inn_map(s32 map) {
    size_t i;
    for (i = 0; i < sizeof(gs_inns) / sizeof(gs_inns[0]); i++) {
        if (gs_inns[i].map == map) {
            return gs_inns[i].inn;
        }
    }
    return 0;
}

static u8 gs_checksum(const GsRecord *r) {
    const u8 *b = (const u8 *)r;
    u8 sum = 0;
    size_t i;
    for (i = offsetof(GsRecord, map_flags); i < sizeof(*r); i++) {
        sum ^= b[i];
    }
    return sum;
}

static void gs_start(struct Mod *mod) {
    port_mod_global_save = 1;
    gs_restore = mod->values[GS_RESTORE].number != 0;
}

void port_global_save_open(void) {
    s32 map = gamestate_data.field_map, inn = gs_inn_map(map);
    gs_opened_from = map;
    gamestate_data.funcs.set_next_map(inn != 0 ? inn : 0xC00, -1);
    port_log("global save: frame %ld: the save screen from map 0x%X%s", port_frames, map,
             inn != 0 ? " (an inn's map)" : "");
}

s32 port_global_save_map_name(s32 map_name) {
    s32 from = gs_opened_from;
    gs_opened_from = 0;
    if (from == 0 || gs_inn_map(from) != 0) {
        return map_name;
    }
    return 0; /* ?SHPNAM entry 0, empty: the area line (stgmcard_map_areas, by the field map) still says where */
}

void port_global_save_record(void *slot) {
    GsRecord r;
    memset(&r, 0, sizeof(r));
    memcpy(r.magic, "GSAV", 4);
    r.version = GS_RECORD_VERSION;
    memcpy(r.map_flags, gamestate_flags.map_flags, sizeof(r.map_flags));
    r.attr_layer = gamestate_data.attr_layer;
    r.unk_26E4 = gamestate_data.unk_26E4;
    r.player_depth = gamestate_data.player_depth;
    r.spot_target = gamestate_data.spot_target;
    r.screen_white = gamestate_data.screen_white;
    r.player_height = gamestate_data.player_height;
    r.meter_random_count = gamestate_data.meter_random_count;
    r.checksum = gs_checksum(&r);
    memcpy((u8 *)slot + GS_RECORD_OFFSET, &r, sizeof(r));
    port_log("global save: frame %ld: the map's state saved with the slot (map 0x%X, layer %d, depth %d)", port_frames,
             gamestate_data.field_map, r.attr_layer, r.player_depth);
}

void port_global_save_restore(const void *slot) {
    GsRecord r;
    memcpy(&r, (const u8 *)slot + GS_RECORD_OFFSET, sizeof(r));
    if (memcmp(r.magic, "GSAV", 4) != 0 || r.version != GS_RECORD_VERSION || r.checksum != gs_checksum(&r)) {
        port_log("global save: frame %ld: the slot carries no map state: a plain load", port_frames);
        return;
    }
    if (!gs_restore) {
        port_log("global save: frame %ld: the slot's map state not restored (restore_map_state off)", port_frames);
        return;
    }
    memcpy(gamestate_flags.map_flags, r.map_flags, sizeof(r.map_flags));
    gamestate_data.attr_layer = r.attr_layer;
    gamestate_data.unk_26E4 = r.unk_26E4;
    gamestate_data.player_depth = r.player_depth;
    gamestate_data.spot_target = r.spot_target;
    gamestate_data.screen_white = r.screen_white;
    gamestate_data.player_height = r.player_height;
    gamestate_data.meter_random_count = r.meter_random_count;
    /* FIELDSTG's fieldstg_update_main: the map is new (a fresh entry) when field_last_map differs; equal, it is a
     * return to it, which keeps what was restored */
    gamestate_data.field_last_map = gamestate_data.field_map;
    port_log("global save: frame %ld: the map's state restored (map 0x%X, layer %d, depth %d, height 0x%X)", port_frames,
             gamestate_data.field_map, r.attr_layer, r.player_depth, r.player_height);
}

/* ---- party_xp (docs/LAUNCHER.md "Party experience"; docs/MECHANICS.md section 6, "Who gets a battle's experience"):
 * while enabled, port_mod_party_xp makes STFGTREP's report ask port_party_xp_share for every party member's
 * experience. A member that took part keeps one fighter's split share; one that did not gets `share` percent of it
 * (at least 1 when both are above 0), a knocked-out one (port_party_xp_knocked_out, from WFIGHTMN) only with
 * `knocked_out`. With `catch_up` a member below the party's highest level gets 10% more per level below it, at most
 * twice as much. The report then shows, counts and adds it as it does a fighter's (item 0x141's fifth included); form
 * experience still goes only to the forms that fought. */
enum { PX_SHARE, PX_KNOCKED_OUT, PX_CATCH_UP };
static const ModOption px_options[] = {
    { .id = "share", .type = MOD_INT, .def = "50", .min = 0, .max = 100, .step = 5, .applies = "live" },
    { .id = "knocked_out", .type = MOD_BOOL, .def = "false", .applies = "live" },
    { .id = "catch_up", .type = MOD_BOOL, .def = "false", .applies = "live" },
};
int port_mod_party_xp;
static int px_share, px_knocked_out, px_catch_up;
static int px_ko[3]; /* the last battle's party slots at 0 HP */

static void px_start(struct Mod *mod) {
    port_mod_party_xp = 1;
    px_share = (int)mod->values[PX_SHARE].number;
    px_knocked_out = mod->values[PX_KNOCKED_OUT].number != 0;
    px_catch_up = mod->values[PX_CATCH_UP].number != 0;
}

void port_party_xp_knocked_out(s32 slot, s32 knocked_out) {
    if (slot >= 0 && slot < 3) {
        px_ko[slot] = knocked_out;
    }
}

static s32 px_level(s32 slot) {
    s32 id = gamestate_data.funcs.get_party_member(slot);
    return id >= 0 ? gamestate_data.digimon[id].record.stats.values[0] : -1;
}

s32 port_party_xp_share(s32 slot, s32 took_part, s32 exp) {
    const char *how = took_part ? "took part" : px_ko[slot] ? "knocked out" : "sat out";
    s32 gain = exp, lv = px_level(slot), top = lv, bonus = 0, k;
    if (!took_part) {
        if (px_ko[slot] && !px_knocked_out) {
            gain = 0;
        } else {
            gain = exp * px_share / 100;
            if (gain == 0 && exp > 0 && px_share > 0) {
                gain = 1;
            }
        }
    }
    if (px_catch_up && gain > 0) {
        for (k = 0; k < 3; k++) {
            top = px_level(k) > top ? px_level(k) : top;
        }
        bonus = (top - lv) * 10 > 100 ? 100 : (top - lv) * 10;
        gain += gain * bonus / 100;
    }
    port_log("party xp: frame %ld: slot %d (level %d) %s: %d of the fighters' %d%s", port_frames, slot, lv, how, gain,
             exp, bonus > 0 ? " (catch-up)" : "");
    return gain;
}

static Mod mods[] = {
    { .id = "fast_forward", .version = "0.1", .options = ff_options,
      .option_count = (int)(sizeof(ff_options) / sizeof(ff_options[0])), .start = ff_start, .frame = ff_frame },
    { .id = "skip_dialogues", .version = "0.1", .options = sd_options,
      .option_count = (int)(sizeof(sd_options) / sizeof(sd_options[0])), .start = sd_start, .frame = sd_frame },
    { .id = "battle_animations", .version = "0.1", .options = ba_options,
      .option_count = (int)(sizeof(ba_options) / sizeof(ba_options[0])), .start = ba_start },
    { .id = "global_save", .version = "0.1", .options = gs_options,
      .option_count = (int)(sizeof(gs_options) / sizeof(gs_options[0])), .start = gs_start },
    { .id = "party_xp", .version = "0.1", .options = px_options,
      .option_count = (int)(sizeof(px_options) / sizeof(px_options[0])), .start = px_start },
};
enum { MOD_FAST_FORWARD, MOD_SKIP_DIALOGUES };
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
            port_settings_fail(where, "a binding (a string or a list: docs/LAUNCHER.md \"Input bindings\")");
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
    ff.base_pace = port_pace_get();
    clock_gettime(CLOCK_MONOTONIC, &ff.t0);
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
        if (mod->start != NULL) {
            mod->start(mod);
        }
    }
}

/* The window's title: the mods that are on. */
static void mods_status(void) {
    char status[64];
    const Mod *f = &mods[MOD_FAST_FORWARD];
    snprintf(status, sizeof(status), "%s%s%s",
             ff.active ? (ff_multiples[(int)f->values[FF_SPEED].number] > 0 ? "fast-forward" : "fast-forward, unlimited")
                       : "",
             ff.active && port_mod_skip_dialogues ? ", " : "", port_mod_skip_dialogues ? "skip dialogues" : "");
    port_video_set_status(status);
}

/* Fast-forward on or off: the mod's bindings, or skip_dialogues' request (with fast_forward's speed and mute). */
static void ff_apply(void) {
    const Mod *f = &mods[MOD_FAST_FORWARD];
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
    int m, skip = port_mod_skip_dialogues, active = ff.active;
    for (m = 0; m < MOD_COUNT; m++) {
        if (mods[m].enabled && mods[m].frame != NULL) {
            mods[m].frame(&mods[m]);
        }
    }
    ff_apply();
    if (skip != port_mod_skip_dialogues || active != ff.active) {
        mods_status();
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
