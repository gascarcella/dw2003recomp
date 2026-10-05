#include "wstag.h"

/* WSTAG310: stage 0x219 (fieldstg_stages). */

extern WstagAnimKey D_WSTAG310_800A858C[];
extern WstagAnimKey D_WSTAG310_800A85A4[];
extern WstagFuncs wstag310_funcs;
extern WstagAnimKey D_WSTAG310_800A8260[];
extern WstagAnimKey D_WSTAG310_800A8284[];
extern WstagAnimKey D_WSTAG310_800A82A4[];
extern WstagAnimKey D_WSTAG310_800A82C0[];
void wstag310_anim4_update(WstagAnim4Object *obj);
extern WstagAnimKey D_WSTAG310_800A82E0[];
extern WstagAnimKey D_WSTAG310_800A8314[];
extern WstagAnimKey D_WSTAG310_800A8348[];
extern WstagAnimKey D_WSTAG310_800A8354[];
extern WstagAnimKey D_WSTAG310_800A8390[];
extern WstagAnimKey D_WSTAG310_800A83C4[];
void wstag310_gate_update(WstagGateObject *obj);

/* A sprite with an animation in Wstag310Gate2. */
typedef struct Wstag310Slot {
    /* 0x0 */ FieldstgSprite *sprite;
    /* 0x4 */ s32 unk_4;
    /* 0x8 */ WstagAnim anim;
} Wstag310Slot; /* size 0xC */

/* WstagGateObject over the sprites of types 0xB-0x10, with a 0xC slot per sprite. */
typedef struct Wstag310Gate2 {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 mode; /* 0: closed, 1: opening, 2: open, 3: closing */
    /* 0x54 */ Wstag310Slot slots[6];
} Wstag310Gate2; /* size 0x9C */

/* The data of the stage object (wstag310_update). */
typedef struct Wstag310Data {
    /* 0x00 */ Object *anims;
    /* 0x04 */ WstagGateObject *gate;
    /* 0x08 */ Wstag310Gate2 *gate2;
    /* 0x0C */ WstagTwoSpriteObject *objs[2];
    /* 0x14 */ FieldstgEvent *event;
} Wstag310Data; /* size 0x18 */

extern WstagAnimKey D_WSTAG310_800A83F8[];
extern WstagAnimKey D_WSTAG310_800A842C[];
extern WstagAnimKey D_WSTAG310_800A8460[];
extern WstagAnimKey D_WSTAG310_800A846C[];
extern WstagAnimKey D_WSTAG310_800A849C[];
extern WstagAnimKey D_WSTAG310_800A84D0[];
extern WstagSpawn D_WSTAG310_800A8504[];
extern WstagAnimKey D_WSTAG310_800A851C[];
extern WstagAnimKey D_WSTAG310_800A8534[];
extern FieldstgBattleLists wstag310_battle_lists;
extern FieldstgVramPlace wstag310_vram_places[];
extern FieldstgPlacedActor *wstag310_actors[];
extern FieldstgSprite wstag310_sprites[];
extern FieldstgMapEvent wstag310_map_events[];
extern FieldstgEventDef wstag310_events[];
void wstag310_gate2_update(Wstag310Gate2 *obj);
WstagTwoSpriteObject *wstag310_two_sprite2_create(s32 x, s32 y, s32 frame);
void wstag310_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *arg1, s32 which);
void wstag310_update();
void wstag310_two_sprite2_draw(WstagTwoSpriteObject *obj, GfxLayer *arg1, s32 which);

s32 wstag310_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
        wstag310_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag310_anim4_update(WstagAnim4Object *obj) {
    FieldstgSprite *sprite;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG310_800A8260[0].time;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG310_800A8284[0].time;
        obj->anims[2].key = 0;
        obj->anims[2].time = D_WSTAG310_800A82A4[0].time;
        obj->anims[3].key = 0;
        obj->anims[3].time = D_WSTAG310_800A82C0[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            switch (sprite->type) {
            case 1:
                sprite->sprite = wstag310_anim_loop(&obj->anims[0], D_WSTAG310_800A8260, 0);
                break;
            case 2:
                sprite->sprite = wstag310_anim_loop(&obj->anims[1], D_WSTAG310_800A8284, 0);
                break;
            case 3:
                sprite->sprite = wstag310_anim_loop(&obj->anims[2], D_WSTAG310_800A82A4, 0);
                break;
            case 4:
                sprite->sprite = wstag310_anim_loop(&obj->anims[3], D_WSTAG310_800A82C0, 0);
                break;
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag310_anim4_create(void) {
    return object_new(wstag310_anim4_update, 0x60, 0);
}

s32 wstag310_sprite_anim_advance(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag310_sprite_anim_advance(sa, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag310_gate_update(WstagGateObject *obj) {
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
                    obj->sprites[0].anim.time = D_WSTAG310_800A82E0[0].time;
                    break;
                case 6:
                    obj->sprites[1].sprite = list;
                    obj->sprites[1].anim.key = 0;
                    obj->sprites[1].anim.time = D_WSTAG310_800A8314[0].time;
                    break;
                case 7:
                    obj->sprites[2].sprite = list;
                    obj->sprites[2].anim.key = 0;
                    obj->sprites[2].anim.time = D_WSTAG310_800A8348[0].time;
                    break;
                case 8:
                    obj->sprites[3].sprite = list;
                    obj->sprites[3].anim.key = 0;
                    obj->sprites[3].anim.time = D_WSTAG310_800A8354[0].time;
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
        obj->mode = 2;
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
                    frame = wstag310_sprite_anim_advance(&obj->sprites[0], D_WSTAG310_800A82E0, 1, 0);
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
                    frame = wstag310_sprite_anim_advance(&obj->sprites[1], D_WSTAG310_800A8314, 1, 0);
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
                    sprite->frame = wstag310_sprite_anim_advance(&obj->sprites[i], D_WSTAG310_800A8348, 0, 0);
                    sprite->shown = 1;
                    break;
                case 3:
                    sprite->sprite = 2;
                    sprite->frame = wstag310_sprite_anim_advance(&obj->sprites[i], D_WSTAG310_800A8354, 0, 0);
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
                    frame = wstag310_sprite_anim_advance(&obj->sprites[0], D_WSTAG310_800A83C4, 1, 0);
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
                    frame = wstag310_sprite_anim_advance(&obj->sprites[1], D_WSTAG310_800A83C4, 1, 0);
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

void wstag310_gate_message(WstagGateObject *obj, s32 msg) {
    if (obj != NULL && msg == 0x359) {
        obj->sprites[0].anim.key = 0;
        obj->sprites[0].anim.time = D_WSTAG310_800A8390[0].time;
        obj->sprites[1].anim.key = 0;
        obj->sprites[1].anim.time = D_WSTAG310_800A83C4[0].time;
        obj->mode = 3;
    }
}

WstagGateObject *wstag310_gate_create(s32 arg0) {
    WstagGateObject *obj = object_create(wstag310_gate_update, sizeof(WstagGateObject), 0, arg0);

    obj->mode = 2;
    return obj;
}

WstagGateObject *wstag310_gate_new(void) {
    return object_new(wstag310_gate_update, sizeof(WstagGateObject), 0);
}

s32 wstag310_slot_anim_advance(Wstag310Slot *slot, WstagAnimKey *keys, s32 once, s32 depth) {
    WstagAnimKey *key = &keys[slot->anim.key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        slot->anim.time -= step;
    }
    if (slot->anim.time <= 0) {
        key++;
        slot->anim.key++;
        slot->anim.time += key->time;
        if (once) {
            if (key->frame == 0xFF) {
                return 0xFF;
            }
        } else if (key->frame == 0xFF) {
            key = keys;
            slot->anim.key = 0;
            slot->anim.time += key->time;
        }
        wstag310_slot_anim_advance(slot, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag310_gate2_update(Wstag310Gate2 *obj) {
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
                    obj->slots[0].sprite = list;
                    obj->slots[0].anim.key = 0;
                    obj->slots[0].anim.time = D_WSTAG310_800A83F8[0].time;
                    break;
                case 0xC:
                    obj->slots[1].sprite = list;
                    obj->slots[1].anim.key = 0;
                    obj->slots[1].anim.time = D_WSTAG310_800A842C[0].time;
                    break;
                case 0xD:
                    obj->slots[2].sprite = list;
                    obj->slots[2].anim.key = 0;
                    obj->slots[2].anim.time = D_WSTAG310_800A8460[0].time;
                    break;
                case 0xE:
                    obj->slots[3].sprite = list;
                    obj->slots[3].anim.key = 0;
                    obj->slots[3].anim.time = D_WSTAG310_800A846C[0].time;
                    break;
                case 0xF:
                    obj->slots[4].sprite = list;
                    obj->slots[4].anim.key = 0;
                    obj->slots[4].anim.time = 0;
                    break;
                case 0x10:
                    obj->slots[5].sprite = list;
                    obj->slots[5].anim.key = 0;
                    obj->slots[5].anim.time = 0;
                    break;
                }
            }
        }
        obj->mode = 2;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        done = 0;
        for (i = 0; i < 6; i++) {
            sprite = obj->slots[i].sprite;
            switch (obj->mode) {
            case 0:
                sprite->shown = 0;
                break;
            case 1:
                switch (i) {
                case 0:
                    frame = wstag310_slot_anim_advance(&obj->slots[0], D_WSTAG310_800A83F8, 1, 0);
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
                    frame = wstag310_slot_anim_advance(&obj->slots[1], D_WSTAG310_800A842C, 1, 0);
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
                    sprite->frame = wstag310_slot_anim_advance(&obj->slots[i], D_WSTAG310_800A8460, 0, 0);
                    sprite->shown = 1;
                    break;
                case 3:
                    sprite->sprite = 0x11;
                    sprite->frame = wstag310_slot_anim_advance(&obj->slots[i], D_WSTAG310_800A846C, 0, 0);
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
                    frame = wstag310_slot_anim_advance(&obj->slots[0], D_WSTAG310_800A84D0, 1, 0);
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
                    frame = wstag310_slot_anim_advance(&obj->slots[1], D_WSTAG310_800A84D0, 1, 0);
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

void wstag310_gate2_message(Wstag310Gate2 *obj, s32 msg) {
    if (obj != NULL && msg == 0x359) {
        obj->slots[0].anim.key = 0;
        obj->slots[0].anim.time = D_WSTAG310_800A849C[0].time;
        obj->slots[1].anim.key = 0;
        obj->slots[1].anim.time = D_WSTAG310_800A84D0[0].time;
        obj->mode = 3;
    }
}

Wstag310Gate2 *wstag310_gate2_create(s32 arg0) {
    Wstag310Gate2 *obj = object_create(wstag310_gate2_update, sizeof(Wstag310Gate2), 0, arg0);

    obj->mode = 2;
    return obj;
}

Wstag310Gate2 *wstag310_gate2_new(void) {
    return object_new(wstag310_gate2_update, sizeof(Wstag310Gate2), 0);
}

void wstag310_update(WstagObject *obj, Wstag310Data *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->anims = wstag310_anim4_create();
        for (i = 0; i < 2; i++) {
            if (D_WSTAG310_800A8504[i].condition == 0) {
                data->objs[i] = wstag310_two_sprite2_create(D_WSTAG310_800A8504[i].x, D_WSTAG310_800A8504[i].y,
                                                       D_WSTAG310_800A8504[i].frame);
            }
        }
        /* Evidence (class B, register priority only; DECISIONS "LOOP_BLOCK audit"): without the block only the
         * obj, data and flags-base saved registers differ, also with scheduling off. */
        LOOP_BLOCK(if (gamestate_data.progress == 0x17 && gamestate_flags.get_flag(0x404E, 0)) {
            data->event = fieldstg_event_start(0x2A8);
        } else if (gamestate_data.progress == 0x17 && gamestate_flags.get_flag(0x404E, 1) && gamestate_flags.get_flag(0x404F, 0)) {
            data->event = fieldstg_event_start(0x2A9);
            data->gate = wstag310_gate_create(0x33F);
            data->gate2 = wstag310_gate2_create(0x340);
        } else if (gamestate_data.progress == 0x17 && gamestate_flags.get_flag(0x404F, 1)) {
            data->event = fieldstg_event_start(0x2AA);
        } else if (gamestate_data.progress == 0x26) {
            data->event = fieldstg_event_start(0x3C1);
        });
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag310_start(void *arg0) {
    WstagObject *obj = object_new(wstag310_update, sizeof(WstagObject), sizeof(Wstag310Data));

    obj->manager = arg0;
    wstag310_funcs.setup();
    return obj;
}

s32 wstag310_anim_advance(WstagAnim *anim, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag310_anim_advance(anim, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag310_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *arg1, s32 which) {
    Sprite spr;
    GfxLayer *layer = arg1;
    WstagFrame *frame;
    s32 y;
    s32 x;
    s32 depth;

    if (obj->base.state == OBJECT_STATE_RUN) {
        frame = &obj->sprites[which];
        y = obj->y;
        x = obj->x;
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
}

s32 wstag310_is_on_screen(s32 x, s32 y, s32 w, s32 h) {
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

void wstag310_two_sprite_update(WstagTwoSpriteObject *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->frame_anim.key = 0;
        obj->base.key1 = obj->x;
        obj->base.key2 = obj->y;
        obj->frame_anim.time = D_WSTAG310_800A8534[0].time;
        obj->palette_anim.key = 0;
        obj->palette_anim.time = D_WSTAG310_800A851C[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->base.step == 0) {
            obj->frame_anim.key = 0;
            obj->frame_anim.time = D_WSTAG310_800A8534[0].time;
            obj->palette_anim.key = 0;
            obj->palette_anim.time = D_WSTAG310_800A851C[0].time;
            obj->base.set_step(obj, 1);
        }
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag310_anim_advance(&obj->palette_anim, D_WSTAG310_800A851C, 0, 0);
        done = 0;
        frame = wstag310_anim_advance(&obj->frame_anim, D_WSTAG310_800A8534, 1, 0);
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
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        if (obj->sprites[0].frame != 0 && wstag310_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag310_two_sprite_draw, obj, obj->y, 0);
        }
        if (obj->sprites[1].frame != 0 && wstag310_is_on_screen(obj->x, obj->y - 0x20, 0x20, 0x40)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag310_two_sprite_draw, obj, obj->y + 0x12, 1);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagTwoSpriteObject *wstag310_two_sprite_create(s32 arg0) {
    WstagTwoSpriteObject *obj = object_create(wstag310_two_sprite_update, sizeof(WstagTwoSpriteObject), 0, arg0);

    obj->x = 0x26C;
    obj->y = 0xFC;
    sound_module.play(0x4001D);
    obj->frame = 0x14;
    return obj;
}

s32 wstag310_anim_advance2(WstagAnim *anim, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag310_anim_advance2(anim, keys, once, depth + 1);
    }
    return key->frame;
}

/* The draw callback of wstag310_two_sprite2_update (GfxLayer.add_callback). The original keeps the layer in a register of
 * its own (a local copy of the parameter gives its allocation order). */
void wstag310_two_sprite2_draw(WstagTwoSpriteObject *obj, GfxLayer *arg1, s32 which) {
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

s32 wstag310_is_on_screen2(s32 x, s32 y, s32 w, s32 h) {
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

void wstag310_two_sprite2_update(WstagTwoSpriteObject *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->frame_anim.key = 0;
        obj->base.key1 = obj->x;
        obj->base.key2 = obj->y;
        obj->frame_anim.time = D_WSTAG310_800A85A4[0].time;
        obj->palette_anim.key = 0;
        obj->palette_anim.time = D_WSTAG310_800A858C[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag310_anim_advance2(&obj->palette_anim, D_WSTAG310_800A858C, 0, 0);
        if (obj->sprites[0].frame != 0 && wstag310_is_on_screen2(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag310_two_sprite2_draw, obj, obj->y, 0);
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->frame_anim.key = 0;
            obj->frame_anim.time = D_WSTAG310_800A85A4[0].time;
            obj->palette_anim.key = 0;
            obj->palette_anim.time = D_WSTAG310_800A858C[0].time;
            obj->base.set_step(obj, 1);
        }
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag310_anim_advance2(&obj->palette_anim, D_WSTAG310_800A858C, 0, 0);
        done = 0;
        frame = wstag310_anim_advance2(&obj->frame_anim, D_WSTAG310_800A85A4, 1, 0);
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
        if (obj->sprites[0].frame != 0 && wstag310_is_on_screen2(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag310_two_sprite2_draw, obj, obj->y, 0);
        }
        if (obj->sprites[1].frame != 0 && wstag310_is_on_screen2(obj->x, obj->y - 0x20, 0x20, 0x40)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag310_two_sprite2_draw, obj, obj->y + 0x12, 1);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

WstagTwoSpriteObject *wstag310_two_sprite2_create(s32 x, s32 y, s32 frame) {
    WstagTwoSpriteObject *obj = object_new(wstag310_two_sprite2_update, sizeof(WstagTwoSpriteObject), 0);

    obj->x = x;
    obj->y = y;
    obj->frame = frame;
    return obj;
}

void wstag310_event_680_end(void) {
    gamestate_flags.set_flag(0xC13, 1);
    gamestate_flags.set_flag(0x7401, 1);
    gamestate_flags.set_flag(0x404E, 1);
}

void wstag310_event_681_end(void) {
    gamestate_flags.set_flag(0x404F, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag310_event_682_end(void) {
    gamestate_flags.set_flag(0x4050, 1);
}

void wstag310_event_961_end(void) {
    gamestate_data.progress = 0x27;
}

void wstag310_setup(void) {
    fieldstg_stage.background_file = 0x342;
    fieldstg_stage.sprite_file = 0x03430000;
    fieldstg_stage.sprites = wstag310_sprites;
    fieldstg_stage.map_events = wstag310_map_events;
    fieldstg_stage.mask_file = 0x341;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13E00, 0x19800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag310_vram_places;
    fieldstg_stage.music = 0x2A;
    fieldstg_stage.sound = 0x60A80000;
    fieldstg_stage.actors = wstag310_actors;
    fieldstg_stage.events = wstag310_events;
    fieldstg_stage.battle_lists = &wstag310_battle_lists;
    fieldstg_attr.set_file(0, 0x03430001);
    fieldstg_attr.set_file(7, 0x03430002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag310_setup(void);

s16 D_WSTAG310_800A7AD8[204] = {
    FIELDSTG_EVENT_PLACE(2, 88, 524),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(108, 571, 235),
    FIELDSTG_EVENT_ANIM(108, 1, 1),
    FIELDSTG_EVENT_PLACE(109, 161, 441),
    FIELDSTG_EVENT_ANIM(109, 1, 1),
    FIELDSTG_EVENT_PLACE(110, 255, 489),
    FIELDSTG_EVENT_ANIM(110, 1, 1),
    FIELDSTG_EVENT_PLACE(119, 235, 452),
    FIELDSTG_EVENT_ANIM(119, 1, 1),
    FIELDSTG_EVENT_PLACE(272, 640, 265),
    FIELDSTG_EVENT_ANIM(272, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 120, 508, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 108),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 176, 480, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 109),
    FIELDSTG_EVENT_ANIM(0x324, 805, 110),
    FIELDSTG_EVENT_ANIM(0x325, 805, 119),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 109),
    FIELDSTG_EVENT_ANIM(0x324, 806, 110),
    FIELDSTG_EVENT_ANIM(0x325, 806, 119),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(109, 184, 452, 0),
    FIELDSTG_EVENT_WALK(110, 231, 476, 2),
    FIELDSTG_EVENT_WALK(119, 208, 464, 1),
    FIELDSTG_EVENT_WAIT_WALK(119),
    FIELDSTG_EVENT_DIALOG(0, 4, 109, 2),
    FIELDSTG_EVENT_ANIM(109, 1, 0),
    FIELDSTG_EVENT_ANIM(110, 1, 2),
    FIELDSTG_EVENT_ANIM(119, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 110, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 119, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG310_800A7C70[210] = {
    FIELDSTG_EVENT_PLACE(2, 176, 480),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(108, 571, 235),
    FIELDSTG_EVENT_ANIM(108, 1, 1),
    FIELDSTG_EVENT_PLACE(272, 640, 265),
    FIELDSTG_EVENT_ANIM(272, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 10, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 368, 384, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 432, 304, 5),
    FIELDSTG_EVENT_ANIM(108, 1, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_ANIM(0x32D, 873, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 108),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 108, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 108, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x354, 821, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_PLACE(272, 0, 0),
    FIELDSTG_EVENT_ANIM(272, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WALK(2, 524, 258, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 108),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x33F, 857, 2),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_ANIM(0x340, 857, 2),
    FIELDSTG_EVENT_WAIT(72),
    FIELDSTG_EVENT_DIALOG(0, 7, 108, 0),
    FIELDSTG_EVENT_ANIM(108, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 108, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG310_800A7E14[275] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 524, 264),
    FIELDSTG_EVENT_PLACE(2, 524, 258),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(101, 336, 400),
    FIELDSTG_EVENT_ANIM(101, 1, 5),
    FIELDSTG_EVENT_PLACE(108, 571, 235),
    FIELDSTG_EVENT_ANIM(108, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 108, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_PLACE(12, 336, 400),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_WALK(101, 368, 384, 5),
    FIELDSTG_EVENT_WAIT_WALK(101),
    FIELDSTG_EVENT_WALK(12, 368, 384, 5),
    FIELDSTG_EVENT_WALK(101, 386, 362, 5),
    FIELDSTG_EVENT_PLACE(103, 336, 400),
    FIELDSTG_EVENT_ANIM(103, 1, 5),
    FIELDSTG_EVENT_WAIT_WALK(101),
    FIELDSTG_EVENT_WALK(12, 414, 326, 5),
    FIELDSTG_EVENT_WALK(101, 432, 304, 5),
    FIELDSTG_EVENT_WAIT_WALK(101),
    FIELDSTG_EVENT_WALK(12, 432, 304, 5),
    FIELDSTG_EVENT_WALK(101, 464, 288, 5),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_WALK(12, 464, 288, 5),
    FIELDSTG_EVENT_WALK(101, 496, 272, 5),
    FIELDSTG_EVENT_WAIT_WALK(101),
    FIELDSTG_EVENT_WALK(12, 496, 272, 5),
    FIELDSTG_EVENT_WALK(101, 512, 280, 5),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_WALK(12, 480, 264, 5),
    FIELDSTG_EVENT_ANIM(101, 1, 5),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_DIALOG(0, 2, 101, 1),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_ANIM(101, 1, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 12, 2),
    FIELDSTG_EVENT_ANIM(12, 1, 6),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(103, 368, 384, 5),
    FIELDSTG_EVENT_WAIT_WALK(103),
    FIELDSTG_EVENT_WALK(103, 432, 304, 5),
    FIELDSTG_EVENT_WAIT_WALK(103),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_ANIM(101, 1, 1),
    FIELDSTG_EVENT_WALK(103, 464, 288, 5),
    FIELDSTG_EVENT_WAIT_WALK(103),
    FIELDSTG_EVENT_DIALOG(0, 4, 103, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 103, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 101, 3),
    FIELDSTG_EVENT_ANIM(101, 1, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 101, 3),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_ANIM(101, 1, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x218, 100, 100, 0),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG310_800A803C[273] = {
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 296, 421, 5),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 58, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_PLACE(272, 64, 536),
    FIELDSTG_EVENT_ANIM(272, 1, 5),
    FIELDSTG_EVENT_PLACE(273, 40, 548),
    FIELDSTG_EVENT_ANIM(273, 1, 5),
    FIELDSTG_EVENT_PLACE(274, 1, 568),
    FIELDSTG_EVENT_ANIM(274, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(1, 96, 520),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_WALK(274, 16, 560, 5),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT_WALK(274),
    FIELDSTG_EVENT_ANIM(272, 1, 4),
    FIELDSTG_EVENT_ANIM(273, 1, 6),
    FIELDSTG_EVENT_ANIM(274, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(272, 1, 5),
    FIELDSTG_EVENT_ANIM(273, 1, 5),
    FIELDSTG_EVENT_ANIM(274, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(272, 1, 6),
    FIELDSTG_EVENT_ANIM(273, 1, 4),
    FIELDSTG_EVENT_ANIM(274, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(272, 1, 5),
    FIELDSTG_EVENT_ANIM(273, 1, 5),
    FIELDSTG_EVENT_ANIM(274, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 272),
    FIELDSTG_EVENT_ANIM(0x324, 805, 273),
    FIELDSTG_EVENT_ANIM(0x325, 805, 274),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 272),
    FIELDSTG_EVENT_ANIM(0x324, 806, 273),
    FIELDSTG_EVENT_ANIM(0x325, 806, 274),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(272, 1, 1),
    FIELDSTG_EVENT_ANIM(273, 1, 1),
    FIELDSTG_EVENT_ANIM(274, 1, 1),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_WALK(274, 1, 568, 1),
    FIELDSTG_EVENT_WAIT_WALK(274),
    FIELDSTG_EVENT_WALK(273, 1, 568, 1),
    FIELDSTG_EVENT_PLACE(274, 0, 0),
    FIELDSTG_EVENT_ANIM(274, 1, 1),
    FIELDSTG_EVENT_WAIT_WALK(273),
    FIELDSTG_EVENT_WALK(272, 1, 568, 1),
    FIELDSTG_EVENT_PLACE(273, 0, 0),
    FIELDSTG_EVENT_ANIM(273, 1, 1),
    FIELDSTG_EVENT_WAIT_WALK(272),
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 1),
    FIELDSTG_EVENT_PLACE(272, 0, 0),
    FIELDSTG_EVENT_ANIM(272, 1, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 112, 512, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x218, 376, 252, 1),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG310_800A8260[9] = {
    { 51, 8 }, { 52, 8 }, { 53, 8 }, { 54, 8 }, { 55, 8 }, { 56, 8 }, { 57, 8 }, { 75, 40 }, { 255, 0 },
};
WstagAnimKey D_WSTAG310_800A8284[8] = {
    { 58, 8 }, { 59, 8 }, { 60, 8 }, { 61, 8 }, { 62, 8 }, { 63, 8 }, { 75, 40 }, { 255, 0 },
};
WstagAnimKey D_WSTAG310_800A82A4[7] = {
    { 64, 8 }, { 65, 8 }, { 66, 8 }, { 67, 8 }, { 68, 8 }, { 75, 40 }, { 255, 0 },
};
WstagAnimKey D_WSTAG310_800A82C0[8] = {
    { 69, 8 }, { 70, 8 }, { 71, 8 }, { 72, 8 }, { 73, 8 }, { 74, 8 }, { 75, 40 }, { 255, 0 },
};
WstagAnimKey D_WSTAG310_800A82E0[13] = {
    { 5, 4 }, { 6, 4 }, { 7, 4 }, { 8, 4 }, { 9, 4 }, { 10, 4 }, { 11, 4 }, { 12, 4 }, { 13, 4 }, { 14, 4 },
    { 15, 4 }, { 16, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG310_800A8314[13] = {
    { 28, 4 }, { 29, 4 }, { 30, 4 }, { 31, 4 }, { 32, 4 }, { 33, 4 }, { 34, 4 }, { 35, 4 }, { 36, 4 }, { 37, 4 },
    { 38, 4 }, { 39, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG310_800A8348[3] = { { 0, 4 }, { 1, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG310_800A8354[15] = {
    { 0, 12 }, { 1, 12 }, { 2, 12 }, { 3, 12 }, { 4, 12 }, { 5, 12 }, { 6, 12 }, { 7, 12 }, { 8, 12 }, { 9, 12 },
    { 10, 12 }, { 11, 12 }, { 12, 12 }, { 13, 12 }, { 255, 0 },
};
WstagAnimKey D_WSTAG310_800A8390[13] = {
    { 16, 4 }, { 15, 4 }, { 14, 4 }, { 13, 4 }, { 12, 4 }, { 11, 4 }, { 10, 4 }, { 9, 4 }, { 8, 4 }, { 7, 4 },
    { 6, 4 }, { 5, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG310_800A83C4[13] = {
    { 39, 4 }, { 38, 4 }, { 37, 4 }, { 36, 4 }, { 35, 4 }, { 34, 4 }, { 33, 4 }, { 32, 4 }, { 31, 4 }, { 30, 4 },
    { 29, 4 }, { 28, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG310_800A83F8[13] = {
    { 76, 4 }, { 77, 4 }, { 78, 4 }, { 79, 4 }, { 80, 4 }, { 81, 4 }, { 82, 4 }, { 83, 4 }, { 84, 4 }, { 85, 4 },
    { 86, 4 }, { 87, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG310_800A842C[13] = {
    { 88, 4 }, { 89, 4 }, { 90, 4 }, { 91, 4 }, { 92, 4 }, { 93, 4 }, { 94, 4 }, { 95, 4 }, { 96, 4 }, { 97, 4 },
    { 98, 4 }, { 99, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG310_800A8460[3] = { { 0, 4 }, { 1, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG310_800A846C[12] = {
    { 0, 12 }, { 1, 12 }, { 2, 12 }, { 3, 12 }, { 4, 12 }, { 5, 12 }, { 6, 12 }, { 7, 12 }, { 8, 12 }, { 9, 12 },
    { 10, 12 }, { 255, 0 },
};
WstagAnimKey D_WSTAG310_800A849C[13] = {
    { 87, 4 }, { 86, 4 }, { 85, 4 }, { 84, 4 }, { 83, 4 }, { 82, 4 }, { 81, 4 }, { 80, 4 }, { 79, 4 }, { 78, 4 },
    { 77, 4 }, { 76, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG310_800A84D0[13] = {
    { 99, 4 }, { 98, 4 }, { 97, 4 }, { 96, 4 }, { 95, 4 }, { 94, 4 }, { 93, 4 }, { 92, 4 }, { 91, 4 }, { 90, 4 },
    { 89, 4 }, { 88, 4 }, { 255, 999 },
};
WstagSpawn D_WSTAG310_800A8504[2] = { { 20, 0, 492, 188 }, { 20, 0, 620, 252 } };
WstagAnimKey D_WSTAG310_800A851C[6] = { { 0, 6 }, { 1, 6 }, { 2, 68 }, { 1, 4 }, { 0, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG310_800A8534[22] = {
    { 300, 18 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 },
    { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 },
    { 255, 999 },
};
WstagAnimKey D_WSTAG310_800A858C[6] = { { 0, 6 }, { 1, 6 }, { 2, 68 }, { 1, 4 }, { 0, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG310_800A85A4[22] = {
    { 300, 18 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 },
    { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 },
    { 255, 999 },
};
FieldstgListedBattle D_WSTAG310_800A85FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A8608 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A8614 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A8620 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A862C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A8638 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A8644 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A8650 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG310_800A865C = {
    0,
    { &D_WSTAG310_800A85FC, &D_WSTAG310_800A8608, &D_WSTAG310_800A8614, &D_WSTAG310_800A8620, &D_WSTAG310_800A862C,
        &D_WSTAG310_800A8638, &D_WSTAG310_800A8644, &D_WSTAG310_800A8650 },
};
FieldstgListedBattle D_WSTAG310_800A8680 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A868C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A8698 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A86A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A86B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A86BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A86C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A86D4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG310_800A86E0 = {
    0,
    { &D_WSTAG310_800A8680, &D_WSTAG310_800A868C, &D_WSTAG310_800A8698, &D_WSTAG310_800A86A4, &D_WSTAG310_800A86B0,
        &D_WSTAG310_800A86BC, &D_WSTAG310_800A86C8, &D_WSTAG310_800A86D4 },
};
FieldstgListedBattle D_WSTAG310_800A8704 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A8710 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A871C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A8728 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A8734 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A8740 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A874C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A8758 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG310_800A8764 = {
    0,
    { &D_WSTAG310_800A8704, &D_WSTAG310_800A8710, &D_WSTAG310_800A871C, &D_WSTAG310_800A8728, &D_WSTAG310_800A8734,
        &D_WSTAG310_800A8740, &D_WSTAG310_800A874C, &D_WSTAG310_800A8758 },
};
FieldstgListedBattle D_WSTAG310_800A8788 = { 10, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG310_800A8794 = { 191, 18, 0x60080000 };
FieldstgListedBattle D_WSTAG310_800A87A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A87AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A87B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A87C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A87D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG310_800A87DC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG310_800A87E8 = {
    0,
    { &D_WSTAG310_800A8788, &D_WSTAG310_800A8794, &D_WSTAG310_800A87A0, &D_WSTAG310_800A87AC, &D_WSTAG310_800A87B8,
        &D_WSTAG310_800A87C4, &D_WSTAG310_800A87D0, &D_WSTAG310_800A87DC },
};
FieldstgBattleLists wstag310_battle_lists = {
    137, 0, 0, { &D_WSTAG310_800A865C, &D_WSTAG310_800A86E0, &D_WSTAG310_800A8764 }, &D_WSTAG310_800A87E8,
};
FieldstgVramPlace wstag310_vram_places[17] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 448, 256, 502, 256, 728, 0, 352, 496 }, { 448, 256, 502, 288, 728, 32, 320, 495 },
    { 320, 256, 374, 256, 216, 0, 336, 495 }, { 320, 256, 374, 296, 216, 40, 352, 495 },
    { 448, 256, 502, 320, 728, 64, 368, 495 }, { 320, 256, 374, 336, 216, 80, 320, 494 },
    { 320, 256, 374, 376, 216, 120, 336, 494 }, { 320, 256, 374, 416, 216, 160, 368, 494 },
    { 448, 256, 482, 376, 648, 120, 320, 493 }, { 448, 256, 490, 376, 680, 120, 336, 493 },
    { 448, 256, 458, 384, 552, 128, 352, 493 },
};
u16 D_WSTAG310_800A8938[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG310_800A8940[4] = { 0x6017, 1, 0xFFFF, 0 };
u16 D_WSTAG310_800A8948[4] = { 0x6017, 1, 0xFFFF, 0 };
u16 D_WSTAG310_800A8950[4] = { 0x6017, 1, 0xFFFF, 0 };
u16 D_WSTAG310_800A8958[4] = { 0x6017, 1, 0xFFFF, 0 };
u16 D_WSTAG310_800A8960[6] = { 0x404E, 0, 0x6017, 1, 0xFFFF, 0 };
u16 D_WSTAG310_800A896C[6] = { 0x404E, 0, 0x6017, 1, 0xFFFF, 0 };
u16 D_WSTAG310_800A8978[6] = { 0x404E, 0, 0x6017, 1, 0xFFFF, 0 };
u16 D_WSTAG310_800A8984[6] = { 0x404F, 0, 0x6017, 1, 0xFFFF, 0 };
u16 D_WSTAG310_800A8990[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG310_800A8998[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG310_800A89A0[4] = { 0x6026, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG310_800A89A8 = { D_WSTAG310_800A8938, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG310_800A89BC = { D_WSTAG310_800A8940, NULL, 12, 5, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG310_800A89D0 = { D_WSTAG310_800A8948, NULL, 101, 6, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG310_800A89E4 = { D_WSTAG310_800A8950, NULL, 103, 7, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG310_800A89F8 = { D_WSTAG310_800A8958, NULL, 108, 8, 571, 235, 1 };
FieldstgPlacedActor D_WSTAG310_800A8A0C = { D_WSTAG310_800A8960, NULL, 109, 9, 161, 441, 1 };
FieldstgPlacedActor D_WSTAG310_800A8A20 = { D_WSTAG310_800A896C, NULL, 110, 10, 255, 489, 1 };
FieldstgPlacedActor D_WSTAG310_800A8A34 = { D_WSTAG310_800A8978, NULL, 119, 11, 235, 452, 1 };
FieldstgPlacedActor D_WSTAG310_800A8A48 = { D_WSTAG310_800A8984, NULL, 272, 12, 640, 265, 1 };
FieldstgPlacedActor D_WSTAG310_800A8A5C = { D_WSTAG310_800A8990, NULL, 272, 12, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG310_800A8A70 = { D_WSTAG310_800A8998, NULL, 273, 13, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG310_800A8A84 = { D_WSTAG310_800A89A0, NULL, 274, 14, 0, 0, 1 };
FieldstgPlacedActor *wstag310_actors[13] = {
    &D_WSTAG310_800A89A8, &D_WSTAG310_800A89BC, &D_WSTAG310_800A89D0, &D_WSTAG310_800A89E4, &D_WSTAG310_800A89F8,
    &D_WSTAG310_800A8A0C, &D_WSTAG310_800A8A20, &D_WSTAG310_800A8A34, &D_WSTAG310_800A8A48, &D_WSTAG310_800A8A5C,
    &D_WSTAG310_800A8A70, &D_WSTAG310_800A8A84, NULL,
};
FieldstgSprite wstag310_sprites[27] = {
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
    { 0, 0xD, 0xFF, 2, 0x29, 0, 0, 0, 0, 0, 426, 99, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 72, 464, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 178, 350, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 303, 267, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 369, 219, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 392, 328, 386, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 545, 243, 266, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag310_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x218, 0x178, 0xFC, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag310_funcs = { wstag310_setup };
FieldstgEventDef wstag310_events[5] = {
    { 680, D_WSTAG310_800A7AD8, 0x0127000A, NULL, wstag310_event_680_end },
    { 681, D_WSTAG310_800A7C70, 0x0127000B, NULL, wstag310_event_681_end },
    { 682, D_WSTAG310_800A7E14, 0x0127000C, NULL, wstag310_event_682_end },
    { 961, D_WSTAG310_800A803C, 0x01270014, NULL, wstag310_event_961_end }, { -1, NULL, 0, NULL, NULL },
};
