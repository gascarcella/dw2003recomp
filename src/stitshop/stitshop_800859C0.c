#include "common.h"
#include "object.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "stitshop.h"

/* Selling, the item list, the item's information (what it does to each party member), the main object and
 * the helpers of stitshop_funcs. */

/* Selling's data block (0x38 bytes). */
typedef struct StitshopSellData {
    /* 0x00 */ StitshopList *list;
    /* 0x04 */ StitshopInfo *info;
    /* 0x08 */ MessageCursor *cursor; /* the cursor */
    /* 0x0C */ MessageWindow *kinds[4];  /* the kinds of item */
    /* 0x1C */ MessageWindow *how_many; /* "how many?" */
    /* 0x20 */ MessageWindow *times;
    /* 0x24 */ MessageWindow *number; /* the number */
    /* 0x28 */ MessageWindow *messages; /* messages */
    /* 0x2C */ MessageWindow *yes;    /* yes */
    /* 0x30 */ MessageWindow *no;     /* no */
    /* 0x34 */ MessageWindow *nothing_to_sell; /* "nothing to sell" */
} StitshopSellData; /* size 0x38 */

/* The item list's data block (0x50 bytes). */
typedef struct StitshopListData {
    /* 0x00 */ MessageWindow *items[14];  /* the items */
    /* 0x38 */ MessageWindow *page;   /* the page */
    /* 0x3C */ MessageWindow *slash;  /* "/" */
    /* 0x40 */ MessageWindow *pages;  /* the pages */
    /* 0x44 */ MessageWindow *prev_label; /* previous page */
    /* 0x48 */ MessageWindow *next_label; /* next page */
    /* 0x4C */ MessageCursor *cursor; /* the cursor */
} StitshopListData; /* size 0x50 */

/* A party member's windows in the item's information. */
typedef struct StitshopMemberWindows {
    /* 0x00 */ MessageWindow *name;   /* name */
    /* 0x04 */ MessageWindow *arrows[2];
    /* 0x0C */ MessageWindow *stats[8];  /* stats: two now/with the item pairs, then the other changes */
} StitshopMemberWindows; /* size 0x2C */

/* The item's information's data block (0xAC bytes). */
typedef struct StitshopInfoData {
    /* 0x00 */ MessageWindow *item_name; /* the item's name */
    /* 0x04 */ MessageWindow *equipped_label;
    /* 0x08 */ MessageWindow *equipped; /* equipped */
    /* 0x0C */ MessageWindow *owned_label;
    /* 0x10 */ MessageWindow *owned;  /* owned */
    /* 0x14 */ MessageWindow *price_label;
    /* 0x18 */ MessageWindow *price;  /* the price */
    /* 0x1C */ MessageWindow *description; /* the description */
    /* 0x20 */ MessageWindow *slot;
    /* 0x24 */ StitshopMemberWindows members[3];
    /* 0xA8 */ MessageWindow *page;   /* the page */
} StitshopInfoData; /* size 0xAC */

/* The main object's data block (0x24 bytes). */
typedef struct StitshopMainData {
    /* 0x00 */ MessageWindow *shopkeeper; /* the shopkeeper */
    /* 0x04 */ MessageWindow *help;   /* help */
    /* 0x08 */ MessageWindow *money;  /* money */
    /* 0x0C */ MessageWindow *money_label;
    /* 0x10 */ MessageWindow *buy;    /* buy */
    /* 0x14 */ MessageWindow *sell;   /* sell */
    /* 0x18 */ MessageCursor *cursor; /* the cursor */
    /* 0x1C */ Object *dealing; /* buying or selling (cleared when it ends) */
    /* 0x20 */ Fade *fade;
} StitshopMainData; /* size 0x24 */

/* A party member's equipment (GamestateRecord.equipment), copied whole. */
typedef struct StitshopItems {
    /* 0x0 */ s16 items[6];
} StitshopItems; /* size 0xC */

/* Which stat stitshop_info_show_stat and stitshop_info_colour_stat show. */
typedef struct StitshopStatRef {
    /* 0x00 */ s32 member;
    /* 0x04 */ s32 stat;  /* index into stitshop_info_stats */
    /* 0x08 */ s32 which; /* 0: now, 1: with the item */
    /* 0x0C */ s32 skip1; /* stats stitshop_info_show_changes leaves out */
    /* 0x10 */ s32 skip2; /* (-1: none) */
} StitshopStatRef; /* size 0x14 */

/* A shop's goods (stitshop_shops, by shop). */
typedef struct StitshopShop {
    /* 0x0 */ s32 count;
    /* 0x4 */ s16 *items; /* 0-terminated */
} StitshopShop; /* size 0x8 */

/* A position (stitshop_info_mark_pos). */
typedef struct StitshopPos {
    /* 0x0 */ s32 x;
    /* 0x4 */ s32 y;
} StitshopPos; /* size 0x8 */

extern s32 stitshop_sell_kinds[4];
extern StitshopPos stitshop_info_mark_pos[6];
extern s32 stitshop_info_stats[13];
extern s32 stitshop_slot_texts[9];
extern s32 stitshop_shop_names[];
extern StitshopShop stitshop_shops[];

void stitshop_list_collect(StitshopList *obj);
void stitshop_info_add_stat(s16 *stats, s32 type, s32 value);
void stitshop_info_compare(StitshopInfo *obj, StitshopInfoData *data, s32 member);
void stitshop_list_create_windows(StitshopList *obj, StitshopListData *data);
void stitshop_list_draw(StitshopList *obj);
void stitshop_info_show_changes(StitshopInfo *obj, MessageWindow **wins, StitshopStatRef *ref);
void stitshop_info_create_windows(StitshopInfo *obj, StitshopInfoData *data);
void stitshop_info_update(StitshopInfo *obj, StitshopInfoData *data);
void stitshop_info_turn_page(StitshopInfo *obj);
s32 stitshop_get_slot(s32 digimon, s32 item);
void stitshop_equip(s32 digimon, s32 slot, s32 item, s32 take);

/* Creates selling's text windows and cursor. */
void stitshop_sell_create_windows(StitshopSell *obj, StitshopSellData *data) {
    s32 i;
    MessageCursor *cursor;

    data->cursor = cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0, 0xB0);
    cursor->show(cursor, 0);
    for (i = 0; i < 4; i++) {
        data->kinds[i] = message_create_window(obj->layer_id, 1, 0xBE, i * 14 + 0x2F);
    }
    data->how_many = message_create_window(obj->layer_id, 1, 0xB9, 0x3A);
    data->times = message_create_window(obj->layer_id, 1, 0x103, 0x54);
    data->number = message_create_window(obj->layer_id, 1, 0x11E, 0x54);
    data->messages = message_create_window(obj->layer_id, 1, 0x9A, 0x2C);
    data->yes = message_create_window(obj->layer_id, 1, 0xC5, 0x49);
    data->no = message_create_window(obj->layer_id, 1, 0xC5, 0x59);
    data->nothing_to_sell = message_create_window(obj->layer_id, 1, 0x9A, 0x8A);
}

/* Draws selling's windows. */
void stitshop_sell_draw(StitshopSell *obj, StitshopSellData *data) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0x100);
    if (obj->unk_AC.level != 0) {
        spr.set_layer_id(obj->layer_id, 1);
        if (obj->unk_AC.level != 0x1000) {
            spr.set_scale(obj->unk_AC.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x41);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0xD, 0xA0, 0x24);
    }
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->kinds_anim.level != 0) {
        if (obj->kinds_anim.level != 0x1000) {
            spr.set_scale(obj->kinds_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x4F);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 4, 0xA9, 0x26);
    }
    if (obj->count_anim.level != 0) {
        if (obj->count_anim.level != 0x1000) {
            spr.set_scale(obj->count_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x42);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0xA, 0x9A, 0x34);
        if (obj->count_anim.level != 0x1000) {
            spr.set_pivot(0x140, 0x59);
        } else {
            if (gfx_module.funcs.get_time() - obj->arrows_time >= 9) {
                obj->arrows_time = gfx_module.funcs.get_time();
                obj->arrows_shown = 1 - obj->arrows_shown;
            }
            if (obj->arrows_shown != 0) {
                if (obj->count < obj->max) {
                    spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0x12, 0x122, 0x51);
                }
                if (obj->count >= 2) {
                    spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0x13, 0x122, 0x5B);
                }
            }
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0xB, 0xF6, 0x4F);
    }
    if (obj->question_anim.level != 0) {
        if (obj->question_anim.level != 0x1000) {
            spr.set_scale(obj->question_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x32);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0x15, 0x7B, 0x26);
        if (obj->question_anim.level != 0x1000) {
            spr.set_pivot(0x140, 0x57);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0xC, 0xAF, 0x43);
    }
    if (obj->nothing_anim.level != 0) {
        if (obj->nothing_anim.level != 0x1000) {
            spr.set_scale(obj->nothing_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x8F);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0x15, 0x7B, 0x84);
    }
}

/* Selling's steps (base.step): choose a kind of item, an item, how many, confirm, and get paid. */
void stitshop_sell_run(StitshopSell *obj, StitshopSellData *data) {
    s32 old;
    s32 i;
    s32 j;
    s32 money;

    switch (obj->base.step) {
    case 0:
    default:
        stitshop_funcs.anim_start(&obj->kinds_anim, 1);
        obj->base.step++;
        break;
    case 1:
        if (stitshop_funcs.anim_update(&obj->kinds_anim)) {
            for (i = 0; i < 4; i++) {
                data->kinds[i]->set_text(data->kinds[i], cdload_module.files.get_file(records_language + 0x71), i + 0x1D);
            }
            data->cursor->set_pos(data->cursor, 0xB0, obj->kind * 14 + 0x2F);
            data->cursor->show(data->cursor, 1);
            obj->base.step++;
        }
        break;
    case 2:
        old = obj->kind;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            if (--obj->kind < 0) {
                obj->kind = 0;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            if (++obj->kind >= 4) {
                obj->kind = 3;
            }
        }
        if (old != obj->kind) {
            sound_module.play(0x8004513E);
            data->cursor->set_pos(data->cursor, 0xB0, obj->kind * 14 + 0x2F);
        }
        if (PAD_PRESSED(0xD)) {
            sound_module.play(0x8004503C);
            data->cursor->show(data->cursor, 0);
            if (data->list != NULL) {
                data->list->base.state = OBJECT_STATE_END;
            }
            obj->base.step++;
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->base.set_step(obj, 50);
        }
        break;
    case 3:
        if (data->list == NULL) {
            data->list = stitshop_list_create(obj, stitshop_sell_kinds[obj->kind], 1);
            obj->base.step++;
        } else {
            data->list->base.state = OBJECT_STATE_END;
        }
        break;
    case 4:
        if (data->list->base.step == 100) {
            if (data->list->count > 0) {
                obj->base.set_step(obj, 50);
                obj->base.substep = 1;
            } else {
                obj->base.step = 200;
            }
        }
        break;
    case 100:
        data->list->base.step = 1;
        obj->base.step = 101;
        break;
    case 101:
        if (data->list->base.state == OBJECT_STATE_DONE) {
            if (data->info == NULL) {
                data->info = stitshop_info_create(1, data->list->get_item(data->list));
            }
            obj->base.step = 5;
        }
        break;
    case 5:
        if (data->info->base.step == 3) {
            data->list->set_input(data->list, 1);
            obj->base.step++;
        }
        break;
    case 6:
        if (PAD_PRESSED(0xD)) {
            sound_module.play(0x8004503C);
            obj->base.step = 20;
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            data->list->close(data->list);
            data->info->close(data->info);
            obj->base.step = 40;
        }
        break;
    case 20:
        data->list->close(data->list);
        obj->base.step++;
        break;
    case 21:
        if (data->list->base.state == OBJECT_STATE_DONE) {
            stitshop_funcs.anim_start(&obj->count_anim, 1);
            obj->base.step++;
        }
        break;
    case 22:
        if (stitshop_funcs.anim_update(&obj->count_anim)) {
            data->how_many->set_text(data->how_many, cdload_module.files.get_file(records_language + 0x71), 0x21);
            data->times->set_text(data->times, cdload_module.files.get_file(records_language + 0x71), 8);
            data->number->set_line_number(data->number, 0, obj->count);
            data->number->measure(data->number, 1);
            obj->item = data->list->get_item(data->list);
            obj->max = gamestate_data.items[obj->item];
            obj->base.step++;
        }
        break;
    case 23:
        old = obj->count;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            obj->count += 1;
            if (obj->count > obj->max) {
                obj->count = obj->max;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            if (--obj->count <= 0) {
                obj->count = 1;
            }
        } else if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
            if ((obj->count -= 10) < 10) {
                obj->count = 1;
            }
        } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
            obj->count += 10;
            if (obj->count > obj->max) {
                obj->count = obj->max;
            }
        }
        if (old != obj->count) {
            records_funcs.get_item(obj->item);
            data->number->set_line_number(data->number, 0, obj->count);
            data->number->measure(data->number, 1);
            data->info->set_item(data->info, obj->item, obj->count);
            sound_module.play(0x4001B);
        }
        if (PAD_PRESSED(0xD)) {
            sound_module.play(0x4001C);
            obj->base.substep = 0;
            obj->base.step++;
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->count = 1;
            data->info->set_item(data->info, obj->item, 1);
            obj->base.substep = 1;
            obj->base.step++;
        }
        break;
    case 24:
        stitshop_funcs.anim_start(&obj->count_anim, 0);
        data->how_many->set_visible(data->how_many, 0);
        data->times->set_visible(data->times, 0);
        data->number->set_visible(data->number, 0);
        if (obj->base.substep != 0) {
            data->list->resume(data->list);
            obj->base.step++;
        } else {
            obj->base.set_step(obj, 30);
        }
        break;
    case 25:
        if (data->list->base.step == 100) {
            data->list->base.step = 1;
        }
        if (stitshop_funcs.anim_update(&obj->count_anim)) {
            obj->base.step++;
        }
        break;
    case 26:
        if (data->list->base.state == OBJECT_STATE_DONE) {
            data->list->set_input(data->list, 1);
            obj->base.step = 6;
        }
        break;
    case 30:
        if (stitshop_funcs.anim_update(&obj->count_anim)) {
            stitshop_funcs.anim_start(&obj->question_anim, 1);
            obj->base.step++;
        }
        break;
    case 31:
        if (stitshop_funcs.anim_update(&obj->question_anim)) {
            data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x71), 0x12);
            data->messages->set_line_number(data->messages, 1, records_funcs.get_item(obj->item)->sell_price * obj->count);
            data->yes->set_text(data->yes, cdload_module.files.get_file(records_language + 0x71), 0x22);
            data->no->set_text(data->no, cdload_module.files.get_file(records_language + 0x71), 0x14);
            obj->cursor = 0;
            data->cursor->set_pos(data->cursor, 0xB8, 0x49);
            data->cursor->show(data->cursor, 1);
            obj->base.step++;
        }
        break;
    case 32:
        old = obj->cursor;
        if (PAD_PRESSED(4)) {
            obj->cursor = 0;
        } else if (PAD_PRESSED(6)) {
            obj->cursor = 1;
        }
        if (old != obj->cursor) {
            sound_module.play(0x8004513E);
            data->cursor->set_pos(data->cursor, 0xB8, obj->cursor * 16 + 0x49);
        } else if (PAD_PRESSED(0xD)) {
            sound_module.play(0x8004503C);
            if (obj->cursor == 0) {
                money = gamestate_data.money + records_funcs.get_item(obj->item)->sell_price * obj->count;
                gamestate_data.money = money;
                if (money > 9999999) {
                    gamestate_data.money = 9999999;
                }
                gamestate_data.items[obj->item] -= obj->count;
                if (gamestate_data.items[obj->item] < 0) {
                    gamestate_data.items[obj->item] = 0;
                }
                obj->main->show_money(obj->main);
            }
            data->list->collect(data->list);
            obj->base.step++;
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->base.step++;
        }
        break;
    case 33:
        stitshop_funcs.anim_start(&obj->question_anim, 0);
        data->messages->set_visible(data->messages, 0);
        data->yes->set_visible(data->yes, 0);
        data->no->set_visible(data->no, 0);
        data->cursor->show(data->cursor, 0);
        if (data->list->count <= 0 && gamestate_data.items[obj->item] == 0) {
            obj->count = 1;
            data->list->base.state = OBJECT_STATE_END;
            data->info->close(data->info);
            obj->base.step = 39;
        } else {
            data->list->resume(data->list);
            obj->base.step++;
        }
        break;
    case 34:
        if (data->list == NULL) {
            data->list = stitshop_list_create(obj, stitshop_sell_kinds[obj->kind], 1);
        } else if (data->list->base.step == 100) {
            data->list->base.step = 1;
            if (data->list->cursor > data->list->count - 1) {
                data->list->cursor--;
            }
            if (data->list->page > data->list->pages - 1) {
                data->list->page--;
            }
            obj->item = data->list->get_item(data->list);
            obj->count = 1;
            data->info->set_item(data->info, obj->item, 1);
        }
        if (stitshop_funcs.anim_update(&obj->question_anim)) {
            obj->base.step = 26;
        }
        break;
    case 39:
        if (stitshop_funcs.anim_update(&obj->question_anim)) {
            obj->base.step = 40;
        }
        break;
    case 40:
        if (data->info == NULL) {
            obj->base.step++;
        }
        break;
    case 41:
        obj->base.step = 0;
        break;
    case 200:
        stitshop_funcs.anim_start(&obj->nothing_anim, 1);
        obj->base.step++;
        break;
    case 201:
        if (stitshop_funcs.anim_update(&obj->nothing_anim)) {
            data->nothing_to_sell->set_text(data->nothing_to_sell, cdload_module.files.get_file(records_language + 0x71), 1);
            data->nothing_to_sell->set_line_text(data->nothing_to_sell, cdload_module.files.get_file(records_language + 0x71), obj->kind + 0x1D, 1);
            obj->base.step++;
        }
        break;
    case 202:
        if (PAD_PRESSED(0xD) || PAD_PRESSED(0xE)) {
            sound_module.play(0x4001C);
            data->nothing_to_sell->set_visible(data->nothing_to_sell, 0);
            stitshop_funcs.anim_start(&obj->nothing_anim, 0);
            obj->base.step++;
        }
        break;
    case 203:
        if (stitshop_funcs.anim_update(&obj->nothing_anim)) {
            data->cursor->show(data->cursor, 1);
            obj->base.step = 2;
        }
        break;
    case 50:
        data->cursor->show(data->cursor, 0);
        for (j = 0; j < 4; j++) {
            data->kinds[j]->set_visible(data->kinds[j], 0);
        }
        stitshop_funcs.anim_start(&obj->kinds_anim, 0);
        obj->base.step++;
        break;
    case 51:
        if (stitshop_funcs.anim_update(&obj->kinds_anim)) {
            if (obj->base.substep != 0) {
                obj->base.set_step(obj, 100);
            } else {
                obj->base.state = OBJECT_STATE_END;
            }
        }
        break;
    }
}

void stitshop_sell_update(StitshopSell *obj, StitshopSellData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stitshop_sell_create_windows(obj, data);
        obj->kinds_anim.duration = 10;
        obj->count_anim.duration = 10;
        obj->question_anim.duration = 10;
        obj->unk_AC.duration = 10;
        obj->nothing_anim.duration = 10;
        obj->count = 1;
        break;
    case OBJECT_STATE_RUN:
        stitshop_sell_run(obj, data);
        stitshop_sell_draw(obj, data);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Shows the item under the list's cursor (StitshopSell.set_item). */
void stitshop_sell_set_item(StitshopSell *obj, s32 item, s32 n) {
    StitshopInfo *info = ((StitshopSellData *)obj->base.children)->info;

    info->set_item(info, item, n);
}

StitshopSell *stitshop_sell_create(StitshopMain *main) {
    StitshopSell *obj = object_new(stitshop_sell_update, sizeof(StitshopSell), sizeof(StitshopSellData));

    obj->set_item = stitshop_sell_set_item;
    obj->layer_id = 0x1000;
    obj->ot_depth = 4;
    obj->main = main;
    return obj;
}

/* Creates the item list's text windows and cursor. The rows are cast to the windows' s16 before y is added
 * (the original adds y last). */
void stitshop_list_create_windows(StitshopList *obj, StitshopListData *data) {
    s32 i;
    s32 y;
    MessageWindow *win;
    MessageCursor *cursor;

    y = obj->player_items * 40;
    for (i = 0; i < 14; i++) {
        data->items[i] = win = message_create_window(obj->layer_id, 1, i % 2 * 0x83 + 0x37, i / 2 * 14 + 0x24);
        win->set_ot_depth(win, obj->ot_depth - 1);
    }
    data->page = message_create_window(obj->layer_id, 1, 0x9B, y + 0x60);
    data->slash = message_create_window(obj->layer_id, 1, 0x9C, y + 0x60);
    data->pages = message_create_window(obj->layer_id, 1, 0xB0, y + 0x60);
    data->prev_label = message_create_window(obj->layer_id, 1, 0x2D, y + (s16)(obj->player_items * 2 + 0x5C));
    data->next_label = message_create_window(obj->layer_id, 1, 0x102, y + (s16)(obj->player_items * 2 + 0x5C));
    data->cursor = cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0x1D, 0x24);
    cursor->show(cursor, 0);
}

/* Shows (`show`) or hides the page's items and the page numbers. */
void stitshop_list_show_text(StitshopList *obj, StitshopListData *data, s32 show) {
    s32 i;
    s32 k;
    s32 item;

    if (show) {
        for (i = 0; i < obj->per_page; i++) {
            k = obj->page * obj->per_page + i;
            if (obj->player_items == 0) {
                item = obj->goods[k];
            } else {
                item = obj->owned_items[k];
            }
            if (k < obj->count && item != 0) {
                data->items[i]->set_text(data->items[i], cdload_module.files.get_file(records_language + 0x6A), item);
            } else {
                data->items[i]->set_visible(data->items[i], 0);
            }
        }
        data->page->set_line_number(data->page, 0, obj->page + 1);
        data->page->measure(data->page, 1);
        data->slash->set_text(data->slash, cdload_module.files.get_file(records_language + 0x71), 0x18);
        data->pages->set_line_number(data->pages, 0, obj->pages);
        data->pages->measure(data->pages, 1);
        if (obj->pages >= 2) {
            if (obj->page != 0) {
                data->prev_label->set_text(data->prev_label, cdload_module.files.get_file(records_language + 0x71), 0x19);
            } else {
                data->prev_label->set_visible(data->prev_label, 0);
            }
            if (obj->page < obj->pages - 1) {
                data->next_label->set_text(data->next_label, cdload_module.files.get_file(records_language + 0x71), 0x1A);
            } else {
                data->next_label->set_visible(data->next_label, 0);
            }
        }
    } else {
        for (i = 0; i < obj->per_page; i++) {
            data->items[i]->set_visible(data->items[i], 0);
        }
        data->page->set_visible(data->page, 0);
        data->slash->set_visible(data->slash, 0);
        data->pages->set_visible(data->pages, 0);
        data->prev_label->set_visible(data->prev_label, 0);
        data->next_label->set_visible(data->next_label, 0);
    }
}

/* Draws the item list: the items' icons and the page arrows. */
void stitshop_list_draw(StitshopList *obj) {
    Sprite spr;
    s32 i;
    s32 k;
    s32 x;
    s32 y;
    s32 item;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    spr.set_vram_pos(0x280, 0x100);
    if (obj->frame_anim.level != 0) {
        if (obj->frame_anim.level != 0x1000) {
            spr.set_scale(0x1000, obj->frame_anim.level, 0x1000);
            spr.set_pivot(0xA0, 0x47);
        } else {
            for (i = 0; i < obj->per_page; i++) {
                k = obj->page * obj->per_page + i;
                if (obj->player_items == 0) {
                    item = obj->goods[k];
                } else {
                    item = obj->owned_items[k];
                }
                if (k >= obj->count || item == 0) {
                    break;
                }
                x = i % 2 * 0x83 + 0x28;
                y = i % obj->per_page / 2 * 14 + 0x24;
                spr.set_vram_pos(0x140, 0);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_funcs.get_item_icon(item), x, y);
                spr.set_vram_pos(0x280, 0x100);
                spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0x31, x, y);
            }
            if (obj->pages >= 2) {
                if (gfx_module.funcs.get_time() - obj->arrows_time >= 11) {
                    obj->arrows_time = gfx_module.funcs.get_time();
                    if (++obj->arrows_step >= 4) {
                        obj->arrows_step = 0;
                    }
                }
                spr.set_vram_pos(0x280, 0x100);
                spr.set_palette(obj->arrows_step);
                if (obj->page != 0) {
                    spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0x10, 0x1E, obj->player_items * 42 + 0x5F);
                }
                if (obj->page < obj->pages - 1) {
                    spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0x11, 0xFD, obj->player_items * 42 + 0x5F);
                }
                spr.set_palette(0);
            }
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), obj->player_items + 6, 0, 0x1D);
    }
}

/* The item list's steps (base.step): fill it, open, close. */
void stitshop_list_run(StitshopList *obj, StitshopListData *data) {
    switch (obj->base.step) {
    case 0:
    default:
        if (obj->player_items == 0) {
            obj->goods = stitshop_funcs.get_items(obj->shop);
            obj->count = stitshop_funcs.count;
            obj->per_page = 8;
            obj->base.step++;
        } else {
            stitshop_list_collect(obj);
            obj->per_page = 14;
            obj->base.step = 100;
        }
        obj->pages = obj->count / obj->per_page + (obj->count % obj->per_page != 0);
        break;
    case 1:
        stitshop_funcs.anim_start(&obj->frame_anim, 1);
        obj->base.step++;
        break;
    case 2:
        if (stitshop_funcs.anim_update(&obj->frame_anim)) {
            stitshop_list_show_text(obj, data, 1);
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        break;
    case 50:
        stitshop_funcs.anim_start(&obj->frame_anim, 0);
        stitshop_list_show_text(obj, data, 0);
        obj->base.step++;
        break;
    case 51:
        if (stitshop_funcs.anim_update(&obj->frame_anim)) {
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        break;
    case 100:
        break;
    }
}

void stitshop_list_update(StitshopList *obj, StitshopListData *data) {
    s32 old;
    s32 prev;
    s32 first;
    s32 last;
    s32 item;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stitshop_list_create_windows(obj, data);
        obj->frame_anim.duration = 10;
        break;
    case OBJECT_STATE_RUN:
        stitshop_list_run(obj, data);
        stitshop_list_draw(obj);
        break;
    case OBJECT_STATE_DONE:
        if (obj->input_enabled != 0) {
            old = obj->page;
            if ((!PAD_HELD(0xB) && PAD_PRESSED(0xA)) || (!PAD_HELD(0xB) && PAD_REPEAT(0xA))) {
                if (--obj->page < 0) {
                    obj->page = 0;
                }
            } else if ((!PAD_HELD(0xA) && PAD_PRESSED(0xB)) || (!PAD_HELD(0xA) && PAD_REPEAT(0xB))) {
                if (++obj->page > obj->pages - 1) {
                    obj->page = obj->pages - 1;
                }
            }
            if (old != obj->page) {
                sound_module.play(0x8004513E);
                obj->cursor = obj->page * obj->per_page;
                if (obj->player_items == 0) {
                    item = obj->goods[obj->cursor];
                } else {
                    item = obj->owned_items[obj->cursor];
                }
                obj->owner->set_item(obj->owner, item, 1);
                stitshop_list_show_text(obj, data, 1);
                data->cursor->set_pos(data->cursor, obj->cursor % 2 * 0x83 + 0x1D,
                                     obj->cursor % obj->per_page / 2 * 14 + 0x24);
            } else {
                first = old * obj->per_page;
                prev = obj->cursor;
                last = (old + 1) * obj->per_page - 1;
                if (last > obj->count - 1) {
                    last = obj->count - 1;
                }
                if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                    obj->cursor -= 2;
                    if (obj->cursor < first) {
                        obj->cursor = first;
                    }
                } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                    obj->cursor += 2;
                    if (obj->cursor > last) {
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
                        if (++obj->cursor > last) {
                            obj->cursor = last;
                        }
                    }
                }
                if (prev != obj->cursor) {
                    if (obj->player_items == 0) {
                        item = obj->goods[obj->cursor];
                    } else {
                        item = obj->owned_items[obj->cursor];
                    }
                    obj->owner->set_item(obj->owner, item, 1);
                    data->cursor->set_pos(data->cursor, obj->cursor % 2 * 0x83 + 0x1D,
                                         obj->cursor % obj->per_page / 2 * 14 + 0x24);
                    sound_module.play(0x8004513E);
                }
            }
        }
        stitshop_list_draw(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

/* Greys the cursor and stops the input (`grey`), or the reverse (StitshopList.grey_cursor). */
void stitshop_list_grey_cursor(StitshopList *obj, s32 grey) {
    StitshopListData *data = (StitshopListData *)obj->base.children;

    if (grey) {
        data->cursor->set_palette(data->cursor, 7);
        data->cursor->stop(data->cursor, 1);
        obj->input_enabled = 0;
    } else {
        data->cursor->set_palette(data->cursor, 0);
        data->cursor->stop(data->cursor, 0);
        obj->input_enabled = 1;
    }
}

/* Turns the input and the cursor on or off (StitshopList.set_input). */
void stitshop_list_set_input(StitshopList *obj, s32 on) {
    StitshopListData *data = (StitshopListData *)obj->base.children;

    obj->input_enabled = on;
    data->cursor->set_pos(data->cursor, obj->cursor % 2 * 0x83 + 0x1D, obj->cursor % obj->per_page / 2 * 14 + 0x24);
    data->cursor->show(data->cursor, on);
}

/* StitshopList.resume. */
void stitshop_list_resume(StitshopList *obj) {
    obj->base.set_state(obj, OBJECT_STATE_RUN);
}

/* StitshopList.close. */
void stitshop_list_close(StitshopList *obj) {
    obj->base.set_state(obj, OBJECT_STATE_RUN);
    obj->base.step = 50;
    stitshop_list_set_input(obj, 0);
}

/* The item under the cursor (StitshopList.get_item). */
s16 stitshop_list_get_item(StitshopList *obj) {
    if (obj->player_items == 0) {
        return obj->goods[obj->cursor];
    }
    return obj->owned_items[obj->cursor];
}

/* Collects the player's items again after a sale (StitshopList.refresh). */
void stitshop_list_refresh(StitshopList *obj) {
    StitshopListData *data = (StitshopListData *)obj->base.children;

    stitshop_list_collect(obj);
    stitshop_list_show_text(obj, data, 1);
    obj->owner->set_item(obj->owner, obj->owned_items[obj->cursor], 1);
}

/* Collects the player's items of the kind that can be sold (StitshopList.collect). */
void stitshop_list_collect(StitshopList *obj) {
    s32 n;
    s32 i;

    obj->count = 0;
    n = records_funcs.list_items(obj->shop, (u16 *)obj->kind_items);
    for (i = 0; i < n; i++) {
        if (records_funcs.get_item(obj->kind_items[i])->sell_price != 0) {
            obj->owned_items[obj->count++] = obj->kind_items[i];
        }
    }
}

StitshopList *stitshop_list_create(void *owner, s32 kind, s32 mode) {
    StitshopList *obj = object_new(stitshop_list_update, sizeof(StitshopList), sizeof(StitshopListData));

    obj->resume = stitshop_list_resume;
    obj->close = stitshop_list_close;
    obj->get_item = (s32 (*)(StitshopList *))stitshop_list_get_item;
    obj->set_input = stitshop_list_set_input;
    obj->grey_cursor = stitshop_list_grey_cursor;
    obj->refresh = stitshop_list_refresh;
    obj->collect = stitshop_list_collect;
    obj->layer_id = 0x1000;
    obj->ot_depth = 4;
    obj->owner = owner;
    obj->player_items = mode;
    obj->shop = kind;
    return obj;
}

/* A party member's stats with its equipment, as gamestate_get_stats (without the set bonus). */
void stitshop_info_get_stats(s32 i, GamestateStats *out) {
    s16 *items;
    RecordsItem *entry;
    void *data;
    u8 type;
    s16 stat;
    s16 value;
    s32 j;
    s32 k;

    *out = gamestate_data.digimon[i].record.stats;
    items = gamestate_data.digimon[i].record.equipment;
    for (j = 0; j < 6; j++) {
        if (items[j] > 0) {
            entry = records_funcs.get_item(items[j]);
            type = entry->type;
            data = entry->data;
            if (type >= 2 && type <= 14) {
                out->stats[0] += ((RecordsWeapon *)data)->power;
                if (out->stats[0] >= 1000) {
                    out->stats[0] = 999;
                }
                for (k = 0; k < 2; k++) {
                    stat = *(((RecordsWeapon *)data)->bonus_stats + k);
                    value = ((RecordsWeapon *)data)->bonus_values[k];
                    if (stat != 0) {
                        stitshop_info_add_stat(out->values, stat, value);
                    }
                }
            } else if (type >= 15 && type <= 20) {
                out->stats[1] += ((RecordsArmor *)data)->guard;
                if (out->stats[1] >= 1000) {
                    out->stats[1] = 999;
                }
                for (k = 0; k < 2; k++) {
                    stat = *(((RecordsArmor *)data)->bonus_stats + k);
                    value = ((RecordsArmor *)data)->bonus_values[k];
                    if (stat != 0) {
                        stitshop_info_add_stat(out->values, stat, value);
                    }
                }
            } else if (type >= 21 && type <= 24) {
                stat = ((RecordsAccessory *)data)->bonus_stat;
                value = ((RecordsAccessory *)data)->bonus_value;
                if (stat != 0) {
                    stitshop_info_add_stat(out->values, stat, value);
                }
            } else {
                continue;
            }
            out->stats[5] += ((RecordsEquip *)data)->charisma;
            if (out->stats[5] >= 1000) {
                out->stats[5] = 999;
            }
        }
    }
    out->stats[0] -= out->penalties[0];
    if (out->stats[0] < 0) {
        out->stats[0] = 0;
    }
    out->stats[1] -= out->penalties[1];
    if (out->stats[1] < 0) {
        out->stats[1] = 0;
    }
    out->stats[4] -= out->penalties[2];
    if (out->stats[4] < 0) {
        out->stats[4] = 0;
    }
}

/* gamestate_add_stat_bonus (gamestate.c), byte for byte: adds `value` to stat `type` (7: all six of 6..11), at most 999. */
void stitshop_info_add_stat(s16 *stats, s32 type, s32 value) {
    s32 i;

    if (type == 7) {
        for (i = 0; i < 6; i++) {
            stats[i + 6] += value;
            if (stats[i + 6] >= 1000) {
                stats[i + 6] = 999;
            }
        }
    } else if (type >= 1 && type <= 6) {
        stats[type + 5] += value;
        if (stats[type + 5] >= 1000) {
            stats[type + 5] = 999;
        }
    } else if (type >= 8 && type <= 14) {
        stats[type + 4] += value;
        if (stats[type + 4] >= 1000) {
            stats[type + 4] = 999;
        }
    }
}

/* Writes a stat's value to `win`. */
void stitshop_info_show_stat(StitshopInfo *obj, MessageWindow *win, StitshopStatRef *ref) {
    StitshopStats *stats = &obj->stats[ref->member];
    u16 value;

    if (ref->which == 0) {
        value = *(stats->now.values + stitshop_info_stats[ref->stat]);
    } else {
        value = *(stats->with_item.values + stitshop_info_stats[ref->stat]);
    }
    win->set_line_number(win, 0, (s16)value);
    win->measure(win, 1);
}

/* Colours a stat: up, down, or lowered by a status. */
void stitshop_info_colour_stat(StitshopInfo *obj, MessageWindow *win, StitshopStatRef *ref) {
    StitshopStats *stats = &obj->stats[ref->member];
    s32 j;
    s16 v[2];

    if (ref->stat == 0) {
        j = 0;
    } else if (ref->stat == 1) {
        j = 1;
    } else if (ref->stat == 4) {
        j = 2;
    } else {
        j = -1;
    }
    v[0] = *(stats->now.values + stitshop_info_stats[ref->stat]);
    if (ref->which == 0) {
        if (j >= 0 && stats->now.penalties[j] != 0) {
            win->set_palette(win, 6);
        } else {
            win->set_palette(win, 0);
        }
    } else {
        v[1] = *(stats->with_item.values + stitshop_info_stats[ref->stat]);
        if (v[0] == v[1]) {
            if (j >= 0 && stats->now.penalties[j] != 0) {
                win->set_palette(win, 6);
            } else {
                win->set_palette(win, 0);
            }
        } else if (v[0] < v[1]) {
            win->set_palette(win, 1);
        } else {
            win->set_palette(win, 5);
        }
    }
}

/* Shows the other stats the item changes (at most four), and records them in StitshopStats.shown. */
void stitshop_info_show_changes(StitshopInfo *obj, MessageWindow **wins, StitshopStatRef *ref) {
    StitshopStats *stats;
    MessageWindow **win;
    s16 v[2];
    s32 *id;
    s32 i;
    s32 n;

    i = n = 0;
    stats = &obj->stats[ref->member];
    id = stitshop_info_stats;
    win = wins;
    for (; i < 13; i++, id++) {
        if (i != ref->skip1 && (ref->skip2 < 0 || i != ref->skip2)) {
            v[0] = *(stats->now.values + *id);
            v[1] = *(stats->with_item.values + *id);
            if (v[0] != v[1]) {
                stats->shown[n + 2] = i + 1;
                (*win)->set_line_number(*win, 0, v[1]);
                (*win)->measure(*win, 1);
                if (v[0] < v[1]) {
                    (*win)->set_palette(*win, 1);
                } else {
                    (*win)->set_palette(*win, 5);
                }
                win++;
                n++;
            }
        }
    }
    stats->shown_count += n;
    for (i = n; i < 4; i++) {
        wins[i]->set_visible(wins[i], 0);
    }
}

/* Compares party member `member`'s stats now and with the item, and shows them. */
void stitshop_info_compare(StitshopInfo *obj, StitshopInfoData *data, s32 member) {
    StitshopItems items;
    StitshopStatRef ref;
    s32 digimon;
    GamestateRecord *rec;
    StitshopStats *stats;
    RecordsAccessory *equip;
    u8 type;

    digimon = gamestate_data.funcs.get_party_member(member);
    rec = gamestate_data.funcs.get_record(digimon);
    stats = &obj->stats[member];
    items = *(StitshopItems *)rec->equipment;
    stitshop_info_get_stats(digimon, &stats->now);
    stats->slot = stitshop_funcs.get_slot(digimon, obj->item);
    stitshop_funcs.equip(digimon, stats->slot, obj->item, 0);
    stitshop_info_get_stats(digimon, &stats->with_item);
    *(StitshopItems *)rec->equipment = items;
    stats->shown_count = 2;
    switch (stats->slot) {
    case 0:
    case 2:
    case 3:
    default:
        type = records_funcs.get_item(obj->item)->type;
        if (type >= 2 && type <= 14) {
            stats->shown[0] = 1;
            ref.member = member;
            ref.stat = 0;
            ref.which = 0;
            stitshop_info_show_stat(obj, data->members[member].stats[0], &ref);
            stitshop_info_colour_stat(obj, data->members[member].stats[0], &ref);
            ref.which = 1;
            stitshop_info_show_stat(obj, data->members[member].stats[2], &ref);
            stitshop_info_colour_stat(obj, data->members[member].stats[2], &ref);
            stats->shown[1] = 6;
            ref.member = member;
            ref.stat = 5;
            ref.which = 0;
            stitshop_info_show_stat(obj, data->members[member].stats[1], &ref);
            stitshop_info_colour_stat(obj, data->members[member].stats[1], &ref);
            ref.which = 1;
            stitshop_info_show_stat(obj, data->members[member].stats[3], &ref);
            stitshop_info_colour_stat(obj, data->members[member].stats[3], &ref);
            ref.skip1 = 0;
            ref.skip2 = 5;
            stitshop_info_show_changes(obj, &data->members[member].stats[4], &ref);
        } else {
            stats->shown[0] = 2;
            ref.member = member;
            ref.stat = 1;
            ref.which = 0;
            stitshop_info_show_stat(obj, data->members[member].stats[0], &ref);
            stitshop_info_colour_stat(obj, data->members[member].stats[0], &ref);
            ref.which = 1;
            stitshop_info_show_stat(obj, data->members[member].stats[2], &ref);
            stitshop_info_colour_stat(obj, data->members[member].stats[2], &ref);
            stats->shown[1] = 6;
            ref.member = member;
            ref.stat = 5;
            ref.which = 0;
            stitshop_info_show_stat(obj, data->members[member].stats[1], &ref);
            stitshop_info_colour_stat(obj, data->members[member].stats[1], &ref);
            ref.which = 1;
            stitshop_info_show_stat(obj, data->members[member].stats[3], &ref);
            stitshop_info_colour_stat(obj, data->members[member].stats[3], &ref);
            ref.skip1 = 1;
            ref.skip2 = 5;
            stitshop_info_show_changes(obj, &data->members[member].stats[4], &ref);
        }
        data->members[member].arrows[0]->set_text(data->members[member].arrows[0],
                                                 cdload_module.files.get_file(records_language + 0x71), 0xC);
        data->members[member].arrows[1]->set_text(data->members[member].arrows[1],
                                                 cdload_module.files.get_file(records_language + 0x71), 0xC);
        break;
    case 1:
        stats->shown[0] = 2;
        ref.member = member;
        ref.stat = 1;
        ref.which = 0;
        stitshop_info_show_stat(obj, data->members[member].stats[0], &ref);
        stitshop_info_colour_stat(obj, data->members[member].stats[0], &ref);
        ref.which = 1;
        stitshop_info_show_stat(obj, data->members[member].stats[2], &ref);
        stitshop_info_colour_stat(obj, data->members[member].stats[2], &ref);
        stats->shown[1] = 6;
        ref.member = member;
        ref.stat = 5;
        ref.which = 0;
        stitshop_info_show_stat(obj, data->members[member].stats[1], &ref);
        stitshop_info_colour_stat(obj, data->members[member].stats[1], &ref);
        ref.which = 1;
        stitshop_info_show_stat(obj, data->members[member].stats[3], &ref);
        stitshop_info_colour_stat(obj, data->members[member].stats[3], &ref);
        ref.skip1 = 1;
        ref.skip2 = 5;
        stitshop_info_show_changes(obj, &data->members[member].stats[4], &ref);
        data->members[member].arrows[0]->set_text(data->members[member].arrows[0],
                                                 cdload_module.files.get_file(records_language + 0x71), 0xC);
        data->members[member].arrows[1]->set_text(data->members[member].arrows[1],
                                                 cdload_module.files.get_file(records_language + 0x71), 0xC);
        break;
    case 4:
    case 5:
        stats->shown[0] = 6;
        ref.member = member;
        ref.stat = 5;
        ref.which = 0;
        stitshop_info_show_stat(obj, data->members[member].stats[0], &ref);
        stitshop_info_colour_stat(obj, data->members[member].stats[0], &ref);
        ref.which = 1;
        stitshop_info_show_stat(obj, data->members[member].stats[2], &ref);
        stitshop_info_colour_stat(obj, data->members[member].stats[2], &ref);
        ref.skip1 = 5;
        equip = records_funcs.get_item(obj->item)->data;
        if ((equip->bonus_stat >= 1 && equip->bonus_stat <= 5) || (equip->bonus_stat >= 8 && equip->bonus_stat <= 14)) {
            if (equip->bonus_stat >= 1 && equip->bonus_stat <= 5) {
                stats->shown[1] = equip->bonus_stat;
            } else {
                stats->shown[1] = equip->bonus_stat - 1;
            }
            ref.member = member;
            ref.stat = stats->shown[1] - 1;
            ref.which = 0;
            stitshop_info_show_stat(obj, data->members[member].stats[1], &ref);
            stitshop_info_colour_stat(obj, data->members[member].stats[1], &ref);
            ref.which = 1;
            stitshop_info_show_stat(obj, data->members[member].stats[3], &ref);
            stitshop_info_colour_stat(obj, data->members[member].stats[3], &ref);
            data->members[member].arrows[0]->set_text(data->members[member].arrows[0],
                                                     cdload_module.files.get_file(records_language + 0x71), 0xC);
            data->members[member].arrows[1]->set_text(data->members[member].arrows[1],
                                                     cdload_module.files.get_file(records_language + 0x71), 0xC);
            ref.skip2 = ref.stat;
        } else {
            data->members[member].stats[1]->set_visible(data->members[member].stats[1], 0);
            data->members[member].stats[3]->set_visible(data->members[member].stats[3], 0);
            ref.skip2 = -1;
            stats->shown[1] = 0;
            data->members[member].arrows[0]->set_text(data->members[member].arrows[0],
                                                     cdload_module.files.get_file(records_language + 0x71), 0xC);
            data->members[member].arrows[1]->set_visible(data->members[member].arrows[1], 0);
        }
        stitshop_info_show_changes(obj, &data->members[member].stats[4], &ref);
        break;
    }
}

/* The y of an item line, two lower when selling. A function of its own: written inline, gcc folds the sum into y. */
static inline s32 stitshop_info_line_y(StitshopInfo *obj, s32 y) {
    return obj->selling * 2 + y;
}

/* Creates the item's information's text windows (the party's only when buying). */
void stitshop_info_create_windows(StitshopInfo *obj, StitshopInfoData *data) {
    s32 y = obj->selling * 40;
    s16 left;
    s16 right;
    s32 x;
    s32 i;
    s32 j;
    s32 column;

    data->item_name = message_create_window(obj->layer_id, 1, 0x25, y + stitshop_info_line_y(obj, 0x73));
    data->equipped_label = message_create_window(obj->layer_id, 1, 0xA1, y + stitshop_info_line_y(obj, 0x75));
    data->equipped = message_create_window(obj->layer_id, 1, 0xE1, y + stitshop_info_line_y(obj, 0x75));
    data->owned_label = message_create_window(obj->layer_id, 1, 0xEA, y + stitshop_info_line_y(obj, 0x75));
    data->owned = message_create_window(obj->layer_id, 1, 0x12A, y + stitshop_info_line_y(obj, 0x75));
    data->price_label = message_create_window(obj->layer_id, 3, 0x11A, y + stitshop_info_line_y(obj, 0x8B));
    data->price = message_create_window(obj->layer_id, 3, 0x117, y + stitshop_info_line_y(obj, 0x8B));
    data->description = message_create_window(obj->layer_id, 1, 0x14, y + 0xA1);
    data->description->set_page_lines(data->description, 2);
    data->slot = message_create_window(obj->layer_id, 1, 0x14, y + 0xAF);
    if (obj->selling == 0) {
        /* FAKE: x is set from a column counter of its own inside the arrows' loop (the shape of the US decomp's
         * STITSHOP_createInfoWindows): it puts x's init among loop.c's giv inits, after the loop's first test, as
         * the original does; x stepped by the loop itself leaves it before them (99.1%). */
        column = 0;
        for (i = 0; i < obj->member_count; i++) {
            data->members[i].name = message_create_window(obj->layer_id, 1, i * 0x63 + 0x17, 0x9E);
            for (j = 0; j < 2; j++) {
                x = column * 0x63;
                data->members[i].arrows[j] = message_create_window(obj->layer_id, 1, i * 0x63 + 0x3C, j * 14 + 0xAC);
            }
            left = x + 0x39;
            data->members[i].stats[0] = message_create_window(obj->layer_id, 1, left, 0xAC);
            data->members[i].stats[1] = message_create_window(obj->layer_id, 1, left, 0xBA);
            right = x + 0x5A;
            data->members[i].stats[2] = message_create_window(obj->layer_id, 1, right, 0xAC);
            data->members[i].stats[3] = message_create_window(obj->layer_id, 1, right, 0xBA);
            data->members[i].stats[4] = message_create_window(obj->layer_id, 1, left, 0xC8);
            data->members[i].stats[5] = message_create_window(obj->layer_id, 1, x + 0x63, 0xC8);
            data->members[i].stats[6] = message_create_window(obj->layer_id, 1, left, 0xD6);
            data->members[i].stats[7] = message_create_window(obj->layer_id, 1, x + 0x63, 0xD6);
            column++;
        }
        data->page = message_create_window(obj->layer_id, 1, 0x13, 0x87);
    }
}

/* Shows (`show`) or hides the item's name, how many are equipped and owned, and the price. */
void stitshop_info_show_item(StitshopInfo *obj, StitshopInfoData *data, s32 show) {
    u16 price;

    if (show) {
        data->item_name->set_text(data->item_name, cdload_module.files.get_file(records_language + 0x6A), obj->item);
        data->equipped_label->set_text(data->equipped_label, cdload_module.files.get_file(records_language + 0x71), 6);
        data->equipped->set_line_number(data->equipped, 0, gamestate_data.items_equipped[obj->item]);
        data->equipped->measure(data->equipped, 1);
        data->owned_label->set_text(data->owned_label, cdload_module.files.get_file(records_language + 0x71), 7);
        data->owned->set_line_number(data->owned, 0, gamestate_data.items[obj->item]);
        data->owned->measure(data->owned, 1);
        data->price_label->set_text(data->price_label, cdload_module.files.get_file(records_language + 0x71), 2);
        if (obj->selling == 0) {
            price = records_funcs.get_item(obj->item)->price;
        } else {
            price = records_funcs.get_item(obj->item)->sell_price;
        }
        data->price->set_line_number(data->price, 0, price * obj->count);
        data->price->measure(data->price, 1);
    } else {
        data->item_name->set_visible(data->item_name, 0);
        data->equipped_label->set_visible(data->equipped_label, 0);
        data->equipped->set_visible(data->equipped, 0);
        data->owned_label->set_visible(data->owned_label, 0);
        data->owned->set_visible(data->owned, 0);
        data->price_label->set_visible(data->price_label, 0);
        data->price->set_visible(data->price, 0);
    }
}

/* Shows (`show`) or hides the item's description and, for equipment, the slot it goes to. */
void stitshop_info_show_description(StitshopInfo *obj, StitshopInfoData *data, s32 show) {
    RecordsItem *entry;
    RecordsEquip *equip;

    if (show) {
        data->description->set_text(data->description, cdload_module.files.get_file(records_language + 0x63), obj->item);
        entry = records_funcs.get_item(obj->item);
        if (entry->type >= 2 && entry->type <= 13) {
            equip = entry->data;
            data->slot->set_text(data->slot, cdload_module.files.get_file(records_language + 0x71),
                                  stitshop_slot_texts[equip->slot]);
            return;
        }
    } else {
        data->description->set_visible(data->description, 0);
    }
    data->slot->set_visible(data->slot, 0);
}

/* Shows (`show`) or hides the party: who can equip the item, and what it does to their stats. */
void stitshop_info_show_party(StitshopInfo *obj, StitshopInfoData *data, s32 show) {
    s32 i;
    s32 j;
    s32 digimon;

    if (show) {
        for (i = 0; i < obj->member_count; i++) {
            digimon = gamestate_data.funcs.get_party_member(i);
            data->members[i].name->set_text(data->members[i].name, (u8 *)gamestate_data.funcs.get_record(digimon), -1);
            if (stitshop_funcs.can_equip(digimon, obj->item)) {
                stitshop_info_compare(obj, data, i);
                data->members[i].name->set_palette(data->members[i].name, 0);
            } else {
                data->members[i].name->set_palette(data->members[i].name, 7);
                for (j = 0; j < 2; j++) {
                    data->members[i].arrows[j]->set_visible(data->members[i].arrows[j], 0);
                }
                for (j = 0; j < 8; j++) {
                    data->members[i].stats[j]->set_visible(data->members[i].stats[j], 0);
                }
            }
        }
    } else {
        for (i = 0; i < obj->member_count; i++) {
            data->members[i].name->set_visible(data->members[i].name, 0);
            for (j = 0; j < 2; j++) {
                data->members[i].arrows[j]->set_visible(data->members[i].arrows[j], 0);
            }
            for (j = 0; j < 8; j++) {
                data->members[i].stats[j]->set_visible(data->members[i].stats[j], 0);
            }
        }
    }
}

/* Draws the party page: each member's frame and the icons of the stats the item changes. */
void stitshop_info_draw_party(StitshopInfo *obj, StitshopInfoData *data) {
    Sprite spr;
    s32 i;
    s32 j;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    spr.set_vram_pos(0x280, 0x100);
    if (obj->party_anim.level != 0) {
        if (obj->party_anim.level != 0x1000) {
            spr.set_scale(obj->party_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0, 0x8C);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 9, 0, 0x81);
    }
    if (obj->members_anim.level != 0) {
        spr.set_scale(0x1000, obj->members_anim.level, 0x1000);
        for (i = 0; i < obj->member_count; i++) {
            if (obj->members_anim.level != 0x1000) {
                spr.set_pivot(i * 100 + 0x3C, 0xC0);
            } else if (stitshop_funcs.can_equip(gamestate_data.funcs.get_party_member(i), obj->item)) {
                spr.set_vram_pos(0x140, 0);
                for (j = 0; j < obj->stats[i].shown_count; j++) {
                    if (obj->stats[i].shown[j] > 0) {
                        spr.draw(cdload_module.get_subfile_by_id(0x02860000), obj->stats[i].shown[j] + 0x24,
                                   stitshop_info_mark_pos[j].x + i * 0x63, stitshop_info_mark_pos[j].y);
                    }
                }
            } else {
                spr.set_vram_pos(0x280, 0x100);
                spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0x16, i * 0x63 + 0x19, 0xAD);
            }
            spr.set_vram_pos(0x280, 0x100);
            spr.draw(cdload_module.get_subfile_by_id(0x04020000), 8, i * 0x63 + 0xB, 0x9C);
        }
    }
}

/* The item's information's steps when buying (base.step). */
void stitshop_info_run_buy(StitshopInfo *obj, StitshopInfoData *data) {
    switch (obj->base.step) {
    case 0:
    default:
        stitshop_funcs.anim_start(&obj->item_anim, 1);
        if (obj->can_equip != 0) {
            stitshop_funcs.anim_start(&obj->party_anim, 1);
        }
        obj->base.step++;
        break;
    case 1:
        if (obj->can_equip != 0) {
            stitshop_funcs.anim_update(&obj->party_anim);
        }
        if (stitshop_funcs.anim_update(&obj->item_anim)) {
            stitshop_info_show_item(obj, data, 1);
            if (obj->can_equip != 0) {
                data->page->set_text(data->page, cdload_module.files.get_file(records_language + 0x71), obj->page + 0xA);
            }
            stitshop_funcs.anim_start(&obj->text_anim, 1);
            obj->base.step++;
        }
        break;
    case 2:
        if (stitshop_funcs.anim_update(&obj->text_anim)) {
            stitshop_info_show_description(obj, data, 1);
            obj->base.step++;
        }
        break;
    case 4:
        obj->shown = 0;
        data->page->set_visible(data->page, 0);
        stitshop_funcs.anim_start(&obj->party_anim, 0);
        if (obj->page == 0) {
            stitshop_funcs.anim_start(&obj->text_anim, 0);
            stitshop_info_show_description(obj, data, 0);
        }
        obj->base.step++;
        break;
    case 5:
        stitshop_funcs.anim_update(&obj->text_anim);
        if (stitshop_funcs.anim_update(&obj->party_anim)) {
            if (obj->page == 0) {
                stitshop_funcs.anim_start(&obj->members_anim, 1);
            }
            obj->base.step++;
        }
        break;
    case 6:
        if (stitshop_funcs.anim_update(&obj->members_anim)) {
            stitshop_info_show_party(obj, data, 1);
            obj->base.step++;
        }
        break;
    case 8:
        if (obj->page == 0) {
            stitshop_info_show_party(obj, data, 0);
            stitshop_funcs.anim_start(&obj->members_anim, 0);
        }
        stitshop_funcs.anim_start(&obj->party_anim, 1);
        obj->base.step++;
        break;
    case 9:
        stitshop_funcs.anim_update(&obj->members_anim);
        if (stitshop_funcs.anim_update(&obj->party_anim)) {
            data->page->set_visible(data->page, 1);
            if (obj->page == 0) {
                stitshop_funcs.anim_start(&obj->text_anim, 1);
            }
            obj->base.step = 1000;
        }
        break;
    case 1000:
        if (stitshop_funcs.anim_update(&obj->text_anim)) {
            if (obj->page == 0) {
                stitshop_info_show_description(obj, data, 1);
            }
            obj->base.step = 3;
            obj->shown = 1;
        }
        break;
    case 10:
        data->page->set_text(data->page, cdload_module.files.get_file(records_language + 0x71), obj->page + 0xA);
        if (obj->page == 0) {
            stitshop_funcs.anim_start(&obj->members_anim, 0);
            stitshop_info_show_party(obj, data, 0);
            obj->base.step = 15;
        } else {
            stitshop_funcs.anim_start(&obj->text_anim, 0);
            stitshop_info_show_description(obj, data, 0);
            obj->base.step = 11;
        }
        break;
    case 11:
        if (stitshop_funcs.anim_update(&obj->text_anim)) {
            stitshop_funcs.anim_start(&obj->members_anim, 1);
            obj->base.step++;
        }
        break;
    case 12:
        if (stitshop_funcs.anim_update(&obj->members_anim)) {
            stitshop_info_show_party(obj, data, 1);
            obj->base.step = 3;
            obj->shown = 1;
        }
        break;
    case 15:
        if (stitshop_funcs.anim_update(&obj->members_anim)) {
            stitshop_funcs.anim_start(&obj->text_anim, 1);
            obj->base.step++;
        }
        break;
    case 16:
        if (stitshop_funcs.anim_update(&obj->text_anim)) {
            stitshop_info_show_description(obj, data, 1);
            obj->base.step = 3;
            obj->shown = 1;
        }
        break;
    case 50:
        stitshop_info_show_item(obj, data, 0);
        data->page->set_visible(data->page, 0);
        stitshop_funcs.anim_start(&obj->item_anim, 0);
        if (obj->can_equip != 0) {
            stitshop_funcs.anim_start(&obj->party_anim, 0);
        }
        if (obj->page == 0) {
            stitshop_info_show_description(obj, data, 0);
            stitshop_funcs.anim_start(&obj->text_anim, 0);
        } else {
            stitshop_info_show_party(obj, data, 0);
            stitshop_funcs.anim_start(&obj->members_anim, 0);
        }
        obj->base.step++;
        break;
    case 51:
        if (obj->page == 0) {
            stitshop_funcs.anim_update(&obj->text_anim);
        } else {
            stitshop_funcs.anim_update(&obj->members_anim);
        }
        if (obj->can_equip != 0) {
            stitshop_funcs.anim_update(&obj->party_anim);
        }
        if (stitshop_funcs.anim_update(&obj->item_anim)) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 3:
    case 7:
        break;
    }
}

/* The item's information's steps when selling (base.step). */
void stitshop_info_run_sell(StitshopInfo *obj, StitshopInfoData *data) {
    switch (obj->base.step) {
    case 0:
    default:
        stitshop_funcs.anim_start(&obj->item_anim, 1);
        obj->base.step++;
        break;
    case 1:
        if (stitshop_funcs.anim_update(&obj->item_anim)) {
            stitshop_info_show_item(obj, data, 1);
            stitshop_funcs.anim_start(&obj->text_anim, 1);
            obj->base.step++;
        }
        break;
    case 2:
        if (stitshop_funcs.anim_update(&obj->text_anim)) {
            stitshop_info_show_description(obj, data, 1);
            obj->base.step++;
        }
        break;
    case 3:
        break;
    case 50:
        stitshop_info_show_item(obj, data, 0);
        stitshop_info_show_description(obj, data, 0);
        stitshop_funcs.anim_start(&obj->item_anim, 0);
        stitshop_funcs.anim_start(&obj->text_anim, 0);
        obj->base.step++;
        break;
    case 51:
        stitshop_funcs.anim_update(&obj->item_anim);
        if (stitshop_funcs.anim_update(&obj->text_anim)) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}

void stitshop_info_update(StitshopInfo *obj, StitshopInfoData *data) {
    Sprite spr;
    s32 i;
    s32 y;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        for (i = 0; i < 3; i++) {
            if (gamestate_data.funcs.get_party_member(i) >= 0) {
                obj->member_count++;
            }
        }
        stitshop_info_create_windows(obj, data);
        obj->item_anim.duration = 10;
        obj->party_anim.duration = 10;
        obj->text_anim.duration = 10;
        obj->members_anim.duration = 10;
        if (obj->selling == 0 && (records_funcs.is_item_category(obj->item, 3) || records_funcs.is_item_category(obj->item, 4) ||
                                 records_funcs.is_item_category(obj->item, 5))) {
            obj->can_equip = 1;
        }
        break;
    case OBJECT_STATE_RUN:
        if (obj->selling == 0) {
            stitshop_info_run_buy(obj, data);
            stitshop_info_draw_party(obj, data);
        } else {
            stitshop_info_run_sell(obj, data);
        }
        y = obj->selling * 40;
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
        spr.set_vram_pos(0x280, 0x100);
        if (obj->item_anim.level != 0) {
            if (obj->item_anim.level != 0x1000) {
                spr.set_scale(obj->item_anim.level, 0x1000, 0x1000);
                spr.set_pivot(0x140, y + 0x81);
            } else {
                spr.set_vram_pos(0x140, 0);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_funcs.get_item_icon(obj->item), 0x16,
                           y + 0x73 + obj->selling * 2);
            }
            spr.set_vram_pos(0x280, 0x100);
            spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0xE, 0xF, y + 0x6C + obj->selling * 2);
        }
        if (obj->text_anim.level != 0) {
            if (obj->text_anim.level != 0x1000) {
                spr.set_scale(0x1000, obj->text_anim.level, 0x1000);
                spr.set_pivot(0xA0, y + 0xAE);
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x04020000), 3, 0, y + 0x9C);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Shows another item, `n` of them (StitshopInfo.set_item). */
void stitshop_info_set_item(StitshopInfo *obj, s32 item, s32 n) {
    StitshopInfoData *data = (StitshopInfoData *)obj->base.children;

    obj->item = item;
    obj->count = n;
    if (obj->shown != 0) {
        stitshop_info_show_item(obj, data, 1);
        if (obj->page == 0) {
            stitshop_info_show_description(obj, data, 1);
        } else {
            stitshop_info_show_description(obj, data, 0);
            stitshop_info_show_party(obj, data, 1);
        }
    }
}

/* StitshopInfo.close. */
void stitshop_info_close(StitshopInfo *obj) {
    if (obj->base.step == 3) {
        obj->base.step = 50;
    }
}

/* StitshopInfo.show_text. */
void stitshop_info_show_text(StitshopInfo *obj, s32 show) {
    ((StitshopInfoData *)obj->base.children)->slot->set_visible(((StitshopInfoData *)obj->base.children)->slot, show);
}

/* Switches between the description and the party, for equipment (StitshopInfo.turn_page). */
void stitshop_info_turn_page(StitshopInfo *obj) {
    if (records_funcs.is_item_category(obj->item, 3) || records_funcs.is_item_category(obj->item, 4) ||
        records_funcs.is_item_category(obj->item, 5)) {
        sound_module.play(0x4001B);
        obj->base.step = 10;
        obj->shown = 0;
        obj->page = 1 - obj->page;
    }
}

/* Shows party member `member` after equipping the item on it (StitshopInfo.show_member). */
void stitshop_info_show_member(StitshopInfo *obj, s32 member) {
    StitshopInfoData *data = (StitshopInfoData *)obj->base.children;

    stitshop_info_show_item(obj, data, 1);
    stitshop_info_compare(obj, data, member);
}

StitshopInfo *stitshop_info_create(s32 mode, s32 item) {
    StitshopInfo *obj = object_new(stitshop_info_update, sizeof(StitshopInfo), sizeof(StitshopInfoData));

    obj->set_item = stitshop_info_set_item;
    obj->close = stitshop_info_close;
    obj->show_text = stitshop_info_show_text;
    obj->turn_page = stitshop_info_turn_page;
    obj->show_member = stitshop_info_show_member;
    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->selling = mode;
    obj->item = item;
    obj->count = 1;
    obj->shown = 1;
    return obj;
}

/* Creates the shop's text windows and cursor. */
void stitshop_main_create_windows(StitshopMain *obj, StitshopMainData *data) {
    MessageCursor *cursor;

    data->help = message_create_window(obj->layer_id, 1, 0xD3, 0xCC);
    data->shopkeeper = message_create_window(obj->layer_id, 1, 0x1D, 0x14);
    data->money = message_create_window(obj->layer_id, 3, 0x117, 0x17);
    data->money_label = message_create_window(obj->layer_id, 3, 0x11A, 0x17);
    data->buy = message_create_window(obj->layer_id, 1, 0xBE, 0x2F);
    data->sell = message_create_window(obj->layer_id, 1, 0xBE, 0x3D);
    data->cursor = cursor = message_create_cursor(obj->layer_id, 0, 0xB0, 0x2F);
    cursor->show(cursor, 0);
}

/* Draws the shop's frames and its scrolling background. */
void stitshop_main_draw(StitshopMain *obj, StitshopMainData *data) {
    Sprite spr;
    s32 scroll;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, 1);
    spr.set_vram_pos(0x280, 0x100);
    if (obj->shopkeeper_anim.level != 0) {
        if (obj->shopkeeper_anim.level != 0x1000) {
            spr.set_scale(obj->shopkeeper_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x57, 0x19);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 1, 0x16, 0x12);
    }
    if (obj->money_anim.level != 0) {
        if (obj->money_anim.level != 0x1000) {
            spr.set_scale(obj->money_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x18);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 2, 0xD6, 0xF);
    }
    if (obj->menu_anim.level != 0) {
        if (obj->menu_anim.level != 0x1000) {
            spr.set_scale(obj->menu_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x3B);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 5, 0xA9, 0x26);
    }
    if (obj->help_anim.level != 0) {
        if (obj->help_anim.level != 0x1000) {
            spr.set_scale(obj->help_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0xD2);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0, 0xC6, 0xC4);
    }
    spr.set_layer_id(obj->layer_id, 7);
    if (obj->odd_frame != 0) {
        scroll = 0;
        if (++obj->scroll < 0x60) {
            scroll = obj->scroll;
        }
        obj->scroll = scroll;
        obj->odd_frame = 0;
    } else {
        obj->odd_frame = 1;
    }
    spr.set_scale(0x1000, 0x1000, 0x1000);
    spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0x14, obj->scroll, obj->scroll);
}

/* The shop's steps (base.step): greet, choose buy or sell, run it, leave with a fade. */
void stitshop_main_run(StitshopMain *obj, StitshopMainData *data) {
    s32 old;
    Fade *fade;

    switch (obj->base.step) {
    case 0:
    default:
        stitshop_funcs.anim_start(&obj->shopkeeper_anim, 1);
        stitshop_funcs.anim_start(&obj->money_anim, 1);
        obj->base.step++;
        break;
    case 1:
        stitshop_funcs.anim_update(&obj->shopkeeper_anim);
        if (stitshop_funcs.anim_update(&obj->money_anim)) {
            stitshop_funcs.anim_start(&obj->menu_anim, 1);
            stitshop_funcs.anim_start(&obj->help_anim, 1);
            data->shopkeeper->set_text(data->shopkeeper, cdload_module.files.get_file(records_language + 0x94),
                                  stitshop_shop_names[obj->shop]);
            data->money_label->set_text(data->money_label, cdload_module.files.get_file(records_language + 0x71), 2);
            data->money->set_line_number(data->money, 0, gamestate_data.money);
            data->money->measure(data->money, 1);
            obj->base.step++;
        }
        break;
    case 2:
        stitshop_funcs.anim_update(&obj->menu_anim);
        if (stitshop_funcs.anim_update(&obj->help_anim)) {
            data->buy->set_text(data->buy, cdload_module.files.get_file(records_language + 0x71), 3);
            data->sell->set_text(data->sell, cdload_module.files.get_file(records_language + 0x71), 4);
            data->cursor->show(data->cursor, 1);
            data->help->set_text(data->help, cdload_module.files.get_file(records_language + 0x71), 5);
            obj->base.step++;
        }
        break;
    case 3:
        old = obj->cursor;
        if (PAD_PRESSED(4)) {
            obj->cursor = 0;
        } else if (PAD_PRESSED(6)) {
            obj->cursor = 1;
        }
        if (old != obj->cursor) {
            sound_module.play(0x8004513E);
            data->cursor->set_pos(data->cursor, 0xB0, obj->cursor * 14 + 0x2F);
        }
        if (PAD_PRESSED(0xD)) {
            sound_module.play(0x8004503C);
            obj->base.step = 20;
            obj->base.substep = 1;
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->base.set_step(obj, 20);
        }
        break;
    case 10:
        if (obj->cursor == 0) {
            data->dealing = (Object *)stitshop_buy_create(obj);
        } else {
            data->dealing = (Object *)stitshop_sell_create(obj);
        }
        obj->base.step++;
        break;
    case 11:
        if (data->dealing == NULL) {
            stitshop_funcs.anim_start(&obj->menu_anim, 1);
            stitshop_funcs.anim_start(&obj->help_anim, 1);
            obj->base.step = 2;
        }
        break;
    case 20:
        if (obj->base.substep == 0) {
            data->fade = fade = stitshop_fade_create();
            fade->start(fade, 0, 30);
        }
        stitshop_funcs.anim_start(&obj->menu_anim, 0);
        stitshop_funcs.anim_start(&obj->help_anim, 0);
        data->help->set_visible(data->help, 0);
        data->buy->set_visible(data->buy, 0);
        data->sell->set_visible(data->sell, 0);
        data->cursor->show(data->cursor, 0);
        obj->base.step++;
        break;
    case 21:
        stitshop_funcs.anim_update(&obj->menu_anim);
        if (stitshop_funcs.anim_update(&obj->help_anim)) {
            if (obj->base.substep != 0) {
                obj->base.set_step(obj, 10);
            } else {
                stitshop_funcs.anim_start(&obj->shopkeeper_anim, 0);
                stitshop_funcs.anim_start(&obj->money_anim, 0);
                data->shopkeeper->set_visible(data->shopkeeper, 0);
                data->money_label->set_visible(data->money_label, 0);
                data->money->set_visible(data->money, 0);
                obj->base.step++;
            }
        }
        break;
    case 22:
        stitshop_funcs.anim_update(&obj->shopkeeper_anim);
        if (stitshop_funcs.anim_update(&obj->money_anim)) {
            obj->base.step++;
        }
        break;
    case 23:
        if (data->fade->base.state == OBJECT_STATE_DONE) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}

void stitshop_main_update(StitshopMain *obj, StitshopMainData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            stitshop_funcs.load();
            obj->base.step++;
            break;
        case 1:
            if (stitshop_funcs.is_loading() == 0) {
                stitshop_main_create_windows(obj, data);
                obj->shopkeeper_anim.duration = 10;
                obj->money_anim.duration = 10;
                obj->menu_anim.duration = 10;
                obj->help_anim.duration = 10;
                obj->base.next_state(obj);
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        stitshop_main_run(obj, data);
        stitshop_main_draw(obj, data);
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        gamestate_data.funcs.set_next_map(gamestate_data.field_map, 0);
        break;
    }
}

/* Shows the player's money (StitshopMain.show_money). */
void stitshop_main_show_money(StitshopMain *obj) {
    StitshopMainData *data = (StitshopMainData *)obj->base.children;

    data->money->set_line_number(data->money, 0, gamestate_data.money);
    data->money->measure(data->money, 1);
}

StitshopMain *stitshop_main_create(void) {
    StitshopMain *obj = object_new(stitshop_main_update, sizeof(StitshopMain), sizeof(StitshopMainData));

    obj->show_money = stitshop_main_show_money;
    obj->layer_id = 0x1000;
    obj->shop = gamestate_data.funcs.get_map_entry();
    return obj;
}

/* Starts loading the overlay's files (stitshop_funcs.load). */
void stitshop_load_files(void) {
    Tim tim;

    tim_init(&tim);
    tim.set_image_pos(0x280, 0x100);
    tim.load_all(cdload_module.get_subfile_by_id(0x04030000));
    cdload_module.queue_file(records_language + 0x71);
    cdload_module.queue_file(records_language + 0x6A);
    cdload_module.queue_file(records_language + 0x63);
    cdload_module.queue_file(records_language + 0x94);
}

/* Returns non-zero while one of the overlay's files is still loading (stitshop_funcs.is_loading). */
s32 stitshop_is_loading(void) {
    if (!cdload_module.is_loading(records_language + 0x71) && !cdload_module.is_loading(records_language + 0x6A) &&
        !cdload_module.is_loading(records_language + 0x63)) {
        return cdload_module.is_loading(records_language + 0x94) != 0;
    }
    return 1;
}

/* window_anim_start (include/window_anim.h), byte for byte (stitshop_funcs.anim_start). */
void stitshop_anim_start(WindowAnim *anim, s32 open) {
    anim->running = 1;
    if (open) {
        sound_module.play(0x40019);
        anim->step = 0x1000 / anim->duration;
        anim->level = 0;
    } else {
        sound_module.play(0x4001A);
        anim->level = 0x1000;
        anim->step = -(0x1000 / anim->duration * 2);
    }
}

/* window_anim_update (include/window_anim.h), byte for byte (stitshop_funcs.anim_update). */
s32 stitshop_anim_update(WindowAnim *anim) {
    if (anim->running == 0) {
        return 1;
    }
    anim->level += anim->step;
    if (anim->step > 0) {
        if (anim->level > 0x1000) {
            anim->level = 0x1000;
            anim->running = 0;
            return 1;
        }
    } else if (anim->level < 0) {
        anim->level = 0;
        anim->running = 0;
        return 1;
    }
    return 0;
}

/* stitshop_funcs.tween_start (unused here). */
void stitshop_tween_start(Tween *obj, s32 from, s32 to, s32 frames) {
    if (from != to) {
        obj->duration = frames;
        obj->acc = from << 8;
        obj->value = from;
        obj->target = to;
        obj->running = 1;
        obj->step = ((to - from) << 8) / obj->duration;
    }
}

/* stitshop_funcs.tween_update (unused here). */
s32 stitshop_tween_update(Tween *obj) {
    if (obj->running == 0) {
        return 1;
    }
    obj->acc += obj->step;
    obj->value = obj->acc >> 8;
    if (obj->step > 0) {
        if (obj->value > obj->target) {
            obj->value = obj->target;
            obj->running = 0;
            return 1;
        }
    } else if (obj->value < obj->target) {
        obj->value = obj->target;
        obj->running = 0;
        return 1;
    }
    return 0;
}

/* Chooses shop `shop`'s goods: sets stitshop_funcs.count and returns the list (stitshop_funcs.get_items). */
s16 *stitshop_get_items(s32 shop) {
    StitshopShop *s;

    if (shop < 0 || stitshop_shops[shop].items == NULL) {
        return NULL;
    }
    s = &stitshop_shops[shop];
    stitshop_funcs.count = s->count;
    return s->items;
}

/* Whether Digimon `digimon` can equip `item` (stitshop_funcs.can_equip). */
s32 stitshop_can_equip(s32 digimon, s32 item) {
    return (((RecordsEquip *)records_funcs.get_item(item)->data)->members >> digimon) & 1;
}

/* The equipment slot `item` goes to on Digimon `digimon` (stitshop_funcs.get_slot): for kinds that take one of
 * two slots, the free one, else the one whose item is weaker; -1: none. */
s32 stitshop_get_slot(s32 digimon, s32 item) {
    RecordsEquip *equip;
    GamestateRecord *rec;
    RecordsItem *entry;
    RecordsWeapon *a;
    RecordsWeapon *b;
    s32 ids[2];
    RecordsAccessory *p[2];
    s32 k;
    s32 n0;
    s32 n1;
    u8 type;

    equip = records_funcs.get_item(item)->data;
    switch (equip->slot) {
    case 3:
        rec = gamestate_data.funcs.get_record(digimon);
        n0 = rec->equipment[2];
        if (n0 <= 0) {
            return 2;
        }
        ids[0] = n0;
        n1 = rec->equipment[3];
        if (n1 <= 0) {
            return 3;
        }
        ids[1] = n1;
        a = records_funcs.get_item(ids[0])->data;
        entry = records_funcs.get_item(ids[1]);
        b = entry->data;
        if (a->slot == 7 || entry->type == 20) {
            return 2;
        }
        for (k = 0; k < 2; k++) {
            type = records_funcs.get_item(ids[k])->type;
            if (type < 2 || type > 14) {
                return -1;
            }
        }
        if (a->power < b->power) {
            return 2;
        }
    case 2:
        return 3;
    case 4:
        return 0;
    case 5:
        return 1;
    case 6:
        rec = gamestate_data.funcs.get_record(digimon);
        n0 = rec->equipment[4];
        if (n0 <= 0) {
            return 4;
        }
        ids[0] = n0;
        n1 = rec->equipment[5];
        if (n1 <= 0) {
            return 5;
        }
        ids[1] = n1;
        p[0] = records_funcs.get_item(ids[0])->data;
        p[1] = records_funcs.get_item(ids[1])->data;
        if (p[0]->bonus_value < p[1]->bonus_value) {
            return 4;
        }
        return 5;
    case 1:
    case 7:
        return 2;
    case 8:
        rec = gamestate_data.funcs.get_record(digimon);
        n0 = rec->equipment[4];
        if (n0 <= 0) {
            return 4;
        }
        ids[0] = n0;
        n1 = rec->equipment[5];
        if (n1 <= 0) {
            return 5;
        }
        ids[1] = n1;
        p[0] = records_funcs.get_item(ids[0])->data;
        if (p[0]->slot == 8 && p[0]->group == equip->group) {
            return 4;
        }
        p[1] = records_funcs.get_item(ids[1])->data;
        if (p[1]->slot == 8 && p[1]->group == equip->group) {
            return 5;
        }
        break;
    case 0:
    default:
        return -1;
    }
    if (p[0]->bonus_value < p[1]->bonus_value) {
        return 4;
    }
    return 5;
}

/* Equips Digimon `digimon` with `item` in `slot` (stitshop_funcs.equip): takes off what is there (an item that
 * takes slots 2 and 3, and accessories of the same group), and with `take` moves the items between the bag
 * (unk_007C) and the equipped counts (unk_020F). */
void stitshop_equip(s32 digimon, s32 slot, s32 item, s32 take) {
    GamestateRecord *rec;
    RecordsEquip *equip;
    s16 *p;
    s16 *q;
    s32 old;
    s32 i;
    s32 group;
    s32 id;

    rec = gamestate_data.funcs.get_record(digimon);
    old = *(rec->equipment + slot);
    id = item; /* the original copies item to a second register here (move s4,s0) */
    if (old != 0) {
        if (take) {
            gamestate_data.items_equipped[old]--;
            gamestate_data.items[old]++;
        }
        equip = records_funcs.get_item(old)->data;
        if (equip->slot == 7) {
            rec->equipment[2] = 0;
            rec->equipment[3] = 0;
        } else {
            *(rec->equipment + slot) = 0;
        }
    }
    if (id > 0) {
        equip = records_funcs.get_item(id)->data;
        if (equip->slot == 7) {
            if (rec->equipment[2] != 0) {
                p = &rec->equipment[2];
            } else if (rec->equipment[3] != 0) {
                p = &rec->equipment[3];
            } else {
                p = NULL;
            }
            if (p != NULL) {
                if (take) {
                    gamestate_data.items_equipped[*p]--;
                    gamestate_data.items[*p]++;
                }
                *p = 0;
            }
        } else if (equip->slot == 8) {
            group = equip->group;
            for (i = 0; i < 2; i++) {
                q = &rec->equipment[i + 4];
                if (*q != 0) {
                    equip = records_funcs.get_item(*q)->data;
                    if (equip->group == group) {
                        if (take) {
                            gamestate_data.items_equipped[*q]--;
                            gamestate_data.items[*q]++;
                        }
                        *q = 0;
                    }
                }
            }
        }
        if (take) {
            gamestate_data.items_equipped[id]++;
            gamestate_data.items[id]--;
        }
        equip = records_funcs.get_item(id)->data;
        if (equip->slot == 7) {
            rec->equipment[2] = id;
            rec->equipment[3] = id;
        } else {
            *(rec->equipment + slot) = id;
        }
    }
}

s32 stitshop_sell_kinds[4] = { 1, 2, 3, 4 };
StitshopPos stitshop_info_mark_pos[6] = {
    { 23, 172 }, { 23, 186 }, { 23, 200 }, { 65, 200 }, { 23, 214 }, { 65, 214 },
};
s32 stitshop_info_stats[13] = {
    6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
};
s32 stitshop_slot_texts[9] = { 0, 35, 36, 37, 38, 39, 40, 41, 42 };
s32 stitshop_shop_names[30] = {
    17, 1, 5, 18, 6, 25, 13, 24, 4, 12,
    29, 30, 16, 21, 9, 19, 2, 7, 20, 8,
    26, 14, 23, 3, 11, 27, 28, 15, 22, 10,
};

s16 stitshop_goods_0[] = {
    92, 106, 157, 167, 190, 215, 226, 236, 249, 262, 275, 0,
};
s16 stitshop_goods_1[] = {
    292, 294, 296, 298, 300, 302, 304, 305, 306, 307, 308, 309,
    310, 311, 312, 313, 314, 315, 0,
};
s16 stitshop_goods_2[] = {
    43, 66, 67, 69, 70, 71, 72, 73, 74, 75, 76, 77,
    78, 79, 80, 81, 82, 83, 84, 0,
};
s16 stitshop_goods_3[] = {
    96, 110, 122, 134, 147, 161, 171, 178, 185, 194, 201, 209,
    219, 230, 240, 253, 266, 279, 0,
};
s16 stitshop_goods_4[] = {
    43, 44, 0,
};
s16 stitshop_goods_5[] = {
    93, 107, 158, 168, 191, 216, 227, 237, 250, 263, 276, 0,
};
s16 stitshop_goods_6[] = {
    43, 66, 67, 69, 70, 71, 72, 73, 74, 75, 76, 77,
    78, 79, 80, 81, 82, 83, 84, 0,
};
s16 stitshop_goods_7[] = {
    94, 108, 159, 169, 192, 217, 228, 238, 251, 264, 277, 0,
};
s16 stitshop_goods_8[] = {
    292, 294, 296, 298, 300, 302, 304, 305, 306, 307, 308, 309,
    310, 311, 312, 313, 314, 315, 0,
};
s16 stitshop_goods_9[] = {
    43, 66, 67, 69, 70, 71, 72, 73, 74, 75, 76, 77,
    78, 79, 80, 81, 82, 83, 84, 0,
};
s16 stitshop_goods_10[] = {
    95, 109, 120, 132, 145, 160, 170, 176, 183, 193, 199, 207,
    218, 229, 239, 252, 265, 278, 0,
};
s16 stitshop_goods_11[] = {
    95, 109, 120, 132, 145, 160, 170, 176, 183, 193, 199, 207,
    218, 229, 239, 252, 265, 278, 0,
};
s16 stitshop_goods_12[] = {
    43, 44, 66, 67, 69, 70, 71, 72, 73, 74, 75, 76,
    77, 78, 79, 80, 81, 82, 83, 84, 0,
};
s16 stitshop_goods_13[] = {
    97, 111, 123, 135, 148, 162, 172, 179, 186, 195, 202, 210,
    220, 231, 241, 254, 267, 280, 0,
};
s16 stitshop_goods_14[] = {
    43, 44, 66, 67, 69, 70, 71, 72, 73, 74, 75, 76,
    77, 78, 79, 80, 81, 82, 83, 84, 0,
};
s16 stitshop_goods_15[] = {
    101, 115, 127, 140, 152, 166, 175, 182, 189, 198, 206, 214,
    225, 235, 248, 261, 274, 291, 0,
};
s16 stitshop_goods_16[] = {
    293, 295, 297, 299, 301, 303, 304, 305, 306, 307, 308, 309,
    310, 311, 312, 313, 314, 315, 0,
};
s16 stitshop_goods_17[] = {
    43, 44, 45, 66, 67, 69, 70, 71, 72, 73, 74, 75,
    76, 77, 78, 79, 80, 81, 82, 83, 84, 0,
};
s16 stitshop_goods_18[] = {
    98, 112, 124, 136, 137, 149, 163, 203, 211, 221, 222, 232,
    242, 243, 244, 245, 255, 256, 257, 258, 268, 269, 270, 271,
    281, 282, 283, 284, 285, 286, 287, 288, 0,
};
s16 stitshop_goods_19[] = {
    47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
    59, 60, 61, 0,
};
s16 stitshop_goods_20[] = {
    96, 110, 122, 134, 147, 161, 171, 178, 185, 194, 201, 209,
    219, 230, 240, 253, 266, 279, 0,
};
s16 stitshop_goods_21[] = {
    43, 44, 66, 67, 69, 70, 71, 72, 73, 74, 75, 76,
    77, 78, 79, 80, 81, 82, 83, 84, 0,
};
s16 stitshop_goods_22[] = {
    97, 111, 123, 135, 148, 162, 172, 179, 186, 195, 202, 210,
    220, 231, 241, 254, 267, 280, 0,
};
s16 stitshop_goods_23[] = {
    292, 294, 296, 298, 300, 302, 304, 305, 306, 307, 308, 309,
    310, 311, 312, 313, 314, 315, 0,
};
s16 stitshop_goods_24[] = {
    43, 44, 66, 67, 69, 70, 71, 72, 73, 74, 75, 76,
    77, 78, 79, 80, 81, 82, 83, 84, 0,
};
s16 stitshop_goods_25[] = {
    99, 113, 125, 138, 150, 164, 173, 180, 187, 196, 204, 212,
    223, 233, 246, 259, 272, 289, 0,
};
s16 stitshop_goods_26[] = {
    100, 114, 126, 139, 151, 165, 174, 181, 188, 197, 205, 213,
    224, 234, 247, 260, 273, 290, 0,
};
s16 stitshop_goods_27[] = {
    43, 44, 66, 67, 69, 70, 71, 72, 73, 74, 75, 76,
    77, 78, 79, 80, 81, 82, 83, 84, 0,
};
s16 stitshop_goods_28[] = {
    100, 114, 126, 139, 151, 165, 174, 181, 188, 197, 205, 213,
    224, 234, 247, 260, 273, 290, 0,
};
s16 stitshop_goods_29[] = {
    43, 44, 66, 67, 69, 70, 71, 72, 73, 74, 75, 76,
    77, 78, 79, 80, 81, 82, 83, 84, 0,
};
s16 stitshop_goods_30[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
    37, 38, 39, 40, 41, 42, 360, 394, 395, 396, 397, 398,
    399, 400, 401, 402, 0,
};

/* by shop */
StitshopShop stitshop_shops[] = {
    { 11, stitshop_goods_0 },
    { 18, stitshop_goods_1 },
    { 19, stitshop_goods_2 },
    { 18, stitshop_goods_3 },
    { 2, stitshop_goods_4 },
    { 11, stitshop_goods_5 },
    { 19, stitshop_goods_6 },
    { 11, stitshop_goods_7 },
    { 18, stitshop_goods_8 },
    { 19, stitshop_goods_9 },
    { 18, stitshop_goods_10 },
    { 18, stitshop_goods_11 },
    { 20, stitshop_goods_12 },
    { 18, stitshop_goods_13 },
    { 20, stitshop_goods_14 },
    { 18, stitshop_goods_15 },
    { 18, stitshop_goods_16 },
    { 21, stitshop_goods_17 },
    { 32, stitshop_goods_18 },
    { 15, stitshop_goods_19 },
    { 18, stitshop_goods_20 },
    { 20, stitshop_goods_21 },
    { 18, stitshop_goods_22 },
    { 18, stitshop_goods_23 },
    { 20, stitshop_goods_24 },
    { 18, stitshop_goods_25 },
    { 18, stitshop_goods_26 },
    { 20, stitshop_goods_27 },
    { 18, stitshop_goods_28 },
    { 20, stitshop_goods_29 },
    { 52, stitshop_goods_30 },
};

StitshopFuncs stitshop_funcs = {
    0,
    stitshop_load_files,
    stitshop_is_loading,
    stitshop_anim_start,
    stitshop_anim_update,
    stitshop_tween_start,
    stitshop_tween_update,
    stitshop_get_items,
    stitshop_can_equip,
    stitshop_get_slot,
    stitshop_equip,
};
