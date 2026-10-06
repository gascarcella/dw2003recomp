#include "wstag.h"

/* WSTAG311: stage 0x288 (fieldstg_stages). */

extern WstagAnimKey D_WSTAG311_800A8150[];
extern WstagAnimKey D_WSTAG311_800A8174[];
extern WstagAnimKey D_WSTAG311_800A8194[];
extern WstagAnimKey D_WSTAG311_800A81B0[];
extern WstagAnimKey D_WSTAG311_800A8418[];
extern WstagAnimKey D_WSTAG311_800A8430[];
extern WstagFuncs wstag311_funcs;
void wstag311_anim4_update();
extern WstagAnimKey D_WSTAG311_800A81D0[];
extern WstagAnimKey D_WSTAG311_800A8204[];
extern WstagAnimKey D_WSTAG311_800A8238[];
extern WstagAnimKey D_WSTAG311_800A8244[];
extern WstagAnimKey D_WSTAG311_800A8280[];
extern WstagAnimKey D_WSTAG311_800A82B4[];
extern WstagAnimKey D_WSTAG311_800A82E8[];
extern WstagAnimKey D_WSTAG311_800A831C[];
extern WstagAnimKey D_WSTAG311_800A8350[];
extern WstagAnimKey D_WSTAG311_800A835C[];
extern WstagAnimKey D_WSTAG311_800A838C[];
extern WstagAnimKey D_WSTAG311_800A83C0[];
extern WstagSpawn D_WSTAG311_800A83F4[];
extern FieldstgBattleLists wstag311_battle_lists;
extern FieldstgVramPlace wstag311_vram_places[];
extern FieldstgPlacedActor *wstag311_actors[];
extern FieldstgSprite wstag311_sprites[];
extern FieldstgMapEvent wstag311_map_events[];
extern FieldstgEventDef wstag311_events[];
void wstag311_gate_update(WstagGateObject *obj);
void wstag311_gate2_update(WstagGateObject *obj);
WstagTwoSpriteObject *wstag311_two_sprite_create(s32 x, s32 y, s32 frame);
void wstag311_two_sprite_update(WstagTwoSpriteObject *obj);

/* The data of the stage object (wstag311_update). */
typedef struct Wstag311Data {
    /* 0x00 */ Object *anims;
    /* 0x04 */ WstagGateObject *gate;
    /* 0x08 */ WstagGateObject *gate2;
    /* 0x0C */ WstagTwoSpriteObject *objs[3];
    /* 0x18 */ FieldstgEvent *event;
} Wstag311Data; /* size 0x1C */
void wstag311_update();
void wstag311_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *layer, s32 which);

s32 wstag311_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
        wstag311_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag311_anim4_update(WstagAnim4Object *obj) {
    FieldstgSprite *sprite;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG311_800A8150[0].time;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG311_800A8174[0].time;
        obj->anims[2].key = 0;
        obj->anims[2].time = D_WSTAG311_800A8194[0].time;
        obj->anims[3].key = 0;
        obj->anims[3].time = D_WSTAG311_800A81B0[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            switch (sprite->type) {
            case 1:
                sprite->sprite = wstag311_anim_loop(&obj->anims[0], D_WSTAG311_800A8150, 0);
                break;
            case 2:
                sprite->sprite = wstag311_anim_loop(&obj->anims[1], D_WSTAG311_800A8174, 0);
                break;
            case 3:
                sprite->sprite = wstag311_anim_loop(&obj->anims[2], D_WSTAG311_800A8194, 0);
                break;
            case 4:
                sprite->sprite = wstag311_anim_loop(&obj->anims[3], D_WSTAG311_800A81B0, 0);
                break;
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag311_anim4_create(void) {
    return object_new(wstag311_anim4_update, sizeof(WstagAnim4Object), 0);
}

s32 wstag311_gate_anim_advance(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 once, s32 depth) {
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
        if (once) {
            if (key->frame == 0xFF) {
                return 0xFF;
            }
        } else if (key->frame == 0xFF) {
            key = keys;
            sa->anim.key = 0;
            sa->anim.time += key->time;
        }
        wstag311_gate_anim_advance(sa, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag311_gate_update(WstagGateObject *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    s32 frame;
    s32 done;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            if (list->type >= 5 && list->type <= 10) {
                switch (list->type) {
                case 5:
                    obj->sprites[0].sprite = list;
                    obj->sprites[0].anim.key = 0;
                    obj->sprites[0].anim.time = D_WSTAG311_800A81D0[0].time;
                    break;
                case 6:
                    obj->sprites[1].sprite = list;
                    obj->sprites[1].anim.key = 0;
                    obj->sprites[1].anim.time = D_WSTAG311_800A8204[0].time;
                    break;
                case 7:
                    obj->sprites[2].sprite = list;
                    obj->sprites[2].anim.key = 0;
                    obj->sprites[2].anim.time = D_WSTAG311_800A8238[0].time;
                    break;
                case 8:
                    obj->sprites[3].sprite = list;
                    obj->sprites[3].anim.key = 0;
                    obj->sprites[3].anim.time = D_WSTAG311_800A8244[0].time;
                    break;
                case 9:
                    obj->sprites[4].sprite = list;
                    obj->sprites[4].anim.key = 0;
                    obj->sprites[4].anim.time = 0;
                    break;
                case 10:
                    obj->sprites[5].sprite = list;
                    obj->sprites[5].anim.key = 0;
                    obj->sprites[5].anim.time = 0;
                    break;
                }
            }
        }
        obj->mode = 0;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        done = 0;
        for (i = 0; i < 6; i++) {
            sprite = obj->sprites[i].sprite;
            switch (obj->mode) {
            case 0:
                sprite->shown = 0;
                break;
            case 1:
                switch (i) {
                case 0:
                    frame = wstag311_gate_anim_advance(&obj->sprites[0], D_WSTAG311_800A81D0, 1, 0);
                    if (frame != 0xFF) {
                        sprite->sprite = frame;
                        sprite->shown = 1;
                    } else {
                        done++;
                        sprite->sprite = 0;
                        sprite->shown = 0;
                    }
                    break;
                case 1:
                    frame = wstag311_gate_anim_advance(&obj->sprites[1], D_WSTAG311_800A8204, 1, 0);
                    if (frame != 0xFF) {
                        sprite->sprite = frame;
                        sprite->shown = 1;
                    } else {
                        done++;
                        sprite->sprite = 0;
                        sprite->shown = 0;
                    }
                    break;
                case 2:
                case 3:
                case 4:
                case 5:
                    sprite->shown = 0;
                    break;
                }
                break;
            case 2:
                switch (i) {
                case 0:
                case 1:
                    sprite->shown = 0;
                    break;
                case 2:
                    sprite->sprite = 0x28;
                    sprite->frame = wstag311_gate_anim_advance(&obj->sprites[i], D_WSTAG311_800A8238, 0, 0);
                    sprite->shown = 1;
                    break;
                case 3:
                    sprite->sprite = 2;
                    sprite->frame = wstag311_gate_anim_advance(&obj->sprites[i], D_WSTAG311_800A8244, 0, 0);
                    sprite->shown = 1;
                    break;
                case 4:
                    sprite->shown = 1;
                    sprite->sprite = 3;
                    sprite->frame = 0;
                    break;
                case 5:
                    sprite->shown = 1;
                    sprite->sprite = 4;
                    sprite->frame = 0;
                    break;
                }
                break;
            case 3:
                switch (i) {
                case 0:
                    frame = wstag311_gate_anim_advance(&obj->sprites[0], D_WSTAG311_800A82B4, 1, 0);
                    if (frame != 0xFF) {
                        sprite->sprite = frame;
                        sprite->shown = 1;
                    } else {
                        done++;
                        sprite->sprite = 0;
                        sprite->shown = 0;
                    }
                    break;
                case 1:
                    frame = wstag311_gate_anim_advance(&obj->sprites[1], D_WSTAG311_800A82B4, 1, 0);
                    if (frame != 0xFF) {
                        sprite->sprite = frame;
                        sprite->shown = 1;
                    } else {
                        done++;
                        sprite->sprite = 0;
                        sprite->shown = 0;
                    }
                    break;
                case 2:
                case 3:
                case 4:
                case 5:
                    sprite->shown = 0;
                    break;
                }
                break;
            }
        }
        if (done >= 2) {
            switch (obj->mode) {
            case 1:
                obj->mode = 2;
                break;
            case 3:
                obj->mode = 0;
                break;
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag311_gate_message(WstagGateObject *obj, s32 open) {
    if (obj != NULL) {
        switch (open) {
        case 0:
            obj->sprites[0].anim.key = 0;
            obj->sprites[0].anim.time = D_WSTAG311_800A81D0[0].time;
            obj->sprites[1].anim.key = 0;
            obj->sprites[1].anim.time = D_WSTAG311_800A8204[0].time;
            obj->mode = 1;
            break;
        case 1:
            obj->sprites[0].anim.key = 0;
            obj->sprites[0].anim.time = D_WSTAG311_800A8280[0].time;
            obj->sprites[1].anim.key = 0;
            obj->sprites[1].anim.time = D_WSTAG311_800A82B4[0].time;
            obj->mode = 3;
            break;
        }
    }
}

WstagGateObject *wstag311_gate_create(s32 arg0) {
    return object_create(wstag311_gate_update, sizeof(WstagGateObject), 0, arg0);
}

WstagGateObject *wstag311_gate_new(void) {
    return object_new(wstag311_gate_update, sizeof(WstagGateObject), 0);
}

s32 wstag311_gate2_anim_advance(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 once, s32 depth) {
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
        if (once) {
            if (key->frame == 0xFF) {
                return 0xFF;
            }
        } else if (key->frame == 0xFF) {
            key = keys;
            sa->anim.key = 0;
            sa->anim.time += key->time;
        }
        wstag311_gate2_anim_advance(sa, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag311_gate2_update(WstagGateObject *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    s32 frame;
    s32 done;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            if (list->type >= 0xB && list->type <= 0x10) {
                switch (list->type) {
                case 0xB:
                    obj->sprites[0].sprite = list;
                    obj->sprites[0].anim.key = 0;
                    obj->sprites[0].anim.time = D_WSTAG311_800A82E8[0].time;
                    break;
                case 0xC:
                    obj->sprites[1].sprite = list;
                    obj->sprites[1].anim.key = 0;
                    obj->sprites[1].anim.time = D_WSTAG311_800A831C[0].time;
                    break;
                case 0xD:
                    obj->sprites[2].sprite = list;
                    obj->sprites[2].anim.key = 0;
                    obj->sprites[2].anim.time = D_WSTAG311_800A8350[0].time;
                    break;
                case 0xE:
                    obj->sprites[3].sprite = list;
                    obj->sprites[3].anim.key = 0;
                    obj->sprites[3].anim.time = D_WSTAG311_800A835C[0].time;
                    break;
                case 0xF:
                    obj->sprites[4].sprite = list;
                    obj->sprites[4].anim.key = 0;
                    obj->sprites[4].anim.time = 0;
                    break;
                case 0x10:
                    obj->sprites[5].sprite = list;
                    obj->sprites[5].anim.key = 0;
                    obj->sprites[5].anim.time = 0;
                    break;
                }
            }
        }
        obj->mode = 0;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        done = 0;
        for (i = 0; i < 6; i++) {
            sprite = obj->sprites[i].sprite;
            switch (obj->mode) {
            case 0:
                sprite->shown = 0;
                break;
            case 1:
                switch (i) {
                case 0:
                    frame = wstag311_gate2_anim_advance(&obj->sprites[0], D_WSTAG311_800A82E8, 1, 0);
                    if (frame != 0xFF) {
                        sprite->sprite = frame;
                        sprite->shown = 1;
                    } else {
                        done++;
                        sprite->sprite = 0;
                        sprite->shown = 0;
                    }
                    break;
                case 1:
                    frame = wstag311_gate2_anim_advance(&obj->sprites[1], D_WSTAG311_800A831C, 1, 0);
                    if (frame != 0xFF) {
                        sprite->sprite = frame;
                        sprite->shown = 1;
                    } else {
                        done++;
                        sprite->sprite = 0;
                        sprite->shown = 0;
                    }
                    break;
                case 2:
                case 3:
                case 4:
                case 5:
                    sprite->shown = 0;
                    break;
                }
                break;
            case 2:
                switch (i) {
                case 0:
                case 1:
                    sprite->shown = 0;
                    break;
                case 2:
                    sprite->sprite = 0x29;
                    sprite->frame = wstag311_gate2_anim_advance(&obj->sprites[i], D_WSTAG311_800A8350, 0, 0);
                    sprite->shown = 1;
                    break;
                case 3:
                    sprite->sprite = 0x11;
                    sprite->frame = wstag311_gate2_anim_advance(&obj->sprites[i], D_WSTAG311_800A835C, 0, 0);
                    sprite->shown = 1;
                    break;
                case 4:
                    sprite->shown = 1;
                    sprite->sprite = 0x12;
                    sprite->frame = 0;
                    break;
                case 5:
                    sprite->shown = 1;
                    sprite->sprite = 0x13;
                    sprite->frame = 0;
                    break;
                }
                break;
            case 3:
                switch (i) {
                case 0:
                    frame = wstag311_gate2_anim_advance(&obj->sprites[0], D_WSTAG311_800A83C0, 1, 0);
                    if (frame != 0xFF) {
                        sprite->sprite = frame;
                        sprite->shown = 1;
                    } else {
                        done++;
                        sprite->sprite = 0;
                        sprite->shown = 0;
                    }
                    break;
                case 1:
                    frame = wstag311_gate2_anim_advance(&obj->sprites[1], D_WSTAG311_800A83C0, 1, 0);
                    if (frame != 0xFF) {
                        sprite->sprite = frame;
                        sprite->shown = 1;
                    } else {
                        done++;
                        sprite->sprite = 0;
                        sprite->shown = 0;
                    }
                    break;
                case 2:
                case 3:
                case 4:
                case 5:
                    sprite->shown = 0;
                    break;
                }
                break;
            }
        }
        if (done >= 2) {
            switch (obj->mode) {
            case 1:
                obj->mode = 2;
                break;
            case 3:
                obj->mode = 0;
                break;
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag311_gate2_message(WstagGateObject *obj, s32 open) {
    if (obj != NULL) {
        switch (open) {
        case 0:
            obj->sprites[0].anim.key = 0;
            obj->sprites[0].anim.time = D_WSTAG311_800A82E8[0].time;
            obj->sprites[1].anim.key = 0;
            obj->sprites[1].anim.time = D_WSTAG311_800A831C[0].time;
            obj->mode = 1;
            break;
        case 1:
            obj->sprites[0].anim.key = 0;
            obj->sprites[0].anim.time = D_WSTAG311_800A838C[0].time;
            obj->sprites[1].anim.key = 0;
            obj->sprites[1].anim.time = D_WSTAG311_800A83C0[0].time;
            obj->mode = 3;
            break;
        }
    }
}

WstagGateObject *wstag311_gate2_create(s32 arg0) {
    return object_create(wstag311_gate2_update, sizeof(WstagGateObject), 0, arg0);
}

WstagGateObject *wstag311_gate2_new(void) {
    return object_new(wstag311_gate2_update, sizeof(WstagGateObject), 0);
}

void wstag311_update(WstagObject *obj, Wstag311Data *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->anims = wstag311_anim4_create();
        for (i = 0; i < 3; i++) {
            if (D_WSTAG311_800A83F4[i].condition == 0 || (D_WSTAG311_800A83F4[i].condition == 2 && gamestate_data.progress >= 0x28)) {
                data->objs[i] = wstag311_two_sprite_create(D_WSTAG311_800A83F4[i].x, D_WSTAG311_800A83F4[i].y,
                                                       D_WSTAG311_800A83F4[i].frame);
            }
        }
        switch (gamestate_data.progress) {
        case 0x25:
            if (gamestate_flags.get_flag(0x405E, 0)) {
                data->event = fieldstg_event_start(0x3A3);
            } else if (gamestate_flags.get_flag(0x4067, 0)) {
                data->event = fieldstg_event_start(0x3A4);
            } else if (gamestate_flags.get_flag(0x405F, 0)) {
                data->event = fieldstg_event_start(0x3A5);
            } else if (gamestate_flags.get_flag(0x4060, 0)) {
                data->event = fieldstg_event_start(0x3A6);
            }
            break;
        case 0x27:
            if (gamestate_flags.get_flag(0x406C, 0)) {
                data->event = fieldstg_event_start(0x3D5);
            } else {
                data->event = fieldstg_event_start(0x3D6);
            }
            break;
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag311_start(void *arg0) {
    WstagObject *obj = object_new(wstag311_update, sizeof(WstagObject), sizeof(Wstag311Data));

    obj->manager = arg0;
    wstag311_funcs.setup();
    return obj;
}

s32 wstag311_anim_advance(WstagAnim *anim, WstagAnimKey *keys, s32 once, s32 depth) {
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
        if (once) {
            if (key->frame == 0xFF) {
                return 0xFF;
            }
        } else if (key->frame == 0xFF) {
            key = keys;
            anim->key = 0;
            anim->time += key->time;
        }
        wstag311_anim_advance(anim, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag311_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *arg1, s32 which) {
    Sprite spr;
    GfxLayer *layer = arg1;
    WstagFrame *frame = &obj->sprites[which];
    s32 y = obj->y;
    s32 x = obj->x;
    s32 depth;

    if (which == 1) {
        y -= 0x20;
        depth = 4;
    } else {
        depth = 6;
    }
    sprite_init(&spr);
    spr.set_vram_pos(0x140, 0x100);
    spr.set_layer(layer, depth);
    spr.set_palette(frame->palette);
    spr.draw(cdload_module.get_subfile_by_id(fieldstg_stage.sprite_file), frame->frame, x, y);
}

s32 wstag311_is_on_screen(s32 x, s32 y, s32 w, s32 h) {
    GfxRect view;
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);

    layer->get_view_rect(layer, &view);
    if (x + w < view.x) {
        return 0;
    }
    if (view.x + view.w < x) {
        return 0;
    }
    if (y + h < view.y) {
        return 0;
    }
    return view.y + view.h >= y;
}

void wstag311_two_sprite_update(WstagTwoSpriteObject *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->frame_anim.key = 0;
        obj->base.key1 = obj->x;
        obj->base.key2 = obj->y;
        obj->frame_anim.time = D_WSTAG311_800A8430[0].time;
        obj->palette_anim.key = 0;
        obj->palette_anim.time = D_WSTAG311_800A8418[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag311_anim_advance(&obj->palette_anim, D_WSTAG311_800A8418, 0, 0);
        if (obj->sprites[0].frame != 0 && wstag311_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag311_two_sprite_draw, obj, obj->y, 0);
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->frame_anim.key = 0;
            obj->frame_anim.time = D_WSTAG311_800A8430[0].time;
            obj->palette_anim.key = 0;
            obj->palette_anim.time = D_WSTAG311_800A8418[0].time;
            obj->base.set_step(obj, 1);
        }
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag311_anim_advance(&obj->palette_anim, D_WSTAG311_800A8418, 0, 0);
        done = 0;
        frame = wstag311_anim_advance(&obj->frame_anim, D_WSTAG311_800A8430, 1, 0);
        switch (frame) {
        case 0xFF:
            obj->sprites[1].frame = 0;
            done = 1;
            break;
        case 0x12C:
            obj->sprites[1].frame = 0;
            break;
        default:
            obj->sprites[1].frame = obj->frame + frame;
            break;
        }
        if (done) {
            obj->base.set_state(obj, OBJECT_STATE_RUN);
        }
        if (obj->sprites[0].frame != 0 && wstag311_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag311_two_sprite_draw, obj, obj->y, 0);
        }
        if (obj->sprites[1].frame != 0 && wstag311_is_on_screen(obj->x, obj->y - 0x20, 0x20, 0x40)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag311_two_sprite_draw, obj, obj->y + 0x12, 1);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

WstagTwoSpriteObject *wstag311_two_sprite_create(s32 x, s32 y, s32 frame) {
    WstagTwoSpriteObject *obj = object_create(wstag311_two_sprite_update, sizeof(WstagTwoSpriteObject), 0, 0x17);

    obj->x = x;
    obj->y = y;
    obj->frame = frame;
    return obj;
}

WstagTwoSpriteObject *wstag311_two_sprite_create_fixed(s32 arg0) {
    WstagTwoSpriteObject *obj = object_create(wstag311_two_sprite_update, sizeof(WstagTwoSpriteObject), 0, arg0);

    obj->x = 0x10F;
    obj->y = 0x19B;
    sound_module.play(0x4001D);
    obj->frame = 0x2A;
    obj->base.set_state(obj, OBJECT_STATE_DONE);
    return obj;
}

void wstag311_event_931_end(void) {
    gamestate_flags.set_flag(0x405E, 1);
    gamestate_flags.set_flag(0xC33, 1);
    gamestate_flags.set_flag(0x7401, 1);
}

void wstag311_event_932_end(void) {
    gamestate_flags.set_flag(0x4067, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag311_event_933_end(void) {
    gamestate_flags.set_flag(0x405F, 1);
}

void wstag311_event_934_end(void) {
    gamestate_flags.set_flag(0x4060, 1);
}

void wstag311_event_981_end(void) {
    gamestate_flags.set_flag(0x406C, 1);
}

void wstag311_event_982_end(void) {
    gamestate_data.progress = 0x28;
}

void wstag311_setup(void) {
    fieldstg_stage.background_file = 0x53C;
    fieldstg_stage.sprite_file = 0x053D0000;
    fieldstg_stage.sprites = wstag311_sprites;
    fieldstg_stage.map_events = wstag311_map_events;
    fieldstg_stage.mask_file = 0x53B;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13500, 0x19D00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag311_vram_places;
    fieldstg_stage.music = 0x2A;
    fieldstg_stage.sound = 0x60A80000;
    fieldstg_stage.actors = wstag311_actors;
    fieldstg_stage.battle_lists = &wstag311_battle_lists;
    fieldstg_stage.events = wstag311_events;
    fieldstg_attr.set_file(0, 0x053D0001);
    fieldstg_attr.set_file(7, 0x053D0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag311_setup(void);

s16 D_WSTAG311_800A76A4[118] = {
    FIELDSTG_EVENT_PLACE(2, 88, 524),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 208, 464, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(210, 1, 7),
    FIELDSTG_EVENT_ANIM(211, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 212, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 211, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 210, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG311_800A7790[161] = {
    FIELDSTG_EVENT_PLACE(2, 208, 464),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(258, 464, 288),
    FIELDSTG_EVENT_ANIM(258, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 8, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 336, 400, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_ANIM(0x32D, 873, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 368, 352),
    FIELDSTG_EVENT_DIALOG(0, 1, 258, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WALK(258, 432, 304, 1),
    FIELDSTG_EVENT_WAIT_WALK(258),
    FIELDSTG_EVENT_WALK(258, 368, 384, 1),
    FIELDSTG_EVENT_WAIT_WALK(258),
    FIELDSTG_EVENT_ANIM(258, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 258, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 258, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 258, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG311_800A78D4[584] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 336, 416),
    FIELDSTG_EVENT_PLACE(2, 336, 400),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(258, 368, 384),
    FIELDSTG_EVENT_ANIM(258, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 258, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_PLACE(103, 160, 488),
    FIELDSTG_EVENT_ANIM(103, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_PLACE(12, 160, 488),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_WALK(103, 192, 472, 5),
    FIELDSTG_EVENT_WAIT_WALK(103),
    FIELDSTG_EVENT_WALK(12, 192, 472, 5),
    FIELDSTG_EVENT_PLACE(101, 160, 488),
    FIELDSTG_EVENT_ANIM(101, 1, 5),
    FIELDSTG_EVENT_WALK(103, 224, 456, 5),
    FIELDSTG_EVENT_WAIT_WALK(103),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 103),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 103),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 103, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 15, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WALK(12, 272, 432, 5),
    FIELDSTG_EVENT_WALK(101, 240, 448, 5),
    FIELDSTG_EVENT_WALK(103, 304, 416, 5),
    FIELDSTG_EVENT_WAIT_WALK(103),
    FIELDSTG_EVENT_WALK(12, 304, 416, 5),
    FIELDSTG_EVENT_WALK(101, 272, 432, 5),
    FIELDSTG_EVENT_WALK(103, 272, 400, 3),
    FIELDSTG_EVENT_WAIT_WALK(103),
    FIELDSTG_EVENT_WALK(12, 336, 432, 7),
    FIELDSTG_EVENT_WALK(101, 304, 416, 5),
    FIELDSTG_EVENT_ANIM(103, 1, 5),
    FIELDSTG_EVENT_WAIT_WALK(101),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_ANIM(101, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 17, 101, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 258, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 101, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 258, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_ANIM(0x324, 805, 101),
    FIELDSTG_EVENT_ANIM(0x325, 805, 12),
    FIELDSTG_EVENT_ANIM(0x326, 805, 103),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_ANIM(0x324, 806, 101),
    FIELDSTG_EVENT_ANIM(0x325, 806, 12),
    FIELDSTG_EVENT_ANIM(0x326, 806, 103),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 101, 1),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(12, 1, 3),
    FIELDSTG_EVENT_ANIM(103, 1, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(1, 18, 12, 3),
    FIELDSTG_EVENT_DIALOG(0, 9, 103, 0),
    FIELDSTG_EVENT_ANIM(12, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WALK(2, 304, 388, 3),
    FIELDSTG_EVENT_ANIM(12, 1, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WALK(258, 336, 400, 1),
    FIELDSTG_EVENT_WAIT_WALK(258),
    FIELDSTG_EVENT_ANIM(258, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_WALK(101, 64, 538, 1),
    FIELDSTG_EVENT_ANIM(103, 1, 1),
    FIELDSTG_EVENT_WALK(258, 64, 538, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_ANIM(0x323, 805, 103),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_ANIM(0x323, 806, 103),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 12, 103, 2),
    FIELDSTG_EVENT_ANIM(103, 1, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_PLACE(101, 0, 0),
    FIELDSTG_EVENT_ANIM(101, 1, 0),
    FIELDSTG_EVENT_PLACE(258, 0, 0),
    FIELDSTG_EVENT_ANIM(258, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 12, 2),
    FIELDSTG_EVENT_ANIM(12, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WALK(103, 304, 416, 7),
    FIELDSTG_EVENT_WAIT_WALK(103),
    FIELDSTG_EVENT_WALK(12, 304, 416, 3),
    FIELDSTG_EVENT_WALK(103, 336, 400, 5),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WALK(12, 336, 400, 5),
    FIELDSTG_EVENT_WALK(103, 368, 384, 5),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_WALK(2, 336, 400, 5),
    FIELDSTG_EVENT_WALK(12, 368, 384, 5),
    FIELDSTG_EVENT_WALK(103, 432, 304, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_DIALOG(0, 10, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 11, 12, 0),
    FIELDSTG_EVENT_ANIM(12, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_ANIM(103, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 12),
    FIELDSTG_EVENT_DIALOG(0, 16, 103, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_ANIM(0x324, 805, 12),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_ANIM(0x324, 806, 12),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 13, 12, 0),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(12, 432, 304, 5),
    FIELDSTG_EVENT_WALK(103, 536, 252, 5),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_DIALOG(0, 14, 2, 2),
    FIELDSTG_EVENT_WALK(12, 538, 252, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0xE07, 336, 400, 5),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG311_800A7D64[274] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 320, 392),
    FIELDSTG_EVENT_PLACE(2, 336, 400),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(12, 494, 276),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_PLACE(103, 494, 276),
    FIELDSTG_EVENT_ANIM(103, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_WALK(12, 432, 304, 1),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 12, 1),
    FIELDSTG_EVENT_ANIM(12, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(12, 368, 384, 1),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_DIALOG(0, 8, 12, 0),
    FIELDSTG_EVENT_ANIM(12, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WALK(103, 432, 304, 1),
    FIELDSTG_EVENT_WAIT_WALK(103),
    FIELDSTG_EVENT_WALK(2, 320, 392, 5),
    FIELDSTG_EVENT_WALK(12, 336, 400, 1),
    FIELDSTG_EVENT_WALK(103, 368, 384, 1),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_WALK(12, 352, 408, 5),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_DIALOG(0, 3, 103, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_ANIM(12, 1, 3),
    FIELDSTG_EVENT_WALK(103, 336, 400, 1),
    FIELDSTG_EVENT_WAIT_WALK(103),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_WALK(103, 112, 512, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_DIALOG(0, 4, 12, 1),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_ANIM(12, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(12, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 12, 1),
    FIELDSTG_EVENT_ANIM(12, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(12, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 336, 400, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 304, 416, 1),
    FIELDSTG_EVENT_WALK(12, 336, 400, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 128, 504, 1),
    FIELDSTG_EVENT_WALK(12, 128, 504, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0x276, 88, 460, 5),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG311_800A7F88[127] = {
    FIELDSTG_EVENT_PLACE(1, 88, 524),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 234, 451, 5),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 58, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 1),
    FIELDSTG_EVENT_ANIM(1, 1, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_ANIM(0x346, 821, 838),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 1, 3),
    FIELDSTG_EVENT_ANIM(1, 7, 5),
    FIELDSTG_EVENT_PLACE(213, 290, 423),
    FIELDSTG_EVENT_ANIM(213, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 2, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 1),
    FIELDSTG_EVENT_ANIM(1, 12, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0xE08, 234, 451, 5),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG311_800A8088[100] = {
    FIELDSTG_EVENT_PLACE(1, 234, 451),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_PLACE(213, 290, 423),
    FIELDSTG_EVENT_ANIM(213, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 1, 1),
    FIELDSTG_EVENT_ANIM(1, 12, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x346, 821, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_PLACE(213, 0, 0),
    FIELDSTG_EVENT_ANIM(213, 1, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 6, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x288, 234, 451, 5),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG311_800A8150[9] = {
    { 51, 8 }, { 52, 8 }, { 53, 8 }, { 54, 8 }, { 55, 8 }, { 56, 8 }, { 57, 8 }, { 75, 40 }, { 255, 0 },
};
WstagAnimKey D_WSTAG311_800A8174[8] = {
    { 58, 8 }, { 59, 8 }, { 60, 8 }, { 61, 8 }, { 62, 8 }, { 63, 8 }, { 75, 40 }, { 255, 0 },
};
WstagAnimKey D_WSTAG311_800A8194[7] = {
    { 64, 8 }, { 65, 8 }, { 66, 8 }, { 67, 8 }, { 68, 8 }, { 75, 40 }, { 255, 0 },
};
WstagAnimKey D_WSTAG311_800A81B0[8] = {
    { 69, 8 }, { 70, 8 }, { 71, 8 }, { 72, 8 }, { 73, 8 }, { 74, 8 }, { 75, 40 }, { 255, 0 },
};
WstagAnimKey D_WSTAG311_800A81D0[13] = {
    { 5, 4 }, { 6, 4 }, { 7, 4 }, { 8, 4 }, { 9, 4 }, { 10, 4 }, { 11, 4 }, { 12, 4 }, { 13, 4 }, { 14, 4 },
    { 15, 4 }, { 16, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG311_800A8204[13] = {
    { 28, 4 }, { 29, 4 }, { 30, 4 }, { 31, 4 }, { 32, 4 }, { 33, 4 }, { 34, 4 }, { 35, 4 }, { 36, 4 }, { 37, 4 },
    { 38, 4 }, { 39, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG311_800A8238[3] = { { 0, 4 }, { 1, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG311_800A8244[15] = {
    { 0, 12 }, { 1, 12 }, { 2, 12 }, { 3, 12 }, { 4, 12 }, { 5, 12 }, { 6, 12 }, { 7, 12 }, { 8, 12 }, { 9, 12 },
    { 10, 12 }, { 11, 12 }, { 12, 12 }, { 13, 12 }, { 255, 0 },
};
WstagAnimKey D_WSTAG311_800A8280[13] = {
    { 16, 4 }, { 15, 4 }, { 14, 4 }, { 13, 4 }, { 12, 4 }, { 11, 4 }, { 10, 4 }, { 9, 4 }, { 8, 4 }, { 7, 4 },
    { 6, 4 }, { 5, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG311_800A82B4[13] = {
    { 39, 4 }, { 38, 4 }, { 37, 4 }, { 36, 4 }, { 35, 4 }, { 34, 4 }, { 33, 4 }, { 32, 4 }, { 31, 4 }, { 30, 4 },
    { 29, 4 }, { 28, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG311_800A82E8[13] = {
    { 76, 4 }, { 77, 4 }, { 78, 4 }, { 79, 4 }, { 80, 4 }, { 81, 4 }, { 82, 4 }, { 83, 4 }, { 84, 4 }, { 85, 4 },
    { 86, 4 }, { 87, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG311_800A831C[13] = {
    { 88, 4 }, { 89, 4 }, { 90, 4 }, { 91, 4 }, { 92, 4 }, { 93, 4 }, { 94, 4 }, { 95, 4 }, { 96, 4 }, { 97, 4 },
    { 98, 4 }, { 99, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG311_800A8350[3] = { { 0, 4 }, { 1, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG311_800A835C[12] = {
    { 0, 12 }, { 1, 12 }, { 2, 12 }, { 3, 12 }, { 4, 12 }, { 5, 12 }, { 6, 12 }, { 7, 12 }, { 8, 12 }, { 9, 12 },
    { 10, 12 }, { 255, 0 },
};
WstagAnimKey D_WSTAG311_800A838C[13] = {
    { 87, 4 }, { 86, 4 }, { 85, 4 }, { 84, 4 }, { 83, 4 }, { 82, 4 }, { 81, 4 }, { 80, 4 }, { 79, 4 }, { 78, 4 },
    { 77, 4 }, { 76, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG311_800A83C0[13] = {
    { 99, 4 }, { 98, 4 }, { 97, 4 }, { 96, 4 }, { 95, 4 }, { 94, 4 }, { 93, 4 }, { 92, 4 }, { 91, 4 }, { 90, 4 },
    { 89, 4 }, { 88, 4 }, { 255, 999 },
};
WstagSpawn D_WSTAG311_800A83F4[3] = { { 20, 0, 492, 188 }, { 20, 0, 620, 252 }, { 42, 2, 271, 411 } };
WstagAnimKey D_WSTAG311_800A8418[6] = { { 0, 6 }, { 1, 6 }, { 2, 68 }, { 1, 4 }, { 0, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG311_800A8430[22] = {
    { 300, 18 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 },
    { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 },
    { 255, 999 },
};
FieldstgListedBattle D_WSTAG311_800A8488 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A8494 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A84A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A84AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A84B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A84C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A84D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A84DC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG311_800A84E8 = {
    0,
    { &D_WSTAG311_800A8488, &D_WSTAG311_800A8494, &D_WSTAG311_800A84A0, &D_WSTAG311_800A84AC, &D_WSTAG311_800A84B8,
        &D_WSTAG311_800A84C4, &D_WSTAG311_800A84D0, &D_WSTAG311_800A84DC },
};
FieldstgListedBattle D_WSTAG311_800A850C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A8518 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A8524 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A8530 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A853C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A8548 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A8554 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A8560 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG311_800A856C = {
    0,
    { &D_WSTAG311_800A850C, &D_WSTAG311_800A8518, &D_WSTAG311_800A8524, &D_WSTAG311_800A8530, &D_WSTAG311_800A853C,
        &D_WSTAG311_800A8548, &D_WSTAG311_800A8554, &D_WSTAG311_800A8560 },
};
FieldstgListedBattle D_WSTAG311_800A8590 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A859C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A85A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A85B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A85C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A85CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A85D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A85E4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG311_800A85F0 = {
    0,
    { &D_WSTAG311_800A8590, &D_WSTAG311_800A859C, &D_WSTAG311_800A85A8, &D_WSTAG311_800A85B4, &D_WSTAG311_800A85C0,
        &D_WSTAG311_800A85CC, &D_WSTAG311_800A85D8, &D_WSTAG311_800A85E4 },
};
FieldstgListedBattle D_WSTAG311_800A8614 = { 32, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG311_800A8620 = { 198, 18, 0x60080000 };
FieldstgListedBattle D_WSTAG311_800A862C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A8638 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A8644 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A8650 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A865C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG311_800A8668 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG311_800A8674 = {
    0,
    { &D_WSTAG311_800A8614, &D_WSTAG311_800A8620, &D_WSTAG311_800A862C, &D_WSTAG311_800A8638, &D_WSTAG311_800A8644,
        &D_WSTAG311_800A8650, &D_WSTAG311_800A865C, &D_WSTAG311_800A8668 },
};
FieldstgBattleLists wstag311_battle_lists = {
    148, 0, 0, { &D_WSTAG311_800A84E8, &D_WSTAG311_800A856C, &D_WSTAG311_800A85F0 }, &D_WSTAG311_800A8674,
};
FieldstgVramPlace wstag311_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 448, 256, 466, 472, 584, 216, 352, 494 }, { 448, 256, 474, 472, 616, 216, 368, 494 },
    { 320, 256, 374, 256, 216, 0, 320, 493 }, { 320, 256, 374, 296, 216, 40, 336, 493 },
    { 448, 256, 478, 424, 632, 168, 352, 493 }, { 448, 256, 488, 424, 672, 168, 368, 493 },
    { 448, 256, 498, 424, 712, 168, 320, 492 }, { 384, 256, 412, 408, 368, 152, 336, 492 },
    { 384, 256, 422, 464, 408, 208, 352, 492 },
};
u16 D_WSTAG311_800A87A4[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_WSTAG311_800A87AC[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG311_800A87B4[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG311_800A87BC[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG311_800A87C4[6] = { 0x6025, 1, 0xC33, 0, 0xFFFF, 0 };
u16 D_WSTAG311_800A87D0[6] = { 0x6025, 1, 0xC33, 0, 0xFFFF, 0 };
u16 D_WSTAG311_800A87DC[6] = { 0x6025, 1, 0xC33, 0, 0xFFFF, 0 };
u16 D_WSTAG311_800A87E8[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_WSTAG311_800A87F0[6] = { 0x6025, 1, 0x405F, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG311_800A87FC = { D_WSTAG311_800A87A4, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG311_800A8810 = { D_WSTAG311_800A87AC, NULL, 12, 5, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG311_800A8824 = { D_WSTAG311_800A87B4, NULL, 101, 6, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG311_800A8838 = { D_WSTAG311_800A87BC, NULL, 103, 7, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG311_800A884C = { D_WSTAG311_800A87C4, NULL, 210, 8, 161, 441, 1 };
FieldstgPlacedActor D_WSTAG311_800A8860 = { D_WSTAG311_800A87D0, NULL, 211, 9, 255, 489, 1 };
FieldstgPlacedActor D_WSTAG311_800A8874 = { D_WSTAG311_800A87DC, NULL, 212, 10, 235, 452, 1 };
FieldstgPlacedActor D_WSTAG311_800A8888 = { D_WSTAG311_800A87E8, NULL, 213, 11, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG311_800A889C = { D_WSTAG311_800A87F0, NULL, 258, 12, 464, 288, 1 };
FieldstgPlacedActor *wstag311_actors[10] = {
    &D_WSTAG311_800A87FC, &D_WSTAG311_800A8810, &D_WSTAG311_800A8824, &D_WSTAG311_800A8838, &D_WSTAG311_800A884C,
    &D_WSTAG311_800A8860, &D_WSTAG311_800A8874, &D_WSTAG311_800A8888, &D_WSTAG311_800A889C, NULL,
};
FieldstgSprite wstag311_sprites[28] = {
    { 1, 1, 0x40, 2, 0x33, 0, 0, 0, 0, 0, 584, 125, 0, 0 }, { 1, 2, 0x40, 2, 0x3A, 0, 0, 0, 0, 0, 592, 144, 0, 0 },
    { 1, 3, 0x40, 2, 0x40, 0, 0, 0, 0, 0, 609, 140, 0, 0 }, { 1, 4, 0x40, 2, 0x45, 0, 0, 0, 0, 0, 610, 167, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 4, 0, 160, 509, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 4, 0, 392, 457, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 4, 0, 448, 339, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 4, 0, 545, 307, 0, 0 },
    { 0, 6, 0x80, 2, 0x1C, 0, 0, 0, 0, 0, 550, 116, 0, 0 },
    { 0, 0xC, 0xFF, 2, 0x58, 0, 0, 0, 0, 0, 430, 103, 0, 0 }, { 0, 8, 0x80, 2, 2, 0, 0, 0, 0, 0, 550, 116, 0, 0 },
    { 0, 9, 0x80, 2, 3, 0, 0, 0, 0, 0, 550, 116, 0, 0 }, { 0, 0xA, 0x80, 2, 4, 0, 0, 0, 0, 0, 550, 116, 0, 0 },
    { 0, 5, 0x80, 2, 5, 0, 0, 0, 0, 0, 550, 116, 0, 0 }, { 0, 7, 0x80, 2, 0x28, 0, 0, 0, 0, 0, 546, 112, 0, 0 },
    { 0, 0xE, 0xFF, 2, 0x11, 0, 0, 0, 0, 0, 430, 103, 0, 0 },
    { 0, 0xF, 0xFF, 2, 0x12, 0, 0, 0, 0, 0, 430, 103, 0, 0 },
    { 0, 0x10, 0xFF, 2, 0x13, 0, 0, 0, 0, 0, 430, 103, 0, 0 },
    { 0, 0xB, 0xFF, 2, 0x4C, 0, 0, 0, 0, 0, 430, 103, 0, 0 },
    { 0, 0xD, 0xFF, 2, 0x29, 0, 0, 0, 0, 0, 426, 99, 0, 0 }, { 0, 0, 0x40, 6, 0x2A, 0, 0, 0, 0, 0, 271, 411, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 72, 464, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 178, 350, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 303, 267, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 369, 219, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 392, 328, 386, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 545, 243, 266, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag311_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x287, 0x178, 0xFC, 1, 0, 0, 0 },
    { 0x6028, 1, 0xFFFF, 0, 0xE, 0x2DD, 0xE0, 0x418, 5, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag311_funcs = { wstag311_setup };
FieldstgEventDef wstag311_events[7] = {
    { 931, D_WSTAG311_800A76A4, 0x01270010, NULL, wstag311_event_931_end },
    { 932, D_WSTAG311_800A7790, 0x01270011, NULL, wstag311_event_932_end },
    { 933, D_WSTAG311_800A78D4, 0x01270012, NULL, wstag311_event_933_end },
    { 934, D_WSTAG311_800A7D64, 0x01270013, NULL, wstag311_event_934_end },
    { 981, D_WSTAG311_800A7F88, 0x01270018, NULL, wstag311_event_981_end },
    { 982, D_WSTAG311_800A8088, 0x01270019, NULL, wstag311_event_982_end }, { -1, NULL, 0, NULL, NULL },
};
