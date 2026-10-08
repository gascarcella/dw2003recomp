#include "common.h"
#include "object.h"
#include "gfx.h"
#include "cdload.h"
#include "gamestate.h"
#include "message.h"
#include "cardgame.h"

/* The root object (cardgame_update_root; the overlay's entry point cardgame_start creates it), which
 * starts the game, and two small objects the game flow creates: an icon and a deck's window. */

/* The windows of a deck's window (its data block). */
typedef struct CardgameDeckWindowData {
    /* 0x00 */ MessageWindow *name;
    /* 0x04 */ MessageWindow *counts[6];
} CardgameDeckWindowData; /* size 0x1C */

/* The screen's layer. */
extern RECT cardgame_screen_rect;
/* Where each count goes in a deck's window. */
extern s16 cardgame_deck_window_count_pos[6][2];

void cardgame_update_root(Object *obj, CardgameGame **data);
void cardgame_deck_window_update(CardgameDeckWindow *obj, CardgameDeckWindowData *data);

s32 cardgame_check_condition(CardgameGame *game, CardgameBoard *board, s32 kind) {
    s32 ok = 0;
    s32 side = game->turns[game->turn - 1].side;
    s32 other = side ^ 1;
    s32 i;
    s32 j;
    s32 n;
    CardgameSlot *slot;
    CardgameTurn *turn;

    switch (kind) {
    case 1:
        if (game->players[other].hand_count == 0) {
            ok = 1;
        }
        break;
    case 2: {
        CardPicture pic;

        card_init(&pic);
        ok = 1;
        for (i = 0; i < game->players[other].hand_count; i++) {
            pic.select(game->card_ids[game->players[other].hand[i]] + 1);
            if (pic.record[0] == 6 && pic.record[3] == 0x10) {
                ok = 0;
                break;
            }
        }
        break;
    }
    case 3: {
        CardPicture pic;

        card_init(&pic);
        ok = 1;
        for (i = 0; i < game->players[other].hand_count; i++) {
            pic.select(game->card_ids[game->players[other].hand[i]] + 1);
            if (pic.record[0] != 5) {
                ok = 0;
                break;
            }
        }
        break;
    }
    case 4:
        if (game->players[side].discard_count == 0) {
            ok = 1;
        }
        break;
    case 5:
        if (game->players[side].deck_count == 0) {
            ok = 1;
        }
        break;
    case 6:
        if (game->players[other].deck_count == 0) {
            ok = 1;
        }
        break;
    case 7: {
        CardPicture pic;

        card_init(&pic);
        ok = 1;
        for (i = game->players[side].deck_pos; i < 40; i++) {
            pic.select(game->card_ids[game->players[side].deck[i]] + 1);
            if (pic.record[3] == 0x10) {
                ok = 0;
                break;
            }
        }
        break;
    }
    case 8: {
        CardPicture pic;

        card_init(&pic);
        ok = 1;
        for (i = game->players[side].deck_pos; i < 40; i++) {
            pic.select(game->card_ids[game->players[side].deck[i]] + 1);
            if (pic.record[3] != 0x10) {
                ok = 0;
                break;
            }
        }
        break;
    }
    case 9:
        ok = 1;
        n = game->turn - 1;
        for (j = 0; j < 12; j++) {
            game->effect.marked[j] = 0;
            if (j < 6) {
                if (j >= game->slots[0].count) {
                    continue;
                }
                slot = &game->slots[0].slots[j];
            } else {
                if (j - 6 >= game->slots[1].count) {
                    continue;
                }
                slot = &game->slots[1].slots[j - 6];
            }
            if (slot->id == game->turns[n].target) {
                ok = 0;
                break;
            }
        }
        break;
    case 10:
        ok = 1;
        for (i = 0; i < 12 && ok == 1; i++) {
            if (i < 6) {
                if (i >= game->slots[0].count) {
                    continue;
                }
            } else if (i - 6 >= game->slots[1].count) {
                continue;
            }
            {
                switch (game->turns[game->turn - 1].target_kind) {
                case 1:
                    if (side == 0) {
                        if (i < 6) {
                            ok = 0;
                        }
                    } else if (i >= 6) {
                        ok = 0;
                    }
                    break;
                case 2:
                    if (side == 0) {
                        if (i >= 6) {
                            ok = 0;
                        }
                    } else if (i < 6) {
                        ok = 0;
                    }
                    break;
                case 3:
                    ok = 0;
                    break;
                case 4:
                    if (board->get_card_kind(board, i < 6 ? game->slots[0].slots[i].card : game->slots[1].slots[i - 6].card) != 1) {
                        ok = 0;
                    }
                    break;
                case 5:
                    if (board->get_card_kind(board, i < 6 ? game->slots[0].slots[i].card : game->slots[1].slots[i - 6].card) != 2) {
                        ok = 0;
                    }
                    break;
                case 6:
                    if (board->get_card_kind(board, i < 6 ? game->slots[0].slots[i].card : game->slots[1].slots[i - 6].card) == 3) {
                        ok = 0;
                    }
                    break;
                case 7:
                    if (board->get_card_kind(board, i < 6 ? game->slots[0].slots[i].card : game->slots[1].slots[i - 6].card) != 4) {
                        ok = 0;
                    }
                    break;
                case 8:
                    if (board->get_card_kind(board, i < 6 ? game->slots[0].slots[i].card : game->slots[1].slots[i - 6].card) == 6) {
                        ok = 0;
                    }
                    break;
                }
            }
        }
        break;
    case 11:
        ok = 1;
        for (j = 0; j < game->players[side].hand_count; j++) {
            if (game->turns[game->turn - 1].target == game->players[side].hand[j]) {
                ok = 0;
                break;
            }
        }
        break;
    }
    return ok;
}

void cardgame_update_root(Object *obj, CardgameGame **data) {
    Tim tim;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0xA000);
        gfx_module.funcs.init_display(320, 240, 0, 0);
        tim_init(&tim);
        tim.set_image_pos(0x280, 0);
        tim.load_all(cdload_module.get_subfile_by_id(0x025D0000));
        tim.set_image_pos(0x340, 0);
        tim.load_all(cdload_module.get_subfile_by_id(0x025D0001));
        layer = gfx_module.funcs.create_layer(&cardgame_screen_rect, 3, 0x100);
        layer->set_bg_color(layer, 0x1F, 0x1F, 0x1F);
        *data = cardgame_game_create(gamestate_data.funcs.get_map_entry());
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if ((*data)->done == 2) {
            obj->set_state(obj, OBJECT_STATE_DONE);
            (*data)->base.set_state(*data, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
        switch (obj->step) {
        case 0:
        default:
            gamestate_data.funcs.set_next_map((gamestate_data.funcs.get_map() & 0xF) ? 0x1500 : gamestate_data.field_map, 0);
            obj->next_step(obj);
            break;
        case 1:
            break;
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

/* The overlay's entry point (overlay_entries): overlay_run_object keeps the object (v0; PC_PORT: FINDINGS 8). */
OBJECT_V0(Object *) cardgame_start(void) {
    OBJECT_V0_TAIL(object_new(cardgame_update_root, sizeof(Object), sizeof(CardgameGame *)))
}

void cardgame_icon_draw(CardgameIcon *obj) {
    Sprite spr;
    s32 frame;

    if (obj->scale_x != 0 && obj->scale_y != 0) {
        sprite_init(&spr);
        spr.set_pivot(obj->x, obj->y + 19);
        spr.set_scale(obj->scale_x, obj->scale_y, 0x1000);
        if (obj->play_once == 0) {
            spr.set_palette((obj->ticks >> 2) % 16);
        } else {
            frame = obj->ticks >> 1;
            if (frame >= 7) {
                frame = 7;
            }
            spr.set_palette(frame);
        }
        spr.set_layer_id(0x100, 1);
        spr.set_vram_pos(0x280, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x025D0002), 0x47, obj->x, obj->y);
        obj->ticks += gfx_module.funcs.get_frame_ticks();
    }
}

void cardgame_icon_update(CardgameIcon *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->duration = 10;
        obj->timer = 10;
        obj->state = 0;
        obj->scale_x = 0x1000;
        obj->scale_y = 0;
        break;
    case OBJECT_STATE_RUN:
        switch (obj->state) {
        case 1:
            break;
        case 0:
            obj->scale_y = 0x1000 - (obj->timer << 12) / obj->duration;
            obj->timer -= gfx_module.funcs.get_frame_ticks();
            if (obj->timer <= 0) {
                obj->scale_y = 0x1000;
                obj->state = 1;
            }
            break;
        case 2:
            obj->base.set_state(obj, OBJECT_STATE_DONE);
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        obj->scale_y = (obj->timer << 12) / obj->duration;
        obj->timer -= gfx_module.funcs.get_frame_ticks();
        if (obj->timer <= 0) {
            obj->scale_y = 0;
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
    cardgame_icon_draw(obj);
}

void cardgame_icon_set_pos(CardgameIcon *obj, s16 x, s16 y) {
    obj->x = x;
    obj->y = y;
}

void cardgame_icon_play(CardgameIcon *obj) {
    obj->play_once = 1;
    obj->ticks = 0;
}

void cardgame_icon_close(CardgameIcon *obj) {
    obj->duration = 5;
    obj->timer = 5;
    obj->state = 2;
    obj->scale_y = 0x1000;
}

CardgameIcon *cardgame_icon_create(s16 x, s16 y) {
    CardgameIcon *obj = object_new(cardgame_icon_update, sizeof(CardgameIcon), 0);

    obj->set_pos = cardgame_icon_set_pos;
    obj->close_icon = cardgame_icon_close;
    obj->play = cardgame_icon_play;
    obj->x = x;
    obj->y = y;
    obj->play_once = 0;
    return obj;
}

void cardgame_deck_window_draw(CardgameDeckWindow *obj) {
    s16 frames[8] = { 0, 1, 2, 3, 2, 1, 0, 0 };
    Sprite frame;
    Sprite cursor;
    s32 i;

    if (obj->scale_x != 0 && obj->scale_y != 0) {
        sprite_init(&frame);
        frame.set_pivot(obj->x, obj->y);
        frame.set_scale(obj->scale_x, obj->scale_y, 0x1000);
        frame.set_layer_id(0x100, 1);
        frame.set_vram_pos(0x280, 0);
        frame.draw(cdload_module.get_subfile_by_id(0x025D0002), 0x45, obj->x, obj->y);
        sprite_init(&cursor);
        cursor.set_pivot(obj->x, obj->y);
        if (obj->playing != 0) {
            i = obj->ticks >> 1;
            if (i >= 7) {
                i = 7;
            }
            cursor.set_palette(frames[i]);
            obj->ticks += gfx_module.funcs.get_frame_ticks();
        }
        cursor.set_scale(obj->scale_x, obj->scale_y, 0x1000);
        cursor.set_layer_id(0x100, 1);
        cursor.set_vram_pos(0x280, 0);
        cursor.draw(cdload_module.get_subfile_by_id(0x025D0002), 0x46, obj->x, obj->y);
    }
}

void cardgame_deck_window_update(CardgameDeckWindow *obj, CardgameDeckWindowData *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->duration = 12;
        obj->timer = 12;
        obj->state = 0;
        obj->scale_y = 0x1000;
        obj->scale_x = 0;
        data->name = message_create_window(0x100, 1, 0, 0);
        data->name->set_text(data->name, (u8 *)gamestate_data.decks[obj->deck].name, -1);
        data->name->set_pos(data->name, obj->x + 5, obj->y + 3);
        for (i = 0; i < 6; i++) {
            data->counts[i] = message_create_window(0x100, 1, 0, 0);
            data->counts[i]->set_pos(data->counts[i], obj->x + cardgame_deck_window_count_pos[i][0],
                                     obj->y + cardgame_deck_window_count_pos[i][1]);
            data->counts[i]->set_line_number(data->counts[i], 0, obj->counts[i]);
            data->counts[i]->measure(data->counts[i], 1);
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->state) {
        case 1:
            break;
        case 0:
            obj->scale_x = 0x1000 - (obj->timer << 12) / obj->duration;
            obj->timer -= gfx_module.funcs.get_frame_ticks();
            if (obj->timer <= 0) {
                obj->scale_x = 0x1000;
                obj->state = 1;
            }
            break;
        case 2:
            obj->base.set_state(obj, OBJECT_STATE_DONE);
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        obj->scale_x = (obj->timer << 12) / obj->duration;
        obj->timer -= gfx_module.funcs.get_frame_ticks();
        if (obj->timer <= 0) {
            obj->scale_x = 0;
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
    if (obj->state == 1) {
        data->name->set_pos(data->name, obj->x + 5, obj->y + 3);
        data->name->set_visible(data->name, 1);
        for (i = 0; i < 6; i++) {
            data->counts[i]->set_visible(data->counts[i], 1);
            data->counts[i]->set_pos(data->counts[i], obj->x + cardgame_deck_window_count_pos[i][0],
                                     obj->y + cardgame_deck_window_count_pos[i][1]);
        }
    } else {
        data->name->set_visible(data->name, 0);
        for (i = 0; i < 6; i++) {
            data->counts[i]->set_visible(data->counts[i], 0);
        }
    }
    cardgame_deck_window_draw(obj);
}

void cardgame_deck_window_play(CardgameDeckWindow *obj) {
    obj->playing = 1;
    obj->ticks = 0;
}

void cardgame_deck_window_close(CardgameDeckWindow *obj) {
    obj->duration = 6;
    obj->timer = 6;
    obj->state = 2;
    obj->scale_x = 0x1000;
}

CardgameDeckWindow *cardgame_deck_window_create(s32 deck, s32 x, s32 y) {
    CardPicture pic;
    CardgameDeckWindow *obj;
    s32 i;

    card_init(&pic);
    obj = object_new(cardgame_deck_window_update, sizeof(CardgameDeckWindow), sizeof(CardgameDeckWindowData));
    for (i = 0; i < 6; i++) {
        obj->counts[i] = 0;
    }
    for (i = 0; i < 40; i++) {
        pic.select((s16)gamestate_data.decks[deck].cards[i]);
        obj->counts[pic.record[0] - 1]++;
    }
    obj->play = cardgame_deck_window_play;
    obj->deck = deck;
    obj->x = x;
    obj->y = y;
    obj->playing = 0;
    obj->close_window = cardgame_deck_window_close;
    return obj;
}

/* .data (address order) */

RECT cardgame_screen_rect = { 0, 0, 320, 240 };
s16 cardgame_deck_window_count_pos[6][2] = { { 36, 20 }, { 71, 20 }, { 106, 20 }, { 141, 20 }, { 176, 20 }, { 211, 20 } };
