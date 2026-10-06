#include "wstag.h"

/* WSTAG233: stage 0x278 (fieldstg_stages). */

extern WstagAnimKey D_WSTAG233_800A61C8[];
extern WstagAnimKey D_WSTAG233_800A6218[];
extern WstagFuncs wstag233_funcs;
extern FieldstgBattleLists wstag233_battle_lists;
extern FieldstgVramPlace wstag233_vram_places[];
extern FieldstgSprite wstag233_sprites[];
void wstag233_anim2_update();

s32 wstag233_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
        wstag233_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag233_anim2_update(WstagAnim2Object *obj) {
    FieldstgSprite *sprite;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG233_800A61C8[0].time;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG233_800A6218[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                sprite->sprite = wstag233_anim_loop(&obj->anims[0], D_WSTAG233_800A61C8, 0);
            }
            if (sprite->type == 2) {
                sprite->sprite = wstag233_anim_loop(&obj->anims[1], D_WSTAG233_800A6218, 0);
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag233_anim2_create(void) {
    return object_new(wstag233_anim2_update, sizeof(WstagAnim2Object), 0);
}

void wstag233_update(WstagObject *obj, Object **data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        *data = wstag233_anim2_create();
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag233_start(void *arg0) {
    WstagObject *obj = object_new(wstag233_update, sizeof(WstagObject), sizeof(Object *));

    obj->manager = arg0;
    wstag233_funcs.setup();
    return obj;
}

void wstag233_setup(void) {
    fieldstg_stage.background_file = 0x4DB;
    fieldstg_stage.sprite_file = 0x04DC0000;
    fieldstg_stage.sprites = wstag233_sprites;
    fieldstg_stage.mask_file = 0x4DA;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x11100, 0x12D00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag233_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.battle_lists = &wstag233_battle_lists;
    fieldstg_attr.set_file(0, 0x04DC0001);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag233_setup(void);

WstagAnimKey D_WSTAG233_800A61C8[20] = {
    { 50, 18 }, { 44, 6 }, { 50, 60 }, { 44, 6 }, { 45, 6 }, { 46, 6 }, { 44, 6 }, { 51, 78 }, { 44, 6 }, { 45, 6 },
    { 46, 6 }, { 44, 6 }, { 52, 60 }, { 44, 6 }, { 52, 18 }, { 44, 6 }, { 45, 6 }, { 46, 6 }, { 44, 6 }, { 255, 0 },
};
WstagAnimKey D_WSTAG233_800A6218[20] = {
    { 56, 60 }, { 47, 6 }, { 56, 18 }, { 47, 6 }, { 48, 6 }, { 49, 6 }, { 47, 6 }, { 57, 78 }, { 47, 6 }, { 48, 6 },
    { 49, 6 }, { 47, 6 }, { 58, 18 }, { 47, 6 }, { 58, 60 }, { 47, 6 }, { 48, 6 }, { 49, 6 }, { 47, 6 }, { 255, 0 },
};
FieldstgListedBattle D_WSTAG233_800A6268 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A6274 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A6280 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A628C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A6298 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A62A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A62B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A62BC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG233_800A62C8 = {
    3,
    { &D_WSTAG233_800A6268, &D_WSTAG233_800A6274, &D_WSTAG233_800A6280, &D_WSTAG233_800A628C, &D_WSTAG233_800A6298,
        &D_WSTAG233_800A62A4, &D_WSTAG233_800A62B0, &D_WSTAG233_800A62BC },
};
FieldstgListedBattle D_WSTAG233_800A62EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A62F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A6304 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A6310 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A631C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A6328 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A6334 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A6340 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG233_800A634C = {
    0,
    { &D_WSTAG233_800A62EC, &D_WSTAG233_800A62F8, &D_WSTAG233_800A6304, &D_WSTAG233_800A6310, &D_WSTAG233_800A631C,
        &D_WSTAG233_800A6328, &D_WSTAG233_800A6334, &D_WSTAG233_800A6340 },
};
FieldstgListedBattle D_WSTAG233_800A6370 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A637C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A6388 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A6394 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A63A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A63AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A63B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A63C4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG233_800A63D0 = {
    0,
    { &D_WSTAG233_800A6370, &D_WSTAG233_800A637C, &D_WSTAG233_800A6388, &D_WSTAG233_800A6394, &D_WSTAG233_800A63A0,
        &D_WSTAG233_800A63AC, &D_WSTAG233_800A63B8, &D_WSTAG233_800A63C4 },
};
FieldstgListedBattle D_WSTAG233_800A63F4 = { 258, 20, 0x600C0000 };
FieldstgListedBattle D_WSTAG233_800A6400 = { 259, 20, 0x600C0000 };
FieldstgListedBattle D_WSTAG233_800A640C = { 260, 20, 0x600C0000 };
FieldstgListedBattle D_WSTAG233_800A6418 = { 261, 20, 0x600C0000 };
FieldstgListedBattle D_WSTAG233_800A6424 = { 262, 20, 0x608C0000 };
FieldstgListedBattle D_WSTAG233_800A6430 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A643C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG233_800A6448 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG233_800A6454 = {
    0,
    { &D_WSTAG233_800A63F4, &D_WSTAG233_800A6400, &D_WSTAG233_800A640C, &D_WSTAG233_800A6418, &D_WSTAG233_800A6424,
        &D_WSTAG233_800A6430, &D_WSTAG233_800A643C, &D_WSTAG233_800A6448 },
};
FieldstgBattleLists wstag233_battle_lists = {
    171, 0, 0, { &D_WSTAG233_800A62C8, &D_WSTAG233_800A634C, &D_WSTAG233_800A63D0 }, &D_WSTAG233_800A6454,
};
FieldstgVramPlace wstag233_vram_places[6] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
};
FieldstgSprite wstag233_sprites[19] = {
    { 1, 0, 0x40, 2, 0x41, 2, 0, 5, 8, 0, 350, 180, 0, 0 }, { 1, 0, 0x40, 2, 0x45, 2, 0, 5, 8, 0, 314, 180, 0, 0 },
    { 1, 2, 0x58, 2, 0x38, 0, 0, 0, 0, 0, 229, 79, 0, 0 }, { 1, 1, 0x58, 2, 0x32, 0, 0, 0, 0, 0, 395, 79, 0, 0 },
    { 1, 0, 0x40, 2, 0x3E, 1, 0x3E, 0x40, 8, 0, 354, 185, 0, 0 },
    { 1, 0, 0x40, 2, 0x42, 1, 0x42, 0x44, 8, 0, 318, 185, 0, 0 },
    { 1, 0, 0x40, 2, 0x46, 2, 0, 3, 4, 0, 206, 186, 0, 0 }, { 1, 0, 0x40, 2, 0x46, 2, 0, 3, 4, 0, 370, 138, 0, 0 },
    { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 4, 0, 238, 170, 0, 0 }, { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 4, 0, 402, 154, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 3, 4, 0, 270, 154, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 3, 4, 0, 434, 170, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 3, 4, 0, 302, 138, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 3, 4, 0, 466, 186, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 6, 0, 387, 77, 0, 0 }, { 1, 0, 0x40, 6, 0x37, 2, 0, 5, 6, 0, 395, 79, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 2, 0, 5, 6, 0, 229, 77, 0, 0 }, { 1, 0, 0x40, 6, 0x3D, 2, 0, 5, 6, 0, 229, 79, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag233_funcs = { wstag233_setup };
