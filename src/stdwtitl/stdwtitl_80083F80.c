#include "common.h"
#include "object.h"
#include "gfx.h"
#include "cdload.h"
#include "gamestate.h"
#include "sound.h"
#include "pad.h"
#include "records.h"
#include "stdwtitl.h"

extern s16 stdwtitl_shine_frames_jp[12];
extern s16 stdwtitl_shine_frames_intl[24]; /* ended by -1 */

void stdwtitl_shine_draw_jp(StdwtitlShine *obj) {
    Sprite spr;

    sprite_init(&spr);
    if (obj->show_base != 0) {
        spr.set_layer_id(obj->layer_id, 0);
        spr.set_vram_pos(0x280, 0x100);
        spr.set_clut8_pos(0, 0x1F0);
        spr.draw(cdload_module.get_subfile_by_id(stdwtitl_module.bank), 2, 0x31, 0x79);
        sprite_init(&spr);
    }
    spr.set_layer_id(obj->layer_id, 0);
    spr.set_vram_pos(0x280, 0x100);
    spr.set_palette(obj->frame);
    spr.draw(cdload_module.get_subfile_by_id(stdwtitl_module.bank), 4, 0x25, 0x72);
}

void stdwtitl_shine_update_jp(StdwtitlShine *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->skip == 0) {
            obj->show_base = 0;
            obj->base.next_state(obj);
        } else {
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            obj->index = 0;
            obj->base.next_step(obj);
            /* fallthrough */
        case 2:
            obj->frame = stdwtitl_shine_frames_jp[obj->index];
            if (++obj->index >= 12) {
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
            stdwtitl_shine_draw_jp(obj);
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->frame = 0xB;
            obj->show_base = 1;
            obj->base.next_step(obj);
        }
        stdwtitl_shine_draw_jp(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void stdwtitl_shine_start(StdwtitlShine *obj) {
    if (obj->base.state == OBJECT_STATE_RUN) {
        obj->base.set_step(obj, 1);
    }
}

StdwtitlShine *stdwtitl_shine_create_jp(s32 arg0) {
    StdwtitlShine *obj = object_new(stdwtitl_shine_update_jp, sizeof(StdwtitlShine), 0);

    obj->start = stdwtitl_shine_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->skip = arg0;
    return obj;
}

void stdwtitl_shine_draw_intl(StdwtitlShine *obj) {
    Sprite spr;

    sprite_init(&spr);
    if (obj->show_base != 0) {
        spr.set_layer_id(obj->layer_id, 0);
        spr.set_vram_pos(0x280, 0x100);
        spr.set_clut8_pos(0, 0x1F0);
        spr.draw(cdload_module.get_subfile_by_id(stdwtitl_module.bank), 2, 0xFD, 0x64);
        sprite_init(&spr);
    }
    if (obj->frame != -1) {
        spr.set_layer_id(obj->layer_id, 0);
        spr.set_vram_pos(0x280, 0x100);
        spr.set_palette(obj->frame);
        spr.draw(cdload_module.get_subfile_by_id(stdwtitl_module.bank), 4, 0xF3, 0x5A);
    }
}

void stdwtitl_shine_update_intl(StdwtitlShine *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->skip == 0) {
            obj->show_base = 0;
            obj->base.next_state(obj);
        } else {
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            obj->index = 0;
            obj->frame = 0;
            obj->base.next_step(obj);
            /* fallthrough */
        case 2:
            obj->frame = stdwtitl_shine_frames_intl[obj->index];
            if (obj->frame == -1) {
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
            stdwtitl_shine_draw_intl(obj);
            obj->index++;
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->frame = -1;
            obj->show_base = 1;
            obj->base.next_step(obj);
        }
        stdwtitl_shine_draw_intl(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

StdwtitlShine *stdwtitl_shine_create_intl(s32 arg0) {
    StdwtitlShine *obj = object_new(stdwtitl_shine_update_intl, sizeof(StdwtitlShine), 0);

    obj->start = stdwtitl_shine_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->skip = arg0;
    return obj;
}

/* Object of stdwtitl_picture_update, created by stdwtitl_picture_create: a picture (by language) fading in,
 * held until a button or 0x78 ticks, fading out; then the story goes on with 0xE01. */
typedef struct StdwtitlPicture {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 palette; /* frame: 0 (bright) .. 0xF (dark) */
    /* 0x52 */ s16 ticks;  /* ticks */
} StdwtitlPicture; /* size 0x54 */

void stdwtitl_picture_draw(StdwtitlPicture *obj) {
    Sprite spr;
    s32 id = 0;

    switch (records_language) {
    case 0:
    case 1:
    case 2:
        id = 0;
        break;
    case 3:
    case 6:
        id = 2;
        break;
    case 4:
        id = 3;
        break;
    case 5:
        id = 1;
        break;
    }
    sprite_init(&spr);
    spr.set_layer_id(0x100, 1);
    spr.set_vram_pos(0x280, 0);
    spr.set_palette(obj->palette);
    spr.draw(cdload_module.get_subfile_by_id(0x08A60000), id, 0, 0);
}

void stdwtitl_picture_update(StdwtitlPicture *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->palette = 0xF;
        obj->ticks = 0;
        sound_module.stop_all();
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            if (obj->ticks >= 2) {
                obj->ticks -= 2;
                if (--obj->palette <= 0) {
                    obj->base.set_step(obj, 1);
                    obj->ticks = 0;
                }
            }
            break;
        case 1:
            if (obj->ticks >= 0x78) {
                obj->base.next_step(obj);
                obj->ticks = 0;
            }
            break;
        case 2:
            if (PAD_PRESSED(3)) {
                obj->ticks = 0x4650;
            }
            if (obj->ticks >= 0x4650) {
                obj->base.next_step(obj);
                obj->ticks = 0;
            }
            break;
        case 3:
            if (obj->ticks >= 2) {
                obj->ticks -= 2;
                if (++obj->palette >= 0xF) {
                    obj->base.next_step(obj);
                    obj->ticks = 0;
                }
            }
            break;
        case 4:
            if (obj->ticks >= 0x3C) {
                obj->base.next_step(obj);
                obj->ticks = 0;
            }
            break;
        case 5:
            gamestate_data.funcs.set_next_map(0xE01, 0);
            obj->base.set_state(obj, OBJECT_STATE_END);
            break;
        }
        obj->ticks += gfx_module.funcs.get_frame_ticks();
        stdwtitl_picture_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *stdwtitl_picture_create(void) {
    return object_new(stdwtitl_picture_update, sizeof(StdwtitlPicture), 0);
}

/* Data block of the title screen's object (stdwtitl_title_screen_update). */
typedef struct StdwtitlTitleScreenData {
    /* 0x0 */ Object *title; /* the title menu (stdwtitl_title_create) */
} StdwtitlTitleScreenData;

/* Object of stdwtitl_title_screen_update, created by stdwtitl_title_screen_create. */
typedef struct StdwtitlTitleScreen {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id;
    /* 0x54 */ s32 ot_depth; /* ordering table depth: set to 2, never read */
} StdwtitlTitleScreen; /* size 0x58 */

void stdwtitl_title_screen_run(StdwtitlTitleScreen *obj, StdwtitlTitleScreenData *data) {
    switch (obj->base.step) {
    case 0:
    default:
        data->title = stdwtitl_title_create(&obj->base);
        obj->base.step++;
        break;
    case 1:
        if (data->title == NULL) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    }
}

void stdwtitl_title_screen_update(StdwtitlTitleScreen *obj, StdwtitlTitleScreenData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (sound_module.is_loading() == 0) {
            obj->base.next_state(obj);
            stdwtitl_module.load_pictures();
        }
        break;
    case OBJECT_STATE_RUN:
        stdwtitl_title_screen_run(obj, data);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *stdwtitl_title_screen_create(void) {
    StdwtitlTitleScreen *obj = object_new(stdwtitl_title_screen_update, sizeof(StdwtitlTitleScreen), sizeof(StdwtitlTitleScreenData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    sound_module.load_extra_bank(0x47);
    return &obj->base;
}

s16 stdwtitl_shine_frames_jp[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

s16 stdwtitl_shine_frames_intl[24] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1 };
