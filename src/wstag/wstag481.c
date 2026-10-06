#include "wstag.h"

/* WSTAG481: stage 0x2A7 (fieldstg_stages). */

extern WstagSeqKey **D_WSTAG481_800A6564[];
extern WstagFuncs wstag481_funcs;
extern const CVECTOR wstag481_color;
extern FieldstgBattleLists wstag481_battle_lists;
extern FieldstgVramPlace wstag481_vram_places[];
extern FieldstgSprite wstag481_sprites[];
extern FieldstgMapEvent wstag481_map_events[];
void wstag481_seq_update();

void wstag481_seq_anim_advance(FieldstgSprite *sprite, WstagSeqKey **seqs, WstagSeqAnim *anim, s32 depth) {
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
        wstag481_seq_anim_advance(sprite, seqs, anim, depth + 1);
    }
    if (depth == 0) {
        sprite->sprite = seqs[anim->seq][anim->key].sprite;
        sprite->frame = seqs[anim->seq][anim->key].frame;
    }
}

void wstag481_seq_update(WstagSeqObject *obj) {
    FieldstgSprite *sprite;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->anims[0].seq = 0;
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG481_800A6564[0][0][0].time;
        obj->anims[1].seq = 0;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG481_800A6564[1][0][0].time;
        obj->anims[2].seq = 0;
        obj->anims[2].key = 0;
        obj->anims[2].time = D_WSTAG481_800A6564[2][0][0].time;
        obj->anims[3].seq = 0;
        obj->anims[3].key = 0;
        obj->anims[3].time = D_WSTAG481_800A6564[3][0][0].time;
        obj->anims[4].seq = 0;
        obj->anims[4].key = 0;
        obj->anims[4].time = D_WSTAG481_800A6564[3][0][0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            switch (sprite->type) {
            case 1:
                wstag481_seq_anim_advance(sprite, D_WSTAG481_800A6564[0], &obj->anims[0], 0);
                break;
            case 2:
                wstag481_seq_anim_advance(sprite, D_WSTAG481_800A6564[1], &obj->anims[1], 0);
                break;
            case 3:
                wstag481_seq_anim_advance(sprite, D_WSTAG481_800A6564[2], &obj->anims[2], 0);
                break;
            case 4:
                wstag481_seq_anim_advance(sprite, D_WSTAG481_800A6564[3], &obj->anims[3], 0);
                break;
            case 5:
                wstag481_seq_anim_advance(sprite, D_WSTAG481_800A6564[4], &obj->anims[4], 0);
                break;
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag481_seq_create(void) {
    return object_new(wstag481_seq_update, sizeof(WstagSeqObject), 0);
}

void wstag481_update(WstagObject *obj, Object **data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        *data = wstag481_seq_create();
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag481_start(void *arg0) {
    WstagObject *obj = object_new(wstag481_update, sizeof(WstagObject), sizeof(Object *));

    obj->manager = arg0;
    wstag481_funcs.setup();
    return obj;
}

void wstag481_setup(void) {
    fieldstg_stage.background_file = 0x5FA;
    fieldstg_stage.sprite_file = 0x05FB0000;
    fieldstg_stage.sprites = wstag481_sprites;
    fieldstg_stage.map_events = wstag481_map_events;
    fieldstg_stage.mask_file = 0x5F9;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x48600, 0xE600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag481_vram_places;
    fieldstg_stage.music = 0x11;
    fieldstg_stage.sound = 0x60440000;
    fieldstg_stage.color = wstag481_color;
    fieldstg_stage.battle_lists = &wstag481_battle_lists;
    fieldstg_attr.set_file(0, 0x05FB0001);
    fieldstg_attr.set_file(1, 0x05FB0003);
    fieldstg_attr.set_file(7, 0x05FB0002);
    fieldstg_attr.set_file(4, 0x05FB0004);
    fieldstg_attr.init_layer(0);
}

const CVECTOR wstag481_color = { 0x80, 0x80, 0x80, 0 };

/* The stage's .data (tools/wstag_data.py). */
void wstag481_setup(void);

WstagSeqKey D_WSTAG481_800A6384[4] = {
    { 0x40, 0xA, 0, 0 }, { 0x40, 0xA, 1, 0 }, { 0x40, 0xA, 2, 0 }, { 0x40, 0xA, 1, 1 },
};
WstagSeqKey D_WSTAG481_800A6394[6] = {
    { 0x41, 4, 2, 0 }, { 0x41, 4, 4, 0 }, { 0x41, 4, 6, 0 }, { 0x41, 4, 4, 0 }, { 0x41, 4, 2, 0 },
    { 0x41, 4, 0, 1 },
};
WstagSeqKey D_WSTAG481_800A63AC[4] = {
    { 0x42, 0xA, 0, 0 }, { 0x42, 0xA, 1, 0 }, { 0x42, 0xA, 2, 0 }, { 0x42, 0xA, 1, 1 },
};
WstagSeqKey D_WSTAG481_800A63BC[6] = {
    { 0x43, 4, 2, 0 }, { 0x43, 4, 4, 0 }, { 0x43, 4, 6, 0 }, { 0x43, 4, 4, 0 }, { 0x43, 4, 2, 0 },
    { 0x43, 4, 0, 1 },
};
WstagSeqKey D_WSTAG481_800A63D4[4] = {
    { 0x44, 0xA, 0, 0 }, { 0x44, 0xA, 1, 0 }, { 0x44, 0xA, 2, 0 }, { 0x44, 0xA, 1, 1 },
};
WstagSeqKey D_WSTAG481_800A63E4[6] = {
    { 0x45, 4, 2, 0 }, { 0x45, 4, 4, 0 }, { 0x45, 4, 6, 0 }, { 0x45, 4, 4, 0 }, { 0x45, 4, 2, 0 },
    { 0x45, 4, 0, 1 },
};
WstagSeqKey D_WSTAG481_800A63FC[4] = {
    { 0x46, 0xA, 0, 0 }, { 0x46, 0xA, 1, 0 }, { 0x46, 0xA, 2, 0 }, { 0x46, 0xA, 1, 1 },
};
WstagSeqKey D_WSTAG481_800A640C[6] = {
    { 0x47, 4, 2, 0 }, { 0x47, 4, 4, 0 }, { 0x47, 4, 6, 0 }, { 0x47, 4, 4, 0 }, { 0x47, 4, 2, 0 },
    { 0x47, 4, 0, 1 },
};
WstagSeqKey D_WSTAG481_800A6424[4] = {
    { 0x48, 0xA, 0, 0 }, { 0x48, 0xA, 1, 0 }, { 0x48, 0xA, 2, 0 }, { 0x48, 0xA, 1, 1 },
};
WstagSeqKey D_WSTAG481_800A6434[6] = {
    { 0x49, 4, 2, 0 }, { 0x49, 4, 4, 0 }, { 0x49, 4, 6, 0 }, { 0x49, 4, 4, 0 }, { 0x49, 4, 2, 0 },
    { 0x49, 4, 0, 1 },
};
WstagSeqKey *D_WSTAG481_800A644C[14] = {
    D_WSTAG481_800A6384, D_WSTAG481_800A6384, D_WSTAG481_800A6384, D_WSTAG481_800A6384, D_WSTAG481_800A6384,
    D_WSTAG481_800A6384, D_WSTAG481_800A6384, D_WSTAG481_800A6384, D_WSTAG481_800A6384, D_WSTAG481_800A6384,
    D_WSTAG481_800A6384, D_WSTAG481_800A6384, D_WSTAG481_800A6394, NULL,
};
WstagSeqKey *D_WSTAG481_800A6484[14] = {
    D_WSTAG481_800A63AC, D_WSTAG481_800A63AC, D_WSTAG481_800A63AC, D_WSTAG481_800A63AC, D_WSTAG481_800A63AC,
    D_WSTAG481_800A63AC, D_WSTAG481_800A63AC, D_WSTAG481_800A63AC, D_WSTAG481_800A63AC, D_WSTAG481_800A63AC,
    D_WSTAG481_800A63AC, D_WSTAG481_800A63AC, D_WSTAG481_800A63BC, NULL,
};
WstagSeqKey *D_WSTAG481_800A64BC[14] = {
    D_WSTAG481_800A63D4, D_WSTAG481_800A63D4, D_WSTAG481_800A63D4, D_WSTAG481_800A63D4, D_WSTAG481_800A63D4,
    D_WSTAG481_800A63D4, D_WSTAG481_800A63D4, D_WSTAG481_800A63D4, D_WSTAG481_800A63D4, D_WSTAG481_800A63D4,
    D_WSTAG481_800A63D4, D_WSTAG481_800A63D4, D_WSTAG481_800A63E4, NULL,
};
WstagSeqKey *D_WSTAG481_800A64F4[14] = {
    D_WSTAG481_800A63FC, D_WSTAG481_800A63FC, D_WSTAG481_800A63FC, D_WSTAG481_800A63FC, D_WSTAG481_800A63FC,
    D_WSTAG481_800A63FC, D_WSTAG481_800A63FC, D_WSTAG481_800A63FC, D_WSTAG481_800A63FC, D_WSTAG481_800A63FC,
    D_WSTAG481_800A63FC, D_WSTAG481_800A63FC, D_WSTAG481_800A640C, NULL,
};
WstagSeqKey *D_WSTAG481_800A652C[14] = {
    D_WSTAG481_800A6424, D_WSTAG481_800A6424, D_WSTAG481_800A6424, D_WSTAG481_800A6424, D_WSTAG481_800A6424,
    D_WSTAG481_800A6424, D_WSTAG481_800A6424, D_WSTAG481_800A6424, D_WSTAG481_800A6424, D_WSTAG481_800A6424,
    D_WSTAG481_800A6424, D_WSTAG481_800A6424, D_WSTAG481_800A6434, NULL,
};
WstagSeqKey * *D_WSTAG481_800A6564[5] = {
    D_WSTAG481_800A644C, D_WSTAG481_800A6484, D_WSTAG481_800A64BC, D_WSTAG481_800A64F4, D_WSTAG481_800A652C,
};
FieldstgListedBattle D_WSTAG481_800A6578 = { 107, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A6584 = { 107, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A6590 = { 107, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A659C = { 107, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A65A8 = { 155, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A65B4 = { 155, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A65C0 = { 155, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A65CC = { 155, 9, 0x60080000 };
FieldstgBattleList D_WSTAG481_800A65D8 = {
    4,
    { &D_WSTAG481_800A6578, &D_WSTAG481_800A6584, &D_WSTAG481_800A6590, &D_WSTAG481_800A659C, &D_WSTAG481_800A65A8,
        &D_WSTAG481_800A65B4, &D_WSTAG481_800A65C0, &D_WSTAG481_800A65CC },
};
FieldstgListedBattle D_WSTAG481_800A65FC = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A6608 = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A6614 = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A6620 = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A662C = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A6638 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A6644 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A6650 = { 159, 8, 0x60080000 };
FieldstgBattleList D_WSTAG481_800A665C = {
    2,
    { &D_WSTAG481_800A65FC, &D_WSTAG481_800A6608, &D_WSTAG481_800A6614, &D_WSTAG481_800A6620, &D_WSTAG481_800A662C,
        &D_WSTAG481_800A6638, &D_WSTAG481_800A6644, &D_WSTAG481_800A6650 },
};
FieldstgListedBattle D_WSTAG481_800A6680 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG481_800A668C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG481_800A6698 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG481_800A66A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG481_800A66B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG481_800A66BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG481_800A66C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG481_800A66D4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG481_800A66E0 = {
    0,
    { &D_WSTAG481_800A6680, &D_WSTAG481_800A668C, &D_WSTAG481_800A6698, &D_WSTAG481_800A66A4, &D_WSTAG481_800A66B0,
        &D_WSTAG481_800A66BC, &D_WSTAG481_800A66C8, &D_WSTAG481_800A66D4 },
};
FieldstgListedBattle D_WSTAG481_800A6704 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG481_800A6710 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG481_800A671C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG481_800A6728 = { 331, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A6734 = { 332, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A6740 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG481_800A674C = { 177, 9, 0x60080000 };
FieldstgListedBattle D_WSTAG481_800A6758 = { 106, 8, 0x60080000 };
FieldstgBattleList D_WSTAG481_800A6764 = {
    0,
    { &D_WSTAG481_800A6704, &D_WSTAG481_800A6710, &D_WSTAG481_800A671C, &D_WSTAG481_800A6728, &D_WSTAG481_800A6734,
        &D_WSTAG481_800A6740, &D_WSTAG481_800A674C, &D_WSTAG481_800A6758 },
};
FieldstgBattleLists wstag481_battle_lists = {
    74, 0, 0, { &D_WSTAG481_800A65D8, &D_WSTAG481_800A665C, &D_WSTAG481_800A66E0 }, &D_WSTAG481_800A6764,
};
FieldstgVramPlace wstag481_vram_places[6] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
};
FieldstgSprite wstag481_sprites[93] = {
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
    { 1, 0, 0xFF, 2, 0x1A, 0, 0, 0, 0, 0, 1280, 250, 0, 0 }, { 1, 0, 0x80, 2, 0x1B, 0, 0, 0, 0, 0, 640, 640, 0, 0 },
    { 1, 0, 0x80, 2, 0x1C, 0, 0, 0, 0, 0, 1152, 640, 0, 0 },
    { 1, 0, 0x80, 2, 0x1D, 0, 0, 0, 0, 0, 1280, 640, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 1125, 1033, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 200, 271, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 299, 546, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 379, 1321, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 388, 925, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 436, 759, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 565, 1172, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 653, 974, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 871, 170, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 883, 1267, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 885, 889, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 900, 760, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 1079, 128, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 1322, 546, 0, 0 },
    { 1, 0, 0x40, 6, 0x22, 1, 0x22, 0x31, 0xA, 0, 1341, 990, 0, 0 },
    { 1, 0, 0xFF, 6, 0x63, 0, 0, 0, 0, 0, 280, 280, 0, 0 },
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
FieldstgMapEvent wstag481_map_events[12] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A8, 0x6C8, 0x1D4, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A2, 0x140, 0x350, 5, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 3, 2 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x1A, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0xFFE8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x30, 0xFFE8, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFD8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0x28, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x30, 0x28, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 6, 0, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 6, 1, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag481_funcs = { wstag481_setup };
