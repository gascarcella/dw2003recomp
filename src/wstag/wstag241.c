#include "wstag.h"

/* WSTAG241: stage 0x27B (fieldstg_stages). */

extern WstagFuncs wstag241_funcs;
extern FieldstgVramPlace wstag241_vram_places[];
extern FieldstgPlacedActor *wstag241_actors[];
extern FieldstgSprite wstag241_sprites[];
extern FieldstgMapEvent wstag241_map_events[];

void wstag241_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag241_start(void *arg0) {
    WstagObject *obj = object_new(wstag241_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag241_funcs.setup();
    return obj;
}

void wstag241_setup(void) {
    fieldstg_stage.background_file = 0x4E9;
    fieldstg_stage.sprite_file = 0x04EA0000;
    fieldstg_stage.sprites = wstag241_sprites;
    fieldstg_stage.map_events = wstag241_map_events;
    fieldstg_stage.mask_file = 0x4E8;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0xB700, 0x10900 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag241_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag241_actors;
    fieldstg_attr.set_file(0, 0x04EA0001);
    fieldstg_attr.set_file(7, 0x04EA0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag241_setup(void);

FieldstgVramPlace wstag241_vram_places[14] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 256, 0, 0, 352, 511 }, { 320, 256, 330, 256, 40, 0, 368, 511 },
    { 320, 256, 340, 256, 80, 0, 352, 510 }, { 320, 256, 348, 256, 112, 0, 368, 510 },
    { 320, 256, 356, 256, 144, 0, 320, 509 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgTalk D_WSTAG241_800A604C[2] = { { NULL, NULL, 101 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG241_800A6064[2] = { { NULL, NULL, 102 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG241_800A607C[2] = { { NULL, NULL, 103 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG241_800A6094[2] = { { NULL, NULL, 99 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG241_800A60AC[2] = { { NULL, NULL, 98 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG241_800A60C4[2] = { { NULL, NULL, 100 }, { NULL, NULL, 0 } };
u16 D_WSTAG241_800A60DC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG241_800A60E4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG241_800A60EC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG241_800A60F4[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG241_800A6100[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG241_800A610C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG241_800A6114[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG241_800A611C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG241_800A6124[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG241_800A612C = { D_WSTAG241_800A60DC, D_WSTAG241_800A604C, 40, 4, 135, 212, 7 };
FieldstgPlacedActor D_WSTAG241_800A6140 = { D_WSTAG241_800A60E4, D_WSTAG241_800A6064, 41, 5, 173, 155, 7 };
FieldstgPlacedActor D_WSTAG241_800A6154 = { D_WSTAG241_800A60EC, D_WSTAG241_800A607C, 42, 6, 56, 193, 1 };
FieldstgPlacedActor D_WSTAG241_800A6168 = { D_WSTAG241_800A60F4, D_WSTAG241_800A6094, 57, 7, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG241_800A617C = { D_WSTAG241_800A6100, D_WSTAG241_800A60AC, 157, 8, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG241_800A6190 = { D_WSTAG241_800A610C, D_WSTAG241_800A60C4, 157, 8, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG241_800A61A4 = { D_WSTAG241_800A6114, NULL, 221, 9, 184, 173, 7 };
FieldstgPlacedActor D_WSTAG241_800A61B8 = { D_WSTAG241_800A611C, NULL, 222, 10, 143, 240, 7 };
FieldstgPlacedActor D_WSTAG241_800A61CC = { D_WSTAG241_800A6124, NULL, 223, 11, 64, 224, 7 };
FieldstgPlacedActor *wstag241_actors[10] = {
    &D_WSTAG241_800A612C, &D_WSTAG241_800A6140, &D_WSTAG241_800A6154, &D_WSTAG241_800A6168, &D_WSTAG241_800A617C,
    &D_WSTAG241_800A6190, &D_WSTAG241_800A61A4, &D_WSTAG241_800A61B8, &D_WSTAG241_800A61CC, NULL,
};
FieldstgSprite wstag241_sprites[7] = {
    { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 62, 115, 0, 0 }, { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 124, 173, 0, 0 },
    { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 218, 227, 0, 0 }, { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 242, 146, 0, 0 },
    { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 295, 210, 0, 0 }, { 1, 0, 0x40, 2, 1, 2, 0, 1, 4, 0, 227, 130, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag241_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x279, 0xD8, 0x54, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0x100, 0xF6, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0xF0, 0xBF, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x120, 0xF6, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x12F, 0xAE, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag241_funcs = { wstag241_setup };
