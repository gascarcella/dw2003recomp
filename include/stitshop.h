#ifndef STITSHOP_H
#define STITSHOP_H

/* STITSHOP.PRO (the item shop): types, module structs and functions shared by its files. */

#include "common.h"
#include "object.h"
#include "message.h"
#include "gamestate.h"
#include "overlay_common.h"

/* stitshop_funcs: the shop's item count and the overlay's helpers, called through this table. */
typedef struct StitshopFuncs {
    /* 0x00 */ s32 count; /* items of the shop (stitshop_get_items) */
    /* 0x04 */ void (*load)(void);                                              /* stitshop_load_files */
    /* 0x08 */ s32 (*is_loading)(void);                                         /* stitshop_is_loading */
    /* 0x0C */ void (*anim_start)(WindowAnim *anim, s32 open);          /* stitshop_anim_start */
    /* 0x10 */ s32 (*anim_update)(WindowAnim *anim);                    /* stitshop_anim_update */
    /* 0x14 */ void (*tween_start)(Tween *obj, s32 from, s32 to, s32 frames); /* stitshop_tween_start */
    /* 0x18 */ s32 (*tween_update)(Tween *obj);                         /* stitshop_tween_update */
    /* 0x1C */ s16 *(*get_items)(s32 shop);                                     /* stitshop_get_items */
    /* 0x20 */ s32 (*can_equip)(s32 digimon, s32 item);                         /* stitshop_can_equip */
    /* 0x24 */ s32 (*get_slot)(s32 digimon, s32 item);                          /* stitshop_get_slot */
    /* 0x28 */ void (*equip)(s32 digimon, s32 slot, s32 item, s32 take);        /* stitshop_equip */
} StitshopFuncs; /* size 0x2C */

extern StitshopFuncs stitshop_funcs;

/* A list of items (stitshop_list_create, size 0x704): the shop's goods or the player's items of a kind,
 * eight or fourteen to a page. */
typedef struct StitshopList {
    /* 0x000 */ Object base;
    /* 0x050 */ struct StitshopBuy *owner;  /* the owner: its owner shows the item under the cursor */
    /* 0x054 */ s32 layer_id; /* layer */
    /* 0x058 */ s32 ot_depth; /* ordering table depth */
    /* 0x05C */ s32 player_items; /* 0: the shop's goods, 1: the player's items */
    /* 0x060 */ s32 shop;   /* the shop, or the kind of item */
    /* 0x064 */ s16 owned_items[0x194]; /* the player's items of the kind */
    /* 0x38C */ s16 kind_items[0x194]; /* all the items of the kind (records_funcs.list_items) */
    /* 0x6B4 */ s16 *goods;   /* the shop's goods */
    /* 0x6B8 */ s32 input_enabled; /* input enabled */
    /* 0x6BC */ s32 cursor;  /* cursor */
    /* 0x6C0 */ s32 count;   /* items */
    /* 0x6C4 */ s32 page;    /* page */
    /* 0x6C8 */ s32 pages;   /* pages */
    /* 0x6CC */ s32 arrows_step; /* the page arrows' colour step */
    /* 0x6D0 */ s32 arrows_time; /* time of its last step */
    /* 0x6D4 */ s32 per_page; /* items per page */
    /* 0x6D8 */ WindowAnim frame_anim;
    /* 0x6E8 */ void (*resume)(struct StitshopList *obj);           /* stitshop_list_resume */
    /* 0x6EC */ void (*close)(struct StitshopList *obj);           /* stitshop_list_close */
    /* 0x6F0 */ s32 (*get_item)(struct StitshopList *obj); /* stitshop_list_get_item (defined returning s16) */
    /* 0x6F4 */ void (*set_input)(struct StitshopList *obj, s32 on);   /* stitshop_list_set_input */
    /* 0x6F8 */ void (*grey_cursor)(struct StitshopList *obj, s32 grey); /* stitshop_list_grey_cursor */
    /* 0x6FC */ void (*refresh)(struct StitshopList *obj);           /* stitshop_list_refresh */
    /* 0x700 */ void (*collect)(struct StitshopList *obj);           /* stitshop_list_collect */
} StitshopList; /* size 0x704 */

/* A party member's stats before and after equipping the item (StitshopInfo.stats). */
typedef struct StitshopStats {
    /* 0x00 */ GamestateStats now;    /* now */
    /* 0x2C */ GamestateStats with_item; /* with the item */
    /* 0x58 */ s32 slot;   /* the slot the item goes to */
    /* 0x5C */ s32 shown_count; /* stats shown */
    /* 0x60 */ s32 shown[8];  /* the stats shown */
} StitshopStats; /* size 0x80 */

/* The item's information (stitshop_info_create, size 0x248): price, owned, and what it does to each party
 * member. */
typedef struct StitshopInfo {
    /* 0x000 */ Object base;
    /* 0x050 */ s32 layer_id; /* layer */
    /* 0x054 */ s32 ot_depth; /* ordering table depth */
    /* 0x058 */ s32 selling; /* 0: buying, 1: selling */
    /* 0x05C */ s32 page;   /* page: 0 the description, 1 the party */
    /* 0x060 */ s32 member_count; /* party members */
    /* 0x064 */ StitshopStats stats[3];
    /* 0x1E4 */ s32 item;    /* the item */
    /* 0x1E8 */ s32 count;   /* how many */
    /* 0x1EC */ s32 can_equip; /* it can be equipped */
    /* 0x1F0 */ s32 shown;   /* shown */
    /* 0x1F4 */ WindowAnim item_anim;
    /* 0x204 */ WindowAnim party_anim;
    /* 0x214 */ WindowAnim text_anim;
    /* 0x224 */ WindowAnim members_anim;
    /* 0x234 */ void (*set_item)(struct StitshopInfo *obj, s32 item, s32 n); /* stitshop_info_set_item */
    /* 0x238 */ void (*close)(struct StitshopInfo *obj);                 /* stitshop_info_close */
    /* 0x23C */ void (*show_text)(struct StitshopInfo *obj, s32 show);       /* stitshop_info_show_text */
    /* 0x240 */ void (*turn_page)(struct StitshopInfo *obj);                 /* stitshop_info_turn_page */
    /* 0x244 */ void (*show_member)(struct StitshopInfo *obj, s32 member);     /* stitshop_info_show_member */
} StitshopInfo; /* size 0x248 */

/* The shop (stitshop_main_create, size 0xAC). */
typedef struct StitshopMain {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id; /* layer */
    /* 0x54 */ s32 unk_54;
    /* 0x58 */ s32 scroll; /* the background's scroll */
    /* 0x5C */ s32 odd_frame; /* the scroll moves every other frame */
    /* 0x60 */ s32 shop;   /* the shop */
    /* 0x64 */ s32 cursor; /* cursor: 0 buy, 1 sell */
    /* 0x68 */ WindowAnim shopkeeper_anim;
    /* 0x78 */ WindowAnim money_anim;
    /* 0x88 */ WindowAnim menu_anim;
    /* 0x98 */ WindowAnim help_anim;
    /* 0xA8 */ void (*show_money)(struct StitshopMain *obj); /* stitshop_main_show_money */
} StitshopMain; /* size 0xAC */

/* Buying (stitshop_buy_create, size 0xC8). */
typedef struct StitshopBuy {
    /* 0x00 */ Object base;
    /* 0x50 */ void (*set_item)(struct StitshopBuy *obj, s32 item, s32 n); /* stitshop_buy_set_item */
    /* 0x54 */ StitshopMain *main;
    /* 0x58 */ s32 layer_id; /* layer */
    /* 0x5C */ s32 ot_depth; /* ordering table depth */
    /* 0x60 */ s32 item;   /* the item */
    /* 0x64 */ s32 count;  /* how many */
    /* 0x68 */ s32 max;    /* at most */
    /* 0x6C */ s32 arrows_shown; /* the arrows blink */
    /* 0x70 */ s32 arrows_time; /* time of their last blink */
    /* 0x74 */ s32 cursor; /* cursor: 0 yes, 1 no */
    /* 0x78 */ s32 member; /* the party member to equip */
    /* 0x7C */ s32 member_cursor_shown; /* the party member cursor is shown */
    /* 0x80 */ s32 member_cursor_step; /* its colour step */
    /* 0x84 */ s32 member_cursor_time; /* time of its last step */
    /* 0x88 */ WindowAnim count_anim; /* how many */
    /* 0x98 */ WindowAnim question_anim; /* yes/no */
    /* 0xA8 */ WindowAnim message_anim; /* message */
    /* 0xB8 */ WindowAnim equipped_anim;
} StitshopBuy; /* size 0xC8 */

/* Selling (stitshop_sell_create, size 0xCC). */
typedef struct StitshopSell {
    /* 0x00 */ Object base;
    /* 0x50 */ void (*set_item)(struct StitshopSell *obj, s32 item, s32 n); /* stitshop_sell_set_item */
    /* 0x54 */ StitshopMain *main;
    /* 0x58 */ s32 layer_id; /* layer */
    /* 0x5C */ s32 ot_depth; /* ordering table depth */
    /* 0x60 */ s32 kind;   /* the kind of item (0..3) */
    /* 0x64 */ s32 item;   /* the item */
    /* 0x68 */ s32 count;  /* how many */
    /* 0x6C */ s32 max;    /* at most */
    /* 0x70 */ s32 arrows_shown; /* the arrows blink */
    /* 0x74 */ s32 arrows_time; /* time of their last blink */
    /* 0x78 */ s32 cursor; /* cursor: 0 yes, 1 no */
    /* 0x7C */ WindowAnim kinds_anim; /* the kinds */
    /* 0x8C */ WindowAnim count_anim; /* how many */
    /* 0x9C */ WindowAnim question_anim; /* yes/no */
    /* 0xAC */ WindowAnim unk_AC;
    /* 0xBC */ WindowAnim nothing_anim; /* "nothing to sell" */
} StitshopSell; /* size 0xCC */

/* stitshop_8008321C.c */
Fade *stitshop_fade_create(void);
StitshopBuy *stitshop_buy_create(StitshopMain *main);

/* stitshop_800859C0.c */
StitshopSell *stitshop_sell_create(StitshopMain *main);
StitshopList *stitshop_list_create(void *owner, s32 kind, s32 mode);
StitshopInfo *stitshop_info_create(s32 mode, s32 item);
StitshopMain *stitshop_main_create(void);

#endif /* STITSHOP_H */
