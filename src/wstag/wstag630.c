#include "wstag.h"

/* WSTAG630: stage 0x257 (fieldstg_stages). */

extern WstagAnimKey D_WSTAG630_800A623C[];
extern WstagAnimKey D_WSTAG630_800A6270[];
extern WstagAnimKey D_WSTAG630_800A62A4[];
extern WstagFuncs wstag630_funcs;
extern FieldstgBattleLists wstag630_battle_lists;
extern FieldstgVramPlace wstag630_vram_places[];
extern FieldstgPlacedActor *wstag630_actors[];
extern FieldstgSprite wstag630_sprites[];
extern FieldstgMapEvent wstag630_map_events[];

s32 wstag630_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
        wstag630_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag630_anim_update(WstagAnimObject *obj) {
    FieldstgSprite *sprite;
    s32 frames[3];

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG630_800A623C[0].time;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG630_800A6270[0].time;
        obj->anims[2].key = 0;
        obj->anims[2].time = D_WSTAG630_800A62A4[0].time;
        break;
    case OBJECT_STATE_RUN:
        sprite = fieldstg_stage.sprites;
        frames[0] = wstag630_anim_loop(&obj->anims[0], D_WSTAG630_800A623C, 0);
        frames[1] = wstag630_anim_loop(&obj->anims[1], D_WSTAG630_800A6270, 0);
        frames[2] = wstag630_anim_loop(&obj->anims[2], D_WSTAG630_800A62A4, 0);
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

WstagAnimObject *wstag630_anim_new(void) {
    return object_new(wstag630_anim_update, sizeof(WstagAnimObject), 0);
}

void wstag630_update(WstagObject *obj, Object **data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        *data = wstag630_anim_new();
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag630_start(void *arg0) {
    WstagObject *obj = object_new(wstag630_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag630_funcs.setup();
    return obj;
}

void wstag630_setup(void) {
    fieldstg_stage.background_file = 0x493;
    fieldstg_stage.sprite_file = 0x04940000;
    fieldstg_stage.sprites = wstag630_sprites;
    fieldstg_stage.map_events = wstag630_map_events;
    fieldstg_stage.mask_file = 0x492;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1AF00, 0x11800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag630_vram_places;
    fieldstg_stage.music = 0x38;
    fieldstg_stage.sound = 0x60E00000;
    fieldstg_stage.actors = wstag630_actors;
    fieldstg_stage.battle_lists = &wstag630_battle_lists;
    fieldstg_attr.set_file(0, 0x04940001);
    fieldstg_attr.set_file(7, 0x04940002);
    fieldstg_attr.set_file(4, 0x04940003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag630_setup(void);

WstagAnimKey D_WSTAG630_800A623C[13] = {
    { 50, 8 }, { 51, 8 }, { 52, 8 }, { 53, 8 }, { 54, 8 }, { 55, 8 }, { 56, 8 }, { 57, 8 }, { 58, 8 }, { 59, 8 },
    { 60, 8 }, { 82, 160 }, { 255, 0 },
};
WstagAnimKey D_WSTAG630_800A6270[13] = {
    { 61, 8 }, { 62, 8 }, { 63, 8 }, { 64, 8 }, { 65, 8 }, { 66, 8 }, { 67, 8 }, { 68, 8 }, { 69, 8 }, { 70, 8 },
    { 71, 8 }, { 82, 160 }, { 255, 0 },
};
WstagAnimKey D_WSTAG630_800A62A4[12] = {
    { 72, 8 }, { 73, 8 }, { 74, 8 }, { 75, 8 }, { 76, 8 }, { 77, 8 }, { 78, 8 }, { 79, 8 }, { 80, 8 }, { 81, 8 },
    { 82, 160 }, { 255, 0 },
};
FieldstgListedBattle D_WSTAG630_800A62D4 = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG630_800A62E0 = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG630_800A62EC = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG630_800A62F8 = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG630_800A6304 = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG630_800A6310 = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG630_800A631C = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG630_800A6328 = { 156, 5, 0x60080000 };
FieldstgBattleList D_WSTAG630_800A6334 = {
    3,
    { &D_WSTAG630_800A62D4, &D_WSTAG630_800A62E0, &D_WSTAG630_800A62EC, &D_WSTAG630_800A62F8, &D_WSTAG630_800A6304,
        &D_WSTAG630_800A6310, &D_WSTAG630_800A631C, &D_WSTAG630_800A6328 },
};
FieldstgListedBattle D_WSTAG630_800A6358 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A6364 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A6370 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A637C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A6388 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A6394 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A63A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A63AC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG630_800A63B8 = {
    0,
    { &D_WSTAG630_800A6358, &D_WSTAG630_800A6364, &D_WSTAG630_800A6370, &D_WSTAG630_800A637C, &D_WSTAG630_800A6388,
        &D_WSTAG630_800A6394, &D_WSTAG630_800A63A0, &D_WSTAG630_800A63AC },
};
FieldstgListedBattle D_WSTAG630_800A63DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A63E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A63F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A6400 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A640C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A6418 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A6424 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A6430 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG630_800A643C = {
    0,
    { &D_WSTAG630_800A63DC, &D_WSTAG630_800A63E8, &D_WSTAG630_800A63F4, &D_WSTAG630_800A6400, &D_WSTAG630_800A640C,
        &D_WSTAG630_800A6418, &D_WSTAG630_800A6424, &D_WSTAG630_800A6430 },
};
FieldstgListedBattle D_WSTAG630_800A6460 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A646C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A6478 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A6484 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A6490 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A649C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A64A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG630_800A64B4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG630_800A64C0 = {
    0,
    { &D_WSTAG630_800A6460, &D_WSTAG630_800A646C, &D_WSTAG630_800A6478, &D_WSTAG630_800A6484, &D_WSTAG630_800A6490,
        &D_WSTAG630_800A649C, &D_WSTAG630_800A64A8, &D_WSTAG630_800A64B4 },
};
FieldstgBattleLists wstag630_battle_lists = {
    36, 0, 0, { &D_WSTAG630_800A6334, &D_WSTAG630_800A63B8, &D_WSTAG630_800A643C }, &D_WSTAG630_800A64C0,
};
FieldstgVramPlace wstag630_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 392, 256, 288, 0, 368, 511 }, { 384, 256, 400, 256, 320, 0, 320, 510 },
};
u16 D_WSTAG630_800A6580[4] = { 0x1C1C, 0, 0xFFFF, 0 };
u16 D_WSTAG630_800A6588[4] = { 0x1C1C, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A6590[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG630_800A6598[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A65A0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A65A8[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG630_800A65B0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A65B8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A65C0[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG630_800A65C8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A65D0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A65D8[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG630_800A65E0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A65E8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A65F0[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG630_800A65F8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A6600[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A6608[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG630_800A6610[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A6618[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A6620[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG630_800A6628[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A6630[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A6638[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG630_800A6640[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A6648[4] = { 0, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG630_800A6650[2] = { { NULL, NULL, 114 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG630_800A6668[3] = {
    { D_WSTAG630_800A6580, NULL, 441 }, { D_WSTAG630_800A6588, NULL, 74 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG630_800A668C[2] = { { NULL, NULL, 445 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG630_800A66A4[2] = { { NULL, NULL, 444 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG630_800A66BC[2] = { { NULL, NULL, 443 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG630_800A66D4[2] = { { NULL, NULL, 442 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG630_800A66EC[2] = { { NULL, NULL, 441 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG630_800A6704[2] = { { NULL, NULL, 115 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG630_800A671C[2] = { { NULL, NULL, 430 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG630_800A6734[3] = {
    { D_WSTAG630_800A6590, D_WSTAG630_800A6598, 447 }, { D_WSTAG630_800A65A0, NULL, 4 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG630_800A6758[2] = { { NULL, NULL, 455 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG630_800A6770[3] = {
    { D_WSTAG630_800A65A8, D_WSTAG630_800A65B0, 454 }, { D_WSTAG630_800A65B8, NULL, 4 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG630_800A6794[3] = {
    { D_WSTAG630_800A65C0, D_WSTAG630_800A65C8, 453 }, { D_WSTAG630_800A65D0, NULL, 4 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG630_800A67B8[3] = {
    { D_WSTAG630_800A65D8, D_WSTAG630_800A65E0, 452 }, { D_WSTAG630_800A65E8, NULL, 4 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG630_800A67DC[3] = {
    { D_WSTAG630_800A65F0, D_WSTAG630_800A65F8, 451 }, { D_WSTAG630_800A6600, NULL, 4 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG630_800A6800[3] = {
    { D_WSTAG630_800A6608, D_WSTAG630_800A6610, 450 }, { D_WSTAG630_800A6618, NULL, 4 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG630_800A6824[3] = {
    { D_WSTAG630_800A6620, D_WSTAG630_800A6628, 449 }, { D_WSTAG630_800A6630, NULL, 4 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG630_800A6848[3] = {
    { D_WSTAG630_800A6638, D_WSTAG630_800A6640, 448 }, { D_WSTAG630_800A6648, NULL, 4 }, { NULL, NULL, 0 },
};
u16 D_WSTAG630_800A686C[4] = { 0x600F, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A6874[4] = { 0x6014, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A687C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A6884[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A688C[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A6894[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A689C[6] = { 0x7017, 1, 0x6014, 0, 0xFFFF, 0 };
u16 D_WSTAG630_800A68A8[4] = { 0x6010, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A68B0[8] = { 0x7016, 1, 0x6010, 0, 0x600F, 0, 0xFFFF, 0 };
u16 D_WSTAG630_800A68C0[4] = { 0x600F, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A68C8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A68D0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A68D8[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A68E0[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A68E8[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A68F0[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A68F8[8] = { 0x600F, 0, 0x6010, 0, 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG630_800A6908[4] = { 0x6010, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG630_800A6910 = { D_WSTAG630_800A686C, D_WSTAG630_800A6650, 34, 4, 433, 280, 7 };
FieldstgPlacedActor D_WSTAG630_800A6924 = { D_WSTAG630_800A6874, D_WSTAG630_800A6668, 34, 4, 433, 280, 7 };
FieldstgPlacedActor D_WSTAG630_800A6938 = { D_WSTAG630_800A687C, D_WSTAG630_800A668C, 34, 4, 433, 280, 7 };
FieldstgPlacedActor D_WSTAG630_800A694C = { D_WSTAG630_800A6884, D_WSTAG630_800A66A4, 34, 4, 433, 280, 7 };
FieldstgPlacedActor D_WSTAG630_800A6960 = { D_WSTAG630_800A688C, D_WSTAG630_800A66BC, 34, 4, 433, 280, 7 };
FieldstgPlacedActor D_WSTAG630_800A6974 = { D_WSTAG630_800A6894, D_WSTAG630_800A66D4, 34, 4, 433, 280, 7 };
FieldstgPlacedActor D_WSTAG630_800A6988 = { D_WSTAG630_800A689C, D_WSTAG630_800A66EC, 34, 4, 433, 280, 7 };
FieldstgPlacedActor D_WSTAG630_800A699C = { D_WSTAG630_800A68A8, D_WSTAG630_800A6704, 34, 4, 433, 280, 7 };
FieldstgPlacedActor D_WSTAG630_800A69B0 = { D_WSTAG630_800A68B0, D_WSTAG630_800A671C, 34, 4, 433, 280, 7 };
FieldstgPlacedActor D_WSTAG630_800A69C4 = { D_WSTAG630_800A68C0, D_WSTAG630_800A6734, 64, 5, 433, 537, 1 };
FieldstgPlacedActor D_WSTAG630_800A69D8 = { D_WSTAG630_800A68C8, D_WSTAG630_800A6758, 64, 5, 433, 537, 1 };
FieldstgPlacedActor D_WSTAG630_800A69EC = { D_WSTAG630_800A68D0, D_WSTAG630_800A6770, 64, 5, 433, 537, 1 };
FieldstgPlacedActor D_WSTAG630_800A6A00 = { D_WSTAG630_800A68D8, D_WSTAG630_800A6794, 64, 5, 433, 537, 1 };
FieldstgPlacedActor D_WSTAG630_800A6A14 = { D_WSTAG630_800A68E0, D_WSTAG630_800A67B8, 64, 5, 433, 537, 1 };
FieldstgPlacedActor D_WSTAG630_800A6A28 = { D_WSTAG630_800A68E8, D_WSTAG630_800A67DC, 64, 5, 433, 537, 1 };
FieldstgPlacedActor D_WSTAG630_800A6A3C = { D_WSTAG630_800A68F0, D_WSTAG630_800A6800, 64, 5, 433, 537, 1 };
FieldstgPlacedActor D_WSTAG630_800A6A50 = { D_WSTAG630_800A68F8, D_WSTAG630_800A6824, 64, 5, 433, 537, 1 };
FieldstgPlacedActor D_WSTAG630_800A6A64 = { D_WSTAG630_800A6908, D_WSTAG630_800A6848, 64, 5, 433, 537, 1 };
FieldstgPlacedActor *wstag630_actors[19] = {
    &D_WSTAG630_800A6910, &D_WSTAG630_800A6924, &D_WSTAG630_800A6938, &D_WSTAG630_800A694C, &D_WSTAG630_800A6960,
    &D_WSTAG630_800A6974, &D_WSTAG630_800A6988, &D_WSTAG630_800A699C, &D_WSTAG630_800A69B0, &D_WSTAG630_800A69C4,
    &D_WSTAG630_800A69D8, &D_WSTAG630_800A69EC, &D_WSTAG630_800A6A00, &D_WSTAG630_800A6A14, &D_WSTAG630_800A6A28,
    &D_WSTAG630_800A6A3C, &D_WSTAG630_800A6A50, &D_WSTAG630_800A6A64, NULL,
};
FieldstgSprite wstag630_sprites[12] = {
    { 1, 3, 0x40, 2, 0x48, 0, 0, 0, 0, 0, 302, 396, 0, 0 }, { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 120, 716, 0, 0 },
    { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 277, 632, 0, 0 }, { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 342, 321, 0, 0 },
    { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 604, 413, 0, 0 }, { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 726, 314, 0, 0 },
    { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 752, 174, 0, 0 }, { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 772, 512, 0, 0 },
    { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 949, 77, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 541, 275, 326, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 364, 459, 509, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag630_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x248, 0x90, 0x540, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x258, 0x340, 0xF0, 1, 0, 1, 1 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag630_funcs = { wstag630_setup };
