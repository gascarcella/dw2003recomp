#include "wstag.h"

/* WSTAG635: stage 0x258 (fieldstg_stages). */

extern WstagAnimKey D_WSTAG635_800A6360[];
extern WstagAnimKey D_WSTAG635_800A6394[];
extern WstagAnimKey D_WSTAG635_800A63C8[];
extern WstagExits *wstag635_exits[];
extern WstagFuncs wstag635_funcs;
extern FieldstgBattleLists wstag635_battle_lists;
extern FieldstgVramPlace wstag635_vram_places[];
extern FieldstgPlacedActor *wstag635_actors[];
extern FieldstgSprite wstag635_sprites[];
extern FieldstgMapEvent wstag635_map_events[];

s32 wstag635_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
        wstag635_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag635_anim_update(WstagAnimObject *obj) {
    FieldstgSprite *sprite;
    s32 frames[3];

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG635_800A6360[0].time;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG635_800A6394[0].time;
        obj->anims[2].key = 0;
        obj->anims[2].time = D_WSTAG635_800A63C8[0].time;
        break;
    case OBJECT_STATE_RUN:
        sprite = fieldstg_stage.sprites;
        frames[0] = wstag635_anim_loop(&obj->anims[0], D_WSTAG635_800A6360, 0);
        frames[1] = wstag635_anim_loop(&obj->anims[1], D_WSTAG635_800A6394, 0);
        frames[2] = wstag635_anim_loop(&obj->anims[2], D_WSTAG635_800A63C8, 0);
        for (; sprite->present != 0; sprite++) {
            switch (sprite->type) {
            case 1:
                sprite->sprite = frames[0];
                break;
            case 2:
                sprite->sprite = frames[1];
                break;
            case 3:
                sprite->sprite = frames[2];
                break;
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagAnimObject *wstag635_anim_new(void) {
    return object_new(wstag635_anim_update, sizeof(WstagAnimObject), 0);
}

/* Returns int: the original declares it so (the tail call needs a value; no path sets one). */
s32 wstag635_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag635_set_exits(dst, list + 1, arg2, arg3);
    }
    exit = exits->exits;
    dst->param = exit->stage;
    dst->x = exit->x;
    dst->y = exit->y;
    dst->dir = exit->dir;
    dst->route = exit->route;
    dst->room = exit->room;
    while (exit->next != NULL) {
        exit = exit->next;
        dst++;
        dst->param = exit->stage;
        dst->x = exit->x;
        dst->y = exit->y;
        dst->dir = exit->dir;
        dst->route = exit->route;
        dst->room = exit->room;
    }
}

void wstag635_update(WstagObject *obj, WstagAnimObject **data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        *data = wstag635_anim_new();
        wstag635_set_exits(fieldstg_stage.map_events, wstag635_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag635_start(void *arg0) {
    WstagObject *obj = object_new(wstag635_update, sizeof(WstagObject), sizeof(WstagAnimObject *));

    obj->manager = arg0;
    wstag635_funcs.setup();
    return obj;
}

void wstag635_setup(void) {
    fieldstg_stage.background_file = 0x49B;
    fieldstg_stage.sprite_file = 0x049C0000;
    fieldstg_stage.sprites = wstag635_sprites;
    fieldstg_stage.map_events = wstag635_map_events;
    fieldstg_stage.mask_file = 0x49A;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x20900, 0x31B00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag635_vram_places;
    fieldstg_stage.music = 0x38;
    fieldstg_stage.sound = 0x60E00000;
    fieldstg_stage.actors = wstag635_actors;
    fieldstg_stage.battle_lists = &wstag635_battle_lists;
    fieldstg_attr.set_file(0, 0x049C0001);
    fieldstg_attr.set_file(7, 0x049C0002);
    fieldstg_attr.set_file(4, 0x049C0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag635_setup(void);

WstagAnimKey D_WSTAG635_800A6360[13] = {
    { 50, 8 }, { 51, 8 }, { 52, 8 }, { 53, 8 }, { 54, 8 }, { 55, 8 }, { 56, 8 }, { 57, 8 }, { 58, 8 }, { 59, 8 },
    { 60, 8 }, { 82, 160 }, { 255, 0 },
};
WstagAnimKey D_WSTAG635_800A6394[13] = {
    { 61, 8 }, { 62, 8 }, { 63, 8 }, { 64, 8 }, { 65, 8 }, { 66, 8 }, { 67, 8 }, { 68, 8 }, { 69, 8 }, { 70, 8 },
    { 71, 8 }, { 82, 160 }, { 255, 0 },
};
WstagAnimKey D_WSTAG635_800A63C8[12] = {
    { 72, 8 }, { 73, 8 }, { 74, 8 }, { 75, 8 }, { 76, 8 }, { 77, 8 }, { 78, 8 }, { 79, 8 }, { 80, 8 }, { 81, 8 },
    { 82, 160 }, { 255, 0 },
};
WstagExit D_WSTAG635_800A63F8 = { 0x259, 1, 2, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG635_800A6408 = { 0x259, 1, 3, 0x358, 0xFC, 1, &D_WSTAG635_800A63F8 };
WstagExit D_WSTAG635_800A6418 = { 0x259, 1, 1, 0x120, 0x100, 7, &D_WSTAG635_800A6408 };
WstagExit D_WSTAG635_800A6428 = { 0x257, 0, 0, 0x100, 0x278, 5, &D_WSTAG635_800A6418 };
WstagExits D_WSTAG635_800A6438 = { 1, 1, &D_WSTAG635_800A6428 };
WstagExit D_WSTAG635_800A6440 = { 0x259, 1, 1, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG635_800A6450 = { 0x259, 1, 4, 0x358, 0xFC, 1, &D_WSTAG635_800A6440 };
WstagExit D_WSTAG635_800A6460 = { 0x259, 1, 2, 0x120, 0x100, 7, &D_WSTAG635_800A6450 };
WstagExit D_WSTAG635_800A6470 = { 0x257, 0, 0, 0x100, 0x278, 5, &D_WSTAG635_800A6460 };
WstagExits D_WSTAG635_800A6480 = { 1, 2, &D_WSTAG635_800A6470 };
WstagExit D_WSTAG635_800A6488 = { 0x259, 1, 3, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG635_800A6498 = { 0x259, 1, 5, 0x358, 0xFC, 1, &D_WSTAG635_800A6488 };
WstagExit D_WSTAG635_800A64A8 = { 0x259, 1, 4, 0x120, 0x100, 7, &D_WSTAG635_800A6498 };
WstagExit D_WSTAG635_800A64B8 = { 0x259, 1, 1, 0x118, 0x2EC, 5, &D_WSTAG635_800A64A8 };
WstagExits D_WSTAG635_800A64C8 = { 1, 3, &D_WSTAG635_800A64B8 };
WstagExit D_WSTAG635_800A64D0 = { 0x259, 1, 4, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG635_800A64E0 = { 0x259, 1, 6, 0x358, 0xFC, 1, &D_WSTAG635_800A64D0 };
WstagExit D_WSTAG635_800A64F0 = { 0x259, 1, 3, 0x120, 0x100, 7, &D_WSTAG635_800A64E0 };
WstagExit D_WSTAG635_800A6500 = { 0x259, 1, 2, 0x118, 0x2EC, 5, &D_WSTAG635_800A64F0 };
WstagExits D_WSTAG635_800A6510 = { 1, 4, &D_WSTAG635_800A6500 };
WstagExit D_WSTAG635_800A6518 = { 0x259, 1, 6, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG635_800A6528 = { 0x259, 1, 7, 0x358, 0xFC, 1, &D_WSTAG635_800A6518 };
WstagExit D_WSTAG635_800A6538 = { 0x259, 1, 5, 0x120, 0x100, 7, &D_WSTAG635_800A6528 };
WstagExit D_WSTAG635_800A6548 = { 0x259, 1, 3, 0x118, 0x2EC, 5, &D_WSTAG635_800A6538 };
WstagExits D_WSTAG635_800A6558 = { 1, 5, &D_WSTAG635_800A6548 };
WstagExit D_WSTAG635_800A6560 = { 0x25A, 0, 0, 0x2F8, 0x244, 3, NULL };
WstagExit D_WSTAG635_800A6570 = { 0x259, 1, 8, 0x358, 0xFC, 1, &D_WSTAG635_800A6560 };
WstagExit D_WSTAG635_800A6580 = { 0x259, 1, 6, 0x120, 0x100, 7, &D_WSTAG635_800A6570 };
WstagExit D_WSTAG635_800A6590 = { 0x259, 1, 4, 0x118, 0x2EC, 5, &D_WSTAG635_800A6580 };
WstagExits D_WSTAG635_800A65A0 = { 1, 6, &D_WSTAG635_800A6590 };
WstagExit D_WSTAG635_800A65A8 = { 0x259, 1, 7, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG635_800A65B8 = { 0x259, 1, 1, 0x358, 0xFC, 1, &D_WSTAG635_800A65A8 };
WstagExit D_WSTAG635_800A65C8 = { 0x259, 1, 8, 0x120, 0x100, 7, &D_WSTAG635_800A65B8 };
WstagExit D_WSTAG635_800A65D8 = { 0x259, 1, 5, 0x118, 0x2EC, 5, &D_WSTAG635_800A65C8 };
WstagExits D_WSTAG635_800A65E8 = { 1, 7, &D_WSTAG635_800A65D8 };
WstagExit D_WSTAG635_800A65F0 = { 0x259, 1, 8, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG635_800A6600 = { 0x259, 1, 2, 0x358, 0xFC, 1, &D_WSTAG635_800A65F0 };
WstagExit D_WSTAG635_800A6610 = { 0x259, 1, 7, 0x120, 0x100, 7, &D_WSTAG635_800A6600 };
WstagExit D_WSTAG635_800A6620 = { 0x259, 1, 6, 0x118, 0x2EC, 5, &D_WSTAG635_800A6610 };
WstagExits D_WSTAG635_800A6630 = { 1, 8, &D_WSTAG635_800A6620 };
WstagExits D_WSTAG635_800A6638 = { 0, 0, &D_WSTAG635_800A6428 };
WstagExits *wstag635_exits[10] = {
    &D_WSTAG635_800A6438, &D_WSTAG635_800A6480, &D_WSTAG635_800A64C8, &D_WSTAG635_800A6510, &D_WSTAG635_800A6558,
    &D_WSTAG635_800A65A0, &D_WSTAG635_800A65E8, &D_WSTAG635_800A6630, &D_WSTAG635_800A6638, NULL,
};
FieldstgListedBattle D_WSTAG635_800A6668 = { 73, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG635_800A6674 = { 73, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG635_800A6680 = { 73, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG635_800A668C = { 74, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG635_800A6698 = { 74, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG635_800A66A4 = { 74, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG635_800A66B0 = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG635_800A66BC = { 156, 5, 0x60080000 };
FieldstgBattleList D_WSTAG635_800A66C8 = {
    3,
    { &D_WSTAG635_800A6668, &D_WSTAG635_800A6674, &D_WSTAG635_800A6680, &D_WSTAG635_800A668C, &D_WSTAG635_800A6698,
        &D_WSTAG635_800A66A4, &D_WSTAG635_800A66B0, &D_WSTAG635_800A66BC },
};
FieldstgListedBattle D_WSTAG635_800A66EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A66F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A6704 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A6710 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A671C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A6728 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A6734 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A6740 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG635_800A674C = {
    0,
    { &D_WSTAG635_800A66EC, &D_WSTAG635_800A66F8, &D_WSTAG635_800A6704, &D_WSTAG635_800A6710, &D_WSTAG635_800A671C,
        &D_WSTAG635_800A6728, &D_WSTAG635_800A6734, &D_WSTAG635_800A6740 },
};
FieldstgListedBattle D_WSTAG635_800A6770 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A677C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A6788 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A6794 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A67A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A67AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A67B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A67C4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG635_800A67D0 = {
    0,
    { &D_WSTAG635_800A6770, &D_WSTAG635_800A677C, &D_WSTAG635_800A6788, &D_WSTAG635_800A6794, &D_WSTAG635_800A67A0,
        &D_WSTAG635_800A67AC, &D_WSTAG635_800A67B8, &D_WSTAG635_800A67C4 },
};
FieldstgListedBattle D_WSTAG635_800A67F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A6800 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A680C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A6818 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A6824 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A6830 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A683C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG635_800A6848 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG635_800A6854 = {
    0,
    { &D_WSTAG635_800A67F4, &D_WSTAG635_800A6800, &D_WSTAG635_800A680C, &D_WSTAG635_800A6818, &D_WSTAG635_800A6824,
        &D_WSTAG635_800A6830, &D_WSTAG635_800A683C, &D_WSTAG635_800A6848 },
};
FieldstgBattleLists wstag635_battle_lists = {
    37, 0, 0, { &D_WSTAG635_800A66C8, &D_WSTAG635_800A674C, &D_WSTAG635_800A67D0 }, &D_WSTAG635_800A6854,
};
FieldstgVramPlace wstag635_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 352, 216, 96, 368, 511 }, { 320, 256, 374, 392, 216, 136, 320, 510 },
    { 384, 256, 432, 392, 448, 136, 336, 510 }, { 384, 256, 392, 404, 288, 148, 352, 510 },
    { 384, 256, 384, 412, 256, 156, 368, 510 }, { 384, 256, 438, 256, 472, 0, 320, 509 },
};
u16 D_WSTAG635_800A6954[4] = { 0x1A2F, 0, 0xFFFF, 0 };
u16 D_WSTAG635_800A695C[4] = { 0x1A2F, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A6964[6] = { 0x1A2F, 1, 0x702E, 0, 0xFFFF, 0 };
u16 D_WSTAG635_800A6970[8] = { 0x1A2F, 1, 0x702E, 1, 0x7027, 0, 0xFFFF, 0 };
u16 D_WSTAG635_800A6980[10] = {
    0x1A2F, 1, 0x702E, 1, 0x7027, 1, 0x7029, 0,
    0xFFFF, 0,
};
u16 D_WSTAG635_800A6994[4] = { 0x604, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A699C[10] = {
    0x1A2F, 1, 0x702E, 1, 0x7027, 1, 0x7029, 1,
    0xFFFF, 0,
};
u16 D_WSTAG635_800A69B0[4] = { 0x7010, 0, 0xFFFF, 0 };
u16 D_WSTAG635_800A69B8[6] = { 0x7037, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A69C4[4] = { 0x7010, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A69CC[4] = { 0x1A30, 0, 0xFFFF, 0 };
u16 D_WSTAG635_800A69D4[4] = { 0x1A30, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A69DC[6] = { 0x1A30, 1, 0x7030, 0, 0xFFFF, 0 };
u16 D_WSTAG635_800A69E8[8] = { 0x1A30, 1, 0x7030, 1, 0x7027, 0, 0xFFFF, 0 };
u16 D_WSTAG635_800A69F8[10] = {
    0x1A30, 1, 0x7030, 1, 0x7027, 1, 0x7029, 0,
    0xFFFF, 0,
};
u16 D_WSTAG635_800A6A0C[4] = { 0x605, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A6A14[10] = {
    0x1A30, 1, 0x7030, 1, 0x7027, 1, 0x7029, 1,
    0xFFFF, 0,
};
u16 D_WSTAG635_800A6A28[4] = { 0x7012, 0, 0xFFFF, 0 };
u16 D_WSTAG635_800A6A30[6] = { 0x7039, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A6A3C[4] = { 0x7012, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A6A44[4] = { 0x1A31, 0, 0xFFFF, 0 };
u16 D_WSTAG635_800A6A4C[4] = { 0x1A31, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A6A54[6] = { 0x1A31, 1, 0x702F, 0, 0xFFFF, 0 };
u16 D_WSTAG635_800A6A60[8] = { 0x1A31, 1, 0x702F, 1, 0x7027, 0, 0xFFFF, 0 };
u16 D_WSTAG635_800A6A70[10] = {
    0x1A31, 1, 0x702F, 1, 0x7027, 1, 0x7029, 0,
    0xFFFF, 0,
};
u16 D_WSTAG635_800A6A84[4] = { 0x606, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A6A8C[10] = {
    0x1A31, 1, 0x702F, 1, 0x7027, 1, 0x7029, 1,
    0xFFFF, 0,
};
u16 D_WSTAG635_800A6AA0[4] = { 0x7011, 0, 0xFFFF, 0 };
u16 D_WSTAG635_800A6AA8[6] = { 0x7038, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A6AB4[4] = { 0x7011, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG635_800A6ABC[6] = {
    { D_WSTAG635_800A6954, D_WSTAG635_800A695C, 829 }, { D_WSTAG635_800A6964, NULL, 835 },
    { D_WSTAG635_800A6970, NULL, 830 }, { D_WSTAG635_800A6980, D_WSTAG635_800A6994, 831 },
    { D_WSTAG635_800A699C, NULL, 836 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG635_800A6B04[3] = {
    { D_WSTAG635_800A69B0, D_WSTAG635_800A69B8, 832 }, { D_WSTAG635_800A69C4, NULL, 834 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG635_800A6B28[6] = {
    { D_WSTAG635_800A69CC, D_WSTAG635_800A69D4, 837 }, { D_WSTAG635_800A69DC, NULL, 843 },
    { D_WSTAG635_800A69E8, NULL, 838 }, { D_WSTAG635_800A69F8, D_WSTAG635_800A6A0C, 839 },
    { D_WSTAG635_800A6A14, NULL, 844 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG635_800A6B70[3] = {
    { D_WSTAG635_800A6A28, D_WSTAG635_800A6A30, 840 }, { D_WSTAG635_800A6A3C, NULL, 842 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG635_800A6B94[2] = { { NULL, NULL, 833 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG635_800A6BAC[2] = { { NULL, NULL, 841 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG635_800A6BC4[2] = { { NULL, NULL, 849 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG635_800A6BDC[6] = {
    { D_WSTAG635_800A6A44, D_WSTAG635_800A6A4C, 845 }, { D_WSTAG635_800A6A54, NULL, 851 },
    { D_WSTAG635_800A6A60, NULL, 846 }, { D_WSTAG635_800A6A70, D_WSTAG635_800A6A84, 847 },
    { D_WSTAG635_800A6A8C, NULL, 852 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG635_800A6C24[3] = {
    { D_WSTAG635_800A6AA0, D_WSTAG635_800A6AA8, 848 }, { D_WSTAG635_800A6AB4, NULL, 850 }, { NULL, NULL, 0 },
};
u16 D_WSTAG635_800A6C48[12] = {
    0x7093, 1, 0x701A, 0, 0x8013, 0, 0x7E00, 1,
    0x7E1E, 1, 0xFFFF, 0,
};
u16 D_WSTAG635_800A6C60[12] = {
    0x7093, 1, 0x701A, 0, 0x8013, 1, 0x7E1E, 1,
    0x7E00, 1, 0xFFFF, 0,
};
u16 D_WSTAG635_800A6C78[12] = {
    0x7093, 1, 0x701A, 0, 0x818B, 0, 0x7E21, 1,
    0x7E00, 1, 0xFFFF, 0,
};
u16 D_WSTAG635_800A6C90[12] = {
    0x7093, 1, 0x701A, 0, 0x818B, 1, 0x7E21, 1,
    0x7E00, 1, 0xFFFF, 0,
};
u16 D_WSTAG635_800A6CA8[8] = { 0x701A, 1, 0x7E1E, 1, 0x7E00, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A6CB8[8] = { 0x701A, 1, 0x7E21, 1, 0x7E00, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A6CC8[8] = { 0x701A, 1, 0x7E00, 1, 0x7E24, 1, 0xFFFF, 0 };
u16 D_WSTAG635_800A6CD8[12] = {
    0x7093, 1, 0x701A, 0, 0x8168, 0, 0x7E24, 1,
    0x7E00, 1, 0xFFFF, 0,
};
u16 D_WSTAG635_800A6CF0[12] = {
    0x7093, 1, 0x701A, 0, 0x8168, 1, 0x7E24, 1,
    0x7E00, 1, 0xFFFF, 0,
};
FieldstgPlacedActor D_WSTAG635_800A6D08 = { D_WSTAG635_800A6C48, D_WSTAG635_800A6ABC, 43, 4, 480, 696, 7 };
FieldstgPlacedActor D_WSTAG635_800A6D1C = { D_WSTAG635_800A6C60, D_WSTAG635_800A6B04, 43, 4, 480, 696, 7 };
FieldstgPlacedActor D_WSTAG635_800A6D30 = { D_WSTAG635_800A6C78, D_WSTAG635_800A6B28, 44, 5, 800, 520, 1 };
FieldstgPlacedActor D_WSTAG635_800A6D44 = { D_WSTAG635_800A6C90, D_WSTAG635_800A6B70, 44, 5, 800, 520, 1 };
FieldstgPlacedActor D_WSTAG635_800A6D58 = { D_WSTAG635_800A6CA8, D_WSTAG635_800A6B94, 157, 6, 480, 696, 7 };
FieldstgPlacedActor D_WSTAG635_800A6D6C = { D_WSTAG635_800A6CB8, D_WSTAG635_800A6BAC, 158, 7, 800, 520, 1 };
FieldstgPlacedActor D_WSTAG635_800A6D80 = { D_WSTAG635_800A6CC8, D_WSTAG635_800A6BC4, 159, 8, 408, 364, 7 };
FieldstgPlacedActor D_WSTAG635_800A6D94 = { D_WSTAG635_800A6CD8, D_WSTAG635_800A6BDC, 163, 9, 408, 364, 7 };
FieldstgPlacedActor D_WSTAG635_800A6DA8 = { D_WSTAG635_800A6CF0, D_WSTAG635_800A6C24, 163, 9, 408, 364, 7 };
FieldstgPlacedActor *wstag635_actors[10] = {
    &D_WSTAG635_800A6D08, &D_WSTAG635_800A6D1C, &D_WSTAG635_800A6D30, &D_WSTAG635_800A6D44, &D_WSTAG635_800A6D58,
    &D_WSTAG635_800A6D6C, &D_WSTAG635_800A6D80, &D_WSTAG635_800A6D94, &D_WSTAG635_800A6DA8, NULL,
};
FieldstgSprite wstag635_sprites[19] = {
    { 1, 3, 0xC8, 2, 0x48, 0, 0, 0, 0, 0, 50, 640, 0, 0 }, { 1, 3, 0xC8, 2, 0x48, 0, 0, 0, 0, 0, 348, 103, 0, 0 },
    { 1, 3, 0xC8, 2, 0x48, 0, 0, 0, 0, 0, 369, 851, 0, 0 }, { 1, 0, 0x40, 2, 7, 1, 7, 0xC, 4, 0, 570, 141, 0, 0 },
    { 1, 0, 0x40, 2, 7, 1, 7, 0xC, 4, 0, 906, 499, 0, 0 }, { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 56, 957, 0, 0 },
    { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 92, 232, 0, 0 }, { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 113, 400, 0, 0 },
    { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 340, 812, 0, 0 }, { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 891, 290, 0, 0 },
    { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 1096, 196, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 417, 278, 329, 0 },
    { 1, 0, 0x48, 4, 1, 0, 0, 0, 0, 0, 348, 278, 342, 0 }, { 1, 0, 0x4A, 4, 2, 0, 0, 0, 0, 0, 529, 611, 680, 0 },
    { 1, 0, 0x5A, 4, 3, 0, 0, 0, 0, 0, 422, 570, 656, 0 }, { 1, 0, 0x58, 4, 4, 0, 0, 0, 0, 0, 384, 635, 719, 0 },
    { 1, 0, 0x60, 4, 5, 0, 0, 0, 0, 0, 919, 519, 608, 0 }, { 1, 0, 0x5E, 4, 6, 0, 0, 0, 0, 0, 583, 159, 248, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag635_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x258, 0x340, 0xF0, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x258, 0x368, 0x2EC, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x258, 0x128, 0x2F4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x258, 0x110, 0x108, 3, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag635_funcs = { wstag635_setup };
