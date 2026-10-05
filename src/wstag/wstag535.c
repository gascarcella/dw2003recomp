#include "wstag.h"

/* WSTAG535: stage 0x243 (fieldstg_stages). */

extern WstagFuncs wstag535_funcs;
extern FieldstgBattleLists wstag535_battle_lists;
extern FieldstgVramPlace wstag535_vram_places[];
extern FieldstgPlacedActor *wstag535_actors[];
extern FieldstgSprite wstag535_sprites[];
extern FieldstgMapEvent wstag535_map_events[];
extern FieldstgEventDef wstag535_events[];
void wstag535_update();

void wstag535_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x4055, 1) && gamestate_flags.get_flag(0x4056, 0)) {
            data->event = fieldstg_event_start(0x4EF);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag535_start(void *arg0) {
    WstagObject *obj = object_new(wstag535_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag535_funcs.setup();
    return obj;
}

void wstag535_event_1262_end(void) {
    gamestate_flags.set_flag(0x4055, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag535_event_1263_end(void) {
    gamestate_flags.set_flag(0x4056, 1);
    gamestate_flags.set_flag(0x8666, 1);
}

void wstag535_setup(void) {
    fieldstg_stage.background_file = 0x293;
    fieldstg_stage.sprites = wstag535_sprites;
    fieldstg_stage.map_events = wstag535_map_events;
    fieldstg_stage.sprite_file = 0x02940001;
    fieldstg_stage.mask_file = 0x3DE;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x19E00, 0x11300 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag535_vram_places;
    fieldstg_stage.music = 0x11;
    fieldstg_stage.sound = 0x60440000;
    fieldstg_stage.actors = wstag535_actors;
    fieldstg_stage.battle_lists = &wstag535_battle_lists;
    fieldstg_stage.events = wstag535_events;
    fieldstg_attr.set_file(0, 0x02940000);
    fieldstg_attr.set_file(7, 0x02940002);
    fieldstg_attr.set_file(4, 0x02940003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag535_setup(void);

s16 D_WSTAG535_800A6090[89] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 855, 147, 5),
    FIELDSTG_EVENT_PLACE(133, 887, 133),
    FIELDSTG_EVENT_ANIM(133, 1, 1),
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
    FIELDSTG_EVENT_DIALOG(0, 2, 133, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 133, 0),
    FIELDSTG_EVENT_DIALOG(0, 0, 0, 0),
    FIELDSTG_EVENT_DIALOG(0, 4, 133, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG535_800A6144[82] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 855, 147),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(133, 887, 133),
    FIELDSTG_EVENT_ANIM(133, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 133, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 133, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 133, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG535_800A61E8 = { 91, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A61F4 = { 91, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A6200 = { 91, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A620C = { 91, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A6218 = { 91, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A6224 = { 91, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A6230 = { 91, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A623C = { 91, 6, 0x60080000 };
FieldstgBattleList D_WSTAG535_800A6248 = {
    4,
    { &D_WSTAG535_800A61E8, &D_WSTAG535_800A61F4, &D_WSTAG535_800A6200, &D_WSTAG535_800A620C, &D_WSTAG535_800A6218,
        &D_WSTAG535_800A6224, &D_WSTAG535_800A6230, &D_WSTAG535_800A623C },
};
FieldstgListedBattle D_WSTAG535_800A626C = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A6278 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A6284 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A6290 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A629C = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A62A8 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A62B4 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A62C0 = { 0, 6, 0x60080000 };
FieldstgBattleList D_WSTAG535_800A62CC = {
    0,
    { &D_WSTAG535_800A626C, &D_WSTAG535_800A6278, &D_WSTAG535_800A6284, &D_WSTAG535_800A6290, &D_WSTAG535_800A629C,
        &D_WSTAG535_800A62A8, &D_WSTAG535_800A62B4, &D_WSTAG535_800A62C0 },
};
FieldstgListedBattle D_WSTAG535_800A62F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG535_800A62FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG535_800A6308 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG535_800A6314 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG535_800A6320 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG535_800A632C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG535_800A6338 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG535_800A6344 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG535_800A6350 = {
    0,
    { &D_WSTAG535_800A62F0, &D_WSTAG535_800A62FC, &D_WSTAG535_800A6308, &D_WSTAG535_800A6314, &D_WSTAG535_800A6320,
        &D_WSTAG535_800A632C, &D_WSTAG535_800A6338, &D_WSTAG535_800A6344 },
};
FieldstgListedBattle D_WSTAG535_800A6374 = { 11, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG535_800A6380 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG535_800A638C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG535_800A6398 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG535_800A63A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG535_800A63B0 = { 91, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG535_800A63BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG535_800A63C8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG535_800A63D4 = {
    0,
    { &D_WSTAG535_800A6374, &D_WSTAG535_800A6380, &D_WSTAG535_800A638C, &D_WSTAG535_800A6398, &D_WSTAG535_800A63A4,
        &D_WSTAG535_800A63B0, &D_WSTAG535_800A63BC, &D_WSTAG535_800A63C8 },
};
FieldstgBattleLists wstag535_battle_lists = {
    59, 0, 0, { &D_WSTAG535_800A6248, &D_WSTAG535_800A62CC, &D_WSTAG535_800A6350 }, &D_WSTAG535_800A63D4,
};
FieldstgVramPlace wstag535_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 384, 256, 256, 0, 368, 501 },
};
u16 D_WSTAG535_800A6484[4] = { 0xA02, 0, 0xFFFF, 0 };
u16 D_WSTAG535_800A648C[6] = { 0xA02, 1, 0x9036, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6498[4] = { 0xA02, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A64A0[4] = { 0xA02, 0, 0xFFFF, 0 };
u16 D_WSTAG535_800A64A8[6] = { 0xA02, 1, 0x9036, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A64B4[4] = { 0xA02, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A64BC[4] = { 0xA02, 0, 0xFFFF, 0 };
u16 D_WSTAG535_800A64C4[6] = { 0xA02, 1, 0x9036, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A64D0[4] = { 0xA02, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A64D8[4] = { 0xA02, 0, 0xFFFF, 0 };
u16 D_WSTAG535_800A64E0[6] = { 0xA02, 1, 0x9036, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A64EC[4] = { 0xA02, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A64F4[4] = { 0xA02, 0, 0xFFFF, 0 };
u16 D_WSTAG535_800A64FC[6] = { 0xA02, 1, 0x9036, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6508[4] = { 0xA02, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6510[4] = { 0xA02, 0, 0xFFFF, 0 };
u16 D_WSTAG535_800A6518[6] = { 0x9036, 1, 0xA02, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6524[4] = { 0xA02, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A652C[4] = { 0xA02, 0, 0xFFFF, 0 };
u16 D_WSTAG535_800A6534[6] = { 0xA02, 1, 0x9036, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6540[4] = { 0xA02, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6548[4] = { 0xA02, 0, 0xFFFF, 0 };
u16 D_WSTAG535_800A6550[6] = { 0xA02, 1, 0x9036, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A655C[4] = { 0xA02, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6564[4] = { 0xA02, 0, 0xFFFF, 0 };
u16 D_WSTAG535_800A656C[6] = { 0xA02, 1, 0x9036, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6578[4] = { 0xA02, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6580[4] = { 0xA02, 0, 0xFFFF, 0 };
u16 D_WSTAG535_800A6588[6] = { 0xA02, 1, 0x9036, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6594[4] = { 0xA02, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG535_800A659C[3] = {
    { D_WSTAG535_800A6484, D_WSTAG535_800A648C, 760 }, { D_WSTAG535_800A6498, NULL, 483 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG535_800A65C0[3] = {
    { D_WSTAG535_800A64A0, D_WSTAG535_800A64A8, 760 }, { D_WSTAG535_800A64B4, NULL, 483 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG535_800A65E4[3] = {
    { D_WSTAG535_800A64BC, D_WSTAG535_800A64C4, 760 }, { D_WSTAG535_800A64D0, NULL, 484 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG535_800A6608[3] = {
    { D_WSTAG535_800A64D8, D_WSTAG535_800A64E0, 760 }, { D_WSTAG535_800A64EC, NULL, 483 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG535_800A662C[3] = {
    { D_WSTAG535_800A64F4, D_WSTAG535_800A64FC, 760 }, { D_WSTAG535_800A6508, NULL, 490 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG535_800A6650[3] = {
    { D_WSTAG535_800A6510, D_WSTAG535_800A6518, 760 }, { D_WSTAG535_800A6524, NULL, 489 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG535_800A6674[3] = {
    { D_WSTAG535_800A652C, D_WSTAG535_800A6534, 760 }, { D_WSTAG535_800A6540, NULL, 488 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG535_800A6698[3] = {
    { D_WSTAG535_800A6548, D_WSTAG535_800A6550, 760 }, { D_WSTAG535_800A655C, NULL, 487 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG535_800A66BC[3] = {
    { D_WSTAG535_800A6564, D_WSTAG535_800A656C, 760 }, { D_WSTAG535_800A6578, NULL, 486 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG535_800A66E0[3] = {
    { D_WSTAG535_800A6580, D_WSTAG535_800A6588, 760 }, { D_WSTAG535_800A6594, NULL, 485 }, { NULL, NULL, 0 },
};
u16 D_WSTAG535_800A6704[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A670C[4] = { 0x6009, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6714[4] = { 0x600A, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A671C[4] = { 0x6007, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6724[4] = { 0x6019, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A672C[4] = { 0x6018, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6734[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A673C[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A6744[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG535_800A674C[4] = { 0x600C, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG535_800A6754 = { D_WSTAG535_800A6704, D_WSTAG535_800A659C, 133, 4, 887, 133, 1 };
FieldstgPlacedActor D_WSTAG535_800A6768 = { D_WSTAG535_800A670C, D_WSTAG535_800A65C0, 133, 4, 887, 133, 1 };
FieldstgPlacedActor D_WSTAG535_800A677C = { D_WSTAG535_800A6714, D_WSTAG535_800A65E4, 133, 4, 887, 133, 1 };
FieldstgPlacedActor D_WSTAG535_800A6790 = { D_WSTAG535_800A671C, D_WSTAG535_800A6608, 133, 4, 887, 133, 1 };
FieldstgPlacedActor D_WSTAG535_800A67A4 = { D_WSTAG535_800A6724, D_WSTAG535_800A662C, 133, 4, 887, 133, 1 };
FieldstgPlacedActor D_WSTAG535_800A67B8 = { D_WSTAG535_800A672C, D_WSTAG535_800A6650, 133, 4, 887, 133, 1 };
FieldstgPlacedActor D_WSTAG535_800A67CC = { D_WSTAG535_800A6734, D_WSTAG535_800A6674, 133, 4, 887, 133, 1 };
FieldstgPlacedActor D_WSTAG535_800A67E0 = { D_WSTAG535_800A673C, D_WSTAG535_800A6698, 133, 4, 887, 133, 1 };
FieldstgPlacedActor D_WSTAG535_800A67F4 = { D_WSTAG535_800A6744, D_WSTAG535_800A66BC, 133, 4, 887, 133, 1 };
FieldstgPlacedActor D_WSTAG535_800A6808 = { D_WSTAG535_800A674C, D_WSTAG535_800A66E0, 133, 4, 887, 133, 1 };
FieldstgPlacedActor *wstag535_actors[11] = {
    &D_WSTAG535_800A6754, &D_WSTAG535_800A6768, &D_WSTAG535_800A677C, &D_WSTAG535_800A6790, &D_WSTAG535_800A67A4,
    &D_WSTAG535_800A67B8, &D_WSTAG535_800A67CC, &D_WSTAG535_800A67E0, &D_WSTAG535_800A67F4, &D_WSTAG535_800A6808,
    NULL,
};
FieldstgSprite wstag535_sprites[28] = {
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
FieldstgMapEvent wstag535_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x242, 0x3E0, 0x1B0, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x3A0, 0x190, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x3B0, 0x1E8, 0, 0, 0, 0 }, { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag535_funcs = { wstag535_setup };
FieldstgEventDef wstag535_events[4] = {
    { 1262, D_WSTAG535_800A6090, 0x013C000C, NULL, wstag535_event_1262_end },
    { 1263, D_WSTAG535_800A6144, 0x013C000D, NULL, wstag535_event_1263_end },
    { 9000, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
