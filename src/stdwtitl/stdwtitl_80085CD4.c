#include "common.h"
#include "object.h"
#include "gfx.h"
#include "cdload.h"
#include "gamestate.h"
#include "sound.h"
#include "pad.h"
#include "memcard.h"
#include "records.h"
#include "psyq/libgpu.h"
#include "psyq/libgte.h"
#include "stdwtitl.h"

/* Object of stdwtitl_flash_update, created by stdwtitl_flash_create: a white flash over the screen
 * (radial: brighter at the edges), started by unk_58 and finished when unk_5C returns non-zero. */
typedef struct StdwtitlFlash {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 done;   /* done */
    /* 0x54 */ s16 level;  /* level 0..0xFF */
    /* 0x56 */ s16 frames; /* frames */
    /* 0x58 */ void (*start)(struct StdwtitlFlash *); /* stdwtitl_flash_start */
    /* 0x5C */ s32 (*is_done)(struct StdwtitlFlash *); /* stdwtitl_flash_is_done */
} StdwtitlFlash; /* size 0x60 */

/* The outer corners of the four triangles of stdwtitl_draw_flash. */
typedef struct StdwtitlFlashTriangle {
    /* 0x0 */ s16 x1;
    /* 0x2 */ s16 y1;
    /* 0x4 */ s16 x2;
    /* 0x6 */ s16 y2;
} StdwtitlFlashTriangle; /* size 0x8 */

extern StdwtitlFlashTriangle stdwtitl_flash_triangles[4];

/* The flash's level at frame `t`: 0 -> 0xFF over 30 frames along a sine. */
s32 stdwtitl_get_flash_level(s32 t) {
    if (t >= 30) {
        return 0xFF;
    }
    return rsin((t << 10) / 30) * 30 / 4096 * 0xFF / 30;
}

void stdwtitl_draw_flash(s32 level) {
    GfxLayer *layer;
    u32 *ot;
    POLY_G3 *poly;
    DR_TPAGE *tpage;
    s32 edge;
    s32 i;

    layer = gfx_module.funcs.get_layer(0x1000);
    ot = layer->get_ot_entry(layer, 0);
    poly = gfx_module.funcs.get_packet();
    edge = level * 2;
    for (i = 0; i < 4; i++) {
        setPolyG3(poly);
        setSemiTrans(poly, 1);
        poly->r0 = poly->g0 = poly->b0 = level;
        if (edge >= 0x100) {
            edge = 0xFF;
        }
        poly->x0 = 0xA0;
        poly->r1 = poly->g1 = poly->b1 = poly->r2 = poly->g2 = poly->b2 = edge;
        poly->y0 = 0x78;
        poly->x1 = stdwtitl_flash_triangles[i].x1;
        poly->y1 = stdwtitl_flash_triangles[i].y1;
        poly->x2 = stdwtitl_flash_triangles[i].x2;
        poly->y2 = stdwtitl_flash_triangles[i].y2;
        addPrim(ot, poly);
        poly++;
    }
    tpage = (DR_TPAGE *)poly;
    setDrawTPage(tpage, 0, 1, getTPage(0, 2, 320, 0));
    addPrim(ot, tpage);
    gfx_module.funcs.set_packet(tpage + 1);
}

void stdwtitl_flash_start(StdwtitlFlash *obj) {
    obj->done = 0;
    obj->frames = 0;
    obj->base.set_step(obj, 1);
}

s32 stdwtitl_flash_is_done(StdwtitlFlash *obj) {
    return obj->done;
}

void stdwtitl_flash_update(StdwtitlFlash *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->level = 0;
        obj->frames = 0;
        obj->done = 0;
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            obj->level = stdwtitl_get_flash_level(obj->frames++);
            if (obj->frames >= 30) {
                obj->base.set_step(obj, 0);
                obj->done = 1;
            }
            break;
        }
        stdwtitl_draw_flash(obj->level);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StdwtitlFlash *stdwtitl_flash_create(void) {
    StdwtitlFlash *obj = object_new(stdwtitl_flash_update, sizeof(StdwtitlFlash), 0);

    obj->start = stdwtitl_flash_start;
    obj->is_done = stdwtitl_flash_is_done;
    return obj;
}

/* A step of an animation: a sprite (0xFF ends the list, 300 draws nothing) shown for some ticks. */
typedef struct StdwtitlAnimFrame {
    /* 0x0 */ s16 sprite;
    /* 0x2 */ s16 ticks;
} StdwtitlAnimFrame; /* size 0x4 */

/* An animation's position in its list of steps. */
typedef struct StdwtitlAnim {
    /* 0x0 */ s16 index;
    /* 0x2 */ s16 ticks; /* left */
} StdwtitlAnim; /* size 0x4 */

/* Object of stdwtitl_background_update, created by stdwtitl_background_create: the title's background, three
 * pictures and eight animations. */
typedef struct StdwtitlBackground {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 unk_50; /* create's argument (0); nothing reads it */
    /* 0x54 */ StdwtitlAnim anims[8];
    /* 0x74 */ s16 anim6_x; /* x, y of animation 6 */
    /* 0x76 */ s16 anim6_y;
    /* 0x78 */ s16 anim7_x; /* x, y of animation 7 */
    /* 0x7A */ s16 anim7_y;
    /* 0x7C */ s32 layer_id; /* 0x1000 */
    /* 0x80 */ s32 ot_depth; /* ordering table depth: set to 2, never read */
    /* 0x84 */ u8 unk_84[0x10];
    /* 0x94 */ void (*start)(struct StdwtitlBackground *);
} StdwtitlBackground; /* size 0x98 */

extern StdwtitlAnimFrame stdwtitl_background_anim_0[];
extern StdwtitlAnimFrame stdwtitl_background_anim_1[];
extern StdwtitlAnimFrame stdwtitl_background_anim_2[];
extern StdwtitlAnimFrame stdwtitl_background_anim_3[];
extern StdwtitlAnimFrame stdwtitl_background_anim_4[];
extern StdwtitlAnimFrame stdwtitl_background_anim_5[];
extern StdwtitlAnimFrame stdwtitl_background_anim_6[];
extern StdwtitlAnimFrame stdwtitl_background_anim_7[];
/* A position. */
typedef struct StdwtitlPos {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
} StdwtitlPos; /* size 0x4 */

extern StdwtitlPos stdwtitl_anim6_positions[3]; /* where animation 6 shows */
extern StdwtitlPos stdwtitl_anim7_positions[5]; /* where animation 7 shows */

/* Steps animation `anim` of `frames` (`depth` 0: a new tick) and returns its sprite. */
s32 stdwtitl_step_anim(StdwtitlAnim *anim, StdwtitlAnimFrame *frames, s32 depth) {
    StdwtitlAnimFrame *frame = &frames[anim->index];

    if (depth == 0) {
        anim->ticks--;
    }
    if (anim->ticks <= 0) {
        frame++;
        anim->index++;
        anim->ticks += frame->ticks;
        if (frame->sprite == 0xFF) {
            frame = frames;
            anim->index = 0;
            anim->ticks += frame->ticks;
        }
        stdwtitl_step_anim(anim, frames, depth + 1);
    }
    return frame->sprite;
}

void stdwtitl_background_draw_pictures(StdwtitlBackground *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, 0);
    spr.set_clut8_pos(0, 0x1F3);
    spr.set_vram_pos(0x300, 0);
    spr.draw(cdload_module.get_subfile_by_id(0x08870000), 0, 0, 0);
    spr.set_vram_pos(0x340, 0);
    spr.draw(cdload_module.get_subfile_by_id(0x08870001), 0, 0, 0);
    spr.set_vram_pos(0x380, 0);
    spr.draw(cdload_module.get_subfile_by_id(0x08870002), 0, 0, 0);
}

void stdwtitl_background_draw_anims(StdwtitlBackground *obj) {
    Sprite spr;
    s32 s0, s1, s2, s3, s4, s5, s6, s7;

    s0 = stdwtitl_step_anim(&obj->anims[0], stdwtitl_background_anim_0, 0);
    s1 = stdwtitl_step_anim(&obj->anims[1], stdwtitl_background_anim_1, 0);
    s2 = stdwtitl_step_anim(&obj->anims[2], stdwtitl_background_anim_2, 0);
    s3 = stdwtitl_step_anim(&obj->anims[3], stdwtitl_background_anim_3, 0);
    s4 = stdwtitl_step_anim(&obj->anims[4], stdwtitl_background_anim_4, 0);
    s5 = stdwtitl_step_anim(&obj->anims[5], stdwtitl_background_anim_5, 0);
    s6 = stdwtitl_step_anim(&obj->anims[6], stdwtitl_background_anim_6, 0);
    s7 = stdwtitl_step_anim(&obj->anims[7], stdwtitl_background_anim_7, 0);
    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, 0);
    spr.set_vram_pos(0x3C0, 0);
    if (s0 != 300) {
        spr.draw(cdload_module.get_subfile_by_id(0x08870003), s0, 0, 0);
    }
    if (s1 != 300) {
        spr.draw(cdload_module.get_subfile_by_id(0x08870003), s1, 0, 0);
    }
    if (s2 != 300) {
        spr.draw(cdload_module.get_subfile_by_id(0x08870003), s2, 0, 0);
    }
    if (s3 != 300) {
        spr.draw(cdload_module.get_subfile_by_id(0x08870003), s3, 0, 0);
    }
    if (s4 != 300) {
        spr.draw(cdload_module.get_subfile_by_id(0x08870008), s4, 0, 0);
    }
    if (s5 != 300) {
        spr.draw(cdload_module.get_subfile_by_id(0x08870008), s5, 0, 0);
    }
    if (s6 != 300) {
        spr.draw(cdload_module.get_subfile_by_id(0x08870008), s6, obj->anim6_x, obj->anim6_y);
    }
    if (s7 != 300) {
        spr.draw(cdload_module.get_subfile_by_id(0x08870008), s7, obj->anim7_x, obj->anim7_y);
    }
}

void stdwtitl_background_update(StdwtitlBackground *obj) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->anims[0].index = 0;
            obj->anims[0].ticks = stdwtitl_background_anim_0[0].ticks;
            obj->anims[1].index = 0;
            obj->anims[1].ticks = stdwtitl_background_anim_1[0].ticks;
            obj->anims[2].index = 0;
            obj->anims[2].ticks = stdwtitl_background_anim_2[0].ticks;
            obj->anims[3].index = 0;
            obj->anims[3].ticks = stdwtitl_background_anim_3[0].ticks;
            obj->anims[4].index = 0;
            obj->anims[4].ticks = stdwtitl_background_anim_4[0].ticks;
            obj->anims[5].index = 0;
            obj->anims[5].ticks = stdwtitl_background_anim_5[0].ticks;
            obj->anims[6].index = 0;
            obj->anims[6].ticks = stdwtitl_background_anim_6[0].ticks + pad_random.next() % 300;
            obj->anims[7].index = 0;
            obj->anims[7].ticks = stdwtitl_background_anim_7[0].ticks + pad_random.next() % 240;
            i = pad_random.next() % 3;
            obj->anim6_x = stdwtitl_anim6_positions[i].x;
            obj->anim6_y = stdwtitl_anim6_positions[i].y;
            i = pad_random.next() % 5;
            obj->anim7_x = stdwtitl_anim7_positions[i].x;
            obj->anim7_y = stdwtitl_anim7_positions[i].y;
            obj->base.set_step(obj, 1);
        }
        stdwtitl_background_draw_anims(obj);
        /* fallthrough */
    case OBJECT_STATE_RUN:
        stdwtitl_background_draw_pictures(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void stdwtitl_background_start(StdwtitlBackground *obj) {
    if (obj->base.state == OBJECT_STATE_RUN) {
        obj->base.set_state(obj, OBJECT_STATE_DONE);
    }
}

StdwtitlBackground *stdwtitl_background_create(s32 arg0) {
    StdwtitlBackground *obj = object_new(stdwtitl_background_update, sizeof(StdwtitlBackground), 0);

    obj->start = stdwtitl_background_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->unk_50 = arg0;
    return obj;
}

/* Data block of the title menu (stdwtitl_title_update): the objects it made. */
typedef struct StdwtitlTitleData {
    /* 0x00 */ StdwtitlFlash *flash; /* the flash */
    /* 0x04 */ StdwtitlShine *shine;
    /* 0x08 */ StdwtitlSlide *slide;
    /* 0x0C */ StdwtitlSlideLeft *slide_left;
    /* 0x10 */ StdwtitlGlow *glow;
    /* 0x14 */ StdwtitlMenu *menu;  /* the menu */
    /* 0x18 */ StdwtitlBackground *background; /* the background */
} StdwtitlTitleData;

/* Object of stdwtitl_title_update (the title menu), created by stdwtitl_title_create. */
typedef struct StdwtitlTitle {
    /* 0x000 */ Object base;
    /* 0x050 */ Object *parent; /* the parent (stdwtitl_title_screen_update's object) */
    /* 0x054 */ s32 ticks;           /* ticks */
    /* 0x058 */ s32 choice;          /* the menu's result */
    /* 0x05C */ s32 layer_id;        /* 0x1000 */
    /* 0x060 */ s32 ot_depth; /* ordering table depth: set to 2, never read */
    /* 0x064 */ u8 unk_64[0xD8];
} StdwtitlTitle; /* size 0x13C */

/* Once the flash is over, goes on with the menu's choice; returns 1 then. */
s32 stdwtitl_title_apply_choice(StdwtitlTitle *obj, StdwtitlTitleData *data) {
    s32 ret = 0;

    if (data->flash->is_done(data->flash) != 0) {
        switch (obj->choice) {
        case 0:
            break;
        case 1:
            ret = 1;
            gamestate_data.funcs.new_game();
            gamestate_data.funcs.reset_playtime();
            records_state.clear_gauges();
            memcard_funcs.set_file_name();
            gamestate_data.funcs.set_next_map(0x2D7, 0);
            break;
        case 2:
            ret = 1;
            gamestate_data.funcs.new_game();
            records_state.clear_gauges();
            memcard_funcs.set_file_name();
            gamestate_data.funcs.set_next_map(0xC00, 0);
            break;
        case 3:
            if (records_language == 0) {
                if (gamestate_data.funcs.get_prev_map() == 0xE01) {
                    gamestate_data.funcs.set_next_map(0xE02, 0);
                } else {
                    gamestate_data.funcs.set_next_map(0xE01, 0);
                }
            } else {
                gamestate_data.funcs.set_next_map(0xE02, 0);
            }
            ret = 1;
            break;
        }
    }
    return ret;
}

s32 stdwtitl_title_run(StdwtitlTitle *obj, StdwtitlTitleData *data) {
    s32 ret = 0;

    switch (obj->base.step) {
    case 0:
    default:
        data->slide_left->start(data->slide_left);
        data->slide->start(data->slide);
        obj->ticks = 0;
        obj->base.set_step(obj, 2);
        break;
    case 1:
        if (obj->ticks++ >= 0) {
            obj->ticks = 0;
            data->slide->start(data->slide);
            obj->base.next_step(obj);
        }
        break;
    case 2:
        if (obj->ticks >= 10) {
            obj->ticks = 0;
            sound_module.play(0x611C0000);
            data->shine->start(data->shine);
            obj->base.next_step(obj);
        }
        obj->ticks += gfx_module.funcs.get_frame_ticks();
        break;
    case 3:
        if (obj->ticks >= 12) {
            obj->ticks = 0;
            obj->base.next_step(obj);
        }
        obj->ticks += gfx_module.funcs.get_frame_ticks();
        break;
    case 4:
        if (obj->ticks >= 12) {
            obj->ticks = 0;
            data->glow->start(data->glow);
            obj->base.next_step(obj);
        }
        obj->ticks += gfx_module.funcs.get_frame_ticks();
        break;
    case 5:
        if (obj->ticks >= 12) {
            obj->ticks = 0;
            data->menu->start(data->menu);
            data->background->start(data->background);
            obj->base.next_step(obj);
        }
        obj->ticks += gfx_module.funcs.get_frame_ticks();
        break;
    case 6:
        obj->choice = data->menu->get_result(data->menu);
        if (obj->choice != 0) {
            obj->base.next_step(obj);
            data->flash = stdwtitl_flash_create();
            if (obj->choice == 1) {
                cdload_module.queue_file(0x166);
            }
        }
        break;
    case 7:
        data->flash->start(data->flash);
        obj->base.next_step(obj);
        break;
    case 8:
        ret = stdwtitl_title_apply_choice(obj, data);
        break;
    }
    return ret;
}

void stdwtitl_title_update(StdwtitlTitle *obj, StdwtitlTitleData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        data->background = stdwtitl_background_create(0);
        if (records_language == 0) {
            data->slide_left = stdwtitl_slide_left_create_jp(0);
            data->slide = stdwtitl_slide_create_jp(0);
            data->glow = stdwtitl_glow_create(0);
            data->shine = stdwtitl_shine_create_jp(0);
            data->menu = stdwtitl_menu_create(0);
        } else {
            data->slide_left = stdwtitl_slide_left_create_intl(0);
            data->slide = stdwtitl_slide_create_intl(0);
            data->shine = stdwtitl_shine_create_intl(0);
            data->glow = stdwtitl_glow_create(0);
            data->menu = stdwtitl_menu_create(0);
        }
        obj->ticks = 0;
        break;
    case OBJECT_STATE_RUN:
        if (stdwtitl_title_run(obj, data) != 0) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        sound_module.stop(0x611C0000);
        break;
    }
}

/* The title menu, made by stdwtitl_title_screen_run (`parent`). */
Object *stdwtitl_title_create(Object *parent) {
    StdwtitlTitle *obj = object_new(stdwtitl_title_update, sizeof(StdwtitlTitle), sizeof(StdwtitlTitleData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->parent = parent;
    return &obj->base;
}

StdwtitlFlashTriangle stdwtitl_flash_triangles[4] = {
    { 0, -15, 320, -15 },
    { 320, -15, 320, 260 },
    { 0, 260, 320, 260 },
    { 0, -15, 0, 260 },
};

StdwtitlAnimFrame stdwtitl_background_anim_0[] = {
    { 300, 180 }, { 0, 2 }, { 1, 3 }, { 2, 5 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 6, 5 },
    { 7, 4 }, { 8, 4 }, { 9, 4 }, { 10, 4 }, { 11, 4 }, { 12, 4 }, { 13, 4 }, { 300, 12 },
    { 255, 0 },
};

StdwtitlAnimFrame stdwtitl_background_anim_1[] = {
    { 300, 240 }, { 83, 1 }, { 14, 1 }, { 82, 1 }, { 84, 2 }, { 85, 2 }, { 86, 2 }, { 87, 2 },
    { 14, 6 }, { 88, 2 }, { 89, 2 }, { 90, 3 }, { 91, 3 }, { 92, 3 }, { 93, 3 }, { 94, 3 },
    { 95, 3 }, { 96, 3 }, { 97, 3 }, { 300, 10 }, { 255, 0 },
};

StdwtitlAnimFrame stdwtitl_background_anim_2[] = {
    { 15, 1 }, { 14, 1 }, { 16, 1 }, { 14, 1 }, { 17, 1 }, { 14, 1 }, { 18, 1 }, { 24, 1 },
    { 19, 1 }, { 25, 1 }, { 20, 1 }, { 26, 1 }, { 21, 1 }, { 27, 1 }, { 22, 1 }, { 28, 1 },
    { 23, 1 }, { 29, 1 }, { 33, 1 }, { 30, 1 }, { 34, 1 }, { 31, 1 }, { 35, 1 }, { 32, 1 },
    { 36, 1 }, { 14, 1 }, { 37, 1 }, { 49, 1 }, { 38, 1 }, { 48, 1 }, { 39, 1 }, { 47, 1 },
    { 40, 1 }, { 46, 1 }, { 14, 1 }, { 45, 1 }, { 14, 1 }, { 44, 1 }, { 14, 1 }, { 43, 1 },
    { 14, 1 }, { 42, 1 }, { 14, 1 }, { 41, 1 }, { 300, 20 }, { 255, 0 },
};

StdwtitlAnimFrame stdwtitl_background_anim_3[] = {
    { 300, 25 }, { 50, 1 }, { 14, 1 }, { 51, 1 }, { 14, 1 }, { 52, 1 }, { 14, 1 }, { 53, 1 },
    { 14, 1 }, { 54, 1 }, { 14, 1 }, { 55, 1 }, { 14, 1 }, { 56, 1 }, { 14, 1 }, { 57, 1 },
    { 14, 1 }, { 58, 1 }, { 66, 1 }, { 59, 1 }, { 67, 1 }, { 60, 1 }, { 68, 1 }, { 61, 1 },
    { 69, 1 }, { 62, 1 }, { 70, 1 }, { 63, 1 }, { 71, 1 }, { 64, 1 }, { 72, 1 }, { 65, 1 },
    { 73, 1 }, { 14, 1 }, { 74, 1 }, { 14, 1 }, { 75, 1 }, { 14, 1 }, { 76, 1 }, { 14, 1 },
    { 77, 1 }, { 14, 1 }, { 78, 1 }, { 14, 1 }, { 79, 1 }, { 14, 1 }, { 80, 1 }, { 14, 1 },
    { 81, 1 }, { 300, 30 }, { 255, 0 },
};

StdwtitlAnimFrame stdwtitl_background_anim_4[] = {
    { 300, 240 }, { 27, 1 }, { 5, 1 }, { 28, 1 }, { 5, 1 }, { 29, 1 }, { 5, 1 }, { 30, 1 },
    { 5, 1 }, { 31, 1 }, { 5, 1 }, { 32, 1 }, { 5, 1 }, { 33, 1 }, { 5, 1 }, { 34, 1 },
    { 5, 1 }, { 35, 1 }, { 5, 1 }, { 36, 1 }, { 5, 1 }, { 37, 1 }, { 5, 1 }, { 38, 1 },
    { 5, 1 }, { 39, 1 }, { 5, 1 }, { 40, 1 }, { 5, 1 }, { 13, 1 }, { 14, 1 }, { 15, 1 },
    { 16, 1 }, { 17, 1 }, { 40, 1 }, { 18, 1 }, { 39, 1 }, { 19, 1 }, { 38, 1 }, { 20, 1 },
    { 37, 1 }, { 21, 1 }, { 36, 1 }, { 22, 1 }, { 35, 1 }, { 23, 1 }, { 34, 1 }, { 24, 1 },
    { 33, 1 }, { 25, 1 }, { 32, 1 }, { 26, 1 }, { 31, 1 }, { 5, 1 }, { 30, 1 }, { 5, 1 },
    { 29, 1 }, { 5, 1 }, { 28, 1 }, { 5, 1 }, { 27, 1 }, { 300, 60 }, { 255, 0 },
};

StdwtitlAnimFrame stdwtitl_background_anim_5[] = {
    { 300, 180 }, { 41, 2 }, { 5, 1 }, { 41, 1 }, { 5, 1 }, { 42, 2 }, { 5, 1 }, { 42, 1 },
    { 5, 1 }, { 43, 2 }, { 5, 1 }, { 43, 1 }, { 5, 1 }, { 44, 2 }, { 5, 1 }, { 44, 1 },
    { 5, 9 }, { 45, 2 }, { 5, 1 }, { 45, 1 }, { 5, 1 }, { 46, 2 }, { 5, 1 }, { 46, 1 },
    { 5, 1 }, { 47, 2 }, { 5, 1 }, { 47, 1 }, { 300, 180 }, { 255, 0 },
};

StdwtitlAnimFrame stdwtitl_background_anim_6[] = {
    { 300, 180 }, { 0, 1 }, { 5, 1 }, { 0, 2 }, { 5, 1 }, { 1, 2 }, { 5, 1 }, { 2, 2 },
    { 5, 1 }, { 2, 1 }, { 5, 1 }, { 3, 1 }, { 5, 1 }, { 3, 1 }, { 5, 1 }, { 4, 1 },
    { 5, 1 }, { 4, 1 }, { 300, 240 }, { 255, 0 },
};

StdwtitlAnimFrame stdwtitl_background_anim_7[] = {
    { 300, 120 }, { 6, 1 }, { 5, 1 }, { 7, 1 }, { 5, 1 }, { 6, 1 }, { 5, 1 }, { 7, 1 },
    { 5, 1 }, { 8, 1 }, { 5, 1 }, { 8, 1 }, { 5, 1 }, { 9, 1 }, { 5, 1 }, { 10, 1 },
    { 5, 1 }, { 11, 1 }, { 5, 1 }, { 12, 1 }, { 300, 240 }, { 255, 0 },
};

StdwtitlPos stdwtitl_anim6_positions[3] = {
    { 16, 165 },
    { 265, 85 },
    { 200, 0 },
};

StdwtitlPos stdwtitl_anim7_positions[5] = {
    { 150, 0 },
    { 282, 8 },
    { 290, 178 },
    { 0, 0 },
    { 0, 108 },
};
