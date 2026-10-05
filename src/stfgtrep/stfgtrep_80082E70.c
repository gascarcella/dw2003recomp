#include "common.h"
#include "object.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "overlay_common.h"

/* STFGTREP.PRO: the battle report. The money and card won, then for every party member the experience
 * gained (counted up), level-ups with their stat gains, and new techniques. */

/* An entry of the reward table stfgtrep_rewards, per battle (records_battle_results.battle). */
typedef struct StfgtrepReward {
    /* 0x0 */ s32 tech_exp; /* technique experience */
    /* 0x4 */ s32 exp;   /* experience */
    /* 0x8 */ s32 money; /* money */
} StfgtrepReward; /* size 0xC */

/* The report (stfgtrep_main_create, size 0xA0). */
typedef struct StfgtrepMain {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id; /* layer */
    /* 0x54 */ s32 ot_depth; /* ordering table depth */
    /* 0x58 */ s32 scroll; /* background scroll */
    /* 0x5C */ s32 odd_frame; /* the scroll moves every other frame */
    /* 0x60 */ s32 member_count; /* party members */
    /* 0x64 */ s32 member_exp[3]; /* experience each member gets */
    /* 0x70 */ s32 techniques_used; /* techniques used */
    /* 0x74 */ s32 arrow_shown; /* the "next" arrow is shown */
    /* 0x78 */ s32 arrow_time; /* time of the arrow's last frame */
    /* 0x7C */ s32 arrow_frame; /* the arrow's frame */
    /* 0x80 */ WindowAnim exp_anim; /* the experience window */
    /* 0x90 */ WindowAnim frame_anim; /* the frame */
} StfgtrepMain; /* size 0xA0 */

/* A technique's "level up" on a party member's panel. */
typedef struct StfgtrepTechLevelUp {
    /* 0x0 */ s32 level_up; /* the technique's level went up */
    /* 0x4 */ s32 step; /* fade-in step, 0..10 */
    /* 0x8 */ s32 time; /* time of the last step */
    /* 0xC */ s32 unk_C;
} StfgtrepTechLevelUp; /* size 0x10 */

/* A party member's panel (stfgtrep_member_create, size 0x174). */
typedef struct StfgtrepMember {
    /* 0x000 */ Object base;
    /* 0x050 */ StfgtrepMain *main;
    /* 0x054 */ s32 layer_id; /* layer */
    /* 0x058 */ s32 ot_depth; /* ordering table depth */
    /* 0x05C */ s32 slot;   /* party slot */
    /* 0x060 */ s32 exp_gained; /* experience gained */
    /* 0x064 */ s32 bonus_applied; /* the experience bonus was applied */
    /* 0x068 */ s32 highlighted; /* the panel is highlighted */
    /* 0x06C */ s32 anim_frame; /* the Digimon's animation frame */
    /* 0x070 */ s32 anim_time; /* time of its last frame */
    /* 0x074 */ s16 chosen_forms[3]; /* its techniques (gamestate_data.funcs.get_chosen_forms) */
    /* 0x07A */ s16 forms[45];  /* gamestate_data.funcs.list_forms */
    /* 0x0D4 */ GamestateForm tech_record; /* the technique's record */
    /* 0x0E8 */ s32 tech_shown; /* technique being shown, -1: none */
    /* 0x0EC */ s32 unk_EC;
    /* 0x0F0 */ s32 level_up; /* the level went up */
    /* 0x0F4 */ s32 level_up_step; /* "level up" fade-in step, 0..10 */
    /* 0x0F8 */ s32 level_up_time; /* time of its last step */
    /* 0x0FC */ s32 unk_FC;
    /* 0x100 */ StfgtrepTechLevelUp tech_level_ups[3];
    /* 0x130 */ s32 sound_playing; /* sound playing */
    /* 0x134 */ s16 voice;   /* its voice, -1: none */
    /* 0x138 */ s32 exp_shown; /* experience shown */
    /* 0x13C */ s32 exp_after; /* experience after the battle */
    /* 0x140 */ s32 count_frames; /* frames of counting */
    /* 0x144 */ s32 unk_144;
    /* 0x148 */ WindowAnim panel_anim;
    /* 0x158 */ s32 highlight_step; /* highlight step */
    /* 0x15C */ s32 highlight_time; /* time of its last step */
    /* 0x160 */ void (*open)(struct StfgtrepMember *obj);  /* stfgtrep_member_open: open */
    /* 0x164 */ void (*close)(struct StfgtrepMember *obj);  /* stfgtrep_member_close: close */
    /* 0x168 */ void (*show_techniques)(struct StfgtrepMember *obj);  /* stfgtrep_member_show_techniques: show the techniques */
    /* 0x16C */ s32 (*get_exp)(struct StfgtrepMember *obj);   /* stfgtrep_member_get_exp: experience with the bonus */
    /* 0x170 */ void (*highlight)(struct StfgtrepMember *obj);  /* stfgtrep_member_highlight: highlight */
} StfgtrepMember; /* size 0x174 */

/* A technique's windows on a party member's panel. */
typedef struct StfgtrepTechWindows {
    /* 0x0 */ MessageWindow *name; /* name */
    /* 0x4 */ MessageWindow *level; /* level */
} StfgtrepTechWindows; /* size 0x8 */

/* A party member panel's data block (0x30 bytes): its text windows. */
typedef struct StfgtrepMemberData {
    /* 0x00 */ MessageWindow *name;   /* name */
    /* 0x04 */ MessageWindow *level;  /* level */
    /* 0x08 */ MessageWindow *level_label;
    /* 0x0C */ MessageWindow *exp;    /* experience */
    /* 0x10 */ MessageWindow *exp_label;
    /* 0x14 */ StfgtrepTechWindows techniques[3]; /* techniques */
    /* 0x2C */ MessageWindow *messages; /* messages */
} StfgtrepMemberData; /* size 0x30 */

/* The report's data block (0x1C bytes). */
typedef struct StfgtrepMainData {
    /* 0x00 */ MessageWindow *exp;
    /* 0x04 */ MessageWindow *exp_label;
    /* 0x08 */ MessageWindow *messages; /* messages */
    /* 0x0C */ StfgtrepMember *members[3];
    /* 0x18 */ Fade *fade;
} StfgtrepMainData; /* size 0x1C */

/* stfgtrep_funcs: the overlay's helpers, called through this table. */
typedef struct StfgtrepFuncs {
    /* 0x00 */ void (*load)(void);                                              /* stfgtrep_load_files */
    /* 0x04 */ s32 (*is_loading)(void);                                         /* stfgtrep_is_loading */
    /* 0x08 */ void (*anim_start)(WindowAnim *anim, s32 open);          /* stfgtrep_window_anim_start */
    /* 0x0C */ s32 (*anim_update)(WindowAnim *anim);                    /* stfgtrep_window_anim_update */
    /* 0x10 */ void (*tween_start)(Tween *obj, s32 from, s32 to, s32 frames); /* stfgtrep_tween_start */
    /* 0x14 */ s32 (*tween_update)(Tween *obj);                         /* stfgtrep_tween_update */
    /* 0x18 */ s32 (*add_exp)(s32 digimon, s32 exp);                            /* stfgtrep_add_exp */
    /* 0x1C */ s32 (*learn_technique)(s32 digimon);                             /* stfgtrep_learn_technique */
    /* 0x20 */ s32 (*add_technique_exp)(s32 digimon, s32 tech, s32 exp);        /* stfgtrep_add_technique_exp */
    /* 0x24 */ s32 (*learn_skill)(s32 digimon, s32 tech); /* stfgtrep_learn_skill (which returns a u16) */
    /* 0x28 */ s32 (*mark_skill)(s32 digimon, s32 tech);                        /* stfgtrep_mark_skill */
    /* 0x2C */ s32 (*get_technique_exp)(s32 digimon, s32 tech, s32 exp, s32 n); /* stfgtrep_get_technique_exp */
} StfgtrepFuncs; /* size 0x30 */

/* A technique another one needs (StfgtrepLearn.requires). */
typedef struct StfgtrepRequirement {
    /* 0x0 */ s16 technique; /* a technique it needs (1-based index into records_digimon), 0: none */
    /* 0x2 */ s16 level; /* at this level at least */
} StfgtrepRequirement; /* size 0x4 */

/* A technique a Digimon can learn (stfgtrep_learn_lists[digimon] points to 44 of them). */
typedef struct StfgtrepLearn {
    /* 0x0 */ s32 technique; /* the technique, 1-based index into records_digimon */
    /* 0x4 */ StfgtrepRequirement requires[2];
    /* 0xC */ s16 stat;  /* a stat it needs: 1-6 GamestateStats.stats[0..5], 7 the level, 8-14 unk_18[0..6] */
    /* 0xE */ s16 min;   /* at least this much */
} StfgtrepLearn; /* size 0x10 */

extern StfgtrepReward stfgtrep_rewards[];
extern StfgtrepLearn *stfgtrep_learn_lists[];
extern s32 stfgtrep_member_anims[][7];
extern StfgtrepFuncs stfgtrep_funcs;
extern s32 stfgtrep_level_exp_bonus[4];
extern s32 stfgtrep_hp_mp_gain_cut[4];
extern s32 stfgtrep_hp_mp_gain_random[9];
extern s32 stfgtrep_stat_gains[][9];
extern s32 stfgtrep_resist_gains[];

StfgtrepMain *stfgtrep_main_create(void);
void stfgtrep_member_draw(StfgtrepMember *obj);
void stfgtrep_member_run(StfgtrepMember *obj, StfgtrepMemberData *data);
void stfgtrep_main_run(StfgtrepMain *obj, StfgtrepMainData *data);
void stfgtrep_main_update(StfgtrepMain *obj, StfgtrepMainData *data);
void stfgtrep_member_show_text(StfgtrepMember *obj, StfgtrepMemberData *data, s32 show);
s32 stfgtrep_member_count_exp(StfgtrepMember *obj);
void stfgtrep_raise_stats(s32 digimon, s32 lv);

/* The overlay's first object: sets up the display and creates the report. */
void stfgtrep_update_main(Object *obj, StfgtrepMain **data) {
    RECT rect;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0x14000);
        gfx_module.funcs.init_display(320, 240, 0, 0);
        rect.x = 0;
        rect.y = 0;
        rect.w = 320;
        rect.h = 240;
        layer = gfx_module.funcs.create_layer(&rect, 3, 0x1000);
        layer->set_bg_color(layer, 0, 0, 0);
        *data = stfgtrep_main_create();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The overlay's entry point (overlay_entries). */
Object *stfgtrep_start(void) {
    return object_new(stfgtrep_update_main, sizeof(Object), 4);
}

void stfgtrep_fade_start(Fade *obj, s32 dir, s32 frames) {
    obj->base.set_state(obj, OBJECT_STATE_RUN);
    obj->base.step = 1;
    obj->from_black = dir;
    if (dir == 0) {
        obj->level = 0;
        obj->step = 0xFF00 / frames;
    } else {
        obj->level = 0xFF00;
        obj->step = -(0xFF00 / frames);
    }
}

void stfgtrep_fade_draw(Fade *obj) {
    GfxLayer *layer;
    u32 *ot;
    POLY_F4 *poly;
    DR_TPAGE *tpage;

    layer = gfx_module.funcs.get_layer(obj->layer_id);
    ot = layer->get_ot_entry(layer, obj->ot_depth);
    poly = gfx_module.funcs.get_packet();
    setPolyF4(poly);
    setSemiTrans(poly, 1);
    poly->r0 = poly->g0 = poly->b0 = obj->level >> 8;
    setXYWH(poly, 0, 0, 320, 256);
    addPrim(ot, poly);
    tpage = (DR_TPAGE *)(poly + 1);
    setDrawTPage(tpage, 0, 1, getTPage(0, 2, 320, 0));
    addPrim(ot, tpage);
    gfx_module.funcs.set_packet(tpage + 1);
}

void stfgtrep_fade_update(Fade *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->base.step != 0) {
            obj->level += obj->step;
            if (obj->from_black == 0) {
                if (obj->level > 0xFF00) {
                    obj->level = 0xFF00;
                    obj->base.state = OBJECT_STATE_DONE;
                }
            } else if (obj->level < 0) {
                obj->level = 0;
                obj->base.state = OBJECT_STATE_DONE;
            }
        case OBJECT_STATE_DONE:
            stfgtrep_fade_draw(obj);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Fade *stfgtrep_fade_create(void) {
    Fade *obj = object_new(stfgtrep_fade_update, sizeof(Fade), 0);

    obj->start = stfgtrep_fade_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    return obj;
}

/* Creates a party member panel's text windows. */
void stfgtrep_member_create_windows(StfgtrepMember *obj, StfgtrepMemberData *data) {
    s32 i;
    s32 y;
    s16 y2;
    MessageWindow *win;

    y = obj->slot * 50;
    y2 = y + 0x38;
    data->name = message_create_window(obj->layer_id, 1, 0x3D, y + 0x28);
    data->level_label = message_create_window(obj->layer_id, 3, 0x3E, y2);
    data->level = message_create_window(obj->layer_id, 3, 0x5B, y2);
    y2 = y + 0x45;
    data->exp_label = message_create_window(obj->layer_id, 3, 0x6A, y2);
    data->exp = message_create_window(obj->layer_id, 3, 0x67, y2);
    for (i = 0; i < 3; i++) {
        y = obj->slot * 50 + i * 15;
        data->techniques[i].name = message_create_window(obj->layer_id, 1, 0x8D, y + 0x28);
        data->techniques[i].level = message_create_window(obj->layer_id, 3, 0x114, y + 0x2C);
    }
    data->messages = win = message_create_window(obj->layer_id, 1, 0x14, 0xC2);
    win->set_page_lines(win, 2);
}

/* Shows (`show`) or hides a party member panel's texts. */
void stfgtrep_member_show_text(StfgtrepMember *obj, StfgtrepMemberData *data, s32 show) {
    s32 i;
    s32 digimon;
    GamestateRecord *rec;

    if (show) {
        digimon = gamestate_data.funcs.get_party_member(obj->slot);
        rec = gamestate_data.funcs.get_record(digimon);
        data->name->set_text(data->name, (u8 *)rec->name, -1);
        data->level->set_line_number(data->level, 0, rec->stats.values[0]);
        data->level->measure(data->level, 1);
        data->level_label->set_text(data->level_label, cdload_module.files.get_file(records_language + 0x55), 3);
        data->exp->set_line_number(data->exp, 0, obj->exp_shown);
        data->exp->measure(data->exp, 1);
        data->exp->set_fixed_size(data->exp, 7, 0);
        data->exp_label->set_text(data->exp_label, cdload_module.files.get_file(records_language + 0x55), 1);
        gamestate_data.funcs.get_chosen_forms(digimon, obj->chosen_forms);
        for (i = 0; i < 3; i++) {
            if (obj->chosen_forms[i] >= 4) {
                gamestate_data.funcs.get_form(digimon, obj->chosen_forms[i], &obj->tech_record);
                data->techniques[i].name->set_text(data->techniques[i].name, cdload_module.files.get_file(records_language + 0x4E),
                                               records_get_digimon_func(obj->chosen_forms[i])->name_id);
                data->techniques[i].level->set_line_number(data->techniques[i].level, 0, obj->tech_record.level);
                data->techniques[i].level->measure(data->techniques[i].level, 1);
            } else {
                data->techniques[i].name->set_visible(data->techniques[i].name, 0);
                data->techniques[i].level->set_visible(data->techniques[i].level, 0);
            }
        }
    } else {
        data->name->set_visible(data->name, 0);
        data->level->set_visible(data->level, 0);
        data->level_label->set_visible(data->level_label, 0);
        data->exp->set_visible(data->exp, 0);
        data->exp_label->set_visible(data->exp_label, 0);
        for (i = 0; i < 3; i++) {
            data->techniques[i].name->set_visible(data->techniques[i].name, 0);
            data->techniques[i].level->set_visible(data->techniques[i].level, 0);
        }
    }
}

/* Draws a party member's panel: the Digimon (animated), the frame, the "level up" and technique highlights. */
void stfgtrep_member_draw(StfgtrepMember *obj) {
    Sprite spr;
    s32 y;
    s32 digimon;
    s32 i;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    spr.set_vram_pos(0x280, 0);
    if (obj->base.step >= 20) {
        if (obj->level_up != 0) {
            if (gfx_module.funcs.get_time() - obj->level_up_time >= 3) {
                obj->level_up_time = gfx_module.funcs.get_time();
                if (++obj->level_up_step > 10) {
                    obj->level_up_step = 10;
                }
            }
            spr.set_palette(obj->level_up_step);
            y = obj->slot * 50;
            spr.draw(cdload_module.get_subfile_by_id(0x079F0000), 0x22, 99, y + 43);
            spr.set_palette(0);
        }
        for (i = 0; i < 3; i++) {
            if (obj->tech_level_ups[i].level_up != 0) {
                if (gfx_module.funcs.get_time() - obj->tech_level_ups[i].time >= 3) {
                    obj->tech_level_ups[i].time = gfx_module.funcs.get_time();
                    if (++obj->tech_level_ups[i].step > 10) {
                        obj->tech_level_ups[i].step = 10;
                    }
                }
                spr.set_palette(obj->tech_level_ups[i].step);
                y = obj->slot * 50 + i * 15;
                spr.draw(cdload_module.get_subfile_by_id(0x079F0000), 0x23, 239, y + 35);
            }
        }
        spr.set_palette(0);
    }
    if (obj->highlighted != 0) {
        if (gfx_module.funcs.get_time() - obj->highlight_time >= 5) {
            obj->highlight_time = gfx_module.funcs.get_time();
            if (++obj->highlight_step >= 4) {
                obj->highlight_step = 0;
            }
        }
        spr.set_palette(obj->highlight_step);
        y = obj->slot * 50;
        spr.draw(cdload_module.get_subfile_by_id(0x079F0000), 0x24, 17, y + 38);
        if (obj->base.step >= 20 && obj->tech_shown >= 0 && obj->chosen_forms[obj->tech_shown] >= 4) {
            y = obj->slot * 50 + obj->tech_shown * 15;
            spr.draw(cdload_module.get_subfile_by_id(0x079F0000), 0x25, 135, y + 38);
        }
        spr.set_palette(0);
    }
    if (obj->panel_anim.level != 0) {
        if (obj->panel_anim.level != 0x1000) {
            spr.set_scale(obj->panel_anim.level, 0x1000, 0x1000);
        }
        digimon = gamestate_data.funcs.get_party_member(obj->slot);
        y = obj->slot * 50;
        if (obj->panel_anim.level != 0x1000) {
            spr.set_pivot(17, y + 58);
        }
        if (gfx_module.funcs.get_time() - obj->anim_time >= 13) {
            obj->anim_time = gfx_module.funcs.get_time();
            if (++obj->anim_frame >= 8 || stfgtrep_member_anims[digimon][obj->anim_frame] == -1) {
                obj->anim_frame = 0;
            }
        }
        spr.draw(cdload_module.get_subfile_by_id(0x079F0000), stfgtrep_member_anims[digimon][obj->anim_frame], 20, y + 40);
        if (obj->level_up != 0) {
            spr.set_palette(obj->level_up_step);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x079F0000), 0x1F, 17, y + 38);
        if (obj->level_up != 0) {
            spr.set_palette(0);
        }
        for (i = 0; i < 3; i++) {
            y = obj->slot * 50 + i * 15;
            if (obj->panel_anim.level != 0x1000) {
                spr.set_pivot(135, y + 45);
            }
            if (obj->tech_level_ups[i].level_up != 0) {
                spr.set_palette(obj->tech_level_ups[i].step);
            } else {
                spr.set_palette(0);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x079F0000), 0x20, 135, y + 38);
        }
    }
}

/* Counts the experience shown up by a step; returns 0 when done. */
s32 stfgtrep_member_count_exp(StfgtrepMember *obj) {
    StfgtrepMemberData *data = (StfgtrepMemberData *)obj->base.children;
    s32 digits;
    s32 n;
    s32 j;
    s32 m;
    s32 add;

    digits = 0;
    if (++obj->count_frames >= 10) {
        obj->exp_shown = obj->exp_after;
        stfgtrep_member_show_text(obj, data, 1);
        return 0;
    }
    n = obj->exp_after;
    for (m = 10; n != 0; m *= 10) {
        digits++;
        n -= n % m;
    }
    add = 1;
    for (m = digits - 1; m != 0; m--) {
        add *= 10;
        add++;
    }
    obj->exp_shown += add;
    if (obj->exp_shown > add * 9) {
        obj->exp_shown -= add * 9 + 1;
    }
    stfgtrep_member_show_text(obj, data, 1);
    return 1;
}

/* A party member panel's steps: open, then the technique experience, the experience, the level-up and the
 * new technique, each with its message, then close. */
void stfgtrep_member_run(StfgtrepMember *obj, StfgtrepMemberData *data) {
    GamestateRecord *rec;
    RecordsDigimon *t;
    s32 digimon;
    s32 id;
    s32 tech;
    s32 lv;
    s32 done;
    s32 i;

    switch (obj->base.step) {
    case 0:
        break;
    case 1:
        stfgtrep_funcs.anim_start(&obj->panel_anim, 1);
        obj->base.step++;
        break;
    case 2:
        if (stfgtrep_funcs.anim_update(&obj->panel_anim) != 0) {
            stfgtrep_member_show_text(obj, data, 1);
            obj->base.step = 5;
        }
        break;
    case 11:
        stfgtrep_funcs.anim_start(&obj->panel_anim, 0);
        stfgtrep_member_show_text(obj, data, 0);
        obj->base.step++;
        break;
    case 12:
        if (stfgtrep_funcs.anim_update(&obj->panel_anim) != 0) {
            obj->base.step = 15;
        }
        break;
    case 20:
        digimon = gamestate_data.funcs.get_party_member(obj->slot);
        gamestate_data.funcs.get_chosen_forms(digimon, obj->chosen_forms);
        if (records_battle_results.members[obj->slot].forms[obj->tech_shown] != 0) {
            tech = obj->chosen_forms[obj->tech_shown];
            if (tech >= 4) {
                obj->tech_level_ups[obj->tech_shown].level_up = stfgtrep_funcs.add_technique_exp(
                    digimon, obj->chosen_forms[obj->tech_shown],
                    stfgtrep_funcs.get_technique_exp(digimon, tech, stfgtrep_rewards[records_battle_results.battle].tech_exp,
                                               obj->main->techniques_used));
            }
        }
        obj->base.step++;
        break;
    case 21:
        if (obj->tech_level_ups[obj->tech_shown].level_up != 0) {
            if (obj->voice != -1) {
                sound_module.key_off(obj->sound_playing, obj->voice);
                obj->voice = -1;
            }
            obj->voice = sound_module.play(0x4000C);
            obj->sound_playing = 0x4000C;
            data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x55), 4);
            data->messages->set_speed(data->messages, 6);
            stfgtrep_member_show_text(obj, data, 1);
            obj->base.step = 25;
            obj->base.substep = 0;
        } else {
            obj->base.step = 24;
        }
        break;
    case 22:
        digimon = gamestate_data.funcs.get_party_member(obj->slot);
        gamestate_data.funcs.get_chosen_forms(digimon, obj->chosen_forms);
        lv = stfgtrep_funcs.learn_skill(digimon, obj->chosen_forms[obj->tech_shown]);
        if (lv != 0) {
            data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x55), 5);
            data->messages->set_line_text(data->messages, cdload_module.files.get_file(records_language + 0xA2), lv, 1);
            data->messages->set_speed(data->messages, 6);
            obj->base.step = 25;
            obj->base.substep = 1;
        } else {
            obj->base.step = 23;
        }
        break;
    case 23:
        digimon = gamestate_data.funcs.get_party_member(obj->slot);
        gamestate_data.funcs.get_chosen_forms(digimon, obj->chosen_forms);
        lv = stfgtrep_funcs.mark_skill(digimon, obj->chosen_forms[obj->tech_shown]);
        if (lv != 0) {
            data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x55), 6);
            data->messages->set_line_text(data->messages, cdload_module.files.get_file(records_language + 0xA2), lv, 1);
            data->messages->set_speed(data->messages, 6);
            obj->base.step = 25;
            obj->base.substep = 2;
        } else {
            obj->base.step = 24;
        }
        break;
    case 24:
        if (++obj->tech_shown >= 3) {
            obj->tech_shown = -1;
            obj->base.step = 40;
        } else {
            obj->base.step = 20;
        }
        break;
    case 25:
    case 45:
        if (data->messages->is_done(data->messages) != 0) {
            data->messages->set_visible(data->messages, 0);
            obj->main->arrow_shown = 0;
            switch (obj->base.substep) {
            case 0:
            case 1:
            default:
                obj->base.step = 22;
                break;
            case 2:
                obj->base.step = 23;
                break;
            case 3:
            case 4:
                obj->base.step = 41;
                break;
            case 5:
                obj->base.step = 50;
                break;
            }
        } else if (data->messages->is_waiting(data->messages) != 0) {
            if (PAD_PRESSED(13)) {
                sound_module.play(0x4001C);
            } else {
                obj->main->arrow_shown = 1;
            }
        } else if (PAD_PRESSED(13)) {
            data->messages->find_page_end(data->messages);
        }
        break;
    case 40:
        id = gamestate_data.funcs.get_party_member(obj->slot);
        if (stfgtrep_funcs.add_exp(id, obj->exp_gained) != 0) {
            rec = gamestate_data.funcs.get_record(id);
            obj->level_up = 1;
            if (obj->voice != -1) {
                sound_module.key_off(obj->sound_playing, obj->voice);
                obj->voice = -1;
            }
            obj->voice = sound_module.play(0x4000B);
            obj->sound_playing = 0x4000B;
            data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x55), 7);
            data->messages->set_line_text(data->messages, (u8 *)rec, -1, 1);
            data->messages->set_line_number(data->messages, 2, rec->stats.values[0]);
            data->messages->set_speed(data->messages, 6);
            obj->base.step = 45;
            obj->base.substep = 3;
        } else {
            obj->base.step = 41;
        }
        stfgtrep_member_show_text(obj, data, 1);
        break;
    case 41:
        i = stfgtrep_funcs.learn_technique(gamestate_data.funcs.get_party_member(obj->slot));
        if (i != 0) {
            t = records_get_digimon_func(i);
            data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x55), 8);
            data->messages->set_line_text(data->messages, cdload_module.files.get_file(records_language + 0x4E), t->name_id, 1);
            data->messages->set_speed(data->messages, 6);
            stfgtrep_member_show_text(obj, data, 1);
            obj->base.step = 45;
            obj->base.substep = 4;
            obj->tech_level_ups[2].unk_C = 1;
        } else if (obj->tech_level_ups[2].unk_C != 0) {
            obj->base.step = 42;
        } else {
            obj->base.step = 50;
        }
        break;
    case 42:
        if (gamestate_data.funcs.list_forms(gamestate_data.funcs.get_party_member(obj->slot), obj->forms) >= 4) {
            data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x55), 9);
            data->messages->set_speed(data->messages, 6);
            obj->base.step = 45;
            obj->base.substep = 5;
        } else {
            obj->base.step = 50;
        }
        break;
    case 50:
        done = 1;
        if (obj->level_up != 0) {
            done = obj->level_up_step >= 10;
        }
        if (done) {
            for (i = 0; i < 3; i++) {
                if (obj->tech_level_ups[i].level_up != 0 && obj->tech_level_ups[i].step < 10) {
                    done = 0;
                }
            }
            if (done) {
                obj->base.step = 5;
                obj->highlighted = 0;
            }
        }
        break;
    }
    if (obj->base.step >= 20 && stfgtrep_member_count_exp(obj) != 0) {
        sound_module.play(0x800452C6);
    }
}

void stfgtrep_member_update(StfgtrepMember *obj, StfgtrepMemberData *data) {
    GamestateRecord *rec;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stfgtrep_member_create_windows(obj, data);
        obj->panel_anim.duration = 10;
        rec = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(obj->slot));
        obj->exp_shown = rec->exp;
        if (obj->exp_shown > 999998) {
            obj->count_frames = 10;
        }
        if (obj->bonus_applied == 0) {
            obj->get_exp(obj);
        }
        obj->exp_after = rec->exp + obj->exp_gained;
        if (obj->exp_after > 999999) {
            obj->exp_after = 999999;
        }
        break;
    case OBJECT_STATE_RUN:
        stfgtrep_member_run(obj, data);
        stfgtrep_member_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void stfgtrep_member_open(StfgtrepMember *obj) {
    obj->base.step = 1;
}

void stfgtrep_member_close(StfgtrepMember *obj) {
    obj->base.step = 11;
}

void stfgtrep_member_show_techniques(StfgtrepMember *obj) {
    obj->base.step = 20;
}

/* Returns the experience the member gets, with a fifth more when it has item 0x141 equipped (once). */
s32 stfgtrep_member_get_exp(StfgtrepMember *obj) {
    GamestateRecord *rec;

    if (obj->bonus_applied == 0) {
        rec = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(obj->slot));
        if (rec->equipment[4] == 0x141 || rec->equipment[5] == 0x141) {
            obj->exp_gained += obj->exp_gained / 5;
        }
    }
    obj->bonus_applied = 1;
    return obj->exp_gained;
}

void stfgtrep_member_highlight(StfgtrepMember *obj) {
    obj->highlighted = 1;
}

StfgtrepMember *stfgtrep_member_create(StfgtrepMain *main, s32 slot, s32 exp) {
    StfgtrepMember *obj = object_new(stfgtrep_member_update, sizeof(StfgtrepMember), sizeof(StfgtrepMemberData));

    obj->open = stfgtrep_member_open;
    obj->close = stfgtrep_member_close;
    obj->show_techniques = stfgtrep_member_show_techniques;
    obj->get_exp = stfgtrep_member_get_exp;
    obj->highlight = stfgtrep_member_highlight;
    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->main = main;
    obj->voice = -1;
    obj->slot = slot;
    obj->exp_gained = exp;
    return obj;
}

/* Creates the report's text windows. */
void stfgtrep_main_create_windows(StfgtrepMain *obj, StfgtrepMainData *data) {
    MessageWindow *win;

    data->exp = message_create_window(obj->layer_id, 3, 0x42, 0x17);
    data->exp_label = message_create_window(obj->layer_id, 3, 0x45, 0x17);
    data->messages = win = message_create_window(obj->layer_id, 1, 0x14, 0xC2);
    win->set_page_lines(win, 2);
}

/* Draws the report: the "next" arrow, the scrolling background and the frames. */
void stfgtrep_main_draw(StfgtrepMain *obj) {
    Sprite spr;

    if (obj->arrow_shown != 0) {
        if (gfx_module.funcs.get_time() - obj->arrow_time >= 5) {
            obj->arrow_time = gfx_module.funcs.get_time();
            if (++obj->arrow_frame >= 5) {
                obj->arrow_frame = 0;
            }
        }
        sprite_init(&spr);
        spr.set_vram_pos(0x140, 0);
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 2);
        spr.set_palette(obj->arrow_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xA, 0x123, 0xD0);
    }
    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->odd_frame != 0) {
        obj->scroll++;
        obj->scroll = obj->scroll < 0x60 ? obj->scroll : 0;
        obj->odd_frame = 0;
    } else {
        obj->odd_frame = 1;
    }
    spr.draw(cdload_module.get_subfile_by_id(0x079F0000), 0x1E, obj->scroll, obj->scroll);
    spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
    if (obj->exp_anim.level != 0) {
        if (obj->exp_anim.level != 0x1000) {
            spr.set_scale(obj->exp_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0, 0x18);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x079F0000), 0x21, 0, 0xF);
    }
    if (obj->frame_anim.level != 0) {
        spr.set_scale(0x1000, obj->frame_anim.level, 0x1000);
        if (obj->frame_anim.level != 0x1000) {
            spr.set_pivot(0xA0, 0xCE);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x079F0000), 0x1D, 0xB, 0xBC);
    }
}

/* The report's steps: open the panels, give each member its experience, then the money and the card. */
void stfgtrep_main_run(StfgtrepMain *obj, StfgtrepMainData *data) {
    GamestateRecord *rec;
    s32 money;
    s32 ready;
    s32 done;
    s32 i;
    s32 j;

    switch (obj->base.step) {
    case 0:
    default:
        obj->base.step++;
        break;
    case 1:
        for (i = 0; i < obj->member_count; i++) {
            data->members[i]->open(data->members[i]);
        }
        obj->base.step++;
        break;
    case 4:
        ready = 1;
        for (j = 0; j < obj->member_count; j++) {
            if (data->members[j]->base.step != 5) {
                ready = 0;
                break;
            }
        }
        if (ready) {
            stfgtrep_funcs.anim_start(&obj->frame_anim, 1);
            obj->base.step++;
        }
        break;
    case 5:
        if (stfgtrep_funcs.anim_update(&obj->frame_anim) != 0) {
            obj->base.step++;
        }
        break;
    case 6:
        if (obj->base.substep < obj->member_count) {
            if (obj->member_exp[obj->base.substep] == 0) {
                obj->base.substep++;
                break;
            }
            stfgtrep_funcs.anim_start(&obj->exp_anim, 1);
            data->members[obj->base.substep]->highlight(data->members[obj->base.substep]);
            obj->base.step++;
        } else {
            obj->base.set_step(obj, 20);
        }
        break;
    case 7:
        if (stfgtrep_funcs.anim_update(&obj->exp_anim) != 0) {
            data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x55), 2);
            data->messages->set_line_number(data->messages, 1, obj->member_exp[obj->base.substep]);
            data->messages->set_speed(data->messages, 6);
            data->exp->set_line_number(data->exp, 0, obj->member_exp[obj->base.substep]);
            data->exp->measure(data->exp, 1);
            data->exp_label->set_text(data->exp_label, cdload_module.files.get_file(records_language + 0x55), 1);
            obj->base.step++;
        }
        break;
    case 8:
        if (data->messages->is_done(data->messages) != 0) {
            obj->arrow_shown = 0;
            data->messages->set_visible(data->messages, 0);
            obj->base.step = 10;
        } else if (data->messages->is_waiting(data->messages) != 0) {
            if (PAD_PRESSED(13)) {
                sound_module.play(0x4001C);
            } else {
                obj->arrow_shown = 1;
            }
        } else if (PAD_PRESSED(13)) {
            data->messages->find_page_end(data->messages);
        }
        break;
    case 10:
        data->members[obj->base.substep]->show_techniques(data->members[obj->base.substep]);
        obj->base.step++;
        break;
    case 11:
        if (data->members[obj->base.substep]->base.step == 5) {
            stfgtrep_funcs.anim_start(&obj->exp_anim, 0);
            data->exp->set_visible(data->exp, 0);
            data->exp_label->set_visible(data->exp_label, 0);
            obj->base.step++;
            obj->base.substep++;
        }
        break;
    case 12:
        if (stfgtrep_funcs.anim_update(&obj->exp_anim) != 0) {
            obj->base.step = 6;
        }
        break;
    case 20:
        rec = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(records_battle_results.member));
        money = stfgtrep_rewards[records_battle_results.battle].money;
        if (rec->equipment[4] == 0x142 || rec->equipment[5] == 0x142) {
            money += stfgtrep_rewards[records_battle_results.battle].money / 5;
        }
        data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x55), 10);
        data->messages->set_line_number(data->messages, 1, money);
        data->messages->set_speed(data->messages, 6);
        obj->base.step = 25;
        obj->base.substep = 0;
        gamestate_data.money += money;
        if (gamestate_data.money > 9999999) {
            gamestate_data.money = 9999999;
        }
        break;
    case 21:
        if (records_battle_results.item != 0) {
            data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x55), 11);
            data->messages->set_line_text(data->messages, cdload_module.files.get_file(records_language + 0x6A), records_battle_results.item, 1);
            data->messages->set_speed(data->messages, 6);
            obj->base.step = 25;
            obj->base.substep = 1;
            gamestate_data.items[records_battle_results.item]++;
            if (gamestate_data.items[records_battle_results.item] >= 100) {
                gamestate_data.items[records_battle_results.item] = 99;
            }
        } else {
            obj->base.step = 50;
        }
        break;
    case 25:
        if (data->messages->is_done(data->messages) != 0) {
            data->messages->set_visible(data->messages, 0);
            obj->arrow_shown = 0;
            switch (obj->base.substep) {
            case 0:
            default:
                obj->base.step = 21;
                break;
            case 1:
                obj->base.step = 50;
                break;
            }
        } else if (data->messages->is_waiting(data->messages) != 0) {
            if (PAD_PRESSED(13)) {
                sound_module.play(0x4001C);
            } else {
                obj->arrow_shown = 1;
            }
        } else if (PAD_PRESSED(13)) {
            data->messages->find_page_end(data->messages);
        }
        break;
    case 50:
        data->fade = stfgtrep_fade_create();
        data->fade->start(data->fade, 0, 30);
        stfgtrep_funcs.anim_start(&obj->frame_anim, 0);
        data->messages->set_visible(data->messages, 0);
        obj->base.step++;
        break;
    case 51:
        if (stfgtrep_funcs.anim_update(&obj->frame_anim) != 0) {
            for (i = 0; i < obj->member_count; i++) {
                data->members[i]->close(data->members[i]);
            }
            obj->base.step++;
        }
        break;
    case 52:
        done = 1;
        for (j = 0; j < obj->member_count; j++) {
            if (data->members[j]->base.step != 15) {
                done = 0;
                break;
            }
        }
        if (done) {
            obj->base.step++;
        }
        break;
    case 53:
        if (data->fade->base.state == OBJECT_STATE_DONE) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}

void stfgtrep_main_update(StfgtrepMain *obj, StfgtrepMainData *data) {
    StfgtrepMember *m;
    s32 exp;
    s32 n;
    s32 i;
    s32 j;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            stfgtrep_funcs.load();
            obj->base.step++;
            break;
        case 1:
            if (stfgtrep_funcs.is_loading() != 0) {
                break;
            }
            obj->base.next_state(obj);
            stfgtrep_main_create_windows(obj, data);
            for (i = 0; i < 3; i++) {
                if (gamestate_data.funcs.get_party_member(i) >= 0) {
                    obj->member_count++;
                }
            }
            n = 0;
            for (i = 0; i < 3; i++) {
                if (records_battle_results.members[i].took_part != 0) {
                    n++;
                    for (j = 0; j < 3; j++) {
                        if (records_battle_results.members[i].forms[j] != 0) {
                            obj->techniques_used++;
                        }
                    }
                }
            }
            switch (n) {
            case 1:
            default:
                exp = stfgtrep_rewards[records_battle_results.battle].exp;
                break;
            case 2:
                exp = stfgtrep_rewards[records_battle_results.battle].exp * 6 / 10;
                break;
            case 3:
                exp = stfgtrep_rewards[records_battle_results.battle].exp / 3;
                break;
            }
            for (i = 0; i < obj->member_count; i++) {
                if (records_battle_results.members[i].took_part != 0) {
                    m = stfgtrep_member_create(obj, i, exp);
                    data->members[i] = m;
                    obj->member_exp[i] = m->get_exp(m);
                } else {
                    data->members[i] = stfgtrep_member_create(obj, i, 0);
                }
            }
            obj->exp_anim.duration = 10;
            obj->frame_anim.duration = 10;
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        stfgtrep_main_run(obj, data);
        stfgtrep_main_draw(obj);
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        gamestate_data.funcs.set_next_map(gamestate_data.field_map, 0);
        break;
    }
}

StfgtrepMain *stfgtrep_main_create(void) {
    StfgtrepMain *obj = object_new(stfgtrep_main_update, sizeof(StfgtrepMain), sizeof(StfgtrepMainData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 7;
    return obj;
}

/* Starts loading the overlay's files (stfgtrep_funcs.load). */
void stfgtrep_load_files(void) {
    Tim tim;

    tim_init(&tim);
    tim.set_image_pos(0x280, 0);
    tim.load_all(cdload_module.get_subfile_by_id(0x07A00000));
    cdload_module.queue_file(records_language + 0x55);
    cdload_module.queue_file(records_language + 0x4E);
    cdload_module.queue_file(records_language + 0x6A);
}

/* Returns non-zero while one of the overlay's files is still loading (stfgtrep_funcs.is_loading). */
s32 stfgtrep_is_loading(void) {
    if (!cdload_module.is_loading(records_language + 0x55) && !cdload_module.is_loading(records_language + 0x4E)) {
        return cdload_module.is_loading(records_language + 0x6A) != 0;
    }
    return 1;
}

/* window_anim_start (include/window_anim.h), byte for byte. */
void stfgtrep_window_anim_start(WindowAnim *anim, s32 open) {
    anim->running = 1;
    if (open) {
        sound_module.play(0x40019);
        anim->step = 0x1000 / anim->duration;
        anim->level = 0;
    } else {
        sound_module.play(0x4001A);
        anim->level = 0x1000;
        anim->step = -(0x1000 / anim->duration * 2);
    }
}

/* window_anim_update (include/window_anim.h), byte for byte. */
s32 stfgtrep_window_anim_update(WindowAnim *anim) {
    if (anim->running == 0) {
        return 1;
    }
    anim->level += anim->step;
    if (anim->step > 0) {
        if (anim->level > 0x1000) {
            anim->level = 0x1000;
            anim->running = 0;
            return 1;
        }
    } else if (anim->level < 0) {
        anim->level = 0;
        anim->running = 0;
        return 1;
    }
    return 0;
}

void stfgtrep_tween_start(Tween *obj, s32 from, s32 to, s32 frames) {
    if (from != to) {
        obj->duration = frames;
        obj->acc = from << 8;
        obj->value = from;
        obj->target = to;
        obj->running = 1;
        obj->step = ((to - from) << 8) / obj->duration;
    }
}

s32 stfgtrep_tween_update(Tween *obj) {
    if (obj->running == 0) {
        return 1;
    }
    obj->acc += obj->step;
    obj->value = obj->acc >> 8;
    if (obj->step > 0) {
        if (obj->value > obj->target) {
            obj->value = obj->target;
            obj->running = 0;
            return 1;
        }
    } else if (obj->value < obj->target) {
        obj->value = obj->target;
        obj->running = 0;
        return 1;
    }
    return 0;
}

/* Raises `digimon`'s stats for reaching level `lv`: random gains that depend on its growth classes and,
 * for max HP/MP, shrink as the level goes up. */
void stfgtrep_raise_stats(s32 digimon, s32 lv) {
    GamestateRecord *rec;
    RecordsDigimon *d;
    GamestateStats *stats;
    s32 k;
    s32 i;
    s32 base;
    s32 gain;

    rec = gamestate_data.funcs.get_record(digimon);
    d = &records_digimon[digimon];
    if (lv < 5) {
        k = 0;
    } else if (lv < 20) {
        k = 1;
    } else if (lv < 40) {
        k = 2;
    } else {
        k = 3;
    }
    base = d->hp_mp_gains[0];
    gain = base - stfgtrep_hp_mp_gain_cut[k] + stfgtrep_hp_mp_gain_random[pad_random.next() % 9];
    rec->stats.values[3] += gain;
    if (rec->stats.values[3] >= 10000) {
        rec->stats.values[3] = 9999;
    }
    base = d->hp_mp_gains[1];
    gain = base - stfgtrep_hp_mp_gain_cut[k] + stfgtrep_hp_mp_gain_random[pad_random.next() % 9];
    rec->stats.values[5] += gain;
    if (rec->stats.values[5] >= 10000) {
        rec->stats.values[5] = 9999;
    }
    stats = &rec->stats;
    if (lv < 5) {
        k = 0;
    } else if (lv < 20) {
        k = 1;
    } else if (lv < 40) {
        k = 2;
    } else if (lv < 60) {
        k = 3;
    } else if (lv < 80) {
        k = 4;
    } else {
        k = 5;
    }
    for (i = 0; i < 6; i++) {
        base = d->stat_gains[i];
        gain = stfgtrep_stat_gains[k][base + pad_random.next() % 5];
        stats->stats[i] += gain;
        if (stats->stats[i] >= 1000) {
            stats->stats[i] = 999;
        }
    }
    if (lv <= 40) {
        for (i = 0; i < 7; i++) {
            base = d->resist_gains[i];
            gain = stfgtrep_resist_gains[base + pad_random.next() % 4];
            stats->resists[i] += gain;
            if (stats->resists[i] >= 1000) {
                stats->resists[i] = 999;
            }
        }
    }
}

/* Adds `exp` to `digimon`'s experience and raises its level (up to 99, with stfgtrep_raise_stats's stat
 * gains); returns 1 if it went up. */
s32 stfgtrep_add_exp(s32 digimon, s32 exp) {
    GamestateRecord *rec;
    RecordsDigimon *d;
    s32 up;
    s32 lv;
    s32 k;
    s32 more;

    rec = gamestate_data.funcs.get_record(digimon);
    d = &records_digimon[digimon];
    rec->exp += exp;
    if (rec->exp > 999999) {
        rec->exp = 999999;
    }
    up = 0;
    lv = rec->stats.values[0];
    do {
        lv++;
        if (lv < 5) {
            k = 0;
        } else if (lv < 20) {
            k = 1;
        } else if (lv < 40) {
            k = 2;
        } else if (lv < 100) {
            k = 3;
        } else {
            break;
        }
        if ((lv * lv * lv + lv * 5 - 6) * d->exp_curve / 10 + stfgtrep_level_exp_bonus[k] < rec->exp) {
            if (++rec->stats.values[0] < 100) {
                stfgtrep_raise_stats(digimon, rec->stats.values[0]);
            } else {
                rec->stats.values[0] = 99;
            }
            rec->stats.values[1] += 5;
            if (rec->stats.values[1] >= 100) {
                rec->stats.values[1] = 99;
            }
            more = up = 1;
        } else {
            more = 0;
        }
    } while (more);
    return up;
}

/* Teaches `digimon` the first technique of its list whose requirements it meets, and puts it in a free slot;
 * returns it, or 0. */
s32 stfgtrep_learn_technique(s32 digimon) {
    GamestateForm rec;
    s16 slots[3];
    StfgtrepLearn *list;
    GamestateRecord *r;
    s32 id;
    s32 i;
    s32 n;
    s32 j;

    list = stfgtrep_learn_lists[digimon];
    for (n = 0; n < 44; n++) {
        id = records_digimon[list[n].technique - 1].id;
        if (gamestate_data.funcs.get_form(digimon, id, &rec) >= 0 || id <= 0) {
            continue;
        }
        for (i = 0; i < 2; i++) {
            if (list[n].requires[i].technique != 0) {
                if (gamestate_data.funcs.get_form(digimon, records_digimon[list[n].requires[i].technique - 1].id, &rec) == -1) {
                    id = -1;
                } else if (rec.level < list[n].requires[i].level) {
                    id = -1;
                }
            }
        }
        if (id <= 0) {
            continue;
        }
        if (list[n].stat != 0) {
            r = gamestate_data.funcs.get_record(digimon);
            if (list[n].stat < 7) {
                if ((&r->stats.values[5])[list[n].stat] < list[n].min) {
                    id = -1;
                }
            } else if (list[n].stat == 7) {
                if (r->stats.values[0] < list[n].min) {
                    id = -1;
                }
            } else if (list[n].stat >= 8) {
                if ((&r->stats.values[4])[list[n].stat] < list[n].min) {
                    id = -1;
                }
            }
        }
        if (id <= 0) {
            continue;
        }
        gamestate_data.funcs.add_form(digimon, id);
        gamestate_data.funcs.get_chosen_forms(digimon, slots);
        for (j = 0; j < 3; j++) {
            if (slots[j] < 3) {
                slots[j] = id;
                gamestate_data.funcs.set_chosen_forms(digimon, slots);
                break;
            }
        }
        return id;
    }
    return 0;
}

/* Adds `exp` to technique `tech` of `digimon` and raises its level (up to 99); returns 1 if it went up. */
s32 stfgtrep_add_technique_exp(s32 digimon, s32 tech, s32 exp) {
    GamestateForm rec;
    RecordsDigimon *t;
    s32 up;
    s32 more;
    s32 need;
    s32 x;

    t = records_get_digimon_func(tech);
    gamestate_data.funcs.get_form(digimon, tech, &rec);
    rec.exp += exp;
    if (rec.exp > 9999999) {
        rec.exp = 9999999;
    }
    if (rec.level >= 99) {
        return 0;
    }
    up = 0;
    do {
        if (rec.level + 1 <= t->level_thresholds[5]) {
            need = rec.level * 10;
        } else {
            x = t->level_thresholds[5] - 1;
            need = x * 10 + (rec.level - x) * 50;
        }
        more = 0;
        if (rec.exp >= need) {
            up = 1;
            more = ++rec.level < 99;
        }
    } while (more);
    gamestate_data.funcs.put_form(digimon, tech, &rec);
    return up;
}

/* Learns the next skill of technique `tech` its level allows; returns it, or 0. */
u16 stfgtrep_learn_skill(s32 digimon, s32 tech) {
    GamestateForm rec;
    RecordsDigimon *t;
    s32 i;

    gamestate_data.funcs.get_form(digimon, tech, &rec);
    for (i = 0; i < 6; i++) {
        if (rec.techniques[i] == 0) {
            t = records_get_digimon_func(tech);
            if (t->techniques[i + 1] != 0 && rec.level >= t->technique_levels[i]) {
                rec.techniques[i] = t->techniques[i + 1];
                if (i == 5) {
                    rec.techniques[i] |= 0x8000;
                }
                gamestate_data.funcs.put_form(digimon, tech, &rec);
                return t->techniques[i + 1];
            }
        }
    }
    return 0;
}

/* Marks (0x2000) the next skill of technique `tech` its level allows; returns it, or 0. */
s32 stfgtrep_mark_skill(s32 digimon, s32 tech) {
    GamestateForm rec;
    RecordsDigimon *t;
    s32 i;

    gamestate_data.funcs.get_form(digimon, tech, &rec);
    t = records_get_digimon_func(tech);
    for (i = 0; i < 5; i++) {
        if (rec.techniques[i] > 0 && !(rec.techniques[i] & 0x2000) && rec.level >= t->level_thresholds[i]) {
            rec.techniques[i] |= 0x2000;
            gamestate_data.funcs.put_form(digimon, tech, &rec);
            return rec.techniques[i] & 0x1FFF;
        }
    }
    return 0;
}

/* The experience technique `tech` gets from `exp` (shared by `n` users), 1..10 below its level threshold,
 * 1..50 above. */
s32 stfgtrep_get_technique_exp(s32 digimon, s32 tech, s32 exp, s32 n) {
    GamestateForm rec;
    RecordsDigimon *t;
    s32 lv;
    s32 a;
    s32 e;
    s32 scaled;

    lv = gamestate_data.funcs.get_record(digimon)->stats.values[0];
    scaled = exp * 10;
    if (lv < 51) {
        a = scaled / lv;
    } else {
        a = exp / 5;
    }
    switch (n) {
    case 0:
    case 1:
        e = a;
        break;
    case 2:
        e = a * 6 / 10;
        break;
    default:
        e = a / n;
        break;
    }
    t = records_get_digimon_func(tech);
    gamestate_data.funcs.get_form(digimon, tech, &rec);
    if (e <= 0) {
        e = 1;
    } else if (rec.level < t->level_thresholds[5]) {
        if (e > 10) {
            e = 10;
        }
    } else if (e > 50) {
        e = 50;
    }
    return e;
}

s32 stfgtrep_get_technique_exp(s32 digimon, s32 tech, s32 exp, s32 n);

StfgtrepReward stfgtrep_rewards[] = {
    { 0, 0, 0 },
    { 12, 120, 240 },
    { 14, 140, 280 },
    { 11, 176, 475 },
    { 14, 216, 675 },
    { 27, 268, 645 },
    { 29, 435, 870 },
    { 32, 325, 500 },
    { 32, 320, 740 },
    { 34, 340, 1000 },
    { 35, 718, 1490 },
    { 40, 380, 800 },
    { 43, 440, 800 },
    { 45, 450, 900 },
    { 41, 627, 1220 },
    { 46, 460, 930 },
    { 43, 649, 1335 },
    { 47, 470, 960 },
    { 46, 705, 1430 },
    { 46, 703, 1410 },
    { 47, 715, 1420 },
    { 48, 719, 1395 },
    { 49, 737, 1470 },
    { 55, 550, 1100 },
    { 57, 570, 1300 },
    { 50, 757, 1450 },
    { 52, 788, 1560 },
    { 52, 783, 1515 },
    { 53, 795, 1580 },
    { 56, 842, 1655 },
    { 99, 990, 1980 },
    { 53, 530, 1060 },
    { 57, 1224, 2515 },
    { 57, 1700, 3000 },
    { 1, 6, 10 },
    { 1, 5, 20 },
    { 3, 17, 30 },
    { 4, 19, 40 },
    { 8, 39, 80 },
    { 2, 10, 25 },
    { 4, 20, 40 },
    { 2, 11, 20 },
    { 3, 16, 30 },
    { 3, 15, 35 },
    { 4, 21, 40 },
    { 4, 22, 45 },
    { 16, 80, 160 },
    { 5, 27, 55 },
    { 13, 66, 130 },
    { 5, 25, 50 },
    { 6, 31, 60 },
    { 6, 30, 60 },
    { 6, 29, 70 },
    { 8, 41, 85 },
    { 8, 40, 80 },
    { 9, 46, 90 },
    { 31, 154, 310 },
    { 13, 96, 160 },
    { 10, 50, 190 },
    { 18, 91, 180 },
    { 29, 145, 290 },
    { 20, 98, 200 },
    { 27, 136, 240 },
    { 40, 200, 400 },
    { 24, 118, 200 },
    { 20, 101, 490 },
    { 12, 58, 120 },
    { 23, 116, 220 },
    { 20, 100, 205 },
    { 20, 102, 200 },
    { 22, 112, 200 },
    { 44, 220, 400 },
    { 45, 223, 440 },
    { 22, 109, 220 },
    { 22, 110, 225 },
    { 25, 125, 250 },
    { 29, 147, 290 },
    { 23, 115, 235 },
    { 23, 117, 230 },
    { 24, 123, 250 },
    { 24, 120, 200 },
    { 25, 124, 300 },
    { 26, 129, 190 },
    { 47, 235, 470 },
    { 47, 234, 100 },
    { 27, 135, 270 },
    { 27, 134, 270 },
    { 46, 233, 450 },
    { 29, 145, 320 },
    { 30, 152, 300 },
    { 29, 146, 280 },
    { 31, 155, 330 },
    { 36, 225, 770 },
    { 43, 215, 420 },
    { 34, 168, 340 },
    { 38, 189, 380 },
    { 35, 176, 350 },
    { 35, 175, 400 },
    { 45, 225, 450 },
    { 34, 172, 320 },
    { 36, 183, 400 },
    { 36, 179, 360 },
    { 45, 225, 420 },
    { 37, 185, 370 },
    { 44, 221, 440 },
    { 35, 174, 350 },
    { 34, 170, 340 },
    { 36, 180, 360 },
    { 39, 196, 390 },
    { 36, 180, 360 },
    { 41, 205, 450 },
    { 41, 204, 400 },
    { 41, 205, 410 },
    { 41, 205, 420 },
    { 41, 206, 420 },
    { 43, 215, 420 },
    { 46, 230, 460 },
    { 43, 218, 430 },
    { 49, 244, 490 },
    { 43, 217, 430 },
    { 43, 214, 390 },
    { 44, 223, 420 },
    { 44, 221, 470 },
    { 44, 223, 460 },
    { 43, 215, 430 },
    { 43, 216, 480 },
    { 44, 220, 450 },
    { 44, 222, 610 },
    { 54, 273, 520 },
    { 49, 247, 490 },
    { 45, 225, 460 },
    { 45, 224, 440 },
    { 44, 222, 460 },
    { 45, 226, 500 },
    { 45, 227, 440 },
    { 55, 276, 530 },
    { 47, 235, 440 },
    { 48, 243, 500 },
    { 56, 280, 560 },
    { 47, 235, 500 },
    { 55, 277, 530 },
    { 56, 282, 550 },
    { 49, 246, 490 },
    { 55, 277, 540 },
    { 56, 279, 600 },
    { 9, 45, 90 },
    { 30, 150, 290 },
    { 8, 40, 80 },
    { 21, 105, 210 },
    { 22, 112, 220 },
    { 33, 167, 350 },
    { 33, 165, 330 },
    { 19, 76, 190 },
    { 41, 205, 400 },
    { 21, 104, 215 },
    { 37, 186, 370 },
    { 22, 158, 300 },
    { 36, 180, 360 },
    { 37, 184, 370 },
    { 35, 175, 350 },
    { 47, 236, 460 },
    { 42, 210, 430 },
    { 46, 232, 460 },
    { 48, 242, 480 },
    { 46, 231, 460 },
    { 31, 222, 660 },
    { 39, 299, 770 },
    { 43, 344, 880 },
    { 43, 366, 990 },
    { 44, 399, 1110 },
    { 40, 201, 400 },
    { 43, 215, 430 },
    { 45, 227, 470 },
    { 32, 160, 330 },
    { 40, 250, 800 },
    { 48, 241, 480 },
    { 36, 179, 360 },
    { 41, 203, 410 },
    { 47, 235, 420 },
    { 44, 221, 450 },
    { 48, 240, 480 },
    { 48, 239, 480 },
    { 44, 220, 500 },
    { 48, 243, 480 },
    { 47, 236, 460 },
    { 54, 271, 550 },
    { 54, 272, 510 },
    { 49, 245, 490 },
    { 29, 291, 600 },
    { 29, 299, 590 },
    { 29, 451, 890 },
    { 32, 160, 325 },
    { 29, 289, 580 },
    { 44, 672, 1370 },
    { 55, 558, 1080 },
    { 49, 493, 990 },
    { 50, 503, 1000 },
    { 51, 770, 1500 },
    { 53, 530, 1060 },
    { 50, 508, 990 },
    { 1, 4, 50 },
    { 4, 65, 145 },
    { 41, 661, 1250 },
    { 7, 125, 220 },
    { 7, 79, 155 },
    { 6, 103, 200 },
    { 9, 45, 180 },
    { 43, 737, 1475 },
    { 6, 100, 225 },
    { 7, 109, 210 },
    { 10, 101, 190 },
    { 10, 105, 210 },
    { 10, 100, 200 },
    { 40, 700, 1305 },
    { 17, 269, 530 },
    { 9, 158, 290 },
    { 41, 205, 820 },
    { 38, 569, 1195 },
    { 38, 565, 1175 },
    { 38, 595, 1110 },
    { 40, 607, 1255 },
    { 40, 682, 1335 },
    { 41, 642, 1265 },
    { 42, 210, 840 },
    { 58, 904, 1730 },
    { 42, 641, 1220 },
    { 42, 638, 1145 },
    { 34, 342, 725 },
    { 35, 524, 990 },
    { 35, 533, 1110 },
    { 35, 357, 715 },
    { 35, 523, 1050 },
    { 36, 361, 680 },
    { 35, 544, 1110 },
    { 44, 665, 1330 },
    { 44, 664, 1510 },
    { 44, 656, 1375 },
    { 45, 454, 545 },
    { 44, 671, 1320 },
    { 44, 669, 1380 },
    { 45, 680, 1485 },
    { 45, 456, 910 },
    { 45, 458, 900 },
    { 52, 791, 1625 },
    { 52, 800, 1565 },
    { 52, 525, 1570 },
    { 52, 814, 1600 },
    { 52, 786, 1570 },
    { 52, 791, 1740 },
    { 53, 813, 1680 },
    { 57, 977, 1920 },
    { 53, 529, 1120 },
    { 53, 535, 1600 },
    { 1, 6, 10 },
    { 1, 6, 10 },
    { 1, 6, 10 },
    { 1, 6, 10 },
    { 1, 6, 10 },
    { 1, 6, 10 },
    { 1, 6, 10 },
    { 1, 6, 10 },
    { 1, 6, 10 },
    { 94, 2365, 5475 },
    { 10, 161, 300 },
    { 17, 160, 340 },
    { 26, 250, 520 },
    { 28, 280, 560 },
    { 29, 300, 580 },
    { 28, 280, 560 },
    { 28, 280, 560 },
    { 28, 280, 560 },
    { 19, 200, 380 },
    { 6, 63, 115 },
    { 43, 215, 430 },
    { 43, 216, 480 },
    { 44, 223, 460 },
    { 44, 221, 470 },
    { 45, 225, 420 },
    { 23, 240, 480 },
    { 36, 538, 1080 },
    { 33, 336, 640 },
    { 26, 394, 820 },
    { 34, 520, 1040 },
    { 13, 204, 420 },
    { 31, 468, 980 },
    { 34, 339, 310 },
    { 31, 475, 940 },
    { 29, 445, 925 },
    { 36, 544, 1220 },
    { 40, 406, 810 },
    { 44, 448, 880 },
    { 37, 566, 1165 },
    { 19, 297, 580 },
    { 23, 230, 470 },
    { 5, 77, 155 },
    { 41, 621, 1240 },
    { 42, 635, 1410 },
    { 30, 466, 980 },
    { 37, 601, 1190 },
    { 40, 399, 840 },
    { 23, 230, 470 },
    { 60, 565, 1200 },
    { 60, 577, 1200 },
    { 65, 684, 1300 },
    { 60, 621, 1200 },
    { 75, 1855, 3660 },
    { 75, 1857, 3875 },
    { 75, 1541, 2695 },
    { 75, 1503, 3180 },
    { 60, 600, 1200 },
    { 70, 700, 1400 },
    { 70, 700, 1400 },
    { 70, 320, 740 },
    { 34, 340, 1000 },
    { 40, 380, 800 },
    { 44, 450, 815 },
    { 65, 650, 1300 },
    { 65, 650, 1310 },
    { 70, 700, 1430 },
    { 70, 700, 1400 },
    { 75, 750, 1710 },
    { 75, 750, 1500 },
    { 28, 144, 0 },
    { 15, 153, 275 },
    { 68, 4, 0 },
    { 70, 4, 0 },
    { 70, 4, 0 },
    { 7, 35, 70 },
    { 12, 60, 120 },
    { 17, 85, 170 },
    { 27, 135, 270 },
    { 37, 185, 370 },
    { 42, 210, 420 },
    { 47, 235, 470 },
    { 47, 235, 470 },
};

StfgtrepLearn stfgtrep_learn_0[44] = {
    { 9, { { 0, 1 }, { 0, 1 } }, 7, 5 },
    { 10, { { 14, 20 }, { 0, 1 } }, 0, 1 },
    { 11, { { 14, 30 }, { 0, 1 } }, 5, 280 },
    { 12, { { 9, 20 }, { 0, 1 } }, 0, 1 },
    { 13, { { 12, 10 }, { 0, 1 } }, 0, 1 },
    { 14, { { 27, 50 }, { 0, 1 } }, 8, 200 },
    { 15, { { 33, 20 }, { 0, 1 } }, 0, 1 },
    { 16, { { 27, 30 }, { 0, 1 } }, 11, 200 },
    { 17, { { 32, 20 }, { 0, 1 } }, 0, 1 },
    { 18, { { 33, 10 }, { 0, 1 } }, 0, 1 },
    { 19, { { 27, 20 }, { 0, 1 } }, 9, 360 },
    { 20, { { 0, 1 }, { 0, 1 } }, 7, 20 },
    { 21, { { 10, 50 }, { 0, 1 } }, 0, 1 },
    { 22, { { 11, 50 }, { 0, 1 } }, 0, 1 },
    { 23, { { 12, 40 }, { 0, 1 } }, 7, 15 },
    { 24, { { 12, 50 }, { 0, 1 } }, 14, 140 },
    { 25, { { 13, 5 }, { 18, 5 } }, 0, 1 },
    { 26, { { 14, 50 }, { 0, 1 } }, 0, 1 },
    { 27, { { 15, 40 }, { 0, 1 } }, 0, 1 },
    { 28, { { 16, 50 }, { 0, 1 } }, 0, 1 },
    { 29, { { 17, 40 }, { 0, 1 } }, 0, 1 },
    { 30, { { 20, 20 }, { 0, 1 } }, 13, 140 },
    { 31, { { 18, 40 }, { 0, 1 } }, 0, 1 },
    { 32, { { 27, 40 }, { 0, 1 } }, 4, 280 },
    { 33, { { 0, 1 }, { 0, 1 } }, 7, 40 },
    { 34, { { 21, 99 }, { 0, 1 } }, 0, 1 },
    { 35, { { 22, 99 }, { 0, 1 } }, 0, 1 },
    { 36, { { 23, 99 }, { 0, 1 } }, 0, 1 },
    { 37, { { 25, 50 }, { 0, 1 } }, 0, 1 },
    { 38, { { 26, 99 }, { 0, 1 } }, 0, 1 },
    { 39, { { 27, 99 }, { 0, 1 } }, 0, 1 },
    { 40, { { 28, 99 }, { 0, 1 } }, 0, 1 },
    { 41, { { 30, 40 }, { 0, 1 } }, 13, 200 },
    { 42, { { 19, 40 }, { 0, 1 } }, 0, 1 },
    { 43, { { 24, 99 }, { 0, 1 } }, 0, 1 },
    { 44, { { 37, 99 }, { 0, 1 } }, 0, 1 },
    { 45, { { 29, 99 }, { 0, 1 } }, 0, 1 },
    { 46, { { 41, 99 }, { 0, 1 } }, 0, 1 },
    { 47, { { 37, 40 }, { 31, 40 } }, 0, 1 },
    { 48, { { 42, 99 }, { 0, 1 } }, 0, 1 },
    { 49, { { 36, 40 }, { 41, 40 } }, 0, 1 },
    { 50, { { 44, 40 }, { 49, 40 } }, 0, 1 },
    { 51, { { 45, 40 }, { 38, 40 } }, 0, 1 },
    { 52, { { 47, 99 }, { 43, 99 } }, 0, 1 },
};

StfgtrepLearn stfgtrep_learn_1[44] = {
    { 9, { { 23, 30 }, { 0, 1 } }, 0, 1 },
    { 10, { { 31, 20 }, { 0, 1 } }, 0, 1 },
    { 11, { { 0, 1 }, { 0, 1 } }, 7, 5 },
    { 12, { { 26, 20 }, { 0, 1 } }, 8, 200 },
    { 13, { { 15, 10 }, { 0, 1 } }, 0, 1 },
    { 14, { { 35, 20 }, { 0, 1 } }, 1, 480 },
    { 15, { { 11, 20 }, { 0, 1 } }, 3, 80 },
    { 16, { { 29, 20 }, { 0, 1 } }, 11, 280 },
    { 17, { { 35, 10 }, { 0, 1 } }, 0, 1 },
    { 18, { { 35, 30 }, { 0, 1 } }, 5, 400 },
    { 19, { { 16, 20 }, { 0, 1 } }, 9, 280 },
    { 20, { { 9, 50 }, { 0, 1 } }, 0, 1 },
    { 21, { { 10, 50 }, { 0, 1 } }, 0, 1 },
    { 22, { { 0, 1 }, { 0, 1 } }, 7, 20 },
    { 23, { { 12, 40 }, { 0, 1 } }, 0, 1 },
    { 24, { { 12, 50 }, { 0, 1 } }, 14, 160 },
    { 25, { { 13, 5 }, { 18, 5 } }, 0, 1 },
    { 26, { { 14, 50 }, { 0, 1 } }, 0, 1 },
    { 27, { { 15, 40 }, { 0, 1 } }, 7, 15 },
    { 28, { { 16, 50 }, { 0, 1 } }, 0, 1 },
    { 29, { { 17, 50 }, { 0, 1 } }, 0, 1 },
    { 30, { { 22, 20 }, { 0, 1 } }, 13, 150 },
    { 31, { { 18, 50 }, { 0, 1 } }, 0, 1 },
    { 32, { { 11, 30 }, { 0, 1 } }, 4, 80 },
    { 33, { { 20, 99 }, { 0, 1 } }, 0, 1 },
    { 34, { { 21, 99 }, { 0, 1 } }, 0, 1 },
    { 35, { { 0, 1 }, { 0, 1 } }, 7, 40 },
    { 36, { { 23, 99 }, { 0, 1 } }, 0, 1 },
    { 37, { { 25, 50 }, { 0, 1 } }, 0, 1 },
    { 38, { { 26, 99 }, { 0, 1 } }, 0, 1 },
    { 39, { { 27, 99 }, { 0, 1 } }, 0, 1 },
    { 40, { { 28, 99 }, { 0, 1 } }, 0, 1 },
    { 41, { { 30, 40 }, { 0, 1 } }, 13, 200 },
    { 42, { { 19, 40 }, { 0, 1 } }, 0, 1 },
    { 43, { { 24, 99 }, { 0, 1 } }, 0, 1 },
    { 44, { { 37, 99 }, { 0, 1 } }, 0, 1 },
    { 45, { { 29, 99 }, { 0, 1 } }, 0, 1 },
    { 46, { { 41, 99 }, { 0, 1 } }, 0, 1 },
    { 47, { { 37, 40 }, { 31, 40 } }, 0, 1 },
    { 48, { { 42, 99 }, { 0, 1 } }, 0, 1 },
    { 49, { { 36, 40 }, { 41, 40 } }, 0, 1 },
    { 50, { { 44, 40 }, { 49, 40 } }, 0, 1 },
    { 51, { { 45, 40 }, { 38, 40 } }, 0, 1 },
    { 52, { { 47, 99 }, { 43, 99 } }, 0, 1 },
};

StfgtrepLearn stfgtrep_learn_2[44] = {
    { 9, { { 23, 20 }, { 0, 1 } }, 1, 250 },
    { 10, { { 0, 1 }, { 0, 1 } }, 7, 5 },
    { 11, { { 14, 20 }, { 0, 1 } }, 2, 460 },
    { 12, { { 34, 10 }, { 0, 1 } }, 0, 1 },
    { 13, { { 10, 20 }, { 0, 1 } }, 0, 1 },
    { 14, { { 20, 20 }, { 0, 1 } }, 5, 320 },
    { 15, { { 19, 10 }, { 0, 1 } }, 3, 200 },
    { 16, { { 21, 40 }, { 0, 1 } }, 11, 120 },
    { 17, { { 24, 20 }, { 0, 1 } }, 3, 300 },
    { 18, { { 23, 30 }, { 0, 1 } }, 12, 200 },
    { 19, { { 28, 30 }, { 0, 1 } }, 9, 110 },
    { 20, { { 9, 50 }, { 0, 1 } }, 0, 1 },
    { 21, { { 0, 1 }, { 0, 1 } }, 7, 20 },
    { 22, { { 11, 50 }, { 0, 1 } }, 0, 1 },
    { 23, { { 12, 40 }, { 0, 1 } }, 0, 1 },
    { 24, { { 12, 50 }, { 0, 1 } }, 14, 300 },
    { 25, { { 13, 5 }, { 18, 5 } }, 0, 1 },
    { 26, { { 14, 50 }, { 0, 1 } }, 0, 1 },
    { 27, { { 15, 40 }, { 0, 1 } }, 0, 1 },
    { 28, { { 16, 50 }, { 0, 1 } }, 0, 1 },
    { 29, { { 17, 40 }, { 0, 1 } }, 0, 1 },
    { 30, { { 21, 20 }, { 0, 1 } }, 13, 180 },
    { 31, { { 18, 50 }, { 0, 1 } }, 0, 1 },
    { 32, { { 21, 30 }, { 0, 1 } }, 1, 160 },
    { 33, { { 20, 99 }, { 0, 1 } }, 0, 1 },
    { 34, { { 0, 1 }, { 0, 1 } }, 7, 40 },
    { 35, { { 22, 99 }, { 0, 1 } }, 0, 1 },
    { 36, { { 23, 99 }, { 0, 1 } }, 0, 1 },
    { 37, { { 25, 50 }, { 0, 1 } }, 0, 1 },
    { 38, { { 26, 99 }, { 0, 1 } }, 0, 1 },
    { 39, { { 27, 99 }, { 0, 1 } }, 0, 1 },
    { 40, { { 28, 99 }, { 0, 1 } }, 0, 1 },
    { 41, { { 30, 40 }, { 0, 1 } }, 13, 240 },
    { 42, { { 19, 40 }, { 0, 1 } }, 0, 1 },
    { 43, { { 24, 99 }, { 0, 1 } }, 0, 1 },
    { 44, { { 37, 99 }, { 0, 1 } }, 0, 1 },
    { 45, { { 29, 99 }, { 0, 1 } }, 0, 1 },
    { 46, { { 41, 99 }, { 0, 1 } }, 0, 1 },
    { 47, { { 37, 40 }, { 31, 40 } }, 0, 1 },
    { 48, { { 42, 99 }, { 0, 1 } }, 0, 1 },
    { 49, { { 36, 40 }, { 41, 40 } }, 0, 1 },
    { 50, { { 44, 40 }, { 49, 40 } }, 0, 1 },
    { 51, { { 45, 40 }, { 38, 40 } }, 0, 1 },
    { 52, { { 47, 99 }, { 43, 99 } }, 0, 1 },
};

StfgtrepLearn stfgtrep_learn_3[44] = {
    { 9, { { 36, 20 }, { 0, 1 } }, 0, 1 },
    { 10, { { 26, 30 }, { 0, 1 } }, 0, 1 },
    { 11, { { 41, 30 }, { 0, 1 } }, 2, 400 },
    { 12, { { 0, 1 }, { 0, 1 } }, 7, 5 },
    { 13, { { 23, 30 }, { 0, 1 } }, 10, 100 },
    { 14, { { 12, 20 }, { 0, 1 } }, 0, 1 },
    { 15, { { 23, 20 }, { 0, 1 } }, 3, 300 },
    { 16, { { 31, 20 }, { 0, 1 } }, 4, 300 },
    { 17, { { 28, 30 }, { 0, 1 } }, 14, 250 },
    { 18, { { 36, 30 }, { 0, 1 } }, 5, 260 },
    { 19, { { 28, 20 }, { 0, 1 } }, 0, 1 },
    { 20, { { 9, 50 }, { 0, 1 } }, 0, 1 },
    { 21, { { 10, 50 }, { 0, 1 } }, 0, 1 },
    { 22, { { 11, 50 }, { 0, 1 } }, 0, 1 },
    { 23, { { 0, 1 }, { 0, 1 } }, 7, 20 },
    { 24, { { 12, 50 }, { 0, 1 } }, 14, 130 },
    { 25, { { 13, 5 }, { 18, 5 } }, 0, 1 },
    { 26, { { 14, 40 }, { 0, 1 } }, 7, 15 },
    { 27, { { 15, 40 }, { 0, 1 } }, 0, 1 },
    { 28, { { 16, 50 }, { 0, 1 } }, 0, 1 },
    { 29, { { 17, 40 }, { 0, 1 } }, 0, 1 },
    { 30, { { 36, 10 }, { 0, 1 } }, 0, 1 },
    { 31, { { 18, 50 }, { 0, 1 } }, 0, 1 },
    { 32, { { 41, 20 }, { 0, 1 } }, 0, 1 },
    { 33, { { 20, 99 }, { 0, 1 } }, 0, 1 },
    { 34, { { 21, 99 }, { 0, 1 } }, 0, 1 },
    { 35, { { 22, 99 }, { 0, 1 } }, 0, 1 },
    { 36, { { 0, 1 }, { 0, 1 } }, 7, 40 },
    { 37, { { 25, 50 }, { 0, 1 } }, 0, 1 },
    { 38, { { 26, 99 }, { 0, 1 } }, 0, 1 },
    { 39, { { 27, 99 }, { 0, 1 } }, 0, 1 },
    { 40, { { 28, 99 }, { 0, 1 } }, 0, 1 },
    { 41, { { 30, 40 }, { 0, 1 } }, 0, 1 },
    { 42, { { 19, 40 }, { 0, 1 } }, 0, 1 },
    { 43, { { 24, 99 }, { 0, 1 } }, 0, 1 },
    { 44, { { 37, 99 }, { 0, 1 } }, 0, 1 },
    { 45, { { 29, 99 }, { 0, 1 } }, 0, 1 },
    { 46, { { 41, 99 }, { 0, 1 } }, 0, 1 },
    { 47, { { 37, 40 }, { 31, 40 } }, 0, 1 },
    { 48, { { 42, 99 }, { 0, 1 } }, 0, 1 },
    { 49, { { 36, 40 }, { 41, 40 } }, 0, 1 },
    { 50, { { 44, 40 }, { 49, 40 } }, 0, 1 },
    { 51, { { 45, 40 }, { 38, 40 } }, 0, 1 },
    { 52, { { 47, 99 }, { 43, 99 } }, 0, 1 },
};

StfgtrepLearn stfgtrep_learn_4[44] = {
    { 9, { { 37, 30 }, { 0, 1 } }, 0, 1 },
    { 10, { { 37, 35 }, { 0, 1 } }, 13, 160 },
    { 11, { { 37, 45 }, { 0, 1 } }, 2, 200 },
    { 12, { { 25, 35 }, { 0, 1 } }, 2, 200 },
    { 13, { { 0, 1 }, { 0, 1 } }, 7, 5 },
    { 14, { { 25, 30 }, { 0, 1 } }, 8, 90 },
    { 15, { { 25, 40 }, { 0, 1 } }, 10, 200 },
    { 16, { { 25, 45 }, { 0, 1 } }, 4, 230 },
    { 17, { { 25, 25 }, { 0, 1 } }, 14, 160 },
    { 18, { { 13, 30 }, { 0, 1 } }, 0, 1 },
    { 19, { { 28, 20 }, { 0, 1 } }, 0, 1 },
    { 20, { { 9, 50 }, { 0, 1 } }, 0, 1 },
    { 21, { { 10, 50 }, { 0, 1 } }, 0, 1 },
    { 22, { { 11, 50 }, { 0, 1 } }, 0, 1 },
    { 23, { { 12, 40 }, { 0, 1 } }, 0, 1 },
    { 24, { { 12, 50 }, { 0, 1 } }, 14, 190 },
    { 25, { { 18, 5 }, { 0, 1 } }, 7, 20 },
    { 26, { { 14, 50 }, { 0, 1 } }, 0, 1 },
    { 27, { { 15, 50 }, { 0, 1 } }, 0, 1 },
    { 28, { { 16, 50 }, { 0, 1 } }, 0, 1 },
    { 29, { { 17, 40 }, { 0, 1 } }, 0, 1 },
    { 30, { { 25, 20 }, { 0, 1 } }, 13, 100 },
    { 31, { { 18, 40 }, { 0, 1 } }, 0, 1 },
    { 32, { { 37, 20 }, { 0, 1 } }, 4, 300 },
    { 33, { { 20, 99 }, { 0, 1 } }, 0, 1 },
    { 34, { { 21, 99 }, { 0, 1 } }, 0, 1 },
    { 35, { { 22, 99 }, { 0, 1 } }, 0, 1 },
    { 36, { { 23, 99 }, { 0, 1 } }, 0, 1 },
    { 37, { { 25, 50 }, { 0, 1 } }, 7, 40 },
    { 38, { { 26, 99 }, { 0, 1 } }, 0, 1 },
    { 39, { { 27, 99 }, { 0, 1 } }, 0, 1 },
    { 40, { { 28, 99 }, { 0, 1 } }, 0, 1 },
    { 41, { { 30, 40 }, { 0, 1 } }, 13, 140 },
    { 42, { { 19, 40 }, { 0, 1 } }, 0, 1 },
    { 43, { { 24, 99 }, { 0, 1 } }, 0, 1 },
    { 44, { { 37, 99 }, { 0, 1 } }, 0, 1 },
    { 45, { { 29, 99 }, { 0, 1 } }, 0, 1 },
    { 46, { { 41, 99 }, { 0, 1 } }, 0, 1 },
    { 47, { { 37, 40 }, { 31, 40 } }, 0, 1 },
    { 48, { { 42, 99 }, { 0, 1 } }, 0, 1 },
    { 49, { { 36, 40 }, { 41, 40 } }, 0, 1 },
    { 50, { { 44, 40 }, { 49, 40 } }, 0, 1 },
    { 51, { { 45, 40 }, { 38, 40 } }, 0, 1 },
    { 52, { { 47, 99 }, { 43, 99 } }, 0, 1 },
};

StfgtrepLearn stfgtrep_learn_5[44] = {
    { 9, { { 23, 20 }, { 0, 1 } }, 1, 400 },
    { 10, { { 9, 20 }, { 0, 1 } }, 0, 1 },
    { 11, { { 38, 10 }, { 0, 1 } }, 0, 1 },
    { 12, { { 38, 30 }, { 0, 1 } }, 8, 280 },
    { 13, { { 38, 20 }, { 0, 1 } }, 11, 300 },
    { 14, { { 0, 1 }, { 0, 1 } }, 7, 5 },
    { 15, { { 19, 10 }, { 0, 1 } }, 3, 200 },
    { 16, { { 26, 20 }, { 0, 1 } }, 3, 180 },
    { 17, { { 22, 20 }, { 0, 1 } }, 14, 160 },
    { 18, { { 14, 30 }, { 0, 1 } }, 12, 80 },
    { 19, { { 26, 40 }, { 0, 1 } }, 9, 140 },
    { 20, { { 9, 50 }, { 0, 1 } }, 0, 1 },
    { 21, { { 10, 50 }, { 0, 1 } }, 0, 1 },
    { 22, { { 11, 50 }, { 0, 1 } }, 0, 1 },
    { 23, { { 12, 40 }, { 0, 1 } }, 0, 1 },
    { 24, { { 12, 50 }, { 0, 1 } }, 14, 140 },
    { 25, { { 13, 5 }, { 18, 5 } }, 0, 1 },
    { 26, { { 0, 1 }, { 0, 1 } }, 7, 20 },
    { 27, { { 15, 40 }, { 0, 1 } }, 0, 1 },
    { 28, { { 16, 50 }, { 0, 1 } }, 0, 1 },
    { 29, { { 17, 50 }, { 0, 1 } }, 0, 1 },
    { 30, { { 14, 20 }, { 0, 1 } }, 2, 100 },
    { 31, { { 18, 40 }, { 0, 1 } }, 0, 1 },
    { 32, { { 26, 30 }, { 0, 1 } }, 14, 120 },
    { 33, { { 20, 99 }, { 0, 1 } }, 0, 1 },
    { 34, { { 21, 99 }, { 0, 1 } }, 0, 1 },
    { 35, { { 22, 99 }, { 0, 1 } }, 0, 1 },
    { 36, { { 23, 99 }, { 0, 1 } }, 0, 1 },
    { 37, { { 25, 50 }, { 0, 1 } }, 0, 1 },
    { 38, { { 0, 1 }, { 0, 1 } }, 7, 40 },
    { 39, { { 27, 99 }, { 0, 1 } }, 0, 1 },
    { 40, { { 28, 99 }, { 0, 1 } }, 0, 1 },
    { 41, { { 30, 40 }, { 0, 1 } }, 13, 190 },
    { 42, { { 19, 40 }, { 0, 1 } }, 0, 1 },
    { 43, { { 24, 99 }, { 0, 1 } }, 0, 1 },
    { 44, { { 37, 99 }, { 0, 1 } }, 0, 1 },
    { 45, { { 29, 99 }, { 0, 1 } }, 0, 1 },
    { 46, { { 41, 99 }, { 0, 1 } }, 0, 1 },
    { 47, { { 37, 40 }, { 31, 40 } }, 0, 1 },
    { 48, { { 42, 99 }, { 0, 1 } }, 0, 1 },
    { 49, { { 36, 40 }, { 41, 40 } }, 0, 1 },
    { 50, { { 44, 40 }, { 49, 40 } }, 0, 1 },
    { 51, { { 45, 40 }, { 38, 40 } }, 0, 1 },
    { 52, { { 47, 99 }, { 43, 99 } }, 0, 1 },
};

StfgtrepLearn stfgtrep_learn_6[44] = {
    { 9, { { 11, 20 }, { 0, 1 } }, 0, 1 },
    { 10, { { 20, 20 }, { 0, 1 } }, 0, 1 },
    { 11, { { 29, 20 }, { 0, 1 } }, 1, 300 },
    { 12, { { 11, 30 }, { 0, 1 } }, 2, 280 },
    { 13, { { 27, 20 }, { 0, 1 } }, 11, 120 },
    { 14, { { 18, 20 }, { 0, 1 } }, 2, 80 },
    { 15, { { 0, 1 }, { 0, 1 } }, 7, 5 },
    { 16, { { 39, 30 }, { 0, 1 } }, 11, 150 },
    { 17, { { 39, 10 }, { 0, 1 } }, 0, 1 },
    { 18, { { 15, 20 }, { 0, 1 } }, 1, 80 },
    { 19, { { 15, 30 }, { 0, 1 } }, 3, 160 },
    { 20, { { 9, 50 }, { 0, 1 } }, 0, 1 },
    { 21, { { 10, 50 }, { 0, 1 } }, 0, 1 },
    { 22, { { 11, 50 }, { 0, 1 } }, 0, 1 },
    { 23, { { 12, 40 }, { 0, 1 } }, 0, 1 },
    { 24, { { 12, 50 }, { 0, 1 } }, 14, 250 },
    { 25, { { 13, 5 }, { 18, 5 } }, 0, 1 },
    { 26, { { 14, 40 }, { 0, 1 } }, 7, 25 },
    { 27, { { 0, 1 }, { 0, 1 } }, 7, 20 },
    { 28, { { 16, 50 }, { 0, 1 } }, 0, 1 },
    { 29, { { 17, 50 }, { 0, 1 } }, 0, 1 },
    { 30, { { 27, 30 }, { 0, 1 } }, 13, 100 },
    { 31, { { 18, 40 }, { 0, 1 } }, 0, 1 },
    { 32, { { 39, 20 }, { 0, 1 } }, 4, 400 },
    { 33, { { 20, 99 }, { 0, 1 } }, 0, 1 },
    { 34, { { 21, 99 }, { 0, 1 } }, 0, 1 },
    { 35, { { 22, 99 }, { 0, 1 } }, 0, 1 },
    { 36, { { 23, 99 }, { 0, 1 } }, 0, 1 },
    { 37, { { 25, 50 }, { 0, 1 } }, 0, 1 },
    { 38, { { 26, 99 }, { 0, 1 } }, 0, 1 },
    { 39, { { 0, 1 }, { 0, 1 } }, 7, 40 },
    { 40, { { 28, 99 }, { 0, 1 } }, 0, 1 },
    { 41, { { 30, 40 }, { 0, 1 } }, 13, 120 },
    { 42, { { 19, 40 }, { 0, 1 } }, 7, 15 },
    { 43, { { 24, 99 }, { 0, 1 } }, 0, 1 },
    { 44, { { 37, 99 }, { 0, 1 } }, 0, 1 },
    { 45, { { 29, 99 }, { 0, 1 } }, 0, 1 },
    { 46, { { 41, 99 }, { 0, 1 } }, 0, 1 },
    { 47, { { 37, 40 }, { 31, 40 } }, 0, 1 },
    { 48, { { 42, 99 }, { 0, 1 } }, 0, 1 },
    { 49, { { 36, 40 }, { 41, 40 } }, 0, 1 },
    { 50, { { 44, 40 }, { 49, 40 } }, 0, 1 },
    { 51, { { 45, 40 }, { 38, 40 } }, 0, 1 },
    { 52, { { 47, 99 }, { 43, 99 } }, 0, 1 },
};

StfgtrepLearn stfgtrep_learn_7[44] = {
    { 9, { { 14, 20 }, { 0, 1 } }, 0, 1 },
    { 10, { { 40, 10 }, { 0, 1 } }, 0, 1 },
    { 11, { { 14, 40 }, { 0, 1 } }, 1, 240 },
    { 12, { { 28, 30 }, { 0, 1 } }, 8, 140 },
    { 13, { { 14, 30 }, { 0, 1 } }, 11, 320 },
    { 14, { { 30, 20 }, { 0, 1 } }, 0, 1 },
    { 15, { { 21, 40 }, { 0, 1 } }, 3, 300 },
    { 16, { { 0, 1 }, { 0, 1 } }, 7, 5 },
    { 17, { { 21, 30 }, { 0, 1 } }, 14, 300 },
    { 18, { { 28, 20 }, { 0, 1 } }, 12, 100 },
    { 19, { { 16, 30 }, { 0, 1 } }, 4, 180 },
    { 20, { { 9, 50 }, { 0, 1 } }, 0, 1 },
    { 21, { { 10, 50 }, { 0, 1 } }, 0, 1 },
    { 22, { { 11, 50 }, { 0, 1 } }, 0, 1 },
    { 23, { { 12, 40 }, { 0, 1 } }, 0, 1 },
    { 24, { { 12, 50 }, { 0, 1 } }, 14, 220 },
    { 25, { { 13, 5 }, { 18, 5 } }, 0, 1 },
    { 26, { { 14, 50 }, { 0, 1 } }, 0, 1 },
    { 27, { { 15, 40 }, { 0, 1 } }, 0, 1 },
    { 28, { { 0, 1 }, { 0, 1 } }, 7, 20 },
    { 29, { { 17, 50 }, { 0, 1 } }, 0, 1 },
    { 30, { { 21, 20 }, { 0, 1 } }, 13, 140 },
    { 31, { { 18, 40 }, { 0, 1 } }, 0, 1 },
    { 32, { { 16, 20 }, { 0, 1 } }, 1, 100 },
    { 33, { { 20, 99 }, { 0, 1 } }, 0, 1 },
    { 34, { { 21, 99 }, { 0, 1 } }, 0, 1 },
    { 35, { { 22, 99 }, { 0, 1 } }, 0, 1 },
    { 36, { { 23, 99 }, { 0, 1 } }, 0, 1 },
    { 37, { { 25, 50 }, { 0, 1 } }, 0, 1 },
    { 38, { { 26, 99 }, { 0, 1 } }, 0, 1 },
    { 39, { { 27, 99 }, { 0, 1 } }, 0, 1 },
    { 40, { { 0, 1 }, { 0, 1 } }, 7, 40 },
    { 41, { { 30, 50 }, { 0, 1 } }, 0, 1 },
    { 42, { { 19, 40 }, { 0, 1 } }, 7, 15 },
    { 43, { { 24, 99 }, { 0, 1 } }, 0, 1 },
    { 44, { { 37, 99 }, { 0, 1 } }, 0, 1 },
    { 45, { { 29, 99 }, { 0, 1 } }, 0, 1 },
    { 46, { { 41, 99 }, { 0, 1 } }, 0, 1 },
    { 47, { { 37, 40 }, { 31, 40 } }, 0, 1 },
    { 48, { { 42, 99 }, { 0, 1 } }, 0, 1 },
    { 49, { { 36, 40 }, { 41, 40 } }, 0, 1 },
    { 50, { { 44, 40 }, { 49, 40 } }, 0, 1 },
    { 51, { { 45, 40 }, { 38, 40 } }, 0, 1 },
    { 52, { { 47, 99 }, { 43, 99 } }, 0, 1 },
};

StfgtrepLearn *stfgtrep_learn_lists[8] = {
    stfgtrep_learn_0, stfgtrep_learn_1, stfgtrep_learn_2, stfgtrep_learn_3,
    stfgtrep_learn_4, stfgtrep_learn_5, stfgtrep_learn_6, stfgtrep_learn_7,
};

s32 stfgtrep_member_anims[8][7] = {
    { 7, 8, 9, 10, 9, 8, -1 },
    { 14, 15, 16, 15, -1, -1, -1 },
    { 11, 12, 13, 12, -1, -1, -1 },
    { 3, 4, 5, 6, 5, 4, -1 },
    { 25, 26, 27, 28, 27, 26, -1 },
    { 0, 1, 2, 1, -1, -1, -1 },
    { 17, 18, 19, 20, 19, 18, -1 },
    { 21, 22, 23, 24, 23, 22, -1 },
};

StfgtrepFuncs stfgtrep_funcs = {
    stfgtrep_load_files,
    stfgtrep_is_loading,
    stfgtrep_window_anim_start,
    stfgtrep_window_anim_update,
    stfgtrep_tween_start,
    stfgtrep_tween_update,
    stfgtrep_add_exp,
    stfgtrep_learn_technique,
    stfgtrep_add_technique_exp,
    (s32 (*)(s32, s32))stfgtrep_learn_skill,
    stfgtrep_mark_skill,
    stfgtrep_get_technique_exp,
};

s32 stfgtrep_level_exp_bonus[4] = { 0, 50, 800, 3000 };
s32 stfgtrep_hp_mp_gain_cut[4] = { 0, 5, 10, 15 };
s32 stfgtrep_hp_mp_gain_random[9] = { -4, -3, -2, -1, 0, 1, 2, 3, 4 };
s32 stfgtrep_stat_gains[6][9] = {
    { 2, 3, 4, 6, 8, 10, 12, 13, 14 },
    { 1, 2, 3, 4, 6, 8, 9, 10, 11 },
    { 0, 1, 3, 4, 4, 4, 5, 7, 8 },
    { 0, 1, 1, 2, 3, 4, 5, 5, 6 },
    { 0, 0, 1, 2, 2, 2, 3, 4, 4 },
    { 0, 0, 1, 1, 1, 1, 1, 2, 2 },
};
s32 stfgtrep_resist_gains[8] = { 0, 0, 0, 1, 1, 1, 2, 2 };
