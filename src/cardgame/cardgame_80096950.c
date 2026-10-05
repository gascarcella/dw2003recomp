#include "common.h"
#include "object.h"
#include "gfx.h"
#include "heap.h"
#include "sound.h"
#include "message.h"
#include "cdload.h"
#include "records.h"
#include "psyq/libgte.h"
#include "pad.h"
#include "cardgame.h"

/* The board display (CardgameBoard): both players' panels, the cards on the table and the effects drawn over
 * them; the game flow drives it through its methods (set_panel_value..load_cards). */

/* The board's data block: a message driver and its windows. */
typedef struct CardgameBoardData {
    /* 0x00 */ MessageCursor *cursor;     /* the dialog's yes/no cursor */
    /* 0x04 */ MessageWindow *numbers[3]; /* numbers (wins, a card's attack/hp, the level) */
    /* 0x10 */ MessageWindow *pass_texts[2]; /* per side: "Pass" (message 0x3F) on the panel animation */
    /* 0x18 */ MessageWindow *texts[8];   /* the popups' and the dialog's texts */
} CardgameBoardData; /* size 0x38 */

/* The open/close animation at CardgameBoardPanel.anim_level (the panel's fields 0x48..0x50). */
typedef struct CardgamePanelAnim {
    /* 0x0 */ s16 level; /* 0..0x1000 */
    /* 0x2 */ s16 unk_02;
    /* 0x4 */ s16 timer;
    /* 0x6 */ s16 duration;
    /* 0x8 */ u8 state;  /* 0 hidden, 1 opening, 2 open, 3 closing */
} CardgamePanelAnim;

/* A number drawn digit by digit (cardgame_board_draw_number); callers build it on the stack. */
typedef struct CardgameNumber {
    /* 0x00 */ s16 value;
    /* 0x02 */ s16 x;
    /* 0x04 */ s16 y;
    /* 0x06 */ s16 scale_x;
    /* 0x08 */ s16 scale_y;
    /* 0x0A */ s16 pivot_x;
    /* 0x0C */ s16 pivot_y;
    /* 0x0E */ u8 digits;
    /* 0x0F */ u8 zero_pad; /* draw leading zeros */
    /* 0x10 */ u8 layer;
} CardgameNumber; /* size 0x12 */

/* Where a panel's icon and window go, relative to the panel (per side), for PAL and NTSC (main_screen_pos). */
typedef struct CardgamePanelLayout {
    /* 0x0 */ s16 icon[2][2];
    /* 0x8 */ s16 window[2][2];
} CardgamePanelLayout; /* size 0x10 */

extern CardgamePanelLayout cardgame_panel_layouts[2];
extern u8 cardgame_card_highlight_palettes[6]; /* the blinking frame's palette cycle (CardgameBoardCard.highlight) */
extern s16 cardgame_turn_mark_pos[3][2];       /* where the three turn marks (CardgameBoard.turn_marks) are drawn */

/* How each kind of CardgamePopup entry (kind) is drawn. */
typedef struct CardgamePopupStyle {
    /* 0x0 */ s16 x;     /* pivot, relative to the entry */
    /* 0x2 */ s16 y;
    /* 0x4 */ u8 frame;
    /* 0x5 */ u8 bank;   /* 2: bank 0x025D0003, else 0x025D0002 */
} CardgamePopupStyle; /* size 0x6 */

extern CardgamePopupStyle cardgame_popup_styles[];
/* A point in pixels. */
typedef struct CardgamePoint {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
} CardgamePoint;
/* How a side's panel is drawn (cardgame_board_draw_panel): sprite frames and where each part goes, relative to the
 * panel. */
typedef struct CardgamePanelArt {
    /* 0x00 */ u8 unk_00;
    /* 0x01 */ u8 frame_01; /* the panel */
    /* 0x02 */ u8 frame_02; /* plus the rounds won */
    /* 0x03 */ u8 frame_03;
    /* 0x04 */ CardgamePoint pos[18];
} CardgamePanelArt; /* size 0x4C */
extern CardgamePanelArt cardgame_panel_art[2];
extern u8 cardgame_panel_icon_palettes[6];
extern u8 cardgame_panel_lamp_frames[][2]; /* per records_language */
/* A panel part's frame offsets while its lamp is lit (cycling every 6 ticks): lamps 7, 6, 5 (the discard box), the
 * five kinds' sprites (cardgame_panel_kind_sprites[kind] + cardgame_panel_kind_anim[frame]). */
extern u8 cardgame_panel_lamp7_anim[5];
extern u8 cardgame_panel_lamp6_anim[5];
extern u8 cardgame_panel_discard_anim[5];
extern u8 cardgame_panel_kind_sprites[5];
extern u8 cardgame_panel_kind_anim[5];
/* Where the dialog (CardgameBoard.dialog) opens, per position (dialog.position). */
typedef struct CardgameDialogOrigin {
    /* 0x0 */ s32 x;
    /* 0x4 */ s32 y;
} CardgameDialogOrigin;
extern CardgameDialogOrigin cardgame_dialog_origins[];
extern s16 cardgame_popup_text_x[]; /* x of a popup's text, by cardgame_board_show_popup_text's i */
/* x offsets of a card (cardgame_board_update_card_jolt, 8009A62C), indexed with & 7: entries 4..7 read past the end,
 * into the next table. */
extern s16 cardgame_card_jolt_offsets[4];
extern s16 cardgame_card_shake_offsets[4];
extern s16 cardgame_extra_cards[];
extern u8 cardgame_card_shake_frames[4]; /* CardgameBoardCard.effect 1's frames */
extern u8 cardgame_card_boost_frames[4]; /* effect 2's */

/* The card's effect timer (CardgameBoardCard.effect_timer). */
#define CARDGAME_CARD_TIMER(card) ((card)->effect_timer)
extern s16 cardgame_fixed_cards[10]; /* nine more card IDs loaded after the two decks (board cards 80-88); [9] (0xA0D)
                                      * is never read */
/* A sprite bank (sprite.c's format) whose cell and frame records are rewritten to show one card's picture. */
extern s32 cardgame_card_sheet[];

/* The files CardgameLoader loads: -2 marks where the game can start, -1 ends the list. */
typedef struct CardgameFileEntry {
    s16 id;
    s16 localized; /* add the language (records_language) to the ID */
} CardgameFileEntry;

extern CardgameFileEntry cardgame_loader_files[];

void cardgame_board_update_turn_mark(CardgameBoard *obj, CardgameBoardData *data, s32 i, CardgameTurnMark *anim);
void cardgame_board_draw_background(CardgameBoard *obj);
void cardgame_board_update_dialog(CardgameBoard *obj, CardgameBoardData *data);
void cardgame_board_update_menu(CardgameBoard *obj, CardgameBoardData *data);
void cardgame_board_update_cards(CardgameBoard *obj, CardgameBoardData *data);
void cardgame_board_update_popup(CardgameBoard *obj, CardgameBoardData *data, CardgamePopup *e, s32 i);
void cardgame_board_draw_popup(CardgameBoard *obj, CardgameBoardData *data, CardgamePopup *e);
void cardgame_board_update(CardgameBoard *obj, CardgameBoardData *data);
s32 cardgame_board_load_cards(s16 *ids, s16 *deck1, s16 *deck2);
s32 cardgame_board_get_extra_card(s32 i);
void cardgame_board_open_popup(CardgameBoard *obj, s32 i, s16 kind, s32 message, s32 x, s32 y);
void cardgame_board_reset_panels(CardgameBoard *obj);
void cardgame_board_open_panels(CardgameBoard *obj);
void cardgame_board_close_panels(CardgameBoard *obj);
void cardgame_board_draw_panel(CardgameBoardPanel *panel, CardgameBoardData *data, s32 side);
void cardgame_board_draw_panel_values(CardgameBoardPanel *panel, CardgameBoardData *data, s32 side);
void cardgame_board_update_panel_anim(CardgameBoard *obj, CardgameBoardData *data, s32 side, CardgamePanelAnim *anim);
void cardgame_board_draw_panel_anim(CardgameBoard *obj, CardgameBoardData *data, s32 side, CardgamePanelAnim *anim);
void cardgame_board_update_mark_motion(CardgameBoardMark *mark);
void cardgame_board_draw_mark(CardgameBoardMark *mark);
void cardgame_board_draw_card_effect(CardgameBoard *obj, CardgameBoardCard *card);
s32 cardgame_board_update_card_lunge(CardgameBoard *obj, CardgameBoardCard *card);
void cardgame_board_draw_card_highlight(CardgameBoard *obj, CardgameBoardCard *card);
void cardgame_board_draw_card_badge(CardgameBoard *obj, CardgameBoardCard *card);
void cardgame_board_draw_card_turn_marks(CardgameBoard *obj, CardgameBoardCard *card);
void cardgame_board_draw_card(CardgameBoard *obj, CardgameBoardCard *card);
void cardgame_board_draw_card_shade(CardgameBoard *obj, CardgameBoardCard *card);
void cardgame_board_draw_card_flash(CardgameBoard *obj, CardgameBoardCard *card);
void cardgame_board_update_card(CardgameBoard *obj, CardgameBoardData *data, CardgameBoardCard *card);

void cardgame_board_draw_background(CardgameBoard *obj) {
    Sprite spr;
    s32 frame;
    s32 x;

    sprite_init(&spr);
    spr.set_layer_id(0x100, 2);
    frame = 0;
    spr.set_vram_pos(0x340, 0);
    switch (obj->bg_state) {
    case 0:
        break;
    case 1:
        if (obj->bg_ticks >= 4) {
            obj->bg_ticks -= 4;
            if (++obj->bg_palette >= 11) {
                obj->bg_palette = 11;
                obj->bg_state = 2;
            }
        }
        frame = obj->bg_palette;
        obj->bg_ticks += gfx_module.funcs.get_frame_ticks();
        break;
    case 2:
        frame = 11;
        break;
    }
    spr.set_palette(frame);
    x = ((u32)obj->ticks >> 1) & 0x3F;
    spr.draw(cdload_module.get_subfile_by_id(0x025D0003), 0, x, x);
}

void cardgame_board_draw_number(CardgameNumber *num, s32 transform) {
    Sprite spr;
    s32 value;
    s32 i;
    s32 digit;

    sprite_init(&spr);
    spr.set_layer_id(0x100, num->layer);
    spr.set_vram_pos(0x340, 0);
    if (transform) {
        spr.set_pivot(num->pivot_x, num->pivot_y);
        spr.set_scale(num->scale_x, num->scale_y, 0x1000);
    }
    value = num->value;
    for (i = 0; i < num->digits; i++) {
        digit = value % 10;
        if (i == 0 || num->zero_pad != 0 || digit != 0 || value / 10 != 0) {
            spr.draw(cdload_module.get_subfile_by_id(0x025D0003), digit + 20, num->x + (num->digits - 1 - i) * 7, num->y);
        }
        value /= 10;
    }
}

void cardgame_board_draw_turn_mark(CardgameBoard *obj, CardgameBoardData *data, s32 i, CardgameTurnMark *anim) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(0x100, 1);
    spr.set_vram_pos(0x340, 0);
    spr.set_pivot(cardgame_turn_mark_pos[i][0] + 4, cardgame_turn_mark_pos[i][1] + 23);
    spr.set_scale(0x1000, anim->level, 0x1000);
    spr.draw(cdload_module.get_subfile_by_id(0x025D0003), anim->side + 6, cardgame_turn_mark_pos[i][0], cardgame_turn_mark_pos[i][1]);
}

void cardgame_board_update_turn_mark(CardgameBoard *obj, CardgameBoardData *data, s32 i, CardgameTurnMark *anim) {
    if (anim->state != 0) {
        switch (anim->state) {
        case 1:
        default:
            anim->level = 0x1000 - (anim->timer << 12) / anim->duration;
            anim->timer -= gfx_module.funcs.get_frame_ticks();
            if (anim->timer <= 0) {
                anim->state = 2;
            }
            break;
        case 2:
            anim->level = 0x1000;
            break;
        case 3:
            anim->timer -= gfx_module.funcs.get_frame_ticks();
            if (anim->timer <= 0) {
                anim->state = 0;
            }
            anim->level = (anim->timer << 12) / anim->duration;
            break;
        }
        cardgame_board_draw_turn_mark(obj, data, i, anim);
    }
}

void cardgame_board_update_turn_marks(CardgameBoard *obj, CardgameBoardData *data) {
    s32 i;

    for (i = 0; i < 3; i++) {
        cardgame_board_update_turn_mark(obj, data, i, &obj->turn_marks[i]);
    }
}

/* Draws a sprite of panel `e` (frame `frame` of image `image`, VRAM x `vram_x`) at (px, py), scaled
 * about the panel's pivot when the panel is scaled. A block of its own, so that GCC 2.8 shares its
 * stack slot with the caller's other nested blocks (cardgame_board_draw_popup's frame, 0xD8). */
#define CARDGAME_PANEL_SPRITE(e, vram_x, image, frame, px, py)                \
    {                                                                         \
        Sprite spr;                                                           \
                                                                              \
        sprite_init(&spr);                                                    \
        spr.set_layer_id(0x100, 1);                                           \
        spr.set_vram_pos(vram_x, 0);                                          \
        if ((e)->scale != 0x1000) {                                          \
            spr.set_pivot((e)->x + cardgame_popup_styles[(e)->kind].x,      \
                          (e)->y + cardgame_popup_styles[(e)->kind].y);     \
            spr.set_scale((e)->scale, 0x1000, 0x1000);                       \
        }                                                                     \
        spr.draw(cdload_module.get_subfile_by_id(image), frame, px, py);      \
    }

/* Draws panel `e`'s icon; a kind-3 panel in state 2 with show_value set also gets a count (value & 0xF)
 * and a sprite (frame by value >> 4). Its blocks are nested so that they share stack slots (0xD8). */
void cardgame_board_draw_popup(CardgameBoard *obj, CardgameBoardData *data, CardgamePopup *e) {
    s32 dx = 0;

    if (e->kind == 2 && e->card_stats[2] == 1) {
        dx = 0x45;
    }
    if (e->state == 2 && e->kind == 3 && e->show_value != 0) {
        {
            CardgameNumber num;

            num.layer = 1;
            num.digits = 2;
            num.zero_pad = 0;
            num.x = e->x + 0x18;
            num.y = e->y + 4;
            num.value = e->value & 0xF;
            num.pivot_x = e->x + cardgame_popup_styles[e->kind].x;
            num.pivot_y = e->y + cardgame_popup_styles[e->kind].y;
            num.scale_x = e->scale;
            num.scale_y = 0x1000;
            cardgame_board_draw_number(&num, 1);
        }
        {
            Sprite spr;

            sprite_init(&spr);
            spr.set_layer_id(0x100, 1);
            spr.set_vram_pos(0x280, 0);
            spr.set_pivot(e->x + cardgame_popup_styles[e->kind].x, e->y + cardgame_popup_styles[e->kind].y);
            spr.set_scale(e->scale, 0x1000, 0x1000);
            spr.draw(cdload_module.get_subfile_by_id(0x025D0002), (e->value >> 4) * 3 + 0x1D, e->x + 6, e->y + 2);
        }
    }
    if (cardgame_popup_styles[e->kind].bank == 2) {
        CARDGAME_PANEL_SPRITE(e, 0x340, 0x025D0003, cardgame_popup_styles[e->kind].frame, e->x + dx, e->y);
    } else {
        CARDGAME_PANEL_SPRITE(e, 0x280, 0x025D0002, cardgame_popup_styles[e->kind].frame, e->x + dx, e->y);
    }
}

void cardgame_board_show_popup_numbers(CardgameBoard *obj, CardgameBoardData *data, CardgamePopup *e) {
    s32 values[2];
    s32 i;

    values[0] = e->card_stats[0];
    values[1] = e->card_stats[1];
    for (i = 0; i < 2; i++) {
        data->numbers[i]->set_pos(data->numbers[i], e->x + 0x67, e->y + 4 + i * 13);
        data->numbers[i]->set_line_number(data->numbers[i], 0, values[i]);
        data->numbers[i]->measure(data->numbers[i], 1);
    }
}

void cardgame_board_show_popup_text(CardgameBoard *obj, CardgamePopup *e, MessageWindow *win, s32 msg, s32 i) {
    if (e->value != 0) {
        win->set_pos(win, e->x + cardgame_popup_text_x[i], e->y + 4);
        win->set_text(win, cdload_module.files.get_file(msg), e->value);
    } else {
        win->set_visible(win, 0);
    }
}

void cardgame_board_show_popup_text_centered(CardgameBoard *obj, CardgameBoardData *data, CardgamePopup *e, MessageWindow *win, s32 msg) {
    Font fnt;
    s32 width;

    if (e->value != 0) {
        win->set_text(win, cdload_module.files.get_file(msg), e->value);
        font_init(&fnt);
        width = fnt.get_text_width(win->lines, win->font, win->fixed_w);
        if (e->value == 31) {
            win->set_palette(win, 3);
        } else {
            win->set_palette(win, 0);
        }
        win->set_visible(win, 1);
        win->set_pos(win, 160 - width / 2, e->y + 4);
    } else {
        win->set_visible(win, 0);
    }
}

void cardgame_board_update_popup(CardgameBoard *obj, CardgameBoardData *data, CardgamePopup *e, s32 i) {
    if (e->state == 0) {
        return;
    }
    switch (e->state) {
    case 1:
    default:
        e->scale = 0x1000 - (e->timer << 12) / e->duration;
        e->timer -= gfx_module.funcs.get_frame_ticks();
        if (e->timer <= 0) {
            e->state = 2;
            switch (i) {
            case 0:
                cardgame_board_show_popup_text(obj, e, data->texts[2], records_language + 15, 1);
                break;
            case 1:
                break;
            case 2:
                cardgame_board_show_popup_text(obj, e, data->texts[4], records_language + 22, 0);
                break;
            case 3:
                cardgame_board_show_popup_text(obj, e, data->texts[3], records_language + 15, 0);
                break;
            case 4:
                break;
            case 5:
                if (e->kind == 5) {
                    cardgame_board_show_popup_text_centered(obj, data, e, data->texts[0], records_language + 15);
                } else {
                    cardgame_board_show_popup_text(obj, e, data->texts[0], records_language + 15, e->value != 0x24);
                }
                break;
            }
        }
        break;
    case 2:
        e->scale = 0x1000;
        switch (i) {
        case 2:
            cardgame_board_show_popup_text(obj, e, data->texts[4], records_language + 22, 0);
            break;
        case 3:
            cardgame_board_show_popup_text(obj, e, data->texts[3], records_language + 15, 0);
            break;
        case 4:
            if (e->value == 500) {
                if (e->card_stats[2] == 0) {
                    e->value = 42;
                    cardgame_board_show_popup_text(obj, e, data->texts[5], records_language + 15, 1);
                } else {
                    e->value = 64;
                    cardgame_board_show_popup_numbers(obj, data, e);
                    cardgame_board_show_popup_text(obj, e, data->texts[5], records_language + 15, 2);
                }
            } else {
                data->numbers[0]->set_visible(data->numbers[0], 0);
                data->numbers[1]->set_visible(data->numbers[1], 0);
                cardgame_board_show_popup_text(obj, e, data->texts[5], records_language + 29, 0);
            }
            break;
        }
        break;
    case 3:
        switch (i) {
        case 0:
            data->texts[2]->set_visible(data->texts[2], 0);
            break;
        case 1:
            break;
        case 2:
            data->texts[4]->set_visible(data->texts[4], 0);
            break;
        case 3:
            data->texts[3]->set_visible(data->texts[3], 0);
            break;
        case 4:
            data->numbers[0]->set_visible(data->numbers[0], 0);
            data->numbers[1]->set_visible(data->numbers[1], 0);
            data->texts[5]->set_visible(data->texts[5], 0);
            break;
        case 5:
            data->texts[0]->set_visible(data->texts[0], 0);
            break;
        }
        e->timer -= gfx_module.funcs.get_frame_ticks();
        if (e->timer <= 0) {
            e->state = 0;
        }
        e->scale = (e->timer << 12) / e->duration;
        break;
    }
    cardgame_board_draw_popup(obj, data, e);
}

void cardgame_board_update_popups(CardgameBoard *obj, CardgameBoardData *data) {
    s32 i;

    for (i = 0; i < 6; i++) {
        cardgame_board_update_popup(obj, data, &obj->popups[i], i);
    }
}

void cardgame_board_draw_dialog_window(CardgameBoard *obj, CardgameBoardData *data, s16 scale, s32 x, s32 y) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(0x100, 1);
    spr.set_vram_pos(0x280, 0);
    spr.set_pivot(0, 0x78);
    spr.set_scale(scale, 0x1000, 0x1000);
    spr.draw(cdload_module.get_subfile_by_id(0x025D0002), 0x1A, x, y);
}

void cardgame_board_draw_dialog_icon(CardgameBoard *obj, CardgameBoardData *data, s32 x, s32 y) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(0x100, 1);
    spr.set_vram_pos(0x280, 0);
    spr.set_scale(0x1000, 0x1000, 0x1000);
    spr.draw(cdload_module.get_subfile_by_id(0x025D0002), 0x18, x, y);
}

/* Updates the dialog (CardgameBoard.dialog): opens, shows its message (message) and the yes/no cursor (cursor,
 * answer), bounces the cursor once confirmed (state 4) and closes. */
void cardgame_board_update_dialog(CardgameBoard *obj, CardgameBoardData *data) {
    Font fnt;
    s16 scale;
    s32 width;
    s32 i;
    s32 t;
    s32 duration;

    if (obj->dialog.state == 0) {
        return;
    }
    switch (obj->dialog.state) {
    case 1:
    default:
        duration = obj->dialog.duration;
        t = obj->dialog.timer << 12;
        scale = 0x1000 - (duration != 0 ? t / duration : t);
        obj->dialog.timer -= gfx_module.funcs.get_frame_ticks();
        if (obj->dialog.timer <= 0) {
            obj->dialog.state = 2;
            switch (obj->dialog.cursor) {
            case 0:
            default:
                data->texts[7]->set_visible(data->texts[7], 0);
                data->cursor->show(data->cursor, 0);
                break;
            case 1:
                data->cursor->show(data->cursor, 1);
                data->cursor->set_pos(data->cursor, 20,
                                     cardgame_dialog_origins[obj->dialog.position].y + 17 + obj->dialog.answer * 14);
                data->texts[7]->set_text(data->texts[7], cdload_module.files.get_file(records_language + 15), 0x18);
                data->texts[7]->set_pos(data->texts[7], cardgame_dialog_origins[obj->dialog.position].x + 27,
                                         cardgame_dialog_origins[obj->dialog.position].y + 18);
                break;
            case 2:
            case 3:
                data->cursor->show(data->cursor, 1);
                data->cursor->set_pos(data->cursor, 20,
                                     cardgame_dialog_origins[obj->dialog.position].y + 17 + obj->dialog.answer * 14);
                data->texts[7]->set_text(data->texts[7], cdload_module.files.get_file(records_language + 15), 0x45);
                if (obj->dialog.cursor == 3) {
                    data->texts[7]->set_palette(data->texts[7], 7);
                }
                data->texts[7]->set_pos(data->texts[7], cardgame_dialog_origins[obj->dialog.position].x + 27,
                                         cardgame_dialog_origins[obj->dialog.position].y + 18);
                data->numbers[0]->set_text(data->numbers[0], cdload_module.files.get_file(records_language + 15), 0x46);
                data->numbers[0]->measure(data->numbers[0], 0);
                data->numbers[0]->set_pos(data->numbers[0], cardgame_dialog_origins[obj->dialog.position].x + 27,
                                         cardgame_dialog_origins[obj->dialog.position].y + 32);
                break;
            }
            if (obj->dialog.position == 3) {
                data->texts[1]->set_pos(data->texts[1], cardgame_dialog_origins[3].x + 64, cardgame_dialog_origins[3].y + 4);
                data->texts[1]->set_text(data->texts[1], cdload_module.files.get_file(records_language + 15), 0x42);
                data->texts[7]->set_text(data->texts[7], cdload_module.files.get_file(records_language + 106),
                                         obj->dialog.message);
                data->texts[7]->set_pos(data->texts[7], cardgame_dialog_origins[obj->dialog.position].x + 64,
                                         cardgame_dialog_origins[obj->dialog.position].y + 18);
            } else if (obj->dialog.message != 0) {
                data->texts[1]->set_pos(data->texts[1], cardgame_dialog_origins[obj->dialog.position].x + 27,
                                         cardgame_dialog_origins[obj->dialog.position].y + 4);
                data->texts[1]->set_text(data->texts[1], cdload_module.files.get_file(records_language + 15),
                                         obj->dialog.message);
                if (obj->dialog.message == 0x13) {
                    data->texts[7]->set_text(data->texts[7], cdload_module.files.get_file(records_language + 15), 0x2B);
                    data->texts[7]->set_pos(data->texts[7], cardgame_dialog_origins[obj->dialog.position].x + 27,
                                             cardgame_dialog_origins[obj->dialog.position].y + 18);
                    font_init(&fnt);
                    width = fnt.get_text_width(data->texts[7]->lines, data->texts[7]->font, data->texts[7]->fixed_w) + 27;
                    for (i = 0; i < 2; i++) {
                        data->numbers[i]->set_pos(data->numbers[i], cardgame_dialog_origins[obj->dialog.position].x + width + 14,
                                                 cardgame_dialog_origins[obj->dialog.position].y + 18 + i * 14);
                        data->numbers[i]->set_line_number(data->numbers[i], 0, obj->panels[i].wins);
                        data->numbers[i]->measure(data->numbers[i], 0);
                    }
                } else if (obj->dialog.message == 0x3E) {
                    data->numbers[0]->set_pos(data->numbers[0], cardgame_dialog_origins[obj->dialog.position].x + 62,
                                             cardgame_dialog_origins[obj->dialog.position].y + 4);
                    data->numbers[0]->set_line_number(data->numbers[0], 0, obj->opponent_level);
                    data->numbers[0]->measure(data->numbers[0], 1);
                    data->numbers[1]->set_text(data->numbers[1], cdload_module.files.get_file(records_language + 43),
                                             (obj->opponent + 1) / 2);
                    data->numbers[1]->set_pos(data->numbers[1], cardgame_dialog_origins[obj->dialog.position].x + 27,
                                             cardgame_dialog_origins[obj->dialog.position].y + 18);
                }
            } else {
                data->texts[1]->set_visible(data->texts[1], 0);
            }
        }
        break;
    case 2:
        scale = 0x1000;
        if (obj->dialog.position == 3) {
            cardgame_board_draw_dialog_icon(obj, data, 28, 100);
        }
        if (obj->dialog.cursor != 0) {
            data->cursor->set_pos(data->cursor, 20,
                                 cardgame_dialog_origins[obj->dialog.position].y + 17 + obj->dialog.answer * 14);
        }
        break;
    case 4:
        scale = 0x1000;
        data->cursor->set_pos(data->cursor, 20 - rsin((obj->dialog.timer << 12) / 20) / 512,
                             cardgame_dialog_origins[obj->dialog.position].y + 17 + obj->dialog.answer * 14);
        obj->dialog.timer -= gfx_module.funcs.get_frame_ticks();
        if (obj->dialog.timer <= 0) {
            data->cursor->show(data->cursor, 0);
            obj->close_dialog(obj);
            data->cursor->set_delay(data->cursor, 0x20);
            obj->dialog.state = 5;
        }
        break;
    case 5:
        data->cursor->show(data->cursor, 0);
        data->texts[1]->set_visible(data->texts[1], 0);
        data->texts[7]->set_palette(data->texts[7], 0);
        data->texts[7]->set_visible(data->texts[7], 0);
        data->numbers[0]->set_visible(data->numbers[0], 0);
        data->numbers[1]->set_visible(data->numbers[1], 0);
        data->numbers[2]->set_visible(data->numbers[2], 0);
        obj->dialog.timer -= gfx_module.funcs.get_frame_ticks();
        if (obj->dialog.timer <= 0) {
            obj->dialog.state = 0;
        }
        scale = (obj->dialog.timer << 12) / obj->dialog.duration;
        break;
    }
    cardgame_board_draw_dialog_window(obj, data, scale, cardgame_dialog_origins[obj->dialog.position].x,
                           cardgame_dialog_origins[obj->dialog.position].y);
}

void cardgame_board_draw_menu_window(CardgameBoard *obj, CardgameBoardData *data, s16 scale, s32 x, s32 y) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(0x100, 1);
    spr.set_vram_pos(0x340, 0);
    spr.set_pivot(0, 0x86);
    spr.set_scale(scale, 0x1000, 0x1000);
    spr.draw(cdload_module.get_subfile_by_id(0x025D0003), 1, x, y);
}

void cardgame_board_update_menu(CardgameBoard *obj, CardgameBoardData *data) {
    s16 level;
    s32 t;
    s32 duration;

    if (obj->menu.state == 0) {
        return;
    }
    switch (obj->menu.state) {
    case 1:
    default:
        duration = obj->menu.duration;
        t = obj->menu.timer << 12;
        level = 0x1000 - (duration != 0 ? t / duration : t);
        obj->menu.timer -= gfx_module.funcs.get_frame_ticks();
        if (obj->menu.timer <= 0) {
            obj->menu.state = 2;
            data->cursor->show(data->cursor, 1);
            data->cursor->set_pos(data->cursor, 10, obj->menu.cursor * 14 + 101);
            data->texts[6]->set_text(data->texts[6], cdload_module.files.get_file(records_language + 15), 32);
            data->texts[6]->set_pos(data->texts[6], 24, 101);
        }
        break;
    case 2:
        level = 0x1000;
        data->cursor->set_pos(data->cursor, 10, obj->menu.cursor * 14 + 101);
        break;
    case 4:
        level = 0x1000;
        data->cursor->set_pos(data->cursor, 10 - rsin((obj->menu.timer << 12) / 20) / 512,
                             obj->menu.cursor * 14 + 101);
        obj->menu.timer -= gfx_module.funcs.get_frame_ticks();
        if (obj->menu.timer <= 0) {
            data->cursor->show(data->cursor, 0);
            obj->close_menu(obj);
            data->cursor->set_delay(data->cursor, 32);
            obj->menu.state = 5;
        }
        break;
    case 5:
        data->cursor->show(data->cursor, 0);
        data->texts[6]->set_visible(data->texts[6], 0);
        obj->menu.timer -= gfx_module.funcs.get_frame_ticks();
        if (obj->menu.timer <= 0) {
            obj->menu.state = 0;
        }
        level = (obj->menu.timer << 12) / obj->menu.duration;
        break;
    }
    cardgame_board_draw_menu_window(obj, data, level, 0, 0x60);
}

s32 cardgame_board_get_card_x(s32 count, s32 i) {
    s32 step;

    if (count < 7) {
        step = 0x2900;
    } else {
        step = 0xF600 / count;
    }
    return step * i;
}

/* Draws a side's panel: card-kind icons and lamps, the numbers, the frame and the rounds won. */
void cardgame_board_draw_panel(CardgameBoardPanel *panel, CardgameBoardData *data, s32 side) {
    Sprite icons;
    Sprite lamps;
    CardgameNumber num;
    Sprite spr;
    s32 y;
    s32 y2;
    s32 t30b;
    s32 frame;
    s32 t30;
    s32 frame2;
    s32 k;
    s32 t36;
    s32 t24;
    s32 j;
    s32 i;

    y = panel->y;
    y2 = panel->wins_y;
    if (main_screen_pos != 0) {
        if (side == 0) {
            y += 12;
            y2 += 12;
        } else {
            y -= 12;
            y2 -= 12;
        }
    }

    t36 = panel->ticks % 36;
    sprite_init(&icons);
    icons.set_layer_id(0x100, 1);
    icons.set_vram_pos(0x340, 0);
    icons.set_palette(cardgame_panel_icon_palettes[t36 / 6]);
    for (j = 0; j < 5; j++) {
        if (panel->lamps[j] != 0) {
            icons.draw(cdload_module.get_subfile_by_id(0x025D0003), 13, panel->x + cardgame_panel_art[side].pos[12].x + j * 42,
                         y + cardgame_panel_art[side].pos[12].y);
        }
    }
    if (panel->lamps[8] != 0) {
        icons.draw(cdload_module.get_subfile_by_id(0x025D0003), 12, panel->x + cardgame_panel_art[side].pos[16].x,
                     y + cardgame_panel_art[side].pos[16].y);
    }
    if (panel->lamps[9] != 0) {
        icons.draw(cdload_module.get_subfile_by_id(0x025D0003), 12, panel->x + cardgame_panel_art[side].pos[17].x,
                     y + cardgame_panel_art[side].pos[17].y);
    }
    if (panel->lamps[6] != 0) {
        icons.draw(cdload_module.get_subfile_by_id(0x025D0003), 14, panel->x + cardgame_panel_art[side].pos[13].x,
                     y + cardgame_panel_art[side].pos[13].y);
    }
    if (panel->lamps[7] != 0) {
        icons.draw(cdload_module.get_subfile_by_id(0x025D0003), 14, panel->x + cardgame_panel_art[side].pos[14].x,
                     y + cardgame_panel_art[side].pos[14].y);
    }
    if (panel->lamps[5] != 0) {
        icons.draw(cdload_module.get_subfile_by_id(0x025D0003), 15, panel->discard_x + cardgame_panel_art[side].pos[15].x,
                     panel->discard_y + cardgame_panel_art[side].pos[15].y);
    }

    t24 = panel->ticks % 24;
    sprite_init(&lamps);
    lamps.set_layer_id(0x100, 1);
    lamps.set_vram_pos(0x340, 0);
    if (panel->lamps[8] != 0) {
        lamps.set_palette(t24 / 6 + 1);
    } else {
        lamps.set_palette(0);
    }
    lamps.draw(cdload_module.get_subfile_by_id(0x025D0003), cardgame_panel_lamp_frames[records_language][0],
                 panel->x + cardgame_panel_art[side].pos[5].x, y + cardgame_panel_art[side].pos[5].y);
    if (panel->lamps[9] != 0) {
        lamps.set_palette(t24 / 6 + 1);
    } else {
        lamps.set_palette(0);
    }
    lamps.draw(cdload_module.get_subfile_by_id(0x025D0003), cardgame_panel_lamp_frames[records_language][1],
                 panel->x + cardgame_panel_art[side].pos[7].x, y + cardgame_panel_art[side].pos[7].y);

    num.layer = 1;
    num.x = panel->x + cardgame_panel_art[side].pos[2].x;
    num.y = cardgame_panel_art[side].pos[2].y + y;
    num.digits = 2;
    num.zero_pad = 1;
    num.value = panel->counts[5];
    cardgame_board_draw_number(&num, 0);
    num.x = panel->x + cardgame_panel_art[side].pos[3].x;
    num.y = cardgame_panel_art[side].pos[3].y + y;
    num.digits = 2;
    num.zero_pad = 1;
    num.value = panel->counts[6];
    cardgame_board_draw_number(&num, 0);
    num.x = panel->x + cardgame_panel_art[side].pos[6].x;
    num.y = cardgame_panel_art[side].pos[6].y + y;
    num.digits = 3;
    num.zero_pad = 0;
    num.value = panel->attack;
    cardgame_board_draw_number(&num, 0);
    num.x = panel->x + cardgame_panel_art[side].pos[8].x;
    num.y = cardgame_panel_art[side].pos[8].y + y;
    num.digits = 3;
    num.zero_pad = 0;
    num.value = panel->hp;
    cardgame_board_draw_number(&num, 0);
    for (i = 0; i < 5; i++) {
        num.x = panel->x + cardgame_panel_art[side].pos[1].x + i * 42;
        num.y = cardgame_panel_art[side].pos[1].y + y;
        num.digits = 2;
        num.zero_pad = 1;
        num.value = panel->counts[i];
        cardgame_board_draw_number(&num, 0);
    }
    num.x = panel->discard_x + cardgame_panel_art[side].pos[4].x;
    num.y = panel->discard_y + cardgame_panel_art[side].pos[4].y;
    num.digits = 2;
    num.zero_pad = 1;
    num.value = panel->discard_count;
    cardgame_board_draw_number(&num, 0);

    sprite_init(&spr);
    spr.set_layer_id(0x100, 1);
    spr.set_vram_pos(0x280, 0);
    spr.draw(cdload_module.get_subfile_by_id(0x025D0002), cardgame_panel_art[side].frame_02 + panel->wins,
               panel->wins_x, y2);
    t30 = panel->ticks % 30;
    if (panel->lamps[7] != 0) {
        frame2 = t30 / 6;
    } else {
        frame2 = 0;
    }
    spr.draw(cdload_module.get_subfile_by_id(0x025D0002), cardgame_panel_lamp7_anim[frame2] + 9,
               panel->x + cardgame_panel_art[side].pos[10].x, y + cardgame_panel_art[side].pos[10].y);
    t30 = panel->ticks % 30;
    if (panel->lamps[6] != 0) {
        frame2 = t30 / 6;
    } else {
        frame2 = 0;
    }
    spr.draw(cdload_module.get_subfile_by_id(0x025D0002), cardgame_panel_lamp6_anim[frame2] + 6,
               panel->x + cardgame_panel_art[side].pos[9].x, y + cardgame_panel_art[side].pos[9].y);
    t30 = panel->ticks % 30;
    if (panel->lamps[5] != 0) {
        frame = t30 / 6;
    } else {
        frame = 0;
    }
    spr.draw(cdload_module.get_subfile_by_id(0x025D0002), cardgame_panel_discard_anim[frame] + 49, panel->discard_x, panel->discard_y);
    t30b = panel->ticks % 30;
    for (k = 0; k < 5; k++) {
        if (panel->lamps[k] != 0) {
            frame = t30b / 6;
        } else {
            frame = 0;
        }
        spr.draw(cdload_module.get_subfile_by_id(0x025D0002), cardgame_panel_kind_sprites[k] + cardgame_panel_kind_anim[frame],
                   panel->x + cardgame_panel_art[side].pos[0].x + k * 42, y + cardgame_panel_art[side].pos[0].y);
    }
    spr.draw(cdload_module.get_subfile_by_id(0x025D0002), cardgame_panel_art[side].frame_03,
               panel->discard_x + cardgame_panel_art[side].pos[11].x, panel->discard_y + cardgame_panel_art[side].pos[11].y);
    spr.draw(cdload_module.get_subfile_by_id(0x025D0002), cardgame_panel_art[side].frame_01, panel->x, y);
    panel->ticks = (panel->ticks + gfx_module.funcs.get_frame_ticks()) & 0xFFFF;
}

void cardgame_board_draw_panel_values(CardgameBoardPanel *panel, CardgameBoardData *data, s32 side) {
    s32 open_x0;
    s32 open_y0;
    s32 shut_x0;
    s32 shut_y0;
    s32 open_x1;
    s32 open_y1;
    s32 shut_x1;
    s32 shut_y1;
    s32 open_x2;
    s32 open_y2;
    s32 shut_x2;
    s32 shut_y2;

    if (side == 0) {
        open_x0 = 0;
        open_y0 = 0x8D;
        shut_x0 = 0;
        shut_y0 = 0xF1;
        open_x1 = 0x120;
        open_y1 = 0x8F;
        shut_x1 = 0x147;
        shut_y1 = 0x8F;
        open_x2 = 0x113;
        open_y2 = 0xBD;
        shut_x2 = 0x13A;
        shut_y2 = 0xBD;
    } else {
        open_x0 = 0;
        open_y0 = 0;
        shut_x0 = 0;
        shut_y0 = -100;
        open_x1 = 0x120;
        open_y1 = 0x50;
        shut_x1 = 0x147;
        shut_y1 = 0x50;
        open_x2 = 0x113;
        open_y2 = 0x26;
        shut_x2 = 0x13A;
        shut_y2 = 0x26;
    }
    switch (panel->unk_06) {
    case 0:
        break;
    case 1:
        panel->unk_06 = 0;
        break;
    case 2:
        panel->unk_06 = 0;
        break;
    }
    switch (panel->state) {
    case 0:
    case 2:
        break;
    case 1:
        panel->timer -= gfx_module.funcs.get_frame_ticks();
        if (panel->timer > 0) {
            panel->x = open_x0 - (open_x0 - shut_x0) * panel->timer / panel->duration;
            panel->y = open_y0 - (open_y0 - shut_y0) * panel->timer / panel->duration;
            panel->discard_x = open_x1 - (open_x1 - shut_x1) * panel->timer / panel->duration;
            panel->discard_y = open_y1 - (open_y1 - shut_y1) * panel->timer / panel->duration;
            panel->wins_x = open_x2 - (open_x2 - shut_x2) * panel->timer / panel->duration;
            panel->wins_y = open_y2 - (open_y2 - shut_y2) * panel->timer / panel->duration;
        } else {
            panel->state = 2;
            panel->timer = 0;
            panel->duration = 0;
            panel->x = open_x0;
            panel->y = open_y0;
            panel->discard_x = open_x1;
            panel->discard_y = open_y1;
            panel->wins_x = open_x2;
            panel->wins_y = open_y2;
        }
        break;
    case 3:
        panel->timer -= gfx_module.funcs.get_frame_ticks();
        if (panel->timer > 0) {
            panel->x = shut_x0 - (shut_x0 - open_x0) * panel->timer / panel->duration;
            panel->y = shut_y0 - (shut_y0 - open_y0) * panel->timer / panel->duration;
            panel->discard_x = shut_x1 - (shut_x1 - open_x1) * panel->timer / panel->duration;
            panel->discard_y = shut_y1 - (shut_y1 - open_y1) * panel->timer / panel->duration;
            panel->wins_x = shut_x2 - (shut_x2 - open_x2) * panel->timer / panel->duration;
            panel->wins_y = shut_y2 - (shut_y2 - open_y2) * panel->timer / panel->duration;
        } else {
            panel->state = 0;
            panel->unk_06 = 1;
            panel->timer = 0;
            panel->duration = 0;
            panel->x = shut_x0;
            panel->y = shut_y0;
            panel->discard_x = shut_x1;
            panel->discard_y = shut_y1;
            panel->wins_x = shut_x2;
            panel->wins_y = shut_x2;
        }
        break;
    case 4:
        panel->discard_x = shut_x1;
        panel->discard_y = shut_y1;
        panel->wins_x = shut_x2;
        panel->wins_y = shut_x2;
        panel->timer -= gfx_module.funcs.get_frame_ticks();
        if (panel->timer > 0) {
            panel->x = open_x0 - (open_x0 - shut_x0) * panel->timer / panel->duration;
            panel->y = open_y0 - (open_y0 - shut_y0) * panel->timer / panel->duration;
        } else {
            panel->state = 2;
            panel->timer = 0;
            panel->duration = 0;
            panel->x = open_x0;
            panel->y = open_y0;
        }
        break;
    case 5:
        panel->discard_x = shut_x1;
        panel->discard_y = shut_y1;
        panel->wins_x = shut_x2;
        panel->wins_y = shut_x2;
        panel->timer -= gfx_module.funcs.get_frame_ticks();
        if (panel->timer > 0) {
            panel->x = shut_x0 - (shut_x0 - open_x0) * panel->timer / panel->duration;
            panel->y = shut_y0 - (shut_y0 - open_y0) * panel->timer / panel->duration;
        } else {
            panel->state = 0;
            panel->unk_06 = 1;
            panel->timer = 0;
            panel->duration = 0;
            panel->x = shut_x0;
            panel->y = shut_y0;
        }
        break;
    }
}

void cardgame_board_draw_panel_anim(CardgameBoard *obj, CardgameBoardData *data, s32 side, CardgamePanelAnim *anim) {
    Sprite spr;
    s32 x;
    s32 y;
    s32 win_x;
    s32 win_y;

    if (side == 0) {
        x = cardgame_panel_layouts[main_screen_pos].icon[0][0];
        y = cardgame_panel_layouts[main_screen_pos].icon[0][1];
        win_x = cardgame_panel_layouts[main_screen_pos].window[0][0];
        win_y = cardgame_panel_layouts[main_screen_pos].window[0][1];
    } else {
        x = cardgame_panel_layouts[main_screen_pos].icon[1][0];
        y = cardgame_panel_layouts[main_screen_pos].icon[1][1];
        win_x = cardgame_panel_layouts[main_screen_pos].window[1][0];
        win_y = cardgame_panel_layouts[main_screen_pos].window[1][1];
    }
    if (anim->state != 0) {
        sprite_init(&spr);
        spr.set_layer_id(0x100, 1);
        spr.set_vram_pos(0x280, 0);
        spr.set_pivot(obj->panels[side].x + x, obj->panels[side].y + y);
        spr.set_scale(anim->level, 0x1000, 0x1000);
        spr.draw(cdload_module.get_subfile_by_id(0x025D0002), side == 0 ? 0x44 : 0x43, obj->panels[side].x + x,
                   obj->panels[side].y + y);
    }
    if (anim->state == 2) {
        data->pass_texts[side]->set_pos(data->pass_texts[side], obj->panels[side].x + win_x,
                                         obj->panels[side].y + win_y);
        data->pass_texts[side]->set_text(data->pass_texts[side], cdload_module.files.get_file(records_language + 15), 0x3F);
    } else {
        data->pass_texts[side]->set_visible(data->pass_texts[side], 0);
    }
}

void cardgame_board_update_panel_anim(CardgameBoard *obj, CardgameBoardData *data, s32 side, CardgamePanelAnim *anim) {
    switch (anim->state) {
    case 0:
    case 2:
        break;
    case 1:
        anim->level = 0x1000 - (anim->timer << 12) / anim->duration;
        anim->timer -= gfx_module.funcs.get_frame_ticks();
        if (anim->timer <= 0) {
            anim->state = 2;
        }
        break;
    case 3:
        anim->level = (anim->timer << 12) / anim->duration;
        anim->timer -= gfx_module.funcs.get_frame_ticks();
        if (anim->timer <= 0) {
            anim->state = 0;
        }
        break;
    }
    cardgame_board_draw_panel_anim(obj, data, side, anim);
}

void cardgame_board_update_mark_motion(CardgameBoardMark *mark) {
    mark->timer -= gfx_module.funcs.get_frame_ticks();
    if (mark->timer != 0) {
        mark->x = mark->target_x - (mark->target_x - mark->start_x) * mark->timer / mark->duration;
        mark->y = mark->target_y - (mark->target_y - mark->start_y) * mark->timer / mark->duration;
    } else {
        mark->state = 1;
        mark->duration = 0;
        mark->timer = 0;
        mark->x = mark->target_x;
        mark->y = mark->target_y;
    }
}

void cardgame_board_draw_mark(CardgameBoardMark *mark) {
    Sprite spr;
    s32 frame;

    sprite_init(&spr);
    spr.set_layer_id(0x100, 1);
    spr.set_vram_pos(0x340, 0);
    mark->blink++;
    switch (mark->blink >> 2) {
    default:
        mark->blink = 0;
    case 0:
        frame = 0;
        break;
    case 1:
        frame = 3;
        break;
    case 2:
        frame = 1;
        break;
    }
    spr.set_palette(frame);
    spr.draw(cdload_module.get_subfile_by_id(0x025D0003), 8, mark->x, mark->y);
}

/* Updates and draws the 12 marks; moving ones (state 2) step their motion first. */
void cardgame_board_update_marks(CardgameBoard *obj, CardgameBoardData *data) {
    u32 i;

    for (i = 0; i < 12; i++) {
        /* FAKE: one state test spelled (obj->marks + i)->state, the other obj->marks[i].state. cse1 then keeps two
         * loads (different address forms), the state giv gets the original's 9 refs (over the mark offset giv's) and
         * reload_cse deletes the second load. One spelling for both gives 98.4% (the two givs swap s0/s1); no
         * natural form found (mark pointer, switch, state local, i types, while loop, base pointer; wip-13). */
        if ((obj->marks + i)->state != 0) {
            if (obj->marks[i].state == 2) {
                cardgame_board_update_mark_motion(&obj->marks[i]);
            }
            cardgame_board_draw_mark(&obj->marks[i]);
        }
    }
}

void cardgame_board_update_panels(CardgameBoard *obj, CardgameBoardData *data) {
    cardgame_board_update_panel_anim(obj, data, 0, (CardgamePanelAnim *)&obj->panels[0].anim_level);
    cardgame_board_update_panel_anim(obj, data, 1, (CardgamePanelAnim *)&obj->panels[1].anim_level);
    cardgame_board_draw_panel_values(&obj->panels[0], data, 0);
    cardgame_board_draw_panel_values(&obj->panels[1], data, 1);
    cardgame_board_draw_panel(&obj->panels[0], data, 0);
    cardgame_board_draw_panel(&obj->panels[1], data, 1);
}

void cardgame_board_update_card_motion(CardgameBoard *obj, CardgameBoardCard *card) {
    card->timer -= gfx_module.funcs.get_frame_ticks();
    if (card->timer > 0) {
        if (card->x != card->target_x || card->y != card->target_y) {
            card->x = card->target_x - (card->target_x - card->start_x) * card->timer / card->duration;
            card->y = card->target_y - (card->target_y - card->start_y) * card->timer / card->duration;
        }
        if (*(s32 *)&card->scale_x != *(s32 *)&card->target_scale_x) {
            card->scale_x = card->target_scale_x - (card->target_scale_x - card->start_scale_x) * card->timer / card->duration;
            card->scale_y = card->target_scale_y - (card->target_scale_y - card->start_scale_y) * card->timer / card->duration;
        }
    } else {
        if (card->state != 3) {
            sound_module.play(0x800460BD);
        }
        card->state = 1;
        card->timer = 0;
        card->duration = 0;
        card->x = card->target_x;
        card->y = card->target_y;
        card->scale_x = card->target_scale_x;
        card->scale_y = card->target_scale_y;
        if (card->highlight == 0) {
            card->zooming = 0;
        }
    }
}

s32 cardgame_board_update_card_lunge(CardgameBoard *obj, CardgameBoardCard *card) {
    s32 done = 0;
    s32 dy;
    s32 offset;
    s32 x;
    s32 y;
    s16 scale_x;
    s16 scale_y;
    s32 timer;
    s32 sx;
    s32 sy;

    if (card->windup > 0) {
        card->windup -= gfx_module.funcs.get_frame_ticks();
        card->scale_x = 0x1C00 - card->windup * 192;
        card->scale_y = 0x1C00 - card->windup * 192;
        if (card->windup <= 0) {
            card->highlight |= 5;
            sound_module.play(0x8004603C);
            card->target_scale_x = 0x1200;
            card->target_scale_y = 0x1200;
            card->scale_x = 0x1C00;
            card->start_scale_x = 0x1C00;
            card->scale_y = 0x1C00;
            card->start_scale_y = 0x1C00;
            card->windup = 0;
        }
        return 0;
    }
    card->timer -= gfx_module.funcs.get_frame_ticks();
    if (card->timer > 0) {
        timer = card->timer;
        x = (card->target_x - card->start_x) * timer / card->duration;
        dy = card->target_y - card->start_y;
        card->x = card->target_x - x;
        y = dy * timer / card->duration;
        card->y = card->target_y - y;
        offset = rsin((card->timer << 12) / (card->duration * 2)) * 0x1C00;
        card->y += (dy > 0 ? -offset : offset) / 4096;
        card->scale_x = card->target_scale_x - (card->target_scale_x - card->start_scale_x) * card->timer / card->duration;
        card->scale_y = card->target_scale_y - (card->target_scale_y - card->start_scale_y) * card->timer / card->duration;
    } else {
        sx = card->start_x;
        sy = card->start_y;
        card->x = card->start_x = card->target_x;
        card->y = card->start_y = card->target_y;
        card->target_x = sx;
        card->target_y = sy;
        scale_x = card->target_scale_x;
        scale_y = card->target_scale_y;
        card->state = 2;
        card->target_scale_x = 0x1000;
        card->target_scale_y = 0x1000;
        done = 1;
        card->scale_x = scale_x;
        card->scale_y = scale_y;
        card->highlight &= ~5;
        card->timer = card->duration = card->duration * 2 + card->duration / 2;
        card->start_scale_x = card->scale_x;
        card->start_scale_y = card->scale_y;
    }
    return done;
}

s32 cardgame_board_update_card_jolt(CardgameBoard *obj, CardgameBoardCard *card) {
    s32 done = 0;

    card->x = card->start_x + cardgame_card_jolt_offsets[card->timer & 7];
    card->scale_y = rsin((card->timer << 12) / 24) / 16 + 0x1000;
    card->timer += gfx_module.funcs.get_frame_ticks();
    if (card->timer >= 12) {
        card->state = 1;
        done = 1;
        card->zooming = 0;
        card->effect = 0;
        card->scale_y = 0x1000;
        card->x = card->start_x;
    }
    return done;
}

s32 cardgame_board_update_card_boost(CardgameBoard *obj, CardgameBoardCard *card) {
    s32 done = 0;

    card->timer += gfx_module.funcs.get_frame_ticks();
    if (card->timer >= 10) {
        done = 1;
        card->state = 11;
        card->duration = 0;
        card->timer = 0;
        card->zooming = 0;
        card->effect = 0;
    }
    return done;
}

s32 cardgame_board_update_card_effect(CardgameBoard *obj, CardgameBoardCard *card, s32 ticks) {
    s32 done = 0;

    card->timer += gfx_module.funcs.get_frame_ticks();
    if (card->timer >= ticks) {
        card->state = 1;
        done = 1;
        card->duration = 0;
        card->timer = 0;
        card->zooming = 0;
        card->effect = 0;
    }
    return done;
}

s32 cardgame_board_update_card_shake(CardgameBoard *obj, CardgameBoardCard *card) {
    s32 done;

    card->x = card->start_x + cardgame_card_shake_offsets[card->timer & 7];
    card->y = card->start_y + cardgame_card_shake_offsets[pad_random.next() & 7];
    done = 0;
    card->scale_y = 0x1000 - rsin((card->timer << 12) / 24) / 8;
    card->timer += gfx_module.funcs.get_frame_ticks();
    if (card->timer >= 12) {
        card->state = 1;
        done = 1;
        card->zooming = 0;
        card->scale_y = 0x1000;
        card->effect = 0;
        card->x = card->start_x;
        card->y = card->start_y;
    }
    return done;
}

s32 cardgame_board_update_card_flip(CardgameBoard *obj, CardgameBoardCard *card) {
    s32 done;

    card->scale_x = 0x1000 - rsin((card->timer << 12) / 24);
    done = 0;
    card->timer += gfx_module.funcs.get_frame_ticks();
    if (card->duration == 0 && card->timer >= 7) {
        card->duration = 1;
        card->style ^= 3;
    }
    if (card->timer >= 12) {
        card->state = 1;
        done = 1;
        card->zooming = 0;
        card->scale_x = 0x1000;
    }
    return done;
}

void cardgame_board_draw_card_picture(CardgameBoardCard *card) {
    Sprite spr;
    s16 *cell;
    s16 *frame;
    s32 u;
    s32 v;
    s32 *sheet;

    u = card->card / 8 * 32;
    v = card->card % 8 * 32;
    sheet = cardgame_card_sheet;
    cell = (s16 *)((u8 *)sheet + sheet[2]);
    cell[0] = 1;
    cell[2] = -1;
    cell[1] = card->card;
    cell[3] = 0;
    cell[4] = 4;
    cell[5] = 2;
    frame = (s16 *)((u8 *)sheet + sheet[0]);
    frame[0] = u;
    frame[1] = v;
    frame[2] = 32;
    frame[3] = 32;
    frame[4] = 0;
    frame[5] = 0x100;
    frame[6] = 1;
    sprite_init(&spr);
    if (*(s32 *)&card->scale_x != 0x10001000) {
        spr.set_pivot((card->x >> 8) + 20, (card->y >> 8) + 23);
        spr.set_scale(card->scale_x, card->scale_y, 0x1000);
    }
    spr.set_layer_id(0x100, 1);
    spr.set_vram_pos(0x140, 0x100);
    spr.set_clut8_pos(0x300, 0x100);
    spr.draw(sheet, 0, card->x >> 8, card->y >> 8);
}

void cardgame_board_draw_card_effect(CardgameBoard *obj, CardgameBoardCard *card) {
    Sprite spr;

    if (card->style != 0 && card->effect != 0) {
        sprite_init(&spr);
        if (*(s32 *)&card->scale_x != 0x10001000) {
            spr.set_pivot((card->x >> 8) + 20, (card->y >> 8) + 23);
            spr.set_scale(card->scale_x, card->scale_y, 0x1000);
        }
        spr.set_layer_id(0x100, 1);
        spr.set_vram_pos(0x340, 0);
        switch (card->effect) {
        case 1:
            spr.draw(cdload_module.get_subfile_by_id(0x025D0003), cardgame_card_shake_frames[((u32)obj->ticks >> 2) & 3] + 0x28,
                       card->x >> 8, card->y >> 8);
            break;
        case 2:
            spr.draw(cdload_module.get_subfile_by_id(0x025D0003), cardgame_card_boost_frames[(CARDGAME_CARD_TIMER(card) >> 2) % 4] + 0x2C,
                       card->x >> 8, card->y >> 8);
            if (CARDGAME_CARD_TIMER(card) >= 8) {
                card->effect = 0;
                CARDGAME_CARD_TIMER(card) = 0;
            }
            CARDGAME_CARD_TIMER(card) += gfx_module.funcs.get_frame_ticks();
            break;
        case 3:
            spr.draw(cdload_module.get_subfile_by_id(0x025D0003), (CARDGAME_CARD_TIMER(card) >> 1) % 9 + 0x3C, card->x >> 8,
                       card->y >> 8);
            if (CARDGAME_CARD_TIMER(card)++ >= 36) {
                card->effect = 0;
                CARDGAME_CARD_TIMER(card) = 0;
            }
            CARDGAME_CARD_TIMER(card) += gfx_module.funcs.get_frame_ticks();
            break;
        case 4:
            spr.draw(cdload_module.get_subfile_by_id(0x025D0003), (CARDGAME_CARD_TIMER(card) >> 1) % 7 + 0x46, card->x >> 8,
                       card->y >> 8);
            if (CARDGAME_CARD_TIMER(card) >= 28) {
                card->effect = 0;
                CARDGAME_CARD_TIMER(card) = 0;
            }
            CARDGAME_CARD_TIMER(card) += gfx_module.funcs.get_frame_ticks();
            break;
        }
    }
}

void cardgame_board_draw_card_highlight(CardgameBoard *obj, CardgameBoardCard *card) {
    Sprite spr;

    if (card->style != 0) {
        if (card->highlight & 6) {
            sprite_init(&spr);
            if (card->highlight & 4) {
                spr.set_palette(cardgame_card_highlight_palettes[((u32)obj->ticks >> 2) % 6] + 5);
            } else {
                spr.set_palette(4);
            }
            if (*(s32 *)&card->scale_x != 0x10001000) {
                spr.set_pivot((card->x >> 8) + 20, (card->y >> 8) + 23);
                spr.set_scale(card->scale_x, card->scale_y, 0x1000);
            }
            spr.set_layer_id(0x100, 1);
            spr.set_vram_pos(0x340, 0);
            spr.draw(cdload_module.get_subfile_by_id(0x025D0003), 8, card->x >> 8, card->y >> 8);
        }
        if (card->highlight & 1) {
            sprite_init(&spr);
            spr.set_palette(cardgame_card_highlight_palettes[((u32)obj->ticks >> 2) % 6]);
            if (*(s32 *)&card->scale_x != 0x10001000) {
                spr.set_pivot((card->x >> 8) + 20, (card->y >> 8) + 23);
                spr.set_scale(card->scale_x, card->scale_y, 0x1000);
            }
            spr.set_layer_id(0x100, 1);
            spr.set_vram_pos(0x340, 0);
            spr.draw(cdload_module.get_subfile_by_id(0x025D0003), 8, card->x >> 8, card->y >> 8);
        }
    }
}

void cardgame_board_draw_card_badge(CardgameBoard *obj, CardgameBoardCard *card) {
    Sprite spr;

    if (card->style != 0 && card->turn_badge != 0) {
        sprite_init(&spr);
        if (*(s32 *)&card->scale_x != 0x10001000) {
            spr.set_pivot((card->x >> 8) + 20, (card->y >> 8) + 23);
            spr.set_scale(card->scale_x, card->scale_y, 0x1000);
        }
        spr.set_layer_id(0x100, 1);
        spr.set_vram_pos(0x280, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x025D0002), card->turn_badge + 0x33, (card->x >> 8) + 3, (card->y >> 8) + 2);
    }
}

void cardgame_board_draw_card_turn_marks(CardgameBoard *obj, CardgameBoardCard *card) {
    Sprite spr;
    s32 i;
    s32 n;

    if (card->style != 0) {
        for (n = i = 0; i < 3; i++) {
            if (card->turn_marks[i] != 0) {
                sprite_init(&spr);
                if (*(s32 *)&card->scale_x != 0x10001000) {
                    spr.set_pivot((card->x >> 8) + 20, (card->y >> 8) + 23);
                    spr.set_scale(card->scale_x, card->scale_y, 0x1000);
                }
                spr.set_layer_id(0x100, 1);
                spr.set_vram_pos(0x280, 0);
                spr.draw(cdload_module.get_subfile_by_id(0x025D0002), i + 0x37, (card->x >> 8) + 3 + n * 8, (card->y >> 8) + 21);
                n++;
            }
        }
    }
}

/* Draws one sprite of a card (frame `frame` of image `image`, VRAM x `vram_x`) at (px, py), scaled about
 * the card's centre when the card is scaled. A block of its own: GCC 2.8 frees the stack slot of a block
 * nested in another block when the outer one ends, so the caller's sprites share slots (the original
 * frame of cardgame_board_draw_card, 0x210, only fits that way). */
#define CARDGAME_CARD_SPRITE(card, vram_x, image, frame, px, py)              \
    {                                                                         \
        Sprite spr;                                                           \
                                                                              \
        sprite_init(&spr);                                                    \
        if (*(s32 *)&(card)->scale_x != 0x10001000) {                         \
            spr.set_pivot(((card)->x >> 8) + 20, ((card)->y >> 8) + 23);      \
            spr.set_scale((card)->scale_x, (card)->scale_y, 0x1000);          \
        }                                                                     \
        spr.set_layer_id(0x100, 1);                                           \
        spr.set_vram_pos(vram_x, 0);                                          \
        spr.draw(cdload_module.get_subfile_by_id(image), frame, px, py);      \
    }

/* Draws a board card by its style (1-3); style 1 also draws its two counts (attack, hp) when
 * is_digimon is set. The counts' block is nested so that its slot is freed for the sprites (frame 0x210). */
void cardgame_board_draw_card(CardgameBoard *obj, CardgameBoardCard *card) {
    if (card->style == 0) {
        return;
    }
    switch (card->style) {
    case 0:
        break;
    case 1:
        if (card->is_digimon != 0) {
            {
                CardgameNumber num;

                num.layer = 1;
                num.digits = 2;
                num.zero_pad = 0;
                num.x = (card->x >> 8) + 4;
                num.y = (card->y >> 8) + 33;
                num.value = card->attack;
                num.pivot_x = (card->x >> 8) + 20;
                num.pivot_y = (card->y >> 8) + 23;
                num.scale_x = card->scale_x;
                num.scale_y = card->scale_y;
                cardgame_board_draw_number(&num, 1);
                num.x = (card->x >> 8) + 23;
                num.y = (card->y >> 8) + 33;
                num.value = card->hp;
                num.pivot_x = (card->x >> 8) + 20;
                num.pivot_y = (card->y >> 8) + 23;
                num.scale_x = card->scale_x;
                num.scale_y = card->scale_y;
                cardgame_board_draw_number(&num, 1);
            }
            CARDGAME_CARD_SPRITE(card, 0x340, 0x025D0003, 0x1F, (card->x >> 8) + 18, (card->y >> 8) + 33);
        } else {
            CARDGAME_CARD_SPRITE(card, 0x340, 0x025D0003, 0x1E, (card->x >> 8) + 4, (card->y >> 8) + 33);
        }
        CARDGAME_CARD_SPRITE(card, 0x280, 0x025D0002, card->dimmed == 1 ? card->color + 0x3C : card->color,
                             card->x >> 8, card->y >> 8);
        cardgame_board_draw_card_picture(card);
        break;
    case 2:
        CARDGAME_CARD_SPRITE(card, 0x280, 0x025D0002, 0x10, card->x >> 8, card->y >> 8);
        break;
    case 3:
        CARDGAME_CARD_SPRITE(card, 0x280, 0x025D0002, 0xC, card->x >> 8, card->y >> 8);
        break;
    }
}

void cardgame_board_draw_card_shade(CardgameBoard *obj, CardgameBoardCard *card) {
    Sprite spr;

    if (card->style != 0 && card->dimmed != 0) {
        sprite_init(&spr);
        if (*(s32 *)&card->scale_x != 0x10001000) {
            spr.set_pivot((card->x >> 8) + 20, (card->y >> 8) + 23);
            spr.set_scale(card->scale_x, card->scale_y, 0x1000);
        }
        spr.set_layer_id(0x100, 1);
        spr.set_vram_pos(0x340, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x025D0003), 10, card->x >> 8, card->y >> 8);
    }
}

void cardgame_board_draw_card_flash(CardgameBoard *obj, CardgameBoardCard *card) {
    Sprite spr;

    if (card->style != 0 && card->state == 11) {
        sprite_init(&spr);
        spr.set_palette(card->duration / 2 % 5);
        spr.set_pivot((card->x >> 8) + 20, (card->y >> 8) + 23);
        spr.set_scale(card->scale_x, card->scale_y, 0x1000);
        spr.set_layer_id(0x100, 1);
        spr.set_vram_pos(0x340, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x025D0003), 9, card->x >> 8, card->y >> 8);
    }
}

void cardgame_board_update_card(CardgameBoard *obj, CardgameBoardData *data, CardgameBoardCard *card) {
    switch (card->state) {
    case 0:
        card->style = 0;
        break;
    case 2:
    case 3:
        obj->card_flags |= 1;
        cardgame_board_update_card_motion(obj, card);
        break;
    case 4:
        obj->card_flags |= 1;
        cardgame_board_update_card_flip(obj, card);
        break;
    case 5:
        obj->card_flags |= 1;
        if (cardgame_board_update_card_lunge(obj, card) != 0) {
            obj->card_flags |= 2;
        }
        break;
    case 6:
        obj->card_flags |= 1;
        cardgame_board_update_card_shake(obj, card);
        break;
    case 7:
        obj->card_flags |= 1;
        cardgame_board_update_card_jolt(obj, card);
        break;
    case 8:
        obj->card_flags |= 1;
        cardgame_board_update_card_boost(obj, card);
        break;
    case 11:
        obj->card_flags |= 1;
        card->duration += gfx_module.funcs.get_frame_ticks();
        if (card->duration >= 11) {
            card->state = 1;
        }
        break;
    case 9:
        cardgame_board_update_card_effect(obj, card, 36);
        obj->card_flags |= 1;
        break;
    case 10:
        cardgame_board_update_card_effect(obj, card, 28);
        obj->card_flags |= 1;
        break;
    }
}

void cardgame_board_draw_card_layers(CardgameBoard *obj, CardgameBoardCard *card) {
    if (card->scale_x != 0 && card->scale_y != 0) {
        cardgame_board_draw_card_highlight(obj, card);
        cardgame_board_draw_card_turn_marks(obj, card);
        cardgame_board_draw_card_badge(obj, card);
        cardgame_board_draw_card_effect(obj, card);
        cardgame_board_draw_card_shade(obj, card);
        cardgame_board_draw_card(obj, card);
    }
}

void cardgame_board_update_cards(CardgameBoard *obj, CardgameBoardData *data) {
    s32 i;
    s32 pass;

    for (i = 39; i >= 0; i--) {
        cardgame_board_update_card(obj, data, &obj->cards[i]);
        cardgame_board_draw_card_flash(obj, &obj->cards[i]);
    }
    for (pass = 0; pass < 2; pass++) {
        for (i = 39; i >= 0; i--) {
            if ((pass == 0 && obj->cards[i].zooming != 0) || (pass != 0 && obj->cards[i].zooming == 0)) {
                cardgame_board_draw_card_layers(obj, &obj->cards[i]);
            }
        }
    }
}

void cardgame_board_update(CardgameBoard *obj, CardgameBoardData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        data->cursor = message_create_cursor(0x100, 0, 0, 0);
        data->cursor->show(data->cursor, 0);
        data->numbers[0] = message_create_window(0x100, 1, 0, 0);
        data->numbers[1] = message_create_window(0x100, 1, 0, 0);
        data->numbers[2] = message_create_window(0x100, 1, 0, 0);
        data->pass_texts[0] = message_create_window(0x100, 1, 0, 0);
        data->pass_texts[1] = message_create_window(0x100, 1, 0, 0);
        data->texts[0] = message_create_window(0x100, 1, 0, 0);
        data->texts[1] = message_create_window(0x100, 1, 0, 0);
        data->texts[1]->set_page_lines(data->texts[1], 3);
        data->texts[2] = message_create_window(0x100, 1, 0, 0);
        data->texts[3] = message_create_window(0x100, 1, 0, 0);
        data->texts[4] = message_create_window(0x100, 1, 0, 0);
        data->texts[5] = message_create_window(0x100, 1, 0, 0);
        data->texts[5]->set_page_lines(data->texts[5], 3);
        data->texts[6] = message_create_window(0x100, 1, 0, 0);
        data->texts[6]->set_page_lines(data->texts[6], 5);
        data->texts[7] = message_create_window(0x100, 1, 0, 0);
        data->texts[7]->set_page_lines(data->texts[7], 2);
        break;
    case OBJECT_STATE_RUN:
        obj->ticks += gfx_module.funcs.get_frame_ticks();
        obj->card_flags = 0;
        cardgame_board_update_marks(obj, data);
        cardgame_board_update_menu(obj, data);
        cardgame_board_update_dialog(obj, data);
        cardgame_board_update_popups(obj, data);
        cardgame_board_update_cards(obj, data);
        cardgame_board_update_turn_marks(obj, data);
        cardgame_board_draw_background(obj);
        cardgame_board_update_panels(obj, data);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void cardgame_board_open_turn_mark(CardgameBoard *obj, s32 i, s16 side) {
    obj->turn_marks[i].unk_00 = 0x1000;
    obj->turn_marks[i].state = 1;
    obj->turn_marks[i].level = 0;
    obj->turn_marks[i].side = side;
    obj->turn_marks[i].duration = 10;
    obj->turn_marks[i].timer = 10;
}

void cardgame_board_close_turn_mark(CardgameBoard *obj, s32 i) {
    obj->turn_marks[i].unk_00 = 0x1000;
    obj->turn_marks[i].level = 0x1000;
    obj->turn_marks[i].state = 3;
    obj->turn_marks[i].duration = 5;
    obj->turn_marks[i].timer = 5;
}

void cardgame_board_open_popup(CardgameBoard *obj, s32 i, s16 kind, s32 message, s32 x, s32 y) {
    sound_module.play(0x40019);
    obj->popups[i].x = x;
    obj->popups[i].y = y;
    obj->popups[i].unk_06 = 0x1000;
    obj->popups[i].state = 1;
    obj->popups[i].scale = 0;
    obj->popups[i].kind = kind;
    obj->popups[i].value = message;
    obj->popups[i].duration = 12;
    obj->popups[i].timer = 12;
}

void cardgame_board_close_popup(CardgameBoard *obj, s32 i) {
    sound_module.play(0x4001A);
    obj->popups[i].scale = 0x1000;
    obj->popups[i].unk_06 = 0x1000;
    obj->popups[i].state = 3;
    obj->popups[i].duration = 6;
    obj->popups[i].timer = 6;
}

s32 cardgame_board_show_mark(CardgameBoard *obj, s32 i, s16 x, s16 y) {
    obj->marks[i].state = 1;
    obj->marks[i].x = x;
    obj->marks[i].y = y;
    obj->marks[i].target_y = 0;
    obj->marks[i].target_x = 0;
    obj->marks[i].start_y = 0;
    obj->marks[i].start_x = 0;
    obj->marks[i].duration = 0;
    obj->marks[i].timer = 0;
    obj->marks[i].blink = 0;
    return 0;
}

s32 cardgame_board_move_mark(CardgameBoard *obj, s32 i, s8 arg2, s16 arg3, s32 arg4) {
    obj->marks[i].state = 2;
    obj->marks[i].target_x = arg3;
    obj->marks[i].duration = arg2;
    obj->marks[i].timer = arg2;
    obj->marks[i].target_y = arg4;
    obj->marks[i].start_x = obj->marks[i].x;
    obj->marks[i].start_y = obj->marks[i].y;
    return 0;
}

void cardgame_board_open_dialog(CardgameBoard *obj, s32 message, s8 cursor, s16 answer, s32 position) {
    sound_module.play(0x40019);
    obj->dialog.duration = 12;
    obj->dialog.timer = 12;
    obj->dialog.answer = answer;
    obj->dialog.message = message;
    obj->dialog.cursor = cursor;
    obj->dialog.position = position;
    obj->dialog.state = 1;
}

void cardgame_board_close_dialog(CardgameBoard *obj) {
    sound_module.play(0x4001A);
    obj->dialog.duration = 6;
    obj->dialog.timer = 6;
    obj->dialog.state = 5;
}

void cardgame_board_confirm_dialog(CardgameBoard *obj) {
    sound_module.play(0x8004503C);
    obj->dialog.duration = 10;
    obj->dialog.timer = 10;
    obj->dialog.state = 4;
}

void cardgame_board_set_dialog_answer(CardgameBoard *obj, s16 answer) {
    obj->dialog.answer = answer;
}

void cardgame_board_open_menu(CardgameBoard *obj, s16 row) {
    sound_module.play(0x40019);
    obj->menu.duration = 12;
    obj->menu.timer = 12;
    obj->menu.cursor = row;
    obj->menu.state = 1;
}

void cardgame_board_close_menu(CardgameBoard *obj) {
    sound_module.play(0x4001A);
    obj->menu.duration = 6;
    obj->menu.timer = 6;
    obj->menu.state = 5;
}

void cardgame_board_confirm_menu(CardgameBoard *obj) {
    sound_module.play(0x8004503C);
    obj->menu.duration = 10;
    obj->menu.timer = 10;
    obj->menu.state = 4;
}

void cardgame_board_set_menu_cursor(CardgameBoard *obj, s16 row) {
    obj->menu.cursor = row;
}

void cardgame_board_reset_panels(CardgameBoard *obj) {
    s16 x = 0x147;
    s16 y = 0x140;

    obj->panels[0].state = 0;
    obj->panels[0].timer = obj->panels[0].duration = 0;
    obj->panels[0].x = 0;
    obj->panels[0].y = 0xF1;
    obj->panels[0].discard_x = x;
    obj->panels[0].discard_y = 0x8F;
    obj->panels[0].wins_x = y;
    obj->panels[0].wins_y = 0xBD;
    obj->panels[1].state = 0;
    obj->panels[1].timer = obj->panels[1].duration = 0;
    obj->panels[1].x = 0;
    obj->panels[1].y = -100;
    obj->panels[1].discard_x = x;
    obj->panels[1].discard_y = 0x50;
    obj->panels[1].wins_x = y;
    obj->panels[1].wins_y = 0x26;
}

void cardgame_board_open_panels(CardgameBoard *obj) {
    s16 state = 1;
    s16 ticks = 20;
    s16 y = 0x140;

    obj->panels[0].state = state;
    obj->panels[0].unk_06 = 2;
    obj->panels[0].timer = obj->panels[0].duration = ticks;
    obj->panels[0].x = 0;
    obj->panels[0].y = 0xF1;
    obj->panels[0].discard_x = 0x147;
    obj->panels[0].discard_y = 0x8F;
    obj->panels[0].wins_x = y;
    obj->panels[0].wins_y = 0xBD;
    obj->panels[1].state = state;
    obj->panels[1].timer = obj->panels[1].duration = ticks;
    obj->panels[1].x = 0;
    obj->panels[1].y = -100;
    obj->panels[1].discard_x = 0xF9;
    obj->panels[1].discard_y = 0x50;
    obj->panels[1].wins_x = y;
    obj->panels[1].wins_y = 0x26;
}

void cardgame_board_close_panels(CardgameBoard *obj) {
    s16 state = 3;
    s16 ticks = 10;
    s16 x = 0x120;
    s16 y = 0x113;

    obj->panels[0].state = state;
    obj->panels[0].timer = obj->panels[0].duration = ticks;
    obj->panels[0].x = 0;
    obj->panels[0].y = 0x8D;
    obj->panels[0].discard_x = x;
    obj->panels[0].discard_y = 0x8F;
    obj->panels[0].wins_x = y;
    obj->panels[0].wins_y = 0xBD;
    obj->panels[1].state = state;
    obj->panels[1].timer = obj->panels[1].duration = ticks;
    obj->panels[1].x = 0;
    obj->panels[1].y = 0;
    obj->panels[1].discard_x = x;
    obj->panels[1].discard_y = 0x50;
    obj->panels[1].wins_x = y;
    obj->panels[1].wins_y = 0x26;
}

void cardgame_board_open_side_panel(CardgameBoard *obj, s32 side) {
    obj->panels[side].state = 4;
    obj->panels[side ^ 1].state = 0;
    obj->panels[side].unk_06 = 2;
    obj->panels[side].duration = 10;
    obj->panels[side].timer = 10;
    if (side == 0) {
        obj->panels[0].x = 0;
        obj->panels[0].y = 0xF1;
    } else {
        obj->panels[side].x = 0;
        obj->panels[side].y = -100;
    }
}

void cardgame_board_close_side_panel(CardgameBoard *obj, s32 side) {
    obj->panels[side].state = 5;
    obj->panels[side ^ 1].state = 0;
    obj->panels[side].duration = 5;
    obj->panels[side].timer = 5;
    if (side == 0) {
        obj->panels[0].x = 0;
        obj->panels[0].y = 0x8D;
    } else {
        obj->panels[side].x = 0;
        obj->panels[side].y = 0;
    }
}

void cardgame_board_open_panel_anim(CardgameBoard *obj, s32 side) {
    obj->panels[side].anim_state = 1;
    obj->panels[side].anim_duration = 12;
    obj->panels[side].anim_timer = 12;
    obj->panels[side].anim_level = 0;
}

void cardgame_board_close_panel_anim(CardgameBoard *obj, s32 side) {
    obj->panels[side].anim_state = 3;
    obj->panels[side].anim_duration = 6;
    obj->panels[side].anim_timer = 6;
    obj->panels[side].anim_level = 0x1000;
}

s32 cardgame_board_place_card(CardgameBoard *obj, s32 i, s32 x, s32 y) {
    heap_funcs.bzero(&obj->cards[i], sizeof(CardgameBoardCard));
    obj->cards[i].style = 1;
    obj->cards[i].state = 1;
    obj->cards[i].x = x;
    obj->cards[i].y = y;
    obj->cards[i].target_scale_x = 0x1000;
    obj->cards[i].scale_x = 0x1000;
    obj->cards[i].target_scale_y = 0x1000;
    obj->cards[i].scale_y = 0x1000;
    obj->cards[i].card = 0;
    obj->cards[i].color = 0;
    obj->cards[i].index = i;
    obj->cards[i].attack = 0;
    obj->cards[i].hp = 0;
    obj->cards[i].effect = 0;
    obj->cards[i].dimmed = 0;
    obj->cards[i].zooming = 0;
    return 0;
}

s32 cardgame_board_flip_card(CardgameBoard *obj, s32 i) {
    sound_module.play(0x8004613E);
    obj->cards[i].state = 4;
    obj->cards[i].timer = 0;
    obj->cards[i].duration = 0;
    obj->cards[i].scale_x = 0x1000;
    obj->cards[i].scale_y = 0x1000;
    obj->cards[i].zooming = 0;
    return 0;
}

s32 cardgame_board_shake_card(CardgameBoard *obj, s32 i) {
    sound_module.play(0x9C0003);
    obj->cards[i].state = 6;
    obj->cards[i].scale_x = 0x1000;
    obj->cards[i].scale_y = 0x1000;
    obj->cards[i].timer = 0;
    obj->cards[i].duration = 0;
    obj->cards[i].effect = 1;
    obj->cards[i].zooming = 0;
    obj->cards[i].start_x = obj->cards[i].x;
    obj->cards[i].start_y = obj->cards[i].y;
    return 0;
}

s32 cardgame_board_jolt_card(CardgameBoard *obj, s32 i) {
    sound_module.play(0x9C0003);
    obj->cards[i].state = 7;
    obj->cards[i].timer = 0;
    obj->cards[i].duration = 0;
    obj->cards[i].effect = 1;
    obj->cards[i].zooming = 0;
    obj->cards[i].start_x = obj->cards[i].x;
    obj->cards[i].start_y = obj->cards[i].y;
    return 0;
}

s32 cardgame_board_boost_card(CardgameBoard *obj, s32 i) {
    sound_module.play(0x40014);
    obj->cards[i].state = 8;
    obj->cards[i].timer = 0;
    obj->cards[i].duration = 0;
    obj->cards[i].effect = 2;
    obj->cards[i].zooming = 0;
    obj->cards[i].start_x = obj->cards[i].x;
    obj->cards[i].start_y = obj->cards[i].y;
    return 0;
}

s32 cardgame_board_show_card_effect(CardgameBoard *obj, s32 i, s32 arg2) {
    switch (arg2) {
    case 0:
    default:
        obj->cards[i].state = 9;
        obj->cards[i].effect = 3;
        break;
    case 1:
        obj->cards[i].effect = 4;
        obj->cards[i].state = 10;
        break;
    }
    obj->cards[i].timer = 0;
    obj->cards[i].duration = 0;
    obj->cards[i].zooming = 0;
    obj->cards[i].start_x = obj->cards[i].x;
    obj->cards[i].start_y = obj->cards[i].y;
    return 0;
}

s32 cardgame_board_scale_card(CardgameBoard *obj, s32 i, s32 ticks, s32 scale_x, s32 scale_y, s32 instant) {
    obj->cards[i].target_scale_x = scale_x;
    obj->cards[i].target_scale_y = scale_y;
    obj->cards[i].start_scale_x = obj->cards[i].scale_x;
    obj->cards[i].start_scale_y = obj->cards[i].scale_y;
    if (instant != 1) {
        obj->cards[i].duration = ticks;
        obj->cards[i].timer = ticks;
        obj->cards[i].state = 2;
        obj->cards[i].zooming = 1;
        obj->cards[i].target_x = obj->cards[i].x;
        obj->cards[i].target_y = obj->cards[i].y;
    }
    return 0;
}

void cardgame_board_set_card_scale(CardgameBoard *obj, s32 i, s32 scale_x, s32 scale_y) {
    cardgame_board_scale_card(obj, i, 0, scale_x, scale_y, 1);
}

void cardgame_board_zoom_card(CardgameBoard *obj, s32 i, s32 ticks, s32 scale_x, s32 scale_y) {
    cardgame_board_scale_card(obj, i, ticks, scale_x, scale_y, 0);
}

void cardgame_board_set_card_target(CardgameBoard *obj, s32 i, s32 ticks, s32 x, s32 y) {
    obj->cards[i].target_x = x;
    obj->cards[i].duration = ticks;
    obj->cards[i].timer = ticks;
    obj->cards[i].target_y = y;
    obj->cards[i].start_x = obj->cards[i].x;
    obj->cards[i].start_y = obj->cards[i].y;
}

void cardgame_board_move_card(CardgameBoard *obj, s32 i, s32 ticks, s32 x, s32 y) {
    sound_module.play(0x8004603C);
    obj->cards[i].state = 2;
    cardgame_board_set_card_target(obj, i, ticks, x, y);
}

void cardgame_board_slide_card(CardgameBoard *obj, s32 i, s32 ticks, s32 x, s32 y) {
    obj->cards[i].state = 3;
    cardgame_board_set_card_target(obj, i, ticks, x, y);
}

s32 cardgame_board_lunge_card(CardgameBoard *obj, s32 i, s32 arg2, s32 x, s32 y) {
    obj->cards[i].state = 5;
    obj->cards[i].windup = 0x10;
    obj->cards[i].scale_x = 0x1000;
    obj->cards[i].scale_y = 0x1000;
    obj->cards[i].target_x = x;
    obj->cards[i].duration = arg2;
    obj->cards[i].timer = arg2;
    obj->cards[i].target_y = y;
    obj->cards[i].start_x = obj->cards[i].x;
    obj->cards[i].start_y = obj->cards[i].y;
    return 0;
}

void cardgame_board_set_panel_value(CardgameBoard *obj, s32 side, u32 field, s32 value) {
    switch (field) {
    case 0:
        obj->panels[side].counts[0] = value;
        break;
    case 1:
        obj->panels[side].counts[1] = value;
        break;
    case 2:
        obj->panels[side].counts[2] = value;
        break;
    case 3:
        obj->panels[side].counts[3] = value;
        break;
    case 4:
        obj->panels[side].counts[4] = value;
        break;
    case 5:
        obj->panels[side].counts[5] = value;
        break;
    case 6:
        obj->panels[side].counts[6] = value;
        break;
    case 7:
        obj->panels[side].discard_count = value;
        break;
    case 8:
        obj->panels[side].attack = value;
        break;
    case 9:
        obj->panels[side].hp = value;
        break;
    }
}

void cardgame_board_set_lamps(CardgameBoard *obj, s32 flags) {
    if (flags & 1) {
        obj->panels[0].lamps[0] = 1;
    }
    if (flags & 4) {
        obj->panels[0].lamps[1] = 1;
    }
    if (flags & 0x10) {
        obj->panels[0].lamps[2] = 1;
    }
    if (flags & 0x40) {
        obj->panels[0].lamps[3] = 1;
    }
    if (flags & 0x100) {
        obj->panels[0].lamps[4] = 1;
    }
    if (flags & 0x400) {
        obj->panels[0].lamps[5] = 1;
    }
    if (flags & 0x1000) {
        obj->panels[0].lamps[6] = 1;
    }
    if (flags & 0x4000) {
        obj->panels[0].lamps[7] = 1;
    }
    if (flags & 0x10000) {
        obj->panels[0].lamps[8] = 1;
    }
    if (flags & 0x40000) {
        obj->panels[0].lamps[9] = 1;
    }
    if (flags & 2) {
        obj->panels[1].lamps[0] = 1;
    }
    if (flags & 8) {
        obj->panels[1].lamps[1] = 1;
    }
    if (flags & 0x20) {
        obj->panels[1].lamps[2] = 1;
    }
    if (flags & 0x80) {
        obj->panels[1].lamps[3] = 1;
    }
    if (flags & 0x200) {
        obj->panels[1].lamps[4] = 1;
    }
    if (flags & 0x800) {
        obj->panels[1].lamps[5] = 1;
    }
    if (flags & 0x2000) {
        obj->panels[1].lamps[6] = 1;
    }
    if (flags & 0x8000) {
        obj->panels[1].lamps[7] = 1;
    }
    if (flags & 0x20000) {
        obj->panels[1].lamps[8] = 1;
    }
    if (flags & 0x80000) {
        obj->panels[1].lamps[9] = 1;
    }
}

void cardgame_board_clear_lamps(CardgameBoard *obj) {
    heap_funcs.bzero(obj->panels[0].lamps, 10);
    heap_funcs.bzero(obj->panels[1].lamps, 10);
}

s32 cardgame_board_remove_card(CardgameBoard *obj, s32 i) {
    obj->cards[i].state = 0;
    obj->cards[i].style = 0;
    obj->cards[i].card = 0;
    return 0;
}

s32 cardgame_board_flash_card(CardgameBoard *obj, s32 i) {
    sound_module.play(0x4001C);
    obj->cards[i].state = 11;
    obj->cards[i].duration = 0;
    obj->cards[i].timer = 0;
    return 0;
}

void cardgame_board_lay_cards(CardgameBoard *obj, s16 ticks, s16 count, s32 x, s32 y) {
    s32 i;

    for (i = 0; i < count; i++) {
        s32 cx = x + cardgame_board_get_card_x(count, i);

        if (ticks == 0) {
            cardgame_board_place_card(obj, i, cx, y);
        } else {
            cardgame_board_move_card(obj, i, ticks, cx, y);
        }
    }
}

s32 cardgame_board_get_card_kind(CardgameBoard *obj, s32 card) {
    CardPicture pic;

    card_init(&pic);
    pic.select(obj->card_ids[card] + 1);
    return pic.record[0];
}

void cardgame_board_set_card(CardgameBoard *obj, s32 i, s32 card) {
    CardPicture pic;

    card_init(&pic);
    pic.select(obj->card_ids[card] + 1);
    obj->cards[i].card = card;
    obj->cards[i].attack = pic.record[1];
    obj->cards[i].hp = pic.record[2];
    obj->cards[i].level = pic.record[5];
    obj->cards[i].color = pic.record[0] - 1;
    if (pic.record[3] == 0x10) {
        obj->cards[i].is_digimon = 1;
    } else {
        obj->cards[i].is_digimon = 0;
    }
}

void cardgame_board_load_card_image(Tim *tim, u32 *data, s32 i) {
    tim->set_image_pos(0x140 + i / 8 * 16, 0x100 + i % 8 * 32);
    tim->set_clut_pos(0x300, i + 0x100);
    tim->load(data);
}

s32 cardgame_board_get_extra_card(s32 i) {
    return cardgame_extra_cards[i];
}

s32 cardgame_board_load_cards(s16 *ids, s16 *deck1, s16 *deck2) {
    Tim tim;
    CardPicture pic;
    s32 i;
    s32 count;

    card_init(&pic);
    tim_init(&tim);
    count = 0;
    for (i = 0; i < 40; i++) {
        pic.select(deck1[i] + 1);
        count++;
        cardgame_board_load_card_image(&tim, (u32 *)(pic.record + 12), i);
        ids[i] = deck1[i];
    }
    for (i = 0; i < 40; i++) {
        pic.select(deck2[i] + 1);
        count++;
        cardgame_board_load_card_image(&tim, (u32 *)(pic.record + 12), i + 40);
        ids[i + 40] = deck2[i];
    }
    for (i = 0; i < 9; i++) {
        pic.select(cardgame_fixed_cards[i] + 1);
        count++;
        cardgame_board_load_card_image(&tim, (u32 *)(pic.record + 12), i + 80);
        ids[i + 80] = cardgame_fixed_cards[i];
    }
    for (i = 0; i < 100; i++) {
        pic.select(cardgame_board_get_extra_card(i) + 1);
        count++;
        cardgame_board_load_card_image(&tim, (u32 *)(pic.record + 12), i + 89);
        ids[i + 89] = cardgame_board_get_extra_card(i);
    }
    return count;
}

CardgameBoard *cardgame_board_create(s16 *ids) {
    CardgameBoard *obj = object_new(cardgame_board_update, sizeof(CardgameBoard), 0x38);

    obj->set_panel_value = cardgame_board_set_panel_value;
    obj->open_turn_mark = cardgame_board_open_turn_mark;
    obj->close_turn_mark = cardgame_board_close_turn_mark;
    obj->open_popup = cardgame_board_open_popup;
    obj->close_popup = cardgame_board_close_popup;
    obj->set_lamps = cardgame_board_set_lamps;
    obj->clear_lamps = cardgame_board_clear_lamps;
    /* The slots take words (their callers pass words, which keeps their bytes); the functions' narrow parameters are
     * what the original's prologues show. The values passed are small, so both agree. */
    obj->open_dialog = (void (*)(struct CardgameBoard *, s32, s32, s32, s32))cardgame_board_open_dialog;
    obj->close_dialog = cardgame_board_close_dialog;
    obj->set_dialog_answer = (void (*)(struct CardgameBoard *, s32))cardgame_board_set_dialog_answer;
    obj->confirm_dialog = cardgame_board_confirm_dialog;
    obj->open_menu = cardgame_board_open_menu;
    obj->close_menu = cardgame_board_close_menu;
    obj->confirm_menu = cardgame_board_confirm_menu;
    obj->set_menu_cursor = cardgame_board_set_menu_cursor;
    obj->get_card_x = cardgame_board_get_card_x;
    obj->show_mark = cardgame_board_show_mark;
    obj->move_mark = cardgame_board_move_mark;
    obj->move_card = cardgame_board_move_card;
    obj->slide_card = cardgame_board_slide_card;
    obj->lunge_card = cardgame_board_lunge_card;
    obj->shake_card = cardgame_board_shake_card;
    obj->jolt_card = cardgame_board_jolt_card;
    obj->boost_card = cardgame_board_boost_card;
    obj->show_card_effect = cardgame_board_show_card_effect;
    obj->flip_card = cardgame_board_flip_card;
    obj->place_card = cardgame_board_place_card;
    obj->remove_card = cardgame_board_remove_card;
    obj->flash_card = cardgame_board_flash_card;
    obj->set_card_scale = cardgame_board_set_card_scale;
    obj->zoom_card = cardgame_board_zoom_card;
    obj->lay_cards = cardgame_board_lay_cards;
    obj->card_ids = ids;
    obj->close_panels = cardgame_board_close_panels;
    obj->open_panels = cardgame_board_open_panels;
    obj->close_side_panel = cardgame_board_close_side_panel;
    obj->open_panel_anim = cardgame_board_open_panel_anim;
    obj->close_panel_anim = cardgame_board_close_panel_anim;
    obj->open_side_panel = cardgame_board_open_side_panel;
    obj->reset_panels = cardgame_board_reset_panels;
    obj->set_card = cardgame_board_set_card;
    obj->get_card_kind = cardgame_board_get_card_kind;
    obj->load_cards = cardgame_board_load_cards;
    return obj;
}

void cardgame_loader_update(CardgameLoader *obj) {
    s16 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->index = 0;
        obj->base.set_state(obj, OBJECT_STATE_RUN);
        obj->ready = 0;
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            obj->file = cardgame_loader_files[obj->index].id;
            if (cardgame_loader_files[obj->index].localized != 0) {
                obj->file += records_language;
            }
            cdload_module.queue_file(obj->file);
            obj->base.step++;
        case 1:
            if (cdload_module.is_loading(obj->file) == 0) {
                i = obj->index;
                obj->index = i + 1;
                if (cardgame_loader_files[obj->index].id == -2) {
                    obj->index = i + 2;
                    obj->ready = 1;
                }
                if (cardgame_loader_files[obj->index].id == -1) {
                    obj->base.set_state(obj, OBJECT_STATE_END);
                }
                obj->base.step = 0;
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

CardgameLoader *cardgame_loader_create(void) {
    return object_new(cardgame_loader_update, sizeof(CardgameLoader), 0);
}

/* .data (address order) */

CardgameSlotOrigin cardgame_slot_origins[2][2] = {
    { { 0x1800, 0x9000 }, { 0x1800, 0x3200 } }, { { 0x1800, 0x9C00 }, { 0x1800, 0x2600 } },
};

s16 cardgame_turn_mark_pos[3][2] = { { 73, 97 }, { 123, 97 }, { 173, 97 } };

CardgamePopupStyle cardgame_popup_styles[6] = {
    { 0, 10, 2, 2 }, { 190, 10, 3, 2 }, { 190, 24, 4, 2 }, { 67, 24, 5, 2 }, { 0, 30, 0x37, 2 },
    { 320, 10, 0x19, 1 },
};

s16 cardgame_popup_text_x[4] = { 4, 24, 73, 0 };
CardgameDialogOrigin cardgame_dialog_origins[4] = { { 0, 138 }, { 0, 96 }, { 0, 50 }, { 0, 96 } };

CardgamePanelArt cardgame_panel_art[2] = {
    { 1, 0xE, 0x11, 0x2F, { { 17, 69 }, { 38, 71 }, { 249, 71 }, { 291, 71 }, { 1, -15 }, { 30, 52 }, { 46, 52 },
        { 78, 52 }, { 94, 52 }, { 228, 69 }, { 270, 69 }, { -7, -22 }, { 11, 67 }, { 222, 64 }, { 264, 64 },
        { -6, -20 }, { 23, 48 }, { 71, 48 } } },
    { 1, 0xF, 0x14, 0x30, { { 17, 14 }, { 38, 16 }, { 249, 16 }, { 291, 16 }, { 1, 21 }, { 30, 35 }, { 46, 35 },
        { 78, 35 }, { 94, 35 }, { 228, 14 }, { 270, 14 }, { -7, -6 }, { 11, 12 }, { 222, 9 }, { 264, 9 },
        { -6, -5 }, { 23, 31 }, { 71, 31 } } },
};

u8 cardgame_panel_icon_palettes[6] = { 0, 1, 2, 3, 2, 1 };

u8 cardgame_panel_lamp_frames[8][2] = {
    { 0x4D, 0x56 }, { 0x4E, 0x57 }, { 0x4F, 0x58 }, { 0x50, 0x59 }, { 0x52, 0x5B }, { 0x51, 0x5A }, { 0x53, 0x5C },
    { 0, 0 },
};

u8 cardgame_panel_lamp7_anim[5] = { 0, 1, 2, 1, 0 };
u8 cardgame_panel_lamp6_anim[5] = { 0, 1, 2, 1, 0 };
u8 cardgame_panel_discard_anim[5] = { 0, 1, 2, 1, 0 };
u8 cardgame_panel_kind_sprites[5] = { 0x1D, 0x20, 0x23, 0x26, 0x29 };
u8 cardgame_panel_kind_anim[5] = { 0, 1, 2, 1, 0 };

CardgamePanelLayout cardgame_panel_layouts[2] = {
    { { { 124, 51 }, { 124, 35 } }, { { 138, 52 }, { 138, 36 } } },
    { { { 124, 63 }, { 124, 23 } }, { { 138, 64 }, { 138, 24 } } },
};

s16 cardgame_card_jolt_offsets[4] = { -256, 768, -768, 256 };
s16 cardgame_card_shake_offsets[4] = { -1024, 512, -512, 1024 };

s32 cardgame_card_sheet[11] = {
    0xC, 0x1C, 0x20, 0x007C00A0, 0x002E0028, 0x00FC0020,
    0, 0xFF00, 1, 0xFFFF, 0,
};

u8 cardgame_card_shake_frames[4] = { 0, 1, 2, 3 };
u8 cardgame_card_boost_frames[4] = { 0, 1, 2, 3 };
u8 cardgame_card_highlight_palettes[6] = { 0, 1, 2, 3, 2, 1 };

s16 cardgame_extra_cards[100] = {
    60, 61, 62, 64, 65, 66, 67, 70,
    71, 72, 73, 74, 75, 76, 103, 104,
    105, 106, 107, 108, 109, 110, 112, 113,
    114, 115, 116, 117, 118, 119, 120, 121,
    147, 148, 149, 150, 151, 152, 154, 155,
    156, 157, 158, 159, 160, 161, 162, 163,
    164, 189, 190, 191, 192, 194, 195, 196,
    197, 198, 199, 200, 201, 202, 203, 205,
    206, 207, 232, 233, 234, 235, 237, 238,
    239, 240, 241, 242, 243, 244, 245, 246,
    247, 248, 249, 250, 275, 276, 280, 281,
    282, 283, 284, 285, 286, 287, 288, 289,
    290, 292, 293, 314,
};

s16 cardgame_fixed_cards[10] = {
    80, 99, 140, 184, 229, 251, 313, 314,
    315, 2573,
};

CardgameFileEntry cardgame_loader_files[12] = {
    { 43, 1 }, { 15, 1 }, { 2038, 0 }, { 2039, 0 }, { 2040, 0 }, { 2041, 0 }, { 2042, 0 }, { -2, 0 }, { 29, 1 },
    { 22, 1 }, { 106, 1 }, { -1, 0 },
};
