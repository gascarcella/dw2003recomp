#include "wstag.h"

/* WSTAG937: stage 0x282 (fieldstg_stages_2d). */

extern FieldstgVramPlace wstag937_vram_places[];
extern FieldstgPlacedActor *wstag937_actors[];
extern FieldstgSprite wstag937_sprites[];
extern FieldstgMapEvent wstag937_map_events[];

extern FieldstgStageFuncs wstag937_funcs; /* the setup and a fade pair, as FIELDSTG's own */

void wstag937_update(WstagObject *obj) {
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

WstagObject *wstag937_start(void *arg0) {
    WstagObject *obj = object_new(wstag937_update, sizeof(WstagObject), sizeof(Object *));

    obj->manager = arg0;
    wstag937_funcs.setup();
    return obj;
}

void wstag937_setup(void) {
    fieldstg_stage.background_file = 0x1B2;
    fieldstg_stage.sprite_file = 0x09010000;
    fieldstg_stage.sprites = wstag937_sprites;
    fieldstg_stage.map_events = wstag937_map_events;
    fieldstg_stage.mask_file = 0x900;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1BB00, 0xF400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag937_vram_places;
    fieldstg_stage.music = 8;
    fieldstg_stage.sound = 0x60200000;
    fieldstg_stage.actors = wstag937_actors;
    fieldstg_attr.set_file(0, 0x09010001);
    fieldstg_attr.set_file(7, 0x09010002);
    fieldstg_attr.init_layer(0);
}

void wstag937_fade_start(WindowAnim *fade, s32 in) {
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

s32 wstag937_fade_update(WindowAnim *fade) {
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
void wstag937_setup(void);

FieldstgVramPlace wstag937_vram_places[13] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 378, 192, 122, 352, 511 }, { 320, 256, 320, 386, 0, 130, 368, 511 },
    { 320, 256, 328, 386, 32, 130, 352, 510 }, { 320, 256, 336, 386, 64, 130, 368, 510 },
    { 320, 256, 344, 386, 96, 130, 320, 509 }, { 320, 256, 352, 393, 128, 137, 336, 509 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG937_800A613C[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG937_800A6144[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG937_800A614C[4] = { 0, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG937_800A6154[2] = { { NULL, NULL, 101 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG937_800A616C[2] = { { NULL, NULL, 100 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG937_800A6184[2] = { { NULL, NULL, 102 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG937_800A619C[2] = { { NULL, NULL, 98 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG937_800A61B4[2] = { { NULL, NULL, 99 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG937_800A61CC[3] = {
    { D_WSTAG937_800A613C, D_WSTAG937_800A6144, 89 }, { D_WSTAG937_800A614C, NULL, 136 }, { NULL, NULL, 0 },
};
FieldstgPlacedActor D_WSTAG937_800A61F0 = { NULL, D_WSTAG937_800A6154, 48, 4, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG937_800A6204 = { NULL, D_WSTAG937_800A616C, 49, 5, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG937_800A6218 = { NULL, D_WSTAG937_800A6184, 50, 6, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG937_800A622C = { NULL, D_WSTAG937_800A619C, 52, 7, 368, 224, 1 };
FieldstgPlacedActor D_WSTAG937_800A6240 = { NULL, D_WSTAG937_800A61B4, 53, 8, 471, 245, 3 };
FieldstgPlacedActor D_WSTAG937_800A6254 = { NULL, D_WSTAG937_800A61CC, 267, 9, 353, 160, 7 };
FieldstgPlacedActor D_WSTAG937_800A6268 = { NULL, NULL, 268, 10, 360, 172, 7 };
FieldstgPlacedActor *wstag937_actors[8] = {
    &D_WSTAG937_800A61F0, &D_WSTAG937_800A6204, &D_WSTAG937_800A6218, &D_WSTAG937_800A622C, &D_WSTAG937_800A6240,
    &D_WSTAG937_800A6254, &D_WSTAG937_800A6268, NULL,
};
FieldstgSprite wstag937_sprites[36] = {
    { 1, 0, 0x40, 2, 0xA, 0, 0, 0, 0, 0, 93, 125, 0, 0 }, { 1, 0, 0x40, 2, 0xB, 0, 0, 0, 0, 0, 126, 109, 0, 0 },
    { 1, 0, 0x40, 2, 0xC, 0, 0, 0, 0, 0, 158, 93, 0, 0 }, { 1, 0, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 235, 67, 0, 0 },
    { 1, 0, 0x40, 2, 0xE, 0, 0, 0, 0, 0, 282, 43, 0, 0 }, { 1, 0, 0x40, 2, 0xF, 0, 0, 0, 0, 0, 379, 34, 0, 0 },
    { 1, 0, 0x40, 2, 0x10, 0, 0, 0, 0, 0, 403, 46, 0, 0 }, { 1, 0, 0x40, 2, 0x11, 0, 0, 0, 0, 0, 427, 90, 0, 0 },
    { 1, 0, 0x40, 2, 0x12, 0, 0, 0, 0, 0, 463, 113, 0, 0 }, { 1, 0, 0x40, 2, 0x13, 0, 0, 0, 0, 0, 482, 109, 0, 0 },
    { 1, 0, 0x40, 2, 0x14, 0, 0, 0, 0, 0, 503, 123, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 84, 130, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 116, 114, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 148, 98, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 272, 44, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 368, 36, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 417, 91, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 453, 113, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 473, 112, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 493, 134, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 497, 124, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 4, 0, 292, 104, 0, 0 },
    { 1, 0, 0x40, 6, 0x15, 0, 0, 0, 0, 0, 416, 59, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 224, 68, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 392, 48, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 416, 60, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 208, 263, 288, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 384, 191, 217, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 320, 169, 184, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 304, 161, 174, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 336, 161, 174, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 288, 153, 166, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 352, 153, 166, 0 }, { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 272, 144, 158, 0 },
    { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 368, 145, 158, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag937_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x271, 0x180, 0xD8, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x281, 0x92, 0x1A2, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0x110, 0xB8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0x100, 0xF0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgStageFuncs wstag937_funcs = { wstag937_setup, wstag937_fade_start, wstag937_fade_update };
