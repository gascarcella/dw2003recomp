#include "wstag.h"

/* WSTAG924: stage 0x275 (fieldstg_stages_2d). */

extern WstagFadeFuncs wstag924_funcs;
const CVECTOR wstag924_color = { 0x54, 0x67, 0x96, 0 };
extern FieldstgVramPlace wstag924_vram_places[];
extern FieldstgPlacedActor *wstag924_actors[];
extern FieldstgSprite wstag924_sprites[];
extern FieldstgMapEvent wstag924_map_events[];
extern FieldstgEventDef wstag924_events[];

void wstag924_update(WstagObject *obj) {
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

WstagObject *wstag924_start(void *arg0) {
    WstagObject *obj = object_new(wstag924_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag924_funcs.setup();
    return obj;
}

void wstag924_setup(void) {
    fieldstg_stage.background_file = 0x19C;
    fieldstg_stage.sprite_file = 0x08E70000;
    fieldstg_stage.sprites = wstag924_sprites;
    fieldstg_stage.map_events = wstag924_map_events;
    fieldstg_stage.mask_file = 0x8E5;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10200, 0x15700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag924_vram_places;
    fieldstg_stage.music = 0x33;
    fieldstg_stage.sound = 0x60CC0000;
    fieldstg_stage.actors = wstag924_actors;
    fieldstg_stage.color = wstag924_color;
    fieldstg_stage.events = wstag924_events;
    fieldstg_attr.set_file(0, 0x08E70001);
    fieldstg_attr.set_file(7, 0x08E70002);
    fieldstg_attr.init_layer(0);
}

void wstag924_fade_start(WindowAnim *fade, s32 in) {
    fade->running = 1;
    if (in) {
        sound_module.play(0x40019);
        fade->step = 0x1000 / fade->duration;
        fade->level = 0;
    } else {
        sound_module.play(0x4001A);
        fade->level = 0x1000;
        fade->step = -(0x1000 / fade->duration * 2);
    }
}

s32 wstag924_fade_update(WindowAnim *fade) {
    if (fade->running == 0) {
        return 1;
    }
    fade->level += fade->step;
    if (fade->step > 0) {
        if (fade->level > 0x1000) {
            fade->level = 0x1000;
            fade->running = 0;
            return 1;
        }
    } else if (fade->level < 0) {
        fade->level = 0;
        fade->running = 0;
        return 1;
    }
    return 0;
}

/* The stage's .data (tools/wstag_data.py). */
void wstag924_setup(void);
Object *wstag924_event_1602_start(void);
Object *wstag924_event_1604_start(void);

FieldstgVramPlace wstag924_vram_places[13] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 358, 376, 152, 120, 336, 504 }, { 320, 256, 366, 376, 184, 120, 352, 504 },
    { 320, 256, 374, 376, 216, 120, 368, 504 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 320, 256, 338, 358, 72, 102, 336, 503 }, { 320, 256, 320, 382, 0, 126, 352, 503 },
};
u16 D_WSTAG924_800A74FC[4] = { 0x906E, 1, 0xFFFF, 0 };
u16 D_WSTAG924_800A7504[4] = { 0x906D, 1, 0xFFFF, 0 };
u16 D_WSTAG924_800A750C[4] = { 0x7C00, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG924_800A7514[2] = { { NULL, D_WSTAG924_800A74FC, 35 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG924_800A752C[2] = { { NULL, D_WSTAG924_800A7504, 34 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG924_800A7544[2] = { { NULL, D_WSTAG924_800A750C, 36 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG924_800A755C[2] = { { NULL, NULL, 37 }, { NULL, NULL, 0 } };
FieldstgPlacedActor D_WSTAG924_800A7574 = { NULL, D_WSTAG924_800A7514, 32, 4, 220, 232, 1 };
FieldstgPlacedActor D_WSTAG924_800A7588 = { NULL, D_WSTAG924_800A752C, 36, 5, 187, 215, 1 };
FieldstgPlacedActor D_WSTAG924_800A759C = { NULL, NULL, 89, 6, 300, 217, 5 };
FieldstgPlacedActor D_WSTAG924_800A75B0 = { NULL, NULL, 112, 7, 202, 241, 1 };
FieldstgPlacedActor D_WSTAG924_800A75C4 = { NULL, NULL, 113, 8, 169, 224, 1 };
FieldstgPlacedActor D_WSTAG924_800A75D8 = { NULL, D_WSTAG924_800A7544, 254, 9, 399, 248, 1 };
FieldstgPlacedActor D_WSTAG924_800A75EC = { NULL, D_WSTAG924_800A755C, 375, 10, 65, 263, 1 };
FieldstgPlacedActor *wstag924_actors[8] = {
    &D_WSTAG924_800A7574, &D_WSTAG924_800A7588, &D_WSTAG924_800A759C, &D_WSTAG924_800A75B0, &D_WSTAG924_800A75C4,
    &D_WSTAG924_800A75D8, &D_WSTAG924_800A75EC, NULL,
};
FieldstgSprite wstag924_sprites[50] = {
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
    { 1, 0, 0x40, 6, 0x2B, 0, 0, 0, 0, 0, 213, 151, 0, 0 }, { 1, 0, 0x40, 6, 0x2B, 0, 0, 0, 0, 0, 248, 286, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 0, 0, 0, 0, 0, 104, 328, 0, 0 }, { 1, 0, 0x40, 6, 0x2C, 0, 0, 0, 0, 0, 197, 141, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 0, 0, 0, 0, 0, 264, 158, 0, 0 }, { 1, 0, 0x40, 6, 0x2D, 0, 0, 0, 0, 0, 236, 159, 0, 0 },
    { 1, 0, 0x40, 6, 0x2D, 0, 0, 0, 0, 0, 391, 343, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x39, 0xE, 0, 318, 172, 0, 0 },
    { 1, 0, 0x40, 6, 0x2F, 1, 0x2F, 0x31, 4, 0, 305, 193, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3F, 8, 0, 354, 209, 0, 0 },
    { 1, 0, 0x40, 6, 0x40, 2, 0, 9, 8, 0, 336, 217, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 2, 0, 2, 0x10, 0, 328, 232, 0, 0 },
    { 1, 0x64, 0x40, 6, 2, 0, 0, 0, 0, 0, 79, 186, 0, 0 }, { 1, 0, 0x40, 6, 0x42, 2, 0, 2, 6, 0, 136, 185, 0, 0 },
    { 1, 0, 0x40, 6, 0x43, 2, 0, 2, 0x10, 0, 306, 231, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 2, 0, 2, 0xA, 0, 321, 223, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 2, 0, 1, 4, 0, 140, 123, 0, 0 }, { 1, 0, 0x40, 6, 0x48, 2, 0, 1, 4, 0, 307, 122, 0, 0 },
    { 1, 0, 0x40, 6, 0x49, 2, 0, 1, 4, 0, 346, 144, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 273, 297, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x56, 1, 0x56, 0x58, 0xA, 0, 215, 300, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x56, 1, 0x56, 0x58, 0xA, 0, 368, 358, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x59, 1, 0x59, 0x5B, 0xA, 0, 92, 347, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x59, 1, 0x59, 0x5B, 0xA, 0, 135, 312, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x59, 1, 0x59, 0x5B, 0xA, 0, 150, 375, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x59, 1, 0x59, 0x5B, 0xA, 0, 418, 326, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 143, 287, 340, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 326, 288, 340, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 398, 240, 263, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 33, 201, 239, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag924_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x273, 0x2C8, 0x24C, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x270, 0x410, 0x200, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFadeFuncs wstag924_funcs = { wstag924_setup, wstag924_fade_start, wstag924_fade_update };
FieldstgEventDef wstag924_events[3] = {
    { 1602, NULL, 0x01580001, (s32 (*)(void))wstag924_event_1602_start, NULL },
    { 1604, NULL, 0x01580002, (s32 (*)(void))wstag924_event_1604_start, NULL }, { -1, NULL, 0, NULL, NULL },
};
