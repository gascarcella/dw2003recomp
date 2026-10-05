#include "wstag.h"

/* WSTAG240: stage 0x20C (fieldstg_stages). */

extern WstagFuncs wstag240_funcs;
extern FieldstgVramPlace wstag240_vram_places[];
extern FieldstgPlacedActor *wstag240_actors[];
extern FieldstgSprite wstag240_sprites[];
extern FieldstgMapEvent wstag240_map_events[];

void wstag240_update(WstagObject *obj) {
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

WstagObject *wstag240_start(void *arg0) {
    WstagObject *obj = object_new(wstag240_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag240_funcs.setup();
    return obj;
}

void wstag240_setup(void) {
    fieldstg_stage.background_file = 0x1A4;
    fieldstg_stage.sprite_file = 0x01A50000;
    fieldstg_stage.sprites = wstag240_sprites;
    fieldstg_stage.map_events = wstag240_map_events;
    fieldstg_stage.mask_file = 0x2C2;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0xB300, 0xFF00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag240_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag240_actors;
    fieldstg_attr.set_file(0, 0x01A50001);
    fieldstg_attr.set_file(7, 0x01A50002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag240_setup(void);

FieldstgVramPlace wstag240_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 256, 0, 0, 352, 511 }, { 320, 256, 330, 256, 40, 0, 368, 511 },
    { 320, 256, 340, 256, 80, 0, 352, 510 }, { 320, 256, 364, 256, 176, 0, 368, 510 },
    { 320, 256, 348, 256, 112, 0, 320, 509 }, { 320, 256, 356, 256, 144, 0, 336, 509 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG240_800A6058[4] = { 0x1C44, 0, 0xFFFF, 0 };
u16 D_WSTAG240_800A6060[4] = { 0x1C44, 1, 0xFFFF, 0 };
u16 D_WSTAG240_800A6068[4] = { 0x1C44, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG240_800A6070[2] = { { NULL, NULL, 32 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG240_800A6088[2] = { { NULL, NULL, 33 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG240_800A60A0[2] = { { NULL, NULL, 34 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG240_800A60B8[2] = { { NULL, NULL, 1071 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG240_800A60D0[2] = { { NULL, NULL, 1073 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG240_800A60E8[2] = { { NULL, NULL, 1075 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG240_800A6100[2] = { { NULL, NULL, 1068 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG240_800A6118[2] = { { NULL, NULL, 1069 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG240_800A6130[2] = { { NULL, NULL, 1070 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG240_800A6148[2] = { { NULL, NULL, 1072 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG240_800A6160[2] = { { NULL, NULL, 1074 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG240_800A6178[2] = { { NULL, NULL, 35 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG240_800A6190[2] = { { NULL, NULL, 1077 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG240_800A61A8[3] = {
    { D_WSTAG240_800A6058, D_WSTAG240_800A6060, 185 }, { D_WSTAG240_800A6068, NULL, 198 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG240_800A61CC[2] = { { NULL, NULL, 1076 }, { NULL, NULL, 0 } };
u16 D_WSTAG240_800A61E4[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG240_800A61EC[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG240_800A61F4[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG240_800A61FC[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG240_800A6204[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG240_800A620C[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG240_800A6214[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG240_800A621C[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG240_800A6224[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG240_800A622C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG240_800A6234[6] = { 0x818F, 0, 0x6006, 1, 0xFFFF, 0 };
u16 D_WSTAG240_800A6240[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG240_800A6248 = { NULL, D_WSTAG240_800A6070, 40, 4, 135, 212, 7 };
FieldstgPlacedActor D_WSTAG240_800A625C = { NULL, D_WSTAG240_800A6088, 41, 5, 173, 155, 7 };
FieldstgPlacedActor D_WSTAG240_800A6270 = { NULL, D_WSTAG240_800A60A0, 42, 6, 56, 193, 1 };
FieldstgPlacedActor D_WSTAG240_800A6284 = { D_WSTAG240_800A61E4, D_WSTAG240_800A60B8, 45, 7, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG240_800A6298 = { D_WSTAG240_800A61EC, D_WSTAG240_800A60D0, 45, 7, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG240_800A62AC = { D_WSTAG240_800A61F4, D_WSTAG240_800A60E8, 45, 7, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG240_800A62C0 = { D_WSTAG240_800A61FC, D_WSTAG240_800A6100, 45, 7, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG240_800A62D4 = { D_WSTAG240_800A6204, D_WSTAG240_800A6118, 45, 7, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG240_800A62E8 = { D_WSTAG240_800A620C, D_WSTAG240_800A6130, 45, 7, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG240_800A62FC = { D_WSTAG240_800A6214, D_WSTAG240_800A6148, 45, 7, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG240_800A6310 = { D_WSTAG240_800A621C, D_WSTAG240_800A6160, 45, 7, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG240_800A6324 = { D_WSTAG240_800A6224, D_WSTAG240_800A6178, 45, 7, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG240_800A6338 = { D_WSTAG240_800A622C, D_WSTAG240_800A6190, 45, 7, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG240_800A634C = { D_WSTAG240_800A6234, D_WSTAG240_800A61A8, 64, 8, 344, 172, 5 };
FieldstgPlacedActor D_WSTAG240_800A6360 = { D_WSTAG240_800A6240, D_WSTAG240_800A61CC, 157, 9, 224, 169, 7 };
FieldstgPlacedActor D_WSTAG240_800A6374 = { NULL, NULL, 221, 10, 184, 173, 7 };
FieldstgPlacedActor D_WSTAG240_800A6388 = { NULL, NULL, 222, 11, 143, 240, 7 };
FieldstgPlacedActor D_WSTAG240_800A639C = { NULL, NULL, 223, 12, 64, 224, 7 };
FieldstgPlacedActor *wstag240_actors[19] = {
    &D_WSTAG240_800A6248, &D_WSTAG240_800A625C, &D_WSTAG240_800A6270, &D_WSTAG240_800A6284, &D_WSTAG240_800A6298,
    &D_WSTAG240_800A62AC, &D_WSTAG240_800A62C0, &D_WSTAG240_800A62D4, &D_WSTAG240_800A62E8, &D_WSTAG240_800A62FC,
    &D_WSTAG240_800A6310, &D_WSTAG240_800A6324, &D_WSTAG240_800A6338, &D_WSTAG240_800A634C, &D_WSTAG240_800A6360,
    &D_WSTAG240_800A6374, &D_WSTAG240_800A6388, &D_WSTAG240_800A639C, NULL,
};
FieldstgSprite wstag240_sprites[7] = {
    { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 62, 115, 0, 0 }, { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 124, 173, 0, 0 },
    { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 218, 227, 0, 0 }, { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 242, 146, 0, 0 },
    { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 295, 210, 0, 0 }, { 1, 0, 0x40, 2, 1, 2, 0, 1, 4, 0, 227, 130, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag240_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x20A, 0xD8, 0x54, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0x100, 0xF6, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0xF0, 0xBF, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x120, 0xF6, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x12F, 0xAE, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag240_funcs = { wstag240_setup };
