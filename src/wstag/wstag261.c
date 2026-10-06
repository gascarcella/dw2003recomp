#include "wstag.h"

/* WSTAG261: stage 0x27F (fieldstg_stages). */

extern s16 D_WSTAG261_800A66A0[];
extern WstagFuncs wstag261_funcs;
extern const CVECTOR wstag261_color;
extern FieldstgVramPlace wstag261_vram_places[];
extern FieldstgPlacedActor *wstag261_actors[];
extern FieldstgSprite wstag261_sprites[];
extern FieldstgMapEvent wstag261_map_events[];
extern FieldstgEventDef wstag261_events[];
void wstag261_update(WstagObject *obj, WstagLiftObject **data);

void wstag261_lift_update(WstagLiftObject *obj) {
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
            dy = D_WSTAG261_800A66A0[obj->step];
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

void wstag261_lift_message(WstagLiftObject *obj, s32 msg) {
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

WstagLiftObject *wstag261_lift_create(s32 arg0) {
    WstagLiftObject *obj = object_create(wstag261_lift_update, sizeof(WstagLiftObject), 0, arg0);

    if (gamestate_flags.get_flag(0x1C3D, 1)) {
        obj->down = 1;
    } else {
        obj->down = 0;
    }
    return obj;
}

void wstag261_update(WstagObject *obj, WstagLiftObject **data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        *data = wstag261_lift_create(0x33B);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}



WstagObject *wstag261_start(void *arg0) {
    WstagObject *obj = object_new(wstag261_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag261_funcs.setup();
    return obj;
}

void wstag261_setup(void) {
    fieldstg_stage.background_file = 0x526;
    fieldstg_stage.sprite_file = 0x05270000;
    fieldstg_stage.sprites = wstag261_sprites;
    fieldstg_stage.map_events = wstag261_map_events;
    fieldstg_stage.mask_file = 0x525;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10300, 0x18500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag261_vram_places;
    fieldstg_stage.music = 0x42;
    fieldstg_stage.actors = wstag261_actors;
    fieldstg_stage.sound = 0x61080002;
    fieldstg_stage.color = wstag261_color;
    fieldstg_stage.events = wstag261_events;
    fieldstg_attr.set_file(0, 0x05270001);
    fieldstg_attr.set_file(1, 0x05270003);
    fieldstg_attr.set_file(7, 0x05270002);
    fieldstg_attr.init_layer(0);
}

const CVECTOR wstag261_color = { 0x80, 0x80, 0x80, 0 };

/* The stage's .data (tools/wstag_data.py). */
void wstag261_setup(void);

s16 D_WSTAG261_800A64D8[114] = {
    FIELDSTG_EVENT_WALK(2, 191, 400, 3),
    FIELDSTG_EVENT_PLACE(63, 176, 392),
    FIELDSTG_EVENT_ANIM(63, 1, 0),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(0x32D, 876, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 236, 376, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 264, 388, 7),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_PLACE(63, 176, 265),
    FIELDSTG_EVENT_ANIM(63, 1, 0),
    FIELDSTG_EVENT_ANIM(0x33B, 840, 2),
    FIELDSTG_EVENT_WAIT(300),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG261_800A65BC[114] = {
    FIELDSTG_EVENT_WALK(2, 191, 273, 3),
    FIELDSTG_EVENT_PLACE(63, 176, 265),
    FIELDSTG_EVENT_ANIM(63, 1, 0),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(0x32D, 876, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 239, 249, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 264, 261, 7),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_PLACE(63, 176, 392),
    FIELDSTG_EVENT_ANIM(63, 1, 0),
    FIELDSTG_EVENT_ANIM(0x33B, 841, 2),
    FIELDSTG_EVENT_WAIT(300),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG261_800A66A0[10] = {
    1, 2, 1, 0, -1, -2, -1, 0,
    1000, 0,
};
FieldstgVramPlace wstag261_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG261_800A6724[4] = { 0x1C3D, 0, 0xFFFF, 0 };
u16 D_WSTAG261_800A672C[6] = { 0x9062, 1, 0x1C3D, 1, 0xFFFF, 0 };
u16 D_WSTAG261_800A6738[4] = { 0x1C3D, 1, 0xFFFF, 0 };
u16 D_WSTAG261_800A6740[6] = { 0x9063, 1, 0x1C3D, 0, 0xFFFF, 0 };
u16 D_WSTAG261_800A674C[4] = { 0x1C3D, 0, 0xFFFF, 0 };
u16 D_WSTAG261_800A6754[6] = { 0x9062, 1, 0x1C3D, 1, 0xFFFF, 0 };
u16 D_WSTAG261_800A6760[4] = { 0x1C3D, 1, 0xFFFF, 0 };
u16 D_WSTAG261_800A6768[6] = { 0x9063, 1, 0x1C3D, 0, 0xFFFF, 0 };
FieldstgTalk D_WSTAG261_800A6774[3] = {
    { D_WSTAG261_800A6724, D_WSTAG261_800A672C, 515 }, { D_WSTAG261_800A6738, D_WSTAG261_800A6740, 516 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG261_800A6798[3] = {
    { D_WSTAG261_800A674C, D_WSTAG261_800A6754, 515 }, { D_WSTAG261_800A6760, D_WSTAG261_800A6768, 516 },
    { NULL, NULL, 0 },
};
u16 D_WSTAG261_800A67BC[4] = { 0x1C3D, 0, 0xFFFF, 0 };
u16 D_WSTAG261_800A67C4[4] = { 0x1C3D, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG261_800A67CC = { D_WSTAG261_800A67BC, D_WSTAG261_800A6774, 63, 4, 176, 392, 1 };
FieldstgPlacedActor D_WSTAG261_800A67E0 = { D_WSTAG261_800A67C4, D_WSTAG261_800A6798, 63, 4, 176, 265, 1 };
FieldstgPlacedActor *wstag261_actors[3] = { &D_WSTAG261_800A67CC, &D_WSTAG261_800A67E0, NULL };
FieldstgSprite wstag261_sprites[11] = {
    { 0, 1, 0x44, 2, 0x3A, 2, 0, 1, 4, 0, 312, 282, 0, 0 },
    { 1, 2, 0x40, 2, 0x32, 1, 0x32, 0x39, 4, 0, 135, 358, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 0, 0, 0, 0, 0, 328, 194, 0, 0 }, { 1, 0, 0x40, 6, 1, 0, 0, 0, 0, 0, 192, 417, 0, 0 },
    { 1, 3, 0xCD, 6, 0, 0, 0, 0, 0, 0, 160, 338, 0, 0 }, { 1, 0, 0x40, 0xA, 0x3E, 0, 0, 0, 0, 0, 328, 272, 0, 0 },
    { 1, 0, 0x48, 0xA, 0x14, 0, 0, 0, 0, 0, 128, 202, 0, 0 },
    { 1, 0, 0x49, 0xA, 0x15, 0, 0, 0, 0, 0, 200, 183, 0, 0 },
    { 1, 0, 0x40, 4, 0x3D, 0, 0, 0, 0, 0, 336, 166, 193, 0 },
    { 1, 0, 0x40, 8, 0x3F, 0, 0, 0, 0, 0, 336, 244, 277, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag261_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x271, 0x368, 0xF4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x281, 0x2FE, 0xA5, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 6, 1, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 5, 8, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag261_funcs = { wstag261_setup };
FieldstgEventDef wstag261_events[3] = {
    { 1321, D_WSTAG261_800A64D8, 0x0112002E, NULL, NULL }, { 1326, D_WSTAG261_800A65BC, 0x0112002F, NULL, NULL },
    { -1, NULL, 0, NULL, NULL },
};
