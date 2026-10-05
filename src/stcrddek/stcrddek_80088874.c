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

/* STCRDDEK.PRO: a scroll bar object (the same code as STGDGLAB's and STSTATUS's) and the deck screen's
 * main object: the three decks with their card counts per type, then edit or rename the chosen one. */

/* A deck's windows: its name and the counts of the six card types. */
typedef struct StcrddekDeckWindows {
    /* 0x00 */ MessageWindow *name;
    /* 0x04 */ MessageWindow *type_counts[6];
} StcrddekDeckWindows; /* size 0x1C */

/* The children of the main object (its data block). */
typedef struct StcrddekMainData {
    /* 0x00 */ Object *screen; /* the deck editor or the name entry */
    /* 0x04 */ MessageWindow *help_line; /* help line */
    /* 0x08 */ StcrddekDeckWindows decks[3];
    /* 0x5C */ MessageWindow *menu[2];   /* "edit", "rename" */
    /* 0x64 */ MessageCursor *menu_cursor; /* the menu cursor */
    /* 0x68 */ Object *fade;   /* fade */
} StcrddekMainData; /* size 0x6C */


void stcrddek_set_bar_x(StcrddekBar *obj, s32 x, s32 width) {
    obj->x = x;
    obj->w = width;
}

void stcrddek_set_bar_range(StcrddekBar *obj, s32 top, s32 bottom) {
    obj->top = top;
    obj->bottom = bottom;
    obj->range_set = 1;
}

void stcrddek_set_bar_lines(StcrddekBar *obj, s32 shown, s32 lines) {
    obj->shown = shown;
    obj->lines = lines;
    obj->lines_set = 1;
}

void stcrddek_set_bar_line(StcrddekBar *obj, s32 line) {
    obj->first_line = line;
}

void stcrddek_update_bar(StcrddekBar *obj) {
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

StcrddekBar *stcrddek_create_bar(void) {
    StcrddekBar *obj = object_new(stcrddek_update_bar, sizeof(StcrddekBar), 0);

    obj->set_x = stcrddek_set_bar_x;
    obj->set_range = stcrddek_set_bar_range;
    obj->set_lines = stcrddek_set_bar_lines;
    obj->set_line = stcrddek_set_bar_line;
    obj->layer_id = 0x1000;
    obj->ot_depth = 3;
    return obj;
}

/* Creates the main object's windows (its children 1 to 26). */
void stcrddek_create_main_windows(StcrddekMain *obj, StcrddekMainData *data) {
    s32 deck;
    s32 i;
    s32 y;
    MessageWindow **windows;

    data->help_line = message_create_window(obj->layer_id, 1, 0x97, 0x22);
    for (deck = 0; deck < 3; deck++) {
        y = deck * 0x2D;
        data->decks[deck].name = message_create_window(obj->layer_id, 1, 0x1C, y + 0x53);
        for (i = 0; i < 6; i++) {
            data->decks[deck].type_counts[i] = message_create_window(obj->layer_id, 1, i * 0x23 + 0x3B, 0x64 + y);
        }
    }
    for (deck = 0; deck < 2; deck++) {
        data->menu[deck] = message_create_window(obj->layer_id, 1, 0xA7, deck * 14 + 0x23);
    }
    data->menu_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 2, 0x9A, 0x23);
    data->menu_cursor->show(data->menu_cursor, 0);
    windows = &data->help_line;
    for (deck = 0; deck < obj->base.child_count - 3; deck++, windows++) {
        (*windows)->set_ot_depth(*windows, obj->ot_depth - 2);
    }
}

/* Shows (or hides) deck `deck`'s name and card counts. */
void stcrddek_show_deck(StcrddekMain *obj, StcrddekMainData *data, s32 deck, s32 show) {
    s32 i;

    if (show) {
        data->decks[deck].name->set_text(data->decks[deck].name, gamestate_data.decks[deck].name, -1);
        for (i = 0; i < 6; i++) {
            data->decks[deck].type_counts[i]->set_line_number(data->decks[deck].type_counts[i], 0, obj->type_counts[deck][i]);
            data->decks[deck].type_counts[i]->measure(data->decks[deck].type_counts[i], 1);
        }
    } else {
        data->decks[deck].name->set_visible(data->decks[deck].name, 0);
        for (i = 0; i < 6; i++) {
            data->decks[deck].type_counts[i]->set_visible(data->decks[deck].type_counts[i], 0);
        }
    }
}

void stcrddek_draw_main(StcrddekMain *obj) {
    Sprite spr;
    s32 i;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
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
    if (obj->title_anim.level != 0) {
        if (obj->title_anim.level != 0x1000) {
            spr.set_scale(obj->title_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x22);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x30, 0x7B, 0x1C);
    }
    for (i = 0; i < 3; i++) {
        if (obj->decks_anim[i].level != 0) {
            spr.set_scale(obj->decks_anim[i].level, 0x1000, 0x1000);
            if (obj->decks_anim[i].level != 0x1000) {
                spr.set_pivot(0x17, i * 0x2D + 0x63);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x41, 0x17, i * 0x2D + 0x50);
        }
    }
    if (obj->menu_anim.level != 0) {
        spr.set_scale(obj->menu_anim.level, 0x1000, 0x1000);
        if (obj->menu_anim.level != 0x1000) {
            spr.set_pivot(0x140, 0x37);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x2D, 0x92, 0x1C);
    }
    if (obj->cursor_anim.level != 0) {
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 2);
        spr.set_scale(0x1000, obj->cursor_anim.level, 0x1000);
        if (obj->cursor_anim.level != 0x1000) {
            spr.set_pivot(0x17, obj->deck_cursor * 0x2D + 0x63);
        }
        if (obj->cursor_stopped == 0) {
            if (gfx_module.funcs.get_time() - obj->cursor_time >= 5) {
                obj->cursor_time = gfx_module.funcs.get_time();
                obj->cursor_frame++;
                if (obj->cursor_frame >= 16) {
                    obj->cursor_frame = 0;
                }
            }
            spr.set_palette(obj->cursor_frame);
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x42, 0x17, obj->deck_cursor * 0x2D + 0x50);
        } else {
            spr.draw(cdload_module.get_subfile_by_id(0x063E0000), 0x44, 0x17, obj->deck_cursor * 0x2D + 0x50);
        }
    }
}

/* type_counts[deck][type - 1]: the cards of each type (byte 0 of the card's record) in the three decks. */
void stcrddek_count_card_types(StcrddekMain *obj) {
    CardPicture card;
    s32 deck;
    s32 i;

    card_init(&card);
    for (deck = 0; deck < 3; deck++) {
        for (i = 0; i < 6; i++) {
            obj->type_counts[deck][i] = 0;
        }
        for (i = 0; i < 40; i++) {
            card.select((s16)gamestate_data.decks[deck].cards[i]);
            obj->type_counts[deck][card.record[0] - 1]++;
        }
    }
}

void stcrddek_update_menu(StcrddekMain *obj, StcrddekMainData *data) {
    s32 old_deck;
    s32 old_mode;
    Fade *fade;

    switch (obj->base.step) {
    case 0:
    default:
        stcrddek_count_card_types(obj);
        stcrddek_util.window_anim_start(&obj->title_anim, 1);
        stcrddek_util.window_anim_start(&obj->decks_anim[0], 1);
        obj->cursor_stopped = 0;
        obj->base.step++;
        break;
    case 1:
        stcrddek_util.window_anim_update(&obj->title_anim);
        if (stcrddek_util.window_anim_update(&obj->decks_anim[0])) {
            data->help_line->set_text(data->help_line, cdload_module.files.get_file(records_language + 0x32), 0x19);
            stcrddek_show_deck(obj, data, 0, 1);
            stcrddek_util.window_anim_start(&obj->decks_anim[1], 1);
            obj->base.step++;
        }
        break;
    case 2:
        if (stcrddek_util.window_anim_update(&obj->decks_anim[1])) {
            stcrddek_show_deck(obj, data, 1, 1);
            stcrddek_util.window_anim_start(&obj->decks_anim[2], 1);
            stcrddek_util.window_anim_start(&obj->cursor_anim, 1);
            obj->base.step++;
        }
        break;
    case 3:
        stcrddek_util.window_anim_update(&obj->cursor_anim);
        if (stcrddek_util.window_anim_update(&obj->decks_anim[2])) {
            stcrddek_show_deck(obj, data, 2, 1);
            obj->base.step++;
        }
        break;
    case 4:
        old_deck = obj->deck_cursor;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            obj->deck_cursor--;
            if (obj->deck_cursor < 0) {
                obj->deck_cursor = 0;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            obj->deck_cursor++;
            if (obj->deck_cursor >= 3) {
                obj->deck_cursor = 2;
            }
        }
        if (old_deck != obj->deck_cursor) {
            sound_module.play(0x4001B);
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            obj->base.step = 10;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.set_step(obj, 100);
        }
        break;
    case 10:
        obj->cursor_stopped = 1;
        data->help_line->set_visible(data->help_line, 0);
        stcrddek_util.window_anim_start(&obj->title_anim, 0);
        obj->base.step++;
        break;
    case 11:
        if (stcrddek_util.window_anim_update(&obj->title_anim)) {
            data->help_line->set_visible(data->help_line, 0);
            stcrddek_util.window_anim_start(&obj->menu_anim, 1);
            obj->base.step++;
        }
        break;
    case 12:
        if (stcrddek_util.window_anim_update(&obj->menu_anim)) {
            obj->rename = 0;
            data->menu_cursor->set_pos(data->menu_cursor, 0x9A, 0x23);
            data->menu_cursor->show(data->menu_cursor, 1);
            data->menu[0]->set_text(data->menu[0], cdload_module.files.get_file(records_language + 0x32), 0x1A);
            data->menu[1]->set_text(data->menu[1], cdload_module.files.get_file(records_language + 0x32), 0x1B);
            obj->base.step++;
        }
        break;
    case 13:
        old_mode = obj->rename;
        if (PAD_PRESSED(4)) {
            obj->rename = 0;
        } else if (PAD_PRESSED(6)) {
            obj->rename = 1;
        }
        if (old_mode != obj->rename) {
            sound_module.play(0x8004513E);
            data->menu_cursor->set_pos(data->menu_cursor, 0x9A, obj->rename * 14 + 0x23);
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x8004503C);
            obj->base.set_step(obj, 50);
            obj->base.substep = 1;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step = 15;
        }
        break;
    case 15:
        data->menu_cursor->show(data->menu_cursor, 0);
        data->menu[0]->set_visible(data->menu[0], 0);
        data->menu[1]->set_visible(data->menu[1], 0);
        stcrddek_util.window_anim_start(&obj->menu_anim, 0);
        obj->base.step++;
        break;
    case 16:
        if (stcrddek_util.window_anim_update(&obj->menu_anim)) {
            stcrddek_util.window_anim_start(&obj->title_anim, 1);
            obj->base.step++;
        }
        break;
    case 17:
        if (stcrddek_util.window_anim_update(&obj->title_anim)) {
            data->help_line->set_text(data->help_line, cdload_module.files.get_file(records_language + 0x32), 0x19);
            obj->cursor_stopped = 0;
            obj->base.set_step(obj, 4);
        }
        break;
    case 50:
        if (obj->base.substep != 0) {
            data->menu_cursor->stop(data->menu_cursor, 1);
            data->menu_cursor->set_palette(data->menu_cursor, 7);
        }
        stcrddek_util.window_anim_start(&obj->cursor_anim, 0);
        stcrddek_util.window_anim_start(&obj->decks_anim[2], 0);
        stcrddek_show_deck(obj, data, 2, 0);
        obj->base.step++;
        break;
    case 51:
        stcrddek_util.window_anim_update(&obj->cursor_anim);
        if (stcrddek_util.window_anim_update(&obj->decks_anim[2])) {
            stcrddek_util.window_anim_start(&obj->decks_anim[1], 0);
            stcrddek_show_deck(obj, data, 1, 0);
            obj->base.step++;
        }
        break;
    case 52:
        if (stcrddek_util.window_anim_update(&obj->decks_anim[1])) {
            if (obj->base.substep != 0) {
                data->menu_cursor->stop(data->menu_cursor, 0);
                data->menu_cursor->set_palette(data->menu_cursor, 0);
                data->menu_cursor->show(data->menu_cursor, 0);
                data->menu[0]->set_visible(data->menu[0], 0);
                data->menu[1]->set_visible(data->menu[1], 0);
                stcrddek_util.window_anim_start(&obj->menu_anim, 0);
            } else {
                data->help_line->set_visible(data->help_line, 0);
                stcrddek_util.window_anim_start(&obj->title_anim, 0);
            }
            stcrddek_util.window_anim_start(&obj->decks_anim[0], 0);
            stcrddek_show_deck(obj, data, 0, 0);
            obj->base.step++;
        }
        break;
    case 53:
        if (obj->base.substep != 0) {
            stcrddek_util.window_anim_update(&obj->menu_anim);
        } else {
            stcrddek_util.window_anim_update(&obj->title_anim);
        }
        if (stcrddek_util.window_anim_update(&obj->decks_anim[0])) {
            obj->base.step++;
        }
        break;
    case 54:
        if (obj->base.substep != 0) {
            if (obj->rename == 0) {
                data->screen = (Object *)stcrddek_create_editor(obj, obj->deck_cursor);
            } else {
                data->screen = (Object *)stcrddek_create_name_entry(gamestate_data.decks[obj->deck_cursor].name);
            }
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        } else {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 100:
        fade = stcrddek_create_fade();
        data->fade = (Object *)fade;
        fade->start(fade, 0, 10);
        obj->base.step++;
        break;
    case 101:
        if (data->fade->state == OBJECT_STATE_DONE) {
            obj->base.step = 54;
        }
        break;
    }
}

void stcrddek_update_main(StcrddekMain *obj, StcrddekMainData *data) {
    s32 deck;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            stcrddek_util.load_files();
            obj->base.step++;
            break;
        case 1:
            if (stcrddek_util.is_loading() == 0) {
                stcrddek_create_main_windows(obj, data);
                obj->title_anim.duration = 10;
                obj->decks_anim[0].duration = 10;
                obj->decks_anim[1].duration = 10;
                obj->decks_anim[2].duration = 10;
                obj->menu_anim.duration = 10;
                obj->cursor_anim.duration = 10;
                obj->base.next_state(obj);
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        stcrddek_update_menu(obj, data);
        stcrddek_draw_main(obj);
        break;
    case OBJECT_STATE_DONE:
        if (obj->rename != 0) {
            switch (obj->base.step) {
            case 0:
            default:
                if (data->screen->step == 100) {
                    ((StcrddekNameEntry *)data->screen)
                        ->get_name((StcrddekNameEntry *)data->screen, gamestate_data.decks[obj->deck_cursor].name);
                    ((StcrddekNameEntry *)data->screen)->close((StcrddekNameEntry *)data->screen);
                    obj->base.step++;
                }
                break;
            case 1:
                if (data->screen->state == OBJECT_STATE_DONE) {
                    data->screen->state = OBJECT_STATE_END;
                    obj->base.set_state(obj, OBJECT_STATE_RUN);
                }
                break;
            }
        } else if (data->screen == NULL) {
            obj->base.set_state(obj, OBJECT_STATE_RUN);
        }
        stcrddek_draw_main(obj);
        break;
    case OBJECT_STATE_END:
        deck = gamestate_data.funcs.get_prev_map();
        gamestate_data.funcs.set_next_map(deck, gamestate_data.funcs.get_map_entry());
        break;
    }
}

StcrddekMain *stcrddek_create_main(void) {
    StcrddekMain *obj = object_new(stcrddek_update_main, sizeof(StcrddekMain), sizeof(StcrddekMainData));

    obj->count_card_types = stcrddek_count_card_types;
    obj->layer_id = 0x1000;
    obj->ot_depth = 7;
    return obj;
}
