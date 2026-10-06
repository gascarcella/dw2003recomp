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

/* A second party page (ststatus_create_option_page): the same code as the first file's, with an option menu. */

/* The second party page's data block (ststatus_create_option_page, 0x9C bytes): its text windows. */
typedef struct StstatusOptionPageData {
    /* 0x00 */ StstatusPanelWindows panels[3]; /* per party member: its panel */
    /* 0x84 */ MessageWindow *help;
    /* 0x88 */ MessageWindow *back_hint;
    /* 0x8C */ MessageWindow *menu_title; /* menu title */
    /* 0x90 */ MessageWindow *menu_choices[2]; /* menu choices */
    /* 0x98 */ MessageCursor *menu_cursor; /* menu cursor */
} StstatusOptionPageData; /* size 0x9C */

/* The second party page (ststatus_create_option_page, size 0xD4): the party page's layout, with a two-choice option menu (gamestate_data.digivolve_demo). */
typedef struct StstatusOptionPage {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 parent;
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 ot_depth; /* ordering table depth */
    /* 0x5C */ s32 member_count; /* party members */
    /* 0x60 */ s32 frames[3]; /* per member: sprite frame */
    /* 0x6C */ s32 frame_time; /* time of the last frame */
    /* 0x70 */ s32 menu_cursor; /* menu cursor, 0..1 */
    /* 0x74 */ WindowAnim member_panels[3]; /* member panels */
    /* 0xA4 */ WindowAnim bars[2];
    /* 0xC4 */ WindowAnim menu_anim;
} StstatusOptionPage; /* size 0xD4 */

s32 ststatus_option_panel_stats[5] = { 0, 2, 3, 4, 5 };

/* Creates the page's text windows. */
void ststatus_option_create_windows(StstatusOptionPage *obj, StstatusOptionPageData *data) {
    s32 i;
    s32 j;
    StstatusLayout *l;

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
    data->menu_title = message_create_window(obj->layer_id, 1, 0xB2, 0x2A);
    for (j = 0; j < 2; j++) {
        data->menu_choices[j] = message_create_window(obj->layer_id, 1, 0xC5, j * 0xE + 0x3A);
    }
    data->menu_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0xB8, 0x3A);
    data->menu_cursor->show(data->menu_cursor, 0);
}

/* Shows (fills in) or hides a member's panel. */
void ststatus_option_show_member(StstatusOptionPage *obj, StstatusOptionPageData *data, s32 member, s32 show) {
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
            win->set_line_number(win, 0, stats.values[ststatus_option_panel_stats[i]]);
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

/* Shows or hides the option menu, its cursor on the current setting. */
void ststatus_option_show_menu(StstatusOptionPage *obj, StstatusOptionPageData *data, s32 show) {
    s32 i;

    if (show) {
        data->menu_title->set_text(data->menu_title, cdload_module.files.get_file(records_language + 0xB0), 0x46);
        for (i = 0; i < 2; i++) {
            data->menu_choices[i]->set_text(data->menu_choices[i], cdload_module.files.get_file(records_language + 0xB0), i + 0x47);
        }
        obj->menu_cursor = 1 - (u8)gamestate_data.digivolve_demo;
        data->menu_cursor->set_pos(data->menu_cursor, 0xB8, obj->menu_cursor * 0xE + 0x3A);
        if (obj->menu_cursor == 0) {
            data->menu_choices[0]->set_palette(data->menu_choices[0], 1);
            data->menu_choices[1]->set_palette(data->menu_choices[1], 0);
        } else {
            data->menu_choices[0]->set_palette(data->menu_choices[0], 0);
            data->menu_choices[1]->set_palette(data->menu_choices[1], 1);
        }
        data->menu_cursor->show(data->menu_cursor, 1);
    } else {
        data->menu_title->set_visible(data->menu_title, 0);
        for (i = 0; i < 2; i++) {
            data->menu_choices[i]->set_visible(data->menu_choices[i], 0);
        }
        data->menu_cursor->show(data->menu_cursor, 0);
    }
}
/* Draws the members' sprites and the panels' frames. */
void ststatus_option_draw(StstatusOptionPage *obj) {
    Sprite spr;
    s32 i;
    s32 digimon;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (gfx_module.funcs.get_time() - obj->frame_time >= 13) {
        obj->frame_time = gfx_module.funcs.get_time();
        for (i = 0; i < obj->member_count; i++) {
            digimon = gamestate_data.funcs.get_party_member(i);
            obj->frames[i]++;
            if (ststatus_module.anims[digimon].frame[obj->frames[i]] == -1 ||
                obj->frames[i] >= 7) {
                obj->frames[i] = 0;
            }
        }
    }
    for (i = 0; i < obj->member_count; i++) {
        if (obj->member_panels[i].level != 0) {
            if (obj->member_panels[i].level != 0x1000) {
                spr.set_scale(obj->member_panels[i].level, obj->member_panels[i].level, 0x1000);
                spr.set_pivot(0x7C, i * 0x2E + 0x27);
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
            }
            digimon = gamestate_data.funcs.get_party_member(i);
            spr.set_vram_pos(0x280, 0x100);
            spr.draw(cdload_module.get_subfile_by_id(0x04040000), ststatus_module.anims[digimon].frame[obj->frames[i]], 0x6B,
                       i * 0x2E + 0x13);
        }
    }
    spr.set_vram_pos(0x140, 0);
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
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x15, 0, i * 0x2E + 0x11);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x16, 0x67, i * 0x2E + 0x13);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x17, 0, i * 0x2E + 0x11);
            }
        }
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
    if (obj->menu_anim.level != 0) {
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        spr.set_vram_pos(0x280, 0x100);
        if (obj->menu_anim.level != 0x1000) {
            spr.set_scale(obj->menu_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x40);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x3A, 0xA8, 0x28);
    }
}

/* The page's states: open the panels one by one, run the option menu, close them again. */
void ststatus_option_run(StstatusOptionPage *obj, StstatusOptionPageData *data) {
    s32 old;

    switch (obj->base.step) {
    case 0:
    default:
        ststatus_module.window_anim_start(&obj->member_panels[0], 1);
        if (obj->member_count == 1) {
            ststatus_module.window_anim_start(&obj->bars[1], 1);
            ststatus_module.window_anim_start(&obj->menu_anim, 1);
        }
        obj->base.step = obj->member_count;
        break;
    case 1:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        ststatus_module.window_anim_update(&obj->menu_anim);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_option_show_member(obj, data, 0, 1);
            data->help->set_text(data->help, cdload_module.files.get_file(records_language + 0xB0), 0x49);
            data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0xB0), 0x15);
            ststatus_option_show_menu(obj, data, 1);
            obj->base.step = 10;
        }
        break;
    case 2:
        if (ststatus_module.window_anim_update(&obj->member_panels[0])) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 1);
            ststatus_module.window_anim_start(&obj->bars[1], 1);
            ststatus_option_show_member(obj, data, 0, 1);
            obj->base.step = 4;
        }
        break;
    case 4:
        ststatus_module.window_anim_update(&obj->member_panels[1]);
        ststatus_module.window_anim_update(&obj->menu_anim);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_option_show_member(obj, data, 1, 1);
            data->help->set_text(data->help, cdload_module.files.get_file(records_language + 0xB0), 0x49);
            data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0xB0), 0x15);
            ststatus_option_show_menu(obj, data, 1);
            obj->base.step = 10;
        }
        break;
    case 3:
        if (ststatus_module.window_anim_update(&obj->member_panels[0])) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 1);
            ststatus_option_show_member(obj, data, 0, 1);
            obj->base.step = 5;
        }
        break;
    case 5:
        if (ststatus_module.window_anim_update(&obj->member_panels[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[2], 1);
            ststatus_module.window_anim_start(&obj->bars[1], 1);
            ststatus_option_show_member(obj, data, 1, 1);
            obj->base.step++;
        }
        break;
    case 6:
        ststatus_module.window_anim_update(&obj->member_panels[2]);
        ststatus_module.window_anim_update(&obj->menu_anim);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_option_show_member(obj, data, 2, 1);
            data->help->set_text(data->help, cdload_module.files.get_file(records_language + 0xB0), 0x49);
            data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0xB0), 0x15);
            ststatus_option_show_menu(obj, data, 1);
            obj->base.step = 10;
        }
        break;
    case 10:
        old = obj->menu_cursor;
        if (PAD_PRESSED(4)) {
            obj->menu_cursor = 0;
        } else if (PAD_PRESSED(6)) {
            obj->menu_cursor = 1;
        }
        if (old != obj->menu_cursor) {
            sound_module.play(0x8004513E);
            data->menu_cursor->set_pos(data->menu_cursor, 0xB8, obj->menu_cursor * 0xE + 0x3A);
        }
        if (PAD_PRESSED(13)) {
            sound_module.play(0x8004503C);
            if (obj->menu_cursor == 0) {
                gamestate_data.digivolve_demo = 1;
            } else {
                gamestate_data.digivolve_demo = 0;
            }
            obj->base.step = 50;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step = 50;
        }
        break;
    case 50:
        ststatus_module.window_anim_start(&obj->member_panels[obj->member_count - 1], 0);
        ststatus_option_show_member(obj, data, obj->member_count - 1, 0);
        ststatus_module.window_anim_start(&obj->bars[1], 0);
        data->help->set_visible(data->help, 0);
        data->back_hint->set_visible(data->back_hint, 0);
        ststatus_module.window_anim_start(&obj->menu_anim, 0);
        ststatus_option_show_menu(obj, data, 0);
        obj->base.step = obj->member_count + 50;
        break;
    case 51:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        ststatus_module.window_anim_update(&obj->menu_anim);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            obj->base.step = 57;
        }
        break;
    case 52:
        ststatus_module.window_anim_update(&obj->member_panels[1]);
        ststatus_module.window_anim_update(&obj->menu_anim);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_option_show_member(obj, data, 0, 0);
            obj->base.step = 54;
        }
        break;
    case 53:
        ststatus_module.window_anim_update(&obj->member_panels[2]);
        ststatus_module.window_anim_update(&obj->menu_anim);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 0);
            ststatus_option_show_member(obj, data, 1, 0);
            obj->base.step = 55;
        }
        break;
    case 55:
        if (ststatus_module.window_anim_update(&obj->member_panels[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_option_show_member(obj, data, 0, 0);
            obj->base.step++;
        }
        break;
    case 54:
    case 56:
        if (ststatus_module.window_anim_update(&obj->member_panels[0])) {
            obj->base.step = 57;
        }
        break;
    case 57:
        obj->base.state = OBJECT_STATE_END;
        break;
    }
}

void ststatus_option_update(StstatusOptionPage *obj, StstatusOptionPageData *data) {
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
            ststatus_module.window_anim_start(&obj->member_panels[i], 1);
        }
        for (i = 0; i < 2; i++) {
            obj->bars[i].duration = 10;
            ststatus_module.window_anim_start(&obj->bars[i], 1);
        }
        obj->menu_anim.duration = 10;
        ststatus_module.window_anim_start(&obj->menu_anim, 1);
        ststatus_option_create_windows(obj, data);
        break;
    case OBJECT_STATE_RUN:
        ststatus_option_run(obj, data);
        ststatus_option_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Creates the second party page. */
StstatusOptionPage *ststatus_create_option_page(s32 arg0) {
    StstatusOptionPage *obj = object_new(ststatus_option_update, sizeof(StstatusOptionPage), sizeof(StstatusOptionPageData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->parent = arg0;
    return obj;
}
/* The equipment page (ststatus_create_equip_page): a member's six equipment slots, and the items that fit one. */

/* A line of the equipment page's item list. */
typedef struct StstatusEquipLine {
    /* 0x0 */ MessageWindow *name; /* name */
    /* 0x4 */ MessageWindow *times; /* "x" */
    /* 0x8 */ MessageWindow *count; /* count */
} StstatusEquipLine; /* size 0xC */

/* Its data block (0xA4 bytes): its text windows, cursors and scroll bar. */
typedef struct StstatusEquipPageData {
    /* 0x00 */ MessageWindow *title;  /* title */
    /* 0x04 */ MessageWindow *slots[6];  /* equipment slots */
    /* 0x1C */ MessageCursor *slot_cursor; /* slot cursor */
    /* 0x20 */ MessageWindow *list_title; /* list title */
    /* 0x24 */ MessageCursor *list_cursor; /* list cursor */
    /* 0x28 */ StstatusEquipLine lines[8]; /* list lines */
    /* 0x88 */ MessageWindow *description; /* item description */
    /* 0x8C */ MessageWindow *kind;   /* item kind */
    /* 0x90 */ MessageWindow *slot_title; /* slot title */
    /* 0x94 */ MessageWindow *slot_item; /* slot item */
    /* 0x98 */ StstatusBar *scroll_bar; /* scroll bar (lists of more than 8) */
    /* 0x9C */ u8 unk_9C[0x8];
} StstatusEquipPageData; /* size 0xA4 */

/* The equipment page (ststatus_create_equip_page, size 0x70C). */
typedef struct StstatusEquipPage {
    /* 0x000 */ Object base;
    /* 0x050 */ StstatusStatusPage *status_page; /* the status page */
    /* 0x054 */ s32 layer_id; /* layer */
    /* 0x058 */ s32 ot_depth; /* ordering table depth */
    /* 0x05C */ s32 digimon; /* the member's Digimon */
    /* 0x060 */ s32 slot;   /* equipment slot, 0..5 */
    /* 0x064 */ s32 list_cursor; /* list cursor, 0..7 */
    /* 0x068 */ s32 list_length; /* list length */
    /* 0x06C */ s32 first_line; /* first line shown */
    /* 0x070 */ s32 arrows_shown; /* scroll arrows shown (blinking) */
    /* 0x074 */ s32 blink_time; /* time of the last blink */
    /* 0x078 */ s16 list[0x194];   /* the list: -1 (take off), then the items that fit, then the others */
    /* 0x3A0 */ s16 category_items[0x194]; /* the slot's category of items */
    /* 0x6C8 */ s32 draw_slot_item; /* draw the slot's item */
    /* 0x6CC */ WindowAnim anims[4];   /* slots, slot frame, list, slot item */
} StstatusEquipPage; /* size 0x70C */

s32 ststatus_equip_slot_texts[6] = { 65, 66, 67, 77, 68, 68 }; /* ?STATUS entries */
s32 ststatus_equip_kind_texts[8] = { 67, 77, 79, 65, 66, 68, 80, 68 }; /* ?STATUS entries */
s32 ststatus_equip_slot_categories[6] = { 6, 7, 5, 5, 4, 4 };

/* Creates the page's text windows. */
void ststatus_equip_create_windows(StstatusEquipPage *obj, StstatusEquipPageData *data) {
    s32 i;

    data->title = message_create_window(obj->layer_id, 1, 0x98, 0x13);
    data->slot_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0xA5, 0x31);
    data->slot_cursor->show(data->slot_cursor, 0);
    for (i = 0; i < 6; i++) {
        data->slots[i] = message_create_window(obj->layer_id, 1, 0xC0, i * 0xE + 0x31);
    }
    data->list_title = message_create_window(obj->layer_id, 1, 0x7F, 0x3B);
    data->list_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0x86, 0x4B);
    data->list_cursor->show(data->list_cursor, 0);
    for (i = 0; i < 8; i++) {
        data->lines[i].name = message_create_window(obj->layer_id, 1, 0xA0, i * 0xE + 0x4B);
        data->lines[i].times = message_create_window(obj->layer_id, 1, 0x10D, i * 0xE + 0x4B);
        data->lines[i].count = message_create_window(obj->layer_id, 1, 0x120, i * 0xE + 0x4B);
    }
    data->description = message_create_window(obj->layer_id, 1, 0x14, 0xC6);
    data->kind = message_create_window(obj->layer_id, 1, 0x14, 0xD5);
    data->slot_title = message_create_window(obj->layer_id, 1, 0xA7, 0x13);
    data->slot_item = message_create_window(obj->layer_id, 1, 0xC0, 0x23);
}

/* Shows (fills in) or hides the six slots. */
void ststatus_equip_show_slots(StstatusEquipPage *obj, StstatusEquipPageData *data, s32 show) {
    GamestateRecord *rec;
    s32 i;
    s16 item;

    if (show) {
        rec = gamestate_data.funcs.get_record(obj->digimon);
        for (i = 0; i < 6; i++) {
            item = rec->equipment[i];
            if (item > 0) {
                data->slots[i]->set_text(data->slots[i], cdload_module.files.get_file(records_language + 0x6A), item);
            } else {
                data->slots[i]->set_text(data->slots[i], cdload_module.files.get_file(records_language + 0xB0),
                                         ststatus_equip_slot_texts[i]);
            }
        }
    } else {
        for (i = 0; i < 6; i++) {
            data->slots[i]->set_visible(data->slots[i], 0);
        }
    }
}

/* Describes the item under the list cursor. */
void ststatus_equip_describe_item(StstatusEquipPage *obj, StstatusEquipPageData *data, s32 item) {
    RecordsEquip *e;

    if (item > 0) {
        data->description->set_text(data->description, cdload_module.files.get_file(records_language + 0x63), item);
        if (obj->slot == 2 || obj->slot == 3) {
            e = records_funcs.get_item(item)->data;
            data->kind->set_text(data->kind, cdload_module.files.get_file(records_language + 0xB0),
                                  ststatus_equip_kind_texts[e->slot - 1]);
            return;
        }
    } else {
        data->description->set_text(data->description, cdload_module.files.get_file(records_language + 0xB0), 0x51);
    }
    data->kind->set_visible(data->kind, 0);
}

/* Shows (fills in) or hides the item list. */
void ststatus_equip_show_list(StstatusEquipPage *obj, StstatusEquipPageData *data, s32 show) {
    s32 i;
    s32 k;
    s32 item;

    if (show) {
        data->list_title->set_text(data->list_title, cdload_module.files.get_file(records_language + 0xB0),
                              ststatus_equip_slot_texts[obj->slot]);
        for (i = 0; i < 8; i++) {
            k = obj->first_line + i;
            if (k > obj->list_length - 1) {
                break;
            }
            item = obj->list[k];
            if (item > 0) {
                if (ststatus_module.can_equip(obj->digimon, obj->slot, item)) {
                    data->lines[i].name->set_palette(data->lines[i].name, 0);
                } else {
                    data->lines[i].name->set_palette(data->lines[i].name, 7);
                }
                data->lines[i].name->set_text(data->lines[i].name, cdload_module.files.get_file(records_language + 0x6A),
                                               item);
                data->lines[i].times->set_text(data->lines[i].times, cdload_module.files.get_file(records_language + 0xB0),
                                               0x40);
                data->lines[i].times->measure(data->lines[i].times, 1);
                data->lines[i].count->set_line_number(data->lines[i].count, 0, gamestate_data.items[item]);
                data->lines[i].count->measure(data->lines[i].count, 1);
            } else {
                if (item == -1) {
                    data->lines[i].name->set_text(data->lines[i].name,
                                                   cdload_module.files.get_file(records_language + 0xB0), 0x45);
                    data->lines[i].name->set_palette(data->lines[i].name, 0);
                } else {
                    data->lines[i].name->set_visible(data->lines[i].name, 0);
                }
                data->description->set_visible(data->description, 0);
                data->kind->set_visible(data->kind, 0);
                data->lines[i].times->set_visible(data->lines[i].times, 0);
                data->lines[i].count->set_visible(data->lines[i].count, 0);
            }
        }
        ststatus_equip_describe_item(obj, data, obj->list[obj->list_cursor + obj->first_line]);
    } else {
        data->list_title->set_visible(data->list_title, 0);
        for (i = 0; i < 8; i++) {
            data->lines[i].name->set_visible(data->lines[i].name, 0);
            data->lines[i].times->set_visible(data->lines[i].times, 0);
            data->lines[i].count->set_visible(data->lines[i].count, 0);
        }
        data->description->set_visible(data->description, 0);
        data->kind->set_visible(data->kind, 0);
    }
}

/* Shows or hides the slot's current item. */
void ststatus_equip_show_current(StstatusEquipPage *obj, StstatusEquipPageData *data, s32 show) {
    s32 item;

    if (show) {
        /* written as a sum for the operand order (docs/MATCHING.md "C patterns") */
        item = *(gamestate_data.funcs.get_record(obj->digimon)->equipment + obj->slot);
        data->slot_title->set_text(data->slot_title, cdload_module.files.get_file(records_language + 0xB0), 0x20);
        if (item > 0) {
            data->slot_item->set_text(data->slot_item, cdload_module.files.get_file(records_language + 0x6A), item);
        } else {
            data->slot_item->set_text(data->slot_item, cdload_module.files.get_file(records_language + 0xB0),
                                  ststatus_equip_slot_texts[obj->slot]);
        }
    } else {
        data->slot_title->set_visible(data->slot_title, 0);
        data->slot_item->set_visible(data->slot_item, 0);
    }
}

/* Draws the item icons and the frames. */
void ststatus_equip_draw(StstatusEquipPage *obj) {
    Sprite spr;
    GamestateRecord *rec;
    s32 i;
    s32 k;
    s16 item;

    rec = gamestate_data.funcs.get_record(obj->digimon);
    sprite_init(&spr);
    if (obj->anims[0].level != 0) {
        spr.set_vram_pos(0x140, 0);
        spr.set_layer_id(obj->layer_id, 6);
        if (obj->anims[0].level != 0x1000) {
            spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x19);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x18, 0x22, 0xD);
    }
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->anims[1].level != 0) {
        if (obj->anims[1].level != 0x1000) {
            spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x58);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
            spr.set_vram_pos(0x140, 0);
            for (i = 0; i < 6; i++) {
                item = rec->equipment[i];
                if (item > 0) {
                    spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_funcs.get_item_icon(item), 0xB2, i * 0xE + 0x31);
                }
            }
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x2A, 0x9D, 0x28);
    }
    if (obj->anims[2].level != 0) {
        if (obj->anims[2].level != 0x1000) {
            spr.set_scale(obj->anims[2].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x89);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
            for (i = 0; i < 8; i++) {
                k = obj->first_line + i;
                if (k < obj->list_length && ((item = obj->list[k]) == -1 || item > 0)) {
                    spr.set_vram_pos(0x280, 0x100);
                    spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x31, 0x91, i * 0xE + 0x4B);
                    if (item > 0) {
                        spr.set_vram_pos(0x140, 0);
                        spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_funcs.get_item_icon(item), 0x91, i * 0xE + 0x4B);
                    }
                }
            }
            if (obj->list_length >= 9) {
                if (gfx_module.funcs.get_time() - obj->blink_time >= 9) {
                    obj->blink_time = gfx_module.funcs.get_time();
                    obj->arrows_shown = 1 - obj->arrows_shown;
                }
                spr.set_vram_pos(0x280, 0x100);
                if (obj->arrows_shown != 0) {
                    if (obj->first_line > 0) {
                        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x32, 0x126, 0x45);
                    }
                    if (obj->first_line < obj->list_length - 8) {
                        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x33, 0x126, 0xB3);
                    }
                }
            }
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x2B, 0x77, 0x39);
        if (obj->anims[2].level != 0x1000) {
            spr.set_pivot(0, 0xD3);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x20, 0, 0xC2);
    }
    if (obj->anims[3].level != 0) {
        if (obj->anims[3].level != 0x1000) {
            spr.set_scale(obj->anims[3].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x20);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
            if (obj->draw_slot_item != 0) {
                item = *(rec->equipment + obj->slot);   /* a sum for the operand order, as in ststatus_equip_show_current */
                if (item > 0) {
                    spr.set_vram_pos(0x140, 0);
                    spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_funcs.get_item_icon(item), 0xB2, 0x23);
                }
            }
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x2C, 0xA0, 0x11);
    }
}

/* The page's states: open, pick a slot, pick an item for it (equipping it), close. */
void ststatus_equip_run(StstatusEquipPage *obj, StstatusEquipPageData *data) {
    s32 old;
    s32 old_scroll;
    s32 count;
    s32 i;
    s32 n;
    s32 item;
    s32 old_slot;

    switch (obj->base.step) {
    case 0:
    default:
        ststatus_module.window_anim_start(&obj->anims[0], 1);
        obj->base.step++;
        break;
    case 1:
        if (ststatus_module.window_anim_update(&obj->anims[0])) {
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x3F);
            ststatus_module.window_anim_start(&obj->anims[1], 1);
            obj->base.step++;
        }
        break;
    case 2:
        if (ststatus_module.window_anim_update(&obj->anims[1])) {
            ststatus_equip_show_slots(obj, data, 1);
            data->slot_cursor->show(data->slot_cursor, 1);
            obj->base.step++;
        }
        break;
    case 3:
        old_slot = obj->slot;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            if (--obj->slot < 0) {
                obj->slot = 0;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            if (++obj->slot >= 6) {
                obj->slot = 5;
            }
        }
        if (old_slot != obj->slot) {
            sound_module.play(0x8004513E);
            data->slot_cursor->set_pos(data->slot_cursor, 0xA5, obj->slot * 0xE + 0x31);
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x8004503C);
            n = 1;
            count = ststatus_module.list_items(ststatus_equip_slot_categories[obj->slot], (u16 *)obj->category_items);
            for (i = 0; i < count; i++) {
                if (ststatus_module.can_equip(obj->digimon, obj->slot, obj->category_items[i])) {
                    obj->list[n++] = obj->category_items[i];
                }
            }
            for (i = 0; i < count; i++) {
                if (!ststatus_module.can_equip(obj->digimon, obj->slot, obj->category_items[i])) {
                    obj->list[n++] = obj->category_items[i];
                }
            }
            obj->list_length = count + 1;
            obj->list[0] = -1;
            obj->base.step = 10;
        }
        if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step = 50;
        }
        break;
    case 10:
        ststatus_module.window_anim_start(&obj->anims[1], 0);
        ststatus_equip_show_slots(obj, data, 0);
        data->slot_cursor->show(data->slot_cursor, 0);
        obj->list_cursor = 0;
        obj->first_line = 0;
        obj->base.step++;
        break;
    case 12:
        if (ststatus_module.window_anim_update(&obj->anims[0])) {
            ststatus_module.window_anim_start(&obj->anims[3], 1);
            obj->base.step++;
        }
        break;
    case 13:
        if (ststatus_module.window_anim_update(&obj->anims[3])) {
            obj->draw_slot_item = 1;
            ststatus_equip_show_current(obj, data, 1);
            ststatus_module.window_anim_start(&obj->anims[2], 1);
            obj->base.step++;
        }
        break;
    case 14:
        if (ststatus_module.window_anim_update(&obj->anims[2])) {
            ststatus_equip_show_list(obj, data, 1);
            data->list_cursor->set_pos(data->list_cursor, 0x89, 0x4B);
            data->list_cursor->show(data->list_cursor, 1);
            obj->status_page->preview_item(obj->status_page, obj->slot, 0);
            if (obj->list_length >= 9) {
                data->scroll_bar = ststatus_create_bar();
                data->scroll_bar->set_x(data->scroll_bar, 0x125, 0xC);
                data->scroll_bar->set_range(data->scroll_bar, 0x4E, 0xB3);
                data->scroll_bar->set_lines(data->scroll_bar, 8, obj->list_length);
                data->scroll_bar->set_line(data->scroll_bar, 0);
            }
            obj->base.step++;
        }
        break;
    case 15:
        old = obj->list_cursor;
        old_scroll = obj->first_line;
        if (obj->list_length >= 9) {
            if ((!PAD_HELD(11) && PAD_PRESSED(10)) || (!PAD_HELD(11) && PAD_REPEAT(10))) {
                obj->first_line -= 8;
                if (obj->first_line < 0) {
                    obj->first_line = 0;
                }
            } else if ((!PAD_HELD(10) && PAD_PRESSED(11)) || (!PAD_HELD(10) && PAD_REPEAT(11))) {
                obj->first_line += 8;
                if (obj->list_length - 8 < obj->first_line) {
                    obj->first_line = obj->list_length - 8;
                }
            }
        }
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            if (--obj->list_cursor < 0) {
                obj->list_cursor = 0;
                if (--obj->first_line < 0) {
                    obj->first_line = 0;
                }
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            if (obj->list_length < 8) {
                obj->list_cursor++;
                if (obj->list_length - 1 < obj->list_cursor) {
                    obj->list_cursor = obj->list_length - 1;
                }
            } else if (++obj->list_cursor >= 8) {
                obj->list_cursor = 7;
                obj->first_line++;
                if (obj->list_length - 8 < obj->first_line) {
                    obj->first_line = obj->list_length - 8;
                }
            }
        }
        if (old != obj->list_cursor) {
            sound_module.play(0x8004513E);
            data->list_cursor->set_pos(data->list_cursor, 0x86, obj->list_cursor * 0xE + 0x4B);
        }
        if (old_scroll != obj->first_line) {
            sound_module.play(0x8004513E);
            ststatus_equip_show_list(obj, data, 1);
            if (data->scroll_bar != NULL) {
                data->scroll_bar->set_line(data->scroll_bar, obj->first_line);
            }
        }
        item = obj->list[obj->list_cursor + obj->first_line];
        if (old != obj->list_cursor || old_scroll != obj->first_line) {
            if (ststatus_module.can_equip(obj->digimon, obj->slot, item)) {
                obj->status_page->preview_item(obj->status_page, obj->slot, item);
            } else {
                obj->status_page->preview_item(obj->status_page, -1, 0);
            }
            ststatus_equip_describe_item(obj, data, item);
        } else if (PAD_PRESSED(13)) {
            if (ststatus_module.can_equip(obj->digimon, obj->slot, item)) {
                ststatus_module.equip_item(obj->digimon, obj->slot, item);
                obj->base.step = 20;
                obj->draw_slot_item = 0;
                obj->status_page->preview_item(obj->status_page, obj->slot, item);
                if (data->scroll_bar != NULL) {
                    data->scroll_bar->base.state = OBJECT_STATE_END;
                }
                sound_module.play(0x8004503C);
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->status_page->preview_item(obj->status_page, -1, 0);
            obj->base.step = 20;
            obj->draw_slot_item = 0;
            if (data->scroll_bar != NULL) {
                data->scroll_bar->base.state = OBJECT_STATE_END;
            }
        }
        break;
    case 20:
        ststatus_module.window_anim_start(&obj->anims[2], 0);
        ststatus_equip_show_list(obj, data, 0);
        data->list_cursor->show(data->list_cursor, 0);
        obj->base.step++;
        break;
    case 21:
        if (ststatus_module.window_anim_update(&obj->anims[2])) {
            ststatus_module.window_anim_start(&obj->anims[3], 0);
            ststatus_equip_show_current(obj, data, 0);
            obj->base.step++;
        }
        break;
    case 22:
        if (ststatus_module.window_anim_update(&obj->anims[3])) {
            obj->base.step = 0;
        }
        break;
    case 50:
        ststatus_module.window_anim_start(&obj->anims[1], 0);
        ststatus_equip_show_slots(obj, data, 0);
        data->slot_cursor->show(data->slot_cursor, 0);
        obj->base.step++;
        break;
    case 11:
    case 51:
        if (ststatus_module.window_anim_update(&obj->anims[1])) {
            data->title->set_visible(data->title, 0);
            ststatus_module.window_anim_start(&obj->anims[0], 0);
            obj->base.step++;
        }
        break;
    case 52:
        if (ststatus_module.window_anim_update(&obj->anims[0])) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}

void ststatus_equip_update(StstatusEquipPage *obj, StstatusEquipPageData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->first_line = 0;
        ststatus_equip_create_windows(obj, data);
        obj->anims[3].duration = 10;
        obj->anims[1].duration = 10;
        obj->anims[2].duration = 10;
        obj->anims[0].duration = 10;
        break;
    case OBJECT_STATE_RUN:
        ststatus_equip_run(obj, data);
        ststatus_equip_draw(obj);
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        obj->status_page->sub_page_open = 0;
        break;
    }
}

/* Creates the equipment page for the status page's member. */
StstatusEquipPage *ststatus_create_equip_page(StstatusStatusPage *parent) {
    StstatusEquipPage *obj = object_new(ststatus_equip_update, sizeof(StstatusEquipPage), sizeof(StstatusEquipPageData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 4;
    obj->status_page = parent;
    obj->digimon = gamestate_data.funcs.get_party_member(parent->member);
    return obj;
}
