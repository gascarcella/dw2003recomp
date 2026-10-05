#include "common.h"
#include "gfx.h"
#include "cdload.h"
#include "sound.h"
#include "records.h"
#include "stcrdshp.h"

/* The overlay's helper table stcrdshp_util and its functions (as STCRDDEK's stcrddek_8008A100.c; only the
 * files loaded differ), then the shops' stock and the card prices. */

/* A card's price (stcrdshp_prices, ended by card == 0). */
typedef struct StcrdshpPrice {
    /* 0x0 */ s16 card;  /* card ID */
    /* 0x2 */ s16 price; /* price */
} StcrdshpPrice; /* size 0x4 */

extern StcrdshpPrice stcrdshp_prices[];
extern StcrdshpShop stcrdshp_shops[];

/* Uploads the screen's image (0x0642) and starts loading the language's text files. */
void stcrdshp_load_files(void) {
    Tim tim;

    tim_init(&tim);
    tim.set_image_pos(0x280, 0);
    tim.load_all(cdload_module.get_subfile_by_id(0x06420000));
    cdload_module.queue_file(records_language + 0x32);
    cdload_module.queue_file(records_language + 0x16);
    cdload_module.queue_file(records_language + 0x1D);
    cdload_module.queue_file(records_language + 0x94);
    cdload_module.queue_file(records_language + 0x6A);
}

/* 1 while one of them is still loading. */
s32 stcrdshp_is_loading(void) {
    if (cdload_module.is_loading(records_language + 0x32)) {
        return 1;
    }
    if (cdload_module.is_loading(records_language + 0x16)) {
        return 1;
    }
    if (cdload_module.is_loading(records_language + 0x1D)) {
        return 1;
    }
    if (cdload_module.is_loading(records_language + 0x94)) {
        return 1;
    }
    return cdload_module.is_loading(records_language + 0x6A) != 0;
}

/* window_anim.h's functions, as non-static copies for the table. */
void stcrdshp_window_anim_start(WindowAnim *anim, s32 open) {
    anim->running = 1;
    if (open) {
        sound_module.play(0x40019);
        anim->step = 0x1000 / anim->duration;
        anim->level = 0;
    } else {
        sound_module.play(0x4001A);
        anim->level = 0x1000;
        anim->step = -(0x1000 / anim->duration * 2);
    }
}

s32 stcrdshp_window_anim_update(WindowAnim *anim) {
    if (anim->running == 0) {
        return 1;
    }
    anim->level += anim->step;
    if (anim->step > 0) {
        if (anim->level > 0x1000) {
            anim->level = 0x1000;
            anim->running = 0;
            return 1;
        }
    } else if (anim->level < 0) {
        anim->level = 0;
        anim->running = 0;
        return 1;
    }
    return 0;
}

void stcrdshp_lerp_start(Tween *lerp, s32 from, s32 to, s32 frames) {
    if (from != to) {
        lerp->duration = frames;
        lerp->acc = from << 8;
        lerp->value = from;
        lerp->target = to;
        lerp->running = 1;
        lerp->step = ((to - from) << 8) / lerp->duration;
    }
}

/* Steps the value; returns 1 once it is at the end (or not running). */
s32 stcrdshp_lerp_update(Tween *lerp) {
    if (lerp->running == 0) {
        return 1;
    }
    lerp->acc += lerp->step;
    lerp->value = lerp->acc >> 8;
    if (lerp->step > 0) {
        if (lerp->value > lerp->target) {
            lerp->value = lerp->target;
            lerp->running = 0;
            return 1;
        }
    } else if (lerp->value < lerp->target) {
        lerp->value = lerp->target;
        lerp->running = 0;
        return 1;
    }
    return 0;
}

/* The stock of shop `id` (the first shop if there is none), with its cards counted. */
StcrdshpShop *stcrdshp_find_shop(s32 id) {
    s32 i;
    s32 found;

    found = -1;
    for (i = 0; stcrdshp_shops[i].id != -1; i++) {
        if (stcrdshp_shops[i].id == id) {
            found = i;
            break;
        }
    }
    if (found == -1) {
        found = 0;
    }
    stcrdshp_shops[found].count = 0;
    for (i = 0; stcrdshp_shops[found].cards[i] != 0; i++) {
        stcrdshp_shops[found].count++;
    }
    return &stcrdshp_shops[found];
}

/* The price of `card` (1 if it has none). */
s16 stcrdshp_get_price(s32 card) {
    s32 i;

    for (i = 0; stcrdshp_prices[i].card != 0; i++) {
        if (stcrdshp_prices[i].card == card) {
            return stcrdshp_prices[i].price;
        }
    }
    return 1;
}

StageUtil stcrdshp_util = {
    stcrdshp_load_files,
    stcrdshp_is_loading,
    stcrdshp_window_anim_start,
    stcrdshp_window_anim_update,
    stcrdshp_lerp_start,
    stcrdshp_lerp_update,
};

StcrdshpStock stcrdshp_stock = { stcrdshp_find_shop, stcrdshp_get_price };

/* The shops' cards (ended by 0). */
s16 stcrdshp_stock_0[] = { 65, 235, 277, 194, 204, 59, 0 };
s16 stcrdshp_stock_1[] = { 10, 210, 80, 168, 30, 59, 0 };
s16 stcrdshp_stock_2[] = { 209, 167, 82, 128, 30, 59, 0 };
s16 stcrdshp_stock_3[] = { 75, 202, 164, 121, 27, 59, 0 };
s16 stcrdshp_stock_4[] = { 115, 157, 73, 204, 249, 27, 0 };
s16 stcrdshp_stock_5[] = { 201, 116, 153, 284, 27, 30, 0 };
s16 stcrdshp_stock_6[] = { 96, 226, 140, 185, 265, 259, 0 };
s16 stcrdshp_stock_7[] = { 209, 167, 252, 82, 128, 259, 0 };
s16 stcrdshp_stock_8[] = { 72, 115, 157, 73, 204, 249, 0 };
s16 stcrdshp_stock_9[] = { 72, 115, 157, 73, 204, 285, 0 };
s16 stcrdshp_stock_10[] = { 96, 10, 210, 80, 168, 30, 0 };
s16 stcrdshp_stock_11[] = { 209, 167, 252, 82, 128, 259, 0 };
s16 stcrdshp_stock_12[] = { 209, 72, 201, 116, 204, 249, 0 };
s16 stcrdshp_stock_13[] = { 72, 115, 157, 73, 204, 285, 0 };
s16 stcrdshp_stock_14[] = { 209, 167, 252, 82, 128, 259, 0 };
s16 stcrdshp_stock_15[] = { 72, 115, 157, 73, 204, 249, 0 };
s16 stcrdshp_stock_16[] = { 209, 72, 201, 116, 204, 249, 0 };
s16 stcrdshp_stock_17[] = { 72, 201, 116, 157, 153, 284, 0 };
s16 stcrdshp_stock_18[] = { 65, 106, 150, 194, 277, 235, 0 };
s16 stcrdshp_stock_19[] = { 30, 75, 202, 164, 121, 27, 0 };
s16 stcrdshp_stock_20[] = { 156, 75, 202, 109, 153, 285, 0 };
s16 stcrdshp_stock_21[] = { 115, 157, 73, 204, 249, 30, 0 };
s16 stcrdshp_stock_22[] = { 156, 70, 200, 109, 285, 27, 0 };
s16 stcrdshp_stock_23[] = { 116, 285, 66, 106, 150, 194, 0 };

StcrdshpPrice stcrdshp_prices[45] = {
    { 10, 2200 }, { 27, 11000 }, { 30, 4000 }, { 59, 1000 }, { 65, 10000 }, { 66, 10000 },
    { 70, 7000 }, { 72, 7700 }, { 73, 7700 }, { 75, 6600 }, { 80, 2200 }, { 82, 3000 },
    { 96, 500 }, { 106, 10000 }, { 109, 8000 }, { 115, 7700 }, { 116, 7700 }, { 121, 4000 },
    { 128, 3000 }, { 140, 500 }, { 150, 10000 }, { 153, 8000 }, { 156, 7000 }, { 157, 7700 },
    { 164, 4000 }, { 167, 3300 }, { 168, 2200 }, { 185, 500 }, { 194, 10000 }, { 200, 7000 },
    { 201, 7700 }, { 202, 6600 }, { 204, 5500 }, { 209, 4400 }, { 210, 2200 }, { 226, 500 },
    { 235, 13000 }, { 249, 6600 }, { 252, 4400 }, { 259, 1000 }, { 265, 500 }, { 277, 10000 },
    { 284, 9000 }, { 285, 9000 }, { 0, 1 },
};

StcrdshpShop stcrdshp_shops[25] = {
    { 49, 0, stcrdshp_stock_0 },
    { 50, 0, stcrdshp_stock_1 },
    { 51, 0, stcrdshp_stock_2 },
    { 52, 0, stcrdshp_stock_3 },
    { 53, 0, stcrdshp_stock_4 },
    { 54, 0, stcrdshp_stock_5 },
    { 55, 0, stcrdshp_stock_6 },
    { 56, 0, stcrdshp_stock_7 },
    { 57, 0, stcrdshp_stock_8 },
    { 58, 0, stcrdshp_stock_9 },
    { 59, 0, stcrdshp_stock_10 },
    { 60, 0, stcrdshp_stock_11 },
    { 61, 0, stcrdshp_stock_12 },
    { 62, 0, stcrdshp_stock_13 },
    { 63, 0, stcrdshp_stock_14 },
    { 64, 0, stcrdshp_stock_15 },
    { 65, 0, stcrdshp_stock_16 },
    { 66, 0, stcrdshp_stock_17 },
    { 67, 0, stcrdshp_stock_18 },
    { 70, 0, stcrdshp_stock_19 },
    { 71, 0, stcrdshp_stock_20 },
    { 72, 0, stcrdshp_stock_21 },
    { 73, 0, stcrdshp_stock_22 },
    { 74, 0, stcrdshp_stock_23 },
    { -1, 0, NULL },
};
