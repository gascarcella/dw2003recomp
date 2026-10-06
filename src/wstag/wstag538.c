#include "wstag.h"

/* WSTAG538: stage 0x2B0 (fieldstg_stages). */

extern WstagFuncs wstag538_funcs;
extern FieldstgBattleLists wstag538_battle_lists;
extern FieldstgVramPlace wstag538_vram_places[];
extern FieldstgPlacedActor *wstag538_actors[];
extern FieldstgSprite wstag538_sprites[];
extern FieldstgMapEvent wstag538_map_events[];
extern FieldstgEventDef wstag538_events[];
void wstag538_update();

void wstag538_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x407C, 1) && gamestate_flags.get_flag(0x407D, 0)) {
            data->event = fieldstg_event_start(0x508);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag538_start(void *arg0) {
    WstagObject *obj = object_new(wstag538_update, sizeof(WstagObject), sizeof(FieldstgEvent *));

    obj->manager = arg0;
    wstag538_funcs.setup();
    return obj;
}

void wstag538_event_1287_end(void) {
    gamestate_flags.set_flag(0x407C, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag538_event_1288_end(void) {
    gamestate_flags.set_flag(0x407D, 1);
    gamestate_flags.set_flag(0x8B00, 1);
}

void wstag538_setup(void) {
    fieldstg_stage.background_file = 0x5CD;
    fieldstg_stage.sprite_file = 0x05CE0000;
    fieldstg_stage.sprites = wstag538_sprites;
    fieldstg_stage.map_events = wstag538_map_events;
    fieldstg_stage.mask_file = 0x5CC;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0xCE00, 0xA700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag538_vram_places;
    fieldstg_stage.music = 0x11;
    fieldstg_stage.sound = 0x60440000;
    fieldstg_stage.actors = wstag538_actors;
    fieldstg_stage.battle_lists = &wstag538_battle_lists;
    fieldstg_stage.events = wstag538_events;
    fieldstg_attr.set_file(0, 0x05CE0001);
    fieldstg_attr.set_file(7, 0x05CE0002);
    fieldstg_attr.set_file(4, 0x05CE0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag538_setup(void);

s16 D_WSTAG538_800A6088[80] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 865, 148, 5),
    FIELDSTG_EVENT_PLACE(263, 887, 133),
    FIELDSTG_EVENT_ANIM(263, 1, 1),
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
    FIELDSTG_EVENT_DIALOG(0, 2, 263, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 263, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x300, /* padding, not read */
};
s16 D_WSTAG538_800A6128[82] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 865, 148),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(263, 887, 133),
    FIELDSTG_EVENT_ANIM(263, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 263, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 263, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 263, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG538_800A61CC = { 108, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A61D8 = { 108, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A61E4 = { 108, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A61F0 = { 108, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A61FC = { 108, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A6208 = { 108, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A6214 = { 108, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A6220 = { 108, 6, 0x60080000 };
FieldstgBattleList D_WSTAG538_800A622C = {
    4,
    { &D_WSTAG538_800A61CC, &D_WSTAG538_800A61D8, &D_WSTAG538_800A61E4, &D_WSTAG538_800A61F0, &D_WSTAG538_800A61FC,
        &D_WSTAG538_800A6208, &D_WSTAG538_800A6214, &D_WSTAG538_800A6220 },
};
FieldstgListedBattle D_WSTAG538_800A6250 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A625C = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A6268 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A6274 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A6280 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A628C = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A6298 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A62A4 = { 0, 6, 0x60080000 };
FieldstgBattleList D_WSTAG538_800A62B0 = {
    0,
    { &D_WSTAG538_800A6250, &D_WSTAG538_800A625C, &D_WSTAG538_800A6268, &D_WSTAG538_800A6274, &D_WSTAG538_800A6280,
        &D_WSTAG538_800A628C, &D_WSTAG538_800A6298, &D_WSTAG538_800A62A4 },
};
FieldstgListedBattle D_WSTAG538_800A62D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG538_800A62E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG538_800A62EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG538_800A62F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG538_800A6304 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG538_800A6310 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG538_800A631C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG538_800A6328 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG538_800A6334 = {
    0,
    { &D_WSTAG538_800A62D4, &D_WSTAG538_800A62E0, &D_WSTAG538_800A62EC, &D_WSTAG538_800A62F8, &D_WSTAG538_800A6304,
        &D_WSTAG538_800A6310, &D_WSTAG538_800A631C, &D_WSTAG538_800A6328 },
};
FieldstgListedBattle D_WSTAG538_800A6358 = { 17, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG538_800A6364 = { 318, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG538_800A6370 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG538_800A637C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG538_800A6388 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG538_800A6394 = { 108, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG538_800A63A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG538_800A63AC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG538_800A63B8 = {
    0,
    { &D_WSTAG538_800A6358, &D_WSTAG538_800A6364, &D_WSTAG538_800A6370, &D_WSTAG538_800A637C, &D_WSTAG538_800A6388,
        &D_WSTAG538_800A6394, &D_WSTAG538_800A63A0, &D_WSTAG538_800A63AC },
};
FieldstgBattleLists wstag538_battle_lists = {
    80, 0, 0, { &D_WSTAG538_800A622C, &D_WSTAG538_800A62B0, &D_WSTAG538_800A6334 }, &D_WSTAG538_800A63B8,
};
FieldstgVramPlace wstag538_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 256, 216, 0, 368, 501 }, { 384, 256, 384, 256, 256, 0, 352, 500 },
};
u16 D_WSTAG538_800A6478[4] = { 0x8675, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A6480[6] = { 0x8675, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG538_800A648C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A6494[8] = { 0x8675, 0, 0, 1, 0x8471, 0, 0xFFFF, 0 };
u16 D_WSTAG538_800A64A4[8] = { 0x8675, 0, 0, 1, 0x8471, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A64B4[10] = {
    0x8675, 1, 0x8674, 0, 0x8471, 0, 0x7013, 1,
    0xFFFF, 0,
};
u16 D_WSTAG538_800A64C8[4] = { 0xA0A, 0, 0xFFFF, 0 };
u16 D_WSTAG538_800A64D0[6] = { 0xA0A, 1, 0x903B, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A64DC[4] = { 0xA0A, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A64E4[4] = { 0xA0A, 0, 0xFFFF, 0 };
u16 D_WSTAG538_800A64EC[6] = { 0xA0A, 1, 0x903B, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A64F8[4] = { 0xA0A, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A6500[4] = { 0xA0A, 0, 0xFFFF, 0 };
u16 D_WSTAG538_800A6508[6] = { 0xA0A, 1, 0x903B, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A6514[4] = { 0xA0A, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A651C[4] = { 0xA0A, 0, 0xFFFF, 0 };
u16 D_WSTAG538_800A6524[6] = { 0xA0A, 1, 0x903B, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A6530[4] = { 0xA0A, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A6538[4] = { 0xA0A, 0, 0xFFFF, 0 };
u16 D_WSTAG538_800A6540[6] = { 0xA0A, 1, 0x903B, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A654C[4] = { 0xA0A, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG538_800A6554[5] = {
    { D_WSTAG538_800A6478, NULL, 742 }, { D_WSTAG538_800A6480, D_WSTAG538_800A648C, 743 },
    { D_WSTAG538_800A6494, NULL, 744 }, { D_WSTAG538_800A64A4, D_WSTAG538_800A64B4, 745 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG538_800A6590[3] = {
    { D_WSTAG538_800A64C8, D_WSTAG538_800A64D0, 724 }, { D_WSTAG538_800A64DC, NULL, 181 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG538_800A65B4[3] = {
    { D_WSTAG538_800A64E4, D_WSTAG538_800A64EC, 724 }, { D_WSTAG538_800A64F8, NULL, 185 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG538_800A65D8[3] = {
    { D_WSTAG538_800A6500, D_WSTAG538_800A6508, 724 }, { D_WSTAG538_800A6514, NULL, 184 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG538_800A65FC[3] = {
    { D_WSTAG538_800A651C, D_WSTAG538_800A6524, 724 }, { D_WSTAG538_800A6530, NULL, 183 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG538_800A6620[3] = {
    { D_WSTAG538_800A6538, D_WSTAG538_800A6540, 724 }, { D_WSTAG538_800A654C, NULL, 182 }, { NULL, NULL, 0 },
};
u16 D_WSTAG538_800A6644[10] = {
    0x7044, 1, 0x704C, 1, 0x8674, 1, 0x8675, 0,
    0xFFFF, 0,
};
u16 D_WSTAG538_800A6658[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A6660[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A6668[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A6670[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG538_800A6678[4] = { 0x6025, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG538_800A6680 = { D_WSTAG538_800A6644, D_WSTAG538_800A6554, 173, 4, 962, 353, 1 };
FieldstgPlacedActor D_WSTAG538_800A6694 = { D_WSTAG538_800A6658, D_WSTAG538_800A6590, 263, 5, 887, 133, 1 };
FieldstgPlacedActor D_WSTAG538_800A66A8 = { D_WSTAG538_800A6660, D_WSTAG538_800A65B4, 263, 5, 887, 133, 1 };
FieldstgPlacedActor D_WSTAG538_800A66BC = { D_WSTAG538_800A6668, D_WSTAG538_800A65D8, 263, 5, 887, 133, 1 };
FieldstgPlacedActor D_WSTAG538_800A66D0 = { D_WSTAG538_800A6670, D_WSTAG538_800A65FC, 263, 5, 887, 133, 1 };
FieldstgPlacedActor D_WSTAG538_800A66E4 = { D_WSTAG538_800A6678, D_WSTAG538_800A6620, 263, 5, 887, 133, 1 };
FieldstgPlacedActor *wstag538_actors[7] = {
    &D_WSTAG538_800A6680, &D_WSTAG538_800A6694, &D_WSTAG538_800A66A8, &D_WSTAG538_800A66BC, &D_WSTAG538_800A66D0,
    &D_WSTAG538_800A66E4, NULL,
};
FieldstgSprite wstag538_sprites[28] = {
    { 1, 0, 0x70, 2, 0x36, 1, 0x36, 0x3B, 6, 0, 949, 142, 0, 0 },
    { 1, 0, 0x70, 2, 0x3C, 1, 0x3C, 0x41, 6, 0, 941, 141, 0, 0 },
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 999, 304, 0, 0 }, { 1, 0, 0x78, 6, 0x32, 2, 0, 0xD, 8, 0, 1000, 393, 0, 0 },
    { 1, 0, 0x78, 6, 0x33, 2, 0, 0xD, 8, 0, 993, 389, 0, 0 },
    { 1, 0, 0x70, 6, 0x34, 2, 0, 0xD, 8, 0, 642, 153, 0, 0 },
    { 1, 0, 0x70, 6, 0x35, 2, 0, 0xD, 8, 0, 634, 144, 0, 0 },
    { 1, 0, 0x70, 6, 0x42, 1, 0x42, 0x47, 6, 0, 955, 71, 0, 0 },
    { 1, 0, 0x70, 6, 0x48, 1, 0x48, 0x4D, 6, 0, 946, 71, 0, 0 },
    { 1, 0, 0x70, 6, 0x4E, 1, 0x4E, 0x53, 6, 0, 618, 227, 0, 0 },
    { 1, 0, 0x70, 6, 0x4E, 1, 0x4E, 0x53, 6, 0, 730, 171, 0, 0 },
    { 1, 0, 0x70, 6, 0x54, 2, 0, 3, 6, 0, 614, 232, 0, 0 }, { 1, 0, 0x70, 6, 0x54, 2, 0, 3, 6, 0, 726, 176, 0, 0 },
    { 1, 0, 0x70, 6, 0x55, 2, 0, 3, 4, 0, 258, 102, 0, 0 }, { 1, 0, 0x70, 6, 0x55, 2, 0, 3, 4, 0, 531, 190, 0, 0 },
    { 1, 0, 0x70, 6, 0x55, 2, 0, 3, 4, 0, 758, 111, 0, 0 }, { 1, 0, 0x70, 6, 0x55, 2, 0, 3, 4, 0, 806, 87, 0, 0 },
    { 1, 0, 0x70, 6, 0x55, 2, 0, 3, 4, 0, 827, 275, 0, 0 }, { 1, 0, 0x70, 6, 0x55, 2, 0, 3, 4, 0, 859, 292, 0, 0 },
    { 1, 0, 0x70, 6, 0x55, 2, 0, 3, 4, 0, 1002, 301, 0, 0 }, { 1, 0, 0x70, 6, 0x56, 2, 0, 3, 4, 0, 266, 105, 0, 0 },
    { 1, 0, 0x70, 6, 0x56, 2, 0, 3, 4, 0, 539, 193, 0, 0 }, { 1, 0, 0x70, 6, 0x56, 2, 0, 3, 4, 0, 766, 114, 0, 0 },
    { 1, 0, 0x70, 6, 0x56, 2, 0, 3, 4, 0, 814, 90, 0, 0 }, { 1, 0, 0x70, 6, 0x56, 2, 0, 3, 4, 0, 835, 279, 0, 0 },
    { 1, 0, 0x70, 6, 0x56, 2, 0, 3, 4, 0, 867, 295, 0, 0 }, { 1, 0, 0x70, 6, 0x56, 2, 0, 3, 4, 0, 1010, 304, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag538_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2AF, 0x3E0, 0x1B0, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x3A0, 0x190, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x3B0, 0x1E8, 0, 0, 0, 0 }, { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag538_funcs = { wstag538_setup };
FieldstgEventDef wstag538_events[4] = {
    { 1287, D_WSTAG538_800A6088, 0x013C0014, NULL, wstag538_event_1287_end },
    { 1288, D_WSTAG538_800A6128, 0x013C0015, NULL, wstag538_event_1288_end },
    { 9000, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
