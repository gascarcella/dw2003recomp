#include "common.h"
#include "object.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "overlay_common.h"

/* STCRDABM.PRO: the card album. Cards 1-314, 12 per page (6 x 2), 27 pages; a card the player owns
 * (gamestate_data.cards_obtained[card]) shows its picture, the others a blank frame. */

/* Card IDs are below this. */
#define STCRDABM_CARDS 315

/* The page of cards (stcrdabm_page_create, size 0x8C): draws the 12 cards of a page and turns
 * them over when the page changes. */
typedef struct StcrdabmPage {
    /* 0x00 */ Object base;
    /* 0x50 */ struct StcrdabmAlbum *album;  /* the album */
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 ot_depth; /* ordering table depth */
    /* 0x5C */ s32 first_card; /* first card of the page */
    /* 0x60 */ s32 prev_first_card; /* first card of the previous page */
    /* 0x64 */ s32 cards_shown; /* cards shown */
    /* 0x68 */ s32 cards_turned; /* cards turned over */
    /* 0x6C */ s32 step_time; /* time of the last step */
    /* 0x70 */ s32 flash_step; /* step of the flash */
    /* 0x74 */ u8 unk_74[0x10];
    /* 0x84 */ void (*turn)(struct StcrdabmPage *obj, s32 first); /* stcrdabm_page_turn */
    /* 0x88 */ void (*close)(struct StcrdabmPage *obj);            /* stcrdabm_page_close */
} StcrdabmPage; /* size 0x8C */

/* The album's data block (unk_24, 0x4C bytes): its text windows and its two helper objects. */
typedef struct StcrdabmAlbumData {
    /* 0x00 */ MessageWindow *windows[17]; /* text windows */
    /* 0x44 */ StcrdabmPage *page;   /* the page */
    /* 0x48 */ Fade *fade;   /* the fade */
} StcrdabmAlbumData; /* size 0x4C */

/* The album (stcrdabm_album_create, size 0xD8). */
typedef struct StcrdabmAlbum {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id; /* layer */
    /* 0x54 */ s32 ot_depth; /* ordering table depth */
    /* 0x58 */ s32 scroll; /* background scroll, 0..0x5F */
    /* 0x5C */ s32 odd_frame; /* the scroll moves every other frame */
    /* 0x60 */ s32 page;   /* page */
    /* 0x64 */ s32 pages;  /* pages */
    /* 0x68 */ s32 input_enabled; /* input enabled */
    /* 0x6C */ s32 arrows_shown; /* arrows shown (blinking) */
    /* 0x70 */ s32 blink_time; /* time of the last blink */
    /* 0x74 */ s32 cursor; /* cursor on the page, 0..11 */
    /* 0x78 */ s32 card;   /* card under the cursor */
    /* 0x7C */ s32 page_has_owned; /* the page has a card the player owns */
    /* 0x80 */ s32 owned[12];  /* the player owns the card */
    /* 0xB0 */ s32 cursor_step; /* cursor colour step */
    /* 0xB4 */ s32 cursor_time; /* time of the last colour step */
    /* 0xB8 */ WindowAnim frame_anim; /* the frame's animation */
    /* 0xC8 */ WindowAnim details_anim; /* the card details' animation */
} StcrdabmAlbum; /* size 0xD8 */

extern s32 stcrdabm_cursor_palettes[6];
extern StageUtil stcrdabm_funcs;

void stcrdabm_album_show_page_text(StcrdabmAlbum *obj, StcrdabmAlbumData *data, s32 show);
void stcrdabm_album_show_card_text(StcrdabmAlbum *obj, StcrdabmAlbumData *data, s32 show);
void stcrdabm_album_draw(StcrdabmAlbum *obj);
void stcrdabm_album_run(StcrdabmAlbum *obj, StcrdabmAlbumData *data);
void stcrdabm_album_update(StcrdabmAlbum *obj, StcrdabmAlbumData *data);

void stcrdabm_fade_start(Fade *obj, s32 dir, s32 frames) {
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

void stcrdabm_fade_draw(Fade *obj) {
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

void stcrdabm_fade_update(Fade *obj) {
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
            stcrdabm_fade_draw(obj);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Fade *stcrdabm_fade_create(void) {
    Fade *obj = object_new(stcrdabm_fade_update, sizeof(Fade), 0);

    obj->start = stcrdabm_fade_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 0;
    return obj;
}

/* Loads the pictures of the page's cards into VRAM. */
void stcrdabm_page_load_pictures(StcrdabmPage *obj) {
    CardPicture card;
    s32 col;
    s32 row;
    s32 id;

    card_init(&card);
    card.set_image_pos(0x140, 0x100);
    card.set_clut_pos(0x300, 0x100);
    id = obj->first_card;
    for (row = 0; row < 2; row++) {
        for (col = 0; col < 6 && id < STCRDABM_CARDS; col++) {
            card.select(id++);
            card.set_cell(col, row);
            card.load_image();
        }
    }
}

void stcrdabm_page_turn(StcrdabmPage *obj, s32 first) {
    s32 prev = obj->first_card;

    obj->first_card = first;
    obj->prev_first_card = prev;
    obj->cards_turned = 0;
    obj->flash_step = 0;
    obj->base.set_state(obj, OBJECT_STATE_DONE);
}

void stcrdabm_page_close(StcrdabmPage *obj) {
    obj->base.set_step(obj, 1);
}

/* Draws the page's cards (of the previous page when `prev`): the picture and two numbers, or a blank
 * frame for a card the player doesn't own. */
void stcrdabm_page_draw_cards(StcrdabmPage *obj, s32 prev) {
    Sprite spr;
    CardPicture card;
    s32 digits[5]; /* 3 used; the frame size needs 5 */
    s32 i;
    s32 id;
    s32 col;
    s32 row;
    s32 x;
    s32 y;
    s32 j;
    s32 dx;
    s32 n;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    spr.set_vram_pos(0x280, 0);
    card_init(&card);
    card.set_image_pos(0x140, 0x100);
    card.set_clut_pos(0x300, 0x100);
    card.set_layer(obj->layer_id, obj->ot_depth);
    for (i = 0; i < obj->cards_shown; i++) {
        if (prev != 0) {
            id = obj->prev_first_card + i;
        } else {
            id = obj->first_card + i;
        }
        if (id >= STCRDABM_CARDS) {
            break;
        }
        col = i % 6;
        row = i / 6;
        x = col * 42;
        y = row * 54;
        if (gamestate_data.cards_obtained[id] != 0) {
            card.select(id);
            card.set_cell(col, row);
            card.draw(x + 0x27, y + 0x34);
            if (card.get_class() != 0) {
                spr.draw(cdload_module.get_subfile_by_id(0x06050000), 0x1D, x + 0x27, y + 0x53);
            } else {
                n = card.record[1];
                j = n / 10;
                if (j != 0) {
                    digits[0] = j + 0x1E;
                } else {
                    digits[0] = 0;
                }
                j = n % 10;
                digits[1] = j + 0x1E;
                digits[2] = 0x1C;
                for (j = 0, dx = 0x27; j < 3; j++, dx += 7) {
                    if (digits[j] != 0) {
                        spr.draw(cdload_module.get_subfile_by_id(0x06050000), digits[j], x + dx, y + 0x53);
                    }
                }
                n = card.record[2];
                j = n / 10;
                if (j != 0) {
                    digits[0] = j + 0x1E;
                } else {
                    digits[0] = 0;
                }
                j = n % 10;
                digits[1] = j + 0x1E;
                for (j = 0, dx = 0x3A; j < 2; j++, dx += 7) {
                    if (digits[j] != 0) {
                        spr.draw(cdload_module.get_subfile_by_id(0x06050000), digits[j], x + dx, y + 0x53);
                    }
                }
            }
            spr.draw(cdload_module.get_subfile_by_id(0x06050000), card.record[0] - 1, x + 0x23, y + 0x32);
        } else {
            spr.draw(cdload_module.get_subfile_by_id(0x06050000), 6, x + 0x23, y + 0x32);
        }
    }
}

/* Draws the flash over the cards being turned over. */
void stcrdabm_page_draw_flash(StcrdabmPage *obj) {
    Sprite spr;
    s32 i;
    s32 id;
    s32 col;
    s32 row;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
    spr.set_vram_pos(0x280, 0);
    for (i = 0; i < obj->cards_turned; i++) {
        id = obj->first_card + i;
        col = i % 6;
        row = i / 6;
        if (gamestate_data.cards_obtained[id] != 0 || id >= STCRDABM_CARDS) {
            spr.set_palette(obj->flash_step);
        } else {
            spr.set_palette(0);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x06050000), 6, col * 42 + 0x23, row * 54 + 0x32);
    }
}

/* Returns 1 if the page has a card the player owns (or ends early). */
s32 stcrdabm_page_has_cards(StcrdabmPage *obj) {
    s32 i;
    s32 id;

    for (i = 0; i < 12; i++) {
        id = obj->first_card + i;
        if (gamestate_data.cards_obtained[id] != 0 || id >= STCRDABM_CARDS) {
            return 1;
        }
    }
    return 0;
}

void stcrdabm_page_update_close(StcrdabmPage *obj) {
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

void stcrdabm_page_update(StcrdabmPage *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->first_card = 1;
        stcrdabm_page_turn(obj, 1);
        break;
    case OBJECT_STATE_RUN:
        stcrdabm_page_update_close(obj);
        stcrdabm_page_draw_cards(obj, 0);
        break;
    case OBJECT_STATE_DONE:
        switch (obj->base.step) {
        case 0:
        default:
            if (++obj->cards_turned < 12) {
                obj->base.next_step(obj);
                obj->base.timer = gfx_module.funcs.get_time();
            } else {
                obj->cards_turned = 12;
                obj->base.step = 2;
            }
            break;
        case 1:
            if (gfx_module.funcs.get_time() - obj->base.timer >= 2) {
                obj->base.step = obj->base.substep;
            }
            break;
        case 2:
            stcrdabm_page_load_pictures(obj);
            obj->cards_shown = 12;
            obj->step_time = gfx_module.funcs.get_time();
            obj->base.next_step(obj);
            if (stcrdabm_page_has_cards(obj) != 0) {
                sound_module.play(0x4001C);
            }
            break;
        case 3:
            if (gfx_module.funcs.get_time() - obj->step_time >= 2) {
                obj->step_time = gfx_module.funcs.get_time();
                if (++obj->flash_step >= 11) {
                    obj->base.state = OBJECT_STATE_RUN;
                }
            }
            break;
        }
        stcrdabm_page_draw_flash(obj);
        if (obj->base.step < 3) {
            stcrdabm_page_draw_cards(obj, 1);
        } else {
            stcrdabm_page_draw_cards(obj, 0);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

StcrdabmPage *stcrdabm_page_create(struct StcrdabmAlbum *album) {
    StcrdabmPage *obj = object_new(stcrdabm_page_update, sizeof(StcrdabmPage), 0);

    obj->turn = stcrdabm_page_turn;
    obj->close = stcrdabm_page_close;
    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->album = album;
    return obj;
}

StcrdabmAlbum *stcrdabm_album_create(void);

/* The overlay's first object: sets up the display and creates the album. */
void stcrdabm_update_main(Object *obj, StcrdabmAlbum **data) {
    RECT rect;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0xF000);
        gfx_module.funcs.init_display(320, 240, 0, 0);
        rect.x = 0;
        rect.y = 0;
        rect.w = 320;
        rect.h = 240;
        layer = gfx_module.funcs.create_layer(&rect, 3, 0x1000);
        layer->set_bg_color(layer, 0, 0, 0);
        *data = stcrdabm_album_create();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The overlay's entry point (overlay_entries[18], called by overlay_run_object). */
Object *stcrdabm_start(void) {
    return object_new(stcrdabm_update_main, sizeof(Object), sizeof(StcrdabmAlbum *));
}

/* Creates the album's text windows. */
void stcrdabm_album_create_windows(StcrdabmAlbum *obj, StcrdabmAlbumData *data) {
    s32 i = 0;
    MessageWindow **win;

    data->windows[0] = message_create_window(obj->layer_id, 1, 0x10, 0x19);
    data->windows[1] = message_create_window(obj->layer_id, 1, 0xD0, 0x19);
    data->windows[2] = message_create_window(obj->layer_id, 1, 0x34, 0x9E);
    data->windows[3] = message_create_window(obj->layer_id, 1, 0x35, 0x9E);
    data->windows[4] = message_create_window(obj->layer_id, 1, 0x4A, 0x9E);
    data->windows[5] = message_create_window(obj->layer_id, 1, 0x12, 0x6C);
    data->windows[6] = message_create_window(obj->layer_id, 1, 0x121, 0x6C);
    data->windows[7] = message_create_window(obj->layer_id, 1, 0x88, 0xA1);
    data->windows[8] = message_create_window(obj->layer_id, 1, 0x115, 0xA1);
    data->windows[9] = message_create_window(obj->layer_id, 1, 0x126, 0xA1);
    data->windows[10] = message_create_window(obj->layer_id, 1, 0x115, 0xC7);
    data->windows[11] = message_create_window(obj->layer_id, 1, 0x12C, 0xC7);
    data->windows[12] = message_create_window(obj->layer_id, 1, 0x50, 0xB8);
    data->windows[13] = message_create_window(obj->layer_id, 1, 0xCE, 0xB8);
    data->windows[14] = message_create_window(obj->layer_id, 1, 0xF0, 0xB8);
    data->windows[15] = message_create_window(obj->layer_id, 1, 0xCE, 0xC5);
    data->windows[16] = message_create_window(obj->layer_id, 1, 0xF0, 0xC5);
    win = (MessageWindow **)obj->base.children;
    while (i < obj->base.child_count - 2) {
        i++;
        (*win)->set_ot_depth(*win, obj->ot_depth - 3);
        win++;
    }
}

/* Shows (`show`) or hides the page's texts: the titles, "page n / m" and the arrows' labels. */
void stcrdabm_album_show_page_text(StcrdabmAlbum *obj, StcrdabmAlbumData *data, s32 show) {
    if (show) {
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x24), 1);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x24), 2);
        data->windows[2]->set_line_number(data->windows[2], 0, obj->page + 1);
        data->windows[2]->measure(data->windows[2], 1);
        data->windows[3]->set_text(data->windows[3], cdload_module.files.get_file(records_language + 0x24), 5);
        data->windows[4]->set_line_number(data->windows[4], 0, obj->pages);
        data->windows[4]->measure(data->windows[4], 1);
        if (obj->input_enabled != 0) {
            if (obj->page > 0) {
                data->windows[5]->set_text(data->windows[5], cdload_module.files.get_file(records_language + 0x24), 3);
            } else {
                data->windows[5]->set_visible(data->windows[5], 0);
            }
            if (obj->page < obj->pages - 1) {
                data->windows[6]->set_text(data->windows[6], cdload_module.files.get_file(records_language + 0x24), 4);
            } else {
                data->windows[6]->set_visible(data->windows[6], 0);
            }
        }
    } else {
        data->windows[0]->set_visible(data->windows[0], 0);
        data->windows[1]->set_visible(data->windows[1], 0);
        data->windows[2]->set_visible(data->windows[2], 0);
        data->windows[3]->set_visible(data->windows[3], 0);
        data->windows[4]->set_visible(data->windows[4], 0);
        data->windows[5]->set_visible(data->windows[5], 0);
        data->windows[6]->set_visible(data->windows[6], 0);
    }
}

/* Shows (`show`, and the player owns it) or hides the details of the card under the cursor. */
void stcrdabm_album_show_card_text(StcrdabmAlbum *obj, StcrdabmAlbumData *data, s32 show) {
    CardPicture card;

    obj->card = obj->page * 12 + obj->cursor + 1;
    if (gamestate_data.cards_obtained[obj->card] != 0 && show) {
        card_init(&card);
        card.select(obj->card);
        data->windows[7]->set_text(data->windows[7], cdload_module.files.get_file(records_language + 0x16), obj->card);
        data->windows[10]->set_text(data->windows[10], cdload_module.files.get_file(records_language + 0x24), 8);
        data->windows[11]->set_line_number(data->windows[11], 0, gamestate_data.cards[obj->card]);
        data->windows[11]->measure(data->windows[11], 1);
        if (card.get_class() != 0) {
            data->windows[8]->set_visible(data->windows[8], 0);
            data->windows[9]->set_visible(data->windows[9], 0);
            data->windows[12]->set_text(data->windows[12], cdload_module.files.get_file(records_language + 0x1D), obj->card);
            data->windows[13]->set_visible(data->windows[13], 0);
            data->windows[14]->set_visible(data->windows[14], 0);
            data->windows[15]->set_visible(data->windows[15], 0);
            data->windows[16]->set_visible(data->windows[16], 0);
        } else {
            data->windows[8]->set_text(data->windows[8], cdload_module.files.get_file(records_language + 0x24), 8);
            data->windows[9]->set_line_number(data->windows[9], 0, card.record[5]);
            data->windows[9]->measure(data->windows[9], 1);
            if (obj->card == 0x45 || obj->card == 0x70 || obj->card == 0x9B || obj->card == 0xC6 ||
                obj->card == 0xF1) {
                data->windows[12]->set_text(data->windows[12], cdload_module.files.get_file(records_language + 0x1D), obj->card);
                data->windows[13]->set_visible(data->windows[13], 0);
                data->windows[14]->set_visible(data->windows[14], 0);
                data->windows[15]->set_visible(data->windows[15], 0);
                data->windows[16]->set_visible(data->windows[16], 0);
            } else {
                data->windows[12]->set_visible(data->windows[12], 0);
                data->windows[13]->set_text(data->windows[13], cdload_module.files.get_file(records_language + 0x24), 6);
                data->windows[14]->set_line_number(data->windows[14], 0, card.record[1]);
                data->windows[14]->measure(data->windows[14], 1);
                data->windows[15]->set_text(data->windows[15], cdload_module.files.get_file(records_language + 0x24), 7);
                data->windows[16]->set_line_number(data->windows[16], 0, card.record[2]);
                data->windows[16]->measure(data->windows[16], 1);
            }
        }
    } else {
        data->windows[7]->set_visible(data->windows[7], 0);
        data->windows[10]->set_visible(data->windows[10], 0);
        data->windows[11]->set_visible(data->windows[11], 0);
        data->windows[8]->set_visible(data->windows[8], 0);
        data->windows[9]->set_visible(data->windows[9], 0);
        data->windows[12]->set_visible(data->windows[12], 0);
        data->windows[13]->set_visible(data->windows[13], 0);
        data->windows[14]->set_visible(data->windows[14], 0);
        data->windows[15]->set_visible(data->windows[15], 0);
        data->windows[16]->set_visible(data->windows[16], 0);
    }
}

/* Draws the album: the scrolling background, the frame (scaled by its window animation), the page arrows,
 * the cursor and the card details' frame. */
void stcrdabm_album_draw(StcrdabmAlbum *obj) {
    Sprite spr;
    CardPicture card;
    s32 type;
    s32 id;

    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->odd_frame != 0) {
        obj->scroll++;
        obj->scroll = obj->scroll < 0x60 ? obj->scroll : 0;
        obj->odd_frame = 0;
    } else {
        obj->odd_frame = 1;
    }
    spr.draw(cdload_module.get_subfile_by_id(0x06050000), 8, obj->scroll, obj->scroll);
    spr.set_layer_id(obj->layer_id, obj->ot_depth - 2);
    if (obj->frame_anim.level != 0) {
        if (obj->frame_anim.level != 0x1000) {
            spr.set_scale(obj->frame_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0, 0x20);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x06050000), 9, 0, 0x15);
        if (obj->frame_anim.level != 0x1000) {
            spr.set_pivot(0x140, 0x20);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x06050000), 0xF, 0xC8, 0x15);
        if (obj->frame_anim.level != 0x1000) {
            spr.set_scale(obj->frame_anim.level, obj->frame_anim.level, 0x1000);
            spr.set_pivot(0x3A, 0xA5);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x06050000), 0xE, 0x22, 0x9A);
    }
    if (obj->input_enabled != 0) {
        if (obj->pages >= 2) {
            if (gfx_module.funcs.get_time() - obj->blink_time > 16) {
                obj->blink_time = gfx_module.funcs.get_time();
                obj->arrows_shown = 1 - obj->arrows_shown;
            }
            if (obj->arrows_shown != 0) {
                if (obj->page > 0) {
                    spr.draw(cdload_module.get_subfile_by_id(0x06050000), 0x1A, 0xE, 0x5A);
                }
                if (obj->page < obj->pages - 1) {
                    spr.draw(cdload_module.get_subfile_by_id(0x06050000), 0x1B, 0x121, 0x5A);
                }
            }
        }
        if (obj->page_has_owned != 0) {
            if (gfx_module.funcs.get_time() - obj->cursor_time > 16) {
                obj->cursor_time = gfx_module.funcs.get_time();
                if (++obj->cursor_step >= 6) {
                    obj->cursor_step = 0;
                }
            }
            spr.set_palette(stcrdabm_cursor_palettes[obj->cursor_step]);
            spr.draw(cdload_module.get_subfile_by_id(0x06050000), 7, obj->cursor % 6 * 42 + 0x24, obj->cursor / 6 * 54 + 0x32);
            spr.set_palette(0);
        }
    }
    if (obj->details_anim.level != 0) {
        card_init(&card);
        card.select(obj->card);
        if (obj->details_anim.level != 0x1000) {
            spr.set_scale(obj->details_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0xA8);
        }
        type = card.get_class();
        if (type == 1) {
            id = 0x12;
        } else if (type == 2) {
            id = 0x13;
        } else {
            id = card.record[0] + 0x13;
        }
        spr.draw(cdload_module.get_subfile_by_id(0x06050000), id, 0x103, 0x9F);
        spr.draw(cdload_module.get_subfile_by_id(0x06050000), 0xD, 0xFC, 0x9D);
        if (obj->details_anim.level != 0x1000) {
            spr.set_pivot(0x140, 0xA8);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x06050000), 0xA, 0x82, 0x9D);
        if (obj->details_anim.level != 0x1000) {
            spr.set_pivot(0x140, 0xD0);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x06050000), 0x10, 0x103, 0xC5);
        spr.draw(cdload_module.get_subfile_by_id(0x06050000), 0xD, 0xFC, 0xC3);
        if (obj->details_anim.level != 0x1000) {
            spr.set_pivot(0x140, 0xC6);
        }
        if (type != 0) {
            spr.draw(cdload_module.get_subfile_by_id(0x06050000), 0xB, 0x4A, 0xB3);
        } else if (obj->card == 0x45 || obj->card == 0x70 || obj->card == 0x9B || obj->card == 0xC6 ||
                   obj->card == 0xF1) {
            spr.draw(cdload_module.get_subfile_by_id(0x06050000), 0xB, 0x4A, 0xB3);
        } else {
            spr.draw(cdload_module.get_subfile_by_id(0x06050000), 0xC, 0xC7, 0xB3);
        }
    }
}

/* Marks which cards of the page the player owns. */
void stcrdabm_album_mark_owned(StcrdabmAlbum *obj) {
    s32 i;
    s32 id;

    obj->page_has_owned = 0;
    id = obj->page * 12;
    for (i = 0; i < 12; i++) {
        id++;
        if (id < STCRDABM_CARDS && gamestate_data.cards_obtained[id] != 0) {
            obj->owned[i] = 1;
            obj->page_has_owned = 1;
        } else {
            obj->owned[i] = 0;
        }
    }
}

/* The album's input and its steps (base.step): open the frame, show a page, move the cursor or turn the
 * page, and leave with a fade. */
void stcrdabm_album_run(StcrdabmAlbum *obj, StcrdabmAlbumData *data) {
    s32 old;
    s32 i;
    Fade *fade;

    switch (obj->base.step) {
    case 0:
    default:
        stcrdabm_funcs.window_anim_start(&obj->frame_anim, 1);
        stcrdabm_album_mark_owned(obj);
        data->page = stcrdabm_page_create(obj);
        obj->base.step++;
        break;
    case 1:
        if (stcrdabm_funcs.window_anim_update(&obj->frame_anim)) {
            stcrdabm_album_show_page_text(obj, data, 1);
            if (obj->page_has_owned != 0) {
                stcrdabm_funcs.window_anim_start(&obj->details_anim, 1);
            }
            obj->base.step = 11;
        }
        break;
    case 3:
        obj->input_enabled = 1;
        obj->base.step++;
        break;
    case 4:
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
            sound_module.play(0x4001B);
            obj->input_enabled = 0;
            obj->cursor = 0;
            stcrdabm_album_mark_owned(obj);
            stcrdabm_album_show_page_text(obj, data, 1);
            if (obj->details_anim.level == 0) {
                if (obj->page_has_owned != 0) {
                    obj->base.step = 10;
                    obj->base.substep = 1;
                    data->page->turn(data->page, obj->page * 12 + 1);
                } else {
                    obj->base.step = 15;
                }
            } else {
                data->page->turn(data->page, obj->page * 12 + 1);
                if (obj->page_has_owned == 0) {
                    obj->base.step = 10;
                    obj->base.substep = 0;
                } else {
                    obj->base.step = 15;
                }
            }
        } else {
            old = obj->cursor;
            if (PAD_PRESSED(4)) {
                if (obj->cursor >= 6) {
                    obj->cursor -= 6;
                }
            } else if (PAD_PRESSED(6)) {
                if (obj->cursor < 6) {
                    obj->cursor += 6;
                }
            }
            if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
                if (--obj->cursor < 0) {
                    obj->cursor = 0;
                }
            } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
                if (++obj->cursor >= 12) {
                    obj->cursor = 11;
                }
            }
            if (obj->page * 12 + obj->cursor >= 0x13A) {
                obj->cursor = 0x139 - obj->page * 12;
            }
            if (old != obj->cursor) {
                i = obj->cursor;
                obj->cursor = -1;
                if (i < old) {
                    for (; i >= 0; i--) {
                        if (obj->owned[i] != 0) {
                            obj->cursor = i;
                            break;
                        }
                    }
                } else if (old < i) {
                    for (; i < 12; i++) {
                        if (obj->owned[i] != 0) {
                            obj->cursor = i;
                            break;
                        }
                    }
                }
                if (obj->cursor == -1) {
                    obj->cursor = old;
                } else {
                    stcrdabm_album_show_card_text(obj, data, 1);
                    sound_module.play(0x4001B);
                }
            }
        }
        if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->input_enabled = 0;
            obj->base.step = 0x32;
            fade = stcrdabm_fade_create();
            data->fade = fade;
            fade->start(fade, 0, 10);
        }
        break;
    case 10:
        if (obj->base.substep == 0) {
            stcrdabm_album_show_card_text(obj, data, 0);
        }
        stcrdabm_funcs.window_anim_start(&obj->details_anim, obj->base.substep);
        obj->base.next_step(obj);
        break;
    case 11:
        if (stcrdabm_funcs.window_anim_update(&obj->details_anim)) {
            obj->base.step = 15;
        }
        break;
    case 15:
        if (data->page->base.state == OBJECT_STATE_RUN) {
            obj->input_enabled = 1;
            stcrdabm_album_show_page_text(obj, data, 1);
            if (obj->details_anim.level != 0) {
                while (1) {
                    if (obj->owned[obj->cursor] != 0) {
                        break;
                    }
                    obj->cursor++;
                }
                stcrdabm_album_show_card_text(obj, data, 1);
            }
            obj->base.step = 3;
        }
        break;
    case 0x32:
        if (data->fade->base.state == OBJECT_STATE_DONE) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 0x33:
        if (stcrdabm_funcs.window_anim_update(&obj->details_anim)) {
            stcrdabm_album_show_page_text(obj, data, 0);
            stcrdabm_funcs.window_anim_start(&obj->frame_anim, 0);
            obj->base.step++;
        }
        break;
    case 0x34:
        if (stcrdabm_funcs.window_anim_update(&obj->frame_anim)) {
            obj->base.step++;
        }
        break;
    case 0x35:
        if (data->page == NULL) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    }
}

void stcrdabm_album_update(StcrdabmAlbum *obj, StcrdabmAlbumData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            stcrdabm_funcs.load_files();
            obj->base.step++;
            break;
        case 1:
            if (stcrdabm_funcs.is_loading() == 0) {
                stcrdabm_album_create_windows(obj, data);
                obj->frame_anim.duration = 8;
                obj->details_anim.duration = 8;
                obj->pages = 27;
                obj->card = 1;
                obj->base.next_state(obj);
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        stcrdabm_album_run(obj, data);
        stcrdabm_album_draw(obj);
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        gamestate_data.funcs.set_next_map(gamestate_data.funcs.get_prev_map(), 0);
        break;
    }
}

StcrdabmAlbum *stcrdabm_album_create(void) {
    StcrdabmAlbum *obj = object_new(stcrdabm_album_update, sizeof(StcrdabmAlbum), sizeof(StcrdabmAlbumData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 7;
    return obj;
}

/* Starts loading the album's files: its sprites, the card pictures and the texts. */
void stcrdabm_load_files(void) {
    Tim tim;

    tim_init(&tim);
    tim.set_image_pos(0x280, 0);
    tim.load_all(cdload_module.get_subfile_by_id(0x06060000));
    cdload_module.queue_file(0x7F6);
    cdload_module.queue_file(0x7F7);
    cdload_module.queue_file(0x7F8);
    cdload_module.queue_file(0x7F9);
    cdload_module.queue_file(0x7FA);
    cdload_module.queue_file(records_language + 0x16);
    cdload_module.queue_file(records_language + 0x1D);
    cdload_module.queue_file(records_language + 0x24);
}

/* Returns non-zero while one of the album's files is still loading. */
s32 stcrdabm_is_loading(void) {
    if (!cdload_module.is_loading(0x7F6) && !cdload_module.is_loading(0x7F7) && !cdload_module.is_loading(0x7F8) &&
        !cdload_module.is_loading(0x7F9) && !cdload_module.is_loading(0x7FA) && !cdload_module.is_loading(records_language + 0x16) &&
        !cdload_module.is_loading(records_language + 0x1D)) {
        return cdload_module.is_loading(records_language + 0x24) != 0;
    }
    return 1;
}

/* window_anim_start (include/window_anim.h), byte for byte. */
void stcrdabm_window_anim_start(WindowAnim *anim, s32 open) {
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

/* window_anim_update (include/window_anim.h), byte for byte. */
s32 stcrdabm_window_anim_update(WindowAnim *anim) {
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

void stcrdabm_tween_start(Tween *obj, s32 from, s32 to, s32 frames) {
    if (from != to) {
        obj->duration = frames;
        obj->acc = from << 8;
        obj->value = from;
        obj->target = to;
        obj->running = 1;
        obj->step = ((to - from) << 8) / obj->duration;
    }
}

s32 stcrdabm_tween_update(Tween *obj) {
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

s32 stcrdabm_cursor_palettes[6] = { 0, 1, 2, 3, 2, 1 };

StageUtil stcrdabm_funcs = {
    stcrdabm_load_files,
    stcrdabm_is_loading,
    stcrdabm_window_anim_start,
    stcrdabm_window_anim_update,
    stcrdabm_tween_start,
    stcrdabm_tween_update,
};
