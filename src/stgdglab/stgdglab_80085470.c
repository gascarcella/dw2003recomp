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

/* STGDGLAB.PRO: the form change (stgdglab_formset_create, from the Digimon screen): the Digimon's forms in two
 * columns; the one chosen replaces the set form `replaced`, and can be made the current form. */

/* The form change (size 0x100). */
struct StgdglabFormset {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id; /* layer */
    /* 0x54 */ s32 ot_depth; /* ordering table entry */
    /* 0x58 */ s32 digimon; /* the Digimon */
    /* 0x5C */ s32 replaced; /* the set form being replaced */
    /* 0x60 */ s32 cursor_column; /* cursor column */
    /* 0x64 */ s32 cursor_row; /* cursor row (from first_row) */
    /* 0x68 */ s32 first_row; /* first row shown */
    /* 0x6C */ s32 blink_time; /* time of the last arrow blink */
    /* 0x70 */ s32 arrows_shown; /* arrows shown */
    /* 0x74 */ s16 set_forms[3]; /* the set forms */
    /* 0x7A */ s16 forms[44];  /* the forms it has, two per row */
    /* 0xD2 */ u8 unk_D2[0x6];
    /* 0xD8 */ s32 form_count; /* how many */
    /* 0xDC */ WindowAnim anims[2];
    /* 0xFC */ s32 menu_cursor; /* cursor of the second menu */
}; /* size 0x100 */

/* Its data block (0xA0 bytes). */
typedef struct StgdglabFormsetData {
    /* 0x00 */ MessageWindow *form_windows[10]; /* the forms shown */
    /* 0x28 */ MessageWindow *form_name;  /* the form's name */
    /* 0x2C */ MessageWindow *skill_lv_label;
    /* 0x30 */ MessageWindow *techniques[6]; /* its techniques */
    /* 0x48 */ MessageWindow *stats[14];  /* its stats, then the level */
    /* 0x80 */ MessageWindow *message;
    /* 0x84 */ MessageWindow *menu[4];    /* the second menu: the set forms, then "none" */
    /* 0x94 */ MessageCursor *menu_cursor; /* its cursor */
    /* 0x98 */ MessageCursor *forms_cursor; /* the forms' cursor */
    /* 0x9C */ s32 unk_9C;
} StgdglabFormsetData; /* size 0xA0 */

void stgdglab_formset_update(StgdglabFormset *obj, StgdglabFormsetData *data);

/* Creates the windows and the two cursors. */
void stgdglab_formset_create_windows(StgdglabFormset *obj, StgdglabFormsetData *data) {
    s32 i;
    MessageWindow **windows;

    for (i = 0; i < 10; i++) {
        data->form_windows[i] = message_create_window(obj->layer_id, 1, (i % 2) * 0x85 + 0x27, (i / 2) * 14 + 0x16);
    }
    data->form_name = message_create_window(obj->layer_id, 1, 0x5B, 0x6A);
    data->skill_lv_label = message_create_window(obj->layer_id, 1, 0xF5, 0x6A);
    data->stats[13] = message_create_window(obj->layer_id, 1, 0x12D, 0x6A);
    for (i = 0; i < 6; i++) {
        data->techniques[i] = message_create_window(obj->layer_id, 1, 0xC2, i * 14 + 0x8B);
    }
    for (i = 0; i < 6; i++) {
        data->stats[i] = message_create_window(obj->layer_id, 1, 0x7D, i * 14 + 0x7F);
    }
    for (i = 0; i < 7; i++) {
        data->stats[i + 6] = message_create_window(obj->layer_id, 1, 0xA7, i * 14 + 0x7F);
    }
    windows = (MessageWindow **)obj->base.children;
    for (i = 0; i < obj->base.child_count - 3; i++, windows++) {
        if (*windows != NULL) {
            (*windows)->set_ot_depth(*windows, obj->ot_depth - 1);
        }
    }
    data->forms_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0x1A, 0x16);
    data->forms_cursor->show(data->forms_cursor, 0);
    data->menu_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 3, 0x9A, 0x83);
    data->menu_cursor->show(data->menu_cursor, 0);
    for (i = 0; i < 4; i++) {
        data->menu[i] = message_create_window(obj->layer_id, 1, 0xA7, i * 14 + 0x83);
    }
    data->message = message_create_window(obj->layer_id, 1, 0x14, 0xC3);
}

/* Shows (`show`) the forms in view and the one under the cursor, or hides the windows. */
void stgdglab_formset_show(StgdglabFormset *obj, StgdglabFormsetData *data, s32 show) {
    GamestateStats stats;
    GamestateForm form;
    s32 i;
    s32 j;
    s32 id;
    s32 v;
    RecordsDigimon *d;
    MessageWindow **windows;

    if (show) {
        for (i = 0; i < 10; i++) {
            id = obj->forms[obj->first_row * 2 + i];
            if (id >= 3) {
                data->form_windows[i]->set_palette(data->form_windows[i], 0);
                for (j = 0; j < 3; j++) {
                    if (obj->set_forms[j] == id) {
                        data->form_windows[i]->set_palette(data->form_windows[i], 7);
                        break;
                    }
                }
                data->form_windows[i]->set_text(data->form_windows[i], cdload_module.files.get_file(records_language + 0x4E),
                                         records_get_digimon_func(id)->name_id);
            } else {
                data->form_windows[i]->set_visible(data->form_windows[i], 0);
            }
        }
        id = obj->forms[(obj->first_row + obj->cursor_row) * 2 + obj->cursor_column];
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
        }
        data->forms_cursor->show(data->forms_cursor, 1);
    } else {
        windows = (MessageWindow **)obj->base.children;
        for (i = 0; i < obj->base.child_count - 3; i++, windows++) {
            if (*windows != NULL) {
                (*windows)->set_visible(*windows, 0);
            }
        }
        data->forms_cursor->show(data->forms_cursor, 0);
    }
}

/* Shows (`show`) the second menu (the set forms and "none") or hides it. */
void stgdglab_formset_show_menu(StgdglabFormset *obj, StgdglabFormsetData *data, s32 show) {
    s32 i;

    if (show) {
        for (i = 0; i < 3; i++) {
            data->menu[i]->set_text(data->menu[i], cdload_module.files.get_file(records_language + 0x4E),
                                     records_get_digimon_func(obj->set_forms[i])->name_id);
        }
        data->menu[3]->set_text(data->menu[3], cdload_module.files.get_file(records_language + 0x39), 0x2C);
        data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x39), 0x2B);
    } else {
        for (i = 0; i < 4; i++) {
            data->menu[i]->set_visible(data->menu[i], 0);
        }
        data->message->set_visible(data->message, 0);
    }
}

/* The form change: steps (base.step) open (0), choose a form (1; L1/R1 page by five rows), the second menu to make
 * one current (2-6); then draws the forms' marks, the arrows (blinking), the technique icons and the menu. */
void stgdglab_formset_update(StgdglabFormset *obj, StgdglabFormsetData *data) {
    Sprite spr;
    GamestateForm form;
    s32 changed;
    s32 old_first;
    s32 old_row;
    s32 old_sel;
    s32 rows;
    s32 i;
    s32 j;
    s32 k;
    s32 n;
    s32 id;
    GamestateDigimon *d;
    GamestateDigimon *rec;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anims[0].duration = 10;
        stgdglab_funcs.window_anim_start(&obj->anims[0], 1);
        gamestate_data.funcs.get_chosen_forms(obj->digimon, obj->set_forms);
        obj->form_count = gamestate_data.funcs.list_forms(obj->digimon, obj->forms);
        stgdglab_formset_create_windows(obj, data);
        obj->anims[1].duration = 10;
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            if (stgdglab_funcs.window_anim_update(&obj->anims[0])) {
                if (obj->base.substep != 0) {
                    obj->base.set_state(obj, OBJECT_STATE_END);
                } else {
                    stgdglab_formset_show(obj, data, 1);
                    obj->base.step++;
                }
            }
            break;
        case 1:
            old_first = obj->first_row;
            changed = 0;
            if (obj->form_count >= 11) {
                if ((!PAD_HELD(0xB) && PAD_PRESSED(0xA)) || (!PAD_HELD(0xB) && PAD_REPEAT(0xA))) {
                    obj->first_row -= 5;
                    if (obj->first_row < 0) {
                        obj->first_row = 0;
                    }
                } else if ((!PAD_HELD(0xA) && PAD_PRESSED(0xB)) || (!PAD_HELD(0xA) && PAD_REPEAT(0xB))) {
                    rows = obj->form_count - obj->form_count / 2;
                    for (n = 0; n < 5; n++) {
                        if (++obj->first_row + 4 > rows - 1) {
                            obj->first_row = rows - 5;
                            break;
                        }
                    }
                }
                if (old_first != obj->first_row) {
                    changed = 1;
                    obj->cursor_row = 0;
                }
            }
            if (changed == 0) {
                old_row = obj->cursor_row;
                if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                    if (--obj->cursor_row < 0) {
                        obj->cursor_row = 0;
                        if (--obj->first_row < 0) {
                            obj->first_row = 0;
                        }
                    }
                    if (old_row != obj->cursor_row || old_first != obj->first_row) {
                        changed = 1;
                    }
                } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                    if (obj->form_count >= 10) {
                        if (++obj->cursor_row >= 5) {
                            obj->cursor_row = 4;
                            if (++obj->first_row > obj->form_count - obj->form_count / 2 - 1) {
                                obj->first_row = obj->form_count - obj->form_count / 2 - 1;
                            }
                        }
                    } else if (++obj->cursor_row > obj->form_count - obj->form_count / 2 - 1) {
                        obj->cursor_row = obj->form_count - obj->form_count / 2 - 1;
                    }
                    if (old_first != obj->first_row || old_row != obj->cursor_row) {
                        if (obj->cursor_column != 0) {
                            if (obj->forms[(obj->first_row + obj->cursor_row) * 2 + obj->cursor_column] >= 3) {
                                changed = 1;
                            } else if (obj->forms[(obj->first_row + obj->cursor_row) * 2] >= 3) {
                                obj->cursor_column = 0;
                                changed = 1;
                            }
                        } else if (obj->forms[(obj->first_row + obj->cursor_row) * 2] >= 3) {
                            changed = 1;
                        }
                        if (changed == 0) {
                            obj->cursor_row = old_row;
                            obj->first_row = old_first;
                        }
                    }
                }
                if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
                    if (obj->cursor_column != 0) {
                        obj->cursor_column = 0;
                        changed = 1;
                    }
                } else if ((PAD_PRESSED(5) || PAD_REPEAT(5)) && obj->cursor_column == 0 &&
                           obj->forms[(obj->first_row + obj->cursor_row) * 2 + 1] >= 3) {
                    obj->cursor_column = 1;
                    changed = 1;
                }
            }
            if (changed != 0) {
                sound_module.play(0x8004513E);
                data->forms_cursor->set_pos(data->forms_cursor, obj->cursor_column * 0x85 + 0x1A, obj->cursor_row * 14 + 0x16);
                stgdglab_formset_show(obj, data, 1);
            } else if (PAD_PRESSED(0xD)) {
                id = obj->forms[(obj->first_row + obj->cursor_row) * 2 + obj->cursor_column];
                for (k = 0; k < 3; k++) {
                    if (obj->set_forms[k] == id) {
                        id = 0;
                        break;
                    }
                }
                if (id == 0) {
                    sound_module.play(0x800450BD);
                } else {
                    sound_module.play(0x8004503C);
                    rec = &gamestate_data.digimon[obj->digimon];
                    if (obj->set_forms[obj->replaced] == rec->shown_form) {
                        obj->base.step = 2;
                    } else {
                        stgdglab_formset_show(obj, data, 0);
                        stgdglab_funcs.window_anim_start(&obj->anims[0], 0);
                        obj->base.step = 0;
                        obj->base.substep = 1;
                    }
                    obj->set_forms[obj->replaced] = id;
                    gamestate_data.funcs.set_chosen_forms(obj->digimon, obj->set_forms);
                }
            } else if (PAD_PRESSED(0xE)) {
                sound_module.play(0x800450BD);
                stgdglab_formset_show(obj, data, 0);
                stgdglab_funcs.window_anim_start(&obj->anims[0], 0);
                obj->base.step = 0;
                obj->base.substep = 1;
            }
            break;
        case 2:
            obj->anims[1].duration = 10;
            stgdglab_funcs.window_anim_start(&obj->anims[1], 1);
            data->forms_cursor->set_palette(data->forms_cursor, 7);
            data->forms_cursor->stop(data->forms_cursor, 1);
            obj->base.step++;
            break;
        case 3:
            if (stgdglab_funcs.window_anim_update(&obj->anims[1])) {
                stgdglab_formset_show_menu(obj, data, 1);
                obj->menu_cursor = 0;
                data->menu_cursor->set_pos(data->menu_cursor, 0x9A, 0x83);
                data->menu_cursor->show(data->menu_cursor, 1);
                obj->base.step++;
            }
            break;
        case 4:
            old_sel = obj->menu_cursor;
            if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                if (--obj->menu_cursor < 0) {
                    obj->menu_cursor = 0;
                }
            } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                if (++obj->menu_cursor >= 4) {
                    obj->menu_cursor = 3;
                }
            }
            if (old_sel != obj->menu_cursor) {
                sound_module.play(0x8004513E);
                data->menu_cursor->set_pos(data->menu_cursor, 0x9A, obj->menu_cursor * 14 + 0x83);
            }
            d = &gamestate_data.digimon[obj->digimon];
            if (PAD_PRESSED(0xD)) {
                sound_module.play(0x8004503C);
                if (obj->menu_cursor < 3) {
                    d->shown_form = obj->set_forms[obj->menu_cursor];
                } else {
                    d->shown_form = 0;
                }
                obj->base.step++;
            } else if (PAD_PRESSED(0xE)) {
                sound_module.play(0x800450BD);
                d->shown_form = 0;
                obj->base.step++;
            }
            break;
        case 5:
            stgdglab_funcs.window_anim_start(&obj->anims[1], 0);
            stgdglab_formset_show_menu(obj, data, 0);
            data->menu_cursor->show(data->menu_cursor, 0);
            obj->base.step++;
            break;
        case 6:
            if (stgdglab_funcs.window_anim_update(&obj->anims[1])) {
                obj->base.step = 0;
                obj->base.substep = 1;
            }
            break;
        }
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        spr.set_vram_pos(0x280, 0x100);
        if (obj->anims[0].level != 0x1000) {
            spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x37);
        } else {
            for (j = 0; j < 10; j++) {
                if (obj->forms[obj->first_row * 2 + j] >= 3) {
                    spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x3E, (j % 2) * 0x85 + 0x18, (j / 2) * 14 + 0x16);
                }
            }
        }
        if (gfx_module.funcs.get_time() - obj->blink_time >= 8) {
            obj->blink_time = gfx_module.funcs.get_time();
            obj->arrows_shown = 1 - obj->arrows_shown;
        }
        if (obj->anims[0].level == 0x1000 && obj->arrows_shown != 0) {
            if (obj->first_row > 0) {
                spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x34, 0x123, 0x13);
            }
            if (obj->form_count >= 10 && obj->first_row < obj->form_count - obj->form_count / 2 - 5) {
                spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x35, 0x123, 0x55);
            }
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x29, 0x12, 0xF);
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x2A, 0x54, 0x64);
        spr.set_vram_pos(0x140, 0);
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x14, 0x5B, 0x7F);
        gamestate_data.funcs.get_form(obj->digimon, obj->forms[(obj->first_row + obj->cursor_row) * 2 + obj->cursor_column], &form);
        for (i = 0; i < 6; i++) {
            if (form.techniques[i] != 0) {
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_techniques[(form.techniques[i] & 0x1FFF) - 1].element + 0x37,
                           0xB4, i * 14 + 0x8B);
            }
        }
        if (obj->anims[1].level != 0) {
            sprite_init(&spr);
            spr.set_layer_id(obj->layer_id, obj->ot_depth - 2);
            spr.set_vram_pos(0x280, 0x100);
            if (obj->anims[1].level != 0x1000) {
                spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
                spr.set_pivot(0x140, 0x9D);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x2C, 0x95, 0x7C);
            if (obj->anims[1].level != 0x1000) {
                spr.set_pivot(0, 0xD0);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x2B, 0, 0xBF);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StgdglabFormset *stgdglab_formset_create(s32 digimon, s32 slot) {
    StgdglabFormset *obj = object_new(stgdglab_formset_update, sizeof(StgdglabFormset), sizeof(StgdglabFormsetData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->digimon = digimon;
    obj->replaced = slot;
    return obj;
}
