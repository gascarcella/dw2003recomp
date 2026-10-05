#include "common.h"
#include "object.h"
#include "gfx.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "records.h"
#include "stdwtitl.h"

/* A position of stdwtitl_menu_positions. */
typedef struct StdwtitlMenuPos {
    /* 0x0 */ s32 x;
    /* 0x4 */ s32 y;
} StdwtitlMenuPos; /* size 0x8 */

extern StdwtitlMenuPos stdwtitl_menu_positions[3]; /* the two choices, then "press start" */
extern u8 stdwtitl_menu_sprites[7][4];     /* per language: sprites of "press start" and the choices */

void stdwtitl_menu_draw(StdwtitlMenu *obj) {
    Sprite spr;

    if (obj->cursor_shown != 0) {
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, 0);
        spr.set_vram_pos(0x280, 0x100);
        spr.set_palette(obj->blink);
        spr.draw(cdload_module.get_subfile_by_id(stdwtitl_module.bank), 7, stdwtitl_menu_positions[obj->cursor].x,
                   stdwtitl_menu_positions[obj->cursor].y - 1);
    }
    if (obj->is_open == 0) {
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, 0);
        spr.set_vram_pos(0x280, 0x100);
        spr.set_palette(obj->cursor != 2);
        spr.draw(cdload_module.get_subfile_by_id(stdwtitl_module.bank), stdwtitl_menu_sprites[records_language][0], 0x4F, 0xA6);
    } else {
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, 0);
        spr.set_vram_pos(0x280, 0x100);
        spr.set_palette(obj->cursor != 0);
        spr.draw(cdload_module.get_subfile_by_id(stdwtitl_module.bank), stdwtitl_menu_sprites[records_language][1], obj->choice1_x,
                   obj->choice1_y);
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, 0);
        spr.set_vram_pos(0x280, 0x100);
        spr.set_palette(obj->cursor != 1);
        spr.draw(cdload_module.get_subfile_by_id(stdwtitl_module.bank), stdwtitl_menu_sprites[records_language][2], obj->choice2_x,
                   obj->choice2_y);
    }
}

void stdwtitl_menu_update(StdwtitlMenu *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->skip_start == 0) {
            obj->base.next_state(obj);
        } else {
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        obj->result = 0;
        break;
    case OBJECT_STATE_RUN:
        if (obj->base.step == 0) {
            break;
        }
        switch (obj->base.step) {
        case 1:
        default:
            if (obj->ticks++ >= 6) {
                obj->cursor = 2;
                obj->ticks = 0;
                obj->cursor_shown = 1;
                obj->base.next_step(obj);
            }
            break;
        case 2:
            if (PAD_PRESSED(3)) {
                obj->base.next_step(obj);
                obj->ticks = 0;
                sound_module.play(0x8004503C);
                obj->blink = 0;
            }
            if (++obj->ticks >= 600) {
                obj->result = 3;
            }
            break;
        case 3:
            if ((++obj->ticks & 3) == 0) {
                obj->ticks = 0;
                if (++obj->blink >= 3) {
                    obj->base.next_step(obj);
                    obj->is_open = 1;
                    obj->choice1_x = obj->choice2_x = 0x4F;
                    obj->choice1_y = obj->choice2_y = 0xA6;
                    obj->blink = 0;
                    obj->cursor_shown = 0;
                    sound_module.play(0x8004113E);
                }
            }
            break;
        case 4:
            obj->choice1_x = (stdwtitl_menu_positions[0].x - 0x4F) * obj->ticks / 15 + 0x4F;
            obj->choice1_y = (stdwtitl_menu_positions[0].y - 0xA6) * obj->ticks / 15 + 0xA6;
            obj->choice2_x = (stdwtitl_menu_positions[1].x - 0x4F) * obj->ticks / 15 + 0x4F;
            obj->choice2_y = (stdwtitl_menu_positions[1].y - 0xA6) * obj->ticks / 15 + 0xA6;
            if (++obj->ticks >= 15) {
                obj->ticks = 0;
                obj->base.next_step(obj);
            }
            break;
        case 5:
            if (++obj->ticks >= 6) {
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
            break;
        }
        stdwtitl_menu_draw(obj);
        break;
    case OBJECT_STATE_DONE:
        switch (obj->base.step) {
        case 0:
        default:
            obj->is_open = 1;
            obj->cursor = 1;
            obj->cursor_shown = 1;
            obj->choice1_x = stdwtitl_menu_positions[0].x;
            obj->choice1_y = stdwtitl_menu_positions[0].y;
            obj->choice2_x = stdwtitl_menu_positions[1].x;
            obj->choice2_y = stdwtitl_menu_positions[1].y;
            obj->base.next_step(obj);
            break;
        case 1:
            if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                if (obj->cursor != 0) {
                    sound_module.play(0x8004513E);
                }
                obj->cursor = 0;
            }
            if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                if (obj->cursor != 1) {
                    sound_module.play(0x8004513E);
                }
                obj->cursor = 1;
            }
            if (PAD_PRESSED(13)) {
                obj->ticks = 0;
                sound_module.play(0x8004503C);
                obj->base.next_step(obj);
            }
            break;
        case 2:
            if ((++obj->ticks & 3) == 0) {
                obj->ticks = 0;
                if (++obj->blink >= 3) {
                    if (obj->cursor != 0) {
                        obj->result = 2;
                    } else {
                        obj->result = 1;
                    }
                    obj->base.next_step(obj);
                    obj->blink = 0;
                }
            }
            break;
        case 3:
            break;
        }
        stdwtitl_menu_draw(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void stdwtitl_menu_start(StdwtitlMenu *obj) {
    if (obj->base.state == OBJECT_STATE_RUN) {
        obj->base.set_step(obj, 1);
    }
}

void stdwtitl_menu_show_press_start(StdwtitlMenu *obj) {
    if (obj->base.state == OBJECT_STATE_RUN) {
        obj->cursor = 2;
        obj->cursor_shown = 1;
        obj->ticks = 0;
        obj->base.set_step(obj, 2);
    }
}

s16 stdwtitl_menu_get_result(StdwtitlMenu *obj) {
    return obj->result;
}

StdwtitlMenu *stdwtitl_menu_create(s16 arg0) {
    StdwtitlMenu *obj = object_new(stdwtitl_menu_update, sizeof(StdwtitlMenu), 0);

    obj->start = stdwtitl_menu_start;
    obj->show_press_start = stdwtitl_menu_show_press_start;
    obj->get_result = (s32 (*)(struct StdwtitlMenu *))stdwtitl_menu_get_result;
    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->skip_start = arg0;
    return obj;
}

StdwtitlMenuPos stdwtitl_menu_positions[3] = {
    { 79, 154 },
    { 79, 174 },
    { 79, 166 },
};

u8 stdwtitl_menu_sprites[7][4] = {
    { 8, 9, 10, 0 },
    { 8, 9, 10, 0 },
    { 8, 9, 10, 0 },
    { 17, 18, 19, 0 },
    { 11, 12, 13, 0 },
    { 20, 21, 22, 0 },
    { 14, 15, 16, 0 },
};
