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

/* STCRDSHP.PRO's first file: the booster screen (the player's booster packs, eight to a page; opening one
 * takes it, adds six random cards and shows them with the card pack object). */

/* The booster screen (stcrdshp_update_booster). */
typedef struct StcrdshpBooster {
    /* 0x000 */ Object base;
    /* 0x050 */ StcrdshpMain *main;
    /* 0x054 */ s32 layer_id; /* layer */
    /* 0x058 */ s32 ot_depth; /* ordering table entry */
    /* 0x05C */ s32 chosen; /* the chosen booster (index of boosters) */
    /* 0x060 */ s32 page;   /* page */
    /* 0x064 */ s32 pages;  /* pages */
    /* 0x068 */ s32 arrows_frame; /* page arrows' animation frame */
    /* 0x06C */ s32 arrows_time;
    /* 0x070 */ s32 cards_got[6]; /* the cards got from the booster */
    /* 0x088 */ s32 card_shown; /* the card shown */
    /* 0x08C */ s32 cursor_shown; /* card cursor shown */
    /* 0x090 */ s32 cursor_frame; /* card cursor animation frame */
    /* 0x094 */ s32 frame_time; /* time of the last frame */
    /* 0x098 */ s16 boosters[404]; /* the player's boosters (items of type 0x62) */
    /* 0x3C0 */ s32 booster_count; /* their number */
    /* 0x3C4 */ s16 items[404];   /* the player's items (records_funcs.list_items) */
    /* 0x6EC */ u8 unk_6EC[0x4];
    /* 0x6F0 */ WindowAnim anims[4];
} StcrdshpBooster; /* size 0x730 */

/* The booster screen's children (its data block). */
typedef struct StcrdshpBoosterData {
    /* 0x00 */ MessageWindow *cards[8];  /* the page's cards, in two columns */
    /* 0x20 */ MessageCursor *cursor; /* the cursor */
    /* 0x24 */ MessageWindow *booster_name;
    /* 0x28 */ MessageWindow *booster_times;
    /* 0x2C */ MessageWindow *booster_count;
    /* 0x30 */ MessageWindow *message;
    /* 0x34 */ MessageWindow *back_hint;
    /* 0x38 */ MessageWindow *l1_window;
    /* 0x3C */ MessageWindow *r1_window;
    /* 0x40 */ MessageWindow *page;
    /* 0x44 */ MessageWindow *slash;
    /* 0x48 */ MessageWindow *pages;
    /* 0x4C */ MessageWindow *card_name;
    /* 0x50 */ MessageWindow *level_label;
    /* 0x54 */ MessageWindow *level;
    /* 0x58 */ MessageWindow *owned_label;
    /* 0x5C */ MessageWindow *owned;
    /* 0x60 */ MessageWindow *card_text;
    /* 0x64 */ MessageWindow *ap_label;
    /* 0x68 */ MessageWindow *ap;
    /* 0x6C */ MessageWindow *hp_label;
    /* 0x70 */ MessageWindow *hp;
    /* 0x74 */ StcrdshpPack *opened_cards; /* the opened booster's cards */
} StcrdshpBoosterData; /* size 0x78 */

void stcrdshp_draw_booster(StcrdshpBooster *obj);
void stcrdshp_run_booster(StcrdshpBooster *obj, StcrdshpBoosterData *data);
void stcrdshp_update_booster(StcrdshpBooster *obj, StcrdshpBoosterData *data);


void stcrdshp_create_booster_windows(StcrdshpBooster *obj, StcrdshpBoosterData *data) {
    s32 i;

    for (i = 0; i < 8; i++) {
        data->cards[i] = message_create_window(obj->layer_id, 1, (i % 2) * 0x83 + 0x37, (i / 2) * 14 + 0x39);
    }
    data->cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0x1D, 0x39);
    data->cursor->show(data->cursor, 0);
    data->page = message_create_window(obj->layer_id, 1, 0x92, 0x75);
    data->slash = message_create_window(obj->layer_id, 1, 0x93, 0x75);
    data->pages = message_create_window(obj->layer_id, 1, 0xA6, 0x75);
    data->l1_window = message_create_window(obj->layer_id, 1, 0x2D, 0x71);
    data->r1_window = message_create_window(obj->layer_id, 1, 0x102, 0x71);
    data->booster_name = message_create_window(obj->layer_id, 1, 0x8F, 0x8A);
    data->booster_times = message_create_window(obj->layer_id, 1, 0x10B, 0x8A);
    data->booster_count = message_create_window(obj->layer_id, 1, 0x121, 0x8A);
    data->message = message_create_window(obj->layer_id, 1, 0x14, 0xC2);
    data->back_hint = message_create_window(obj->layer_id, 1, 0x14, 0xD0);
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
}


/* Shows (or hides) page `page` of the list, "page n / pages" and the page arrows. */
void stcrdshp_show_booster_page(StcrdshpBooster *obj, StcrdshpBoosterData *data, s32 show) {
    s32 i;
    s32 n;
    s32 id;

    if (show) {
        for (i = 0; i < 8; i++) {
            n = obj->page * 8 + i;
            id = obj->boosters[n];
            if (n < obj->booster_count && id != 0) {
                data->cards[i]->set_text(data->cards[i], cdload_module.files.get_file(records_language + 0x6A), id);
            } else {
                data->cards[i]->set_visible(data->cards[i], 0);
            }
        }
        data->page->set_line_number(data->page, 0, obj->page + 1);
        data->page->measure(data->page, 1);
        data->slash->set_text(data->slash, cdload_module.files.get_file(records_language + 0x32), 9);
        data->pages->set_line_number(data->pages, 0, obj->pages);
        data->pages->measure(data->pages, 1);
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
    } else {
        for (i = 0; i < 8; i++) {
            data->cards[i]->set_visible(data->cards[i], 0);
        }
        data->page->set_visible(data->page, 0);
        data->slash->set_visible(data->slash, 0);
        data->pages->set_visible(data->pages, 0);
        data->l1_window->set_visible(data->l1_window, 0);
        data->r1_window->set_visible(data->r1_window, 0);
    }
}

/* Shows (or hides) the chosen booster's name and copies, and the "open?" question. */
void stcrdshp_show_booster_card(StcrdshpBooster *obj, StcrdshpBoosterData *data, s32 show) {
    s32 id;

    if (show) {
        id = obj->boosters[obj->chosen];
        data->booster_name->set_text(data->booster_name, cdload_module.files.get_file(records_language + 0x6A), id);
        data->booster_times->set_text(data->booster_times, cdload_module.files.get_file(records_language + 0x32), 8);
        data->booster_count->set_line_number(data->booster_count, 0, gamestate_data.items[id]);
        data->booster_count->measure(data->booster_count, 1);
        data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x32), 0xF);
        data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0x32), 4);
    } else {
        data->booster_name->set_visible(data->booster_name, 0);
        data->booster_times->set_visible(data->booster_times, 0);
        data->booster_count->set_visible(data->booster_count, 0);
        data->message->set_visible(data->message, 0);
        data->back_hint->set_visible(data->back_hint, 0);
    }
}


/* Shows (or hides) card cards_got[card_shown], if the player has seen it (gamestate_data.cards_obtained), as STCRDDEK's card
 * info. */
void stcrdshp_show_booster_result(StcrdshpBooster *obj, StcrdshpBoosterData *data, s32 show) {
    CardPicture card;
    s32 id;

    id = obj->cards_got[obj->card_shown];
    if (gamestate_data.cards_obtained[id] != 0 && show) {
        card_init(&card);
        card.select(id);
        data->card_name->set_text(data->card_name, cdload_module.files.get_file(records_language + 0x16), id);
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


extern s32 stcrdshp_booster_cursor_palettes[]; /* the cursor's animation frames */

/* Draws the list (item icons from image 0x0286), the chosen booster and the cards got from it. */
void stcrdshp_draw_booster(StcrdshpBooster *obj) {
    Sprite spr;
    CardPicture card;
    s32 i;
    s32 n;
    s32 id;
    s32 item;
    s32 special;
    s32 icon;

    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->anims[0].level != 0) {
        if (obj->anims[0].level != 0x1000) {
            spr.set_scale(0x1000, obj->anims[0].level, 0x1000);
            spr.set_pivot(0xA0, 0x5C);
        } else {
            for (i = 0; i < 8; i++) {
                n = obj->page * 8 + i;
                item = obj->boosters[n];
                if (n >= obj->booster_count || item <= 0) {
                    break;
                }
                spr.set_vram_pos(0x140, 0);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_funcs.get_item_icon(item), (i % 2) * 0x83 + 0x28,
                           (i / 2) * 14 + 0x39);
                spr.set_vram_pos(0x280, 0);
                spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x31, (i % 2) * 0x83 + 0x28, (i / 2) * 14 + 0x39);
            }
            spr.set_vram_pos(0x280, 0);
            if (obj->pages >= 2) {
                if (gfx_module.funcs.get_time() - obj->arrows_time >= 11) {
                    obj->arrows_frame++;
                    if (obj->arrows_frame >= 4) {
                        obj->arrows_frame = 0;
                    }
                }
                spr.set_palette(obj->arrows_frame);
                if (obj->page > 0) {
                    spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x48, 0x1E, 0x74);
                }
                if (obj->page < obj->pages - 1) {
                    spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x47, 0xFD, 0x74);
                }
                spr.set_palette(0);
            }
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x2B, 0, 0x32);
    }
    if (obj->anims[1].level != 0) {
        if (obj->anims[1].level != 0x1000) {
            spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x8F);
        } else {
            spr.set_vram_pos(0x140, 0);
            item = obj->boosters[obj->chosen];
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), records_funcs.get_item_icon(item), 0x80, 0x8A);
        }
        spr.set_vram_pos(0x280, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x2E, 0x79, 0x83);
    }
    if (obj->anims[2].level != 0) {
        spr.set_scale(0x1000, obj->anims[2].level, 0x1000);
        if (obj->anims[2].level != 0x1000) {
            spr.set_pivot(0xA0, 0xCF);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x2A, 0, 0xBD);
    }
    if (obj->anims[3].level != 0) {
        id = obj->cards_got[obj->card_shown];
        card_init(&card);
        card.select(id);
        spr.set_scale(obj->anims[3].level, 0x1000, 0x1000);
        if (obj->anims[3].level != 0x1000) {
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
        if (obj->anims[3].level != 0x1000) {
            spr.set_pivot(0x140, 0x87);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xA, 0x82, 0x7C);
        if (obj->anims[3].level != 0x1000) {
            spr.set_pivot(0x140, 0xAF);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x10, 0x103, 0xA4);
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xD, 0xFC, 0xA2);
        if (obj->anims[3].level != 0x1000) {
            spr.set_pivot(0x140, 0xA5);
        }
        if (special != 0) {
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xB, 0x4A, 0x92);
        } else if (id == 0x45 || id == 0x70 || id == 0x9B || id == 0xC6 || id == 0xF1) {
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xB, 0x4A, 0x92);
        } else {
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xC, 0xC7, 0x92);
        }
        if (obj->cursor_shown != 0) {
            if (gfx_module.funcs.get_time() - obj->frame_time >= 5) {
                obj->frame_time = gfx_module.funcs.get_time();
                obj->cursor_frame++;
                if (obj->cursor_frame >= 6) {
                    obj->cursor_frame = 0;
                }
            }
            spr.set_layer_id(obj->layer_id, obj->ot_depth - 2);
            spr.set_palette(stcrdshp_booster_cursor_palettes[obj->cursor_frame]);
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 7, obj->card_shown * 0x2A + 0x24, 0x44);
        }
    }
}


/* Lists the player's items that are booster packs (type 0x62) and counts the pages. */
void stcrdshp_find_boosters(StcrdshpBooster *obj) {
    s32 count;
    s32 i;

    count = records_funcs.list_items(1, (u16 *)obj->items);
    obj->booster_count = 0;
    for (i = 0; i < count; i++) {
        if (records_funcs.get_item_icon(obj->items[i]) == 0x62) {
            obj->boosters[obj->booster_count++] = obj->items[i];
        }
    }
    if (obj->booster_count != 0) {
        obj->pages = obj->booster_count / 8 + (obj->booster_count % 8 != 0);
    }
}


/* A booster pack item and what it holds: each of its six cards is one of 16 (ended by
 * item == 0). */
typedef struct StcrdshpBoosterContents {
    /* 0x00 */ s32 item;   /* the item */
    /* 0x04 */ s32 *choices[6]; /* per card: 16 card IDs to choose from */
} StcrdshpBoosterContents; /* size 0x1C */

extern StcrdshpBoosterContents stcrdshp_booster_contents[];

/* Where the cursor goes for booster `i`: two columns, four rows a page. */
#define STCRDSHP_BOOSTER_X(i) ((i) % 2 * 0x83 + 0x1D)
#define STCRDSHP_BOOSTER_Y(i) ((i) % 8 / 2 * 14 + 0x39)

/* Step 4: the cursor on the page's boosters. L1/R1 turn the page, the pad moves the cursor (kept on the page and
 * within the list); cross opens the booster, triangle leaves. */
static inline void stcrdshp_choose_booster(StcrdshpBooster *obj, StcrdshpBoosterData *data) {
    s32 cur;
    s32 first;
    s32 last;

    first = obj->page;
    if (!PAD_HELD(0xB) && PAD_PRESSED(0xA)) {
        obj->page--;
        if (obj->page < 0) {
            obj->page = 0;
        }
    } else if (!PAD_HELD(0xA) && PAD_PRESSED(0xB)) {
        obj->page++;
        if (obj->page > obj->pages - 1) {
            obj->page = obj->pages - 1;
        }
    }
    if (first != obj->page) {
        sound_module.play(0x8004513E);
        obj->chosen = obj->page * 8;
        data->cursor->set_pos(data->cursor, STCRDSHP_BOOSTER_X(obj->chosen), STCRDSHP_BOOSTER_Y(obj->chosen));
        stcrdshp_show_booster_page(obj, data, 1);
        stcrdshp_show_booster_card(obj, data, 1);
        return;
    }
    last = (first + 1) * 8 - 1;
    cur = obj->chosen;
    first *= 8;
    if (last > obj->booster_count - 1) {
        last = obj->booster_count - 1;
    }
    if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
        obj->chosen -= 2;
        if (obj->chosen < first) {
            obj->chosen = first;
        }
    } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
        obj->chosen += 2;
        if (obj->chosen > last) {
            obj->chosen = last;
        }
    }
    if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
        obj->chosen--;
        if (obj->chosen < first) {
            obj->chosen = first;
        }
    } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
        obj->chosen++;
        if (obj->chosen > last) {
            obj->chosen = last;
        }
    }
    if (cur != obj->chosen) {
        sound_module.play(0x8004513E);
        data->cursor->set_pos(data->cursor, STCRDSHP_BOOSTER_X(obj->chosen), STCRDSHP_BOOSTER_Y(obj->chosen));
        stcrdshp_show_booster_card(obj, data, 1);
    } else if (PAD_PRESSED(0xD)) {
        sound_module.play(0x8004503C);
        obj->base.step = 10;
    } else if (PAD_PRESSED(0xE)) {
        sound_module.play(0x800450BD);
        obj->base.step = 50;
        obj->base.substep = 0;
    }
}

/* Step 10: closes the list and the chosen booster; the message says the booster is being opened. */
static inline void stcrdshp_close_booster_list(StcrdshpBooster *obj, StcrdshpBoosterData *data) {
    stcrdshp_util.window_anim_start(&obj->anims[0], 0);
    stcrdshp_show_booster_page(obj, data, 0);
    stcrdshp_util.window_anim_start(&obj->anims[1], 0);
    stcrdshp_show_booster_card(obj, data, 0);
    data->cursor->show(data->cursor, 0);
    data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x32), 0x10);
    data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0x32), 4);
    obj->base.step++;
}

/* Step 11, once the panels have closed (and the last booster's cards have gone): opens the chosen booster. Each of
 * its six cards is one of its 16 at random; they are added to the player's, the booster is used up and the card
 * pack object shows them. */
static inline void stcrdshp_open_booster(StcrdshpBooster *obj, StcrdshpBoosterData *data) {
    s32 booster;
    s32 k;
    s32 i;
    s32 n;

    stcrdshp_util.window_anim_update(&obj->anims[0]);
    if (stcrdshp_util.window_anim_update(&obj->anims[1])) {
        booster = obj->boosters[obj->chosen];
        if (data->opened_cards != NULL) {
            data->opened_cards->base.state = OBJECT_STATE_END;
            return;
        }
        k = 0;
        for (i = 0; stcrdshp_booster_contents[i].item != 0; i++) {
            if (stcrdshp_booster_contents[i].item == booster) {
                k = i;
            }
        }
        for (i = 0; i < 6; i++) {
            n = pad_random.next() % 16;
            obj->cards_got[i] = stcrdshp_booster_contents[k].choices[i][n];
            gamestate_data.funcs.add_card(obj->cards_got[i], 1);
        }
        gamestate_data.items[booster]--;
        data->opened_cards = stcrdshp_create_pack(obj->main, obj->cards_got);
        obj->card_shown = 0;
        stcrdshp_util.window_anim_start(&obj->anims[3], 1);
        obj->base.step++;
    }
}

/* Step 13: left/right show each card got; triangle closes them. */
static inline void stcrdshp_choose_booster_card(StcrdshpBooster *obj, StcrdshpBoosterData *data) {
    s32 old;

    old = obj->card_shown;
    if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
        obj->card_shown--;
        if (obj->card_shown < 0) {
            obj->card_shown = 0;
        }
    } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
        obj->card_shown++;
        if (obj->card_shown >= 6) {
            obj->card_shown = 5;
        }
    }
    if (old != obj->card_shown) {
        sound_module.play(0x4001B);
        stcrdshp_show_booster_result(obj, data, 1);
    } else if (PAD_PRESSED(0xE)) {
        sound_module.play(0x800450BD);
        obj->base.step = 50;
        obj->cursor_shown = 0;
        obj->base.substep = 1;
    }
}

/* Step 55, once the message's panel has closed: closes the list and the chosen booster. */
static inline void stcrdshp_close_booster(StcrdshpBooster *obj, StcrdshpBoosterData *data) {
    if (stcrdshp_util.window_anim_update(&obj->anims[2])) {
        stcrdshp_util.window_anim_start(&obj->anims[0], 0);
        stcrdshp_show_booster_page(obj, data, 0);
        stcrdshp_util.window_anim_start(&obj->anims[1], 0);
        stcrdshp_show_booster_card(obj, data, 0);
        data->cursor->show(data->cursor, 0);
        obj->base.step++;
    }
}

/* The booster screen's steps (base.step): choose a booster, open it, look through its cards. */
void stcrdshp_run_booster(StcrdshpBooster *obj, StcrdshpBoosterData *data) {
    s32 id;
    s32 end;
    s32 first;

    switch (obj->base.step) {
    case 0:
    default:
        stcrdshp_util.window_anim_start(&obj->anims[0], 1);
        obj->base.step++;
        break;
    case 1:
        if (stcrdshp_util.window_anim_update(&obj->anims[0])) {
            stcrdshp_show_booster_page(obj, data, 1);
            stcrdshp_util.window_anim_start(&obj->anims[1], 1);
            stcrdshp_util.window_anim_start(&obj->anims[2], 1);
            obj->base.step++;
        }
        break;
    case 2:
        stcrdshp_util.window_anim_update(&obj->anims[2]);
        if (stcrdshp_util.window_anim_update(&obj->anims[1])) {
            stcrdshp_show_booster_card(obj, data, 1);
            data->cursor->set_pos(data->cursor, STCRDSHP_BOOSTER_X(obj->chosen), STCRDSHP_BOOSTER_Y(obj->chosen));
            data->cursor->show(data->cursor, 1);
            obj->base.step++;
        }
        break;
    case 3:
        obj->base.step++;
        break;
    case 4:
        stcrdshp_choose_booster(obj, data);
        break;
    case 10:
        stcrdshp_close_booster_list(obj, data);
        break;
    case 11:
        stcrdshp_open_booster(obj, data);
        break;
    case 12:
        if (stcrdshp_util.window_anim_update(&obj->anims[3])) {
            stcrdshp_show_booster_result(obj, data, 1);
            if (data->opened_cards->base.state == OBJECT_STATE_RUN) {
                obj->cursor_shown = 1;
                obj->base.step++;
            }
        }
        break;
    case 13:
        stcrdshp_choose_booster_card(obj, data);
        break;
    case 50:
        stcrdshp_util.window_anim_start(&obj->anims[2], 0);
        data->message->set_visible(data->message, 0);
        data->back_hint->set_visible(data->back_hint, 0);
        if (obj->base.substep == 0) {
            obj->base.step = 55;
        } else {
            obj->base.step++;
        }
        break;
    case 51:
        if (stcrdshp_util.window_anim_update(&obj->anims[2])) {
            stcrdshp_util.window_anim_start(&obj->anims[3], 0);
            stcrdshp_show_booster_result(obj, data, 0);
            data->opened_cards->open(data->opened_cards);
            obj->base.step++;
        }
        break;
    case 52:
        if (stcrdshp_util.window_anim_update(&obj->anims[3]) || data->opened_cards == NULL) {
            /* The page's last index is written (page + 1) * 8 - 1 as in step 4 (the page is shifted twice) and its
             * first has a variable of its own. */
            obj->base.step = 0;
            end = (obj->page + 1) * 8 - 1;
            id = obj->boosters[obj->chosen];
            first = obj->page * 8;
            stcrdshp_find_boosters(obj);
            if (end > obj->booster_count - 1) {
                end = obj->booster_count - 1;
            }
            /* The booster is used up: move the cursor back, to the previous page if need be; none left: end. */
            if (gamestate_data.items[id] <= 0) {
                id = obj->boosters[obj->chosen];
                if (gamestate_data.items[id] <= 0) {
                    obj->chosen--;
                    if (obj->chosen < first) {
                        obj->page--;
                        if (obj->page >= 0) {
                            obj->chosen = end;
                        } else {
                            obj->base.state = OBJECT_STATE_END;
                        }
                    }
                }
            }
        }
        break;
    case 55:
        stcrdshp_close_booster(obj, data);
        break;
    case 56:
        stcrdshp_util.window_anim_update(&obj->anims[0]);
        if (stcrdshp_util.window_anim_update(&obj->anims[1])) {
            obj->base.step = 60;
        }
        break;
    case 60:
        obj->base.state = OBJECT_STATE_END;
        break;
    }
}

void stcrdshp_update_booster(StcrdshpBooster *obj, StcrdshpBoosterData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stcrdshp_create_booster_windows(obj, data);
        obj->anims[3].duration = 10;
        obj->anims[2].duration = 10;
        obj->anims[0].duration = 10;
        obj->anims[1].duration = 10;
        stcrdshp_find_boosters(obj);
        break;
    case OBJECT_STATE_RUN:
        stcrdshp_run_booster(obj, data);
        stcrdshp_draw_booster(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StcrdshpBooster *stcrdshp_create_booster(StcrdshpMain *main) {
    StcrdshpBooster *obj = object_new(stcrdshp_update_booster, sizeof(StcrdshpBooster), sizeof(StcrdshpBoosterData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->main = main;
    return obj;
}

/* The booster packs' card lists: 16 card IDs to choose from, per card of a pack. */
s32 stcrdshp_booster_0_0[16] = { 101, 101, 101, 138, 139, 139, 139, 139, 184, 184, 184, 184, 225, 225, 225, 225 };
s32 stcrdshp_booster_0_1[16] = { 57, 57, 57, 58, 58, 58, 186, 186, 230, 230, 311, 311, 311, 312, 312, 312 };
s32 stcrdshp_booster_0_2[16] = { 99, 99, 103, 103, 144, 144, 186, 186, 230, 230, 232, 232, 271, 271, 272, 272 };
s32 stcrdshp_booster_0_3[16] = { 99, 103, 144, 144, 146, 146, 189, 189, 232, 232, 271, 271, 272, 272, 275, 275 };
s32 stcrdshp_booster_0_4[16] = { 144, 144, 144, 146, 146, 189, 189, 232, 232, 271, 271, 272, 272, 272, 275, 275 };
s32 stcrdshp_booster_0_5[16] = { 146, 146, 146, 146, 189, 189, 189, 189, 232, 232, 232, 232, 275, 275, 275, 275 };
s32 stcrdshp_booster_1_0[16] = { 100, 100, 100, 141, 141, 141, 182, 182, 182, 227, 227, 227, 228, 228, 228, 266 };
s32 stcrdshp_booster_1_1[16] = { 59, 59, 100, 142, 142, 142, 187, 187, 187, 187, 268, 270, 313, 313, 313, 313 };
s32 stcrdshp_booster_1_2[16] = { 59, 59, 142, 142, 187, 187, 228, 268, 268, 268, 270, 270, 270, 313, 313, 313 };
s32 stcrdshp_booster_1_3[16] = { 53, 53, 53, 53, 231, 231, 231, 231, 273, 273, 273, 273, 314, 314, 314, 314 };
s32 stcrdshp_booster_1_4[16] = { 102, 102, 143, 143, 145, 145, 188, 188, 231, 231, 273, 273, 274, 274, 314, 314 };
s32 stcrdshp_booster_1_5[16] = { 102, 102, 102, 143, 143, 143, 145, 145, 145, 145, 188, 188, 188, 274, 274, 274 };
s32 stcrdshp_booster_2_0[16] = { 96, 96, 96, 140, 140, 140, 185, 185, 226, 226, 226, 229, 229, 229, 254, 295 };
s32 stcrdshp_booster_2_1[16] = { 60, 60, 60, 136, 136, 136, 178, 178, 178, 185, 264, 264, 264, 265, 265, 265 };
s32 stcrdshp_booster_2_2[16] = { 97, 97, 97, 97, 97, 136, 136, 136, 178, 178, 178, 229, 264, 264, 265, 265 };
s32 stcrdshp_booster_2_3[16] = { 55, 55, 93, 93, 93, 95, 95, 95, 95, 181, 181, 181, 223, 223, 223, 223 };
s32 stcrdshp_booster_2_4[16] = { 55, 55, 98, 98, 98, 98, 138, 138, 138, 224, 224, 224, 269, 269, 269, 269 };
s32 stcrdshp_booster_2_5[16] = { 93, 93, 95, 95, 98, 98, 138, 138, 181, 181, 223, 223, 224, 224, 269, 269 };
s32 stcrdshp_booster_3_0[16] = { 47, 47, 47, 83, 83, 83, 91, 91, 91, 129, 129, 129, 220, 220, 220, 220 };
s32 stcrdshp_booster_3_1[16] = { 24, 24, 24, 24, 24, 56, 56, 56, 130, 130, 130, 130, 220, 299, 299, 299 };
s32 stcrdshp_booster_3_2[16] = { 56, 87, 87, 87, 87, 130, 130, 130, 173, 173, 173, 259, 259, 259, 259, 299 };
s32 stcrdshp_booster_3_3[16] = { 24, 24, 51, 51, 51, 52, 52, 52, 87, 137, 137, 137, 173, 259, 300, 300 };
s32 stcrdshp_booster_3_4[16] = { 94, 94, 94, 137, 137, 179, 179, 179, 180, 180, 180, 222, 222, 222, 300, 300 };
s32 stcrdshp_booster_3_5[16] = { 51, 51, 52, 52, 94, 94, 137, 137, 179, 179, 180, 180, 222, 222, 300, 300 };
s32 stcrdshp_booster_4_0[16] = { 44, 44, 81, 81, 81, 126, 126, 126, 175, 175, 175, 175, 218, 218, 218, 218 };
s32 stcrdshp_booster_4_1[16] = { 30, 30, 81, 85, 85, 85, 174, 174, 174, 175, 216, 216, 216, 218, 257, 257 };
s32 stcrdshp_booster_4_2[16] = { 30, 85, 85, 85, 85, 174, 174, 174, 174, 216, 216, 216, 216, 257, 257, 257 };
s32 stcrdshp_booster_4_3[16] = { 12, 12, 12, 12, 18, 18, 18, 18, 54, 54, 54, 54, 267, 267, 267, 267 };
s32 stcrdshp_booster_4_4[16] = { 12, 18, 54, 54, 134, 134, 134, 134, 221, 221, 221, 221, 266, 266, 266, 266 };
s32 stcrdshp_booster_4_5[16] = { 12, 18, 267, 267, 308, 308, 308, 308, 309, 309, 309, 309, 310, 310, 310, 310 };
s32 stcrdshp_booster_5_0[16] = { 10, 10, 10, 80, 80, 80, 80, 125, 125, 125, 168, 168, 168, 210, 210, 210 };
s32 stcrdshp_booster_5_1[16] = { 43, 43, 43, 43, 43, 50, 50, 50, 50, 50, 125, 168, 212, 212, 212, 212 };
s32 stcrdshp_booster_5_2[16] = { 84, 84, 84, 84, 84, 212, 215, 215, 215, 215, 215, 258, 258, 258, 258, 258 };
s32 stcrdshp_booster_5_3[16] = { 6, 6, 86, 86, 219, 219, 219, 263, 263, 263, 306, 306, 306, 307, 307, 307 };
s32 stcrdshp_booster_5_4[16] = { 6, 6, 6, 11, 11, 17, 17, 86, 86, 86, 92, 92, 92, 219, 219, 219 };
s32 stcrdshp_booster_5_5[16] = { 11, 11, 11, 17, 17, 17, 92, 92, 92, 263, 263, 306, 306, 307, 307, 307 };
s32 stcrdshp_booster_6_0[16] = { 48, 48, 49, 49, 49, 167, 167, 167, 167, 167, 209, 209, 252, 252, 252, 252 };
s32 stcrdshp_booster_6_1[16] = { 15, 15, 15, 42, 42, 128, 128, 128, 128, 214, 214, 214, 214, 298, 298, 298 };
s32 stcrdshp_booster_6_2[16] = { 82, 82, 128, 128, 128, 128, 214, 214, 214, 214, 253, 253, 253, 298, 298, 298 };
s32 stcrdshp_booster_6_3[16] = { 135, 135, 135, 170, 170, 170, 177, 177, 177, 262, 262, 262, 303, 303, 305, 305 };
s32 stcrdshp_booster_6_4[16] = { 23, 23, 23, 29, 29, 29, 170, 170, 170, 170, 262, 262, 305, 305, 305, 305 };
s32 stcrdshp_booster_6_5[16] = { 23, 23, 23, 29, 29, 29, 135, 135, 177, 177, 177, 177, 303, 303, 303, 303 };
s32 stcrdshp_booster_7_0[16] = { 16, 16, 16, 22, 22, 38, 38, 169, 169, 169, 208, 208, 208, 208, 211, 211 };
s32 stcrdshp_booster_7_1[16] = { 3, 3, 3, 3, 3, 16, 122, 122, 122, 124, 124, 124, 169, 213, 213, 213 };
s32 stcrdshp_booster_7_2[16] = { 46, 46, 46, 46, 46, 122, 122, 122, 124, 124, 124, 208, 208, 213, 213, 213 };
s32 stcrdshp_booster_7_3[16] = { 5, 5, 5, 5, 88, 88, 88, 254, 254, 254, 295, 295, 295, 304, 304, 304 };
s32 stcrdshp_booster_7_4[16] = { 5, 5, 5, 5, 90, 90, 90, 132, 132, 132, 254, 254, 254, 304, 304, 304 };
s32 stcrdshp_booster_7_5[16] = { 5, 5, 5, 5, 90, 90, 90, 132, 132, 132, 261, 261, 261, 302, 302, 302 };
s32 stcrdshp_booster_8_0[16] = { 21, 28, 41, 41, 41, 45, 45, 45, 75, 75, 75, 75, 202, 202, 202, 202 };
s32 stcrdshp_booster_8_1[16] = { 21, 27, 27, 28, 79, 79, 79, 79, 164, 164, 164, 164, 294, 294, 294, 294 };
s32 stcrdshp_booster_8_2[16] = { 27, 27, 121, 121, 121, 121, 171, 171, 172, 172, 293, 293, 293, 293, 294, 294 };
s32 stcrdshp_booster_8_3[16] = { 79, 164, 171, 171, 171, 171, 172, 172, 176, 176, 176, 176, 217, 217, 217, 217 };
s32 stcrdshp_booster_8_4[16] = { 89, 89, 89, 89, 121, 133, 133, 133, 260, 260, 260, 293, 301, 301, 301, 301 };
s32 stcrdshp_booster_8_5[16] = { 89, 89, 89, 133, 133, 133, 133, 176, 176, 217, 217, 260, 260, 301, 301, 301 };
s32 stcrdshp_booster_9_0[16] = { 9, 9, 9, 14, 14, 113, 113, 113, 113, 158, 158, 158, 160, 160, 160, 160 };
s32 stcrdshp_booster_9_1[16] = { 4, 4, 4, 4, 113, 160, 249, 249, 249, 249, 249, 287, 287, 287, 287, 287 };
s32 stcrdshp_booster_9_2[16] = { 74, 74, 74, 74, 78, 78, 165, 165, 203, 203, 203, 203, 206, 250, 250, 250 };
s32 stcrdshp_booster_9_3[16] = { 78, 78, 78, 78, 127, 127, 127, 255, 255, 255, 296, 296, 296, 297, 297, 297 };
s32 stcrdshp_booster_9_4[16] = { 131, 131, 131, 206, 206, 206, 206, 255, 255, 255, 256, 256, 256, 296, 296, 296 };
s32 stcrdshp_booster_9_5[16] = { 127, 127, 127, 131, 131, 131, 165, 165, 165, 165, 256, 256, 256, 297, 297, 297 };
s32 stcrdshp_booster_10_0[16] = { 26, 26, 67, 67, 110, 110, 110, 110, 115, 115, 115, 115, 157, 157, 157, 157 };
s32 stcrdshp_booster_10_1[16] = { 2, 2, 2, 36, 36, 36, 36, 73, 73, 73, 152, 152, 204, 204, 204, 204 };
s32 stcrdshp_booster_10_2[16] = { 154, 154, 154, 154, 161, 161, 161, 161, 161, 161, 205, 205, 205, 205, 205, 205 };
s32 stcrdshp_booster_10_3[16] = { 2, 77, 77, 77, 120, 120, 120, 207, 207, 207, 251, 251, 251, 290, 290, 290 };
s32 stcrdshp_booster_10_4[16] = { 120, 120, 120, 123, 123, 123, 154, 166, 166, 166, 207, 207, 207, 290, 290, 290 };
s32 stcrdshp_booster_10_5[16] = { 77, 77, 77, 77, 123, 123, 123, 123, 166, 166, 166, 166, 251, 251, 251, 251 };
s32 stcrdshp_booster_11_0[16] = { 8, 72, 72, 72, 72, 116, 116, 116, 116, 116, 199, 199, 201, 201, 201, 201 };
s32 stcrdshp_booster_11_1[16] = { 20, 20, 20, 71, 71, 111, 111, 111, 111, 111, 153, 153, 284, 284, 284, 284 };
s32 stcrdshp_booster_11_2[16] = { 37, 37, 37, 111, 111, 111, 111, 111, 153, 153, 153, 153, 153, 284, 284, 284 };
s32 stcrdshp_booster_11_3[16] = { 76, 76, 117, 162, 162, 162, 162, 247, 247, 247, 247, 286, 286, 286, 286, 289 };
s32 stcrdshp_booster_11_4[16] = { 118, 118, 118, 118, 162, 162, 163, 163, 163, 163, 247, 286, 291, 291, 291, 291 };
s32 stcrdshp_booster_11_5[16] = { 76, 76, 76, 76, 117, 117, 117, 117, 118, 118, 163, 289, 289, 289, 289, 291 };
s32 stcrdshp_booster_12_0[16] = { 40, 40, 40, 148, 148, 148, 148, 191, 191, 191, 191, 193, 193, 281, 281, 281 };
s32 stcrdshp_booster_12_1[16] = { 70, 70, 70, 148, 148, 156, 156, 156, 191, 191, 200, 200, 200, 285, 285, 285 };
s32 stcrdshp_booster_12_2[16] = { 109, 109, 109, 156, 156, 156, 156, 280, 280, 282, 282, 282, 285, 285, 285, 285 };
s32 stcrdshp_booster_12_3[16] = { 70, 114, 114, 114, 114, 114, 242, 242, 242, 242, 248, 248, 248, 248, 248, 248 };
s32 stcrdshp_booster_12_4[16] = { 159, 159, 159, 159, 159, 200, 243, 243, 243, 243, 246, 246, 246, 246, 246, 246 };
s32 stcrdshp_booster_12_5[16] = { 114, 114, 119, 119, 119, 119, 119, 159, 159, 246, 248, 288, 288, 288, 288, 288 };
s32 stcrdshp_booster_13_0[16] = { 63, 63, 63, 64, 64, 105, 105, 105, 105, 105, 149, 149, 149, 149, 149, 192 };
s32 stcrdshp_booster_13_1[16] = { 39, 39, 106, 106, 106, 106, 150, 150, 150, 194, 194, 194, 196, 196, 196, 196 };
s32 stcrdshp_booster_13_2[16] = { 66, 66, 66, 106, 106, 106, 196, 196, 196, 279, 279, 279, 283, 283, 283, 283 };
s32 stcrdshp_booster_13_3[16] = { 239, 239, 239, 240, 240, 240, 244, 244, 244, 244, 245, 245, 245, 292, 292, 292 };
s32 stcrdshp_booster_13_4[16] = { 68, 68, 68, 108, 108, 108, 244, 244, 244, 245, 245, 245, 245, 292, 292, 292 };
s32 stcrdshp_booster_13_5[16] = { 244, 244, 244, 244, 245, 245, 245, 245, 283, 283, 292, 292, 292, 292, 292, 292 };
s32 stcrdshp_booster_14_0[16] = { 61, 62, 62, 62, 62, 62, 104, 104, 104, 104, 104, 147, 147, 190, 190, 233 };
s32 stcrdshp_booster_14_1[16] = { 65, 65, 65, 65, 276, 276, 276, 277, 277, 277, 278, 278, 278, 278, 278, 278 };
s32 stcrdshp_booster_14_2[16] = { 234, 234, 234, 234, 235, 235, 235, 236, 236, 236, 236, 236, 237, 237, 238, 238 };
s32 stcrdshp_booster_14_3[16] = { 107, 107, 107, 107, 151, 151, 151, 151, 195, 195, 195, 195, 197, 197, 197, 197 };
s32 stcrdshp_booster_14_4[16] = { 151, 151, 151, 195, 195, 195, 237, 237, 237, 237, 237, 237, 237, 278, 278, 278 };
s32 stcrdshp_booster_14_5[16] = { 107, 107, 107, 197, 197, 197, 238, 238, 238, 238, 238, 238, 238, 278, 278, 278 };
s32 stcrdshp_booster_15_0[16] = { 100, 100, 100, 136, 139, 139, 140, 140, 140, 225, 225, 225, 228, 228, 228, 264 };
s32 stcrdshp_booster_15_1[16] = { 100, 100, 101, 101, 141, 141, 146, 146, 189, 189, 227, 227, 228, 228, 275, 275 };
s32 stcrdshp_booster_15_2[16] = { 57, 57, 59, 97, 97, 103, 103, 142, 142, 186, 186, 268, 268, 311, 311, 312 };
s32 stcrdshp_booster_15_3[16] = { 59, 59, 103, 103, 145, 145, 145, 146, 146, 189, 189, 268, 268, 268, 312, 312 };
s32 stcrdshp_booster_15_4[16] = { 102, 102, 138, 143, 143, 145, 145, 146, 146, 188, 188, 189, 189, 274, 274, 275 };
s32 stcrdshp_booster_15_5[16] = { 102, 102, 143, 143, 145, 145, 146, 146, 188, 188, 189, 189, 274, 274, 275, 275 };
s32 stcrdshp_booster_16_0[16] = { 43, 43, 44, 56, 56, 84, 84, 125, 125, 126, 259, 259, 270, 270, 270, 270 };
s32 stcrdshp_booster_16_1[16] = { 30, 30, 30, 56, 56, 83, 91, 91, 129, 129, 130, 130, 130, 258, 258, 258 };
s32 stcrdshp_booster_16_2[16] = { 12, 12, 43, 84, 137, 137, 137, 137, 144, 144, 259, 263, 263, 263, 263, 270 };
s32 stcrdshp_booster_16_3[16] = { 11, 12, 12, 53, 53, 134, 137, 137, 144, 144, 263, 271, 271, 307, 314, 314 };
s32 stcrdshp_booster_16_4[16] = { 11, 11, 12, 12, 53, 134, 134, 137, 137, 144, 263, 263, 271, 307, 307, 314 };
s32 stcrdshp_booster_16_5[16] = { 11, 11, 12, 53, 53, 134, 137, 144, 144, 263, 263, 271, 271, 307, 307, 314 };
s32 stcrdshp_booster_17_0[16] = { 96, 96, 182, 182, 183, 183, 183, 184, 184, 184, 185, 185, 226, 226, 229, 229 };
s32 stcrdshp_booster_17_1[16] = { 24, 24, 93, 93, 93, 95, 95, 95, 181, 181, 181, 183, 183, 184, 185, 229 };
s32 stcrdshp_booster_17_2[16] = { 58, 60, 99, 99, 99, 178, 178, 178, 187, 230, 230, 230, 265, 265, 265, 313 };
s32 stcrdshp_booster_17_3[16] = { 24, 55, 58, 58, 60, 60, 187, 187, 232, 232, 269, 269, 272, 272, 313, 313 };
s32 stcrdshp_booster_17_4[16] = { 24, 55, 93, 93, 95, 95, 181, 181, 223, 223, 232, 232, 269, 269, 272, 272 };
s32 stcrdshp_booster_17_5[16] = { 24, 55, 93, 93, 95, 95, 181, 181, 223, 223, 232, 232, 269, 269, 272, 272 };
s32 stcrdshp_booster_18_0[16] = { 47, 47, 48, 168, 168, 175, 175, 183, 183, 183, 210, 210, 218, 218, 220, 220 };
s32 stcrdshp_booster_18_1[16] = { 18, 18, 47, 47, 48, 92, 92, 92, 183, 183, 220, 220, 222, 222, 267, 267 };
s32 stcrdshp_booster_18_2[16] = { 50, 50, 87, 87, 87, 173, 173, 173, 212, 214, 214, 257, 298, 298, 299, 299 };
s32 stcrdshp_booster_18_3[16] = { 50, 50, 87, 87, 173, 173, 179, 179, 179, 222, 222, 224, 257, 257, 257, 267 };
s32 stcrdshp_booster_18_4[16] = { 17, 17, 98, 98, 179, 179, 222, 222, 224, 224, 231, 231, 267, 267, 273, 273 };
s32 stcrdshp_booster_18_5[16] = { 17, 17, 18, 18, 23, 23, 29, 29, 92, 92, 179, 179, 222, 222, 267, 267 };
s32 stcrdshp_booster_19_0[16] = { 10, 10, 15, 15, 42, 42, 49, 49, 128, 128, 209, 209, 252, 252, 253, 253 };
s32 stcrdshp_booster_19_1[16] = { 80, 80, 81, 81, 82, 82, 85, 85, 174, 174, 184, 184, 216, 216, 229, 229 };
s32 stcrdshp_booster_19_2[16] = { 15, 42, 42, 42, 82, 82, 85, 85, 128, 128, 174, 174, 216, 216, 253, 253 };
s32 stcrdshp_booster_19_3[16] = { 6, 51, 52, 54, 135, 135, 135, 300, 300, 300, 303, 303, 303, 310, 310, 310 };
s32 stcrdshp_booster_19_4[16] = { 6, 6, 6, 52, 52, 52, 135, 135, 300, 303, 306, 306, 309, 309, 310, 310 };
s32 stcrdshp_booster_19_5[16] = { 51, 51, 52, 52, 54, 54, 300, 300, 303, 303, 306, 306, 309, 309, 310, 310 };
s32 stcrdshp_booster_20_0[16] = { 16, 16, 22, 22, 96, 96, 96, 160, 169, 169, 182, 182, 182, 211, 226, 226 };
s32 stcrdshp_booster_20_1[16] = { 3, 3, 96, 164, 164, 169, 169, 169, 182, 208, 208, 226, 226, 226, 293, 294 };
s32 stcrdshp_booster_20_2[16] = { 3, 3, 164, 164, 164, 208, 208, 208, 213, 215, 293, 293, 293, 294, 294, 294 };
s32 stcrdshp_booster_20_3[16] = { 86, 86, 170, 170, 170, 213, 213, 215, 215, 226, 226, 226, 295, 295, 304, 304 };
s32 stcrdshp_booster_20_4[16] = { 86, 86, 88, 88, 94, 94, 170, 170, 177, 177, 180, 180, 219, 219, 221, 221 };
s32 stcrdshp_booster_20_5[16] = { 88, 88, 94, 94, 177, 177, 180, 180, 219, 219, 221, 221, 295, 295, 304, 304 };
s32 stcrdshp_booster_21_0[16] = { 9, 9, 27, 27, 75, 75, 79, 79, 79, 113, 113, 122, 122, 122, 249, 249 };
s32 stcrdshp_booster_21_1[16] = { 38, 38, 45, 45, 46, 46, 46, 121, 121, 124, 124, 124, 167, 167, 185, 185 };
s32 stcrdshp_booster_21_2[16] = { 27, 27, 27, 46, 46, 46, 79, 79, 121, 121, 122, 122, 124, 124, 249, 249 };
s32 stcrdshp_booster_21_3[16] = { 5, 132, 254, 261, 262, 262, 262, 266, 266, 266, 305, 305, 305, 308, 308, 308 };
s32 stcrdshp_booster_21_4[16] = { 5, 5, 5, 132, 132, 132, 254, 254, 254, 261, 261, 261, 262, 266, 305, 308 };
s32 stcrdshp_booster_21_5[16] = { 5, 5, 132, 132, 133, 133, 254, 254, 260, 260, 261, 261, 262, 262, 305, 305 };
s32 stcrdshp_booster_22_0[16] = { 8, 71, 71, 71, 72, 72, 72, 110, 115, 115, 115, 115, 116, 116, 116, 116 };
s32 stcrdshp_booster_22_1[16] = { 2, 2, 20, 20, 20, 36, 36, 36, 37, 37, 111, 111, 115, 116, 205, 205 };
s32 stcrdshp_booster_22_2[16] = { 2, 2, 20, 20, 36, 36, 123, 123, 127, 127, 131, 131, 205, 205, 255, 255 };
s32 stcrdshp_booster_22_3[16] = { 77, 77, 77, 78, 120, 120, 120, 123, 123, 123, 127, 131, 255, 290, 290, 290 };
s32 stcrdshp_booster_22_4[16] = { 77, 77, 77, 78, 78, 78, 120, 120, 120, 123, 127, 131, 255, 290, 290, 290 };
s32 stcrdshp_booster_22_5[16] = { 77, 78, 78, 78, 120, 123, 127, 127, 127, 131, 131, 131, 255, 255, 255, 290 };
s32 stcrdshp_booster_23_0[16] = { 14, 21, 21, 21, 28, 28, 28, 41, 41, 41, 158, 158, 158, 202, 202, 202 };
s32 stcrdshp_booster_23_1[16] = { 21, 89, 89, 90, 90, 158, 176, 176, 176, 202, 217, 217, 217, 301, 301, 301 };
s32 stcrdshp_booster_23_2[16] = { 4, 4, 4, 74, 74, 74, 203, 203, 203, 250, 250, 250, 287, 287, 301, 302 };
s32 stcrdshp_booster_23_3[16] = { 4, 74, 89, 89, 89, 90, 90, 90, 171, 171, 176, 176, 176, 217, 217, 250 };
s32 stcrdshp_booster_23_4[16] = { 89, 89, 89, 90, 171, 172, 176, 176, 176, 217, 217, 217, 301, 301, 301, 302 };
s32 stcrdshp_booster_23_5[16] = { 89, 90, 90, 90, 171, 171, 171, 172, 172, 172, 176, 217, 301, 302, 302, 302 };
s32 stcrdshp_booster_24_0[16] = { 26, 26, 73, 73, 73, 73, 152, 152, 157, 157, 157, 157, 201, 201, 201, 201 };
s32 stcrdshp_booster_24_1[16] = { 73, 73, 166, 166, 166, 199, 256, 256, 256, 256, 296, 296, 296, 297, 297, 297 };
s32 stcrdshp_booster_24_2[16] = { 153, 153, 154, 154, 157, 161, 161, 161, 204, 204, 204, 207, 207, 207, 284, 284 };
s32 stcrdshp_booster_24_3[16] = { 165, 165, 166, 166, 166, 201, 206, 206, 251, 251, 256, 256, 296, 296, 297, 297 };
s32 stcrdshp_booster_24_4[16] = { 165, 165, 165, 166, 206, 206, 206, 207, 207, 207, 251, 251, 251, 256, 296, 297 };
s32 stcrdshp_booster_24_5[16] = { 165, 166, 166, 166, 206, 207, 251, 256, 256, 256, 296, 296, 296, 297, 297, 297 };
s32 stcrdshp_booster_25_0[16] = { 21, 21, 21, 21, 148, 148, 148, 158, 158, 158, 158, 193, 202, 202, 202, 202 };
s32 stcrdshp_booster_25_1[16] = { 21, 158, 159, 159, 159, 162, 162, 162, 202, 202, 242, 242, 242, 248, 248, 248 };
s32 stcrdshp_booster_25_2[16] = { 76, 76, 80, 80, 156, 156, 163, 163, 200, 200, 209, 209, 252, 252, 280, 280 };
s32 stcrdshp_booster_25_3[16] = { 76, 76, 80, 80, 159, 159, 162, 162, 163, 163, 209, 209, 242, 248, 252, 252 };
s32 stcrdshp_booster_25_4[16] = { 159, 159, 159, 159, 242, 242, 242, 242, 248, 248, 248, 248, 286, 288, 288, 289 };
s32 stcrdshp_booster_25_5[16] = { 76, 162, 163, 163, 286, 286, 286, 286, 288, 288, 288, 288, 289, 289, 289, 289 };
s32 stcrdshp_booster_26_0[16] = { 67, 70, 70, 70, 73, 73, 73, 109, 109, 201, 201, 281, 281, 285, 285, 285 };
s32 stcrdshp_booster_26_1[16] = { 40, 40, 41, 41, 81, 81, 81, 115, 115, 115, 178, 178, 191, 282, 282, 282 };
s32 stcrdshp_booster_26_2[16] = { 70, 70, 70, 73, 73, 73, 115, 115, 115, 117, 118, 119, 285, 285, 285, 291 };
s32 stcrdshp_booster_26_3[16] = { 114, 114, 117, 117, 118, 118, 119, 119, 243, 243, 246, 246, 247, 247, 291, 291 };
s32 stcrdshp_booster_26_4[16] = { 58, 58, 99, 99, 114, 114, 178, 178, 243, 243, 246, 246, 246, 247, 247, 247 };
s32 stcrdshp_booster_26_5[16] = { 58, 58, 99, 117, 117, 117, 118, 118, 118, 119, 119, 119, 178, 291, 291, 291 };
s32 stcrdshp_booster_27_0[16] = { 14, 28, 28, 28, 28, 28, 63, 63, 63, 63, 64, 105, 105, 105, 105, 199 };
s32 stcrdshp_booster_27_1[16] = { 28, 28, 63, 63, 66, 66, 66, 105, 105, 108, 108, 194, 194, 194, 239, 239 };
s32 stcrdshp_booster_27_2[16] = { 66, 66, 72, 72, 72, 106, 106, 110, 110, 116, 116, 116, 194, 194, 279, 279 };
s32 stcrdshp_booster_27_3[16] = { 2, 2, 4, 4, 4, 36, 36, 72, 72, 72, 106, 110, 116, 116, 116, 279 };
s32 stcrdshp_booster_27_4[16] = { 2, 4, 36, 74, 74, 74, 205, 244, 244, 244, 250, 250, 250, 292, 292, 292 };
s32 stcrdshp_booster_27_5[16] = { 2, 2, 2, 4, 4, 4, 36, 36, 36, 74, 205, 205, 205, 244, 250, 292 };
s32 stcrdshp_booster_28_0[16] = { 39, 147, 149, 149, 149, 149, 149, 149, 157, 157, 157, 157, 157, 157, 192, 233 };
s32 stcrdshp_booster_28_1[16] = { 10, 10, 10, 149, 150, 150, 152, 152, 157, 157, 196, 196, 236, 236, 277, 277 };
s32 stcrdshp_booster_28_2[16] = { 10, 10, 150, 152, 152, 187, 187, 187, 196, 196, 230, 230, 230, 236, 277, 277 };
s32 stcrdshp_booster_28_3[16] = { 68, 68, 151, 151, 197, 197, 237, 237, 240, 240, 245, 245, 245, 283, 283, 283 };
s32 stcrdshp_booster_28_4[16] = { 60, 68, 68, 68, 187, 230, 240, 240, 240, 245, 245, 245, 283, 283, 283, 313 };
s32 stcrdshp_booster_28_5[16] = { 60, 151, 151, 151, 187, 197, 197, 197, 230, 237, 237, 237, 240, 240, 240, 313 };
s32 stcrdshp_booster_29_0[16] = { 8, 26, 61, 62, 62, 62, 62, 62, 62, 104, 104, 104, 104, 104, 104, 190 };
s32 stcrdshp_booster_29_1[16] = { 9, 9, 49, 49, 49, 62, 65, 65, 65, 104, 234, 234, 235, 235, 276, 276 };
s32 stcrdshp_booster_29_2[16] = { 9, 9, 49, 49, 65, 65, 65, 71, 71, 203, 234, 234, 234, 276, 276, 287 };
s32 stcrdshp_booster_29_3[16] = { 107, 107, 111, 111, 111, 111, 195, 195, 238, 238, 265, 265, 265, 265, 278, 278 };
s32 stcrdshp_booster_29_4[16] = { 20, 20, 20, 20, 37, 37, 37, 37, 107, 107, 195, 195, 238, 238, 278, 278 };
s32 stcrdshp_booster_29_5[16] = { 20, 20, 37, 37, 111, 111, 111, 111, 203, 203, 265, 265, 265, 265, 287, 287 };
s32 stcrdshp_booster_30_0[16] = { 96, 96, 96, 96, 183, 183, 183, 185, 185, 185, 227, 227, 227, 228, 228, 228 };
s32 stcrdshp_booster_30_1[16] = { 101, 101, 101, 139, 139, 139, 139, 184, 184, 184, 226, 226, 226, 229, 229, 229 };
s32 stcrdshp_booster_30_2[16] = { 100, 100, 100, 140, 140, 140, 141, 141, 141, 182, 182, 182, 182, 225, 225, 225 };
s32 stcrdshp_booster_30_3[16] = { 100, 100, 100, 141, 141, 141, 182, 182, 182, 183, 183, 183, 225, 225, 225, 225 };
s32 stcrdshp_booster_30_4[16] = { 101, 101, 101, 139, 139, 139, 184, 184, 184, 226, 226, 226, 226, 229, 229, 229 };
s32 stcrdshp_booster_30_5[16] = { 96, 96, 96, 140, 140, 140, 185, 185, 185, 227, 227, 227, 227, 228, 228, 228 };
s32 stcrdshp_booster_31_0[16] = { 44, 44, 91, 91, 91, 91, 125, 125, 125, 126, 126, 126, 126, 129, 129, 129 };
s32 stcrdshp_booster_31_1[16] = { 10, 10, 10, 10, 80, 80, 80, 81, 81, 81, 175, 175, 175, 218, 218, 218 };
s32 stcrdshp_booster_31_2[16] = { 47, 47, 47, 47, 91, 91, 91, 126, 126, 126, 168, 168, 168, 220, 220, 220 };
s32 stcrdshp_booster_31_3[16] = { 10, 10, 10, 81, 81, 81, 83, 83, 83, 210, 210, 210, 210, 218, 218, 218 };
s32 stcrdshp_booster_31_4[16] = { 47, 47, 47, 125, 125, 125, 125, 129, 129, 129, 168, 168, 168, 220, 220, 220 };
s32 stcrdshp_booster_31_5[16] = { 80, 80, 80, 80, 83, 83, 83, 83, 175, 175, 175, 175, 210, 210, 210, 210 };
s32 stcrdshp_booster_32_0[16] = { 28, 28, 28, 28, 38, 38, 38, 38, 45, 45, 45, 45, 49, 49, 49, 49 };
s32 stcrdshp_booster_32_1[16] = { 16, 16, 16, 16, 16, 16, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41 };
s32 stcrdshp_booster_32_2[16] = { 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 48, 48, 48, 48, 48, 48 };
s32 stcrdshp_booster_32_3[16] = { 75, 75, 75, 75, 75, 75, 75, 75, 202, 202, 202, 202, 202, 202, 202, 202 };
s32 stcrdshp_booster_32_4[16] = { 167, 167, 169, 169, 209, 209, 209, 209, 209, 211, 211, 211, 211, 211, 252, 252 };
s32 stcrdshp_booster_32_5[16] = { 167, 167, 167, 167, 169, 169, 169, 169, 209, 209, 211, 211, 252, 252, 252, 252 };
s32 stcrdshp_booster_33_0[16] = { 8, 8, 8, 8, 14, 14, 14, 14, 14, 14, 26, 26, 26, 26, 26, 26 };
s32 stcrdshp_booster_33_1[16] = { 67, 67, 67, 67, 72, 72, 113, 113, 113, 113, 115, 115, 157, 157, 201, 201 };
s32 stcrdshp_booster_33_2[16] = { 73, 73, 73, 110, 110, 116, 116, 116, 152, 152, 158, 158, 158, 160, 160, 160 };
s32 stcrdshp_booster_33_3[16] = { 71, 71, 72, 72, 72, 113, 113, 113, 115, 115, 115, 199, 199, 201, 201, 201 };
s32 stcrdshp_booster_33_4[16] = { 9, 9, 72, 72, 72, 110, 110, 113, 113, 113, 115, 115, 115, 201, 201, 201 };
s32 stcrdshp_booster_33_5[16] = { 9, 9, 73, 73, 73, 110, 110, 116, 116, 116, 157, 157, 157, 158, 158, 158 };
s32 stcrdshp_booster_34_0[16] = { 39, 39, 39, 40, 40, 40, 105, 105, 105, 191, 191, 191, 281, 281, 281, 281 };
s32 stcrdshp_booster_34_1[16] = { 61, 61, 61, 105, 105, 105, 149, 149, 149, 192, 192, 192, 281, 281, 281, 281 };
s32 stcrdshp_booster_34_2[16] = { 62, 62, 62, 104, 104, 104, 148, 148, 148, 233, 233, 233, 281, 281, 281, 281 };
s32 stcrdshp_booster_34_3[16] = { 63, 63, 63, 147, 147, 147, 149, 149, 149, 190, 190, 190, 281, 281, 281, 281 };
s32 stcrdshp_booster_34_4[16] = { 63, 63, 63, 64, 64, 64, 191, 191, 191, 193, 193, 193, 281, 281, 281, 281 };
s32 stcrdshp_booster_34_5[16] = { 40, 40, 40, 62, 62, 62, 104, 104, 104, 148, 148, 148, 281, 281, 281, 281 };

StcrdshpBoosterContents stcrdshp_booster_contents[36] = {
    { 91,
      { stcrdshp_booster_0_0, stcrdshp_booster_0_1, stcrdshp_booster_0_2,
        stcrdshp_booster_0_3, stcrdshp_booster_0_4, stcrdshp_booster_0_5 } },
    { 361,
      { stcrdshp_booster_1_0, stcrdshp_booster_1_1, stcrdshp_booster_1_2,
        stcrdshp_booster_1_3, stcrdshp_booster_1_4, stcrdshp_booster_1_5 } },
    { 362,
      { stcrdshp_booster_2_0, stcrdshp_booster_2_1, stcrdshp_booster_2_2,
        stcrdshp_booster_2_3, stcrdshp_booster_2_4, stcrdshp_booster_2_5 } },
    { 363,
      { stcrdshp_booster_3_0, stcrdshp_booster_3_1, stcrdshp_booster_3_2,
        stcrdshp_booster_3_3, stcrdshp_booster_3_4, stcrdshp_booster_3_5 } },
    { 364,
      { stcrdshp_booster_4_0, stcrdshp_booster_4_1, stcrdshp_booster_4_2,
        stcrdshp_booster_4_3, stcrdshp_booster_4_4, stcrdshp_booster_4_5 } },
    { 365,
      { stcrdshp_booster_5_0, stcrdshp_booster_5_1, stcrdshp_booster_5_2,
        stcrdshp_booster_5_3, stcrdshp_booster_5_4, stcrdshp_booster_5_5 } },
    { 366,
      { stcrdshp_booster_6_0, stcrdshp_booster_6_1, stcrdshp_booster_6_2,
        stcrdshp_booster_6_3, stcrdshp_booster_6_4, stcrdshp_booster_6_5 } },
    { 367,
      { stcrdshp_booster_7_0, stcrdshp_booster_7_1, stcrdshp_booster_7_2,
        stcrdshp_booster_7_3, stcrdshp_booster_7_4, stcrdshp_booster_7_5 } },
    { 368,
      { stcrdshp_booster_8_0, stcrdshp_booster_8_1, stcrdshp_booster_8_2,
        stcrdshp_booster_8_3, stcrdshp_booster_8_4, stcrdshp_booster_8_5 } },
    { 369,
      { stcrdshp_booster_9_0, stcrdshp_booster_9_1, stcrdshp_booster_9_2,
        stcrdshp_booster_9_3, stcrdshp_booster_9_4, stcrdshp_booster_9_5 } },
    { 370,
      { stcrdshp_booster_10_0, stcrdshp_booster_10_1, stcrdshp_booster_10_2,
        stcrdshp_booster_10_3, stcrdshp_booster_10_4, stcrdshp_booster_10_5 } },
    { 371,
      { stcrdshp_booster_11_0, stcrdshp_booster_11_1, stcrdshp_booster_11_2,
        stcrdshp_booster_11_3, stcrdshp_booster_11_4, stcrdshp_booster_11_5 } },
    { 372,
      { stcrdshp_booster_12_0, stcrdshp_booster_12_1, stcrdshp_booster_12_2,
        stcrdshp_booster_12_3, stcrdshp_booster_12_4, stcrdshp_booster_12_5 } },
    { 373,
      { stcrdshp_booster_13_0, stcrdshp_booster_13_1, stcrdshp_booster_13_2,
        stcrdshp_booster_13_3, stcrdshp_booster_13_4, stcrdshp_booster_13_5 } },
    { 374,
      { stcrdshp_booster_14_0, stcrdshp_booster_14_1, stcrdshp_booster_14_2,
        stcrdshp_booster_14_3, stcrdshp_booster_14_4, stcrdshp_booster_14_5 } },
    { 375,
      { stcrdshp_booster_15_0, stcrdshp_booster_15_1, stcrdshp_booster_15_2,
        stcrdshp_booster_15_3, stcrdshp_booster_15_4, stcrdshp_booster_15_5 } },
    { 376,
      { stcrdshp_booster_16_0, stcrdshp_booster_16_1, stcrdshp_booster_16_2,
        stcrdshp_booster_16_3, stcrdshp_booster_16_4, stcrdshp_booster_16_5 } },
    { 377,
      { stcrdshp_booster_17_0, stcrdshp_booster_17_1, stcrdshp_booster_17_2,
        stcrdshp_booster_17_3, stcrdshp_booster_17_4, stcrdshp_booster_17_5 } },
    { 378,
      { stcrdshp_booster_18_0, stcrdshp_booster_18_1, stcrdshp_booster_18_2,
        stcrdshp_booster_18_3, stcrdshp_booster_18_4, stcrdshp_booster_18_5 } },
    { 379,
      { stcrdshp_booster_19_0, stcrdshp_booster_19_1, stcrdshp_booster_19_2,
        stcrdshp_booster_19_3, stcrdshp_booster_19_4, stcrdshp_booster_19_5 } },
    { 380,
      { stcrdshp_booster_20_0, stcrdshp_booster_20_1, stcrdshp_booster_20_2,
        stcrdshp_booster_20_3, stcrdshp_booster_20_4, stcrdshp_booster_20_5 } },
    { 381,
      { stcrdshp_booster_21_0, stcrdshp_booster_21_1, stcrdshp_booster_21_2,
        stcrdshp_booster_21_3, stcrdshp_booster_21_4, stcrdshp_booster_21_5 } },
    { 382,
      { stcrdshp_booster_22_0, stcrdshp_booster_22_1, stcrdshp_booster_22_2,
        stcrdshp_booster_22_3, stcrdshp_booster_22_4, stcrdshp_booster_22_5 } },
    { 383,
      { stcrdshp_booster_23_0, stcrdshp_booster_23_1, stcrdshp_booster_23_2,
        stcrdshp_booster_23_3, stcrdshp_booster_23_4, stcrdshp_booster_23_5 } },
    { 384,
      { stcrdshp_booster_24_0, stcrdshp_booster_24_1, stcrdshp_booster_24_2,
        stcrdshp_booster_24_3, stcrdshp_booster_24_4, stcrdshp_booster_24_5 } },
    { 385,
      { stcrdshp_booster_25_0, stcrdshp_booster_25_1, stcrdshp_booster_25_2,
        stcrdshp_booster_25_3, stcrdshp_booster_25_4, stcrdshp_booster_25_5 } },
    { 386,
      { stcrdshp_booster_26_0, stcrdshp_booster_26_1, stcrdshp_booster_26_2,
        stcrdshp_booster_26_3, stcrdshp_booster_26_4, stcrdshp_booster_26_5 } },
    { 387,
      { stcrdshp_booster_27_0, stcrdshp_booster_27_1, stcrdshp_booster_27_2,
        stcrdshp_booster_27_3, stcrdshp_booster_27_4, stcrdshp_booster_27_5 } },
    { 388,
      { stcrdshp_booster_28_0, stcrdshp_booster_28_1, stcrdshp_booster_28_2,
        stcrdshp_booster_28_3, stcrdshp_booster_28_4, stcrdshp_booster_28_5 } },
    { 389,
      { stcrdshp_booster_29_0, stcrdshp_booster_29_1, stcrdshp_booster_29_2,
        stcrdshp_booster_29_3, stcrdshp_booster_29_4, stcrdshp_booster_29_5 } },
    { 390,
      { stcrdshp_booster_30_0, stcrdshp_booster_30_1, stcrdshp_booster_30_2,
        stcrdshp_booster_30_3, stcrdshp_booster_30_4, stcrdshp_booster_30_5 } },
    { 391,
      { stcrdshp_booster_31_0, stcrdshp_booster_31_1, stcrdshp_booster_31_2,
        stcrdshp_booster_31_3, stcrdshp_booster_31_4, stcrdshp_booster_31_5 } },
    { 392,
      { stcrdshp_booster_32_0, stcrdshp_booster_32_1, stcrdshp_booster_32_2,
        stcrdshp_booster_32_3, stcrdshp_booster_32_4, stcrdshp_booster_32_5 } },
    { 393,
      { stcrdshp_booster_33_0, stcrdshp_booster_33_1, stcrdshp_booster_33_2,
        stcrdshp_booster_33_3, stcrdshp_booster_33_4, stcrdshp_booster_33_5 } },
    { 394,
      { stcrdshp_booster_34_0, stcrdshp_booster_34_1, stcrdshp_booster_34_2,
        stcrdshp_booster_34_3, stcrdshp_booster_34_4, stcrdshp_booster_34_5 } },
    { 0, { NULL, NULL, NULL, NULL, NULL, NULL } },
};

/* the cursor's animation frames */
s32 stcrdshp_booster_cursor_palettes[6] = { 0, 1, 2, 3, 2, 1 };
