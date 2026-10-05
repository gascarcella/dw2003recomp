#include "wstag.h"

/* WSTAG741: stage 0x2D3 (fieldstg_stages). */

extern s32 D_WSTAG741_800A6AE4[];
extern WstagAnimKey D_WSTAG741_800A6AF0[];
extern WstagAnimKey D_WSTAG741_800A6B0C[];
extern WstagAnimKey *D_WSTAG741_800A6B44[];
extern WstagFuncs wstag741_funcs;
const CVECTOR wstag741_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag741_battle_lists;
extern FieldstgVramPlace wstag741_vram_places[];
extern FieldstgPlacedActor *wstag741_actors[];
extern FieldstgSprite wstag741_sprites[];
extern FieldstgMapEvent wstag741_map_events[];
extern FieldstgEventDef wstag741_events[];
void wstag741_event_9000_update();
void wstag741_update();
WstagTwoAnimObject *wstag741_two_anim_create(s32 x, s32 y);

void wstag741_event_9000_update(WstagObject *obj, WstagTwoAnimData *data) {
    Object *other;
    FieldstgActor *actor;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            switch (obj->base.substep) {
            case 0:
            default:
                other = heap_objects.find(5, -1, 0);
                other->set_step(other, 1);
                sound_module.play(0x800410BD);
                obj->base.next_substep(obj);
            case 1:
                obj->base.timer += gfx_module.funcs.get_frame_ticks();
                if (obj->base.timer >= 0x1E) {
                    obj->base.next_step(obj);
                }
                break;
            }
            break;
        case 1:
        case 2:
        case 3:
            switch (obj->base.substep) {
            case 0:
            default:
                i = obj->base.step - 1;
                actor = (FieldstgActor *)heap_objects.find(5, -1, D_WSTAG741_800A6AE4[i]);
                if (actor != NULL) {
                    data->objs[i] = wstag741_two_anim_create(actor->pixel_pos.x, actor->pixel_pos.y);
                    sound_module.play(0x800446C9);
                    actor->base.set_step(actor, 4);
                    gamestate_data.digimon[gamestate_data.party[i]].record.stats.values[2] = 1;
                    obj->base.next_substep(obj);
                } else {
                    obj->base.next_step(obj);
                }
                break;
            case 1:
                obj->base.timer += gfx_module.funcs.get_frame_ticks();
                if (obj->base.timer >= 0x1E) {
                    obj->base.next_step(obj);
                }
                break;
            }
            break;
        case 4:
            obj->base.next_state(obj);
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        if (data->objs[0] == NULL) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag741_event_9000_start(void) {
    return object_new(wstag741_event_9000_update, sizeof(WstagObject), sizeof(WstagTwoAnimData));
}

void wstag741_update(WstagObject *obj, WstagObjEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x4084, 1) && gamestate_flags.get_flag(0x4085, 0)) {
            data->event = fieldstg_event_start(0x510);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag741_start(void *arg0) {
    WstagObject *obj = object_new(wstag741_update, sizeof(WstagObject), sizeof(WstagObjEventData));

    obj->manager = arg0;
    wstag741_funcs.setup();
    return obj;
}

s32 wstag741_anim_play_once(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
            return 0xFF;
        }
        wstag741_anim_play_once(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag741_two_anim_draw(WstagTwoAnimObject *obj, GfxLayer *layer, s32 which) {
    Sprite spr;
    WstagFrame *frame = &obj->sprites[which];

    sprite_init(&spr);
    spr.set_vram_pos(0x140, 0x100);
    spr.set_layer(layer, 4);
    spr.set_palette(frame->palette);
    spr.draw(cdload_module.get_subfile_by_id(fieldstg_stage.sprite_file), frame->frame, obj->x, obj->y);
}

void wstag741_two_anim_update(WstagTwoAnimObject *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 done;
    s32 i;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG741_800A6AF0[0].time;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG741_800A6B0C[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        done = 0;
        for (i = 0; i < 2; i++) {
            frame = wstag741_anim_play_once(&obj->anims[i], D_WSTAG741_800A6B44[i], 0);
            switch (frame) {
            default:
                obj->sprites[i].frame = frame;
                break;
            case 0xFF:
                obj->sprites[i].frame = 0;
                done++;
                break;
            case 0x12C:
                obj->sprites[i].frame = 0;
                break;
            }
            if (obj->sprites[i].frame != 0) {
                layer->add_callback(layer, wstag741_two_anim_draw, obj, obj->y + (i == 0 ? -0xF0 : 0x14), i);
            }
        }
        if (done == 2) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagTwoAnimObject *wstag741_two_anim_create(s32 x, s32 y) {
    WstagTwoAnimObject *obj = object_new(wstag741_two_anim_update, sizeof(WstagTwoAnimObject), 0);

    obj->x = x;
    obj->y = y;
    return obj;
}

void wstag741_event_1295_end(void) {
    gamestate_flags.set_flag(0x4084, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag741_event_1296_end(void) {
    gamestate_flags.set_flag(0x4085, 1);
    gamestate_flags.set_flag(0x8233, 1);
}

void wstag741_event_9000_end(void) {
    gamestate_flags.set_flag(0x40BA, 1);
}

void wstag741_event_9001_end(void) {
    gamestate_flags.set_flag(0x40BB, 1);
}

void wstag741_event_9002_end(void) {
    gamestate_flags.set_flag(0x40BC, 1);
}

void wstag741_event_9003_end(void) {
    gamestate_flags.set_flag(0x40BD, 1);
}

void wstag741_event_9004_end(void) {
    gamestate_flags.set_flag(0x40BE, 1);
}

void wstag741_event_9005_end(void) {
    gamestate_flags.set_flag(0x40BF, 1);
}

void wstag741_event_9006_end(void) {
    gamestate_flags.set_flag(0x40C0, 1);
}

void wstag741_event_9007_end(void) {
    gamestate_flags.set_flag(0x40C1, 1);
}

void wstag741_event_9008_end(void) {
    gamestate_flags.set_flag(0x40C2, 1);
}

void wstag741_event_9009_end(void) {
    gamestate_flags.set_flag(0x40C3, 1);
}

void wstag741_event_9010_end(void) {
    gamestate_flags.set_flag(0x40C4, 1);
}

void wstag741_event_9011_end(void) {
    gamestate_flags.set_flag(0x40C5, 1);
}

void wstag741_event_9012_end(void) {
    gamestate_flags.set_flag(0x40C6, 1);
}

void wstag741_event_9013_end(void) {
    gamestate_flags.set_flag(0x40C7, 1);
}

void wstag741_event_9014_end(void) {
    gamestate_flags.set_flag(0x40C8, 1);
}

void wstag741_event_9015_end(void) {
    gamestate_flags.set_flag(0x40C9, 1);
}

void wstag741_setup(void) {
    fieldstg_stage.background_file = 0x750;
    fieldstg_stage.sprite_file = 0x07510000;
    fieldstg_stage.sprites = wstag741_sprites;
    fieldstg_stage.map_events = wstag741_map_events;
    fieldstg_stage.mask_file = 0x74F;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1CD00, 0x29A00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag741_vram_places;
    fieldstg_stage.music = 0x19;
    fieldstg_stage.sound = 0x60640000;
    fieldstg_stage.actors = wstag741_actors;
    fieldstg_stage.color = wstag741_color;
    fieldstg_stage.battle_lists = &wstag741_battle_lists;
    fieldstg_stage.events = wstag741_events;
    fieldstg_attr.set_file(0, 0x07510001);
    fieldstg_attr.set_file(7, 0x07510002);
    fieldstg_attr.set_file(4, 0x07510003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag741_setup(void);

s16 D_WSTAG741_800A6994[75] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 533, 684, 3),
    FIELDSTG_EVENT_PLACE(204, 512, 673),
    FIELDSTG_EVENT_ANIM(204, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 204, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 204, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG741_800A6A2C[91] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 204),
    FIELDSTG_EVENT_PLACE(2, 533, 684),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(204, 512, 673),
    FIELDSTG_EVENT_ANIM(204, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 204, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_ANIM(204, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(204, 464, 648, 3),
    FIELDSTG_EVENT_WAIT_WALK(204),
    FIELDSTG_EVENT_ANIM(204, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 512, 673, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 204, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s32 D_WSTAG741_800A6AE4[3] = { 2, 4, 8 };
WstagAnimKey D_WSTAG741_800A6AF0[7] = {
    { 50, 4 }, { 51, 4 }, { 52, 4 }, { 53, 4 }, { 54, 4 }, { 55, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG741_800A6B0C[14] = {
    { 300, 4 }, { 56, 4 }, { 57, 4 }, { 58, 4 }, { 59, 4 }, { 60, 4 }, { 61, 4 }, { 62, 4 }, { 63, 4 }, { 64, 4 },
    { 65, 4 }, { 66, 4 }, { 67, 4 }, { 255, 999 },
};
WstagAnimKey *D_WSTAG741_800A6B44[2] = { D_WSTAG741_800A6AF0, D_WSTAG741_800A6B0C };
FieldstgListedBattle D_WSTAG741_800A6B4C = { 136, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG741_800A6B58 = { 136, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG741_800A6B64 = { 136, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG741_800A6B70 = { 183, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG741_800A6B7C = { 183, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG741_800A6B88 = { 183, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG741_800A6B94 = { 118, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG741_800A6BA0 = { 118, 24, 0x60080000 };
FieldstgBattleList D_WSTAG741_800A6BAC = {
    5,
    { &D_WSTAG741_800A6B4C, &D_WSTAG741_800A6B58, &D_WSTAG741_800A6B64, &D_WSTAG741_800A6B70, &D_WSTAG741_800A6B7C,
        &D_WSTAG741_800A6B88, &D_WSTAG741_800A6B94, &D_WSTAG741_800A6BA0 },
};
FieldstgListedBattle D_WSTAG741_800A6BD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6BDC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6BE8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6BF4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6C00 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6C0C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6C18 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6C24 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG741_800A6C30 = {
    0,
    { &D_WSTAG741_800A6BD0, &D_WSTAG741_800A6BDC, &D_WSTAG741_800A6BE8, &D_WSTAG741_800A6BF4, &D_WSTAG741_800A6C00,
        &D_WSTAG741_800A6C0C, &D_WSTAG741_800A6C18, &D_WSTAG741_800A6C24 },
};
FieldstgListedBattle D_WSTAG741_800A6C54 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6C60 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6C6C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6C78 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6C84 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6C90 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6C9C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6CA8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG741_800A6CB4 = {
    0,
    { &D_WSTAG741_800A6C54, &D_WSTAG741_800A6C60, &D_WSTAG741_800A6C6C, &D_WSTAG741_800A6C78, &D_WSTAG741_800A6C84,
        &D_WSTAG741_800A6C90, &D_WSTAG741_800A6C9C, &D_WSTAG741_800A6CA8 },
};
FieldstgListedBattle D_WSTAG741_800A6CD8 = { 28, 24, 0x608C0000 };
FieldstgListedBattle D_WSTAG741_800A6CE4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6CF0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6CFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6D08 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6D14 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6D20 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG741_800A6D2C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG741_800A6D38 = {
    0,
    { &D_WSTAG741_800A6CD8, &D_WSTAG741_800A6CE4, &D_WSTAG741_800A6CF0, &D_WSTAG741_800A6CFC, &D_WSTAG741_800A6D08,
        &D_WSTAG741_800A6D14, &D_WSTAG741_800A6D20, &D_WSTAG741_800A6D2C },
};
FieldstgBattleLists wstag741_battle_lists = {
    121, 0, 0, { &D_WSTAG741_800A6BAC, &D_WSTAG741_800A6C30, &D_WSTAG741_800A6CB4 }, &D_WSTAG741_800A6D38,
};
FieldstgVramPlace wstag741_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 384, 344, 256, 88, 320, 507 }, { 384, 256, 422, 344, 408, 88, 336, 507 },
};
u16 D_WSTAG741_800A6DF8[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG741_800A6E00[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG741_800A6E08[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG741_800A6E10[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG741_800A6E18[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG741_800A6E20[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG741_800A6E28[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG741_800A6E30[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG741_800A6E38[2] = { { NULL, NULL, 532 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG741_800A6E50[2] = { { NULL, NULL, 532 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG741_800A6E68[5] = {
    { D_WSTAG741_800A6DF8, NULL, 529 }, { D_WSTAG741_800A6E00, NULL, 530 }, { D_WSTAG741_800A6E08, NULL, 531 },
    { D_WSTAG741_800A6E10, NULL, 518 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG741_800A6EA4[5] = {
    { D_WSTAG741_800A6E18, NULL, 529 }, { D_WSTAG741_800A6E20, NULL, 530 }, { D_WSTAG741_800A6E28, NULL, 531 },
    { D_WSTAG741_800A6E30, NULL, 533 }, { NULL, NULL, 0 },
};
u16 D_WSTAG741_800A6EE0[6] = { 0x4085, 0, 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG741_800A6EEC[6] = { 0x701A, 1, 0x4085, 1, 0xFFFF, 0 };
u16 D_WSTAG741_800A6EF8[8] = { 0x7008, 1, 0x4085, 0, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG741_800A6F08[8] = { 0x7008, 1, 0x4085, 1, 0x701A, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG741_800A6F18 = { D_WSTAG741_800A6EE0, D_WSTAG741_800A6E38, 157, 4, 512, 673, 7 };
FieldstgPlacedActor D_WSTAG741_800A6F2C = { D_WSTAG741_800A6EEC, D_WSTAG741_800A6E50, 157, 4, 464, 648, 7 };
FieldstgPlacedActor D_WSTAG741_800A6F40 = { D_WSTAG741_800A6EF8, D_WSTAG741_800A6E68, 204, 5, 512, 673, 7 };
FieldstgPlacedActor D_WSTAG741_800A6F54 = { D_WSTAG741_800A6F08, D_WSTAG741_800A6EA4, 204, 5, 464, 648, 7 };
FieldstgPlacedActor *wstag741_actors[5] = {
    &D_WSTAG741_800A6F18, &D_WSTAG741_800A6F2C, &D_WSTAG741_800A6F40, &D_WSTAG741_800A6F54, NULL,
};
FieldstgSprite wstag741_sprites[17] = {
    { 1, 0, 0x40, 4, 0x50, 1, 0x50, 0x63, 6, 0, 224, 601, 638, 0 },
    { 1, 0, 0x40, 4, 0x50, 1, 0x50, 0x63, 5, 0, 253, 334, 371, 0 },
    { 1, 0, 0x40, 4, 0x50, 1, 0x50, 0x63, 5, 0, 465, 242, 274, 0 },
    { 1, 0, 0x40, 4, 0x50, 1, 0x50, 0x63, 6, 0, 607, 402, 441, 0 },
    { 1, 0, 0x40, 4, 0x50, 1, 0x50, 0x63, 4, 0, 647, 343, 371, 0 },
    { 1, 0, 0x40, 4, 0x50, 1, 0x50, 0x63, 4, 0, 699, 740, 781, 0 },
    { 1, 0, 0x40, 4, 0x50, 1, 0x50, 0x63, 4, 0, 760, 579, 615, 0 },
    { 1, 0, 0x40, 4, 0xA, 1, 0xA, 0x1D, 5, 0, 213, 576, 638, 0 },
    { 1, 0, 0x40, 4, 0xA, 1, 0xA, 0x1D, 6, 0, 267, 297, 371, 0 },
    { 1, 0, 0x40, 4, 0xA, 1, 0xA, 0x1D, 4, 0, 472, 403, 466, 0 },
    { 1, 0, 0x40, 4, 0xA, 1, 0xA, 0x1D, 5, 0, 616, 385, 441, 0 },
    { 1, 0, 0x40, 4, 0xA, 1, 0xA, 0x1D, 6, 0, 855, 692, 756, 0 },
    { 1, 0, 0x40, 4, 0x1E, 1, 0x1E, 0x31, 5, 0, 331, 427, 493, 0 },
    { 1, 0, 0x40, 4, 0x1E, 1, 0x1E, 0x31, 6, 0, 465, 211, 274, 0 },
    { 1, 0, 0x40, 4, 0x1E, 1, 0x1E, 0x31, 4, 0, 698, 429, 493, 0 },
    { 1, 0, 0x40, 4, 0x1E, 1, 0x1E, 0x31, 5, 0, 699, 715, 781, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag741_map_events[20] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2D2, 0x70, 0x2EC, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2D4, 0x448, 0x288, 1, 0, 0, 0 },
    { 0x40BA, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 }, { 0x40BB, 0, 0xFFFF, 0, 8, 0x2329, 0, 0, 0, 0, 0, 0 },
    { 0x40BC, 0, 0xFFFF, 0, 8, 0x232A, 0, 0, 0, 0, 0, 0 }, { 0x40BD, 0, 0xFFFF, 0, 8, 0x232B, 0, 0, 0, 0, 0, 0 },
    { 0x40BE, 0, 0xFFFF, 0, 8, 0x232C, 0, 0, 0, 0, 0, 0 }, { 0x40BF, 0, 0xFFFF, 0, 8, 0x232D, 0, 0, 0, 0, 0, 0 },
    { 0x40C0, 0, 0xFFFF, 0, 8, 0x232E, 0, 0, 0, 0, 0, 0 }, { 0x40C1, 0, 0xFFFF, 0, 8, 0x232F, 0, 0, 0, 0, 0, 0 },
    { 0x40C2, 0, 0xFFFF, 0, 8, 0x2330, 0, 0, 0, 0, 0, 0 }, { 0x40C3, 0, 0xFFFF, 0, 8, 0x2331, 0, 0, 0, 0, 0, 0 },
    { 0x40C4, 0, 0xFFFF, 0, 8, 0x2332, 0, 0, 0, 0, 0, 0 }, { 0x40C5, 0, 0xFFFF, 0, 8, 0x2333, 0, 0, 0, 0, 0, 0 },
    { 0x40C6, 0, 0xFFFF, 0, 8, 0x2334, 0, 0, 0, 0, 0, 0 }, { 0x40C7, 0, 0xFFFF, 0, 8, 0x2335, 0, 0, 0, 0, 0, 0 },
    { 0x40C8, 0, 0xFFFF, 0, 8, 0x2336, 0, 0, 0, 0, 0, 0 }, { 0x40C9, 0, 0xFFFF, 0, 8, 0x2337, 0, 0, 0, 0, 0, 0 },
    { 0x4084, 0, 0x701A, 0, 8, 0x50F, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag741_funcs = { wstag741_setup };
FieldstgEventDef wstag741_events[19] = {
    { 1295, D_WSTAG741_800A6994, 0x014A0015, NULL, wstag741_event_1295_end },
    { 1296, D_WSTAG741_800A6A2C, 0x014A0016, NULL, wstag741_event_1296_end },
    { 9000, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9000_end },
    { 9001, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9001_end },
    { 9002, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9002_end },
    { 9003, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9003_end },
    { 9004, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9004_end },
    { 9005, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9005_end },
    { 9006, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9006_end },
    { 9007, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9007_end },
    { 9008, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9008_end },
    { 9009, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9009_end },
    { 9010, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9010_end },
    { 9011, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9011_end },
    { 9012, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9012_end },
    { 9013, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9013_end },
    { 9014, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9014_end },
    { 9015, NULL, 0, (s32 (*)(void))wstag741_event_9000_start, wstag741_event_9015_end }, { -1, NULL, 0, NULL, NULL },
};
