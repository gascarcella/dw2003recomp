#ifndef STSTATUS_H
#define STSTATUS_H

/* STSTATUS.PRO (the status menu): types, module structs and functions shared by its files. */

#include "common.h"
#include "object.h"
#include "window_anim_type.h"
#include "overlay_common.h"

/* A text window's place on screen (ststatus_module.party_layout points to a table of them). */
typedef struct StstatusLayout {
    /* 0x0 */ s32 text; /* line of the menu text file */
    /* 0x4 */ s32 x;
    /* 0x8 */ s32 y;
} StstatusLayout; /* size 0xC */

/* The text windows of a party member's panel (the pages' data blocks, one per member). */
typedef struct StstatusPanelWindows {
    /* 0x00 */ struct MessageWindow *name;    /* name */
    /* 0x04 */ struct MessageWindow *labels[5]; /* labels */
    /* 0x18 */ struct MessageWindow *values[5]; /* values */
} StstatusPanelWindows; /* size 0x2C */

/* A position. */
typedef struct StstatusPoint {
    /* 0x0 */ s32 x;
    /* 0x4 */ s32 y;
} StstatusPoint; /* size 0x8 */

/* A Digimon's sprite animation: its frames, -1 ends them (ststatus_module.anims). */
typedef struct StstatusAnim {
    /* 0x00 */ s32 frame[7];
} StstatusAnim; /* size 0x1C */

/* The status page (ststatus_create_status_page, size 0x14C): the party's panels, then a member's stats, forms
 * and equipment, with the forms page (ststatus_create_forms_page) and the equipment page (ststatus_create_equip_page)
 * as sub-pages. base.substep/timer are the member view's state (ststatus_status_run_member) and its "back to the
 * party" flag. */
typedef struct StstatusStatusPage {
    /* 0x000 */ Object base;
    /* 0x050 */ s32 parent;
    /* 0x054 */ s32 layer_id; /* layer */
    /* 0x058 */ s32 ot_depth; /* ordering table depth */
    /* 0x05C */ s32 member_count; /* party members */
    /* 0x060 */ s32 frames[3]; /* per member: sprite frame */
    /* 0x06C */ s32 frame_time; /* time of the last frame */
    /* 0x070 */ s32 sub_page_open; /* a sub-page is open */
    /* 0x074 */ s32 menu_cursor; /* menu cursor: 0 forms, 1 equipment */
    /* 0x078 */ s32 cursor_shown; /* the member cursor is shown */
    /* 0x07C */ s32 member; /* party member shown */
    /* 0x080 */ s32 cursor_frame; /* member cursor frame, 0..7 */
    /* 0x084 */ s32 cursor_time; /* time of its last frame */
    /* 0x088 */ WindowAnim panel_anims[3]; /* member panels */
    /* 0x0B8 */ WindowAnim bar_anims[2]; /* title, bottom bar */
    /* 0x0D8 */ WindowAnim member_anim; /* the member's panel */
    /* 0x0E8 */ WindowAnim name_anim; /* name */
    /* 0x0F8 */ WindowAnim stats_anim; /* stats */
    /* 0x108 */ WindowAnim menu_anim; /* menu */
    /* 0x118 */ WindowAnim list_anim; /* forms and equipment */
    /* 0x128 */ WindowAnim exp_anim; /* experience */
    /* 0x138 */ WindowAnim unused_anim; /* only its duration is set (8): nothing starts it */
    /* 0x148 */ void (*preview_item)(struct StstatusStatusPage *obj, s32 slot, s32 item); /* ststatus_status_preview_item: previews an
                                                                               * item in an equipment slot */
} StstatusStatusPage; /* size 0x14C */

/* A scroll bar (ststatus_create_bar, size 0x9C): the same object as STCRDDEK's StcrddekBar. */
typedef struct StstatusBar {
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
    /* 0x8C */ void (*set_x)(struct StstatusBar *obj, s32 x, s32 width);     /* ststatus_set_bar_x */
    /* 0x90 */ void (*set_range)(struct StstatusBar *obj, s32 top, s32 bottom); /* ststatus_set_bar_range */
    /* 0x94 */ void (*set_lines)(struct StstatusBar *obj, s32 shown, s32 lines); /* ststatus_set_bar_lines */
    /* 0x98 */ void (*set_line)(struct StstatusBar *obj, s32 line);          /* ststatus_set_bar_line */
} StstatusBar; /* size 0x9C */

StstatusBar *ststatus_create_bar(void);

/* ststatus_module: the stage module (ststatus_80099B6C.c): its state and its function table. The
 * other files reach the table through the struct (ststatus_module + 0x670). */
typedef struct StstatusModule {
    /* 0x000 */ StstatusAnim *anims;   /* ststatus_anims: per Digimon */
    /* 0x004 */ StstatusLayout *party_layout; /* ststatus_party_layout: the party page's windows */
    /* 0x008 */ StstatusLayout *area_layout; /* ststatus_area_layout: the map's areas (from 1): sprite, position */
    /* 0x00C */ StstatusPoint *mark_positions; /* ststatus_mark_positions: the map marks' positions (unk_680) */
    /* 0x010 */ s16 list_2[0x194];  /* list 2 (records's records_funcs.list_items(2, ...)) */
    /* 0x338 */ s32 list_2_count; /* entries in list_2 */
    /* 0x33C */ s16 list_3[0x194];  /* list 3 */
    /* 0x664 */ s32 list_3_count; /* entries in list_3 */
    /* 0x668 */ void (*load_files)(void);                                     /* ststatus_load_files */
    /* 0x66C */ s32 (*is_loading)(void);                                      /* ststatus_is_loading */
    /* 0x670 */ void (*window_anim_start)(WindowAnim *anim, s32 open);        /* ststatus_window_anim_start */
    /* 0x674 */ s32 (*window_anim_update)(WindowAnim *anim);                  /* ststatus_window_anim_update */
    /* 0x678 */ void (*lerp_start)(Tween *lerp, s32 from, s32 to, s32 frames); /* ststatus_lerp_start */
    /* 0x67C */ s32 (*lerp_update)(Tween *lerp);                       /* ststatus_lerp_update */
    /* 0x680 */ void *(*get_kind_list)(s32 arg0, s32 arg1);                   /* ststatus_get_kind_list */
    /* 0x684 */ s32 (*list_items)(s32 arg0, u16 *out);                        /* ststatus_list_items */
    /* 0x688 */ s32 (*can_equip)(s32 arg0, s32 arg1, s32 arg2);               /* ststatus_can_equip */
    /* 0x68C */ s32 (*equip_item)();                                          /* ststatus_equip_item */
} StstatusModule; /* size 0x690 */

extern StstatusModule ststatus_module;

/* ststatus_map: the map-area functions (ststatus_80099B6C.c). */
typedef struct StstatusMap {
    /* 0x0 */ s32 (*get_region)(void);  /* ststatus_map_get_region: the region (0, 1; -1 past both) */
    /* 0x4 */ s32 (*get_area)(void);    /* ststatus_map_get_area: the area */
    /* 0x8 */ void (*mark_visited)(s32 *out); /* ststatus_map_mark_visited: marks the region's visited areas */
} StstatusMap; /* size 0xC */

extern StstatusMap ststatus_map;

#endif /* STSTATUS_H */
