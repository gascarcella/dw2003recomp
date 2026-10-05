#include "wstag.h"

/* WSTAG480: stage 0x23A (fieldstg_stages). */

extern WstagFuncs wstag480_funcs;
void wstag480_update();
extern WstagSeqKey **D_WSTAG480_800A685C[];
extern const CVECTOR wstag480_color;
extern FieldstgBattleLists wstag480_battle_lists[];
extern FieldstgVramPlace wstag480_vram_places[];
extern FieldstgPlacedActor *wstag480_actors[];
extern FieldstgSprite wstag480_sprites[];
extern FieldstgMapEvent wstag480_map_events[];
extern FieldstgEventDef wstag480_events[];
void wstag480_seq_update(WstagSeqObject *obj);

void wstag480_seq_anim_advance(FieldstgSprite *sprite, WstagSeqKey **seqs, WstagSeqAnim *anim, s32 depth) {
    s32 step;

    if (depth == 0) {
        step = gfx_module.funcs.get_frame_ticks();
        if (step >= 5) {
            step = 4;
        }
        anim->time -= step;
    }
    if (anim->time <= 0) {
        if (seqs[anim->seq][anim->key].last) {
            anim->seq++;
            anim->key = 0;
            if (seqs[anim->seq] == NULL) {
                anim->seq = 0;
            }
        } else {
            anim->key++;
        }
        anim->time += seqs[anim->seq][anim->key].time;
        wstag480_seq_anim_advance(sprite, seqs, anim, depth + 1);
    }
    if (depth == 0) {
        sprite->sprite = seqs[anim->seq][anim->key].sprite;
        sprite->frame = seqs[anim->seq][anim->key].frame;
    }
}

void wstag480_seq_update(WstagSeqObject *obj) {
    FieldstgSprite *sprite;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->anims[0].seq = 0;
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG480_800A685C[0][0][0].time;
        obj->anims[1].seq = 0;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG480_800A685C[1][0][0].time;
        obj->anims[2].seq = 0;
        obj->anims[2].key = 0;
        obj->anims[2].time = D_WSTAG480_800A685C[2][0][0].time;
        obj->anims[3].seq = 0;
        obj->anims[3].key = 0;
        obj->anims[3].time = D_WSTAG480_800A685C[3][0][0].time;
        obj->anims[4].seq = 0;
        obj->anims[4].key = 0;
        obj->anims[4].time = D_WSTAG480_800A685C[3][0][0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            switch (sprite->type) {
            case 1:
                wstag480_seq_anim_advance(sprite, D_WSTAG480_800A685C[0], &obj->anims[0], 0);
                break;
            case 2:
                wstag480_seq_anim_advance(sprite, D_WSTAG480_800A685C[1], &obj->anims[1], 0);
                break;
            case 3:
                wstag480_seq_anim_advance(sprite, D_WSTAG480_800A685C[2], &obj->anims[2], 0);
                break;
            case 4:
                wstag480_seq_anim_advance(sprite, D_WSTAG480_800A685C[3], &obj->anims[3], 0);
                break;
            case 5:
                wstag480_seq_anim_advance(sprite, D_WSTAG480_800A685C[4], &obj->anims[4], 0);
                break;
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}


Object *wstag480_seq_create(void) {
    return object_new(wstag480_seq_update, 0x78, 0);
}

void wstag480_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->object = wstag480_seq_create();
        if (gamestate_data.progress == 7 && gamestate_flags.get_flag(0x4000, 1)) {
            data->event = fieldstg_event_start(0xAB);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag480_start(void *arg0) {
    WstagObject *obj = object_new(wstag480_update, sizeof(WstagObject), sizeof(WstagEventData));

    obj->manager = arg0;
    wstag480_funcs.setup();
    return obj;
}

void wstag480_event_170_end(void) {
    gamestate_flags.set_flag(0x4000, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag480_event_171_end(void) {
    gamestate_data.progress = 0x8;
}

void wstag480_setup(void) {
    fieldstg_stage.background_file = 0x365;
    fieldstg_stage.sprite_file = 0x03660000;
    fieldstg_stage.sprites = wstag480_sprites;
    fieldstg_stage.map_events = wstag480_map_events;
    fieldstg_stage.mask_file = 0x638;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1F000, 0x34400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag480_vram_places;
    fieldstg_stage.music = 0x11;
    fieldstg_stage.sound = 0x60440000;
    fieldstg_stage.actors = wstag480_actors;
    fieldstg_stage.color = wstag480_color;
    fieldstg_stage.events = wstag480_events;
    fieldstg_stage.battle_lists = wstag480_battle_lists;
    fieldstg_attr.set_file(0, 0x03660001);
    fieldstg_attr.set_file(1, 0x03660003);
    fieldstg_attr.set_file(7, 0x03660002);
    fieldstg_attr.set_file(4, 0x03660004);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress < 0xA) {
        fieldstg_stage.battle_lists = wstag480_battle_lists;
    } else if (gamestate_data.progress < 0x18) {
        fieldstg_stage.battle_lists = &wstag480_battle_lists[1];
    } else {
        fieldstg_stage.battle_lists = &wstag480_battle_lists[2];
    }
}

const CVECTOR wstag480_color = { 0x80, 0x80, 0x80, 0 };

/* The stage's .data (tools/wstag_data.py). */
void wstag480_setup(void);

s16 D_WSTAG480_800A6478[88] = {
    FIELDSTG_EVENT_WALK(2, 312, 821, 3),
    FIELDSTG_EVENT_PLACE(133, 280, 805),
    FIELDSTG_EVENT_ANIM(133, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 133),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 133),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 133, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 133, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG480_800A6528[62] = {
    FIELDSTG_EVENT_PLACE(2, 312, 821),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(133, 280, 805),
    FIELDSTG_EVENT_ANIM(133, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 133, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 133, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 312, 852, 0),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 344, 868, 7),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_GOTO_MAP(0x234, 304, 864, 5),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG480_800A65A4[108] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WALK(2, 312, 821, 3),
    FIELDSTG_EVENT_PLACE(133, 280, 805),
    FIELDSTG_EVENT_ANIM(133, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 133, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 133),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 133),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 133, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 6, 133, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(133, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(133, 128, 729, 3),
    FIELDSTG_EVENT_WAIT_WALK(133),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 3),
    FIELDSTG_EVENT_PLACE(133, 0, 0),
    FIELDSTG_EVENT_ANIM(133, 1, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
WstagSeqKey D_WSTAG480_800A667C[4] = {
    { 0x40, 0xA, 0, 0 }, { 0x40, 0xA, 1, 0 }, { 0x40, 0xA, 2, 0 }, { 0x40, 0xA, 1, 1 },
};
WstagSeqKey D_WSTAG480_800A668C[6] = {
    { 0x41, 4, 2, 0 }, { 0x41, 4, 4, 0 }, { 0x41, 4, 6, 0 }, { 0x41, 4, 4, 0 }, { 0x41, 4, 2, 0 },
    { 0x41, 4, 0, 1 },
};
WstagSeqKey D_WSTAG480_800A66A4[4] = {
    { 0x42, 0xA, 0, 0 }, { 0x42, 0xA, 1, 0 }, { 0x42, 0xA, 2, 0 }, { 0x42, 0xA, 1, 1 },
};
WstagSeqKey D_WSTAG480_800A66B4[6] = {
    { 0x43, 4, 2, 0 }, { 0x43, 4, 4, 0 }, { 0x43, 4, 6, 0 }, { 0x43, 4, 4, 0 }, { 0x43, 4, 2, 0 },
    { 0x43, 4, 0, 1 },
};
WstagSeqKey D_WSTAG480_800A66CC[4] = {
    { 0x44, 0xA, 0, 0 }, { 0x44, 0xA, 1, 0 }, { 0x44, 0xA, 2, 0 }, { 0x44, 0xA, 1, 1 },
};
WstagSeqKey D_WSTAG480_800A66DC[6] = {
    { 0x45, 4, 2, 0 }, { 0x45, 4, 4, 0 }, { 0x45, 4, 6, 0 }, { 0x45, 4, 4, 0 }, { 0x45, 4, 2, 0 },
    { 0x45, 4, 0, 1 },
};
WstagSeqKey D_WSTAG480_800A66F4[4] = {
    { 0x46, 0xA, 0, 0 }, { 0x46, 0xA, 1, 0 }, { 0x46, 0xA, 2, 0 }, { 0x46, 0xA, 1, 1 },
};
WstagSeqKey D_WSTAG480_800A6704[6] = {
    { 0x47, 4, 2, 0 }, { 0x47, 4, 4, 0 }, { 0x47, 4, 6, 0 }, { 0x47, 4, 4, 0 }, { 0x47, 4, 2, 0 },
    { 0x47, 4, 0, 1 },
};
WstagSeqKey D_WSTAG480_800A671C[4] = {
    { 0x48, 0xA, 0, 0 }, { 0x48, 0xA, 1, 0 }, { 0x48, 0xA, 2, 0 }, { 0x48, 0xA, 1, 1 },
};
WstagSeqKey D_WSTAG480_800A672C[6] = {
    { 0x49, 4, 2, 0 }, { 0x49, 4, 4, 0 }, { 0x49, 4, 6, 0 }, { 0x49, 4, 4, 0 }, { 0x49, 4, 2, 0 },
    { 0x49, 4, 0, 1 },
};
WstagSeqKey *D_WSTAG480_800A6744[14] = {
    D_WSTAG480_800A667C, D_WSTAG480_800A667C, D_WSTAG480_800A667C, D_WSTAG480_800A667C, D_WSTAG480_800A667C,
    D_WSTAG480_800A667C, D_WSTAG480_800A667C, D_WSTAG480_800A667C, D_WSTAG480_800A667C, D_WSTAG480_800A667C,
    D_WSTAG480_800A667C, D_WSTAG480_800A667C, D_WSTAG480_800A668C, NULL,
};
WstagSeqKey *D_WSTAG480_800A677C[14] = {
    D_WSTAG480_800A66A4, D_WSTAG480_800A66A4, D_WSTAG480_800A66A4, D_WSTAG480_800A66A4, D_WSTAG480_800A66A4,
    D_WSTAG480_800A66A4, D_WSTAG480_800A66A4, D_WSTAG480_800A66A4, D_WSTAG480_800A66A4, D_WSTAG480_800A66A4,
    D_WSTAG480_800A66A4, D_WSTAG480_800A66A4, D_WSTAG480_800A66B4, NULL,
};
WstagSeqKey *D_WSTAG480_800A67B4[14] = {
    D_WSTAG480_800A66CC, D_WSTAG480_800A66CC, D_WSTAG480_800A66CC, D_WSTAG480_800A66CC, D_WSTAG480_800A66CC,
    D_WSTAG480_800A66CC, D_WSTAG480_800A66CC, D_WSTAG480_800A66CC, D_WSTAG480_800A66CC, D_WSTAG480_800A66CC,
    D_WSTAG480_800A66CC, D_WSTAG480_800A66CC, D_WSTAG480_800A66DC, NULL,
};
WstagSeqKey *D_WSTAG480_800A67EC[14] = {
    D_WSTAG480_800A66F4, D_WSTAG480_800A66F4, D_WSTAG480_800A66F4, D_WSTAG480_800A66F4, D_WSTAG480_800A66F4,
    D_WSTAG480_800A66F4, D_WSTAG480_800A66F4, D_WSTAG480_800A66F4, D_WSTAG480_800A66F4, D_WSTAG480_800A66F4,
    D_WSTAG480_800A66F4, D_WSTAG480_800A66F4, D_WSTAG480_800A6704, NULL,
};
WstagSeqKey *D_WSTAG480_800A6824[14] = {
    D_WSTAG480_800A671C, D_WSTAG480_800A671C, D_WSTAG480_800A671C, D_WSTAG480_800A671C, D_WSTAG480_800A671C,
    D_WSTAG480_800A671C, D_WSTAG480_800A671C, D_WSTAG480_800A671C, D_WSTAG480_800A671C, D_WSTAG480_800A671C,
    D_WSTAG480_800A671C, D_WSTAG480_800A671C, D_WSTAG480_800A672C, NULL,
};
WstagSeqKey * *D_WSTAG480_800A685C[5] = {
    D_WSTAG480_800A6744, D_WSTAG480_800A677C, D_WSTAG480_800A67B4, D_WSTAG480_800A67EC, D_WSTAG480_800A6824,
};
FieldstgListedBattle D_WSTAG480_800A6870 = { 91, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A687C = { 91, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6888 = { 91, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6894 = { 91, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A68A0 = { 91, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A68AC = { 91, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A68B8 = { 91, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A68C4 = { 91, 9, 0x60080000 };
FieldstgBattleList D_WSTAG480_800A68D0 = {
    4,
    { &D_WSTAG480_800A6870, &D_WSTAG480_800A687C, &D_WSTAG480_800A6888, &D_WSTAG480_800A6894, &D_WSTAG480_800A68A0,
        &D_WSTAG480_800A68AC, &D_WSTAG480_800A68B8, &D_WSTAG480_800A68C4 },
};
FieldstgListedBattle D_WSTAG480_800A68F4 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6900 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A690C = { 54, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6918 = { 54, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6924 = { 91, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6930 = { 91, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A693C = { 91, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6948 = { 91, 8, 0x60080000 };
FieldstgBattleList D_WSTAG480_800A6954 = {
    1,
    { &D_WSTAG480_800A68F4, &D_WSTAG480_800A6900, &D_WSTAG480_800A690C, &D_WSTAG480_800A6918, &D_WSTAG480_800A6924,
        &D_WSTAG480_800A6930, &D_WSTAG480_800A693C, &D_WSTAG480_800A6948 },
};
FieldstgListedBattle D_WSTAG480_800A6978 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6984 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6990 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A699C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A69A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A69B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A69C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A69CC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG480_800A69D8 = {
    0,
    { &D_WSTAG480_800A6978, &D_WSTAG480_800A6984, &D_WSTAG480_800A6990, &D_WSTAG480_800A699C, &D_WSTAG480_800A69A8,
        &D_WSTAG480_800A69B4, &D_WSTAG480_800A69C0, &D_WSTAG480_800A69CC },
};
FieldstgListedBattle D_WSTAG480_800A69FC = { 11, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG480_800A6A08 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6A14 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6A20 = { 329, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6A2C = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6A38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6A44 = { 48, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6A50 = { 66, 8, 0x60080000 };
FieldstgBattleList D_WSTAG480_800A6A5C = {
    0,
    { &D_WSTAG480_800A69FC, &D_WSTAG480_800A6A08, &D_WSTAG480_800A6A14, &D_WSTAG480_800A6A20, &D_WSTAG480_800A6A2C,
        &D_WSTAG480_800A6A38, &D_WSTAG480_800A6A44, &D_WSTAG480_800A6A50 },
};
FieldstgListedBattle D_WSTAG480_800A6A80 = { 38, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6A8C = { 38, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6A98 = { 38, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6AA4 = { 38, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6AB0 = { 55, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6ABC = { 55, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6AC8 = { 55, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6AD4 = { 55, 9, 0x60080000 };
FieldstgBattleList D_WSTAG480_800A6AE0 = {
    3,
    { &D_WSTAG480_800A6A80, &D_WSTAG480_800A6A8C, &D_WSTAG480_800A6A98, &D_WSTAG480_800A6AA4, &D_WSTAG480_800A6AB0,
        &D_WSTAG480_800A6ABC, &D_WSTAG480_800A6AC8, &D_WSTAG480_800A6AD4 },
};
FieldstgListedBattle D_WSTAG480_800A6B04 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6B10 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6B1C = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6B28 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6B34 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6B40 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6B4C = { 54, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6B58 = { 54, 8, 0x60080000 };
FieldstgBattleList D_WSTAG480_800A6B64 = {
    2,
    { &D_WSTAG480_800A6B04, &D_WSTAG480_800A6B10, &D_WSTAG480_800A6B1C, &D_WSTAG480_800A6B28, &D_WSTAG480_800A6B34,
        &D_WSTAG480_800A6B40, &D_WSTAG480_800A6B4C, &D_WSTAG480_800A6B58 },
};
FieldstgListedBattle D_WSTAG480_800A6B88 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6B94 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6BA0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6BAC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6BB8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6BC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6BD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6BDC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG480_800A6BE8 = {
    0,
    { &D_WSTAG480_800A6B88, &D_WSTAG480_800A6B94, &D_WSTAG480_800A6BA0, &D_WSTAG480_800A6BAC, &D_WSTAG480_800A6BB8,
        &D_WSTAG480_800A6BC4, &D_WSTAG480_800A6BD0, &D_WSTAG480_800A6BDC },
};
FieldstgListedBattle D_WSTAG480_800A6C0C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6C18 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6C24 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6C30 = { 329, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6C3C = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6C48 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6C54 = { 48, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6C60 = { 66, 8, 0x60080000 };
FieldstgBattleList D_WSTAG480_800A6C6C = {
    0,
    { &D_WSTAG480_800A6C0C, &D_WSTAG480_800A6C18, &D_WSTAG480_800A6C24, &D_WSTAG480_800A6C30, &D_WSTAG480_800A6C3C,
        &D_WSTAG480_800A6C48, &D_WSTAG480_800A6C54, &D_WSTAG480_800A6C60 },
};
FieldstgListedBattle D_WSTAG480_800A6C90 = { 38, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6C9C = { 55, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6CA8 = { 56, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6CB4 = { 56, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6CC0 = { 56, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6CCC = { 56, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6CD8 = { 56, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6CE4 = { 56, 9, 0x60080000 };
FieldstgBattleList D_WSTAG480_800A6CF0 = {
    3,
    { &D_WSTAG480_800A6C90, &D_WSTAG480_800A6C9C, &D_WSTAG480_800A6CA8, &D_WSTAG480_800A6CB4, &D_WSTAG480_800A6CC0,
        &D_WSTAG480_800A6CCC, &D_WSTAG480_800A6CD8, &D_WSTAG480_800A6CE4 },
};
FieldstgListedBattle D_WSTAG480_800A6D14 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6D20 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6D2C = { 60, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6D38 = { 60, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6D44 = { 60, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6D50 = { 60, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6D5C = { 60, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6D68 = { 60, 8, 0x60080000 };
FieldstgBattleList D_WSTAG480_800A6D74 = {
    2,
    { &D_WSTAG480_800A6D14, &D_WSTAG480_800A6D20, &D_WSTAG480_800A6D2C, &D_WSTAG480_800A6D38, &D_WSTAG480_800A6D44,
        &D_WSTAG480_800A6D50, &D_WSTAG480_800A6D5C, &D_WSTAG480_800A6D68 },
};
FieldstgListedBattle D_WSTAG480_800A6D98 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6DA4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6DB0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6DBC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6DC8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6DD4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6DE0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6DEC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG480_800A6DF8 = {
    0,
    { &D_WSTAG480_800A6D98, &D_WSTAG480_800A6DA4, &D_WSTAG480_800A6DB0, &D_WSTAG480_800A6DBC, &D_WSTAG480_800A6DC8,
        &D_WSTAG480_800A6DD4, &D_WSTAG480_800A6DE0, &D_WSTAG480_800A6DEC },
};
FieldstgListedBattle D_WSTAG480_800A6E1C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6E28 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6E34 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6E40 = { 329, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6E4C = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6E58 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG480_800A6E64 = { 48, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG480_800A6E70 = { 66, 8, 0x60080000 };
FieldstgBattleList D_WSTAG480_800A6E7C = {
    0,
    { &D_WSTAG480_800A6E1C, &D_WSTAG480_800A6E28, &D_WSTAG480_800A6E34, &D_WSTAG480_800A6E40, &D_WSTAG480_800A6E4C,
        &D_WSTAG480_800A6E58, &D_WSTAG480_800A6E64, &D_WSTAG480_800A6E70 },
};
FieldstgBattleLists wstag480_battle_lists[3] = {
    { 17, 0, 0, { &D_WSTAG480_800A68D0, &D_WSTAG480_800A6954, &D_WSTAG480_800A69D8 }, &D_WSTAG480_800A6A5C },
    { 18, 1, 0, { &D_WSTAG480_800A6AE0, &D_WSTAG480_800A6B64, &D_WSTAG480_800A6BE8 }, &D_WSTAG480_800A6C6C },
    { 56, 2, 0, { &D_WSTAG480_800A6CF0, &D_WSTAG480_800A6D74, &D_WSTAG480_800A6DF8 }, &D_WSTAG480_800A6E7C },
};
FieldstgVramPlace wstag480_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 256, 192, 0, 352, 506 },
};
u16 D_WSTAG480_800A6F64[4] = { 0x6007, 1, 0xFFFF, 0 };
u16 D_WSTAG480_800A6F6C[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_WSTAG480_800A6F74[4] = { 0x6009, 1, 0xFFFF, 0 };
u16 D_WSTAG480_800A6F7C[4] = { 0x600A, 1, 0xFFFF, 0 };
u16 D_WSTAG480_800A6F84[6] = { 0x1C09, 1, 0x901F, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG480_800A6F90[5] = {
    { D_WSTAG480_800A6F64, NULL, 581 }, { D_WSTAG480_800A6F6C, NULL, 581 }, { D_WSTAG480_800A6F74, NULL, 582 },
    { D_WSTAG480_800A6F7C, D_WSTAG480_800A6F84, 583 }, { NULL, NULL, 0 },
};
u16 D_WSTAG480_800A6FCC[4] = { 0x1C09, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG480_800A6FD4 = { D_WSTAG480_800A6FCC, D_WSTAG480_800A6F90, 133, 4, 280, 805, 7 };
FieldstgPlacedActor *wstag480_actors[2] = { &D_WSTAG480_800A6FD4, NULL };
FieldstgSprite wstag480_sprites[239] = {
    { 1, 1, 0x40, 2, 0x40, 0, 0, 0, 0, 0, 310, 197, 0, 0 }, { 1, 2, 0x40, 2, 0x42, 0, 0, 0, 0, 0, 381, 208, 0, 0 },
    { 1, 3, 0x40, 2, 0x44, 0, 0, 0, 0, 0, 416, 204, 0, 0 }, { 1, 5, 0x40, 2, 0x48, 0, 0, 0, 0, 0, 464, 258, 0, 0 },
    { 1, 4, 0x40, 2, 0x46, 0, 0, 0, 0, 0, 468, 206, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x37, 8, 0, 191, 408, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x37, 8, 0, 564, 488, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x37, 8, 0, 594, 498, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x37, 8, 0, 724, 120, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x37, 8, 0, 772, 419, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x37, 8, 0, 804, 410, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x37, 8, 0, 901, 541, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 1, 0x38, 0x3D, 8, 0, 977, 774, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 1, 0x38, 0x3D, 8, 0, 986, 772, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 1, 0x38, 0x3D, 8, 0, 995, 783, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 1, 0x38, 0x3D, 8, 0, 1010, 771, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 1, 0x38, 0x3D, 8, 0, 1017, 771, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 1, 0x38, 0x3D, 8, 0, 1024, 784, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 1, 0x38, 0x3D, 8, 0, 1047, 799, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 1, 0x38, 0x3D, 8, 0, 1057, 791, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 1, 0x38, 0x3D, 8, 0, 1066, 796, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 1, 0x38, 0x3D, 8, 0, 1081, 795, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 1, 0x38, 0x3D, 8, 0, 1088, 789, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 1, 0x38, 0x3D, 8, 0, 1096, 796, 0, 0 },
    { 1, 0, 0x40, 2, 0x3E, 2, 0, 0xF, 6, 0, 79, 617, 0, 0 },
    { 1, 0, 0x40, 2, 0x3E, 2, 0, 0xF, 6, 0, 262, 882, 0, 0 },
    { 1, 0, 0x40, 2, 0x3E, 2, 0, 0xF, 6, 0, 431, 638, 0, 0 },
    { 1, 0, 0x40, 2, 0x3E, 2, 0, 0xF, 6, 0, 702, 664, 0, 0 },
    { 1, 0, 0x40, 2, 0x3E, 2, 0, 0xF, 6, 0, 931, 679, 0, 0 },
    { 1, 0, 0x40, 2, 0x3E, 2, 0, 0xF, 6, 0, 1151, 642, 0, 0 },
    { 1, 0, 0x40, 2, 0x3E, 2, 0, 0xF, 6, 0, 1293, 701, 0, 0 },
    { 1, 0, 0x40, 2, 0x3F, 2, 0, 0xF, 6, 0, 707, 1103, 0, 0 },
    { 1, 0, 0x40, 2, 0x3F, 2, 0, 0xF, 6, 0, 732, 964, 0, 0 },
    { 1, 0, 0x40, 2, 0x3F, 2, 0, 0xF, 6, 0, 1051, 956, 0, 0 },
    { 1, 0, 0x40, 2, 0x3F, 2, 0, 0xF, 6, 0, 1076, 1132, 0, 0 },
    { 1, 0, 0x40, 2, 0x3F, 2, 0, 0xF, 6, 0, 1290, 929, 0, 0 },
    { 1, 0, 0x40, 2, 0x4B, 1, 0x4B, 0x56, 0x10, 0, 871, 187, 0, 0 },
    { 1, 0, 0x40, 2, 0x57, 1, 0x57, 0x62, 0x10, 0, 256, 462, 0, 0 },
    { 1, 0, 0xFF, 2, 0x1A, 0, 0, 0, 0, 0, 1280, 250, 0, 0 }, { 1, 0, 0x80, 2, 0x2D, 0, 0, 0, 0, 0, 640, 640, 0, 0 },
    { 1, 0, 0xFF, 2, 0x4A, 0, 0, 0, 0, 0, 1152, 640, 0, 0 }, { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 1127, 1030, 0, 0 },
    { 1, 0, 0x40, 6, 0x1B, 1, 0x1B, 0x1D, 0xA, 0, 548, 1332, 0, 0 },
    { 1, 0, 0x40, 6, 0x1B, 1, 0x1B, 0x1D, 0xA, 0, 718, 1206, 0, 0 },
    { 1, 0, 0x40, 6, 0x1B, 1, 0x1B, 0x1D, 0xA, 0, 1060, 992, 0, 0 },
    { 1, 0, 0x40, 6, 0x1B, 1, 0x1B, 0x1D, 0xA, 0, 1185, 379, 0, 0 },
    { 1, 0, 0x40, 6, 0x1B, 1, 0x1B, 0x1D, 0xA, 0, 1201, 1022, 0, 0 },
    { 1, 0, 0x40, 6, 0x1E, 1, 0x1E, 0x20, 0xA, 0, 308, 945, 0, 0 },
    { 1, 0, 0x40, 6, 0x1E, 1, 0x1E, 0x20, 0xA, 0, 399, 792, 0, 0 },
    { 1, 0, 0x40, 6, 0x1E, 1, 0x1E, 0x20, 0xA, 0, 716, 561, 0, 0 },
    { 1, 0, 0x40, 6, 0x1E, 1, 0x1E, 0x20, 0xA, 0, 733, 801, 0, 0 },
    { 1, 0, 0x40, 6, 0x1E, 1, 0x1E, 0x20, 0xA, 0, 774, 561, 0, 0 },
    { 1, 0, 0x40, 6, 0x1E, 1, 0x1E, 0x20, 0xA, 0, 778, 921, 0, 0 },
    { 1, 0, 0x40, 6, 0x1E, 1, 0x1E, 0x20, 0xA, 0, 1397, 1163, 0, 0 },
    { 1, 0, 0x40, 6, 0x21, 1, 0x21, 0x23, 0xA, 0, 807, 1209, 0, 0 },
    { 1, 0, 0x40, 6, 0x21, 1, 0x21, 0x23, 0xA, 0, 849, 1138, 0, 0 },
    { 1, 0, 0x40, 6, 0x21, 1, 0x21, 0x23, 0xA, 0, 1038, 926, 0, 0 },
    { 1, 0, 0x40, 6, 0x21, 1, 0x21, 0x23, 0xA, 0, 1111, 887, 0, 0 },
    { 1, 0, 0x40, 6, 0x21, 1, 0x21, 0x23, 0xA, 0, 1208, 1014, 0, 0 },
    { 1, 0, 0x40, 6, 0x21, 1, 0x21, 0x23, 0xA, 0, 1294, 541, 0, 0 },
    { 1, 0, 0x40, 6, 0x21, 1, 0x21, 0x23, 0xA, 0, 1343, 315, 0, 0 },
    { 1, 0, 0x40, 6, 0x21, 1, 0x21, 0x23, 0xA, 0, 1377, 734, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 190, 620, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 256, 1260, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 271, 861, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 354, 1089, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 467, 1200, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 544, 925, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 550, 1203, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 582, 1143, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 884, 148, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 923, 1263, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 1068, 1156, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 1221, 1220, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 189, 608, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 272, 404, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 383, 545, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 507, 1100, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 647, 505, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 743, 998, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 806, 518, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 858, 817, 0, 0 },
    { 1, 0, 0x40, 6, 0x24, 1, 0x24, 0x26, 0xA, 0, 961, 743, 0, 0 },
    { 1, 0, 0x40, 6, 0x27, 1, 0x27, 0x29, 0xA, 0, 265, 1270, 0, 0 },
    { 1, 0, 0x40, 6, 0x27, 1, 0x27, 0x29, 0xA, 0, 337, 461, 0, 0 },
    { 1, 0, 0x40, 6, 0x27, 1, 0x27, 0x29, 0xA, 0, 388, 555, 0, 0 },
    { 1, 0, 0x40, 6, 0x27, 1, 0x27, 0x29, 0xA, 0, 599, 726, 0, 0 },
    { 1, 0, 0x40, 6, 0x27, 1, 0x27, 0x29, 0xA, 0, 658, 358, 0, 0 },
    { 1, 0, 0x40, 6, 0x27, 1, 0x27, 0x29, 0xA, 0, 660, 285, 0, 0 },
    { 1, 0, 0x40, 6, 0x27, 1, 0x27, 0x29, 0xA, 0, 714, 1139, 0, 0 },
    { 1, 0, 0x40, 6, 0x27, 1, 0x27, 0x29, 0xA, 0, 765, 97, 0, 0 },
    { 1, 0, 0x40, 6, 0x27, 1, 0x27, 0x29, 0xA, 0, 809, 222, 0, 0 },
    { 1, 0, 0x40, 6, 0x27, 1, 0x27, 0x29, 0xA, 0, 874, 1029, 0, 0 },
    { 1, 0, 0x40, 6, 0x27, 1, 0x27, 0x29, 0xA, 0, 893, 157, 0, 0 },
    { 1, 0, 0x40, 6, 0x27, 1, 0x27, 0x29, 0xA, 0, 928, 871, 0, 0 },
    { 1, 0, 0x40, 6, 0x27, 1, 0x27, 0x29, 0xA, 0, 1225, 216, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 429, 1292, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 508, 1256, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 601, 1307, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 609, 1298, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 740, 1203, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 827, 1044, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 839, 1046, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 904, 1256, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 957, 1156, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 959, 1149, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1020, 810, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1028, 1041, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1044, 1105, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1064, 919, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1065, 831, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1110, 1001, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1155, 1152, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1168, 747, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1172, 739, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1183, 385, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1184, 749, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1224, 1022, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1273, 1198, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1357, 1139, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1378, 723, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1399, 723, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1428, 1151, 0, 0 },
    { 1, 0, 0x40, 6, 0x2A, 1, 0x2A, 0x2C, 0xA, 0, 1508, 1083, 0, 0 },
    { 1, 0, 0xFF, 6, 0x63, 0, 0, 0, 0, 0, 280, 280, 0, 0 }, { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 31, 277, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 59, 658, 0, 0 }, { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 215, 936, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 258, 235, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 277, 1117, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 293, 769, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 321, 1004, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 329, 1232, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 336, 615, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 349, 501, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 426, 720, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 444, 1104, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 478, 1313, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 500, 1150, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 516, 860, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 522, 188, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 546, 1010, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 592, 274, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 627, 1062, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 628, 1190, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 647, 1284, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 658, 794, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 675, 223, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 675, 500, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 689, 925, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 695, 359, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 752, 518, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 765, 187, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 768, 1184, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 787, 97, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 819, 1001, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 831, 586, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 863, 772, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 909, 1064, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 928, 561, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 951, 1243, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 996, 869, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 996, 869, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 1010, 95, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 1012, 1165, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 1077, 1061, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 1121, 928, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 1126, 178, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 1136, 400, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 1155, 305, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 1210, 1101, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 1247, 712, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 1319, 451, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 1354, 1028, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 1362, 347, 0, 0 },
    { 1, 0, 0x40, 0xA, 6, 1, 6, 9, 0xA, 0, 1396, 620, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 64, 753, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 144, 413, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 146, 568, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 193, 175, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 218, 836, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 257, 1190, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 261, 651, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 274, 1049, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 410, 1279, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 424, 842, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 425, 1235, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 449, 1160, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 460, 932, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 475, 1012, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 493, 701, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 660, 735, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 725, 327, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 728, 1128, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 732, 984, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 976, 249, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 1007, 556, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 1083, 1286, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 1087, 762, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 1190, 127, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 1232, 1060, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 1352, 221, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 1453, 531, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 1478, 397, 0, 0 },
    { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 1041, 566, 603, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 1133, 585, 596, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 862, 659, 666, 0 },
    { 1, 0, 0x80, 4, 0xD, 0, 0, 0, 0, 0, 1022, 313, 367, 0 },
    { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 693, 475, 479, 0 },
    { 1, 0, 0x40, 4, 0xF, 0, 0, 0, 0, 0, 661, 583, 589, 0 },
    { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 1029, 891, 899, 0 },
    { 1, 0, 0x40, 4, 0x11, 0, 0, 0, 0, 0, 1011, 936, 945, 0 },
    { 1, 0, 0x40, 4, 0x12, 0, 0, 0, 0, 0, 321, 1025, 1031, 0 },
    { 1, 0, 0x40, 4, 0x13, 0, 0, 0, 0, 0, 821, 1047, 1057, 0 },
    { 1, 0, 0x40, 4, 0x14, 0, 0, 0, 0, 0, 261, 1055, 1060, 0 },
    { 1, 0, 0x40, 4, 0x15, 0, 0, 0, 0, 0, 612, 1088, 1097, 0 },
    { 1, 0, 0x40, 4, 0x16, 0, 0, 0, 0, 0, 993, 1122, 1127, 0 },
    { 1, 0, 0x40, 4, 0x17, 0, 0, 0, 0, 0, 804, 1152, 1160, 0 },
    { 1, 0, 0x40, 4, 0x18, 0, 0, 0, 0, 0, 401, 1225, 1231, 0 },
    { 1, 0, 0x40, 4, 0x19, 0, 0, 0, 0, 0, 608, 1249, 1255, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 496, 807, 807, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 528, 951, 951, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 560, 743, 743, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 577, 359, 359, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 607, 768, 768, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 704, 847, 847, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 800, 847, 847, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1072, 471, 471, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1120, 447, 447, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1136, 487, 487, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1152, 527, 527, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1175, 441, 441, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1184, 559, 559, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1200, 471, 471, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1248, 495, 495, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1303, 605, 605, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1312, 431, 431, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1344, 623, 623, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag480_map_events[13] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x23B, 0x6C8, 0x1D4, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x234, 0x140, 0x350, 5, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 3, 2 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 0xE, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0xFFE8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x30, 0xFFE8, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFD8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0x28, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x30, 0x28, 0, 0, 0, 0, 0 }, { 0x6007, 1, 0xFFFF, 0, 8, 0xAA, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 6, 0, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 6, 1, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag480_funcs = { wstag480_setup };
FieldstgEventDef wstag480_events[4] = {
    { 170, D_WSTAG480_800A6478, 0x01350008, NULL, wstag480_event_170_end },
    { 171, D_WSTAG480_800A6528, 0x01350009, NULL, wstag480_event_171_end },
    { 240, D_WSTAG480_800A65A4, 0x0135000F, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
