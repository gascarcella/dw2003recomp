#include "common.h"
#include "object.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "stgdglab.h"

/* STGDGLAB.PRO: the lab's Digimon screen (stgdglab_menu's entry 1): the party member's stats and set forms,
 * and two entries; each opens the forms list, then one of two screens for the form chosen
 * (stgdglab_formset_create, stgdglab_techs_create). */

/* The Digimon screen (size 0x130). */
struct StgdglabDigimonScreen {
    /* 0x000 */ Object base;
    /* 0x050 */ StgdglabMain *main;
    /* 0x054 */ WindowAnim anims[6]; /* [4], [5]: the two messages */
    /* 0x0B4 */ s32 layer_id; /* layer */
    /* 0x0B8 */ s32 ot_depth; /* ordering table entry */
    /* 0x0BC */ s32 cursor; /* the entry under the cursor */
    /* 0x0C0 */ s32 unk_C0;
    /* 0x0C4 */ s32 unk_C4;
    /* 0x0C8 */ s32 chosen_form; /* the form chosen in the list */
    /* 0x0CC */ s16 set_forms[3]; /* the set forms (gamestate_data.funcs.get_chosen_forms) */
    /* 0x0D2 */ s16 other_forms[44]; /* the other forms (unk_44) */
    /* 0x12C */ s32 form_count; /* how many (unk_44's result) */
}; /* StgdglabDigimonScreen, size 0x130 */

/* Its data block (0x60 bytes). */
typedef struct StgdglabDigimonData {
    /* 0x00 */ MessageWindow *title;      /* title */
    /* 0x04 */ MessageWindow *stats[13];  /* stats */
    /* 0x38 */ MessageWindow *set_forms[3]; /* the set forms */
    /* 0x44 */ MessageWindow *entries[2]; /* the entries */
    /* 0x4C */ MessageWindow *message;    /* message */
    /* 0x50 */ MessageCursor *cursor;     /* cursor */
    /* 0x54 */ StgdglabForms *forms_list; /* the forms list */
    /* 0x58 */ StgdglabFormset *form_change; /* the form change */
    /* 0x5C */ StgdglabTechs *techniques; /* the techniques */
} StgdglabDigimonData; /* size 0x60 */

/* Creates the screen's windows and its cursor. */
void stgdglab_digimon_create_windows(StgdglabDigimonScreen *obj, StgdglabDigimonData *data) {
    s32 i;
    MessageWindow **windows;

    data->title = message_create_window(obj->layer_id, 1, 0xAE, 0x57);
    for (i = 0; i < 3; i++) {
        data->set_forms[i] = message_create_window(obj->layer_id, 1, 0xA7, i * 14 + 0x16);
    }
    for (i = 0; i < 2; i++) {
        data->entries[i] = message_create_window(obj->layer_id, 1, 0xA7, i * 14 + 0x73);
    }
    for (i = 0; i < 6; i++) {
        data->stats[i] = message_create_window(obj->layer_id, 1, 0x32, i * 14 + 0x4F);
    }
    for (i = 0; i < 7; i++) {
        data->stats[i + 6] = message_create_window(obj->layer_id, 1, 0x5B, i * 14 + 0x4F);
    }
    windows = (MessageWindow **)obj->base.children;
    for (i = 0; i < obj->base.child_count - 4; i++, windows++) {
        if (*windows != NULL) {
            (*windows)->set_ot_depth(*windows, obj->ot_depth - 1);
        }
    }
    data->cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0x9A, 0x73);
    data->cursor->show(data->cursor, 0);
}

/* Shows (`show`) the party member's stats and set forms, or hides the windows. */
void stgdglab_digimon_show(StgdglabDigimonScreen *obj, StgdglabDigimonData *data, s32 show) {
    GamestateStats stats;
    s32 i;
    s32 digimon;
    GamestateDigimon *d;
    MessageWindow **windows;

    if (show) {
        i = 0;
        digimon = gamestate_data.funcs.get_party_member(obj->main->member);
        gamestate_data.funcs.get_stats(digimon, &stats);
        d = &gamestate_data.digimon[digimon];
        data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x39), 1);
        for (; i < 3; i++) {
            if (obj->set_forms[i] < 2) {
                if (i == 0) {
                    data->set_forms[i]->set_text(data->set_forms[i], cdload_module.files.get_file(records_language + 0x39), 0x1B);
                } else {
                    data->set_forms[i]->set_visible(data->set_forms[i], 0);
                }
            } else {
                data->set_forms[i]->set_text(data->set_forms[i], cdload_module.files.get_file(records_language + 0x4E),
                                         records_get_digimon_func(obj->set_forms[i])->name_id);
                if (d->shown_form == obj->set_forms[i]) {
                    data->set_forms[i]->set_palette(data->set_forms[i], 1);
                } else {
                    data->set_forms[i]->set_palette(data->set_forms[i], 0);
                }
            }
        }
        for (i = 0; i < 2; i++) {
            data->entries[i]->set_text(data->entries[i], cdload_module.files.get_file(records_language + 0x39), i + 0x21);
        }
        for (i = 0; i < 13; i++) {
            s32 v = (stats.values + 6)[i]; /* not stats.unk_00[i + 6]: the original steps a pointer to stats */

            if (v >= 1000) {
                v = 999;
            }
            data->stats[i]->set_line_number(data->stats[i], 0, v);
        }
        if (stats.penalties[0] != 0) {
            data->stats[0]->set_palette(data->stats[0], 6);
        }
        if (stats.penalties[1] != 0) {
            data->stats[1]->set_palette(data->stats[1], 6);
        }
        if (stats.penalties[2] != 0) {
            data->stats[4]->set_palette(data->stats[4], 6);
        }
        for (i = 0; i < 13; i++) {
            data->stats[i]->measure(data->stats[i], 1);
        }
        data->cursor->show(data->cursor, 1);
    } else {
        windows = (MessageWindow **)obj->base.children;
        for (i = 0; i < obj->base.child_count - 4; i++, windows++) {
            if (*windows != NULL) {
                (*windows)->set_visible(*windows, 0);
            }
        }
        data->cursor->show(data->cursor, 0);
    }
}

/* The screen's steps (base.step): open (0-1), choose an entry (2), close (3-4), the forms list (40-42), the form
 * change (10-12), the other screen (20-22) and the two messages (30-32, 50-52). */
void stgdglab_digimon_run(StgdglabDigimonScreen *obj, StgdglabDigimonData *data) {
    s16 set[4];
    GamestateForm form;
    s32 digimon;
    s32 old;
    s32 i;
    s32 found;
    s32 member;

    switch (obj->base.step) {
    case 0:
    default:
        member = gamestate_data.funcs.get_party_member(obj->main->member);
        gamestate_data.funcs.get_chosen_forms(member, obj->set_forms);
        obj->form_count = gamestate_data.funcs.list_forms(member, obj->other_forms);
        stgdglab_funcs.window_anim_start(&obj->anims[0], 1);
        stgdglab_funcs.window_anim_start(&obj->anims[1], 1);
        stgdglab_funcs.window_anim_start(&obj->anims[2], 1);
        stgdglab_funcs.window_anim_start(&obj->anims[3], 1);
        obj->base.step++;
        break;
    case 1:
        stgdglab_funcs.window_anim_update(&obj->anims[0]);
        stgdglab_funcs.window_anim_update(&obj->anims[1]);
        stgdglab_funcs.window_anim_update(&obj->anims[2]);
        if (stgdglab_funcs.window_anim_update(&obj->anims[3])) {
            stgdglab_digimon_show(obj, data, 1);
            obj->base.step++;
        }
        break;
    case 2:
        old = obj->cursor;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            if (--obj->cursor < 0) {
                obj->cursor = 0;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            if (++obj->cursor >= 2) {
                obj->cursor = 1;
            }
        }
        if (old != obj->cursor) {
            sound_module.play(0x8004513E);
            data->cursor->set_pos(data->cursor, 0x9A, obj->cursor * 14 + 0x72);
        }
        if (PAD_PRESSED(0xD)) {
            sound_module.play(0x8004503C);
            if ((obj->cursor == 0 && obj->form_count < 4) || (obj->cursor == 1 && obj->form_count == 0)) {
                obj->base.set_step(obj, 30);
                stgdglab_funcs.window_anim_start(&obj->anims[4], 1);
                data->cursor->show(data->cursor, 0);
            } else {
                obj->base.substep = 0;
                obj->base.step++;
            }
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->base.substep = 1;
            obj->base.step++;
        }
        break;
    case 3:
        stgdglab_funcs.window_anim_start(&obj->anims[0], 0);
        stgdglab_funcs.window_anim_start(&obj->anims[1], 0);
        stgdglab_funcs.window_anim_start(&obj->anims[2], 0);
        stgdglab_funcs.window_anim_start(&obj->anims[3], 0);
        stgdglab_digimon_show(obj, data, 0);
        obj->base.step++;
        break;
    case 4:
        stgdglab_funcs.window_anim_update(&obj->anims[0]);
        stgdglab_funcs.window_anim_update(&obj->anims[1]);
        stgdglab_funcs.window_anim_update(&obj->anims[2]);
        if (stgdglab_funcs.window_anim_update(&obj->anims[3])) {
            if (obj->base.substep == 0) {
                obj->chosen_form = 0;
                obj->base.set_step(obj, 40);
                switch (obj->cursor) {
                case 0:
                default:
                    obj->base.substep = 10;
                    break;
                case 1:
                    obj->base.substep = 20;
                    break;
                case 2:
                    obj->base.set_state(obj, OBJECT_STATE_END);
                    break;
                }
            } else {
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
        }
        break;
    case 40:
        if (data->forms_list == NULL) {
            data->forms_list = stgdglab_forms_create(gamestate_data.funcs.get_party_member(obj->main->member), 0, 0);
        }
        data->forms_list->cursor = obj->chosen_form;
        obj->base.step++;
        break;
    case 41:
        if (data->forms_list == NULL || data->forms_list->base.step >= 2) {
            if (PAD_PRESSED(0xD)) {
                sound_module.play(0x8004503C);
                if (obj->base.substep == 20) {
                    digimon = gamestate_data.funcs.get_party_member(obj->main->member);
                    gamestate_data.funcs.get_chosen_forms(digimon, set);
                    gamestate_data.funcs.get_form(digimon, set[data->forms_list->cursor], &form);
                    digimon = 0;
                    for (i = 0; i < 6; i++) {
                        if (form.techniques[i] != 0) {
                            digimon = 1;
                            break;
                        }
                    }
                    if (digimon == 0) {
                        obj->base.set_step(obj, 50);
                        stgdglab_funcs.window_anim_start(&obj->anims[5], 1);
                        data->cursor->show(data->cursor, 0);
                        data->forms_list->base.step = 4;
                        break;
                    }
                }
                obj->chosen_form = data->forms_list->cursor;
                obj->base.step = obj->base.substep;
                obj->base.set_substep(obj, 0);
            } else if (PAD_PRESSED(0xE)) {
                sound_module.play(0x800450BD);
                data->forms_list->close(data->forms_list);
                obj->base.step++;
            }
        }
        break;
    case 42:
        if (data->forms_list == NULL) {
            obj->base.set_step(obj, 0);
        }
        break;
    case 10:
        obj->main->close_menu(obj->main);
        data->forms_list->close(data->forms_list);
        obj->base.step++;
        break;
    case 11:
        if (obj->main->is_menu_running(obj->main) != 0 && data->forms_list == NULL) {
            if (data->form_change == NULL) {
                data->form_change = stgdglab_formset_create(gamestate_data.funcs.get_party_member(obj->main->member), obj->chosen_form);
            }
            obj->base.step++;
        }
        break;
    case 12:
        if (data->form_change == NULL) {
            obj->main->open_menu(obj->main);
            obj->base.set_step(obj, 40);
            obj->base.substep = 10;
        }
        break;
    case 20:
        data->forms_list->close(data->forms_list);
        obj->base.step++;
        break;
    case 21:
        if (data->forms_list == NULL) {
            if (data->techniques == NULL) {
                data->techniques = stgdglab_techs_create(gamestate_data.funcs.get_party_member(obj->main->member), obj->chosen_form);
            }
            obj->base.step++;
        }
        break;
    case 22:
        if (data->techniques == NULL) {
            obj->base.set_step(obj, 40);
            obj->base.substep = 20;
        }
        break;
    case 30:
        if (stgdglab_funcs.window_anim_update(&obj->anims[4])) {
            if (data->message == NULL) {
                data->message = message_create_window(obj->layer_id, 1, 0xA4, 0xB8);
            }
            data->message->set_ot_depth(data->message, obj->ot_depth - 1);
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x39), obj->cursor + 0x24);
            obj->base.step++;
        }
        break;
    case 31:
        if (PAD_PRESSED(0xD)) {
            sound_module.play(0x4001C);
            stgdglab_funcs.window_anim_start(&obj->anims[4], 0);
            data->message->set_visible(data->message, 0);
            obj->base.step++;
        }
        break;
    case 32:
        if (stgdglab_funcs.window_anim_update(&obj->anims[4])) {
            data->cursor->show(data->cursor, 1);
            obj->base.set_step(obj, 2);
        }
        break;
    case 50:
        if (stgdglab_funcs.window_anim_update(&obj->anims[5])) {
            if (data->message == NULL) {
                data->message = message_create_window(obj->layer_id, 1, 0xA4, 0xB8);
            }
            data->message->set_ot_depth(data->message, 0);
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x39), 0x25);
            obj->base.step++;
        }
        break;
    case 51:
        if (PAD_PRESSED(0xD)) {
            sound_module.play(0x4001C);
            stgdglab_funcs.window_anim_start(&obj->anims[5], 0);
            data->message->set_visible(data->message, 0);
            obj->base.step++;
        }
        break;
    case 52:
        if (stgdglab_funcs.window_anim_update(&obj->anims[5])) {
            obj->base.set_step(obj, 41);
            obj->base.substep = 20;
            data->forms_list->base.step = 5;
        }
        break;
    }
}

/* Draws the screen's frames, each scaled by its window_anim. */
void stgdglab_digimon_draw(StgdglabDigimonScreen *obj, StgdglabDigimonData *data) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->anims[5].level != 0) {
        spr.set_layer_id(obj->layer_id, 1);
        if (obj->anims[5].level != 0x1000) {
            spr.set_scale(obj->anims[5].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0xBE);
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x25, 0x92, 0xB0);
    }
    if (obj->anims[0].level != 0x1000) {
        spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
        spr.set_pivot(0, 0x7F);
    } else {
        spr.set_scale(0x1000, 0x1000, 0x1000);
    }
    spr.set_vram_pos(0x140, 0);
    spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xF, 0, 0x4B);
    spr.set_vram_pos(0x280, 0x100);
    spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x1F, 0, 0x4B);
    if (obj->anims[1].level != 0x1000) {
        spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
        spr.set_pivot(0x140, 0x2A);
    } else {
        spr.set_scale(0x1000, 0x1000, 0x1000);
    }
    spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x22, 0x92, 0xF);
    if (obj->anims[2].level != 0x1000) {
        spr.set_scale(obj->anims[3].level, 0x1000, 0x1000);
        spr.set_pivot(0x140, 0x5C);
    } else {
        spr.set_scale(0x1000, 0x1000, 0x1000);
    }
    spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x20, 0x92, 0x51);
    if (obj->anims[3].level != 0x1000) {
        spr.set_scale(obj->anims[3].level, 0x1000, 0x1000);
        spr.set_pivot(0x140, 0x87);
    } else {
        spr.set_scale(0x1000, 0x1000, 0x1000);
    }
    spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x23, 0x92, 0x6C);
    if (obj->anims[4].level != 0) {
        if (obj->anims[4].level != 0x1000) {
            spr.set_scale(obj->anims[4].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0xBE);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x25, 0x92, 0xB0);
    }
}

void stgdglab_digimon_update(StgdglabDigimonScreen *obj, StgdglabDigimonData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anims[0].duration = 8;
        obj->anims[1].duration = 10;
        obj->anims[2].duration = 10;
        obj->anims[3].duration = 10;
        obj->anims[4].duration = 10;
        obj->anims[5].duration = 8;
        stgdglab_digimon_create_windows(obj, data);
        break;
    case OBJECT_STATE_RUN:
        stgdglab_digimon_run(obj, data);
        stgdglab_digimon_draw(obj, data);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StgdglabDigimonScreen *stgdglab_digimon_create(StgdglabMain *main) {
    StgdglabDigimonScreen *obj =
        object_new(stgdglab_digimon_update, sizeof(StgdglabDigimonScreen), sizeof(StgdglabDigimonData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->main = main;
    return obj;
}
