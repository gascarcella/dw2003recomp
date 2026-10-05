#include "wstag.h"

/* WSTAG455: stage 0x235 (fieldstg_stages). */

extern WstagFuncs wstag455_funcs;
void wstag455_update();
extern WstagAnimKey D_WSTAG455_800A6378[];
extern CVECTOR wstag455_color;
extern FieldstgBattleLists wstag455_battle_lists;
extern FieldstgVramPlace wstag455_vram_places[];
extern FieldstgPlacedActor *wstag455_actors[];
extern FieldstgSprite wstag455_sprites[];
extern FieldstgMapEvent wstag455_map_events[];
extern FieldstgEventDef wstag455_events[];
void wstag455_sprite_anim1_update(WstagSpriteAnim1Object *obj);

s32 wstag455_sprite_anim_play_once(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 depth) {
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
        wstag455_sprite_anim_play_once(sa, keys, depth + 1);
    }
    return key->frame;
}

void wstag455_sprite_anim1_update(WstagSpriteAnim1Object *obj) {
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->sprite.anim.key = 0;
        obj->sprite.anim.time = D_WSTAG455_800A6378[0].time;
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                obj->sprite.sprite = sprite;
            }
        }
        break;
    case OBJECT_STATE_RUN:
        if (obj->base.step == 0) {
            obj->sprite.anim.key = 0;
            obj->sprite.anim.time = D_WSTAG455_800A6378[0].time;
            obj->base.set_step(obj, 1);
            sound_module.play(0x8004213E);
        }
        spr = obj->sprite.sprite;
        frame = wstag455_sprite_anim_play_once(&obj->sprite, D_WSTAG455_800A6378, 0);
        if (frame == 0xFF) {
            spr->shown = 0;
            obj->base.set_state(obj, OBJECT_STATE_END);
        } else {
            spr->shown = 1;
            spr->sprite = frame;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag455_sprite_anim1_create(s32 arg0) {
    return object_create(wstag455_sprite_anim1_update, sizeof(WstagSpriteAnim1Object), 0, arg0);
}

void wstag455_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_data.progress == 0xE && gamestate_flags.get_flag(0x800E, 1)) {
            data->event = fieldstg_event_start(0x168);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag455_start(void *arg0) {
    WstagObject *obj = object_new(wstag455_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag455_funcs.setup();
    return obj;
}

void wstag455_event_360_end(void) {
    gamestate_flags.set_flag(0x4007, 1);
}

void wstag455_setup(void) {
    fieldstg_stage.background_file = 0x390;
    fieldstg_stage.sprite_file = 0x03910000;
    fieldstg_stage.sprites = wstag455_sprites;
    fieldstg_stage.map_events = wstag455_map_events;
    fieldstg_stage.mask_file = 0x38F;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1FA00, 0x21F00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag455_vram_places;
    fieldstg_stage.music = 0x34;
    fieldstg_stage.sound = 0x60D00000;
    fieldstg_stage.actors = wstag455_actors;
    fieldstg_stage.color = wstag455_color;
    fieldstg_stage.battle_lists = &wstag455_battle_lists;
    fieldstg_stage.events = wstag455_events;
    fieldstg_attr.set_file(0, 0x03910001);
    fieldstg_attr.set_file(7, 0x03910002);
    fieldstg_attr.set_file(4, 0x03910003);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag455/nonmatchings/wstag455", wstag455_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag455_setup(void);

s16 D_WSTAG455_800A6294[114] = {
    FIELDSTG_EVENT_PLACE(1, 1112, 204),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_PLACE(1, 1112, 204),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 1048, 148),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_WALK(1, 1064, 180, 3),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x32D, 876, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_WALK(1, 1136, 216, 7),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 67, 7),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x32D, 882, 1),
    FIELDSTG_EVENT_ANIM(0x33C, 821, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x32D, 883, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 1, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 1),
    FIELDSTG_EVENT_DIALOG(0, 2, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 1064, 180, 3),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_GOTO_MAP(0x236, 214, 132, 1),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG455_800A6378[17] = {
    { 11, 4 }, { 12, 4 }, { 13, 4 }, { 14, 4 }, { 15, 4 }, { 16, 4 }, { 17, 4 }, { 18, 4 }, { 19, 4 }, { 20, 4 },
    { 21, 4 }, { 22, 4 }, { 23, 4 }, { 23, 4 }, { 24, 4 }, { 25, 4 }, { 255, 999 },
};
FieldstgListedBattle D_WSTAG455_800A63BC = { 54, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG455_800A63C8 = { 54, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG455_800A63D4 = { 66, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG455_800A63E0 = { 66, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG455_800A63EC = { 66, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG455_800A63F8 = { 66, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG455_800A6404 = { 66, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG455_800A6410 = { 66, 8, 0x60080000 };
FieldstgBattleList D_WSTAG455_800A641C = {
    3,
    { &D_WSTAG455_800A63BC, &D_WSTAG455_800A63C8, &D_WSTAG455_800A63D4, &D_WSTAG455_800A63E0, &D_WSTAG455_800A63EC,
        &D_WSTAG455_800A63F8, &D_WSTAG455_800A6404, &D_WSTAG455_800A6410 },
};
FieldstgListedBattle D_WSTAG455_800A6440 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A644C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A6458 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A6464 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A6470 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A647C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A6488 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A6494 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG455_800A64A0 = {
    0,
    { &D_WSTAG455_800A6440, &D_WSTAG455_800A644C, &D_WSTAG455_800A6458, &D_WSTAG455_800A6464, &D_WSTAG455_800A6470,
        &D_WSTAG455_800A647C, &D_WSTAG455_800A6488, &D_WSTAG455_800A6494 },
};
FieldstgListedBattle D_WSTAG455_800A64C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A64D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A64DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A64E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A64F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A6500 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A650C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A6518 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG455_800A6524 = {
    0,
    { &D_WSTAG455_800A64C4, &D_WSTAG455_800A64D0, &D_WSTAG455_800A64DC, &D_WSTAG455_800A64E8, &D_WSTAG455_800A64F4,
        &D_WSTAG455_800A6500, &D_WSTAG455_800A650C, &D_WSTAG455_800A6518 },
};
FieldstgListedBattle D_WSTAG455_800A6548 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A6554 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A6560 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A656C = { 329, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG455_800A6578 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG455_800A6584 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG455_800A6590 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG455_800A659C = { 66, 8, 0x60080000 };
FieldstgBattleList D_WSTAG455_800A65A8 = {
    0,
    { &D_WSTAG455_800A6548, &D_WSTAG455_800A6554, &D_WSTAG455_800A6560, &D_WSTAG455_800A656C, &D_WSTAG455_800A6578,
        &D_WSTAG455_800A6584, &D_WSTAG455_800A6590, &D_WSTAG455_800A659C },
};
FieldstgBattleLists wstag455_battle_lists = {
    16, 0, 0, { &D_WSTAG455_800A641C, &D_WSTAG455_800A64A0, &D_WSTAG455_800A6524 }, &D_WSTAG455_800A65A8,
};
FieldstgVramPlace wstag455_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 430, 288, 440, 32, 368, 511 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG455_800A6668[4] = { 0x1A02, 0, 0xFFFF, 0 };
u16 D_WSTAG455_800A6670[6] = { 0x1A02, 1, 0x8010, 0, 0xFFFF, 0 };
u16 D_WSTAG455_800A667C[4] = { 0x1A2A, 1, 0xFFFF, 0 };
u16 D_WSTAG455_800A6684[6] = { 0x1A02, 1, 0x8010, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG455_800A6690[4] = {
    { D_WSTAG455_800A6668, NULL, 746 }, { D_WSTAG455_800A6670, D_WSTAG455_800A667C, 747 },
    { D_WSTAG455_800A6684, NULL, 748 }, { NULL, NULL, 0 },
};
u16 D_WSTAG455_800A66C0[8] = { 0x600E, 1, 0x800E, 1, 0x4007, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG455_800A66D0 = { D_WSTAG455_800A66C0, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG455_800A66E4 = { NULL, D_WSTAG455_800A6690, 63, 5, 1051, 195, 5 };
FieldstgPlacedActor *wstag455_actors[3] = { &D_WSTAG455_800A66D0, &D_WSTAG455_800A66E4, NULL };
FieldstgSprite wstag455_sprites[136] = {
    { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 250, 247, 0, 0 }, { 0, 1, 0x40, 6, 0xB, 0, 0, 0, 0, 0, 1038, 93, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 759, 577, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 869, 106, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 994, 307, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 246, 499, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 424, 419, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 614, 701, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 668, 634, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 195, 282, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 572, 492, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 658, 704, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1002, 297, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 175, 158, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 234, 281, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 254, 557, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 339, 312, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 420, 249, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 553, 243, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 724, 281, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1182, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1216, 245, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 466, 173, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 497, 669, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 568, 405, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 601, 271, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 752, 258, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 883, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1144, 314, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1162, 505, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 181, 167, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 496, 372, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 667, 405, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 679, 276, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 794, 376, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 969, 437, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 970, 588, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1087, 527, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 178, 286, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 225, 218, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 279, 323, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 285, 331, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 294, 145, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 296, 531, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 299, 336, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 321, 97, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 333, 157, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 338, 595, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 425, 96, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 443, 339, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 489, 139, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 529, 228, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 567, 255, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 620, 372, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 644, 280, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 653, 273, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 655, 284, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 681, 567, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 687, 716, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 726, 322, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 785, 99, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 793, 433, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 825, 78, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 850, 452, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 879, 564, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 896, 560, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 927, 455, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 951, 92, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1040, 353, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1166, 137, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1193, 278, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1200, 176, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 196, 394, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 233, 570, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 266, 262, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 280, 143, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 338, 500, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 440, 190, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 480, 610, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 484, 436, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 509, 342, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 620, 259, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 622, 389, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 628, 558, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 690, 250, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 718, 579, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 771, 678, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 780, 152, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 823, 557, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 916, 401, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 994, 524, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1047, 396, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1055, 275, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1143, 443, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1162, 211, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 234, 174, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 235, 470, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 305, 395, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 307, 577, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 396, 324, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 418, 117, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 470, 418, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 521, 199, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 553, 384, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 561, 338, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 566, 185, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 569, 289, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 581, 678, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 587, 258, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 662, 523, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 666, 213, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 687, 469, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 690, 89, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 729, 413, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 765, 344, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 818, 233, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 868, 378, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 915, 101, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 927, 583, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 974, 343, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1019, 574, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1121, 179, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1124, 520, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1126, 309, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1178, 348, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1183, 153, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1202, 460, 0, 0 },
    { 1, 0, 0xDA, 4, 0, 0, 0, 0, 0, 0, 923, 72, 210, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 979, 425, 436, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 674, 460, 490, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 392, 185, 185, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 481, 549, 549, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 545, 613, 613, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 817, 189, 189, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 865, 165, 165, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag455_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x234, 0x570, 0x3B8, 3, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x48, 0xFFEC, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag455_funcs = { wstag455_setup };
FieldstgEventDef wstag455_events[2] = {
    { 360, D_WSTAG455_800A6294, 0x01350027, NULL, wstag455_event_360_end }, { -1, NULL, 0, NULL, NULL },
};
