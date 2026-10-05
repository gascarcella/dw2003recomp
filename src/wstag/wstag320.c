#include "wstag.h"

/* WSTAG320: stage 0x21B (fieldstg_stages). */

extern WstagFuncs wstag320_funcs;
const CVECTOR wstag320_color = { 0x54, 0x67, 0x96, 0 };
extern FieldstgBattleLists wstag320_battle_lists;
extern FieldstgVramPlace wstag320_vram_places[];
extern FieldstgPlacedActor *wstag320_actors[];
extern FieldstgSprite wstag320_sprites[];
extern FieldstgMapEvent wstag320_map_events[];
extern FieldstgEventDef wstag320_events[];
void wstag320_update();

void wstag320_update(WstagObject *obj) {
    FieldstgSprite *sprite;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_data.progress >= 0x16) {
            for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
                if (sprite->type == 10) {
                    sprite->shown = 0;
                }
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag320_start(void *arg0) {
    WstagObject *obj = object_new(wstag320_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag320_funcs.setup();
    return obj;
}

void wstag320_event_290_end(void) {
    gamestate_flags.set_flag(0x800F, 1);
}

void wstag320_setup(void) {
    fieldstg_stage.background_file = 0x2A3;
    fieldstg_stage.sprite_file = 0x02A40000;
    fieldstg_stage.sprites = wstag320_sprites;
    fieldstg_stage.map_events = wstag320_map_events;
    fieldstg_stage.mask_file = 0x326;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0x26F00, 0x1EF00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag320_vram_places;
    fieldstg_stage.music = 6;
    fieldstg_stage.sound = 0x60180000;
    fieldstg_stage.actors = wstag320_actors;
    fieldstg_stage.color = wstag320_color;
    fieldstg_stage.events = wstag320_events;
    fieldstg_stage.battle_lists = &wstag320_battle_lists;
    fieldstg_attr.set_file(0, 0x02A40001);
    fieldstg_attr.set_file(7, 0x02A40002);
    fieldstg_attr.set_file(4, 0x02A40003);
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
void wstag320_setup(void);

s16 D_WSTAG320_800A6094[134] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 385, 193, 7),
    FIELDSTG_EVENT_PLACE(55, 417, 209),
    FIELDSTG_EVENT_ANIM(55, 1, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 55, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 55, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 55, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 55, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 409, 181, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 349, 119, 3),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_GOTO_MAP(0x20B, 576, 320, 3),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG320_800A61A0 = { 85, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG320_800A61AC = { 85, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG320_800A61B8 = { 85, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG320_800A61C4 = { 85, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG320_800A61D0 = { 86, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG320_800A61DC = { 86, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG320_800A61E8 = { 86, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG320_800A61F4 = { 86, 7, 0x60080000 };
FieldstgBattleList D_WSTAG320_800A6200 = {
    4,
    { &D_WSTAG320_800A61A0, &D_WSTAG320_800A61AC, &D_WSTAG320_800A61B8, &D_WSTAG320_800A61C4, &D_WSTAG320_800A61D0,
        &D_WSTAG320_800A61DC, &D_WSTAG320_800A61E8, &D_WSTAG320_800A61F4 },
};
FieldstgListedBattle D_WSTAG320_800A6224 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A6230 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A623C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A6248 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A6254 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A6260 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A626C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A6278 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG320_800A6284 = {
    0,
    { &D_WSTAG320_800A6224, &D_WSTAG320_800A6230, &D_WSTAG320_800A623C, &D_WSTAG320_800A6248, &D_WSTAG320_800A6254,
        &D_WSTAG320_800A6260, &D_WSTAG320_800A626C, &D_WSTAG320_800A6278 },
};
FieldstgListedBattle D_WSTAG320_800A62A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A62B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A62C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A62CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A62D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A62E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A62F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A62FC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG320_800A6308 = {
    0,
    { &D_WSTAG320_800A62A8, &D_WSTAG320_800A62B4, &D_WSTAG320_800A62C0, &D_WSTAG320_800A62CC, &D_WSTAG320_800A62D8,
        &D_WSTAG320_800A62E4, &D_WSTAG320_800A62F0, &D_WSTAG320_800A62FC },
};
FieldstgListedBattle D_WSTAG320_800A632C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A6338 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A6344 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A6350 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A635C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A6368 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A6374 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG320_800A6380 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG320_800A638C = {
    0,
    { &D_WSTAG320_800A632C, &D_WSTAG320_800A6338, &D_WSTAG320_800A6344, &D_WSTAG320_800A6350, &D_WSTAG320_800A635C,
        &D_WSTAG320_800A6368, &D_WSTAG320_800A6374, &D_WSTAG320_800A6380 },
};
FieldstgBattleLists wstag320_battle_lists = {
    49, 0, 0, { &D_WSTAG320_800A6200, &D_WSTAG320_800A6284, &D_WSTAG320_800A6308 }, &D_WSTAG320_800A638C,
};
FieldstgVramPlace wstag320_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 440, 256, 480, 0, 368, 501 }, { 384, 256, 404, 336, 336, 80, 320, 500 },
    { 384, 256, 414, 304, 376, 48, 352, 500 }, { 384, 256, 412, 336, 368, 80, 368, 500 },
    { 384, 256, 420, 368, 400, 112, 320, 499 },
};
u16 D_WSTAG320_800A647C[8] = { 0x708C, 1, 0x200, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A648C[4] = { 0x1A16, 0, 0xFFFF, 0 };
u16 D_WSTAG320_800A6494[4] = { 0x1A15, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A649C[6] = { 0x800F, 0, 0x1A16, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A64A8[4] = { 0x9021, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A64B0[6] = { 0x800F, 1, 0x1A16, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A64BC[8] = { 0x7013, 1, 0x207, 1, 0x845E, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A64CC[4] = { 0x8015, 0, 0xFFFF, 0 };
u16 D_WSTAG320_800A64D4[6] = { 0x8015, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A64E0[4] = { 0x8015, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG320_800A64E8[2] = { { NULL, D_WSTAG320_800A647C, 1107 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG320_800A6500[4] = {
    { D_WSTAG320_800A648C, D_WSTAG320_800A6494, 84 }, { D_WSTAG320_800A649C, D_WSTAG320_800A64A8, 85 },
    { D_WSTAG320_800A64B0, NULL, 1109 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG320_800A6530[2] = { { NULL, NULL, 1109 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG320_800A6548[2] = { { NULL, NULL, 1110 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG320_800A6560[2] = { { NULL, NULL, 1114 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG320_800A6578[2] = { { NULL, NULL, 1112 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG320_800A6590[2] = { { NULL, NULL, 1117 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG320_800A65A8[2] = { { NULL, NULL, 1109 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG320_800A65C0[2] = { { NULL, NULL, 1113 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG320_800A65D8[2] = { { NULL, NULL, 1115 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG320_800A65F0[2] = { { NULL, NULL, 1111 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG320_800A6608[2] = { { NULL, D_WSTAG320_800A64BC, 1101 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG320_800A6620[2] = { { NULL, NULL, 137 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG320_800A6638[2] = { { NULL, NULL, 83 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG320_800A6650[3] = {
    { D_WSTAG320_800A64CC, D_WSTAG320_800A64D4, 86 }, { D_WSTAG320_800A64E0, NULL, 87 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG320_800A6674[2] = { { NULL, NULL, 1116 }, { NULL, NULL, 0 } };
u16 D_WSTAG320_800A668C[4] = { 0x200, 0, 0xFFFF, 0 };
u16 D_WSTAG320_800A6694[6] = { 0x600C, 1, 0x1A14, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A66A0[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A66A8[4] = { 0x6014, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A66B0[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A66B8[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A66C0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A66C8[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A66D0[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A66D8[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A66E0[4] = { 0x6015, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A66E8[4] = { 0x207, 0, 0xFFFF, 0 };
u16 D_WSTAG320_800A66F0[4] = { 0x600A, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A66F8[6] = { 0x600C, 1, 0x1A14, 0, 0xFFFF, 0 };
u16 D_WSTAG320_800A6704[6] = { 0x6009, 1, 0x1A1B, 1, 0xFFFF, 0 };
u16 D_WSTAG320_800A6710[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG320_800A6718 = { D_WSTAG320_800A668C, D_WSTAG320_800A64E8, 33, 4, 465, 169, 1 };
FieldstgPlacedActor D_WSTAG320_800A672C = { D_WSTAG320_800A6694, D_WSTAG320_800A6500, 55, 5, 417, 209, 7 };
FieldstgPlacedActor D_WSTAG320_800A6740 = { D_WSTAG320_800A66A0, D_WSTAG320_800A6530, 55, 5, 417, 209, 7 };
FieldstgPlacedActor D_WSTAG320_800A6754 = { D_WSTAG320_800A66A8, D_WSTAG320_800A6548, 55, 5, 417, 209, 7 };
FieldstgPlacedActor D_WSTAG320_800A6768 = { D_WSTAG320_800A66B0, D_WSTAG320_800A6560, 55, 5, 417, 209, 7 };
FieldstgPlacedActor D_WSTAG320_800A677C = { D_WSTAG320_800A66B8, D_WSTAG320_800A6578, 55, 5, 417, 209, 7 };
FieldstgPlacedActor D_WSTAG320_800A6790 = { D_WSTAG320_800A66C0, D_WSTAG320_800A6590, 55, 5, 417, 209, 7 };
FieldstgPlacedActor D_WSTAG320_800A67A4 = { D_WSTAG320_800A66C8, D_WSTAG320_800A65A8, 55, 5, 417, 209, 7 };
FieldstgPlacedActor D_WSTAG320_800A67B8 = { D_WSTAG320_800A66D0, D_WSTAG320_800A65C0, 55, 5, 417, 209, 7 };
FieldstgPlacedActor D_WSTAG320_800A67CC = { D_WSTAG320_800A66D8, D_WSTAG320_800A65D8, 55, 5, 417, 209, 7 };
FieldstgPlacedActor D_WSTAG320_800A67E0 = { D_WSTAG320_800A66E0, D_WSTAG320_800A65F0, 55, 5, 417, 209, 7 };
FieldstgPlacedActor D_WSTAG320_800A67F4 = { D_WSTAG320_800A66E8, D_WSTAG320_800A6608, 77, 6, 272, 217, 1 };
FieldstgPlacedActor D_WSTAG320_800A6808 = { D_WSTAG320_800A66F0, D_WSTAG320_800A6620, 97, 7, 417, 209, 7 };
FieldstgPlacedActor D_WSTAG320_800A681C = { D_WSTAG320_800A66F8, D_WSTAG320_800A6638, 97, 7, 417, 209, 7 };
FieldstgPlacedActor D_WSTAG320_800A6830 = { D_WSTAG320_800A6704, D_WSTAG320_800A6650, 97, 7, 417, 209, 7 };
FieldstgPlacedActor D_WSTAG320_800A6844 = { D_WSTAG320_800A6710, D_WSTAG320_800A6674, 157, 8, 417, 209, 7 };
FieldstgPlacedActor *wstag320_actors[17] = {
    &D_WSTAG320_800A6718, &D_WSTAG320_800A672C, &D_WSTAG320_800A6740, &D_WSTAG320_800A6754, &D_WSTAG320_800A6768,
    &D_WSTAG320_800A677C, &D_WSTAG320_800A6790, &D_WSTAG320_800A67A4, &D_WSTAG320_800A67B8, &D_WSTAG320_800A67CC,
    &D_WSTAG320_800A67E0, &D_WSTAG320_800A67F4, &D_WSTAG320_800A6808, &D_WSTAG320_800A681C, &D_WSTAG320_800A6830,
    &D_WSTAG320_800A6844, NULL,
};
FieldstgSprite wstag320_sprites[129] = {
    { 1, 0, 0x40, 2, 0x47, 1, 0x47, 0x4C, 4, 0, 395, 311, 0, 0 },
    { 1, 0, 0x40, 2, 0x47, 1, 0x47, 0x4C, 4, 0, 871, 533, 0, 0 },
    { 1, 0, 0x40, 2, 0x47, 1, 0x47, 0x4C, 4, 0, 1037, 703, 0, 0 },
    { 1, 0, 0x40, 2, 0x4D, 2, 0, 3, 4, 0, 1012, 671, 0, 0 }, { 1, 0, 0x40, 2, 0x52, 2, 0, 3, 8, 0, 442, 116, 0, 0 },
    { 1, 0, 0x40, 2, 0x52, 2, 0, 3, 8, 0, 473, 132, 0, 0 }, { 1, 0, 0x40, 2, 0x53, 2, 0, 3, 4, 0, 299, 39, 0, 0 },
    { 1, 0, 0x40, 2, 0x5A, 0, 0, 0, 0, 0, 472, 396, 0, 0 }, { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 417, 0, 0 },
    { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 449, 0, 0 }, { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 481, 0, 0 },
    { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 513, 0, 0 }, { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 544, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 1, 0x47, 0x4C, 4, 0, 735, 465, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 2, 0, 3, 6, 0, 328, 658, 0, 0 }, { 1, 0, 0x40, 6, 0x4E, 2, 0, 3, 6, 0, 616, 802, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 2, 0, 3, 6, 0, 760, 730, 0, 0 }, { 1, 0, 0x40, 6, 0x4E, 2, 0, 3, 6, 0, 904, 802, 0, 0 },
    { 1, 0, 0x40, 6, 0x4F, 2, 0, 3, 6, 0, 351, 643, 0, 0 }, { 1, 0, 0x40, 6, 0x4F, 2, 0, 3, 6, 0, 639, 787, 0, 0 },
    { 1, 0, 0x40, 6, 0x4F, 2, 0, 3, 6, 0, 783, 715, 0, 0 }, { 1, 0, 0x40, 6, 0x4F, 2, 0, 3, 6, 0, 927, 787, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 2, 0, 3, 6, 0, 324, 632, 0, 0 }, { 1, 0, 0x40, 6, 0x51, 2, 0, 3, 6, 0, 612, 776, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 2, 0, 3, 6, 0, 756, 704, 0, 0 }, { 1, 0, 0x40, 6, 0x51, 2, 0, 3, 6, 0, 900, 776, 0, 0 },
    { 1, 0, 0x40, 6, 0x50, 2, 0, 3, 6, 0, 305, 643, 0, 0 }, { 1, 0, 0x40, 6, 0x50, 2, 0, 3, 6, 0, 593, 788, 0, 0 },
    { 1, 0, 0x40, 6, 0x50, 2, 0, 3, 6, 0, 737, 715, 0, 0 }, { 1, 0, 0x40, 6, 0x50, 2, 0, 3, 6, 0, 881, 787, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 202, 416, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 210, 420, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 218, 424, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 226, 428, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 234, 432, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 242, 436, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 250, 440, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 258, 444, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 266, 352, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 274, 356, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 282, 360, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 290, 364, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 298, 368, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 306, 372, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 314, 376, 0, 0 }, { 1, 0, 0x40, 6, 0x55, 2, 0, 3, 8, 0, 322, 380, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 270, 444, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 278, 440, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 286, 436, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 294, 432, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 302, 428, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 310, 424, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 358, 400, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 366, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 374, 392, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 382, 388, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 390, 384, 0, 0 }, { 1, 0, 0x40, 6, 0x57, 2, 0, 3, 8, 0, 349, 405, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 3, 4, 0, 439, 395, 0, 0 }, { 1, 0, 0x40, 6, 0x59, 2, 0, 3, 4, 0, 540, 398, 0, 0 },
    { 1, 0, 0x40, 6, 0x5B, 2, 0, 3, 4, 0, 612, 416, 0, 0 }, { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 646, 530, 0, 0 },
    { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 688, 509, 0, 0 }, { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 790, 602, 0, 0 },
    { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 790, 746, 0, 0 }, { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 832, 581, 0, 0 },
    { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 832, 725, 0, 0 }, { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 358, 674, 0, 0 },
    { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 400, 653, 0, 0 }, { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 519, 754, 0, 0 },
    { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 535, 458, 0, 0 }, { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 560, 733, 0, 0 },
    { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 576, 437, 0, 0 }, { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 648, 724, 0, 0 },
    { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 690, 746, 0, 0 }, { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 792, 653, 0, 0 },
    { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 833, 674, 0, 0 }, { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 936, 725, 0, 0 },
    { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 978, 746, 0, 0 }, { 1, 0, 0x40, 6, 0x62, 2, 0, 1, 4, 0, 654, 640, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 102, 448, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 159, 484, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 174, 374, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 273, 382, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 390, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 128, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 177, 460, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 209, 367, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 94, 418, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 194, 530, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 98, 495, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 546, 532, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 366, 486, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 251, 381, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 70, 467, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 76, 433, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 129, 505, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 179, 534, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 186, 472, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 187, 526, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 263, 395, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 352, 473, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 393, 415, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 553, 516, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 956, 689, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 966, 668, 0, 0 },
    { 1, 0xA, 0xA0, 6, 0, 0, 0, 0, 0, 0, 792, 381, 0, 0 }, { 1, 0, 0x60, 6, 1, 0, 0, 0, 0, 0, 721, 691, 0, 0 },
    { 1, 0, 0x60, 6, 1, 0, 0, 0, 0, 0, 865, 763, 0, 0 }, { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 369, 0, 0 },
    { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 401, 0, 0 }, { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 433, 0, 0 },
    { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 465, 0, 0 }, { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 497, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x20, 1, 0x20, 0x22, 6, 0, 546, 665, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x20, 1, 0x20, 0x22, 6, 0, 833, 798, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2A, 1, 0x2A, 0x2D, 6, 0, 545, 558, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2A, 1, 0x2A, 0x2D, 6, 0, 834, 701, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x23, 1, 0x23, 0x25, 6, 0, -12, 814, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x23, 1, 0x23, 0x25, 6, 0, 0, 364, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x23, 1, 0x23, 0x25, 6, 0, 100, 758, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x23, 1, 0x23, 0x25, 6, 0, 212, 702, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x23, 1, 0x23, 0x25, 6, 0, 324, 646, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, -14, 713, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 99, 658, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 210, 602, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 322, 546, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x26, 1, 0x26, 0x29, 6, 0, 11, 408, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag320_map_events[8] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x20B, 0x242, 0x13E, 3, 0, 0, 0 },
    { 0x703F, 1, 0xFFFF, 0, 1, 0x21A, 0x68, 0x304, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x21C, 0x88, 0x1B4, 5, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E0, 0x240, 0xD8, 1, 0, 2, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x130, 0xF8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x120, 0x140, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E0, 0x240, 0xD8, 1, 0, 2, 1 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag320_funcs = { wstag320_setup };
FieldstgEventDef wstag320_events[2] = {
    { 290, D_WSTAG320_800A6094, 0x01270002, NULL, wstag320_event_290_end }, { -1, NULL, 0, NULL, NULL },
};
