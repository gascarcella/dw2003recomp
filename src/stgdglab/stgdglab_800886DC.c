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

/* STGDGLAB.PRO: the lab's menu (the main object's first child): a party member's stats and the lab's entries;
 * the entry chosen (`entry`) starts one of the screens of stgdglab_screens. */

/* The menu's data block (0x4C bytes): its windows. */
typedef struct StgdglabMenuData {
    /* 0x00 */ MessageWindow *help_line; /* help line */
    /* 0x04 */ MessageWindow *entries[3]; /* the entries */
    /* 0x10 */ MessageWindow *title;  /* title */
    /* 0x14 */ MessageWindow *stat_names[5]; /* stat names */
    /* 0x28 */ MessageWindow *stat_values[5]; /* stat values */
    /* 0x3C */ MessageWindow *name;      /* the Digimon's name */
    /* 0x40 */ MessageWindow *details_hint;
    /* 0x44 */ MessageCursor *cursor; /* the entries' cursor */
    /* 0x48 */ StgdglabForms *forms;  /* the party member's forms (stgdglab_forms_create) */
} StgdglabMenuData; /* size 0x4C */

s32 stgdglab_menu_stats[5] = { 0, 2, 3, 4, 5 };

void stgdglab_menu_create_windows(StgdglabMenu *obj, StgdglabMenuData *data);
void stgdglab_menu_run(StgdglabMenu *obj, StgdglabMenuData *data);
void stgdglab_menu_draw(StgdglabMenu *obj, StgdglabMenuData *data);

/* StgdglabMenu.open: opens the menu again (its windows come back). */
void stgdglab_menu_open(StgdglabMenu *obj) {
    obj->base.state = OBJECT_STATE_DONE;
    obj->base.timer = 0;
    stgdglab_funcs.window_anim_start(&obj->anims[3], 1);
}

/* StgdglabMenu.close: closes the menu and hides its windows. */
void stgdglab_menu_close(StgdglabMenu *obj) {
    StgdglabMenuData *data = (StgdglabMenuData *)obj->base.children;
    s32 i;

    obj->base.state = OBJECT_STATE_DONE;
    obj->base.timer = 1;
    stgdglab_funcs.window_anim_start(&obj->anims[3], 0);
    for (i = 0; i < 5; i++) {
        if (data->stat_names[i] != NULL) {
            data->stat_names[i]->set_visible(data->stat_names[i], 0);
        }
        if (data->stat_values[i] != NULL) {
            data->stat_values[i]->set_visible(data->stat_values[i], 0);
        }
    }
    if (data->name != NULL) {
        data->name->set_visible(data->name, 0);
    }
    for (i = 4; i >= 0; i--) {
        obj->frames[i] = 0;
    }
}

/* Creates the menu's windows and shows the stats of the party member main->member (none: a dash). */
void stgdglab_menu_create_windows(StgdglabMenu *obj, StgdglabMenuData *data) {
    GamestateStats stats;
    s32 i;
    s32 digimon;
    StgdglabWindowPos *pos;
    MessageWindow *win;

    for (i = 0; i < 5; i++) {
        pos = &stgdglab_funcs.menu_layout[i];
        if (data->stat_names[i] == NULL) {
            data->stat_names[i] = win = message_create_window(obj->layer_id, 3, pos->x, pos->y);
            win->set_ot_depth(win, obj->ot_depth - 1);
        }
        data->stat_names[i]->set_text(data->stat_names[i], cdload_module.files.get_file(records_language + 0x39), pos->message);
    }
    digimon = gamestate_data.funcs.get_party_member(obj->main->member);
    if (digimon >= 0) {
        gamestate_data.funcs.get_stats(digimon, &stats);
    }
    for (i = 0; i < 5; i++) {
        pos = &stgdglab_funcs.menu_layout[i + 5];
        if (data->stat_values[i] == NULL) {
            data->stat_values[i] = win = message_create_window(obj->layer_id, 3, pos->x, pos->y);
            win->set_ot_depth(win, obj->ot_depth - 1);
        }
        if (digimon < 0) {
            if (i == 0) {
                data->stat_values[i]->set_text(data->stat_values[i], cdload_module.files.get_file(records_language + 0x39), 0x1C);
            } else {
                data->stat_values[i]->set_text(data->stat_values[i], cdload_module.files.get_file(records_language + 0x39), 0x12);
            }
        } else {
            data->stat_values[i]->set_line_number(data->stat_values[i], 0, stats.values[stgdglab_menu_stats[i]]);
        }
        data->stat_values[i]->measure(data->stat_values[i], 1);
    }
    pos = &stgdglab_funcs.menu_layout[10];
    if (data->name == NULL) {
        data->name = win = message_create_window(obj->layer_id, 1, pos->x, pos->y);
        win->set_ot_depth(win, obj->ot_depth - 1);
    }
    if (digimon < 0) {
        data->name->set_text(data->name, cdload_module.files.get_file(records_language + 0x39), 0x1B);
        if (data->details_hint != NULL) {
            data->details_hint->set_palette(data->details_hint, 7);
        }
    } else {
        data->name->set_text(data->name, gamestate_data.funcs.get_record(digimon)->name, -1);
        if (data->details_hint != NULL) {
            data->details_hint->set_palette(data->details_hint, 0);
        }
    }
    obj->frames[3] = 0;
}

/* The menu's steps (base.step): open the frames and the entries (0-2), choose an entry (3), close (4-5); then
 * the party member view (10-19) and its sub-screen (30-31). */
void stgdglab_menu_run(StgdglabMenu *obj, StgdglabMenuData *data) {
    s32 i;
    s32 j;
    s32 n;
    s32 old;
    s32 opened;
    MessageWindow *win;
    MessageWindow **windows;

    switch (obj->base.step) {
    case 0:
    default:
        stgdglab_funcs.window_anim_start(&obj->anims[0], 1);
        stgdglab_funcs.window_anim_start(&obj->anims[2], 1);
        obj->base.step++;
        break;
    case 1:
        opened = 0;
        if (stgdglab_funcs.window_anim_update(&obj->anims[0])) {
            if (data->help_line == NULL) {
                data->help_line = win = message_create_window(obj->layer_id, 1, 0xAE, 0x15);
                win->set_ot_depth(win, obj->ot_depth - 1);
            }
            data->help_line->set_pos(data->help_line, 0xAE, 0x15);
            opened = 1;
            data->help_line->set_text(data->help_line, cdload_module.files.get_file(records_language + 0x39), opened);
        }
        if (stgdglab_funcs.window_anim_update(&obj->anims[2])) {
            if (data->title == NULL) {
                data->title = win = message_create_window(obj->layer_id, 1, 0xD3, 0xCC);
                win->set_ot_depth(win, obj->ot_depth - 1);
            }
            data->title->set_pos(data->title, 0xD3, 0xCC);
            opened++;
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x39), 5);
        }
        if (opened == 2) {
            stgdglab_funcs.window_anim_start(&obj->anims[1], 1);
            obj->base.step++;
        }
        break;
    case 2:
        if (stgdglab_funcs.window_anim_update(&obj->anims[1])) {
            for (i = 0; i < 3; i++) {
                if (data->entries[i] == NULL) {
                    data->entries[i] = win = message_create_window(obj->layer_id, 1, 0xA7, i * 14 + 0x31);
                    win->set_ot_depth(win, obj->ot_depth - 1);
                }
                data->entries[i]->set_text(data->entries[i], cdload_module.files.get_file(records_language + 0x39), i + 2);
            }
            if (data->cursor == NULL) {
                data->cursor = message_create_cursor(obj->layer_id, 1, 0x9A, obj->entry * 14 + 0x31);
            }
            data->cursor->show(data->cursor, 1);
            obj->base.step++;
        }
        break;
    case 3:
        old = obj->entry;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            if (--obj->entry < 0) {
                obj->entry = 0;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            if (++obj->entry >= 3) {
                obj->entry = 2;
            }
        }
        if (old != obj->entry) {
            sound_module.play(0x8004513E);
            data->cursor->set_pos(data->cursor, 0x9A, obj->entry * 14 + 0x31);
        }
        if (PAD_PRESSED(0xD)) {
            sound_module.play(0x8004503C);
            obj->base.substep = 0;
            obj->base.step++;
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->base.substep = 1;
            obj->base.step++;
            obj->main->fade_out(obj->main);
        }
        break;
    case 4:
        windows = (MessageWindow **)obj->base.children;
        for (j = 0; j < obj->base.child_count - 2; j++, windows++) {
            if (*windows != NULL) {
                (*windows)->set_visible(*windows, 0);
            }
        }
        data->cursor->show(data->cursor, 0);
        stgdglab_funcs.window_anim_start(&obj->anims[0], 0);
        stgdglab_funcs.window_anim_start(&obj->anims[1], 0);
        stgdglab_funcs.window_anim_start(&obj->anims[2], 0);
        obj->base.step++;
        break;
    case 5:
        for (n = 0, i = 0; n < 3; n++) {
            i += stgdglab_funcs.window_anim_update(&obj->anims[n]);
            if (i == 3) {
                if (obj->base.substep == 0) {
                    obj->base.set_step(obj, 10);
                    stgdglab_funcs.window_anim_start(&obj->anims[3], 1);
                    obj->main->member = 0;
                    if (obj->entry == 0) {
                        obj->member_choices = obj->main->slot_count;
                    } else {
                        obj->member_choices = obj->main->member_count;
                    }
                } else {
                    obj->base.set_state(obj, OBJECT_STATE_END);
                }
            }
        }
        break;
    case 10:
        stgdglab_funcs.window_anim_start(&obj->anims[4], 1);
        stgdglab_funcs.window_anim_start(&obj->anims[5], 1);
        stgdglab_funcs.window_anim_start(&obj->anims[6], 1);
        stgdglab_funcs.window_anim_start(&obj->anims[7], 1);
        if (obj->entry == 0) {
            stgdglab_funcs.window_anim_start(&obj->anims[8], 1);
        }
        obj->base.step++;
        break;
    case 11:
        stgdglab_funcs.window_anim_update(&obj->anims[3]);
        if (stgdglab_funcs.window_anim_update(&obj->anims[4])) {
            stgdglab_menu_create_windows(obj, data);
            if (data->title == NULL) {
                data->title = win = message_create_window(obj->layer_id, 1, 0xA1, 0x17);
                win->set_ot_depth(win, obj->ot_depth - 1);
            }
            data->title->set_pos(data->title, 0xA1, 0x17);
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x39), 6);
            obj->base.step++;
        }
        break;
    case 12:
        if (obj->entry == 0 && stgdglab_funcs.window_anim_update(&obj->anims[8])) {
            if (data->details_hint == NULL) {
                data->details_hint = win = message_create_window(obj->layer_id, 1, 0xF, 0x55);
                win->set_ot_depth(win, obj->ot_depth - 1);
            }
            data->details_hint->set_text(data->details_hint, cdload_module.files.get_file(records_language + 0x39), 0x14);
        }
        stgdglab_funcs.window_anim_update(&obj->anims[5]);
        if (stgdglab_funcs.window_anim_update(&obj->anims[6])) {
            if (data->help_line == NULL) {
                data->help_line = message_create_window(obj->layer_id, 1, 0xAE, 0x49);
            }
            data->help_line->set_pos(data->help_line, 0xAE, 0x49);
            data->help_line->set_text(data->help_line, cdload_module.files.get_file(records_language + 0x39), obj->entry + 0x18);
            obj->base.step++;
        }
        break;
    case 14:
        if (stgdglab_funcs.window_anim_update(&obj->anims[7])) {
            obj->base.step++;
        }
        break;
    case 15:
        old = obj->main->member;
        if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
            if (--obj->main->member < 0) {
                obj->main->member = obj->member_choices - 1;
            }
        } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
            if (++obj->main->member > obj->member_choices - 1) {
                obj->main->member = 0;
            }
        }
        if (old != obj->main->member) {
            sound_module.play(0x4001B);
            stgdglab_menu_create_windows(obj, data);
        } else if (PAD_PRESSED(0xD)) {
            sound_module.play(0x4001C);
            obj->base.substep = 1;
            obj->base.step++;
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->base.substep = 0;
            obj->base.step++;
        } else if (PAD_PRESSED(0xC) && obj->entry == 0 &&
                   gamestate_data.funcs.get_party_member(obj->main->member) >= 0) {
            obj->base.substep = 2;
            obj->base.step++;
            sound_module.play(0x4001C);
        }
        break;
    case 16:
        stgdglab_funcs.window_anim_start(&obj->anims[7], 0);
        obj->base.step++;
        break;
    case 17:
        if (stgdglab_funcs.window_anim_update(&obj->anims[7])) {
            if (obj->base.substep == 0) {
                stgdglab_funcs.window_anim_start(&obj->anims[3], 0);
                for (i = 0; i < 5; i++) {
                    if (data->stat_names[i] != NULL) {
                        data->stat_names[i]->set_visible(data->stat_names[i], 0);
                    }
                    if (data->stat_values[i] != NULL) {
                        data->stat_values[i]->set_visible(data->stat_values[i], 0);
                    }
                }
                if (data->name != NULL) {
                    data->name->set_visible(data->name, 0);
                }
            }
            stgdglab_funcs.window_anim_start(&obj->anims[4], 0);
            stgdglab_funcs.window_anim_start(&obj->anims[5], 0);
            stgdglab_funcs.window_anim_start(&obj->anims[6], 0);
            if (obj->entry == 0) {
                stgdglab_funcs.window_anim_start(&obj->anims[8], 0);
            }
            if (data->help_line != NULL) {
                data->help_line->set_visible(data->help_line, 0);
            }
            if (data->title != NULL) {
                data->title->set_visible(data->title, 0);
            }
            if (data->details_hint != NULL) {
                data->details_hint->set_visible(data->details_hint, 0);
            }
            obj->base.step++;
        }
        break;
    case 18:
        n = 0;
        for (j = obj->base.substep != 0; j < 6; j++) {
            n += stgdglab_funcs.window_anim_update(&obj->anims[j + 3]);
        }
        if (obj->base.substep == 1) {
            if (n == 5) {
                obj->base.next_step(obj);
                obj->chosen = 1;
            }
        } else if (obj->base.substep == 2) {
            if (n == 5) {
                obj->base.set_step(obj, 30);
            }
        } else if (n == 6) {
            obj->base.set_step(obj, 0);
        }
        break;
    case 19:
        if (obj->chosen == 0) {
            obj->base.set_step(obj, 10);
            if (obj->anims[3].level == 0) {
                stgdglab_funcs.window_anim_start(&obj->anims[3], 1);
            }
        }
        break;
    case 30:
        if (data->forms == NULL) {
            data->forms = stgdglab_forms_create(gamestate_data.funcs.get_party_member(obj->main->member), 1, 1);
        }
        obj->base.step++;
        break;
    case 31:
        if (data->forms == NULL) {
            obj->base.set_step(obj, 10);
        }
        break;
    }
}

/* Draws the menu: while it opens (base.step < 10) its three frames, then the party members' sprites (animated),
 * the cursor, the shown member and the other frames, each scaled by its window_anim. */
void stgdglab_menu_draw(StgdglabMenu *obj, StgdglabMenuData *data) {
    Sprite spr;
    s32 digimon;
    StgdglabAnim *anim;
    s32 i;
    s32 member;

    sprite_init(&spr);
    if (obj->base.step < 10) {
        spr.set_vram_pos(0x280, 0x100);
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        if (obj->anims[0].level != 0) {
            if (obj->anims[0].level != 0x1000) {
                spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
                spr.set_pivot(0x140, 0x15);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x20, 0x92, 0xF);
        }
        if (obj->anims[1].level != 0) {
            if (obj->anims[1].level != 0x1000) {
                spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
                spr.set_pivot(0x140, 0x45);
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x21, 0x92, 0x2A);
        }
        if (obj->anims[2].level != 0) {
            if (obj->anims[2].level != 0x1000) {
                spr.set_scale(obj->anims[2].level, 0x1000, 0x1000);
                spr.set_pivot(0x140, 0xD2);
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x24, 0xC6, 0xC4);
        }
        return;
    }
    digimon = gamestate_data.funcs.get_party_digimon(obj->main->member);
    if (gfx_module.funcs.get_time() - obj->anim_time >= 12) {
        obj->anim_time = gfx_module.funcs.get_time();
        if (digimon >= 0) {
            anim = &stgdglab_funcs.anims[digimon];
            if (anim->sprite[++obj->frames[3]] == -1 || obj->frames[3] >= 7) {
                obj->frames[3] = 0;
            }
        }
        for (i = 0; i < obj->main->slot_count; i++) {
            member = gamestate_data.funcs.get_party_digimon(i);
            if (member >= 0) {
                anim = &stgdglab_funcs.anims[member];
                if (anim->sprite[++obj->frames[i]] == -1) {
                    obj->frames[i] = 0;
                }
            }
        }
    }
    if (gfx_module.funcs.get_time() - obj->blink_time >= 10) {
        obj->blink_time = gfx_module.funcs.get_time();
        if (++obj->frames[4] >= 4) {
            obj->frames[4] = 0;
        }
    }
    spr.set_vram_pos(0x280, 0x100);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->anims[7].level != 0) {
        if (obj->anims[7].level != 0x1000) {
            spr.set_scale(obj->anims[7].level, 0x1000, 0x1000);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
            spr.set_vram_pos(0x140, 0);
            spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
            spr.set_palette(obj->frames[4]);
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xD, obj->main->member * 0x30 + 0xA5, 0x65);
            spr.set_layer_id(obj->layer_id, obj->ot_depth);
            spr.set_palette(0);
        }
        spr.set_vram_pos(0x280, 0x100);
        for (i = 0; i < obj->main->slot_count; i++) {
            member = gamestate_data.funcs.get_party_digimon(i);
            if (member >= 0) {
                if (obj->anims[7].level != 0x1000) {
                    spr.set_pivot(i * 0x30 + 0xB4, 0x8A);
                }
                anim = &stgdglab_funcs.anims[member];
                spr.draw(cdload_module.get_subfile_by_id(0x02C50000), anim->sprite[obj->frames[i]], i * 0x30 + 0xA5, 0x81);
            }
        }
    }
    if (obj->anims[6].level != 0) {
        if (obj->anims[6].level != 0x1000) {
            spr.set_scale(obj->anims[6].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x8A);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.set_vram_pos(0x140, 0);
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x10, 0x90, 0x5F);
        spr.set_vram_pos(0x140, 0);
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x11, 0x90, 0x5F);
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x27, 0x90, 0x5F);
    }
    if (obj->anims[3].level != 0) {
        if (obj->anims[3].level != 0x1000) {
            spr.set_scale(obj->anims[3].level, 0x1000, 0x1000);
            spr.set_pivot(0, 0x2F);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        if (digimon >= 0) {
            anim = &stgdglab_funcs.anims[digimon];
            spr.draw(cdload_module.get_subfile_by_id(0x02C50000), anim->sprite[obj->frames[3]], 0x10, 0x16);
        }
        spr.set_vram_pos(0x140, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xE, 0, 0x13);
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x1E, 0, 0x13);
    }
    spr.set_vram_pos(0x280, 0x100);
    if (obj->anims[4].level != 0) {
        if (obj->anims[4].level != 0x1000) {
            spr.set_scale(obj->anims[4].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x1D);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x25, 0x8F, 0xF);
    }
    if (obj->anims[5].level != 0) {
        if (obj->anims[5].level != 0x1000) {
            spr.set_scale(obj->anims[5].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x4E);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x20, 0x92, 0x43);
    }
    if (obj->anims[8].level != 0) {
        if (obj->anims[8].level != 0x1000) {
            spr.set_scale(obj->anims[8].level, 0x1000, 0x1000);
            spr.set_pivot(0, 0x5A);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x26, 0, 0x4D);
    }
}

void stgdglab_menu_update(StgdglabMenu *obj, StgdglabMenuData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anims[0].duration = 10;
        obj->anims[1].duration = 10;
        obj->anims[2].duration = 8;
        obj->anims[3].duration = 10;
        obj->anims[4].duration = 10;
        obj->anims[5].duration = 10;
        obj->anims[6].duration = 10;
        obj->anims[7].duration = 8;
        obj->anims[8].duration = 8;
        break;
    case OBJECT_STATE_RUN:
        stgdglab_menu_run(obj, data);
        stgdglab_menu_draw(obj, data);
        break;
    case OBJECT_STATE_DONE:
        if (stgdglab_funcs.window_anim_update(&obj->anims[3])) {
            obj->base.state = OBJECT_STATE_RUN;
            if (obj->base.timer == 0) {
                stgdglab_menu_create_windows(obj, data);
            }
        }
        stgdglab_menu_draw(obj, data);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

StgdglabMenu *stgdglab_menu_create(StgdglabMain *main) {
    StgdglabMenu *obj = object_new(stgdglab_menu_update, sizeof(StgdglabMenu), sizeof(StgdglabMenuData));

    obj->open = stgdglab_menu_open;
    obj->close = stgdglab_menu_close;
    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->main = main;
    return obj;
}
