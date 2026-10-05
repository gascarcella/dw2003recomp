#include "wstag.h"

/* WSTAG221: stage 0x275 (fieldstg_stages). */

extern WstagFuncs wstag221_funcs;
extern CVECTOR wstag221_color;
extern FieldstgVramPlace wstag221_vram_places[];
extern FieldstgPlacedActor *wstag221_actors[];
extern FieldstgSprite wstag221_sprites[];
extern FieldstgMapEvent wstag221_map_events[];

void wstag221_update(WstagObject *obj) {
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

WstagObject *wstag221_start(void *arg0) {
    WstagObject *obj = object_new(wstag221_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag221_funcs.setup();
    return obj;
}

void wstag221_setup(void) {
    fieldstg_stage.background_file = 0x4DF;
    fieldstg_stage.sprite_file = 0x04E00000;
    fieldstg_stage.sprites = wstag221_sprites;
    fieldstg_stage.map_events = wstag221_map_events;
    fieldstg_stage.mask_file = 0x4DE;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10500, 0x15F00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag221_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.actors = wstag221_actors;
    fieldstg_stage.color = wstag221_color;
    fieldstg_attr.set_file(0, 0x04E00001);
    fieldstg_attr.set_file(7, 0x04E00002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

INCLUDE_RODATA("asm/wstag221/nonmatchings/wstag221", wstag221_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag221_setup(void);

FieldstgVramPlace wstag221_vram_places[19] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 344, 384, 96, 128, 336, 503 }, { 320, 256, 352, 400, 128, 144, 352, 503 },
    { 320, 256, 360, 400, 160, 144, 368, 503 }, { 320, 256, 368, 400, 192, 144, 336, 502 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 320, 256, 340, 424, 80, 168, 352, 502 },
    { 320, 256, 320, 430, 0, 174, 368, 502 }, { 320, 256, 328, 430, 32, 174, 320, 501 },
    { 320, 256, 348, 440, 112, 184, 336, 501 }, { 320, 256, 376, 400, 224, 144, 352, 501 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG221_800A6108[4] = { 0x7C00, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A6110[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG221_800A611C[4] = { 0x7C00, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A6124[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A6130[4] = { 0x7C00, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG221_800A6138[2] = { { NULL, NULL, 81 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG221_800A6150[2] = { { NULL, NULL, 86 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG221_800A6168[2] = { { NULL, NULL, 82 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG221_800A6180[2] = { { NULL, NULL, 87 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG221_800A6198[2] = { { NULL, NULL, 89 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG221_800A61B0[2] = { { NULL, NULL, 77 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG221_800A61C8[2] = { { NULL, NULL, 83 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG221_800A61E0[2] = { { NULL, NULL, 78 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG221_800A61F8[2] = { { NULL, NULL, 84 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG221_800A6210[2] = { { NULL, D_WSTAG221_800A6108, 85 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG221_800A6228[3] = {
    { D_WSTAG221_800A6110, D_WSTAG221_800A611C, 79 }, { D_WSTAG221_800A6124, D_WSTAG221_800A6130, 80 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG221_800A624C[2] = { { NULL, NULL, 88 }, { NULL, NULL, 0 } };
u16 D_WSTAG221_800A6264[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A6270[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A6278[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A6284[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A628C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A6294[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A629C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A62A8[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A62B4[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A62C0[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG221_800A62CC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A62D4[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG221_800A62E0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A62E8[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG221_800A62F4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A62FC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A6304[4] = { 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A630C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG221_800A6314[6] = { 0x7094, 1, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG221_800A6320[6] = { 0x6026, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG221_800A632C[6] = { 0x7094, 1, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG221_800A6338[6] = { 0x6026, 1, 0x1A0A, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG221_800A6344 = { D_WSTAG221_800A6264, D_WSTAG221_800A6138, 32, 4, 187, 215, 1 };
FieldstgPlacedActor D_WSTAG221_800A6358 = { D_WSTAG221_800A6270, D_WSTAG221_800A6150, 32, 4, 187, 215, 1 };
FieldstgPlacedActor D_WSTAG221_800A636C = { D_WSTAG221_800A6278, D_WSTAG221_800A6168, 36, 5, 220, 232, 1 };
FieldstgPlacedActor D_WSTAG221_800A6380 = { D_WSTAG221_800A6284, D_WSTAG221_800A6180, 36, 5, 220, 232, 1 };
FieldstgPlacedActor D_WSTAG221_800A6394 = { D_WSTAG221_800A628C, D_WSTAG221_800A6198, 49, 6, 256, 385, 3 };
FieldstgPlacedActor D_WSTAG221_800A63A8 = { D_WSTAG221_800A6294, NULL, 89, 7, 300, 217, 5 };
FieldstgPlacedActor D_WSTAG221_800A63BC = { D_WSTAG221_800A629C, NULL, 89, 7, 300, 217, 5 };
FieldstgPlacedActor D_WSTAG221_800A63D0 = { D_WSTAG221_800A62A8, NULL, 112, 8, 169, 224, 1 };
FieldstgPlacedActor D_WSTAG221_800A63E4 = { D_WSTAG221_800A62B4, NULL, 113, 9, 202, 241, 1 };
FieldstgPlacedActor D_WSTAG221_800A63F8 = { D_WSTAG221_800A62C0, D_WSTAG221_800A61B0, 157, 10, 187, 215, 1 };
FieldstgPlacedActor D_WSTAG221_800A640C = { D_WSTAG221_800A62CC, D_WSTAG221_800A61C8, 157, 10, 187, 215, 1 };
FieldstgPlacedActor D_WSTAG221_800A6420 = { D_WSTAG221_800A62D4, D_WSTAG221_800A61E0, 158, 11, 220, 232, 1 };
FieldstgPlacedActor D_WSTAG221_800A6434 = { D_WSTAG221_800A62E0, D_WSTAG221_800A61F8, 158, 11, 220, 232, 1 };
FieldstgPlacedActor D_WSTAG221_800A6448 = { D_WSTAG221_800A62E8, NULL, 159, 12, 300, 217, 5 };
FieldstgPlacedActor D_WSTAG221_800A645C = { D_WSTAG221_800A62F4, NULL, 159, 12, 300, 217, 5 };
FieldstgPlacedActor D_WSTAG221_800A6470 = { D_WSTAG221_800A62FC, D_WSTAG221_800A6210, 160, 13, 399, 248, 1 };
FieldstgPlacedActor D_WSTAG221_800A6484 = { D_WSTAG221_800A6304, D_WSTAG221_800A6228, 254, 14, 399, 248, 1 };
FieldstgPlacedActor D_WSTAG221_800A6498 = { D_WSTAG221_800A630C, D_WSTAG221_800A624C, 254, 14, 399, 248, 1 };
FieldstgPlacedActor D_WSTAG221_800A64AC = { D_WSTAG221_800A6314, NULL, 270, 15, 169, 224, 1 };
FieldstgPlacedActor D_WSTAG221_800A64C0 = { D_WSTAG221_800A6320, NULL, 270, 15, 169, 224, 1 };
FieldstgPlacedActor D_WSTAG221_800A64D4 = { D_WSTAG221_800A632C, NULL, 271, 16, 202, 241, 1 };
FieldstgPlacedActor D_WSTAG221_800A64E8 = { D_WSTAG221_800A6338, NULL, 271, 16, 202, 241, 1 };
FieldstgPlacedActor *wstag221_actors[23] = {
    &D_WSTAG221_800A6344, &D_WSTAG221_800A6358, &D_WSTAG221_800A636C, &D_WSTAG221_800A6380, &D_WSTAG221_800A6394,
    &D_WSTAG221_800A63A8, &D_WSTAG221_800A63BC, &D_WSTAG221_800A63D0, &D_WSTAG221_800A63E4, &D_WSTAG221_800A63F8,
    &D_WSTAG221_800A640C, &D_WSTAG221_800A6420, &D_WSTAG221_800A6434, &D_WSTAG221_800A6448, &D_WSTAG221_800A645C,
    &D_WSTAG221_800A6470, &D_WSTAG221_800A6484, &D_WSTAG221_800A6498, &D_WSTAG221_800A64AC, &D_WSTAG221_800A64C0,
    &D_WSTAG221_800A64D4, &D_WSTAG221_800A64E8, NULL,
};
FieldstgSprite wstag221_sprites[38] = {
    { 1, 0, 0x40, 2, 0x45, 2, 0, 2, 0xA, 0, 376, 237, 0, 0 }, { 1, 0, 0x40, 2, 5, 0, 0, 0, 0, 0, 368, 277, 0, 0 },
    { 1, 0, 0x40, 2, 6, 0, 0, 0, 0, 0, 0, 204, 0, 0 }, { 1, 0, 0x40, 2, 0x46, 2, 0, 1, 4, 0, 38, 197, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 110, 341, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 198, 149, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 221, 148, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 235, 149, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 249, 297, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 271, 168, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 398, 350, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 102, 333, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 200, 134, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 213, 145, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 242, 167, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 248, 230, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 254, 280, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 265, 153, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 391, 343, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 1, 0x4A, 0x59, 0xA, 0, 124, 331, 0, 0 },
    { 1, 0, 0x40, 6, 0x14, 1, 0x14, 0x1E, 0xA, 0, 349, 336, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x39, 0xE, 0, 318, 172, 0, 0 },
    { 1, 0, 0x40, 6, 0x2F, 1, 0x2F, 0x31, 4, 0, 305, 193, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3F, 8, 0, 354, 209, 0, 0 },
    { 1, 0, 0x40, 6, 0x40, 2, 0, 9, 8, 0, 336, 217, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 2, 0, 2, 0x10, 0, 328, 232, 0, 0 },
    { 1, 0x64, 0x40, 6, 4, 0, 0, 0, 0, 0, 79, 186, 0, 0 }, { 1, 0, 0x40, 6, 0x42, 2, 0, 2, 6, 0, 136, 185, 0, 0 },
    { 1, 0, 0x40, 6, 0x43, 2, 0, 2, 0x10, 0, 306, 231, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 2, 0, 2, 0xA, 0, 321, 223, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 2, 0, 1, 4, 0, 140, 123, 0, 0 }, { 1, 0, 0x40, 6, 0x48, 2, 0, 1, 4, 0, 307, 122, 0, 0 },
    { 1, 0, 0x40, 6, 0x49, 2, 0, 1, 4, 0, 346, 144, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 143, 287, 340, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 326, 288, 340, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 398, 240, 263, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 33, 201, 239, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag221_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x273, 0x2C8, 0x24C, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x270, 0x410, 0x200, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag221_funcs = { wstag221_setup };
