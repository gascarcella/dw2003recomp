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

/* STGDGLAB.PRO: the forms list of a party member's Digimon (stgdglab_forms_create): its three set forms
 * (gamestate_data.funcs.get_chosen_forms) and, with list_all, the others it has (unk_44); the form under the cursor
 * shows its stat bonuses and its techniques. Created by the menu and two of the lab's screens. */

/* Its data block (0x70 bytes). */
typedef struct StgdglabFormsData {
    /* 0x00 */ MessageWindow *title;     /* title */
    /* 0x04 */ MessageWindow *form_windows[3]; /* the forms shown */
    /* 0x10 */ MessageWindow *form_name; /* the form's name */
    /* 0x14 */ MessageWindow *skill_lv_label;
    /* 0x18 */ MessageWindow *techniques[6]; /* its techniques */
    /* 0x30 */ MessageWindow *stats[14];  /* its stats (with the form's bonuses), then the level */
    /* 0x68 */ MessageCursor *cursor;    /* cursor */
    /* 0x6C */ StgdglabBar *scroll_bar; /* scroll bar */
} StgdglabFormsData; /* size 0x70 */

/* Creates the list's windows. */
void stgdglab_forms_create_windows(StgdglabForms *obj, StgdglabFormsData *data) {
    s32 i;
    MessageWindow **windows;

    data->title = message_create_window(obj->layer_id, 1, 0xAE, 0x15);
    for (i = 0; i < 3; i++) {
        data->form_windows[i] = message_create_window(obj->layer_id, 1, 0xA7, i * 14 + 0x31);
    }
    data->form_name = message_create_window(obj->layer_id, 1, 0x5B, 0x6A);
    for (i = 0; i < 6; i++) {
        data->stats[i] = message_create_window(obj->layer_id, 1, 0x7D, i * 14 + 0x7F);
    }
    for (i = 0; i < 7; i++) {
        data->stats[i + 6] = message_create_window(obj->layer_id, 1, 0xA7, i * 14 + 0x7F);
    }
    data->skill_lv_label = message_create_window(obj->layer_id, 1, 0xEB, 0x6A);
    data->stats[13] = message_create_window(obj->layer_id, 1, 0x12D, 0x6A);
    for (i = 0; i < 6; i++) {
        data->techniques[i] = message_create_window(obj->layer_id, 1, 0xC2, i * 14 + 0x8B);
    }
    windows = (MessageWindow **)obj->base.children;
    for (i = 0; i < obj->base.child_count - 2; i++, windows++) {
        if (*windows != NULL) {
            (*windows)->set_ot_depth(*windows, obj->ot_depth - 1);
        }
    }
}

/* Shows (`show`) the forms in view and the one under the cursor, or hides the windows. */
void stgdglab_forms_show(StgdglabForms *obj, StgdglabFormsData *data, s32 show) {
    GamestateStats stats;
    GamestateForm form;
    s32 i;
    s32 id;
    s32 v;
    RecordsDigimon *d;
    MessageWindow **windows;
    GamestateDigimon *digimon;

    if (show) {
        if (obj->form_count == 0) {
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x39), 0x1E);
        } else {
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x39), 0x1F);
        }
        digimon = &gamestate_data.digimon[obj->digimon];
        for (i = 0; i < 3; i++) {
            id = obj->forms[obj->first_form + i];
            if (id < 3) {
                if (i == 0) {
                    data->form_windows[i]->set_text(data->form_windows[i], cdload_module.files.get_file(records_language + 0x39), 0x1B);
                } else {
                    data->form_windows[i]->set_visible(data->form_windows[i], 0);
                }
            } else {
                data->form_windows[i]->set_text(data->form_windows[i], cdload_module.files.get_file(records_language + 0x4E),
                                         records_get_digimon_func(id)->name_id);
                if (digimon->shown_form == id) {
                    data->form_windows[i]->set_palette(data->form_windows[i], 1);
                } else {
                    data->form_windows[i]->set_palette(data->form_windows[i], 0);
                }
            }
        }
        id = obj->forms[obj->first_form + obj->cursor];
        if (id != 0) {
            gamestate_data.funcs.get_form(obj->digimon, id, &form);
            gamestate_data.funcs.get_stats(obj->digimon, &stats);
            i = 0;
            d = records_get_digimon_func(id);
            data->form_name->set_text(data->form_name, cdload_module.files.get_file(records_language + 0x4E), d->name_id);
            data->skill_lv_label->set_text(data->skill_lv_label, cdload_module.files.get_file(records_language + 0x39), 0x13);
            data->stats[13]->set_line_number(data->stats[13], i, form.level);
            for (; i < 6; i++) {
                v = stats.stats[i] + d->stats[i];
                if (v >= 1000) {
                    v = 999;
                }
                data->stats[i]->set_line_number(data->stats[i], 0, v);
                if ((i == 0 && stats.penalties[0] != 0) || (i == 1 && stats.penalties[1] != 0) ||
                    (i == 4 && stats.penalties[2] != 0)) {
                    data->stats[i]->set_palette(data->stats[i], 6);
                }
            }
            for (i = 0; i < 7; i++) {
                v = stats.resists[i] + d->resists[i];
                if (v >= 1000) {
                    v = 999;
                }
                data->stats[i + 6]->set_line_number(data->stats[i + 6], 0, v);
            }
            for (i = 0; i < 6; i++) {
                if (form.techniques[i] != 0) {
                    data->techniques[i]->set_text(data->techniques[i], cdload_module.files.get_file(records_language + 0xA2),
                                             form.techniques[i] & 0x1FFF);
                    if (form.techniques[i] & 0x8000) {
                        data->techniques[i]->set_palette(data->techniques[i], 3);
                    } else if (form.techniques[i] & 0x4000) {
                        data->techniques[i]->set_palette(data->techniques[i], 4);
                    } else {
                        data->techniques[i]->set_palette(data->techniques[i], 0);
                    }
                } else {
                    data->techniques[i]->set_visible(data->techniques[i], 0);
                }
            }
            for (i = 0; i < 14; i++) {
                data->stats[i]->measure(data->stats[i], 1);
            }
        } else {
            data->form_name->set_visible(data->form_name, 0);
            data->skill_lv_label->set_visible(data->skill_lv_label, 0);
            for (i = 0; i < 14; i++) {
                data->stats[i]->set_visible(data->stats[i], 0);
            }
        }
    } else {
        windows = (MessageWindow **)obj->base.children;
        for (i = 0; i < obj->base.child_count - 2; i++, windows++) {
            if (*windows != NULL) {
                (*windows)->set_visible(*windows, 0);
            }
        }
    }
}

/* StgdglabForms.close: closes the list. */
void stgdglab_forms_close(StgdglabForms *obj) {
    StgdglabFormsData *data = (StgdglabFormsData *)obj->base.children;

    stgdglab_funcs.window_anim_start(&obj->anims[2], 0);
    stgdglab_forms_show(obj, data, 0);
    data->cursor->show(data->cursor, 0);
    obj->base.step++;
}

void stgdglab_forms_run(StgdglabForms *obj, StgdglabFormsData *data) {
    s32 old_cursor;
    s32 old_first;

    switch (obj->base.step) {
    case 0:
    default:
        stgdglab_funcs.window_anim_start(&obj->anims[2], 1);
        obj->base.step++;
        break;
    case 1:
        if (stgdglab_funcs.window_anim_update(&obj->anims[2])) {
            stgdglab_forms_show(obj, data, 1);
            data->cursor->show(data->cursor, 1);
            if (obj->form_count >= 4) {
                data->scroll_bar = stgdglab_bar_create();
                data->scroll_bar->set_x(data->scroll_bar, 0x122, 0xC);
                data->scroll_bar->set_range(data->scroll_bar, 0x35, 0x55);
                data->scroll_bar->set_lines(data->scroll_bar, 3, obj->form_count);
                data->scroll_bar->set_line(data->scroll_bar, obj->cursor);
            }
            obj->base.step++;
        }
        break;
    case 2:
        if (obj->form_count > 0) {
            old_cursor = obj->cursor;
            old_first = obj->first_form;
            if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                if (--obj->cursor < 0) {
                    obj->cursor = 0;
                    if (--obj->first_form < 0) {
                        obj->first_form = 0;
                    }
                }
            } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                if (obj->form_count >= 3) {
                    if (++obj->cursor >= 3) {
                        obj->cursor = 2;
                        if (++obj->first_form > obj->form_count - 3) {
                            obj->first_form = obj->form_count - 3;
                        }
                    }
                } else if (++obj->cursor > obj->form_count - 1) {
                    obj->cursor = obj->form_count - 1;
                }
            }
            if (data->scroll_bar != NULL && old_first != obj->first_form) {
                data->scroll_bar->set_line(data->scroll_bar, obj->first_form);
            }
            if (old_cursor != obj->cursor || old_first != obj->first_form) {
                sound_module.play(0x8004513E);
                stgdglab_forms_show(obj, data, 1);
                data->cursor->set_pos(data->cursor, 0x9A, obj->cursor * 14 + 0x31);
            }
        }
        if (obj->can_cancel != 0 && PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            stgdglab_forms_close(obj);
            if (data->scroll_bar != NULL) {
                data->scroll_bar->base.state = OBJECT_STATE_END;
            }
        }
        break;
    case 3:
        if (stgdglab_funcs.window_anim_update(&obj->anims[2])) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case 4:
        data->cursor->show(data->cursor, 0);
        break;
    case 5:
        data->cursor->show(data->cursor, 1);
        obj->base.step = 2;
        break;
    }
    obj->anims[0].level = obj->anims[1].level = obj->anims[2].level;
}

/* Draws the list's frames, its arrows (blinking) and the form's technique icons. */
void stgdglab_forms_draw(StgdglabForms *obj, StgdglabFormsData *data) {
    Sprite spr;
    GamestateForm form;
    s32 i;

    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0x100);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->anims[0].level != 0x1000) {
        spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
        spr.set_pivot(0x140, 0x15);
    }
    spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x20, 0x92, 0xF);
    if (obj->anims[1].level != 0x1000) {
        spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
        spr.set_pivot(0x140, 0x45);
    } else {
        spr.set_scale(0x1000, 0x1000, 0x1000);
    }
    if (obj->anims[1].level == 0x1000) {
        if (gfx_module.funcs.get_time() - obj->blink_time >= 8) {
            obj->blink_time = gfx_module.funcs.get_time();
            obj->arrows_shown = 1 - obj->arrows_shown;
        }
        if (obj->arrows_shown != 0) {
            if (obj->first_form != 0) {
                spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x34, 0x123, 0x2D);
            }
            if (obj->first_form < obj->form_count - 3) {
                spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x35, 0x123, 0x55);
            }
        }
    }
    spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x22, 0x92, 0x2A);
    if (obj->form_count > 0) {
        if (obj->anims[2].level != 0x1000) {
            spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0xA4);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x2A, 0x54, 0x64);
        spr.set_vram_pos(0x140, 0);
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x14, 0x5B, 0x7F);
        gamestate_data.funcs.get_form(obj->digimon, obj->forms[obj->first_form + obj->cursor], &form);
        for (i = 0; i < 6; i++) {
            if (form.techniques[i] != 0) {
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_techniques[(form.techniques[i] & 0x1FFF) - 1].element + 0x37,
                           0xB4, i * 14 + 0x8B);
            }
        }
    }
}

void stgdglab_forms_update(StgdglabForms *obj, StgdglabFormsData *data) {
    s16 set[4];
    s16 all[44];
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        gamestate_data.funcs.get_chosen_forms(obj->digimon, set);
        for (i = 0; i < 3; i++) {
            if (set[i] < 3) {
                break;
            }
            obj->forms[obj->form_count] = set[i];
            obj->form_count++;
        }
        if (obj->list_all != 0) {
            gamestate_data.funcs.list_forms(obj->digimon, all);
            for (i = 0; i < 44; i++) {
                if (all[i] >= 3 && obj->forms[0] != all[i] && obj->forms[1] != all[i] &&
                    obj->forms[2] != all[i]) {
                    obj->forms[obj->form_count] = all[i];
                    obj->form_count++;
                }
            }
        }
        stgdglab_forms_create_windows(obj, data);
        data->cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0x9A, obj->cursor * 14 + 0x31);
        data->cursor->show(data->cursor, 0);
        obj->anims[0].duration = 10;
        obj->anims[1].duration = 10;
        obj->anims[2].duration = 10;
        break;
    case OBJECT_STATE_RUN:
        stgdglab_forms_run(obj, data);
        stgdglab_forms_draw(obj, data);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StgdglabForms *stgdglab_forms_create(s32 digimon, s32 all, s32 cancel) {
    StgdglabForms *obj = object_new(stgdglab_forms_update, sizeof(StgdglabForms), sizeof(StgdglabFormsData));

    obj->close = stgdglab_forms_close;
    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->list_all = all;
    obj->digimon = digimon;
    obj->can_cancel = cancel;
    return obj;
}
