#include "common.h"
#include "object.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "psyq/libgpu.h"
#include "stitshop.h"

/* STITSHOP.PRO: the item shop. This file holds the overlay's entry point, the fade and buying (the second
 * file, stitshop_800859C0.c, selling, the item list, the item's information and the main object). */

/* Buying's data block (0x24 bytes). */
typedef struct StitshopBuyData {
    /* 0x00 */ StitshopList *list;
    /* 0x04 */ StitshopInfo *info;
    /* 0x08 */ MessageWindow *how_many; /* "how many?" */
    /* 0x0C */ MessageWindow *times;
    /* 0x10 */ MessageWindow *number; /* the number */
    /* 0x14 */ MessageWindow *messages; /* messages */
    /* 0x18 */ MessageWindow *yes;    /* yes */
    /* 0x1C */ MessageWindow *no;     /* no */
    /* 0x20 */ MessageCursor *cursor; /* the cursor */
} StitshopBuyData; /* size 0x24 */

void stitshop_buy_run(StitshopBuy *obj, StitshopBuyData *data);

/* The overlay's first object: sets up the display and creates the shop. */
void stitshop_update_main(Object *obj, StitshopMain **data) {
    RECT rect;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0x14000);
        gfx_module.funcs.init_display(320, 240, 0, 0);
        rect.x = 0;
        rect.y = 0;
        rect.w = 320;
        rect.h = 240;
        layer = gfx_module.funcs.create_layer(&rect, 3, 0x1000);
        layer->set_bg_color(layer, 0, 0, 0);
        *data = stitshop_main_create();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The overlay's entry point (overlay_entries). */
Object *stitshop_start(void) {
    return object_new(stitshop_update_main, sizeof(Object), 4);
}

void stitshop_fade_start(Fade *obj, s32 dir, s32 frames) {
    obj->base.set_state(obj, OBJECT_STATE_RUN);
    obj->base.step = 1;
    obj->from_black = dir;
    if (dir == 0) {
        obj->level = 0;
        obj->step = 0xFF00 / frames;
    } else {
        obj->level = 0xFF00;
        obj->step = -(0xFF00 / frames);
    }
}

void stitshop_fade_draw(Fade *obj) {
    GfxLayer *layer;
    u32 *ot;
    POLY_F4 *poly;
    DR_TPAGE *tpage;

    layer = gfx_module.funcs.get_layer(obj->layer_id);
    ot = layer->get_ot_entry(layer, obj->ot_depth);
    poly = gfx_module.funcs.get_packet();
    setPolyF4(poly);
    setSemiTrans(poly, 1);
    poly->r0 = poly->g0 = poly->b0 = obj->level >> 8;
    setXYWH(poly, 0, 0, 320, 256);
    addPrim(ot, poly);
    tpage = (DR_TPAGE *)(poly + 1);
    setDrawTPage(tpage, 0, 1, getTPage(0, 2, 320, 0));
    addPrim(ot, tpage);
    gfx_module.funcs.set_packet(tpage + 1);
}

void stitshop_fade_update(Fade *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->base.step != 0) {
            obj->level += obj->step;
            if (obj->from_black == 0) {
                if (obj->level > 0xFF00) {
                    obj->level = 0xFF00;
                    obj->base.state = OBJECT_STATE_DONE;
                }
            } else if (obj->level < 0) {
                obj->level = 0;
                obj->base.state = OBJECT_STATE_DONE;
            }
        case OBJECT_STATE_DONE:
            stitshop_fade_draw(obj);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Fade *stitshop_fade_create(void) {
    Fade *obj = object_new(stitshop_fade_update, sizeof(Fade), 0);

    obj->start = stitshop_fade_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    return obj;
}

/* Creates buying's text windows and cursor. */
void stitshop_buy_create_windows(StitshopBuy *obj, StitshopBuyData *data) {
    MessageCursor *cursor;

    data->how_many = message_create_window(obj->layer_id, 1, 0xB9, 0x3A);
    data->times = message_create_window(obj->layer_id, 1, 0x103, 0x54);
    data->number = message_create_window(obj->layer_id, 1, 0x11E, 0x54);
    data->messages = message_create_window(obj->layer_id, 1, 0x9A, 0x2C);
    data->yes = message_create_window(obj->layer_id, 1, 0xC5, 0x49);
    data->no = message_create_window(obj->layer_id, 1, 0xC5, 0x59);
    data->cursor = cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0xB8, 0x49);
    cursor->show(cursor, 0);
}

/* Shows (`show`) or hides "how many?" and the number. */
void stitshop_buy_show_number(StitshopBuy *obj, StitshopBuyData *data, s32 show) {
    if (show) {
        data->how_many->set_text(data->how_many, cdload_module.files.get_file(records_language + 0x71), 0x11);
        data->times->set_text(data->times, cdload_module.files.get_file(records_language + 0x71), 8);
        data->number->set_line_number(data->number, 0, obj->count);
        data->number->measure(data->number, 1);
    } else {
        data->how_many->set_visible(data->how_many, 0);
        data->times->set_visible(data->times, 0);
        data->number->set_visible(data->number, 0);
    }
}

/* Shows (`show`) or hides the price and "buy?" with yes/no. */
void stitshop_buy_show_confirm(StitshopBuy *obj, StitshopBuyData *data, s32 show) {
    if (show) {
        data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x71), 0x12);
        data->messages->set_line_number(data->messages, 1, records_funcs.get_item(obj->item)->price * obj->count);
        data->yes->set_text(data->yes, cdload_module.files.get_file(records_language + 0x71), 0x13);
        data->no->set_text(data->no, cdload_module.files.get_file(records_language + 0x71), 0x14);
    } else {
        data->messages->set_visible(data->messages, 0);
        data->yes->set_visible(data->yes, 0);
        data->no->set_visible(data->no, 0);
    }
}

/* Draws buying's windows and the party member cursor. */
void stitshop_buy_draw(StitshopBuy *obj, StitshopBuyData *data) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    spr.set_vram_pos(0x280, 0x100);
    if (obj->count_anim.level != 0) {
        spr.set_scale(obj->count_anim.level, 0x1000, 0x1000);
        if (obj->count_anim.level != 0x1000) {
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
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0x15, 0x7B, 0x26);
        if (obj->question_anim.level != 0x1000) {
            spr.set_pivot(0x140, 0x57);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0xC, 0xAF, 0x43);
    }
    if (obj->message_anim.level != 0) {
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 2);
        if (obj->message_anim.level != 0x1000) {
            spr.set_scale(obj->message_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x32);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0x15, 0x7B, 0x26);
    }
    if (obj->equipped_anim.level != 0) {
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 2);
        if (obj->equipped_anim.level != 0x1000) {
            spr.set_scale(obj->equipped_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x32);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0x15, 0x7B, 0x26);
    }
    if (obj->member_cursor_shown != 0) {
        if (gfx_module.funcs.get_time() - obj->member_cursor_time >= 9) {
            obj->member_cursor_time = gfx_module.funcs.get_time();
            if (++obj->member_cursor_step >= 8) {
                obj->member_cursor_step = 0;
            }
        }
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 2);
        spr.set_scale(0x1000, 0x1000, 0x1000);
        spr.set_palette(obj->member_cursor_step);
        spr.draw(cdload_module.get_subfile_by_id(0x04020000), 0xF, obj->member * 0x63 + 0x10, 0x9C);
    }
}

/* Buying's steps (base.step): choose an item, how many, confirm, pay, and maybe equip it on a party
 * member. */
void stitshop_buy_run(StitshopBuy *obj, StitshopBuyData *data) {
    s32 old;
    s32 n;
    s32 i;
    s32 digimon;
    u16 price;

    switch (obj->base.step) {
    case 0:
    default:
        if (data->list == NULL) {
            data->list = stitshop_list_create(obj, obj->main->shop, 0);
        }
        obj->base.step++;
        break;
    case 1:
        if (data->list->base.state == OBJECT_STATE_DONE) {
            if (data->info == NULL) {
                data->info = stitshop_info_create(0, data->list->get_item(data->list));
            }
            obj->base.step++;
        }
        break;
    case 2:
        if (data->info->base.step == 3) {
            data->list->set_input(data->list, 1);
            obj->base.step++;
        }
        break;
    case 3:
        if (PAD_PRESSED(0xD)) {
            sound_module.play(0x8004503C);
            obj->item = data->list->get_item(data->list);
            if (gamestate_data.items[obj->item] == 99) {
                data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x71), 0x1C);
                data->messages->set_visible(data->messages, 0);
                obj->base.step = 45;
            } else if (gamestate_data.money < records_funcs.get_item(obj->item)->price) {
                data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x71), 0x1B);
                data->messages->set_visible(data->messages, 0);
                obj->base.step = 45;
            } else {
                data->list->set_input(data->list, 0);
                obj->base.step = 5;
            }
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            data->list->close(data->list);
            data->info->close(data->info);
            obj->base.step = 50;
        } else if (PAD_PRESSED(0xC) && data->info->base.step == 3) {
            data->info->turn_page(data->info);
            data->list->grey_cursor(data->list, 1);
            obj->base.step = 4;
        }
        break;
    case 4:
        if (data->info->base.step == 3) {
            data->list->grey_cursor(data->list, 0);
            obj->base.step = 3;
        }
        break;
    case 5:
        data->list->close(data->list);
        price = records_funcs.get_item(obj->item)->price;
        if (price == 0) {
            price = 1;
        }
        obj->max = gamestate_data.money / price;
        if (obj->max + gamestate_data.items[obj->item] >= 100) {
            obj->max = 99 - gamestate_data.items[obj->item];
        }
        obj->base.step++;
        break;
    case 6:
        if (data->list->base.state == OBJECT_STATE_DONE) {
            stitshop_funcs.anim_start(&obj->count_anim, 1);
            obj->base.step++;
        }
        break;
    case 7:
        if (stitshop_funcs.anim_update(&obj->count_anim)) {
            stitshop_buy_show_number(obj, data, 1);
            obj->base.step++;
        }
        break;
    case 8:
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
            price = records_funcs.get_item(obj->item)->price;
            if (gamestate_data.money < price * obj->count) {
                obj->count = gamestate_data.money / price;
            }
            stitshop_buy_show_number(obj, data, 1);
            data->info->set_item(data->info, obj->item, obj->count);
            sound_module.play(0x4001B);
        } else if (PAD_PRESSED(0xD)) {
            sound_module.play(0x4001C);
            obj->base.next_step(obj);
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->base.next_step(obj);
            obj->base.substep = 1;
            obj->count = 1;
            data->info->set_item(data->info, obj->item, 1);
        } else if (PAD_PRESSED(0xC) && data->info->base.step == 3) {
            data->info->turn_page(data->info);
            obj->base.step = 12;
        }
        break;
    case 9:
        stitshop_buy_show_number(obj, data, 0);
        stitshop_funcs.anim_start(&obj->count_anim, 0);
        obj->base.step++;
        break;
    case 10:
        if (stitshop_funcs.anim_update(&obj->count_anim)) {
            if (obj->base.substep != 0) {
                data->list->resume(data->list);
                obj->base.next_step(obj);
            } else {
                obj->base.step = 15;
            }
        }
        break;
    case 11:
        if (data->list->base.state == OBJECT_STATE_DONE) {
            data->list->set_input(data->list, 1);
            obj->base.step = 2;
        }
        break;
    case 12:
        if (data->info->base.step == 3) {
            obj->base.step = 8;
        }
        break;
    case 15:
        stitshop_funcs.anim_start(&obj->question_anim, 1);
        obj->base.step++;
        break;
    case 16:
        if (stitshop_funcs.anim_update(&obj->question_anim)) {
            stitshop_buy_show_confirm(obj, data, 1);
            obj->cursor = 0;
            data->cursor->set_pos(data->cursor, 0xB8, 0x49);
            data->cursor->show(data->cursor, 1);
            obj->base.step++;
        }
        break;
    case 17:
        old = obj->cursor;
        if (PAD_PRESSED(4)) {
            obj->cursor = 0;
        } else if (PAD_PRESSED(6)) {
            obj->cursor = 1;
        }
        if (old != obj->cursor) {
            sound_module.play(0x8004513E);
            data->cursor->set_pos(data->cursor, 0xB8, obj->cursor * 16 + 0x49);
        } else if (data->info->shown != 0 && PAD_PRESSED(0xD)) {
            sound_module.play(0x8004503C);
            obj->base.set_step(obj, 20);
            if (obj->cursor == 0) {
                if (records_funcs.is_item_category(obj->item, 3) || records_funcs.is_item_category(obj->item, 4) ||
                    records_funcs.is_item_category(obj->item, 5)) {
                    obj->base.substep = 1;
                }
                if (gamestate_data.items[obj->item] + obj->count >= 100) {
                    gamestate_data.items[obj->item] = 99;
                } else {
                    gamestate_data.items[obj->item] += obj->count;
                }
                data->info->set_item(data->info, obj->item, obj->count);
                gamestate_data.money -= records_funcs.get_item(obj->item)->price * obj->count;
                obj->main->show_money(obj->main);
            }
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->base.set_step(obj, 20);
        } else if (PAD_PRESSED(0xC) && data->info->base.step == 3) {
            data->info->turn_page(data->info);
            obj->base.step = 18;
        }
        break;
    case 18:
        if (data->info->base.step == 3) {
            obj->base.step = 17;
        }
        break;
    case 20:
        obj->count = 1;
        data->info->set_item(data->info, obj->item, 1);
        stitshop_buy_show_confirm(obj, data, 0);
        data->cursor->show(data->cursor, 0);
        stitshop_funcs.anim_start(&obj->question_anim, 0);
        obj->base.step++;
        break;
    case 21:
        if (stitshop_funcs.anim_update(&obj->question_anim)) {
            if (obj->base.substep != 0) {
                for (i = 0, n = 0; i < 3; i++) {
                    if (stitshop_funcs.can_equip(gamestate_data.funcs.get_party_member(i), obj->item)) {
                        n++;
                    }
                }
                if (n != 0) {
                    obj->base.set_step(obj, 25);
                } else {
                    obj->base.step = 10;
                    obj->base.substep = 1;
                }
            } else {
                obj->base.step = 10;
                obj->base.substep = 1;
            }
        }
        break;
    case 25:
        if (data->info->shown != 0) {
            data->info->base.next_step(data->info);
            stitshop_funcs.anim_start(&obj->question_anim, 1);
            stitshop_funcs.anim_start(&obj->message_anim, 1);
            obj->base.step++;
        }
        break;
    case 26:
        stitshop_funcs.anim_update(&obj->question_anim);
        if (stitshop_funcs.anim_update(&obj->message_anim)) {
            data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x71), 0x15);
            data->yes->set_text(data->yes, cdload_module.files.get_file(records_language + 0x71), 0x16);
            data->no->set_text(data->no, cdload_module.files.get_file(records_language + 0x71), 0x17);
            obj->cursor = 0;
            data->cursor->set_pos(data->cursor, 0xB8, 0x49);
            data->cursor->show(data->cursor, 1);
            obj->base.step++;
        }
        break;
    case 27:
        old = obj->cursor;
        if (PAD_PRESSED(4)) {
            obj->cursor = 0;
        } else if (PAD_PRESSED(6)) {
            obj->cursor = 1;
        }
        if (old != obj->cursor) {
            sound_module.play(0x8004513E);
            data->cursor->set_pos(data->cursor, 0xB8, obj->cursor * 16 + 0x49);
        }
        if (PAD_PRESSED(0xD)) {
            sound_module.play(0x8004503C);
            obj->base.next_step(obj);
            if (obj->cursor == 0) {
                obj->base.substep = 1;
            }
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->base.next_step(obj);
        }
        break;
    case 28:
        data->messages->set_visible(data->messages, 0);
        data->yes->set_visible(data->yes, 0);
        data->no->set_visible(data->no, 0);
        data->cursor->show(data->cursor, 0);
        stitshop_funcs.anim_start(&obj->question_anim, 0);
        if (obj->base.substep == 0) {
            stitshop_funcs.anim_start(&obj->message_anim, 0);
        }
        obj->base.step++;
        break;
    case 29:
        if (obj->base.substep == 0) {
            stitshop_funcs.anim_update(&obj->message_anim);
        }
        if (stitshop_funcs.anim_update(&obj->question_anim)) {
            if (obj->base.substep != 0) {
                obj->base.set_step(obj, 30);
            } else {
                obj->base.step = 10;
                obj->base.substep = 1;
                data->info->base.next_step(data->info);
            }
        }
        break;
    case 30:
        stitshop_funcs.anim_start(&obj->message_anim, 1);
        obj->base.step++;
        break;
    case 31:
        if (stitshop_funcs.anim_update(&obj->message_anim)) {
            if (obj->base.substep == 0) {
                obj->member = 0;
                while (!stitshop_funcs.can_equip(gamestate_data.funcs.get_party_member(obj->member), obj->item) &&
                       ++obj->member < 3) {
                }
            }
            obj->member_cursor_shown = 1;
            data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x71), 0x10);
            obj->base.step++;
        }
        break;
    case 32:
        old = obj->member;
        if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
            do {
                if (--obj->member < 0) {
                    obj->member = 0;
                    break;
                }
            } while (!stitshop_funcs.can_equip(gamestate_data.funcs.get_party_member(obj->member), obj->item));
        } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
            do {
                if (++obj->member > data->info->member_count - 1) {
                    obj->member = data->info->member_count - 1;
                    break;
                }
            } while (!stitshop_funcs.can_equip(gamestate_data.funcs.get_party_member(obj->member), obj->item));
        }
        if (old != obj->member) {
            if (stitshop_funcs.can_equip(gamestate_data.funcs.get_party_member(obj->member), obj->item)) {
                sound_module.play(0x4001B);
            } else {
                obj->member = old;
            }
        } else if (PAD_PRESSED(0xD)) {
            sound_module.play(0x4001C);
            digimon = gamestate_data.funcs.get_party_member(obj->member);
            stitshop_funcs.equip(digimon, stitshop_funcs.get_slot(digimon, obj->item), obj->item, 1);
            data->info->show_member(data->info, obj->member);
            obj->base.step = 36;
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->base.step++;
        }
        break;
    case 33:
        stitshop_funcs.anim_start(&obj->message_anim, 0);
        data->messages->set_visible(data->messages, 0);
        obj->base.step++;
        break;
    case 34:
        if (stitshop_funcs.anim_update(&obj->message_anim)) {
            obj->base.step++;
        }
        break;
    case 35:
        obj->base.step = 10;
        obj->base.substep = 1;
        data->info->base.next_step(data->info);
        obj->member_cursor_shown = 0;
        break;
    case 36:
        obj->member_cursor_shown = 0;
        stitshop_funcs.anim_start(&obj->message_anim, 0);
        data->messages->set_visible(data->messages, 0);
        obj->base.step++;
        break;
    case 37:
        if (stitshop_funcs.anim_update(&obj->message_anim)) {
            stitshop_funcs.anim_start(&obj->equipped_anim, 1);
            obj->base.step++;
        }
        break;
    case 38:
        if (stitshop_funcs.anim_update(&obj->equipped_anim)) {
            data->messages->set_text(data->messages, cdload_module.files.get_file(records_language + 0x71), 9);
            obj->base.step++;
        }
        break;
    case 39:
        if (PAD_PRESSED(0xD)) {
            sound_module.play(0x4001C);
            stitshop_funcs.anim_start(&obj->equipped_anim, 0);
            data->messages->set_visible(data->messages, 0);
            obj->base.step++;
        }
        break;
    case 40:
        if (stitshop_funcs.anim_update(&obj->equipped_anim)) {
            if (gamestate_data.items[obj->item] <= 0) {
                obj->base.step = 35;
            } else {
                obj->base.step = 30;
                obj->base.substep = 1;
            }
        }
        break;
    case 45:
        stitshop_funcs.anim_start(&obj->message_anim, 1);
        data->list->grey_cursor(data->list, 1);
        obj->base.step++;
        break;
    case 46:
        if (stitshop_funcs.anim_update(&obj->message_anim)) {
            data->messages->set_visible(data->messages, 1);
            obj->base.step++;
        }
        break;
    case 47:
        if (PAD_PRESSED(0xD) || PAD_PRESSED(0xE)) {
            sound_module.play(0x4001C);
            data->messages->set_visible(data->messages, 0);
            stitshop_funcs.anim_start(&obj->message_anim, 0);
            obj->base.step++;
        }
        break;
    case 48:
        if (stitshop_funcs.anim_update(&obj->message_anim)) {
            data->list->grey_cursor(data->list, 0);
            obj->base.step = 3;
        }
        break;
    case 50:
        if (data->info == NULL) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}

/* An unreferenced zero word: the rest of this file's .rodata. */
INCLUDE_RODATA("asm/stitshop/nonmatchings/stitshop_8008321C", D_STITSHOP_80082D7C);

void stitshop_buy_update(StitshopBuy *obj, StitshopBuyData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stitshop_buy_create_windows(obj, data);
        obj->count_anim.duration = 10;
        obj->question_anim.duration = 10;
        obj->message_anim.duration = 10;
        obj->equipped_anim.duration = 10;
        obj->count = 1;
        break;
    case OBJECT_STATE_RUN:
        stitshop_buy_run(obj, data);
        stitshop_buy_draw(obj, data);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Shows the item under the list's cursor (StitshopBuy.set_item). */
void stitshop_buy_set_item(StitshopBuy *obj, s32 item, s32 n) {
    StitshopInfo *info = ((StitshopBuyData *)obj->base.children)->info;

    info->set_item(info, item, n);
}

StitshopBuy *stitshop_buy_create(StitshopMain *main) {
    StitshopBuy *obj = object_new(stitshop_buy_update, sizeof(StitshopBuy), sizeof(StitshopBuyData));

    obj->set_item = stitshop_buy_set_item;
    obj->layer_id = 0x1000;
    obj->ot_depth = 4;
    obj->main = main;
    return obj;
}
