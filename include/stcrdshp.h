#ifndef STCRDSHP_H
#define STCRDSHP_H

/* STCRDSHP.PRO: the card shop (buy cards, open booster packs). */

#include "common.h"
#include "object.h"
#include "window_anim_type.h"
#include "overlay_common.h"

extern StageUtil stcrdshp_util;

/* A shop's stock (stcrdshp_shops, ended by id == -1). */
typedef struct StcrdshpShop {
    /* 0x0 */ s32 id;     /* shop ID */
    /* 0x4 */ s32 count;  /* cards in cards (stcrdshp_find_shop counts them) */
    /* 0x8 */ s16 *cards;  /* card IDs, ended by 0 */
} StcrdshpShop; /* size 0xC */

StcrdshpShop *stcrdshp_find_shop(s32 id);
s16 stcrdshp_get_price(s32 card);

/* stcrdshp_stock: the shop data's functions, which the shop's files call through. */
typedef struct StcrdshpStock {
    /* 0x0 */ StcrdshpShop *(*find_shop)(s32 id); /* stcrdshp_find_shop */
    /* 0x4 */ s16 (*get_price)(s32 card);         /* stcrdshp_get_price */
} StcrdshpStock; /* size 0x8 */

extern StcrdshpStock stcrdshp_stock;

/* The shop's main object (stcrdshp_update_main). */
typedef struct StcrdshpMain {
    /* 0x000 */ Object base;
    /* 0x050 */ s32 layer_id; /* layer */
    /* 0x054 */ s32 ot_depth; /* ordering table entry */
    /* 0x058 */ s32 scroll; /* background scroll, 0..95 */
    /* 0x05C */ s32 odd_frame; /* toggles every frame: scroll every other frame */
    /* 0x060 */ s32 shop;   /* gamestate_data.funcs.get_map_entry(): the shop */
    /* 0x064 */ s32 greeting; /* the shopkeeper's greeting (stcrdshp_greetings) */
    /* 0x068 */ s32 left;   /* left through the menu's third choice */
    /* 0x06C */ s32 menu_cursor; /* menu cursor: buy, open boosters, leave */
    /* 0x070 */ s16 items[404];  /* the player's items (records_funcs.list_items) */
    /* 0x398 */ WindowAnim anims[3];
    /* 0x3C8 */ void (*update_money)(struct StcrdshpMain *obj); /* stcrdshp_update_money */
} StcrdshpMain; /* size 0x3CC */

StcrdshpMain *stcrdshp_create_main(void);

/* Six cards shown face down, then turned over one by one (stcrdshp_update_pack). */
typedef struct StcrdshpPack {
    /* 0x00 */ Object base;
    /* 0x50 */ StcrdshpMain *main;
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 ot_depth; /* ordering table entry */
    /* 0x5C */ u8 unk_5C[0x8];
    /* 0x64 */ s32 cards_shown; /* cards shown */
    /* 0x68 */ s32 backs_shown; /* backs shown */
    /* 0x6C */ s32 frame_time; /* time of the last frame */
    /* 0x70 */ s32 backs_frame; /* backs' animation frame */
    /* 0x74 */ u8 unk_74[0x8];
    /* 0x7C */ s32 cards[6];  /* the cards */
    /* 0x94 */ s32 prev_cards[6]; /* the cards before (stcrdshp_set_pack) */
    /* 0xAC */ u8 unk_AC[0x10];
    /* 0xBC */ void (*set)(struct StcrdshpPack *obj, s32 *cards); /* stcrdshp_set_pack */
    /* 0xC0 */ void (*open)(struct StcrdshpPack *obj);             /* stcrdshp_open_pack */
} StcrdshpPack; /* size 0xC4 */

StcrdshpPack *stcrdshp_create_pack(StcrdshpMain *main, s32 *cards);

#endif /* STCRDSHP_H */
