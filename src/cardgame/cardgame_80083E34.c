#include "common.h"
#include "object.h"
#include "gfx.h"
#include "cardgame.h"

/* The card data table and the card effect dispatcher (cardgame_run_effect): its small helpers come first,
 * the effects' handlers are in cardgame_80085DE8.c. */

/* Card effect IDs that card scripts use (CardgameCardData.script; the full list, with the game flow's and the
 * use effects, is at cardgame_run_effect). "side" is the side that played the card, "other" its opponent. */
enum CardgameEffect {
    CARDGAME_EFFECT_WAIT = 0x01,                        /* wait 45 ticks */
    CARDGAME_EFFECT_SKIP_2 = 0x02,                      /* skip 2 entries */
    CARDGAME_EFFECT_REPEAT_2 = 0x03,                    /* repeat up to REPEAT_END 2 times */
    CARDGAME_EFFECT_REPEAT_3 = 0x04,
    CARDGAME_EFFECT_REPEAT_5 = 0x05,
    CARDGAME_EFFECT_REPEAT_END = 0x06,
    CARDGAME_EFFECT_SKIP_1_IF_CPU = 0x07,               /* skip 1 if the CPU played the card */
    CARDGAME_EFFECT_SKIP_2_IF_CPU = 0x08,
    CARDGAME_EFFECT_SKIP_9_IF_HAND_LE_10 = 0x09,        /* skip 9 if side's hand has <= 10 cards */
    CARDGAME_EFFECT_BACK_9_IF_HAND_GT_10 = 0x0A,        /* go back 9 if it has > 10 */
    CARDGAME_EFFECT_SKIP_9_IF_HAND_LE_3 = 0x0B,
    CARDGAME_EFFECT_BACK_9_IF_HAND_GT_3 = 0x0C,
    CARDGAME_EFFECT_SKIP_4_IF_HAND_GT_2 = 0x0D,
    CARDGAME_EFFECT_SKIP_2_IF_OTHER_DECK = 0x0E,        /* skip 2 if other's deck isn't empty */
    CARDGAME_EFFECT_BACK_4_IF_OTHER_HAND_NOT_KIND_5 = 0x0F,/* go back 4 if other's hand has a card not of kind 5 */
    CARDGAME_EFFECT_SKIP_5_IF_FREE_SLOTS = 0x10,        /* skip 5 if side has free slots */
    CARDGAME_EFFECT_SWAP_SIDE = 0x11,                   /* the card's side becomes the other side */
    CARDGAME_EFFECT_GAIN_POINT_KIND_1 = 0x1E,           /* side's points of kind 1 + 1 */
    CARDGAME_EFFECT_GAIN_POINT_KIND_2 = 0x1F,
    CARDGAME_EFFECT_GAIN_POINT_KIND_3 = 0x20,
    CARDGAME_EFFECT_GAIN_2_POINTS_KIND_3 = 0x21,
    CARDGAME_EFFECT_GAIN_POINT_KIND_4 = 0x22,
    CARDGAME_EFFECT_GAIN_POINT_KIND_5 = 0x23,
    CARDGAME_EFFECT_TAKE_2_POINTS_KIND_1 = 0x24,        /* other's points of kind 1 - 2 */
    CARDGAME_EFFECT_TAKE_2_POINTS_KIND_2 = 0x25,
    CARDGAME_EFFECT_TAKE_2_POINTS_KIND_3 = 0x26,
    CARDGAME_EFFECT_TAKE_2_POINTS_KIND_4 = 0x27,
    CARDGAME_EFFECT_TAKE_2_POINTS_KIND_5 = 0x28,
    CARDGAME_EFFECT_CLEAR_POINTS = 0x29,                /* clear both sides' points */
    CARDGAME_EFFECT_SWAP_TOTALS = 0x2A,                 /* queue a swap of the sides' totals */
    CARDGAME_EFFECT_HIDE_PANELS_PLAYER = 0x2B,          /* hide the panels (only for the player's card) */
    CARDGAME_EFFECT_SHOW_PANELS_PLAYER = 0x2C,
    CARDGAME_EFFECT_HIDE_PANELS = 0x2D,
    CARDGAME_EFFECT_SHOW_PANELS = 0x2E,
    CARDGAME_EFFECT_MESSAGE_39_PLAYER = 0x2F,           /* dialog message 0x39 (for the player's card) */
    CARDGAME_EFFECT_MESSAGE_35 = 0x30,                  /* dialog message 0x35 */
    CARDGAME_EFFECT_MESSAGE_3B_PLAYER = 0x31,           /* dialog message 0x3B (for the player's card) */
    CARDGAME_EFFECT_MARK_DECK = 0x32,                   /* mark selectable cards (cardgame_mark_selectable_cards): side's deck */
    CARDGAME_EFFECT_MARK_OTHER_DECK = 0x33,
    CARDGAME_EFFECT_MARK_DECK_DIGIMON = 0x34,
    CARDGAME_EFFECT_MARK_DECK_OPTIONS = 0x35,
    CARDGAME_EFFECT_MARK_OTHER_HAND = 0x36,
    CARDGAME_EFFECT_MARK_HAND = 0x37,
    CARDGAME_EFFECT_MARK_OTHER_HAND_KIND_6 = 0x38,      /* Digimon of kind 6 in other's hand */
    CARDGAME_EFFECT_MARK_OTHER_HAND_NOT_KIND_5 = 0x39,
    CARDGAME_EFFECT_MARK_DISCARD = 0x3A,                /* side's discard pile */
    CARDGAME_EFFECT_MARK_SLOTS = 0x3B,                  /* mark selectable slots (cardgame_mark_selectable_slots): both sides' */
    CARDGAME_EFFECT_MARK_OTHER_SLOTS = 0x3C,
    CARDGAME_EFFECT_MARK_SIDE_SLOTS = 0x3D,
    CARDGAME_EFFECT_DISCARD_CHOSEN_OTHER_HAND = 0x3E,   /* discard the chosen card of other's hand */
    CARDGAME_EFFECT_DISCARD_CHOSEN_HAND = 0x3F,
    CARDGAME_EFFECT_DISCARD_CHOSEN_OTHER_DECK = 0x40,
    CARDGAME_EFFECT_CANCEL_PREVIOUS = 0x41,             /* cancel the previous card */
    CARDGAME_EFFECT_TAKE_CHOSEN_DECK = 0x42,            /* take the chosen card of side's deck */
    CARDGAME_EFFECT_TAKE_CHOSEN_DISCARD = 0x43,
    CARDGAME_EFFECT_SHUFFLE_DISCARD = 0x44,             /* shuffle side's discard pile into its deck */
    CARDGAME_EFFECT_MARK_DRAW_2 = 0x45,                 /* mark cards of side's deck to draw: 2 */
    CARDGAME_EFFECT_MARK_DRAW_TO_3 = 0x46,              /* 3 - hand */
    CARDGAME_EFFECT_MARK_DRAW_6 = 0x47,
    CARDGAME_EFFECT_DRAW_MARKED = 0x48,
    CARDGAME_EFFECT_DISCARD_HAND = 0x49,
    CARDGAME_EFFECT_CHOOSE_FIRST_OTHER_HAND = 0x4A,     /* choose the first selectable card of other's hand */
    CARDGAME_EFFECT_CHOOSE_OTHER_DECK_TOP = 0x4B,       /* cardgame_choose_deck_top */
    CARDGAME_EFFECT_CLOSE_SLOT_GAPS = 0x4C,             /* close the gaps between the slots */
    CARDGAME_EFFECT_DISCARD_MARKED_SLOTS = 0x4D,
    CARDGAME_EFFECT_RETURN_MARKED_SLOTS = 0x4E,         /* return the marked slots to their owners' hands */
    CARDGAME_EFFECT_SUMMON_50 = 0x4F,                   /* summon card 0x50 into side's slots */
    CARDGAME_EFFECT_SUMMON_51 = 0x50,
    CARDGAME_EFFECT_SUMMON_52 = 0x51,
    CARDGAME_EFFECT_SUMMON_53 = 0x52,
    CARDGAME_EFFECT_SUMMON_54 = 0x53,
    CARDGAME_EFFECT_SUMMON_55 = 0x54,
    CARDGAME_EFFECT_SUMMON_56 = 0x55,
    CARDGAME_EFFECT_PLACE_TARGET = 0x56,                /* put the chosen hand card (the turn's target) into a slot */
    CARDGAME_EFFECT_COPY_MARKED_SLOT = 0x57,
    CARDGAME_EFFECT_ATTACK_ANIM = 0x58,                 /* attack animation on the marked slots */
    CARDGAME_EFFECT_DESTROY_ANIM = 0x59,
    CARDGAME_EFFECT_RECOUNT = 0x5A,                     /* recount the totals */
    CARDGAME_EFFECT_MARK_TARGET_SLOT = 0x5B,            /* mark the turn's target slot */
    CARDGAME_EFFECT_MARK_TARGET_AREA = 0x5C,
    CARDGAME_EFFECT_UPDATE_SLOTS = 0x5D,                /* update the slots' shown values */
    CARDGAME_EFFECT_ATTACK_HP_PLUS_10 = 0x5E,           /* the marked slots' attack and hp + 10 */
    CARDGAME_EFFECT_HP_PLUS_30 = 0x5F,
    CARDGAME_EFFECT_ATTACK_HP_PLUS_50 = 0x60,
    CARDGAME_EFFECT_ATTACK_HP_PLUS_20 = 0x61,
    CARDGAME_EFFECT_ATTACK_HP_PLUS_30 = 0x62,
    CARDGAME_EFFECT_HP_PLUS_10 = 0x63,
    CARDGAME_EFFECT_ATTACK_PLUS_10 = 0x64,
    CARDGAME_EFFECT_HP_MINUS_60 = 0x65,
    CARDGAME_EFFECT_HP_MINUS_15 = 0x66,
    CARDGAME_EFFECT_HP_MINUS_30 = 0x67,
    CARDGAME_EFFECT_ATTACK_PLUS_10_HP_MINUS_10 = 0x68,
    CARDGAME_EFFECT_ATTACK_ZERO = 0x69,
    CARDGAME_EFFECT_FADE_1 = 0x6A,                      /* cardgame_fade_colors */
    CARDGAME_EFFECT_FADE_2 = 0x6B,
    CARDGAME_EFFECT_FADE_3 = 0x6C,
    CARDGAME_EFFECT_FADE_4 = 0x6D,
    CARDGAME_EFFECT_FADE_5 = 0x6E,
    CARDGAME_EFFECT_FADE_6 = 0x6F,
    CARDGAME_EFFECT_CHOOSE_OTHER_HAND = 0xA9,           /* choose a card of other's hand (CPU: the lowest value) */
    CARDGAME_EFFECT_CHOOSE_HAND = 0xAA,                 /* of side's hand (CPU: the highest) */
    CARDGAME_EFFECT_CHOOSE_DISCARD = 0xAB,              /* of side's discard pile (CPU: the lowest) */
    CARDGAME_EFFECT_CHOOSE_DECK = 0xAC,                 /* of side's deck (CPU: the next planned) */
    CARDGAME_EFFECT_CHOOSE_OTHER_DECK = 0xAD,           /* of other's deck (CPU: the lowest of the player's deck) */
    CARDGAME_EFFECT_CHOOSE_SLOT = 0xAE                  /* a selectable slot (CPU: the best score) */
};

/* A card's data (cardgame_card_data, 60 entries; cardgame_get_card_data fields 0-4). Playing the card runs
 * use_effect (0x70-0x97: ask the player to use it and choose its target); once played, its effect checks
 * the condition (when cardgame_check_condition returns nonzero the card has no effect and the dialog shows
 * condition_message), otherwise it runs the script, a 0-terminated list of effect IDs
 * (cardgame_game_resolve_effect; the IDs are listed at cardgame_run_effect). */
typedef struct CardgameCardData {
    /* 0x00 */ u8 use_effect;
    /* 0x01 */ u8 condition;         /* cardgame_check_condition's kind */
    /* 0x02 */ u8 condition_message; /* dialog message when the card has no effect */
    /* 0x03 */ u8 target_kind;       /* CardgameTurn.target_kind */
    /* 0x04 */ u8 script[0x28];      /* CardgameEffect IDs, 0-terminated */
} CardgameCardData; /* size 0x2C */

extern CardgameCardData cardgame_card_data[60];

s32 cardgame_points_update(CardgameGame *game, CardgameBoard *board, s32 side, s32 kind);

u8 cardgame_get_card_data(s32 card, u32 field, s32 i) {
    u8 value = 0;

    switch (field) {
    case 0:
        value = cardgame_card_data[card].use_effect;
        break;
    case 1:
        value = cardgame_card_data[card].condition;
        break;
    case 2:
        value = cardgame_card_data[card].condition_message;
        break;
    case 3:
        value = cardgame_card_data[card].target_kind;
        break;
    case 4:
        value = cardgame_card_data[card].script[i];
        break;
    }
    return value;
}

void cardgame_skip_if_played_by(CardgameGame *game, CardgameBoard *board, s32 side, s32 n) {
    if (game->turns[game->turn - 1].side == side) {
        game->display.script_pos += n;
    }
}

void cardgame_skip_if_hand_at_most(CardgameGame *game, s32 side, s32 count, s32 n) {
    if (count >= game->players[side].hand_count) {
        game->display.script_pos += n;
    }
}

void cardgame_skip_if_hand_above(CardgameGame *game, s32 side, s32 count, s32 n) {
    if (count < game->players[side].hand_count) {
        game->display.script_pos += n;
    }
}

void cardgame_skip_if_deck_left(CardgameGame *game, s32 side, s32 n) {
    if (game->players[side].deck_count > 0) {
        game->display.script_pos += n;
    }
}

void cardgame_skip_if_slots_free(CardgameGame *game, s32 side, s32 n) {
    if (game->slots[side].count < 6) {
        game->display.script_pos += n;
    }
}

s32 cardgame_effect_wait(CardgameGame *game, CardgameBoard *board) {
    game->effect_time -= gfx_module.funcs.get_frame_ticks();
    return game->effect_time <= 0;
}

void cardgame_choose_deck_top(CardgameGame *game, CardgameBoard *board, s32 arg2) {
    if (arg2 == 0) {
        game->choice = game->players[0].deck_pos;
    } else {
        game->choice = game->cpu_deck_last;
    }
}

void cardgame_mark_turn_target(CardgameGame *game, CardgameBoard *board) {
    CardgameSlot *slot;
    s32 turn = game->turn - 1;
    s32 i;

    for (i = 0; i < 12; i++) {
        game->marked[i] = 0;
        if (i < 6) {
            if (i >= game->slots[0].count) {
                continue;
            }
            slot = &game->slots[0].slots[i];
        } else {
            if (i - 6 >= game->slots[1].count) {
                continue;
            }
            slot = &game->slots[1].slots[i - 6];
        }
        if (slot->id == game->turns[turn].target) {
            game->marked[i] = 1;
        }
    }
}

void cardgame_mark_best_cpu_slot(CardgameGame *game) {
    s32 best = 0;
    s32 besti = 6;
    s32 i;
    s32 v;

    for (i = 0; i < 12; i++) {
        game->marked[i] = 0;
        if (i >= 6) {
            v = game->get_score(game, 1, 1 << (i - 6));
            if (best < v) {
                best = v;
                game->marked[besti] = 0;
                besti = i;
                game->choice = i;
                game->marked[i] = 1;
            }
        }
    }
}

s32 cardgame_effect_points_update(CardgameGame *game, CardgameBoard *board) {
    s32 side = 0;
    s32 kind = 0;
    s32 card;

    switch (game->effect) {
    case 30:
    case 36:
        kind = 0;
        break;
    case 31:
    case 37:
        kind = 1;
        break;
    case 32:
    case 33:
    case 38:
        kind = 2;
        break;
    case 34:
    case 39:
        kind = 3;
        break;
    case 35:
    case 40:
        kind = 4;
        break;
    }
    card = game->effect;
    if (card >= 30) {
        if (card < 36) {
            side = game->turns[game->turn - 1].side;
        } else if (card < 41) {
            side = game->turns[game->turn - 1].side ^ 1;
        }
    }
    return cardgame_points_update(game, board, side, kind);
}

/* The effects' handlers (cardgame_80085DE8.c). */
void cardgame_ask_play_start(CardgameGame *game, CardgameBoard *board, s32 mode);
s32 cardgame_ask_play_update(CardgameGame *game, CardgameBoard *board);
void cardgame_hand_start(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player);
void cardgame_view_start(CardgameGame *game, CardgameBoard *board, s32 n, s32 request);
s32 cardgame_hand_update(CardgameGame *game, CardgameBoard *board, CardgamePlayer *player);
void cardgame_reveal_start(CardgameGame *game, CardgameBoard *board);
s32 cardgame_reveal_update(CardgameGame *game, CardgameBoard *board);
void cardgame_banner_start(CardgameGame *game, CardgameBoard *board, s32 i);
s32 cardgame_banner_update(CardgameGame *game, CardgameBoard *board);
void cardgame_choose_first_start(CardgameGame *game, CardgameBoard *board);
s32 cardgame_choose_first_update(CardgameGame *game, CardgameBoard *board);
void cardgame_view_board_start(CardgameGame *game, CardgameBoard *board);
s32 cardgame_view_board_update(CardgameGame *game, CardgameBoard *board);
void cardgame_choose_slot_start(CardgameGame *game, CardgameBoard *board, s32 arg2);
s32 cardgame_choose_slot_update(CardgameGame *game, CardgameBoard *board);
void cardgame_choose_card_start(CardgameGame *game, CardgameBoard *board, s32 side, s32 mode);
void cardgame_choose_hand_card_start(CardgameGame *game, CardgameBoard *board, s32 arg2);
s32 cardgame_choose_card_update(CardgameGame *game, CardgameBoard *board, s32 mode);
s32 cardgame_cpu_select_cards(CardgameGame *game, CardgameBoard *board);
void cardgame_cpu_choose_card_by_value(CardgameGame *game, CardgameBoard *board, s32 mode);
void cardgame_cpu_choose_own_deck_card(CardgameGame *game, CardgameBoard *board);
void cardgame_cpu_choose_player_deck_card(CardgameGame *game, CardgameBoard *board);
void cardgame_ask_use_start(CardgameGame *game, CardgameBoard *board, s32 flags);
s32 cardgame_ask_use_update(CardgameGame *game, CardgameBoard *board);
void cardgame_ask_cancel_start(CardgameGame *game, CardgameBoard *board, s32 mode);
s32 cardgame_ask_cancel_update(CardgameGame *game, CardgameBoard *board);
void cardgame_ask_area_start(CardgameGame *game, CardgameBoard *board, s32 side, s32 mode);
s32 cardgame_ask_area_update(CardgameGame *game, CardgameBoard *board);
void cardgame_card_effect_start(CardgameGame *game, CardgameBoard *board, s32 mode);
s32 cardgame_card_effect_update(CardgameGame *game, CardgameBoard *board, s32 limit);
void cardgame_change_stats_start(CardgameGame *game, CardgameBoard *board, s32 delta, s32 mode);
s32 cardgame_change_stats_update(CardgameGame *game, CardgameBoard *board);
void cardgame_mark_turn_area(CardgameGame *game, CardgameBoard *board);
void cardgame_compact_slots_start(CardgameGame *game, CardgameBoard *board);
s32 cardgame_compact_slots_update(CardgameGame *game, CardgameBoard *board);
s32 cardgame_mark_selectable_cards(CardgameGame *game, CardgameBoard *board, s32 side, s32 where, s32 flags);
s32 cardgame_mark_selectable_slots(CardgameGame *game, CardgameBoard *board, s32 mode, s32 flags);
void cardgame_discard_hand_start(CardgameGame *game, CardgameBoard *board, s32 side);
s32 cardgame_discard_hand_update(CardgameGame *game, CardgameBoard *board, s32 side);
void cardgame_points_start(CardgameGame *game, CardgameBoard *board, s32 delta, s32 ticks);
void cardgame_clear_points_start(CardgameGame *game, CardgameBoard *board);
s32 cardgame_clear_points_update(CardgameGame *game, CardgameBoard *board);
void cardgame_cancel_card_start(CardgameGame *game, CardgameBoard *board);
s32 cardgame_cancel_card_update(CardgameGame *game, CardgameBoard *board);
s32 cardgame_queue_swap_totals(CardgameGame *game, CardgameBoard *board);
void cardgame_discard_card_start(CardgameGame *game, CardgameBoard *board, s32 side, s32 which);
s32 cardgame_discard_card_update(CardgameGame *game, CardgameBoard *board, s32 side, s32 which);
void cardgame_take_card_start(CardgameGame *game, CardgameBoard *board, s32 side, s32 which);
s32 cardgame_take_card_update(CardgameGame *game, CardgameBoard *board, s32 side, s32 which);
void cardgame_recycle_discard_start(CardgameGame *game, CardgameBoard *board, s32 side);
s32 cardgame_recycle_discard_update(CardgameGame *game, CardgameBoard *board, s32 side);
void cardgame_mark_draw_start(CardgameGame *game, CardgameBoard *board, s32 n);
s32 cardgame_mark_draw_update(CardgameGame *game, CardgameBoard *board, s32 side);
void cardgame_draw_start(CardgameGame *game, CardgameBoard *board, s32 side);
s32 cardgame_draw_update(CardgameGame *game, CardgameBoard *board, s32 side);
void cardgame_remove_marked_start(CardgameGame *game, CardgameBoard *board);
s32 cardgame_remove_marked_update(CardgameGame *game, CardgameBoard *board, s32 mode);
void cardgame_summon_start(CardgameGame *game, CardgameBoard *board, s32 side, s32 card);
void cardgame_summon_from_hand_start(CardgameGame *game, CardgameBoard *board, s32 side);
s32 cardgame_summon_update(CardgameGame *game, CardgameBoard *board, s32 side);
void cardgame_copy_slot_start(CardgameGame *game, CardgameBoard *board, s32 side);
s32 cardgame_copy_slot_update(CardgameGame *game, CardgameBoard *board, s32 side);
void cardgame_play_start(CardgameGame *game, CardgameBoard *board, s32 mode);
s32 cardgame_play_update(CardgameGame *game, CardgameBoard *board);
void cardgame_deal_start(CardgameGame *game, CardgameBoard *board);
s32 cardgame_deal_update(CardgameGame *game, CardgameBoard *board);
void cardgame_attack_start(CardgameGame *game, CardgameBoard *board, s32 side);
s32 cardgame_attack_update(CardgameGame *game, CardgameBoard *board, s32 side);
void cardgame_discard_slots_start(CardgameGame *game, CardgameBoard *board, s32 side);
s32 cardgame_discard_slots_update(CardgameGame *game, CardgameBoard *board, s32 side);
void cardgame_swap_totals_start(CardgameGame *game, CardgameBoard *board);
s32 cardgame_swap_totals_update(CardgameGame *game, CardgameBoard *board);
void cardgame_combo_start(CardgameGame *game, CardgameBoard *board, s32 side);
s32 cardgame_combo_update(CardgameGame *game, CardgameBoard *board, s32 side);
void cardgame_recount_totals_start(CardgameGame *game, CardgameBoard *board);
s32 cardgame_recount_totals_update(CardgameGame *game, CardgameBoard *board);
void cardgame_clear_slots_start(CardgameGame *game, CardgameBoard *board);
s32 cardgame_clear_slots_update(CardgameGame *game, CardgameBoard *board, s32 side);
void cardgame_message_start(CardgameGame *game, CardgameBoard *board, s32 arg2);
s32 cardgame_message_update(CardgameGame *game, CardgameBoard *board);
void cardgame_hide_panels_start(CardgameGame *game, CardgameBoard *board, s32 force);
s32 cardgame_hide_panels_update(CardgameGame *game, CardgameBoard *board);
void cardgame_show_panels_start(CardgameGame *game, CardgameBoard *board, s32 force);
s32 cardgame_show_panels_update(CardgameGame *game, CardgameBoard *board);
s32 cardgame_sync_slot_values(CardgameGame *game, CardgameBoard *board);

/* Runs the current card effect: starts the one in new_effect (its first handler), then runs effect's second
 * handler every frame; the second handlers' results set effect_state (2: back to the card's script, 0: back to
 * the game flow, which reads answer). side = the side that played the card (previous turn), other = its opponent,
 * cur = this turn's side. "Kind" is a card's picture data byte 0 (1-6; points[k - 1]); Digimon cards are the
 * ones whose byte 3 is 0x10 (they go in the slots), the others are option cards.
 *
 * The effect IDs. Script control (cardgame_game_resolve_effect steps through the played card's script):
 *   0x01 wait 45 ticks                       0x02 skip 2 entries
 *   0x03-0x05 repeat up to 0x06, 2/3/5 times 0x06 end of the repeat
 *   0x07/0x08 skip 1/2 if the CPU played it  0x09/0x0B skip 9 if side's hand <= 10/3 cards
 *   0x0A/0x0C back 9 if side's hand > 10/3   0x0D skip 4 if side's hand > 2
 *   0x0E skip 2 if other's deck isn't empty  0x0F back 4 if other's hand has a card not of kind 5
 *   0x10 skip 5 if side has free slots       0x11 the card's side becomes the other side
 * The game flow's effects:
 *   0x12/0x13 play a card                    0x14 deal
 *   0x15/0x16 attack (side 0/1)              0x17/0x18 discard the slots (side 0/1)
 *   0x19/0x1A combo (side 0/1)               0x1B swap the sides' totals (queued by 0x2A)
 *   0x1C/0x1D clear the slots (round end)
 * The card effects:
 *   0x1E-0x23 side's points +1 (kinds 1, 2, 3, 3, 4, 5; 0x21: +2)
 *   0x24-0x28 other's points -2 (kinds 1-5)  0x29 clear both sides' points
 *   0x2A queue a swap of the totals          0x2B/0x2D hide the panels (0x2B only for the player's card)
 *   0x2C/0x2E show the panels (likewise)     0x2F/0x30/0x31 message 0x39 (player)/0x35/0x3B (player)
 *   0x32-0x3A mark selectable cards (cardgame_mark_selectable_cards): side's deck (0x32 all, 0x34 Digimon,
 *             0x35 options), other's deck (0x33), other's hand (0x36 all, 0x38 Digimon of kind 6, 0x39 not
 *             kind 5), side's hand (0x37), side's discard pile (0x3A)
 *   0x3B-0x3D mark selectable slots (cardgame_mark_selectable_slots): both sides', other's, side's
 *   0x3E/0x3F discard the chosen card of other's/side's hand    0x40 of other's deck
 *   0x41 cancel the previous card            0x42/0x43 take the chosen card of side's deck/discard pile
 *   0x44 shuffle side's discard pile into its deck
 *   0x45-0x47 mark 2 / 3 - hand / 6 cards of side's deck to draw                0x48 draw the marked cards
 *   0x49 discard side's hand                 0x4A choose the first selectable card of other's hand
 *   0x4B choose the next card of other's deck (cardgame_choose_deck_top)  0x4C close the gaps between the slots
 *   0x4D discard the marked slots            0x4E return the marked slots to their owners' hands
 *   0x4F-0x55 summon card 0x50-0x56 into side's slots
 *   0x56 put the chosen hand card (the turn's target) into a slot             0x57 copy the marked slot
 *   0x58/0x59 attack/destroy animation on the marked slots
 *   0x5A recount the totals                  0x5B mark the turn's target slot
 *   0x5C mark the turn's target area         0x5D update the slots' shown values (dead slots: 0x4D)
 *   0x5E-0x69 change the marked slots' attack/hp: +10/+10, hp +30, +50/+50, +20/+20, +30/+30, hp +10,
 *             attack +10, hp -60, hp -15, hp -30, attack +10/hp -10, attack to 0
 *   0x6A-0x6F fade 1-6 (cardgame_fade_colors)
 * Using a card (CardgameCardData.use_effect; the answer is 1 to use it):
 *   0x70-0x82 ask, lighting the points it costs   0x83/0x84 ask to cancel the previous card (0x84: kind 6)
 *   0x85-0x8C mark selectable slots of this turn's side, then 0x8D choose one (the target)
 *   0x8E-0x95 ask, showing the target area (CardgameTurn.target_kind 1-8)
 *   0x96 mark this turn's side's Digimon cards in hand, then 0x97 choose one (the target)
 * The player's choices (the CPU's in brackets):
 *   0x98 ask whether to play                 0x99 choose the card to play (the side playing)
 *   0x9A the player's hand (placement)       0x9B/0x9C view the hand/discard pile (menu)
 *   0x9D (CPU: select the cards to place)    0x9E reveal                       0x9F-0xA6 banners 0-6
 *   0xA7 choose who plays first              0xA8 view the board (menu)
 *   0xA9 choose a card of other's hand (lowest value)  0xAA of side's hand (highest)
 *   0xAB of side's discard pile (lowest)     0xAC of side's deck (next planned)
 *   0xAD of other's deck (lowest of the player's deck)  0xAE choose a selectable slot (best score) */
void cardgame_run_effect(CardgameGame *game, CardgameBoard *board) {
    CardPicture pic;
    s32 side;
    s32 cur;
    s32 other;
    s32 i;
    s32 j;
    s32 k;
    s32 n;
    s32 ret;

    side =game->turns[game->turn - 1].side;
    cur = game->turns[game->turn].side;
    other = side ^ 1;
    if (game->new_effect != 0) {
        switch (game->new_effect) {
        case 0x01:
            game->effect_time = 45;
            break;
        case 0x12:
            cardgame_play_start(game, board, 0);
            break;
        case 0x13:
            cardgame_play_start(game, board, 1);
            break;
        case 0x14:
            cardgame_deal_start(game, board);
            break;
        case 0x1B:
            cardgame_swap_totals_start(game, board);
            break;
        case 0x15:
            cardgame_attack_start(game, board, 0);
            break;
        case 0x16:
            cardgame_attack_start(game, board, 1);
            break;
        case 0x17:
            cardgame_discard_slots_start(game, board, 0);
            break;
        case 0x18:
            cardgame_discard_slots_start(game, board, 1);
            break;
        case 0x19:
            cardgame_combo_start(game, board, 0);
            break;
        case 0x1A:
            cardgame_combo_start(game, board, 1);
            break;
        case 0x1C:
        case 0x1D:
            cardgame_clear_slots_start(game, board);
            break;
        case 0x1E:
        case 0x1F:
        case 0x20:
        case 0x22:
        case 0x23:
            cardgame_points_start(game, board, 1, 30);
            break;
        case 0x21:
            cardgame_points_start(game, board, 2, 30);
            break;
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
            cardgame_points_start(game, board, -2, 30);
            break;
        case 0x29:
            cardgame_clear_points_start(game, board);
            break;
        case 0x2F:
            cardgame_message_start(game, board, 0x39);
            break;
        case 0x30:
            cardgame_message_start(game, board, 0x35);
            break;
        case 0x31:
            if (side == 0) {
                cardgame_message_start(game, board, 0x3B);
            }
            break;
        case 0x2B:
            cardgame_hide_panels_start(game, board, 0);
            break;
        case 0x2D:
            cardgame_hide_panels_start(game, board, 1);
            break;
        case 0x2C:
            cardgame_show_panels_start(game, board, 0);
            break;
        case 0x2E:
            cardgame_show_panels_start(game, board, 1);
            break;
        case 0x3E:
            cardgame_discard_card_start(game, board, other, 0);
            break;
        case 0x3F:
            cardgame_discard_card_start(game, board, side, 0);
            break;
        case 0x40:
            cardgame_discard_card_start(game, board, other, 1);
            break;
        case 0x41:
            cardgame_cancel_card_start(game, board);
            break;
        case 0x42:
            cardgame_take_card_start(game, board, side, 3);
            break;
        case 0x43:
            cardgame_take_card_start(game, board, side, 4);
            break;
        case 0x44:
            cardgame_recycle_discard_start(game, board, side);
            break;
        case 0x45:
            cardgame_mark_draw_start(game, board, 2);
            break;
        case 0x46:
            n = 3;
            n -= game->players[side].hand_count;
            if (n <= 0) {
                n = 0;
            }
            cardgame_mark_draw_start(game, board, n);
            break;
        case 0x47:
            cardgame_mark_draw_start(game, board, 6);
            break;
        case 0x48:
            cardgame_draw_start(game, board, side);
            break;
        case 0x49:
            cardgame_discard_hand_start(game, board, side);
            break;
        case 0x4C:
            cardgame_compact_slots_start(game, board);
            break;
        case 0x4D:
        case 0x4E:
            cardgame_remove_marked_start(game, board);
            break;
        case 0x5D:
            game->effect_step = 1;
            break;
        case 0x5A:
            cardgame_recount_totals_start(game, board);
            break;
        case 0x4F:
            cardgame_summon_start(game, board, side, 0x50);
            break;
        case 0x50:
            cardgame_summon_start(game, board, side, 0x51);
            break;
        case 0x51:
            cardgame_summon_start(game, board, side, 0x52);
            break;
        case 0x52:
            cardgame_summon_start(game, board, side, 0x53);
            break;
        case 0x53:
            cardgame_summon_start(game, board, side, 0x54);
            break;
        case 0x54:
            cardgame_summon_start(game, board, side, 0x55);
            break;
        case 0x55:
            cardgame_summon_start(game, board, side, 0x56);
            break;
        case 0x56:
            cardgame_summon_from_hand_start(game, board, side);
            break;
        case 0x57:
            cardgame_copy_slot_start(game, board, side);
            break;
        case 0x58:
            cardgame_card_effect_start(game, board, 0);
            break;
        case 0x59:
            cardgame_card_effect_start(game, board, 1);
            break;
        case 0x5E:
            cardgame_change_stats_start(game, board, 0x000A000A, 1);
            break;
        case 0x5F:
            cardgame_change_stats_start(game, board, 0x0000001E, 1);
            break;
        case 0x60:
            cardgame_change_stats_start(game, board, 0x00320032, 1);
            break;
        case 0x61:
            cardgame_change_stats_start(game, board, 0x00140014, 1);
            break;
        case 0x62:
            cardgame_change_stats_start(game, board, 0x001E001E, 1);
            break;
        case 0x63:
            cardgame_change_stats_start(game, board, 0x0000000A, 1);
            break;
        case 0x64:
            cardgame_change_stats_start(game, board, 0x000A0000, 1);
            break;
        case 0x65:
            cardgame_change_stats_start(game, board, 0x0000FFC4, 0);
            break;
        case 0x66:
            cardgame_change_stats_start(game, board, 0x0000FFF1, 0);
            break;
        case 0x67:
            cardgame_change_stats_start(game, board, 0x0000FFE2, 0);
            break;
        case 0x68:
            cardgame_change_stats_start(game, board, 0x000AFFF6, 0);
            break;
        case 0x69:
            cardgame_change_stats_start(game, board, 0xFF9D0000, 2);
            break;
        case 0x6A:
            game->fade = 1;
            game->effect_time = 10;
            break;
        case 0x6B:
            game->fade = 2;
            game->effect_time = 10;
            break;
        case 0x6C:
            game->fade = 3;
            game->effect_time = 10;
            break;
        case 0x6D:
            game->fade = 4;
            game->effect_time = 10;
            break;
        case 0x6E:
            game->fade = 5;
            game->effect_time = 15;
            break;
        case 0x6F:
            game->fade = 6;
            game->effect_time = 10;
            break;
        case 0x98:
            cardgame_ask_play_start(game, board, 14);
            break;
        case 0x9B:
            cardgame_view_start(game, board, game->players[0].hand_count, 5);
            break;
        case 0x9C:
            cardgame_view_start(game, board, game->players[0].discard_count, 9);
            break;
        case 0xA8:
            cardgame_view_board_start(game, board);
            break;
        case 0x9A:
            cardgame_hand_start(game, board, &game->players[0]);
            break;
        case 0x9D:
            game->selected = 0;
            break;
        case 0x9E:
            cardgame_reveal_start(game, board);
            break;
        case 0x99:
            cardgame_hand_start(game, board, &game->players[game->side]);
            break;
        case 0x9F:
        case 0xA0:
            cardgame_banner_start(game, board, 0);
            break;
        case 0xA1:
            cardgame_banner_start(game, board, 1);
            break;
        case 0xA2:
            cardgame_banner_start(game, board, 2);
            break;
        case 0xA3:
            cardgame_banner_start(game, board, 3);
            break;
        case 0xA4:
            cardgame_banner_start(game, board, 4);
            break;
        case 0xA5:
            cardgame_banner_start(game, board, 5);
            break;
        case 0xA6:
            cardgame_banner_start(game, board, 6);
            break;
        case 0xA7:
            cardgame_choose_first_start(game, board);
            break;
        case 0xAC:
            cardgame_choose_card_start(game, board, side, 2);
            break;
        case 0xAD:
            cardgame_choose_card_start(game, board, other, 1);
            break;
        case 0xA9:
            cardgame_choose_card_start(game, board, other, 0);
            break;
        case 0xAA:
            cardgame_choose_card_start(game, board, side, 0);
            break;
        case 0xAB:
            cardgame_choose_card_start(game, board, side, 3);
            break;
        case 0xAE:
            cardgame_choose_slot_start(game, board, 0);
            break;
        case 0x70:
            cardgame_ask_use_start(game, board, 0);
            break;
        case 0x71:
            cardgame_ask_use_start(game, board, 0x4000);
            break;
        case 0x72:
            cardgame_ask_use_start(game, board, 0x2000);
            break;
        case 0x73:
            cardgame_ask_use_start(game, board, 0x1);
            break;
        case 0x74:
            cardgame_ask_use_start(game, board, 0x4);
            break;
        case 0x75:
            cardgame_ask_use_start(game, board, 0x10);
            break;
        case 0x76:
            cardgame_ask_use_start(game, board, 0x40);
            break;
        case 0x77:
            cardgame_ask_use_start(game, board, 0x100);
            break;
        case 0x78:
            cardgame_ask_use_start(game, board, 0x2);
            break;
        case 0x79:
            cardgame_ask_use_start(game, board, 0x8);
            break;
        case 0x7A:
            cardgame_ask_use_start(game, board, 0x20);
            break;
        case 0x7B:
            cardgame_ask_use_start(game, board, 0x80);
            break;
        case 0x7C:
            cardgame_ask_use_start(game, board, 0x200);
            break;
        case 0x7D:
            cardgame_ask_use_start(game, board, 0x3FF);
            break;
        case 0x7E:
            cardgame_ask_use_start(game, board, 0xF0000);
            break;
        case 0x7F:
            cardgame_ask_use_start(game, board, 0x8000);
            break;
        case 0x81:
            cardgame_ask_use_start(game, board, 0x400);
            break;
        case 0x80:
            cardgame_ask_use_start(game, board, 0x1400);
            break;
        case 0x82:
            cardgame_ask_use_start(game, board, 0x1000);
            break;
        case 0x83:
            cardgame_ask_cancel_start(game, board, 0);
            break;
        case 0x84:
            cardgame_ask_cancel_start(game, board, 1);
            break;
        case 0x8D:
            cardgame_choose_slot_start(game, board, 1);
            break;
        case 0x8E:
            cardgame_ask_area_start(game, board, cur, 1);
            break;
        case 0x8F:
            cardgame_ask_area_start(game, board, cur, 2);
            break;
        case 0x90:
            cardgame_ask_area_start(game, board, cur, 3);
            break;
        case 0x91:
            cardgame_ask_area_start(game, board, cur, 4);
            break;
        case 0x92:
            cardgame_ask_area_start(game, board, cur, 5);
            break;
        case 0x93:
            cardgame_ask_area_start(game, board, cur, 6);
            break;
        case 0x94:
            cardgame_ask_area_start(game, board, cur, 7);
            break;
        case 0x95:
            cardgame_ask_area_start(game, board, cur, 8);
            break;
        case 0x97:
            cardgame_choose_hand_card_start(game, board, cur);
            break;
        }
        game->effect = game->new_effect;
        game->new_effect = 0;
    }

    switch (game->effect) {
    case 0x00:
        break;
    case 0x01:
        if (cardgame_effect_wait(game, board) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x02:
        game->effect_state = 2;
        game->display.script_pos += 2;
        break;
    case 0x03:
        game->display.repeat_count = 2;
        game->effect_state = 2;
        game->display.repeat_pos = game->display.script_pos;
        break;
    case 0x04:
        game->display.repeat_count = 3;
        game->effect_state = 2;
        game->display.repeat_pos = game->display.script_pos;
        break;
    case 0x05:
        game->display.repeat_count = 5;
        game->effect_state = 2;
        game->display.repeat_pos = game->display.script_pos;
        break;
    case 0x06:
        if (--game->display.repeat_count > 0) {
            game->display.script_pos = game->display.repeat_pos;
        }
        game->effect_state = 2;
        break;
    case 0x07:
        cardgame_skip_if_played_by(game, board, 1, 1);
        game->effect_state = 2;
        break;
    case 0x08:
        cardgame_skip_if_played_by(game, board, 1, 2);
        game->effect_state = 2;
        break;
    case 0x0F:
        card_init(&pic);
        for (i = 0; i < game->players[other].hand_count; i++) {
            pic.select(game->card_ids[game->players[other].hand[i]] + 1);
            if (pic.record[0] != 5) {
                game->display.script_pos -= 4;
                break;
            }
        }
        game->effect_state = 2;
        break;
    case 0x09:
        cardgame_skip_if_hand_at_most(game, side, 10, 9);
        game->effect_state = 2;
        break;
    case 0x0A:
        cardgame_skip_if_hand_above(game, side, 10, -9);
        game->effect_state = 2;
        break;
    case 0x0B:
        cardgame_skip_if_hand_at_most(game, side, 3, 9);
        game->effect_state = 2;
        break;
    case 0x0C:
        cardgame_skip_if_hand_above(game, side, 3, -9);
        game->effect_state = 2;
        break;
    case 0x10:
        cardgame_skip_if_slots_free(game, side, 5);
        game->effect_state = 2;
        break;
    case 0x0D:
        cardgame_skip_if_hand_above(game, side, 2, 4);
        game->effect_state = 2;
        break;
    case 0x0E:
        cardgame_skip_if_deck_left(game, other, 2);
        game->effect_state = 2;
        break;
    case 0x11:
        game->turns[game->turn - 1].side ^= 1;
        game->effect_state = 2;
        break;
    case 0x12:
    case 0x13:
        if (cardgame_play_update(game, board) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0x14:
        if (cardgame_deal_update(game, board) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0x1B:
        if (cardgame_swap_totals_update(game, board) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0x15:
        if (cardgame_attack_update(game, board, 0) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0x16:
        if (cardgame_attack_update(game, board, 1) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0x17:
        if (cardgame_discard_slots_update(game, board, 0) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0x18:
        if (cardgame_discard_slots_update(game, board, 1) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0x19:
        if (cardgame_combo_update(game, board, 0) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0x1A:
        if (cardgame_combo_update(game, board, 1) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0x1C:
        if (cardgame_clear_slots_update(game, board, 0) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0x1D:
        if (cardgame_clear_slots_update(game, board, 1) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0x1E:
    case 0x1F:
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x27:
    case 0x28:
        if (cardgame_effect_points_update(game, board) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x29:
        if (cardgame_clear_points_update(game, board) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x2A:
        if (cardgame_queue_swap_totals(game, board) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x2B:
    case 0x2D:
        if (cardgame_hide_panels_update(game, board) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x2C:
    case 0x2E:
        if (cardgame_show_panels_update(game, board) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x2F:
        if (side != 0) {
            game->effect_state = 2;
        } else if (cardgame_message_update(game, board) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x30:
        if (cardgame_message_update(game, board) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x31:
        if (side != 0) {
            game->effect_state = 2;
        } else if (cardgame_message_update(game, board) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x34:
        cardgame_mark_selectable_cards(game, board, side, 3, 0xFD);
        game->effect_state = 2;
        break;
    case 0x35:
        cardgame_mark_selectable_cards(game, board, side, 3, 0xFE);
        game->effect_state = 2;
        break;
    case 0x32:
        cardgame_mark_selectable_cards(game, board, side, 3, 0xFF);
        game->effect_state = 2;
        break;
    case 0x33:
        cardgame_mark_selectable_cards(game, board, other, 3, 0xFF);
        game->effect_state = 2;
        break;
    case 0x37:
        cardgame_mark_selectable_cards(game, board, side, 2, 0xFF);
        game->effect_state = 2;
        break;
    case 0x36:
        cardgame_mark_selectable_cards(game, board, other, 2, 0xFF);
        game->effect_state = 2;
        break;
    case 0x38:
        cardgame_mark_selectable_cards(game, board, other, 2, 0x81);
        game->effect_state = 2;
        break;
    case 0x39:
        cardgame_mark_selectable_cards(game, board, other, 2, 0xBF);
        game->effect_state = 2;
        break;
    case 0x3A:
        cardgame_mark_selectable_cards(game, board, side, 4, 0xFF);
        game->effect_state = 2;
        break;
    case 0x3B:
        cardgame_mark_selectable_slots(game, board, side, 0x3FC);
        game->effect_state = 2;
        break;
    case 0x3C:
        cardgame_mark_selectable_slots(game, board, side, 0x2FC);
        game->effect_state = 2;
        break;
    case 0x3D:
        cardgame_mark_selectable_slots(game, board, side, 0x1FC);
        game->effect_state = 2;
        break;
    case 0x3E:
        if (cardgame_discard_card_update(game, board, other, 0) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x3F:
        if (cardgame_discard_card_update(game, board, side, 0) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x40:
        if (cardgame_discard_card_update(game, board, other, 1) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x41:
        if (cardgame_cancel_card_update(game, board) != 0) {
            game->display.cancelled = 1;
            game->effect_state = 2;
        }
        break;
    case 0x42:
        if (cardgame_take_card_update(game, board, side, 3) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x43:
        if (cardgame_take_card_update(game, board, side, 4) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x44:
        if (cardgame_recycle_discard_update(game, board, side) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x45:
    case 0x46:
    case 0x47:
        if (cardgame_mark_draw_update(game, board, side) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x48:
        if (cardgame_draw_update(game, board, side) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x49:
        if (cardgame_discard_hand_update(game, board, side) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x4A:
        for (j = 0; j < game->players[other].hand_count; j++) {
            if (game->selectable[j] != 0) {
                game->choice = j;
                break;
            }
        }
        game->effect_state = 2;
        break;
    case 0x4B:
        cardgame_choose_deck_top(game, board, other);
        game->effect_state = 2;
        break;
    case 0x4C:
        if (cardgame_compact_slots_update(game, board) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x4D:
        if (cardgame_remove_marked_update(game, board, 0) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x4E:
        if (cardgame_remove_marked_update(game, board, 1) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x4F:
    case 0x50:
    case 0x51:
    case 0x52:
    case 0x53:
    case 0x54:
    case 0x55:
    case 0x56:
        if (cardgame_summon_update(game, board, side) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x57:
        if (cardgame_copy_slot_update(game, board, side) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x58:
        if (cardgame_card_effect_update(game, board, 36) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x59:
        if (cardgame_card_effect_update(game, board, 28) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x5A:
        if (cardgame_recount_totals_update(game, board) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x5B:
        cardgame_mark_turn_target(game, board);
        game->effect_state = 2;
        break;
    case 0x5C:
        cardgame_mark_turn_area(game, board);
        game->effect_state = 2;
        break;
    case 0x5D:
        ret = cardgame_sync_slot_values(game, board);
        if (ret == 1) {
            game->effect_state = 2;
        } else if (ret == 2) {
            game->new_effect = 0x4D;
        }
        break;
    case 0x5E:
    case 0x5F:
    case 0x60:
    case 0x61:
    case 0x62:
    case 0x63:
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
        if (cardgame_change_stats_update(game, board) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x6A:
    case 0x6B:
    case 0x6C:
    case 0x6D:
    case 0x6E:
    case 0x6F:
        if (cardgame_effect_wait(game, board) != 0) {
            game->effect_state = 2;
        }
        break;
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
    case 0x76:
    case 0x77:
    case 0x78:
    case 0x79:
    case 0x7A:
    case 0x7B:
    case 0x7C:
    case 0x7D:
    case 0x7E:
        ret = cardgame_ask_use_update(game, board);
        if (ret != -1) {
            game->answer = ret;
            game->effect_state = 0;
        }
        break;
    case 0x7F:
        ret = cardgame_ask_use_update(game, board);
        if (ret != -1) {
            game->answer = ret;
            game->effect_state = 0;
        }
        break;
    case 0x80:
    case 0x81:
    case 0x82:
        ret = cardgame_ask_use_update(game, board);
        if (ret != -1) {
            game->answer = ret;
            game->effect_state = 0;
        }
        break;
    case 0x83:
    case 0x84:
        ret = cardgame_ask_cancel_update(game, board);
        if (ret != -1) {
            game->answer = ret;
            game->effect_state = 0;
        }
        break;
    case 0x87:
        cardgame_mark_selectable_slots(game, board, cur, 0x180);
        game->new_effect = 0x8D;
        break;
    case 0x86:
        cardgame_mark_selectable_slots(game, board, cur, 0x1BC);
        game->new_effect = 0x8D;
        break;
    case 0x85:
        cardgame_mark_selectable_slots(game, board, cur, 0x1FC);
        game->new_effect = 0x8D;
        break;
    case 0x8A:
        cardgame_mark_selectable_slots(game, board, cur, 0x280);
        game->new_effect = 0x8D;
        break;
    case 0x89:
        cardgame_mark_selectable_slots(game, board, cur, 0x2BC);
        game->new_effect = 0x8D;
        break;
    case 0x88:
        cardgame_mark_selectable_slots(game, board, cur, 0x2FC);
        game->new_effect = 0x8D;
        break;
    case 0x8B:
        cardgame_mark_selectable_slots(game, board, cur, 0x3FC);
        game->new_effect = 0x8D;
        break;
    case 0x8C:
        cardgame_mark_selectable_slots(game, board, cur, 0x380);
        game->new_effect = 0x8D;
        break;
    case 0x8D:
        switch (cardgame_choose_slot_update(game, board)) {
        case 1:
            game->answer = 0;
            game->effect_state = 0;
            break;
        case 2:
            k = game->choice;
            if (k < 6) {
                game->turns[game->turn].target = game->slots[0].slots[k].id;
            } else {
                k -= 6;
                game->turns[game->turn].target = game->slots[1].slots[k].id;
            }
            game->answer = 1;
            game->effect_state = 0;
            break;
        }
        break;
    case 0x98:
        switch (cardgame_ask_play_update(game, board)) {
        case 0:
            break;
        case 1:
            game->answer = 1;
            game->effect_state = 0;
            break;
        case 2:
            game->answer = 0;
            game->effect_state = 0;
            break;
        }
        break;
    case 0x99:
        ret = cardgame_hand_update(game, board, &game->players[game->side]);
        if (ret != -1) {
            game->answer = ret;
            game->effect_state = 0;
        }
        break;
    case 0x9A:
        if (cardgame_hand_update(game, board, &game->players[0]) != -1) {
            game->effect_state = 0;
        }
        break;
    case 0x9D:
        if (cardgame_cpu_select_cards(game, board) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0x9B:
    case 0x9C:
        if (cardgame_hand_update(game, board, &game->players[0]) != -1) {
            game->effect_state = 3;
        }
        break;
    case 0xA8:
        if (cardgame_view_board_update(game, board) != 0) {
            game->effect_state = 3;
        }
        break;
    case 0x9E:
        if (cardgame_reveal_update(game, board) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0x9F:
    case 0xA0:
    case 0xA1:
    case 0xA2:
    case 0xA3:
    case 0xA4:
    case 0xA5:
    case 0xA6:
        if (cardgame_banner_update(game, board) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0xA7:
        if (cardgame_choose_first_update(game, board) != 0) {
            game->effect_state = 0;
        }
        break;
    case 0xAC:
        if (side == 0) {
            if (cardgame_choose_card_update(game, board, 2) != 0) {
                game->effect_state = 2;
            }
        } else {
            cardgame_cpu_choose_own_deck_card(game, board);
            game->effect_state = 2;
        }
        break;
    case 0xAD:
        if (side == 0) {
            if (cardgame_choose_card_update(game, board, 0) != 0) {
                game->effect_state = 2;
            }
        } else {
            cardgame_cpu_choose_player_deck_card(game, board);
            game->effect_state = 2;
        }
        break;
    case 0xA9:
        if (side == 0) {
            if (cardgame_choose_card_update(game, board, 0) != 0) {
                game->effect_state = 2;
            }
        } else {
            cardgame_cpu_choose_card_by_value(game, board, 1);
            game->effect_state = 2;
        }
        break;
    case 0xAA:
        if (side == 0) {
            if (cardgame_choose_card_update(game, board, 0) != 0) {
                game->effect_state = 2;
            }
        } else {
            cardgame_cpu_choose_card_by_value(game, board, 0);
            game->effect_state = 2;
        }
        break;
    case 0xAB:
        if (side == 0) {
            if (cardgame_choose_card_update(game, board, 0) != 0) {
                game->effect_state = 2;
            }
        } else {
            cardgame_cpu_choose_card_by_value(game, board, 1);
            game->effect_state = 2;
        }
        break;
    case 0xAE:
        if (side == 0) {
            if (cardgame_choose_slot_update(game, board) != 0) {
                game->effect_state = 2;
            }
        } else {
            cardgame_mark_best_cpu_slot(game);
            game->effect_state = 2;
        }
        break;
    case 0x8E:
    case 0x8F:
    case 0x90:
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
    case 0x95:
        ret = cardgame_ask_area_update(game, board);
        if (ret != -1) {
            game->answer = ret;
            game->effect_state = 0;
        }
        break;
    case 0x96:
        cardgame_mark_selectable_cards(game, board, cur, 2, 0xFD);
        game->new_effect = 0x97;
        break;
    case 0x97:
        switch (cardgame_choose_card_update(game, board, 1)) {
        case 1:
            game->answer = 0;
            game->effect_state = 0;
            break;
        case 2:
            if (cur == 0) {
                game->turns[game->turn].target = game->players[0].hand[game->choice];
            } else {
                game->turns[game->turn].target = game->players[1].hand[game->choice];
            }
            game->answer = 1;
            game->effect_state = 0;
            break;
        }
        break;
    }
}

/* .data (address order) */

CardgameCardData cardgame_card_data[60] = {
    /*  0 */ { 0x91, 10, 0x2F, 4, { CARDGAME_EFFECT_FADE_1, CARDGAME_EFFECT_MARK_TARGET_AREA,
        CARDGAME_EFFECT_DESTROY_ANIM, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS, CARDGAME_EFFECT_CLOSE_SLOT_GAPS } },
    /*  1 */ { 0x70, 0, 0, 9, { CARDGAME_EFFECT_REPEAT_2, CARDGAME_EFFECT_SKIP_5_IF_FREE_SLOTS,
        CARDGAME_EFFECT_MESSAGE_3B_PLAYER, CARDGAME_EFFECT_MARK_SIDE_SLOTS, CARDGAME_EFFECT_CHOOSE_SLOT,
        CARDGAME_EFFECT_DISCARD_MARKED_SLOTS, CARDGAME_EFFECT_CLOSE_SLOT_GAPS, CARDGAME_EFFECT_SUMMON_50,
        CARDGAME_EFFECT_REPEAT_END } },
    /*  2 */ { 0x8E, 10, 0x2F, 1, { CARDGAME_EFFECT_MARK_TARGET_AREA, CARDGAME_EFFECT_ATTACK_HP_PLUS_10 } },
    /*  3 */ { 0x85, 9, 0x2F, 0, { CARDGAME_EFFECT_MARK_TARGET_SLOT, CARDGAME_EFFECT_HP_PLUS_30 } },
    /*  4 */ { 0x73, 0, 0, 9, { CARDGAME_EFFECT_GAIN_POINT_KIND_1 } },
    /*  5 */ { 0x70, 0, 0, 9, { CARDGAME_EFFECT_SKIP_5_IF_FREE_SLOTS, CARDGAME_EFFECT_MESSAGE_3B_PLAYER,
        CARDGAME_EFFECT_MARK_SIDE_SLOTS, CARDGAME_EFFECT_CHOOSE_SLOT, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS,
        CARDGAME_EFFECT_CLOSE_SLOT_GAPS, CARDGAME_EFFECT_SUMMON_51 } },
    /*  6 */ { 0x92, 10, 0x2F, 5, { CARDGAME_EFFECT_FADE_2, CARDGAME_EFFECT_MARK_TARGET_AREA,
        CARDGAME_EFFECT_RETURN_MARKED_SLOTS, CARDGAME_EFFECT_CLOSE_SLOT_GAPS, CARDGAME_EFFECT_REPEAT_2,
        CARDGAME_EFFECT_SKIP_9_IF_HAND_LE_10, CARDGAME_EFFECT_SKIP_2_IF_CPU, CARDGAME_EFFECT_MESSAGE_39_PLAYER,
        CARDGAME_EFFECT_HIDE_PANELS_PLAYER, CARDGAME_EFFECT_MARK_HAND, CARDGAME_EFFECT_CHOOSE_HAND,
        CARDGAME_EFFECT_SKIP_1_IF_CPU, CARDGAME_EFFECT_SHOW_PANELS_PLAYER, CARDGAME_EFFECT_DISCARD_CHOSEN_HAND,
        CARDGAME_EFFECT_BACK_9_IF_HAND_GT_10, CARDGAME_EFFECT_SWAP_SIDE, CARDGAME_EFFECT_REPEAT_END } },
    /*  7 */ { 0x88, 9, 0x2F, 0, { CARDGAME_EFFECT_SKIP_5_IF_FREE_SLOTS, CARDGAME_EFFECT_MESSAGE_3B_PLAYER,
        CARDGAME_EFFECT_MARK_SIDE_SLOTS, CARDGAME_EFFECT_CHOOSE_SLOT, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS,
        CARDGAME_EFFECT_CLOSE_SLOT_GAPS, CARDGAME_EFFECT_MARK_TARGET_SLOT, CARDGAME_EFFECT_COPY_MARKED_SLOT,
        CARDGAME_EFFECT_CLOSE_SLOT_GAPS } },
    /*  8 */ { 0x83, 0, 0, 9, { CARDGAME_EFFECT_CANCEL_PREVIOUS } },
    /*  9 */ { 0x8B, 9, 0x2F, 0, { CARDGAME_EFFECT_MARK_TARGET_SLOT, CARDGAME_EFFECT_RETURN_MARKED_SLOTS,
        CARDGAME_EFFECT_CLOSE_SLOT_GAPS, CARDGAME_EFFECT_REPEAT_2, CARDGAME_EFFECT_SKIP_9_IF_HAND_LE_10,
        CARDGAME_EFFECT_SKIP_2_IF_CPU, CARDGAME_EFFECT_MESSAGE_39_PLAYER, CARDGAME_EFFECT_HIDE_PANELS_PLAYER,
        CARDGAME_EFFECT_MARK_HAND, CARDGAME_EFFECT_CHOOSE_HAND, CARDGAME_EFFECT_SKIP_1_IF_CPU,
        CARDGAME_EFFECT_SHOW_PANELS_PLAYER, CARDGAME_EFFECT_DISCARD_CHOSEN_HAND, CARDGAME_EFFECT_BACK_9_IF_HAND_GT_10,
        CARDGAME_EFFECT_SWAP_SIDE, CARDGAME_EFFECT_REPEAT_END } },
    /* 10 */ { 0x74, 0, 0, 9, { CARDGAME_EFFECT_GAIN_POINT_KIND_2 } },
    /* 11 */ { 0x70, 0, 0, 9, { CARDGAME_EFFECT_SKIP_5_IF_FREE_SLOTS, CARDGAME_EFFECT_MESSAGE_3B_PLAYER,
        CARDGAME_EFFECT_MARK_SIDE_SLOTS, CARDGAME_EFFECT_CHOOSE_SLOT, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS,
        CARDGAME_EFFECT_CLOSE_SLOT_GAPS, CARDGAME_EFFECT_SUMMON_52 } },
    /* 12 */ { 0x93, 10, 0x2F, 6, { CARDGAME_EFFECT_FADE_3, CARDGAME_EFFECT_MARK_TARGET_AREA,
        CARDGAME_EFFECT_ATTACK_HP_PLUS_50 } },
    /* 13 */ { 0x81, 4, 0, 9, { CARDGAME_EFFECT_HIDE_PANELS, CARDGAME_EFFECT_MARK_DISCARD,
        CARDGAME_EFFECT_CHOOSE_DISCARD, CARDGAME_EFFECT_TAKE_CHOSEN_DISCARD, CARDGAME_EFFECT_SHOW_PANELS } },
    /* 14 */ { 0x75, 0, 0, 9, { CARDGAME_EFFECT_GAIN_2_POINTS_KIND_3 } },
    /* 15 */ { 0x85, 9, 0x2F, 0, { CARDGAME_EFFECT_MARK_TARGET_SLOT, CARDGAME_EFFECT_ATTACK_HP_PLUS_20 } },
    /* 16 */ { 0x75, 0, 0, 9, { CARDGAME_EFFECT_GAIN_POINT_KIND_3 } },
    /* 17 */ { 0x70, 0, 0, 9, { CARDGAME_EFFECT_SKIP_5_IF_FREE_SLOTS, CARDGAME_EFFECT_MESSAGE_3B_PLAYER,
        CARDGAME_EFFECT_MARK_SIDE_SLOTS, CARDGAME_EFFECT_CHOOSE_SLOT, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS,
        CARDGAME_EFFECT_CLOSE_SLOT_GAPS, CARDGAME_EFFECT_SUMMON_53 } },
    /* 18 */ { 0x94, 10, 0x2F, 7, { CARDGAME_EFFECT_FADE_4, CARDGAME_EFFECT_MARK_TARGET_AREA,
        CARDGAME_EFFECT_ATTACK_ANIM, CARDGAME_EFFECT_HP_MINUS_60, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS,
        CARDGAME_EFFECT_CLOSE_SLOT_GAPS } },
    /* 19 */ { 0x95, 10, 0x2F, 8, { CARDGAME_EFFECT_MARK_TARGET_AREA, CARDGAME_EFFECT_DESTROY_ANIM,
        CARDGAME_EFFECT_DISCARD_MARKED_SLOTS, CARDGAME_EFFECT_CLOSE_SLOT_GAPS } },
    /* 20 */ { 0x90, 10, 0x2F, 3, { CARDGAME_EFFECT_MARK_TARGET_AREA, CARDGAME_EFFECT_ATTACK_ANIM,
        CARDGAME_EFFECT_HP_MINUS_15, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS, CARDGAME_EFFECT_CLOSE_SLOT_GAPS } },
    /* 21 */ { 0x88, 9, 0x2F, 0, { CARDGAME_EFFECT_MARK_TARGET_SLOT, CARDGAME_EFFECT_ATTACK_ANIM,
        CARDGAME_EFFECT_HP_MINUS_30, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS, CARDGAME_EFFECT_CLOSE_SLOT_GAPS } },
    /* 22 */ { 0x76, 0, 0, 9, { CARDGAME_EFFECT_GAIN_POINT_KIND_4 } },
    /* 23 */ { 0x70, 0, 0, 9, { CARDGAME_EFFECT_SKIP_5_IF_FREE_SLOTS, CARDGAME_EFFECT_MESSAGE_3B_PLAYER,
        CARDGAME_EFFECT_MARK_SIDE_SLOTS, CARDGAME_EFFECT_CHOOSE_SLOT, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS,
        CARDGAME_EFFECT_CLOSE_SLOT_GAPS, CARDGAME_EFFECT_SUMMON_54 } },
    /* 24 */ { 0x7F, 3, 0x3A, 9, { CARDGAME_EFFECT_FADE_5, CARDGAME_EFFECT_MARK_OTHER_HAND_NOT_KIND_5,
        CARDGAME_EFFECT_CHOOSE_FIRST_OTHER_HAND, CARDGAME_EFFECT_DISCARD_CHOSEN_OTHER_HAND,
        CARDGAME_EFFECT_BACK_4_IF_OTHER_HAND_NOT_KIND_5 } },
    /* 25 */ { 0x82, 5, 0x35, 9, { CARDGAME_EFFECT_HIDE_PANELS, CARDGAME_EFFECT_MARK_DECK, CARDGAME_EFFECT_CHOOSE_DECK,
        CARDGAME_EFFECT_TAKE_CHOSEN_DECK, CARDGAME_EFFECT_SHOW_PANELS, CARDGAME_EFFECT_SKIP_9_IF_HAND_LE_10,
        CARDGAME_EFFECT_SKIP_2_IF_CPU, CARDGAME_EFFECT_MESSAGE_39_PLAYER, CARDGAME_EFFECT_HIDE_PANELS_PLAYER,
        CARDGAME_EFFECT_MARK_HAND, CARDGAME_EFFECT_CHOOSE_HAND, CARDGAME_EFFECT_SKIP_1_IF_CPU,
        CARDGAME_EFFECT_SHOW_PANELS_PLAYER, CARDGAME_EFFECT_DISCARD_CHOSEN_HAND,
        CARDGAME_EFFECT_BACK_9_IF_HAND_GT_10 } },
    /* 26 */ { 0x89, 9, 0x2F, 0, { CARDGAME_EFFECT_MARK_TARGET_SLOT, CARDGAME_EFFECT_DESTROY_ANIM,
        CARDGAME_EFFECT_DISCARD_MARKED_SLOTS, CARDGAME_EFFECT_CLOSE_SLOT_GAPS } },
    /* 27 */ { 0x7F, 1, 0x33, 9, { CARDGAME_EFFECT_SKIP_1_IF_CPU, CARDGAME_EFFECT_MESSAGE_39_PLAYER,
        CARDGAME_EFFECT_SKIP_1_IF_CPU, CARDGAME_EFFECT_HIDE_PANELS_PLAYER, CARDGAME_EFFECT_MARK_OTHER_HAND,
        CARDGAME_EFFECT_CHOOSE_OTHER_HAND, CARDGAME_EFFECT_SKIP_1_IF_CPU, CARDGAME_EFFECT_SHOW_PANELS_PLAYER,
        CARDGAME_EFFECT_DISCARD_CHOSEN_OTHER_HAND } },
    /* 28 */ { 0x77, 0, 0, 9, { CARDGAME_EFFECT_GAIN_POINT_KIND_5 } },
    /* 29 */ { 0x70, 0, 0, 9, { CARDGAME_EFFECT_SKIP_5_IF_FREE_SLOTS, CARDGAME_EFFECT_MESSAGE_3B_PLAYER,
        CARDGAME_EFFECT_MARK_SIDE_SLOTS, CARDGAME_EFFECT_CHOOSE_SLOT, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS,
        CARDGAME_EFFECT_CLOSE_SLOT_GAPS, CARDGAME_EFFECT_SUMMON_55 } },
    /* 30 */ { 0x90, 10, 0x2F, 3, { CARDGAME_EFFECT_FADE_6, CARDGAME_EFFECT_MARK_TARGET_AREA,
        CARDGAME_EFFECT_DESTROY_ANIM, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS, CARDGAME_EFFECT_CLOSE_SLOT_GAPS } },
    /* 31 */ { 0x7E, 0, 0, 9, { CARDGAME_EFFECT_FADE_6, CARDGAME_EFFECT_SWAP_TOTALS } },
    /* 32 */ { 0x72, 6, 0x35, 9, { CARDGAME_EFFECT_FADE_6, CARDGAME_EFFECT_REPEAT_3, CARDGAME_EFFECT_SKIP_2_IF_CPU,
        CARDGAME_EFFECT_MESSAGE_39_PLAYER, CARDGAME_EFFECT_HIDE_PANELS_PLAYER, CARDGAME_EFFECT_MARK_OTHER_DECK,
        CARDGAME_EFFECT_CHOOSE_OTHER_DECK, CARDGAME_EFFECT_SKIP_1_IF_CPU, CARDGAME_EFFECT_SHOW_PANELS_PLAYER,
        CARDGAME_EFFECT_DISCARD_CHOSEN_OTHER_DECK, CARDGAME_EFFECT_SKIP_2_IF_OTHER_DECK, CARDGAME_EFFECT_MESSAGE_35,
        CARDGAME_EFFECT_SKIP_2, CARDGAME_EFFECT_REPEAT_END } },
    /* 33 */ { 0x82, 5, 0x35, 9, { CARDGAME_EFFECT_FADE_6, CARDGAME_EFFECT_DISCARD_HAND, CARDGAME_EFFECT_HIDE_PANELS,
        CARDGAME_EFFECT_MARK_DRAW_6, CARDGAME_EFFECT_DRAW_MARKED, CARDGAME_EFFECT_SHOW_PANELS } },
    /* 34 */ { 0x7D, 0, 0, 9, { CARDGAME_EFFECT_FADE_6, CARDGAME_EFFECT_CLEAR_POINTS } },
    /* 35 */ { 0x80, 4, 0, 9, { CARDGAME_EFFECT_SHUFFLE_DISCARD } },
    /* 36 */ { 0x70, 0, 0, 9, { CARDGAME_EFFECT_SKIP_4_IF_HAND_GT_2, CARDGAME_EFFECT_HIDE_PANELS,
        CARDGAME_EFFECT_MARK_DRAW_TO_3, CARDGAME_EFFECT_DRAW_MARKED, CARDGAME_EFFECT_SHOW_PANELS,
        CARDGAME_EFFECT_SKIP_9_IF_HAND_LE_3, CARDGAME_EFFECT_SKIP_2_IF_CPU, CARDGAME_EFFECT_MESSAGE_39_PLAYER,
        CARDGAME_EFFECT_HIDE_PANELS_PLAYER, CARDGAME_EFFECT_MARK_HAND, CARDGAME_EFFECT_CHOOSE_HAND,
        CARDGAME_EFFECT_SKIP_1_IF_CPU, CARDGAME_EFFECT_SHOW_PANELS_PLAYER, CARDGAME_EFFECT_DISCARD_CHOSEN_HAND,
        CARDGAME_EFFECT_BACK_9_IF_HAND_GT_3, CARDGAME_EFFECT_SWAP_SIDE, CARDGAME_EFFECT_SKIP_4_IF_HAND_GT_2,
        CARDGAME_EFFECT_HIDE_PANELS, CARDGAME_EFFECT_MARK_DRAW_TO_3, CARDGAME_EFFECT_DRAW_MARKED,
        CARDGAME_EFFECT_SHOW_PANELS, CARDGAME_EFFECT_SKIP_9_IF_HAND_LE_3, CARDGAME_EFFECT_SKIP_2_IF_CPU,
        CARDGAME_EFFECT_MESSAGE_39_PLAYER, CARDGAME_EFFECT_HIDE_PANELS_PLAYER, CARDGAME_EFFECT_MARK_HAND,
        CARDGAME_EFFECT_CHOOSE_HAND, CARDGAME_EFFECT_SKIP_1_IF_CPU, CARDGAME_EFFECT_SHOW_PANELS_PLAYER,
        CARDGAME_EFFECT_DISCARD_CHOSEN_HAND, CARDGAME_EFFECT_BACK_9_IF_HAND_GT_3, CARDGAME_EFFECT_SWAP_SIDE } },
    /* 37 */ { 0x72, 6, 0x35, 9, { CARDGAME_EFFECT_REPEAT_5, CARDGAME_EFFECT_MARK_OTHER_DECK,
        CARDGAME_EFFECT_CHOOSE_OTHER_DECK_TOP, CARDGAME_EFFECT_DISCARD_CHOSEN_OTHER_DECK,
        CARDGAME_EFFECT_SKIP_2_IF_OTHER_DECK, CARDGAME_EFFECT_MESSAGE_35, CARDGAME_EFFECT_SKIP_2,
        CARDGAME_EFFECT_REPEAT_END } },
    /* 38 */ { 0x8F, 10, 0x2F, 2, { CARDGAME_EFFECT_MARK_TARGET_AREA, CARDGAME_EFFECT_ATTACK_ZERO } },
    /* 39 */ { 0x96, 11, 0x2F, 9, { CARDGAME_EFFECT_SKIP_5_IF_FREE_SLOTS, CARDGAME_EFFECT_MESSAGE_3B_PLAYER,
        CARDGAME_EFFECT_MARK_SIDE_SLOTS, CARDGAME_EFFECT_CHOOSE_SLOT, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS,
        CARDGAME_EFFECT_CLOSE_SLOT_GAPS, CARDGAME_EFFECT_PLACE_TARGET } },
    /* 40 */ { 0x87, 9, 0x2F, 0, { CARDGAME_EFFECT_MARK_TARGET_SLOT, CARDGAME_EFFECT_ATTACK_HP_PLUS_30 } },
    /* 41 */ { 0x82, 5, 0x35, 9, { CARDGAME_EFFECT_HIDE_PANELS, CARDGAME_EFFECT_MARK_DRAW_2,
        CARDGAME_EFFECT_DRAW_MARKED, CARDGAME_EFFECT_SHOW_PANELS, CARDGAME_EFFECT_SKIP_9_IF_HAND_LE_10,
        CARDGAME_EFFECT_SKIP_2_IF_CPU, CARDGAME_EFFECT_MESSAGE_39_PLAYER, CARDGAME_EFFECT_HIDE_PANELS_PLAYER,
        CARDGAME_EFFECT_MARK_HAND, CARDGAME_EFFECT_CHOOSE_HAND, CARDGAME_EFFECT_SKIP_1_IF_CPU,
        CARDGAME_EFFECT_SHOW_PANELS_PLAYER, CARDGAME_EFFECT_DISCARD_CHOSEN_HAND,
        CARDGAME_EFFECT_BACK_9_IF_HAND_GT_10 } },
    /* 42 */ { 0x82, 7, 0x36, 9, { CARDGAME_EFFECT_HIDE_PANELS, CARDGAME_EFFECT_MARK_DECK_DIGIMON,
        CARDGAME_EFFECT_CHOOSE_DECK, CARDGAME_EFFECT_TAKE_CHOSEN_DECK, CARDGAME_EFFECT_SHOW_PANELS,
        CARDGAME_EFFECT_SKIP_9_IF_HAND_LE_10, CARDGAME_EFFECT_SKIP_2_IF_CPU, CARDGAME_EFFECT_MESSAGE_39_PLAYER,
        CARDGAME_EFFECT_HIDE_PANELS_PLAYER, CARDGAME_EFFECT_MARK_HAND, CARDGAME_EFFECT_CHOOSE_HAND,
        CARDGAME_EFFECT_SKIP_1_IF_CPU, CARDGAME_EFFECT_SHOW_PANELS_PLAYER, CARDGAME_EFFECT_DISCARD_CHOSEN_HAND,
        CARDGAME_EFFECT_BACK_9_IF_HAND_GT_10 } },
    /* 43 */ { 0x82, 8, 0x37, 9, { CARDGAME_EFFECT_HIDE_PANELS, CARDGAME_EFFECT_MARK_DECK_OPTIONS,
        CARDGAME_EFFECT_CHOOSE_DECK, CARDGAME_EFFECT_TAKE_CHOSEN_DECK, CARDGAME_EFFECT_SHOW_PANELS,
        CARDGAME_EFFECT_SKIP_9_IF_HAND_LE_10, CARDGAME_EFFECT_SKIP_2_IF_CPU, CARDGAME_EFFECT_MESSAGE_39_PLAYER,
        CARDGAME_EFFECT_HIDE_PANELS_PLAYER, CARDGAME_EFFECT_MARK_HAND, CARDGAME_EFFECT_CHOOSE_HAND,
        CARDGAME_EFFECT_SKIP_1_IF_CPU, CARDGAME_EFFECT_SHOW_PANELS_PLAYER, CARDGAME_EFFECT_DISCARD_CHOSEN_HAND,
        CARDGAME_EFFECT_BACK_9_IF_HAND_GT_10 } },
    /* 44 */ { 0x82, 5, 0x35, 9, { CARDGAME_EFFECT_HIDE_PANELS, CARDGAME_EFFECT_MARK_DECK, CARDGAME_EFFECT_CHOOSE_DECK,
        CARDGAME_EFFECT_TAKE_CHOSEN_DECK, CARDGAME_EFFECT_SHOW_PANELS, CARDGAME_EFFECT_SKIP_2_IF_CPU,
        CARDGAME_EFFECT_MESSAGE_39_PLAYER, CARDGAME_EFFECT_HIDE_PANELS_PLAYER, CARDGAME_EFFECT_MARK_HAND,
        CARDGAME_EFFECT_CHOOSE_HAND, CARDGAME_EFFECT_SKIP_1_IF_CPU, CARDGAME_EFFECT_SHOW_PANELS_PLAYER,
        CARDGAME_EFFECT_DISCARD_CHOSEN_HAND } },
    /* 45 */ { 0x72, 6, 0x35, 9, { CARDGAME_EFFECT_REPEAT_2, CARDGAME_EFFECT_MARK_OTHER_DECK,
        CARDGAME_EFFECT_CHOOSE_OTHER_DECK_TOP, CARDGAME_EFFECT_DISCARD_CHOSEN_OTHER_DECK,
        CARDGAME_EFFECT_SKIP_2_IF_OTHER_DECK, CARDGAME_EFFECT_MESSAGE_35, CARDGAME_EFFECT_SKIP_2,
        CARDGAME_EFFECT_REPEAT_END } },
    /* 46 */ { 0x8A, 9, 0x2F, 0, { CARDGAME_EFFECT_MARK_TARGET_SLOT, CARDGAME_EFFECT_ATTACK_ANIM,
        CARDGAME_EFFECT_HP_MINUS_30, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS, CARDGAME_EFFECT_CLOSE_SLOT_GAPS } },
    /* 47 */ { 0x84, 0, 0, 9, { CARDGAME_EFFECT_CANCEL_PREVIOUS } },
    /* 48 */ { 0x7F, 2, 0x38, 9, { CARDGAME_EFFECT_SKIP_1_IF_CPU, CARDGAME_EFFECT_MESSAGE_39_PLAYER,
        CARDGAME_EFFECT_SKIP_1_IF_CPU, CARDGAME_EFFECT_HIDE_PANELS_PLAYER, CARDGAME_EFFECT_MARK_OTHER_HAND_KIND_6,
        CARDGAME_EFFECT_CHOOSE_OTHER_HAND, CARDGAME_EFFECT_SKIP_1_IF_CPU, CARDGAME_EFFECT_SHOW_PANELS_PLAYER,
        CARDGAME_EFFECT_DISCARD_CHOSEN_OTHER_HAND } },
    /* 49 */ { 0x70, 0, 0, 9, { CARDGAME_EFFECT_REPEAT_2, CARDGAME_EFFECT_SKIP_5_IF_FREE_SLOTS,
        CARDGAME_EFFECT_MESSAGE_3B_PLAYER, CARDGAME_EFFECT_MARK_SIDE_SLOTS, CARDGAME_EFFECT_CHOOSE_SLOT,
        CARDGAME_EFFECT_DISCARD_MARKED_SLOTS, CARDGAME_EFFECT_CLOSE_SLOT_GAPS, CARDGAME_EFFECT_SUMMON_56,
        CARDGAME_EFFECT_REPEAT_END } },
    /* 50 */ { 0x78, 0, 0, 9, { CARDGAME_EFFECT_TAKE_2_POINTS_KIND_1 } },
    /* 51 */ { 0x79, 0, 0, 9, { CARDGAME_EFFECT_TAKE_2_POINTS_KIND_2 } },
    /* 52 */ { 0x7A, 0, 0, 9, { CARDGAME_EFFECT_TAKE_2_POINTS_KIND_3 } },
    /* 53 */ { 0x7B, 0, 0, 9, { CARDGAME_EFFECT_TAKE_2_POINTS_KIND_4 } },
    /* 54 */ { 0x7C, 0, 0, 9, { CARDGAME_EFFECT_TAKE_2_POINTS_KIND_5 } },
    /* 55 */ { 0x8C, 9, 0x2F, 0, { CARDGAME_EFFECT_MARK_TARGET_SLOT, CARDGAME_EFFECT_RETURN_MARKED_SLOTS,
        CARDGAME_EFFECT_CLOSE_SLOT_GAPS, CARDGAME_EFFECT_REPEAT_2, CARDGAME_EFFECT_SKIP_9_IF_HAND_LE_10,
        CARDGAME_EFFECT_SKIP_2_IF_CPU, CARDGAME_EFFECT_MESSAGE_39_PLAYER, CARDGAME_EFFECT_HIDE_PANELS_PLAYER,
        CARDGAME_EFFECT_MARK_HAND, CARDGAME_EFFECT_CHOOSE_HAND, CARDGAME_EFFECT_SKIP_1_IF_CPU,
        CARDGAME_EFFECT_SHOW_PANELS_PLAYER, CARDGAME_EFFECT_DISCARD_CHOSEN_HAND, CARDGAME_EFFECT_BACK_9_IF_HAND_GT_10,
        CARDGAME_EFFECT_SWAP_SIDE, CARDGAME_EFFECT_REPEAT_END } },
    /* 56 */ { 0x8B, 9, 0x2F, 0, { CARDGAME_EFFECT_MARK_TARGET_SLOT, CARDGAME_EFFECT_ATTACK_ANIM,
        CARDGAME_EFFECT_ATTACK_PLUS_10_HP_MINUS_10, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS,
        CARDGAME_EFFECT_CLOSE_SLOT_GAPS } },
    /* 57 */ { 0x8A, 9, 0x2F, 0, { CARDGAME_EFFECT_MARK_TARGET_SLOT, CARDGAME_EFFECT_DISCARD_MARKED_SLOTS,
        CARDGAME_EFFECT_CLOSE_SLOT_GAPS } },
    /* 58 */ { 0x85, 9, 0x2F, 0, { CARDGAME_EFFECT_MARK_TARGET_SLOT, CARDGAME_EFFECT_HP_PLUS_10 } },
    /* 59 */ { 0x85, 9, 0x2F, 0, { CARDGAME_EFFECT_MARK_TARGET_SLOT, CARDGAME_EFFECT_ATTACK_PLUS_10 } },
};
