#ifndef STCRDDEK_H
#define STCRDDEK_H

/* STCRDDEK.PRO: the card deck screen (edit and name the three decks in gamestate_data.decks). */

#include "common.h"
#include "object.h"
#include "window_anim_type.h"
#include "overlay_common.h"

extern StageUtil stcrddek_util;

/* The main object (stcrddek_update_main). */
typedef struct StcrddekMain {
    /* 0x000 */ Object base;
    /* 0x050 */ s32 layer_id; /* layer */
    /* 0x054 */ s32 ot_depth; /* ordering table entry */
    /* 0x058 */ s32 scroll; /* background scroll, 0..95 */
    /* 0x05C */ s32 odd_frame; /* toggles every frame: scroll every other frame */
    /* 0x060 */ s32 deck_cursor; /* deck under the cursor, 0..2 */
    /* 0x064 */ s32 cursor_stopped; /* cursor stopped */
    /* 0x068 */ s32 cursor_frame; /* cursor animation frame, 0..15 */
    /* 0x06C */ s32 cursor_time; /* time of the last cursor frame */
    /* 0x070 */ s32 rename; /* 0: edit, 1: rename */
    /* 0x074 */ s32 type_counts[3][6]; /* per deck: cards of each type */
    /* 0x0BC */ WindowAnim title_anim; /* title */
    /* 0x0CC */ WindowAnim menu_anim; /* edit/rename menu */
    /* 0x0DC */ WindowAnim cursor_anim; /* cursor */
    /* 0x0EC */ WindowAnim decks_anim[3]; /* decks */
    /* 0x11C */ void (*count_card_types)(struct StcrddekMain *obj); /* stcrddek_count_card_types */
} StcrddekMain; /* size 0x120 */

/* The name entry object (stcrddek_update_name_entry). */
typedef struct StcrddekNameEntry {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 mode;
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 ot_depth; /* ordering table entry */
    /* 0x5C */ s32 image_pos; /* image position in VRAM */
    /* 0x60 */ s32 image_y;
    /* 0x64 */ s32 portrait_anim;
    /* 0x68 */ s32 portrait_frame;
    /* 0x6C */ s32 portrait_time;
    /* 0x70 */ s32 glow_frame;
    /* 0x74 */ s32 glow_time;
    /* 0x78 */ u16 name[10];   /* the name, SJIS; 0x4081 (a full-width space) pads it */
    /* 0x8C */ s32 unk_8C;
    /* 0x90 */ s32 name_cursor; /* cursor in the name */
    /* 0x94 */ s32 length; /* length of the name */
    /* 0x98 */ s32 cursor_column; /* cursor column */
    /* 0x9C */ s32 cursor_row; /* cursor row */
    /* 0xA0 */ s32 cursor_frame;
    /* 0xA4 */ s32 cursor_time;
    /* 0xA8 */ s32 cursor_shown;
    /* 0xAC */ s32 page;   /* page */
    /* 0xB0 */ s32 arrows_frame;
    /* 0xB4 */ s32 arrows_time;
    /* 0xB8 */ s32 unk_B8;
    /* 0xBC */ WindowAnim anims[3];
    /* 0xEC */ void (*get_name)(struct StcrddekNameEntry *obj, u8 *name); /* stcrddek_get_name */
    /* 0xF0 */ void (*close)(struct StcrddekNameEntry *obj);           /* stcrddek_close_name_entry */
} StcrddekNameEntry; /* size 0xF4 */

/* A scroll bar: a white bar w wide at x, moving between y top and bottom. */
typedef struct StcrddekBar {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id; /* layer */
    /* 0x54 */ s32 ot_depth; /* ordering table entry */
    /* 0x58 */ s32 x;      /* x */
    /* 0x5C */ s32 y;      /* y */
    /* 0x60 */ s32 w;      /* width */
    /* 0x64 */ s32 h;      /* height, 24.8 */
    /* 0x68 */ s32 lines_set; /* shown/lines set */
    /* 0x6C */ s32 shown;  /* lines shown */
    /* 0x70 */ s32 lines;  /* lines */
    /* 0x74 */ s32 first_line; /* first line shown */
    /* 0x78 */ s32 range_set; /* top/bottom set */
    /* 0x7C */ s32 top;    /* top */
    /* 0x80 */ s32 bottom; /* bottom */
    /* 0x84 */ s32 unk_84;
    /* 0x88 */ s32 line_height; /* height of a line, 24.8 */
    /* 0x8C */ void (*set_x)(struct StcrddekBar *obj, s32 x, s32 width);    /* stcrddek_set_bar_x */
    /* 0x90 */ void (*set_range)(struct StcrddekBar *obj, s32 top, s32 bottom); /* stcrddek_set_bar_range */
    /* 0x94 */ void (*set_lines)(struct StcrddekBar *obj, s32 shown, s32 lines); /* stcrddek_set_bar_lines */
    /* 0x98 */ void (*set_line)(struct StcrddekBar *obj, s32 line);            /* stcrddek_set_bar_line */
} StcrddekBar; /* size 0x9C */

StcrddekBar *stcrddek_create_bar(void);

/* The deck editor (its fields are stcrddek_800831F0.c's). */
typedef struct StcrddekEditor StcrddekEditor;

Fade *stcrddek_create_fade(void);
StcrddekEditor *stcrddek_create_editor(StcrddekMain *main, s32 deck);
StcrddekNameEntry *stcrddek_create_name_entry(u8 *name);
StcrddekMain *stcrddek_create_main(void);

#endif /* STCRDDEK_H */
