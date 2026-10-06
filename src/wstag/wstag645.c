#include "wstag.h"

/* WSTAG645: stage 0x25A (fieldstg_stages). */

extern WstagAnimKey D_WSTAG645_800A6230[];
extern WstagAnimKey D_WSTAG645_800A6264[];
extern WstagAnimKey D_WSTAG645_800A6298[];
extern WstagFuncs wstag645_funcs;
extern FieldstgBattleLists wstag645_battle_lists;
extern FieldstgVramPlace wstag645_vram_places[];
extern FieldstgSprite wstag645_sprites[];
extern FieldstgMapEvent wstag645_map_events[];

s32 wstag645_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
    WstagAnimKey *key = &keys[anim->key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        anim->time -= step;
    }
    if (anim->time <= 0) {
        key++;
        anim->key++;
        anim->time += key->time;
        if (key->frame == 0xFF) {
            key = keys;
            anim->key = 0;
            anim->time += key->time;
        }
        wstag645_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag645_anim_update(WstagAnimObject *obj) {
    FieldstgSprite *sprite;
    s32 frames[3];

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG645_800A6230[0].time;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG645_800A6264[0].time;
        obj->anims[2].key = 0;
        obj->anims[2].time = D_WSTAG645_800A6298[0].time;
        break;
    case OBJECT_STATE_RUN:
        sprite = fieldstg_stage.sprites;
        frames[0] = wstag645_anim_loop(&obj->anims[0], D_WSTAG645_800A6230, 0);
        frames[1] = wstag645_anim_loop(&obj->anims[1], D_WSTAG645_800A6264, 0);
        frames[2] = wstag645_anim_loop(&obj->anims[2], D_WSTAG645_800A6298, 0);
        for (; sprite->present != 0; sprite++) {
            switch (sprite->type) {
            case 1:
                sprite->sprite = frames[0];
                break;
            case 2:
                sprite->sprite = frames[1];
                break;
            case 3:
                sprite->sprite = frames[2];
                break;
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagAnimObject *wstag645_anim_new(void) {
    return object_new(wstag645_anim_update, sizeof(WstagAnimObject), 0);
}

void wstag645_update(WstagObject *obj, WstagAnimObject **data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        *data = wstag645_anim_new();
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag645_start(void *arg0) {
    WstagObject *obj = object_new(wstag645_update, sizeof(WstagObject), sizeof(WstagAnimObject *));

    obj->manager = arg0;
    wstag645_funcs.setup();
    return obj;
}

void wstag645_setup(void) {
    fieldstg_stage.background_file = 0x4A7;
    fieldstg_stage.sprite_file = 0x04A80000;
    fieldstg_stage.sprites = wstag645_sprites;
    fieldstg_stage.map_events = wstag645_map_events;
    fieldstg_stage.mask_file = 0x4A6;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x18C00, 0x22D00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag645_vram_places;
    fieldstg_stage.music = 0x38;
    fieldstg_stage.sound = 0x60E00000;
    fieldstg_stage.battle_lists = &wstag645_battle_lists;
    fieldstg_attr.set_file(0, 0x04A80001);
    fieldstg_attr.set_file(7, 0x04A80002);
    fieldstg_attr.set_file(4, 0x04A80003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag645_setup(void);

WstagAnimKey D_WSTAG645_800A6230[13] = {
    { 50, 8 }, { 51, 8 }, { 52, 8 }, { 53, 8 }, { 54, 8 }, { 55, 8 }, { 56, 8 }, { 57, 8 }, { 58, 8 }, { 59, 8 },
    { 60, 8 }, { 82, 160 }, { 255, 0 },
};
WstagAnimKey D_WSTAG645_800A6264[13] = {
    { 61, 8 }, { 62, 8 }, { 63, 8 }, { 64, 8 }, { 65, 8 }, { 66, 8 }, { 67, 8 }, { 68, 8 }, { 69, 8 }, { 70, 8 },
    { 71, 8 }, { 82, 160 }, { 255, 0 },
};
WstagAnimKey D_WSTAG645_800A6298[12] = {
    { 72, 8 }, { 73, 8 }, { 74, 8 }, { 75, 8 }, { 76, 8 }, { 77, 8 }, { 78, 8 }, { 79, 8 }, { 80, 8 }, { 81, 8 },
    { 82, 160 }, { 255, 0 },
};
FieldstgListedBattle D_WSTAG645_800A62C8 = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG645_800A62D4 = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG645_800A62E0 = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG645_800A62EC = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG645_800A62F8 = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG645_800A6304 = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG645_800A6310 = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG645_800A631C = { 156, 5, 0x60080000 };
FieldstgBattleList D_WSTAG645_800A6328 = {
    3,
    { &D_WSTAG645_800A62C8, &D_WSTAG645_800A62D4, &D_WSTAG645_800A62E0, &D_WSTAG645_800A62EC, &D_WSTAG645_800A62F8,
        &D_WSTAG645_800A6304, &D_WSTAG645_800A6310, &D_WSTAG645_800A631C },
};
FieldstgListedBattle D_WSTAG645_800A634C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A6358 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A6364 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A6370 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A637C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A6388 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A6394 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A63A0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG645_800A63AC = {
    0,
    { &D_WSTAG645_800A634C, &D_WSTAG645_800A6358, &D_WSTAG645_800A6364, &D_WSTAG645_800A6370, &D_WSTAG645_800A637C,
        &D_WSTAG645_800A6388, &D_WSTAG645_800A6394, &D_WSTAG645_800A63A0 },
};
FieldstgListedBattle D_WSTAG645_800A63D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A63DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A63E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A63F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A6400 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A640C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A6418 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A6424 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG645_800A6430 = {
    0,
    { &D_WSTAG645_800A63D0, &D_WSTAG645_800A63DC, &D_WSTAG645_800A63E8, &D_WSTAG645_800A63F4, &D_WSTAG645_800A6400,
        &D_WSTAG645_800A640C, &D_WSTAG645_800A6418, &D_WSTAG645_800A6424 },
};
FieldstgListedBattle D_WSTAG645_800A6454 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A6460 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A646C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A6478 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A6484 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A6490 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A649C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG645_800A64A8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG645_800A64B4 = {
    0,
    { &D_WSTAG645_800A6454, &D_WSTAG645_800A6460, &D_WSTAG645_800A646C, &D_WSTAG645_800A6478, &D_WSTAG645_800A6484,
        &D_WSTAG645_800A6490, &D_WSTAG645_800A649C, &D_WSTAG645_800A64A8 },
};
FieldstgBattleLists wstag645_battle_lists = {
    39, 0, 0, { &D_WSTAG645_800A6328, &D_WSTAG645_800A63AC, &D_WSTAG645_800A6430 }, &D_WSTAG645_800A64B4,
};
FieldstgVramPlace wstag645_vram_places[6] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
};
FieldstgSprite wstag645_sprites[19] = {
    { 1, 3, 0xC8, 2, 0x48, 0, 0, 0, 0, 0, 59, 435, 0, 0 }, { 1, 3, 0xC8, 2, 0x48, 0, 0, 0, 0, 0, 384, 640, 0, 0 },
    { 1, 3, 0xC8, 2, 0x48, 0, 0, 0, 0, 0, 606, 469, 0, 0 }, { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 260, 441, 0, 0 },
    { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 394, 596, 0, 0 }, { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 569, 465, 0, 0 },
    { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 706, 596, 0, 0 }, { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 858, 657, 0, 0 },
    { 1, 0, 0x40, 6, 4, 0, 0, 0, 0, 0, 562, 254, 0, 0 }, { 1, 0, 0x40, 4, 0x53, 2, 0, 0xF, 8, 0, 604, 194, 256, 0 },
    { 1, 0, 0x40, 4, 0x54, 2, 0, 0xF, 8, 0, 604, 194, 252, 0 },
    { 1, 0, 0x40, 4, 0x55, 2, 0, 0xF, 8, 0, 604, 194, 248, 0 },
    { 1, 0, 0x40, 4, 0x56, 2, 0, 0xF, 8, 0, 604, 194, 244, 0 },
    { 1, 0, 0x40, 4, 0x57, 2, 0, 0xF, 8, 0, 604, 194, 240, 0 },
    { 1, 0, 0x40, 4, 0x58, 2, 0, 0xF, 8, 0, 604, 194, 236, 0 },
    { 1, 0, 0x40, 4, 0x59, 2, 0, 0xF, 8, 0, 604, 194, 232, 0 },
    { 1, 0, 0x40, 4, 0x5A, 2, 0, 0xF, 8, 0, 604, 194, 228, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 566, 208, 264, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag645_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x25B, 0x27A, 0x256, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x258, 0x110, 0x108, 7, 0, 1, 6 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag645_funcs = { wstag645_setup };
