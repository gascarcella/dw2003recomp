#include "wstag.h"

/* WSTAG640: stage 0x259 (fieldstg_stages). */

extern WstagAnimKey D_WSTAG640_800A6354[];
extern WstagAnimKey D_WSTAG640_800A6388[];
extern WstagAnimKey D_WSTAG640_800A63BC[];
extern WstagExits *wstag640_exits[];
extern WstagFuncs wstag640_funcs;
extern FieldstgBattleLists wstag640_battle_lists;
extern FieldstgVramPlace wstag640_vram_places[];
extern FieldstgSprite wstag640_sprites[];
extern FieldstgMapEvent wstag640_map_events[];

s32 wstag640_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
        wstag640_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag640_anim_update(WstagAnimObject *obj) {
    FieldstgSprite *sprite;
    s32 frames[3];

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG640_800A6354[0].time;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG640_800A6388[0].time;
        obj->anims[2].key = 0;
        obj->anims[2].time = D_WSTAG640_800A63BC[0].time;
        break;
    case OBJECT_STATE_RUN:
        sprite = fieldstg_stage.sprites;
        frames[0] = wstag640_anim_loop(&obj->anims[0], D_WSTAG640_800A6354, 0);
        frames[1] = wstag640_anim_loop(&obj->anims[1], D_WSTAG640_800A6388, 0);
        frames[2] = wstag640_anim_loop(&obj->anims[2], D_WSTAG640_800A63BC, 0);
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

WstagAnimObject *wstag640_anim_new(void) {
    return object_new(wstag640_anim_update, sizeof(WstagAnimObject), 0);
}

s32 wstag640_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag640_set_exits(dst, list + 1, arg2, arg3);
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

void wstag640_update(WstagObject *obj, WstagAnimObject **data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        *data = wstag640_anim_new();
        wstag640_set_exits(fieldstg_stage.map_events, wstag640_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag640_start(void *arg0) {
    WstagObject *obj = object_new(wstag640_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag640_funcs.setup();
    return obj;
}

void wstag640_setup(void) {
    fieldstg_stage.background_file = 0x49F;
    fieldstg_stage.sprite_file = 0x04A00000;
    fieldstg_stage.sprites = wstag640_sprites;
    fieldstg_stage.map_events = wstag640_map_events;
    fieldstg_stage.mask_file = 0x49E;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1F400, 0x32000 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag640_vram_places;
    fieldstg_stage.music = 0x38;
    fieldstg_stage.sound = 0x60E00000;
    fieldstg_stage.battle_lists = &wstag640_battle_lists;
    fieldstg_attr.set_file(0, 0x04A00001);
    fieldstg_attr.set_file(7, 0x04A00002);
    fieldstg_attr.set_file(4, 0x04A00003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag640_setup(void);

WstagAnimKey D_WSTAG640_800A6354[13] = {
    { 50, 8 }, { 51, 8 }, { 52, 8 }, { 53, 8 }, { 54, 8 }, { 55, 8 }, { 56, 8 }, { 57, 8 }, { 58, 8 }, { 59, 8 },
    { 60, 8 }, { 82, 160 }, { 255, 0 },
};
WstagAnimKey D_WSTAG640_800A6388[13] = {
    { 61, 8 }, { 62, 8 }, { 63, 8 }, { 64, 8 }, { 65, 8 }, { 66, 8 }, { 67, 8 }, { 68, 8 }, { 69, 8 }, { 70, 8 },
    { 71, 8 }, { 82, 160 }, { 255, 0 },
};
WstagAnimKey D_WSTAG640_800A63BC[12] = {
    { 72, 8 }, { 73, 8 }, { 74, 8 }, { 75, 8 }, { 76, 8 }, { 77, 8 }, { 78, 8 }, { 79, 8 }, { 80, 8 }, { 81, 8 },
    { 82, 160 }, { 255, 0 },
};
WstagExit D_WSTAG640_800A63EC = { 0x258, 1, 1, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG640_800A63FC = { 0x258, 1, 3, 0x340, 0xF0, 1, &D_WSTAG640_800A63EC };
WstagExit D_WSTAG640_800A640C = { 0x258, 1, 2, 0x110, 0x108, 7, &D_WSTAG640_800A63FC };
WstagExit D_WSTAG640_800A641C = { 0x257, 0, 0, 0x100, 0x278, 5, &D_WSTAG640_800A640C };
WstagExits D_WSTAG640_800A642C = { 1, 1, &D_WSTAG640_800A641C };
WstagExit D_WSTAG640_800A6434 = { 0x258, 1, 2, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG640_800A6444 = { 0x258, 1, 4, 0x340, 0xF0, 1, &D_WSTAG640_800A6434 };
WstagExit D_WSTAG640_800A6454 = { 0x258, 1, 1, 0x110, 0x108, 7, &D_WSTAG640_800A6444 };
WstagExit D_WSTAG640_800A6464 = { 0x257, 0, 0, 0x100, 0x278, 5, &D_WSTAG640_800A6454 };
WstagExits D_WSTAG640_800A6474 = { 1, 2, &D_WSTAG640_800A6464 };
WstagExit D_WSTAG640_800A647C = { 0x258, 1, 4, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG640_800A648C = { 0x258, 1, 5, 0x340, 0xF0, 1, &D_WSTAG640_800A647C };
WstagExit D_WSTAG640_800A649C = { 0x258, 1, 3, 0x110, 0x108, 7, &D_WSTAG640_800A648C };
WstagExit D_WSTAG640_800A64AC = { 0x258, 1, 1, 0x128, 0x2F4, 5, &D_WSTAG640_800A649C };
WstagExits D_WSTAG640_800A64BC = { 1, 3, &D_WSTAG640_800A64AC };
WstagExit D_WSTAG640_800A64C4 = { 0x258, 1, 3, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG640_800A64D4 = { 0x258, 1, 6, 0x340, 0xF0, 1, &D_WSTAG640_800A64C4 };
WstagExit D_WSTAG640_800A64E4 = { 0x258, 1, 4, 0x110, 0x108, 7, &D_WSTAG640_800A64D4 };
WstagExit D_WSTAG640_800A64F4 = { 0x258, 1, 2, 0x128, 0x2F4, 5, &D_WSTAG640_800A64E4 };
WstagExits D_WSTAG640_800A6504 = { 1, 4, &D_WSTAG640_800A64F4 };
WstagExit D_WSTAG640_800A650C = { 0x258, 1, 5, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG640_800A651C = { 0x258, 1, 7, 0x340, 0xF0, 1, &D_WSTAG640_800A650C };
WstagExit D_WSTAG640_800A652C = { 0x258, 1, 6, 0x110, 0x108, 7, &D_WSTAG640_800A651C };
WstagExit D_WSTAG640_800A653C = { 0x258, 1, 3, 0x128, 0x2F4, 5, &D_WSTAG640_800A652C };
WstagExits D_WSTAG640_800A654C = { 1, 5, &D_WSTAG640_800A653C };
WstagExit D_WSTAG640_800A6554 = { 0x258, 1, 6, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG640_800A6564 = { 0x258, 1, 8, 0x340, 0xF0, 1, &D_WSTAG640_800A6554 };
WstagExit D_WSTAG640_800A6574 = { 0x258, 1, 5, 0x110, 0x108, 7, &D_WSTAG640_800A6564 };
WstagExit D_WSTAG640_800A6584 = { 0x258, 1, 4, 0x128, 0x2F4, 5, &D_WSTAG640_800A6574 };
WstagExits D_WSTAG640_800A6594 = { 1, 6, &D_WSTAG640_800A6584 };
WstagExit D_WSTAG640_800A659C = { 0x258, 1, 8, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG640_800A65AC = { 0x258, 1, 1, 0x340, 0xF0, 1, &D_WSTAG640_800A659C };
WstagExit D_WSTAG640_800A65BC = { 0x258, 1, 7, 0x110, 0x108, 7, &D_WSTAG640_800A65AC };
WstagExit D_WSTAG640_800A65CC = { 0x258, 1, 5, 0x128, 0x2F4, 5, &D_WSTAG640_800A65BC };
WstagExits D_WSTAG640_800A65DC = { 1, 7, &D_WSTAG640_800A65CC };
WstagExit D_WSTAG640_800A65E4 = { 0x258, 1, 7, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG640_800A65F4 = { 0x258, 1, 2, 0x340, 0xF0, 1, &D_WSTAG640_800A65E4 };
WstagExit D_WSTAG640_800A6604 = { 0x258, 1, 8, 0x110, 0x108, 7, &D_WSTAG640_800A65F4 };
WstagExit D_WSTAG640_800A6614 = { 0x258, 1, 6, 0x128, 0x2F4, 5, &D_WSTAG640_800A6604 };
WstagExits D_WSTAG640_800A6624 = { 1, 8, &D_WSTAG640_800A6614 };
WstagExits D_WSTAG640_800A662C = { 0, 0, &D_WSTAG640_800A641C };
WstagExits *wstag640_exits[10] = {
    &D_WSTAG640_800A642C, &D_WSTAG640_800A6474, &D_WSTAG640_800A64BC, &D_WSTAG640_800A6504, &D_WSTAG640_800A654C,
    &D_WSTAG640_800A6594, &D_WSTAG640_800A65DC, &D_WSTAG640_800A6624, &D_WSTAG640_800A662C, NULL,
};
FieldstgListedBattle D_WSTAG640_800A665C = { 73, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG640_800A6668 = { 73, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG640_800A6674 = { 73, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG640_800A6680 = { 74, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG640_800A668C = { 74, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG640_800A6698 = { 74, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG640_800A66A4 = { 156, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG640_800A66B0 = { 156, 5, 0x60080000 };
FieldstgBattleList D_WSTAG640_800A66BC = {
    3,
    { &D_WSTAG640_800A665C, &D_WSTAG640_800A6668, &D_WSTAG640_800A6674, &D_WSTAG640_800A6680, &D_WSTAG640_800A668C,
        &D_WSTAG640_800A6698, &D_WSTAG640_800A66A4, &D_WSTAG640_800A66B0 },
};
FieldstgListedBattle D_WSTAG640_800A66E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A66EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A66F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A6704 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A6710 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A671C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A6728 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A6734 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG640_800A6740 = {
    0,
    { &D_WSTAG640_800A66E0, &D_WSTAG640_800A66EC, &D_WSTAG640_800A66F8, &D_WSTAG640_800A6704, &D_WSTAG640_800A6710,
        &D_WSTAG640_800A671C, &D_WSTAG640_800A6728, &D_WSTAG640_800A6734 },
};
FieldstgListedBattle D_WSTAG640_800A6764 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A6770 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A677C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A6788 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A6794 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A67A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A67AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A67B8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG640_800A67C4 = {
    0,
    { &D_WSTAG640_800A6764, &D_WSTAG640_800A6770, &D_WSTAG640_800A677C, &D_WSTAG640_800A6788, &D_WSTAG640_800A6794,
        &D_WSTAG640_800A67A0, &D_WSTAG640_800A67AC, &D_WSTAG640_800A67B8 },
};
FieldstgListedBattle D_WSTAG640_800A67E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A67F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A6800 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A680C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A6818 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A6824 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A6830 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG640_800A683C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG640_800A6848 = {
    0,
    { &D_WSTAG640_800A67E8, &D_WSTAG640_800A67F4, &D_WSTAG640_800A6800, &D_WSTAG640_800A680C, &D_WSTAG640_800A6818,
        &D_WSTAG640_800A6824, &D_WSTAG640_800A6830, &D_WSTAG640_800A683C },
};
FieldstgBattleLists wstag640_battle_lists = {
    38, 0, 0, { &D_WSTAG640_800A66BC, &D_WSTAG640_800A6740, &D_WSTAG640_800A67C4 }, &D_WSTAG640_800A6848,
};
FieldstgVramPlace wstag640_vram_places[6] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
};
FieldstgSprite wstag640_sprites[26] = {
    { 1, 3, 0xC8, 2, 0x48, 0, 0, 0, 0, 0, 384, 97, 0, 0 }, { 1, 3, 0xC8, 2, 0x48, 0, 0, 0, 0, 0, 384, 871, 0, 0 },
    { 1, 3, 0xC8, 2, 0x48, 0, 0, 0, 0, 0, 916, 384, 0, 0 }, { 1, 0, 0x40, 2, 8, 1, 8, 0xD, 4, 0, 328, 532, 0, 0 },
    { 1, 0, 0x40, 2, 8, 1, 8, 0xD, 4, 0, 375, 220, 0, 0 }, { 1, 0, 0x40, 2, 8, 1, 8, 0xD, 4, 0, 519, 660, 0, 0 },
    { 1, 0, 0x40, 2, 8, 1, 8, 0xD, 4, 0, 536, 283, 0, 0 }, { 1, 0, 0x40, 2, 8, 1, 8, 0xD, 4, 0, 583, 454, 0, 0 },
    { 1, 0, 0x40, 2, 8, 1, 8, 0xD, 4, 0, 711, 612, 0, 0 }, { 1, 0, 0x40, 2, 8, 1, 8, 0xD, 4, 0, 728, 267, 0, 0 },
    { 1, 0, 0x40, 2, 8, 1, 8, 0xD, 4, 0, 791, 460, 0, 0 }, { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 166, 937, 0, 0 },
    { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 171, 489, 0, 0 }, { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 184, 363, 0, 0 },
    { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 335, 801, 0, 0 }, { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 933, 296, 0, 0 },
    { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 968, 498, 0, 0 }, { 1, 0, 0x60, 4, 0, 0, 0, 0, 0, 0, 724, 631, 721, 0 },
    { 1, 0, 0x60, 4, 1, 0, 0, 0, 0, 0, 532, 679, 769, 0 }, { 1, 0, 0x60, 4, 2, 0, 0, 0, 0, 0, 340, 551, 641, 0 },
    { 1, 0, 0x60, 4, 3, 0, 0, 0, 0, 0, 804, 479, 569, 0 }, { 1, 0, 0x60, 4, 4, 0, 0, 0, 0, 0, 596, 471, 562, 0 },
    { 1, 0, 0x60, 4, 5, 0, 0, 0, 0, 0, 740, 287, 378, 0 }, { 1, 0, 0x60, 4, 6, 0, 0, 0, 0, 0, 548, 303, 393, 0 },
    { 1, 0, 0x60, 4, 7, 0, 0, 0, 0, 0, 388, 239, 329, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag640_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x259, 0x358, 0xFC, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x259, 0x350, 0x2F8, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x259, 0x118, 0x2EC, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x259, 0x120, 0x100, 3, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag640_funcs = { wstag640_setup };
