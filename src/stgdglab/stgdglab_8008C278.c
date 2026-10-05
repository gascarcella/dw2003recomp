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

/* STGDGLAB.PRO: the techniques of a set form (stgdglab_techs_create, from the Digimon screen): its six
 * techniques, which ones are set (flag 0x4000), and the cost of the one under the cursor. */

/* The techniques screen (size 0xAC). */
struct StgdglabTechs {
    /* 0x00 */ Object base;
    /* 0x50 */ WindowAnim anims[2];
    /* 0x70 */ s32 layer_id; /* layer */
    /* 0x74 */ s32 ot_depth; /* ordering table entry */
    /* 0x78 */ s32 digimon; /* the Digimon */
    /* 0x7C */ s32 set_form; /* the set form (index into set_forms) */
    /* 0x80 */ s16 set_forms[3]; /* the set forms */
    /* 0x86 */ u8 pad_86[0x2];
    /* 0x88 */ GamestateForm form_record; /* the form's record; its techniques: 0x1FFF the technique, 0x4000 set */
    /* 0x9C */ s32 set_count; /* techniques set */
    /* 0xA0 */ s32 question_cursor; /* cursor of the yes/no question */
    /* 0xA4 */ s32 cursor; /* cursor */
    /* 0xA8 */ s32 techniques; /* techniques */
}; /* size 0xAC */

/* Its data block (0x5C bytes). */
typedef struct StgdglabTechsData {
    /* 0x00 */ MessageWindow *form_name; /* the form's name */
    /* 0x04 */ MessageWindow *set_max;
    /* 0x08 */ MessageWindow *set_count; /* techniques set */
    /* 0x0C */ MessageWindow *techniques[6]; /* the techniques */
    /* 0x24 */ MessageWindow *set_marks[6]; /* set or not */
    /* 0x3C */ MessageWindow *message;
    /* 0x40 */ MessageWindow *yes;
    /* 0x44 */ MessageWindow *no;
    /* 0x48 */ MessageWindow *description; /* the technique's description */
    /* 0x4C */ MessageWindow *mp_label;
    /* 0x50 */ MessageWindow *cost;      /* its cost */
    /* 0x54 */ MessageCursor *yes_no_cursor;
    /* 0x58 */ MessageCursor *cursor;    /* cursor */
} StgdglabTechsData; /* size 0x5C */

void stgdglab_techs_update(StgdglabTechs *obj, StgdglabTechsData *data);

/* Creates the screen's windows and its two cursors. */
void stgdglab_techs_create_windows(StgdglabTechs *obj, StgdglabTechsData *data) {
    s32 i;
    MessageWindow **windows;

    data->form_name = message_create_window(obj->layer_id, 1, 0x9A, 0x46);
    data->set_max = message_create_window(obj->layer_id, 1, 0x122, 0x46);
    data->set_count = message_create_window(obj->layer_id, 1, 0x121, 0x46);
    for (i = 0; i < 6; i++) {
        data->techniques[i] = message_create_window(obj->layer_id, 1, 0x6B, i * 17 + 0x58);
        data->set_marks[i] = message_create_window(obj->layer_id, 1, 0xF1, i * 17 + 0x58);
    }
    data->message = message_create_window(obj->layer_id, 1, 0x9A, 0x1A);
    data->yes = message_create_window(obj->layer_id, 1, 0xC3, 0x38);
    data->no = message_create_window(obj->layer_id, 1, 0xC3, 0x48);
    data->description = message_create_window(obj->layer_id, 1, 0x14, 0xC3);
    data->description->set_page_lines(data->description, 2);
    data->mp_label = message_create_window(obj->layer_id, 1, 0x109, 0xD1);
    data->cost = message_create_window(obj->layer_id, 1, 0x12C, 0xD1);
    windows = (MessageWindow **)obj->base.children;
    for (i = 0; i < obj->base.child_count - 2; i++, windows++) {
        if (*windows != NULL) {
            (*windows)->set_ot_depth(*windows, obj->ot_depth - 1);
        }
    }
    data->yes->set_ot_depth(data->yes, 0);
    data->no->set_ot_depth(data->no, 0);
    data->yes_no_cursor = message_create_cursor(obj->layer_id, 0, 0xB8, 0x38);
    data->yes_no_cursor->show(data->yes_no_cursor, 0);
    data->cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0x4C, 0x58);
    data->cursor->show(data->cursor, 0);
}

/* Shows (`show`) the form's techniques, or hides the windows. */
void stgdglab_techs_show(StgdglabTechs *obj, StgdglabTechsData *data, s32 show) {
    s32 i;
    u16 tech;
    s32 id;
    MessageWindow **windows;

    if (show) {
        if (obj->form_record.id >= 3) {
            data->form_name->set_text(data->form_name, cdload_module.files.get_file(records_language + 0x4E),
                                  records_get_digimon_func(obj->form_record.id)->name_id);
            obj->set_count = 0;
            for (i = 0; i < obj->techniques; i++) {
                tech = obj->form_record.techniques[i];
                if (obj->form_record.techniques[i] != 0) {
                    data->techniques[i]->set_text(data->techniques[i], cdload_module.files.get_file(records_language + 0xA2),
                                             tech & 0x1FFF);
                    if (tech & 0x4000) {
                        data->set_marks[i]->set_text(data->set_marks[i], cdload_module.files.get_file(records_language + 0x39), 0x11);
                        obj->set_count++;
                    } else {
                        data->set_marks[i]->set_text(data->set_marks[i], cdload_module.files.get_file(records_language + 0x39), 0x12);
                    }
                    if (tech & 0x4000) {
                        data->techniques[i]->set_palette(data->techniques[i], 4);
                        data->set_marks[i]->set_palette(data->set_marks[i], 4);
                    } else if (tech & 0x8000) {
                        data->techniques[i]->set_palette(data->techniques[i], 3);
                        data->set_marks[i]->set_palette(data->set_marks[i], 3);
                    } else if (!(tech & 0x2000)) {
                        data->techniques[i]->set_palette(data->techniques[i], 7);
                        data->set_marks[i]->set_palette(data->set_marks[i], 7);
                    } else {
                        data->techniques[i]->set_palette(data->techniques[i], 0);
                        data->set_marks[i]->set_palette(data->set_marks[i], 0);
                    }
                } else {
                    data->techniques[i]->set_visible(data->techniques[i], 0);
                    data->set_marks[i]->set_visible(data->set_marks[i], 0);
                }
            }
            data->set_max->set_text(data->set_max, cdload_module.files.get_file(records_language + 0x39), 0x10);
            data->set_count->set_line_number(data->set_count, 0, obj->set_count);
            data->set_count->measure(data->set_count, 1);
            id = obj->form_record.techniques[obj->cursor] & 0x1FFF;
            if (id != 0) {
                data->description->set_text(data->description, cdload_module.files.get_file(records_language + 0x9B), id);
                data->mp_label->set_text(data->mp_label, cdload_module.files.get_file(records_language + 0x39), 0xE);
                data->cost->set_line_number(data->cost, 0, records_techniques[id - 1].mp_cost);
                data->cost->measure(data->cost, 1);
            } else {
                data->description->set_visible(data->description, 0);
                data->mp_label->set_visible(data->mp_label, 0);
                data->cost->set_visible(data->cost, 0);
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

/* The screen's object: steps (base.step) choose a technique (0-1), the messages (5-7, 30), set or clear it (10-11,
 * 20-21) and leave, writing the form's record back (40); then draws the icons and frames. */
void stgdglab_techs_update(StgdglabTechs *obj, StgdglabTechsData *data) {
    Sprite spr;
    s32 old;
    s32 i;
    s32 j;
    s32 y;
    s16 tech;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anims[0].duration = 10;
        obj->anims[1].duration = 10;
        stgdglab_funcs.window_anim_start(&obj->anims[0], 1);
        gamestate_data.funcs.get_chosen_forms(obj->digimon, obj->set_forms);
        gamestate_data.funcs.get_form(obj->digimon, obj->set_forms[obj->set_form], &obj->form_record);
        obj->techniques = 6;
        /* Evidence (class A2, sched2 barrier; DECISIONS "LOOP_BLOCK audit"): the original stores unk_A8 through obj
         * before the call's argument moves. */
        LOOP_BARRIER();
        stgdglab_techs_create_windows(obj, data);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        default:
            obj->base.set_state(obj, OBJECT_STATE_END);
        case 0:
            if (stgdglab_funcs.window_anim_update(&obj->anims[0])) {
                stgdglab_techs_show(obj, data, 1);
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x39), 0x26);
                data->cursor->set_pos(data->cursor, 0x4C, obj->cursor * 17 + 0x58);
                data->cursor->show(data->cursor, 1);
                data->cursor->set_palette(data->cursor, 0);
                data->yes_no_cursor->show(data->yes_no_cursor, 0);
                obj->base.step++;
            }
            break;
        case 1:
            old = obj->cursor;
            if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                if (--obj->cursor < 0) {
                    obj->cursor = 0;
                }
            } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                if (++obj->cursor > obj->techniques - 1) {
                    obj->cursor = obj->techniques - 1;
                }
            }
            if (old != obj->cursor) {
                sound_module.play(0x8004513E);
                stgdglab_techs_show(obj, data, 1);
                data->cursor->set_pos(data->cursor, 0x4C, obj->cursor * 17 + 0x58);
            } else {
                i = obj->form_record.techniques[obj->cursor];
                if (PAD_PRESSED(0xD)) {
                    sound_module.play(0x8004503C);
                    if (i != 0) {
                        if (i & 0x4000) {
                            obj->base.step = 5;
                            obj->base.substep = 10;
                        } else if (i & 0x8000) {
                            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x39), 0xB);
                            obj->base.step = 30;
                        } else if (!(i & 0x2000)) {
                            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x39), 0xA);
                            obj->base.step = 30;
                        } else if (obj->set_count >= 3) {
                            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x39), 9);
                            obj->base.step = 30;
                        } else {
                            obj->base.step = 5;
                            obj->base.substep = 20;
                        }
                        data->cursor->set_palette(data->cursor, 7);
                        data->cursor->stop(data->cursor, 1);
                    }
                } else if (PAD_PRESSED(0xE)) {
                    sound_module.play(0x800450BD);
                    obj->base.set_step(obj, 40);
                    stgdglab_funcs.window_anim_start(&obj->anims[0], 0);
                    data->yes_no_cursor->show(data->yes_no_cursor, 0);
                    data->cursor->show(data->cursor, 0);
                    stgdglab_techs_show(obj, data, 0);
                }
            }
            break;
        case 5:
            stgdglab_funcs.window_anim_start(&obj->anims[1], 1);
            obj->base.step++;
            break;
        case 6:
            if (stgdglab_funcs.window_anim_update(&obj->anims[1])) {
                obj->base.step = obj->base.substep;
                obj->base.set_substep(obj, 0);
            }
            break;
        case 7:
            data->yes_no_cursor->show(data->yes_no_cursor, 0);
            stgdglab_funcs.window_anim_start(&obj->anims[1], 0);
            obj->base.step = 6;
            break;
        case 10:
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x39), 7);
            data->yes->set_text(data->yes, cdload_module.files.get_file(records_language + 0x39), 0xC);
            data->no->set_text(data->no, cdload_module.files.get_file(records_language + 0x39), 0xD);
            obj->question_cursor = 0;
            data->yes_no_cursor->show(data->yes_no_cursor, 1);
            data->yes_no_cursor->set_pos(data->yes_no_cursor, 0xB8, obj->question_cursor * 16 + 0x38);
            obj->base.step++;
            break;
        case 11:
            old = obj->question_cursor;
            if (PAD_PRESSED(4)) {
                obj->question_cursor = 0;
            } else if (PAD_PRESSED(6)) {
                obj->question_cursor = 1;
            }
            if (old != obj->question_cursor) {
                sound_module.play(0x8004513E);
                data->yes_no_cursor->set_pos(data->yes_no_cursor, 0xB8, obj->question_cursor * 16 + 0x38);
            }
            if (PAD_PRESSED(0xD)) {
                sound_module.play(0x8004503C);
                if (obj->question_cursor == 0) {
                    obj->form_record.techniques[obj->cursor] &= ~0x4000;
                }
            } else if (PAD_PRESSED(0xE)) {
                sound_module.play(0x800450BD);
            } else {
                break;
            }
            data->yes->set_visible(data->yes, 0);
            data->no->set_visible(data->no, 0);
            data->cursor->stop(data->cursor, 0);
            obj->base.set_step(obj, 7);
            obj->base.substep = 0;
            break;
        case 20:
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x39), 8);
            data->yes->set_text(data->yes, cdload_module.files.get_file(records_language + 0x39), 0xC);
            data->no->set_text(data->no, cdload_module.files.get_file(records_language + 0x39), 0xD);
            obj->question_cursor = 0;
            data->yes_no_cursor->show(data->yes_no_cursor, 1);
            data->yes_no_cursor->set_pos(data->yes_no_cursor, 0xB8, obj->question_cursor * 16 + 0x38);
            obj->base.step++;
            break;
        case 21:
            old = obj->question_cursor;
            if (PAD_PRESSED(4)) {
                obj->question_cursor = 0;
            } else if (PAD_PRESSED(6)) {
                obj->question_cursor = 1;
            }
            if (old != obj->question_cursor) {
                sound_module.play(0x8004513E);
                data->yes_no_cursor->set_pos(data->yes_no_cursor, 0xB8, obj->question_cursor * 16 + 0x38);
            }
            if (PAD_PRESSED(0xD)) {
                sound_module.play(0x8004503C);
                if (obj->question_cursor == 0) {
                    obj->form_record.techniques[obj->cursor] |= 0x4000;
                }
            } else if (PAD_PRESSED(0xE)) {
                sound_module.play(0x800450BD);
            } else {
                break;
            }
            data->yes->set_visible(data->yes, 0);
            data->no->set_visible(data->no, 0);
            data->cursor->stop(data->cursor, 0);
            obj->base.set_step(obj, 7);
            obj->base.substep = 0;
            break;
        case 30:
            if (PAD_PRESSED(0xD)) {
                sound_module.play(0x8004503C);
                data->cursor->stop(data->cursor, 0);
                obj->base.set_step(obj, 0);
            }
            break;
        case 40:
            if (stgdglab_funcs.window_anim_update(&obj->anims[0])) {
                gamestate_data.funcs.put_form(obj->digimon, obj->set_forms[obj->set_form], &obj->form_record);
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
            break;
        }
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        if (obj->anims[0].level != 0x1000) {
            spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x7D);
        } else {
            spr.set_vram_pos(0x140, 0);
            for (j = 0, y = 0x58; j < 6; y += 17, j++) {
                tech = obj->form_record.techniques[j];
                if (obj->form_record.techniques[j] != 0) {
                    tech &= 0x1FFF;
                    spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_techniques[tech - 1].element + 0x37, 0x5D, y);
                }
            }
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x28, 0x46, 0x41);
        if (obj->anims[0].level != 0x1000) {
            spr.set_pivot(0, 0xD0);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x2B, 0, 0xBF);
        if (obj->anims[0].level != 0x1000) {
            spr.set_pivot(0x140, 0x1F);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x24, 0x92, 0x13);
        if (obj->anims[1].level != 0) {
            if (obj->anims[1].level != 0x1000) {
                spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
                spr.set_pivot(0x140, 0x46);
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
            }
            spr.set_layer_id(obj->layer_id, 1);
            spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x23, 0xAE, 0x32);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StgdglabTechs *stgdglab_techs_create(s32 digimon, s32 slot) {
    StgdglabTechs *obj = object_new(stgdglab_techs_update, sizeof(StgdglabTechs), sizeof(StgdglabTechsData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->digimon = digimon;
    obj->set_form = slot;
    return obj;
}
