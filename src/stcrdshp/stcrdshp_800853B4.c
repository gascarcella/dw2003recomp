#include "common.h"
#include "object.h"
#include "gfx.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "records.h"
#include "card.h"
#include "message.h"
#include "psyq/libgpu.h"
#include "stcrdshp.h"

/* STCRDSHP.PRO's second file: a fade, the card pack (six cards turned over one by one), the purchase
 * screen, the root object and the shop's main object. */

/* The main object's children (its data block). */
typedef struct StcrdshpMainData {
    /* 0x00 */ MessageWindow *shopkeeper; /* the shopkeeper's line */
    /* 0x04 */ MessageWindow *help_line; /* help line */
    /* 0x08 */ MessageWindow *no_boosters; /* "no boosters" */
    /* 0x0C */ MessageWindow *money_label;
    /* 0x10 */ MessageWindow *money;  /* money */
    /* 0x14 */ MessageWindow *menu[3];   /* the menu */
    /* 0x20 */ MessageCursor *menu_cursor; /* the menu cursor */
    /* 0x24 */ Object *screen; /* the buy or booster screen */
    /* 0x28 */ Object *fade;   /* fade */
} StcrdshpMainData; /* size 0x2C */

/* A shop's greeting (stcrdshp_greetings, ended by map == 0). */
typedef struct StcrdshpGreeting {
    /* 0x0 */ s32 map;   /* gamestate_data.field_map */
    /* 0x4 */ s32 message; /* message */
} StcrdshpGreeting; /* size 0x8 */

extern StcrdshpGreeting stcrdshp_greetings[];

/* The buy screen (stcrdshp_update_buy): the shop's cards, six to a page, shown by a card pack. */
typedef struct StcrdshpBuy {
    /* 0x00 */ Object base;
    /* 0x50 */ StcrdshpMain *main;
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 ot_depth; /* ordering table entry */
    /* 0x5C */ s32 cursor_column; /* cursor column */
    /* 0x60 */ s32 cursor_shown; /* cursor shown */
    /* 0x64 */ s32 cursor_frame; /* cursor animation frame, 0..5 */
    /* 0x68 */ s32 frame_time; /* time of the last frame */
    /* 0x6C */ s32 page;   /* page */
    /* 0x70 */ s32 pages;  /* pages */
    /* 0x74 */ s32 cards[6];  /* the page's cards */
    /* 0x8C */ s32 arrows_shown; /* page arrows blink */
    /* 0x90 */ s32 blink_time; /* time of the last blink */
    /* 0x94 */ s32 question_cursor; /* yes/no cursor */
    /* 0x98 */ s32 chosen; /* the chosen card */
    /* 0x9C */ s32 price;  /* its price */
    /* 0xA0 */ s32 shop;   /* shop */
    /* 0xA4 */ s32 card_count; /* cards */
    /* 0xA8 */ StcrdshpShop *stock;
    /* 0xAC */ WindowAnim anims[3];
} StcrdshpBuy; /* size 0xDC */

/* The buy screen's children (its data block). */
typedef struct StcrdshpBuyData {
    /* 0x00 */ MessageWindow *card_name; /* card name */
    /* 0x04 */ MessageWindow *level_label;
    /* 0x08 */ MessageWindow *level;
    /* 0x0C */ MessageWindow *owned_label;
    /* 0x10 */ MessageWindow *owned;  /* copies owned */
    /* 0x14 */ MessageWindow *card_text;
    /* 0x18 */ MessageWindow *ap_label;
    /* 0x1C */ MessageWindow *ap;
    /* 0x20 */ MessageWindow *hp_label;
    /* 0x24 */ MessageWindow *hp;
    /* 0x28 */ MessageWindow *price_label;
    /* 0x2C */ MessageWindow *price;  /* price */
    /* 0x30 */ MessageWindow *l1_window;
    /* 0x34 */ MessageWindow *r1_window;
    /* 0x38 */ MessageWindow *message;
    /* 0x3C */ MessageWindow *yes;
    /* 0x40 */ MessageWindow *no;
    /* 0x44 */ MessageCursor *question_cursor; /* yes/no cursor */
    /* 0x48 */ StcrdshpPack *pack;   /* the card pack */
} StcrdshpBuyData; /* size 0x4C */

void stcrdshp_update_fade(Fade *obj);
void stcrdshp_update_pack(StcrdshpPack *obj);
void stcrdshp_run_buy(StcrdshpBuy *obj, StcrdshpBuyData *data);
void stcrdshp_draw_buy(StcrdshpBuy *obj);
void stcrdshp_update_buy(StcrdshpBuy *obj, StcrdshpBuyData *data);
StcrdshpBuy *stcrdshp_create_buy(StcrdshpMain *main, s32 shop);
Object *stcrdshp_create_booster(StcrdshpMain *main);


/* Starts a fade in (out == 0) or out over `frames` frames. */
void stcrdshp_start_fade(Fade *obj, s32 out, s32 frames) {
    obj->base.set_state(obj, OBJECT_STATE_RUN);
    obj->base.step = 1;
    obj->from_black = out;
    if (out == 0) {
        obj->level = 0;
        obj->step = 0xFF00 / frames;
    } else {
        obj->level = 0xFF00;
        obj->step = -(0xFF00 / frames);
    }
}

/* Draws a full-screen (320x256) quad with subtractive blending (abr 2), each channel level >> 8. */
void stcrdshp_draw_fade(Fade *obj) {
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
    poly->x0 = poly->x2 = 0;
    poly->x1 = poly->x3 = 320;
    poly->y0 = poly->y1 = 0;
    poly->y2 = poly->y3 = 256;
    addPrim(ot, poly);
    tpage = (DR_TPAGE *)(poly + 1);
    setDrawTPage(tpage, 0, 1, getTPage(0, 2, 320, 0));
    addPrim(ot, tpage);
    gfx_module.funcs.set_packet(tpage + 1);
}

void stcrdshp_update_fade(Fade *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->base.step == 0) {
            break;
        }
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
        /* fallthrough */
    case OBJECT_STATE_DONE:
        stcrdshp_draw_fade(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Fade *stcrdshp_create_fade(void) {
    Fade *obj = object_new(stcrdshp_update_fade, sizeof(Fade), 0);

    obj->start = stcrdshp_start_fade;
    obj->layer_id = 0x1000;
    obj->ot_depth = 0;
    return obj;
}

/* Uploads the six cards' pictures (up to the first invalid one). */
void stcrdshp_load_pack(StcrdshpPack *obj) {
    CardPicture card;
    s32 *cards;
    s32 i;

    card_init(&card);
    card.set_image_pos(0x140, 0x100);
    card.set_clut_pos(0x300, 0x100);
    cards = obj->cards;
    for (i = 0; i < 6; i++) {
        if (cards[i] < 1 || cards[i] > 314) {
            break;
        }
        card.select(cards[i]);
        card.set_cell(0, i);
        card.load_image();
    }
}

void stcrdshp_set_pack(StcrdshpPack *obj, s32 *cards) {
    s32 i;

    for (i = 0; i < 6; i++) {
        obj->prev_cards[i] = obj->cards[i];
        obj->cards[i] = cards[i];
    }
    obj->backs_shown = 0;
    obj->backs_frame = 0;
    obj->base.set_state(obj, OBJECT_STATE_DONE);
}

void stcrdshp_open_pack(StcrdshpPack *obj) {
    obj->base.set_step(obj, 1);
}

/* Draws the first cards_shown cards (the previous ones if `old`): picture, type, and the two values. */
void stcrdshp_draw_pack(StcrdshpPack *obj, s32 old) {
    Sprite spr;
    CardPicture card;
    s32 digits[5];
    s32 dx;
    s32 *cards;
    s32 i;
    s32 j;
    s32 x;
    s32 y;
    s32 value;
    s32 col;
    s32 row;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    spr.set_vram_pos(0x280, 0);
    card_init(&card);
    card.set_image_pos(0x140, 0x100);
    card.set_clut_pos(0x300, 0x100);
    card.set_layer(obj->layer_id, obj->ot_depth);
    if (old) {
        cards = obj->prev_cards;
    } else {
        cards = obj->cards;
    }
    for (i = 0; i < obj->cards_shown; i++, cards++) {
        if (*cards < 1 || *cards > 314) {
            break;
        }
        col = i % 6;
        row = i / 6;
        x = col * 0x2A;
        y = row * 0x36;
        card.select(*cards);
        card.set_cell(0, i);
        card.draw(x + 0x27, y + 0x46);
        if (card.get_class() != 0) {
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x1D, x + 0x27, y + 0x65);
        } else {
            value = card.record[1];
            j = value / 10;
            if (j != 0) {
                digits[0] = j + 0x1E;
            } else {
                digits[0] = 0;
            }
            j = value % 10;
            digits[1] = j + 0x1E;
            digits[2] = 0x1C;
            for (j = 0, dx = 0; j < 3; j++, dx += 7) {
                if (digits[j] != 0) {
                    spr.draw(cdload_module.get_subfile_by_id(0x063E0000), digits[j], dx + 0x27 + x, y + 0x65);
                }
            }
            value = card.record[2];
            j = value / 10;
            if (j != 0) {
                digits[0] = j + 0x1E;
            } else {
                digits[0] = 0;
            }
            j = value % 10;
            digits[1] = j + 0x1E;
            for (j = 0, dx = 0; j < 2; j++, dx += 7) {
                if (digits[j] != 0) {
                    spr.draw(cdload_module.get_subfile_by_id(0x063E0000), digits[j], dx + 0x3A + x, y + 0x65);
                }
            }
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), card.record[0] - 1, x + 0x23, y + 0x44);
    }
}

/* Draws the backs_shown card backs. */
void stcrdshp_draw_pack_backs(StcrdshpPack *obj) {
    Sprite spr;
    s32 i;
    s32 col;
    s32 row;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
    spr.set_vram_pos(0x280, 0);
    for (i = 0; i < obj->backs_shown; i++) {
        col = i % 6;
        row = i / 6;
        spr.set_palette(obj->backs_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 6, col * 0x2A + 0x23, row * 0x36 + 0x44);
    }
}

/* Turns over the next card every other frame. */
void stcrdshp_step_pack(StcrdshpPack *obj) {
    switch (obj->base.step) {
    case 0:
        break;
    case 1:
        if (obj->cards_shown != 0) {
            obj->cards_shown--;
            obj->base.next_step(obj);
            obj->base.timer = gfx_module.funcs.get_time();
        } else {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 2:
        if (gfx_module.funcs.get_time() - obj->base.timer >= 2) {
            obj->base.step = 1;
        }
        break;
    }
}

void stcrdshp_update_pack(StcrdshpPack *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stcrdshp_set_pack(obj, obj->cards);
        break;
    case OBJECT_STATE_RUN:
        stcrdshp_step_pack(obj);
        stcrdshp_draw_pack(obj, 0);
        break;
    case OBJECT_STATE_DONE:
        switch (obj->base.step) {
        case 0:
        default:
            obj->backs_shown++;
            if (obj->backs_shown < 6) {
                obj->base.next_step(obj);
                obj->base.timer = gfx_module.funcs.get_time();
            } else {
                obj->backs_shown = 6;
                obj->base.step = 2;
            }
            sound_module.play(0x800460BD);
            break;
        case 1:
            if (gfx_module.funcs.get_time() - obj->base.timer >= 2) {
                obj->base.step = obj->base.substep;
            }
            break;
        case 2:
            stcrdshp_load_pack(obj);
            obj->cards_shown = 6;
            obj->frame_time = gfx_module.funcs.get_time();
            obj->base.next_step(obj);
            sound_module.play(0x4001C);
            break;
        case 3:
            if (gfx_module.funcs.get_time() - obj->frame_time >= 2) {
                obj->frame_time = gfx_module.funcs.get_time();
                obj->backs_frame++;
                if (obj->backs_frame >= 11) {
                    obj->base.state = OBJECT_STATE_RUN;
                }
            }
            break;
        }
        stcrdshp_draw_pack_backs(obj);
        if (obj->base.step < 3) {
            stcrdshp_draw_pack(obj, 1);
        } else {
            stcrdshp_draw_pack(obj, 0);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

StcrdshpPack *stcrdshp_create_pack(StcrdshpMain *main, s32 *cards) {
    StcrdshpPack *obj = object_new(stcrdshp_update_pack, sizeof(StcrdshpPack), 0);
    s32 i;

    obj->set = stcrdshp_set_pack;
    obj->open = stcrdshp_open_pack;
    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->main = main;
    for (i = 0; i < 6; i++, cards++) {
        obj->cards[i] = *cards;
    }
    return obj;
}


void stcrdshp_create_buy_windows(StcrdshpBuy *obj, StcrdshpBuyData *data) {
    data->card_name = message_create_window(obj->layer_id, 1, 0x88, 0x80);
    data->level_label = message_create_window(obj->layer_id, 1, 0x115, 0x80);
    data->level = message_create_window(obj->layer_id, 1, 0x126, 0x80);
    data->owned_label = message_create_window(obj->layer_id, 1, 0x115, 0xA6);
    data->owned = message_create_window(obj->layer_id, 1, 0x12C, 0xA6);
    data->card_text = message_create_window(obj->layer_id, 1, 0x50, 0x97);
    data->ap_label = message_create_window(obj->layer_id, 1, 0xCE, 0x97);
    data->ap = message_create_window(obj->layer_id, 1, 0xF0, 0x97);
    data->hp_label = message_create_window(obj->layer_id, 1, 0xCE, 0xA4);
    data->hp = message_create_window(obj->layer_id, 1, 0xF0, 0xA4);
    data->price = message_create_window(obj->layer_id, 3, 0x117, 0xC7);
    data->price_label = message_create_window(obj->layer_id, 3, 0x11A, 0xC7);
    data->l1_window = message_create_window(obj->layer_id, 1, 0x12, 0x67);
    data->r1_window = message_create_window(obj->layer_id, 1, 0x121, 0x67);
    data->r1_window->set_ot_depth(data->r1_window, obj->ot_depth);
    data->message = message_create_window(obj->layer_id, 1, 0x9A, 0x39);
    data->yes = message_create_window(obj->layer_id, 1, 0xC5, 0x56);
    data->no = message_create_window(obj->layer_id, 1, 0xC5, 0x66);
    data->question_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 3, 0xB8, 0x56);
    data->question_cursor->show(data->question_cursor, 0);
}

/* Shows (or hides) the card under the cursor: name, price, copies owned, then as STCRDDEK's card info. */
void stcrdshp_show_buy_card(StcrdshpBuy *obj, StcrdshpBuyData *data, s32 show) {
    CardPicture card;
    s32 id;

    id = obj->stock->cards[obj->page * 6 + obj->cursor_column];
    if (show) {
        card_init(&card);
        card.select(id);
        data->card_name->set_text(data->card_name, cdload_module.files.get_file(records_language + 0x16), id);
        data->price_label->set_text(data->price_label, cdload_module.files.get_file(records_language + 0x32), 3);
        data->price->set_line_number(data->price, 0, stcrdshp_stock.get_price(id));
        data->price->measure(data->price, 1);
        data->owned_label->set_text(data->owned_label, cdload_module.files.get_file(records_language + 0x32), 8);
        data->owned->set_line_number(data->owned, 0, gamestate_data.cards[id]);
        data->owned->measure(data->owned, 1);
        if (card.get_class() != 0) {
            data->level_label->set_visible(data->level_label, 0);
            data->level->set_visible(data->level, 0);
            data->card_text->set_text(data->card_text, cdload_module.files.get_file(records_language + 0x1D), id);
            data->ap_label->set_visible(data->ap_label, 0);
            data->ap->set_visible(data->ap, 0);
            data->hp_label->set_visible(data->hp_label, 0);
            data->hp->set_visible(data->hp, 0);
        } else {
            data->level_label->set_text(data->level_label, cdload_module.files.get_file(records_language + 0x32), 8);
            data->level->set_line_number(data->level, 0, card.record[5]);
            data->level->measure(data->level, 1);
            if (id == 0x45 || id == 0x70 || id == 0x9B || id == 0xC6 || id == 0xF1) {
                data->card_text->set_text(data->card_text, cdload_module.files.get_file(records_language + 0x1D), id);
                data->ap_label->set_visible(data->ap_label, 0);
                data->ap->set_visible(data->ap, 0);
                data->hp_label->set_visible(data->hp_label, 0);
                data->hp->set_visible(data->hp, 0);
            } else {
                data->card_text->set_visible(data->card_text, 0);
                data->ap_label->set_text(data->ap_label, cdload_module.files.get_file(records_language + 0x32), 0x11);
                data->ap->set_line_number(data->ap, 0, card.record[1]);
                data->ap->measure(data->ap, 1);
                data->hp_label->set_text(data->hp_label, cdload_module.files.get_file(records_language + 0x32), 0x12);
                data->hp->set_line_number(data->hp, 0, card.record[2]);
                data->hp->measure(data->hp, 1);
            }
        }
    } else {
        data->card_name->set_visible(data->card_name, 0);
        data->price_label->set_visible(data->price_label, 0);
        data->price->set_visible(data->price, 0);
        data->owned_label->set_visible(data->owned_label, 0);
        data->owned->set_visible(data->owned, 0);
        data->level_label->set_visible(data->level_label, 0);
        data->level->set_visible(data->level, 0);
        data->card_text->set_visible(data->card_text, 0);
        data->ap_label->set_visible(data->ap_label, 0);
        data->ap->set_visible(data->ap, 0);
        data->hp_label->set_visible(data->hp_label, 0);
        data->hp->set_visible(data->hp, 0);
    }
}


extern s32 stcrdshp_buy_cursor_palettes[]; /* the cursor's animation frames */

void stcrdshp_draw_buy(StcrdshpBuy *obj) {
    Sprite spr;
    CardPicture card;
    s32 id;
    s32 special;
    s32 icon;

    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    id = obj->stock->cards[obj->page * 6 + obj->cursor_column];
    if (obj->anims[0].level != 0) {
        card_init(&card);
        card.select(id);
        if (obj->anims[0].level != 0x1000) {
            spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x87);
        }
        special = card.get_class();
        if (special == 1) {
            icon = 0x12;
        } else if (special == 2) {
            icon = 0x13;
        } else {
            icon = card.record[0] + 0x13;
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), icon, 0x103, 0x7E);
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xD, 0xFC, 0x7C);
        if (obj->anims[0].level != 0x1000) {
            spr.set_pivot(0x140, 0x87);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xA, 0x82, 0x7C);
        if (obj->anims[0].level != 0x1000) {
            spr.set_pivot(0x140, 0xAF);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x10, 0x103, 0xA4);
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xD, 0xFC, 0xA2);
        if (obj->anims[0].level != 0x1000) {
            spr.set_pivot(0x140, 0xA5);
        }
        if (special != 0) {
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xB, 0x4A, 0x92);
        } else if (id == 0x45 || id == 0x70 || id == 0x9B || id == 0xC6 || id == 0xF1) {
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xB, 0x4A, 0x92);
        } else {
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xC, 0xC7, 0x92);
        }
        if (obj->anims[1].level != 0x1000) {
            spr.set_pivot(0x140, 0xC8);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x29, 0xD6, 0xBF);
    }
    if (obj->cursor_shown != 0) {
        if (gfx_module.funcs.get_time() - obj->frame_time >= 4) {
            obj->frame_time = gfx_module.funcs.get_time();
            obj->cursor_frame++;
            if (obj->cursor_frame >= 6) {
                obj->cursor_frame = 0;
            }
        }
        spr.set_palette(stcrdshp_buy_cursor_palettes[obj->cursor_frame]);
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 7, obj->cursor_column * 0x2A + 0x24, 0x44);
        spr.set_palette(0);
        if (obj->pages >= 2) {
            if (gfx_module.funcs.get_time() - obj->blink_time >= 17) {
                obj->blink_time = gfx_module.funcs.get_time();
                obj->arrows_shown = 1 - obj->arrows_shown;
            }
            if (obj->anims[0].level == 0x1000 && obj->arrows_shown != 0) {
                if (obj->page > 0) {
                    spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x1A, 0xE, 0x55);
                }
                if (obj->page < obj->pages - 1) {
                    spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x1B, 0x121, 0x55);
                }
            }
        }
    }
    if (obj->anims[1].level != 0) {
        sprite_init(&spr);
        spr.set_vram_pos(0x280, 0);
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 2);
        if (obj->anims[1].level != 0x1000) {
            spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x3F);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x2C, 0x7B, 0x33);
        if (obj->anims[2].level != 0) {
            if (obj->anims[2].level != 0x1000) {
                spr.set_scale(obj->anims[2].level, 0x1000, 0x1000);
                spr.set_pivot(0x140, 0x64);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x2D, 0xAF, 0x50);
        }
    }
}


/* The buy screen's steps (base.step), while the card pack is idle: choose a card, confirm, buy. */
void stcrdshp_run_buy(StcrdshpBuy *obj, StcrdshpBuyData *data) {
    CardPicture card;
    s32 old;
    s32 i;

    if (data->pack->base.state == OBJECT_STATE_RUN) {
        switch (obj->base.step) {
        case 0:
        default:
            stcrdshp_util.window_anim_start(&obj->anims[0], 1);
            obj->base.step++;
            break;
        case 1:
            if (stcrdshp_util.window_anim_update(&obj->anims[0])) {
                obj->cursor_shown = 1;
                stcrdshp_show_buy_card(obj, data, 1);
                if (obj->pages >= 2) {
                    if (obj->page < obj->pages - 1) {
                        data->r1_window->set_text(data->r1_window, cdload_module.files.get_file(records_language + 0x32), 0xB);
                    } else {
                        data->r1_window->set_visible(data->r1_window, 0);
                    }
                }
                obj->base.step++;
            }
            break;
        case 2:
            old = obj->cursor_column;
            if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
                if (--obj->cursor_column < 0) {
                    obj->cursor_column = 0;
                }
            } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
                if (++obj->cursor_column > 5) {
                    obj->cursor_column = 5;
                }
            }
            if (old != obj->cursor_column) {
                if (obj->stock->cards[obj->page * 6 + obj->cursor_column] != 0) {
                    sound_module.play(0x4001B);
                    stcrdshp_show_buy_card(obj, data, 1);
                } else {
                    obj->cursor_column = old;
                }
            }
            old = obj->page;
            if (!PAD_HELD(0xB) && PAD_PRESSED(0xA)) {
                if (--obj->page < 0) {
                    obj->page = 0;
                }
            } else if (!PAD_HELD(0xA) && PAD_PRESSED(0xB)) {
                if (++obj->page > obj->pages - 1) {
                    obj->page = obj->pages - 1;
                }
            }
            if (old != obj->page) {
                sound_module.play(0x4001B);
                obj->cursor_shown = 0;
                obj->cursor_column = 0;
                for (i = 0; i < 6; i++) {
                    obj->cards[i] = obj->stock->cards[obj->page * 6 + i];
                }
                data->pack->set(data->pack, obj->cards);
                if (obj->pages >= 2) {
                    if (obj->page > 0) {
                        data->l1_window->set_text(data->l1_window, cdload_module.files.get_file(records_language + 0x32), 0xA);
                    } else {
                        data->l1_window->set_visible(data->l1_window, 0);
                    }
                    if (obj->page < obj->pages - 1) {
                        data->r1_window->set_text(data->r1_window, cdload_module.files.get_file(records_language + 0x32), 0xB);
                    } else {
                        data->r1_window->set_visible(data->r1_window, 0);
                    }
                }
                obj->base.step = 1;
            }
            if (PAD_PRESSED(0xD)) {
                card_init(&card);
                obj->chosen = obj->stock->cards[obj->page * 6 + obj->cursor_column];
                card.select(obj->chosen);
                obj->price = stcrdshp_stock.get_price(obj->chosen);
                sound_module.play(0x4001C);
                if (gamestate_data.money < obj->price) {
                    obj->base.step = 10;
                    obj->base.substep = 0x13; /* not enough money */
                } else if (gamestate_data.cards[obj->chosen] == 9) {
                    obj->base.step = 10;
                    obj->base.substep = 0x14; /* nine copies already */
                } else {
                    obj->base.step = 5;
                }
            } else if (PAD_PRESSED(0xE)) {
                sound_module.play(0x800450BD);
                obj->base.step = 50;
            }
            break;
        case 5:
            stcrdshp_util.window_anim_start(&obj->anims[1], 1);
            stcrdshp_util.window_anim_start(&obj->anims[2], 1);
            obj->base.step++;
            break;
        case 6:
            stcrdshp_util.window_anim_update(&obj->anims[2]);
            if (stcrdshp_util.window_anim_update(&obj->anims[1])) {
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x32), 0xC);
                data->message->set_line_number(data->message, 1, obj->price);
                data->yes->set_text(data->yes, cdload_module.files.get_file(records_language + 0x32), 0xD);
                data->no->set_text(data->no, cdload_module.files.get_file(records_language + 0x32), 0xE);
                data->question_cursor->set_pos(data->question_cursor, 0xB8, obj->question_cursor * 16 + 0x56);
                data->question_cursor->show(data->question_cursor, 1);
                obj->base.step++;
            }
            break;
        case 7:
            old = obj->question_cursor;
            if (PAD_PRESSED(4)) {
                obj->question_cursor = 0;
            } else if (PAD_PRESSED(6)) {
                obj->question_cursor = 1;
            }
            if (old != obj->question_cursor) {
                sound_module.play(0x8004513E);
                data->question_cursor->set_pos(data->question_cursor, 0xB8, obj->question_cursor * 16 + 0x56);
            }
            if (PAD_PRESSED(0xD)) {
                sound_module.play(0x8004503C);
                if (obj->question_cursor == 0) {
                    gamestate_data.funcs.add_card(obj->chosen, 1);
                    gamestate_data.money -= obj->price;
                    obj->main->update_money(obj->main);
                }
                obj->base.step++;
            } else if (PAD_PRESSED(0xE)) {
                sound_module.play(0x800450BD);
                obj->base.step++;
            }
            break;
        case 8:
            data->message->set_visible(data->message, 0);
            data->yes->set_visible(data->yes, 0);
            data->no->set_visible(data->no, 0);
            data->question_cursor->show(data->question_cursor, 0);
            obj->question_cursor = 0;
            stcrdshp_util.window_anim_start(&obj->anims[1], 0);
            obj->base.step++;
            break;
        case 10:
            stcrdshp_util.window_anim_start(&obj->anims[1], 1);
            obj->anims[2].level = 0;
            obj->base.step++;
            break;
        case 11:
            if (stcrdshp_util.window_anim_update(&obj->anims[1])) {
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x32), obj->base.substep);
                obj->base.step++;
            }
            break;
        case 12:
            if (PAD_PRESSED(0xD) || PAD_PRESSED(0xE)) {
                sound_module.play(0x4001C);
                data->message->set_visible(data->message, 0);
                stcrdshp_util.window_anim_start(&obj->anims[1], 0);
                obj->base.step++;
            }
            break;
        case 9:
        case 13:
            if (stcrdshp_util.window_anim_update(&obj->anims[1])) {
                obj->base.step = 1;
            }
            break;
        case 50:
            obj->cursor_shown = 0;
            data->l1_window->set_visible(data->l1_window, 0);
            data->r1_window->set_visible(data->r1_window, 0);
            stcrdshp_show_buy_card(obj, data, 0);
            stcrdshp_util.window_anim_start(&obj->anims[0], 0);
            obj->base.step++;
            break;
        case 51:
            if (stcrdshp_util.window_anim_update(&obj->anims[0])) {
                data->pack->open(data->pack);
                obj->base.state = OBJECT_STATE_DONE;
            }
            break;
        }
    }
}

void stcrdshp_update_buy(StcrdshpBuy *obj, StcrdshpBuyData *data) {
    s32 i;
    s32 count;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->stock = stcrdshp_stock.find_shop(obj->shop);
        for (i = 0; i < 6; i++) {
            obj->cards[i] = obj->stock->cards[i];
        }
        obj->card_count = obj->stock->count;
        /* Pages of six cards, the last one partly filled. */
        count = obj->card_count % 6 != 0;
        /* FAKE: count holds the partial page and then the sum (so local-alloc can't tie the sum to the quotient
         * copy, which leaves it the original's v0), and `- -count` keeps the original's add operand order
         * (quotient first); `obj->unk_A4 / 6 + count` is reordered (count first). */
        count = obj->card_count / 6 - -count;
        obj->pages = count;
        data->pack = stcrdshp_create_pack(obj->main, obj->cards);
        stcrdshp_create_buy_windows(obj, data);
        obj->anims[0].duration = 10;
        obj->anims[1].duration = 10;
        obj->anims[2].duration = 10;
        break;
    case OBJECT_STATE_RUN:
        stcrdshp_run_buy(obj, data);
        stcrdshp_draw_buy(obj);
        break;
    case OBJECT_STATE_DONE:
        if (data->pack == NULL) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

StcrdshpBuy *stcrdshp_create_buy(StcrdshpMain *main, s32 shop) {
    StcrdshpBuy *obj = object_new(stcrdshp_update_buy, sizeof(StcrdshpBuy), sizeof(StcrdshpBuyData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->main = main;
    obj->shop = shop;
    return obj;
}


/* The overlay's root object: sets up the display and creates the main object. */
void stcrdshp_update_root(Object *obj, StcrdshpMain **data) {
    RECT rect;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0xF000);
        gfx_module.funcs.init_display(0x140, 0xF0, 0, 0);
        rect.x = 0;
        rect.y = 0;
        rect.w = 0x140;
        rect.h = 0xF0;
        layer = gfx_module.funcs.create_layer(&rect, 3, 0x1000);
        layer->set_bg_color(layer, 0, 0, 0);
        *data = stcrdshp_create_main();
        obj->next_state(obj);
        break;
    }
}

/* The overlay's entry. */
Object *stcrdshp_start(void) {
    return object_new(stcrdshp_update_root, sizeof(Object), 4);
}

/* Creates the main object's windows; all of its children go to ordering table entry ot_depth - 2. */
void stcrdshp_create_main_windows(StcrdshpMain *obj, StcrdshpMainData *data) {
    s32 i;
    s32 j;
    MessageWindow **windows;

    data->shopkeeper = message_create_window(obj->layer_id, 1, 0x1D, 0x16);
    data->help_line = message_create_window(obj->layer_id, 1, 0xD3, 0xCC);
    data->money = message_create_window(obj->layer_id, 3, 0x117, 0x1C);
    data->money_label = message_create_window(obj->layer_id, 3, 0x11A, 0x1C);
    for (i = 0; i < 3; i++) {
        data->menu[i] = message_create_window(obj->layer_id, 1, 0xA7, i * 14 + 0x35);
    }
    data->menu_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 2, 0xA7, obj->menu_cursor * 14 + 0x35);
    data->menu_cursor->show(data->menu_cursor, 0);
    data->no_boosters = message_create_window(obj->layer_id, 1, 0x9A, 0x71);
    windows = (MessageWindow **)obj->base.children;
    for (j = 0; j < obj->base.child_count - 3; j++, windows++) {
        (*windows)->set_ot_depth(*windows, obj->ot_depth - 2);
    }
}

/* Shows (or hides) the shopkeeper's line, "money" and the player's money. */
void stcrdshp_show_money(StcrdshpMain *obj, StcrdshpMainData *data, s32 show) {
    if (show) {
        data->shopkeeper->set_text(data->shopkeeper, cdload_module.files.get_file(records_language + 0x94), obj->greeting);
        data->money_label->set_text(data->money_label, cdload_module.files.get_file(records_language + 0x32), 3);
        data->money->set_line_number(data->money, 0, gamestate_data.money);
        data->money->measure(data->money, 1);
    } else {
        data->shopkeeper->set_visible(data->shopkeeper, 0);
        data->money_label->set_visible(data->money_label, 0);
        data->money->set_visible(data->money, 0);
    }
}

/* Shows (or hides) the menu: buy, open boosters, leave. */
void stcrdshp_show_menu(StcrdshpMain *obj, StcrdshpMainData *data, s32 show) {
    s32 i;

    if (show) {
        for (i = 0; i < 3; i++) {
            data->menu[i]->set_text(data->menu[i], cdload_module.files.get_file(records_language + 0x32), i + 5);
        }
        data->help_line->set_text(data->help_line, cdload_module.files.get_file(records_language + 0x32), 4);
    } else {
        for (i = 0; i < 3; i++) {
            data->menu[i]->set_visible(data->menu[i], 0);
        }
        data->help_line->set_visible(data->help_line, 0);
    }
}

void stcrdshp_draw_main(StcrdshpMain *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, 7);
    spr.set_vram_pos(0x280, 0);
    if (obj->odd_frame != 0) {
        obj->scroll++;
        obj->scroll = obj->scroll < 0x60 ? obj->scroll : 0;
        obj->odd_frame = 0;
    } else {
        obj->odd_frame = 1;
    }
    spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 8, obj->scroll, obj->scroll);
    spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
    if (obj->anims[0].level != 0) {
        if (obj->anims[0].level != 0x1000) {
            spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
            spr.set_pivot(0x57, 0x1B);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x28, 0x16, 0x14);
        if (obj->anims[0].level != 0x1000) {
            spr.set_pivot(0x140, 0x1D);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x29, 0xD6, 0x15);
    }
    if (obj->anims[1].level != 0) {
        if (obj->anims[1].level != 0x1000) {
            spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x49);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x2F, 0x92, 0x2E);
        if (obj->anims[1].level != 0x1000) {
            spr.set_pivot(0x140, 0xD2);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xF, 0xC6, 0xC4);
    }
    if (obj->anims[2].level != 0) {
        if (obj->anims[2].level != 0x1000) {
            spr.set_scale(obj->anims[2].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x77);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x2C, 0x7B, 0x6B);
    }
}

void stcrdshp_update_menu(StcrdshpMain *obj, StcrdshpMainData *data) {
    s32 old;
    s32 count;
    s32 i;
    Fade *fade;

    switch (obj->base.step) {
    case 0:
    default:
        stcrdshp_util.window_anim_start(&obj->anims[0], 1);
        obj->base.step++;
        break;
    case 1:
        if (stcrdshp_util.window_anim_update(&obj->anims[0])) {
            stcrdshp_show_money(obj, data, 1);
            stcrdshp_util.window_anim_start(&obj->anims[1], 1);
            obj->base.step++;
        }
        break;
    case 2:
        if (stcrdshp_util.window_anim_update(&obj->anims[1])) {
            stcrdshp_show_menu(obj, data, 1);
            data->menu_cursor->set_pos(data->menu_cursor, 0x9A, obj->menu_cursor * 14 + 0x35);
            data->menu_cursor->show(data->menu_cursor, 1);
            obj->base.step++;
        }
        break;
    case 3:
        old = obj->menu_cursor;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            obj->menu_cursor--;
            if (obj->menu_cursor < 0) {
                obj->menu_cursor = 0;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            obj->menu_cursor++;
            if (obj->menu_cursor >= 3) {
                obj->menu_cursor = 2;
            }
        }
        if (old != obj->menu_cursor) {
            sound_module.play(0x8004513E);
            data->menu_cursor->set_pos(data->menu_cursor, 0x9A, obj->menu_cursor * 14 + 0x35);
        }
        if (PAD_PRESSED(13)) {
            sound_module.play(0x8004503C);
            obj->base.step = 10;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.set_step(obj, 50);
        }
        break;
    case 10:
        obj->base.substep = 1;
        switch (obj->menu_cursor) {
        case 0:
        default:
            data->screen = (Object *)stcrdshp_create_buy(obj, obj->shop);
            obj->base.step = 50;
            break;
        case 1:
            count = records_funcs.list_items(1, (u16 *)obj->items);
            for (i = 0; i < count; i++) {
                if (records_funcs.get_item_icon(obj->items[i]) == 0x62) {
                    data->screen = (Object *)stcrdshp_create_booster(obj);
                    obj->base.step = 50;
                    break;
                }
            }
            if (data->screen == NULL) {
                obj->base.set_step(obj, 20);
            }
            break;
        case 2:
            obj->base.step = 100;
            obj->left = 1;
            break;
        }
        break;
    case 11:
        if (data->screen == NULL) {
            obj->base.step = 1;
        }
        break;
    case 20:
        data->menu_cursor->stop(data->menu_cursor, 1);
        data->menu_cursor->set_palette(data->menu_cursor, 7);
        stcrdshp_util.window_anim_start(&obj->anims[2], 1);
        obj->base.step++;
        break;
    case 21:
        if (stcrdshp_util.window_anim_update(&obj->anims[2])) {
            data->no_boosters->set_text(data->no_boosters, cdload_module.files.get_file(records_language + 0x32), 0x15);
            obj->base.step++;
        }
        break;
    case 22:
        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            data->no_boosters->set_visible(data->no_boosters, 0);
            stcrdshp_util.window_anim_start(&obj->anims[2], 0);
            obj->base.step++;
        }
        break;
    case 23:
        if (stcrdshp_util.window_anim_update(&obj->anims[2])) {
            data->menu_cursor->stop(data->menu_cursor, 0);
            data->menu_cursor->set_palette(data->menu_cursor, 0);
            obj->base.step = 3;
        }
        break;
    case 50:
        if (obj->base.substep == 0) {
            fade = stcrdshp_create_fade();
            data->fade = (Object *)fade;
            fade->start(fade, 0, 30);
            ((Fade *)data->fade)->ot_depth = 6;
        }
        stcrdshp_show_menu(obj, data, 0);
        data->menu_cursor->show(data->menu_cursor, 0);
        stcrdshp_util.window_anim_start(&obj->anims[1], 0);
        obj->base.step++;
        break;
    case 51:
        if (stcrdshp_util.window_anim_update(&obj->anims[1])) {
            if (obj->base.substep == 0) {
                stcrdshp_show_money(obj, data, 0);
                stcrdshp_util.window_anim_start(&obj->anims[0], 0);
                obj->base.step++;
            } else {
                obj->base.step = 11;
            }
        }
        break;
    case 52:
        if (stcrdshp_util.window_anim_update(&obj->anims[0])) {
            obj->base.step = 101;
        }
        break;
    case 100:
        fade = stcrdshp_create_fade();
        data->fade = (Object *)fade;
        fade->start(fade, 0, 10);
        obj->base.step++;
        break;
    case 101:
        if (data->fade->state == OBJECT_STATE_DONE) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}

/* Shows the player's money again (update_money, after a purchase). */
void stcrdshp_update_money(StcrdshpMain *obj) {
    StcrdshpMainData *data = (StcrdshpMainData *)obj->base.children;

    data->money->set_line_number(data->money, 0, gamestate_data.money);
    data->money->measure(data->money, 1);
}

void stcrdshp_update_main(StcrdshpMain *obj, StcrdshpMainData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            stcrdshp_util.load_files();
            obj->base.step++;
            break;
        case 1:
            if (stcrdshp_util.is_loading() == 0) {
                stcrdshp_create_main_windows(obj, data);
                obj->anims[0].duration = 10;
                obj->anims[1].duration = 10;
                obj->anims[2].duration = 10;
                obj->base.next_state(obj);
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        stcrdshp_update_menu(obj, data);
        stcrdshp_draw_main(obj);
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        if (obj->left != 0) {
            gamestate_data.funcs.set_next_map(0x400, gamestate_data.funcs.get_map_entry());
        } else {
            gamestate_data.funcs.set_next_map(gamestate_data.field_map, 0);
        }
        break;
    }
}

StcrdshpMain *stcrdshp_create_main(void) {
    StcrdshpMain *obj = object_new(stcrdshp_update_main, sizeof(StcrdshpMain), sizeof(StcrdshpMainData));
    s32 i;
    s32 place;

    obj->update_money = stcrdshp_update_money;
    obj->layer_id = 0x1000;
    obj->ot_depth = 7;
    obj->shop = gamestate_data.funcs.get_map_entry();
    place = gamestate_data.field_map;
    for (i = 0; stcrdshp_greetings[i].map != 0; i++) {
        if (stcrdshp_greetings[i].map == place) {
            obj->greeting = stcrdshp_greetings[i].message;
        }
    }
    if (obj->greeting == 0) {
        obj->greeting = 0x1F;
    }
    if (gamestate_data.funcs.get_prev_map(place) == 0x400) {
        obj->menu_cursor = 2;
    }
    cdload_module.queue_file(0x7F6);
    cdload_module.queue_file(0x7F7);
    cdload_module.queue_file(0x7F8);
    cdload_module.queue_file(0x7F9);
    cdload_module.queue_file(0x7FA);
    cdload_module.queue_file(records_language + 0x16);
    cdload_module.queue_file(records_language + 0x1D);
    return obj;
}

/* the cursor's animation frames */
s32 stcrdshp_buy_cursor_palettes[6] = { 0, 1, 2, 3, 2, 1 };

StcrdshpGreeting stcrdshp_greetings[11] = {
    { 529, 31 },
    { 640, 32 },
    { 559, 33 },
    { 669, 34 },
    { 574, 35 },
    { 683, 36 },
    { 606, 37 },
    { 711, 38 },
    { 623, 39 },
    { 726, 40 },
    { 0, 31 },
};
