#include "common.h"
#include "object.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "fieldmenu.h"
#include "records.h"
#include "message.h"
#include "ststatus.h"

/* The party-order page (ststatus_create_order_page): pick two party members and swap their places. */

/* Its data block (0x8C bytes). */
typedef struct StstatusOrderPageData {
    /* 0x00 */ MessageWindow *title;  /* title */
    /* 0x04 */ StstatusPanelWindows panels[3]; /* per party member: its panel */
    /* 0x88 */ MessageWindow *message; /* message */
} StstatusOrderPageData; /* size 0x8C */

/* The party-order page (ststatus_create_order_page, size 0xF4). */
typedef struct StstatusOrderPage {
    /* 0x000 */ Object base;
    /* 0x050 */ s32 parent;
    /* 0x054 */ s32 layer_id; /* layer */
    /* 0x058 */ s32 ot_depth; /* ordering table depth */
    /* 0x05C */ s32 member_count; /* party members */
    /* 0x060 */ s32 frames[3]; /* per member: sprite frame */
    /* 0x06C */ s32 frame_time; /* time of the last frame */
    /* 0x070 */ s32 second_cursor_shown; /* the second cursor is shown */
    /* 0x074 */ s32 second_cursor_frame; /* its frame, 0..7 */
    /* 0x078 */ s32 second_cursor_time; /* time of its last frame */
    /* 0x07C */ s32 member_cursor_shown; /* the member cursor is shown */
    /* 0x080 */ s32 member_cursor; /* member cursor */
    /* 0x084 */ s32 first;  /* the first member picked */
    /* 0x088 */ s32 second; /* the second */
    /* 0x08C */ s32 member_cursor_frame; /* member cursor's frame, 0..7 */
    /* 0x090 */ s32 member_cursor_time; /* time of its last frame */
    /* 0x094 */ WindowAnim member_panels[3]; /* member panels */
    /* 0x0C4 */ WindowAnim bars[2];   /* title, bottom bar */
    /* 0x0E4 */ WindowAnim swap;   /* the swap */
} StstatusOrderPage; /* size 0xF4 */

s32 ststatus_order_panel_stats[5] = { 0, 2, 3, 4, 5 };

/* Creates the page's text windows. */
void ststatus_order_create_windows(StstatusOrderPage *obj, StstatusOrderPageData *data) {
    s32 i;
    s32 j;
    StstatusLayout *l;
    StstatusLayout *p;

    p = &ststatus_module.party_layout[11];
    data->title = message_create_window(obj->layer_id, 1, p->x, p->y);
    p = &ststatus_module.party_layout[12];
    data->message = message_create_window(obj->layer_id, 1, p->x, p->y);
    for (i = 0; i < 3; i++) {
        p = ststatus_module.party_layout;
        data->panels[i].name = message_create_window(obj->layer_id, 1, p->x, p->y + i * 0x2E);
        l = &ststatus_module.party_layout[1];
        for (j = 0; j < 5; j++, l++) {
            data->panels[i].labels[j] = message_create_window(obj->layer_id, 3, l->x, l->y + i * 0x2E);
        }
        l = &ststatus_module.party_layout[6];
        for (j = 0; j < 5; j++, l++) {
            data->panels[i].values[j] = message_create_window(obj->layer_id, 3, l->x, l->y + i * 0x2E);
        }
    }
}

/* Shows (fills in) or hides a member's panel. */
void ststatus_order_show_member(StstatusOrderPage *obj, StstatusOrderPageData *data, s32 member, s32 show) {
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
            win->set_line_number(win, 0, stats.values[ststatus_order_panel_stats[i]]);
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

/* Draws the member cursor, the members (the two being swapped squashed), the frames and the swap arrow. */
void ststatus_order_draw(StstatusOrderPage *obj) {
    Sprite spr;
    s32 i;
    s32 digimon;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (gfx_module.funcs.get_time() - obj->member_cursor_time >= 8) {
        obj->member_cursor_time = gfx_module.funcs.get_time();
        if (++obj->member_cursor_frame >= 8) {
            obj->member_cursor_frame = 0;
        }
    }
    spr.set_vram_pos(0x280, 0x100);
    if (obj->member_cursor_shown != 0) {
        if (obj->base.step == 6) {
            spr.set_palette(obj->member_cursor_frame);
            spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x1E, 0, obj->member_cursor * 0x2E + 0x11);
        } else if (obj->base.step == 7) {
            spr.set_palette(obj->member_cursor_frame);
            spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x1E, 0, obj->second * 0x2E + 0x11);
            spr.set_palette(8);
            spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x1E, 0, obj->first * 0x2E + 0x11);
        } else if (obj->base.step == 8) {
            spr.set_scale(0x1000, obj->swap.level, 0x1000);
            spr.set_pivot(0, obj->first * 0x2E + 0x25);
            spr.set_palette(8);
            spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x1E, 0, obj->first * 0x2E + 0x11);
            spr.set_pivot(0, obj->second * 0x2E + 0x25);
            spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x1E, 0, obj->second * 0x2E + 0x11);
        }
        spr.set_palette(0);
    }
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
    for (i = 0; i < obj->member_count; i++) {
        if (obj->member_panels[i].level != 0) {
            if (obj->member_panels[i].level != 0x1000) {
                spr.set_scale(obj->member_panels[i].level, obj->member_panels[i].level, 0x1000);
                spr.set_pivot(0x7C, i * 0x2E + 0x27);
            } else if (obj->base.step == 8 || obj->base.step == 9) {
                if (i == obj->first || i == obj->second) {
                    spr.set_scale(0x1000, obj->swap.level, 0x1000);
                    spr.set_pivot(0x7C, i * 0x2E + 0x27);
                } else {
                    spr.set_scale(0x1000, 0x1000, 0x1000);
                }
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
            }
            digimon = gamestate_data.funcs.get_party_member(i);
            spr.draw(cdload_module.get_subfile_by_id(0x04040000), ststatus_module.anims[digimon].frame[obj->frames[i]],
                       0x6B, i * 0x2E + 0x13);
        }
    }
    spr.set_vram_pos(0x140, 0);
    /* The frames: the same tests as the members' loop above (the plain frame is written twice; cross-jumping merges
     * the copies, but they count for the register allocation). */
    for (i = 0; i < obj->member_count; i++) {
        if (obj->member_panels[i].level != 0) {
            if (obj->member_panels[i].level != 0x1000) {
                spr.set_scale(obj->member_panels[i].level, 0x1000, 0x1000);
                spr.set_pivot(0, i * 0x2E + 0x25);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x15, 0, i * 0x2E + 0x11);
                spr.set_scale(obj->member_panels[i].level, obj->member_panels[i].level, 0x1000);
                spr.set_pivot(0x7C, i * 0x2E + 0x27);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x16, 0x67, i * 0x2E + 0x13);
                spr.set_scale(obj->member_panels[i].level, 0x1000, 0x1000);
                spr.set_pivot(0, i * 0x2E + 0x25);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x17, 0, i * 0x2E + 0x11);
            } else if (obj->base.step == 8 || obj->base.step == 9) {
                if (i == obj->first || i == obj->second) {
                    spr.set_scale(0x1000, obj->swap.level, 0x1000);
                    spr.set_pivot(0, i * 0x2E + 0x25);
                    spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x15, 0, i * 0x2E + 0x11);
                    spr.set_pivot(0x7C, i * 0x2E + 0x27);
                    spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x16, 0x67, i * 0x2E + 0x13);
                    spr.set_pivot(0, i * 0x2E + 0x25);
                    spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x17, 0, i * 0x2E + 0x11);
                } else {
                    spr.set_scale(0x1000, 0x1000, 0x1000);
                    spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x15, 0, i * 0x2E + 0x11);
                    spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x16, 0x67, i * 0x2E + 0x13);
                    spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x17, 0, i * 0x2E + 0x11);
                }
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x15, 0, i * 0x2E + 0x11);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x16, 0x67, i * 0x2E + 0x13);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x17, 0, i * 0x2E + 0x11);
            }
        }
    }
    if (obj->bars[0].level != 0) {
        if (obj->bars[0].level != 0x1000) {
            spr.set_scale(obj->bars[0].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x19);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x18, 0x22, 0xD);
    }
    if (obj->bars[1].level != 0) {
        if (obj->bars[1].level != 0x1000) {
            spr.set_scale(obj->bars[1].level, 0x1000, 0x1000);
            spr.set_pivot(0, 0xD3);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x20, 0, 0xC2);
    }
    if (obj->second_cursor_shown != 0) {
        if (gfx_module.funcs.get_time() - obj->second_cursor_time >= 11) {
            obj->second_cursor_time = gfx_module.funcs.get_time();
            if (++obj->second_cursor_frame >= 8) {
                obj->second_cursor_frame = 0;
            }
        }
        spr.set_layer_id(obj->layer_id, 0);
        spr.set_palette(obj->second_cursor_frame);
        switch (obj->first) {
        case 0:
        default:
            if (obj->second == 1) {
                spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x29, 0x93, 0x22);
            } else {
                spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x28, 0x93, 0x22);
            }
            break;
        case 1:
            if (obj->second == 0) {
                spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x29, 0x93, 0x22);
            } else {
                spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x29, 0x93, 0x4F);
            }
            break;
        case 2:
            if (obj->second == 0) {
                spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x28, 0x93, 0x22);
            } else {
                spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x29, 0x93, 0x4F);
            }
            break;
        }
    }
}

/* The page's states: open, pick a member, then the one to swap places with, swap, close. */
void ststatus_order_run(StstatusOrderPage *obj, StstatusOrderPageData *data) {
    s32 old;
    s32 digimon;

    switch (obj->base.step) {
    case 0:
    default:
        if (obj->member_count == 1) {
            ststatus_module.window_anim_update(&obj->bars[1]);
        } else {
            ststatus_module.window_anim_update(&obj->bars[0]);
        }
        if (ststatus_module.window_anim_update(&obj->member_panels[0])) {
            ststatus_order_show_member(obj, data, 0, 1);
            if (obj->member_count == 1) {
                obj->base.step = 15;
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x10);
            } else {
                obj->base.step++;
                data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x11);
                ststatus_module.window_anim_start(&obj->member_panels[1], 1);
            }
        }
        break;
    case 1:
        if (obj->member_count == 2) {
            ststatus_module.window_anim_update(&obj->bars[1]);
        }
        if (ststatus_module.window_anim_update(&obj->member_panels[1])) {
            ststatus_order_show_member(obj, data, 1, 1);
            if (obj->member_count == 2) {
                obj->base.step = 5;
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0xF);
            } else {
                obj->base.step++;
                ststatus_module.window_anim_start(&obj->member_panels[2], 1);
            }
        }
        break;
    case 2:
        ststatus_module.window_anim_update(&obj->member_panels[2]);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_order_show_member(obj, data, 2, 1);
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0xF);
            obj->base.step = 5;
        }
        break;
    case 5:
        obj->member_cursor_shown = 1;
        obj->base.step++;
        break;
    case 6:
        old = obj->member_cursor;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            if (--obj->member_cursor < 0) {
                obj->member_cursor = obj->member_count - 1;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            if (++obj->member_cursor > obj->member_count - 1) {
                obj->member_cursor = 0;
            }
        }
        if (old != obj->member_cursor) {
            sound_module.play(0x4001B);
        }
        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            obj->second_cursor_shown = 1;
            obj->base.step++;
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x12);
            obj->first = obj->second = obj->member_cursor;
            do {
                if (++obj->second > obj->member_count - 1) {
                    obj->second = 0;
                }
            } while (obj->first == obj->second);
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->member_cursor_shown = 0;
            obj->base.step = 20;
        }
        break;
    case 7:
        old = obj->second;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            do {
                if (--obj->second < 0) {
                    obj->second = obj->member_count - 1;
                }
            } while (obj->first == obj->second);
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            do {
                if (++obj->second > obj->member_count - 1) {
                    obj->second = 0;
                }
            } while (obj->first == obj->second);
        }
        if (old != obj->second) {
            sound_module.play(0x4001B);
        }
        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            ststatus_order_show_member(obj, data, obj->first, 0);
            ststatus_order_show_member(obj, data, obj->second, 0);
            ststatus_module.window_anim_start(&obj->swap, 0);
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x11);
            obj->base.step++;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->second_cursor_shown = 0;
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x11);
            obj->base.step--;
        }
        break;
    case 8:
        if (ststatus_module.window_anim_update(&obj->swap)) {
            ststatus_module.window_anim_start(&obj->swap, 1);
            obj->second_cursor_shown = 0;
            digimon = gamestate_data.funcs.get_party_member(obj->first);
            gamestate_data.party[obj->first] = gamestate_data.funcs.get_party_member(obj->second);
            gamestate_data.party[obj->second] = digimon;
            obj->frames[obj->first] = 0;
            obj->frames[obj->second] = 0;
            obj->base.step++;
        }
        break;
    case 9:
        if (ststatus_module.window_anim_update(&obj->swap)) {
            ststatus_order_show_member(obj, data, obj->first, 1);
            ststatus_order_show_member(obj, data, obj->second, 1);
            obj->base.step = 6;
        }
        break;
    case 15:
        obj->member_cursor_shown = 1;
        obj->base.step++;
        break;
    case 16:
        if (PAD_PRESSED(14)) {
            obj->base.step = 20;
        }
        break;
    case 20:
        switch (obj->member_count) {
        case 1:
        default:
            data->message->set_visible(data->message, 0);
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_order_show_member(obj, data, 0, 0);
            ststatus_module.window_anim_start(&obj->bars[1], 0);
            break;
        case 2:
            ststatus_module.window_anim_start(&obj->member_panels[1], 0);
            ststatus_module.window_anim_start(&obj->bars[1], 0);
            data->message->set_visible(data->message, 0);
            ststatus_order_show_member(obj, data, 1, 0);
            break;
        case 3:
            ststatus_module.window_anim_start(&obj->member_panels[2], 0);
            ststatus_module.window_anim_start(&obj->bars[1], 0);
            data->message->set_visible(data->message, 0);
            ststatus_order_show_member(obj, data, 2, 0);
            break;
        }
        obj->base.step = obj->member_count + 20;
        break;
    case 21:
        if (ststatus_module.window_anim_update(&obj->member_panels[0])) {
            obj->base.state = OBJECT_STATE_END;
        }
        ststatus_module.window_anim_update(&obj->bars[1]);
        break;
    case 22:
        ststatus_module.window_anim_update(&obj->member_panels[1]);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_module.window_anim_start(&obj->bars[0], 0);
            data->title->set_visible(data->title, 0);
            ststatus_order_show_member(obj, data, 0, 0);
            obj->base.step = 24;
        }
        break;
    case 24:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        if (ststatus_module.window_anim_update(&obj->bars[0])) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 23:
        ststatus_module.window_anim_update(&obj->member_panels[2]);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 0);
            ststatus_order_show_member(obj, data, 1, 0);
            obj->base.step = 25;
        }
        break;
    case 25:
        if (ststatus_module.window_anim_update(&obj->member_panels[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_module.window_anim_start(&obj->bars[0], 0);
            data->title->set_visible(data->title, 0);
            ststatus_order_show_member(obj, data, 0, 0);
            obj->base.step = 24;
        }
        break;
    }
}


void ststatus_order_update(StstatusOrderPage *obj, StstatusOrderPageData *data) {
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
            obj->member_panels[i].duration = 10;
        }
        ststatus_module.window_anim_start(&obj->member_panels[0], 1);
        for (i = 0; i < 2; i++) {
            obj->bars[i].duration = 10;
            ststatus_module.window_anim_start(&obj->bars[i], 1);
            obj->swap.duration = 8;
            ststatus_module.window_anim_start(&obj->swap, 0);
        }
        ststatus_order_create_windows(obj, data);
        break;
    case OBJECT_STATE_RUN:
        ststatus_order_run(obj, data);
        ststatus_order_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Creates the party-order page. */
StstatusOrderPage *ststatus_create_order_page(s32 arg0) {
    StstatusOrderPage *obj = object_new(ststatus_order_update, sizeof(StstatusOrderPage), sizeof(StstatusOrderPageData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->parent = arg0;
    return obj;
}

/* The map page (ststatus_create_map_page): the region's map with the areas visited, a cursor to scroll it with,
 * and the name of the area under the cursor. */

/* Its data block (4 bytes). */
typedef struct StstatusMapPageData {
    /* 0x0 */ MessageWindow *area_name; /* the area's name */
} StstatusMapPageData;

/* The map page (ststatus_create_map_page, size 0x1A0). */
typedef struct StstatusMapPage {
    /* 0x000 */ Object base;
    /* 0x050 */ s32 parent;
    /* 0x054 */ s32 layer_id; /* layer */
    /* 0x058 */ s32 ot_depth; /* ordering table depth */
    /* 0x05C */ s32 screen_h; /* screen height */
    /* 0x060 */ s32 map_y;  /* y offset of the map */
    /* 0x064 */ u8 unk_64[0x4];
    /* 0x068 */ s32 file;   /* file */
    /* 0x06C */ u8 unk_6C[0xC];
    /* 0x078 */ s32 region; /* region */
    /* 0x07C */ s32 marks_shown; /* the marks shown, from the story's progress */
    /* 0x080 */ s32 images; /* images: map, marks, areas and cursor */
    /* 0x084 */ s32 mark_sprites;
    /* 0x088 */ s32 cursor_sprites;
    /* 0x08C */ s32 names_file; /* text file of the area names */
    /* 0x090 */ s32 scroll; /* map scroll */
    /* 0x094 */ s32 scroll_y;
    /* 0x098 */ s32 current_pos; /* the current area's position */
    /* 0x09C */ s32 current_y;
    /* 0x0A0 */ s32 marker_frame; /* its marker's frame, 0..2 */
    /* 0x0A4 */ u8 unk_A4[0x4];
    /* 0x0A8 */ s32 visited[0x2F]; /* the areas visited (ststatus_map.mark_visited) */
    /* 0x164 */ s32 cursor;  /* cursor */
    /* 0x168 */ s32 cursor_y;
    /* 0x16C */ s32 set_up;  /* the map is set up */
    /* 0x170 */ s32 cursor_frame; /* cursor frame, 0..3 */
    /* 0x174 */ s32 frame_time; /* time of the last frame */
    /* 0x178 */ s32 cursor_speed; /* cursor speed, 0..4 */
    /* 0x17C */ s32 cursor_speed_y;
    /* 0x180 */ s32 on_area; /* the cursor is on an area */
    /* 0x184 */ s32 area;    /* which (from 1) */
    /* 0x188 */ s32 area_cursor; /* the cursor when it got there */
    /* 0x18C */ s32 area_cursor_y;
    /* 0x190 */ s32 unk_190;
    /* 0x194 */ u8 unk_194[0xC];
} StstatusMapPage; /* size 0x1A0 */

/* The map cursor's animation (sprite offsets, by cursor_frame). */
s32 ststatus_map_cursor_frames[4] = { 0, 1, 2, 1 };

/* Finds the visited area under the cursor and shows its name. */
void ststatus_map_find_area(StstatusMapPage *obj, StstatusMapPageData *data) {
    s32 old;
    s32 x;
    s32 y;
    s32 i;
    StstatusLayout *l;
    s32 lx;
    s32 ly;

    old = obj->on_area;
    obj->on_area = 0;
    x = obj->cursor - obj->scroll;
    y = obj->cursor_y - obj->scroll_y;
    l = ststatus_module.area_layout;
    for (i = 1; i < 47; i++) {
        if (obj->visited[i - 1] != 0) {
            lx = l[i].x;
            ly = l[i].y;
            if (lx + 6 < x && lx + 0x12 >= x && ly + 6 < y && ly + 0x12 >= y) {
                obj->on_area = 1;
                obj->area = i;
                break;
            }
        }
    }
    if (old != obj->on_area) {
        if (obj->on_area != 0) {
            sound_module.play(0x4001C);
            obj->area_cursor = obj->cursor;
            obj->area_cursor_y = obj->cursor_y;
        } else {
            data->area_name->set_visible(data->area_name, 0);
        }
    }
    if (obj->on_area != 0) {
        if (obj->area_cursor_y < obj->screen_h / 2) {
            data->area_name->set_pos(data->area_name, 0x10, 0xB9);
        } else {
            data->area_name->set_pos(data->area_name, 0x10, 0x19);
        }
        data->area_name->set_text(data->area_name, cdload_module.files.get_file(obj->names_file), obj->area);
    }
}

/* Moves the cursor by (dx, dy) steps of cursor_speed/cursor_speed_y; past the middle of the screen the map scrolls. */
void ststatus_move_map_cursor(StstatusMapPage *obj, s32 dx, s32 dy) {
    s32 mid;

    if (dx < 0) {
        if (obj->cursor > 0xA0) {
            obj->cursor -= obj->cursor_speed;
            if (obj->cursor < 0xA0) {
                obj->cursor = 0xA0;
            }
        } else if (obj->cursor < 0xA0) {
            obj->cursor -= obj->cursor_speed;
            if (obj->cursor < 0x16) {
                obj->cursor = 0x16;
            }
        } else {
            obj->scroll += obj->cursor_speed;
            if (obj->scroll > 0) {
                obj->scroll = 0;
                obj->cursor = 0x9F;
            }
        }
    } else if (dx > 0) {
        if (obj->cursor < 0xA0) {
            obj->cursor += obj->cursor_speed;
            if (obj->cursor > 0xA0) {
                obj->cursor = 0xA0;
            }
        } else if (obj->cursor > 0xA0) {
            obj->cursor += obj->cursor_speed;
            if (obj->cursor > 0x12A) {
                obj->cursor = 0x12A;
            }
        } else {
            obj->scroll -= obj->cursor_speed;
            if (obj->scroll < -0x48) {
                obj->scroll = -0x48;
                obj->cursor = 0xA1;
            }
        }
    }
    mid = obj->screen_h / 2;
    if (dy < 0) {
        if (obj->cursor_y > mid) {
            obj->cursor_y -= obj->cursor_speed_y;
            if (obj->cursor_y < mid) {
                obj->cursor_y = mid;
            }
        } else if (obj->cursor_y < mid) {
            obj->cursor_y -= obj->cursor_speed_y;
            if (obj->cursor_y < 0xF) {
                obj->cursor_y = 0xF;
            }
        } else {
            obj->scroll_y += obj->cursor_speed_y;
            if (obj->scroll_y > 0) {
                obj->scroll_y = 0;
                obj->cursor_y = mid - 1;
            }
        }
    } else if (dy > 0) {
        if (obj->cursor_y < mid) {
            obj->cursor_y += obj->cursor_speed_y;
            if (obj->cursor_y > mid) {
                obj->cursor_y = mid;
            }
        } else if (obj->cursor_y > mid) {
            obj->cursor_y += obj->cursor_speed_y;
            if (obj->cursor_y > obj->screen_h - obj->map_y - 0x1E) {
                obj->cursor_y = obj->screen_h - obj->map_y - 0x1E;
            }
        } else {
            obj->scroll_y -= obj->cursor_speed_y;
            if (obj->scroll_y < -0x60) {
                obj->scroll_y = -0x60;
                obj->cursor_y = mid + 1;
            }
        }
    }
}

/* The pad: the d-pad moves the cursor (faster while held), cancel closes the page. */
void ststatus_map_handle_pad(StstatusMapPage *obj, StstatusMapPageData *data) {
    if (PAD_HELD(7) || PAD_HELD(5)) {
        if (PAD_PRESSED(7) || PAD_PRESSED(5)) {
            obj->cursor_speed = 2;
        } else if (PAD_REPEAT(7) || PAD_REPEAT(5)) {
            if (++obj->cursor_speed >= 5) {
                obj->cursor_speed = 4;
            }
        }
    } else {
        obj->cursor_speed = 0;
    }
    if (PAD_HELD(4) || PAD_HELD(6)) {
        if (PAD_PRESSED(4) || PAD_PRESSED(6)) {
            obj->cursor_speed_y = 2;
        } else if (PAD_REPEAT(4) || PAD_REPEAT(6)) {
            if (++obj->cursor_speed_y >= 5) {
                obj->cursor_speed_y = 4;
            }
        }
    } else {
        obj->cursor_speed_y = 0;
    }
    if (PAD_HELD(7)) {
        ststatus_move_map_cursor(obj, -1, 0);
    } else if (PAD_HELD(5)) {
        ststatus_move_map_cursor(obj, 1, 0);
    }
    if (PAD_HELD(4)) {
        ststatus_move_map_cursor(obj, 0, -1);
    } else if (PAD_HELD(6)) {
        ststatus_move_map_cursor(obj, 0, 1);
    }
    if (PAD_PRESSED(14)) {
        sound_module.play(0x800450BD);
        obj->base.state = OBJECT_STATE_END;
    }
}

/* Draws the name's frame, the cursor, the current area's marker, the areas visited, the marks and the map. */
void ststatus_map_draw(StstatusMapPage *obj) {
    Sprite spr;
    s32 *marks;
    s32 x;
    s32 frame;
    s32 i;

    if (obj->on_area != 0) {
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 2);
        spr.set_vram_pos(0x280, 0x100);
        if (obj->area_cursor_y < obj->screen_h / 2) {
            spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x3B, 0, 0xB4);
        } else {
            spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x3B, 0, 0x14);
        }
    }
    sprite_init(&spr);
    if (obj->set_up != 0) {
        if (gfx_module.funcs.get_time() - obj->frame_time >= 4) {
            obj->frame_time = gfx_module.funcs.get_time();
            if (++obj->cursor_frame >= 4) {
                obj->cursor_frame = 0;
            }
            if (++obj->marker_frame >= 3) {
                obj->marker_frame = 0;
            }
        }
        spr.set_vram_pos(0x300, 0x100);
        if (obj->on_area != 0) {
            spr.set_layer_id(obj->layer_id, obj->ot_depth);
            spr.draw(cdload_module.get_subfile_by_id(obj->cursor_sprites), ststatus_map_cursor_frames[obj->cursor_frame] + 0x38,
                       ststatus_module.area_layout[obj->area].x + 0xC + obj->scroll,
                       ststatus_module.area_layout[obj->area].y + 0xC + obj->scroll_y + obj->map_y);
        } else {
            spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
            spr.draw(cdload_module.get_subfile_by_id(obj->cursor_sprites), ststatus_map_cursor_frames[obj->cursor_frame] + 0x32, obj->cursor,
                       obj->cursor_y + obj->map_y);
        }
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 2);
        if (obj->on_area != 0) {
            x = obj->area_cursor;
        } else {
            x = obj->cursor;
        }
        frame = 0x3B;
        if (x < obj->current_pos + obj->scroll) {
            frame = 0x35;
        }
        spr.draw(cdload_module.get_subfile_by_id(obj->cursor_sprites), frame + obj->marker_frame, obj->current_pos + obj->scroll,
                   obj->current_y + obj->scroll_y + obj->map_y);
    }
    spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
    if (obj->set_up != 0) {
        spr.set_vram_pos(0x300, 0x100);
        for (i = 1; i < 47; i++) {
            if (obj->visited[i - 1] != 0) {
                spr.draw(cdload_module.get_subfile_by_id(obj->cursor_sprites), ststatus_module.area_layout[i].text,
                           ststatus_module.area_layout[i].x + obj->scroll,
                           ststatus_module.area_layout[i].y + obj->scroll_y + obj->map_y);
            }
        }
    }
    spr.set_layer_id(obj->layer_id, obj->ot_depth + 1);
    marks = ststatus_module.get_kind_list(obj->region, obj->marks_shown);
    if (gamestate_data.progress < 7) {
        obj->marks_shown = 0;
    } else if (gamestate_data.progress < 15) {
        obj->marks_shown = 1;
    } else if (gamestate_data.progress < 27) {
        obj->marks_shown = 2;
    } else if (gamestate_data.progress < 30) {
        obj->marks_shown = 3;
    } else {
        obj->marks_shown = 4;
    }
    spr.set_vram_pos(0x380, 0x100);
    for (i = 0; marks[i] != 0; i++) {
        spr.draw(cdload_module.get_subfile_by_id(obj->mark_sprites), marks[i] - 1, ststatus_module.mark_positions[marks[i]].x + obj->scroll,
                   ststatus_module.mark_positions[marks[i]].y + obj->scroll_y + obj->map_y);
    }
    spr.set_vram_pos(0x280, 0);
    spr.set_clut8_pos(0x140, 0x100);
    spr.draw(cdload_module.get_subfile_by_id(obj->images), 0, obj->scroll, obj->scroll_y + obj->map_y);
}

void ststatus_map_update(StstatusMapPage *obj, StstatusMapPageData *data) {
    s32 area;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            if (!cdload_module.is_loading(obj->names_file)) {
                obj->base.step++;
            }
            break;
        case 1:
            data->area_name = message_create_window(obj->layer_id, 1, 0x14, 0x16);
            data->area_name->set_page_lines(data->area_name, 2);
            obj->unk_190 = 10;
            ststatus_map.mark_visited(obj->visited);
            area = ststatus_map.get_area() + 1;
            obj->current_pos = ststatus_module.area_layout[area].x + 0xC;
            obj->current_y = ststatus_module.area_layout[area].y + 0xC;
            obj->cursor_speed = 1;
            obj->cursor_speed_y = 1;
            while (obj->current_pos + obj->scroll != obj->cursor) {
                ststatus_move_map_cursor(obj, 1, 0);
            }
            while (obj->current_y + obj->scroll_y != obj->cursor_y) {
                ststatus_move_map_cursor(obj, 0, 1);
            }
            obj->cursor_speed = 0;
            obj->cursor_speed_y = 0;
            obj->set_up = 1;
            obj->base.next_state(obj);
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        ststatus_map_handle_pad(obj, data);
        ststatus_map_find_area(obj, data);
        ststatus_map_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Creates the map page. */
StstatusMapPage *ststatus_create_map_page(s32 arg0) {
    StstatusMapPage *obj = object_new(ststatus_map_update, sizeof(StstatusMapPage), sizeof(StstatusMapPageData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 5;
    obj->parent = arg0;
    obj->region = ststatus_map.get_region();
    if (obj->region == 0) {
        obj->images = 0x07990000;
        obj->mark_sprites = 0x07990001;
        obj->cursor_sprites = 0x07990002;
        obj->file = 0x799;
        obj->names_file = records_language + 8;
    } else {
        obj->images = 0x079B0000;
        obj->mark_sprites = 0x079B0001;
        obj->cursor_sprites = 0x079B0002;
        obj->file = 0x79B;
        obj->names_file = records_language + 1;
    }
    cdload_module.queue_file(obj->names_file);
    obj->screen_h = 0xF0;
    obj->map_y = 0x10;
    return obj;
}

/* The scroll bar (the same code as STCRDDEK's, created on ordering table entry 0). */
void ststatus_set_bar_x(StstatusBar *obj, s32 x, s32 width) {
    obj->x = x;
    obj->w = width;
}

void ststatus_set_bar_range(StstatusBar *obj, s32 top, s32 bottom) {
    obj->top = top;
    obj->bottom = bottom;
    obj->range_set = 1;
}

void ststatus_set_bar_lines(StstatusBar *obj, s32 shown, s32 lines) {
    obj->shown = shown;
    obj->lines = lines;
    obj->lines_set = 1;
}

void ststatus_set_bar_line(StstatusBar *obj, s32 line) {
    obj->first_line = line;
}

void ststatus_update_bar(StstatusBar *obj) {
    GfxLayer *layer;
    u32 *ot;
    POLY_F4 *poly;
    s32 range;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->range_set != 0 && obj->lines_set != 0) {
            range = (obj->bottom - obj->top) << 8;
            obj->h = range / obj->lines * obj->shown;
            obj->line_height = range / obj->lines;
            obj->base.next_state(obj);
        }
        break;
    case OBJECT_STATE_RUN:
        layer = gfx_module.funcs.get_layer(obj->layer_id);
        ot = layer->get_ot_entry(layer, obj->ot_depth);
        poly = gfx_module.funcs.get_packet();
        if (obj->first_line < obj->lines - 1) {
            obj->y = obj->top + ((obj->first_line * obj->line_height) >> 8);
            if (obj->bottom - (obj->h >> 8) < obj->y) {
                obj->y = obj->bottom - (obj->h >> 8);
            }
        } else {
            obj->y = obj->bottom - (obj->h >> 8);
        }
        setPolyF4(poly);
        poly->r0 = poly->g0 = poly->b0 = 0xFF;
        poly->x0 = poly->x2 = obj->x;
        poly->x1 = poly->x3 = obj->x + obj->w;
        poly->y0 = poly->y1 = obj->y;
        poly->y2 = poly->y3 = obj->y + (obj->h >> 8);
        addPrim(ot, poly);
        gfx_module.funcs.set_packet(poly + 1);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StstatusBar *ststatus_create_bar(void) {
    StstatusBar *obj = object_new(ststatus_update_bar, sizeof(StstatusBar), 0);

    obj->set_x = ststatus_set_bar_x;
    obj->set_range = ststatus_set_bar_range;
    obj->set_lines = ststatus_set_bar_lines;
    obj->set_line = ststatus_set_bar_line;
    obj->layer_id = 0x1000;
    obj->ot_depth = 0;
    return obj;
}

/* The menu's root object (ststatus_create_root): loads the files, draws the scrolling background and
 * opens the page D_8005CCF0 selects. */

/* In the other files (each returns its own page type there). */
Object *ststatus_create_items_page(Object *parent);
Object *ststatus_create_tech_page(Object *parent);
Object *ststatus_create_status_page(Object *parent);
Object *ststatus_create_option_page(Object *parent);
Object *ststatus_create_party_page(Object *parent);

/* The pages' creators, 7 per row (NULL: the items page). */
Object *(*ststatus_pages[2][7])(Object *parent) = {
    {
        ststatus_create_items_page,
        (Object *(*)(Object *))ststatus_create_order_page,
        (Object *(*)(Object *))ststatus_create_map_page,
        ststatus_create_tech_page,
        ststatus_create_status_page,
        ststatus_create_option_page,
        NULL,
    },
    {
        ststatus_create_items_page,
        (Object *(*)(Object *))ststatus_create_order_page,
        (Object *(*)(Object *))ststatus_create_map_page,
        ststatus_create_tech_page,
        ststatus_create_status_page,
        ststatus_create_party_page,
        ststatus_create_option_page,
    },
};

/* The root's data block (8 bytes). */
typedef struct StstatusRootData {
    /* 0x0 */ void *menu;   /* fieldmenu_create's object */
    /* 0x4 */ Object *page;   /* the page */
} StstatusRootData;

/* The root object (ststatus_create_root, size 0x78). */
typedef struct StstatusRoot {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id; /* layer */
    /* 0x54 */ s32 scroll; /* background scroll, 0..0x5F */
    /* 0x58 */ s32 odd_frame; /* toggles every frame: scroll every other frame */
    /* 0x5C */ s32 unk_5C;
    /* 0x60 */ s32 map_region; /* map region */
    /* 0x64 */ s32 files;  /* files to load */
    /* 0x68 */ s32 map_file;
    /* 0x6C */ s32 images; /* images */
    /* 0x70 */ s32 images_1;
    /* 0x74 */ s32 images_2;
} StstatusRoot; /* size 0x78 */

/* Opens the page, then waits for it to close. */
void ststatus_open_page(StstatusRoot *obj, StstatusRootData *data) {
    switch (obj->base.step) {
    case 0:
    default:
        if (ststatus_pages[D_8005CCF0.extended][D_8005CCF0.option] != NULL) {
            data->page = ststatus_pages[D_8005CCF0.extended][D_8005CCF0.option](&obj->base);
        } else {
            data->page = ststatus_create_items_page(&obj->base);
        }
        obj->base.step++;
        break;
    case 1:
        if (data->page == NULL) {
            data->menu = fieldmenu_create(obj->layer_id, D_8005CCF0.option);
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        break;
    }
}

/* Draws the background, scrolling it diagonally. */
void ststatus_draw_background(StstatusRoot *obj) {
    Sprite spr;
    s32 pos;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, 7);
    spr.set_vram_pos(0x280, 0x100);
    if (obj->odd_frame != 0) {
        pos = 0;
        if (++obj->scroll < 0x60) {
            pos = obj->scroll;
        }
        obj->scroll = pos;
        obj->odd_frame = 0;
    } else {
        obj->odd_frame = 1;
    }
    spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x1D, obj->scroll, obj->scroll);
}

void ststatus_update_root(StstatusRoot *obj, StstatusRootData *data) {
    Tim tim;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            ststatus_module.load_files();
            obj->base.step++;
            break;
        case 1:
            if (!ststatus_module.is_loading() && !cdload_module.is_loading(obj->files) &&
                !cdload_module.is_loading(obj->map_file)) {
                tim_init(&tim);
                tim.set_image_pos(0x380, 0x100);
                tim.load_all(cdload_module.get_subfile_by_id(obj->images_1));
                tim.set_image_pos(0x300, 0x100);
                tim.load_all(cdload_module.get_subfile_by_id(obj->images_2));
                tim.set_image_pos(0x280, 0);
                tim.set_clut_pos(0x140, 0x100);
                tim.load_all(cdload_module.get_subfile_by_id(obj->images));
                obj->base.next_state(obj);
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        ststatus_open_page(obj, data);
        ststatus_draw_background(obj);
        break;
    case OBJECT_STATE_DONE:
        if (data->menu == NULL) {
            obj->base.state = OBJECT_STATE_RUN;
        }
        ststatus_draw_background(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

/* Creates the root object and starts loading the region's files. */
StstatusRoot *ststatus_create_root(void) {
    StstatusRoot *obj = object_new(ststatus_update_root, sizeof(StstatusRoot), sizeof(StstatusRootData));

    obj->layer_id = 0x1000;
    obj->map_region = ststatus_map.get_region();
    if (obj->map_region == 0) {
        obj->images = 0x079A0000;
        obj->images_1 = 0x079A0001;
        obj->images_2 = 0x079A0002;
        obj->files = 0x79A;
        obj->map_file = 0x799;
    } else {
        obj->images = 0x079C0000;
        obj->images_1 = 0x079C0001;
        obj->images_2 = 0x079C0002;
        obj->files = 0x79C;
        obj->map_file = 0x79B;
    }
    cdload_module.queue_file(obj->files);
    cdload_module.queue_file(obj->map_file);
    return obj;
}
