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

/* The items page (ststatus_create_items_page): the party's panels, the item categories, and the item list
 * (ststatus_create_item_list) it opens; a usable item is then used on a party member. */

/* An entry of ststatus_stat_raises: an effect that raises a stat, up to a limit (-1 ends the table). */
typedef struct StstatusStatRaise {
    /* 0x0 */ s16 effect; /* effect */
    /* 0x2 */ s16 stat;  /* stat (GamestateStats.values) */
    /* 0x4 */ s16 limit; /* limit */
} StstatusStatRaise; /* size 0x6 */

/* The items page's data block (0xD4 bytes). */
typedef struct StstatusItemsPageData {
    /* 0x00 */ MessageWindow *title;  /* title */
    /* 0x04 */ StstatusPanelWindows panels[3]; /* per party member: its panel */
    /* 0x88 */ MessageWindow *message; /* message, or item description */
    /* 0x8C */ MessageWindow *kind;   /* item kind */
    /* 0x90 */ MessageWindow *choice_1; /* choice */
    /* 0x94 */ MessageWindow *choice_2; /* choice */
    /* 0x98 */ MessageCursor *choice_cursor; /* choice cursor */
    /* 0x9C */ MessageWindow *money;  /* money */
    /* 0xA0 */ MessageWindow *money_label; /* money label */
    /* 0xA4 */ MessageWindow *categories[5]; /* categories */
    /* 0xB8 */ MessageCursor *category_cursor; /* category cursor */
    /* 0xBC */ MessageWindow *item_name; /* item name */
    /* 0xC0 */ MessageWindow *equipped_label; /* "equipped" */
    /* 0xC4 */ MessageWindow *equipped_count; /* equipped count */
    /* 0xC8 */ MessageWindow *held_label; /* "held" */
    /* 0xCC */ MessageWindow *held_count; /* held count */
    /* 0xD0 */ Object *item_list; /* the item list */
} StstatusItemsPageData; /* size 0xD4 */

/* The items page (ststatus_create_items_page, size 0x45C). */
typedef struct StstatusItemsPage {
    /* 0x000 */ Object base; /* base.substep: the panels are already open */
    /* 0x050 */ s32 parent;
    /* 0x054 */ s32 layer_id; /* layer */
    /* 0x058 */ s32 ot_depth; /* ordering table depth */
    /* 0x05C */ s32 member_count; /* party members */
    /* 0x060 */ s32 frames[3]; /* per member: sprite frame */
    /* 0x06C */ s32 frame_time; /* time of the last frame */
    /* 0x070 */ s32 category_cursor; /* category cursor, 0..4 */
    /* 0x074 */ s32 unk_74;
    /* 0x078 */ s32 item;   /* the item */
    /* 0x07C */ s32 list_result; /* the list's result: the item's index, -1 cancelled, -2 */
    /* 0x080 */ s32 counts_shown; /* the item's counts are shown */
    /* 0x084 */ s16 items[0x194];  /* the category's items */
    /* 0x3AC */ s32 item_count; /* their number */
    /* 0x3B0 */ s32 member;  /* the member to use the item on */
    /* 0x3B4 */ s32 member_cursor_shown; /* the member cursor is shown */
    /* 0x3B8 */ s32 member_cursor_frame; /* its frame, 0..7 */
    /* 0x3BC */ s32 member_cursor_time; /* time of its last frame */
    /* 0x3C0 */ s32 arrow_shown; /* the "next" arrow is shown */
    /* 0x3C4 */ s32 arrow_frame; /* its frame, 0..4 */
    /* 0x3C8 */ s32 arrow_time; /* time of its last frame */
    /* 0x3CC */ WindowAnim member_panels[3]; /* member panels */
    /* 0x3FC */ WindowAnim bars[2];    /* title, bottom bar */
    /* 0x41C */ WindowAnim categories_anim; /* categories */
    /* 0x42C */ WindowAnim money_anim; /* money */
    /* 0x43C */ WindowAnim counts_anim; /* the item's counts */
    /* 0x44C */ u8 unk_44C[0x10];
} StstatusItemsPage; /* size 0x45C */

/* The item list's data block (0x6C bytes). */
typedef struct StstatusItemListData {
    /* 0x00 */ MessageWindow *title;  /* title */
    /* 0x04 */ MessageWindow *item_windows[8][2]; /* items, two per line */
    /* 0x44 */ MessageWindow *page;   /* page */
    /* 0x48 */ MessageWindow *slash;  /* "/" */
    /* 0x4C */ MessageWindow *pages;  /* pages */
    /* 0x50 */ MessageWindow *prev_label; /* "previous page" */
    /* 0x54 */ MessageWindow *next_label; /* "next page" */
    /* 0x58 */ MessageCursor *cursor; /* cursor */
    /* 0x5C */ u8 unk_5C[0x10];
} StstatusItemListData; /* size 0x6C */

/* The item list (ststatus_create_item_list, size 0x70C): a category's items, 16 per page. */
typedef struct StstatusItemList {
    /* 0x000 */ Object base;
    /* 0x050 */ StstatusItemsPage *items_page; /* the items page */
    /* 0x054 */ s32 layer_id; /* layer */
    /* 0x058 */ s32 ot_depth; /* ordering table depth */
    /* 0x05C */ s32 category; /* category */
    /* 0x060 */ s32 initial_item; /* item to put the cursor on */
    /* 0x064 */ s32 shown;  /* the list is shown */
    /* 0x068 */ s32 items;  /* items */
    /* 0x06C */ s16 list[0x194];
    /* 0x394 */ s16 buffer[0x194];
    /* 0x6BC */ s32 cursor;  /* cursor */
    /* 0x6C0 */ s32 arrows_frame; /* page arrows' frame, 0..3 */
    /* 0x6C4 */ s32 arrows_time; /* time of their last frame */
    /* 0x6C8 */ s32 page;    /* page */
    /* 0x6CC */ s32 pages;   /* pages */
    /* 0x6D0 */ s32 prev_arrow_shown; /* previous-page arrow shown */
    /* 0x6D4 */ s32 next_arrow_shown; /* next-page arrow shown */
    /* 0x6D8 */ s32 unk_6D8;
    /* 0x6DC */ WindowAnim frame_anim;
    /* 0x6EC */ WindowAnim title_anim;
    /* 0x6FC */ WindowAnim unused_anim; /* nothing starts it: the sprite it scales (0x26) never shows */
} StstatusItemList; /* size 0x70C */

s32 ststatus_items_categories[5] = { 1, 0x80000002, 0x80000003, 0x80000004, 0 };
s32 ststatus_items_panel_stats[5] = { 0, 2, 3, 4, 5 };
s32 ststatus_items_kind_texts[9] = { 0, 67, 77, 79, 65, 66, 68, 80, 68 }; /* per kind: ?STATUS entries */
StstatusStatRaise ststatus_stat_raises[16] = {
    { 2, 3, 9999 },
    { 3, 5, 9999 },
    { 4, 6, 999 },
    { 5, 7, 999 },
    { 6, 8, 999 },
    { 7, 9, 999 },
    { 8, 10, 999 },
    { 9, 11, 999 },
    { 10, 12, 999 },
    { 11, 13, 999 },
    { 12, 14, 999 },
    { 13, 15, 999 },
    { 14, 16, 999 },
    { 15, 17, 999 },
    { 16, 18, 999 },
    { -1, 0, 0 },
};

StstatusItemList *ststatus_create_item_list(StstatusItemsPage *parent, s32 category, s32 item);

/* Creates the page's text windows. */
void ststatus_items_create_windows(StstatusItemsPage *obj, StstatusItemsPageData *data) {
    s32 i;
    s32 j;
    StstatusLayout *l;

    l = &ststatus_module.party_layout[11];
    data->title = message_create_window(obj->layer_id, 1, l->x, l->y);
    l = &ststatus_module.party_layout[12];
    data->message = message_create_window(obj->layer_id, 1, l->x, l->y);
    data->kind = message_create_window(obj->layer_id, 1, l->x, l->y + 0xE);
    data->choice_1 = message_create_window(obj->layer_id, 1, l->x + 0x10, l->y + 0xE);
    data->choice_2 = message_create_window(obj->layer_id, 1, l->x + 0x42, l->y + 0xE);
    data->choice_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, l->x, l->y + 0xE);
    data->choice_cursor->show(data->choice_cursor, 0);
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
    data->money = message_create_window(obj->layer_id, 3, 0x42, 0xA6);
    data->money_label = message_create_window(obj->layer_id, 3, 0x46, 0xA6);
    l = &ststatus_module.party_layout[15];
    for (j = 0; j < 5; j++) {
        data->categories[j] = message_create_window(obj->layer_id, 1, l->x, l->y + j * 0xE);
    }
    data->category_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0xB0, obj->category_cursor * 0xE + 0x31);
    data->category_cursor->show(data->category_cursor, 0);
    data->item_name = message_create_window(obj->layer_id, 1, 0x25, 0xAE);
    data->equipped_label = message_create_window(obj->layer_id, 1, 0xA1, 0xAF);
    data->equipped_count = message_create_window(obj->layer_id, 1, 0xE0, 0xAF);
    data->held_label = message_create_window(obj->layer_id, 1, 0xEA, 0xAF);
    data->held_count = message_create_window(obj->layer_id, 1, 0x128, 0xAF);
}

/* Shows (fills in) or hides a member's panel. */
void ststatus_items_show_member(StstatusItemsPage *obj, StstatusItemsPageData *data, s32 member, s32 show) {
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
            win->set_line_number(win, 0, stats.values[ststatus_items_panel_stats[i]]);
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

/* Shows or hides the money. */
void ststatus_items_show_money(StstatusItemsPage *obj, StstatusItemsPageData *data, s32 show) {
    if (show) {
        data->money_label->set_text(data->money_label, cdload_module.files.get_file(records_language + 0xB0), 5);
        data->money->set_line_number(data->money, 0, gamestate_data.money);
        data->money->measure(data->money, 1);
    } else {
        data->money_label->set_visible(data->money_label, 0);
        data->money->set_visible(data->money, 0);
    }
}

/* Shows or hides the categories. */
void ststatus_items_show_categories(StstatusItemsPage *obj, StstatusItemsPageData *data, s32 show) {
    s32 i;
    StstatusLayout *l;

    if (show) {
        l = &ststatus_module.party_layout[15];
        for (i = 0; i < 5; i++) {
            data->categories[i]->set_text(data->categories[i], cdload_module.files.get_file(records_language + 0xB0), l->text + i);
        }
    } else {
        for (i = 0; i < 5; i++) {
            data->categories[i]->set_visible(data->categories[i], 0);
        }
    }
}

/* Shows (fills in) or hides the item's name and counts. */
void ststatus_items_show_item(StstatusItemsPage *obj, s32 show) {
    StstatusItemsPageData *data = (StstatusItemsPageData *)obj->base.children;

    if (show) {
        data->item_name->set_text(data->item_name, cdload_module.files.get_file(records_language + 0x6A), obj->item);
        data->equipped_label->set_text(data->equipped_label, cdload_module.files.get_file(records_language + 0xB0), 0x20);
        data->equipped_count->set_line_number(data->equipped_count, 0, gamestate_data.items_equipped[obj->item]);
        data->equipped_count->measure(data->equipped_count, 1);
        data->held_label->set_text(data->held_label, cdload_module.files.get_file(records_language + 0xB0), 0x16);
        data->held_count->set_line_number(data->held_count, 0, gamestate_data.items[obj->item]);
        data->held_count->measure(data->held_count, 1);
        obj->counts_shown = 1;
    } else {
        data->item_name->set_visible(data->item_name, 0);
        data->equipped_label->set_visible(data->equipped_label, 0);
        data->equipped_count->set_visible(data->equipped_count, 0);
        data->held_label->set_visible(data->held_label, 0);
        data->held_count->set_visible(data->held_count, 0);
        obj->counts_shown = 0;
    }
}

/* The description line: 1 the item's description and kind, 2 the use/cancel choice, -2 hides the choice,
 * -1 the kind, else everything. */
void ststatus_items_show_description(StstatusItemsPage *obj, s32 mode) {
    StstatusItemsPageData *data = (StstatusItemsPageData *)obj->base.children;
    RecordsItem *rec;
    RecordsEquip *e;

    if (mode == 1) {
        data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x63), obj->item);
        rec = records_funcs.get_item(obj->item);
        if (rec->type >= 2 && rec->type < 15) {
            e = rec->data;
            data->kind->set_text(data->kind, cdload_module.files.get_file(records_language + 0xB0), ststatus_items_kind_texts[e->slot]);
        }
    } else if (mode == -1) {
        data->kind->set_visible(data->kind, 0);
    } else if (mode == 2) {
        data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x31);
        data->choice_1->set_text(data->choice_1, cdload_module.files.get_file(records_language + 0xB0), 0x32);
        data->choice_2->set_text(data->choice_2, cdload_module.files.get_file(records_language + 0xB0), 0x33);
        data->choice_cursor->show(data->choice_cursor, 1);
    } else if (mode == -2) {
        data->choice_1->set_visible(data->choice_1, 0);
        data->choice_2->set_visible(data->choice_2, 0);
        data->choice_cursor->show(data->choice_cursor, 0);
    } else {
        data->message->set_visible(data->message, 0);
        data->kind->set_visible(data->kind, 0);
    }
}

/* Opens (or closes, hiding its texts) the item's counts. */
void ststatus_items_open_counts(StstatusItemsPage *obj, s32 open) {
    if (open) {
        ststatus_module.window_anim_start(&obj->counts_anim, 1);
    } else {
        ststatus_module.window_anim_start(&obj->counts_anim, 0);
        ststatus_items_show_item(obj, 0);
        ststatus_items_show_description(obj, 0);
    }
}

s32 ststatus_items_update_counts(StstatusItemsPage *obj) {
    return ststatus_module.window_anim_update(&obj->counts_anim) != 0;
}

/* Uses the item on the member: heals HP, raises GamestateStats.values[1], or raises a stat by a random amount. */
void ststatus_items_use(StstatusItemsPage *obj, StstatusItemsPageData *data) {
    RecordsUsable *e;
    GamestateRecord *rec;
    StstatusStatRaise *t;
    GamestateStats *stats;
    s32 result;
    s32 amount;
    s32 i;

    amount = 0;
    e = records_funcs.get_item(obj->item)->data;
    rec = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(obj->member));
    result = 0;
    switch (e->effect) {
    case 0:
        break;
    case 1:
        if (rec->stats.values[2] < rec->stats.values[3]) {
            rec->stats.values[2] += e->amount;
            amount = e->amount;
            if (rec->stats.values[3] < rec->stats.values[2]) {
                rec->stats.values[2] = rec->stats.values[3];
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x52);
                result = 2;
            } else {
                result = 1;
            }
        }
        break;
    case 17:
        if (rec->stats.values[1] < 99) {
            rec->stats.values[1] += e->amount;
            amount = e->amount;
            result = 1;
            if (rec->stats.values[1] >= 100) {
                rec->stats.values[1] = 99;
            }
        }
        break;
    default:
        for (i = 0; ststatus_stat_raises[i].effect != -1; i++) {
            t = &ststatus_stat_raises[i];
            if (e->effect == t->effect) {
                stats = &rec->stats;
                if (*(stats->values + t->stat) < t->limit) {
                    amount = pad_random.next() % e->amount + 1;
                    *(stats->values + t->stat) += amount;
                    if (*(stats->values + t->stat) > t->limit) {
                        *(stats->values + t->stat) = t->limit;
                    }
                    result = 1;
                }
                break;
            }
        }
        break;
    }
    if (result != 0) {
        if (result == 1) {
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), e->effect + 0x52);
            data->message->set_line_number(data->message, 1, amount);
        }
        gamestate_data.items[obj->item]--;
        ststatus_items_show_member(obj, data, obj->member, 1);
        sound_module.play(0x40014);
    } else {
        data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x6A);
        sound_module.play(0x4001C);
    }
}

/* Draws the members' sprites, the frames, the item icon and the member cursor. */
void ststatus_items_draw(StstatusItemsPage *obj) {
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
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
            }
            digimon = gamestate_data.funcs.get_party_member(i);
            spr.set_vram_pos(0x280, 0x100);
            spr.draw(cdload_module.get_subfile_by_id(0x04040000), ststatus_module.anims[digimon].frame[obj->frames[i]],
                       0x6B, i * 0x2E + 0x13);
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
    if (obj->bars[0].level != 0) {
        if (obj->bars[0].level != 0x1000) {
            spr.set_scale(obj->bars[0].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x19);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x18, 0x22, 0xD);
    }
    if (obj->categories_anim.level != 0) {
        if (obj->categories_anim.level != 0x1000) {
            spr.set_scale(obj->categories_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x52);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x1C, 0xA8, 0x28);
    }
    if (obj->money_anim.level != 0) {
        if (obj->money_anim.level != 0x1000) {
            spr.set_scale(obj->money_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0, 0xA8);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x1A, 0, 0x9E);
    }
    if (obj->bars[1].level != 0) {
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
        if (obj->bars[1].level != 0x1000) {
            spr.set_scale(obj->bars[1].level, 0x1000, 0x1000);
            spr.set_pivot(0, 0xD3);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x20, 0, 0xC2);
    }
    if (obj->counts_anim.level != 0) {
        if (obj->counts_anim.level != 0x1000) {
            spr.set_scale(obj->counts_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0xB2);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
            spr.set_vram_pos(0x140, 0);
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_funcs.get_item_icon(obj->item), 0x16, 0xAE);
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x2F, 0xF, 0xA5);
    }
    if (obj->member_cursor_shown != 0) {
        if (gfx_module.funcs.get_time() - obj->member_cursor_time >= 9) {
            obj->member_cursor_time = gfx_module.funcs.get_time();
            if (++obj->member_cursor_frame >= 8) {
                obj->member_cursor_frame = 0;
            }
        }
        spr.set_scale(0x1000, 0x1000, 0x1000);
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
        spr.set_palette(obj->member_cursor_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x1E, 0, obj->member * 0x2E + 0x11);
    }
}

/* The page's states: open, pick a category, open its item list, use an item on a member, close. */
void ststatus_items_run(StstatusItemsPage *obj, StstatusItemsPageData *data) {
    s32 old;
    s32 i;
    s32 j;
    s32 done;

    switch (obj->base.step) {
    case 0:
    default:
        switch (obj->member_count) {
        case 1:
        default:
            ststatus_module.window_anim_start(&obj->member_panels[0], 1);
            ststatus_module.window_anim_start(&obj->bars[0], 1);
            if (obj->base.substep == 0) {
                ststatus_module.window_anim_start(&obj->bars[1], 1);
            }
            ststatus_module.window_anim_start(&obj->money_anim, 1);
            ststatus_module.window_anim_start(&obj->categories_anim, 1);
            break;
        case 2:
        case 3:
            ststatus_module.window_anim_start(&obj->member_panels[0], 1);
            ststatus_module.window_anim_start(&obj->bars[0], 1);
            break;
        }
        obj->base.step = obj->member_count;
        break;
    case 1:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        ststatus_module.window_anim_update(&obj->bars[0]);
        if (obj->base.substep == 0) {
            ststatus_module.window_anim_update(&obj->bars[1]);
        }
        ststatus_module.window_anim_update(&obj->money_anim);
        if (ststatus_module.window_anim_update(&obj->categories_anim)) {
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x14);
            ststatus_items_show_member(obj, data, 0, 1);
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), obj->category_cursor + 0x1B);
            data->kind->set_text(data->kind, cdload_module.files.get_file(records_language + 0xB0), 0x15);
            ststatus_items_show_money(obj, data, 1);
            ststatus_items_show_categories(obj, data, 1);
            data->category_cursor->show(data->category_cursor, 1);
            obj->base.step = 10;
        }
        break;
    case 2:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        if (ststatus_module.window_anim_update(&obj->bars[0])) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 1);
            if (obj->base.substep == 0) {
                ststatus_module.window_anim_start(&obj->bars[1], 1);
            }
            ststatus_module.window_anim_start(&obj->money_anim, 1);
            ststatus_module.window_anim_start(&obj->categories_anim, 1);
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x14);
            ststatus_items_show_member(obj, data, 0, 1);
            obj->base.step = 4;
        }
        break;
    case 4:
        ststatus_module.window_anim_update(&obj->member_panels[1]);
        if (obj->base.substep == 0) {
            ststatus_module.window_anim_update(&obj->bars[1]);
        }
        ststatus_module.window_anim_update(&obj->money_anim);
        if (ststatus_module.window_anim_update(&obj->categories_anim)) {
            ststatus_items_show_member(obj, data, 1, 1);
            ststatus_items_show_categories(obj, data, 1);
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), obj->category_cursor + 0x1B);
            data->kind->set_text(data->kind, cdload_module.files.get_file(records_language + 0xB0), 0x15);
            ststatus_items_show_money(obj, data, 1);
            data->category_cursor->show(data->category_cursor, 1);
            obj->base.step = 10;
        }
        break;
    case 3:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        if (ststatus_module.window_anim_update(&obj->bars[0])) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 1);
            ststatus_module.window_anim_start(&obj->categories_anim, 1);
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x14);
            ststatus_items_show_member(obj, data, 0, 1);
            obj->base.step = 5;
        }
        break;
    case 5:
        ststatus_module.window_anim_update(&obj->member_panels[1]);
        if (ststatus_module.window_anim_update(&obj->categories_anim)) {
            ststatus_module.window_anim_start(&obj->member_panels[2], 1);
            if (obj->base.substep == 0) {
                ststatus_module.window_anim_start(&obj->bars[1], 1);
            }
            ststatus_module.window_anim_start(&obj->money_anim, 1);
            ststatus_items_show_member(obj, data, 1, 1);
            ststatus_items_show_categories(obj, data, 1);
            obj->base.step++;
        }
        break;
    case 6:
        ststatus_module.window_anim_update(&obj->member_panels[2]);
        if (obj->base.substep == 0) {
            ststatus_module.window_anim_update(&obj->bars[1]);
        }
        if (ststatus_module.window_anim_update(&obj->money_anim)) {
            ststatus_items_show_member(obj, data, 2, 1);
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), obj->category_cursor + 0x1B);
            data->kind->set_text(data->kind, cdload_module.files.get_file(records_language + 0xB0), 0x15);
            data->category_cursor->show(data->category_cursor, 1);
            ststatus_items_show_money(obj, data, 1);
            obj->base.step = 10;
        }
        break;
    case 10:
        old = obj->category_cursor;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            if (--obj->category_cursor < 0) {
                obj->category_cursor = 0;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            if (++obj->category_cursor >= 5) {
                obj->category_cursor = 4;
            }
        }
        if (old != obj->category_cursor) {
            sound_module.play(0x8004513E);
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), obj->category_cursor + 0x1B);
            data->category_cursor->set_pos(data->category_cursor, 0xB0, obj->category_cursor * 0xE + 0x31);
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x8004503C);
            obj->item_count = records_funcs.list_items(ststatus_items_categories[obj->category_cursor], (u16 *)obj->items);
            if (obj->item_count <= 0) {
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), obj->category_cursor + 0x64);
            } else {
                obj->base.step = 11;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step = 50;
        }
        break;
    case 11:
        for (i = 0; i < obj->member_count; i++) {
            ststatus_module.window_anim_start(&obj->member_panels[i], 0);
            ststatus_items_show_member(obj, data, i, 0);
        }
        ststatus_module.window_anim_start(&obj->bars[0], 0);
        data->title->set_visible(data->title, 0);
        data->message->set_visible(data->message, 0);
        data->kind->set_visible(data->kind, 0);
        ststatus_module.window_anim_start(&obj->money_anim, 0);
        data->money_label->set_visible(data->money_label, 0);
        data->money->set_visible(data->money, 0);
        ststatus_module.window_anim_start(&obj->categories_anim, 0);
        for (i = 0; i < 5; i++) {
            data->categories[i]->set_visible(data->categories[i], 0);
        }
        data->category_cursor->show(data->category_cursor, 0);
        obj->base.step++;
        break;
    case 12:
        for (j = 0; j < obj->member_count; j++) {
            ststatus_module.window_anim_update(&obj->member_panels[j]);
        }
        ststatus_module.window_anim_update(&obj->bars[0]);
        ststatus_module.window_anim_update(&obj->money_anim);
        if (ststatus_module.window_anim_update(&obj->categories_anim)) {
            data->item_list = (Object *)ststatus_create_item_list(obj, obj->category_cursor, 0);
            ststatus_items_open_counts(obj, 1);
            obj->base.step = 15;
        }
        break;
    case 15:
        if (ststatus_items_update_counts(obj)) {
            obj->base.step++;
        }
        break;
    case 16:
        if (data->item_list == NULL) {
            if (obj->list_result == -1) {
                obj->base.step = 0;
                obj->base.substep = 1;
            } else if (obj->list_result == -2) {
                obj->base.step = 40;
            } else {
                obj->base.set_step(obj, 20);
            }
        }
        break;
    case 20:
        ststatus_module.window_anim_start(&obj->member_panels[0], 1);
        ststatus_module.window_anim_start(&obj->bars[0], 1);
        obj->base.step++;
        break;
    case 21:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        if (ststatus_module.window_anim_update(&obj->bars[0])) {
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x27);
            ststatus_items_show_member(obj, data, 0, 1);
            if (obj->member_count != 1) {
                ststatus_module.window_anim_start(&obj->member_panels[1], 1);
                obj->base.step++;
            } else {
                obj->base.step = 24;
            }
        }
        break;
    case 22:
        if (ststatus_module.window_anim_update(&obj->member_panels[1])) {
            ststatus_items_show_member(obj, data, 1, 1);
            if (obj->member_count == 2) {
                obj->base.step = 24;
            } else {
                ststatus_module.window_anim_start(&obj->member_panels[2], 1);
                obj->base.step++;
            }
        }
        break;
    case 23:
        if (ststatus_module.window_anim_update(&obj->member_panels[2])) {
            ststatus_items_show_member(obj, data, 2, 1);
            obj->base.step++;
        }
        break;
    case 24:
        obj->member_cursor_shown = 1;
        obj->base.step++;
        break;
    case 25:
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
            ststatus_items_open_counts(obj, 0);
            ststatus_module.window_anim_start(&obj->bars[0], 0);
            data->title->set_visible(data->title, 0);
            ststatus_items_use(obj, data);
            obj->base.step = 100;
            obj->base.substep = 0;
            obj->arrow_shown = 1;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.substep = 1;
            obj->base.step++;
            ststatus_module.window_anim_start(&obj->bars[0], 0);
            data->title->set_visible(data->title, 0);
        }
        break;
    case 26:
        obj->member_cursor_shown = 0;
        done = ststatus_module.window_anim_update(&obj->bars[0]);
        if (ststatus_items_update_counts(obj) && done) {
            obj->base.step++;
        }
        break;
    case 27:
        switch (obj->member_count) {
        case 1:
        default:
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_items_show_member(obj, data, 0, 0);
            data->title->set_visible(data->title, 0);
            obj->base.step = 30;
            break;
        case 2:
            ststatus_module.window_anim_start(&obj->member_panels[1], 0);
            ststatus_items_show_member(obj, data, 1, 0);
            obj->base.step = 29;
            break;
        case 3:
            ststatus_module.window_anim_start(&obj->member_panels[2], 0);
            ststatus_items_show_member(obj, data, 2, 0);
            obj->base.step = 28;
            break;
        }
        break;
    case 28:
        if (ststatus_module.window_anim_update(&obj->member_panels[2])) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 0);
            ststatus_items_show_member(obj, data, 1, 0);
            obj->base.step++;
        }
        break;
    case 29:
        if (ststatus_module.window_anim_update(&obj->member_panels[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_items_show_member(obj, data, 0, 0);
            data->title->set_visible(data->title, 0);
            obj->base.step++;
        }
        break;
    case 30:
        if (ststatus_module.window_anim_update(&obj->member_panels[0])) {
            if (obj->base.substep == 0) {
                ststatus_items_open_counts(obj, 1);
            }
            data->item_list = (Object *)ststatus_create_item_list(obj, obj->category_cursor, obj->item);
            obj->base.step = 15;
        }
        break;
    case 100:
        obj->member_cursor_shown = 0;
        ststatus_module.window_anim_update(&obj->bars[0]);
        if (ststatus_items_update_counts(obj)) {
            obj->base.step++;
        }
        break;
    case 101:
        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            obj->item_count = records_funcs.list_items(ststatus_items_categories[obj->category_cursor], (u16 *)obj->items);
            if (obj->item_count <= 0) {
                obj->base.next_step(obj);
                ststatus_module.window_anim_start(&obj->bars[0], 1);
                ststatus_module.window_anim_start(&obj->money_anim, 1);
                ststatus_module.window_anim_start(&obj->categories_anim, 1);
                obj->arrow_shown = 0;
            } else {
                obj->base.step = 27;
                obj->arrow_shown = 0;
            }
        }
        break;
    case 102:
        ststatus_module.window_anim_update(&obj->bars[0]);
        ststatus_module.window_anim_update(&obj->money_anim);
        if (ststatus_module.window_anim_update(&obj->categories_anim)) {
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x14);
            ststatus_items_show_categories(obj, data, 1);
            ststatus_items_show_money(obj, data, 1);
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), obj->category_cursor + 0x1B);
            data->kind->set_text(data->kind, cdload_module.files.get_file(records_language + 0xB0), 0x15);
            data->category_cursor->show(data->category_cursor, 1);
            obj->base.step = 10;
        }
        break;
    case 110:
        data->category_cursor->stop(data->category_cursor, 1);
        data->category_cursor->set_palette(data->category_cursor, 7);
        data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), obj->category_cursor + 0x64);
        obj->base.step++;
        break;
    case 111:
        if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            data->category_cursor->stop(data->category_cursor, 0);
            data->category_cursor->set_palette(data->category_cursor, 0);
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), obj->category_cursor + 0x1B);
            obj->base.step = 10;
        }
        break;
    case 50:
        switch (obj->member_count) {
        case 1:
        default:
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_module.window_anim_start(&obj->bars[0], 0);
            ststatus_module.window_anim_start(&obj->bars[1], 0);
            ststatus_module.window_anim_start(&obj->money_anim, 0);
            ststatus_module.window_anim_start(&obj->categories_anim, 0);
            ststatus_items_show_member(obj, data, 0, 0);
            data->title->set_visible(data->title, 0);
            data->message->set_visible(data->message, 0);
            data->kind->set_visible(data->kind, 0);
            data->money_label->set_visible(data->money_label, 0);
            data->money->set_visible(data->money, 0);
            ststatus_items_show_categories(obj, data, 0);
            break;
        case 2:
            ststatus_module.window_anim_start(&obj->member_panels[1], 0);
            ststatus_module.window_anim_start(&obj->bars[1], 0);
            ststatus_module.window_anim_start(&obj->money_anim, 0);
            ststatus_module.window_anim_start(&obj->categories_anim, 0);
            ststatus_items_show_member(obj, data, 1, 0);
            data->message->set_visible(data->message, 0);
            data->kind->set_visible(data->kind, 0);
            data->money_label->set_visible(data->money_label, 0);
            data->money->set_visible(data->money, 0);
            ststatus_items_show_categories(obj, data, 0);
            break;
        case 3:
            ststatus_module.window_anim_start(&obj->member_panels[2], 0);
            ststatus_module.window_anim_start(&obj->bars[1], 0);
            ststatus_module.window_anim_start(&obj->money_anim, 0);
            ststatus_items_show_member(obj, data, 2, 0);
            data->message->set_visible(data->message, 0);
            data->kind->set_visible(data->kind, 0);
            data->money_label->set_visible(data->money_label, 0);
            data->money->set_visible(data->money, 0);
            break;
        }
        data->category_cursor->show(data->category_cursor, 0);
        obj->base.step = obj->member_count + 50;
        break;
    case 51:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        ststatus_module.window_anim_update(&obj->bars[0]);
        ststatus_module.window_anim_update(&obj->bars[1]);
        ststatus_module.window_anim_update(&obj->money_anim);
        if (ststatus_module.window_anim_update(&obj->categories_anim)) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 40:
        obj->base.state = OBJECT_STATE_END;
        break;
    case 52:
        ststatus_module.window_anim_update(&obj->member_panels[1]);
        ststatus_module.window_anim_update(&obj->bars[1]);
        ststatus_module.window_anim_update(&obj->money_anim);
        if (ststatus_module.window_anim_update(&obj->categories_anim)) {
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_module.window_anim_start(&obj->bars[0], 0);
            data->title->set_visible(data->title, 0);
            ststatus_items_show_member(obj, data, 0, 0);
            obj->base.step = 54;
        }
        break;
    case 54:
        ststatus_module.window_anim_update(&obj->member_panels[1]);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 53:
        ststatus_module.window_anim_update(&obj->member_panels[2]);
        ststatus_module.window_anim_update(&obj->bars[1]);
        if (ststatus_module.window_anim_update(&obj->money_anim)) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 0);
            ststatus_module.window_anim_start(&obj->categories_anim, 0);
            ststatus_items_show_member(obj, data, 1, 0);
            ststatus_items_show_categories(obj, data, 0);
            obj->base.step = 55;
        }
        break;
    case 55:
        ststatus_module.window_anim_update(&obj->member_panels[1]);
        if (ststatus_module.window_anim_update(&obj->categories_anim)) {
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_module.window_anim_start(&obj->bars[0], 0);
            data->title->set_visible(data->title, 0);
            ststatus_items_show_member(obj, data, 0, 0);
            obj->base.step++;
        }
        break;
    case 56:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        if (ststatus_module.window_anim_update(&obj->bars[0])) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}

void ststatus_items_update(StstatusItemsPage *obj, StstatusItemsPageData *data) {
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
        for (i = 0; i < 2; i++) {
            obj->bars[i].duration = 10;
        }
        obj->categories_anim.duration = 10;
        obj->money_anim.duration = 10;
        obj->counts_anim.duration = 10;
        ststatus_items_create_windows(obj, data);
        break;
    case OBJECT_STATE_RUN:
        ststatus_items_run(obj, data);
        ststatus_items_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Creates the items page. */
StstatusItemsPage *ststatus_create_items_page(s32 arg0) {
    StstatusItemsPage *obj = object_new(ststatus_items_update, sizeof(StstatusItemsPage), sizeof(StstatusItemsPageData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->parent = arg0;
    return obj;
}

/* Creates the item list's text windows. */
void ststatus_item_list_create_windows(StstatusItemList *obj, StstatusItemListData *data) {
    s32 i;
    s32 j;

    data->title = message_create_window(obj->layer_id, 1, 0x13, 0x14);
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 2; j++) {
            data->item_windows[i][j] = message_create_window(obj->layer_id, 1, j * 0x83 + 0x38, i * 0xE + 0x25);
            data->item_windows[i][j]->set_ot_depth(data->item_windows[i][j], obj->ot_depth - 1);
        }
    }
    data->page = message_create_window(obj->layer_id, 1, 0x9B, 0x9C);
    data->slash = message_create_window(obj->layer_id, 1, 0x9E, 0x9C);
    data->pages = message_create_window(obj->layer_id, 1, 0xB2, 0x9C);
    data->prev_label = message_create_window(obj->layer_id, 1, 0x2D, 0x97);
    data->next_label = message_create_window(obj->layer_id, 1, 0x102, 0x97);
    data->cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, obj->cursor % 2 * 0x83 + 0x1E,
                                 obj->cursor % 16 / 2 * 0xE + 0x25);
    data->cursor->show(data->cursor, 0);
}

/* Shows (fills in) or hides the page's items and the page arrows' labels. */
void ststatus_item_list_show_items(StstatusItemList *obj, StstatusItemListData *data, s32 show) {
    s32 i;
    s32 j;
    s32 k;

    if (show) {
        data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), obj->category + 0x16);
        k = obj->page * 16;
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 2; j++) {
                if (obj->list[k + i * 2 + j] != 0) {
                    data->item_windows[i][j]->set_text(data->item_windows[i][j], cdload_module.files.get_file(records_language + 0x6A),
                                                obj->list[k + i * 2 + j]);
                } else {
                    data->item_windows[i][j]->set_visible(data->item_windows[i][j], 0);
                }
            }
        }
        if (obj->page == 0) {
            obj->prev_arrow_shown = 0;
            data->prev_label->set_visible(data->prev_label, 0);
        } else {
            obj->prev_arrow_shown = 1;
            data->prev_label->set_text(data->prev_label, cdload_module.files.get_file(records_language + 0xB0), 0x22);
        }
        if (obj->page != obj->pages - 1) {
            obj->next_arrow_shown = 1;
            data->next_label->set_text(data->next_label, cdload_module.files.get_file(records_language + 0xB0), 0x23);
            return;
        }
    } else {
        data->title->set_visible(data->title, 0);
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 2; j++) {
                data->item_windows[i][j]->set_visible(data->item_windows[i][j], 0);
            }
        }
        obj->prev_arrow_shown = 0;
        data->prev_label->set_visible(data->prev_label, 0);
    }
    obj->next_arrow_shown = 0;
    data->next_label->set_visible(data->next_label, 0);
}

/* Shows the current page: its arrows, items, page numbers and the item under the cursor. */
void ststatus_item_list_show_page(StstatusItemList *obj, StstatusItemListData *data) {
    if (obj->page == 0) {
        obj->prev_arrow_shown = 0;
        data->prev_label->set_visible(data->prev_label, 0);
    } else {
        obj->prev_arrow_shown = 1;
        data->prev_label->set_text(data->prev_label, cdload_module.files.get_file(records_language + 0xB0), 0x22);
    }
    if (obj->page == obj->pages - 1) {
        obj->next_arrow_shown = 0;
        data->next_label->set_visible(data->next_label, 0);
    } else {
        obj->next_arrow_shown = 1;
        data->next_label->set_text(data->next_label, cdload_module.files.get_file(records_language + 0xB0), 0x23);
    }
    ststatus_item_list_show_items(obj, data, 1);
    obj->items_page->item = obj->list[obj->cursor];
    ststatus_items_show_item(obj->items_page, 1);
    ststatus_items_show_description(obj->items_page, 1);
    data->page->set_line_number(data->page, 0, obj->page + 1);
    data->page->measure(data->page, 1);
    data->slash->set_text(data->slash, cdload_module.files.get_file(records_language + 0xB0), 4);
    data->pages->set_line_number(data->pages, 0, obj->pages);
    data->pages->measure(data->pages, 1);
}

/* Moves the page or the cursor; 1 if it moved. */
s32 ststatus_item_list_move(StstatusItemList *obj, StstatusItemListData *data) {
    s32 old_page;
    s32 old;
    s32 last;
    s32 first;

    old_page = obj->page;
    old = obj->cursor;
    if ((!PAD_HELD(11) && PAD_PRESSED(10)) || (!PAD_HELD(11) && PAD_REPEAT(10))) {
        if (--obj->page < 0) {
            obj->page = 0;
        }
    } else if ((!PAD_HELD(10) && PAD_PRESSED(11)) || (!PAD_HELD(10) && PAD_REPEAT(11))) {
        obj->page++;
        if (obj->pages - 1 < obj->page) {
            obj->page = obj->pages - 1;
        }
    }
    if (old_page != obj->page) {
        sound_module.play(0x8004513E);
        obj->cursor = obj->page * 16;
        data->cursor->set_pos(data->cursor, obj->cursor % 2 * 0x83 + 0x1E, obj->cursor % 16 / 2 * 0xE + 0x25);
        ststatus_item_list_show_page(obj, data);
        return 1;
    }
    last = (old_page + 1) * 16 - 1;
    first = old_page * 16;
    if (obj->items - 1 < last) {
        last = obj->items - 1;
    }
    if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
        obj->cursor -= 2;
        if (obj->cursor < first) {
            obj->cursor = first;
        }
    } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
        obj->cursor += 2;
        if (last < obj->cursor) {
            obj->cursor = last;
        }
    }
    if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
        if (!PAD_HELD(6)) {
            if (--obj->cursor < first) {
                obj->cursor = first;
            }
        }
    } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
        if (!PAD_HELD(4)) {
            obj->cursor++;
            if (last < obj->cursor) {
                obj->cursor = last;
            }
        }
    }
    if (old != obj->cursor) {
        sound_module.play(0x8004513E);
        data->cursor->set_pos(data->cursor, obj->cursor % 2 * 0x83 + 0x1E, obj->cursor % 16 / 2 * 0xE + 0x25);
        obj->items_page->item = obj->list[obj->cursor];
        ststatus_items_show_item(obj->items_page, 1);
        ststatus_items_show_description(obj->items_page, 1);
        return 1;
    }
    return 0;
}

/* The list's states: open, pick an item (usable ones go back to the items page to be used), close. */
void ststatus_item_list_run(StstatusItemList *obj, StstatusItemListData *data) {
    switch (obj->base.step) {
    case 0:
    default:
        obj->frame_anim.duration = 10;
        obj->title_anim.duration = 10;
        ststatus_module.window_anim_start(&obj->frame_anim, 1);
        ststatus_module.window_anim_start(&obj->title_anim, 1);
        obj->base.step++;
        break;
    case 1:
        ststatus_module.window_anim_update(&obj->frame_anim);
        if (ststatus_module.window_anim_update(&obj->title_anim)) {
            data->cursor->show(data->cursor, 1);
            ststatus_item_list_show_page(obj, data);
            data->cursor->set_pos(data->cursor, obj->cursor % 2 * 0x83 + 0x1E, obj->cursor % 16 / 2 * 0xE + 0x25);
            obj->shown = 1;
            obj->base.step++;
        }
        break;
    case 2:
        if (!ststatus_item_list_move(obj, data)) {
            if (PAD_PRESSED(13)) {
                if (obj->category == 0) {
                    obj->items_page->list_result = obj->cursor;
                    obj->items_page->item = obj->list[obj->cursor];
                    if (((RecordsUsable *)records_funcs.get_item(obj->items_page->item)->data)->flags & 1) {
                        sound_module.play(0x8004503C);
                        ststatus_items_show_description(obj->items_page, -1);
                        obj->base.step++;
                    }
                }
            } else if (PAD_PRESSED(14)) {
                sound_module.play(0x800450BD);
                obj->items_page->list_result = -1;
                ststatus_items_open_counts(obj->items_page, 0);
                obj->base.step++;
            }
        }
        break;
    case 3:
        ststatus_item_list_show_items(obj, data, 0);
        data->cursor->show(data->cursor, 0);
        data->page->set_visible(data->page, 0);
        data->slash->set_visible(data->slash, 0);
        data->pages->set_visible(data->pages, 0);
        ststatus_module.window_anim_start(&obj->frame_anim, 0);
        ststatus_module.window_anim_start(&obj->title_anim, 0);
        obj->prev_arrow_shown = 0;
        obj->next_arrow_shown = 0;
        obj->shown = 0;
        obj->base.step++;
        break;
    case 4:
        ststatus_items_update_counts(obj->items_page);
        ststatus_module.window_anim_update(&obj->frame_anim);
        if (ststatus_module.window_anim_update(&obj->title_anim)) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}

/* Draws the page arrows, the item icons and the frames. */
void ststatus_item_list_draw(StstatusItemList *obj) {
    Sprite spr;
    RecordsItem *rec;
    s32 i;
    s32 j;
    s32 k;

    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0x100);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->pages >= 2) {
        if (gfx_module.funcs.get_time() - obj->arrows_time >= 11) {
            obj->arrows_time = gfx_module.funcs.get_time();
            if (++obj->arrows_frame >= 4) {
                obj->arrows_frame = 0;
            }
        }
        spr.set_palette(obj->arrows_frame);
    }
    if (obj->prev_arrow_shown != 0) {
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x34, 0x1E, 0x98);
    }
    if (obj->next_arrow_shown != 0) {
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x35, 0xFD, 0x98);
    }
    spr.set_palette(0);
    if (obj->shown != 0) {
        rec = records_funcs.get_item(obj->list[obj->cursor]);
        if ((rec->type == 0x19 || rec->type == 0x1A) && (*(u8 *)rec->data & 1)) {
            spr.set_vram_pos(0x140, 0);
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x40, obj->cursor % 2 * 0x83 + 0x29,
                       obj->cursor % 16 / 2 * 0xE + 0x25);
        }
        k = obj->page * 16;
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 2; j++) {
                if (obj->list[k + i * 2 + j] != 0) {
                    spr.set_vram_pos(0x140, 0);
                    spr.draw(cdload_module.get_subfile_by_id(0x02860000),
                               records_funcs.get_item_icon(obj->list[k + i * 2 + j]), j * 0x83 + 0x29,
                               i * 0xE + 0x25);
                    spr.set_vram_pos(0x280, 0x100);
                    spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x31, j * 0x83 + 0x29, i * 0xE + 0x25);
                }
            }
        }
    }
    if (obj->title_anim.level != 0x1000) {
        spr.set_scale(obj->title_anim.level, 0x1000, 0x1000);
        spr.set_pivot(0xC, 0x17);
    }
    spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x30, 0xC, 0x12);
    if (obj->frame_anim.level != 0x1000) {
        spr.set_scale(0x1000, obj->frame_anim.level, 0x1000);
        spr.set_pivot(0xA0, 0x60);
    } else {
        spr.set_scale(0x1000, 0x1000, 0x1000);
    }
    spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x2E, 0, 0x1E);
    if (obj->unused_anim.level != 0) {
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 2);
        if (obj->unused_anim.level != 0x1000) {
            spr.set_scale(obj->unused_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x26);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x26, 0xA0, 0x12);
    }
}

void ststatus_item_list_fill(StstatusItemList *obj);

void ststatus_item_list_update(StstatusItemList *obj, StstatusItemListData *data) {
    s32 i;
    s32 found;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        ststatus_item_list_fill(obj);
        if (obj->items / 16 != 0) {
            obj->pages = obj->items / 16 + ((obj->items & 0xF) != 0);
        } else {
            obj->pages = 1;
        }
        found = 0;
        if (obj->initial_item != 0) {
            for (i = 0; obj->list[i] != 0; i++) {
                if (obj->initial_item == obj->list[i]) {
                    obj->cursor = i;
                    obj->page = i / 16;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                if (obj->list[obj->items_page->list_result] > 0) {
                    obj->cursor = obj->items_page->list_result;
                    obj->page = obj->items_page->list_result / 16;
                } else if (obj->items_page->list_result - 1 > 0) {
                    obj->cursor = obj->items_page->list_result - 1;
                    obj->page = obj->cursor / 16;
                }
            }
        }
        ststatus_item_list_create_windows(obj, data);
        break;
    case OBJECT_STATE_RUN:
        ststatus_item_list_run(obj, data);
        ststatus_item_list_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Creates the item list of a category for the items page, the cursor on `item` if it is there. */
StstatusItemList *ststatus_create_item_list(StstatusItemsPage *parent, s32 category, s32 item) {
    StstatusItemList *obj = object_new(ststatus_item_list_update, sizeof(StstatusItemList), sizeof(StstatusItemListData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 3;
    obj->items_page = parent;
    if (item != 0) {
        obj->initial_item = item;
    }
    obj->category = category;
    return obj;
}

/* Fills the list with the category's items (category 0: list ststatus_items_categories[0]'s, copied). */
void ststatus_item_list_fill(StstatusItemList *obj) {
    s32 i;

    if (obj->category == 0) {
        obj->items = records_funcs.list_items(ststatus_items_categories[0], (u16 *)obj->buffer);
        for (i = 0; i < obj->items; i++) {
            obj->list[i] = obj->buffer[i];
        }
    } else {
        obj->items = records_funcs.list_items(ststatus_items_categories[obj->category], (u16 *)obj->list);
    }
}
