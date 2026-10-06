#include "wstag.h"

/* WSTAG954: stage 0x29A (fieldstg_stages_2d). */

extern WstagAnimKey D_WSTAG954_800A61A4[];
extern WstagFuncs wstag954_funcs;
extern FieldstgVramPlace wstag954_vram_places[];
extern FieldstgPlacedActor *wstag954_actors[];
extern FieldstgSprite wstag954_sprites[];
extern FieldstgMapEvent wstag954_map_events[];
extern FieldstgBattleLists wstag954_battle_lists;
void wstag954_anim1_update();
void wstag954_update();

s32 wstag954_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
        wstag954_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag954_anim1_update(WstagAnim1Object *obj) {
    FieldstgSprite *sprite;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anim.key = 0;
        obj->anim.time = D_WSTAG954_800A61A4[0].time;
        break;
    case OBJECT_STATE_RUN:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                sprite->sprite = wstag954_anim_loop(&obj->anim, D_WSTAG954_800A61A4, 0);
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag954_anim1_create(void) {
    return object_new(wstag954_anim1_update, sizeof(WstagAnim1Object), 0);
}

void wstag954_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->object = wstag954_anim1_create();
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag954_start(void *arg0) {
    WstagObject *obj = object_new(wstag954_update, sizeof(WstagObject), sizeof(WstagEventData));

    obj->manager = arg0;
    wstag954_funcs.setup();
    return obj;
}

void wstag954_setup(void) {
    fieldstg_stage.background_file = 0x256;
    fieldstg_stage.sprite_file = 0x09230000;
    fieldstg_stage.sprites = wstag954_sprites;
    fieldstg_stage.map_events = wstag954_map_events;
    fieldstg_stage.mask_file = 0x922;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x16B00, 0x44400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag954_vram_places;
    fieldstg_stage.music = 0x2F;
    fieldstg_stage.sound = 0x60BC0000;
    fieldstg_stage.actors = wstag954_actors;
    fieldstg_stage.battle_lists = &wstag954_battle_lists;
    fieldstg_attr.set_file(0, 0x09230002);
    fieldstg_attr.set_file(7, 0x09230003);
    fieldstg_attr.set_file(4, 0x09230001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag954_setup(void);

WstagAnimKey D_WSTAG954_800A61A4[35] = {
    { 50, 8 }, { 51, 4 }, { 52, 8 }, { 53, 4 }, { 54, 8 }, { 55, 4 }, { 56, 8 }, { 57, 16 }, { 58, 4 }, { 59, 8 },
    { 60, 4 }, { 61, 8 }, { 62, 8 }, { 63, 12 }, { 64, 20 }, { 65, 4 }, { 66, 8 }, { 67, 4 }, { 68, 8 }, { 69, 8 },
    { 70, 8 }, { 71, 8 }, { 72, 8 }, { 73, 4 }, { 74, 8 }, { 75, 4 }, { 76, 8 }, { 77, 8 }, { 78, 8 }, { 79, 8 },
    { 80, 8 }, { 81, 12 }, { 82, 8 }, { 83, 30 }, { 255, 0 },
};
FieldstgVramPlace wstag954_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 412, 328, 368, 72, 368, 511 }, { 384, 256, 426, 288, 424, 32, 336, 510 },
    { 384, 256, 434, 288, 456, 32, 352, 510 },
};
u16 D_WSTAG954_800A62C0[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG954_800A62CC[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG954_800A62D8[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG954_800A62E4[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG954_800A62F4[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG954_800A6300[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG954_800A6308[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG954_800A6314[4] = { 0x782F, 1, 0xFFFF, 0 };
u16 D_WSTAG954_800A631C[4] = { 0x7096, 0, 0xFFFF, 0 };
u16 D_WSTAG954_800A6324[6] = { 0x7096, 1, 0x1010, 0, 0xFFFF, 0 };
u16 D_WSTAG954_800A6330[6] = { 0x1010, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG954_800A633C[6] = { 0x7096, 1, 0x1010, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG954_800A6348[2] = { { NULL, NULL, 146 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG954_800A6360[2] = { { NULL, NULL, 73 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG954_800A6378[5] = {
    { D_WSTAG954_800A62C0, D_WSTAG954_800A62CC, 75 }, { D_WSTAG954_800A62D8, D_WSTAG954_800A62E4, 76 },
    { D_WSTAG954_800A62F4, D_WSTAG954_800A6300, 73 }, { D_WSTAG954_800A6308, D_WSTAG954_800A6314, 74 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG954_800A63B4[4] = {
    { D_WSTAG954_800A631C, NULL, 143 }, { D_WSTAG954_800A6324, D_WSTAG954_800A6330, 144 },
    { D_WSTAG954_800A633C, NULL, 145 }, { NULL, NULL, 0 },
};
u16 D_WSTAG954_800A63E4[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG954_800A63EC[4] = { 0x8192, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG954_800A63F4 = { NULL, D_WSTAG954_800A6348, 45, 4, 764, 1138, 5 };
FieldstgPlacedActor D_WSTAG954_800A6408 = { D_WSTAG954_800A63E4, D_WSTAG954_800A6360, 56, 5, 976, 632, 1 };
FieldstgPlacedActor D_WSTAG954_800A641C = { D_WSTAG954_800A63EC, D_WSTAG954_800A6378, 56, 5, 976, 632, 1 };
FieldstgPlacedActor D_WSTAG954_800A6430 = { NULL, D_WSTAG954_800A63B4, 146, 6, 924, 323, 1 };
FieldstgPlacedActor *wstag954_actors[5] = {
    &D_WSTAG954_800A63F4, &D_WSTAG954_800A6408, &D_WSTAG954_800A641C, &D_WSTAG954_800A6430, NULL,
};
FieldstgSprite wstag954_sprites[7] = {
    { 1, 1, 0xE6, 2, 0x32, 0, 0, 0, 0, 0, 966, 364, 0, 0 }, { 1, 0, 0x40, 2, 0x54, 2, 0, 2, 6, 0, 931, 293, 0, 0 },
    { 1, 0, 0x40, 2, 1, 1, 1, 6, 8, 0, 179, 400, 0, 0 }, { 1, 0, 0x40, 2, 1, 1, 1, 6, 8, 0, 602, 1071, 0, 0 },
    { 1, 0, 0xC8, 6, 7, 0, 0, 0, 0, 0, 745, 598, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 298, 442, 505, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag954_map_events[13] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x299, 0x648, 0xD0, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 8, 0xF0, 0x4D8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 8, 0x100, 0x450, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x102, 0x420, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0xF2, 0x388, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x1D2, 0x4C8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x1C2, 0x460, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 0xF, 0x1AF, 0x428, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 0xF, 0x1BF, 0x330, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 0x16, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x330, 0x208, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x340, 0x170, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag954_funcs = { wstag954_setup };
FieldstgListedBattle D_WSTAG954_800A6614 = { 50, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A6620 = { 50, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A662C = { 130, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A6638 = { 130, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A6644 = { 131, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A6650 = { 131, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A665C = { 134, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A6668 = { 134, 4, 0x60080000 };
FieldstgBattleList D_WSTAG954_800A6674 = {
    3,
    { &D_WSTAG954_800A6614, &D_WSTAG954_800A6620, &D_WSTAG954_800A662C, &D_WSTAG954_800A6638, &D_WSTAG954_800A6644,
        &D_WSTAG954_800A6650, &D_WSTAG954_800A665C, &D_WSTAG954_800A6668 },
};
FieldstgListedBattle D_WSTAG954_800A6698 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A66A4 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A66B0 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A66BC = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A66C8 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A66D4 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A66E0 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG954_800A66EC = { 100, 4, 0x60080000 };
FieldstgBattleList D_WSTAG954_800A66F8 = {
    5,
    { &D_WSTAG954_800A6698, &D_WSTAG954_800A66A4, &D_WSTAG954_800A66B0, &D_WSTAG954_800A66BC, &D_WSTAG954_800A66C8,
        &D_WSTAG954_800A66D4, &D_WSTAG954_800A66E0, &D_WSTAG954_800A66EC },
};
FieldstgListedBattle D_WSTAG954_800A671C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG954_800A6728 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG954_800A6734 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG954_800A6740 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG954_800A674C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG954_800A6758 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG954_800A6764 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG954_800A6770 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG954_800A677C = {
    0,
    { &D_WSTAG954_800A671C, &D_WSTAG954_800A6728, &D_WSTAG954_800A6734, &D_WSTAG954_800A6740, &D_WSTAG954_800A674C,
        &D_WSTAG954_800A6758, &D_WSTAG954_800A6764, &D_WSTAG954_800A6770 },
};
FieldstgListedBattle D_WSTAG954_800A67A0 = { 308, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG954_800A67AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG954_800A67B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG954_800A67C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG954_800A67D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG954_800A67DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG954_800A67E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG954_800A67F4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG954_800A6800 = {
    0,
    { &D_WSTAG954_800A67A0, &D_WSTAG954_800A67AC, &D_WSTAG954_800A67B8, &D_WSTAG954_800A67C4, &D_WSTAG954_800A67D0,
        &D_WSTAG954_800A67DC, &D_WSTAG954_800A67E8, &D_WSTAG954_800A67F4 },
};
FieldstgBattleLists wstag954_battle_lists = {
    391, 0, 0, { &D_WSTAG954_800A6674, &D_WSTAG954_800A66F8, &D_WSTAG954_800A677C }, &D_WSTAG954_800A6800,
};
