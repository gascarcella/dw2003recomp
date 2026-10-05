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

/* STGDGLAB.PRO: the party change (stgdglab_menu's entry 0): the lab's Digimon that aren't in the party
 * (others), each with its stats, to put in the party member's place. */

/* The party change (size 0x108). */
struct StgdglabSwap {
    /* 0x000 */ Object base;
    /* 0x050 */ StgdglabMain *main;
    /* 0x054 */ s32 layer_id; /* layer */
    /* 0x058 */ s32 ot_depth; /* ordering table entry */
    /* 0x05C */ s32 stats_down; /* the stats panel is moved down by 0x7A lines when set */
    /* 0x060 */ s32 shown;  /* the Digimon shown (index into others) */
    /* 0x064 */ WindowAnim anims[5];
    /* 0x0B4 */ u8 unk_B4[0x10];
    /* 0x0C4 */ s32 anim_time; /* time of the last animation frame */
    /* 0x0C8 */ s32 blink_time; /* time of the last blink */
    /* 0x0CC */ s32 member_frames[3]; /* per party member: animation frame */
    /* 0x0D8 */ s32 shown_frame; /* animation frame of the Digimon shown */
    /* 0x0DC */ s32 blink;  /* blink, 0..3 */
    /* 0x0E0 */ s32 arrow_frame; /* arrow frame, 0..5 */
    /* 0x0E4 */ s32 others[8]; /* the Digimon that aren't in the party */
    /* 0x104 */ s32 other_count; /* how many */
}; /* StgdglabSwap, size 0x108 */

/* Its data block (0x38 bytes). */
typedef struct StgdglabSwapData {
    /* 0x00 */ MessageWindow *title;     /* title */
    /* 0x04 */ MessageWindow *stat_names[5]; /* stat names */
    /* 0x18 */ MessageWindow *stat_values[5]; /* stat values */
    /* 0x2C */ MessageWindow *name;      /* the Digimon's name */
    /* 0x30 */ MessageWindow *help_line; /* help line */
    /* 0x34 */ StgdglabForms *forms;   /* the Digimon's forms */
} StgdglabSwapData; /* size 0x38 */

s32 stgdglab_swap_stats[5] = { 0, 2, 3, 4, 5 };

void stgdglab_swap_run(StgdglabSwap *obj, StgdglabSwapData *data);
void stgdglab_swap_draw(StgdglabSwap *obj, StgdglabSwapData *data);

/* Creates the stats panel's windows and shows the stats of the Digimon others[shown] (none: a dash). */
void stgdglab_swap_show(StgdglabSwap *obj, StgdglabSwapData *data) {
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
        data->stat_names[i]->set_pos(data->stat_names[i], pos->x, pos->y + obj->stats_down * 0x7A);
    }
    digimon = obj->others[obj->shown];
    if (digimon >= 0) {
        gamestate_data.funcs.get_stats(digimon, &stats);
    }
    for (i = 0; i < 5; i++) {
        pos = &stgdglab_funcs.menu_layout[i + 5];
        if (data->stat_values[i] == NULL) {
            data->stat_values[i] = win = message_create_window(obj->layer_id, 3, pos->x, pos->y);
            win->set_ot_depth(win, obj->ot_depth - 1);
        }
        data->stat_values[i]->set_pos(data->stat_values[i], pos->x, pos->y + obj->stats_down * 0x7A);
        if (digimon < 0) {
            if (i == 0) {
                data->stat_values[i]->set_text(data->stat_values[i], cdload_module.files.get_file(records_language + 0x39), 0x1C);
            } else {
                data->stat_values[i]->set_text(data->stat_values[i], cdload_module.files.get_file(records_language + 0x39), 0x12);
            }
        } else {
            data->stat_values[i]->set_line_number(data->stat_values[i], 0, stats.values[stgdglab_swap_stats[i]]);
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
        data->help_line->set_palette(data->help_line, 7);
    } else {
        data->name->set_text(data->name, gamestate_data.funcs.get_record(digimon)->name, -1);
        data->help_line->set_palette(data->help_line, 0);
    }
    data->name->set_pos(data->name, pos->x, pos->y + obj->stats_down * 0x7A);
    obj->shown_frame = 0;
}

/* Hides the windows. */
void stgdglab_swap_hide(StgdglabSwap *obj) {
    s32 i;
    MessageWindow **windows = (MessageWindow **)obj->base.children;

    for (i = 0; i < obj->base.child_count - 1; i++, windows++) {
        if (*windows != NULL) {
            (*windows)->set_visible(*windows, 0);
        }
    }
}

/* Draws the screen: the Digimon shown (animated), the party members, the frames and the arrows, each scaled by
 * its window_anim. */
void stgdglab_swap_draw(StgdglabSwap *obj, StgdglabSwapData *data) {
    Sprite spr;
    s32 digimon;
    s32 member;
    s32 i;
    StgdglabAnim *anim;

    digimon = obj->others[obj->shown];
    if (gfx_module.funcs.get_time() - obj->anim_time >= 12) {
        obj->anim_time = gfx_module.funcs.get_time();
        if (digimon >= 0) {
            anim = &stgdglab_funcs.anims[digimon];
            if (anim->sprite[++obj->shown_frame] == -1 || obj->shown_frame >= 7) {
                obj->shown_frame = 0;
            }
        }
        for (i = 0; i < obj->main->slot_count; i++) {
            member = gamestate_data.funcs.get_party_digimon(i);
            if (member >= 0) {
                obj->member_frames[i]++;
                anim = &stgdglab_funcs.anims[member];
                if (anim->sprite[obj->member_frames[i]] == -1) {
                    obj->member_frames[i] = 0;
                }
            }
        }
    }
    if (gfx_module.funcs.get_time() - obj->blink_time >= 10) {
        obj->blink_time = gfx_module.funcs.get_time();
        if (++obj->blink >= 4) {
            obj->blink = 0;
        }
        if (++obj->arrow_frame >= 6) {
            obj->arrow_frame = 0;
        }
    }
    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0x100);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->anims[0].level != 0) {
        if (obj->anims[0].level != 0x1000) {
            spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
            spr.set_pivot(0, obj->stats_down * 0x7A + 0x2F);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        if (digimon >= 0) {
            anim = &stgdglab_funcs.anims[digimon];
            spr.draw(cdload_module.get_subfile_by_id(0x02C50000), anim->sprite[obj->shown_frame], 0x10, obj->stats_down * 0x7A + 0x16);
        }
        spr.set_vram_pos(0x140, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xE, 0, obj->stats_down * 0x7A + 0x13);
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x1E, 0, obj->stats_down * 0x7A + 0x13);
    }
    if (obj->anims[1].level != 0) {
        if (obj->anims[1].level != 0x1000) {
            spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
            spr.set_pivot(0, 0xD1);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x26, 0, 0xC4);
    }
    if (obj->anims[2].level != 0) {
        if (obj->anims[2].level != 0x1000) {
            spr.set_scale(obj->anims[2].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x15);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x20, 0x92, 0xF);
    }
    if (obj->anims[4].level != 0) {
        if (obj->anims[4].level != 0x1000) {
            spr.set_scale(obj->anims[4].level, 0x1000, 0x1000);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
            if (obj->other_count >= 2) {
                spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
                spr.set_palette(obj->blink);
                spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x31, 0xB8, 0xB6);
                spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x32, 0x100, 0xB6);
                spr.set_layer_id(obj->layer_id, obj->ot_depth);
                spr.set_palette(0);
            }
        }
        for (i = 0; i < obj->main->slot_count; i++) {
            member = gamestate_data.funcs.get_party_digimon(i);
            if (member >= 0) {
                if (obj->anims[4].level != 0x1000) {
                    spr.set_pivot(i * 0x30 + 0xB4, 0x4E);
                }
                anim = &stgdglab_funcs.anims[member];
                spr.draw(cdload_module.get_subfile_by_id(0x02C50000), anim->sprite[obj->member_frames[i]], i * 0x30 + 0xA5, 0x47);
            }
        }
        if (digimon >= 0) {
            if (obj->anims[4].level != 0x1000) {
                spr.set_pivot(0xE4, 0xB5);
            }
            anim = &stgdglab_funcs.anims[digimon];
            spr.draw(cdload_module.get_subfile_by_id(0x02C50000), anim->sprite[obj->shown_frame], 0xD5, 0xAE);
        }
    }
    if (obj->anims[3].level != 0) {
        if (obj->anims[3].level != 0x1000) {
            spr.set_scale(obj->anims[3].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x85);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x33, 0xAD, 0x66);
        spr.set_palette(obj->blink);
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), obj->main->member + 0x38, 0xA5, 0x41);
        spr.set_palette(0);
        spr.set_vram_pos(0x140, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x10, 0x90, 0x25);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xB, 0xD5, 0x92);
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x11, 0x90, 0x25);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xC, 0xD5, 0x92);
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x2D, 0x90, 0x2A);
        if (obj->anims[3].level != 0x1000) {
            spr.set_pivot(0x48, 0x69);
        }
        spr.set_palette(obj->arrow_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x3B, 0x33, 0x4A);
    }
}

/* The screen's steps (base.step): open (0-3), choose a Digimon (4), close (5-8: base.substep 1 puts it in the party, 2
 * shows its forms, 0 leaves), its forms (9-11). */
void stgdglab_swap_run(StgdglabSwap *obj, StgdglabSwapData *data) {
    s32 old;

    switch (obj->base.step) {
    case 0:
    default:
        stgdglab_funcs.window_anim_start(&obj->anims[1], 1);
        stgdglab_funcs.window_anim_start(&obj->anims[2], 1);
        obj->base.step++;
        break;
    case 1:
        stgdglab_funcs.window_anim_update(&obj->anims[1]);
        if (stgdglab_funcs.window_anim_update(&obj->anims[2])) {
            stgdglab_funcs.window_anim_start(&obj->anims[0], 1);
            stgdglab_funcs.window_anim_start(&obj->anims[3], 1);
            if (data->title == NULL) {
                data->title = message_create_window(obj->layer_id, 1, 0xAE, 0x15);
            }
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x39), 0x1D);
            if (data->help_line == NULL) {
                data->help_line = message_create_window(obj->layer_id, 1, 0xF, 0xCC);
            }
            data->help_line->set_text(data->help_line, cdload_module.files.get_file(records_language + 0x39), 0x14);
            if (obj->other_count == 0) {
                data->help_line->set_palette(data->help_line, 7);
            } else {
                data->help_line->set_palette(data->help_line, 0);
            }
            obj->base.step++;
        }
        break;
    case 2:
        stgdglab_funcs.window_anim_update(&obj->anims[0]);
        if (stgdglab_funcs.window_anim_update(&obj->anims[3])) {
            stgdglab_funcs.window_anim_start(&obj->anims[4], 1);
            stgdglab_swap_show(obj, data);
            obj->base.step++;
        }
        break;
    case 3:
        if (stgdglab_funcs.window_anim_update(&obj->anims[4])) {
            obj->base.step++;
        }
        break;
    case 4:
        old = obj->shown;
        if (obj->other_count >= 2) {
            if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
                if (--obj->shown < 0) {
                    obj->shown = obj->other_count - 1;
                }
            } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
                if (++obj->shown > obj->other_count - 1) {
                    obj->shown = 0;
                }
            }
        }
        if (old != obj->shown) {
            sound_module.play(0x4001B);
            stgdglab_swap_show(obj, data);
        } else if (PAD_PRESSED(0xC)) {
            if (obj->others[obj->shown] >= 0) {
                obj->base.substep = 2;
                obj->base.step++;
                sound_module.play(0x4001C);
            }
        } else if (PAD_PRESSED(0xD)) {
            if (gamestate_data.funcs.get_party_member(obj->main->member) >= 0 || obj->others[obj->shown] >= 0) {
                obj->base.substep = 1;
                obj->base.step++;
                sound_module.play(0x4001C);
            } else {
                sound_module.play(0x800450BD);
            }
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->base.substep = 0;
            obj->base.step++;
        }
        break;
    case 5:
        stgdglab_funcs.window_anim_start(&obj->anims[4], 0);
        obj->base.step++;
        break;
    case 6:
        if (stgdglab_funcs.window_anim_update(&obj->anims[4])) {
            stgdglab_swap_hide(obj);
            stgdglab_funcs.window_anim_start(&obj->anims[0], 0);
            stgdglab_funcs.window_anim_start(&obj->anims[1], 0);
            stgdglab_funcs.window_anim_start(&obj->anims[2], 0);
            stgdglab_funcs.window_anim_start(&obj->anims[3], 0);
            if (obj->base.substep != 0) {
                obj->main->close_menu(obj->main);
            }
            obj->base.step++;
        }
        break;
    case 7:
        stgdglab_funcs.window_anim_update(&obj->anims[0]);
        stgdglab_funcs.window_anim_update(&obj->anims[1]);
        stgdglab_funcs.window_anim_update(&obj->anims[2]);
        if (stgdglab_funcs.window_anim_update(&obj->anims[3]) && obj->main->is_menu_running(obj->main) != 0) {
            obj->base.step++;
        }
        break;
    case 8:
        if (obj->base.substep == 1) {
            gamestate_data.party[obj->main->member] = obj->others[obj->shown];
            obj->main->pack_party(obj->main);
        }
        if (obj->base.substep == 2) {
            obj->stats_down = 0;
            stgdglab_funcs.window_anim_start(&obj->anims[0], 1);
            if (data->forms == NULL) {
                data->forms = stgdglab_forms_create(obj->others[obj->shown], 1, 1);
            }
            obj->base.step++;
        } else {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case 9:
        if (stgdglab_funcs.window_anim_update(&obj->anims[0])) {
            stgdglab_swap_show(obj, data);
            obj->base.step++;
        }
        break;
    case 10:
        if (data->forms->base.step >= 2 && PAD_PRESSED(0xE)) {
            stgdglab_funcs.window_anim_start(&obj->anims[0], 0);
            stgdglab_swap_hide(obj);
            obj->base.step++;
        }
        break;
    case 11:
        if (stgdglab_funcs.window_anim_update(&obj->anims[0]) && data->forms == NULL) {
            if (obj->main->is_menu_running(obj->main) != 0) {
                obj->main->open_menu(obj->main);
                obj->base.step++;
            }
            obj->stats_down = 1;
            obj->base.set_step(obj, 0);
        }
        break;
    }
}

void stgdglab_swap_update(StgdglabSwap *obj, StgdglabSwapData *data) {
    s32 party[3];
    s32 i;
    s32 j;
    s32 id;
    s32 ok;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anims[0].duration = 10;
        obj->anims[1].duration = 8;
        obj->anims[2].duration = 10;
        obj->anims[3].duration = 10;
        obj->anims[4].duration = 8;
        obj->stats_down = 1;
        for (i = 0; i < 3; i++) {
            party[i] = gamestate_data.funcs.get_party_digimon(i);
        }
        for (i = 0; i < 8; i++) {
            id = gamestate_data.digimon[i].joined - 3;
            obj->others[i] = -1;
            if (id >= 0) {
                for (j = 0, ok = 1; j < 3; j++) {
                    if (party[j] >= 0 && id == party[j]) {
                        ok = 0;
                    }
                }
            } else {
                ok = 0;
            }
            if (ok) {
                obj->others[obj->other_count++] = id;
            }
        }
        id = party[obj->main->member];
        if (obj->main->member_count >= 2 && id >= 0) {
            obj->other_count++;
        }
        break;
    case OBJECT_STATE_RUN:
        stgdglab_swap_run(obj, data);
        stgdglab_swap_draw(obj, data);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StgdglabSwap *stgdglab_swap_create(StgdglabMain *main) {
    StgdglabSwap *obj = object_new(stgdglab_swap_update, sizeof(StgdglabSwap), sizeof(StgdglabSwapData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->main = main;
    return obj;
}
