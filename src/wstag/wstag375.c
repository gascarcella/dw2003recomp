#include "wstag.h"

/* WSTAG375: stage 0x226 (fieldstg_stages). */

extern WstagFuncs wstag375_funcs;
void wstag375_update();
extern GamestatePos D_WSTAG375_800A63C8[];
const CVECTOR wstag375_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag375_battle_lists;
extern FieldstgBattleLists wstag375_battle_lists2;
extern FieldstgVramPlace wstag375_vram_places[];
extern FieldstgPlacedActor *wstag375_actors[];
extern FieldstgSprite wstag375_sprites[];
extern FieldstgMapEvent wstag375_map_events[];
extern FieldstgEventDef wstag375_events[];
void wstag375_parallax_update(Object *obj);

void wstag375_parallax_update(Object *obj) {
    Sprite spr;
    GamestatePos base;
    GamestatePos view;
    GfxLayer *layer;
    s32 i;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        sprite_init(&spr);
        spr.set_layer_id(0x1002, 0xC);
        spr.set_vram_pos(0x140, 0x100);
        spr.set_clut8_pos(0, 0x1F0);
        layer = gfx_module.funcs.get_layer(0x1002);
        layer->get_scroll(layer, &view.x);
        base.x = (view.x - 0x2C0) >> 3;
        base.y = (view.y - 0x280) >> 3;
        for (i = 0; i < 0x24; i++) {
            spr.draw(cdload_module.get_subfile_by_id(0x01BF0000), 0, D_WSTAG375_800A63C8[i].x + base.x,
                       D_WSTAG375_800A63C8[i].y + base.y);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag375_parallax_new(void) {
    return object_new(wstag375_parallax_update, sizeof(Object), 0);
}

void wstag375_update(WstagObject *obj, WstagObjEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->object = wstag375_parallax_new();
        if (gamestate_flags.get_flag(0x4053, 1) && gamestate_flags.get_flag(0x4054, 0)) {
            data->event = fieldstg_event_start(0x4ED);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag375_start(void *arg0) {
    WstagObject *obj = object_new(wstag375_update, sizeof(WstagObject), sizeof(WstagObjEventData));

    obj->manager = arg0;
    wstag375_funcs.setup();
    return obj;
}

void wstag375_event_1260_end(void) {
    gamestate_flags.set_flag(0x4053, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag375_event_1261_end(void) {
    gamestate_flags.set_flag(0x868D, 1);
    gamestate_flags.set_flag(0x4054, 1);
}

void wstag375_setup(void) {
    fieldstg_stage.background_file = 0x1BE;
    fieldstg_stage.sprite_file = 0x01BF0000;
    fieldstg_stage.sprites = wstag375_sprites;
    fieldstg_stage.map_events = wstag375_map_events;
    fieldstg_stage.mask_file = 0x2D1;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1AF00, 0x3B400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag375_vram_places;
    fieldstg_stage.music = 0xB;
    fieldstg_stage.sound = 0x602C0000;
    fieldstg_stage.actors = wstag375_actors;
    fieldstg_stage.color = wstag375_color;
    fieldstg_stage.events = wstag375_events;
    fieldstg_attr.set_file(0, 0x01BF0001);
    fieldstg_attr.set_file(1, 0x01BF0004);
    fieldstg_attr.set_file(7, 0x01BF0002);
    fieldstg_attr.set_file(4, 0x01BF0003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress < 0xE) {
        fieldstg_stage.battle_lists = &wstag375_battle_lists;
    } else {
        fieldstg_stage.battle_lists = &wstag375_battle_lists2;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag375_setup(void);

s16 D_WSTAG375_800A6284[80] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 1044, 159, 5),
    FIELDSTG_EVENT_PLACE(59, 1073, 145),
    FIELDSTG_EVENT_ANIM(59, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 59, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 59, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x640, /* padding, not read */
};
s16 D_WSTAG375_800A6324[82] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 1044, 159),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(59, 1073, 145),
    FIELDSTG_EVENT_ANIM(59, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 59, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 59, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 59, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
GamestatePos D_WSTAG375_800A63C8[36] = {
    { 596, 399 }, { 809, 564 }, { 820, 832 }, { 1143, 1009 }, { 738, 1035 }, { 416, 1055 }, { 36, 28 }, { 140, 0 },
    { 178, 159 }, { 218, 202 }, { 186, 242 }, { 299, 93 }, { 418, 16 }, { 440, 329 }, { 476, 369 }, { 725, 82 },
    { 1137, 24 }, { 1213, 103 }, { 1281, 147 }, { 1349, 48 }, { 669, 632 }, { 491, 796 }, { 661, 889 },
    { 624, 926 }, { 998, 634 }, { 1026, 648 }, { 890, 940 }, { 915, 959 }, { 826, 1119 }, { 1028, 1089 },
    { 1227, 1002 }, { 1306, 861 }, { 498, 1190 }, { 146, 569 }, { 187, 590 }, { 394, 616 },
};
FieldstgListedBattle D_WSTAG375_800A64E8 = { 47, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A64F4 = { 47, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6500 = { 47, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A650C = { 47, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6518 = { 45, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6524 = { 45, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6530 = { 45, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A653C = { 45, 7, 0x60080000 };
FieldstgBattleList D_WSTAG375_800A6548 = {
    3,
    { &D_WSTAG375_800A64E8, &D_WSTAG375_800A64F4, &D_WSTAG375_800A6500, &D_WSTAG375_800A650C, &D_WSTAG375_800A6518,
        &D_WSTAG375_800A6524, &D_WSTAG375_800A6530, &D_WSTAG375_800A653C },
};
FieldstgListedBattle D_WSTAG375_800A656C = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6578 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6584 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6590 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A659C = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A65A8 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A65B4 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A65C0 = { 0, 7, 0x60080000 };
FieldstgBattleList D_WSTAG375_800A65CC = {
    0,
    { &D_WSTAG375_800A656C, &D_WSTAG375_800A6578, &D_WSTAG375_800A6584, &D_WSTAG375_800A6590, &D_WSTAG375_800A659C,
        &D_WSTAG375_800A65A8, &D_WSTAG375_800A65B4, &D_WSTAG375_800A65C0 },
};
FieldstgListedBattle D_WSTAG375_800A65F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A65FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A6608 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A6614 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A6620 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A662C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A6638 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A6644 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG375_800A6650 = {
    0,
    { &D_WSTAG375_800A65F0, &D_WSTAG375_800A65FC, &D_WSTAG375_800A6608, &D_WSTAG375_800A6614, &D_WSTAG375_800A6620,
        &D_WSTAG375_800A662C, &D_WSTAG375_800A6638, &D_WSTAG375_800A6644 },
};
FieldstgListedBattle D_WSTAG375_800A6674 = { 1, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG375_800A6680 = { 309, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG375_800A668C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A6698 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A66A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A66B0 = { 47, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A66BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A66C8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG375_800A66D4 = {
    0,
    { &D_WSTAG375_800A6674, &D_WSTAG375_800A6680, &D_WSTAG375_800A668C, &D_WSTAG375_800A6698, &D_WSTAG375_800A66A4,
        &D_WSTAG375_800A66B0, &D_WSTAG375_800A66BC, &D_WSTAG375_800A66C8 },
};
FieldstgListedBattle D_WSTAG375_800A66F8 = { 47, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6704 = { 45, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6710 = { 46, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A671C = { 46, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6728 = { 46, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6734 = { 46, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6740 = { 46, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A674C = { 46, 7, 0x60080000 };
FieldstgBattleList D_WSTAG375_800A6758 = {
    3,
    { &D_WSTAG375_800A66F8, &D_WSTAG375_800A6704, &D_WSTAG375_800A6710, &D_WSTAG375_800A671C, &D_WSTAG375_800A6728,
        &D_WSTAG375_800A6734, &D_WSTAG375_800A6740, &D_WSTAG375_800A674C },
};
FieldstgListedBattle D_WSTAG375_800A677C = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6788 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A6794 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A67A0 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A67AC = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A67B8 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A67C4 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A67D0 = { 0, 7, 0x60080000 };
FieldstgBattleList D_WSTAG375_800A67DC = {
    0,
    { &D_WSTAG375_800A677C, &D_WSTAG375_800A6788, &D_WSTAG375_800A6794, &D_WSTAG375_800A67A0, &D_WSTAG375_800A67AC,
        &D_WSTAG375_800A67B8, &D_WSTAG375_800A67C4, &D_WSTAG375_800A67D0 },
};
FieldstgListedBattle D_WSTAG375_800A6800 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A680C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A6818 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A6824 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A6830 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A683C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A6848 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A6854 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG375_800A6860 = {
    0,
    { &D_WSTAG375_800A6800, &D_WSTAG375_800A680C, &D_WSTAG375_800A6818, &D_WSTAG375_800A6824, &D_WSTAG375_800A6830,
        &D_WSTAG375_800A683C, &D_WSTAG375_800A6848, &D_WSTAG375_800A6854 },
};
FieldstgListedBattle D_WSTAG375_800A6884 = { 1, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG375_800A6890 = { 309, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG375_800A689C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A68A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A68B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A68C0 = { 47, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG375_800A68CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG375_800A68D8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG375_800A68E4 = {
    0,
    { &D_WSTAG375_800A6884, &D_WSTAG375_800A6890, &D_WSTAG375_800A689C, &D_WSTAG375_800A68A8, &D_WSTAG375_800A68B4,
        &D_WSTAG375_800A68C0, &D_WSTAG375_800A68CC, &D_WSTAG375_800A68D8 },
};
FieldstgBattleLists wstag375_battle_lists = {
    11, 0, 0, { &D_WSTAG375_800A6548, &D_WSTAG375_800A65CC, &D_WSTAG375_800A6650 }, &D_WSTAG375_800A66D4,
};
FieldstgBattleLists wstag375_battle_lists2 = {
    30, 1, 0, { &D_WSTAG375_800A6758, &D_WSTAG375_800A67DC, &D_WSTAG375_800A6860 }, &D_WSTAG375_800A68E4,
};
FieldstgVramPlace wstag375_vram_places[14] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 354, 296, 136, 40, 368, 511 }, { 320, 256, 362, 296, 168, 40, 320, 510 },
    { 320, 256, 346, 256, 104, 0, 336, 510 }, { 320, 256, 336, 256, 64, 0, 352, 510 },
    { 320, 256, 354, 256, 136, 0, 368, 510 }, { 320, 256, 362, 256, 168, 0, 320, 509 },
    { 320, 256, 336, 304, 64, 48, 336, 509 }, { 320, 256, 370, 320, 200, 64, 352, 509 },
};
u16 D_WSTAG375_800A6A20[4] = { 0x700D, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6A28[6] = { 0x7013, 1, 0x7032, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6A34[4] = { 0x700D, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6A3C[4] = { 0x1A29, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6A44[4] = { 0x1A29, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6A4C[6] = { 0x1A29, 1, 0x702B, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6A58[8] = { 0x1A29, 1, 0x702B, 1, 0x7025, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6A68[4] = { 0x607, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6A70[8] = { 0x1A29, 1, 0x702B, 1, 0x7025, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6A80[4] = { 0x700D, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6A88[6] = { 0x7013, 1, 0x7032, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6A94[4] = { 0x700D, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6A9C[4] = { 0x1A29, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6AA4[4] = { 0x1A29, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6AAC[6] = { 0x1A29, 1, 0x702B, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6AB8[8] = { 0x1A29, 1, 0x702B, 1, 0x7025, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6AC8[4] = { 0x607, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6AD0[8] = { 0x1A29, 1, 0x702B, 1, 0x7025, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6AE0[4] = { 0xA00, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6AE8[6] = { 0xA00, 1, 0x9035, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6AF4[4] = { 0xA00, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6AFC[4] = { 0xA00, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B04[6] = { 0xA00, 1, 0x9035, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B10[4] = { 0xA00, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B18[4] = { 0xA00, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B20[6] = { 0xA00, 1, 0x9035, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B2C[4] = { 0xA00, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B34[4] = { 0xA00, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B3C[6] = { 0xA00, 1, 0x9035, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B48[4] = { 0xA00, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B50[4] = { 0xA00, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B58[6] = { 0xA00, 1, 0x9035, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B64[4] = { 0xA00, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B6C[4] = { 0xA00, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B74[6] = { 0xA00, 1, 0x9035, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B80[4] = { 0xA00, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B88[4] = { 0xA00, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B90[6] = { 0xA00, 1, 0x9035, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6B9C[4] = { 0xA00, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6BA4[4] = { 0xA00, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6BAC[6] = { 0xA00, 1, 0x9035, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6BB8[4] = { 0xA00, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6BC0[4] = { 0xA00, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6BC8[6] = { 0xA00, 1, 0x9035, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6BD4[4] = { 0xA00, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6BDC[4] = { 0xA00, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6BE4[6] = { 0xA00, 1, 0x9035, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6BF0[4] = { 0xA00, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6BF8[4] = { 0xA00, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C00[6] = { 0x9035, 1, 0xA00, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C0C[4] = { 0xA00, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C14[4] = { 0x1C0F, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C1C[6] = { 0x1C0F, 1, 5, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C28[6] = { 0x1C10, 1, 5, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C34[6] = { 0x1C0F, 1, 5, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C40[4] = { 0x1A17, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C48[6] = { 0x1A17, 1, 0x1A18, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C54[4] = { 0x1A18, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C5C[6] = { 0x1A17, 1, 0x1A18, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C68[6] = { 0x800E, 0, 0x1A2B, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C74[8] = { 0x800E, 0, 0x1A2B, 1, 0x1C4F, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C84[4] = { 0x1C4F, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6C8C[10] = {
    0x825A, 0, 0x800E, 0, 0x1A2B, 1, 0x1C4F, 1,
    0xFFFF, 0,
};
u16 D_WSTAG375_800A6CA0[10] = {
    0x800E, 0, 0x1A2B, 1, 0x825A, 1, 0x1C4F, 1,
    0xFFFF, 0,
};
u16 D_WSTAG375_800A6CB4[6] = { 0x7013, 1, 0x800E, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A6CC0[4] = { 0x800E, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG375_800A6CC8[2] = { { NULL, NULL, 237 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG375_800A6CE0[2] = { { NULL, NULL, 239 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG375_800A6CF8[3] = {
    { D_WSTAG375_800A6A20, D_WSTAG375_800A6A28, 677 }, { D_WSTAG375_800A6A34, NULL, 679 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6D1C[5] = {
    { D_WSTAG375_800A6A3C, D_WSTAG375_800A6A44, 675 }, { D_WSTAG375_800A6A4C, NULL, 680 },
    { D_WSTAG375_800A6A58, D_WSTAG375_800A6A68, 676 }, { D_WSTAG375_800A6A70, NULL, 681 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6D58[3] = {
    { D_WSTAG375_800A6A80, D_WSTAG375_800A6A88, 677 }, { D_WSTAG375_800A6A94, NULL, 679 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6D7C[5] = {
    { D_WSTAG375_800A6A9C, D_WSTAG375_800A6AA4, 675 }, { D_WSTAG375_800A6AAC, NULL, 680 },
    { D_WSTAG375_800A6AB8, D_WSTAG375_800A6AC8, 676 }, { D_WSTAG375_800A6AD0, NULL, 681 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6DB8[3] = {
    { D_WSTAG375_800A6AE0, D_WSTAG375_800A6AE8, 235 }, { D_WSTAG375_800A6AF4, NULL, 440 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6DDC[3] = {
    { D_WSTAG375_800A6AFC, D_WSTAG375_800A6B04, 235 }, { D_WSTAG375_800A6B10, NULL, 432 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6E00[3] = {
    { D_WSTAG375_800A6B18, D_WSTAG375_800A6B20, 235 }, { D_WSTAG375_800A6B2C, NULL, 433 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6E24[3] = {
    { D_WSTAG375_800A6B34, D_WSTAG375_800A6B3C, 235 }, { D_WSTAG375_800A6B48, NULL, 434 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6E48[3] = {
    { D_WSTAG375_800A6B50, D_WSTAG375_800A6B58, 235 }, { D_WSTAG375_800A6B64, NULL, 435 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6E6C[3] = {
    { D_WSTAG375_800A6B6C, D_WSTAG375_800A6B74, 235 }, { D_WSTAG375_800A6B80, NULL, 436 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6E90[3] = {
    { D_WSTAG375_800A6B88, D_WSTAG375_800A6B90, 235 }, { D_WSTAG375_800A6B9C, NULL, 437 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6EB4[3] = {
    { D_WSTAG375_800A6BA4, D_WSTAG375_800A6BAC, 235 }, { D_WSTAG375_800A6BB8, NULL, 438 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6ED8[3] = {
    { D_WSTAG375_800A6BC0, D_WSTAG375_800A6BC8, 235 }, { D_WSTAG375_800A6BD4, NULL, 439 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6EFC[3] = {
    { D_WSTAG375_800A6BDC, D_WSTAG375_800A6BE4, 235 }, { D_WSTAG375_800A6BF0, NULL, 431 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6F20[3] = {
    { D_WSTAG375_800A6BF8, D_WSTAG375_800A6C00, 235 }, { D_WSTAG375_800A6C0C, NULL, 744 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6F44[4] = {
    { D_WSTAG375_800A6C14, NULL, 236 }, { D_WSTAG375_800A6C1C, D_WSTAG375_800A6C28, 730 },
    { D_WSTAG375_800A6C34, NULL, 731 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6F74[4] = {
    { D_WSTAG375_800A6C40, NULL, 682 }, { D_WSTAG375_800A6C48, D_WSTAG375_800A6C54, 683 },
    { D_WSTAG375_800A6C5C, NULL, 684 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A6FA4[2] = { { NULL, NULL, 684 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG375_800A6FBC[2] = { { NULL, NULL, 685 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG375_800A6FD4[6] = {
    { D_WSTAG375_800A6C68, NULL, 686 }, { D_WSTAG375_800A6C74, D_WSTAG375_800A6C84, 687 },
    { D_WSTAG375_800A6C8C, NULL, 4 }, { D_WSTAG375_800A6CA0, D_WSTAG375_800A6CB4, 688 },
    { D_WSTAG375_800A6CC0, NULL, 689 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG375_800A701C[2] = { { NULL, NULL, 238 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG375_800A7034[2] = { { NULL, NULL, 678 }, { NULL, NULL, 0 } };
u16 D_WSTAG375_800A704C[6] = { 0x1C11, 0, 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A7058[6] = { 0x1C11, 0, 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A7064[6] = { 0x703B, 1, 0x8023, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A7070[6] = { 0x602B, 1, 0x8023, 0, 0xFFFF, 0 };
u16 D_WSTAG375_800A707C[6] = { 0x602B, 1, 0x8023, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A7088[6] = { 0x8023, 0, 0x703B, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A7094[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A709C[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A70A4[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A70AC[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A70B4[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A70BC[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A70C4[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A70CC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A70D4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A70DC[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A70E4[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A70EC[6] = { 0x1C11, 0, 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A70F8[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A7100[4] = { 0x6009, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A7108[4] = { 0x703C, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A7110[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A7118[6] = { 0x1C11, 0, 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG375_800A7124[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG375_800A712C = { D_WSTAG375_800A704C, D_WSTAG375_800A6CC8, 34, 4, 505, 213, 7 };
FieldstgPlacedActor D_WSTAG375_800A7140 = { D_WSTAG375_800A7058, D_WSTAG375_800A6CE0, 35, 5, 480, 225, 7 };
FieldstgPlacedActor D_WSTAG375_800A7154 = { D_WSTAG375_800A7064, D_WSTAG375_800A6CF8, 43, 6, 304, 328, 1 };
FieldstgPlacedActor D_WSTAG375_800A7168 = { D_WSTAG375_800A7070, D_WSTAG375_800A6D1C, 43, 6, 304, 328, 1 };
FieldstgPlacedActor D_WSTAG375_800A717C = { D_WSTAG375_800A707C, D_WSTAG375_800A6D58, 43, 6, 304, 328, 1 };
FieldstgPlacedActor D_WSTAG375_800A7190 = { D_WSTAG375_800A7088, D_WSTAG375_800A6D7C, 43, 6, 304, 328, 1 };
FieldstgPlacedActor D_WSTAG375_800A71A4 = { D_WSTAG375_800A7094, D_WSTAG375_800A6DB8, 59, 7, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG375_800A71B8 = { D_WSTAG375_800A709C, D_WSTAG375_800A6DDC, 59, 7, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG375_800A71CC = { D_WSTAG375_800A70A4, D_WSTAG375_800A6E00, 59, 7, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG375_800A71E0 = { D_WSTAG375_800A70AC, D_WSTAG375_800A6E24, 59, 7, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG375_800A71F4 = { D_WSTAG375_800A70B4, D_WSTAG375_800A6E48, 59, 7, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG375_800A7208 = { D_WSTAG375_800A70BC, D_WSTAG375_800A6E6C, 59, 7, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG375_800A721C = { D_WSTAG375_800A70C4, D_WSTAG375_800A6E90, 59, 7, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG375_800A7230 = { D_WSTAG375_800A70CC, D_WSTAG375_800A6EB4, 59, 7, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG375_800A7244 = { D_WSTAG375_800A70D4, D_WSTAG375_800A6ED8, 59, 7, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG375_800A7258 = { D_WSTAG375_800A70DC, D_WSTAG375_800A6EFC, 59, 7, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG375_800A726C = { D_WSTAG375_800A70E4, D_WSTAG375_800A6F20, 59, 7, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG375_800A7280 = { D_WSTAG375_800A70EC, D_WSTAG375_800A6F44, 60, 8, 560, 240, 3 };
FieldstgPlacedActor D_WSTAG375_800A7294 = { D_WSTAG375_800A70F8, D_WSTAG375_800A6F74, 61, 9, 97, 242, 7 };
FieldstgPlacedActor D_WSTAG375_800A72A8 = { D_WSTAG375_800A7100, D_WSTAG375_800A6FA4, 61, 9, 97, 242, 7 };
FieldstgPlacedActor D_WSTAG375_800A72BC = { D_WSTAG375_800A7108, D_WSTAG375_800A6FBC, 61, 9, 97, 242, 7 };
FieldstgPlacedActor D_WSTAG375_800A72D0 = { D_WSTAG375_800A7110, D_WSTAG375_800A6FD4, 61, 9, 97, 242, 7 };
FieldstgPlacedActor D_WSTAG375_800A72E4 = { D_WSTAG375_800A7118, D_WSTAG375_800A701C, 64, 10, 529, 201, 7 };
FieldstgPlacedActor D_WSTAG375_800A72F8 = { D_WSTAG375_800A7124, D_WSTAG375_800A7034, 157, 11, 304, 328, 1 };
FieldstgPlacedActor *wstag375_actors[25] = {
    &D_WSTAG375_800A712C, &D_WSTAG375_800A7140, &D_WSTAG375_800A7154, &D_WSTAG375_800A7168, &D_WSTAG375_800A717C,
    &D_WSTAG375_800A7190, &D_WSTAG375_800A71A4, &D_WSTAG375_800A71B8, &D_WSTAG375_800A71CC, &D_WSTAG375_800A71E0,
    &D_WSTAG375_800A71F4, &D_WSTAG375_800A7208, &D_WSTAG375_800A721C, &D_WSTAG375_800A7230, &D_WSTAG375_800A7244,
    &D_WSTAG375_800A7258, &D_WSTAG375_800A726C, &D_WSTAG375_800A7280, &D_WSTAG375_800A7294, &D_WSTAG375_800A72A8,
    &D_WSTAG375_800A72BC, &D_WSTAG375_800A72D0, &D_WSTAG375_800A72E4, &D_WSTAG375_800A72F8, NULL,
};
FieldstgSprite wstag375_sprites[1] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
FieldstgMapEvent wstag375_map_events[11] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x225, 0x510, 0x88, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 5, 8, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 5, 4, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 6, 1, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 6, 0, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 6, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 5, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x7E, 0x1AF, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x6D, 0x117, 0, 0, 0, 0 }, { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag375_funcs = { wstag375_setup };
FieldstgEventDef wstag375_events[4] = {
    { 1260, D_WSTAG375_800A6284, 0x0127001B, NULL, wstag375_event_1260_end },
    { 1261, D_WSTAG375_800A6324, 0x0127001C, NULL, wstag375_event_1261_end },
    { 9000, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
