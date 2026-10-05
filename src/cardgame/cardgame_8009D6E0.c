#include "common.h"
#include "object.h"
#include "gfx.h"
#include "cdload.h"
#include "sound.h"
#include "gamestate.h"
#include "pad.h"
#include "records.h"
#include "cardgame.h"

/* The game flow: the main object (cardgame_game_create creates it, cardgame_game_update updates it) and
 * the turn phases. */

/* Colour (r, g, b) and blending of the fades cardgame_game_start_fade starts (CardgameGame.fade - 1). */
typedef struct CardgameFadeColor {
    u8 r, g, b;
    u8 abr;
} CardgameFadeColor;

extern CardgameFadeColor cardgame_fade_colors[];
extern u8 cardgame_phase_next[][4];
extern s16 cardgame_bonus_cards[5];
extern s16 cardgame_prize_items[];

/* (side, which) for cardgame_game_show_cards, per phase. */
extern s16 cardgame_view_sources[][2];
/* A copy of CardgameBoard.dialog's first 0x18 bytes (the yes/no dialog), kept while the in-game menu runs. */
typedef struct CardgameDialogSave {
    /* 0x00 */ s32 words[6];
} CardgameDialogSave; /* size 0x18 */
CardgameDialogSave cardgame_saved_dialog; /* .bss */

/* A screen position in pixels. */
typedef struct CardgameScreenPos {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
} CardgameScreenPos;
/* The three deck windows of the deck choice (cardgame_game_choose_deck); the cursor icon goes to the chosen one. */
extern CardgameScreenPos cardgame_deck_choice_pos[3];
/* The deck used when the chosen saved deck is empty (card IDs). */
extern s16 cardgame_default_deck[40];
/* Next state of cardgame_game_run_turns, per step and side (CardgameGame.side). */
extern u8 cardgame_turn_next_state[4][2];
/* Where the played card is shown, per screen mode (main_screen_pos) and side. */
extern s16 cardgame_played_card_pos[2][2][2];

/* A card in an opponent's deck (CardgameOpponent.cards). */
typedef struct CardgameOpponentCard {
    /* 0x0 */ s16 card;  /* bits 0-11: card number + 1; bit 15: flag (CardgameCardInfo.unk_01) */
    /* 0x2 */ u8 stage;  /* the CPU can draw it from this stage of the match on (CardgamePair.stage); 7: last */
    /* 0x3 */ u8 kind;   /* its play class for the CPU (CardgameCardInfo.kind) */
} CardgameOpponentCard;

/* An opponent's record in file 0x7A4 (0xD0 bytes each, indexed by CardgameGame.opponent - 1). */
typedef struct CardgameOpponent {
    /* 0x00 */ CardgameOpponentCard cards[40];
    /* 0xA0 */ s16 counter_ids[20]; /* card IDs + 1 the CPU counters, zero-terminated (CardgameGame.cpu_counter_ids) */
    /* 0xC8 */ u8 level;  /* shown after "LV" in the match's opening dialog (message 0x3E) */
    /* 0xC9 */ u8 unk_C9[3];
    /* 0xCC */ s32 prize;     /* index in cardgame_prize_items */
} CardgameOpponent; /* size 0xD0 */

s32 cardgame_get_card_data(s32 card, u32 field, s32 i); /* defined returning u8; callers use the full word */

void cardgame_game_reset_cpu_deck(CardgameGame *game, CardgameGameData *data);
void cardgame_game_update_display(CardgameGame *game, CardgameGameData *data);
void cardgame_sort_cards(CardgameGame *game, s16 *cards, s32 range, s32 flags);
void cardgame_shuffle_deck(CardgameGame *game, s32 start, s32 count);
s32 cardgame_game_run_phase(CardgameGame *game, CardgameGameData *data);
u8 cardgame_game_resolve_effect(CardgameGame *game, CardgameGameData *data);
s32 cardgame_game_run_menu(CardgameGame *game, CardgameGameData *data);
void cardgame_game_update(CardgameGame *game, CardgameGameData *data);
void cardgame_game_apply_bonus(CardgameGame *game, CardgameSlot *slot, s32 side, s32 step);
s32 cardgame_game_choose_deck(CardgameGame *game, CardgameGameData *data);
s32 cardgame_game_run_turns(CardgameGame *game, CardgameGameData *data);
s32 cardgame_game_end_round(CardgameGame *game, CardgameGameData *data);
s32 cardgame_check_condition(CardgameGame *game, CardgameBoard *board, s32 kind);
s32 cardgame_cpu_get_score(CardgameGame *game, s32 side, s32 mask);
s32 cardgame_cpu_choose_card(CardgameGame *game, CardgameBoard *board);

void cardgame_game_load_opponent(CardgameGame *game, CardgameGameData *data) {
    CardgameOpponent *opp;
    s32 i;

    game->opponents = (s32)cdload_module.files.get_file(0x7A4);
    opp = &((CardgameOpponent *)game->opponents)[(u8)game->opponent - 1];
    game->opponent_level = opp->level;
    game->prize = cardgame_prize_items[opp->prize];
    for (i = 0; i < 40; i++) {
        game->cpu_deck_info[i].pos = i;
        game->cpu_deck_info[i].stage = opp->cards[i].stage;
        game->cpu_cards[i].kind = opp->cards[i].kind;
        game->cpu_cards[i].unk_01 = (opp->cards[i].card & 0x8000) != 0;
        switch (game->cpu_cards[i].kind) {
        case 1:
            game->cpu_cards[i].order = i + 400;
            break;
        case 2:
            game->cpu_cards[i].order = i + 300;
            break;
        case 3:
            game->cpu_cards[i].order = i + 500;
            break;
        case 4:
            game->cpu_cards[i].order = i + 200;
            break;
        case 5:
            game->cpu_cards[i].order = i + 600;
            break;
        case 6:
            game->cpu_cards[i].order = i + 100;
            break;
        case 7:
            game->cpu_cards[i].order = i + 700;
            break;
        }
    }
    for (i = 0; i < 27; i++) {
        if (opp->counter_ids[i] == 0) {
            game->cpu_counter_ids[i] = 7;
            game->cpu_counter_ids[i + 1] = 8;
            game->cpu_counter_ids[i + 2] = 0x18;
            game->cpu_counter_ids[i + 3] = 0x1F;
            game->cpu_counter_ids[i + 4] = 0x20;
            game->cpu_counter_ids[i + 5] = 0;
            game->cpu_counter_ids[i + 6] = 6;
            game->cpu_counter_ids[i + 7] = 0xC;
            game->cpu_counter_ids[i + 8] = 0x12;
            game->cpu_counter_ids[i + 9] = 0x1B;
            game->cpu_counter_ids[i + 10] = 0x1E;
            game->cpu_counter_ids[i + 11] = 0x21;
            game->cpu_counter_ids[i + 12] = 0x25;
            game->cpu_counter_ids[i + 13] = 0x26;
            game->cpu_counter_ids[i + 14] = 0xFF;
            break;
        }
        game->cpu_counter_ids[i] = opp->counter_ids[i] - 1;
    }
    for (i = 0; i < 40; i++) {
        game->opp_deck[i] = (opp->cards[i].card & 0xFFF) - 1;
    }
}

void cardgame_game_sort_cpu_hand(CardgameGame *game) {
    s32 i;
    s32 j;
    s32 swap;
    s32 ka;
    s32 kb;
    s32 ca;
    s32 cb;
    s32 t;

    for (i = 0; i < game->players[1].hand_count - 1; i++) {
        swap = 0;
        for (j = i + 1; j < game->players[1].hand_count; j++, swap = 0) {
            ka = game->cpu_cards[game->players[1].hand[i] - 40].kind;
            kb = game->cpu_cards[game->players[1].hand[j] - 40].kind;
            ca = game->cpu_cards[game->players[1].hand[i] - 40].order;
            cb = game->cpu_cards[game->players[1].hand[j] - 40].order;
            if (kb < ka || (ka == kb && cb < ca)) {
                swap = 1;
            }
            if (swap) {
                t = game->players[1].hand[i];
                game->players[1].hand[i] = game->players[1].hand[j];
                game->players[1].hand[j] = t;
            }
        }
    }
}

void cardgame_game_set_cpu_deck_limits(CardgameGame *game, CardgameGameData *data) {
    s32 i;

    for (i = game->players[1].deck_pos; i < 40; i++) {
        if (game->cpu_deck_info[i].stage > game->round * 2 + 2) {
            break;
        }
    }
    game->cpu_deck_end = i;
    for (i = 40; i > 0; i--) {
        if (game->cpu_deck_info[i - 1].stage != 7) {
            break;
        }
    }
    game->cpu_deck_last = i;
}

void cardgame_game_reset_cpu_deck(CardgameGame *game, CardgameGameData *data) {
    CardgamePlayer *player = &game->players[1];
    s32 count = player->deck_pos;
    s32 i;
    s32 j;
    s16 card;

    while (player->hand_count > 0) {
        player->deck[--player->deck_pos] = player->hand[--player->hand_count];
        player->deck_count++;
    }
    if (count != player->deck_pos) {
        for (i = player->deck_pos; i < count; i++) {
            game->cpu_deck_info[i].stage = game->round * 2 + 1;
        }
    }
    for (i = 0; i < 40; i++) {
        j = player->deck_pos;
        if (game->cpu_deck_info[j].stage > game->round * 2 + 2) {
            break;
        }
        card = player->deck[j];
        for (; j < game->cpu_deck_last - 1; j++) {
            player->deck[j] = player->deck[j + 1];
            game->cpu_deck_info[j].stage = game->cpu_deck_info[j + 1].stage;
        }
        player->deck[game->cpu_deck_last - 1] = card;
        game->cpu_deck_info[game->cpu_deck_last - 1].stage = 7;
    }
    for (i = 39; i >= 0; i--) {
        game->cpu_deck_info[i].pos = i;
    }
}

void cardgame_game_show_cards(CardgameGame *game, CardgameGameData *data, s32 side, s32 which) {
    CardgameBoard *board = data->board;
    CardgameDisplay *g = &game->display;
    s32 i;

    switch (which) {
    case 0:
        g->count = game->players[side].hand_count;
        break;
    case 1:
        g->count = game->players[side].discard_count;
        break;
    case 2:
        g->count = game->players[side].deck_count;
        break;
    }
    g->time = 0;
    g->shown = 0;
    g->step_time = 0;
    g->duration = g->count * 4 + 10;
    if (g->count != 0) {
        for (i = 0; i < 40; i++) {
            if (i < g->count) {
                board->place_card(board, i, board->get_card_x(g->count, i) + 0x1800, 0x6100);
                switch (which) {
                case 0:
                    board->set_card(board, i, game->players[side].hand[i]);
                    break;
                case 1:
                    board->set_card(board, i, game->players[side].discard[i]);
                    break;
                case 2:
                    board->set_card(board, i, game->players[side].deck[game->players[side].deck_pos + i]);
                    break;
                }
                if (g->face_down != 0) {
                    board->cards[i].style = 2;
                }
                board->cards[i].scale_x = 0;
                data->board->cards[i].dimmed = game->display.dimmed[i];
            } else {
                board->remove_card(board, i);
            }
        }
        g->face_down = 0;
    } else {
        g->face_down = 0;
        board->place_card(board, 0, board->get_card_x(g->count, 0) + 0x1800, 0x6100);
        board->cards[0].style = 3;
        board->cards[0].scale_x = 0;
        for (i = 1; i < 40; i++) {
            board->remove_card(board, i);
        }
    }
}

s32 cardgame_game_close_panels(CardgameGame *game, CardgameGameData *data) {
    s32 done = 0;

    if (game->display.substep == 0) {
        game->display.request = 2;
        data->board->close_panels(data->board);
        game->display.substep++;
    }
    if (data->board->panels[0].state == 0 && game->display.state == 0) {
        done = 1;
    }
    return done;
}

void cardgame_game_count_cards(CardgameGame *game, CardgameGameData *data, s32 side, s32 which) {
    CardgameDisplay *x = &game->display;

    switch (which) {
    case 0:
        x->count = game->players[side].hand_count;
        break;
    case 1:
        x->count = game->players[side].discard_count;
        break;
    case 2:
        x->count = game->players[side].deck_count;
        break;
    }
    x->time = 0;
    x->shown = 0;
    x->step_time = 0;
    x->duration = x->count * 4 + 10;
}

void cardgame_game_show_board(CardgameGame *game, CardgameGameData *data) {
    CardgameBoard *board = data->board;
    CardgameSlot *slot;
    s32 base;
    CardgameBoardCard *row;
    s32 card;
    s32 i;
    s32 j;

    for (i = 0; i < 6; i++) {
        if (i >= game->slots[0].count) {
            break;
        }
        board->place_card(board, i, cardgame_slot_origins[main_screen_pos][0].x + i * 0x2900, cardgame_slot_origins[main_screen_pos][0].y);
        board->set_card(board, i, game->slots[0].slots[i].card);
        board->cards[i].attack = game->slots[0].slots[i].attack;
        board->cards[i].hp = game->slots[0].slots[i].hp;
        board->cards[i].scale_x = 0;
    }
    for (i = 0; i < 6; i++) {
        if (i >= game->slots[1].count) {
            break;
        }
        board->place_card(board, i + 6, cardgame_slot_origins[main_screen_pos][1].x + i * 0x2900, cardgame_slot_origins[main_screen_pos][1].y);
        board->set_card(board, i + 6, game->slots[1].slots[i].card);
        board->cards[i + 6].attack = game->slots[1].slots[i].attack;
        board->cards[i + 6].hp = game->slots[1].slots[i].hp;
        board->cards[i + 6].scale_x = 0;
        if (game->display.face_down != 0) {
            board->cards[i + 6].style = 2;
        }
    }
    game->display.face_down = 0;
    for (i = 0; i < game->turn; i++) {
        board->place_card(board, i + 12, i * 0x3200 + 0x5100, 0x6100);
        board->set_card(board, i + 12, game->turns[i].card);
        board->cards[i + 12].scale_x = 0;
        board->cards[i + 12].turn_badge = i + 1;
        board->cards[i + 12].turn_marks[0] = 0;
        board->cards[i + 12].turn_marks[1] = 0;
        board->cards[i + 12].turn_marks[2] = 0;
        board->cards[i + 12].zooming = 0;
        if (game->turns[i].mark != 0) {
            board->cards[i + 12].turn_marks[game->turns[i].mark] = 1;
        }
        switch (game->turns[i].target_kind) {
        case 0:
            for (j = 0; j < 12; j++) {
                if (j < 6) {
                    if (j >= game->slots[0].count) {
                        continue;
                    }
                    slot = &game->slots[0].slots[j];
                    if (slot->id != game->turns[i].target) {
                        continue;
                    }
                } else {
                    if (j - 6 >= game->slots[1].count) {
                        continue;
                    }
                    slot = &game->slots[1].slots[j - 6];
                    if (slot->id != game->turns[i].target) {
                        continue;
                    }
                }
                board->cards[j].turn_marks[i] = 1;
            }
            break;
        case 1:
            base = game->turns[i].side != 0 ? 6 : 0;
            row = &board->cards[base];
            for (j = 0; j < 6; j++) {
                row[j].turn_marks[i] = 1;
            }
            break;
        case 2:
            base = game->turns[i].side == 0 ? 6 : 0;
            row = &board->cards[base];
            for (j = 0; j < 6; j++) {
                row[j].turn_marks[i] = 1;
            }
            break;
        case 3:
            for (j = 0; j < 12; j++) {
                board->cards[j].turn_marks[i] = 1;
            }
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            for (j = 0; j < 12; j++) {
                card = j < 6 ? game->slots[0].slots[j].card : game->slots[1].slots[j - 6].card;
                switch (game->turns[i].target_kind) {
                case 4:
                    if (board->get_card_kind(board, card) != 1) {
                        board->cards[j].turn_marks[i] = 1;
                    }
                    break;
                case 5:
                    if (board->get_card_kind(board, card) != 2) {
                        board->cards[j].turn_marks[i] = 1;
                    }
                    break;
                case 6:
                    if (board->get_card_kind(board, card) == 3) {
                        board->cards[j].turn_marks[i] = 1;
                    }
                    break;
                case 7:
                    if (board->get_card_kind(board, card) != 4) {
                        board->cards[j].turn_marks[i] = 1;
                    }
                    break;
                case 8:
                    if (board->get_card_kind(board, card) == 6) {
                        board->cards[j].turn_marks[i] = 1;
                    }
                    break;
                }
            }
            break;
        case 9:
            break;
        }
    }
}

void cardgame_game_start_fade(CardgameGame *game, CardgameGameData *data) {
    s32 i;
    s32 ticks;

    if (game->fade != 0) {
        i = game->fade - 1;
        ticks = 10;
        data->fade = cardgame_fade_create(cardgame_fade_colors[i].abr);
        data->fade->set_color(data->fade, cardgame_fade_colors[i].r, cardgame_fade_colors[i].g, cardgame_fade_colors[i].b);
        if (cardgame_fade_colors[i].abr == 2) {
            ticks = 15;
        }
        data->fade->start(data->fade, 0, 0, 0, ticks, 1);
        game->fade = 0;
    }
}

void cardgame_game_update_dimmed(CardgameGame *game) {
    CardgameDisplay *x = &game->display;
    s32 i;
    u8 on;

    if (x->set_dimmed != 0) {
        on = x->set_dimmed == 2;
        for (i = 0; i < 15; i++) {
            game->display.dimmed[i] = on;
        }
        x->set_dimmed = 0;
    }
}

void cardgame_game_update_display(CardgameGame *game, CardgameGameData *data) {
    CardgameDisplay *g = &game->display;
    CardgameBoard *board = data->board;
    s32 n = 0;
    s32 i;
    s32 max;

    if (g->request != 0) {
        switch (g->request) {
        case 1:
            cardgame_game_show_board(game, data);
            g->time = 3;
            g->shown = 0;
            g->step_time = 4;
            g->duration = (game->slots[0].count > game->slots[1].count ? game->slots[0].count : game->slots[1].count) * 8;
            for (i = 0; i < 40; i++) {
                data->board->cards[i].dimmed = game->display.dimmed[i];
            }
            break;
        case 3:
            g->time = 3;
            g->shown = 0;
            g->step_time = 4;
            g->duration = game->turn * 10;
            break;
        case 2:
            g->time = 3;
            g->shown = 0;
            g->step_time = 4;
            g->duration = (game->slots[0].count > game->slots[1].count ? game->slots[0].count : game->slots[1].count) * 5;
            break;
        case 4:
            g->time = 3;
            g->shown = 0;
            g->step_time = 4;
            g->duration = game->turn * 5 + 22;
            break;
        case 5:
        case 17:
            n++;
        case 7:
            n++;
        case 9:
            n++;
        case 11:
        case 18:
            n++;
        case 13:
            n++;
        case 15:
            n++;
            cardgame_game_show_cards(game, data, cardgame_view_sources[n][0], cardgame_view_sources[n][1]);
            break;
        case 6:
            cardgame_game_count_cards(game, data, 0, 0);
            break;
        case 10:
            cardgame_game_count_cards(game, data, 0, 1);
            break;
        case 8:
            cardgame_game_count_cards(game, data, 0, 2);
            break;
        case 16:
            cardgame_game_count_cards(game, data, 1, 1);
            break;
        case 14:
            cardgame_game_count_cards(game, data, 1, 2);
            break;
        case 12:
            cardgame_game_count_cards(game, data, 1, 0);
            break;
        }
        g->reopen = g->request + 1;
        g->state = g->request;
        g->request = 0;
    }
    switch (g->state) {
    case 1:
        if (g->step_time >= 4) {
            if (g->shown < game->slots[0].count) {
                data->board->zoom_card(data->board, g->shown, 10, 0x1000, 0x1000);
            }
            if (g->shown < game->slots[1].count) {
                data->board->zoom_card(data->board, g->shown + 6, 10, 0x1000, 0x1000);
            }
            g->shown++;
            g->step_time -= 4;
        }
        g->time += gfx_module.funcs.get_frame_ticks();
        g->step_time += gfx_module.funcs.get_frame_ticks();
        if (g->duration < g->time) {
            g->request = 3;
        }
        break;
    case 3:
        if (g->step_time >= 4) {
            if (g->shown < game->turn) {
                data->board->zoom_card(data->board, g->shown + 12, 10, 0x1000, 0x1000);
            }
            if (g->shown - 1 < game->turn && g->shown - 1 >= 0) {
                data->board->open_turn_mark(data->board, g->shown - 1, game->turns[g->shown - 1].side);
            }
            g->shown++;
            g->step_time -= 4;
        }
        g->time += gfx_module.funcs.get_frame_ticks();
        g->step_time += gfx_module.funcs.get_frame_ticks();
        if (g->duration < g->time) {
            g->state = 0;
        }
        break;
    case 2:
        if (g->step_time >= 4) {
            if (g->shown < game->slots[0].count) {
                data->board->zoom_card(data->board, g->shown, 5, 0, 0x1000);
            }
            if (g->shown < game->slots[1].count) {
                data->board->zoom_card(data->board, g->shown + 6, 5, 0, 0x1000);
            }
            g->shown++;
            g->step_time -= 4;
        }
        g->time += gfx_module.funcs.get_frame_ticks();
        g->step_time += gfx_module.funcs.get_frame_ticks();
        if (g->duration < g->time) {
            g->request = 4;
        }
        break;
    case 4:
        if (g->step_time >= 4) {
            if (g->shown < game->turn) {
                data->board->zoom_card(data->board, g->shown + 12, 5, 0, 0x1000);
                data->board->close_turn_mark(data->board, g->shown);
            }
            g->shown++;
            g->step_time -= 4;
        }
        g->time += gfx_module.funcs.get_frame_ticks();
        g->step_time += gfx_module.funcs.get_frame_ticks();
        if (g->duration < g->time) {
            g->state = 0;
        }
        break;
    case 5:
    case 7:
    case 9:
    case 11:
    case 13:
    case 15:
        max = g->count;
        if (max == 0) {
            max = 1;
        }
        if (g->shown < max && g->step_time >= 4) {
            board->zoom_card(board, g->shown, 10, 0x1000, 0x1000);
            g->shown++;
            g->step_time -= 4;
        }
        if (g->time > g->duration) {
            g->state = 0;
        }
        g->time += gfx_module.funcs.get_frame_ticks();
        g->step_time += gfx_module.funcs.get_frame_ticks();
        break;
    case 6:
    case 8:
    case 10:
    case 12:
    case 14:
    case 16:
        max = g->count;
        if (max == 0) {
            max = 1;
        }
        if (g->shown < max && g->step_time >= 4) {
            board->zoom_card(board, g->shown, 5, 0, 0x1000);
            g->shown++;
            g->step_time -= 4;
        }
        if (g->time > g->duration) {
            g->state = 0;
        }
        g->time += gfx_module.funcs.get_frame_ticks();
        g->step_time += gfx_module.funcs.get_frame_ticks();
        break;
    case 17:
        g->state = 0;
        g->reopen = 6;
        break;
    case 18:
        g->state = 0;
        g->reopen = 12;
        break;
    }
}

void cardgame_game_update_panels(CardgameGame *game, CardgameGameData *data) {
    s32 side;
    s32 i;

    for (side = 0; side < 2; side++) {
        for (i = 0; i < 5; i++) {
            data->board->set_panel_value(data->board, side, i, game->players[side].points[i]);
        }
        data->board->set_panel_value(data->board, side, 5, game->players[side].deck_count);
        data->board->set_panel_value(data->board, side, 6, game->players[side].hand_count);
        data->board->set_panel_value(data->board, side, 7, game->players[side].discard_count);
        data->board->set_panel_value(data->board, side, 8, game->players[side].attack);
        data->board->set_panel_value(data->board, side, 9, game->players[side].hp);
    }
}

/* Sorts cards[start..end) (range = start | end << 16) by card ID; flags & 1 also swaps the matching cpu_deck_info entries,
 * flags & 2 the selectable flags (indexed from start). */
void cardgame_sort_cards(CardgameGame *game, s16 *cards, s32 range, s32 flags) {
    s32 end = range >> 16;
    s32 start = range & 0xFFFF;
    /* FAKE: every access goes through a local copy of cards. Its own pseudo (copied from a1 at entry) changes the
     * global-alloc order so start gets s0 and the hoisted flags & 1 a1, as in the original; with cards itself, start
     * and the flag swap (95.8%). Found from the permuter's `cards++; cards--;` (wip-13/final-rest searches: types,
     * orders, a sort_pairs local, nested swaps, LOOP_BLOCK: no natural form). */
    s16 *list = cards;
    s32 i;
    s32 j;
    s16 t;
    CardgamePair pair;
    u8 f;

    for (i = start; i < end - 1; i++) {
        for (j = i + 1; j < end; j++) {
            if (game->card_ids[list[i]] > game->card_ids[list[j]]) {
                t = list[i];
                list[i] = list[j];
                list[j] = t;
                if (flags & 1) {
                    pair = game->cpu_deck_info[i];
                    game->cpu_deck_info[i] = game->cpu_deck_info[j];
                    game->cpu_deck_info[j] = pair;
                }
                if (flags & 2) {
                    f = game->selectable[i - start];
                    game->selectable[i - start] = game->selectable[j - start];
                    game->selectable[j - start] = f;
                }
            }
        }
    }
}

void cardgame_shuffle_deck(CardgameGame *game, s32 start, s32 count) {
    CardgamePlayer *player = game->players;
    s32 i;
    s32 a;
    s32 b;
    s32 t;

    if (count >= 2) {
        for (i = 0; i < 40; i++) {
            a = pad_random.next() % count + start;
            b = pad_random.next() % count + start;
            t = player->deck[a];
            player->deck[a] = player->deck[b];
            player->deck[b] = t;
        }
    }
}

/* The deck choice before a match (state in CardgameGame.step): opens the three deck windows and the cursor, the
 * player picks one (CardgameGame.deck_sel) and confirms; then both decks are built. Returns 1 when done. */
s32 cardgame_game_choose_deck(CardgameGame *game, CardgameGameData *data) {
    s32 done = 0;
    s32 i;
    CardgamePlayer *player;

    switch (game->step) {
    case 0:
    default:
        if (data->loader == NULL) {
            game->step++;
        } else if (data->loader->base.state == OBJECT_STATE_RUN && data->loader->ready != 0) {
            game->step++;
        }
        break;
    case 1:
        data->board->open_popup(data->board, 5, 5, 0x41, 0, 0x26);
        sound_module.play(0x40019);
        data->deck_choice.windows[0] = cardgame_deck_window_create(0, cardgame_deck_choice_pos[0].x, cardgame_deck_choice_pos[0].y);
        game->timer = 0;
        game->step++;
        break;
    case 2:
        game->timer += gfx_module.funcs.get_frame_ticks();
        if (game->timer >= 11) {
            game->timer = 0;
            game->step++;
        }
        break;
    case 3:
        sound_module.play(0x40019);
        data->deck_choice.windows[1] = cardgame_deck_window_create(1, cardgame_deck_choice_pos[1].x, cardgame_deck_choice_pos[1].y);
        game->timer = 0;
        game->step++;
        break;
    case 4:
        game->timer += gfx_module.funcs.get_frame_ticks();
        if (game->timer >= 11) {
            game->timer = 0;
            game->step++;
        }
        break;
    case 5:
        sound_module.play(0x40019);
        data->deck_choice.windows[2] = cardgame_deck_window_create(2, cardgame_deck_choice_pos[2].x, cardgame_deck_choice_pos[2].y);
        game->timer = 0;
        game->step++;
        break;
    case 6:
        game->timer += gfx_module.funcs.get_frame_ticks();
        if (game->timer >= 11) {
            game->timer = 0;
            game->step++;
        }
        break;
    case 7:
        data->deck_choice.icon = cardgame_icon_create(cardgame_deck_choice_pos[0].x, cardgame_deck_choice_pos[0].y);
        game->timer = 0;
        game->step++;
        break;
    case 8:
        game->timer += gfx_module.funcs.get_frame_ticks();
        if (game->timer >= 11) {
            game->timer = 0;
            game->deck_sel = 0;
            game->step++;
        }
        break;
    case 9:
        if ((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 4))) |
            (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 4)))) {
            if (game->deck_sel > 0) {
                game->deck_sel--;
                sound_module.play(0x4001B);
            }
        } else if ((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 6))) |
                   (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 6)))) {
            if (game->deck_sel < 2) {
                game->deck_sel++;
                sound_module.play(0x4001B);
            }
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            game->timer = 0;
            game->step++;
            data->deck_choice.windows[game->deck_sel]->play(data->deck_choice.windows[game->deck_sel]);
            data->deck_choice.icon->play(data->deck_choice.icon);
        }
        data->deck_choice.icon->set_pos(data->deck_choice.icon, cardgame_deck_choice_pos[game->deck_sel].x, cardgame_deck_choice_pos[game->deck_sel].y);
        break;
    case 10:
        game->timer += gfx_module.funcs.get_frame_ticks();
        if (game->timer >= 15) {
            game->timer = 0;
            game->step++;
        }
        break;
    case 11:
        data->deck_choice.icon->close_icon(data->deck_choice.icon);
        game->timer = 0;
        game->step++;
        break;
    case 13:
        sound_module.play(0x4001A);
        data->deck_choice.windows[0]->close_window(data->deck_choice.windows[0]);
        game->timer = 0;
        game->step++;
        break;
    case 14:
        game->timer += gfx_module.funcs.get_frame_ticks();
        if (game->timer >= 4) {
            game->timer = 0;
            game->step++;
        }
        break;
    case 15:
        sound_module.play(0x4001A);
        data->deck_choice.windows[1]->close_window(data->deck_choice.windows[1]);
        game->timer = 0;
        game->step++;
        break;
    case 16:
        game->timer += gfx_module.funcs.get_frame_ticks();
        if (game->timer >= 4) {
            game->timer = 0;
            game->step++;
        }
        break;
    case 17:
        sound_module.play(0x4001A);
        data->deck_choice.windows[2]->close_window(data->deck_choice.windows[2]);
        game->timer = 0;
        game->step++;
        break;
    case 18:
        game->timer += gfx_module.funcs.get_frame_ticks();
        if (game->timer >= 4) {
            game->timer = 0;
            game->step++;
        }
        break;
    case 19:
        data->board->close_popup(data->board, 5);
        game->timer = 0;
        game->step++;
        break;
    case 12:
    case 20:
        game->timer += gfx_module.funcs.get_frame_ticks();
        if (game->timer >= 6) {
            game->timer = 0;
            game->step++;
        }
        break;
    case 21:
        if ((s16)gamestate_data.decks[game->deck_sel].cards[0] != 0) {
            for (i = 0; i < 40; i++) {
                game->deck[i] = gamestate_data.decks[game->deck_sel].cards[i] - 1;
            }
        } else {
            for (i = 0; i < 40; i++) {
                game->deck[i] = cardgame_default_deck[i];
            }
        }
        game->card_count =
            data->board->load_cards(game->card_ids, game->deck, game->opp_deck);
        for (i = 0; i < 40; i++) {
            game->players[0].deck[i] = i;
            game->players[1].deck[i] = i + 40;
        }
        game->round = 0;
        cardgame_game_set_cpu_deck_limits(game, data);
        game->players[1].deck_count = 40;
        game->players[0].deck_count = 40;
        game->players[1].deck_pos = 0;
        game->players[0].deck_pos = 0;
        cardgame_game_update_panels(game, data);
        player = &game->players[0];
        game->shuffle_deck(game, player->deck_pos, player->deck_count);
        done = 1;
        break;
    }
    return done;
}

s32 cardgame_game_choose_first(CardgameGame *game, CardgameGameData *data) {
    s32 done = 0;

    if (game->step == 0) {
        game->new_effect = 0xA7;
        game->effect_state = 1;
        game->step = 1;
    } else {
        if (game->choice == 0) {
            game->first_side = 0;
        } else {
            game->first_side = 1;
        }
        done = 1;
    }
    return done;
}

s32 cardgame_game_deal(CardgameGame *game, CardgameGameData *data) {
    s32 result = 0;

    if (game->step == 0) {
        game->new_effect = 0x14;
        game->effect_state = 1;
        game->step++;
    } else {
        switch (game->choice) {
        case 0:
            result = 1;
            break;
        case 1:
            result = 2;
            break;
        case 2:
            result = 3;
            break;
        }
    }
    return result;
}

s32 cardgame_game_play_card(CardgameGame *game, CardgameGameData *data) {
    s32 i;
    s32 side;
    s32 id;
    s32 j;
    s16 card;

    for (i = 0; i < 10; i++) {
        if (game->marked[i] != 0) {
            break;
        }
    }
    side = game->side;
    id = game->card_ids[game->players[side].hand[i]];
    game->turns[game->turn].side = side;
    card = game->turns[game->turn].card = game->players[side].hand[i];
    game->turns[game->turn].target_kind = cardgame_get_card_data(game->card_ids[card], 3, 0);
    for (j = i; j < game->players[side].hand_count - 1; j++) {
        game->players[side].hand[j] = game->players[side].hand[j + 1];
    }
    game->players[side].hand_count--;
    return cardgame_get_card_data(id, 0, 0);
}

void cardgame_game_mark_targets(CardgameGame *game, CardgameBoard *board) {
    CardgameSlot *slot;
    s32 i;

    for (i = 0; i < 12; i++) {
        game->marked[i] = 0;
        switch (game->turns[game->turn].target_kind) {
        case 0:
            if (i < 6) {
                if (i >= game->slots[0].count) {
                    break;
                }
                slot = &game->slots[0].slots[i];
            } else {
                if (i - 6 >= game->slots[1].count) {
                    break;
                }
                slot = &game->slots[1].slots[i - 6];
            }
            if (slot->id == game->turns[game->turn].target) {
                game->marked[i] = 1;
            }
            break;
        case 2:
            if (i < 6 && i < game->slots[0].count) {
                game->marked[i] = 1;
            }
            break;
        case 1:
            if (i >= 6 && i - 6 < game->slots[1].count) {
                game->marked[i] = 1;
            }
            break;
        case 4:
            if (board->get_card_kind(board, i < 6 ? game->slots[0].slots[i].card : game->slots[1].slots[i - 6].card) != 1) {
                game->marked[i] = 1;
            }
            break;
        case 5:
            if (board->get_card_kind(board, i < 6 ? game->slots[0].slots[i].card : game->slots[1].slots[i - 6].card) != 2) {
                game->marked[i] = 1;
            }
            break;
        case 6:
            if (board->get_card_kind(board, i < 6 ? game->slots[0].slots[i].card : game->slots[1].slots[i - 6].card) == 3) {
                game->marked[i] = 1;
            }
            break;
        case 7:
            if (board->get_card_kind(board, i < 6 ? game->slots[0].slots[i].card : game->slots[1].slots[i - 6].card) != 4) {
                game->marked[i] = 1;
            }
            break;
        case 8:
            if (board->get_card_kind(board, i < 6 ? game->slots[0].slots[i].card : game->slots[1].slots[i - 6].card) == 6) {
                game->marked[i] = 1;
            }
            break;
        case 3:
            game->marked[i] = 1;
            break;
        case 9:
            break;
        }
    }
}

/* The turn loop of a match (state in CardgameGame.turn_state; the player is state 2, the CPU state 3). Returns 1 when
 * the match is over. */
s32 cardgame_game_run_turns(CardgameGame *game, CardgameGameData *data) {
    s32 done = 0;
    s32 i;
    s32 j;
    s32 side;

    switch (game->turn_state) {
    case 0:
    default:
        game->turn_state = 16;
        game->turn = 0;
        game->passes = 0;
        game->turns[0].mark = 0;
        game->turns[1].mark = 0;
        game->turns[2].mark = 0;
        game->display.set_dimmed = 1;
        game->display.request = 1;
        game->round_first = game->side = game->first_side;
        data->board->open_panels(data->board);
        break;
    case 16:
        if (data->board->panels[0].state == 2 && game->display.state == 0) {
            game->turn_state = 17;
            data->board->open_popup(data->board, 5, 5, game->phase == 5 ? 0x3C : 0x3D, 0, 0x6E);
        }
        break;
    case 17:
        if (data->board->popups[5].state == 2 && (PAD_PRESSED(13) || PAD_PRESSED(14))) {
            game->turn_state = 18;
            data->board->close_popup(data->board, 5);
        }
        break;
    case 18:
        if (data->board->popups[5].state == 0) {
            game->turn_state = 1;
        }
        break;
    case 1:
        switch (game->turn) {
        case 0:
        case 2:
            if (game->round_first != 0) {
                game->turn_state = 3;
            } else {
                game->turn_state = 2;
            }
            break;
        case 1:
            if (game->round_first != 0) {
                game->turn_state = 2;
            } else {
                game->turn_state = 3;
            }
            break;
        default:
            game->turn_state = 12;
            break;
        }
        if (game->turn != 0) {
            game->passes = 0;
        }
        game->answer = -1;
        data->board->set_panel_value(data->board, 0, 6, game->players[0].hand_count);
        data->board->set_panel_value(data->board, 1, 6, game->players[1].hand_count);
        break;
    case 3:
        cardgame_game_sort_cpu_hand(game);
        if (cardgame_cpu_choose_card(game, data->board) != 0) {
            i = cardgame_game_play_card(game, data);
            if (i == 0x83 || i == 0x84) {
                for (j = 0; j < 15; j++) {
                    game->marked[j] = 0;
                }
                game->marked[game->turn + 11] = 1;
                game->turns[game->turn - 1].mark = 1;
            }
            if (game->passes != 0) {
                sound_module.play(0x40019);
                data->board->close_panel_anim(data->board, 0);
            }
            game->turn_state = 9;
            cardgame_game_mark_targets(game, data->board);
        } else if (game->turn == 0) {
            game->turn_state = 11;
            game->pass_timer = 20;
            game->passes++;
            sound_module.play(0x40019);
            data->board->open_panel_anim(data->board, 1);
        } else {
            game->turn_state = 12;
        }
        break;
    case 2:
        if (game->answer == -1) {
            game->effect_state = 1;
            game->new_effect = 0x98;
            game->sort_cards(game, game->players[0].hand, game->players[0].hand_count << 16, 0);
        } else if (game->answer != 0) {
            game->turn_state = cardgame_turn_next_state[1][game->side];
            game->answer = -1;
            if (game->passes != 0) {
                data->board->panels[1].anim_state = 0;
            }
        } else if (game->turn == 0) {
            game->turn_state = 11;
            game->pass_timer = 20;
            game->passes++;
            sound_module.play(0x40019);
            data->board->open_panel_anim(data->board, 0);
        } else {
            game->turn_state = 12;
        }
        break;
    case 4:
    case 5:
        if (game->answer == -1) {
            game->effect_state = 1;
            game->new_effect = 0x99;
        } else if (game->answer != 0) {
            game->turn_state = cardgame_turn_next_state[2][game->side];
            game->answer = -1;
        } else {
            game->turn_state = cardgame_turn_next_state[0][game->side];
            game->answer = -1;
            if (game->passes != 0) {
                data->board->panels[1].anim_state = 2;
            }
        }
        break;
    case 6:
    case 7:
        if (game->answer == -1) {
            game->new_effect = cardgame_game_play_card(game, data);
            game->effect_state = 1;
        } else if (game->answer != 0) {
            game->turn_state = cardgame_turn_next_state[3][game->side];
        } else {
            side = game->side;
            game->players[side].hand[game->players[side].hand_count] = game->turns[game->turn].card;
            game->players[side].hand_count++;
            game->sort_cards(game, game->players[0].hand, game->players[0].hand_count << 16, 0);
            game->turn_state = cardgame_turn_next_state[1][game->side];
            game->answer = -1;
        }
        break;
    case 9:
        game->new_effect = 0x13;
        game->effect_state = 1;
        game->turn_state = 10;
        break;
    case 8:
        game->new_effect = 0x12;
        game->effect_state = 1;
        game->turn_state = 10;
        break;
    case 10:
        game->turn_state = 1;
        game->side ^= 1;
        game->turn++;
        break;
    case 11:
        if (--game->pass_timer <= 0) {
            game->turn_state = 14;
        }
        break;
    case 12:
        if (game->turn > 0) {
            game->effect_state = 2;
            game->turn_state = 13;
            game->display.resolve_step = 0;
            game->menu.timer = 0;
            game->display.substep = 0;
            game->display.unk_46 = game->turn;
        } else {
            game->turn_state = 14;
        }
        break;
    case 13:
        game->turn_state = 12;
        break;
    case 14:
        game->turn_state = 1;
        game->turn = 0;
        game->turns[0].mark = 0;
        game->turns[1].mark = 0;
        game->turns[2].mark = 0;
        game->side = game->round_first ^= 1;
        game->unk_576++;
        if (game->passes >= 2) {
            game->turn_state = 15;
            game->display.substep = 0;
        }
        break;
    case 15:
        data->board->close_panel_anim(data->board, 0);
        data->board->close_panel_anim(data->board, 1);
        if (cardgame_game_close_panels(game, data) != 0) {
            done = 1;
        }
        break;
    }
    return done;
}

void cardgame_game_start_turns(CardgameGame *game, CardgameGameData *data) {
    game->turn_state = 0;
    cardgame_game_update_panels(game, data);
    game->sort_cards(game, game->players[0].hand, game->players[0].hand_count << 16, 0);
}

void cardgame_game_remove_marked(CardgameGame *game, CardgamePlayer *player) {
    s32 i;
    s32 j;

    for (i = player->hand_count - 1; i >= 0; i--) {
        if (game->marked[i] != 0) {
            for (j = i; j < player->hand_count - 1; j++) {
                player->hand[j] = player->hand[j + 1];
            }
            player->hand_count--;
        }
    }
}

void cardgame_game_set_slot(CardgameGame *game, s32 side, s32 card, s32 i) {
    CardPicture pic;
    CardgameSlots *slots = &game->slots[side];

    card_init(&pic);
    pic.select(game->card_ids[card] + 1);
    slots->slots[i].attack = pic.record[1];
    slots->slots[i].hp = pic.record[2];
    slots->slots[i].attack_bonus = 0;
    slots->slots[i].hp_bonus = 0;
    slots->slots[i].card = card;
    slots->slots[i].side = side;
    slots->slots[i].owner = side;
    slots->slots[i].id = game->next_slot_id++;
}

void cardgame_game_add_slot(CardgameGame *game, s32 side, s32 card) {
    u8 *count = &game->slots[side].count;

    cardgame_game_set_slot(game, side, card, (*count)++);
}

void cardgame_game_place_cards(CardgameGame *game, s32 side) {
    CardgameSlots *slots = &game->slots[side];
    CardgamePlayer *player = &game->players[side];
    s32 i;

    slots->count = 0;
    for (i = 0; i < player->hand_count; i++) {
        if (slots->count < game->selected && game->marked[i] != 0) {
            cardgame_game_add_slot(game, side, player->hand[i]);
        }
    }
    cardgame_game_remove_marked(game, player);
}

void cardgame_game_apply_bonus(CardgameGame *game, CardgameSlot *slot, s32 side, s32 step) {
    s32 values[2];
    CardgamePlayer *player = &game->players[side];
    CardgameSlots *slots = &game->slots[side];
    s32 i;
    s32 base;

    if (game->card_ids[slot->card] == cardgame_bonus_cards[step]) {
        for (i = 0; i < 2; i++) {
            if (i == 0) {
                base = slot->attack_bonus;
            } else {
                base = slot->hp_bonus;
            }
            switch (step) {
            case 0:
                values[i] = slots->count * 20 + base;
                break;
            case 1:
                values[i] = player->hand_count * 10 + 10 + base;
                break;
            case 2:
                values[i] = player->discard_count * 20 + 10 + base;
                break;
            case 3:
                values[i] = (game->slots[0].count + game->slots[1].count) * 10 + base;
                break;
            case 4:
                values[i] = (game->players[0].discard_count + game->players[1].discard_count) * 10 + 10 + base;
                break;
            }
            if (values[i] >= 99) {
                values[i] = 99;
            }
            if (values[i] <= 0) {
                values[i] = 0;
            }
        }
        slot->attack = values[0];
        slot->hp = values[1];
    }
}

s32 cardgame_game_apply_bonuses(CardgameGame *game) {
    s32 result = 0;
    s32 k;
    s32 side;
    s32 i;
    CardgameSlots *slots;

    for (k = 0; k < 5; k++) {
        for (side = 0; side < 2; side++) {
            slots = &game->slots[side];
            for (i = 0; i < slots->count; i++) {
                result = 1;
                cardgame_game_apply_bonus(game, &slots->slots[i], side, k);
            }
        }
    }
    return result;
}

void cardgame_game_sum_slots(CardgameGame *game, s32 side) {
    s32 i;

    game->players[side].attack = 0;
    game->players[side].hp = 0;
    for (i = 0; i < game->slots[side].count; i++) {
        game->players[side].attack += game->slots[side].slots[i].attack;
        game->players[side].hp += game->slots[side].slots[i].hp;
    }
}

s32 cardgame_game_run_placement(CardgameGame *game, CardgameGameData *data) {
    s32 done = 0;

    switch (game->step) {
    case 0:
        game->new_effect = 0x9A;
        game->effect_state = 1;
        game->step = 1;
        break;
    case 1:
        game->place_cards(game, 0);
        game->step = 2;
        data->board->set_panel_value(data->board, 0, 6, game->players[0].hand_count);
        break;
    case 2:
        cardgame_game_sort_cpu_hand(game);
        game->new_effect = 0x9D;
        game->effect_state = 1;
        game->step = 3;
        break;
    case 3:
        game->place_cards(game, 1);
        game->step = 4;
        cardgame_game_apply_bonuses(game);
        cardgame_game_sum_slots(game, 1);
        cardgame_game_sum_slots(game, 0);
        data->board->set_panel_value(data->board, 1, 6, game->players[1].hand_count);
        break;
    case 4:
        game->new_effect = 0x9E;
        game->effect_state = 1;
        game->step = 5;
        break;
    case 5:
        done = 1;
        break;
    }
    return done;
}

s32 cardgame_game_find_combo(CardgameGame *game, s32 side, s32 start) {
    CardgameSortEntry list[6];
    CardgameSortEntry tmp;
    CardPicture pic;
    s32 result = -1;
    s32 value = 0x51;
    s32 count = game->slots[side].count;
    s32 i;
    s32 j;
    s32 k;

    card_init(&pic);
    for (i = 0; i < count; i++) {
        list[i].id = game->card_ids[game->slots[side].slots[i].card];
        list[i].index = i;
    }
    for (i = 0; i < count - 1; i++) {
        for (j = i + 1; j < count; j++) {
            if (list[i].id > list[j].id) {
                tmp = list[i];
                list[i] = list[j];
                list[j] = tmp;
            }
        }
    }
    j = 0;
    for (i = start; i < count - 1; i++) {
        pic.select(list[i].id + 1);
        if (*(s16 *)(pic.record + 10) != 0 && list[i].id == list[i + 1].id) {
            value = *(s16 *)(pic.record + 10);
            j++;
        } else {
            if (j >= 2) {
                break;
            }
            j = 0;
        }
    }
    if (j >= 2) {
        for (k = 0; k < count; k++) {
            game->selectable[k] = 0;
        }
        for (; j >= 0; j--) {
            game->selectable[list[i - j].index] = 1;
        }
        game->unk_438 = value;
        result = i + 1;
    }
    return result;
}

void cardgame_game_start_end_round(CardgameGame *game, CardgameGameData *data) {
    data->board->open_panels(data->board);
    game->display.set_dimmed = 1;
    game->display.request = 1;
    game->step = 0;
}

/* The end of a round (state in CardgameGame.step): scores both sides' cards, picks the round's winner
 * (CardgameGame.round_winner), counts up its wins and shows the results. Returns 1 for the next round, 2 when a side has
 * won two rounds. */
s32 cardgame_game_end_round(CardgameGame *game, CardgameGameData *data) {
    CardgameBoard *board = data->board;
    s32 done = 0;
    s32 side;
    s32 ready;

    switch (game->step) {
    case 0:
        if (data->board->panels[0].state == 2 && game->display.state == 0) {
            game->step = 1;
            game->timer = 0;
        }
        break;
    case 1:
        game->timer = cardgame_game_find_combo(game, 0, game->timer);
        if (game->timer == -1) {
            game->step = 2;
            game->timer = 0;
        } else {
            game->new_effect = 0x19;
            game->effect_state = 1;
        }
        break;
    case 2:
        game->timer = cardgame_game_find_combo(game, 1, game->timer);
        if (game->timer == -1) {
            game->step = 8;
        } else {
            game->new_effect = 0x1A;
            game->effect_state = 1;
        }
        break;
    case 8:
        if (game->swap_card != 0) {
            game->new_effect = 0x1B;
            game->effect_state = 1;
            game->swap_count--;
        }
        if (game->swap_count == 0) {
            game->step = 9;
        } else {
            game->step = 8;
        }
        break;
    case 9:
        if (game->slots[0].count == 0 || game->slots[1].count == 0) {
            if (game->players[0].hp > game->players[1].hp) {
                game->round_winner = 0;
            } else {
                game->round_winner = 1;
            }
            game->step = 11;
            if (game->slots[game->round_winner].count == 0 && game->slots[game->round_winner ^ 1].count != 0) {
                game->step = 10;
                game->timer = 0;
            }
        } else {
            game->step = 3;
            board->open_popup(board, 5, 5, 0xD, 0, 0x6E);
        }
        break;
    case 3:
        if (board->popups[5].state == 2) {
            game->step = 4;
        }
        break;
    case 4:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            game->step = 5;
            board->close_popup(board, 5);
        }
        break;
    case 5:
        if (board->popups[5].state == 0) {
            game->step = 6;
        }
        break;
    case 6:
        game->new_effect = 0x15;
        game->effect_state = 1;
        game->step = 7;
        break;
    case 7:
        game->new_effect = 0x16;
        game->effect_state = 1;
        game->step = 10;
        game->timer = 0;
        break;
    case 10:
        if (game->players[0].hp > game->players[1].hp) {
            game->new_effect = 0x18;
            game->round_winner = 0;
        } else {
            game->new_effect = 0x17;
            game->round_winner = 1;
        }
        game->effect_state = 1;
        game->step = 11;
        game->timer = 0;
        break;
    case 11:
        side = game->round_winner;
        if (game->timer & 1) {
            data->board->panels[side].wins = game->players[side].wins[0] + 1;
        } else {
            data->board->panels[side].wins = game->players[side].wins[0];
        }
        if (game->timer == 0) {
            sound_module.play(0x9C0002);
            data->fade = cardgame_fade_create(1);
            data->fade->set_color(data->fade, 0x80, 0x80, 0x80);
            data->fade->start(data->fade, 0, 0, 0, 10, 1);
        }
        game->timer += gfx_module.funcs.get_frame_ticks();
        if (game->timer >= 51) {
            data->board->panels[side].wins = game->players[side].wins[0] + 1;
            game->players[side].wins[0]++;
            game->step = 12;
            if (game->round_winner == 0) {
                board->open_popup(board, 5, 5, 0x10, 0, 0x6E);
            } else if (game->players[0].hp != game->players[1].hp) {
                board->open_popup(board, 5, 5, 0x11, 0, 0x6E);
            } else {
                board->open_dialog(board, 0x12, 0, 0, 1);
            }
        }
        break;
    case 12:
        if (game->players[0].hp != game->players[1].hp) {
            ready = board->popups[5].state == 2;
        } else {
            ready = board->dialog.state == 2;
        }
        if (ready) {
            game->step = 13;
        }
        break;
    case 13:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            if (game->players[0].hp != game->players[1].hp) {
                board->close_popup(board, 5);
            } else {
                board->close_dialog(board);
            }
            game->step = 14;
        }
        break;
    case 14:
        if (game->players[0].hp != game->players[1].hp) {
            ready = board->popups[5].state == 0;
        } else {
            ready = board->dialog.state == 0;
        }
        if (ready) {
            game->step = 15;
            board->open_dialog(board, 0x13, 0, 0, 1);
        }
        break;
    case 15:
        if (board->dialog.state == 2) {
            game->step = 16;
        }
        break;
    case 16:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            board->close_dialog(board);
            game->step = 17;
        }
        break;
    case 17:
        if (board->dialog.state == 0) {
            if (game->players[0].wins[0] >= 2) {
                sound_module.play(0x6004001E);
                board->open_popup(board, 5, 5, 0x15, 0, 0x6E);
                game->step = 19;
                game->winner = 0;
            } else if (game->players[1].wins[0] >= 2) {
                board->open_popup(board, 5, 5, 0x16, 0, 0x6E);
                game->step = 19;
                game->winner = 1;
            } else {
                game->step = 18;
            }
        }
        break;
    case 18:
        if (game->round_winner == 0) {
            game->new_effect = 0x1C;
        } else {
            game->new_effect = 0x1D;
        }
        game->effect_state = 1;
        game->step = 26;
        break;
    case 19:
        if (board->popups[5].state == 2) {
            game->step = 20;
        }
        break;
    case 20:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            if (game->round_winner == 0) {
                board->close_popup(board, 5);
                game->step = 21;
                game->display.substep = 0;
            } else {
                game->step = 25;
            }
        }
        break;
    case 21:
        if (board->popups[5].state == 0 && cardgame_game_close_panels(game, data) != 0) {
            game->step = 22;
            board->open_dialog(board, game->prize, 0, 0, 3);
        }
        break;
    case 22:
        if (board->dialog.state == 2) {
            game->step = 23;
        }
        break;
    case 23:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            game->step = 25;
        }
        break;
    case 25:
        done = 2;
        break;
    case 26:
        done = 1;
        break;
    }
    return done;
}

void cardgame_game_return_hand(CardgamePlayer *player) {
    while (player->hand_count > 0) {
        player->deck[--player->deck_pos] = player->hand[--player->hand_count];
        player->deck_count++;
    }
}

void cardgame_game_next_round(CardgameGame *game, CardgameGameData *data) {
    CardgamePlayer *player = &game->players[0];

    cardgame_game_return_hand(player);
    cardgame_game_reset_cpu_deck(game, data);
    game->shuffle_deck(game, player->deck_pos, player->deck_count);
    game->swap_card = 0;
    game->swap_count = 0;
    game->players[0].attack = 0;
    game->players[0].hp = 0;
    game->players[1].attack = 0;
    game->players[1].hp = 0;
    cardgame_game_update_panels(game, data);
    game->round++;
    cardgame_game_set_cpu_deck_limits(game, data);
}

void cardgame_game_next_phase(CardgameGame *game, CardgameGameData *data) {
    u8 (*table)[4] = cardgame_phase_next;
    u8 *e;

    game->effect_state = 1;
    e = table[game->prev_phase];
    game->new_effect = e[0];
    game->next_phase = e[2];
}

void cardgame_game_init(CardgameGame *game, CardgameGameData *data) {
    game->players[0].side = 0;
    game->players[1].side = 1;
    game->done = 0;
    game->winner = 1;
    game->phase = 1;
}

s32 cardgame_game_run_phase(CardgameGame *game, CardgameGameData *data) {
    s32 done = 0;

    if (game->next_phase != 0) {
        switch (game->next_phase) {
        case 1:
        case 2:
            game->step = 0;
            game->timer = 0;
            break;
        case 3:
        case 4:
        case 6:
            game->step = 0;
            break;
        case 5:
        case 7:
            cardgame_game_start_turns(game, data);
            break;
        case 8:
            cardgame_game_start_end_round(game, data);
            break;
        case 9:
            cardgame_game_next_round(game, data);
            break;
        }
        game->prev_phase = game->phase;
        game->phase = game->next_phase;
        game->next_phase = 0;
    }
    switch (game->phase) {
    case 1:
        switch (game->step) {
        case 0:
        default:
            data->board->bg_state = 1;
            game->step = 1;
            break;
        case 1:
            if (data->board->bg_state == 2) {
                game->next_phase = 10;
            }
            break;
        }
        break;
    case 2:
        if (cardgame_game_choose_deck(game, data) != 0) {
            game->next_phase = 10;
        }
        break;
    case 3:
        if (cardgame_game_choose_first(game, data) != 0) {
            game->next_phase = 10;
        }
        break;
    case 4:
        switch (cardgame_game_deal(game, data)) {
        case 1:
            game->next_phase = 10;
            break;
        case 2:
            game->winner = 1;
            done = 1;
            break;
        case 3:
            game->winner = 0;
            done = 1;
            break;
        }
        break;
    case 5:
    case 7:
        if (cardgame_game_run_turns(game, data) != 0) {
            game->next_phase = 10;
        }
        break;
    case 6:
        if (cardgame_game_run_placement(game, data) != 0) {
            game->next_phase = 10;
        }
        break;
    case 8:
        switch (cardgame_game_end_round(game, data)) {
        case 1:
            game->next_phase = 9;
            break;
        case 2:
            done = 1;
            break;
        }
        break;
    case 9:
        game->next_phase = 10;
        break;
    case 10:
        cardgame_game_next_phase(game, data);
        break;
    }
    return done;
}

s32 cardgame_game_open_panels(CardgameGame *game, CardgameGameData *data) {
    s32 done = 0;

    if (game->display.substep == 0) {
        game->display.set_dimmed = 1;
        game->display.request = 1;
        data->board->open_panels(data->board);
        game->display.substep++;
    }
    if (data->board->panels[0].state == 2 && game->display.state == 0) {
        done = 1;
    }
    return done;
}

s32 cardgame_game_show_played_card(CardgameGame *game, CardgameGameData *data) {
    s32 done = 0;
    s32 side;
    s32 i;

    switch (game->display.substep) {
    case 0:
        game->display.substep = 1;
        game->menu.timer = 0;
        side = game->turns[game->turn - 1].side;
        data->board->open_popup(data->board, 2, 1, game->card_ids[game->turns[game->turn - 1].card] + 1,
                             cardgame_played_card_pos[main_screen_pos][side][0],
                             cardgame_played_card_pos[main_screen_pos][side][1]);
        for (i = 0; i < 15; i++) {
            data->board->cards[i].zooming = 0;
        }
        break;
    case 1:
        if (game->menu.timer >= 20 && PAD_HELD(13)) {
            game->menu.timer = 35;
        }
        game->menu.timer += gfx_module.funcs.get_frame_ticks();
        if (game->menu.timer >= 35) {
            data->board->close_popup(data->board, 2);
            data->board->zoom_card(data->board, game->turn + 11, 8, 0x1400, 0x1400);
            data->board->close_turn_mark(data->board, game->turn - 1);
            game->display.substep = 2;
            game->menu.timer = 0;
            sound_module.play(0x8004603C);
        }
        break;
    case 2:
        game->menu.timer += gfx_module.funcs.get_frame_ticks();
        if (game->menu.timer >= 8) {
            data->board->flash_card(data->board, game->turn + 11);
            game->display.substep = 3;
            game->menu.timer = 0;
        }
        break;
    case 3:
        game->menu.timer += gfx_module.funcs.get_frame_ticks();
        if (game->menu.timer >= 15) {
            data->board->zoom_card(data->board, game->turn + 11, 4, 0, 0);
            game->display.substep = 5;
            game->menu.timer = 0;
            sound_module.play(0x8004603C);
        }
        break;
    case 4:
        break;
    case 5:
        game->menu.timer += gfx_module.funcs.get_frame_ticks();
        if (game->menu.timer >= 15) {
            done = 1;
        }
        break;
    }
    return done;
}

u8 cardgame_game_resolve_effect(CardgameGame *game, CardgameGameData *data) {
    u8 done = 0;
    s32 id;
    s32 card;
    s32 value;
    s32 i;
    s32 side;

    switch (game->display.resolve_step) {
    case 0:
    default:
        if (cardgame_game_apply_bonuses(game) != 0) {
            game->new_effect = 0x5D;
            game->effect_state = 1;
            game->display.resolve_step = 1;
            break;
        }
        game->display.resolve_step = 3;
    case 3:
        if (data->board->panels[0].state != 0) {
            game->display.resolve_step = 5;
            game->menu.timer = 0;
            game->display.substep = 0;
            game->display.cancelled = 0;
        } else {
            game->display.resolve_step = 4;
        }
        break;
    case 1:
        game->new_effect = 0x4C;
        game->effect_state = 1;
        game->display.resolve_step = 2;
        break;
    case 2:
        game->new_effect = 0x5A;
        game->effect_state = 1;
        game->display.resolve_step = 3;
        break;
    case 4:
        if (cardgame_game_open_panels(game, data) != 0) {
            game->display.resolve_step = 5;
            game->menu.timer = 0;
            game->display.substep = 0;
            game->display.cancelled = 0;
        }
        break;
    case 5:
        if (cardgame_game_show_played_card(game, data) != 0) {
            game->display.resolve_step = 6;
            game->menu.timer = 0;
            game->display.substep = 0;
        }
        break;
    case 6:
        id = game->card_ids[game->turns[game->turn - 1].card];
        if (cardgame_check_condition(game, data->board, cardgame_get_card_data(id, 1, 0)) != 0) {
            data->board->open_dialog(data->board, cardgame_get_card_data(id, 2, 0), 0, 1, 1);
            game->display.resolve_step = 8;
        } else {
            game->display.script_pos = 0;
            game->display.resolve_step = 7;
        }
        break;
    case 7:
        value = cardgame_get_card_data(game->card_ids[game->turns[game->turn - 1].card], 4, game->display.script_pos);
        if (value != 0) {
            game->new_effect = value;
            game->effect_state = 1;
            game->display.script_pos++;
        } else {
            if (game->turn >= 2) {
                game->turns[game->turn - 2].mark = 0;
            }
            for (i = 0; i < 12; i++) {
                data->board->cards[i].turn_marks[game->turn - 1] = 0;
            }
            game->display.resolve_step = 10;
        }
        break;
    case 8:
        if (PAD_PRESSED(13) || PAD_PRESSED(14)) {
            data->board->close_dialog(data->board);
            game->display.resolve_step = 9;
        }
        break;
    case 9:
        if (data->board->dialog.state == 0) {
            game->display.resolve_step = 12;
        }
        break;
    case 10:
        game->new_effect = 0x5A;
        game->effect_state = 1;
        game->display.resolve_step = 11;
        break;
    case 11:
        game->display.resolve_step = 12;
        break;
    case 12:
        card = game->turns[game->turn - 1].card;
        side = game->turns[game->turn - 1].side;
        if (game->card_ids[card] != 13) {
            game->players[side].discard[game->players[side].discard_count] = card;
            game->players[side].discard_count++;
        }
        game->turn--;
        if (game->display.cancelled != 0) {
            game->turn--;
        }
        game->display.resolve_step = 13;
        game->menu.timer = 0;
        game->display.substep = 0;
        break;
    case 13:
        cardgame_game_update_panels(game, data);
        game->display.resolve_step = 14;
        break;
    case 14:
        if (cardgame_game_apply_bonuses(game) != 0) {
            game->new_effect = 0x5D;
            game->effect_state = 1;
            game->display.resolve_step = 15;
        } else {
            done = 1;
        }
        break;
    case 15:
        game->new_effect = 0x4C;
        game->effect_state = 1;
        game->display.resolve_step = 16;
        break;
    case 16:
        game->new_effect = 0x5A;
        game->effect_state = 1;
        game->display.resolve_step = 17;
        break;
    case 17:
        done = 1;
        break;
    }
    return done;
}

/* The in-game menu: saves the effect state and the dialog, the player picks one of five entries (0-2 run effects
 * 0x9B, 0xA8, 0x9C; 3 closes the menu; 4 asks yes/no). Returns 1 when closed, 2 when the answer was 0 (dialog.answer). */
s32 cardgame_game_run_menu(CardgameGame *game, CardgameGameData *data) {
    s32 done = 0;
    s32 sel;

    switch (game->menu.state) {
    default:
    case 1:
        game->menu.saved = *(CardgameEffectSave *)&game->effect;
        game->menu.sel = 0;
        cardgame_saved_dialog = *(CardgameDialogSave *)&data->board->dialog;
        data->board->open_menu(data->board, 0);
        game->menu.state = 2;
        break;
    case 2:
        if (data->board->menu.state == 2) {
            game->menu.state = 3;
        }
        break;
    case 3:
        if ((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 4))) |
            (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 4)))) {
            sel = game->menu.sel - 1;
            if (sel < 0) {
                sel = 4;
            }
            game->menu.sel = sel;
            sound_module.play(0x8004513E);
        } else if ((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 6))) |
                   (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 6)))) {
            game->menu.sel = (u8)(game->menu.sel + 1) % 5;
            sound_module.play(0x8004513E);
        }
        if (PAD_PRESSED(13)) {
            data->board->confirm_menu(data->board);
            game->menu.state = 4;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            data->board->close_menu(data->board);
            game->menu.state = 4;
            game->menu.sel = 3;
        }
        data->board->set_menu_cursor(data->board, game->menu.sel);
        break;
    case 4:
        if (data->board->menu.state == 0) {
            switch (game->menu.sel) {
            case 0:
                game->menu.state = 5;
                break;
            case 1:
                game->menu.state = 6;
                break;
            case 2:
                game->menu.state = 7;
                break;
            case 3:
                game->menu.state = 15;
                break;
            case 4:
                data->board->open_dialog(data->board, 0x2C, 1, 0, 1);
                game->menu.state = 8;
                break;
            }
        }
        break;
    case 5:
        game->new_effect = 0x9B;
        game->effect_state = 1;
        game->menu.state = 11;
        break;
    case 6:
        game->new_effect = 0xA8;
        game->effect_state = 1;
        game->menu.state = 12;
        break;
    case 7:
        game->new_effect = 0x9C;
        game->effect_state = 1;
        game->menu.state = 13;
        break;
    case 8:
        if (data->board->dialog.state == 2) {
            game->menu.state = 9;
        }
        break;
    case 9:
        if (PAD_PRESSED(13)) {
            data->board->confirm_dialog(data->board);
            game->menu.state = 10;
        } else if ((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 4))) |
                   (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 4)))) {
            if (data->board->dialog.answer != 0) {
                sound_module.play(0x8004513E);
            }
            data->board->dialog.answer = 0;
        } else if ((pad_state.get_pressed(0) & (1 << pad_state.get_button_map(0, 6))) |
                   (pad_state.get_repeat(0) & (1 << pad_state.get_button_map(0, 6)))) {
            if (data->board->dialog.answer != 1) {
                sound_module.play(0x8004513E);
            }
            data->board->dialog.answer = 1;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            data->board->dialog.answer = 1;
            data->board->confirm_dialog(data->board);
            data->board->dialog.answer = 1;
            game->menu.state = 10;
        }
        break;
    case 10:
        if (data->board->dialog.state == 0) {
            if (data->board->dialog.answer == 0) {
                done = 2;
            } else {
                game->menu.state = 14;
            }
        }
        break;
    case 11:
    case 12:
    case 13:
    case 14:
        data->board->open_menu(data->board, game->menu.sel);
        game->menu.state = 2;
        break;
    case 15:
        *(CardgameEffectSave *)&game->effect = game->menu.saved;
        done = 1;
        *(CardgameDialogSave *)&data->board->dialog = cardgame_saved_dialog;
        game->menu.state = 0;
        break;
    }
    return done;
}

s32 cardgame_game_run(CardgameGame *game, CardgameGameData *data) {
    s32 done = 0;

    cardgame_game_update_dimmed(game);
    cardgame_game_update_display(game, data);
    cardgame_game_start_fade(game, data);
    switch (game->effect_state) {
    case 0:
    default:
        if (cardgame_game_run_phase(game, data)) {
            done = 1;
        }
        break;
    case 1:
        cardgame_run_effect(game, data->board);
        break;
    case 2:
        if (cardgame_game_resolve_effect(game, data)) {
            game->effect_state = 0;
        }
        break;
    case 3:
        switch (cardgame_game_run_menu(game, data)) {
        case 1:
            game->effect_state = 1;
            break;
        case 2:
            done = 1;
            break;
        }
        break;
    }
    return done;
}

void cardgame_game_update(CardgameGame *game, CardgameGameData *data) {
    Tim tim;

    switch (game->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (sound_module.is_loading() != 0) {
            break;
        }
        cdload_module.files.get_file(0x25D);
        cardgame_game_init(game, data);
        cardgame_game_load_opponent(game, data);
        data->board = cardgame_board_create(game->card_ids);
        data->loader = cardgame_loader_create();
        data->board->reset_panels(data->board);
        data->board->opponent_level = (u8)game->opponent_level;
        data->board->opponent = (u8)game->opponent;
        tim_init(&tim);
        tim.set_image_pos(0x280, 0);
        tim.load_all(cdload_module.get_subfile_by_id(0x025D0000));
        data->fade = cardgame_fade_create(2);
        data->fade->set_color(data->fade, 0xFF, 0xFF, 0xFF);
        data->fade->start(data->fade, 0, 0, 0, 100, 0);
        game->base.set_state(game, OBJECT_STATE_DONE);
        sound_module.play(0x609C0004);
        break;
    case OBJECT_STATE_RUN:
        if (cardgame_game_run(game, data) != 0) {
            data->fade = cardgame_fade_create(2);
            data->fade->set_color(data->fade, 0, 0, 0);
            data->fade->start(data->fade, 0xFF, 0xFF, 0xFF, 100, 0);
            game->base.set_state(game, OBJECT_STATE_DONE);
            game->done = 1;
        }
        break;
    case OBJECT_STATE_DONE:
        if (game->base.step == 0 && data->fade->is_idle(data->fade) != 0) {
            data->fade->close_fade(data->fade);
            switch (game->done) {
            default:
                if (game->winner == 0) {
                    if (gamestate_data.items[game->prize] < 99) {
                        gamestate_data.items[game->prize]++;
                    }
                    gamestate_flags.card_game_won = 1;
                } else {
                    gamestate_flags.card_game_won = 0;
                }
            case 2:
                game->done = 2;
                break;
            case 0:
                game->base.set_state(game, OBJECT_STATE_RUN);
                break;
            }
            game->base.step = 1;
        }
        break;
    case OBJECT_STATE_END:
        if (game->winner == 0) {
            sound_module.stop(0x6004001E);
        } else {
            sound_module.stop(0x609C0004);
        }
        break;
    }
}

CardgameGame *cardgame_game_create(s32 opponent) {
    CardgameGame *obj = object_new(cardgame_game_update, sizeof(CardgameGame), sizeof(CardgameGameData));

    obj->shuffle_deck = cardgame_shuffle_deck;
    obj->sort_cards = cardgame_sort_cards;
    obj->place_cards = cardgame_game_place_cards;
    obj->add_slot = cardgame_game_add_slot;
    obj->get_score = cardgame_cpu_get_score;
    obj->opponent = opponent;
    sound_module.load_extra_bank(0x27);
    return obj;
}

/* .data (address order) */

s16 cardgame_bonus_cards[5] = { 68, 111, 154, 197, 240 };

s16 cardgame_prize_items[36] = {
    91, 91, 361, 362, 363, 364, 365, 366,
    367, 368, 369, 370, 371, 372, 373, 374,
    375, 376, 377, 378, 379, 380, 381, 382,
    383, 384, 385, 386, 387, 388, 389, 390,
    391, 392, 393, 394,
};

s16 cardgame_default_deck[40] = {
    312, 42, 43, 310, 310, 310, 33, 33, 80, 41,
    49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
};

/* unreferenced: a second deck like cardgame_default_deck */
s16 cardgame_unused_deck[40] = {
    41, 41, 80, 21, 7, 26, 8, 41, 2, 8,
    46, 13, 13, 13, 54, 80, 11, 11, 36, 11,
    25, 11, 25, 31, 80, 40, 20, 80, 80, 80,
    80, 1, 1, 313, 35, 24, 24, 80, 59, 58,
};

CardgameFadeColor cardgame_fade_colors[6] = {
    { 0x80, 0x80, 0x80, 1 }, { 0, 0, 0x80, 1 }, { 0, 0x80, 0, 1 }, { 0x80, 0, 0, 1 }, { 0x80, 0x80, 0x80, 2 },
    { 0x80, 0x80, 0, 1 },
};

s16 cardgame_view_sources[7][2] = { { 0, 0 }, { 1, 1 }, { 1, 2 }, { 1, 0 }, { 0, 1 }, { 0, 2 }, { 0, 0 } };
CardgameScreenPos cardgame_deck_choice_pos[3] = { { 23, 80 }, { 23, 125 }, { 23, 170 } };
u8 cardgame_turn_next_state[4][2] = { { 2, 3 }, { 4, 5 }, { 6, 7 }, { 8, 9 } };

u8 cardgame_phase_next[10][4] = {
    { 0, 0, 0, 0 }, { 0xA6, 0, 2, 0 }, { 0xA0, 0, 3, 0 }, { 0xA1, 0, 4, 0 }, { 0xA2, 0, 5, 0 }, { 0xA3, 0, 6, 0 },
    { 0xA4, 0, 7, 0 }, { 0xA5, 0, 8, 0 }, { 0xA0, 0, 3, 0 }, { 0xA0, 0, 3, 0 },
};

s16 cardgame_played_card_pos[2][2][2] = { { { 130, 191 }, { 130, 29 } }, { { 130, 203 }, { 130, 17 } } };
