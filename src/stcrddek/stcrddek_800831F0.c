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
#include "stcrddek.h"

/* STCRDDEK.PRO's first file: the root object and the deck editor (the card list on the left, the deck's
 * cards in a grid). */

/* The deck editor (stcrddek_update_editor). */
struct StcrddekEditor {
    /* 0x000 */ Object base;
    /* 0x050 */ StcrddekMain *main;   /* the main object */
    /* 0x054 */ s32 layer_id; /* layer */
    /* 0x058 */ s32 ot_depth; /* ordering table entry */
    /* 0x05C */ s32 deck;   /* deck: index of gamestate_data.decks */
    /* 0x060 */ s32 grid_column; /* grid cursor column */
    /* 0x064 */ s32 grid_row; /* grid cursor row */
    /* 0x068 */ s32 grid_frame; /* grid cursor animation frame, 0..5 */
    /* 0x06C */ s32 frame_time; /* time of the last frame */
    /* 0x070 */ s32 grid_cursor_shown; /* grid cursor shown */
    /* 0x074 */ s32 list_cursor; /* list cursor (line on screen) */
    /* 0x078 */ s32 first_line; /* first line of the card list shown */
    /* 0x07C */ s32 spare_count; /* cards in spare */
    /* 0x080 */ s32 arrows_shown; /* scroll arrows blink */
    /* 0x084 */ s32 blink_time; /* time of the last blink */
    /* 0x088 */ s16 spare[315];  /* the cards the player has spare, in ID order */
    /* 0x2FE */ s8 spare_copies[315]; /* per card ID: copies not in the deck */
    /* 0x439 */ u8 unk_439[0x3];
    /* 0x43C */ s32 details;
    /* 0x440 */ s32 hint_time;
    /* 0x444 */ WindowAnim anims[5];
}; /* size 0x494 */

/* A line of the card list: the card's name, its type and the copies left. */
typedef struct StcrddekEditorLine {
    /* 0x0 */ MessageWindow *name;
    /* 0x4 */ MessageWindow *times;
    /* 0x8 */ MessageWindow *copies;
} StcrddekEditorLine; /* size 0xC */

/* The deck editor's children (its data block). */
typedef struct StcrddekEditorData {
    /* 0x00 */ MessageWindow *back_hint;
    /* 0x04 */ MessageWindow *hint;
    /* 0x08 */ MessageWindow *deck_name;
    /* 0x0C */ MessageWindow *type_counts[6];
    /* 0x24 */ MessageWindow *deck_card_name;
    /* 0x28 */ MessageWindow *deck_card_level_label;
    /* 0x2C */ MessageWindow *deck_card_level;
    /* 0x30 */ MessageWindow *deck_card_ap_label;
    /* 0x34 */ MessageWindow *deck_card_ap;
    /* 0x38 */ MessageWindow *deck_card_hp_label;
    /* 0x3C */ MessageWindow *deck_card_hp;
    /* 0x40 */ MessageWindow *deck_card_text;
    /* 0x44 */ MessageWindow *list_card_name;
    /* 0x48 */ MessageWindow *list_card_level_label;
    /* 0x4C */ MessageWindow *list_card_level;
    /* 0x50 */ MessageWindow *list_card_ap_label;
    /* 0x54 */ MessageWindow *list_card_ap;
    /* 0x58 */ MessageWindow *list_card_hp_label;
    /* 0x5C */ MessageWindow *list_card_hp;
    /* 0x60 */ MessageWindow *list_card_text;
    /* 0x64 */ StcrddekEditorLine lines[8];
    /* 0xC4 */ MessageCursor *list_cursor;
    /* 0xC8 */ MessageWindow *warning;
    /* 0xCC */ Object *grid;   /* the grid */
    /* 0xD0 */ StcrddekBar *scroll_bar; /* scroll bar of the card list */
} StcrddekEditorData; /* size 0xD4 */

/* The deck's cards drawn in a 9-column grid (stcrddek_update_grid); cards_shown counts up to 40 while they
 * appear one by one. */
typedef struct StcrddekGrid {
    /* 0x00 */ Object base;
    /* 0x50 */ StcrddekEditor *editor;
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 ot_depth; /* ordering table entry */
    /* 0x5C */ s32 cards_shown; /* cards shown */
    /* 0x60 */ u8 unk_60[0x10];
} StcrddekGrid; /* size 0x70 */

/* An object skeleton: empty init and draw steps, ends on button 14; stcrddek_stub_create is never called (STGTRAIN
 * has the same skeleton, stgtrain_stub_update). */
typedef struct StcrddekStub {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 unk_50;
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 ot_depth; /* ordering table entry */
    /* 0x5C */ u8 unk_5C[0x10];
} StcrddekStub; /* size 0x6C */

void stcrddek_update_fade(Fade *obj);
void stcrddek_start_fade(Fade *obj, s32 out, s32 frames);
void stcrddek_update_grid(StcrddekGrid *obj);
void stcrddek_update_editor(StcrddekEditor *obj, StcrddekEditorData *data);
void stcrddek_create_editor_windows(StcrddekEditor *obj, StcrddekEditorData *data);
void stcrddek_draw_editor(StcrddekEditor *obj);
void stcrddek_update_editor_menu(StcrddekEditor *obj, StcrddekEditorData *data);
void stcrddek_stub_update(StcrddekStub *obj, void *data);

/* The overlay's root object: sets up the display and creates the main object. */
void stcrddek_update_root(Object *obj, StcrddekMain **data) {
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
        *data = stcrddek_create_main();
        obj->next_state(obj);
        break;
    }
}

/* The overlay's entry. */
Object *stcrddek_start(void) {
    return object_new(stcrddek_update_root, sizeof(Object), 4);
}

/* Starts a fade in (out == 0) or out over `frames` frames. */
void stcrddek_start_fade(Fade *obj, s32 out, s32 frames) {
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
void stcrddek_draw_fade(Fade *obj) {
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

void stcrddek_update_fade(Fade *obj) {
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
        stcrddek_draw_fade(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Fade *stcrddek_create_fade(void) {
    Fade *obj = object_new(stcrddek_update_fade, sizeof(Fade), 0);

    obj->start = stcrddek_start_fade;
    obj->layer_id = 0x1000;
    obj->ot_depth = 0;
    return obj;
}

/* Draws the deck's first cards_shown cards: frame (image 0x063E) and picture, 9 per row. */
void stcrddek_draw_grid(StcrddekGrid *obj) {
    Sprite spr;
    CardPicture card;
    s32 i;
    s32 col;
    s32 row;

    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    card_init(&card);
    card.set_image_pos(0x140, 0x100);
    card.set_clut_pos(0x300, 0x100);
    card.set_layer(obj->layer_id, obj->ot_depth);
    for (i = 0; i < obj->cards_shown; i++) {
        card.select((s16)gamestate_data.decks[obj->editor->deck].cards[i]);
        col = i % 9;
        row = i / 9;
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), card.record[0] + 0x52, col * 32 + 16, row * 32 + 58);
        card.set_cell(col, row);
        card.draw(col * 32 + 16, row * 32 + 58);
    }
}

/* Shows one more card (uploads its picture); state 2 once all 40 are shown. */
void stcrddek_step_grid(StcrddekGrid *obj) {
    CardPicture card;

    obj->cards_shown++;
    if (obj->cards_shown > 40) {
        obj->base.state = OBJECT_STATE_DONE;
        obj->cards_shown = 40;
        return;
    }
    card_init(&card);
    card.select((s16)gamestate_data.decks[obj->editor->deck].cards[obj->cards_shown - 1]);
    card.set_image_pos(0x140, 0x100);
    card.set_clut_pos(0x300, 0x100);
    card.set_cell((obj->cards_shown - 1) % 9, (obj->cards_shown - 1) / 9);
    card.load_image();
}

void stcrddek_update_grid(StcrddekGrid *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->cards_shown = 0;
        break;
    case OBJECT_STATE_RUN:
        stcrddek_step_grid(obj);
        /* fallthrough */
    case OBJECT_STATE_DONE:
        stcrddek_draw_grid(obj);
        /* fallthrough */
    case OBJECT_STATE_END:
        break;
    }
}

StcrddekGrid *stcrddek_create_grid(StcrddekEditor *editor) {
    StcrddekGrid *obj = object_new(stcrddek_update_grid, sizeof(StcrddekGrid), 0);

    obj->layer_id = 0x1000;
    obj->ot_depth = 5;
    obj->editor = editor;
    return obj;
}

void stcrddek_create_editor_windows(StcrddekEditor *obj, StcrddekEditorData *data) {
    s32 i;

    data->back_hint = message_create_window(obj->layer_id, 1, 0xC1, 0x17);
    data->back_hint->set_ot_depth(data->back_hint, obj->ot_depth - 1);
    data->deck_name = message_create_window(obj->layer_id, 1, 0x15, 0x18);
    data->deck_name->set_ot_depth(data->deck_name, obj->ot_depth - 1);
    for (i = 0; i < 6; i++) {
        data->type_counts[i] = message_create_window(obj->layer_id, 1, i * 0x23 + 0x34, 0x29);
        data->type_counts[i]->set_ot_depth(data->type_counts[i], obj->ot_depth - 1);
    }
    data->hint = message_create_window(obj->layer_id, 1, 0x96, 0xCC);
    data->deck_card_name = message_create_window(obj->layer_id, 1, 0x96, 0xBD);
    data->deck_card_level_label = message_create_window(obj->layer_id, 1, 0x115, 0x26);
    data->deck_card_level = message_create_window(obj->layer_id, 1, 0x126, 0x26);
    data->deck_card_text = message_create_window(obj->layer_id, 1, 0x50, 0x17);
    data->deck_card_ap_label = message_create_window(obj->layer_id, 1, 0xCE, 0x17);
    data->deck_card_ap = message_create_window(obj->layer_id, 1, 0xF0, 0x17);
    data->deck_card_hp_label = message_create_window(obj->layer_id, 1, 0xCE, 0x24);
    data->deck_card_hp = message_create_window(obj->layer_id, 1, 0xF0, 0x24);
    for (i = 0; i < 8; i++) {
        data->lines[i].name = message_create_window(obj->layer_id, 1, 0xA3, i * 14 + 0x27);
        data->lines[i].name->set_ot_depth(data->lines[i].name, obj->ot_depth - 1);
        data->lines[i].times = message_create_window(obj->layer_id, 1, 0x111, i * 14 + 0x27);
        data->lines[i].times->set_ot_depth(data->lines[i].times, obj->ot_depth - 1);
        data->lines[i].copies = message_create_window(obj->layer_id, 1, 0x120, i * 14 + 0x27);
        data->lines[i].copies->set_ot_depth(data->lines[i].copies, obj->ot_depth - 1);
    }
    data->list_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0x89, 0x27);
    data->list_cursor->show(data->list_cursor, 0);
    data->list_card_name = message_create_window(obj->layer_id, 1, 0x88, 0xA1);
    data->list_card_level_label = message_create_window(obj->layer_id, 1, 0x115, 0xA1);
    data->list_card_level = message_create_window(obj->layer_id, 1, 0x126, 0xA1);
    data->list_card_text = message_create_window(obj->layer_id, 1, 0x50, 0xB8);
    data->list_card_ap_label = message_create_window(obj->layer_id, 1, 0xCE, 0xB8);
    data->list_card_ap = message_create_window(obj->layer_id, 1, 0xF0, 0xB8);
    data->list_card_hp_label = message_create_window(obj->layer_id, 1, 0xCE, 0xC5);
    data->list_card_hp = message_create_window(obj->layer_id, 1, 0xF0, 0xC5);
    data->warning = message_create_window(obj->layer_id, 1, 0x3E, 0x6B);
    data->warning->set_page_lines(data->warning, 2);
}

/* Shows (or hides) the deck's counts per type and the card under the grid cursor: its name and, unless
 * it is special (unk_50() != 0), its level and two values; the five cards 0x45, 0x70, 0x9B, 0xC6, 0xF1
 * show a text instead of the values. */
void stcrddek_show_deck_card(StcrddekEditor *obj, StcrddekEditorData *data, s32 show) {
    CardPicture card;
    s32 i;
    s32 msg;
    s32 id;

    if (show) {
        data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0x32), 4);
        data->deck_name->set_text(data->deck_name, gamestate_data.decks[obj->deck].name, -1);
        for (i = 0; i < 6; i++) {
            data->type_counts[i]->set_line_number(data->type_counts[i], 0, obj->main->type_counts[obj->deck][i]);
            data->type_counts[i]->measure(data->type_counts[i], 1);
        }
        msg = 0x1F;
        if (obj->hint_time < 60) {
            msg = obj->details + 0x1C;
        }
        data->hint->set_text(data->hint, cdload_module.files.get_file(records_language + 0x32), msg);
        id = (s16)gamestate_data.decks[obj->deck].cards[obj->grid_column + obj->grid_row * 9];
        data->deck_card_name->set_text(data->deck_card_name, cdload_module.files.get_file(records_language + 0x16), id);
        if (obj->details != 0) {
            card_init(&card);
            card.select(id);
            if (card.get_class() != 0) {
                data->deck_card_level_label->set_visible(data->deck_card_level_label, 0);
                data->deck_card_level->set_visible(data->deck_card_level, 0);
                data->deck_card_text->set_text(data->deck_card_text, cdload_module.files.get_file(records_language + 0x1D), id);
                data->deck_card_ap_label->set_visible(data->deck_card_ap_label, 0);
                data->deck_card_ap->set_visible(data->deck_card_ap, 0);
                data->deck_card_hp_label->set_visible(data->deck_card_hp_label, 0);
                data->deck_card_hp->set_visible(data->deck_card_hp, 0);
            } else {
                data->deck_card_level_label->set_text(data->deck_card_level_label, cdload_module.files.get_file(records_language + 0x32), 8);
                data->deck_card_level->set_line_number(data->deck_card_level, 0, card.record[5]);
                data->deck_card_level->measure(data->deck_card_level, 1);
                if (id == 0x45 || id == 0x70 || id == 0x9B || id == 0xC6 || id == 0xF1) {
                    data->deck_card_text->set_text(data->deck_card_text, cdload_module.files.get_file(records_language + 0x1D), id);
                    data->deck_card_ap_label->set_visible(data->deck_card_ap_label, 0);
                    data->deck_card_ap->set_visible(data->deck_card_ap, 0);
                    data->deck_card_hp_label->set_visible(data->deck_card_hp_label, 0);
                    data->deck_card_hp->set_visible(data->deck_card_hp, 0);
                } else {
                    data->deck_card_text->set_visible(data->deck_card_text, 0);
                    data->deck_card_ap_label->set_text(data->deck_card_ap_label, cdload_module.files.get_file(records_language + 0x32), 0x11);
                    data->deck_card_ap->set_line_number(data->deck_card_ap, 0, card.record[1]);
                    data->deck_card_ap->measure(data->deck_card_ap, 1);
                    data->deck_card_hp_label->set_text(data->deck_card_hp_label, cdload_module.files.get_file(records_language + 0x32), 0x12);
                    data->deck_card_hp->set_line_number(data->deck_card_hp, 0, card.record[2]);
                    data->deck_card_hp->measure(data->deck_card_hp, 1);
                }
            }
        } else {
            data->deck_card_level_label->set_visible(data->deck_card_level_label, 0);
            data->deck_card_level->set_visible(data->deck_card_level, 0);
            data->deck_card_text->set_visible(data->deck_card_text, 0);
            data->deck_card_ap_label->set_visible(data->deck_card_ap_label, 0);
            data->deck_card_ap->set_visible(data->deck_card_ap, 0);
            data->deck_card_hp_label->set_visible(data->deck_card_hp_label, 0);
            data->deck_card_hp->set_visible(data->deck_card_hp, 0);
        }
    } else {
        data->back_hint->set_visible(data->back_hint, 0);
        data->deck_name->set_visible(data->deck_name, 0);
        for (i = 0; i < 6; i++) {
            data->type_counts[i]->set_visible(data->type_counts[i], 0);
        }
        data->hint->set_visible(data->hint, 0);
        data->deck_card_name->set_visible(data->deck_card_name, 0);
        data->deck_card_level_label->set_visible(data->deck_card_level_label, 0);
        data->deck_card_level->set_visible(data->deck_card_level, 0);
        data->deck_card_text->set_visible(data->deck_card_text, 0);
        data->deck_card_ap_label->set_visible(data->deck_card_ap_label, 0);
        data->deck_card_ap->set_visible(data->deck_card_ap, 0);
        data->deck_card_hp_label->set_visible(data->deck_card_hp_label, 0);
        data->deck_card_hp->set_visible(data->deck_card_hp, 0);
    }
}

/* Fills (or clears) the 8 lines of the card list from line first_line. */
void stcrddek_show_card_list(StcrddekEditor *obj, StcrddekEditorData *data, s32 show) {
    s32 i;
    s16 card;

    if (show) {
        for (i = 0; i < 8; i++) {
            card = obj->spare[obj->first_line + i];
            if (card != 0) {
                data->lines[i].name->set_text(data->lines[i].name, cdload_module.files.get_file(records_language + 0x16), card);
                data->lines[i].times->set_text(data->lines[i].times, cdload_module.files.get_file(records_language + 0x32), 8);
                data->lines[i].copies->set_line_number(data->lines[i].copies, 0, obj->spare_copies[card]);
                data->lines[i].copies->measure(data->lines[i].copies, 1);
            } else {
                data->lines[i].name->set_visible(data->lines[i].name, 0);
                data->lines[i].times->set_visible(data->lines[i].times, 0);
                data->lines[i].copies->set_visible(data->lines[i].copies, 0);
            }
        }
    } else {
        for (i = 0; i < 8; i++) {
            data->lines[i].name->set_visible(data->lines[i].name, 0);
            data->lines[i].times->set_visible(data->lines[i].times, 0);
            data->lines[i].copies->set_visible(data->lines[i].copies, 0);
        }
    }
}

/* Shows (or hides) the card under the list cursor, as stcrddek_show_deck_card. */
void stcrddek_show_list_card(StcrddekEditor *obj, StcrddekEditorData *data, s32 show) {
    CardPicture card;
    s32 id;

    if (show) {
        id = obj->spare[obj->first_line + obj->list_cursor];
        data->list_card_name->set_text(data->list_card_name, cdload_module.files.get_file(records_language + 0x16), id);
        card_init(&card);
        card.select(id);
        if (card.get_class() != 0) {
            data->list_card_level_label->set_visible(data->list_card_level_label, 0);
            data->list_card_level->set_visible(data->list_card_level, 0);
            data->list_card_text->set_text(data->list_card_text, cdload_module.files.get_file(records_language + 0x1D), id);
            data->list_card_ap_label->set_visible(data->list_card_ap_label, 0);
            data->list_card_ap->set_visible(data->list_card_ap, 0);
            data->list_card_hp_label->set_visible(data->list_card_hp_label, 0);
            data->list_card_hp->set_visible(data->list_card_hp, 0);
        } else {
            data->list_card_level_label->set_text(data->list_card_level_label, cdload_module.files.get_file(records_language + 0x32), 8);
            data->list_card_level->set_line_number(data->list_card_level, 0, card.record[5]);
            data->list_card_level->measure(data->list_card_level, 1);
            if (id == 0x45 || id == 0x70 || id == 0x9B || id == 0xC6 || id == 0xF1) {
                data->list_card_text->set_text(data->list_card_text, cdload_module.files.get_file(records_language + 0x1D), id);
                data->list_card_ap_label->set_visible(data->list_card_ap_label, 0);
                data->list_card_ap->set_visible(data->list_card_ap, 0);
                data->list_card_hp_label->set_visible(data->list_card_hp_label, 0);
                data->list_card_hp->set_visible(data->list_card_hp, 0);
            } else {
                data->list_card_text->set_visible(data->list_card_text, 0);
                data->list_card_ap_label->set_text(data->list_card_ap_label, cdload_module.files.get_file(records_language + 0x32), 0x11);
                data->list_card_ap->set_line_number(data->list_card_ap, 0, card.record[1]);
                data->list_card_ap->measure(data->list_card_ap, 1);
                data->list_card_hp_label->set_text(data->list_card_hp_label, cdload_module.files.get_file(records_language + 0x32), 0x12);
                data->list_card_hp->set_line_number(data->list_card_hp, 0, card.record[2]);
                data->list_card_hp->measure(data->list_card_hp, 1);
            }
        }
    } else {
        data->list_card_name->set_visible(data->list_card_name, 0);
        data->list_card_level_label->set_visible(data->list_card_level_label, 0);
        data->list_card_level->set_visible(data->list_card_level, 0);
        data->list_card_text->set_visible(data->list_card_text, 0);
        data->list_card_ap_label->set_visible(data->list_card_ap_label, 0);
        data->list_card_ap->set_visible(data->list_card_ap, 0);
        data->list_card_hp_label->set_visible(data->list_card_hp_label, 0);
        data->list_card_hp->set_visible(data->list_card_hp, 0);
    }
}

/* Lists the cards the player has that aren't all in the deck: spare_copies = owned - in the deck. */
void stcrddek_count_spare_cards(StcrddekEditor *obj) {
    s32 i;
    s32 j;
    s16 *deck;

    for (i = 0; i < 315; i++) {
        obj->spare[i] = 0;
        obj->spare_copies[i] = gamestate_data.cards[i];
    }
    deck = (s16 *)gamestate_data.decks[obj->deck].cards;
    for (j = 0; j < 40; j++, deck++) {
        obj->spare_copies[*deck]--;
    }
    obj->spare_count = 0;
    for (i = 0, j = 0; i < 315; i++) {
        if (obj->spare_copies[i] > 0) {
            obj->spare[j++] = i;
            obj->spare_count++;
        }
    }
}


extern s32 stcrddek_grid_cursor_palettes[]; /* the grid cursor's animation frames */

void stcrddek_draw_editor(StcrddekEditor *obj) {
    Sprite spr;
    CardPicture card;
    s32 i;
    s32 id;
    s32 special;
    s32 type;

    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0);
    spr.set_layer_id(obj->layer_id, obj->ot_depth - 2);
    if (obj->grid_cursor_shown != 0) {
        if (gfx_module.funcs.get_time() - obj->frame_time >= 5) {
            obj->frame_time = gfx_module.funcs.get_time();
            obj->grid_frame++;
            if (obj->grid_frame >= 6) {
                obj->grid_frame = 0;
            }
        }
        spr.set_palette(stcrddek_grid_cursor_palettes[obj->grid_frame]);
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x43, (obj->grid_column << 5) | 0x10, (obj->grid_row << 5) + 0x3A);
        spr.set_palette(0);
    }
    if (obj->anims[1].level != 0) {
        if (obj->anims[1].level != 0x1000) {
            spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x25);
        }
        card_init(&card);
        id = (s16)gamestate_data.decks[obj->deck].cards[obj->grid_column + obj->grid_row * 9];
        card.select(id);
        special = card.get_class();
        if (special != 0) {
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), special + 0x11, 0x103, 0x24);
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xD, 0xFC, 0x22);
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xB, 0x4A, 0x12);
        } else {
            type = card.record[0];
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), type + 0x13, 0x103, 0x24);
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xD, 0xFC, 0x22);
            if (id == 0x45 || id == 0x70 || id == 0x9B || id == 0xC6 || id == 0xF1) {
                spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xB, 0x4A, 0x12);
            } else {
                spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xC, 0xC7, 0x12);
            }
        }
    }
    if (obj->anims[0].level != 0) {
        spr.set_scale(0x1000, obj->anims[0].level, 0x1000);
        if (obj->anims[0].level != 0x1000) {
            spr.set_pivot(0xA0, 0x74);
        }
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x40, 0x10, 0x15);
    }
    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->anims[2].level != 0) {
        if (obj->anims[2].level != 0x1000) {
            spr.set_scale(obj->anims[2].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x5C);
        } else {
            i = 0;
            card_init(&card);
            for (; i < 8; i++) {
                id = obj->spare[obj->first_line + i];
                if (id != 0) {
                    card.select(id);
                    if (card.get_class() != 0) {
                        spr.set_palette(card.record[0] - 1);
                        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x4B, 0x94, i * 14 + 0x27);
                    } else {
                        spr.set_palette(0);
                        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), card.record[0] + 0x4B, 0x94, i * 14 + 0x27);
                    }
                    spr.set_palette(0);
                    spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x31, 0x94, i * 14 + 0x27);
                }
            }
            if (obj->spare_count >= 9) {
                if (gfx_module.funcs.get_time() - obj->blink_time >= 9) {
                    obj->blink_time = gfx_module.funcs.get_time();
                    obj->arrows_shown = 1 - obj->arrows_shown;
                }
                if (obj->arrows_shown != 0) {
                    if (obj->first_line > 0) {
                        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x45, 0x126, 0x22);
                    }
                    if (obj->first_line < obj->spare_count - 8) {
                        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x46, 0x126, 0x8F);
                    }
                }
            }
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x3F, 0x81, 0x1E);
    }
    if (obj->anims[3].level != 0) {
        if (obj->anims[3].level != 0x1000) {
            spr.set_scale(obj->anims[3].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x25);
        }
        card_init(&card);
        id = obj->spare[obj->first_line + obj->list_cursor];
        card.select(id);
        special = card.get_class();
        if (special != 0) {
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), special + 0x11, 0x103, 0x9F);
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xD, 0xFC, 0x9D);
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xB, 0x4A, 0xB3);
        } else {
            type = card.record[0];
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), type + 0x13, 0x103, 0x9F);
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xD, 0xFC, 0x9D);
            if (id == 0x45 || id == 0x70 || id == 0x9B || id == 0xC6 || id == 0xF1) {
                spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xB, 0x4A, 0xB3);
            } else {
                spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xC, 0xC7, 0xB3);
            }
        }
        if (obj->anims[3].level != 0x1000) {
            spr.set_pivot(0x140, 0xA8);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0xA, 0x82, 0x9D);
    }
    if (obj->anims[4].level != 0) {
        if (obj->anims[4].level != 0x1000) {
            spr.set_scale(0x1000, obj->anims[4].level, 0x1000);
            spr.set_pivot(0, 0x78);
        }
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 3);
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x36, 0, 0x64);
    }
}

/* The deck editor's states: 5 moves the cursor in the deck's grid, 10 shows/hides the card info, 20-26
 * pick a card from the list to put in the deck (at most 4 copies: else 60-63 say so), 30-32 sort the deck,
 * 50-51 leave. */
void stcrddek_update_editor_menu(StcrddekEditor *obj, StcrddekEditorData *data) {
    s32 old;
    s32 old_line;
    s32 i;
    s32 j;
    s32 card;
    s32 slot;
    s32 tmp;
    s32 k;

    switch (obj->base.step) {
    case 0:
    default:
        stcrddek_util.window_anim_start(&obj->anims[0], 1);
        obj->base.step++;
        break;
    case 1:
        if (stcrddek_util.window_anim_update(&obj->anims[0])) {
            obj->hint_time = 0;
            stcrddek_show_deck_card(obj, data, 1);
            stcrddek_count_spare_cards(obj);
            data->grid = (Object *)stcrddek_create_grid(obj);
            obj->base.step++;
        }
        break;
    case 2:
        if (data->grid->state == OBJECT_STATE_DONE) {
            obj->grid_cursor_shown = 1;
            obj->base.step = 5;
        }
        break;
    case 5:
        old = obj->grid_column + obj->grid_row * 9;
        if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
            obj->grid_column--;
            if (obj->grid_column < 0) {
                obj->grid_column = 0;
            }
        } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
            if (obj->grid_row == 4) {
                obj->grid_column++;
                if (obj->grid_column >= 4) {
                    obj->grid_column = 3;
                }
            } else {
                obj->grid_column++;
                if (obj->grid_column >= 9) {
                    obj->grid_column = 8;
                }
            }
        }
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            obj->grid_row--;
            if (obj->grid_row < 0) {
                obj->grid_row = 0;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            obj->grid_row++;
            if (obj->grid_column >= 4) {
                if (obj->grid_row >= 4) {
                    obj->grid_row = 3;
                }
            } else if (obj->grid_row >= 5) {
                obj->grid_row = 4;
            }
        }
        stcrddek_show_deck_card(obj, data, 1);
        if (old != obj->grid_column + obj->grid_row * 9) {
            sound_module.play(0x4001B);
        } else if (!PAD_HELD(10) && PAD_PRESSED(11)) {
            sound_module.play(0x4001B);
            obj->base.step = 10;
        } else if (PAD_PRESSED(13)) {
            if (obj->spare_count != 0) {
                sound_module.play(0x4001C);
                obj->base.step = 20;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step = 50;
        } else if (PAD_PRESSED(12)) {
            sound_module.play(0x800450BD);
            obj->base.step = 30;
        }
        break;
    case 10:
        obj->grid_cursor_shown = 0;
        obj->details = 1 - obj->details;
        if (obj->details != 0) {
            stcrddek_util.window_anim_start(&obj->anims[1], 1);
            obj->base.step = 11;
            obj->base.substep = 0;
        } else {
            stcrddek_show_deck_card(obj, data, 1);
            stcrddek_util.window_anim_start(&obj->anims[1], 0);
            obj->base.step = 11;
            obj->base.substep = 1;
        }
        break;
    case 11:
        if (stcrddek_util.window_anim_update(&obj->anims[1])) {
            if (obj->base.substep == 0) {
                stcrddek_show_deck_card(obj, data, 1);
            }
            obj->grid_cursor_shown = 1;
            obj->base.set_step(obj, 5);
        }
        break;
    case 20:
        data->grid->state = OBJECT_STATE_END;
        stcrddek_count_spare_cards(obj);
        stcrddek_util.window_anim_start(&obj->anims[2], 1);
        while (obj->spare[obj->first_line + obj->list_cursor] == 0) {
            obj->first_line--;
            if (obj->first_line < 0) {
                obj->first_line = 0;
                obj->list_cursor--;
                if (obj->list_cursor < 0) {
                    obj->list_cursor = 0;
                }
            }
        }
        obj->grid_cursor_shown = 0;
        stcrddek_util.window_anim_start(&obj->anims[0], 0);
        if (obj->details != 0) {
            obj->anims[1].level = 0;
            obj->details = 0;
        }
        stcrddek_show_deck_card(obj, data, 0);
        obj->base.step++;
        break;
    case 21:
        if (stcrddek_util.window_anim_update(&obj->anims[0])) {
            obj->base.step++;
        }
        break;
    case 22:
        if (stcrddek_util.window_anim_update(&obj->anims[2])) {
            stcrddek_show_card_list(obj, data, 1);
            stcrddek_util.window_anim_start(&obj->anims[3], 1);
            obj->base.step++;
        }
        break;
    case 23:
        if (stcrddek_util.window_anim_update(&obj->anims[3])) {
            if (obj->spare_count >= 9 && data->scroll_bar == NULL) {
                data->scroll_bar = stcrddek_create_bar();
                data->scroll_bar->set_x(data->scroll_bar, 0x125, 0xC);
                data->scroll_bar->set_range(data->scroll_bar, 0x2A, 0x8F);
                data->scroll_bar->set_lines(data->scroll_bar, 8, obj->spare_count);
                data->scroll_bar->set_line(data->scroll_bar, obj->list_cursor);
            }
            data->list_cursor->set_pos(data->list_cursor, 0x89, obj->list_cursor * 14 + 0x27);
            data->list_cursor->show(data->list_cursor, 1);
            stcrddek_show_list_card(obj, data, 1);
            obj->base.step++;
        }
        break;
    case 24:
        old = obj->first_line + obj->list_cursor;
        old_line = obj->first_line;
        if (obj->spare_count >= 9) {
            if ((!PAD_HELD(11) && PAD_PRESSED(10)) || (!PAD_HELD(11) && PAD_REPEAT(10))) {
                obj->first_line -= 7;
                if (obj->first_line < 0) {
                    obj->first_line = 0;
                }
            } else if ((!PAD_HELD(10) && PAD_PRESSED(11)) || (!PAD_HELD(10) && PAD_REPEAT(11))) {
                for (k = 0; k < 7; k++) {
                    obj->first_line++;
                    if (obj->first_line > obj->spare_count - 8) {
                        obj->first_line = obj->spare_count - 8;
                        break;
                    }
                }
            }
        }
        if (old == obj->first_line + obj->list_cursor) {
            if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                obj->list_cursor--;
                if (obj->list_cursor < 0) {
                    obj->list_cursor = 0;
                    obj->first_line--;
                    if (obj->first_line < 0) {
                        obj->first_line = 0;
                    }
                }
            } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                if (obj->spare_count >= 9) {
                    obj->list_cursor++;
                    if (obj->list_cursor >= 8) {
                        obj->list_cursor = 7;
                        obj->first_line++;
                        if (obj->first_line > obj->spare_count - 8) {
                            obj->first_line = obj->spare_count - 8;
                        }
                    }
                } else {
                    obj->list_cursor++;
                    if (obj->list_cursor > obj->spare_count - 1) {
                        obj->list_cursor = obj->spare_count - 1;
                    }
                }
            }
        }
        if (data->scroll_bar != NULL && old_line != obj->first_line) {
            data->scroll_bar->set_line(data->scroll_bar, obj->first_line);
        }
        if (old != obj->first_line + obj->list_cursor) {
            sound_module.play(0x8004513E);
            data->list_cursor->set_pos(data->list_cursor, 0x89, obj->list_cursor * 14 + 0x27);
            stcrddek_show_card_list(obj, data, 1);
            stcrddek_show_list_card(obj, data, 1);
        } else if (PAD_PRESSED(13)) {
            card = obj->spare[obj->first_line + obj->list_cursor];
            slot = (s16)gamestate_data.decks[obj->deck].cards[obj->grid_column + obj->grid_row * 9];
            sound_module.play(0x8004503C);
            if (card != slot && gamestate_data.cards[card] - obj->spare_copies[card] >= 4) {
                obj->base.step = 60;
            } else {
                gamestate_data.decks[obj->deck].cards[obj->grid_column + obj->grid_row * 9] = card;
                obj->main->count_card_types(obj->main);
                obj->base.step++;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step++;
        }
        break;
    case 25:
        stcrddek_util.window_anim_start(&obj->anims[2], 0);
        stcrddek_show_card_list(obj, data, 0);
        stcrddek_util.window_anim_start(&obj->anims[3], 0);
        stcrddek_show_list_card(obj, data, 0);
        data->list_cursor->show(data->list_cursor, 0);
        if (data->scroll_bar != NULL) {
            data->scroll_bar->base.state = OBJECT_STATE_END;
        }
        obj->base.step++;
        break;
    case 26:
        stcrddek_util.window_anim_update(&obj->anims[2]);
        if (stcrddek_util.window_anim_update(&obj->anims[3])) {
            obj->list_cursor = 0;
            obj->first_line = 0;
            obj->base.step = 0;
        }
        break;
    case 30:
        data->grid->state = OBJECT_STATE_END;
        stcrddek_count_spare_cards(obj);
        obj->base.timer = 0;
        obj->base.step++;
        stcrddek_util.window_anim_start(&obj->anims[0], 0);
        obj->grid_cursor_shown = 0;
        stcrddek_util.window_anim_start(&obj->anims[0], 0);
        obj->anims[1].level = 0;
        obj->details = 0;
        stcrddek_show_deck_card(obj, data, 0);
        break;
    case 31:
        if (stcrddek_util.window_anim_update(&obj->anims[3])) {
            obj->base.step++;
        }
        break;
    case 32:
        /* Sorts the deck by card ID. */
        for (i = 0; i < 39; i++) {
            for (j = i + 1; j < 40; j++) {
                tmp = (s16)gamestate_data.decks[obj->deck].cards[i];
                if ((s16)gamestate_data.decks[obj->deck].cards[j] < tmp) {
                    gamestate_data.decks[obj->deck].cards[i] = gamestate_data.decks[obj->deck].cards[j];
                    gamestate_data.decks[obj->deck].cards[j] = tmp;
                }
            }
        }
        obj->base.step = 0;
        break;
    case 50:
        data->grid->state = OBJECT_STATE_END;
        obj->grid_cursor_shown = 0;
        stcrddek_util.window_anim_start(&obj->anims[0], 0);
        if (obj->details != 0) {
            obj->anims[1].level = 0;
            obj->details = 0;
        }
        stcrddek_show_deck_card(obj, data, 0);
        obj->base.step++;
        break;
    case 51:
        if (stcrddek_util.window_anim_update(&obj->anims[0])) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 60:
        data->list_cursor->set_palette(data->list_cursor, 7);
        data->list_cursor->stop(data->list_cursor, 1);
        stcrddek_util.window_anim_start(&obj->anims[4], 1);
        obj->base.step++;
        break;
    case 61:
        if (stcrddek_util.window_anim_update(&obj->anims[4])) {
            data->warning->set_text(data->warning, cdload_module.files.get_file(records_language + 0x32), 0x1E);
            obj->base.step++;
        }
        break;
    case 62:
        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            data->warning->set_visible(data->warning, 0);
            stcrddek_util.window_anim_start(&obj->anims[4], 0);
            obj->base.step++;
        }
        break;
    case 63:
        if (stcrddek_util.window_anim_update(&obj->anims[4])) {
            data->list_cursor->set_palette(data->list_cursor, 0);
            data->list_cursor->stop(data->list_cursor, 0);
            obj->base.step = 23;
        }
        break;
    }
    obj->hint_time += gfx_module.funcs.get_frame_ticks();
    if (obj->hint_time > 120) {
        obj->hint_time -= 120;
    }
}


void stcrddek_update_editor(StcrddekEditor *obj, StcrddekEditorData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stcrddek_create_editor_windows(obj, data);
        obj->anims[0].duration = 10;
        obj->anims[1].duration = 10;
        obj->anims[2].duration = 10;
        obj->anims[3].duration = 10;
        obj->anims[4].duration = 10;
        break;
    case OBJECT_STATE_RUN:
        stcrddek_update_editor_menu(obj, data);
        stcrddek_draw_editor(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StcrddekEditor *stcrddek_create_editor(StcrddekMain *main, s32 deck) {
    StcrddekEditor *obj = object_new(stcrddek_update_editor, sizeof(StcrddekEditor), 0xD4);

    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->main = main;
    obj->deck = deck;
    return obj;
}

void stcrddek_stub_init(StcrddekStub *obj, void *data) {
}

void stcrddek_stub_draw(StcrddekStub *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
}

/* Ends the object (OBJECT_STATE_END) on a press of button 14. */
void stcrddek_stub_input(StcrddekStub *obj, void *data) {
    if (PAD_PRESSED(14)) {
        obj->base.state = OBJECT_STATE_END;
    }
}

void stcrddek_stub_update(StcrddekStub *obj, void *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stcrddek_stub_init(obj, data);
        break;
    case OBJECT_STATE_RUN:
        stcrddek_stub_input(obj, data);
        stcrddek_stub_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StcrddekStub *stcrddek_stub_create(s32 arg0) {
    StcrddekStub *obj = object_new(stcrddek_stub_update, sizeof(StcrddekStub), 0);

    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->unk_50 = arg0;
    return obj;
}

/* the grid cursor's animation frames */
s32 stcrddek_grid_cursor_palettes[6] = { 0, 1, 2, 3, 2, 1 };
