#include "wstag.h"

/* WSTAG925: stage 0x276 (fieldstg_stages_2d). */

extern WstagFuncs wstag925_funcs;
extern WstagAnimKey *D_WSTAG925_800A67B4[3];
extern s8 D_WSTAG925_800A67C0[];
extern s8 D_WSTAG925_800A67C4[];
extern FieldstgVramPlace wstag925_vram_places[];
extern FieldstgPlacedActor *wstag925_actors[];
extern FieldstgSprite wstag925_sprites[];
extern FieldstgMapEvent wstag925_map_events[];
extern FieldstgEventDef wstag925_events[];
void wstag925_sprite_anim_update(WstagSpriteAnimObject *obj);
void wstag925_update();

void wstag925_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_flags.get_flag(0x40CD, 0)) {
            data->event = fieldstg_event_start(0x640);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag925_start(void *arg0) {
    WstagObject *obj = object_new(wstag925_update, sizeof(WstagObject), sizeof(FieldstgEvent *));

    obj->manager = arg0;
    wstag925_funcs.setup();
    return obj;
}

void wstag925_event_1600_end(void) {
    gamestate_flags.set_flag(0x40CD, 1);
    gamestate_flags.set_flag(0x7053, 1);
}

void wstag925_setup(void) {
    fieldstg_stage.background_file = 0x19E;
    fieldstg_stage.sprite_file = 0x08E90000;
    fieldstg_stage.sprites = wstag925_sprites;
    fieldstg_stage.map_events = wstag925_map_events;
    fieldstg_stage.mask_file = 0x8E8;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0xFD00, 0x17700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag925_vram_places;
    fieldstg_stage.music = 0x33;
    fieldstg_stage.sound = 0x60CC0000;
    fieldstg_stage.actors = wstag925_actors;
    fieldstg_stage.events = wstag925_events;
    fieldstg_attr.set_file(0, 0x08E90001);
    fieldstg_attr.set_file(7, 0x08E90002);
    fieldstg_attr.init_layer(0);
}

s32 wstag925_sprite_anim_play_once(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 depth) {
    WstagAnimKey *key = &keys[sa->anim.key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        sa->anim.time -= step;
    }
    if (sa->anim.time <= 0) {
        key++;
        sa->anim.key++;
        sa->anim.time += key->time;
        if (key->frame == 0xFF) {
            return 0xFF;
        }
        wstag925_sprite_anim_play_once(sa, keys, depth + 1);
    }
    return key->frame;
}

void wstag925_sprite_anim_reset(WstagSpriteAnimObject *obj) {
    s32 i;

    for (i = 0; i < 3; i++) {
        obj->sprites[i].anim.key = 0;
        obj->sprites[i].anim.time = D_WSTAG925_800A67B4[i]->time;
    }
}

void wstag925_sprite_anim_update(WstagSpriteAnimObject *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    s32 n;
    s32 i;
    s32 j;
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag925_sprite_anim_reset(obj);
        obj->base.next_state(obj);
        obj->base.set_step(obj, 1);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            switch (obj->base.substep) {
            case 0:
                list = fieldstg_stage.sprites;
                for (n = 0; list->present != 0; list++) {
                    if (list->type >= 1 && list->type <= 3) {
                        obj->sprites[n].sprite = list;
                        list->x = obj->x;
                        list->y = obj->y + D_WSTAG925_800A67C4[n];
                        list->priority = obj->y + D_WSTAG925_800A67C0[n];
                        n++;
                    }
                }
                sound_module.play(0xCC0001);
                obj->base.next_substep(obj);
            case 1:
                done = 0;
                for (i = 0; i < 3; i++) {
                    sprite = obj->sprites[i].sprite;
                    frame = wstag925_sprite_anim_play_once(&obj->sprites[i], D_WSTAG925_800A67B4[i], 0);
                    switch (frame) {
                    case 0xFF:
                        done++;
                        sprite->shown = 0;
                        sprite->sprite = 0;
                        break;
                    case 0x12C:
                        sprite->shown = 0;
                        sprite->sprite = 0;
                        break;
                    default:
                        sprite->shown = 1;
                        sprite->sprite = frame;
                        break;
                    }
                }
                if (done < 3) {
                    break;
                }
                obj->base.next_substep(obj);
            case 2:
                for (j = 0; j < 3; j++) {
                    obj->sprites[j].sprite->shown = 0;
                }
                wstag925_sprite_anim_reset(obj);
                obj->base.set_state(obj, OBJECT_STATE_END);
                break;
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

OBJECT_V0(WstagSpriteAnimObject *) wstag925_sprite_anim_create(s32 arg0) {
    WstagSpriteAnimObject *obj = object_create(wstag925_sprite_anim_update, sizeof(WstagSpriteAnimObject), 0, arg0);

    obj->x = 0x150;
    obj->y = 0x112;
    OBJECT_V0_RETURN(obj) /* PC_PORT: FINDINGS 8: callers use the object (v0) */
}

/* The stage's .data (tools/wstag_data.py). */
void wstag925_setup(void);
extern s16 D_WSTAG925_800A65E4[];

FieldstgVramPlace wstag925_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 330, 440, 40, 184, 368, 510 }, { 320, 256, 368, 432, 192, 176, 352, 509 },
};
FieldstgTalk D_WSTAG925_800A645C[2] = { { NULL, NULL, 38 }, { NULL, NULL, 0 } };
u16 D_WSTAG925_800A6474[4] = { 0x40CD, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG925_800A647C = { D_WSTAG925_800A6474, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG925_800A6490 = { NULL, D_WSTAG925_800A645C, 13, 5, 265, 397, 3 };
FieldstgPlacedActor *wstag925_actors[3] = { &D_WSTAG925_800A647C, &D_WSTAG925_800A6490, NULL };
FieldstgSprite wstag925_sprites[12] = {
    { 0, 1, 0x50, 6, 0x46, 0, 0, 0, 0, 0, 112, 274, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x42, 6, 0, 116, 258, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x42, 6, 0, 228, 282, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x42, 6, 0, 340, 258, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x42, 6, 0, 355, 346, 0, 0 },
    { 0, 2, 0x50, 4, 0x48, 0, 0, 0, 0, 0, 112, 227, 301, 0 },
    { 0, 3, 0x50, 4, 0x55, 0, 0, 0, 0, 0, 112, 227, 294, 0 },
    { 1, 0, 0x40, 4, 0x3B, 1, 0x3B, 0x3E, 6, 0, 116, 274, 300, 0 },
    { 1, 0, 0x40, 4, 0x3B, 1, 0x3B, 0x3E, 6, 0, 228, 298, 324, 0 },
    { 1, 0, 0x40, 4, 0x3B, 1, 0x3B, 0x3E, 6, 0, 340, 274, 300, 0 },
    { 1, 0, 0x40, 4, 0x3B, 1, 0x3B, 0x3E, 6, 0, 355, 362, 388, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag925_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x273, 0x2DA, 0x17E, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag925_funcs = { wstag925_setup };
FieldstgEventDef wstag925_events[2] = {
    { 1600, D_WSTAG925_800A65E4, 0x01580000, NULL, wstag925_event_1600_end }, { -1, NULL, 0, NULL, NULL },
};
s16 D_WSTAG925_800A65E4[149] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 286, 295),
    FIELDSTG_EVENT_PLACE(1, 0, 0),
    FIELDSTG_EVENT_ANIM(1, 1, 0),
    FIELDSTG_EVENT_PLACE(13, 271, 401),
    FIELDSTG_EVENT_ANIM(13, 1, 3),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x357, 821, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_PLACE(1, 368, 287),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(180),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 336, 304, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 58, 1),
    FIELDSTG_EVENT_WAIT(180),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 304, 352, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 1),
    FIELDSTG_EVENT_WALK(1, 240, 384, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 13, 2),
    FIELDSTG_EVENT_ANIM(13, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(13, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 1),
    FIELDSTG_EVENT_ANIM(1, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 80, 464, 1),
    FIELDSTG_EVENT_ANIM(13, 1, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0x273, 730, 382, 1),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG925_800A6710[5] = { { 70, 4 }, { 71, 4 }, { 70, 4 }, { 71, 4 }, { 255, 999 } };
WstagAnimKey D_WSTAG925_800A6724[20] = {
    { 300, 56 }, { 85, 6 }, { 86, 6 }, { 87, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 },
    { 88, 4 }, { 89, 4 }, { 90, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG925_800A6774[16] = {
    { 300, 16 }, { 72, 4 }, { 73, 4 }, { 74, 4 }, { 73, 4 }, { 75, 6 }, { 76, 6 }, { 77, 14 }, { 78, 114 },
    { 79, 6 }, { 80, 6 }, { 81, 6 }, { 82, 6 }, { 83, 6 }, { 84, 8 }, { 255, 999 },
};
WstagAnimKey *D_WSTAG925_800A67B4[3] = { D_WSTAG925_800A6710, D_WSTAG925_800A6774, D_WSTAG925_800A6724 };
s8 D_WSTAG925_800A67C0[4] = { 30, 30, 30, 0 };
s8 D_WSTAG925_800A67C4[3] = { 0, -47, -47 };
