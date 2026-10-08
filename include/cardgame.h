#ifndef CARDGAME_H
#define CARDGAME_H

/* CARDGAME.PRO: the card battle game. Seven files (config/cardgame.yaml): card data and the effect
 * dispatcher (83E34), the card effects (85DE8), the root object (954F8), the board display (96950), the game
 * flow (9D6E0), a colour fade (A32D8) and the CPU player (A36E4). */

#include "common.h"
#include "object.h"

/* A full-screen colour fade (cardgame_fade_create, size 0x78; cardgame_800A32D8.c): a semi-transparent
 * POLY_F4 whose colour moves to a target in a number of ticks. */
typedef struct CardgameFade {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 state;   /* 0 idle, 1 fading, 2 done (closes the object) */
    /* 0x54 */ s32 timer;   /* ticks left */
    /* 0x58 */ s32 duration;
    /* 0x5C */ u8 abr;      /* semi-transparency mode */
    /* 0x5D */ u8 close;    /* close when the fade ends */
    /* 0x5E */ u8 to[3];    /* target colour */
    /* 0x61 */ u8 from[3];
    /* 0x64 */ u8 color[3]; /* current colour */
    /* 0x68 */ void (*set_color)(struct CardgameFade *obj, u8 r, u8 g, u8 b);  /* cardgame_fade_set_color */
    /* 0x6C */ void (*start)(struct CardgameFade *obj, u8 r, u8 g, u8 b, s32 ticks, s32 close); /* cardgame_fade_start */
    /* 0x70 */ s32 (*is_idle)(struct CardgameFade *obj); /* cardgame_fade_is_idle */
    /* 0x74 */ void (*close_fade)(struct CardgameFade *obj); /* cardgame_fade_close */
} CardgameFade; /* size 0x78 */

/* A short open/close animation (the board's 0xDC0 entries): level goes 0 -> 0x1000 while opening (state 1),
 * stays at 0x1000 (2) and goes back to 0 while closing (3). */
typedef struct CardgameTurnMark {
    /* 0x0 */ s16 scale_x;  /* set to 0x1000 by open/close, never read: the draw passes 0x1000 for x */
    /* 0x2 */ s16 level;    /* 0..0x1000: the y scale */
    /* 0x4 */ s16 timer;    /* ticks left */
    /* 0x6 */ s16 duration;
    /* 0x8 */ s16 state;    /* 0 hidden, 1 opening, 2 open, 3 closing */
    /* 0xA */ s16 side;     /* whose turn it marks: sprite 6 + side */
} CardgameTurnMark; /* size 0xC */

/* One player's side of the board display (CardgameBoard.panels). */
typedef struct CardgameBoardPanel {
    /* 0x00 */ s16 timer;
    /* 0x02 */ s16 duration;
    /* 0x04 */ s16 state;
    /* 0x06 */ s16 event;  /* for one update: 1 the bar closed, 2 it starts opening (open_panels, open_side_panel) */
    /* 0x08 */ s32 ticks;  /* frame counter, wrapped to 16 bits */
    /* 0x0C */ s16 x;      /* the panel's bar (slides in and out with state) */
    /* 0x0E */ s16 y;
    /* 0x10 */ s16 attack; /* the round's totals shown (set_panel_value fields 8, 9) */
    /* 0x12 */ s16 hp;
    /* 0x14 */ u8 unk_14[4];
    /* 0x18 */ u8 counts[7]; /* set_panel_value fields 0-6: [0..4] CardgamePlayer.points, [5] deck_count, [6] hand_count */
    /* 0x1F */ u8 pad_1F;
    /* 0x20 */ s16 discard_x; /* the discard pile's box, with discard_count and lamp 5 */
    /* 0x22 */ s16 discard_y;
    /* 0x24 */ u8 unk_24[4];
    /* 0x28 */ s32 discard_count; /* set_panel_value field 7 */
    /* 0x2C */ u8 unk_2C[4];
    /* 0x30 */ u8 lamps[10]; /* lamps, set from bit pairs by set_lamps */
    /* 0x3A */ u8 unk_3A[2];
    /* 0x3C */ s16 wins_x;  /* the rounds-won counter */
    /* 0x3E */ s16 wins_y;
    /* 0x40 */ u8 unk_40[4];
    /* 0x44 */ s32 wins;   /* rounds won by this side (CardgamePlayer.wins[0]) */
    /* 0x48 */ s16 anim_level;    /* 0x48..0x50: a CardgamePanelAnim (cardgame_board_update_panel_anim) */
    /* 0x4A */ s16 unk_4A;
    /* 0x4C */ s16 anim_timer;
    /* 0x4E */ s16 anim_duration;
    /* 0x50 */ u8 anim_state;
    /* 0x51 */ u8 pad_51[3];
} CardgameBoardPanel; /* size 0x54 */

/* A card on the board display (CardgameBoard.cards). */
typedef struct CardgameBoardCard {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 y;
    /* 0x08 */ s32 target_x;
    /* 0x0C */ s32 target_y;
    /* 0x10 */ s32 start_x;
    /* 0x14 */ s32 start_y;
    /* 0x18 */ s16 scale_x;
    /* 0x1A */ s16 scale_y;
    /* 0x1C */ s16 target_scale_x; /* zoom_card's target scale */
    /* 0x1E */ s16 target_scale_y;
    /* 0x20 */ s16 start_scale_x;  /* and the scale it started from */
    /* 0x22 */ s16 start_scale_y;
    /* 0x24 */ s16 index;
    /* 0x26 */ s16 zooming;  /* set by zoom_card (and by the effects for the card under the cursor): drawn in a pass of
                            * its own, before the others */
    /* 0x28 */ s32 timer;
    /* 0x2C */ s32 duration;
    /* 0x30 */ s32 windup;   /* lunge_card: ticks of growing before it moves (0x10) */
    /* 0x34 */ s32 effect_timer; /* the effect animation's timer (CARDGAME_CARD_TIMER) */
    /* 0x38 */ s16 color;    /* card record byte 0 - 1 (its frame sprite; + 0x3C when dimmed) */
    /* 0x3A */ s16 card;
    /* 0x3C */ s16 is_digimon; /* card record byte 3 is 0x10: attack and hp are drawn */
    /* 0x3E */ u8 turn_marks[3]; /* [i]: turn i's cancel targets this card (sprite 0x37 + i; CardgameTurn.mark) */
    /* 0x41 */ u8 level;     /* card record byte 5 */
    /* 0x42 */ u8 state;
    /* 0x43 */ u8 attack;
    /* 0x44 */ u8 hp;
    /* 0x45 */ u8 style; /* 0: not drawn, 1: face up with its values, 2: face down, 3: an empty place */
    /* 0x46 */ u8 turn_badge; /* a played card's turn + 1 (sprite 0x33 + it), 0: none */
    /* 0x47 */ u8 effect;    /* an effect drawn over it: 1 shake/jolt, 2 boost, 3 attack, 4 destroy */
    /* 0x48 */ u8 highlight; /* frame flags: 1 the cursor (also while lunging), 2 selected, 4 blinking */
    /* 0x49 */ u8 dimmed;    /* drawn shaded (not selectable; CardgameDisplay.dimmed) */
    /* 0x4A */ u8 pad_4A[2];
} CardgameBoardCard; /* size 0x4C */

/* CardgameBoard.marks entries. */
typedef struct CardgameBoardMark {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 start_x;
    /* 0x06 */ s16 start_y;
    /* 0x08 */ s16 target_x; /* move_mark's target */
    /* 0x0A */ s16 target_y;
    /* 0x0C */ s16 blink;    /* palette animation counter */
    /* 0x0E */ u8 timer;
    /* 0x0F */ u8 duration;
    /* 0x10 */ u8 state;
    /* 0x11 */ u8 pad_11;
} CardgameBoardMark; /* size 0x12 */

/* CardgameBoard.dialog. */
typedef struct CardgameDialog {
    /* 0x00 */ s32 position; /* index into cardgame_dialog_origins */
    /* 0x04 */ s32 message;  /* message number (CARDGM) */
    /* 0x08 */ u8 unk_08[4];
    /* 0x0C */ s16 timer;
    /* 0x0E */ s16 duration;
    /* 0x10 */ s16 answer;   /* the yes/no cursor's row */
    /* 0x12 */ u8 unk_12[4];
    /* 0x16 */ u8 state;
    /* 0x17 */ u8 cursor;    /* the yes/no cursor is shown */
    /* 0x18 */ u8 unk_18[4];
} CardgameDialog; /* size 0x1C */

/* CardgameBoard.menu: the in-game menu's window and its cursor (at row cursor). */
typedef struct CardgameMenuWindow {
    /* 0x0 */ s16 timer;
    /* 0x2 */ s16 duration;
    /* 0x4 */ s16 cursor;
    /* 0x6 */ u8 unk_06[4];
    /* 0xA */ s16 state;
} CardgameMenuWindow; /* size 0xC */

/* CardgameBoard.popups entries. */
typedef struct CardgamePopup {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 scale;    /* x scale while opening/closing, 0x1000 open */
    /* 0x06 */ s16 scale_y;  /* set to 0x1000 by open/close, never read: the draw passes 0x1000 for y */
    /* 0x08 */ s16 timer;
    /* 0x0A */ s16 duration;
    /* 0x0C */ s16 kind;     /* index into cardgame_popup_styles */
    /* 0x0E */ s16 show_value; /* kind 3: draw value's count and sprite */
    /* 0x10 */ s32 value;    /* a message number, or for popup 1 a card's level | colour << 4; 500: card_stats */
    /* 0x14 */ u8 card_stats[3]; /* popup 4: attack, hp, 1 if they are shown (a Digimon card) */
    /* 0x17 */ u8 state;
} CardgamePopup; /* size 0x18 */

/* The board display (cardgame_80096950.c; created by cardgame_board_create, size 0xF48): both players'
 * panels, up to 40 cards and its methods. */
typedef struct CardgameBoard {
    /* 0x000 */ Object base;
    /* 0x050 */ s16 *card_ids; /* card IDs (cardgame_board_load_cards fills it) */
    /* 0x054 */ s32 card_flags; /* this frame: 1 a card is animating, 2 a lunging card reached its target */
    /* 0x058 */ s32 ticks; /* ticks */
    /* 0x05C */ s16 opponent;       /* CardgameGame.opponent: the "LV" dialog (message 0x3E) shows its deck's name */
    /* 0x05E */ s16 opponent_level; /* CardgameGame.opponent_level, shown after "LV" */
    /* 0x060 */ CardgameBoardPanel panels[2];
    /* 0x108 */ CardgameBoardCard cards[40];
    /* 0xCE8 */ CardgameBoardMark marks[12];
    /* 0xDC0 */ CardgameTurnMark turn_marks[3];
    /* 0xDE4 */ CardgameDialog dialog;
    /* 0xE00 */ CardgameMenuWindow menu;
    /* 0xE0C */ CardgamePopup popups[6];
    /* 0xE9C */ u8 bg_palette; /* the background's fade-in: palette 0..11, one step per 4 ticks */
    /* 0xE9D */ u8 bg_ticks;
    /* 0xE9E */ u8 bg_state;   /* the background's fade-in: 0 none, 1 running (phase 1 starts it), 2 done */
    /* 0xE9F */ u8 pad_E9F;
    /* 0xEA0 */ void (*set_panel_value)(struct CardgameBoard *obj, s32 side, u32 field, s32 value);
    /* 0xEA4 */ void (*open_turn_mark)(struct CardgameBoard *obj, s32 i, s16 side);
    /* 0xEA8 */ void (*close_turn_mark)(struct CardgameBoard *obj, s32 i);
    /* 0xEAC */ void (*open_popup)(struct CardgameBoard *obj, s32 i, s16 kind, s32 message, s32 x, s32 y);
    /* 0xEB0 */ void (*close_popup)(struct CardgameBoard *obj, s32 i);
    /* 0xEB4 */ void (*clear_lamps)(struct CardgameBoard *obj);
    /* 0xEB8 */ void (*set_lamps)(struct CardgameBoard *obj, s32 flags);
    /* 0xEBC */ void (*close_side_panel)(struct CardgameBoard *obj, s32 side); /* one side's hand row */
    /* 0xEC0 */ void (*open_side_panel)(struct CardgameBoard *obj, s32 side);
    /* 0xEC4 */ void (*close_panels)(struct CardgameBoard *obj);
    /* 0xEC8 */ void (*open_panels)(struct CardgameBoard *obj);
    /* 0xECC */ void (*reset_panels)(struct CardgameBoard *obj);
    /* 0xED0 */ void (*open_panel_anim)(struct CardgameBoard *obj, s32 side);
    /* 0xED4 */ void (*close_panel_anim)(struct CardgameBoard *obj, s32 side);
    /* 0xED8 */ s32 (*get_card_x)(s32 count, s32 i);
    /* 0xEDC */ s32 (*show_mark)(struct CardgameBoard *obj, s32 i, s16 x, s16 y);
    /* 0xEE0 */ s32 (*move_mark)(struct CardgameBoard *obj, s32 i, s8 arg2, s16 arg3, s32 arg4);
    /* 0xEE4 */ void (*open_dialog)(struct CardgameBoard *obj, s32 message, s32 cursor, s32 answer, s32 position); /* callers pass words */
    /* 0xEE8 */ void (*close_dialog)(struct CardgameBoard *obj);
    /* 0xEEC */ void (*confirm_dialog)(struct CardgameBoard *obj);
    /* 0xEF0 */ void (*set_dialog_answer)(struct CardgameBoard *obj, s32 answer); /* callers pass a word */
    /* 0xEF4 */ void (*open_menu)(struct CardgameBoard *obj, s16 row);
    /* 0xEF8 */ void (*close_menu)(struct CardgameBoard *obj);
    /* 0xEFC */ void (*confirm_menu)(struct CardgameBoard *obj);
    /* 0xF00 */ void (*set_menu_cursor)(struct CardgameBoard *obj, s16 row);
    /* 0xF04 */ void (*lay_cards)(struct CardgameBoard *obj, s16 ticks, s16 count, s32 x, s32 y);
    /* 0xF08 */ void (*move_card)(struct CardgameBoard *obj, s32 i, s32 ticks, s32 x, s32 y);
    /* 0xF0C */ void (*slide_card)(struct CardgameBoard *obj, s32 i, s32 ticks, s32 x, s32 y);
    /* 0xF10 */ s32 (*lunge_card)(struct CardgameBoard *obj, s32 i, s32 ticks, s32 x, s32 y); /* attacks (x, y) and comes back */
    /* 0xF14 */ s32 (*place_card)(struct CardgameBoard *obj, s32 i, s32 x, s32 y);
    /* 0xF18 */ s32 (*remove_card)(struct CardgameBoard *obj, s32 i);
    /* 0xF1C */ s32 (*flash_card)(struct CardgameBoard *obj, s32 i); /* a sparkle (11 ticks) */
    /* 0xF20 */ void (*set_card_scale)(struct CardgameBoard *obj, s32 i, s32 scale_x, s32 scale_y);
    /* 0xF24 */ void (*zoom_card)(struct CardgameBoard *obj, s32 i, s32 ticks, s32 scale_x, s32 scale_y);
    /* 0xF28 */ s32 (*flip_card)(struct CardgameBoard *obj, s32 i); /* turns it face up/down */
    /* 0xF2C */ s32 (*shake_card)(struct CardgameBoard *obj, s32 i); /* hit by an attack */
    /* 0xF30 */ s32 (*jolt_card)(struct CardgameBoard *obj, s32 i); /* hit by an effect (damage, discard) */
    /* 0xF34 */ s32 (*boost_card)(struct CardgameBoard *obj, s32 i); /* raised by an effect */
    /* 0xF38 */ s32 (*show_card_effect)(struct CardgameBoard *obj, s32 i, s32 kind); /* 0 attack (36 ticks), 1 destroy (28) */
    /* 0xF3C */ void (*set_card)(struct CardgameBoard *obj, s32 i, s32 card);
    /* 0xF40 */ s32 (*get_card_kind)(struct CardgameBoard *obj, s32 card); /* the card kind (a u8) */
    /* 0xF44 */ s32 (*load_cards)(s16 *ids, s16 *deck1, s16 *deck2);
} CardgameBoard; /* size 0xF48 */

/* A player in the card game (CardgameGame.players): deck, hand and discard pile, the points of each card kind and
 * the round's totals of its slots (cardgame_game_sum_slots). */
typedef struct CardgamePlayer {
    /* 0x00 */ s16 attack; /* sum of its slots' attack */
    /* 0x02 */ s16 hp;     /* sum of its slots' hp, lowered by the other side's attack; the higher wins the round */
    /* 0x04 */ s16 deck_pos; /* next card of deck[] */
    /* 0x06 */ s16 discard_count;
    /* 0x08 */ s16 deck_count;
    /* 0x0A */ s16 hand_count;     /* cards in hand */
    /* 0x0C */ u8 points[5]; /* per card kind 1-5: what cards of that kind cost to play (cardgame_can_play_card) */
    /* 0x11 */ u8 side;
    /* 0x12 */ u8 wins[2]; /* [0]: rounds won (two win the match) */
    /* 0x14 */ s16 deck[40]; /* deck (card indices), shuffled by cardgame_shuffle_deck */
    /* 0x64 */ s16 hand[10]; /* hand (hand_count cards) */
    /* 0x78 */ s16 discard[40]; /* discard_count cards */
} CardgamePlayer; /* size 0xC8 */

/* CardgameGame.cpu_deck_info entries, parallel to the CPU's deck (swapped whole when the deck is sorted). */
typedef struct CardgamePair {
    /* 0x0 */ u8 pos;    /* its position in the deck as dealt (cardgame_restore_cpu_deck sorts back by it) */
    /* 0x1 */ u8 stage;  /* CardgameOpponentCard.stage: drawable from the round's stage round * 2 + 1 / + 2 on; 7: kept
                         * for the end of the deck (cardgame_game_set_cpu_deck_limits) */
} CardgamePair; /* size 0x2 */

/* CardgameGame.cpu_cards entries. */
typedef struct CardgameCardInfo {
    /* 0x0 */ u8 kind;   /* CardgameOpponentCard.kind: the CPU's play class (cardgame_cpu_choose_card) */
    /* 0x1 */ u8 may_counter; /* bit 15 of its CardgameOpponentCard entry: the CPU may answer a played class 3/9 card
                            * with it (cardgame_cpu_choose_card) */
    /* 0x2 */ s16 order; /* sort key of the CPU's hand: a rank per kind * 100 + deck position */
} CardgameCardInfo; /* size 0x4 */

/* A card played in a round's turns (CardgameGame.turns, up to 3 per round; indexed by turn - 1). */
typedef struct CardgameTurn {
    /* 0x0 */ s16 card;
    /* 0x2 */ s16 mark;  /* a cancel card (effects 0x83/0x84) played on the next turn targets it: the board cards'
                         * turn_marks index; 0: none */
    /* 0x4 */ u8 side;
    /* 0x5 */ u8 target_kind; /* the card's data field 3: 0 one slot (target), 1-3 a side or both, 4-8 by card kind */
    /* 0x6 */ u8 target;
    /* 0x7 */ u8 pad_07;
} CardgameTurn; /* size 0x8 */

/* A card placed on the table (CardgameGame.slots, cardgame_game_set_slot): its attack and hp (from the card's picture
 * data, raised by bonus cards: cardgame_game_apply_bonus). */
typedef struct CardgameSlot {
    /* 0x0 */ s16 card;
    /* 0x2 */ s16 attack_bonus;
    /* 0x4 */ s16 hp_bonus;
    /* 0x6 */ s16 attack;
    /* 0x8 */ s16 hp;
    /* 0xA */ u8 owner;
    /* 0xB */ u8 side;
    /* 0xC */ u8 id; /* unique (CardgameGame.next_slot_id), a turn's target */
    /* 0xD */ u8 pad_0D;
} CardgameSlot; /* size 0xE */

/* CardgameGame.slots. */
typedef struct CardgameSlots {
    /* 0x00 */ u8 count;
    /* 0x01 */ u8 pad_01;
    /* 0x02 */ CardgameSlot slots[8];
} CardgameSlots; /* size 0x72 */

/* CardgameGame.display: the board's card animations (cardgame_game_update_display: request -> state 1-18), and the
 * effect resolution's state (resolve_step..; cardgame_game_resolve_effect). cardgame_game_count_cards and
 * cardgame_game_update_dimmed take its address. */
typedef struct CardgameDisplay {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 request;
    /* 0x02 */ u8 unk_02;
    /* 0x03 */ u8 reopen;   /* the request that shows the current view again (request + 1; 17 -> 6, 18 -> 12) */
    /* 0x04 */ u8 face_down; /* the next view's cards are shown face down */
    /* 0x05 */ u8 set_dimmed; /* cardgame_game_update_dimmed: 1 undims, 2 dims cards 0-14 */
    /* 0x06 */ u8 dimmed[40];
    /* 0x2E */ u8 pad_2E[2];
    /* 0x30 */ s32 time;
    /* 0x34 */ s32 shown;
    /* 0x38 */ s32 step_time;
    /* 0x3C */ s32 count;
    /* 0x40 */ s32 duration;
    /* 0x44 */ u8 resolve_step; /* step of cardgame_game_resolve_effect */
    /* 0x45 */ u8 cancelled; /* the card cancelled the one before it (effect 0x41): two turns are taken off */
    /* 0x46 */ u8 resolve_count; /* the turns played as the resolution starts (CardgameGame.turn); never read */
    /* 0x47 */ u8 pad_47;
    /* 0x48 */ s16 script_pos;   /* next entry of the played card's script (CardgameCardData.script) */
    /* 0x4A */ s16 repeat_count; /* script effects 3-6: repeat */
    /* 0x4C */ s16 repeat_pos;
    /* 0x4E */ u8 pad_4E[2];
    /* 0x50 */ s32 substep;  /* the game phases' sub-step (cardgame_game_close_panels, ...) */
} CardgameDisplay; /* size 0x54 */

/* CardgameGame.effect (0x420): the effect running and its state. The in-game menu copies it whole
 * (CardgameGameMenu.saved) while it runs, so it was one struct in the original too. */
typedef struct CardgameEffectState {
    /* 0x00 */ u8 id; /* the effect running (cardgame_run_effect) */
    /* 0x01 */ u8 new_id; /* the effect to start */
    /* 0x02 */ u8 step;
    /* 0x03 */ u8 next_step; /* step to take on the next update (0: none) */
    /* 0x04 */ s32 time; /* the step's frame count (cardgame_effect_wait counts it down) or sub-state */
    /* 0x08 */ s32 vars[5]; /* values each effect uses its own way */
    /* 0x1C */ s32 cursor; /* card under the cursor */
    /* 0x20 */ s32 choice; /* an effect's result */
    /* 0x24 */ u8 selected; /* hand cards selected (at most 6) */
    /* 0x25 */ u8 target_rows; /* rows with selectable targets: 1 slots[0], 2 slots[1] (some effects reuse it) */
    /* 0x26 */ s8 selectable[41]; /* per card: may be selected (the CPU: candidate targets) */
    /* 0x4F */ s8 marked[41]; /* per card: selected, or a target */
} CardgameEffectState; /* size 0x78 */

/* CardgameGame.menu (0x4EC): the in-game menu (cardgame_game_run_menu). */
typedef struct CardgameGameMenu {
    /* 0x00 */ s32 timer;
    /* 0x04 */ CardgameEffectState saved; /* CardgameGame.effect while the menu runs */
    /* 0x7C */ u8 state;
    /* 0x7D */ u8 sel;
} CardgameGameMenu; /* size 0x80 with padding */

/* The card game (cardgame_game_create, size 0x824; its update is cardgame_game_update). The card effects
 * take it as their first argument and the board display as their second. A match: choose the deck (phase 2), decide
 * who starts (3), deal (4), then per round the first play phase (5), the placement of the slots (6), the second play
 * phase (7) and the round's end (8); cardgame_game_next_phase runs a phase's banner effect (phase_next) between
 * phases. Two rounds won win the match. */
typedef struct CardgameGame {
    /* 0x000 */ Object base;
    /* 0x050 */ s16 card_ids[189]; /* card IDs, given to the board display */
    /* 0x1CA */ u8 unk_1CA[0x244 - 0x1CA];
    /* 0x244 */ u8 card_count; /* number of cards in card_ids (the board's load_cards result) */
    /* 0x245 */ u8 unk_245[3];
    /* 0x248 */ s16 opp_deck[40]; /* the opponent's deck (card IDs) */
    /* 0x298 */ s16 deck[40];     /* the player's deck (card IDs) */
    /* 0x2E8 */ s8 opponent; /* record of file 0x7A4, from 1 */
    /* 0x2E9 */ s8 opponent_level; /* CardgameOpponent.level */
    /* 0x2EA */ s16 deck_sel; /* the player's saved deck chosen (gamestate_data.decks) */
    /* 0x2EC */ s32 prize; /* the item won (cardgame_prize_items) */
    /* 0x2F0 */ struct CardgameOpponent *opponents; /* file 0x7A4 (cardgame_8009D6E0.c) */
    /* 0x2F4 */ u8 effect_state; /* 0: the phases run, 1: an effect runs, 2: its result is resolved, 3: the in-game menu */
    /* 0x2F5 */ u8 first_side;
    /* 0x2F6 */ u8 prev_phase;
    /* 0x2F7 */ u8 next_phase; /* 0: none */
    /* 0x2F8 */ u8 phase;
    /* 0x2F9 */ u8 step; /* of the phase */
    /* 0x2FA */ u8 pad_2FA[2];
    /* 0x2FC */ s32 timer;
    /* 0x300 */ u8 round;
    /* 0x301 */ u8 round_winner;
    /* 0x302 */ u8 winner; /* of the match: 0 the player */
    /* 0x303 */ u8 done; /* 1: fading out, 2: over (the root object ends the overlay) */
    /* 0x304 */ u8 swap_card;  /* card + 1 shown when the round's totals are swapped (effects 0x2A, 0x1B) */
    /* 0x305 */ u8 swap_count; /* swaps queued for the round's end */
    /* 0x306 */ u8 fade; /* a fade to start, cardgame_fade_colors[fade - 1] */
    /* 0x307 */ u8 unk_307;
    /* 0x308 */ u8 next_slot_id;
    /* 0x309 */ u8 unk_309;
    /* 0x30A */ CardgamePair cpu_deck_info[40];
    /* 0x35A */ u8 unk_35A[2];
    /* 0x35C */ CardgameCardInfo cpu_cards[41]; /* indexed by card - 40 */
    /* 0x400 */ u8 cpu_counter_ids[0x41B - 0x400]; /* card IDs the CPU answers with a kind-4 card when the player plays
                                                  * one (CardgameOpponent.counter_ids - 1, or a default list); 0xFF ends */
    /* 0x41B */ u8 cpu_deck_end;  /* the CPU may take deck cards up to here (stage <= round * 2 + 2) */
    /* 0x41C */ u8 cpu_deck_last; /* the end of its cards before the stage-7 ones */
    /* 0x41D */ u8 unk_41D[3];
    /* 0x420 */ CardgameEffectState effect;
    /* 0x498 */ CardgameDisplay display;
    /* 0x4EC */ CardgameGameMenu menu;
    /* 0x56C */ u8 unk_56C[0x574 - 0x56C];
    /* 0x574 */ u8 turn_state; /* cardgame_game_run_turns: 2 the player plays, 3 the CPU, 12-15 the end */
    /* 0x575 */ s8 turn; /* cards played this round, 0..3 */
    /* 0x576 */ u8 turn_cycles; /* rounds of turns played (counted when turn goes back to 0); never read */
    /* 0x577 */ u8 passes; /* sides that played nothing (2: the play phase ends) */
    /* 0x578 */ u8 round_first; /* the side that plays first */
    /* 0x579 */ u8 side; /* the side playing */
    /* 0x57A */ s8 answer; /* -1: no answer yet */
    /* 0x57B */ u8 pad_57B;
    /* 0x57C */ s32 pass_timer;
    /* 0x580 */ CardgameTurn turns[3]; /* indexed by turn - 1 */
    /* 0x598 */ u8 unk_598[4];
    /* 0x59C */ CardgamePlayer players[2];
    /* 0x72C */ CardgameSlots slots[2]; /* per side: the cards placed on the table */
    /* 0x810 */ void (*shuffle_deck)(struct CardgameGame *obj, s32 start, s32 count);
    /* 0x814 */ void (*sort_cards)(struct CardgameGame *obj, s16 *cards, s32 range, s32 flags);
    /* 0x818 */ void (*place_cards)(struct CardgameGame *obj, s32 side);
    /* 0x81C */ void (*add_slot)(struct CardgameGame *obj, s32 side, s32 card);
    /* 0x820 */ s32 (*get_score)(struct CardgameGame *obj, s32 side, s32 mask);
} CardgameGame; /* size 0x824 */

/* CardgameGameData.deck_choice (cardgame_game_choose_deck): the cursor and the three deck windows. */
typedef struct CardgameDeckChoice {
    /* 0x0 */ struct CardgameIcon *icon;
    /* 0x4 */ struct CardgameDeckWindow *windows[3];
} CardgameDeckChoice; /* size 0x10 */

/* The card game's data block (CardgameGame's base.children, 0x1C bytes). */
typedef struct CardgameGameData {
    /* 0x00 */ struct CardgameLoader *loader;
    /* 0x04 */ CardgameFade *fade;
    /* 0x08 */ CardgameDeckChoice deck_choice;
    /* 0x18 */ CardgameBoard *board;
} CardgameGameData; /* size 0x1C */

CardgameGame *cardgame_game_create(s32 opponent);
void cardgame_run_effect(CardgameGame *game, CardgameBoard *board);

CardgameBoard *cardgame_board_create(s16 *ids);

/* A blinking icon (cardgame_icon_create, size 0x74; cardgame_800954F8.c): zooms in, animates, zooms out. */
typedef struct CardgameIcon {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 ticks;
    /* 0x54 */ s16 x;
    /* 0x56 */ s16 y;
    /* 0x58 */ s16 scale_x;
    /* 0x5A */ s16 scale_y;
    /* 0x5C */ u8 unk_5C[6];
    /* 0x62 */ u8 state;    /* 0 opening, 1 open, 2 closing */
    /* 0x63 */ u8 play_once;   /* play the 8-frame animation once instead of looping 16 frames */
    /* 0x64 */ s16 timer;
    /* 0x66 */ s16 duration;
    /* 0x68 */ void (*set_pos)(struct CardgameIcon *obj, s16 x, s16 y); /* cardgame_icon_set_pos */
    /* 0x6C */ void (*close_icon)(struct CardgameIcon *obj);         /* cardgame_icon_close */
    /* 0x70 */ void (*play)(struct CardgameIcon *obj);               /* cardgame_icon_play */
} CardgameIcon; /* size 0x74 */

/* A deck's window (cardgame_deck_window_create, size 0x74; cardgame_800954F8.c): its name and how many cards of
 * each of the 6 kinds it has. */
typedef struct CardgameDeckWindow {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 deck;    /* gamestate_data.decks[deck] */
    /* 0x54 */ s32 ticks;
    /* 0x58 */ s16 x;
    /* 0x5A */ s16 y;
    /* 0x5C */ s16 scale_x;
    /* 0x5E */ s16 scale_y;
    /* 0x60 */ u8 counts[6];
    /* 0x66 */ u8 state;    /* 0 opening, 1 open, 2 closing */
    /* 0x67 */ u8 playing;  /* the cursor animation runs (cardgame_deck_window_play) */
    /* 0x68 */ s16 timer;
    /* 0x6A */ s16 duration;
    /* 0x6C */ void (*play)(struct CardgameDeckWindow *obj);        /* cardgame_deck_window_play */
    /* 0x70 */ void (*close_window)(struct CardgameDeckWindow *obj); /* cardgame_deck_window_close */
} CardgameDeckWindow; /* size 0x74 */

/* Loads the game's files one by one (cardgame_loader_create, size 0x58; cardgame_80096950.c). */
typedef struct CardgameLoader {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 ready;  /* passed the list's -2 marker */
    /* 0x54 */ s16 index;  /* in cardgame_loader_files */
    /* 0x56 */ s16 file;   /* being loaded */
} CardgameLoader; /* size 0x58 */

CardgameLoader *cardgame_loader_create(void);
CardgameIcon *cardgame_icon_create(s16 x, s16 y);
CardgameDeckWindow *cardgame_deck_window_create(s32 deck, s32 x, s32 y);

CardgameFade *cardgame_fade_create(u8 abr);

/* card-board */
/* A slot's position and card ID, sorted by ID (cardgame_game_find_combo, cardgame_cpu_get_score). */
typedef struct CardgameSortEntry {
    /* 0x0 */ s16 index; /* in CardgameSlots.slots */
    /* 0x2 */ s16 id;    /* CardgameGame.card_ids[card] */
} CardgameSortEntry; /* size 0x4 */
/* end card-board */

/* Where each side's slots go on the board (16.8 fixed point), [screen mode (main_screen_pos)][side]: the dealt
 * cards (cardgame_80085DE8.c) and the slots (cardgame_8009D6E0.c). Defined in cardgame_80096950.c (the
 * first of its .data). */
typedef struct CardgameSlotOrigin {
    /* 0x0 */ s32 x;
    /* 0x4 */ s32 y;
} CardgameSlotOrigin;

extern CardgameSlotOrigin cardgame_slot_origins[2][2];

#endif /* CARDGAME_H */
