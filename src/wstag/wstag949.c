#include "wstag.h"

/* WSTAG949: stage 0x295 (fieldstg_stages_2d). */

extern GamestatePos D_WSTAG949_800A6174[];
extern WstagFuncs wstag949_funcs;
extern CVECTOR wstag949_color;
extern FieldstgVramPlace wstag949_vram_places[];
extern FieldstgPlacedActor *wstag949_actors[];
extern FieldstgSprite wstag949_sprites[];
extern FieldstgMapEvent wstag949_map_events[];
extern FieldstgEventDef wstag949_events[];
extern FieldstgBattleLists wstag949_battle_lists;
void wstag949_parallax_update();

void wstag949_parallax_update(Object *obj) {
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
            spr.draw(cdload_module.get_subfile_by_id(0x9190000), 0, D_WSTAG949_800A6174[i].x + base.x,
                       D_WSTAG949_800A6174[i].y + base.y);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag949_parallax_new(void) {
    return object_new(wstag949_parallax_update, 0x50, 0);
}

void wstag949_update(WstagObject *obj, Object **data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        *data = wstag949_parallax_new();
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag949_start(void *arg0) {
    WstagObject *obj = object_new(wstag949_update, sizeof(WstagObject), sizeof(WstagEventData));

    obj->manager = arg0;
    wstag949_funcs.setup();
    return obj;
}

void wstag949_setup(void) {
    fieldstg_stage.background_file = 0x1BE;
    fieldstg_stage.sprite_file = 0x09190000;
    fieldstg_stage.sprites = wstag949_sprites;
    fieldstg_stage.map_events = wstag949_map_events;
    fieldstg_stage.mask_file = 0x918;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1AF00, 0x3B400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag949_vram_places;
    fieldstg_stage.music = 0xB;
    fieldstg_stage.sound = 0x602C0000;
    fieldstg_stage.actors = wstag949_actors;
    fieldstg_stage.color = wstag949_color;
    fieldstg_stage.events = wstag949_events;
    fieldstg_stage.battle_lists = &wstag949_battle_lists;
    fieldstg_attr.set_file(0, 0x09190002);
    fieldstg_attr.set_file(1, 0x09190003);
    fieldstg_attr.set_file(7, 0x09190004);
    fieldstg_attr.set_file(4, 0x09190001);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag949/nonmatchings/wstag949", wstag949_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag949_setup(void);

GamestatePos D_WSTAG949_800A6174[36] = {
    { 596, 399 }, { 809, 564 }, { 820, 832 }, { 1143, 1009 }, { 738, 1035 }, { 416, 1055 }, { 36, 28 }, { 140, 0 },
    { 178, 159 }, { 218, 202 }, { 186, 242 }, { 299, 93 }, { 418, 16 }, { 440, 329 }, { 476, 369 }, { 725, 82 },
    { 1137, 24 }, { 1213, 103 }, { 1281, 147 }, { 1349, 48 }, { 669, 632 }, { 491, 796 }, { 661, 889 },
    { 624, 926 }, { 998, 634 }, { 1026, 648 }, { 890, 940 }, { 915, 959 }, { 826, 1119 }, { 1028, 1089 },
    { 1227, 1002 }, { 1306, 861 }, { 498, 1190 }, { 146, 569 }, { 187, 590 }, { 394, 616 },
};
FieldstgVramPlace wstag949_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 362, 256, 168, 0, 320, 511 }, { 320, 256, 346, 256, 104, 0, 336, 511 },
    { 320, 256, 336, 256, 64, 0, 352, 511 }, { 320, 256, 354, 256, 136, 0, 368, 511 },
};
u16 D_WSTAG949_800A6334[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG949_800A6340[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG949_800A634C[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG949_800A6358[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG949_800A6368[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG949_800A6374[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG949_800A637C[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG949_800A6388[4] = { 0x7842, 1, 0xFFFF, 0 };
u16 D_WSTAG949_800A6390[4] = { 0x7C01, 1, 0xFFFF, 0 };
u16 D_WSTAG949_800A6398[4] = { 0x7096, 0, 0xFFFF, 0 };
u16 D_WSTAG949_800A63A0[6] = { 0x7096, 1, 0x100E, 0, 0xFFFF, 0 };
u16 D_WSTAG949_800A63AC[6] = { 0x7400, 1, 0x100E, 1, 0xFFFF, 0 };
u16 D_WSTAG949_800A63B8[6] = { 0x7096, 1, 0x100E, 1, 0xFFFF, 0 };
u16 D_WSTAG949_800A63C4[4] = { 0x7096, 0, 0xFFFF, 0 };
u16 D_WSTAG949_800A63CC[6] = { 0x7096, 1, 0x100F, 0, 0xFFFF, 0 };
u16 D_WSTAG949_800A63D8[6] = { 0x100F, 1, 0x7401, 1, 0xFFFF, 0 };
u16 D_WSTAG949_800A63E4[6] = { 0x7096, 1, 0x100F, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG949_800A63F0[2] = { { NULL, NULL, 57 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG949_800A6408[5] = {
    { D_WSTAG949_800A6334, D_WSTAG949_800A6340, 59 }, { D_WSTAG949_800A634C, D_WSTAG949_800A6358, 60 },
    { D_WSTAG949_800A6368, D_WSTAG949_800A6374, 57 }, { D_WSTAG949_800A637C, D_WSTAG949_800A6388, 58 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG949_800A6444[2] = { { NULL, D_WSTAG949_800A6390, 135 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG949_800A645C[4] = {
    { D_WSTAG949_800A6398, NULL, 129 }, { D_WSTAG949_800A63A0, D_WSTAG949_800A63AC, 130 },
    { D_WSTAG949_800A63B8, NULL, 131 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG949_800A648C[4] = {
    { D_WSTAG949_800A63C4, NULL, 132 }, { D_WSTAG949_800A63CC, D_WSTAG949_800A63D8, 133 },
    { D_WSTAG949_800A63E4, NULL, 134 }, { NULL, NULL, 0 },
};
u16 D_WSTAG949_800A64BC[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG949_800A64C4[4] = { 0x8192, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG949_800A64CC = { D_WSTAG949_800A64BC, D_WSTAG949_800A63F0, 57, 4, 320, 344, 7 };
FieldstgPlacedActor D_WSTAG949_800A64E0 = { D_WSTAG949_800A64C4, D_WSTAG949_800A6408, 57, 4, 320, 344, 7 };
FieldstgPlacedActor D_WSTAG949_800A64F4 = { NULL, D_WSTAG949_800A6444, 61, 5, 97, 242, 1 };
FieldstgPlacedActor D_WSTAG949_800A6508 = { NULL, D_WSTAG949_800A645C, 144, 6, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG949_800A651C = { NULL, D_WSTAG949_800A648C, 145, 7, 520, 220, 7 };
FieldstgPlacedActor *wstag949_actors[6] = {
    &D_WSTAG949_800A64CC, &D_WSTAG949_800A64E0, &D_WSTAG949_800A64F4, &D_WSTAG949_800A6508, &D_WSTAG949_800A651C,
    NULL,
};
FieldstgSprite wstag949_sprites[1] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
FieldstgMapEvent wstag949_map_events[10] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x294, 0x510, 0x88, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 5, 8, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 5, 4, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 6, 1, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 6, 0, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 6, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 5, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x7E, 0x1AF, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x6D, 0x117, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag949_funcs = { wstag949_setup };
FieldstgEventDef wstag949_events[2] = {
    { 9000, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
FieldstgListedBattle D_WSTAG949_800A6678 = { 46, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG949_800A6684 = { 46, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG949_800A6690 = { 150, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG949_800A669C = { 150, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG949_800A66A8 = { 96, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG949_800A66B4 = { 96, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG949_800A66C0 = { 97, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG949_800A66CC = { 97, 7, 0x60080000 };
FieldstgBattleList D_WSTAG949_800A66D8 = {
    3,
    { &D_WSTAG949_800A6678, &D_WSTAG949_800A6684, &D_WSTAG949_800A6690, &D_WSTAG949_800A669C, &D_WSTAG949_800A66A8,
        &D_WSTAG949_800A66B4, &D_WSTAG949_800A66C0, &D_WSTAG949_800A66CC },
};
FieldstgListedBattle D_WSTAG949_800A66FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A6708 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A6714 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A6720 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A672C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A6738 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A6744 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A6750 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG949_800A675C = {
    0,
    { &D_WSTAG949_800A66FC, &D_WSTAG949_800A6708, &D_WSTAG949_800A6714, &D_WSTAG949_800A6720, &D_WSTAG949_800A672C,
        &D_WSTAG949_800A6738, &D_WSTAG949_800A6744, &D_WSTAG949_800A6750 },
};
FieldstgListedBattle D_WSTAG949_800A6780 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A678C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A6798 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A67A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A67B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A67BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A67C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A67D4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG949_800A67E0 = {
    0,
    { &D_WSTAG949_800A6780, &D_WSTAG949_800A678C, &D_WSTAG949_800A6798, &D_WSTAG949_800A67A4, &D_WSTAG949_800A67B0,
        &D_WSTAG949_800A67BC, &D_WSTAG949_800A67C8, &D_WSTAG949_800A67D4 },
};
FieldstgListedBattle D_WSTAG949_800A6804 = { 306, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG949_800A6810 = { 307, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG949_800A681C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A6828 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A6834 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A6840 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A684C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG949_800A6858 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG949_800A6864 = {
    0,
    { &D_WSTAG949_800A6804, &D_WSTAG949_800A6810, &D_WSTAG949_800A681C, &D_WSTAG949_800A6828, &D_WSTAG949_800A6834,
        &D_WSTAG949_800A6840, &D_WSTAG949_800A684C, &D_WSTAG949_800A6858 },
};
FieldstgBattleLists wstag949_battle_lists = {
    387, 0, 0, { &D_WSTAG949_800A66D8, &D_WSTAG949_800A675C, &D_WSTAG949_800A67E0 }, &D_WSTAG949_800A6864,
};
