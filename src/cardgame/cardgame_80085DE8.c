#include "common.h"
#include "object.h"
#include "gfx.h"
#include "records.h"
#include "sound.h"
#include "pad.h"
#include "cardgame.h"

/* The card effects: one pair of handlers per effect (cardgame_run_effect's two switches), taking the game
 * and the board display. */

s32 cardgame_view_board_get_duration(CardgameGame *game, CardgameBoard *board);
void cardgame_info_show(CardgameGame *game, CardgameBoard *board, s32 first);
void cardgame_player_change_points(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player, s32 add, s32 card);
void cardgame_choose_card_start(CardgameGame *game, CardgameBoard *board, s32 arg2, s32 arg3);
void cardgame_choose_card_input(CardgameGame *game, CardgameBoard *board);
void cardgame_hand_input_pick(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player);
void cardgame_hand_input_select(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player);
void cardgame_view_input(CardgameGame *game, CardgameBoard *board, s32 count);
void cardgame_info_show_row(CardgameGame *game, CardgameBoard *board, s32 side);
s32 cardgame_info_open_windows(CardgameGame *game, CardgameBoard *board, s32 a, s32 card, s32 ticks, s32 step);

/* Screen positions: [NTSC][a][b][x, y] (cardgame_get_window_pos). */
extern s16 cardgame_window_pos[2][3][4][2];
extern u8 cardgame_banners[][2];

/* Timed steps (cardgame_info_open_windows): after `time` ticks, step `kind` opens its board windows. */
typedef struct CardgameIntroStep {
    s16 time;
    s16 kind;
} CardgameIntroStep;
extern CardgameIntroStep cardgame_intro_steps[4];

/* A 2D step passed in one register (x low half, y high half). */
typedef struct CardgameVec2 {
    s16 x;
    s16 y;
} CardgameVec2;

typedef struct CardgamePos {
    s32 x;
    s32 y;
} CardgamePos;

extern CardgamePos cardgame_choose_first_pos[2];
extern CardgamePos cardgame_turn_card_pos[];
extern CardgamePos cardgame_discard_pos[2];
extern s32 cardgame_row_first_card[4]; /* first board card of each row (0, 6, 12) */
extern s32 cardgame_view_board_next[4]; /* cardgame_view_board_update's next state per row */

/* Slot compaction (cardgame_compact_slots_update): per side, the slot moves and the counts. */
typedef struct CardgameSlotMove {
    u8 from;
    u8 to;
} CardgameSlotMove;

/* .bss, in address order (GCC 2.8 emits uninitialized definitions in the order of their first declaration,
 * so they are defined here, before their first use). */
s32 cardgame_hand_title; /* cardgame_hand_update: the message of the info windows' title (popup 0) */
s16 cardgame_view_board_saved[2]; /* cardgame_view_board_update saves both panels' anim_state here */
s32 cardgame_choose_card_title; /* the same for cardgame_choose_card_update */
CardgameSlotMove cardgame_slot_moves[2][6];
u8 cardgame_slot_move_counts[2]; /* moves: entries in cardgame_slot_moves[side] */
u8 cardgame_slot_removed_counts[2]; /* removed: slots flagged in marked */
s32 D_CARDGAME_800A5DC4; /* unreferenced */

extern CardgameVec2 cardgame_popup_pos[2][2]; /* [NTSC][side] */
/* Card kind masks for effect filters (cardgame_mark_selectable_slots). */
extern s16 cardgame_kind_masks[6];
/* Card IDs cardgame_info_show treats specially. */
extern s16 cardgame_bonus_cards[5];

/* Where each side's dealt cards start (cardgame_deal_cards): cardgame_slot_origins, cardgame.h. */
void cardgame_view_board_move_cursor(CardgameGame *game, CardgameBoard *board, s32 which, s32 delta);
s32 cardgame_slot_animate_discard(CardgameGame *game, CardgameBoard *board, s32 i);
void cardgame_slot_to_discard(CardgameGame *game, CardgameBoard *board, s32 side, s32 i);
s32 cardgame_slot_animate_return(CardgameGame *game, CardgameBoard *board, s32 i);
void cardgame_slot_to_hand(CardgameGame *game, CardgameBoard *board, s32 side, s32 i);
void cardgame_set_card_turn_marks(CardgameGame *game, CardgameBoard *board, s32 arg2, s32 i);
void cardgame_game_apply_bonus(CardgameGame *game, CardgameSlot *slot, s32 side, s32 i); /* cardgame_8009D6E0.c */

void cardgame_restore_cpu_deck(CardgameGame *game) {
    s16 *deck = game->players[1].deck;
    CardgamePair pair;
    s32 i;
    s32 j;
    s32 k;
    s16 t;

    for (i = game->players[1].deck_pos; i < 39; i++) {
        for (j = i + 1; j < 40; j++) {
            if (game->cpu_deck_info[i].pos > game->cpu_deck_info[j].pos) {
                t = deck[i];
                deck[i] = deck[j];
                deck[j] = t;
                pair = game->cpu_deck_info[i];
                game->cpu_deck_info[i] = game->cpu_deck_info[j];
                game->cpu_deck_info[j] = pair;
            }
        }
    }
    k = game->cpu_deck_last;
    pair = game->cpu_deck_info[game->choice];
    game->cpu_deck_info[game->choice] = game->cpu_deck_info[k];
    game->cpu_deck_info[k] = pair;
    t = deck[game->choice];
    deck[game->choice] = deck[k];
    deck[k] = t;
    game->choice = k;
}

s32 cardgame_is_card_playable(CardgameGame *game, s32 card) {
    CardPicture pic;
    s32 ok;
    s32 kind;

    card_init(&pic);
    pic.select(game->card_ids[card] + 1);
    ok = 0;
    kind = pic.get_class();
    if (kind != 0 && (game->phase == 7 || (game->phase == 5 && kind == 2))) {
        ok = 1;
    }
    return ok;
}

s32 cardgame_can_play_card(CardgameGame *game, u8 *counts, s32 card) {
    CardPicture pic;
    s32 ok = 0;

    if (game->phase == 6) {
        card_init(&pic);
        pic.select(game->card_ids[card] + 1);
        if (pic.record[3] == 0x10) {
            ok = counts[pic.record[0] - 1] >= pic.record[5];
        }
    } else {
        ok = cardgame_is_card_playable(game, card);
    }
    return ok;
}

s16 cardgame_get_window_pos(s32 a, s32 b, s32 c) {
    return cardgame_window_pos[main_screen_pos][a][b][c];
}

s32 cardgame_info_open_windows(CardgameGame *game, CardgameBoard *board, s32 a, s32 card, s32 ticks, s32 step) {
    if (cardgame_intro_steps[step].time < ticks) {
        switch (cardgame_intro_steps[step].kind) {
        case 0:
            board->open_popup(board, 2, 1, 0, cardgame_get_window_pos(a, 0, 0), cardgame_get_window_pos(a, 0, 1));
            board->open_popup(board, 4, 2, 0, cardgame_get_window_pos(a, 1, 0), cardgame_get_window_pos(a, 1, 1));
            if (card != 0) {
                board->open_popup(board, 0, 0, card, 0, 0x42);
            }
            if (game->effect == 0x9A) {
                board->open_popup(board, 5, 4, 0x24, 0, 0x14);
            }
            break;
        case 1:
            board->open_popup(board, 3, 1, 0, cardgame_get_window_pos(a, 2, 0), cardgame_get_window_pos(a, 2, 1));
            break;
        case 2:
            board->open_popup(board, 1, 3, 0, cardgame_get_window_pos(a, 3, 0), cardgame_get_window_pos(a, 3, 1));
            break;
        }
        if (step < 3) {
            step++;
        }
    }
    return step;
}

void cardgame_ask_play_start(CardgameGame *game, CardgameBoard *board, s32 mode) {
    s32 i;
    s32 found;

    game->choice = 0;
    game->effect_step = 0;
    if (board->panels[0].state == 0) {
        game->display.set_dimmed = 1;
        game->display.request = 1;
        board->open_panels(board);
    }
    game->unk_424 = 0;
    board->dialog.message = mode;
    board->dialog.position = 0;
    found = 0;
    for (i = 0; i < game->players[0].hand_count; i++) {
        if (cardgame_can_play_card(game, game->players[0].points, game->players[0].hand[i]) != 0) {
            found = 1;
            break;
        }
    }
    if (found) {
        board->dialog.answer = 0;
        game->choice = 0;
        game->unk_438 = 2;
    } else {
        board->dialog.answer = 1;
        game->choice = 1;
        game->unk_438 = 3;
    }
}

s32 cardgame_ask_play_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;

    switch (game->effect_step) {
    case 0:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 >= 36 && game->display.state == 0) {
            game->unk_424 = 0;
            if (game->turn != 0 && game->unk_438 == 3) {
                game->effect_step = 8;
            } else {
                game->effect_step = 1;
                board->open_dialog(board, board->dialog.message, game->unk_438, board->dialog.answer,
                               board->dialog.position);
            }
        }
        break;
    case 2:
        if (PAD_PRESSED(13)) {
            board->confirm_dialog(board);
            game->effect_step = 7;
        } else if (PAD_PRESSED(14)) {
            game->choice = 1;
            board->set_dialog_answer(board, 1);
            board->confirm_dialog(board);
            game->effect_step = 7;
        } else if ((PAD_PRESSED(4) || PAD_PRESSED(6)) && game->unk_438 == 2) {
            sound_module.play(0x8004513E);
            game->choice ^= 1;
            board->set_dialog_answer(board, game->choice);
        } else if ((!PAD_HELD(11) && PAD_PRESSED(10)) || (!PAD_HELD(10) && PAD_PRESSED(11))) {
            board->close_dialog(board);
            game->effect_step = 6;
        } else if (PAD_PRESSED(12)) {
            board->close_dialog(board);
            game->unk_424 = 0;
            game->effect_step = 3;
        }
        break;
    case 6:
        if (board->dialog.state == 0) {
            board->dialog.position ^= 2;
            board->open_dialog(board, board->dialog.message, game->unk_438, board->dialog.answer,
                           board->dialog.position);
            game->effect_step = 1;
        }
        break;
    case 7:
        if (board->dialog.state == 0) {
            if (game->choice == 0) {
                game->effect_step = 9;
                game->display.request = 2;
                board->close_panels(board);
            } else {
                game->effect_step = 8;
            }
        }
        break;
    case 3:
        switch (game->unk_424) {
        case 0:
            if (board->dialog.state == 0) {
                game->display.request = 2;
                board->close_panels(board);
                game->unk_424 = 1;
            }
            break;
        case 1:
            if (board->panels[0].state == 0 && game->display.state == 0) {
                game->effect_state = 3;
                game->menu.state = 0;
                game->effect_step = 4;
            }
            break;
        }
        break;
    case 4:
        game->display.set_dimmed = 1;
        game->display.request = 1;
        board->open_panels(board);
        game->effect_step = 5;
        game->unk_424 = 0;
        break;
    case 5:
        switch (game->unk_424) {
        case 0:
            if (game->display.state == 0 && board->panels[0].state == 2) {
                board->open_dialog(board, board->dialog.message, game->unk_438, board->dialog.answer,
                               board->dialog.position);
                game->unk_424 = 1;
            }
            break;
        case 1:
            if (board->dialog.state == 2) {
                game->effect_step = 2;
            }
            break;
        }
        break;
    case 1:
        if (board->dialog.state == 2) {
            game->effect_step = 2;
        }
        break;
    case 9:
        if (board->panels[0].state == 0 && game->display.state == 0) {
            if (game->choice == 0) {
                done = 1;
            } else {
                done = 2;
            }
        }
        break;
    case 8:
        if (game->choice == 0) {
            done = 1;
        } else {
            done = 2;
        }
        break;
    }
    return done;
}

void cardgame_info_clear(CardgameGame *game, CardgameBoard *board) {
    board->popups[1].show_value = 0;
    board->popups[1].value = 0;
    board->popups[2].value = 0;
    board->popups[3].value = 0;
    board->popups[4].value = 0;
}

void cardgame_info_show(CardgameGame *game, CardgameBoard *board, s32 first) {
    CardPicture pic;
    s32 id;
    s32 found;
    s32 i;
    s32 t;

    cardgame_info_clear(game, board);
    if (board->cards[first + game->cursor].is_digimon != 0) {
        board->popups[1].value |= board->cards[first + game->cursor].level;
        board->popups[1].value |= board->cards[first + game->cursor].color << 4;
        board->popups[1].show_value = 1;
    }
    found = 0;
    if (board->cards[first + game->cursor].style != 3) {
        id = game->card_ids[board->cards[first + game->cursor].card] + 1;
        board->popups[2].value = id;
        for (i = 0; i < 5; i++) {
            if (cardgame_bonus_cards[i] == game->card_ids[board->cards[first + game->cursor].card]) {
                found = 1;
            }
        }
        if (board->cards[first + game->cursor].is_digimon != 0 && !found) {
            board->popups[4].value = 500;
            board->popups[4].card_stats[2] = 1;
            board->popups[4].card_stats[0] = board->cards[first + game->cursor].attack;
            board->popups[4].card_stats[1] = board->cards[first + game->cursor].hp;
        } else {
            board->popups[4].card_stats[2] = 0;
            board->popups[4].value = id;
        }
        t = game->card_ids[board->cards[first + game->cursor].card];
        card_init(&pic);
        pic.select(t + 1);
        switch (pic.record[6]) {
        case 0:
            board->popups[3].value = 0;
            break;
        case 1:
            board->popups[3].value = 0x25;
            break;
        case 2:
            board->popups[3].value = 0x26;
            break;
        case 3:
            board->popups[3].value = 0x27;
            break;
        case 4:
            board->popups[3].value = 0x28;
            break;
        case 5:
            board->popups[3].value = 0x29;
            break;
        }
    } else {
        board->popups[4].card_stats[2] = 0;
        board->popups[4].value = 500;
    }
}

void cardgame_info_show_row(CardgameGame *game, CardgameBoard *board, s32 side) {
    s32 first = 0;

    switch (side) {
    case 1:
        first = 6;
        break;
    case 2:
        first = 12;
        break;
    }
    cardgame_info_show(game, board, first);
}

void cardgame_hand_start(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player) {
    s32 i;

    if (player->hand_count != 0) {
        for (i = 0; i < 40; i++) {
            game->selectable[i] = 0;
            game->marked[i] = 0;
            if (i < player->hand_count) {
                if (cardgame_can_play_card(game, player->points, player->hand[i]) != 0) {
                    game->selectable[i] = 1;
                    game->display.dimmed[i] = 0;
                } else {
                    game->display.dimmed[i] = 1;
                }
            }
        }
    } else {
        game->display.dimmed[0] = 0;
    }
    board->reset_panels(board);
    if (player->side == 0) {
        game->display.request = 5;
        board->open_side_panel(board, player->side);
    } else {
        game->display.request = 11;
    }
    game->next_step = 1;
    cardgame_info_clear(game, board);
    game->cursor = 0;
    game->choice = -1;
}

void cardgame_view_start(CardgameGame *game, CardgameBoard *board, s32 n, s32 request) {
    s32 i;

    if (n > 0) {
        for (i = 0; i < 40; i++) {
            game->display.dimmed[i] = 0;
        }
    } else {
        game->display.dimmed[0] = 0;
    }
    cardgame_info_clear(game, board);
    game->next_step = 1;
    game->display.request = request;
    game->cursor = 0;
    game->choice = -1;
}

void cardgame_view_move_cursor(CardgameGame *game, CardgameBoard *board, s32 count, s32 delta) {
    sound_module.play(0x4001B);
    board->slide_card(board, game->cursor, 5, board->get_card_x(count, game->cursor) + 0x1800, 0x6100);
    board->cards[game->cursor].highlight &= ~1;
    board->cards[game->cursor].zooming = 0;
    game->cursor += delta;
    board->slide_card(board, game->cursor, 1, board->get_card_x(count, game->cursor) + 0x1800, 0x5C00);
    board->cards[game->cursor].highlight |= 1;
    board->cards[game->cursor].zooming = 1;
}

void cardgame_hand_move_cursor(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player, s32 delta) {
    sound_module.play(0x4001B);
    board->slide_card(board, game->cursor, 5, board->get_card_x(player->hand_count, game->cursor) + 0x1800, 0x6100);
    board->cards[game->cursor].highlight &= ~1;
    board->cards[game->cursor].zooming = 0;
    game->cursor += delta;
    board->slide_card(board, game->cursor, 1, board->get_card_x(player->hand_count, game->cursor) + 0x1800, 0x5C00);
    board->cards[game->cursor].highlight |= 1;
    board->cards[game->cursor].zooming = 1;
}

void cardgame_hand_update_playable(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player) {
    s32 i;

    for (i = 0; i < player->hand_count; i++) {
        if (cardgame_can_play_card(game, player->points, player->hand[i]) != 0 && game->marked[i] == 0) {
            game->selectable[i] = 1;
            board->cards[i].dimmed = 0;
        } else {
            game->selectable[i] = 0;
            if (game->marked[i] == 0) {
                board->cards[i].dimmed = 1;
            } else {
                board->cards[i].dimmed = 0;
            }
        }
    }
}

s32 cardgame_hand_is_full(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player) {
    s32 ok = 1;
    s32 i;

    if (game->selected < 6) {
        for (i = 0; i < player->hand_count; i++) {
            if (game->selectable[i] != 0) {
                ok = 0;
                break;
            }
        }
    }
    return ok;
}

void cardgame_player_change_points(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player, s32 add, s32 card) {
    CardPicture pic;
    s32 kind;

    card_init(&pic);
    pic.select(game->card_ids[card] + 1);
    if (pic.record[0] < 6) {
        if (add != 0) {
            player->points[pic.record[0] - 1] += pic.record[5];
        } else {
            player->points[pic.record[0] - 1] -= pic.record[5];
        }
        kind = pic.record[0] - 1;
        board->set_panel_value(board, player->side, kind, player->points[kind]);
    }
}

s32 cardgame_hand_toggle_card(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player) {
    s32 i = game->cursor;
    s32 result = 0;

    if (game->selectable[i] != 0) {
        if (game->selected < 6) {
            cardgame_player_change_points(game, board, player, 0, player->hand[i]);
            game->marked[game->cursor] = 1;
            board->flash_card(board, game->cursor);
            board->cards[game->cursor].highlight |= 2;
            result = 1;
            game->selected++;
        }
    } else if (game->marked[i] != 0) {
        cardgame_player_change_points(game, board, player, 1, player->hand[i]);
        game->marked[game->cursor] = 0;
        board->cards[game->cursor].highlight &= ~2;
        result = 2;
        game->selected--;
    }
    return result;
}

void cardgame_hand_input_pick(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player) {
    board->cards[game->cursor].highlight |= 1;
    board->cards[game->cursor].zooming = 1;
    if (PAD_PRESSED(14)) {
        sound_module.play(0x800450BD);
        game->choice = -1;
        game->next_step = 14;
    }
    if (PAD_PRESSED(12)) {
        game->next_step = 11;
    }
    if (player->hand_count != 0) {
        if ((pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 7))) |
            (pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 7)))) {
            if (game->cursor > 0) {
                cardgame_hand_move_cursor(game, board, player, -1);
            }
        } else if ((pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 5))) |
                   (pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 5)))) {
            if (game->cursor < player->hand_count - 1) {
                cardgame_hand_move_cursor(game, board, player, 1);
            }
        } else if (PAD_PRESSED(13) && game->selectable[game->cursor] != 0) {
            game->next_step = 5;
            game->choice = game->cursor;
            game->marked[game->choice] = 1;
        }
    }
}

void cardgame_hand_input_select(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player) {
    board->cards[game->cursor].highlight |= 1;
    board->cards[game->cursor].zooming = 1;
    if (PAD_PRESSED(15)) {
        game->next_step = 6;
    }
    if (PAD_PRESSED(12)) {
        game->next_step = 11;
    }
    if (player->hand_count != 0) {
        if ((pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 7))) |
            (pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 7)))) {
            if (game->cursor > 0) {
                cardgame_hand_move_cursor(game, board, player, -1);
            }
        } else if ((pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 5))) |
                   (pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 5)))) {
            if (game->cursor < player->hand_count - 1) {
                cardgame_hand_move_cursor(game, board, player, 1);
            }
        } else if (PAD_PRESSED(13) && cardgame_hand_toggle_card(game, board, player) != 0) {
            game->next_step = 4;
        }
    }
}

void cardgame_view_input(CardgameGame *game, CardgameBoard *board, s32 count) {
    board->cards[game->cursor].highlight |= 1;
    board->cards[game->cursor].zooming = 1;
    if (PAD_PRESSED(14)) {
        sound_module.play(0x800450BD);
        game->choice = -1;
        game->next_step = 10;
    }
    if (count != 0) {
        if (((pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 7))) |
             (pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 7)))) &&
            game->cursor > 0) {
            cardgame_view_move_cursor(game, board, count, -1);
        }
        if (((pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 5))) |
             (pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 5)))) &&
            game->cursor < count - 1) {
            cardgame_view_move_cursor(game, board, count, 1);
        }
    }
}

s32 cardgame_hand_pulse_card(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;

    switch (game->unk_424) {
    case 0:
    default:
        board->zoom_card(board, game->cursor, 5, 0x1400, 0x1400);
        game->unk_428 = 0;
        game->unk_424++;
        break;
    case 1:
        game->unk_428 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_428 >= 5) {
            game->unk_424++;
        }
        break;
    case 2:
        board->zoom_card(board, game->cursor, 5, 0x1000, 0x1000);
        game->unk_428 = 0;
        game->unk_424++;
        break;
    case 3:
        game->unk_428 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_428 >= 5) {
            done = 1;
        }
        break;
    }
    return done;
}

s32 cardgame_hand_update(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player) {
    s32 done = -1;
    s32 n;
    s32 i;
    s32 j;
    s32 k;
    s32 m;
    s32 t;

    if (game->next_step != 0) {
        switch (game->next_step) {
        case 1:
        case 2:
            switch (game->effect) {
            case 0x99:
                if (player->side != 0) {
                    cardgame_hand_title = 0x1C;
                } else {
                    cardgame_hand_title = 0x19;
                }
                break;
            case 0x9D:
                cardgame_hand_title = 0x1C;
                break;
            case 0x9C:
                cardgame_hand_title = 0x1B;
                break;
            case 0x9A:
            case 0x9B:
                cardgame_hand_title = 0x19;
                break;
            }
            game->unk_42C = 0;
            game->unk_428 = 0;
            game->unk_424 = 0;
            game->choice = 0;
            game->selected = 0;
            break;
        case 5:
            sound_module.play(0x4001C);
            game->unk_428 = 0;
            game->unk_424 = 0;
            break;
        case 4:
            sound_module.play(0x4001C);
            break;
        case 14:
            if (player->side == 0) {
                game->display.request = 6;
                board->close_side_panel(board, 0);
            } else {
                game->display.request = 12;
            }
            game->unk_428 = 0;
            game->unk_424 = 0;
            board->close_popup(board, 4);
            board->close_popup(board, 0);
            board->close_popup(board, 1);
            board->close_popup(board, 2);
            board->close_popup(board, 3);
            break;
        case 10:
            game->unk_428 = 0;
            game->unk_424 = 0;
            game->display.request = game->display.reopen;
            board->close_popup(board, 4);
            board->close_popup(board, 0);
            board->close_popup(board, 1);
            board->close_popup(board, 2);
            board->close_popup(board, 3);
            break;
        case 6:
            game->unk_428 = 0;
            game->unk_424 = 0;
            cardgame_hand_title = board->popups[0].value;
            for (m = 0; m < player->hand_count; m++) {
                if (game->marked[m] != 0) {
                    board->cards[m].dimmed = 0;
                    board->cards[m].highlight |= 4;
                } else {
                    board->cards[m].dimmed = 1;
                }
            }
            break;
        case 7:
            game->choice = 0;
            game->unk_424 = 0;
            break;
        case 3:
        case 8:
            game->unk_428 = 0;
            game->unk_424 = 0;
            break;
        case 11:
            game->display.request = game->display.reopen;
            cardgame_hand_title = board->popups[0].value;
            game->unk_424 = game->display.reopen;
            board->close_popup(board, 4);
            board->close_popup(board, 0);
            board->close_popup(board, 1);
            board->close_popup(board, 2);
            board->close_popup(board, 3);
            if (game->effect == 0x9A) {
                board->close_popup(board, 5);
            }
            if (player->side == 0) {
                board->close_side_panel(board, 0);
            }
            break;
        case 13:
            cardgame_hand_title = 0x19;
            game->unk_428 = 0;
            game->display.request = game->unk_424 - 1;
            switch (game->effect) {
            case 0x99:
            case 0x9A:
            case 0x9D:
                for (k = 0; k < player->hand_count; k++) {
                    if (cardgame_can_play_card(game, player->points, player->hand[k]) != 0) {
                        game->display.dimmed[k] = 0;
                    } else {
                        game->display.dimmed[k] = 1;
                    }
                }
                break;
            }
            board->open_popup(board, 0, 0, cardgame_hand_title, 0, 0x42);
            board->open_popup(board, 2, 1, 0, 0x82, 0xA5);
            board->open_popup(board, 4, 2, 0, 0x86, 0x31);
            board->open_popup(board, 3, 1, 0, 0x82, 0x90);
            board->open_popup(board, 1, 3, 0, 0xFD, 0x90);
            if (game->effect == 0x9A) {
                board->open_popup(board, 5, 4, 0x24, 0, 0x14);
            }
            if (player->side == 0) {
                board->open_side_panel(board, 0);
            }
            break;
        case 9:
            game->unk_428 = 0;
            game->unk_424 = 0;
            for (k = 0; k < player->hand_count; k++) {
                if (cardgame_can_play_card(game, player->points, player->hand[k]) != 0) {
                    board->cards[k].dimmed = 0;
                } else {
                    board->cards[k].dimmed = 1;
                }
                if (game->marked[k] != 0) {
                    board->cards[k].dimmed = 0;
                    board->cards[k].highlight &= ~4;
                }
            }
            break;
        case 12:
            break;
        }
        game->effect_step = game->next_step;
        game->next_step = 0;
    }
    switch (game->effect_step) {
    case 1:
        game->unk_42C = cardgame_info_open_windows(game, board, 2, cardgame_hand_title, game->unk_424, game->unk_42C);
        cardgame_info_show(game, board, 0);
        if (game->display.count * 4 + 14 < game->unk_424) {
            game->next_step = 3;
            board->slide_card(board, 0, 5, 0x1800, 0x5C00);
            board->cards[game->cursor].highlight |= 1;
            board->cards[game->cursor].zooming = 1;
        }
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        break;
    case 2:
        board->zoom_card(board, 0, 5, 0x1000, 0x1000);
        if (game->unk_424 >= 11) {
            game->next_step = 3;
            board->slide_card(board, 0, 5, 0x1800, 0x5C00);
            board->cards[game->cursor].highlight |= 1;
            board->cards[game->cursor].zooming = 1;
        }
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        break;
    case 3:
        switch (game->effect) {
        case 0x99:
            cardgame_hand_input_pick(game, board, player);
            break;
        case 0x9A:
        case 0x9D:
            cardgame_hand_input_select(game, board, player);
            break;
        case 0x9B:
        case 0x9C:
            cardgame_view_input(game, board, game->display.count);
            break;
        }
        cardgame_info_show(game, board, 0);
        break;
    case 4:
        if (board->cards[game->cursor].state == 1) {
            cardgame_hand_update_playable(game, board, player);
            if (cardgame_hand_is_full(game, board, player) != 0) {
                game->next_step = 6;
            } else {
                game->next_step = 3;
            }
        }
        cardgame_info_show(game, board, 0);
        break;
    case 5:
        if (cardgame_hand_pulse_card(game, board) != 0) {
            game->next_step = 14;
        }
        cardgame_info_show(game, board, 0);
        break;
    case 14:
        if (player->hand_count * 4 + 5 < game->unk_424) {
            game->next_step = 0;
            game->new_effect = 0;
            game->effect = 0;
            game->new_effect = 0;
            done = game->choice != -1;
        }
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        break;
    case 10:
        if (game->display.count * 4 + 14 < game->unk_424) {
            done = 0;
            game->next_step = 0;
            game->new_effect = 0;
            game->effect = 0;
            game->new_effect = 0;
        }
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        break;
    case 6:
        switch (game->unk_424) {
        case 0:
            board->close_popup(board, 4);
            board->close_popup(board, 0);
            board->close_popup(board, 5);
            board->move_card(board, game->cursor, 5, board->get_card_x(player->hand_count, game->cursor) + 0x1800, 0x6100);
            board->cards[game->cursor].highlight &= ~1;
            board->cards[game->cursor].zooming = 0;
            break;
        case 6:
            board->open_dialog(board, 15, 1, 0, 2);
            break;
        }
        game->unk_424++;
        if (game->unk_424 >= 19) {
            game->next_step = 7;
        }
        break;
    case 7:
        if (PAD_PRESSED(13)) {
            board->confirm_dialog(board);
            if (game->choice == 0) {
                game->next_step = 8;
            } else {
                game->next_step = 9;
            }
        } else if (PAD_PRESSED(4) || PAD_PRESSED(6)) {
            game->choice ^= 1;
            sound_module.play(0x8004513E);
            board->set_dialog_answer(board, game->choice);
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            game->choice = 1;
            board->set_dialog_answer(board, 1);
            board->confirm_dialog(board);
            game->next_step = 9;
        }
        break;
    case 8:
        switch (game->unk_424) {
        case 20:
            n = 0;
            for (i = 0; i < player->hand_count; i++) {
                if (game->marked[i] != 0) {
                    board->zoom_card(board, i, 6, 0x1400, 0x1400);
                    n++;
                }
            }
            if (n != 0) {
                sound_module.play(0x4001C);
            }
            break;
        case 25:
            for (j = 0; j < player->hand_count; j++) {
                if (game->marked[j] != 0) {
                    board->zoom_card(board, j, 6, 0x1000, 0x1000);
                }
            }
            break;
        case 35:
            if (player->side == 0) {
                game->display.request = 6;
                board->close_side_panel(board, 0);
            } else {
                game->display.request = 12;
            }
            board->close_popup(board, 1);
            board->close_popup(board, 2);
            board->close_popup(board, 3);
            break;
        }
        game->unk_424++;
        if (game->unk_424 >= 46 && game->display.state == 0) {
            done = 1;
        }
        break;
    case 9:
        switch (game->unk_424) {
        case 6:
            break;
        case 18:
            board->open_popup(board, 0, 0, cardgame_hand_title, 0, 0x42);
            board->open_popup(board, 4, 2, 0, 0x86, 0x31);
            board->open_popup(board, 5, 4, 0x24, 0, 0x14);
            cardgame_info_show(game, board, 0);
            break;
        }
        game->unk_424++;
        if (game->unk_424 >= 31) {
            game->next_step = 3;
            board->move_card(board, game->cursor, 5, board->get_card_x(player->hand_count, game->cursor) + 0x1800, 0x5C00);
            board->cards[game->cursor].highlight |= 1;
            board->cards[game->cursor].zooming = 1;
        }
        break;
    case 11:
        if (board->panels[0].state == 0 && game->display.state == 0) {
            game->next_step = 12;
            game->menu.state = 0;
            game->effect_state = 3;
        }
        break;
    case 12:
        game->next_step = 13;
        break;
    case 13:
        if ((player->side == 0 && board->panels[0].state == 2) || (player->side != 0 && ++game->unk_428 >= 11)) {
            if (game->display.state == 0) {
                game->next_step = 3;
                board->slide_card(board, game->cursor, 5, board->get_card_x(player->hand_count, game->cursor) + 0x1800, 0x5C00);
                board->cards[game->cursor].highlight |= 1;
                board->cards[game->cursor].zooming = 1;
                for (t = 0; t < player->hand_count; t++) {
                    if (game->marked[t] == 1) {
                        board->cards[t].dimmed = 0;
                        board->cards[t].highlight |= 2;
                    }
                }
            }
        }
        cardgame_info_show(game, board, 0);
        break;
    }
    return done;
}

void cardgame_reveal_start(CardgameGame *game, CardgameBoard *board) {
    board->reset_panels(board);
    game->choice = 0;
    game->effect_step = 1;
    board->open_panels(board);
    game->display.set_dimmed = 1;
    game->display.face_down = 1;
    game->display.request = 1;
}

s32 cardgame_reveal_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;
    s32 more = 0;
    s32 step;

    switch (game->effect_step) {
    case 1:
        if (board->panels[0].state == 2 && game->display.state == 0) {
            if (game->slots[0].count + game->slots[1].count == 0) {
                game->effect_step = 3;
                game->unk_424 = 0;
                game->unk_428 = 0;
                game->unk_42C = 0;
                game->unk_430 = 0;
                game->unk_434 = 0;
            } else {
                game->effect_step = 2;
                game->unk_424 = 0;
                game->unk_428 = 0;
                game->unk_434 = 0;
                if (game->slots[0].count > game->slots[1].count) {
                    game->unk_42C = game->slots[0].count;
                } else {
                    game->unk_42C = game->slots[1].count;
                }
                game->unk_42C = game->unk_42C * 6 + 18;
            }
        }
        break;
    case 2:
        if (game->unk_434 >= 6) {
            if (game->unk_428 < game->slots[1].count) {
                board->flip_card(board, game->unk_428 + 6);
            }
            game->unk_428++;
            game->unk_434 -= 6;
        }
        if (game->unk_424 > game->unk_42C) {
            game->effect_step = 3;
            game->unk_424 = 0;
            game->unk_428 = 0;
            game->unk_42C = 0;
            game->unk_430 = 0;
            game->unk_434 = 0;
        }
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        game->unk_434 += gfx_module.funcs.get_frame_ticks();
        break;
    case 3:
        step = game->players[0].attack / 60;
        game->unk_428 += step == 0 ? 1 : step;
        if (game->players[0].attack < game->unk_428) {
            game->unk_428 = game->players[0].attack;
        } else {
            more = 1;
        }
        step = game->players[0].hp / 60;
        game->unk_42C += step == 0 ? 1 : step;
        if (game->players[0].hp < game->unk_42C) {
            game->unk_42C = game->players[0].hp;
        } else {
            more = 1;
        }
        step = game->players[1].attack / 60;
        game->unk_430 += step == 0 ? 1 : step;
        if (game->players[1].attack < game->unk_430) {
            game->unk_430 = game->players[1].attack;
        } else {
            more = 1;
        }
        step = game->players[1].hp / 60;
        game->unk_434 += step == 0 ? 1 : step;
        if (game->players[1].hp < game->unk_434) {
            game->unk_434 = game->players[1].hp;
        } else {
            more = 1;
        }
        if (game->unk_424++ >= 61) {
            game->effect_step = 4;
            game->unk_424 = 0;
            game->unk_428 = game->players[0].attack;
            game->unk_42C = game->players[0].hp;
            game->unk_430 = game->players[1].attack;
            game->unk_434 = game->players[1].hp;
        } else if (more) {
            sound_module.play(0x800452C6);
        }
        board->set_panel_value(board, 0, 8, game->unk_428);
        board->set_panel_value(board, 0, 9, game->unk_42C);
        board->set_panel_value(board, 1, 8, game->unk_430);
        board->set_panel_value(board, 1, 9, game->unk_434);
        break;
    case 4:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            game->unk_424 = 90;
        }
        game->unk_424++;
        if (game->unk_424 >= 91) {
            board->close_panels(board);
            game->effect_step = 5;
            game->display.request = 2;
        }
        break;
    case 5:
        if (board->panels[0].state == 0 && game->display.state == 0) {
            done = 1;
        }
        break;
    }
    return done;
}

void cardgame_banner_start(CardgameGame *game, CardgameBoard *board, s32 i) {
    board->open_dialog(board, cardgame_banners[i][1], 0, 0, 1);
    board->open_popup(board, 5, 5, cardgame_banners[i][0], 0, 0x42);
    game->effect_step = 1;
}

s32 cardgame_banner_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;

    switch (game->effect_step) {
    case 1:
        if (board->popups[5].state == 2) {
            game->effect_step = 2;
        }
        break;
    case 2:
        if (((pad_state.get_pressed(0) >> pad_state.get_button_map(0, 13)) & 1) ||
            ((pad_state.get_pressed(0) >> pad_state.get_button_map(0, 14)) & 1)) {
            game->effect_step = 3;
            board->close_popup(board, 5);
            board->close_dialog(board);
        }
        break;
    case 3:
        if (board->popups[5].state == 0) {
            game->effect_step = 4;
        }
        break;
    case 4:
        done = 1;
        break;
    }
    return done;
}

void cardgame_choose_first_move_cursor(CardgameGame *game, CardgameBoard *board) {
    sound_module.play(0x4001B);
    board->slide_card(board, game->cursor, 5, cardgame_choose_first_pos[game->cursor].x,
                   cardgame_choose_first_pos[game->cursor].y);
    board->cards[game->cursor].highlight &= ~1;
    board->cards[game->cursor].zooming = 0;
    game->cursor ^= 1;
    board->slide_card(board, game->cursor, 1, cardgame_choose_first_pos[game->cursor].x,
                   cardgame_choose_first_pos[game->cursor].y - 0x500);
    board->cards[game->cursor].highlight |= 1;
    board->cards[game->cursor].zooming = 1;
}

void cardgame_choose_first_start(CardgameGame *game, CardgameBoard *board) {
    s32 first = pad_random.next() & 1;

    game->unk_434 = first;
    game->unk_430 = 0;
    game->unk_42C = 0;
    game->unk_428 = 0;
    game->unk_424 = 0;
    board->place_card(board, 0, 0x7400, 0x6100);
    board->place_card(board, 1, 0xA400, 0x6100);
    if (first) {
        board->set_card(board, 0, 0x57);
        board->set_card(board, 1, 0x58);
    } else {
        board->set_card(board, 1, 0x57);
        board->set_card(board, 0, 0x58);
    }
    board->cards[0].style = 2;
    board->cards[1].style = 2;
    board->cards[0].scale_x = 0;
    board->cards[0].dimmed = 0;
    board->cards[0].highlight = 0;
    board->cards[1].scale_x = 0;
    board->cards[1].dimmed = 0;
    board->cards[1].highlight = 0;
    game->cursor = 0;
    game->choice = 0;
    game->effect_step = 1;
}

s32 cardgame_choose_first_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;

    switch (game->effect_step) {
    case 1:
        switch (game->unk_424) {
        case 0:
            board->zoom_card(board, 0, 10, 0x1000, 0x1000);
            break;
        case 4:
            board->zoom_card(board, 1, 10, 0x1000, 0x1000);
            break;
        }
        game->unk_424++;
        if (game->unk_424 >= 15) {
            game->effect_step = 2;
            board->slide_card(board, 0, 5, 0x7400, 0x5C00);
            board->cards[0].zooming = 1;
            board->cards[0].highlight |= 1;
        }
        break;
    case 2:
        board->cards[game->cursor].highlight |= 1;
        board->cards[game->cursor].zooming = 1;
        if (((pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 7))) |
             (pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 7)))) ||
            ((pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 5))) |
             (pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 5))))) {
            cardgame_choose_first_move_cursor(game, board);
        } else if (PAD_PRESSED(13)) {
            game->effect_step = 3;
            game->choice = game->cursor;
            game->marked[game->choice] = 1;
            game->unk_42C = 0;
            game->unk_428 = 0;
            game->unk_424 = 0;
            sound_module.play(0x4001C);
        }
        break;
    case 3:
        if (cardgame_hand_pulse_card(game, board) != 0) {
            game->effect_step = 4;
            game->unk_42C = 0;
            game->unk_428 = 0;
            game->unk_424 = 0;
            board->flip_card(board, game->cursor);
        }
        break;
    case 4:
        if (game->unk_424 == 20) {
            board->flip_card(board, game->cursor ^ 1);
            board->open_popup(board, 5, 5, game->unk_434 == game->choice ? 0x44 : 0x43, 0, 0x42);
            sound_module.play(0x40019);
        }
        if (game->unk_424 >= 31 && (PAD_PRESSED(13) || PAD_PRESSED(14))) {
            game->unk_424 = 90;
        }
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 >= 91) {
            game->effect_step = 5;
            game->unk_42C = 0;
            game->unk_428 = 0;
            game->unk_424 = 0;
            board->close_popup(board, 5);
        }
        break;
    case 5:
        switch (game->unk_424) {
        case 0:
            board->zoom_card(board, 0, 5, 0, 0x1000);
            break;
        case 4:
            board->zoom_card(board, 1, 5, 0, 0x1000);
            break;
        }
        game->unk_424++;
        if (game->unk_424 >= 15) {
            game->effect_step = 6;
        }
        break;
    case 6:
        if (game->unk_434 == game->choice) {
            game->choice = 1;
        } else {
            game->choice = 0;
        }
        done = 1;
        break;
    }
    return done;
}

void cardgame_view_board_start(CardgameGame *game, CardgameBoard *board) {
    game->next_step = 1;
}

s32 cardgame_view_board_get_duration(CardgameGame *game, CardgameBoard *board) {
    s32 n;

    if (game->slots[0].count > game->slots[1].count) {
        n = game->slots[0].count;
    } else {
        n = game->slots[1].count;
    }
    if (n < 3) {
        n = 3;
    }
    return n * 8 + 14;
}

void cardgame_view_board_move_cursor(CardgameGame *game, CardgameBoard *board, s32 which, s32 delta) {
    s32 base = 0;

    sound_module.play(0x4001B);
    switch (which) {
    case 1:
        base = 6;
        break;
    case 2:
        base = 12;
        break;
    }
    board->cards[base + game->cursor].highlight &= ~1;
    board->cards[base + game->cursor].zooming = 0;
    game->cursor += delta;
    board->cards[base + game->cursor].highlight |= 1;
    board->cards[base + game->cursor].zooming = 1;
}

s32 cardgame_view_board_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;

    if (game->next_step != 0) {
        switch (game->next_step) {
        case 1:
            cardgame_view_board_saved[0] = board->panels[0].anim_state;
            cardgame_view_board_saved[1] = board->panels[1].anim_state;
            board->panels[0].anim_state = 0;
            board->panels[1].anim_state = 0;
            game->display.set_dimmed = 1;
            game->display.request = 1;
            game->unk_42C = cardgame_view_board_get_duration(game, board);
            game->cursor = 0;
            board->reset_panels(board);
            board->open_panels(board);
            cardgame_info_clear(game, board);
            break;
        case 2:
            cardgame_info_clear(game, board);
            game->unk_428 = 0;
            game->unk_424 = 0;
            game->unk_434 = 0;
            break;
        case 4:
            cardgame_info_clear(game, board);
            game->unk_428 = 0;
            game->unk_424 = 0;
            game->unk_434 = 1;
            break;
        case 3:
            cardgame_info_clear(game, board);
            game->unk_428 = 0;
            game->unk_424 = 0;
            game->unk_434 = 2;
            break;
        case 8:
            board->cards[game->cursor].highlight &= ~1;
            board->cards[game->cursor].zooming = 0;
            board->close_popup(board, 4);
            board->close_popup(board, 1);
            board->close_popup(board, 2);
            board->close_popup(board, 3);
            game->unk_428 = 0;
            game->unk_424 = 0;
            break;
        case 9:
            board->cards[game->cursor + 12].highlight &= ~1;
            board->cards[game->cursor + 12].zooming = 0;
            board->close_popup(board, 4);
            board->close_popup(board, 1);
            board->close_popup(board, 2);
            board->close_popup(board, 3);
            game->unk_428 = 0;
            game->unk_424 = 0;
            break;
        case 10:
            board->cards[game->cursor + 6].highlight &= ~1;
            board->cards[game->cursor + 6].zooming = 0;
            board->close_popup(board, 4);
            board->close_popup(board, 1);
            board->close_popup(board, 2);
            board->close_popup(board, 3);
            game->unk_428 = 0;
            game->unk_424 = 0;
            break;
        case 11:
            board->close_panels(board);
            board->close_popup(board, 4);
            board->close_popup(board, 1);
            board->close_popup(board, 2);
            board->close_popup(board, 3);
            game->display.request = 2;
            break;
        case 12:
            board->open_dialog(board, 0x17, 0, 0, 1);
            break;
        case 14:
            board->close_dialog(board);
            board->close_panels(board);
            break;
        case 15:
            break;
        }
        game->effect_step = game->next_step;
        game->next_step = 0;
    }
    switch (game->effect_step) {
    case 1:
        if (board->panels[0].state == 2 && game->display.state == 0) {
            if (game->slots[0].count != 0) {
                game->next_step = 2;
                game->unk_434 = 0;
            } else if (game->slots[1].count != 0) {
                game->next_step = 4;
                game->unk_434 = 1;
            } else if (game->turn > 0) {
                game->next_step = 3;
                game->unk_434 = 2;
            } else {
                game->next_step = 12;
                game->unk_434 = 3;
            }
        }
        break;
    case 2:
    case 3:
    case 4:
        game->unk_428 = cardgame_info_open_windows(game, board, game->unk_434, 0, game->unk_424, game->unk_428);
        cardgame_info_show(game, board, cardgame_row_first_card[game->unk_434]);
        game->unk_424++;
        if (game->unk_424 >= 11) {
            cardgame_view_board_move_cursor(game, board, game->unk_434, 0);
            game->next_step = cardgame_view_board_next[game->unk_434];
        }
        break;
    case 5:
        if (PAD_PRESSED(4) && (game->turn > 0 || game->slots[1].count != 0)) {
            game->next_step = 8;
        }
        if (((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 5))) |
             (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 5)))) &&
            game->cursor < game->slots[0].count - 1) {
            cardgame_view_board_move_cursor(game, board, 0, 1);
        }
        if (((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 7))) |
             (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 7)))) &&
            game->cursor > 0) {
            cardgame_view_board_move_cursor(game, board, 0, -1);
        }
        cardgame_info_show_row(game, board, 0);
        if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            game->next_step = 11;
        }
        break;
    case 6:
        if (PAD_PRESSED(6) && (game->turn > 0 || game->slots[0].count != 0)) {
            game->next_step = 10;
        }
        if (((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 5))) |
             (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 5)))) &&
            game->cursor < game->slots[1].count - 1) {
            cardgame_view_board_move_cursor(game, board, 1, 1);
        }
        if (((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 7))) |
             (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 7)))) &&
            game->cursor > 0) {
            cardgame_view_board_move_cursor(game, board, 1, -1);
        }
        cardgame_info_show_row(game, board, 1);
        if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            game->next_step = 11;
        }
        break;
    case 7:
        if (PAD_PRESSED(4)) {
            if (game->slots[1].count != 0) {
                game->unk_42C = 1;
                game->next_step = 9;
            }
        } else if (PAD_PRESSED(6) && game->slots[0].count != 0) {
            game->unk_42C = 0;
            game->next_step = 9;
        }
        if (((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 5))) |
             (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 5)))) &&
            game->cursor < game->turn - 1) {
            cardgame_view_board_move_cursor(game, board, 2, 1);
        }
        if (((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 7))) |
             (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 7)))) &&
            game->cursor > 0) {
            cardgame_view_board_move_cursor(game, board, 2, -1);
        }
        if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            game->next_step = 11;
        }
        cardgame_info_show_row(game, board, 2);
        break;
    case 8:
        game->unk_424++;
        if (game->unk_424 >= 11) {
            if (game->turn > 0) {
                game->next_step = 3;
                game->cursor /= 2;
                if (game->turn - 1 < game->cursor) {
                    game->cursor = game->turn - 1;
                }
            } else if (game->slots[1].count != 0) {
                game->next_step = 4;
                if (game->slots[1].count - 1 < game->cursor) {
                    game->cursor = game->slots[1].count - 1;
                }
            }
        }
        break;
    case 9:
        game->unk_424++;
        if (game->unk_424 >= 11) {
            game->cursor = game->cursor * 2 + 1;
            if (game->unk_42C != 0) {
                game->next_step = 4;
                if (game->slots[1].count - 1 < game->cursor) {
                    game->cursor = game->slots[1].count - 1;
                }
            } else {
                game->next_step = 2;
                if (game->slots[0].count - 1 < game->cursor) {
                    game->cursor = game->slots[0].count - 1;
                }
            }
        }
        break;
    case 10:
        game->unk_424++;
        if (game->unk_424 >= 11) {
            if (game->turn > 0) {
                game->next_step = 3;
                game->cursor /= 2;
                if (game->turn - 1 < game->cursor) {
                    game->cursor = game->turn - 1;
                }
            } else if (game->slots[0].count != 0) {
                game->next_step = 2;
                if (game->slots[0].count - 1 < game->cursor) {
                    game->cursor = game->slots[0].count - 1;
                }
            }
        }
        break;
    case 11:
        if (board->panels[0].state == 0 && game->display.state == 0) {
            game->next_step = 15;
        }
        break;
    case 12:
        if (board->dialog.state == 2) {
            game->next_step = 13;
        }
        break;
    case 13:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            game->next_step = 14;
        }
        break;
    case 14:
        if (board->dialog.state == 0) {
            game->next_step = 15;
        }
        break;
    case 15:
        board->panels[0].anim_state = cardgame_view_board_saved[0];
        done = 1;
        board->panels[1].anim_state = cardgame_view_board_saved[1];
        break;
    }
    return done;
}

void cardgame_choose_slot_start(CardgameGame *game, CardgameBoard *board, s32 arg2) {
    game->unk_438 = arg2;
    game->next_step = 1;
}

s32 cardgame_choose_slot_get_duration(CardgameGame *game, CardgameBoard *board) {
    return cardgame_view_board_get_duration(game, board);
}

void cardgame_choose_slot_move_cursor(CardgameGame *game, CardgameBoard *board, s32 arg2, s32 arg3) {
    cardgame_view_board_move_cursor(game, board, arg2, arg3);
}

s32 cardgame_choose_slot_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;
    s32 i;
    s32 j;

    if (game->next_step != 0) {
        switch (game->next_step) {
        case 1:
            if (game->unk_438 != 0) {
                for (i = 0; i < 3; i++) {
                    game->display.dimmed[12 + i] = 1;
                }
                for (i = 0; i < 12; i++) {
                    if (game->selectable[i] != 0) {
                        game->display.dimmed[i] = 0;
                    } else {
                        game->display.dimmed[i] = 1;
                    }
                }
                game->display.request = 1;
                game->cursor = 0;
                board->reset_panels(board);
                board->open_panels(board);
                board->place_card(board, 15, 0xE500, 0x6100);
                board->set_card(board, 15, game->turns[game->turn].card);
                board->cards[15].scale_x = 0;
                board->zoom_card(board, 15, 8, 0x1000, 0x1000);
            } else {
                for (j = 0; j < 3; j++) {
                    board->cards[12 + j].dimmed = 1;
                }
                for (j = 0; j < 12; j++) {
                    if (game->selectable[j] != 0) {
                        board->cards[j].dimmed = 0;
                    } else {
                        board->cards[j].dimmed = 1;
                    }
                }
            }
            game->unk_42C = cardgame_choose_slot_get_duration(game, board);
            cardgame_info_clear(game, board);
            break;
        case 2:
            cardgame_info_clear(game, board);
            game->unk_428 = 0;
            game->unk_424 = 0;
            game->unk_434 = 0;
            break;
        case 3:
            cardgame_info_clear(game, board);
            game->unk_428 = 0;
            game->unk_424 = 0;
            game->unk_434 = 1;
            break;
        case 7:
            board->cards[game->cursor].highlight &= ~1;
            board->cards[game->cursor].zooming = 0;
            board->close_popup(board, 4);
            board->close_popup(board, 1);
            board->close_popup(board, 2);
            board->close_popup(board, 3);
            game->unk_428 = 0;
            game->unk_424 = 0;
            break;
        case 8:
            board->cards[game->cursor + 6].highlight &= ~1;
            board->cards[game->cursor + 6].zooming = 0;
            board->close_popup(board, 4);
            board->close_popup(board, 1);
            board->close_popup(board, 2);
            board->close_popup(board, 3);
            game->unk_428 = 0;
            game->unk_424 = 0;
            break;
        case 6:
            board->close_popup(board, 4);
            board->close_popup(board, 1);
            board->close_popup(board, 2);
            board->close_popup(board, 3);
            board->flash_card(board, game->choice);
            sound_module.play(0x4001C);
            break;
        case 9:
            board->close_panels(board);
            board->close_popup(board, 4);
            board->close_popup(board, 1);
            board->close_popup(board, 2);
            board->close_popup(board, 3);
            game->display.request = 2;
            board->zoom_card(board, 15, 8, 0, 0x1000);
            break;
        case 10:
            board->close_panels(board);
            game->display.request = 2;
            board->zoom_card(board, 15, 8, 0, 0x1000);
            break;
        case 14:
            break;
        }
        game->effect_step = game->next_step;
        game->next_step = 0;
    }
    switch (game->effect_step) {
    case 1:
        if (game->unk_438 == 0 || (board->panels[0].state == 2 && game->display.state == 0)) {
            if (game->target_rows & 1) {
                if (game->slots[0].count != 0) {
                    game->next_step = 2;
                    game->unk_434 = 0;
                } else {
                    game->target_rows &= ~1;
                }
            }
            if (game->target_rows & 2) {
                if (game->slots[1].count != 0) {
                    game->next_step = 3;
                    game->unk_434 = 1;
                } else {
                    game->target_rows &= ~2;
                }
            }
            if (game->target_rows == 0) {
                game->next_step = 12;
                game->unk_434 = 3;
            }
        }
        break;
    case 2:
    case 3:
        game->unk_428 = cardgame_info_open_windows(game, board, game->unk_434, 0, game->unk_424, game->unk_428);
        cardgame_info_show(game, board, cardgame_row_first_card[game->unk_434]);
        game->unk_424++;
        if (game->unk_424 >= 11) {
            switch (game->unk_434) {
            case 0:
                cardgame_choose_slot_move_cursor(game, board, 0, 0);
                game->next_step = 4;
                break;
            case 1:
                cardgame_choose_slot_move_cursor(game, board, 1, 0);
                game->next_step = 5;
                break;
            }
        }
        break;
    case 4:
        if ((game->target_rows & 2) && PAD_PRESSED(4) && game->slots[1].count != 0) {
            game->next_step = 7;
        }
        if (((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 5))) |
             (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 5)))) &&
            game->cursor < game->slots[0].count - 1) {
            cardgame_view_board_move_cursor(game, board, 0, 1);
        }
        if (((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 7))) |
             (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 7)))) &&
            game->cursor > 0) {
            cardgame_view_board_move_cursor(game, board, 0, -1);
        }
        cardgame_info_show_row(game, board, 0);
        if (game->unk_438 != 0 && PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            game->next_step = 9;
        }
        if (PAD_PRESSED(13) && game->selectable[game->cursor] != 0) {
            game->next_step = 6;
            game->choice = game->cursor;
        }
        break;
    case 5:
        if ((game->target_rows & 1) && PAD_PRESSED(6) && game->slots[0].count != 0) {
            game->next_step = 8;
        }
        if (((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 5))) |
             (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 5)))) &&
            game->cursor < game->slots[1].count - 1) {
            cardgame_view_board_move_cursor(game, board, 1, 1);
        }
        if (((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 7))) |
             (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 7)))) &&
            game->cursor > 0) {
            cardgame_view_board_move_cursor(game, board, 1, -1);
        }
        cardgame_info_show_row(game, board, 1);
        if (game->unk_438 != 0 && PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            game->next_step = 9;
        } else if (PAD_PRESSED(13) && game->selectable[game->cursor + 6] != 0) {
            game->next_step = 6;
            game->choice = game->cursor + 6;
        }
        break;
    case 6:
        if (board->cards[game->choice].state == 1) {
            if (game->unk_438 == 0) {
                for (j = 0; j < 3; j++) {
                    board->cards[12 + j].dimmed = 0;
                }
                for (j = 0; j < 12; j++) {
                    board->cards[j].dimmed = 0;
                }
            }
            game->next_step = 14;
        }
        break;
    case 7:
        game->unk_424++;
        if (game->unk_424 >= 11) {
            game->next_step = 3;
            if (game->slots[1].count - 1 < game->cursor) {
                game->cursor = game->slots[1].count - 1;
            }
        }
        break;
    case 8:
        game->unk_424++;
        if (game->unk_424 >= 11) {
            game->next_step = 2;
            if (game->slots[0].count - 1 < game->cursor) {
                game->cursor = game->slots[0].count - 1;
            }
        }
        break;
    case 11:
        game->next_step = 14;
        break;
    case 9:
    case 10:
        if (board->panels[0].state == 0 && game->display.state == 0) {
            game->next_step = 13;
        }
        break;
    case 12:
        if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            game->next_step = 10;
        }
        break;
    case 13:
        done = 1;
        break;
    case 14:
        for (j = 0; j < 15; j++) {
            game->marked[j] = 0;
        }
        done = 2;
        game->marked[game->choice] = 1;
        break;
    }
    return done;
}

void cardgame_choose_card_start(CardgameGame *game, CardgameBoard *board, s32 side, s32 mode) {
    s32 i;

    switch (mode) {
    case 0:
        game->unk_438 = side == 0 ? 5 : 11;
        game->unk_434 = game->players[side].hand_count;
        break;
    case 1:
    case 2:
        if (side == 0) {
            game->unk_438 = 7;
            game->sort_cards(game, game->players[0].deck, game->players[0].deck_pos | (40 << 16), 2);
        } else {
            game->unk_438 = 13;
            if (mode != 2) {
                game->sort_cards(game, game->players[1].deck, game->players[1].deck_pos | (40 << 16), 3);
            }
        }
        game->unk_434 = game->players[side].deck_count;
        break;
    case 3:
        game->unk_438 = side == 0 ? 9 : 15;
        game->unk_434 = game->players[side].discard_count;
        break;
    }
    for (i = 0; i < 40; i++) {
        if (i < game->unk_434) {
            game->marked[i] = 0;
            if (game->selectable[i] != 0) {
                game->display.dimmed[i] = 0;
            } else {
                game->display.dimmed[i] = 1;
            }
        }
    }
    game->next_step = 1;
    cardgame_info_clear(game, board);
    game->cursor = 0;
    game->choice = -1;
}

void cardgame_choose_hand_card_start(CardgameGame *game, CardgameBoard *board, s32 arg2) {
    cardgame_choose_card_start(game, board, arg2, 0);
}

void cardgame_choose_card_move_cursor(CardgameGame *game, CardgameBoard *board, s32 delta) {
    sound_module.play(0x4001B);
    board->slide_card(board, game->cursor, 5, board->get_card_x(game->unk_434, game->cursor) + 0x1800, 0x6100);
    board->cards[game->cursor].highlight &= ~1;
    board->cards[game->cursor].zooming = 0;
    game->cursor += delta;
    board->slide_card(board, game->cursor, 1, board->get_card_x(game->unk_434, game->cursor) + 0x1800, 0x5C00);
    board->cards[game->cursor].highlight |= 1;
    board->cards[game->cursor].zooming = 1;
}

void cardgame_choose_card_input(CardgameGame *game, CardgameBoard *board) {
    board->cards[game->cursor].highlight |= 1;
    board->cards[game->cursor].zooming = 1;
    if ((pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 7))) |
        (pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 7)))) {
        if (game->cursor > 0) {
            cardgame_choose_card_move_cursor(game, board, -1);
        }
    } else if ((pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 5))) |
               (pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 5)))) {
        if (game->cursor < game->unk_434 - 1) {
            cardgame_choose_card_move_cursor(game, board, 1);
        }
    } else if (PAD_PRESSED(13) && game->selectable[game->cursor] != 0) {
        game->next_step = 3;
    }
    game->target_rows = 0;
}

s32 cardgame_choose_card_update(CardgameGame *game, CardgameBoard *board, s32 mode) {
    s32 done = 0;
    s32 t;

    if (game->next_step != 0) {
        switch (game->next_step) {
        case 1:
            game->display.request = game->unk_438;
            switch (game->unk_438) {
            case 5:
                cardgame_choose_card_title = 0x19;
                break;
            case 7:
                cardgame_choose_card_title = 0x1A;
                break;
            case 13:
                cardgame_choose_card_title = 0x1D;
                break;
            case 9:
                cardgame_choose_card_title = 0x1B;
                break;
            case 11:
            case 15:
                cardgame_choose_card_title = 0x1C;
                break;
            }
            if (mode == 2) {
                board->open_side_panel(board, 0);
            }
            game->unk_42C = 0;
            game->unk_428 = 0;
            game->unk_424 = 0;
            game->choice = 0;
            game->selected = 0;
            break;
        case 3:
            sound_module.play(0x4001C);
        case 2:
            game->unk_428 = 0;
            game->unk_424 = 0;
            break;
        case 4:
            if (mode == 1) {
                board->zoom_card(board, 15, 8, 0, 0x1000);
            }
            game->unk_428 = 0;
            game->unk_424 = 0;
            game->display.request = game->display.reopen;
            board->close_popup(board, 4);
            board->close_popup(board, 0);
            board->close_popup(board, 1);
            board->close_popup(board, 2);
            board->close_popup(board, 3);
            break;
        }
        game->effect_step = game->next_step;
        game->next_step = 0;
    }
    switch (game->effect_step) {
    case 1:
        game->unk_42C = cardgame_info_open_windows(game, board, 2, cardgame_choose_card_title, game->unk_424, game->unk_42C);
        cardgame_info_show(game, board, 0);
        if (game->unk_424 == 2 && mode == 1) {
            board->place_card(board, 15, cardgame_slot_origins[main_screen_pos][0].x, cardgame_slot_origins[main_screen_pos][0].y);
            board->set_card(board, 15, game->turns[game->turn].card);
            board->cards[15].scale_x = 0;
            board->zoom_card(board, 15, 8, 0x1000, 0x1000);
        }
        if (game->display.count * 4 + 14 < game->unk_424) {
            game->next_step = 2;
            board->slide_card(board, 0, 5, 0x1800, 0x5C00);
            board->cards[game->cursor].highlight |= 1;
            board->cards[game->cursor].zooming = 1;
        }
        game->unk_424++;
        break;
    case 2:
        cardgame_choose_card_input(game, board);
        if (mode == 1 && PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            game->target_rows = mode;
            game->next_step = 4;
        }
        cardgame_info_show(game, board, 0);
        break;
    case 3:
        if (cardgame_hand_pulse_card(game, board) != 0) {
            game->target_rows = 2;
            game->next_step = 4;
            if (mode == 2) {
                board->close_side_panel(board, 0);
            }
        }
        cardgame_info_show(game, board, 0);
        break;
    case 4:
        if (game->unk_424++ > game->unk_434 * 4 + 5) {
            game->choice = game->cursor;
            switch (game->unk_438) {
            case 7:
                t = game->players[0].deck[game->players[0].deck_pos];
                game->players[0].deck[game->players[0].deck_pos] =
                    game->players[0].deck[game->players[0].deck_pos + game->choice];
                game->players[0].deck[game->players[0].deck_pos + game->choice] = t;
                game->choice = game->players[0].deck_pos;
                game->shuffle_deck(game, game->players[0].deck_pos + 1, game->players[0].deck_count - 1);
                break;
            case 13:
                game->choice = game->cpu_deck_info[game->choice + game->players[1].deck_pos].pos;
                cardgame_restore_cpu_deck(game);
                break;
            }
            game->marked[game->choice] = 1;
            done = game->target_rows;
            game->next_step = 0;
            game->new_effect = 0;
            game->effect = 0;
            game->new_effect = 0;
        }
        break;
    }
    return done;
}

s32 cardgame_cpu_select_cards(CardgameGame *game, CardgameBoard *board) {
    s32 i;
    s16 card;

    for (i = 0; i < game->players[1].hand_count; i++) {
        game->marked[i] = 0;
        if (game->selected < 6 &&
            cardgame_can_play_card(game, game->players[1].points, game->players[1].hand[i]) != 0) {
            card = game->players[1].hand[i];
            if (game->cpu_cards[card - 40].kind != 5) {
                cardgame_player_change_points(game, board, &game->players[1], 0, card);
                game->marked[i] = 1;
                game->selected++;
            }
        }
    }
    return 1;
}

void cardgame_cpu_choose_card_by_value(CardgameGame *game, CardgameBoard *board, s32 mode) {
    CardPicture pic;
    u8 *best;
    u8 *data;
    s32 n = 0;
    s32 card;
    s32 found;
    s32 i;

    switch (game->unk_438) {
    case 5:
        n = game->players[0].hand_count;
        break;
    case 7:
        n = game->players[0].deck_count;
        break;
    case 11:
        n = game->players[1].hand_count;
        break;
    case 13:
        n = game->players[1].hand_count;
        break;
    case 9:
        n = game->players[0].discard_count;
        break;
    case 15:
        n = game->players[1].discard_count;
        break;
    }
    best = NULL;
    card = 0;
    card_init(&pic);
    found = -1;
    for (i = 0; i < n; i++) {
        if (game->selectable[i] != 0) {
            switch (game->unk_438) {
            case 5:
                card = game->players[0].hand[i];
                break;
            case 7:
                card = game->players[0].deck[i];
                break;
            case 11:
                card = game->players[1].hand[i];
                break;
            case 13:
                card = game->players[1].hand[i];
                break;
            case 9:
                card = game->players[0].discard[i];
                break;
            case 15:
                card = game->players[1].discard[i];
                break;
            }
            pic.select(game->card_ids[card] + 1);
            data = pic.record;
            if (found == -1) {
                found = i;
                best = data;
            } else if (mode == 0) {
                if (*(s16 *)(data + 8) >= *(s16 *)(best + 8)) {
                    found = i;
                    best = data;
                }
            } else if (*(s16 *)(data + 8) < *(s16 *)(best + 8)) {
                found = i;
                best = data;
            }
        }
    }
    game->choice = found;
}

void cardgame_cpu_choose_own_deck_card(CardgameGame *game, CardgameBoard *board) {
    s32 found = 0;
    s32 i;

    for (i = game->players[1].deck_pos; i < game->cpu_deck_end; i++) {
        if (game->selectable[i - game->players[1].deck_pos] != 0) {
            game->choice = i;
            found = 1;
            break;
        }
    }
    if (!found) {
        for (i = 39; game->players[1].deck_pos < i; i--) {
            if (game->selectable[i - game->players[1].deck_pos] != 0) {
                break;
            }
        }
        game->choice = i;
    }
    game->marked[game->choice] = 1;
}

void cardgame_cpu_choose_player_deck_card(CardgameGame *game, CardgameBoard *board) {
    CardPicture pic;
    s32 best = game->players[0].deck_pos;
    s32 min;
    s32 i;

    card_init(&pic);
    min = 400;
    for (i = game->players[0].deck_pos; i < 40; i++) {
        pic.select(game->card_ids[game->players[0].deck[i]] + 1);
        if (*(s16 *)(pic.record + 8) < min) {
            min = *(s16 *)(pic.record + 8);
            best = i;
        }
    }
    game->choice = best;
}

void cardgame_ask_use_start(CardgameGame *game, CardgameBoard *board, s32 flags) {
    s32 i;

    board->reset_panels(board);
    board->set_lamps(board, flags);
    game->choice = 0;
    game->effect_step = 1;
    game->unk_438 = flags;
    board->place_card(board, 15, 0xE500, 0x6100);
    board->set_card(board, 15, game->turns[game->turn].card);
    board->cards[15].scale_x = 0;
    board->zoom_card(board, 15, 8, 0x1000, 0x1000);
    board->open_panels(board);
    game->display.set_dimmed = 2;
    game->display.dimmed[15] = 0;
    for (i = 0; i < 15; i++) {
        game->marked[i] = 0;
    }
    game->display.request = 1;
}

s32 cardgame_ask_use_update(CardgameGame *game, CardgameBoard *board) {
    s32 result = -1;

    switch (game->effect_step) {
    case 1:
        if (board->panels[0].state == 2 && game->display.state == 0) {
            game->effect_step = 2;
        }
        break;
    case 2:
        if (PAD_PRESSED(13)) {
            switch (game->unk_438) {
            case 0x400:
            case 0x1400:
                if (game->players[0].discard_count != 0) {
                    result = 1;
                }
                break;
            case 0x1000:
                if (game->players[0].deck_count != 0) {
                    result = 1;
                }
                break;
            case 0x4000:
                if (game->players[0].hand_count != 0) {
                    result = 1;
                }
                break;
            case 0x2000:
                if (game->players[1].deck_count != 0) {
                    result = 1;
                }
                break;
            case 0x8000:
                if (game->players[1].hand_count != 0) {
                    result = 1;
                }
                break;
            default:
                result = 1;
                break;
            }
            if (result == 1) {
                sound_module.play(0x4001C);
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            board->close_panels(board);
            game->choice = 1;
            game->effect_step = 3;
            board->zoom_card(board, 15, 4, 0, 0x1000);
            game->display.request = 2;
        }
        break;
    case 3:
        if (board->panels[0].state == 0 && game->display.state == 0) {
            board->remove_card(board, 15);
            result = 0;
            board->clear_lamps(board);
        }
        break;
    }
    return result;
}

void cardgame_ask_cancel_start(CardgameGame *game, CardgameBoard *board, s32 mode) {
    CardPicture pic;
    s32 i;
    s32 j;
    s32 k;
    s32 ok;

    board->reset_panels(board);
    game->choice = 0;
    game->effect_step = 1;
    game->unk_438 = 0;
    board->place_card(board, 15, 0xE500, 0x6100);
    board->set_card(board, 15, game->turns[game->turn].card);
    board->cards[15].scale_x = 0;
    board->zoom_card(board, 15, 8, 0x1000, 0x1000);
    board->open_panels(board);
    for (i = 0; i < 15; i++) {
        game->display.dimmed[i] = 1;
    }
    game->display.dimmed[15] = 0;
    for (j = 0; j < 15; j++) {
        game->marked[j] = 0;
    }
    if (game->turn != 0) {
        ok = 0;
        if (mode == 0) {
            ok = 1;
        } else if (mode == 1) {
            s32 card = game->turns[game->turn - 1].card;

            card_init(&pic);
            pic.select(game->card_ids[card] + 1);
            if (pic.record[0] == 6) {
                ok = 1;
            }
        }
        if (ok) {
            k = game->turn + 11;
            board->cards[k].highlight |= 1;
            game->display.dimmed[k] = 0;
            game->unk_438 = 1;
        }
    }
    game->display.request = 1;
}

s32 cardgame_ask_cancel_update(CardgameGame *game, CardgameBoard *board) {
    s32 result = -1;
    s32 k;

    switch (game->effect_step) {
    case 1:
        if (board->panels[0].state == 2 && game->display.state == 0) {
            game->effect_step = 2;
            if (game->turn != 0) {
                board->cards[game->turn + 11].highlight |= 1;
            }
        }
        break;
    case 2:
        if (PAD_PRESSED(13)) {
            if (game->unk_438 == 1) {
                sound_module.play(0x4001C);
                k = game->turn + 11;
                board->cards[k].highlight &= ~1;
                game->marked[k] = game->turn + 1;
                result = 1;
                game->turns[game->turn - 1].mark = game->turn;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            board->close_panels(board);
            game->choice = 1;
            game->effect_step = 3;
            board->zoom_card(board, 15, 4, 0, 0x1000);
            game->display.request = 2;
        }
        break;
    case 3:
        if (board->panels[0].state == 0 && game->display.state == 0) {
            board->remove_card(board, 15);
            result = 0;
            board->clear_lamps(board);
        }
        break;
    }
    return result;
}

void cardgame_ask_area_start(CardgameGame *game, CardgameBoard *board, s32 side, s32 mode) {
    s32 i;
    s32 found;
    s32 card;
    s32 j;

    board->reset_panels(board);
    game->choice = 0;
    game->effect_step = 1;
    board->place_card(board, 15, 0xE500, 0x6100);
    board->set_card(board, 15, game->turns[game->turn].card);
    board->cards[15].scale_x = 0;
    board->zoom_card(board, 15, 8, 0x1000, 0x1000);
    board->open_panels(board);
    game->unk_438 = 0;
    for (i = 0, found = 0; i < 12; i++, found = 0) {
        game->marked[i] = 0;
        game->display.dimmed[i] = 1;
        board->cards[i].highlight &= ~1;
        if (i < 6) {
            if (i >= game->slots[0].count) {
                continue;
            }
            card = game->slots[0].slots[i].card;
        } else {
            if (i - 6 >= game->slots[1].count) {
                continue;
            }
            card = game->slots[1].slots[i - 6].card;
        }
        switch (mode) {
        case 1:
            if (side == 0) {
                if (i < 6) {
                    found = 1;
                }
            } else if (i >= 6) {
                found = 1;
            }
            break;
        case 2:
            if (side == 0) {
                if (i >= 6) {
                    found = 1;
                }
            } else if (i < 6) {
                found = 1;
            }
            break;
        case 3:
            found = 1;
            break;
        case 4:
            if (board->get_card_kind(board, card) != 1) {
                found = 1;
            }
            break;
        case 5:
            if (board->get_card_kind(board, card) != 2) {
                found = 1;
            }
            break;
        case 6:
            if (board->get_card_kind(board, card) == 3) {
                found = 1;
            }
            break;
        case 7:
            if (board->get_card_kind(board, card) != 4) {
                found = 1;
            }
            break;
        case 8:
            if (board->get_card_kind(board, card) == 6) {
                found = 1;
            }
            break;
        }
        if (found) {
            game->marked[i] = 1;
            game->display.dimmed[i] = 0;
            game->unk_438 = 1;
        }
    }
    for (j = 0; j < 3; j++) {
        game->display.dimmed[12 + j] = 1;
    }
    game->display.dimmed[15] = 0;
    game->display.request = 1;
}

s32 cardgame_ask_area_update(CardgameGame *game, CardgameBoard *board) {
    s32 result = -1;
    s32 i;
    s32 j;
    s32 k;

    switch (game->effect_step) {
    case 1:
        if (board->panels[0].state == 2 && game->display.state == 0) {
            game->effect_step = 2;
            for (i = 0; i < 12; i++) {
                if (game->marked[i] != 0) {
                    board->cards[i].highlight |= 1;
                }
            }
        }
        break;
    case 2:
        if (PAD_PRESSED(13)) {
            if (game->unk_438 != 0) {
                sound_module.play(0x4001C);
                result = 1;
                for (j = 0; j < 12; j++) {
                    if (game->marked[j] != 0) {
                        board->cards[j].highlight &= ~1;
                    }
                }
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            board->close_panels(board);
            game->choice = 1;
            game->effect_step = 3;
            board->zoom_card(board, 15, 4, 0, 0x1000);
            game->display.request = 2;
        }
        break;
    case 3:
        if (board->panels[0].state == 0 && game->display.state == 0) {
            board->remove_card(board, 15);
            result = 0;
            board->clear_lamps(board);
            for (k = 0; k < 12; k++) {
                if (game->marked[k] != 0) {
                    board->cards[k].highlight &= ~1;
                }
            }
        }
        break;
    }
    return result;
}

s32 cardgame_player_add_card_point(CardgameGame *game, CardgameBoard *board, s32 side, s32 card) {
    CardPicture pic;
    CardgamePlayer *player = &game->players[side];
    s32 kind;
    s32 ok = 0;

    card_init(&pic);
    pic.select(game->card_ids[card] + 1);
    kind = pic.record[0] - 1;
    if (kind < 5) {
        if (player->points[kind] < 99) {
            player->points[kind]++;
        }
        board->set_panel_value(board, side, kind, player->points[kind]);
        ok = 1;
    }
    return ok;
}

s32 cardgame_slot_animate_discard(CardgameGame *game, CardgameBoard *board, s32 i) {
    s32 done = 0;

    switch (game->unk_424) {
    case 0:
    default:
        board->jolt_card(board, i);
        game->unk_424 = 1;
        break;
    case 1:
        if (board->cards[i].state == 1) {
            game->unk_424 = 2;
            board->zoom_card(board, i, 5, 0, 0x1000);
        }
        break;
    case 2:
        if (board->cards[i].state == 1) {
            done = 1;
        }
        break;
    }
    return done;
}

void cardgame_slot_to_discard(CardgameGame *game, CardgameBoard *board, s32 side, s32 i) {
    switch (game->slots[side].slots[i].owner) {
    case 0:
        game->players[0].discard[game->players[0].discard_count] = game->slots[side].slots[i].card;
        board->set_panel_value(board, 0, 7, ++game->players[0].discard_count);
        break;
    case 1:
        game->players[1].discard[game->players[1].discard_count] = game->slots[side].slots[i].card;
        board->set_panel_value(board, 1, 7, ++game->players[1].discard_count);
        break;
    }
}

s32 cardgame_slot_discard(CardgameGame *game, CardgameBoard *board, s32 side, s32 i) {
    s32 found = 0;
    s32 j = i;

    if (side != 0) {
        j = i + 6;
    }
    if (cardgame_slot_animate_discard(game, board, j) != 0) {
        cardgame_slot_to_discard(game, board, side, i);
        found = 1;
    }
    return found;
}

s32 cardgame_slot_animate_return(CardgameGame *game, CardgameBoard *board, s32 i) {
    s32 done = 0;

    switch (game->unk_424) {
    case 0:
    default:
        game->unk_424 = 1;
        board->cards[i].zooming = 1;
        board->move_card(board, i, 15, -0x5000, 0x6100);
        board->set_card_scale(board, i, 0x1200, 0x1200);
        break;
    case 1:
        if (board->cards[i].state == 1) {
            board->cards[i].zooming = 0;
            board->cards[i].scale_x = 0;
            done = 1;
        }
        break;
    }
    return done;
}

void cardgame_slot_to_hand(CardgameGame *game, CardgameBoard *board, s32 side, s32 i) {
    switch (game->slots[side].slots[i].owner) {
    case 0:
        game->players[0].hand[game->players[0].hand_count] = game->slots[side].slots[i].card;
        board->set_panel_value(board, 0, 6, ++game->players[0].hand_count);
        break;
    case 1:
        game->players[1].hand[game->players[1].hand_count] = game->slots[side].slots[i].card;
        board->set_panel_value(board, 1, 6, ++game->players[1].hand_count);
        break;
    }
}

s32 cardgame_slot_return(CardgameGame *game, CardgameBoard *board, s32 side, s32 i) {
    s32 found = 0;
    s32 j = i;

    if (side != 0) {
        j = i + 6;
    }
    if (cardgame_slot_animate_return(game, board, j) != 0) {
        cardgame_slot_to_hand(game, board, side, i);
        found = 1;
    }
    return found;
}

/* The call is written per side (cross-jumped later; its references decide board's register). */
void cardgame_card_effect_start(CardgameGame *game, CardgameBoard *board, s32 mode) {
    s32 found = 0;
    s32 i;

    for (i = 0; i < 12; i++) {
        if (game->marked[i] != 0) {
            if (i < 6) {
                if (i < game->slots[0].count) {
                    board->show_card_effect(board, i, mode);
                    found = 1;
                }
            } else {
                if (i - 6 < game->slots[1].count) {
                    board->show_card_effect(board, i, mode);
                    found = 1;
                }
            }
        }
    }
    if (found) {
        switch (mode) {
        case 0:
        default:
            sound_module.play(0x9C0001);
            break;
        case 1:
            sound_module.play(0x9C0000);
            break;
        }
    }
    game->unk_424 = 0;
}

s32 cardgame_card_effect_update(CardgameGame *game, CardgameBoard *board, s32 limit) {
    game->unk_424 += gfx_module.funcs.get_frame_ticks();
    return limit < game->unk_424;
}

void cardgame_change_stats_start(CardgameGame *game, CardgameBoard *board, s32 delta, s32 mode) {
    s16 dy = delta >> 16;
    s16 dx = delta;
    s32 i;

    game->unk_424 = 0;
    game->unk_428 = dy;
    game->unk_42C = dx;
    game->unk_430 = dy;
    if (dy < 0) {
        game->unk_430 = -dy;
    }
    game->unk_434 = dx;
    if (dx < 0) {
        game->unk_434 = -dx;
    }
    for (i = 0; i < 12; i++) {
        if (game->marked[i] != 0) {
            if (i < 6) {
                if (i >= game->slots[0].count) {
                    continue;
                }
                game->slots[0].slots[i].attack_bonus += dy;
                game->slots[0].slots[i].hp_bonus += dx;
                if (mode == 2) {
                    game->slots[0].slots[i].attack_bonus = game->slots[0].slots[i].attack * -1;
                }
            } else {
                if (i - 6 >= game->slots[1].count) {
                    continue;
                }
                game->slots[1].slots[i - 6].attack_bonus += dy;
                game->slots[1].slots[i - 6].hp_bonus += dx;
                if (mode == 2) {
                    game->slots[1].slots[i - 6].attack_bonus = game->slots[1].slots[i - 6].attack * -1;
                }
            }
            if (mode == 0) {
                board->jolt_card(board, i);
            } else {
                board->boost_card(board, i);
            }
        }
    }
}

s32 cardgame_change_stats_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 1;
    s32 i;

    if (game->unk_424 < game->unk_430) {
        for (i = 0; i < 12; i++) {
            if (game->marked[i] != 0) {
                if (i < 6) {
                    if (i >= game->slots[0].count) {
                        continue;
                    }
                    if (game->unk_428 > 0) {
                        if (game->slots[0].slots[i].attack < 99) {
                            game->slots[0].slots[i].attack++;
                            board->cards[i].attack++;
                        }
                    } else if (game->slots[0].slots[i].attack > 0) {
                        game->slots[0].slots[i].attack--;
                        board->cards[i].attack--;
                    }
                } else {
                    if (i - 6 >= game->slots[1].count) {
                        continue;
                    }
                    if (game->unk_428 > 0) {
                        if (game->slots[1].slots[i - 6].attack < 99) {
                            game->slots[1].slots[i - 6].attack++;
                            board->cards[i].attack++;
                        }
                    } else if (game->slots[1].slots[i - 6].attack > 0) {
                        game->slots[1].slots[i - 6].attack--;
                        board->cards[i].attack--;
                    }
                }
            }
        }
        done = 0;
    }
    if (game->unk_424 < game->unk_434) {
        for (i = 0; i < 12; i++) {
            if (game->marked[i] != 0) {
                if (i < 6) {
                    if (i >= game->slots[0].count) {
                        continue;
                    }
                    if (game->unk_42C > 0) {
                        if (game->slots[0].slots[i].hp < 99) {
                            game->slots[0].slots[i].hp++;
                            board->cards[i].hp++;
                        }
                    } else if (game->slots[0].slots[i].hp > 0) {
                        game->slots[0].slots[i].hp--;
                        board->cards[i].hp--;
                    }
                } else {
                    if (i - 6 >= game->slots[1].count) {
                        continue;
                    }
                    if (game->unk_42C > 0) {
                        if (game->slots[1].slots[i - 6].hp < 99) {
                            game->slots[1].slots[i - 6].hp++;
                            board->cards[i].hp++;
                        }
                    } else if (game->slots[1].slots[i - 6].hp > 0) {
                        game->slots[1].slots[i - 6].hp--;
                        board->cards[i].hp--;
                    }
                }
            }
        }
        done = 0;
    }
    game->unk_424++;
    if (done) {
        for (i = 0; i < 12; i++) {
            game->marked[i] = 0;
            if (i < 6) {
                if (i < game->slots[0].count && game->slots[0].slots[i].hp <= 0) {
                    game->marked[i] = 1;
                }
            } else if (i - 6 < game->slots[1].count && game->slots[1].slots[i - 6].hp <= 0) {
                game->marked[i] = 1;
            }
        }
    }
    return done;
}

void cardgame_mark_turn_area(CardgameGame *game, CardgameBoard *board) {
    s32 t = game->turn - 1;
    s32 side = game->turns[t].side;
    s32 card;
    s32 i;

    for (i = 0; i < 12; i++) {
        game->marked[i] = 0;
        if (i < 6) {
            if (i >= game->slots[0].count) {
                continue;
            }
            card = game->slots[0].slots[i].card;
        } else {
            if (i - 6 >= game->slots[1].count) {
                continue;
            }
            card = game->slots[1].slots[i - 6].card;
        }
        switch (game->turns[t].target_kind) {
        case 1:
            if (side == 0) {
                if (i < 6) {
                    game->marked[i] = 1;
                }
            } else if (i >= 6) {
                game->marked[i] = 1;
            }
            break;
        case 2:
            if (side == 0) {
                if (i >= 6) {
                    game->marked[i] = 1;
                }
            } else if (i < 6) {
                game->marked[i] = 1;
            }
            break;
        case 3:
            game->marked[i] = 1;
            break;
        case 4:
            if (board->get_card_kind(board, card) != 1) {
                game->marked[i] = 1;
            }
            break;
        case 5:
            if (board->get_card_kind(board, card) != 2) {
                game->marked[i] = 1;
            }
            break;
        case 6:
            if (board->get_card_kind(board, card) == 3) {
                game->marked[i] = 1;
            }
            break;
        case 7:
            if (board->get_card_kind(board, card) != 4) {
                game->marked[i] = 1;
            }
            break;
        case 8:
            if (board->get_card_kind(board, card) == 6) {
                game->marked[i] = 1;
            }
            break;
        }
    }
}

void cardgame_compact_slots_start(CardgameGame *game, CardgameBoard *board) {
    game->effect_step = 1;
}

s32 cardgame_compact_slots_update(CardgameGame *game, CardgameBoard *board) {
    CardgameBoardCard tmp;
    s32 done = 0;
    s32 side;
    s32 s;
    s32 n;
    s32 i;
    s32 j;
    s32 k;
    s8 *flags;
    s8 t;
    s32 from;
    s32 to;

    switch (game->effect_step) {
    case 1:
        cardgame_slot_removed_counts[1] = 0;
        cardgame_slot_removed_counts[0] = 0;
        cardgame_slot_move_counts[1] = 0;
        cardgame_slot_move_counts[0] = 0;
        for (side = 0; side < 2; side++) {
            flags = &game->marked[side * 6];
            for (k = 0; k < game->slots[side].count; k++) {
                if (flags[k] == 1) {
                    cardgame_slot_removed_counts[side]++;
                }
            }
            n = 0;
            for (k = 0; k < game->slots[side].count - 1; k++) {
                if (flags[k] == 1) {
                    for (j = k + 1; j < game->slots[side].count; j++) {
                        if (flags[j] == 0) {
                            t = flags[k];
                            flags[k] = flags[j];
                            flags[j] = t;
                            cardgame_slot_moves[side][n].from = k;
                            cardgame_slot_moves[side][n].to = j;
                            n++;
                            break;
                        }
                    }
                }
            }
            cardgame_slot_move_counts[side] = n;
            if (side == 0) {
                for (k = 0; k < n; k++) {
                    board->move_card(board, cardgame_slot_moves[0][k].to, 10,
                                   cardgame_slot_origins[main_screen_pos][0].x + cardgame_slot_moves[0][k].from * 0x2900,
                                   cardgame_slot_origins[main_screen_pos][0].y);
                }
            } else {
                for (k = 0; k < n; k++) {
                    board->move_card(board, cardgame_slot_moves[1][k].to + 6, 10,
                                   cardgame_slot_origins[main_screen_pos][1].x + cardgame_slot_moves[1][k].from * 0x2900,
                                   cardgame_slot_origins[main_screen_pos][1].y);
                }
            }
        }
        if (cardgame_slot_removed_counts[0] + cardgame_slot_removed_counts[1] != 0) {
            game->effect_step = 2;
            game->unk_424 = 20;
        } else {
            game->effect_step = 4;
        }
        break;
    case 2:
        game->unk_424 -= gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 <= 0) {
            game->effect_step = 3;
        }
        break;
    case 3:
        for (s = 0; s < 2; s++) {
            for (i = 0; i < cardgame_slot_move_counts[s]; i++) {
                from = cardgame_slot_moves[s][i].from;
                to = cardgame_slot_moves[s][i].to;
                game->slots[s].slots[from] = game->slots[s].slots[to];
                from += s * 6;
                to += s * 6;
                tmp = board->cards[from];
                board->cards[from] = board->cards[to];
                board->cards[to] = tmp;
            }
            game->slots[s].count -= cardgame_slot_removed_counts[s];
            for (i = 0; i < 6; i++) {
                if (i >= game->slots[s].count) {
                    board->remove_card(board, s * 6 + i);
                    board->cards[s * 6 + i].scale_x = 0;
                }
            }
        }
        game->effect_step = 4;
        break;
    case 4:
        done = 1;
        break;
    }
    return done;
}

s32 cardgame_mark_selectable_cards(CardgameGame *game, CardgameBoard *board, s32 side, s32 where, s32 flags) {
    CardPicture pic;
    s32 found = 0;
    s32 n = 0;
    s32 card;
    s32 t;
    s32 i;
    s32 k;

    switch (where) {
    case 2:
        n = game->players[side].hand_count;
        break;
    case 3:
        n = game->players[side].deck_count;
        break;
    case 4:
        n = game->players[side].discard_count;
        break;
    }
    card = 0;
    for (i = 0; i < n; i++) {
        game->selectable[i] = 0;
        switch (where) {
        case 2:
            card = game->players[side].hand[i];
            break;
        case 3:
            card = game->players[side].deck[i + game->players[side].deck_pos];
            break;
        case 4:
            card = game->players[side].discard[i];
            break;
        }
        t = game->card_ids[card];
        card_init(&pic);
        pic.select(t + 1);
        t = 0;
        if (pic.record[3] == 0x10) {
            t = flags & 1;
        } else if (flags & 2) {
            t = 1;
        }
        if (t) {
            for (k = 0; k < 6; k++) {
                if ((flags & cardgame_kind_masks[k]) && pic.record[0] == k + 1) {
                    game->selectable[i] = 1;
                    found = 1;
                    break;
                }
            }
        }
    }
    return found;
}

s32 cardgame_mark_selectable_slots(CardgameGame *game, CardgameBoard *board, s32 mode, s32 flags) {
    CardPicture pic;
    s32 found = 0;
    s32 card = 0;
    s32 skip;
    s32 i;
    s32 k;

    card_init(&pic);
    game->target_rows = 0;
    for (i = 0, skip = 0; i < 15; i++, skip = 0) {
        game->selectable[i] = 0;
        if (i < 6) {
            if ((mode == 0 && (flags & 0x100)) || (mode != 0 && (flags & 0x200))) {
                game->target_rows |= 1;
                if (i < game->slots[0].count) {
                    card = game->slots[0].slots[i].card;
                } else {
                    skip = 1;
                }
            } else {
                skip = 1;
            }
        } else if (i < 12) {
            if ((mode == 0 && (flags & 0x200)) || (mode != 0 && (flags & 0x100))) {
                game->target_rows |= 2;
                if (i - 6 < game->slots[1].count) {
                    card = game->slots[1].slots[i - 6].card;
                } else {
                    skip = 1;
                }
            } else {
                skip = 1;
            }
        } else {
            skip = 1;
        }
        if (!skip) {
            pic.select(game->card_ids[card] + 1);
            for (k = 0; k < 6; k++) {
                if ((flags & cardgame_kind_masks[k]) && pic.record[0] == k + 1) {
                    game->selectable[i] = 1;
                    found = 1;
                    break;
                }
            }
        }
    }
    return found;
}

void cardgame_discard_hand_start(CardgameGame *game, CardgameBoard *board, s32 side) {
    game->effect_step = 1;
    game->unk_424 = 0;
    game->unk_428 = game->players[side].hand_count;
    game->unk_42C = game->players[side].discard_count;
    game->unk_430 = 0;
}

s32 cardgame_discard_hand_update(CardgameGame *game, CardgameBoard *board, s32 side) {
    s32 done = 0;
    s32 more;
    s32 i;

    switch (game->effect_step) {
    case 1:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        game->unk_430 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_430 >= 7) {
            more = 0;
            if (game->unk_428 > 0) {
                more = 1;
                game->unk_428--;
                game->unk_42C++;
            }
            if (!more) {
                game->effect_step = 2;
            }
            board->set_panel_value(board, side, 6, game->unk_428);
            board->set_panel_value(board, side, 7, game->unk_42C);
            game->unk_430 -= 7;
        }
        break;
    case 2:
        for (i = 0; i < game->players[side].hand_count; i++) {
            game->players[side].discard[game->players[side].discard_count] = game->players[side].hand[i];
            game->players[side].discard_count++;
        }
        game->players[side].hand_count = 0;
        done = 1;
        break;
    }
    return done;
}

void cardgame_points_start(CardgameGame *game, CardgameBoard *board, s32 delta, s32 ticks) {
    game->unk_430 = ticks;
    game->unk_434 = delta;
    game->next_step = 1;
}

s32 cardgame_points_update(CardgameGame *game, CardgameBoard *board, s32 side, s32 kind) {
    s32 done = 0;

    if (game->next_step != 0) {
        switch (game->next_step) {
        case 1:
            board->set_lamps(board, (1 << kind * 2) << side);
            game->unk_424 = 0;
            game->unk_428 = 0;
            break;
        case 2:
            board->clear_lamps(board);
            break;
        }
        game->effect_step = game->next_step;
        game->next_step = 0;
    }
    switch (game->effect_step) {
    case 1:
        if (game->unk_424 == game->unk_430 / 2) {
            if (game->unk_434 > 0) {
                if (game->players[side].points[kind] < 99) {
                    game->players[side].points[kind]++;
                }
                game->unk_434--;
                sound_module.play(0x800452C6);
            } else {
                if (game->players[side].points[kind] != 0) {
                    game->players[side].points[kind]--;
                }
                game->unk_434++;
                sound_module.play(0x800452C6);
            }
            board->set_panel_value(board, side, kind, game->players[side].points[kind]);
        }
        game->unk_424++;
        if (game->unk_430 < game->unk_424) {
            if (game->unk_434 == 0 || game->players[side].points[kind] == 0) {
                game->next_step = 2;
            } else {
                game->next_step = 1;
            }
        }
        break;
    case 2:
        done = 1;
        break;
    }
    return done;
}

void cardgame_clear_points_start(CardgameGame *game, CardgameBoard *board) {
    cardgame_points_start(game, board, -0x80, 0x10);
    game->unk_438 = 0;
}

s32 cardgame_clear_points_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;
    s32 side;
    s32 k;
    s32 any;
    s32 side2;
    s32 k2;

    if (game->next_step != 0) {
        switch (game->next_step) {
        case 1:
            board->set_lamps(board, 0x3FF);
            game->unk_424 = 0;
            game->unk_428 = 0;
            break;
        case 2:
            board->clear_lamps(board);
            break;
        }
        game->effect_step = game->next_step;
        game->next_step = 0;
    }
    switch (game->effect_step) {
    case 1:
        if (game->unk_424 == game->unk_430 / 2) {
            for (side = 0; side < 2; side++) {
                for (k = 0; k < 5; k++) {
                    if (game->players[side].points[k] != 0) {
                        game->players[side].points[k]--;
                    }
                    board->set_panel_value(board, side, k, game->players[side].points[k]);
                }
            }
        }
        game->unk_424++;
        if (game->unk_430 < game->unk_424) {
            any = 0;
            for (side2 = 0; side2 < 2; side2++) {
                for (k2 = 0; k2 < 5; k2++) {
                    if (game->players[side2].points[k2] != 0) {
                        any = 1;
                    }
                }
            }
            if (any) {
                game->next_step = 1;
            } else {
                game->next_step = 2;
            }
        }
        break;
    case 2:
        done = 1;
        break;
    }
    return done;
}

void cardgame_cancel_card_start(CardgameGame *game, CardgameBoard *board) {
    game->unk_424 = 0;
    game->effect_step = 1;
    board->close_turn_mark(board, game->turn - 2);
}

s32 cardgame_cancel_card_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;
    s32 t;
    s32 side;
    s32 i;

    switch (game->effect_step) {
    case 1:
        if (cardgame_slot_animate_discard(game, board, game->turn + 10) != 0) {
            game->unk_428 = 10;
            game->effect_step = 2;
            side = game->turns[game->turn - 2].side;
            game->players[side].discard[game->players[side].discard_count] = game->turns[game->turn - 2].card;
            game->players[side].discard_count++;
            board->set_panel_value(board, 0, 7, game->players[0].discard_count);
            board->set_panel_value(board, 1, 7, game->players[1].discard_count);
        }
        break;
    case 2:
        game->unk_428 -= gfx_module.funcs.get_frame_ticks();
        if (game->unk_428 <= 0) {
            t = game->turn - 1;
            game->effect_step = 3;
            if (t >= 2) {
                game->turns[t - 2].mark = 0;
            }
            for (i = 0; i < 15; i++) {
                board->cards[i].turn_marks[t - 1] = 0;
            }
        }
        break;
    case 3:
        done = 1;
        break;
    }
    return done;
}

s32 cardgame_queue_swap_totals(CardgameGame *game, CardgameBoard *board) {
    s32 n = game->turns[game->turn - 1].card;

    game->swap_count++;
    game->swap_card = n + 1;
    return 1;
}

void cardgame_discard_card_start(CardgameGame *game, CardgameBoard *board, s32 side, s32 which) {
    s32 card;

    board->place_card(board, 17, 0xE500, 0x6100);
    card = 0;
    board->cards[17].scale_x = 0;
    switch (which) {
    case 0:
        card = game->players[side].hand[game->choice];
        game->unk_438 = card;
        break;
    case 1:
        card = game->players[side].deck[game->choice];
        game->unk_438 = card;
        break;
    }
    board->set_card(board, 17, card);
    board->zoom_card(board, 17, 8, 0x1000, 0x1000);
    game->unk_424 = 0;
    game->effect_step = 1;
}

s32 cardgame_discard_card_update(CardgameGame *game, CardgameBoard *board, s32 side, s32 which) {
    s32 done = 0;
    s32 i;
    s32 j;
    s32 k;

    switch (game->effect_step) {
    case 1:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 >= 21) {
            board->move_card(board, 17, 10, cardgame_discard_pos[side].x, cardgame_discard_pos[side].y);
            board->set_card_scale(board, 17, 0, 0);
            game->effect_step = 2;
        }
        break;
    case 2:
        if (board->cards[17].state == 1) {
            game->effect_step = 3;
            game->unk_424 = 0;
            game->players[side].discard[game->players[side].discard_count] = game->unk_438;
            game->players[side].discard_count++;
            switch (which) {
            case 0:
                for (i = game->choice; i < game->players[side].hand_count - 1; i++) {
                    game->players[side].hand[i] = game->players[side].hand[i + 1];
                }
                board->set_panel_value(board, side, 6, --game->players[side].hand_count);
                break;
            case 1:
                if (side == 0) {
                    for (j = game->choice; j >= game->players[side].deck_pos + 1; j--) {
                        game->players[side].deck[j] = game->players[side].deck[j - 1];
                    }
                    game->players[side].deck_pos++;
                    game->players[side].deck_count--;
                } else {
                    for (k = game->choice; k >= game->players[side].deck_pos + 1; k--) {
                        game->players[side].deck[k] = game->players[side].deck[k - 1];
                        game->cpu_deck_info[k] = game->cpu_deck_info[k - 1];
                    }
                    if (++game->cpu_deck_end >= 40) {
                        game->cpu_deck_end = 39;
                    }
                    if (++game->cpu_deck_last >= 40) {
                        game->cpu_deck_last = 39;
                    }
                    game->players[side].deck_pos++;
                    game->players[side].deck_count--;
                    for (k = 0; k < 40; k++) {
                        game->cpu_deck_info[k].pos = k;
                    }
                }
                board->set_panel_value(board, side, 5, game->players[side].deck_count);
                break;
            }
            board->set_panel_value(board, side, 7, game->players[side].discard_count);
        }
        break;
    case 3:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 >= 46) {
            game->effect_step = 4;
        }
        break;
    case 4:
        done = 1;
        break;
    }
    return done;
}

void cardgame_take_card_start(CardgameGame *game, CardgameBoard *board, s32 side, s32 which) {
    s32 i;
    s32 card;

    game->unk_438 = game->players[side].hand_count;
    if (which == 4) {
        card = game->players[side].discard[game->choice];
    } else {
        card = game->players[side].deck[game->choice];
    }
    game->players[side].hand[game->players[side].hand_count] = card;
    game->players[side].hand_count++;
    if (game->unk_438 != 0) {
        if (side == 0) {
            game->display.request = 5;
        } else {
            game->display.request = 11;
            game->display.face_down = 1;
        }
    } else {
        if (side == 0) {
            game->display.request = 17;
        } else {
            game->display.request = 18;
            game->display.face_down = 1;
        }
    }
    board->open_side_panel(board, side);
    for (i = 0; i < 40; i++) {
        game->display.dimmed[i] = 0;
    }
    game->effect_step = 1;
}

/* The opponent's deck-shift loops have their own counter (pos). */
s32 cardgame_take_card_update(CardgameGame *game, CardgameBoard *board, s32 side, s32 which) {
    CardPicture pic;
    s32 done = 0;
    s32 ok;
    s32 k;
    s32 m;
    s32 n;
    s32 j;
    s32 i;
    s32 pos;
    s32 t;
    s32 card;
    s32 v;
    s32 card2;

    switch (game->effect_step) {
    case 1:
        ok = 0;
        if (side == 0) {
            ok = board->panels[0].state == 2;
        } else if (board->panels[1].state == 2) {
            ok = 1;
        }
        n = game->players[side].hand_count - 1;
        board->cards[n].x = 0x14A00;
        if (ok && game->display.state == 0) {
            v = board->get_card_x(game->players[side].hand_count, n);
            board->cards[n].scale_x = 0x1000;
            board->move_card(board, n, 15, v + 0x1800, 0x6100);
            game->effect_step = 2;
        }
        break;
    case 2:
        k = game->players[side].hand_count - 1;
        if (board->cards[k].state == 1) {
            if (which == 4) {
                for (i = game->choice; i < game->players[side].discard_count - 1; i++) {
                    game->players[side].discard[i] = game->players[side].discard[i + 1];
                }
                game->players[side].discard_count--;
                game->effect_step = 4;
                game->unk_424 = 45;
            } else {
                if (side == 0) {
                    for (j = game->choice; j >= game->players[0].deck_pos + 1; j--) {
                        game->players[0].deck[j] = game->players[0].deck[j - 1];
                    }
                } else {
                    if (game->choice < game->cpu_deck_end) {
                        for (pos = game->choice; pos >= game->players[1].deck_pos + 1; pos--) {
                            game->players[1].deck[pos] = game->players[1].deck[pos - 1];
                            game->cpu_deck_info[pos] = game->cpu_deck_info[pos - 1];
                        }
                    } else {
                        if (game->cpu_deck_info[game->choice].stage == 7) {
                            for (pos = game->choice; pos >= game->players[1].deck_pos + 1; pos--) {
                                game->players[1].deck[pos] = game->players[1].deck[pos - 1];
                                game->cpu_deck_info[pos] = game->cpu_deck_info[pos - 1];
                            }
                            game->cpu_deck_end++;
                            game->cpu_deck_last++;
                        } else {
                            t = game->players[1].deck[game->choice];
                            game->players[1].deck[game->choice] = game->players[1].deck[39];
                            game->players[1].deck[39] = t;
                            for (pos = 39; pos >= game->players[1].deck_pos + 1; pos--) {
                                game->players[1].deck[pos] = game->players[1].deck[pos - 1];
                                game->cpu_deck_info[pos] = game->cpu_deck_info[pos - 1];
                            }
                            game->cpu_deck_end++;
                            game->cpu_deck_last++;
                        }
                    }
                    for (v = 0; v < 40; v++) {
                        game->cpu_deck_info[v].pos = v;
                    }
                }
                game->players[side].deck_pos++;
                game->players[side].deck_count--;
                card2 = game->players[side].hand[k];
                card_init(&pic);
                pic.select(game->card_ids[card2] + 1);
                if (pic.record[0] < 6) {
                    game->effect_step = 3;
                    board->flash_card(board, k);
                } else {
                    game->effect_step = 4;
                    game->unk_424 = 45;
                }
            }
            board->set_panel_value(board, 0, 5, game->players[0].deck_count);
            board->set_panel_value(board, 0, 6, game->players[0].hand_count);
            board->set_panel_value(board, 0, 7, game->players[0].discard_count);
            board->set_panel_value(board, 1, 5, game->players[1].deck_count);
            board->set_panel_value(board, 1, 6, game->players[1].hand_count);
            board->set_panel_value(board, 1, 7, game->players[1].discard_count);
        }
        break;
    case 3:
        m = game->players[side].hand_count - 1;
        if (board->cards[m].state == 1) {
            game->effect_step = 4;
            game->unk_424 = 45;
            card = game->players[side].hand[m];
            card_init(&pic);
            pic.select(game->card_ids[card] + 1);
            if (pic.record[0] < 6) {
                if (game->players[side].points[pic.record[0] - 1] < 99) {
                    game->players[side].points[pic.record[0] - 1]++;
                }
                board->set_panel_value(board, side, pic.record[0] - 1, game->players[side].points[pic.record[0] - 1]);
            }
        }
        break;
    case 4:
        game->unk_424 -= gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 <= 0) {
            game->effect_step = 5;
            board->close_side_panel(board, side);
            game->display.request = game->display.reopen;
        }
        break;
    case 5:
        t = 0;
        if (side == 0) {
            t = board->panels[0].state == 0;
        } else if (board->panels[1].state == 0) {
            t = 1;
        }
        if (t && game->display.state == 0) {
            game->effect_step = 6;
        }
        break;
    case 6:
        done = 1;
        break;
    }
    return done;
}

void cardgame_recycle_discard_start(CardgameGame *game, CardgameBoard *board, s32 side) {
    game->unk_424 = 0;
    game->unk_434 = 0;
    game->unk_428 = game->players[side].discard_count;
    game->unk_42C = game->players[side].deck_count;
    game->effect_step = 1;
}

s32 cardgame_recycle_discard_update(CardgameGame *game, CardgameBoard *board, s32 side) {
    s32 done = 0;
    CardgamePlayer *player;
    s16 *deck;
    s16 *discard;
    s32 more;
    s32 i;
    s32 j;

    switch (game->effect_step) {
    case 1:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        game->unk_434 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_434 >= 7) {
            more = 0;
            if (game->unk_428 > 0) {
                more = 1;
                game->unk_428--;
                game->unk_42C++;
            }
            if (!more) {
                game->effect_step = 2;
            }
            board->set_panel_value(board, side, 7, game->unk_428);
            board->set_panel_value(board, side, 5, game->unk_42C);
            game->unk_434 -= 7;
        }
        break;
    case 2:
        player = &game->players[side];
        deck = player->deck;
        discard = player->discard;
        if (side == 0) {
            j = player->deck_pos - 1;
            for (i = player->discard_count - 1; i >= 0; i--) {
                deck[j] = discard[i];
                j--;
                player->deck_pos--;
                player->deck_count++;
            }
            player->discard_count = 0;
            game->shuffle_deck(game, player->deck_pos, player->deck_count);
        } else {
            for (i = player->discard_count - 1; i >= 0; i--) {
                player->deck_pos--;
                player->deck_count++;
                for (j = player->deck_pos; j < 40; j++) {
                    deck[j] = deck[j + 1];
                    game->cpu_deck_info[j] = game->cpu_deck_info[j + 1];
                }
                game->cpu_deck_end--;
                game->cpu_deck_last--;
                deck[39] = discard[i];
                game->cpu_deck_info[39].stage = 7;
            }
            for (i = 0; i < 40; i++) {
                game->cpu_deck_info[i].pos = i;
            }
            player->discard_count = 0;
        }
        done = 1;
        break;
    }
    return done;
}

void cardgame_mark_draw_start(CardgameGame *game, CardgameBoard *board, s32 n) {
    s32 i;

    for (i = 0; i < 40; i++) {
        game->marked[i] = 0;
    }
    game->selected = n;
    game->target_rows = 0;
}

s32 cardgame_mark_draw_update(CardgameGame *game, CardgameBoard *board, s32 side) {
    s32 i;
    s32 n;
    s32 lo;
    s32 hi;

    if (game->selected != 0) {
        if (side == 0) {
            n = 0;
            for (i = game->players[0].deck_pos; i < game->players[0].deck_pos + game->selected; i++) {
                if (i >= 40) {
                    game->target_rows = 1;
                    game->selected = n;
                    break;
                }
                game->marked[i] = 1;
                n++;
            }
        } else {
            lo = game->players[1].deck_pos;
            hi = 39;
            if (game->selected > game->players[1].deck_count) {
                game->target_rows = 1;
                game->selected = game->players[1].deck_count;
            }
            for (i = 0; i < game->selected; i++) {
                if (game->cpu_deck_info[lo].stage == game->round * 2 + 2) {
                    game->marked[lo] = 1;
                    lo++;
                } else {
                    game->marked[hi] = 1;
                    hi--;
                }
            }
        }
    }
    return 1;
}

void cardgame_draw_start(CardgameGame *game, CardgameBoard *board, s32 side) {
    s32 i;
    s32 j;
    s32 card;

    game->unk_438 = game->players[side].hand_count;
    game->unk_42C = 0;
    for (i = game->players[side].deck_pos; i < 40; i++) {
        if (game->marked[i] != 0) {
            card = game->players[side].deck[i];
            game->players[side].hand[game->players[side].hand_count] = card;
            game->players[side].hand_count++;
            game->unk_42C = 1;
        }
    }
    for (j = 0; j < 40; j++) {
        game->display.dimmed[j] = 0;
    }
    if (game->unk_438 != 0) {
        if (side == 0) {
            game->display.request = 5;
        } else {
            game->display.request = 11;
            game->display.face_down = 1;
        }
    } else {
        if (side == 0) {
            game->display.request = 17;
        } else {
            game->display.request = 18;
            game->display.face_down = 1;
        }
    }
    board->open_side_panel(board, side);
    game->effect_step = 1;
}

/* The original reuses `v` (case 2) as the last loop's counter and `found` as case 9's flag (register
 * allocation), and repeats the deck counters' increments in both branches (the deck-shift loops are
 * entered at their test where the branches end in a jump). */
s32 cardgame_draw_update(CardgameGame *game, CardgameBoard *board, s32 side) {
    CardPicture pic;
    s32 done = 0;
    s32 ok;
    s32 found;
    s32 card;
    s32 v;
    s32 t;
    s32 i;
    s32 j;
    s32 k;

    switch (game->effect_step) {
    case 1:
        ok = 0;
        if (side == 0) {
            ok = board->panels[0].state == 2;
        } else if (board->panels[1].state == 2) {
            ok = 1;
        }
        for (i = game->unk_438; i < game->players[side].hand_count; i++) {
            board->cards[i].x = 0x14A00;
        }
        if (ok && game->display.state == 0) {
            if (game->unk_42C == 0) {
                board->open_dialog(board, 0x35, 0, 0, side == 0 ? 2 : 0);
                game->effect_step = 5;
            } else {
                game->effect_step = 2;
            }
            game->unk_424 = 0;
            game->unk_428 = game->unk_438;
        }
        break;
    case 2:
        v = board->get_card_x(game->players[side].hand_count, game->unk_428);
        board->cards[game->unk_428].scale_x = 0x1000;
        board->move_card(board, game->unk_428, 15, v + 0x1800, 0x6100);
        game->effect_step = 3;
        break;
    case 3:
        if (board->cards[game->unk_428].state == 1) {
            card = game->players[side].hand[game->unk_428];
            card_init(&pic);
            pic.select(game->card_ids[card] + 1);
            if (pic.record[0] < 6) {
                if (game->players[side].points[pic.record[0] - 1] < 99) {
                    game->players[side].points[pic.record[0] - 1]++;
                }
                board->set_panel_value(board, side, pic.record[0] - 1, game->players[side].points[pic.record[0] - 1]);
                board->flash_card(board, game->unk_428);
                game->effect_step = 4;
            } else {
                game->unk_428++;
                if (game->unk_428 < game->players[side].hand_count) {
                    game->effect_step = 2;
                } else if (game->target_rows != 0) {
                    board->open_dialog(board, 0x35, 0, 0, side == 0 ? 2 : 0);
                    game->effect_step = 5;
                } else {
                    game->unk_424 = 45;
                    game->effect_step = 8;
                }
            }
            for (i = 39, found = 0; i >= 0; i--) {
                if (game->marked[i] != 0) {
                    found = 1;
                    break;
                }
            }
            if (found) {
                game->marked[i] = 0;
                if (side == 0) {
                    for (k = i; k >= game->players[0].deck_pos + 1; k--) {
                        game->players[0].deck[k] = game->players[0].deck[k - 1];
                        game->marked[k] = game->marked[k - 1];
                    }
                } else {
                    if (i < game->cpu_deck_end) {
                        for (j = i; j >= game->players[1].deck_pos + 1; j--) {
                            game->players[1].deck[j] = game->players[1].deck[j - 1];
                            game->marked[j] = game->marked[j - 1];
                            game->cpu_deck_info[j] = game->cpu_deck_info[j - 1];
                        }
                    } else {
                        if (game->cpu_deck_info[i].stage == 7) {
                            for (j = i; j >= game->players[1].deck_pos + 1; j--) {
                                game->players[1].deck[j] = game->players[1].deck[j - 1];
                                game->marked[j] = game->marked[j - 1];
                                game->cpu_deck_info[j] = game->cpu_deck_info[j - 1];
                            }
                            game->cpu_deck_end++;
                            game->cpu_deck_last++;
                        } else {
                            t = game->players[1].deck[i];
                            game->players[1].deck[i] = game->players[1].deck[39];
                            game->players[1].deck[39] = t;
                            for (j = 39; j >= game->players[1].deck_pos + 1; j--) {
                                game->players[1].deck[j] = game->players[1].deck[j - 1];
                                game->marked[j] = game->marked[j - 1];
                                game->cpu_deck_info[j] = game->cpu_deck_info[j - 1];
                            }
                            game->cpu_deck_end++;
                            game->cpu_deck_last++;
                        }
                    }
                    for (v = 0; v < 40; v++) {
                        game->cpu_deck_info[v].pos = v;
                    }
                }
                game->players[side].deck_pos++;
                game->players[side].deck_count--;
            }
        }
        break;
    case 4:
        if (board->cards[game->unk_428].state == 1) {
            game->unk_428++;
            if (game->unk_428 < game->players[side].hand_count) {
                game->effect_step = 2;
            } else if (game->target_rows != 0) {
                board->open_dialog(board, 0x35, 0, 0, side == 0 ? 2 : 0);
                game->effect_step = 5;
            } else {
                game->unk_424 = 45;
                game->effect_step = 8;
            }
        }
        break;
    case 5:
        if (board->dialog.state == 2) {
            game->effect_step = 6;
        }
        break;
    case 6:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            game->effect_step = 7;
            board->close_dialog(board);
        }
        break;
    case 7:
        if (board->dialog.state == 0) {
            game->unk_424 = 45;
            game->effect_step = 8;
        }
        break;
    case 8:
        board->set_panel_value(board, 0, 5, game->players[0].deck_count);
        board->set_panel_value(board, 0, 6, game->players[0].hand_count);
        board->set_panel_value(board, 0, 7, game->players[0].discard_count);
        board->set_panel_value(board, 1, 5, game->players[1].deck_count);
        board->set_panel_value(board, 1, 6, game->players[1].hand_count);
        board->set_panel_value(board, 1, 7, game->players[1].discard_count);
        game->unk_424 -= gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 <= 0) {
            game->effect_step = 9;
            board->close_side_panel(board, side);
            game->display.request = game->display.reopen;
        }
        break;
    case 9:
        found = 0;
        if (side == 0) {
            found = board->panels[0].state == 0;
        } else if (board->panels[1].state == 0) {
            found = 1;
        }
        if (found && game->display.state == 0) {
            game->effect_step = 10;
        }
        break;
    case 10:
        done = 1;
        break;
    }
    return done;
}

void cardgame_remove_marked_start(CardgameGame *game, CardgameBoard *board) {
    game->unk_428 = 0;
    game->effect_step = 1;
}

s32 cardgame_remove_marked_update(CardgameGame *game, CardgameBoard *board, s32 mode) {
    s32 done = 0;
    s32 i;
    s32 found;

    switch (game->effect_step) {
    case 1:
        for (i = game->unk_428; i < 12; i++) {
            if (game->marked[i] != 0) {
                game->effect_step = 2;
                game->unk_424 = 0;
                game->unk_428 = i;
                game->unk_42C = i >= 6;
                game->unk_430 = i;
                if (i >= 6) {
                    game->unk_430 = i - 6;
                }
                break;
            }
        }
        if (i >= 12) {
            game->effect_step = 3;
        }
        break;
    case 2:
        if (mode == 0) {
            found = cardgame_slot_discard(game, board, game->unk_42C, game->unk_430);
        } else {
            found = cardgame_slot_return(game, board, game->unk_42C, game->unk_430);
        }
        if (found != 0) {
            game->effect_step = 1;
            game->unk_428++;
        }
        break;
    case 3:
        done = 1;
        break;
    }
    return done;
}

void cardgame_set_card_turn_marks(CardgameGame *game, CardgameBoard *board, s32 arg2, s32 i) {
    s32 n = game->turn - 1;
    s32 kind = board->cards[i].color + 1;
    s32 k;

    for (k = 0; k < n; k++) {
        board->cards[i].turn_marks[k] = 0;
        switch (game->turns[k].target_kind) {
        case 1:
            if (game->turns[k].side == 0) {
                if (i < 6) {
                    board->cards[i].turn_marks[k] = 1;
                }
            } else if (i >= 6) {
                board->cards[i].turn_marks[k] = 1;
            }
            break;
        case 2:
            if (game->turns[k].side == 0) {
                if (i >= 6) {
                    board->cards[i].turn_marks[k] = 1;
                }
            } else if (i < 6) {
                board->cards[i].turn_marks[k] = 1;
            }
            break;
        case 3:
            board->cards[i].turn_marks[k] = 1;
            break;
        case 4:
            if (kind != 1) {
                board->cards[i].turn_marks[k] = 1;
            }
            break;
        case 5:
            if (kind != 2) {
                board->cards[i].turn_marks[k] = 1;
            }
            break;
        case 6:
            if (kind == 3) {
                board->cards[i].turn_marks[k] = 1;
            }
            break;
        case 7:
            if (kind != 4) {
                board->cards[i].turn_marks[k] = 1;
            }
            break;
        case 8:
            if (kind == 6) {
                board->cards[i].turn_marks[k] = 1;
            }
            break;
        }
    }
}

void cardgame_summon_start(CardgameGame *game, CardgameBoard *board, s32 side, s32 card) {
    CardgameSlots *slots = &game->slots[side];
    CardPicture pic;
    s32 n;
    s32 k;

    k = n = slots->count;
    if (side != 0) {
        k += 6;
    }
    card_init(&pic);
    pic.select(game->card_ids[card] + 1);
    slots->slots[n].attack = pic.record[1];
    slots->slots[n].hp = pic.record[2];
    slots->slots[n].attack_bonus = 0;
    slots->slots[n].hp_bonus = 0;
    slots->slots[n].card = card;
    slots->slots[n].side = side;
    slots->slots[n].owner = 2;
    slots->slots[n].id = game->next_slot_id++;
    board->place_card(board, k, 0xE500, 0x6100);
    board->set_card(board, k, card);
    board->cards[k].attack = slots->slots[n].attack;
    board->cards[k].hp = slots->slots[n].hp;
    board->cards[k].scale_x = 0;
    cardgame_set_card_turn_marks(game, board, side, k);
    game->effect_step = 1;
    board->zoom_card(board, k, 20, 0x1000, 0x1000);
}

void cardgame_summon_from_hand_start(CardgameGame *game, CardgameBoard *board, s32 side) {
    CardgameSlots *slots = &game->slots[side];
    s32 k = slots->count;
    CardgamePlayer *player = &game->players[side];
    s32 n = k;
    s32 i;
    s32 j;
    s32 m;

    if (side != 0) {
        k += 6;
    }
    for (i = 0; i < player->hand_count; i++) {
        if (game->turns[game->turn - 1].target == player->hand[i]) {
            game->add_slot(game, side, game->turns[game->turn - 1].target);
            break;
        }
    }
    for (j = i; j < player->hand_count - 1; j++) {
        player->hand[j] = player->hand[j + 1];
    }
    player->hand_count--;
    slots->count--;
    board->place_card(board, k, 0xE500, 0x6100);
    board->set_card(board, k, slots->slots[n].card);
    for (m = 0; m < 5; m++) {
        cardgame_game_apply_bonus(game, &slots->slots[n], side, m);
    }
    board->cards[k].attack = slots->slots[n].attack;
    board->cards[k].hp = slots->slots[n].hp;
    board->cards[k].scale_x = 0;
    cardgame_set_card_turn_marks(game, board, side, k);
    game->effect_step = 1;
    board->set_panel_value(board, side, 6, player->hand_count);
    board->zoom_card(board, k, 20, 0x1000, 0x1000);
}

s32 cardgame_summon_update(CardgameGame *game, CardgameBoard *board, s32 side) {
    s32 done = 0;
    CardgameSlots *slots = &game->slots[side];
    s32 k = slots->count;
    s32 x;
    s32 y;

    if (side != 0) {
        k += 6;
    }
    if (side == 0) {
        x = cardgame_slot_origins[main_screen_pos][0].x + slots->count * 0x2900;
    } else {
        x = cardgame_slot_origins[main_screen_pos][1].x + slots->count * 0x2900;
    }
    if (side == 0) {
        y = cardgame_slot_origins[main_screen_pos][0].y;
    } else {
        y = cardgame_slot_origins[main_screen_pos][1].y;
    }
    switch (game->effect_step) {
    case 1:
        if (board->cards[k].state == 1) {
            board->flash_card(board, k);
            game->effect_step = 2;
        }
        break;
    case 2:
        if (board->cards[k].state == 1) {
            board->move_card(board, k, 20, x, y);
            board->cards[k].zooming = 1;
            game->effect_step = 3;
        }
        break;
    case 3:
        if (board->cards[k].state == 1) {
            game->effect_step = 4;
            board->cards[k].zooming = 0;
            slots->count++;
        }
        break;
    case 4:
        done = 1;
        break;
    }
    return done;
}

void cardgame_copy_slot_start(CardgameGame *game, CardgameBoard *board, s32 side) {
    CardgameSlots *slots = &game->slots[side];
    s32 k = slots->count;
    s32 n = k;
    s32 found;
    CardgameSlot *src;
    s32 i;
    s32 j;

    if (side != 0) {
        k += 6;
    }
    found = 0;
    src = &slots->slots[n];
    for (i = 0; i < 12; i++) {
        if (i < 6) {
            if (i >= game->slots[0].count) {
                continue;
            }
            src = &game->slots[0].slots[i];
        } else {
            if (i - 6 >= game->slots[1].count) {
                continue;
            }
            src = &game->slots[1].slots[i - 6];
        }
        if (game->marked[i] != 0) {
            found = i;
            break;
        }
    }
    slots->slots[n] = *src;
    board->place_card(board, k, 0xE500, 0x6100);
    board->set_card(board, k, slots->slots[n].card);
    board->cards[k] = board->cards[found];
    board->cards[k].scale_x = 0;
    cardgame_set_card_turn_marks(game, board, side, k);
    for (j = 0; j < game->turn - 1; j++) {
        if (game->turns[j].target_kind == 0 && board->cards[found].turn_marks[j] != 0) {
            board->cards[k].turn_marks[j] = 1;
        }
    }
    game->effect_step = 1;
    board->zoom_card(board, found, 5, 0, 0x1000);
    game->unk_424 = 7;
}

s32 cardgame_copy_slot_update(CardgameGame *game, CardgameBoard *board, s32 side) {
    s32 done = 0;
    CardgameSlots *slots = &game->slots[side];
    s32 k = slots->count;
    s32 x;
    s32 y;

    if (side != 0) {
        k += 6;
    }
    if (side == 0) {
        x = cardgame_slot_origins[main_screen_pos][0].x + slots->count * 0x2900;
    } else {
        x = cardgame_slot_origins[main_screen_pos][1].x + slots->count * 0x2900;
    }
    if (side == 0) {
        y = cardgame_slot_origins[main_screen_pos][0].y;
    } else {
        y = cardgame_slot_origins[main_screen_pos][1].y;
    }
    switch (game->effect_step) {
    case 1:
        game->unk_424 -= gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 <= 0) {
            board->zoom_card(board, k, 5, 0x1000, 0x1000);
            game->effect_step = 2;
        }
        break;
    case 2:
        if (board->cards[k].state == 1) {
            board->flash_card(board, k);
            game->effect_step = 3;
        }
        break;
    case 3:
        if (board->cards[k].state == 1) {
            board->move_card(board, k, 20, x, y);
            board->cards[k].zooming = 1;
            game->effect_step = 4;
        }
        break;
    case 4:
        if (board->cards[k].state == 1) {
            game->effect_step = 5;
            board->cards[k].zooming = 0;
            slots->count++;
        }
        break;
    case 5:
        done = 1;
        break;
    }
    return done;
}

void cardgame_play_start(CardgameGame *game, CardgameBoard *board, s32 mode) {
    s32 i;
    s32 j;

    if (mode == 0 || board->panels[0].state == 0) {
        if (board->panels[0].state == 0) {
            board->reset_panels(board);
            game->choice = 0;
            board->place_card(board, 15, 0xE500, 0x6100);
            board->set_card(board, 15, game->turns[game->turn].card);
            board->cards[15].scale_x = 0;
            board->zoom_card(board, 15, 8, 0x1000, 0x1000);
            board->open_panels(board);
            game->display.set_dimmed = 1;
            game->display.dimmed[15] = 0;
            if (mode == 0) {
                for (j = 0; j < 15; j++) {
                    game->marked[j] = 0;
                }
            }
            game->display.request = 1;
            game->effect_step = 1;
        } else {
            board->move_card(board, 15, 10, cardgame_turn_card_pos[game->turn].x, cardgame_turn_card_pos[game->turn].y);
            game->effect_step = 5;
            game->unk_424 = 0;
        }
    } else {
        board->place_card(board, 15, 0xE500, 0x6100);
        board->set_card(board, 15, game->turns[game->turn].card);
        board->cards[15].scale_x = 0;
        board->zoom_card(board, 15, 8, 0x1000, 0x1000);
        game->effect_step = 2;
        game->unk_424 = 0;
        game->unk_428 = 0;
    }
    board->clear_lamps(board);
    for (i = 0; i < 15; i++) {
        board->cards[i].dimmed = 0;
    }
}

s32 cardgame_play_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;
    s32 id;
    s32 i;
    s32 j;
    s32 k;

    switch (game->effect_step) {
    case 2:
        board->popups[1].show_value = 0;
        board->popups[1].value = 0;
        board->popups[2].value = 0;
        board->popups[3].value = 0;
        board->popups[4].value = 0;
        game->unk_428 = cardgame_info_open_windows(game, board, 2, 0, game->unk_424, game->unk_428);
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 >= 21) {
            game->unk_424 = 0;
            game->unk_428 = 0;
            game->effect_step = 3;
        }
        break;
    case 3:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            game->effect_step = 4;
        }
        id = game->card_ids[game->turns[game->turn].card] + 1;
        board->popups[4].card_stats[2] = 0;
        board->popups[2].value = id;
        board->popups[4].value = id;
        break;
    case 4:
        board->move_card(board, 15, 10, cardgame_turn_card_pos[game->turn].x, cardgame_turn_card_pos[game->turn].y);
        board->close_popup(board, 4);
        board->close_popup(board, 1);
        board->close_popup(board, 2);
        board->close_popup(board, 3);
        game->effect_step = 5;
        game->unk_424 = 0;
        game->unk_428 = 0;
        break;
    case 1:
        if (board->panels[0].state == 2 && game->display.state == 0) {
            board->move_card(board, 15, 10, cardgame_turn_card_pos[game->turn].x, cardgame_turn_card_pos[game->turn].y);
            game->effect_step = 5;
            game->unk_424 = 0;
        }
        break;
    case 5:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 >= 12) {
            game->unk_424 = 0;
            game->effect_step = 6;
            board->flash_card(board, 15);
            for (i = 0; i < 15; i++) {
                if (game->marked[i] != 0) {
                    board->flash_card(board, i);
                }
            }
        }
        break;
    case 6:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 >= 21) {
            game->effect_step = 7;
            game->unk_424 = 0;
            board->zoom_card(board, 15, 5, 0, 0x1000);
            for (j = 0; j < 15; j++) {
                if (game->marked[j] != 0) {
                    board->cards[j].turn_marks[game->turn] = 1;
                }
            }
        }
        break;
    case 7:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 >= 7) {
            game->unk_424 = 0;
            board->cards[15].turn_badge = game->turn + 1;
            board->zoom_card(board, 15, 8, 0x1000, 0x1000);
            game->effect_step = 8;
            board->open_turn_mark(board, game->turn, game->side);
        }
        break;
    case 8:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 >= 11) {
            game->unk_424 = 0;
            game->effect_step = 9;
            board->cards[game->turn + 12] = board->cards[15];
            board->remove_card(board, 15);
            for (k = 0; k < 12; k++) {
                board->cards[k].highlight &= ~1;
            }
        }
        break;
    case 9:
        done = 1;
        break;
    }
    return done;
}

void cardgame_deal_start(CardgameGame *game, CardgameBoard *board) {
    game->players[0].hand_count = 0;
    game->players[1].hand_count = 0;
    board->set_panel_value(board, 0, 6, game->players[0].hand_count);
    board->set_panel_value(board, 1, 6, game->players[1].hand_count);
    board->set_panel_value(board, 0, 5, game->players[0].deck_count);
    board->set_panel_value(board, 1, 5, game->players[1].deck_count);
    game->choice = 0;
    game->next_step = 1;
}

void cardgame_deal_cards(CardgameGame *game, CardgameBoard *board) {
    s32 side;
    s32 k;
    s32 i;
    s32 j;

    game->unk_424 = 0;
    game->unk_428 = 0;
    game->unk_434 = 0;
    game->unk_42C = game->players[0].deck_count;
    game->unk_430 = game->players[1].deck_count;
    for (side = 0; side < 2; side++) {
        for (k = 0; k < 6; k++) {
            game->players[side].hand[k] = game->players[side].deck[game->players[side].deck_pos];
            game->players[side].hand_count++;
            game->players[side].deck_pos++;
            game->players[side].deck_count--;
        }
    }
    for (i = 0; i < 6; i++) {
        board->place_card(board, i, cardgame_slot_origins[main_screen_pos][0].x + 0x14800, cardgame_slot_origins[main_screen_pos][0].y);
        board->set_card(board, i, game->players[0].hand[i]);
        board->cards[i].style = 2;
        board->cards[i].scale_y = 0;
        board->cards[i].scale_x = 0;
    }
    for (j = 0; j < 6; j++) {
        board->place_card(board, j + 6, cardgame_slot_origins[main_screen_pos][1].x + 0x14800,
                       cardgame_slot_origins[main_screen_pos][1].y);
        board->set_card(board, j + 6, game->players[1].hand[j]);
        board->cards[j + 6].style = 2;
        board->cards[j + 6].scale_y = 0;
        board->cards[j + 6].scale_x = 0;
    }
}

s32 cardgame_deal_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;

    if (game->next_step != 0) {
        switch (game->next_step) {
        case 1:
            board->open_panels(board);
            break;
        case 2:
            cardgame_deal_cards(game, board);
            break;
        case 3:
        case 4:
            game->unk_424 = 0;
            game->unk_428 = 0;
            game->unk_434 = 0;
            break;
        case 5:
            game->unk_424 = 0;
            game->unk_428 = 0;
            game->unk_434 = 0;
            board->close_panels(board);
            break;
        case 6:
            board->set_lamps(board, 0x1000);
            board->open_dialog(board, 0x21, 0, 1, 1);
            game->choice = 1;
            break;
        case 7:
            sound_module.play(0x6004001E);
            board->set_lamps(board, 0x2000);
            board->open_dialog(board, 0x22, 0, 1, 1);
            game->choice = 2;
            break;
        case 9:
            game->unk_424 = 0;
            game->unk_428 = 0;
            game->unk_434 = 0;
            board->close_panels(board);
            board->close_dialog(board);
            break;
        case 11:
            board->open_dialog(board, game->prize, 0, 0, 3);
            break;
        case 12:
            break;
        }
        game->effect_step = game->next_step;
        game->next_step = 0;
    }
    switch (game->effect_step) {
    case 1:
        if (board->panels[0].state == 2) {
            if (game->players[0].deck_count < 6) {
                game->next_step = 6;
            } else if (game->players[1].deck_count < 6) {
                game->next_step = 7;
            } else {
                game->next_step = 2;
            }
        }
        break;
    case 2:
        if (game->unk_434 >= 7) {
            if (game->unk_428 < 6) {
                board->move_card(board, game->unk_428, 20, cardgame_slot_origins[main_screen_pos][0].x + game->unk_428 * 0x2900,
                               cardgame_slot_origins[main_screen_pos][0].y);
                board->set_card_scale(board, game->unk_428, 0x1000, 0x1000);
                board->move_card(board, game->unk_428 + 6, 20,
                               cardgame_slot_origins[main_screen_pos][1].x + game->unk_428 * 0x2900,
                               cardgame_slot_origins[main_screen_pos][1].y);
                board->set_card_scale(board, game->unk_428 + 6, 0x1000, 0x1000);
                game->unk_428++;
                game->unk_42C--;
                game->unk_430--;
                board->set_panel_value(board, 0, 6, game->unk_428);
                board->set_panel_value(board, 1, 6, game->unk_428);
                board->set_panel_value(board, 0, 5, game->unk_42C);
                board->set_panel_value(board, 1, 5, game->unk_430);
            }
            game->unk_434 -= 7;
        }
        if (game->unk_424 >= 61) {
            game->next_step = 3;
        }
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        game->unk_434 += gfx_module.funcs.get_frame_ticks();
        break;
    case 3:
        if (game->unk_434 >= 7) {
            if (game->unk_428 < 6) {
                board->flip_card(board, game->unk_428);
                game->unk_428++;
            }
            game->unk_434 -= 7;
        }
        if (game->unk_424 >= 66) {
            game->next_step = 4;
        }
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        game->unk_434 += gfx_module.funcs.get_frame_ticks();
        break;
    case 4:
        if (game->unk_434 >= 7) {
            if (game->unk_428 < 6) {
                if (cardgame_player_add_card_point(game, board, 0, game->players[0].hand[game->unk_428]) != 0) {
                    board->flash_card(board, game->unk_428);
                }
                if (cardgame_player_add_card_point(game, board, 1, game->players[1].hand[game->unk_428]) != 0) {
                    board->flash_card(board, game->unk_428 + 6);
                }
                game->unk_428++;
            }
            game->unk_434 -= 7;
        }
        if (game->unk_424 >= 81) {
            game->next_step = 5;
        }
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        game->unk_434 += gfx_module.funcs.get_frame_ticks();
        break;
    case 5:
        if (game->unk_434 >= 3) {
            if (game->unk_428 < 6) {
                board->zoom_card(board, game->unk_428, 5, 0, 0x1000);
                board->zoom_card(board, game->unk_428 + 6, 5, 0, 0x1000);
                game->unk_428++;
            }
            game->unk_434 -= 3;
        }
        if (game->unk_424 >= 41) {
            game->next_step = 10;
        }
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        game->unk_434 += gfx_module.funcs.get_frame_ticks();
        break;
    case 6:
    case 7:
        if (board->dialog.state == 2) {
            game->next_step = 8;
        }
        break;
    case 8:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            game->next_step = 9;
        }
        break;
    case 9:
        if (board->dialog.state == 0 && board->panels[0].state == 0) {
            game->next_step = 10;
        }
        break;
    case 10:
        if (game->choice == 2) {
            game->next_step = 11;
        } else {
            done = 1;
        }
        break;
    case 11:
        if (board->dialog.state == 2) {
            game->next_step = 12;
        }
        break;
    case 12:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            done = 1;
        }
        break;
    }
    return done;
}

void cardgame_attack_start(CardgameGame *game, CardgameBoard *board, s32 side) {
    game->unk_424 = 0;
    game->unk_428 = 0;
    game->unk_42C = game->players[side].attack;
    game->unk_430 = game->players[side ^ 1].hp << 8;
    game->players[side ^ 1].hp -= game->players[side].attack;
    if (game->players[side ^ 1].hp < 0) {
        game->players[side ^ 1].hp = 0;
    }
    if (game->slots[side].count != 0) {
        game->unk_434 = (game->unk_430 - (game->players[side ^ 1].hp << 8)) / (game->slots[side].count * 28 - 16);
        if (game->unk_434 == 0) {
            game->unk_434 = 1;
        }
    } else {
        game->unk_434 = 1;
    }
    game->effect_step = 1;
}

s32 cardgame_attack_update(CardgameGame *game, CardgameBoard *board, s32 side) {
    s32 done = 0;
    s32 other = side ^ 1;
    s32 k;
    s32 x;
    s32 v;
    s32 i;

    switch (game->effect_step) {
    case 1:
        if (game->unk_424 % 27 == 0 && game->unk_428 < game->slots[side].count) {
            k = game->unk_428;
            if (side != 0) {
                k += 6;
            }
            x = cardgame_slot_origins[main_screen_pos][0].x + (game->slots[other].count - 1) * 0x1480;
            if (main_screen_pos != 0) {
                board->lunge_card(board, k, 6, x, side != 0 ? 0x6D00 : 0x5500);
            } else {
                board->lunge_card(board, k, 6, x, 0x6100);
            }
            game->unk_428++;
            board->cards[k].zooming = 1;
        }
        if (game->unk_424 >= 16) {
            v = 0;
            if (game->unk_424 < game->slots[side].count * 28) {
                v = game->unk_42C - (game->unk_42C / (game->slots[side].count * 56 + 1) + 1) * game->unk_424;
                if (v < 0) {
                    v = 0;
                }
            }
            board->set_panel_value(board, side, 8, v);
            if (game->unk_424 < game->slots[side].count * 28) {
                game->unk_430 -= game->unk_434;
                if (game->unk_430 < game->players[other].hp << 8) {
                    game->unk_430 = game->players[other].hp << 8;
                }
            } else {
                game->unk_430 = game->players[other].hp << 8;
            }
            board->set_panel_value(board, other, 9, game->unk_430 >> 8);
        }
        if (board->card_flags & 2) {
            for (i = 0; i < game->slots[other].count; i++) {
                board->shake_card(board, side == 0 ? i + 6 : i);
            }
        }
        game->unk_424++;
        if (game->slots[side].count * 28 + 25 < game->unk_424) {
            game->effect_step = 2;
        }
        break;
    case 2:
        done = 1;
        break;
    }
    return done;
}

void cardgame_discard_slots_start(CardgameGame *game, CardgameBoard *board, s32 side) {
    game->unk_424 = 0;
    game->unk_428 = game->slots[side].count - 1;
    game->effect_step = 1;
}

s32 cardgame_discard_slots_update(CardgameGame *game, CardgameBoard *board, s32 side) {
    s32 done = 0;

    switch (game->effect_step) {
    case 1:
        if (cardgame_slot_discard(game, board, side, game->unk_428) != 0) {
            game->unk_424 = 0;
            if (--game->unk_428 < 0) {
                game->effect_step = 2;
                game->slots[side].count = 0;
            }
        }
        break;
    case 2:
        done = 1;
        break;
    }
    return done;
}

s32 cardgame_lerp(s32 to, s32 from, s32 total, s32 t) {
    s32 d = to - from;
    s32 before;

    if (t == total || from == to) {
        return to;
    }
    from += d * t / total;
    if (d > 0) {
        before = from < to;
    } else {
        before = to < from;
    }
    if (!before) {
        from = to;
    }
    return from;
}

void cardgame_swap_totals_start(CardgameGame *game, CardgameBoard *board) {
    board->place_card(board, 17, 0x8300, 0x6100);
    board->set_card(board, 17, game->swap_card - 1);
    board->cards[17].scale_x = 0;
    board->zoom_card(board, 17, 10, 0x1000, 0x1000);
    game->effect_step = 1;
}

s32 cardgame_swap_totals_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;
    s32 t0;
    s32 t2;

    switch (game->effect_step) {
    case 1:
        if (board->cards[17].state == 1) {
            board->flash_card(board, 17);
            game->effect_step = 2;
        }
        break;
    case 2:
        if (board->cards[17].state == 1) {
            game->effect_step = 3;
            game->unk_424 = 0;
        }
        break;
    case 3:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 < 15) {
            board->set_panel_value(board, 0, 8, cardgame_lerp(game->players[1].attack, game->players[0].attack, 15, game->unk_424));
            board->set_panel_value(board, 0, 9, cardgame_lerp(game->players[1].hp, game->players[0].hp, 15, game->unk_424));
            board->set_panel_value(board, 1, 8, cardgame_lerp(game->players[0].attack, game->players[1].attack, 15, game->unk_424));
            board->set_panel_value(board, 1, 9, cardgame_lerp(game->players[0].hp, game->players[1].hp, 15, game->unk_424));
        } else {
            t0 = game->players[0].attack;
            t2 = game->players[0].hp;
            game->players[0].attack = game->players[1].attack;
            game->players[0].hp = game->players[1].hp;
            game->players[1].attack = t0;
            game->players[1].hp = t2;
            board->set_panel_value(board, 0, 8, game->players[0].attack);
            board->set_panel_value(board, 0, 9, game->players[0].hp);
            board->set_panel_value(board, 1, 8, game->players[1].attack);
            board->set_panel_value(board, 1, 9, game->players[1].hp);
            board->zoom_card(board, 17, 10, 0, 0x1000);
            game->effect_step = 4;
        }
        break;
    case 4:
        if (board->cards[17].state == 1) {
            game->effect_step = 5;
            board->open_dialog(board, 0x2D, 0, 0, 1);
        }
        break;
    case 5:
        if (board->dialog.state == 2) {
            game->effect_step = 6;
        }
        break;
    case 6:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            game->effect_step = 7;
            board->close_dialog(board);
        }
        break;
    case 7:
        if (board->dialog.state == 0) {
            game->effect_step = 8;
        }
        break;
    case 8:
        done = 1;
        break;
    }
    return done;
}

void cardgame_combo_start(CardgameGame *game, CardgameBoard *board, s32 side) {
    s32 n;
    s32 card;
    s32 base;
    s32 i;
    s32 j;
    s32 k;

    game->unk_42C = 0;
    game->unk_430 = 0;
    n = 0;
    for (i = 0; i < game->slots[side].count; i++) {
        if (game->selectable[i] != 0) {
            game->unk_42C += game->slots[side].slots[i].attack;
            if (game->unk_42C >= 100) {
                game->unk_42C = 99;
            }
            game->unk_430 += game->slots[side].slots[i].hp;
            if (game->unk_430 >= 100) {
                game->unk_430 = 99;
            }
            n++;
        }
    }
    if (n >= 4) {
        game->unk_42C += 20;
        if (game->unk_42C >= 100) {
            game->unk_42C = 99;
        }
        game->unk_430 += 20;
        if (game->unk_430 >= 100) {
            game->unk_430 = 99;
        }
    }
    board->place_card(board, 17, 0x8300, 0x6100);
    card = 0;
    for (j = 89; j < game->card_count; j++) {
        if (game->card_ids[j] == game->unk_438 - 1) {
            card = j;
            break;
        }
    }
    if (card == 0) {
        card = 87;
        game->unk_438 = 315;
    }
    board->set_card(board, 17, card);
    board->cards[17].color = 5;
    board->cards[17].attack = game->unk_42C;
    board->cards[17].hp = game->unk_430;
    base = 0;
    if (side != 0) {
        base = 6;
    }
    for (k = 0; k < game->slots[side].count; k++) {
        if (game->selectable[k] != 0) {
            board->cards[base + k].highlight |= 4;
        }
    }
    board->cards[17].scale_x = 0;
    game->unk_424 = 0;
    game->effect_step = 1;
}

s32 cardgame_combo_update(CardgameGame *game, CardgameBoard *board, s32 side) {
    s32 done = 0;
    s32 base;
    s32 first;
    s32 k;
    s32 i;
    s32 j;
    s32 v;

    switch (game->effect_step) {
    case 1:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 >= 21) {
            game->effect_step = 2;
            base = 0;
            if (side != 0) {
                base = 6;
            }
            for (i = 0; i < game->slots[side].count; i++) {
                if (game->selectable[i] != 0) {
                    k = base + i;
                    board->flash_card(board, k);
                    game->unk_434 = k;
                }
            }
        }
        break;
    case 2:
        if (board->cards[game->unk_434].state == 1) {
            board->zoom_card(board, 17, 10, 0x1000, 0x1000);
            first = 0;
            game->effect_step = 3;
            if (side != 0) {
                first = 6;
            }
            for (j = 0; j < game->slots[side].count; j++) {
                if (game->selectable[j] != 0) {
                    board->cards[first + j].highlight &= ~4;
                }
            }
        }
        break;
    case 3:
        if (board->cards[17].state == 1) {
            game->effect_step = 4;
            board->open_popup(board, 2, 1, game->unk_438, cardgame_popup_pos[main_screen_pos][side].x,
                           cardgame_popup_pos[main_screen_pos][side].y);
        }
        break;
    case 4:
        if (board->popups[2].state == 2) {
            game->effect_step = 5;
            game->unk_424 = 90;
        }
        break;
    case 5:
        game->unk_424 -= gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 <= 0 || PAD_PRESSED(13) || PAD_PRESSED(14)) {
            game->effect_step = 6;
            board->close_popup(board, 2);
        }
        break;
    case 6:
        if (board->popups[2].state == 0) {
            board->flash_card(board, 17);
            game->effect_step = 7;
        }
        break;
    case 7:
        if (board->cards[17].state == 1) {
            game->effect_step = 8;
            game->unk_424 = 0;
        }
        break;
    case 8:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 < 20) {
            v = game->players[side].attack;
            board->set_panel_value(board, side, 8, cardgame_lerp(v + game->unk_42C, v, 20, game->unk_424));
            v = game->players[side].hp;
            board->set_panel_value(board, side, 9, cardgame_lerp(v + game->unk_430, v, 20, game->unk_424));
        } else {
            game->players[side].attack += game->unk_42C;
            game->players[side].hp += game->unk_430;
            board->set_panel_value(board, side, 8, game->players[side].attack);
            board->set_panel_value(board, side, 9, game->players[side].hp);
            board->zoom_card(board, 17, 10, 0, 0x1000);
            game->effect_step = 9;
        }
        break;
    case 9:
        if (board->cards[17].state == 1) {
            game->effect_step = 10;
        }
        break;
    case 10:
        done = 1;
        break;
    }
    return done;
}

void cardgame_recount_totals_start(CardgameGame *game, CardgameBoard *board) {
    game->effect_step = 1;
    game->unk_424 = 0;
    game->unk_428 = 0;
    game->unk_42C = 0;
    game->unk_430 = 0;
    game->unk_434 = 0;
}

s32 cardgame_recount_totals_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;
    s32 i;
    s32 j;

    switch (game->effect_step) {
    case 1:
        for (i = 0; i < game->slots[0].count; i++) {
            game->unk_428 += game->slots[0].slots[i].attack;
            game->unk_42C += game->slots[0].slots[i].hp;
        }
        for (j = 0; j < game->slots[1].count; j++) {
            game->unk_430 += game->slots[1].slots[j].attack;
            game->unk_434 += game->slots[1].slots[j].hp;
        }
        if (game->players[0].attack != game->unk_428 || game->players[0].hp != game->unk_42C ||
            game->players[1].attack != game->unk_430 || game->players[1].hp != game->unk_434) {
            game->unk_424 = 0;
            game->effect_step = 2;
        } else {
            game->effect_step = 4;
        }
        break;
    case 2:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 < 15) {
            sound_module.play(0x800452C6);
            board->set_panel_value(board, 0, 8, cardgame_lerp(game->unk_428, game->players[0].attack, 15, game->unk_424));
            board->set_panel_value(board, 0, 9, cardgame_lerp(game->unk_42C, game->players[0].hp, 15, game->unk_424));
            board->set_panel_value(board, 1, 8, cardgame_lerp(game->unk_430, game->players[1].attack, 15, game->unk_424));
            board->set_panel_value(board, 1, 9, cardgame_lerp(game->unk_434, game->players[1].hp, 15, game->unk_424));
        } else {
            game->players[0].attack = game->unk_428;
            game->players[0].hp = game->unk_42C;
            game->players[1].attack = game->unk_430;
            game->players[1].hp = game->unk_434;
            board->set_panel_value(board, 0, 8, game->players[0].attack);
            board->set_panel_value(board, 0, 9, game->players[0].hp);
            board->set_panel_value(board, 1, 8, game->players[1].attack);
            board->set_panel_value(board, 1, 9, game->players[1].hp);
            game->effect_step = 3;
        }
        break;
    case 3:
        game->effect_step = 4;
        break;
    case 4:
        done = 1;
        break;
    }
    return done;
}

void cardgame_clear_slots_start(CardgameGame *game, CardgameBoard *board) {
    game->effect_step = 1;
    game->unk_424 = 0;
    game->unk_428 = 0;
    game->unk_42C = 0;
    game->unk_430 = 0;
    game->unk_434 = 0;
    game->unk_438 = 0;
}

s32 cardgame_clear_slots_update(CardgameGame *game, CardgameBoard *board, s32 side) {
    s32 done = 0;
    s32 k;
    s32 more;

    switch (game->effect_step) {
    case 1:
        if (game->unk_438 >= 3) {
            if (game->unk_428 < game->slots[side].count) {
                k = game->unk_428;
                if (side != 0) {
                    k += 6;
                }
                board->zoom_card(board, k, 4, 0, 0x1000);
                cardgame_slot_to_discard(game, board, side, game->unk_428);
                game->unk_428++;
            }
            game->unk_438 -= 3;
        }
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        game->unk_438 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_424 >= 61) {
            game->slots[side].count = 0;
            game->unk_424 = 0;
            game->effect_step = 2;
            game->unk_438 = 0;
            game->unk_428 = game->players[0].hand_count;
            game->unk_42C = game->players[0].deck_count;
            game->unk_430 = game->players[1].hand_count;
            game->unk_434 = game->players[1].deck_count;
        }
        break;
    case 2:
        game->unk_424 += gfx_module.funcs.get_frame_ticks();
        game->unk_438 += gfx_module.funcs.get_frame_ticks();
        if (game->unk_438 >= 3) {
            more = 0;
            if (game->unk_428 > 0) {
                more = 1;
                game->unk_428--;
                game->unk_42C++;
            }
            if (game->unk_430 > 0) {
                more = 1;
                game->unk_430--;
                game->unk_434++;
            }
            if (!more) {
                game->effect_step = 4;
                board->open_popup(board, 5, 5, 0x14, 0, 0x6E);
            } else {
                sound_module.play(0x800452C6);
            }
            board->set_panel_value(board, 0, 6, game->unk_428);
            board->set_panel_value(board, 0, 5, game->unk_42C);
            board->set_panel_value(board, 1, 6, game->unk_430);
            board->set_panel_value(board, 1, 5, game->unk_434);
            game->unk_438 -= 3;
        }
        break;
    case 3:
        if (board->popups[5].state == 2) {
            game->effect_step = 4;
        }
        break;
    case 4:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            game->effect_step = 5;
            board->close_popup(board, 5);
        }
        break;
    case 5:
        if (board->popups[5].state == 0) {
            game->effect_step = 6;
            board->close_panels(board);
        }
        break;
    case 6:
        if (board->panels[0].state == 0) {
            game->effect_step = 7;
        }
        break;
    case 7:
        done = 1;
        break;
    }
    return done;
}

void cardgame_message_start(CardgameGame *game, CardgameBoard *board, s32 arg2) {
    game->unk_424 = 0;
    board->open_dialog(board, arg2, 0, 0, 1);
    game->effect_step = 1;
}

s32 cardgame_message_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;

    switch (game->effect_step) {
    case 1:
        if (board->dialog.state == 2) {
            game->effect_step = 2;
        }
        break;
    case 2:
        if (((pad_state.get_pressed(0) >> pad_state.get_button_map(0, 13)) & 1) ||
            ((pad_state.get_pressed(0) >> pad_state.get_button_map(0, 14)) & 1)) {
            game->effect_step = 3;
            board->close_dialog(board);
        }
        break;
    case 3:
        if (board->dialog.state == 0) {
            game->effect_step = 4;
        }
        break;
    case 4:
        done = 1;
        break;
    }
    return done;
}

void cardgame_hide_panels_start(CardgameGame *game, CardgameBoard *board, s32 force) {
    if (game->turns[game->turn - 1].side == 0 || force != 0) {
        game->turn--;
        board->close_panels(board);
        game->display.request = 2;
        game->effect_step = 1;
    } else {
        game->effect_step = 2;
    }
}

s32 cardgame_hide_panels_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;

    switch (game->effect_step) {
    case 1:
        if (board->panels[0].state == 0 && game->display.state == 0) {
            game->effect_step = 2;
            game->turn++;
        }
        break;
    case 2:
        done = 1;
        break;
    }
    return done;
}

void cardgame_show_panels_start(CardgameGame *game, CardgameBoard *board, s32 force) {
    if (game->turns[game->turn - 1].side == 0 || force != 0) {
        board->open_panels(board);
        game->display.set_dimmed = 1;
        game->display.request = 1;
        game->effect_step = 1;
        game->turn--;
    } else {
        game->effect_step = 2;
    }
}

s32 cardgame_show_panels_update(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;

    switch (game->effect_step) {
    case 1:
        if (game->display.state == 0 && board->panels[0].state == 2) {
            game->effect_step = 2;
            game->turn++;
        }
        break;
    case 2:
        done = 1;
        break;
    }
    return done;
}

s32 cardgame_sync_slot_values(CardgameGame *game, CardgameBoard *board) {
    s32 done = 0;
    s32 changed = 0;
    s32 side;
    s32 i;
    CardgameBoardCard *card;

    switch (game->effect_step) {
    case 1:
    default:
        for (i = 0; i < 12; i++) {
            game->marked[i] = 0;
        }
        game->effect_step = 2;
        game->unk_438 = 0;
        break;
    case 2:
        done = 1;
        for (side = 0; side < 2; side++) {
            card = &board->cards[6];
            if (side == 0) {
                card = &board->cards[0];
            }
            for (i = 0; i < game->slots[side].count; i++) {
                if (card[i].attack != game->slots[side].slots[i].attack) {
                    if (game->slots[side].slots[i].attack < card[i].attack) {
                        card[i].attack--;
                    } else {
                        card[i].attack++;
                    }
                    done = 0;
                    changed = 1;
                }
                if (card[i].hp != game->slots[side].slots[i].hp) {
                    if (game->slots[side].slots[i].hp < card[i].hp) {
                        card[i].hp--;
                    } else {
                        card[i].hp++;
                    }
                    done = 0;
                    if (game->slots[side].slots[i].hp == 0) {
                        game->marked[i + side * 6] = 1;
                        game->unk_438 = 1;
                    }
                    changed = 1;
                }
            }
        }
        if (done && game->unk_438 != 0) {
            done = 2;
        }
        if (changed) {
            sound_module.play(0x800452C6);
        }
        break;
    }
    return done;
}

/* .data (address order) */

s16 cardgame_window_pos[2][3][4][2] = {
    { { { 130, 212 }, { 130, 103 }, { 130, 191 }, { 253, 191 } }, { { 130, 29 }, { 130, 97 }, { 130, 8 },
        { 253, 8 } }, { { 130, 165 }, { 134, 49 }, { 130, 144 }, { 253, 144 } } },
    { { { 130, 224 }, { 130, 115 }, { 130, 203 }, { 253, 203 } }, { { 130, 17 }, { 130, 85 }, { 130, -4 },
        { 253, -4 } }, { { 130, 165 }, { 134, 49 }, { 130, 144 }, { 253, 144 } } },
};

s32 cardgame_row_first_card[4] = { 0, 6, 12, 0 };
CardgameIntroStep cardgame_intro_steps[4] = { { 0, 0 }, { 4, 1 }, { 8, 2 }, { -1, 3 } };

u8 cardgame_banners[8][2] = {
    { 1, 2 }, { 3, 4 }, { 5, 6 }, { 7, 8 }, { 9, 0xA }, { 0xB, 0xC }, { 0x1F, 0x3E }, { 0xB, 0xC },
};

CardgamePos cardgame_choose_first_pos[2] = { { 0x7400, 0x6100 }, { 0xA400, 0x6100 } };
s32 cardgame_view_board_next[4] = { 5, 6, 7, 0 };
s16 cardgame_kind_masks[6] = { 4, 8, 16, 32, 64, 128 };
CardgamePos cardgame_discard_pos[2] = { { 0x00011400, 0x6F00 }, { 0x00011400, 0x5600 } };
CardgamePos cardgame_turn_card_pos[3] = { { 0x5100, 0x6100 }, { 0x8300, 0x6100 }, { 0xB500, 0x6100 } };
CardgameVec2 cardgame_popup_pos[2][2] = { { { 130, 191 }, { 130, 29 } }, { { 130, 203 }, { 130, 17 } } };
