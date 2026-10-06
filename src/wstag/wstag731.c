#include "wstag.h"

/* WSTAG731: stage 0x2D1 (fieldstg_stages). */

extern WstagAnimKey D_WSTAG731_800A61E4[];
extern WstagFuncs wstag731_funcs;
extern FieldstgVramPlace wstag731_vram_places[];
extern FieldstgPlacedActor *wstag731_actors[];
extern FieldstgSprite wstag731_sprites[];
extern FieldstgMapEvent wstag731_map_events[];
extern FieldstgEventDef wstag731_events[];
void wstag731_anim1_update();

s32 wstag731_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
    WstagAnimKey *key = &keys[anim->key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        anim->time -= step;
    }
    if (anim->time <= 0) {
        key++;
        anim->key++;
        anim->time += key->time;
        if (key->frame == 0xFF) {
            key = keys;
            anim->key = 0;
            anim->time += key->time;
        }
        wstag731_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag731_anim1_update(WstagAnim1Object *obj) {
    FieldstgSprite *sprite;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anim.key = 0;
        obj->anim.time = D_WSTAG731_800A61E4[0].time;
        break;
    case OBJECT_STATE_RUN:
        sprite = fieldstg_stage.sprites;
        frame = wstag731_anim_loop(&obj->anim, D_WSTAG731_800A61E4, 0);
        for (; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                sprite->sprite = frame;
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag731_anim1_create(void) {
    return object_new(wstag731_anim1_update, sizeof(WstagAnim1Object), 0);
}

void wstag731_update(WstagObject *obj, Object **data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        *data = wstag731_anim1_create();
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag731_start(void *arg0) {
    WstagObject *obj = object_new(wstag731_update, sizeof(WstagObject), sizeof(Object *));

    obj->manager = arg0;
    wstag731_funcs.setup();
    return obj;
}

void wstag731_setup(void) {
    fieldstg_stage.background_file = 0x6C6;
    fieldstg_stage.sprite_file = 0x06C70000;
    fieldstg_stage.sprites = wstag731_sprites;
    fieldstg_stage.map_events = wstag731_map_events;
    fieldstg_stage.mask_file = 0x6C5;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0xCE00, 0xDD00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag731_vram_places;
    fieldstg_stage.music = 8;
    fieldstg_stage.sound = 0x60200000;
    fieldstg_stage.actors = wstag731_actors;
    fieldstg_stage.events = wstag731_events;
    fieldstg_attr.set_file(0, 0x06C70001);
    fieldstg_attr.set_file(7, 0x06C70002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag731_setup(void);

s16 D_WSTAG731_800A6170[57] = {
    FIELDSTG_EVENT_WALK(2, 429, 206, 5),
    FIELDSTG_EVENT_PLACE(21, 461, 190),
    FIELDSTG_EVENT_ANIM(21, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 21, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(21, 54, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 885, 2),
    FIELDSTG_EVENT_WAIT_ANIM(21),
    FIELDSTG_EVENT_ANIM(21, 55, 3),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0xC10, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG731_800A61E4[7] = {
    { 53, 8 }, { 54, 8 }, { 55, 8 }, { 56, 4 }, { 57, 40 }, { 58, 8 }, { 255, 0 },
};
FieldstgVramPlace wstag731_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 370, 316, 200, 60, 368, 508 }, { 320, 256, 352, 468, 128, 212, 336, 507 },
};
u16 D_WSTAG731_800A6280[4] = { 0x9016, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A6288[4] = { 0x869C, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A6290[6] = { 0x869C, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A629C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A62A4[8] = { 0x869C, 0, 0, 1, 0x8498, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A62B4[8] = { 0x869C, 0, 0, 1, 0x8498, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A62C4[10] = {
    0x869C, 1, 0x869B, 0, 0x8498, 0, 0x7013, 1,
    0xFFFF, 0,
};
u16 D_WSTAG731_800A62D8[4] = { 0x8690, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A62E0[6] = { 0x8690, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A62EC[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A62F4[8] = { 0x8690, 0, 1, 1, 0x848C, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A6304[8] = { 0x8690, 0, 1, 1, 0x848C, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A6314[10] = {
    0x8690, 1, 0x868F, 0, 0x848C, 0, 0x7013, 1,
    0xFFFF, 0,
};
u16 D_WSTAG731_800A6328[4] = { 0x8677, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A6330[6] = { 0x8677, 0, 2, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A633C[4] = { 2, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A6344[8] = { 0x8677, 0, 2, 1, 0x8473, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A6354[8] = { 0x8677, 0, 2, 1, 0x8473, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A6364[10] = {
    0x8677, 1, 0x8676, 0, 0x8473, 0, 0x7013, 1,
    0xFFFF, 0,
};
u16 D_WSTAG731_800A6378[4] = { 0x8683, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A6380[6] = { 0x8683, 0, 3, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A638C[4] = { 3, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A6394[8] = { 0x8683, 0, 3, 1, 0x847F, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A63A4[8] = { 0x8683, 0, 3, 1, 0x847F, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A63B4[10] = {
    0x8683, 1, 0x8682, 0, 0x847F, 0, 0x7013, 1,
    0xFFFF, 0,
};
u16 D_WSTAG731_800A63C8[4] = { 0x8669, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A63D0[6] = { 0x8669, 0, 4, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A63DC[4] = { 4, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A63E4[8] = { 0x8669, 0, 4, 1, 0x8465, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A63F4[8] = { 0x8669, 0, 4, 1, 0x8465, 1, 0xFFFF, 0 };
u16 D_WSTAG731_800A6404[10] = {
    0x8669, 1, 0x8668, 0, 0x8465, 0, 0x7013, 1,
    0xFFFF, 0,
};
FieldstgTalk D_WSTAG731_800A6418[2] = { { NULL, D_WSTAG731_800A6280, 719 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG731_800A6430[2] = { { NULL, NULL, 721 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG731_800A6448[5] = {
    { D_WSTAG731_800A6288, NULL, 778 }, { D_WSTAG731_800A6290, D_WSTAG731_800A629C, 779 },
    { D_WSTAG731_800A62A4, NULL, 780 }, { D_WSTAG731_800A62B4, D_WSTAG731_800A62C4, 781 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG731_800A6484[2] = { { NULL, NULL, 790 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG731_800A649C[5] = {
    { D_WSTAG731_800A62D8, NULL, 782 }, { D_WSTAG731_800A62E0, D_WSTAG731_800A62EC, 783 },
    { D_WSTAG731_800A62F4, NULL, 784 }, { D_WSTAG731_800A6304, D_WSTAG731_800A6314, 785 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG731_800A64D8[2] = { { NULL, NULL, 791 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG731_800A64F0[5] = {
    { D_WSTAG731_800A6328, NULL, 786 }, { D_WSTAG731_800A6330, D_WSTAG731_800A633C, 787 },
    { D_WSTAG731_800A6344, NULL, 788 }, { D_WSTAG731_800A6354, D_WSTAG731_800A6364, 789 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG731_800A652C[2] = { { NULL, NULL, 792 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG731_800A6544[5] = {
    { D_WSTAG731_800A6378, NULL, 793 }, { D_WSTAG731_800A6380, D_WSTAG731_800A638C, 794 },
    { D_WSTAG731_800A6394, NULL, 795 }, { D_WSTAG731_800A63A4, D_WSTAG731_800A63B4, 796 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG731_800A6580[2] = { { NULL, NULL, 797 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG731_800A6598[5] = {
    { D_WSTAG731_800A63C8, NULL, 799 }, { D_WSTAG731_800A63D0, D_WSTAG731_800A63DC, 800 },
    { D_WSTAG731_800A63E4, NULL, 801 }, { D_WSTAG731_800A63F4, D_WSTAG731_800A6404, 802 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG731_800A65D4[2] = { { NULL, NULL, 798 }, { NULL, NULL, 0 } };
u16 D_WSTAG731_800A65EC[6] = { 0x869B, 0, 0x869C, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A65F8[6] = { 0x869B, 1, 0x869C, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A6604[8] = { 0x868F, 0, 0x869C, 1, 0x8690, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A6614[8] = { 0x869C, 1, 0x868F, 1, 0x8690, 0, 0xFFFF, 0 };
u16 D_WSTAG731_800A6624[10] = {
    0x869C, 1, 0x8690, 1, 0x8676, 0, 0x8677, 0,
    0xFFFF, 0,
};
u16 D_WSTAG731_800A6638[10] = {
    0x869C, 1, 0x8690, 1, 0x8676, 1, 0x8677, 0,
    0xFFFF, 0,
};
u16 D_WSTAG731_800A664C[12] = {
    0x869C, 1, 0x8690, 1, 0x8677, 1, 0x8682, 0,
    0x8683, 0, 0xFFFF, 0,
};
u16 D_WSTAG731_800A6664[12] = {
    0x869C, 1, 0x8690, 1, 0x8677, 1, 0x8682, 1,
    0x8683, 0, 0xFFFF, 0,
};
u16 D_WSTAG731_800A667C[14] = {
    0x8668, 0, 0x869C, 1, 0x8690, 1, 0x8677, 1,
    0x8683, 1, 0x8669, 0, 0xFFFF, 0,
};
u16 D_WSTAG731_800A6698[14] = {
    0x869C, 1, 0x8690, 1, 0x8677, 1, 0x8683, 1,
    0x8668, 1, 0x8669, 0, 0xFFFF, 0,
};
u16 D_WSTAG731_800A66B4[12] = {
    0x869C, 1, 0x8690, 1, 0x8677, 1, 0x8683, 1,
    0x8669, 1, 0xFFFF, 0,
};
FieldstgPlacedActor D_WSTAG731_800A66CC = { NULL, D_WSTAG731_800A6418, 21, 4, 461, 190, 1 };
FieldstgPlacedActor D_WSTAG731_800A66E0 = { D_WSTAG731_800A65EC, D_WSTAG731_800A6430, 193, 5, 304, 176, 1 };
FieldstgPlacedActor D_WSTAG731_800A66F4 = { D_WSTAG731_800A65F8, D_WSTAG731_800A6448, 193, 5, 304, 176, 1 };
FieldstgPlacedActor D_WSTAG731_800A6708 = { D_WSTAG731_800A6604, D_WSTAG731_800A6484, 193, 5, 304, 176, 1 };
FieldstgPlacedActor D_WSTAG731_800A671C = { D_WSTAG731_800A6614, D_WSTAG731_800A649C, 193, 5, 304, 176, 1 };
FieldstgPlacedActor D_WSTAG731_800A6730 = { D_WSTAG731_800A6624, D_WSTAG731_800A64D8, 193, 5, 304, 176, 1 };
FieldstgPlacedActor D_WSTAG731_800A6744 = { D_WSTAG731_800A6638, D_WSTAG731_800A64F0, 193, 5, 304, 176, 1 };
FieldstgPlacedActor D_WSTAG731_800A6758 = { D_WSTAG731_800A664C, D_WSTAG731_800A652C, 193, 5, 304, 176, 1 };
FieldstgPlacedActor D_WSTAG731_800A676C = { D_WSTAG731_800A6664, D_WSTAG731_800A6544, 193, 5, 304, 176, 1 };
FieldstgPlacedActor D_WSTAG731_800A6780 = { D_WSTAG731_800A667C, D_WSTAG731_800A6580, 193, 5, 304, 176, 1 };
FieldstgPlacedActor D_WSTAG731_800A6794 = { D_WSTAG731_800A6698, D_WSTAG731_800A6598, 193, 5, 304, 176, 1 };
FieldstgPlacedActor D_WSTAG731_800A67A8 = { D_WSTAG731_800A66B4, D_WSTAG731_800A65D4, 193, 5, 304, 176, 1 };
FieldstgPlacedActor *wstag731_actors[13] = {
    &D_WSTAG731_800A66CC, &D_WSTAG731_800A66E0, &D_WSTAG731_800A66F4, &D_WSTAG731_800A6708, &D_WSTAG731_800A671C,
    &D_WSTAG731_800A6730, &D_WSTAG731_800A6744, &D_WSTAG731_800A6758, &D_WSTAG731_800A676C, &D_WSTAG731_800A6780,
    &D_WSTAG731_800A6794, &D_WSTAG731_800A67A8, NULL,
};
FieldstgSprite wstag731_sprites[19] = {
    { 1, 0, 0x40, 2, 0x34, 2, 0, 5, 6, 0, 124, 112, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 5, 6, 0, 187, 83, 0, 0 },
    { 1, 0, 0x40, 2, 0x41, 2, 0, 3, 6, 0, 470, 34, 0, 0 }, { 1, 0, 0x56, 2, 1, 0, 0, 0, 0, 0, 335, 60, 0, 0 },
    { 1, 1, 0x80, 6, 0x35, 0, 0, 0, 0, 0, 518, 136, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 284, 28, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 286, 62, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 313, 43, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 315, 77, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 342, 57, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 345, 92, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 289, 26, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 291, 60, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 317, 40, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 319, 74, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 346, 54, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 348, 89, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 312, 144, 195, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag731_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2D0, 0x318, 0xAC, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag731_funcs = { wstag731_setup };
FieldstgEventDef wstag731_events[2] = {
    { 1236, D_WSTAG731_800A6170, 0x014A0005, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
