#ifndef STGDGLAB_H
#define STGDGLAB_H

/* STGDGLAB.PRO (the Digimon lab): types, module structs and functions shared by its files. */

#include "common.h"
#include "object.h"
#include "window_anim_type.h"
#include "overlay_common.h"

/* An entry of stgdglab_ids (ended by id 0). */
typedef struct StgdglabIdEntry {
    /* 0x0 */ s16 id;
    /* 0x2 */ s16 sprite;
    /* 0x4 */ s16 unk_04;
} StgdglabIdEntry; /* size 0x6 */

/* A scroll bar (stgdglab_8008A670.c; the same object as STCRDDEK's): a white bar w wide at x,
 * moving between y top and bottom. */
typedef struct StgdglabBar {
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
    /* 0x8C */ void (*set_x)(struct StgdglabBar *obj, s32 x, s32 width);     /* stgdglab_set_bar_x */
    /* 0x90 */ void (*set_range)(struct StgdglabBar *obj, s32 top, s32 bottom);  /* stgdglab_set_bar_range */
    /* 0x94 */ void (*set_lines)(struct StgdglabBar *obj, s32 shown, s32 lines); /* stgdglab_set_bar_lines */
    /* 0x98 */ void (*set_line)(struct StgdglabBar *obj, s32 line);             /* stgdglab_set_bar_line */
} StgdglabBar; /* size 0x9C */

/* A text window and where it goes. */
typedef struct StgdglabWindowPos {
    /* 0x0 */ s32 message;
    /* 0x4 */ s32 x;
    /* 0x8 */ s32 y;
} StgdglabWindowPos; /* size 0xC */

/* A Digimon's animation: its sprites, ended by -1. */
typedef struct StgdglabAnim {
    /* 0x00 */ s32 sprite[7];
} StgdglabAnim; /* size 0x1C */

/* A requirement of the lab's charts (stgdglab_80082F48.c): `count` of the IDs. */
typedef struct StgdglabReq {
    /* 0x0 */ s16 count;
    /* 0x2 */ s16 ids[5];
} StgdglabReq; /* size 0xC */

/* stgdglab_funcs: the overlay's tables and its helper functions (stgdglab_8008EB30.c), which the other files
 * call through it (the same helpers end STCRDDEK, STCRDSHP and other menus). One object: code that reuses
 * its address reaches the functions as base + 0x28.. (stgdglab_menu_run). */
typedef struct StgdglabFuncs {
    /* 0x00 */ StgdglabAnim *anims;        /* stgdglab_anims: per Digimon, its animation */
    /* 0x04 */ StgdglabWindowPos *menu_layout; /* stgdglab_menu_layout: the menu's windows */
    /* 0x08 */ StgdglabReq *charts[8];     /* stgdglab_chart_0..: 8 charts of 4 x 4 requirements */
    /* 0x28 */ void (*load_files)(void);                                 /* stgdglab_load_files */
    /* 0x2C */ s32 (*is_loading)(void);                                  /* stgdglab_is_loading */
    /* 0x30 */ void (*window_anim_start)(WindowAnim *anim, s32 open);    /* stgdglab_window_anim_start */
    /* 0x34 */ s32 (*window_anim_update)(WindowAnim *anim);              /* stgdglab_window_anim_update */
    /* 0x38 */ void (*lerp_start)(Tween *lerp, s32 from, s32 to, s32 frames); /* stgdglab_lerp_start */
    /* 0x3C */ s32 (*lerp_update)(Tween *lerp);                   /* stgdglab_lerp_update */
    /* 0x40 */ s32 (*get_sprite)(s32 id); /* stgdglab_get_sprite: stgdglab_ids's sprite for id */
    /* 0x44 */ s32 (*get_unk_04)(s32 id); /* stgdglab_get_unk_04: stgdglab_ids's unk_04 for id (unused) */
} StgdglabFuncs; /* size 0x48 */

extern StgdglabFuncs stgdglab_funcs;

/* The lab's menu (stgdglab_menu_create, stgdglab_800886DC.c, size 0x11C). */
typedef struct StgdglabMenu {
    /* 0x000 */ Object base;
    /* 0x050 */ struct StgdglabMain *main;
    /* 0x054 */ s32 chosen; /* an entry was chosen */
    /* 0x058 */ s32 layer_id; /* layer */
    /* 0x05C */ s32 ot_depth; /* ordering table entry */
    /* 0x060 */ s32 entry;  /* the entry chosen (index into stgdglab_screens) */
    /* 0x064 */ WindowAnim anims[9]; /* [3]: the menu */
    /* 0x0F4 */ s32 anim_time;
    /* 0x0F8 */ s32 blink_time;
    /* 0x0FC */ s32 frames[5];
    /* 0x110 */ s32 member_choices;
    /* 0x114 */ void (*open)(struct StgdglabMenu *obj); /* stgdglab_menu_open */
    /* 0x118 */ void (*close)(struct StgdglabMenu *obj); /* stgdglab_menu_close */
} StgdglabMenu; /* size 0x11C */

/* The overlay's main object (stgdglab_main_create, size 0x7C). */
typedef struct StgdglabMain {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id; /* layer */
    /* 0x54 */ s32 scroll; /* background scroll */
    /* 0x58 */ s32 odd_frame; /* the scroll moves every other frame */
    /* 0x5C */ s32 slot_count;
    /* 0x60 */ s32 member_count; /* party members (stgdglab_main_pack_party) */
    /* 0x64 */ s32 member;
    /* 0x68 */ s32 (*open_menu)(struct StgdglabMain *obj); /* stgdglab_main_open_menu */
    /* 0x6C */ s32 (*close_menu)(struct StgdglabMain *obj); /* stgdglab_main_close_menu */
    /* 0x70 */ s32 (*is_menu_running)(struct StgdglabMain *obj); /* stgdglab_main_is_menu_running */
    /* 0x74 */ void (*pack_party)(struct StgdglabMain *obj); /* stgdglab_main_pack_party */
    /* 0x78 */ void (*fade_out)(struct StgdglabMain *obj); /* stgdglab_main_fade_out */
} StgdglabMain; /* size 0x7C */

/* The main object's data block (0xC bytes). */
typedef struct StgdglabMainData {
    /* 0x0 */ StgdglabMenu *menu;    /* the menu */
    /* 0x4 */ Object *screen;   /* the screen chosen in the menu (stgdglab_screens) */
    /* 0x8 */ Fade *fade;    /* the fade */
} StgdglabMainData; /* size 0xC */

/* stgdglab_80082F48.c */
Fade *stgdglab_fade_create(void);
typedef struct StgdglabChart StgdglabChart; /* the evolution charts */
StgdglabChart *stgdglab_chart_create(StgdglabMain *main);

/* stgdglab_80087070.c */
typedef struct StgdglabDigimonScreen StgdglabDigimonScreen;
StgdglabDigimonScreen *stgdglab_digimon_create(StgdglabMain *main);

/* stgdglab_8008A92C.c */
typedef struct StgdglabSwap StgdglabSwap;
StgdglabSwap *stgdglab_swap_create(StgdglabMain *main);

/* stgdglab_800886DC.c */
StgdglabMenu *stgdglab_menu_create(StgdglabMain *main);

/* stgdglab_8008A670.c */
StgdglabBar *stgdglab_bar_create(void);

/* stgdglab_8008D7CC.c: the forms list of a party member's Digimon (size 0x174). */
typedef struct StgdglabForms {
    /* 0x000 */ Object base;
    /* 0x050 */ s32 list_all; /* also list the forms that aren't set */
    /* 0x054 */ s32 digimon; /* the Digimon */
    /* 0x058 */ s32 layer_id; /* layer */
    /* 0x05C */ s32 ot_depth; /* ordering table entry */
    /* 0x060 */ WindowAnim anims[3];
    /* 0x090 */ s32 forms[50];  /* the forms listed */
    /* 0x158 */ s32 first_form; /* first form shown */
    /* 0x15C */ s32 form_count; /* forms listed */
    /* 0x160 */ s32 cursor;  /* cursor (from first_form) */
    /* 0x164 */ s32 blink_time; /* time of the last arrow blink */
    /* 0x168 */ s32 arrows_shown; /* arrows shown */
    /* 0x16C */ s32 can_cancel; /* can be cancelled */
    /* 0x170 */ void (*close)(struct StgdglabForms *obj); /* stgdglab_forms_close */
} StgdglabForms; /* size 0x174 */

StgdglabForms *stgdglab_forms_create(s32 digimon, s32 all, s32 cancel);

/* stgdglab_80085470.c: the form change. */
typedef struct StgdglabFormset StgdglabFormset;
StgdglabFormset *stgdglab_formset_create(s32 digimon, s32 slot);

/* stgdglab_8008C278.c: the techniques of a set form. */
typedef struct StgdglabTechs StgdglabTechs;
StgdglabTechs *stgdglab_techs_create(s32 digimon, s32 slot);

/* stgdglab_8008EB30.c */
StgdglabMain *stgdglab_main_create(void);

#endif /* STGDGLAB_H */
