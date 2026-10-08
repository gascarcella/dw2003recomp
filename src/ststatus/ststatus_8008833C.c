#include "common.h"
#include "object.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "ststatus.h"

/* The forms page (ststatus_create_forms_page), opened from the status page, then the status page itself
 * (ststatus_create_status_page). */

/* The forms page's data block (0x90 bytes): its text windows and cursors. */
typedef struct StstatusFormsPageData {
    /* 0x00 */ MessageWindow *forms[3];  /* forms */
    /* 0x0C */ MessageCursor *form_cursor; /* form cursor */
    /* 0x10 */ MessageWindow *own_form; /* the member's own form */
    /* 0x14 */ MessageWindow *menu_title; /* menu title */
    /* 0x18 */ MessageWindow *menu_choices[2]; /* menu choices */
    /* 0x20 */ MessageCursor *menu_cursor; /* menu cursor */
    /* 0x24 */ MessageCursor *technique_cursor; /* technique cursor */
    /* 0x28 */ MessageWindow *name;   /* name */
    /* 0x2C */ MessageWindow *level;  /* level */
    /* 0x30 */ MessageWindow *lv_label; /* "Lv" */
    /* 0x34 */ MessageWindow *stats[13];  /* stats */
    /* 0x68 */ MessageWindow *techniques[6]; /* techniques */
    /* 0x80 */ MessageWindow *title;  /* title */
    /* 0x84 */ MessageWindow *message; /* message, or technique */
    /* 0x88 */ MessageWindow *mp_label; /* "MP" */
    /* 0x8C */ MessageWindow *mp_cost; /* MP cost */
} StstatusFormsPageData; /* size 0x90 */

/* The forms page (ststatus_create_forms_page, size 0xFC): a member's forms, the stats and techniques of one. */
typedef struct StstatusFormsPage {
    /* 0x00 */ Object base;
    /* 0x50 */ StstatusStatusPage *status_page; /* the status page */
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 ot_depth; /* ordering table depth */
    /* 0x5C */ s32 form_cursor; /* form cursor */
    /* 0x60 */ s32 menu_cursor; /* menu cursor */
    /* 0x64 */ s32 technique_cursor; /* technique cursor (-1: none) */
    /* 0x68 */ s32 techniques; /* techniques */
    /* 0x6C */ s32 show_form; /* 0: the member's own stats, else the form's */
    /* 0x70 */ s32 member; /* party member */
    /* 0x74 */ s16 forms[3];  /* forms (Digimon IDs) */
    /* 0x7A */ u8 pad_7A[0x2];
    /* 0x7C */ s32 form_count; /* forms the member has */
    /* 0x80 */ s32 stats_shown; /* the stats are shown */
    /* 0x84 */ Tween stats_y; /* the stats panel's y offset */
    /* 0xA0 */ s32 arrow_shown; /* the "next" arrow is shown */
    /* 0xA4 */ s32 arrow_frame; /* its frame, 0..4 */
    /* 0xA8 */ s32 arrow_time; /* time of its last frame */
    /* 0xAC */ WindowAnim anims[5];  /* frame, forms, menu, stats, message */
} StstatusFormsPage; /* size 0xFC */

s32 ststatus_forms_stats[13] = { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };

/* Creates the page's text windows. */
void ststatus_forms_create_windows(StstatusFormsPage *obj, StstatusFormsPageData *data) {
    s32 i;
    StstatusLayout *l;
    StstatusLayout *p;

    for (i = 0; i < 3; i++) {
        data->forms[i] = message_create_window(obj->layer_id, 1, 0xBD, i * 0xE + 0x31);
    }
    data->form_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0xB0, obj->form_cursor * 0xE + 0x31);
    data->form_cursor->show(data->form_cursor, 0);
    data->own_form = message_create_window(obj->layer_id, 1, 0x5E, 0x4D);
    data->menu_title = message_create_window(obj->layer_id, 1, 0xB2, 0x2A);
    for (i = 0; i < 2; i++) {
        data->menu_choices[i] = message_create_window(obj->layer_id, 1, 0xBD, i * 0xE + 0x3A);
    }
    data->menu_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0xB4, obj->menu_cursor * 0xE + 0x3A);
    data->menu_cursor->show(data->menu_cursor, 0);
    data->name = message_create_window(obj->layer_id, 1, 0x4F, 0x68);
    data->lv_label = message_create_window(obj->layer_id, 1, 0xEB, 0x68);
    data->level = message_create_window(obj->layer_id, 1, 0x12F, 0x68);
    for (i = 0; i < 6; i++) {
        data->stats[i] = message_create_window(obj->layer_id, 1, 0x71, i * 0xE + 0x7C);
    }
    for (i = 0; i < 7; i++) {
        data->stats[i + 6] = message_create_window(obj->layer_id, 1, 0x9B, i * 0xE + 0x7C);
    }
    for (i = 0; i < 6; i++) {
        data->techniques[i] = message_create_window(obj->layer_id, 1, 0xC4, i * 0xE + 0x88);
    }
    data->technique_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0xA9, 0x88);
    data->technique_cursor->show(data->technique_cursor, 0);
    data->title = message_create_window(obj->layer_id, 1, 0x98, 0x13);
    l = ststatus_module.party_layout;
    p = &l[12];
    data->message = message_create_window(obj->layer_id, 1, p->x, p->y);
    p = &l[13];
    data->mp_label = message_create_window(obj->layer_id, 1, p->x, p->y);
    p = &l[14];
    data->mp_cost = message_create_window(obj->layer_id, 1, p->x, p->y);
}

/* Shows (fills in) or hides the forms, the one the status shows highlighted. */
void ststatus_forms_show_forms(StstatusFormsPage *obj, StstatusFormsPageData *data, s32 show) {
    s32 digimon;
    s32 i;
    GamestateDigimon *member;
    RecordsDigimon *form;

    if (show) {
        digimon = gamestate_data.funcs.get_party_member(obj->member);
        member = &gamestate_data.digimon[digimon];
        for (i = 0; i < 3; i++) {
            if (obj->forms[i] > 0) {
                form = records_get_digimon_func(obj->forms[i]);
                data->forms[i]->set_text(data->forms[i], cdload_module.files.get_file(records_language + 0x4E),
                                         form->name_id);
                if (member->shown_form == obj->forms[i]) {
                    data->forms[i]->set_palette(data->forms[i], 1);
                } else {
                    data->forms[i]->set_palette(data->forms[i], 0);
                }
            } else {
                data->forms[i]->set_visible(data->forms[i], 0);
            }
        }
        form = &records_digimon[digimon];
        data->own_form->set_text(data->own_form, cdload_module.files.get_file(records_language + 0x4E), form->name_id);
    } else {
        for (i = 0; i < 3; i++) {
            data->forms[i]->set_visible(data->forms[i], 0);
        }
        data->own_form->set_visible(data->own_form, 0);
    }
}

/* Shows or hides the menu for the form under the cursor ("show it" or "stop showing it", "techniques"). */
void ststatus_forms_show_menu(StstatusFormsPage *obj, StstatusFormsPageData *data, s32 show) {
    GamestateDigimon *member;
    RecordsDigimon *form;
    s32 i;

    if (show) {
        member = &gamestate_data.digimon[gamestate_data.funcs.get_party_member(obj->member)];
        form = records_get_digimon_func(obj->forms[obj->form_cursor]);
        data->menu_title->set_text(data->menu_title, cdload_module.files.get_file(records_language + 0x4E), form->name_id);
        if (member->shown_form == obj->forms[obj->form_cursor]) {
            data->menu_choices[0]->set_text(data->menu_choices[0], cdload_module.files.get_file(records_language + 0xB0), 0x3A);
        } else {
            data->menu_choices[0]->set_text(data->menu_choices[0], cdload_module.files.get_file(records_language + 0xB0), 0x39);
        }
        data->menu_choices[1]->set_text(data->menu_choices[1], cdload_module.files.get_file(records_language + 0xB0), 0x3B);
    } else {
        data->menu_title->set_visible(data->menu_title, 0);
        for (i = 0; i < 2; i++) {
            data->menu_choices[i]->set_visible(data->menu_choices[i], 0);
        }
    }
}

/* Shows (fills in) or hides the stats and techniques: the member's own, or the selected form's. */
void ststatus_forms_show_stats(StstatusFormsPage *obj, StstatusFormsPageData *data, s32 show) {
    GamestateStats stats;
    GamestateForm rec;
    s32 digimon;
    s32 i;
    s32 value;
    RecordsDigimon *form;
    s16 id;
    s32 tech;

    if (show) {
        digimon = gamestate_data.funcs.get_party_member(obj->member);
        if (obj->show_form != 0) {
            id = obj->forms[obj->form_cursor];
            gamestate_data.funcs.get_form(digimon, id, &rec);
            gamestate_data.funcs.get_stats(digimon, &stats);
            form = records_get_digimon_func(id);
            data->name->set_text(data->name, cdload_module.files.get_file(records_language + 0x4E), form->name_id);
            data->lv_label->set_text(data->lv_label, cdload_module.files.get_file(records_language + 0xB0), 0x38);
            data->level->set_line_number(data->level, 0, rec.level);
            data->level->measure(data->level, 1);
            for (i = 0; i < 6; i++) {
                value = stats.values[ststatus_forms_stats[i]] + form->stats[i];
                if (value >= 1000) {
                    value = 999;
                }
                data->stats[i]->set_line_number(data->stats[i], 0, value);
                if (i == 0) {
                    if (stats.penalties[0] != 0) {
                        data->stats[0]->set_palette(data->stats[0], 6);
                    }
                } else if (i == 1) {
                    if (stats.penalties[1] != 0) {
                        data->stats[1]->set_palette(data->stats[1], 6);
                    }
                } else if (i == 4) {
                    if (stats.penalties[2] != 0) {
                        data->stats[4]->set_palette(data->stats[4], 6);
                    }
                }
            }
            for (i = 0; i < 7; i++) {
                value = stats.values[ststatus_forms_stats[i + 6]] + form->resists[i];
                if (value >= 1000) {
                    value = 999;
                }
                data->stats[i + 6]->set_line_number(data->stats[i + 6], 0, value);
            }
            for (i = 0; i < 13; i++) {
                data->stats[i]->measure(data->stats[i], 1);
            }
            for (i = 0; i < 6; i++) {
                tech = rec.techniques[i];
                if (tech != 0) {
                    data->techniques[i]->set_text(data->techniques[i], cdload_module.files.get_file(records_language + 0xA2),
                                             tech & 0x1FFF);
                    if (tech & 0x8000) {
                        data->techniques[i]->set_palette(data->techniques[i], 3);
                    } else if (tech & 0x4000) {
                        data->techniques[i]->set_palette(data->techniques[i], 4);
                    } else {
                        data->techniques[i]->set_palette(data->techniques[i], 0);
                    }
                } else {
                    data->techniques[i]->set_visible(data->techniques[i], 0);
                }
            }
        } else {
            form = &records_digimon[digimon];
            gamestate_data.funcs.get_stats(digimon, &stats);
            data->name->set_text(data->name, cdload_module.files.get_file(records_language + 0x4E), form->name_id);
            data->lv_label->set_text(data->lv_label, cdload_module.files.get_file(records_language + 0xB0), 0x38);
            data->level->set_text(data->level, cdload_module.files.get_file(records_language + 0xB0), 0xD);
            data->level->measure(data->level, 1);
            for (i = 0; i < 6; i++) {
                value = stats.values[ststatus_forms_stats[i]];
                if (value >= 1000) {
                    value = 999;
                }
                data->stats[i]->set_line_number(data->stats[i], 0, value);
                if (i == 0) {
                    if (stats.penalties[0] != 0) {
                        data->stats[0]->set_palette(data->stats[0], 6);
                    }
                } else if (i == 1) {
                    if (stats.penalties[1] != 0) {
                        data->stats[1]->set_palette(data->stats[1], 6);
                    }
                } else if (i == 4) {
                    if (stats.penalties[2] != 0) {
                        data->stats[4]->set_palette(data->stats[4], 6);
                    }
                }
            }
            for (i = 0; i < 7; i++) {
                value = stats.values[ststatus_forms_stats[i + 6]];
                if (value >= 1000) {
                    value = 999;
                }
                data->stats[i + 6]->set_line_number(data->stats[i + 6], 0, value);
            }
            for (i = 0; i < 13; i++) {
                data->stats[i]->measure(data->stats[i], 1);
            }
            for (i = 0; i < 6; i++) {
                if (form->techniques[i + 1] != 0) {
                    data->techniques[i]->set_text(data->techniques[i], cdload_module.files.get_file(records_language + 0xA2),
                                             form->techniques[i + 1]);
                    data->techniques[i]->set_palette(data->techniques[i], 3);
                } else {
                    data->techniques[i]->set_visible(data->techniques[i], 0);
                }
            }
        }
        obj->stats_shown = 1;
    } else {
        data->name->set_visible(data->name, 0);
        data->lv_label->set_visible(data->lv_label, 0);
        data->level->set_visible(data->level, 0);
        for (i = 0; i < 13; i++) {
            data->stats[i]->set_visible(data->stats[i], 0);
        }
        for (i = 0; i < 6; i++) {
            data->techniques[i]->set_visible(data->techniques[i], 0);
        }
        obj->stats_shown = 0;
    }
}

/* Moves the stats panel's windows to its y offset. */
void ststatus_forms_move_stats(StstatusFormsPage *obj, StstatusFormsPageData *data) {
    s32 i;

    data->name->set_pos(data->name, 0x4F, obj->stats_y.value + 0x68);
    data->lv_label->set_pos(data->lv_label, 0xEB, obj->stats_y.value + 0x68);
    data->level->set_pos(data->level, 0x12F, obj->stats_y.value + 0x68);
    for (i = 0; i < 6; i++) {
        data->stats[i]->set_pos(data->stats[i], 0x71, i * 0xE + (s16)(obj->stats_y.value + 0x7C));
    }
    for (i = 0; i < 7; i++) {
        data->stats[i + 6]->set_pos(data->stats[i + 6], 0x9B, i * 0xE + (s16)(obj->stats_y.value + 0x7C));
    }
    for (i = 0; i < 6; i++) {
        data->techniques[i]->set_pos(data->techniques[i], 0xC4, i * 0xE + (s16)(obj->stats_y.value + 0x88));
    }
}

/* Shows (fills in) or hides the technique under the cursor and its MP cost. */
void ststatus_forms_show_technique(StstatusFormsPage *obj, StstatusFormsPageData *data, s32 show) {
    GamestateForm rec;
    s32 digimon;
    s32 tech;
    RecordsDigimon *form;

    if (show) {
        digimon = gamestate_data.funcs.get_party_member(obj->member);
        if (obj->show_form != 0) {
            gamestate_data.funcs.get_form(digimon, obj->forms[obj->form_cursor], &rec);
            tech = rec.techniques[obj->technique_cursor] & 0x1FFF;
            if (tech > 0) {
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x9B), tech);
                data->mp_label->set_text(data->mp_label, cdload_module.files.get_file(records_language + 0xB0), 3);
                data->mp_cost->set_line_number(data->mp_cost, 0, records_techniques[tech - 1].mp_cost);
                data->mp_cost->measure(data->mp_cost, 1);
                return;
            }
        } else {
            form = &records_digimon[digimon];
            tech = form->techniques[obj->technique_cursor + 1];
            if (tech > 0) {
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x9B), tech);
                data->mp_label->set_text(data->mp_label, cdload_module.files.get_file(records_language + 0xB0), 3);
                data->mp_cost->set_line_number(data->mp_cost, 0, records_techniques[tech - 1].mp_cost);
                data->mp_cost->measure(data->mp_cost, 1);
                return;
            }
        }
    }
    data->message->set_visible(data->message, 0);
    data->mp_label->set_visible(data->mp_label, 0);
    data->mp_cost->set_visible(data->mp_cost, 0);
}

/* Draws the frames, the technique element icons and the "next" arrow. */
void ststatus_forms_draw(StstatusFormsPage *obj) {
    Sprite spr;
    GamestateForm rec;
    s32 digimon;
    s32 i;
    s32 tech;
    RecordsDigimon *form;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    spr.set_vram_pos(0x280, 0x100);
    if (obj->anims[1].level != 0) {
        if (obj->anims[1].level != 0x1000) {
            spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x44);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x24, 0xA8, 0x28);
    }
    if (obj->anims[2].level != 0) {
        if (obj->anims[2].level != 0x1000) {
            spr.set_scale(obj->anims[2].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x41);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x26, 0xA8, 0x28);
    }
    if (obj->anims[3].level != 0) {
        if (obj->anims[3].level != 0x1000) {
            spr.set_scale(obj->anims[3].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0xA2);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.set_vram_pos(0x140, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x14, 0x4F, obj->stats_y.value + 0x7C);
        if (obj->stats_shown != 0) {
            digimon = gamestate_data.funcs.get_party_member(obj->member);
            if (obj->show_form != 0) {
                gamestate_data.funcs.get_form(digimon, obj->forms[obj->form_cursor], &rec);
                for (i = 0; i < 6; i++) {
                    tech = rec.techniques[i] & 0x1FFF;
                    if (tech != 0) {
                        spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_techniques[tech - 1].element + 0x37, 0xB6,
                                   obj->stats_y.value + 0x88 + i * 0xE);
                    }
                }
            } else {
                form = &records_digimon[digimon];
                for (i = 0; i < 6; i++) {
                    tech = form->techniques[i + 1];
                    if (tech > 0) {
                        spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_techniques[tech - 1].element + 0x37, 0xB6,
                                   obj->stats_y.value + 0x88 + i * 0xE);
                    }
                }
            }
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x27, 0x48, obj->stats_y.value + 0x61);
    }
    if (obj->anims[4].level != 0) {
        if (obj->arrow_shown != 0) {
            if (gfx_module.funcs.get_time() - obj->arrow_time >= 4) {
                obj->arrow_time = gfx_module.funcs.get_time();
                if (++obj->arrow_frame >= 5) {
                    obj->arrow_frame = 0;
                }
            }
            spr.set_vram_pos(0x140, 0);
            spr.set_palette(obj->arrow_frame);
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xA, 0x123, 0xD6);
            spr.set_palette(0);
        }
        if (obj->anims[4].level != 0x1000) {
            spr.set_scale(obj->anims[4].level, 0x1000, 0x1000);
            spr.set_pivot(0, 0xD3);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x20, 0, 0xC2);
    }
    if (obj->anims[0].level != 0) {
        if (obj->anims[0].level != 0x1000) {
            spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0xD);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.set_vram_pos(0x140, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x18, 0x22, 0xD);
    }
}

/* The page's states: open, pick a form (or the member itself), show it on the status page or list its
 * techniques, close. */
void ststatus_forms_run(StstatusFormsPage *obj, StstatusFormsPageData *data) {
    GamestateForm rec;
    s32 old;
    s32 i;
    GamestateDigimon *member;
    s32 id;
    s32 tech;
    s32 digimon;
    RecordsDigimon *form;

    switch (obj->base.step) {
    case 0:
    default:
        ststatus_module.window_anim_start(&obj->anims[0], 1);
        obj->base.step++;
        break;
    case 1:
        if (ststatus_module.window_anim_update(&obj->anims[0])) {
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x37);
            ststatus_module.window_anim_start(&obj->anims[1], 1);
            obj->base.step++;
        }
        break;
    case 2:
        if (ststatus_module.window_anim_update(&obj->anims[1])) {
            ststatus_forms_show_forms(obj, data, 1);
            ststatus_module.window_anim_start(&obj->anims[3], 1);
            obj->base.step++;
        }
        break;
    case 3:
        if (ststatus_module.window_anim_update(&obj->anims[3])) {
            ststatus_forms_show_stats(obj, data, 1);
            data->form_cursor->set_pos(data->form_cursor, 0x50, 0x4D);
            data->form_cursor->show(data->form_cursor, 1);
            obj->base.step++;
        }
        break;
    case 4:
        if (obj->show_form != 0) {
            old = obj->form_cursor;
            if (PAD_PRESSED(7)) {
                obj->show_form = 0;
                ststatus_forms_show_stats(obj, data, 1);
                data->form_cursor->set_pos(data->form_cursor, 0x50, 0x4D);
                sound_module.play(0x8004513E);
                break;
            }
            if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                if (--obj->form_cursor < 0) {
                    obj->form_cursor = 0;
                }
            } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                obj->form_cursor++;
                if (obj->form_count - 1 < obj->form_cursor) {
                    obj->form_cursor = obj->form_count - 1;
                }
            }
            if (old != obj->form_cursor) {
                ststatus_forms_show_stats(obj, data, 1);
                data->form_cursor->set_pos(data->form_cursor, 0xB0, obj->form_cursor * 0xE + 0x31);
                sound_module.play(0x8004513E);
            } else if (PAD_PRESSED(13)) {
                sound_module.play(0x8004503C);
                obj->base.step = 10;
            } else if (PAD_PRESSED(14)) {
                sound_module.play(0x800450BD);
                obj->base.step = 50;
            }
        } else {
            if (obj->form_count > 0 && PAD_PRESSED(5)) {
                obj->show_form = 1;
                ststatus_forms_show_stats(obj, data, 1);
                data->form_cursor->set_pos(data->form_cursor, 0xB0, obj->form_cursor * 0xE + 0x31);
                sound_module.play(0x8004513E);
            } else if (PAD_PRESSED(13)) {
                sound_module.play(0x8004503C);
                obj->base.step = 60;
            } else if (PAD_PRESSED(14)) {
                sound_module.play(0x800450BD);
                obj->base.step = 50;
            }
        }
        break;
    case 10:
        ststatus_module.window_anim_start(&obj->anims[1], 0);
        ststatus_forms_show_forms(obj, data, 0);
        data->form_cursor->show(data->form_cursor, 0);
        data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x2C);
        obj->base.step++;
        break;
    case 11:
        if (ststatus_module.window_anim_update(&obj->anims[1])) {
            ststatus_module.window_anim_start(&obj->anims[2], 1);
            obj->base.step++;
        }
        break;
    case 12:
        if (ststatus_module.window_anim_update(&obj->anims[2])) {
            ststatus_forms_show_menu(obj, data, 1);
            data->menu_cursor->show(data->menu_cursor, 1);
            obj->base.step++;
        }
        break;
    case 13:
        old = obj->menu_cursor;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            obj->menu_cursor = 0;
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            obj->menu_cursor = 1;
        }
        if (old != obj->menu_cursor) {
            sound_module.play(0x8004513E);
            data->menu_cursor->set_pos(data->menu_cursor, 0xB4, obj->menu_cursor * 0xE + 0x3A);
        }
        if (PAD_PRESSED(13)) {
            sound_module.play(0x8004503C);
            if (obj->menu_cursor == 0) {
                member = &gamestate_data.digimon[gamestate_data.funcs.get_party_member(obj->member)];
                id = obj->forms[obj->form_cursor];
                if (member->shown_form == id) {
                    member->shown_form = 0;
                } else {
                    member->shown_form = id;
                }
                obj->base.step = 25;
            } else {
                obj->base.step = 40;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step = 20;
        }
        break;
    case 20:
        ststatus_module.window_anim_start(&obj->anims[2], 0);
        ststatus_forms_show_menu(obj, data, 0);
        data->menu_cursor->show(data->menu_cursor, 0);
        data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x37);
        obj->base.step++;
        break;
    case 21:
        if (ststatus_module.window_anim_update(&obj->anims[2])) {
            ststatus_module.window_anim_start(&obj->anims[1], 1);
            obj->base.step++;
        }
        break;
    case 25:
        ststatus_module.window_anim_start(&obj->anims[0], 0);
        data->title->set_visible(data->title, 0);
        ststatus_module.window_anim_start(&obj->anims[2], 0);
        ststatus_forms_show_menu(obj, data, 0);
        data->menu_cursor->show(data->menu_cursor, 0);
        obj->base.step++;
        break;
    case 26:
        ststatus_module.window_anim_update(&obj->anims[0]);
        if (ststatus_module.window_anim_update(&obj->anims[2])) {
            ststatus_module.lerp_start(&obj->stats_y, 0, -0x22, 4);
            obj->base.step++;
        }
        break;
    case 27:
        if (ststatus_module.lerp_update(&obj->stats_y)) {
            ststatus_module.window_anim_start(&obj->anims[4], 1);
            obj->base.step++;
        }
        ststatus_forms_move_stats(obj, data);
        break;
    case 28:
        if (ststatus_module.window_anim_update(&obj->anims[4])) {
            /* (records + i)->: the record address gamestate_data + 0x75C, not a folded 0x764 */
            if ((gamestate_data.digimon + gamestate_data.funcs.get_party_member(obj->member))->shown_form != 0) {
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x3C);
            } else {
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x3D);
            }
            obj->arrow_shown = 1;
            obj->base.step++;
        }
        break;
    case 29:
        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            obj->arrow_shown = 0;
            obj->base.step++;
        }
        break;
    case 30:
        ststatus_module.window_anim_start(&obj->anims[4], 0);
        data->message->set_visible(data->message, 0);
        ststatus_module.window_anim_start(&obj->anims[3], 0);
        ststatus_forms_show_stats(obj, data, 0);
        obj->base.step++;
        break;
    case 31:
        ststatus_module.window_anim_update(&obj->anims[4]);
        if (ststatus_module.window_anim_update(&obj->anims[3])) {
            obj->show_form = 0;
            ststatus_module.lerp_start(&obj->stats_y, 0, -0x22, 4);
            ststatus_forms_move_stats(obj, data);
            obj->base.step = 0;
        }
        break;
    case 40:
        ststatus_module.window_anim_start(&obj->anims[2], 0);
        ststatus_forms_show_menu(obj, data, 0);
        data->menu_cursor->show(data->menu_cursor, 0);
        data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x2A);
        obj->base.step++;
        break;
    case 41:
        if (ststatus_module.window_anim_update(&obj->anims[2])) {
            ststatus_module.lerp_start(&obj->stats_y, 0, -0x22, 4);
            obj->base.step++;
        }
        break;
    case 42:
        if (ststatus_module.lerp_update(&obj->stats_y)) {
            ststatus_module.window_anim_start(&obj->anims[4], 1);
            obj->base.step++;
        }
        ststatus_forms_move_stats(obj, data);
        break;
    case 43:
        if (ststatus_module.window_anim_update(&obj->anims[4])) {
            gamestate_data.funcs.get_form(gamestate_data.funcs.get_party_member(obj->member), obj->forms[obj->form_cursor], &rec);
            obj->technique_cursor = -1;
            obj->techniques = 0;
            for (i = 0; i < 6; i++) {
                if ((u16)rec.techniques[i] & 0x1FFF) {
                    obj->techniques++;
                    if (obj->technique_cursor == -1) {
                        obj->technique_cursor = i;
                    }
                }
            }
            if (obj->techniques != 0) {
                data->technique_cursor->set_pos(data->technique_cursor, 0xA9, obj->technique_cursor * 0xE + 0x88 + obj->stats_y.value);
                data->technique_cursor->show(data->technique_cursor, 1);
                ststatus_forms_show_technique(obj, data, 1);
                obj->base.step++;
            } else {
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x3E);
                obj->base.step++;
            }
        }
        break;
    case 44:
        if (obj->techniques != 0) {
            old = obj->technique_cursor;
            gamestate_data.funcs.get_form(gamestate_data.funcs.get_party_member(obj->member), obj->forms[obj->form_cursor], &rec);
            if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                do {
                    if (--obj->technique_cursor < 0) {
                        obj->technique_cursor = old;
                        break;
                    }
                } while ((rec.techniques[obj->technique_cursor] & 0x1FFF) <= 0);
            } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                do {
                    if (++obj->technique_cursor >= 6) {
                        obj->technique_cursor = old;
                        break;
                    }
                } while ((rec.techniques[obj->technique_cursor] & 0x1FFF) <= 0);
            }
            if (old != obj->technique_cursor) {
                sound_module.play(0x8004513E);
                data->technique_cursor->set_pos(data->technique_cursor, 0xA9, obj->technique_cursor * 0xE + 0x88 + obj->stats_y.value);
                ststatus_forms_show_technique(obj, data, 1);
            }
        }
        if (PAD_PRESSED(13)) {
            sound_module.play(0x8004503C);
            obj->base.step++;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step++;
        }
        break;
    case 45:
        data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x2C);
        data->technique_cursor->show(data->technique_cursor, 0);
        ststatus_forms_show_technique(obj, data, 0);
        ststatus_module.lerp_start(&obj->stats_y, -0x22, 0, 4);
        ststatus_module.window_anim_start(&obj->anims[4], 0);
        obj->base.step++;
        break;
    case 46:
        ststatus_module.lerp_update(&obj->stats_y);
        if (ststatus_module.window_anim_update(&obj->anims[4])) {
            ststatus_module.window_anim_start(&obj->anims[2], 1);
            obj->base.step++;
        }
        ststatus_forms_move_stats(obj, data);
        break;
    case 47:
        if (ststatus_module.window_anim_update(&obj->anims[2])) {
            data->menu_cursor->show(data->menu_cursor, 1);
            ststatus_forms_show_menu(obj, data, 1);
            obj->base.step = 13;
        }
        break;
    case 50:
        ststatus_module.window_anim_start(&obj->anims[0], 0);
        ststatus_module.window_anim_start(&obj->anims[1], 0);
        ststatus_module.window_anim_start(&obj->anims[3], 0);
        ststatus_forms_show_forms(obj, data, 0);
        ststatus_forms_show_stats(obj, data, 0);
        data->form_cursor->show(data->form_cursor, 0);
        data->title->set_visible(data->title, 0);
        obj->base.step++;
        break;
    case 51:
        ststatus_module.window_anim_update(&obj->anims[0]);
        ststatus_module.window_anim_update(&obj->anims[1]);
        if (ststatus_module.window_anim_update(&obj->anims[3])) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 60:
        ststatus_module.window_anim_start(&obj->anims[1], 0);
        ststatus_forms_show_forms(obj, data, 0);
        data->form_cursor->show(data->form_cursor, 0);
        data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x2A);
        obj->base.step++;
        break;
    case 61:
        if (ststatus_module.window_anim_update(&obj->anims[1])) {
            ststatus_module.lerp_start(&obj->stats_y, 0, -0x22, 4);
            obj->base.step++;
        }
        break;
    case 62:
        if (ststatus_module.lerp_update(&obj->stats_y)) {
            ststatus_module.window_anim_start(&obj->anims[4], 1);
            obj->base.step++;
        }
        ststatus_forms_move_stats(obj, data);
        break;
    case 63:
        if (ststatus_module.window_anim_update(&obj->anims[4])) {
            digimon = gamestate_data.funcs.get_party_member(obj->member);
            form = &records_digimon[digimon];
            tech = form->techniques[6];
            obj->technique_cursor = 5;
            data->technique_cursor->set_pos(data->technique_cursor, 0xA9, obj->stats_y.value + 0xCE);
            data->technique_cursor->show(data->technique_cursor, 1);
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x9B), tech);
            data->mp_label->set_text(data->mp_label, cdload_module.files.get_file(records_language + 0xB0), 3);
            data->mp_cost->set_line_number(data->mp_cost, 0, records_techniques[tech - 1].mp_cost);
            data->mp_cost->measure(data->mp_cost, 1);
            obj->base.step++;
        }
        break;
    case 64:
        if (PAD_PRESSED(13)) {
            sound_module.play(0x8004503C);
            obj->base.step++;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step++;
        }
        break;
    case 65:
        data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x37);
        data->technique_cursor->show(data->technique_cursor, 0);
        ststatus_forms_show_technique(obj, data, 0);
        ststatus_module.lerp_start(&obj->stats_y, -0x22, 0, 4);
        ststatus_module.window_anim_start(&obj->anims[4], 0);
        obj->base.step++;
        break;
    case 66:
        ststatus_module.lerp_update(&obj->stats_y);
        if (ststatus_module.window_anim_update(&obj->anims[4])) {
            ststatus_module.window_anim_start(&obj->anims[1], 1);
            obj->base.step++;
        }
        ststatus_forms_move_stats(obj, data);
        break;
    case 22:
    case 67:
        if (ststatus_module.window_anim_update(&obj->anims[1])) {
            ststatus_forms_show_forms(obj, data, 1);
            data->form_cursor->show(data->form_cursor, 1);
            obj->base.step = 4;
        }
        break;
    }
}

void ststatus_forms_update(StstatusFormsPage *obj, StstatusFormsPageData *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        ststatus_forms_create_windows(obj, data);
        gamestate_data.funcs.get_chosen_forms(gamestate_data.funcs.get_party_member(obj->member), obj->forms);
        for (i = 0; i < 3; i++) {
            if (obj->forms[i] >= 4) {
                obj->form_count++;
            }
        }
        obj->anims[0].duration = 10;
        obj->anims[1].duration = 10;
        obj->anims[2].duration = 10;
        obj->anims[3].duration = 10;
        obj->anims[4].duration = 10;
        break;
    case OBJECT_STATE_RUN:
        ststatus_forms_run(obj, data);
        ststatus_forms_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Creates the forms page for the status page's member. */
StstatusFormsPage *ststatus_create_forms_page(StstatusStatusPage *parent) {
    StstatusFormsPage *obj = object_new(ststatus_forms_update, sizeof(StstatusFormsPage), sizeof(StstatusFormsPageData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->status_page = parent;
    obj->member = parent->member;
    return obj;
}

/* The status page's data block (0x110 bytes): its text windows, its cursor and the open sub-page. */
typedef struct StstatusStatusPageData {
    /* 0x000 */ MessageWindow *title;  /* title */
    /* 0x004 */ StstatusPanelWindows panels[3]; /* per party member: its panel */
    /* 0x088 */ MessageWindow *help;
    /* 0x08C */ MessageWindow *back_hint;
    /* 0x090 */ MessageWindow *menu_choices[2]; /* menu choices */
    /* 0x098 */ MessageCursor *menu_cursor; /* menu cursor */
    /* 0x09C */ MessageWindow *exp;    /* experience */
    /* 0x0A0 */ MessageWindow *exp_label; /* "EXP" */
    /* 0x0A4 */ MessageWindow *forms_title; /* forms title */
    /* 0x0A8 */ MessageWindow *forms[3];  /* forms */
    /* 0x0B4 */ MessageWindow *equipment_title; /* equipment title */
    /* 0x0B8 */ MessageWindow *equipment[6]; /* equipment */
    /* 0x0D0 */ MessageWindow *stats[13];  /* stats */
    /* 0x104 */ MessageWindow *level;   /* level */
    /* 0x108 */ MessageWindow *lv_label; /* "Lv" */
    /* 0x10C */ Object *sub_page; /* the open sub-page */
} StstatusStatusPageData; /* size 0x110 */

/* A member's six equipment slots (GamestateRecord.equipment), copied whole. */
typedef struct StstatusEquip {
    s16 slot[6];
} StstatusEquip; /* size 0xC */

s32 ststatus_status_panel_stats[5] = { 0, 2, 3, 4, 5 };
s32 ststatus_status_stats[13] = { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
s32 ststatus_status_slot_texts[6] = { 65, 66, 67, 77, 68, 68 }; /* ?STATUS entries */

/* In ststatus_80084F54.c (it returns its own page type there). */
Object *ststatus_create_equip_page(StstatusStatusPage *parent);
void ststatus_status_preview_item(StstatusStatusPage *obj, s32 slot, s32 item);
void ststatus_status_draw(StstatusStatusPage *obj);

/* Creates the page's text windows. */
void ststatus_status_create_windows(StstatusStatusPage *obj, StstatusStatusPageData *data) {
    s32 i;
    s32 j;
    StstatusLayout *l;

    l = &ststatus_module.party_layout[11];
    data->title = message_create_window(obj->layer_id, 1, l->x, l->y);
    for (i = 0; i < 3; i++) {
        l = ststatus_module.party_layout;
        data->panels[i].name = message_create_window(obj->layer_id, 1, l->x, l->y + i * 0x2E);
        l = &ststatus_module.party_layout[1];
        for (j = 0; j < 5; j++, l++) {
            data->panels[i].labels[j] = message_create_window(obj->layer_id, 3, l->x, l->y + i * 0x2E);
        }
        l = &ststatus_module.party_layout[6];
        for (j = 0; j < 5; j++, l++) {
            data->panels[i].values[j] = message_create_window(obj->layer_id, 3, l->x, l->y + i * 0x2E);
        }
    }
    l = &ststatus_module.party_layout[12];
    data->help = message_create_window(obj->layer_id, 1, l->x, l->y);
    data->help->set_page_lines(data->help, 2);
    data->back_hint = message_create_window(obj->layer_id, 1, l->x, l->y + 0xE);
    for (j = 0; j < 2; j++) {
        data->menu_choices[j] = message_create_window(obj->layer_id, 1, 0xAC, j * 0xE + 0x13);
    }
    data->menu_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0x98, obj->menu_cursor * 0xE + 0x13);
    data->menu_cursor->show(data->menu_cursor, 0);
    data->exp = message_create_window(obj->layer_id, 3, 0x42, 0x45);
    data->exp_label = message_create_window(obj->layer_id, 3, 0x46, 0x45);
    data->forms_title = message_create_window(obj->layer_id, 1, 0xA7, 0x37);
    for (j = 0; j < 3; j++) {
        data->forms[j] = message_create_window(obj->layer_id, 1, 0xB2, j * 0xE + 0x47);
    }
    data->equipment_title = message_create_window(obj->layer_id, 1, 0xA7, 0x79);
    for (j = 0; j < 6; j++) {
        data->equipment[j] = message_create_window(obj->layer_id, 1, 0xC0, j * 0xE + 0x89);
    }
    for (j = 0; j < 6; j++) {
        data->stats[j] = message_create_window(obj->layer_id, 1, 0x32, j * 0xE + 0x5B);
    }
    for (j = 0; j < 7; j++) {
        data->stats[j + 6] = message_create_window(obj->layer_id, 1, 0x5B, j * 0xE + 0x5B);
    }
    data->lv_label = message_create_window(obj->layer_id, 1, 0x10, 0xCB);
    data->lv_label->set_ot_depth(data->lv_label, obj->ot_depth - 1);
    data->level = message_create_window(obj->layer_id, 1, 0x2D, 0xCB);
    data->level->set_ot_depth(data->level, obj->ot_depth - 1);
}

/* Shows (fills in) or hides a member's panel. */
void ststatus_status_show_member(StstatusStatusPage *obj, StstatusStatusPageData *data, s32 member, s32 show) {
    GamestateStats stats;
    s32 digimon;
    s32 i;
    MessageWindow *win;
    StstatusLayout *l;

    if (show) {
        digimon = gamestate_data.funcs.get_party_member(member);
        gamestate_data.funcs.get_stats(digimon, &stats);
        data->panels[member].name->set_text(data->panels[member].name,
                                             (u8 *)gamestate_data.funcs.get_record(digimon), -1);
        l = &ststatus_module.party_layout[1];
        for (i = 0; i < 5; i++, l++) {
            data->panels[member].labels[i]->set_text(data->panels[member].labels[i],
                                                    cdload_module.files.get_file(records_language + 0xB0), l->text);
        }
        for (i = 0; i < 5; i++) {
            win = data->panels[member].values[i];
            win->set_line_number(win, 0, stats.values[ststatus_status_panel_stats[i]]);
            data->panels[member].values[i]->measure(data->panels[member].values[i], 1);
        }
    } else {
        data->panels[member].name->set_visible(data->panels[member].name, 0);
        for (i = 0; i < 5; i++) {
            data->panels[member].labels[i]->set_visible(data->panels[member].labels[i], 0);
        }
        for (i = 0; i < 5; i++) {
            data->panels[member].values[i]->set_visible(data->panels[member].values[i], 0);
        }
    }
}

/* Shows (fills in) or hides the member's own panel (the first one's windows). */
void ststatus_status_show_own_panel(StstatusStatusPage *obj, StstatusStatusPageData *data, s32 member, s32 show) {
    GamestateStats stats;
    s32 digimon;
    s32 i;
    u8 *name;
    StstatusLayout *l;

    if (show) {
        digimon = gamestate_data.funcs.get_party_member(member);
        name = (u8 *)gamestate_data.funcs.get_record(digimon);
        gamestate_data.funcs.get_stats(digimon, &stats);
        data->panels[0].name->set_text(data->panels[0].name, name, -1);
        l = &ststatus_module.party_layout[1];
        for (i = 0; i < 5; i++, l++) {
            data->panels[0].labels[i]->set_text(data->panels[0].labels[i],
                                               cdload_module.files.get_file(records_language + 0xB0), l->text);
        }
        for (i = 0; i < 5; i++) {
            data->panels[0].values[i]->set_line_number(data->panels[0].values[i], 0, stats.values[ststatus_status_panel_stats[i]]);
            data->panels[0].values[i]->measure(data->panels[0].values[i], 1);
        }
    } else {
        data->panels[0].name->set_visible(data->panels[0].name, 0);
        for (i = 0; i < 5; i++) {
            data->panels[0].labels[i]->set_visible(data->panels[0].labels[i], 0);
        }
        for (i = 0; i < 5; i++) {
            data->panels[0].values[i]->set_visible(data->panels[0].values[i], 0);
        }
    }
}

/* Shows or hides the menu's choices. */
void ststatus_status_show_menu(StstatusStatusPage *obj, StstatusStatusPageData *data, s32 show) {
    s32 i;

    if (show) {
        for (i = 0; i < 2; i++) {
            data->menu_choices[i]->set_text(data->menu_choices[i], cdload_module.files.get_file(records_language + 0xB0), i + 0x2D);
        }
    } else {
        for (i = 0; i < 2; i++) {
            data->menu_choices[i]->set_visible(data->menu_choices[i], 0);
        }
    }
}

/* Shows (fills in) or hides the member's forms, the one the status shows highlighted. */
void ststatus_status_show_forms(StstatusStatusPage *obj, StstatusStatusPageData *data, s32 show) {
    s16 forms[3];
    GamestateDigimon *member;
    s32 digimon;
    s32 i;
    RecordsDigimon *form;

    if (show) {
        digimon = gamestate_data.funcs.get_party_member(obj->member);
        gamestate_data.funcs.get_chosen_forms(digimon, forms);
        member = &gamestate_data.digimon[digimon];
        data->forms_title->set_text(data->forms_title, cdload_module.files.get_file(records_language + 0xB0), 0x34);
        for (i = 0; i < 3; i++) {
            if (forms[i] >= 4) {
                form = records_get_digimon_func(forms[i]);
                data->forms[i]->set_text(data->forms[i], cdload_module.files.get_file(records_language + 0x4E), form->name_id);
                if (member->shown_form == forms[i]) {
                    data->forms[i]->set_palette(data->forms[i], 1);
                } else {
                    data->forms[i]->set_palette(data->forms[i], 0);
                }
            } else {
                data->forms[i]->set_visible(data->forms[i], 0);
            }
        }
    } else {
        data->forms_title->set_visible(data->forms_title, 0);
        for (i = 0; i < 3; i++) {
            data->forms[i]->set_visible(data->forms[i], 0);
        }
    }
}

/* Shows (fills in) or hides the member's equipment. */
void ststatus_status_show_equipment(StstatusStatusPage *obj, StstatusStatusPageData *data, s32 show) {
    GamestateRecord *rec;
    s32 i;
    s16 item;

    if (show) {
        rec = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(obj->member));
        data->equipment_title->set_text(data->equipment_title, cdload_module.files.get_file(records_language + 0xB0), 0x20);
        for (i = 0; i < 6; i++) {
            item = rec->equipment[i];
            if (item != 0) {
                data->equipment[i]->set_text(data->equipment[i], cdload_module.files.get_file(records_language + 0x6A), item);
            } else {
                data->equipment[i]->set_text(data->equipment[i], cdload_module.files.get_file(records_language + 0xB0),
                                         ststatus_status_slot_texts[i]);
            }
        }
    } else {
        data->equipment_title->set_visible(data->equipment_title, 0);
        for (i = 0; i < 6; i++) {
            data->equipment[i]->set_visible(data->equipment[i], 0);
        }
    }
}

/* Shows (fills in) or hides the member's stats and level. */
void ststatus_status_show_stats(StstatusStatusPage *obj, StstatusStatusPageData *data, s32 show) {
    GamestateStats stats;
    s32 i;
    s32 value;

    if (show) {
        gamestate_data.funcs.get_stats(gamestate_data.funcs.get_party_member(obj->member), &stats);
        for (i = 0; i < 6; i++) {
            value = stats.values[ststatus_status_stats[i]];
            if (value >= 1000) {
                value = 999;
            }
            data->stats[i]->set_line_number(data->stats[i], 0, value);
        }
        for (i = 0; i < 7; i++) {
            value = stats.values[ststatus_status_stats[i + 6]];
            if (value >= 1000) {
                value = 999;
            }
            data->stats[i + 6]->set_line_number(data->stats[i + 6], 0, value);
        }
        for (i = 0; i < 13; i++) {
            data->stats[i]->measure(data->stats[i], 1);
            data->stats[i]->set_palette(data->stats[i], 0);
        }
        data->lv_label->set_text(data->lv_label, cdload_module.files.get_file(records_language + 0xB0), 0x4E);
        data->level->set_line_number(data->level, 0, stats.values[1]);
        data->level->measure(data->level, 1);
        if (stats.penalties[0] != 0) {
            data->stats[0]->set_palette(data->stats[0], 6);
        }
        if (stats.penalties[1] != 0) {
            data->stats[1]->set_palette(data->stats[1], 6);
        }
        if (stats.penalties[2] != 0) {
            data->stats[4]->set_palette(data->stats[4], 6);
        }
    } else {
        for (i = 0; i < 13; i++) {
            data->stats[i]->set_visible(data->stats[i], 0);
        }
        data->lv_label->set_visible(data->lv_label, 0);
        data->level->set_visible(data->level, 0);
    }
}

/* The status page's preview_item: shows the stats the member would have with `item` in equipment slot `slot`
 * (slot -1: the current stats), higher ones in one colour and lower ones in another. */
void ststatus_status_preview_item(StstatusStatusPage *obj, s32 slot, s32 item) {
    GamestateStats stats;
    GamestateStats after;
    StstatusEquip saved;
    StstatusStatusPageData *data;
    s32 digimon;
    GamestateRecord *rec;
    s16 *p;
    s32 i;
    RecordsEquip *e;
    s32 group;
    s16 old;
    s32 now;
    s32 then;
    s32 j;

    data = (StstatusStatusPageData *)obj->base.children;
    if (slot == -1) {
        ststatus_status_show_stats(obj, data, 1);
        return;
    }
    digimon = gamestate_data.funcs.get_party_member(obj->member);
    rec = gamestate_data.funcs.get_record(digimon);
    gamestate_data.funcs.get_stats(digimon, &stats);
    saved = *(StstatusEquip *)rec->equipment;
    old = *(rec->equipment + slot);
    if (old != 0) {
        e = records_funcs.get_item(old)->data;
        if (e->slot == 7) {
            rec->equipment[2] = 0;
            rec->equipment[3] = 0;
        } else {
            *(rec->equipment + slot) = 0;
        }
    }
    if (item > 0) {
        e = records_funcs.get_item(item)->data;
        if (e->slot == 7) {
            s16 *hand = &rec->equipment[2];

            if (*hand == 0) {
                hand = NULL;
                if (rec->equipment[3] != 0) {
                    hand = &rec->equipment[3];
                }
            }
            if (hand != NULL) {
                *hand = 0;
            }
        } else if (e->slot == 8) {
            group = e->group;
            for (j = 0; j < 2; j++) {
                p = &rec->equipment[4 + j];
                if (*p != 0) {
                    e = records_funcs.get_item(*p)->data;
                    if (e->group == group) {
                        *p = 0;
                    }
                }
            }
        }
        e = records_funcs.get_item(item)->data;
        if (e->slot == 7) {
            rec->equipment[2] = item;
            rec->equipment[3] = item;
        } else {
            *(rec->equipment + slot) = item;
        }
    }
    gamestate_data.funcs.get_stats(digimon, &after);
    *(StstatusEquip *)rec->equipment = saved;
    for (i = 0; i < 6; i++) {
        then = *(after.values + ststatus_status_stats[i]);
        now = *(stats.values + ststatus_status_stats[i]);
        if (then >= 1000) {
            data->stats[i]->set_line_number(data->stats[i], 0, 999);
        } else {
            data->stats[i]->set_line_number(data->stats[i], 0, then);
        }
        if (now < then) {
            data->stats[i]->set_palette(data->stats[i], 1);
        } else if (then < now) {
            data->stats[i]->set_palette(data->stats[i], 5);
        } else {
            data->stats[i]->set_palette(data->stats[i], 0);
            if (i == 0) {
                if (stats.penalties[0] != 0) {
                    data->stats[0]->set_palette(data->stats[0], 6);
                }
            } else if (i == 1) {
                if (stats.penalties[1] != 0) {
                    data->stats[1]->set_palette(data->stats[1], 6);
                }
            } else if (i == 4) {
                if (stats.penalties[2] != 0) {
                    data->stats[4]->set_palette(data->stats[4], 6);
                }
            }
        }
    }
    for (i = 0; i < 7; i++) {
        then = *(after.values + ststatus_status_stats[i + 6]);
        now = *(stats.values + ststatus_status_stats[i + 6]);
        if (then >= 1000) {
            data->stats[i + 6]->set_line_number(data->stats[i + 6], 0, 999);
        } else {
            data->stats[i + 6]->set_line_number(data->stats[i + 6], 0, then);
        }
        if (now < then) {
            data->stats[i + 6]->set_palette(data->stats[i + 6], 1);
        } else if (then < now) {
            data->stats[i + 6]->set_palette(data->stats[i + 6], 5);
        } else {
            data->stats[i + 6]->set_palette(data->stats[i + 6], 0);
        }
    }
    for (i = 0; i < 13; i++) {
        data->stats[i]->measure(data->stats[i], 1);
    }
}

/* Draws the members' sprites, the panels' frames, the equipment icons and the member cursor. */
void ststatus_status_draw(StstatusStatusPage *obj) {
    Sprite spr;
    GamestateRecord *rec;
    s32 i;
    s32 digimon;
    s16 item;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (gfx_module.funcs.get_time() - obj->frame_time >= 13) {
        obj->frame_time = gfx_module.funcs.get_time();
        for (i = 0; i < obj->member_count; i++) {
            digimon = gamestate_data.funcs.get_party_member(i);
            obj->frames[i]++;
            if (ststatus_module.anims[digimon].frame[obj->frames[i]] == -1 || obj->frames[i] >= 7) {
                obj->frames[i] = 0;
            }
        }
    }
    spr.set_vram_pos(0x280, 0x100);
    for (i = 0; i < obj->member_count; i++) {
        if (obj->panel_anims[i].level != 0) {
            if (obj->panel_anims[i].level != 0x1000) {
                spr.set_scale(obj->panel_anims[i].level, obj->panel_anims[i].level, 0x1000);
                spr.set_pivot(0x7C, i * 0x2E + 0x27);
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
            }
            digimon = gamestate_data.funcs.get_party_member(i);
            spr.draw(cdload_module.get_subfile_by_id(0x04040000), ststatus_module.anims[digimon].frame[obj->frames[i]],
                       0x6B, i * 0x2E + 0x13);
        }
    }
    spr.set_vram_pos(0x140, 0);
    for (i = 0; i < obj->member_count; i++) {
        if (obj->panel_anims[i].level != 0) {
            if (obj->panel_anims[i].level != 0x1000) {
                spr.set_scale(obj->panel_anims[i].level, 0x1000, 0x1000);
                spr.set_pivot(0, i * 0x2E + 0x25);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x15, 0, i * 0x2E + 0x11);
                spr.set_scale(obj->panel_anims[i].level, obj->panel_anims[i].level, 0x1000);
                spr.set_pivot(0x7C, i * 0x2E + 0x27);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x16, 0x67, i * 0x2E + 0x13);
                spr.set_scale(obj->panel_anims[i].level, 0x1000, 0x1000);
                spr.set_pivot(0, i * 0x2E + 0x25);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x17, 0, i * 0x2E + 0x11);
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x15, 0, i * 0x2E + 0x11);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x16, 0x67, i * 0x2E + 0x13);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x17, 0, i * 0x2E + 0x11);
            }
        }
    }
    if (obj->member_anim.level != 0) {
        spr.set_vram_pos(0x280, 0x100);
        if (obj->member_anim.level != 0x1000) {
            spr.set_scale(obj->member_anim.level, obj->member_anim.level, 0x1000);
            spr.set_pivot(0x7C, 0x27);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        digimon = gamestate_data.funcs.get_party_member(obj->member);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), ststatus_module.anims[digimon].frame[obj->frames[obj->member]],
                   0x6B, 0x13);
        spr.set_vram_pos(0x140, 0);
        if (obj->member_anim.level != 0x1000) {
            spr.set_scale(obj->member_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0, 0x25);
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x15, 0, 0x11);
            spr.set_scale(obj->member_anim.level, obj->member_anim.level, 0x1000);
            spr.set_pivot(0x7C, 0x27);
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x16, 0x67, 0x13);
            spr.set_scale(obj->member_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0, 0x25);
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x17, 0, 0x11);
        } else {
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x15, 0, 0x11);
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x16, 0x67, 0x13);
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x17, 0, 0x11);
        }
    }
    if (obj->bar_anims[0].level != 0) {
        if (obj->bar_anims[0].level != 0x1000) {
            spr.set_scale(obj->bar_anims[0].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x19);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x18, 0x22, 0xD);
    }
    if (obj->exp_anim.level != 0) {
        if (obj->exp_anim.level != 0x1000) {
            spr.set_scale(obj->exp_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0, 0x47);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x1A, 0, 0x3D);
    }
    spr.set_vram_pos(0x280, 0x100);
    if (obj->bar_anims[1].level != 0) {
        if (obj->bar_anims[1].level != 0x1000) {
            spr.set_scale(obj->bar_anims[1].level, 0x1000, 0x1000);
            spr.set_pivot(0, 0xD3);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x20, 0, 0xC2);
    }
    if (obj->stats_anim.level != 0) {
        if (obj->stats_anim.level != 0x1000) {
            spr.set_scale(obj->stats_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0, 0x88);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.set_vram_pos(0x140, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xF, 0, 0x57);
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x21, 0, 0x54);
        if (obj->stats_anim.level != 0x1000) {
            spr.set_pivot(0, 0xD0);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x3C, 0, 0xC4);
    }
    if (obj->menu_anim.level != 0) {
        if (obj->menu_anim.level != 0x1000) {
            spr.set_scale(obj->menu_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x55);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x22, 0xA0, 0x35);
    }
    if (obj->list_anim.level != 0) {
        if (obj->list_anim.level != 0x1000) {
            spr.set_scale(obj->list_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0xAC);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
            rec = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(obj->member));
            for (i = 0; i < 6; i++) {
                item = rec->equipment[i];
                if (item > 0) {
                    spr.set_vram_pos(0x140, 0);
                    spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_funcs.get_item_icon(item), 0xB2, i * 0xE + 0x89);
                }
            }
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x23, 0xA0, 0x77);
    }
    if (obj->name_anim.level != 0) {
        if (obj->name_anim.level != 0x1000) {
            spr.set_scale(obj->name_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x20);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x1F, 0x14, 0xD);
    }
    if (obj->cursor_shown != 0) {
        if (gfx_module.funcs.get_time() - obj->cursor_time >= 9) {
            obj->cursor_time = gfx_module.funcs.get_time();
            if (++obj->cursor_frame >= 8) {
                obj->cursor_frame = 0;
            }
        }
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
        spr.set_vram_pos(0x280, 0x100);
        spr.set_palette(obj->cursor_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x1E, 0, obj->member * 0x2E + 0x11);
    }
}

/* The member view's states (base.substep): open the member's panels, run the forms/equipment menu, close. */
void ststatus_status_run_member(StstatusStatusPage *obj, StstatusStatusPageData *data) {
    s16 forms[3]; /* unused: the frame has 8 bytes of locals */
    s32 old;

    switch (obj->base.substep) {
    case 0:
    default:
        if (obj->base.timer == 0) {
            ststatus_module.window_anim_start(&obj->member_anim, 1);
        }
        ststatus_module.window_anim_start(&obj->name_anim, 1);
        obj->base.substep++;
        break;
    case 1:
        if (obj->base.timer == 0) {
            ststatus_module.window_anim_update(&obj->member_anim);
        }
        if (ststatus_module.window_anim_update(&obj->name_anim)) {
            if (obj->exp_anim.level == 0) {
                ststatus_module.window_anim_start(&obj->exp_anim, 1);
            }
            ststatus_module.window_anim_start(&obj->menu_anim, 1);
            ststatus_status_show_own_panel(obj, data, obj->member, 1);
            ststatus_status_show_menu(obj, data, 1);
            obj->base.substep++;
        }
        break;
    case 2:
        ststatus_module.window_anim_update(&obj->exp_anim);
        if (ststatus_module.window_anim_update(&obj->menu_anim)) {
            if (obj->stats_anim.level == 0) {
                ststatus_module.window_anim_start(&obj->stats_anim, 1);
            }
            ststatus_module.window_anim_start(&obj->list_anim, 1);
            ststatus_status_show_forms(obj, data, 1);
            data->exp->set_line_number(data->exp, 0,
                                  gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(obj->member))->exp);
            data->exp->measure(data->exp, 1);
            data->exp_label->set_text(data->exp_label, cdload_module.files.get_file(records_language + 0xB0), 0x35);
            data->menu_cursor->set_pos(data->menu_cursor, 0x98, obj->menu_cursor * 0xE + 0x13);
            data->menu_cursor->show(data->menu_cursor, 1);
            obj->base.substep++;
        }
        break;
    case 3:
        ststatus_module.window_anim_update(&obj->stats_anim);
        if (ststatus_module.window_anim_update(&obj->list_anim)) {
            ststatus_status_show_equipment(obj, data, 1);
            ststatus_status_show_stats(obj, data, 1);
            obj->base.substep = 5;
        }
        break;
    case 5:
        old = obj->menu_cursor;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            obj->menu_cursor = 0;
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            obj->menu_cursor = 1;
        }
        if (old != obj->menu_cursor) {
            sound_module.play(0x8004513E);
            data->menu_cursor->set_pos(data->menu_cursor, 0x98, obj->menu_cursor * 0xE + 0x13);
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x8004503C);
            obj->base.substep = 10;
            obj->base.timer = 0;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.substep = 10;
            obj->menu_cursor = 0;
            obj->base.timer = 1;
        }
        break;
    case 10:
        if (obj->base.timer != 0) {
            ststatus_module.window_anim_start(&obj->member_anim, 0);
            ststatus_status_show_own_panel(obj, data, 0, 0);
        }
        ststatus_module.window_anim_start(&obj->name_anim, 0);
        ststatus_module.window_anim_start(&obj->menu_anim, 0);
        ststatus_module.window_anim_start(&obj->list_anim, 0);
        ststatus_status_show_menu(obj, data, 0);
        ststatus_status_show_forms(obj, data, 0);
        ststatus_status_show_equipment(obj, data, 0);
        if (obj->menu_cursor == 0) {
            ststatus_module.window_anim_start(&obj->exp_anim, 0);
            ststatus_module.window_anim_start(&obj->stats_anim, 0);
            ststatus_status_show_stats(obj, data, 0);
            data->exp->set_visible(data->exp, 0);
            data->exp_label->set_visible(data->exp_label, 0);
        }
        data->menu_cursor->show(data->menu_cursor, 0);
        obj->base.substep++;
        break;
    case 11:
        if (obj->base.timer != 0) {
            ststatus_module.window_anim_update(&obj->member_anim);
        }
        ststatus_module.window_anim_update(&obj->name_anim);
        ststatus_module.window_anim_update(&obj->menu_anim);
        if (obj->menu_cursor == 0) {
            ststatus_module.window_anim_update(&obj->stats_anim);
            ststatus_module.window_anim_update(&obj->exp_anim);
        }
        if (ststatus_module.window_anim_update(&obj->list_anim)) {
            obj->base.substep++;
        }
        break;
    case 12:
        if (obj->base.timer != 0) {
            obj->base.set_step(obj, 0);
        } else {
            obj->base.set_step(obj, 20);
        }
        break;
    }
}

/* The page's states: open the party's panels, pick a member, run the member view and its sub-pages, close. */
void ststatus_status_run(StstatusStatusPage *obj, StstatusStatusPageData *data) {
    s32 old;

    switch (obj->base.step) {
    case 0:
    default:
        ststatus_module.window_anim_start(&obj->panel_anims[0], 1);
        ststatus_module.window_anim_start(&obj->bar_anims[0], 1);
        if (obj->member_count == 1) {
            ststatus_module.window_anim_start(&obj->bar_anims[1], 1);
        }
        obj->base.step = obj->member_count;
        break;
    case 1:
        ststatus_module.window_anim_update(&obj->panel_anims[0]);
        ststatus_module.window_anim_update(&obj->bar_anims[0]);
        if (ststatus_module.window_anim_update(&obj->bar_anims[1])) {
            ststatus_status_show_member(obj, data, 0, 1);
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x27);
            data->help->set_text(data->help, cdload_module.files.get_file(records_language + 0xB0), 0x30);
            data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0xB0), 0x15);
            obj->base.step = 10;
        }
        break;
    case 2:
        ststatus_module.window_anim_update(&obj->panel_anims[0]);
        if (ststatus_module.window_anim_update(&obj->bar_anims[0])) {
            ststatus_module.window_anim_start(&obj->panel_anims[1], 1);
            ststatus_module.window_anim_start(&obj->bar_anims[1], 1);
            ststatus_status_show_member(obj, data, 0, 1);
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x27);
            obj->base.step = 4;
        }
        break;
    case 4:
        ststatus_module.window_anim_update(&obj->panel_anims[1]);
        if (ststatus_module.window_anim_update(&obj->bar_anims[1])) {
            ststatus_status_show_member(obj, data, 1, 1);
            data->help->set_text(data->help, cdload_module.files.get_file(records_language + 0xB0), 0x30);
            data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0xB0), 0x15);
            obj->base.step = 10;
        }
        break;
    case 3:
        ststatus_module.window_anim_update(&obj->panel_anims[0]);
        if (ststatus_module.window_anim_update(&obj->bar_anims[0])) {
            ststatus_module.window_anim_start(&obj->panel_anims[1], 1);
            ststatus_status_show_member(obj, data, 0, 1);
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x27);
            obj->base.step = 5;
        }
        break;
    case 5:
        if (ststatus_module.window_anim_update(&obj->panel_anims[1])) {
            ststatus_module.window_anim_start(&obj->panel_anims[2], 1);
            ststatus_module.window_anim_start(&obj->bar_anims[1], 1);
            ststatus_status_show_member(obj, data, 1, 1);
            obj->base.step++;
        }
        break;
    case 6:
        ststatus_module.window_anim_update(&obj->panel_anims[2]);
        if (ststatus_module.window_anim_update(&obj->bar_anims[1])) {
            ststatus_status_show_member(obj, data, 2, 1);
            data->help->set_text(data->help, cdload_module.files.get_file(records_language + 0xB0), 0x30);
            data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0xB0), 0x15);
            obj->base.step = 10;
        }
        break;
    case 10:
        obj->cursor_shown = 1;
        obj->base.step++;
        break;
    case 11:
        old = obj->member;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            if (--obj->member < 0) {
                obj->member = 0;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            obj->member++;
            if (obj->member_count - 1 < obj->member) {
                obj->member = obj->member_count - 1;
            }
        }
        if (old != obj->member) {
            sound_module.play(0x4001B);
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            obj->cursor_shown = 0;
            obj->menu_cursor = 0;
            obj->base.set_step(obj, 50);
            obj->base.substep = 1;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->cursor_shown = 0;
            obj->base.set_step(obj, 50);
        }
        break;
    case 15:
        ststatus_status_run_member(obj, data);
        break;
    case 20:
        if (obj->menu_cursor == 0) {
            if (data->sub_page == NULL) {
                data->sub_page = (Object *)ststatus_create_forms_page(obj);
            }
        } else if (data->sub_page == NULL) {
            data->sub_page = ststatus_create_equip_page(obj);
        }
        obj->base.step++;
        break;
    case 21:
        if (data->sub_page == NULL) {
            obj->base.set_step(obj, 15);
            obj->base.timer = 1;
        }
        break;
    case 50:
        ststatus_module.window_anim_start(&obj->panel_anims[obj->member_count - 1], 0);
        ststatus_status_show_member(obj, data, obj->member_count - 1, 0);
        ststatus_module.window_anim_start(&obj->bar_anims[1], 0);
        data->help->set_visible(data->help, 0);
        data->back_hint->set_visible(data->back_hint, 0);
        if (obj->member_count == 1) {
            ststatus_module.window_anim_start(&obj->bar_anims[0], 0);
            data->title->set_visible(data->title, 0);
        }
        obj->base.step = obj->member_count + 50;
        break;
    case 51:
        ststatus_module.window_anim_update(&obj->panel_anims[0]);
        ststatus_module.window_anim_update(&obj->bar_anims[0]);
        if (ststatus_module.window_anim_update(&obj->bar_anims[1])) {
            obj->base.step = 57;
        }
        break;
    case 52:
        ststatus_module.window_anim_update(&obj->panel_anims[1]);
        if (ststatus_module.window_anim_update(&obj->bar_anims[1])) {
            ststatus_module.window_anim_start(&obj->panel_anims[0], 0);
            ststatus_module.window_anim_start(&obj->bar_anims[0], 0);
            ststatus_status_show_member(obj, data, 0, 0);
            data->title->set_visible(data->title, 0);
            obj->base.step = 54;
        }
        break;
    case 53:
        ststatus_module.window_anim_update(&obj->panel_anims[2]);
        if (ststatus_module.window_anim_update(&obj->bar_anims[1])) {
            ststatus_module.window_anim_start(&obj->panel_anims[1], 0);
            ststatus_status_show_member(obj, data, 1, 0);
            obj->base.step = 55;
        }
        break;
    case 55:
        if (ststatus_module.window_anim_update(&obj->panel_anims[1])) {
            ststatus_module.window_anim_start(&obj->panel_anims[0], 0);
            ststatus_module.window_anim_start(&obj->bar_anims[0], 0);
            ststatus_status_show_member(obj, data, 0, 0);
            data->title->set_visible(data->title, 0);
            obj->base.step++;
        }
        break;
    case 54:
    case 56:
        ststatus_module.window_anim_update(&obj->panel_anims[0]);
        if (ststatus_module.window_anim_update(&obj->bar_anims[0])) {
            obj->base.step = 57;
        }
        break;
    case 57:
        if (obj->base.substep != 0) {
            obj->base.set_step(obj, 15);
        } else {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}

void ststatus_status_update(StstatusStatusPage *obj, StstatusStatusPageData *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        for (i = 0; i < 3; i++) {
            if (gamestate_data.funcs.get_party_member(i) >= 0) {
                obj->member_count++;
            }
        }
        for (i = 0; i < obj->member_count; i++) {
            obj->panel_anims[i].duration = 10;
            ststatus_module.window_anim_start(&obj->panel_anims[i], 1);
        }
        for (i = 0; i < 2; i++) {
            obj->bar_anims[i].duration = 10;
            ststatus_module.window_anim_start(&obj->bar_anims[i], 1);
        }
        obj->member_anim.duration = 10;
        obj->name_anim.duration = 10;
        obj->stats_anim.duration = 10;
        obj->list_anim.duration = 10;
        obj->menu_anim.duration = 10;
        obj->exp_anim.duration = 10;
        obj->unused_anim.duration = 8;
        ststatus_status_create_windows(obj, data);
        break;
    case OBJECT_STATE_RUN:
        ststatus_status_run(obj, data);
        ststatus_status_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Creates the status page. */
StstatusStatusPage *ststatus_create_status_page(s32 arg0) {
    StstatusStatusPage *obj = object_new(ststatus_status_update, sizeof(StstatusStatusPage), sizeof(StstatusStatusPageData));

    obj->preview_item = ststatus_status_preview_item;
    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->parent = arg0;
    return obj;
}
