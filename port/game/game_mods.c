/* The game's built-in mods (docs/LAUNCHER.md "Built-in mods"; psxstack/game.h game_mods): skip_dialogues,
 * battle_animations, global_save, preset_language, party_xp, xp_boost and widescreen, as PortMod records the
 * runtime's engine (port/runtime/mods.c) reads the settings of, gives hotkeys to and runs every vsync. Each reaches the game's C through
 * the port_mod_* flags and functions of include/port.h, read inside `#ifdef PC_PORT` blocks. The runtime's own mod,
 * fast_forward, is the engine's; skip_dialogues asks for it with port_fast_forward_request. */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "battle_scan.h"
#include "fieldstg.h"
#include "gamestate.h"
#include "port_harness.h"
#include "port_runtime.h"
#include "savestate.h"

#define PORT_BATTLE_SCAN_MAX 4096 /* words: the longest script on the disc has 567 */

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
static const PortModOption sd_options[] = {
    { .id = "toggle", .type = PORT_MOD_BINDING, .def = "\"F2\"", .applies = "live" },
    { .id = "hold", .type = PORT_MOD_BINDING, .def = "\"\"", .applies = "live" },
    { .id = "fast_forward_waits", .type = PORT_MOD_BOOL, .def = "false", .applies = "live" },
};
int port_mod_skip_dialogues;
static int sd_toggled;

static void sd_start(struct PortMod *mod) {
    const char *test = getenv("DW3_PORT_SKIP_DIALOGUES");
    (void)mod;
    if (test != NULL && strcmp(test, "1") == 0) {
        sd_toggled = 1;
        port_log("skip dialogues: on from the start (DW3_PORT_SKIP_DIALOGUES)");
    }
}

static void sd_frame(struct PortMod *mod) {
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
    port_fast_forward_request(on && mod->values[SD_FF_WAITS].number != 0 && field != NULL &&
                              strcmp(field->name, "FIELDSTG") == 0 && fieldstg_stage.event_running != 0);
}

/* The window's title while it is on. */
static const char *sd_status(struct PortMod *mod) {
    (void)mod;
    return port_mod_skip_dialogues ? "skip dialogues" : NULL;
}

/* ---- battle_animations (docs/LAUNCHER.md "Disable battle animations"): while enabled, port_mod_battle_animations makes fightstg_script_update ask
 * port_battle_cut at the INIT of every script 5 and up (the attacks', techniques' and items' animations; the rules
 * already ran): one with a child command becomes the target's reaction (results[3] + 1: flinch, heavy hit, KO, dodge)
 * with `hit_reaction` (the default), else ends at once; one without ends at once. A knock-out keeps its KO reaction in
 * both cases (it ends on the KO pose, animation 10). The hit sound the script would have played is played. A script
 * that does not scan to its end (none on the disc: tests/port/battle.py) is left to run. */
enum { BA_HIT_REACTION };
static const PortModOption ba_options[] = {
    { .id = "hit_reaction", .type = PORT_MOD_BOOL, .def = "true", .applies = "live" },
};
int port_mod_battle_animations;
static int ba_hit_reaction;

static void ba_start(struct PortMod *mod) {
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
static const PortModOption gs_options[] = {
    { .id = "restore_map_state", .type = PORT_MOD_BOOL, .def = "true", .applies = "live" },
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
    s32 alt_layout;
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

static void gs_start(struct PortMod *mod) {
    port_mod_global_save = 1;
    gs_restore = mod->values[GS_RESTORE].number != 0;
}

void port_global_save_open(void) {
    s32 map = gamestate_data.field_map, inn = gs_inn_map(map);
    gs_opened_from = map;
    /* through a prototyped pointer: clang's -Wdeprecated-non-prototype is an error in port/runtime */
    ((void (*)(s32, s32))gamestate_data.funcs.set_next_map)(inn != 0 ? inn : 0xC00, -1);
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
    r.alt_layout = gamestate_data.alt_layout;
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
    gamestate_data.alt_layout = r.alt_layout;
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

/* ---- preset_language (docs/LAUNCHER.md "Preset language"): while enabled, port_mod_preset_language makes CNTY_SEL's root object set
 * records_language to the option's code and go on to the opening (0xE02; 0xE01 for Japanese, as the menu) instead of
 * drawing the language select screen. It acts at boot only (and after a reset, which goes through CNTY_SEL again).
 * The codes are the text sets' (records_language: the offset of every localized file ID): the five the screen offers
 * in its order (cnty_sel_languages), then USA and JPN, whose sets are on the EU disc but which the screen never
 * reaches. */
static const char *const pl_languages[] = { "english", "french", "german", "italian", "spanish", "usa", "japanese", NULL };
static const s32 pl_codes[] = { 2, 3, 5, 4, 6, 1, 0 };
enum { PL_LANGUAGE };
static const PortModOption pl_options[] = {
    { .id = "language", .type = PORT_MOD_ENUM, .def = "\"english\"", .values = pl_languages, .applies = "restart" },
};
int port_mod_preset_language;
s32 port_preset_language = 2;

static void pl_start(struct PortMod *mod) {
    int i = (int)mod->values[PL_LANGUAGE].number;
    port_mod_preset_language = 1;
    port_preset_language = pl_codes[i];
    port_log("preset language: %s (records_language %d), no language select screen", pl_languages[i],
             (int)port_preset_language);
}

/* ---- party_xp (docs/LAUNCHER.md "Party experience"; docs/MECHANICS.md section 6, "Who gets a battle's experience"):
 * while enabled, port_mod_party_xp makes STFGTREP's report ask port_party_xp_share for every party member's
 * experience. A member that took part keeps one fighter's split share; one that did not gets `share` percent of it
 * (at least 1 when both are above 0), a knocked-out one (port_party_xp_knocked_out, from WFIGHTMN) only with
 * `knocked_out`. With `catch_up` a member below the party's highest level gets 10% more per level below it, at most
 * twice as much. The report then shows, counts and adds it as it does a fighter's (item 0x141's fifth included); form
 * experience still goes only to the forms that fought. */
enum { PX_SHARE, PX_KNOCKED_OUT, PX_CATCH_UP };
static const PortModOption px_options[] = {
    { .id = "share", .type = PORT_MOD_INT, .def = "50", .min = 0, .max = 100, .step = 5, .applies = "live" },
    { .id = "knocked_out", .type = PORT_MOD_BOOL, .def = "false", .applies = "live" },
    { .id = "catch_up", .type = PORT_MOD_BOOL, .def = "false", .applies = "live" },
};
int port_mod_party_xp;
static int px_share, px_knocked_out, px_catch_up;
static int px_ko[3]; /* the last battle's party slots at 0 HP */

static void px_start(struct PortMod *mod) {
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

/* ---- xp_boost (docs/LAUNCHER.md "XP boost"): while enabled, port_mod_xp_boost makes STFGTREP's report pass a won
 * battle's experience (one fighter's split share), each form's experience (after the game's 10 or 50 cap) and the money
 * (item 0x142's fifth included) through port_xp_boost, which multiplies them by `exp`, `form_exp` and `bits` (in tenths,
 * rounded down, at most 9,999,999). Above 5x (the sliders' top) a value needs `manual` (mod_resolve caps it without).
 * The launcher's presets (Boost, Turbo, Ultra: the manifest's `presets`) only fill in the three values. */
enum { XB_EXP, XB_FORM_EXP, XB_BITS, XB_MANUAL };
static const PortModOption xb_options[] = {
    { .id = "exp", .type = PORT_MOD_FLOAT, .def = "2", .min = 1, .max = 10, .step = 0.1, .slider_max = 5,
      .input_toggle = "manual", .applies = "live" },
    { .id = "form_exp", .type = PORT_MOD_FLOAT, .def = "2", .min = 1, .max = 10, .step = 0.1, .slider_max = 5,
      .input_toggle = "manual", .applies = "live" },
    { .id = "bits", .type = PORT_MOD_FLOAT, .def = "2", .min = 1, .max = 10, .step = 0.1, .slider_max = 5,
      .input_toggle = "manual", .applies = "live" },
    { .id = "manual", .type = PORT_MOD_BOOL, .def = "false", .applies = "live" },
};
static const char *const xb_names[] = { "experience", "form experience", "bits" };
int port_mod_xp_boost;
static s32 xb_tenths[3];

static void xb_start(struct PortMod *mod) {
    int k;
    port_mod_xp_boost = 1;
    for (k = 0; k < 3; k++) {
        xb_tenths[k] = (s32)(mod->values[XB_EXP + k].number * 10 + 0.5);
    }
    port_log("xp boost: experience %d.%dx, form experience %d.%dx, bits %d.%dx", (int)xb_tenths[0] / 10,
             (int)xb_tenths[0] % 10, (int)xb_tenths[1] / 10, (int)xb_tenths[1] % 10, (int)xb_tenths[2] / 10,
             (int)xb_tenths[2] % 10);
}

s32 port_xp_boost(s32 kind, s32 amount) {
    long long r;
    if (kind < 0 || kind > 2 || amount <= 0) {
        return amount;
    }
    r = (long long)amount * xb_tenths[kind] / 10;
    r = r > 9999999 ? 9999999 : r;
    port_log("xp boost: frame %ld: %s %d -> %lld", port_frames, xb_names[kind], (int)amount, r);
    return (s32)r;
}

/* ---- widescreen (docs/LAUNCHER.md "Widescreen battles"): the battle at 16:9 through the hardware renderer's wide
 * canvas (psxstack render_gpu_wide.c). FIGHTSTG already sends the 3D a 16:9 frame needs: its only screen-space cull,
 * fightstg_model_mesh_is_visible, keeps a mesh with a bounding point within the clip plus 64 pixels (x within 224 of
 * the centre; 16:9 reaches 213.5), and the GPU clips the rest to the 320-wide drawing area (issue #71). Nothing in the
 * game changes: the mod only tells the renderer which scenes to widen. The field and the menus are 320-wide 2D and stay
 * 4:3 (pillarboxed in a 16:9 window). */
static void ws_start(struct PortMod *mod) {
    (void)mod;
    port_video_widescreen_enable();
}

static void ws_frame(struct PortMod *mod) {
    const PortOverlay *ovl = port_overlay_current(1);
    (void)mod;
    port_video_widescreen(ovl != NULL && strcmp(ovl->name, "FIGHTSTG") == 0);
}

static PortMod game_mod_table[] = {
    { .id = "skip_dialogues", .version = "0.1", .options = sd_options,
      .option_count = (int)(sizeof(sd_options) / sizeof(sd_options[0])), .start = sd_start, .frame = sd_frame,
      .status = sd_status },
    { .id = "battle_animations", .version = "0.1", .options = ba_options,
      .option_count = (int)(sizeof(ba_options) / sizeof(ba_options[0])), .start = ba_start },
    { .id = "global_save", .version = "0.1", .options = gs_options,
      .option_count = (int)(sizeof(gs_options) / sizeof(gs_options[0])), .start = gs_start },
    { .id = "preset_language", .version = "0.1", .options = pl_options,
      .option_count = (int)(sizeof(pl_options) / sizeof(pl_options[0])), .start = pl_start },
    { .id = "party_xp", .version = "0.1", .options = px_options,
      .option_count = (int)(sizeof(px_options) / sizeof(px_options[0])), .start = px_start },
    { .id = "xp_boost", .version = "0.1", .options = xb_options,
      .option_count = (int)(sizeof(xb_options) / sizeof(xb_options[0])), .start = xb_start },
    { .id = "widescreen", .version = "0.1", .start = ws_start, .frame = ws_frame },
};

int game_mod_count(void) {
    return (int)(sizeof(game_mod_table) / sizeof(game_mod_table[0]));
}

PortMod *game_mods(void) {
    return game_mod_table;
}

/* A save state (psxstack/game.h game_savestate): what the mods carry from one vsync to the next about the game (the
 * map a save-anywhere was opened from, the last battle's knocked-out party slots, the boost's tenths). Their options
 * and toggles are settings: the loading run's own. */
void game_savestate(struct PortState *s) {
    PORT_STATE_VAR(s, gs_opened_from);
    PORT_STATE_VAR(s, px_ko);
    PORT_STATE_VAR(s, xb_tenths);
}
