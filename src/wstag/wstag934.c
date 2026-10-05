#include "wstag.h"

/* WSTAG934: stage 0x27F (fieldstg_stages_2d). */

extern s16 D_WSTAG934_800A65B8[];
extern CVECTOR wstag934_color;
extern FieldstgVramPlace wstag934_vram_places[];
extern FieldstgPlacedActor *wstag934_actors[];
extern FieldstgSprite wstag934_sprites[];
extern FieldstgMapEvent wstag934_map_events[];
extern FieldstgStageFuncs wstag934_funcs; /* the setup and a fade pair, as FIELDSTG's own */

void wstag934_lift_update(WstagLiftObject *obj) {
    FieldstgSprite *sprite;
    FieldstgSprite *top;
    FieldstgSprite *bottom;
    FieldstgActor *player;
    s32 dy;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            switch (sprite->type) {
            case 2:
                obj->sprite_2 = sprite;
                obj->low_y_2 = sprite->y;
                if (obj->down != 0) {
                    sprite->y -= 0x7F;
                }
                sprite->shown = 0;
                break;
            case 3:
                obj->sprite_3 = sprite;
                obj->low_y_3 = sprite->y;
                if (obj->down != 0) {
                    sprite->y -= 0x7F;
                }
                sprite->shown = 1;
                break;
            }
        }
        obj->down = 0;
        break;
    case OBJECT_STATE_RUN:
        break;
    case OBJECT_STATE_DONE:
        top = obj->sprite_3;
        bottom = obj->sprite_2;
        player = (FieldstgActor *)heap_objects.find(5, -1, 0);
        switch (obj->base.step) {
        case 0:
        default:
            bottom->shown = 1;
            obj->timer = 0;
            obj->start_y_3 = top->y;
            obj->start_y_2 = bottom->y;
            obj->start_player_y = player->pos.y;
            sound_module.play(0x8004103C);
            obj->base.next_step(obj);
            break;
        case 1:
            obj->timer += gfx_module.funcs.get_frame_ticks();
            if (obj->timer >= 0x1E) {
                obj->step = 0;
                obj->base.next_step(obj);
                sound_module.play(0x1080001);
            }
            break;
        case 2:
        case 4:
            dy = D_WSTAG934_800A65B8[obj->step];
            if (dy != 1000) {
                top->y = obj->start_y_3 + dy;
                bottom->y = obj->start_y_2 + dy;
                player->pos.y = obj->start_player_y + dy;
                obj->step++;
            } else {
                obj->base.next_step(obj);
                obj->step = 0;
            }
            break;
        case 3:
            obj->step++;
            if (obj->step >= 0xFE) {
                if (obj->down != 0) {
                    top->y = obj->low_y_3;
                    bottom->y = obj->low_y_2;
                    player->pos.y = obj->start_player_y + 0x7F00;
                } else {
                    top->y = obj->low_y_3 - 0x7F;
                    bottom->y = obj->low_y_2 - 0x7F;
                    player->pos.y = obj->start_player_y - 0x7F00;
                }
                obj->start_y_3 = top->y;
                obj->start_y_2 = bottom->y;
                obj->start_player_y = player->pos.y;
                obj->base.next_step(obj);
                obj->step = 0;
            } else if (obj->step & 1) {
                if (obj->down != 0) {
                    top->y++;
                    bottom->y++;
                    player->pos.y += 0x100;
                } else {
                    top->y--;
                    bottom->y--;
                    player->pos.y -= 0x100;
                }
            }
            break;
        case 5:
            bottom->shown = 0;
            obj->base.set_state(obj, OBJECT_STATE_RUN);
            obj->down ^= 1;
            break;
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void wstag934_lift_message(WstagLiftObject *obj, s32 msg) {
    if (obj != NULL) {
        switch (msg) {
        case 0x348:
            obj->base.set_state(obj, OBJECT_STATE_DONE);
            obj->down = 0;
            break;
        case 0x349:
            obj->base.set_state(obj, OBJECT_STATE_DONE);
            obj->down = 1;
            break;
        }
    }
}

WstagLiftObject *wstag934_lift_create(s32 arg0) {
    WstagLiftObject *obj = object_create(wstag934_lift_update, sizeof(WstagLiftObject), 0, arg0);

    if (gamestate_flags.get_flag(0x1C3D, 1)) {
        obj->down = 1;
    } else {
        obj->down = 0;
    }
    return obj;
}

void wstag934_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The stage object is 4 bytes bigger here. */
/* The stage object is 4 bytes bigger here. */
WstagObject *wstag934_start(void *arg0) {
    WstagObject *obj = object_new(wstag934_update, 0x58, 8);

    obj->manager = arg0;
    wstag934_funcs.setup();
    return obj;
}

void wstag934_setup(void) {
    fieldstg_stage.background_file = 0x1AC;
    fieldstg_stage.sprite_file = 0x08FB0000;
    fieldstg_stage.sprites = wstag934_sprites;
    fieldstg_stage.map_events = wstag934_map_events;
    fieldstg_stage.mask_file = 0x8FA;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x6700, 0x12300 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag934_vram_places;
    fieldstg_stage.music = 0x42;
    fieldstg_stage.sound = 0x61080002;
    fieldstg_stage.actors = wstag934_actors;
    fieldstg_stage.color = wstag934_color;
    fieldstg_attr.set_file(0, 0x08FB0001);
    fieldstg_attr.set_file(1, 0x08FB0002);
    fieldstg_attr.set_file(7, 0x08FB0003);
    fieldstg_attr.init_layer(0);
}

void wstag934_fade_start(WindowAnim *fade, s32 in) {
    fade->running = 1;
    if (in) {
        sound_module.play(0x40019);
        fade->step = 0x1000 / fade->duration;
        fade->level = 0;
    } else {
        sound_module.play(0x4001A);
        fade->level = 0x1000;
        fade->step = -(0x1000 / fade->duration * 2);
    }
}

s32 wstag934_fade_update(WindowAnim *fade) {
    if (fade->running == 0) {
        return 1;
    }
    fade->level += fade->step;
    if (fade->step > 0) {
        if (fade->level > 0x1000) {
            fade->level = 0x1000;
            fade->running = 0;
            return 1;
        }
    } else if (fade->level < 0) {
        fade->level = 0;
        fade->running = 0;
        return 1;
    }
    return 0;
}

INCLUDE_RODATA("asm/wstag934/nonmatchings/wstag934", wstag934_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag934_setup(void);

s16 D_WSTAG934_800A65B8[10] = {
    1, 2, 1, 0, -1, -2, -1, 0,
    1000, 5,
};
FieldstgVramPlace wstag934_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 396, 477, 304, 221, 352, 510 }, { 448, 256, 490, 316, 680, 60, 368, 510 },
};
u16 D_WSTAG934_800A664C[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG934_800A6654[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG934_800A665C[6] = { 0, 1, 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG934_800A6668[6] = { 0x8192, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG934_800A6674[6] = { 0, 1, 0x8192, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG934_800A6680[2] = { { NULL, NULL, 65 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG934_800A6698[4] = {
    { D_WSTAG934_800A664C, D_WSTAG934_800A6654, 62 }, { D_WSTAG934_800A665C, D_WSTAG934_800A6668, 63 },
    { D_WSTAG934_800A6674, NULL, 64 }, { NULL, NULL, 0 },
};
FieldstgPlacedActor D_WSTAG934_800A66C8 = { NULL, D_WSTAG934_800A6680, 45, 4, 271, 361, 5 };
FieldstgPlacedActor D_WSTAG934_800A66DC = { NULL, D_WSTAG934_800A6698, 103, 5, 191, 400, 3 };
FieldstgPlacedActor *wstag934_actors[3] = { &D_WSTAG934_800A66C8, &D_WSTAG934_800A66DC, NULL };
FieldstgSprite wstag934_sprites[14] = {
    { 1, 1, 0x44, 2, 0x52, 2, 0, 1, 4, 0, 312, 282, 0, 0 },
    { 1, 2, 0x40, 2, 0x4A, 1, 0x4A, 0x51, 4, 0, 135, 358, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x43, 4, 0, 357, 161, 0, 0 },
    { 1, 0, 0x40, 6, 1, 0, 0, 0, 0, 0, 192, 417, 0, 0 }, { 1, 0, 0x40, 6, 2, 1, 2, 7, 4, 0, 357, 161, 0, 0 },
    { 1, 3, 0xCD, 6, 0, 0, 0, 0, 0, 0, 160, 338, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x44, 1, 0x44, 0x49, 4, 0, 357, 241, 0, 0 },
    { 1, 0, 0x40, 0xA, 8, 1, 8, 0xD, 4, 0, 357, 241, 0, 0 },
    { 1, 0, 0x40, 0xA, 0xF, 1, 0xF, 0x13, 4, 0, 62, 326, 0, 0 },
    { 1, 0, 0x48, 0xA, 0x14, 0, 0, 0, 0, 0, 128, 202, 0, 0 },
    { 1, 0, 0x49, 0xA, 0x15, 0, 0, 0, 0, 0, 200, 183, 0, 0 },
    { 1, 0, 0x40, 4, 0x32, 1, 0x32, 0x37, 4, 0, 357, 161, 200, 0 },
    { 1, 0, 0x40, 8, 0x38, 1, 0x38, 0x3D, 4, 0, 357, 241, 280, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag934_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x271, 0x368, 0xF4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x281, 0x2FE, 0xA5, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 6, 1, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 5, 8, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgStageFuncs wstag934_funcs = { wstag934_setup, wstag934_fade_start, wstag934_fade_update };
