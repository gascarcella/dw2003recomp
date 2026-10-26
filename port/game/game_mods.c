/* The game's built-in mods (docs/LAUNCHER.md "Built-in mods"; psxstack/game.h game_mods): skip_dialogues,
 * battle_animations, global_save, preset_language, party_xp, xp_boost, level_up_recover, widescreen,
 * random_encounters, fast_transitions and fast_travel, as PortMod records the runtime's engine (port/runtime/mods.c) reads the
 * settings of, gives hotkeys to and runs every vsync. Each reaches the game's C through
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
#include "text_pack.h"

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
 * reaches, then the fan translations. A translation is not a text set of its own (records_language has 7 values: the
 * text files' IDs and several of the game's tables): it plays as English, with its text pack (pl_packs, in the mod's
 * directory beside the binary; text_pack.c, docs/PORT.md "Text packs") laid over the English files' entries. Without
 * a valid pack it plays in English. */
static const char *const pl_languages[] = { "english", "french", "german", "italian", "spanish", "usa", "japanese",
                                            "portuguese_br", NULL };
static const s32 pl_codes[] = { 2, 3, 5, 4, 6, 1, 0, 2 };
static const char *const pl_packs[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, "pt_br.pack" };
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
    if (pl_packs[i] != NULL) {
        port_text_pack_open(pl_packs[i]);
    }
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

/* ---- level_up_recover (docs/LAUNCHER.md "Recover on level up"; docs/MECHANICS.md section 6): while enabled,
 * port_mod_level_up_recover makes STFGTREP's report call port_level_up_recover when stfgtrep_add_exp says a party
 * member went up, which sets the record's HP and MP to the new maximums. The game's level up raises only max HP and
 * max MP (stfgtrep_raise_stats); WFIGHTMN has already written the battle's HP and MP back to the records, and nothing
 * after the report writes them again, so the field gets the full values. */
int port_mod_level_up_recover;

static void lr_start(struct PortMod *mod) {
    (void)mod;
    port_mod_level_up_recover = 1;
}

void port_level_up_recover(s32 digimon) {
    GamestateStats *st = &gamestate_data.digimon[digimon].record.stats;
    port_log("level up recover: frame %ld: Digimon %d level %d: HP %d -> %d, MP %d -> %d", port_frames, digimon,
             st->values[0], st->values[2], st->values[3], st->values[4], st->values[5]);
    st->values[2] = st->values[3];
    st->values[4] = st->values[5];
}

/* ---- widescreen (docs/LAUNCHER.md "Widescreen"): the battle, and with `field` the field, at the screen's shape up to
 * 16:9 through the hardware renderer's wide canvas (psxstack render_gpu_wide.c), which draws what the game sends past
 * the 320-wide screen's edges.
 *  - The battle: FIGHTSTG already sends the 3D a 16:9 frame needs: its only screen-space cull,
 *    fightstg_model_mesh_is_visible, keeps a mesh with a bounding point within the clip plus 64 pixels (x within 224
 *    of the centre; 16:9 reaches 213.5), and the GPU clips the rest to the 320-wide drawing area (issue #71). Nothing in
 *    the game changes: the mod only tells the renderer which scenes to widen.
 *  - The field (issue #78): its map is streamed tiles, and the tile objects in RAM already cover a 426-wide view, but
 *    it draws only the tiles a 320-wide screen shows. port_field_wide (include/port.h) is the canvas's columns past
 *    each side while FIELDSTG runs; the field's hooks then draw that far, in 3 more tile slots (port_field_slots) in
 *    the VRAM's extra rows (port_video_vram_extend). The game's state changes with it (the camera stops at the
 *    picture's edge), so a run's record differs from the 4:3 one; with the software renderer it is 0 and nothing does.
 * The menus and the other overlays are 320-wide 2D whose art ends at the screen's edge: they stay 4:3 (pillarboxed). */
enum { WS_FIELD };
static const PortModOption ws_options[] = {
    { .id = "field", .type = PORT_MOD_BOOL, .def = "true", .applies = "restart" },
};
s32 port_field_wide;
s32 port_field_slots[3][3];

static void ws_start(struct PortMod *mod) {
    (void)mod;
    port_video_widescreen_enable();
    port_video_vram_extend();
}

static void ws_frame(struct PortMod *mod) {
    const PortOverlay *ovl = port_overlay_current(1);
    int field = mod->values[WS_FIELD].number != 0 && ovl != NULL && strcmp(ovl->name, "FIELDSTG") == 0;
    int pad;

    port_video_widescreen(ovl != NULL && (strcmp(ovl->name, "FIGHTSTG") == 0 || field));
    pad = field ? port_video_widescreen_pad() : 0;
    port_field_wide = pad < PORT_FIELD_WIDE_MAX ? pad : PORT_FIELD_WIDE_MAX;
}

/* ---- random_encounters (docs/LAUNCHER.md "Disable random encounters"; docs/MECHANICS.md section 1, "Random
 * encounters"): while it is on, port_mod_random_encounters makes FIELDSTG's encounter step, when encounter_timer runs
 * out, redraw the timer without starting the battle, as the game does with its debug switch (records_state.encounters)
 * off: the timer still runs and its redraw takes its one draw, so only the battle is gone. On from the start; `toggle`
 * turns it off and on again. The battles events start (the story's, the trainers', the spots' and the meters')
 * are other callers of fieldstg_start_battle and still start. */
enum { RE_TOGGLE };
static const PortModOption re_options[] = {
    { .id = "toggle", .type = PORT_MOD_BINDING, .def = "\"\"", .applies = "live" },
};
int port_mod_random_encounters;

static void re_start(struct PortMod *mod) {
    (void)mod;
    port_mod_random_encounters = 1;
}

static void re_frame(struct PortMod *mod) {
    if (port_input_pressed(mod->values[RE_TOGGLE].action)) {
        port_mod_random_encounters = !port_mod_random_encounters;
        port_log("random encounters: %s at frame %ld", port_mod_random_encounters ? "off" : "on", port_frames);
    }
}

/* The window's title while it is on. */
static const char *re_status(struct PortMod *mod) {
    (void)mod;
    return port_mod_random_encounters ? "no encounters" : NULL;
}

void port_random_encounter_skipped(s32 map, s32 area) {
    port_log("random encounters: frame %ld: map 0x%X, area %d: the encounter skipped", port_frames, (int)map, (int)area);
}

/* ---- fast_transitions (docs/LAUNCHER.md "Faster transitions"; docs/MECHANICS.md section 15): with `speed` "fast" or
 * "instant", port_mod_fast_transitions makes the game's own transition and save-screen animations take fewer frames:
 * the full-screen fades' lengths, FIELDSTG's map change (the closing window, the map title's rectangles, text and hold,
 * the warp's hold), the field menu's panels and picture, STGMCARD's slides, panel and finishing bar. Each still runs
 * through its own update to the end state the code waits for; only its frame count or per-frame step changes. The music
 * plays on untouched (no fast-forward). "normal" leaves everything as the game's. The speed is read every vsync. */
static const char *const ft_speeds[] = { "normal", "fast", "instant", NULL };
enum { FT_SPEED };
static const PortModOption ft_options[] = {
    { .id = "speed", .type = PORT_MOD_ENUM, .def = "\"normal\"", .values = ft_speeds, .applies = "live" },
};
int port_mod_fast_transitions;

static void ft_frame(struct PortMod *mod) {
    port_mod_fast_transitions = (int)mod->values[FT_SPEED].number;
}

static void ft_start(struct PortMod *mod) {
    ft_frame(mod);
    port_log("fast transitions: %s", ft_speeds[port_mod_fast_transitions]);
}

/* The window's title while it is on. */
static const char *ft_status(struct PortMod *mod) {
    (void)mod;
    return port_mod_fast_transitions == 1 ? "fast transitions"
           : port_mod_fast_transitions == 2 ? "instant transitions" : NULL;
}

s32 port_transition_frames(s32 frames) {
    if (frames <= 1 || port_mod_fast_transitions == 0) {
        return frames;
    }
    return port_mod_fast_transitions == 1 ? (frames + 1) / 2 : 1;
}

s32 port_transition_extra(s32 left, s32 step) {
    s32 rest; /* the steps after this frame's that the original takes to cover `left` */
    if (port_mod_fast_transitions == 0 || step <= 0 || left <= step) {
        return 0;
    }
    rest = (left + step - 1) / step - 1;
    return port_mod_fast_transitions == 1 ? (rest < 1 ? rest : 1) : rest;
}

s32 port_transition_ticks(s32 ticks) {
    return port_mod_fast_transitions == 0 ? ticks : port_mod_fast_transitions == 1 ? ticks / 2 : 0;
}

s32 port_transition_speed(s32 speed) {
    return port_mod_fast_transitions == 0 ? speed : port_mod_fast_transitions == 1 ? speed * 2 : 1024;
}

/* ---- fast_travel (docs/LAUNCHER.md "Fast travel"; docs/MECHANICS.md section 16): while enabled, port_mod_fast_travel
 * makes STSTATUS's map page (MAP in the field menu) a travel map, as Flawe's Mod 2.0 does on the PS1 (docs/THIRD_PARTY.md):
 * confirm on a visited area (an area lights up once any of its maps has been entered: flag 0x2000 + map) picks it, the
 * square button shows the other server's map, and leaving the menus enters the destination. The pick sets
 * gamestate_data.field_map and player_pos; the menu's way back to the field already enters field_map at player_pos
 * (FIELDSTG takes return_pos from them when it comes from outside the field), so no map change is made here. The
 * player's direction, route and room are left as they were, as Flawe's are. */
int port_mod_fast_travel;
static s32 tr_images = -1; /* the region whose map images STSTATUS last uploaded (port_fast_travel_images) */

/* Each area's destination (Flawe's Mod 2.0's table, by STSTATUS's area 1..46): the map on the Asuka (region 0) and
 * the Amaterasu (region 1) server, and the place in it (24.8, the same on both servers). Mod 2.0 has Amaterasu's
 * Swamp Inn at 0x2A6 and Tranquil Swamp at 0x2A5, one map off (Shaman House's and Swamp Inn's: ststatus_map_areas, the
 * inns' maps); these are its 1.x maps, the areas' own. */
static const struct { s16 map[2]; s32 x, y; } tr_destinations[46] = {
    { { 0x267, 0x2CF }, 38686, 178983 },  /*  1 Legendary Gym */
    { { 0x266, 0x2CE }, 135635, 114668 }, /*  2 Lake of Ice */
    { { 0x265, 0x2CD }, 175891, 76459 },  /*  3 Kulon Mine */
    { { 0x262, 0x2CA }, 221247, 143146 }, /*  4 Snow Mountain */
    { { 0x264, 0x2CC }, 140088, 114794 }, /*  5 Freeze Mountain */
    { { 0x26F, 0x2D6 }, 190842, 170543 }, /*  6 Genbu City / Xuan Wu City */
    { { 0x228, 0x297 }, 79728, 206286 },  /*  7 Duel Island */
    { { 0x24D, 0x2B7 }, 200458, 176851 }, /*  8 Dum Dum Factory */
    { { 0x268, 0x2D0 }, 32520, 87528 },   /*  9 Kulon Pit */
    { { 0x261, 0x2C9 }, 209802, 104242 }, /* 10 Boot Mountain */
    { { 0x263, 0x2CB }, 66356, 74958 },   /* 11 Mountain Inn */
    { { 0x223, 0x292 }, 103672, 85128 },  /* 12 Forest Inn */
    { { 0x238, 0x2A5 }, 124294, 91646 },  /* 13 Swamp Inn */
    { { 0x225, 0x294 }, 245462, 124066 }, /* 14 Protocol Ruins, Protocol Forest */
    { { 0x22E, 0x29C }, 139226, 119768 }, /* 15 Seiryu City / Qing Long City */
    { { 0x22B, 0x29A }, 131158, 208544 }, /* 16 Tyranno Valley */
    { { 0x24C, 0x2B6 }, 119592, 214191 }, /* 17 Bullet Valley */
    { { 0x220, 0x28F }, 160127, 175488 }, /* 18 Plug Cape */
    { { 0x23D, 0x2AA }, 67512, 121476 },  /* 19 South Cape */
    { { 0x200, 0x270 }, 186824, 102500 }, /* 20 Asuka City / Amaterasu City */
    { { 0x222, 0x291 }, 87454, 66170 },   /* 21 Wire Forest */
    { { 0x21E, 0x28D }, 111434, 91323 },  /* 22 Wire Forest Entrance */
    { { 0x227, 0x296 }, 93870, 160229 },  /* 23 Divermon's Lake */
    { { 0x229, 0x298 }, 202275, 140071 }, /* 24 Wind Prairie */
    { { 0x22A, 0x299 }, 124997, 169747 }, /* 25 Kicking Forest */
    { { 0x249, 0x2B3 }, 84211, 84884 },   /* 26 Pelche Oasis */
    { { 0x24B, 0x2B5 }, 177438, 134580 }, /* 27 North Badland E */
    { { 0x247, 0x2B1 }, 165214, 175746 }, /* 28 South Badland */
    { { 0x24A, 0x2B4 }, 238112, 147459 }, /* 29 North Badland W */
    { { 0x21D, 0x28C }, 225700, 164434 }, /* 30 Central Park */
    { { 0x22C, 0x29B }, 121278, 67293 },  /* 31 East Station */
    { { 0x232, 0x2A0 }, 88798, 70093 },   /* 32 South Station */
    { { 0x21F, 0x28E }, 152401, 96119 },  /* 33 Shell Beach */
    { { 0x25D, 0x2C6 }, 137276, 186980 }, /* 34 Byakko City / Bai Hu City */
    { { 0x248, 0x2B2 }, 151850, 133014 }, /* 35 Noise Desert */
    { { 0x242, 0x2AF }, 168262, 121823 }, /* 36 Jungle Shrine */
    { { 0x239, 0x2A6 }, 43514, 70079 },   /* 37 Shaman House */
    { { 0x235, 0x2A3 }, 234910, 67359 },  /* 38 Bios Swamp */
    { { 0x25A, 0x2C3 }, 125323, 133383 }, /* 39 Mirage Tower */
    { { 0x258, 0x2C1 }, 134408, 109539 }, /* 40 Mobius Desert */
    { { 0x23C, 0x2A9 }, 153707, 128746 }, /* 41 Ether Jungle */
    { { 0x23E, 0x2AB }, 151383, 184308 }, /* 42 Suzaku City / Zhu Que City */
    { { 0x234, 0x2A2 }, 253014, 128892 }, /* 43 Bulk Swamp, Bulk Bridge */
    { { 0x237, 0x2A4 }, 184837, 67238 },  /* 44 Tranquil Swamp */
    { { 0x23A, 0x2A7 }, 168058, 229804 }, /* 45 Jungle Grave */
    { { 0x23B, 0x2A8 }, 276466, 169065 }, /* 46 Phoenix Bay */
};

static void tr_start(struct PortMod *mod) {
    (void)mod;
    port_mod_fast_travel = 1;
}

s32 port_fast_travel_select(s32 region, s32 area) {
    if (region < 0 || region > 1 || area < 1 || area > 46) {
        return 0;
    }
    gamestate_data.field_map = tr_destinations[area - 1].map[region];
    gamestate_data.player_pos.x = tr_destinations[area - 1].x;
    gamestate_data.player_pos.y = tr_destinations[area - 1].y;
    port_log("fast travel: frame %ld: area %d of region %d: map 0x%X at (%d, %d) on leaving the menus", port_frames,
             (int)area, (int)region, (int)gamestate_data.field_map, (int)(gamestate_data.player_pos.x >> 8),
             (int)(gamestate_data.player_pos.y >> 8));
    return 1;
}

void port_fast_travel_images_loaded(s32 region) {
    tr_images = region;
}

s32 port_fast_travel_images(void) {
    return tr_images;
}

/* ---- The stops (psxstack docs/LAUNCHER.md "Mod runtime" rule 6; the in-game menu switches a mod off live): each
 * undoes what its start set, the flag the game's hooks test and the mod's own state. preset_language and widescreen
 * have none and say `applies = "restart"`: the language is chosen at the boot's select screen, and the wide canvas
 * must be asked for before the window opens. */
static void sd_stop(struct PortMod *mod) {
    (void)mod;
    sd_toggled = 0;
    port_mod_skip_dialogues = 0;
    port_fast_forward_request(0);
}

static void ba_stop(struct PortMod *mod) {
    (void)mod;
    port_mod_battle_animations = 0;
}

static void gs_stop(struct PortMod *mod) {
    (void)mod;
    port_mod_global_save = 0;
}

static void px_stop(struct PortMod *mod) {
    (void)mod;
    port_mod_party_xp = 0;
}

static void xb_stop(struct PortMod *mod) {
    (void)mod;
    port_mod_xp_boost = 0;
}

static void lr_stop(struct PortMod *mod) {
    (void)mod;
    port_mod_level_up_recover = 0;
}

static void re_stop(struct PortMod *mod) {
    (void)mod;
    port_mod_random_encounters = 0;
}

static void ft_stop(struct PortMod *mod) {
    (void)mod;
    port_mod_fast_transitions = 0;
}

static void tr_stop(struct PortMod *mod) {
    (void)mod;
    port_mod_fast_travel = 0;
}

static PortMod game_mod_table[] = {
    { .id = "skip_dialogues", .version = "0.1", .options = sd_options,
      .option_count = (int)(sizeof(sd_options) / sizeof(sd_options[0])), .start = sd_start, .stop = sd_stop, .frame = sd_frame,
      .status = sd_status },
    { .id = "battle_animations", .version = "0.1", .options = ba_options,
      .option_count = (int)(sizeof(ba_options) / sizeof(ba_options[0])), .start = ba_start, .stop = ba_stop },
    { .id = "global_save", .version = "0.1", .options = gs_options,
      .option_count = (int)(sizeof(gs_options) / sizeof(gs_options[0])), .start = gs_start, .stop = gs_stop },
    { .id = "preset_language", .version = "0.1", .options = pl_options,
      .option_count = (int)(sizeof(pl_options) / sizeof(pl_options[0])), .start = pl_start, .applies = "restart" },
    { .id = "party_xp", .version = "0.1", .options = px_options,
      .option_count = (int)(sizeof(px_options) / sizeof(px_options[0])), .start = px_start, .stop = px_stop },
    { .id = "xp_boost", .version = "0.1", .options = xb_options,
      .option_count = (int)(sizeof(xb_options) / sizeof(xb_options[0])), .start = xb_start, .stop = xb_stop },
    { .id = "level_up_recover", .version = "0.1", .start = lr_start, .stop = lr_stop },
    { .id = "widescreen", .version = "0.2", .options = ws_options,
      .option_count = (int)(sizeof(ws_options) / sizeof(ws_options[0])), .start = ws_start, .applies = "restart",
      .frame = ws_frame },
    { .id = "random_encounters", .version = "0.1", .options = re_options,
      .option_count = (int)(sizeof(re_options) / sizeof(re_options[0])), .start = re_start, .stop = re_stop, .frame = re_frame,
      .status = re_status },
    { .id = "fast_transitions", .version = "0.1", .options = ft_options,
      .option_count = (int)(sizeof(ft_options) / sizeof(ft_options[0])), .start = ft_start, .stop = ft_stop, .frame = ft_frame,
      .status = ft_status },
    { .id = "fast_travel", .version = "0.1", .start = tr_start, .stop = tr_stop },
};

int game_mod_count(void) {
    return (int)(sizeof(game_mod_table) / sizeof(game_mod_table[0]));
}

PortMod *game_mods(void) {
    return game_mod_table;
}

/* A save state (psxstack/game.h game_savestate): what the mods carry from one vsync to the next about the game (the
 * map a save-anywhere was opened from, the last battle's knocked-out party slots, the boost's tenths, the region whose
 * map images are in VRAM, widescreen's three field tile slots and the field's width in force, which the next vsync's
 * camera and culling read before the mod's frame sets it again). Their options and toggles are settings: the loading
 * run's own. */
void game_savestate(struct PortState *s) {
    PORT_STATE_VAR(s, gs_opened_from);
    PORT_STATE_VAR(s, px_ko);
    PORT_STATE_VAR(s, xb_tenths);
    PORT_STATE_VAR(s, tr_images);
    PORT_STATE_VAR(s, port_field_slots);
    PORT_STATE_VAR(s, port_field_wide);
}
