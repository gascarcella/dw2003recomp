#include "wstag.h"

/* WSTAG680: stage 0x25F (fieldstg_stages). */

/* A sprite that falls when it is told to (wstag680_drops_message), animated. */
typedef struct Wstag680Drop {
    /* 0x00 */ s32 y;      /* the sprite's first y, 24.8 */
    /* 0x04 */ s32 dy;     /* added to the sprite's y (its animation's keys set it) */
    /* 0x08 */ s16 active;
    /* 0x0A */ s16 type;   /* the sprite's type */
    /* 0x0C */ WstagAnim anim;
    /* 0x10 */ FieldstgSprite *sprite;
} Wstag680Drop; /* size 0x14 */

/* A sprite that falls faster and faster (types 8-10). */
typedef struct Wstag680Fall {
    /* 0x0 */ s16 active;
    /* 0x2 */ s16 speed; /* 8.8 */
    /* 0x4 */ FieldstgSprite *sprite;
} Wstag680Fall; /* size 0x8 */

typedef struct Wstag680Drops {
    /* 0x000 */ Object base;
    /* 0x050 */ Wstag680Drop drops[11];
    /* 0x12C */ Wstag680Fall falls[3];
    /* 0x144 */ s32 fall_time; /* frames the player falls */
    /* 0x148 */ s16 falling; /* the player falls */
    /* 0x14A */ s16 fall_speed; /* the player's speed */
} Wstag680Drops; /* size 0x14C */

extern WstagAnimKeyB *D_WSTAG680_800A6790[];
extern WstagFuncs wstag680_funcs;
extern FieldstgVramPlace wstag680_vram_places[];
extern FieldstgPlacedActor *wstag680_actors[];
extern FieldstgSprite wstag680_sprites[];
extern FieldstgEventDef wstag680_events[];
void wstag680_update();
void wstag680_drops_update(Wstag680Drops *obj);

s32 wstag680_drop_anim_loop_b(Wstag680Drop *drop, WstagAnimKeyB *keys, s32 depth) {
    WstagAnimKeyB *key = &keys[drop->anim.key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        drop->anim.time -= step;
    }
    if (drop->anim.time <= 0) {
        key++;
        drop->anim.key++;
        drop->anim.time += key->time;
        if (key->frame == 0xFF) {
            drop->dy = key->end_value;
            key = keys;
            drop->anim.key = 0;
            drop->anim.time += key->time;
        }
        wstag680_drop_anim_loop_b(drop, keys, depth + 1);
    }
    return key->frame;
}

void wstag680_drops_update(Wstag680Drops *obj) {
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    FieldstgActor *player;
    s32 n;
    s32 m;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        n = 0;
        m = 0;
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            switch (sprite->type) {
            case 0:
                break;
            case 8:
            case 9:
            case 10:
                obj->falls[m].active = 0;
                obj->falls[m].speed = 0;
                obj->falls[m].sprite = sprite;
                m++;
                break;
            default:
                obj->drops[n].type = sprite->type;
                obj->drops[n].y = sprite->y << 8;
                obj->drops[n].active = 0;
                obj->drops[n].anim.key = 0;
                obj->drops[n].anim.time = D_WSTAG680_800A6790[n][0].time;
                obj->drops[n].sprite = sprite;
                n++;
                break;
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (i = 0; i < 11; i++) {
            if (obj->drops[i].active) {
                spr = obj->drops[i].sprite;
                spr->sprite = wstag680_drop_anim_loop_b(&obj->drops[i], D_WSTAG680_800A6790[i], 0);
                spr->y += obj->drops[i].dy;
                obj->drops[i].dy = 0;
                if (spr->y >= 0x191) {
                    obj->drops[i].active = 0;
                }
            }
        }
        for (i = 0; i < 3; i++) {
            if (obj->falls[i].active) {
                spr = obj->falls[i].sprite;
                obj->falls[i].speed += gfx_module.funcs.get_frame_ticks() << 7;
                spr->y += obj->falls[i].speed >> 8;
                if (spr->y >= 0x191) {
                    obj->falls[i].active = 0;
                }
            }
        }
        if (obj->falling) {
            player = (FieldstgActor *)heap_objects.find(5, -1, 0);
            obj->fall_speed += 0x40;
            player->pos.y += obj->fall_speed;
            if (obj->fall_time++ >= 0x79) {
                obj->falling = 0;
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag680_drops_message(void *arg0, s32 msg) {
    Wstag680Drops *obj = arg0;
    FieldstgActor *player;
    s32 type;
    s32 i;

    if (obj == NULL) {
        return;
    }
    player = (FieldstgActor *)heap_objects.find(5, -1, 0);
    type = 0;
    switch (msg) {
    case 0x344:
        type = 0;
        break;
    case 0x33D:
        type = 1;
        break;
    case 0x33E:
        type = 2;
        break;
    case 0x33F:
        obj->falls[0].active = 1;
        type = 3;
        break;
    case 0x340:
        type = 4;
        break;
    case 0x341:
        obj->falls[2].active = 1;
        type = 5;
        break;
    case 0x342:
        type = 6;
        break;
    case 0x343:
        type = 7;
        obj->falls[1].active = 1;
        player->has_shadow = 0;
        break;
    case 0x37B:
        obj->falling = 1;
        return;
    }
    if (type != 0) {
        for (i = 0; i < 11; i++) {
            if (type == obj->drops[i].type) {
                obj->drops[i].active = 1;
            }
        }
        sound_module.play(0xEC0001);
    }
}

Object *wstag680_drops_create(s32 arg0) {
    return object_create(wstag680_drops_update, sizeof(Wstag680Drops), 0, arg0);
}

void wstag680_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_data.progress == 0xF) {
            data->event = fieldstg_event_start(0x19C);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag680_start(void *arg0) {
    WstagObject *obj = object_new(wstag680_update, sizeof(WstagObject), sizeof(FieldstgEvent *));

    obj->manager = arg0;
    wstag680_funcs.setup();
    return obj;
}

void wstag680_setup(void) {
    fieldstg_stage.background_file = 0x63C;
    fieldstg_stage.sprite_file = 0x063D0000;
    fieldstg_stage.sprites = wstag680_sprites;
    fieldstg_stage.mask_file = 0x63B;
    fieldstg_stage.talk_file = records_language + 0xCC;
    fieldstg_stage.start_pos = (GamestatePos){ 0x9F00, 0x8500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag680_vram_places;
    fieldstg_stage.music = 0x3B;
    fieldstg_stage.sound = 0x60EC0000;
    fieldstg_stage.actors = wstag680_actors;
    fieldstg_stage.events = wstag680_events;
    fieldstg_attr.set_file(0, 0x063D0001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag680_setup(void);

s16 D_WSTAG680_800A657C[239] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 162, 122),
    FIELDSTG_EVENT_PLACE(1, 260, 186),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_ANIM(0x32E, 836, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 144, 128, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 58, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32E, 829, 814),
    FIELDSTG_EVENT_WAIT_ANIM(0x32E),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 873, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x32E, 830, 814),
    FIELDSTG_EVENT_WAIT_ANIM(0x32E),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 1, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32E, 831, 814),
    FIELDSTG_EVENT_WAIT_ANIM(0x32E),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 176, 112, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(0x32E, 832, 814),
    FIELDSTG_EVENT_WAIT_ANIM(0x32E),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x32E, 833, 814),
    FIELDSTG_EVENT_WAIT_ANIM(0x32E),
    FIELDSTG_EVENT_ANIM(1, 51, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 1, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 7),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 144, 96, 7),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(0x32E, 834, 814),
    FIELDSTG_EVENT_WAIT_ANIM(0x32E),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x32E, 835, 814),
    FIELDSTG_EVENT_WAIT_ANIM(0x32E),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 37, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32E, 891, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0x260, 816, 264, 0),
    FIELDSTG_EVENT_END,
};
WstagAnimKeyB D_WSTAG680_800A675C[4] = { { 14, 6, 0 }, { 15, 6, 0 }, { 16, 6, 0 }, { 255, 0, 0x48 } };
WstagAnimKeyB D_WSTAG680_800A676C[4] = { { 10, 6, 0 }, { 11, 6, 0 }, { 12, 6, 0 }, { 255, 0, 0x50 } };
WstagAnimKeyB D_WSTAG680_800A677C[5] = { { 5, 6, 0 }, { 6, 6, 0 }, { 7, 6, 0 }, { 8, 6, 0 }, { 255, 0, 0x50 } };
WstagAnimKeyB *D_WSTAG680_800A6790[11] = {
    D_WSTAG680_800A675C, D_WSTAG680_800A676C, D_WSTAG680_800A675C, D_WSTAG680_800A676C, D_WSTAG680_800A676C,
    D_WSTAG680_800A675C, D_WSTAG680_800A677C, D_WSTAG680_800A675C, D_WSTAG680_800A676C, D_WSTAG680_800A677C,
    D_WSTAG680_800A677C,
};
FieldstgVramPlace wstag680_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 356, 216, 100, 336, 511 },
};
u16 D_WSTAG680_800A682C[6] = { 0x600F, 1, 0x401C, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG680_800A6838 = { D_WSTAG680_800A682C, NULL, 1, 4, 0, 0, 0 };
FieldstgPlacedActor *wstag680_actors[2] = { &D_WSTAG680_800A6838, NULL };
FieldstgSprite wstag680_sprites[18] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 1, 6, 0, 56, 56, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 1, 6, 0, 124, 22, 0, 0 },
    { 1, 0, 0x40, 6, 3, 0, 0, 0, 0, 0, 240, 176, 0, 0 }, { 1, 1, 0x40, 6, 0xD, 0, 0, 0, 0, 0, 208, 160, 0, 0 },
    { 1, 2, 0x40, 6, 9, 0, 0, 0, 0, 0, 176, 144, 0, 0 }, { 1, 3, 0x40, 6, 0xD, 0, 0, 0, 0, 0, 112, 144, 0, 0 },
    { 1, 3, 0x40, 6, 9, 0, 0, 0, 0, 0, 80, 128, 0, 0 }, { 1, 4, 0x40, 6, 9, 0, 0, 0, 0, 0, 144, 128, 0, 0 },
    { 1, 3, 0x40, 6, 0xD, 0, 0, 0, 0, 0, 48, 112, 0, 0 }, { 1, 4, 0x40, 6, 4, 0, 0, 0, 0, 0, 112, 112, 0, 0 },
    { 1, 5, 0x40, 6, 0xD, 0, 0, 0, 0, 0, 176, 112, 0, 0 }, { 1, 4, 0x40, 6, 9, 0, 0, 0, 0, 0, 80, 96, 0, 0 },
    { 1, 6, 0x40, 6, 4, 0, 0, 0, 0, 0, 144, 96, 0, 0 }, { 1, 7, 0x40, 6, 4, 0, 0, 0, 0, 0, 112, 80, 0, 0 },
    { 1, 8, 0x40, 6, 0, 0, 0, 0, 0, 0, 55, 119, 0, 0 }, { 1, 0xA, 0x40, 6, 1, 0, 0, 0, 0, 0, 122, 85, 0, 0 },
    { 1, 9, 0x40, 6, 2, 0, 0, 0, 0, 0, 216, 127, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag680_funcs = { wstag680_setup };
FieldstgEventDef wstag680_events[2] = {
    { 412, D_WSTAG680_800A657C, 0x01430002, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
