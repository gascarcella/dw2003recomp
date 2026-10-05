#include "wstag.h"

/* WSTAG740: stage 0x26B (fieldstg_stages). */

extern WstagAnimKey D_WSTAG740_800A6ADC[];
extern WstagAnimKey D_WSTAG740_800A6AF8[];
extern WstagAnimKey *D_WSTAG740_800A6B30[];
extern WstagFuncs wstag740_funcs;
void wstag740_two_anim_update(WstagTwoAnimObject *obj);
void wstag740_update();
extern s32 D_WSTAG740_800A6AD0[];
const CVECTOR wstag740_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag740_battle_lists;
extern FieldstgVramPlace wstag740_vram_places[];
extern FieldstgPlacedActor *wstag740_actors[];
extern FieldstgSprite wstag740_sprites[];
extern FieldstgMapEvent wstag740_map_events[];
extern FieldstgEventDef wstag740_events[];

void wstag740_event_9000_update(WstagObject *obj, WstagTwoAnimData *data);
WstagTwoAnimObject *wstag740_two_anim_create(s32 x, s32 y);

void wstag740_event_9000_update(WstagObject *obj, WstagTwoAnimData *data) {
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
                actor = (FieldstgActor *)heap_objects.find(5, -1, D_WSTAG740_800A6AD0[i]);
                if (actor != NULL) {
                    data->objs[i] = wstag740_two_anim_create(actor->pixel_pos.x, actor->pixel_pos.y);
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

Object *wstag740_event_9000_start(void) {
    return object_new(wstag740_event_9000_update, sizeof(WstagObject), sizeof(WstagTwoAnimData));
}

void wstag740_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x403F, 1) && gamestate_flags.get_flag(0x4040, 0)) {
            data->event = fieldstg_event_start(0x321);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag740_start(void *arg0) {
    WstagObject *obj = object_new(wstag740_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag740_funcs.setup();
    return obj;
}

s32 wstag740_anim_play_once(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
        wstag740_anim_play_once(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag740_two_anim_draw(WstagTwoAnimObject *obj, GfxLayer *layer, s32 which) {
    Sprite spr;
    WstagFrame *frame = &obj->sprites[which];

    sprite_init(&spr);
    spr.set_vram_pos(0x140, 0x100);
    spr.set_layer(layer, 4);
    spr.set_palette(frame->palette);
    spr.draw(cdload_module.get_subfile_by_id(fieldstg_stage.sprite_file), frame->frame, obj->x, obj->y);
}

void wstag740_two_anim_update(WstagTwoAnimObject *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 done;
    s32 i;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG740_800A6ADC[0].time;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG740_800A6AF8[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        done = 0;
        for (i = 0; i < 2; i++) {
            frame = wstag740_anim_play_once(&obj->anims[i], D_WSTAG740_800A6B30[i], 0);
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
                layer->add_callback(layer, wstag740_two_anim_draw, obj, obj->y + (i == 0 ? -0xF0 : 0x14), i);
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

WstagTwoAnimObject *wstag740_two_anim_create(s32 x, s32 y) {
    WstagTwoAnimObject *obj = object_new(wstag740_two_anim_update, sizeof(WstagTwoAnimObject), 0);

    obj->x = x;
    obj->y = y;
    return obj;
}

void wstag740_event_800_end(void) {
    gamestate_flags.set_flag(0x403F, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag740_event_801_end(void) {
    gamestate_flags.set_flag(0x8488, 1);
    gamestate_flags.set_flag(0x4040, 1);
}

void wstag740_event_9000_end(void) {
    gamestate_flags.set_flag(0x40AA, 1);
}

void wstag740_event_9001_end(void) {
    gamestate_flags.set_flag(0x40AB, 1);
}

void wstag740_event_9002_end(void) {
    gamestate_flags.set_flag(0x40AC, 1);
}

void wstag740_event_9003_end(void) {
    gamestate_flags.set_flag(0x40AD, 1);
}

void wstag740_event_9004_end(void) {
    gamestate_flags.set_flag(0x40AE, 1);
}

void wstag740_event_9005_end(void) {
    gamestate_flags.set_flag(0x40AF, 1);
}

void wstag740_event_9006_end(void) {
    gamestate_flags.set_flag(0x40B0, 1);
}

void wstag740_event_9007_end(void) {
    gamestate_flags.set_flag(0x40B1, 1);
}

void wstag740_event_9008_end(void) {
    gamestate_flags.set_flag(0x40B2, 1);
}

void wstag740_event_9009_end(void) {
    gamestate_flags.set_flag(0x40B3, 1);
}

void wstag740_event_9010_end(void) {
    gamestate_flags.set_flag(0x40B4, 1);
}

void wstag740_event_9011_end(void) {
    gamestate_flags.set_flag(0x40B5, 1);
}

void wstag740_event_9012_end(void) {
    gamestate_flags.set_flag(0x40B6, 1);
}

void wstag740_event_9013_end(void) {
    gamestate_flags.set_flag(0x40B7, 1);
}

void wstag740_event_9014_end(void) {
    gamestate_flags.set_flag(0x40B8, 1);
}

void wstag740_event_9015_end(void) {
    gamestate_flags.set_flag(0x40B9, 1);
}

void wstag740_setup(void) {
    fieldstg_stage.background_file = 0x6DC;
    fieldstg_stage.sprite_file = 0x06DD0000;
    fieldstg_stage.sprites = wstag740_sprites;
    fieldstg_stage.map_events = wstag740_map_events;
    fieldstg_stage.mask_file = 0x6DB;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x12200, 0x2F400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag740_vram_places;
    fieldstg_stage.music = 0x19;
    fieldstg_stage.sound = 0x60640000;
    fieldstg_stage.actors = wstag740_actors;
    fieldstg_stage.color = wstag740_color;
    fieldstg_stage.battle_lists = &wstag740_battle_lists;
    fieldstg_stage.events = wstag740_events;
    fieldstg_attr.set_file(0, 0x06DD0001);
    fieldstg_attr.set_file(7, 0x06DD0002);
    fieldstg_attr.set_file(4, 0x06DD0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag740_setup(void);

s16 D_WSTAG740_800A6994[83] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 533, 684, 3),
    FIELDSTG_EVENT_PLACE(201, 512, 673),
    FIELDSTG_EVENT_ANIM(201, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 201, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 201, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 201, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG740_800A6A3C[73] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 201),
    FIELDSTG_EVENT_PLACE(2, 533, 684),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(201, 512, 673),
    FIELDSTG_EVENT_ANIM(201, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 201, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(201, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(201, 464, 648, 3),
    FIELDSTG_EVENT_WAIT_WALK(201),
    FIELDSTG_EVENT_ANIM(201, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 512, 673, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 201, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s32 D_WSTAG740_800A6AD0[3] = { 2, 4, 8 };
WstagAnimKey D_WSTAG740_800A6ADC[7] = {
    { 50, 4 }, { 51, 4 }, { 52, 4 }, { 53, 4 }, { 54, 4 }, { 55, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG740_800A6AF8[14] = {
    { 300, 4 }, { 56, 4 }, { 57, 4 }, { 58, 4 }, { 59, 4 }, { 60, 4 }, { 61, 4 }, { 62, 4 }, { 63, 4 }, { 64, 4 },
    { 65, 4 }, { 66, 4 }, { 67, 4 }, { 255, 999 },
};
WstagAnimKey *D_WSTAG740_800A6B30[2] = { D_WSTAG740_800A6ADC, D_WSTAG740_800A6AF8 };
FieldstgListedBattle D_WSTAG740_800A6B38 = { 119, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6B44 = { 119, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6B50 = { 119, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6B5C = { 168, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6B68 = { 168, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6B74 = { 168, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6B80 = { 111, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6B8C = { 111, 24, 0x60080000 };
FieldstgBattleList D_WSTAG740_800A6B98 = {
    4,
    { &D_WSTAG740_800A6B38, &D_WSTAG740_800A6B44, &D_WSTAG740_800A6B50, &D_WSTAG740_800A6B5C, &D_WSTAG740_800A6B68,
        &D_WSTAG740_800A6B74, &D_WSTAG740_800A6B80, &D_WSTAG740_800A6B8C },
};
FieldstgListedBattle D_WSTAG740_800A6BBC = { 0, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6BC8 = { 0, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6BD4 = { 0, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6BE0 = { 0, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6BEC = { 0, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6BF8 = { 0, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6C04 = { 0, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6C10 = { 0, 24, 0x60080000 };
FieldstgBattleList D_WSTAG740_800A6C1C = {
    0,
    { &D_WSTAG740_800A6BBC, &D_WSTAG740_800A6BC8, &D_WSTAG740_800A6BD4, &D_WSTAG740_800A6BE0, &D_WSTAG740_800A6BEC,
        &D_WSTAG740_800A6BF8, &D_WSTAG740_800A6C04, &D_WSTAG740_800A6C10 },
};
FieldstgListedBattle D_WSTAG740_800A6C40 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG740_800A6C4C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG740_800A6C58 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG740_800A6C64 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG740_800A6C70 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG740_800A6C7C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG740_800A6C88 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG740_800A6C94 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG740_800A6CA0 = {
    0,
    { &D_WSTAG740_800A6C40, &D_WSTAG740_800A6C4C, &D_WSTAG740_800A6C58, &D_WSTAG740_800A6C64, &D_WSTAG740_800A6C70,
        &D_WSTAG740_800A6C7C, &D_WSTAG740_800A6C88, &D_WSTAG740_800A6C94 },
};
FieldstgListedBattle D_WSTAG740_800A6CC4 = { 20, 24, 0x608C0000 };
FieldstgListedBattle D_WSTAG740_800A6CD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG740_800A6CDC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG740_800A6CE8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG740_800A6CF4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG740_800A6D00 = { 168, 24, 0x60080000 };
FieldstgListedBattle D_WSTAG740_800A6D0C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG740_800A6D18 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG740_800A6D24 = {
    0,
    { &D_WSTAG740_800A6CC4, &D_WSTAG740_800A6CD0, &D_WSTAG740_800A6CDC, &D_WSTAG740_800A6CE8, &D_WSTAG740_800A6CF4,
        &D_WSTAG740_800A6D00, &D_WSTAG740_800A6D0C, &D_WSTAG740_800A6D18 },
};
FieldstgBattleLists wstag740_battle_lists = {
    88, 0, 0, { &D_WSTAG740_800A6B98, &D_WSTAG740_800A6C1C, &D_WSTAG740_800A6CA0 }, &D_WSTAG740_800A6D24,
};
FieldstgVramPlace wstag740_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 384, 344, 256, 88, 368, 508 }, { 384, 256, 422, 344, 408, 88, 320, 507 },
};
FieldstgTalk D_WSTAG740_800A6DE4[2] = { { NULL, NULL, 520 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG740_800A6DFC[2] = { { NULL, NULL, 521 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG740_800A6E14[2] = { { NULL, NULL, 519 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG740_800A6E2C[2] = { { NULL, NULL, 518 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG740_800A6E44[2] = { { NULL, NULL, 518 }, { NULL, NULL, 0 } };
u16 D_WSTAG740_800A6E5C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG740_800A6E64[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG740_800A6E6C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG740_800A6E74[6] = { 0x4040, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG740_800A6E80[6] = { 0x4040, 1, 0x7019, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG740_800A6E8C = { D_WSTAG740_800A6E5C, D_WSTAG740_800A6DE4, 157, 4, 464, 648, 7 };
FieldstgPlacedActor D_WSTAG740_800A6EA0 = { D_WSTAG740_800A6E64, D_WSTAG740_800A6DFC, 201, 5, 464, 648, 7 };
FieldstgPlacedActor D_WSTAG740_800A6EB4 = { D_WSTAG740_800A6E6C, D_WSTAG740_800A6E14, 201, 5, 464, 648, 7 };
FieldstgPlacedActor D_WSTAG740_800A6EC8 = { D_WSTAG740_800A6E74, D_WSTAG740_800A6E2C, 201, 5, 512, 673, 7 };
FieldstgPlacedActor D_WSTAG740_800A6EDC = { D_WSTAG740_800A6E80, D_WSTAG740_800A6E44, 201, 5, 464, 648, 7 };
FieldstgPlacedActor *wstag740_actors[6] = {
    &D_WSTAG740_800A6E8C, &D_WSTAG740_800A6EA0, &D_WSTAG740_800A6EB4, &D_WSTAG740_800A6EC8, &D_WSTAG740_800A6EDC,
    NULL,
};
FieldstgSprite wstag740_sprites[17] = {
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
FieldstgMapEvent wstag740_map_events[21] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x26A, 0x70, 0x2EC, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x26C, 0x448, 0x288, 1, 0, 0, 0 },
    { 0x40AA, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 }, { 0x40AB, 0, 0xFFFF, 0, 8, 0x2329, 0, 0, 0, 0, 0, 0 },
    { 0x40AC, 0, 0xFFFF, 0, 8, 0x232A, 0, 0, 0, 0, 0, 0 }, { 0x40AD, 0, 0xFFFF, 0, 8, 0x232B, 0, 0, 0, 0, 0, 0 },
    { 0x40AE, 0, 0xFFFF, 0, 8, 0x232C, 0, 0, 0, 0, 0, 0 }, { 0x40AF, 0, 0xFFFF, 0, 8, 0x232D, 0, 0, 0, 0, 0, 0 },
    { 0x40B0, 0, 0xFFFF, 0, 8, 0x232E, 0, 0, 0, 0, 0, 0 }, { 0x40B1, 0, 0xFFFF, 0, 8, 0x232F, 0, 0, 0, 0, 0, 0 },
    { 0x40B2, 0, 0xFFFF, 0, 8, 0x2330, 0, 0, 0, 0, 0, 0 }, { 0x40B3, 0, 0xFFFF, 0, 8, 0x2331, 0, 0, 0, 0, 0, 0 },
    { 0x40B4, 0, 0xFFFF, 0, 8, 0x2332, 0, 0, 0, 0, 0, 0 }, { 0x40B5, 0, 0xFFFF, 0, 8, 0x2333, 0, 0, 0, 0, 0, 0 },
    { 0x40B6, 0, 0xFFFF, 0, 8, 0x2334, 0, 0, 0, 0, 0, 0 }, { 0x40B7, 0, 0xFFFF, 0, 8, 0x2335, 0, 0, 0, 0, 0, 0 },
    { 0x40B8, 0, 0xFFFF, 0, 8, 0x2336, 0, 0, 0, 0, 0, 0 }, { 0x40B9, 0, 0xFFFF, 0, 8, 0x2337, 0, 0, 0, 0, 0, 0 },
    { 0x8011, 1, 0x403F, 0, 8, 0x320, 0, 0, 0, 0, 0, 0 }, { 0xF, 0, 0xFFFF, 0, 8, 0x2338, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag740_funcs = { wstag740_setup };
FieldstgEventDef wstag740_events[20] = {
    { 800, D_WSTAG740_800A6994, 0x014A000C, NULL, wstag740_event_800_end },
    { 801, D_WSTAG740_800A6A3C, 0x014A000D, NULL, wstag740_event_801_end },
    { 9000, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9000_end },
    { 9001, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9001_end },
    { 9002, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9002_end },
    { 9003, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9003_end },
    { 9004, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9004_end },
    { 9005, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9005_end },
    { 9006, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9006_end },
    { 9007, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9007_end },
    { 9008, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9008_end },
    { 9009, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9009_end },
    { 9010, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9010_end },
    { 9011, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9011_end },
    { 9012, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9012_end },
    { 9013, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9013_end },
    { 9014, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9014_end },
    { 9015, NULL, 0, (s32 (*)(void))wstag740_event_9000_start, wstag740_event_9015_end },
    { 9016, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
