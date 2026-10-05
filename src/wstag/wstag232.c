#include "wstag.h"

/* WSTAG232: stage 0x209 (fieldstg_stages). */

extern WstagFuncs wstag232_funcs;
extern WstagAnimKey D_WSTAG232_800A61CC[];
extern WstagAnimKey D_WSTAG232_800A621C[];
extern FieldstgBattleLists wstag232_battle_lists;
extern FieldstgVramPlace wstag232_vram_places[];
extern FieldstgSprite wstag232_sprites[];
void wstag232_anim2_update(WstagAnim2Object *obj);

s32 wstag232_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
        wstag232_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag232_anim2_update(WstagAnim2Object *obj) {
    FieldstgSprite *sprite;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG232_800A61CC[0].time;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG232_800A621C[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                sprite->sprite = wstag232_anim_loop(&obj->anims[0], D_WSTAG232_800A61CC, 0);
            }
            if (sprite->type == 2) {
                sprite->sprite = wstag232_anim_loop(&obj->anims[1], D_WSTAG232_800A621C, 0);
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag232_anim2_create(void) {
    return object_new(wstag232_anim2_update, 0x58, 0);
}

void wstag232_update(WstagObject *obj, Object **data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        *data = wstag232_anim2_create();
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag232_start(void *arg0) {
    WstagObject *obj = object_new(wstag232_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag232_funcs.setup();
    return obj;
}

void wstag232_setup(void) {
    fieldstg_stage.background_file = 0x346;
    fieldstg_stage.sprite_file = 0x03470000;
    fieldstg_stage.sprites = wstag232_sprites;
    fieldstg_stage.mask_file = 0x345;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0x11D00, 0x12800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag232_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.battle_lists = &wstag232_battle_lists;
    fieldstg_attr.set_file(0, 0x03470001);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x14 && gamestate_data.progress < 0x18) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag232_setup(void);

WstagAnimKey D_WSTAG232_800A61CC[20] = {
    { 50, 18 }, { 44, 6 }, { 50, 60 }, { 44, 6 }, { 45, 6 }, { 46, 6 }, { 44, 6 }, { 51, 78 }, { 44, 6 }, { 45, 6 },
    { 46, 6 }, { 44, 6 }, { 52, 60 }, { 44, 6 }, { 52, 18 }, { 44, 6 }, { 45, 6 }, { 46, 6 }, { 44, 6 }, { 255, 0 },
};
WstagAnimKey D_WSTAG232_800A621C[20] = {
    { 56, 60 }, { 47, 6 }, { 56, 18 }, { 47, 6 }, { 48, 6 }, { 49, 6 }, { 47, 6 }, { 57, 78 }, { 47, 6 }, { 48, 6 },
    { 49, 6 }, { 47, 6 }, { 58, 18 }, { 47, 6 }, { 58, 60 }, { 47, 6 }, { 48, 6 }, { 49, 6 }, { 47, 6 }, { 255, 0 },
};
FieldstgListedBattle D_WSTAG232_800A626C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A6278 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A6284 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A6290 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A629C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A62A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A62B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A62C0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG232_800A62CC = {
    3,
    { &D_WSTAG232_800A626C, &D_WSTAG232_800A6278, &D_WSTAG232_800A6284, &D_WSTAG232_800A6290, &D_WSTAG232_800A629C,
        &D_WSTAG232_800A62A8, &D_WSTAG232_800A62B4, &D_WSTAG232_800A62C0 },
};
FieldstgListedBattle D_WSTAG232_800A62F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A62FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A6308 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A6314 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A6320 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A632C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A6338 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A6344 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG232_800A6350 = {
    0,
    { &D_WSTAG232_800A62F0, &D_WSTAG232_800A62FC, &D_WSTAG232_800A6308, &D_WSTAG232_800A6314, &D_WSTAG232_800A6320,
        &D_WSTAG232_800A632C, &D_WSTAG232_800A6338, &D_WSTAG232_800A6344 },
};
FieldstgListedBattle D_WSTAG232_800A6374 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A6380 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A638C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A6398 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A63A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A63B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A63BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A63C8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG232_800A63D4 = {
    0,
    { &D_WSTAG232_800A6374, &D_WSTAG232_800A6380, &D_WSTAG232_800A638C, &D_WSTAG232_800A6398, &D_WSTAG232_800A63A4,
        &D_WSTAG232_800A63B0, &D_WSTAG232_800A63BC, &D_WSTAG232_800A63C8 },
};
FieldstgListedBattle D_WSTAG232_800A63F8 = { 253, 20, 0x600C0000 };
FieldstgListedBattle D_WSTAG232_800A6404 = { 254, 20, 0x600C0000 };
FieldstgListedBattle D_WSTAG232_800A6410 = { 255, 20, 0x600C0000 };
FieldstgListedBattle D_WSTAG232_800A641C = { 256, 20, 0x600C0000 };
FieldstgListedBattle D_WSTAG232_800A6428 = { 257, 20, 0x608C0000 };
FieldstgListedBattle D_WSTAG232_800A6434 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A6440 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG232_800A644C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG232_800A6458 = {
    0,
    { &D_WSTAG232_800A63F8, &D_WSTAG232_800A6404, &D_WSTAG232_800A6410, &D_WSTAG232_800A641C, &D_WSTAG232_800A6428,
        &D_WSTAG232_800A6434, &D_WSTAG232_800A6440, &D_WSTAG232_800A644C },
};
FieldstgBattleLists wstag232_battle_lists = {
    170, 0, 0, { &D_WSTAG232_800A62CC, &D_WSTAG232_800A6350, &D_WSTAG232_800A63D4 }, &D_WSTAG232_800A6458,
};
FieldstgVramPlace wstag232_vram_places[6] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
};
FieldstgSprite wstag232_sprites[19] = {
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
WstagFuncs wstag232_funcs = { wstag232_setup };
