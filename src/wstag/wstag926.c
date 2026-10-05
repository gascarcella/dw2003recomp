#include "wstag.h"

/* WSTAG926: stage 0x277 (fieldstg_stages_2d). */

extern WstagFuncs wstag926_funcs;
extern FieldstgVramPlace wstag926_vram_places[];
extern FieldstgPlacedActor *wstag926_actors[];
extern FieldstgSprite wstag926_sprites[];
extern FieldstgMapEvent wstag926_map_events[];
extern FieldstgEventDef wstag926_events[];
/* An object that lowers the sprite of type 1 and the player by one pixel a frame, 150 times. */
typedef struct Wstag926Lowering {
    /* 0x00 */ Object base;
    /* 0x50 */ FieldstgSprite *sprite;
    /* 0x54 */ s32 frames; /* frames */
} Wstag926Lowering; /* size 0x58 */

void wstag926_lowering_update(Wstag926Lowering *obj);

void wstag926_update(WstagObject *obj) {
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

WstagObject *wstag926_start(void *arg0) {
    WstagObject *obj = object_new(wstag926_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag926_funcs.setup();
    return obj;
}

void wstag926_setup(void) {
    fieldstg_stage.background_file = 0x1A0;
    fieldstg_stage.sprite_file = 0x08EB0000;
    fieldstg_stage.sprites = wstag926_sprites;
    fieldstg_stage.map_events = wstag926_map_events;
    fieldstg_stage.mask_file = 0x8EA;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0xDE00, 0xDE00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag926_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.actors = wstag926_actors;
    fieldstg_stage.events = wstag926_events;
    fieldstg_attr.set_file(0, 0x08EB0001);
    fieldstg_attr.set_file(7, 0x08EB0002);
    fieldstg_attr.init_layer(0);
}

void wstag926_lowering_update(Wstag926Lowering *obj) {
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    FieldstgActor *player;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                obj->sprite = sprite;
                sprite->y--;
                sprite->shown = 1;
            }
        }
        obj->frames = 0;
        break;
    case OBJECT_STATE_RUN:
        spr = obj->sprite;
        player = (FieldstgActor *)heap_objects.find(5, -1, 0);
        spr->y++;
        obj->frames++;
        player->pos.y += 0x100;
        if (obj->frames >= 150) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag926_lowering_create(s32 arg0) {
    return object_create(wstag926_lowering_update, sizeof(Wstag926Lowering), 0, arg0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag926_setup(void);
extern s16 D_WSTAG926_800A6424[];

FieldstgVramPlace wstag926_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 372, 328, 208, 72, 368, 511 }, { 320, 256, 348, 353, 112, 97, 352, 510 },
    { 320, 256, 364, 353, 176, 97, 368, 510 },
};
u16 D_WSTAG926_800A615C[4] = { 0x7054, 0, 0xFFFF, 0 };
u16 D_WSTAG926_800A6164[6] = { 0x7054, 1, 0x100C, 0, 0xFFFF, 0 };
u16 D_WSTAG926_800A6170[4] = { 0x906F, 1, 0xFFFF, 0 };
u16 D_WSTAG926_800A6178[6] = { 0x7054, 1, 0x100C, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG926_800A6184[4] = {
    { D_WSTAG926_800A615C, NULL, 39 }, { D_WSTAG926_800A6164, D_WSTAG926_800A6170, 40 },
    { D_WSTAG926_800A6178, NULL, 41 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG926_800A61B4[2] = { { NULL, NULL, 42 }, { NULL, NULL, 0 } };
FieldstgPlacedActor D_WSTAG926_800A61CC = { NULL, D_WSTAG926_800A6184, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG926_800A61E0 = { NULL, NULL, 36, 5, 240, 329, 3 };
FieldstgPlacedActor D_WSTAG926_800A61F4 = { NULL, D_WSTAG926_800A61B4, 373, 6, 203, 218, 3 };
FieldstgPlacedActor *wstag926_actors[4] = {
    &D_WSTAG926_800A61CC, &D_WSTAG926_800A61E0, &D_WSTAG926_800A61F4, NULL,
};
FieldstgSprite wstag926_sprites[24] = {
    { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 192, 256, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 144, 104, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 168, 92, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 224, 64, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 127, 112, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 127, 144, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 159, 96, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 159, 128, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 191, 80, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 191, 112, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 223, 64, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 223, 96, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 0xB, 8, 0, 196, 289, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 0xB, 8, 0, 226, 274, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 0xB, 8, 0, 212, 266, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 0xB, 8, 0, 212, 281, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 0xB, 8, 0, 196, 275, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 0xB, 8, 0, 226, 260, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 1, 0x37, 0x39, 0xA, 0, 200, 290, 0, 0 },
    { 1, 1, 0x40, 6, 2, 0, 0, 0, 0, 0, 256, 336, 0, 0 },
    { 1, 0, 0x40, 4, 0x3A, 1, 0x3A, 0x3C, 0xA, 0, 256, 286, 315, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 304, 312, 329, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 256, 280, 315, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag926_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x273, 0x200, 0x11C, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag926_funcs = { wstag926_setup };
FieldstgEventDef wstag926_events[2] = {
    { 1606, D_WSTAG926_800A6424, 0x01580003, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
s16 D_WSTAG926_800A6424[99] = {
    FIELDSTG_EVENT_WALK(2, 303, 360, 3),
    FIELDSTG_EVENT_ANIM(32, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 32, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 32, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x356, 841, 2),
    FIELDSTG_EVENT_WAIT(48),
    FIELDSTG_EVENT_GOTO_MAP(0x278, 192, 344, 5),
    FIELDSTG_EVENT_END,
};
