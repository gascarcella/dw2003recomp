#include "stgtrain.h"

/* STGTRAIN.PRO, third file: an unused object, the trainer level choice, the trainee (a Digimon's training
 * animation), the training choice, and the overlay's helpers, called through stgtrain_module.funcs (the animation files of
 * the trainer menu and the training menus). */

extern StgtrainMenuEntry stgtrain_menus[][16];
extern StgtrainAnimFile stgtrain_anim_files[];
/* A part's header in an animation file: offsets from the file's start. */
typedef struct StgtrainAnimHeader {
    /* 0x00 */ s32 count;      /* offsets that follow: 7 + images */
    /* 0x04 */ s32 sprite_data;
    /* 0x08 */ s32 anims[6];
    /* 0x20 */ s32 images[4];
} StgtrainAnimHeader;

/* The read position of stgtrain_anim_parse and of each of its helpers. Each is a one-member struct: a store
 * into a part makes the code reload it, which GCC 2.8 only does for a struct member (it assumes a struct field
 * never aliases a scalar global), and a struct holding all four would be reached through one base register. */
typedef struct StgtrainAnimReader {
    /* 0x0 */ StgtrainAnimHeader *hdr;
} StgtrainAnimReader; /* size 0x4 */

struct {
    s32 *pos;
} stgtrain_header_reader; /* stgtrain_anim_parse_header: walks the header word by word */
StgtrainAnimReader stgtrain_anims_reader; /* stgtrain_anim_parse_10 */
StgtrainAnimReader stgtrain_images_reader; /* stgtrain_anim_parse_images */
StgtrainAnimReader stgtrain_parse_reader; /* stgtrain_anim_parse */


void stgtrain_stub_init(Object *obj, void *data) {
}

void stgtrain_stub_show(Object *obj, void *data, s32 arg2) {
}

void stgtrain_stub_draw(Object *obj) {
    Sprite spr;

    sprite_init(&spr);
}

void stgtrain_stub_input(Object *obj, void *data) {
}

/* An object that does nothing (its steps are empty); stgtrain_stub_create, its creator, is never called. STCRDDEK
 * has the same creator. */
void stgtrain_stub_update(Object *obj, void *data) {
    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        obj->next_state(obj);
        stgtrain_stub_init(obj, data);
        stgtrain_stub_show(obj, data, 1);
        break;
    case OBJECT_STATE_RUN:
        stgtrain_stub_input(obj, data);
        stgtrain_stub_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

typedef struct StgtrainStub {
    /* 0x00 */ Object base;
    /* 0x50 */ StgtrainMain *main;
    /* 0x54 */ s32 layer;
    /* 0x58 */ s32 ot_depth;
    /* 0x5C */ u8 unk_5C[0x10];
} StgtrainStub; /* size 0x6C */

StgtrainStub *stgtrain_stub_create(StgtrainMain *main) {
    StgtrainStub *obj = object_new(stgtrain_stub_update, sizeof(StgtrainStub), 0);

    obj->layer = 0x1000;
    obj->ot_depth = 6;
    obj->main = main;
    return obj;
}

/* The trainer level choice's data block (0x28 bytes). */
typedef struct StgtrainLevelData {
    /* 0x00 */ MessageWindow *message;
    /* 0x04 */ MessageWindow *trainer_name; /* the trainer's name */
    /* 0x08 */ MessageWindow *level_name;
    /* 0x0C */ MessageWindow *levels[3]; /* the levels */
    /* 0x18 */ MessageWindow *warning;
    /* 0x1C */ MessageWindow *yes;    /* yes */
    /* 0x20 */ MessageWindow *no;     /* no */
    /* 0x24 */ MessageCursor *cursor; /* the cursor */
} StgtrainLevelData; /* size 0x28 */

/* What each trainer level costs (GamestateStats.values[1]). */
extern s32 stgtrain_tp_costs[3];

void stgtrain_level_run(StgtrainLevel *obj, StgtrainLevelData *data);

void stgtrain_level_create_windows(StgtrainLevel *obj, StgtrainLevelData *data) {
    data->message = message_create_window(obj->layer, 1, 0xA2, 0x49);
    data->trainer_name = message_create_window(obj->layer, 1, 0xBC, 0x14);
    data->level_name = message_create_window(obj->layer, 1, 0xC0, 0x29);
    data->levels[0] = message_create_window(obj->layer, 1, 0x98, 0x66);
    data->levels[1] = message_create_window(obj->layer, 1, 0xC0, 0x66);
    data->levels[2] = message_create_window(obj->layer, 1, 0xE6, 0x66);
    data->warning = message_create_window(obj->layer, 1, 0x94, 0x87);
    data->yes = message_create_window(obj->layer, 1, 0xA2, 0x64);
    data->no = message_create_window(obj->layer, 1, 0xA2, 0x74);
    data->cursor = message_create_cursor(obj->layer, obj->ot_depth - 1, 0, 0);
    data->cursor->show(data->cursor, 0);
}

void stgtrain_level_draw(StgtrainLevel *obj) {
    Sprite spr;
    s32 i; /* the trainer's sprite, then the loop index */

    sprite_init(&spr);
    spr.set_layer_id(obj->layer, obj->ot_depth);
    spr.set_vram_pos(0x240, 0x100);
    if (obj->anims[2].level != 0) {
        if (obj->anims[2].level != 0x1000) {
            spr.set_scale(obj->anims[2].level, obj->anims[2].level, 0x1000);
            spr.set_pivot(0xA6, 0x26);
        }
        if (gfx_module.funcs.get_time() - obj->trainer_time >= 16) {
            obj->trainer_time = gfx_module.funcs.get_time();
            obj->trainer_frame++;
            if (obj->trainer_frame >= 4) {
                obj->trainer_frame = 0;
            }
        }
        i = stgtrain_module.funcs.trainings[obj->main->training].icons[obj->trainer_frame];
        spr.draw(cdload_module.get_subfile_by_id(0x028C0000), i, 0x94, 0x14);
    }
    if (obj->anims[3].level != 0) {
        if (obj->anims[3].level != 0x1000) {
            spr.set_scale(obj->anims[3].level, obj->anims[3].level, 0x1000);
            spr.set_pivot(0xCE, 0x2F);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x2A, 0xBC, 0x26);
    }
    if (obj->anims[1].level != 0) {
        spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
        if (obj->anims[1].level != 0x1000) {
            spr.set_pivot(0x140, 0x26);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x25, 0x8F, 0xF);
    }
    if (obj->anims[0].level != 0) {
        if (obj->anims[0].level != 0x1000) {
            spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x4E);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x26, 0x85, 0x43);
    }
    if (obj->cursor != 0) {
        if (gfx_module.funcs.get_time() - obj->cursor_time >= 11) {
            obj->cursor_time = gfx_module.funcs.get_time();
            obj->cursor_frame++;
            if (obj->cursor_frame >= 4) {
                obj->cursor_frame = 0;
            }
        }
        spr.set_palette(obj->cursor_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x29, obj->level * 0x28 + 0x94, 0x63);
        spr.set_palette(0);
    }
    if (obj->anims[6].level != 0) {
        if (obj->anims[6].level != 0x1000) {
            spr.set_scale(obj->anims[6].level, obj->anims[6].level, 0x1000);
        }
        for (i = 0; i < 3; i++) {
            if (obj->anims[6].level != 0x1000) {
                spr.set_pivot(0xA6 + i * 0x28, 0x6C);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x2A, 0x94 + i * 0x28, 0x63);
        }
    }
    if (obj->anims[5].level != 0) {
        spr.set_scale(obj->anims[5].level, 0x1000, 0x1000);
        spr.set_pivot(0x140, 0x6C);
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x27, 0x82, 0x5E);
    }
    if (obj->anims[7].level != 0) {
        spr.set_scale(obj->anims[7].level, 0x1000, 0x1000);
        spr.set_pivot(0x140, 0x8D);
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x27, 0x82, 0x7F);
    }
    if (obj->anims[4].level != 0) {
        spr.set_scale(obj->anims[4].level, 0x1000, 0x1000);
        spr.set_pivot(0x140, 0x72);
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x1D, 0x82, 0x5E);
    }
}

void stgtrain_level_run(StgtrainLevel *obj, StgtrainLevelData *data) {
    GamestateStats stats;
    GamestateRecord *digimon;
    s32 level;
    s32 yes;
    s32 i;
    s32 j;
    s32 k;

    switch (obj->base.step) {
    case 0:
    default:
        stgtrain_module.funcs.anim_start(&obj->anims[1], 1);
        obj->base.step++;
        break;
    case 1:
        if (stgtrain_module.funcs.anim_update(&obj->anims[1])) {
            i = stgtrain_module.funcs.trainings[obj->main->training].name;
            data->trainer_name->set_text(data->trainer_name, cdload_module.files.get_file(records_language + 0x10B), i);
            stgtrain_module.funcs.anim_start(&obj->anims[2], 1);
            obj->base.step++;
        }
        break;
    case 2:
        if (stgtrain_module.funcs.anim_update(&obj->anims[2])) {
            stgtrain_module.funcs.anim_start(&obj->anims[0], 1);
            obj->base.step++;
        }
        break;
    case 3:
        if (stgtrain_module.funcs.anim_update(&obj->anims[0])) {
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x10B), 8);
            stgtrain_module.funcs.anim_start(&obj->anims[5], 1);
            obj->base.step++;
        }
        break;
    case 4:
        if (stgtrain_module.funcs.anim_update(&obj->anims[5])) {
            stgtrain_module.funcs.anim_start(&obj->anims[6], 1);
            obj->base.step++;
        }
        break;
    case 5:
        if (stgtrain_module.funcs.anim_update(&obj->anims[6])) {
            for (i = 0; i < 3; i++) {
                data->levels[i]->set_text(data->levels[i], cdload_module.files.get_file(records_language + 0x10B), i + 0xE);
            }
            obj->cursor = 1;
            obj->base.step++;
        }
        break;
    case 6:
        level = obj->level;
        if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
            obj->level--;
            if (obj->level < 0) {
                obj->level = 0;
            }
        } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
            obj->level++;
            if (obj->level >= 3) {
                obj->level = 2;
            }
        }
        if (level != obj->level) {
            sound_module.play(0x4001B);
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            gamestate_data.funcs.get_stats(gamestate_data.funcs.get_party_member(obj->main->member), &stats);
            if (stats.values[1] < stgtrain_tp_costs[obj->level]) {
                obj->base.step = 10;
            } else {
                obj->base.step = 15;
                obj->main->level = obj->level;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step = 50;
            obj->base.substep = 1;
        }
        break;
    case 10:
        obj->cursor = 0;
        stgtrain_module.funcs.anim_start(&obj->anims[7], 1);
        obj->base.step++;
        break;
    case 11:
        if (stgtrain_module.funcs.anim_update(&obj->anims[7])) {
            data->warning->set_text(data->warning, cdload_module.files.get_file(records_language + 0x10B), 0x12);
            obj->base.step++;
        }
        break;
    case 12:
        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            data->warning->set_visible(data->warning, 0);
            stgtrain_module.funcs.anim_start(&obj->anims[7], 0);
            obj->base.step++;
        }
        break;
    case 13:
        if (stgtrain_module.funcs.anim_update(&obj->anims[7])) {
            obj->cursor = 1;
            obj->base.step = 6;
        }
        break;
    case 15:
        obj->cursor = 0;
        data->message->set_visible(data->message, 0);
        for (j = 0; j < 3; j++) {
            data->levels[j]->set_visible(data->levels[j], 0);
        }
        obj->anims[6].level = 0;
        stgtrain_module.funcs.anim_start(&obj->anims[5], 0);
        stgtrain_module.funcs.anim_start(&obj->anims[0], 0);
        obj->base.step++;
        break;
    case 16:
        stgtrain_module.funcs.anim_update(&obj->anims[5]);
        if (stgtrain_module.funcs.anim_update(&obj->anims[0])) {
            stgtrain_module.funcs.anim_start(&obj->anims[0], 1);
            stgtrain_module.funcs.anim_start(&obj->anims[4], 1);
            stgtrain_module.funcs.anim_start(&obj->anims[3], 1);
            obj->base.step++;
        }
        break;
    case 17:
        stgtrain_module.funcs.anim_update(&obj->anims[3]);
        stgtrain_module.funcs.anim_update(&obj->anims[0]);
        if (stgtrain_module.funcs.anim_update(&obj->anims[4])) {
            data->level_name->set_text(data->level_name, cdload_module.files.get_file(records_language + 0x10B), obj->level + 0xE);
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x10B), 9);
            data->yes->set_text(data->yes, cdload_module.files.get_file(records_language + 0x10B), 0xA);
            data->no->set_text(data->no, cdload_module.files.get_file(records_language + 0x10B), 0xB);
            obj->yes = 0;
            data->cursor->set_pos(data->cursor, 0x94, 0x64);
            data->cursor->show(data->cursor, 1);
            obj->base.step++;
        }
        break;
    case 18:
        yes = obj->yes;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            obj->yes = 0;
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            obj->yes = 1;
        }
        if (yes != obj->yes) {
            sound_module.play(0x8004513E);
            data->cursor->set_pos(data->cursor, 0x94, obj->yes * 0x10 + 0x64);
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x8004503C);
            if (obj->yes == 0) {
                obj->base.step = 25;
                digimon = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(obj->main->member));
                digimon->stats.values[1] -= stgtrain_tp_costs[obj->level];
            } else {
                obj->base.step++;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step++;
        }
        break;
    case 19:
        stgtrain_module.funcs.anim_start(&obj->anims[0], 0);
        stgtrain_module.funcs.anim_start(&obj->anims[4], 0);
        stgtrain_module.funcs.anim_start(&obj->anims[3], 0);
        data->level_name->set_visible(data->level_name, 0);
        data->message->set_visible(data->message, 0);
        data->yes->set_visible(data->yes, 0);
        data->no->set_visible(data->no, 0);
        data->cursor->show(data->cursor, 0);
        obj->base.step++;
        break;
    case 20:
        stgtrain_module.funcs.anim_update(&obj->anims[3]);
        stgtrain_module.funcs.anim_update(&obj->anims[0]);
        if (stgtrain_module.funcs.anim_update(&obj->anims[4])) {
            stgtrain_module.funcs.anim_start(&obj->anims[0], 1);
            obj->base.step = 3;
        }
        break;
    case 25:
        stgtrain_module.funcs.anim_start(&obj->anims[0], 0);
        data->message->set_visible(data->message, 0);
        stgtrain_module.funcs.anim_start(&obj->anims[4], 0);
        data->yes->set_visible(data->yes, 0);
        data->no->set_visible(data->no, 0);
        data->cursor->show(data->cursor, 0);
        obj->base.step++;
        break;
    case 26:
        stgtrain_module.funcs.anim_update(&obj->anims[0]);
        if (stgtrain_module.funcs.anim_update(&obj->anims[4])) {
            obj->base.step = 35;
        }
        break;
    case 30:
        data->trainer_name->set_visible(data->trainer_name, 0);
        data->level_name->set_visible(data->level_name, 0);
        obj->anims[2].level = 0;
        obj->anims[3].level = 0;
        stgtrain_module.funcs.anim_start(&obj->anims[1], 0);
        obj->base.step++;
        break;
    case 31:
        if (stgtrain_module.funcs.anim_update(&obj->anims[1])) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 35:
        break;
    case 50:
        obj->cursor = 0;
        data->message->set_visible(data->message, 0);
        for (k = 0; k < 3; k++) {
            data->levels[k]->set_visible(data->levels[k], 0);
        }
        data->trainer_name->set_visible(data->trainer_name, 0);
        stgtrain_module.funcs.anim_start(&obj->anims[1], 0);
        stgtrain_module.funcs.anim_start(&obj->anims[2], 0);
        stgtrain_module.funcs.anim_start(&obj->anims[0], 0);
        stgtrain_module.funcs.anim_start(&obj->anims[6], 0);
        stgtrain_module.funcs.anim_start(&obj->anims[5], 0);
        obj->base.step++;
        break;
    case 51:
        stgtrain_module.funcs.anim_update(&obj->anims[1]);
        stgtrain_module.funcs.anim_update(&obj->anims[2]);
        stgtrain_module.funcs.anim_update(&obj->anims[0]);
        stgtrain_module.funcs.anim_update(&obj->anims[6]);
        if (stgtrain_module.funcs.anim_update(&obj->anims[5]) && obj->base.substep != 0) {
            obj->base.state = OBJECT_STATE_DONE;
        }
        break;
    }
}

void stgtrain_level_update(StgtrainLevel *obj, StgtrainLevelData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stgtrain_level_create_windows(obj, data);
        obj->anims[6].duration = 8;
        obj->anims[5].duration = 10;
        obj->anims[4].duration = 10;
        obj->anims[0].duration = 10;
        obj->anims[1].duration = 10;
        obj->anims[3].duration = 6;
        obj->anims[2].duration = 6;
        obj->anims[7].duration = 10;
        obj->level = obj->main->level;
        break;
    case OBJECT_STATE_RUN:
        stgtrain_level_run(obj, data);
    case OBJECT_STATE_DONE:
        stgtrain_level_draw(obj);
    case OBJECT_STATE_END:
        break;
    }
}

/* StgtrainLevel.close: ends the choice (step 30). */
void stgtrain_level_close(StgtrainLevel *obj) {
    obj->base.step = 30;
}

StgtrainLevel *stgtrain_level_create(StgtrainMain *main) {
    StgtrainLevel *obj = object_new(stgtrain_level_update, sizeof(StgtrainLevel), sizeof(StgtrainLevelData));

    obj->close = stgtrain_level_close;
    obj->layer = 0x1000;
    obj->ot_depth = 6;
    obj->main = main;
    return obj;
}

typedef struct StgtrainTraineeData StgtrainTraineeData;
void stgtrain_trainee_update(StgtrainTrainee *obj, StgtrainTraineeData *data);

/* The trainee's data block (0xC bytes): its two sprites (the Digimon, and part 8 of the file if it has one). */
struct StgtrainTraineeData {
    /* 0x0 */ StgtrainSprite *sprite;
    /* 0x4 */ StgtrainSprite *sprite_2;
    /* 0x8 */ s32 unk_8;
}; /* size 0xC */

void stgtrain_trainee_update(StgtrainTrainee *obj, StgtrainTraineeData *data) {
    u32 pos;
    s32 done;
    s32 success;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (cdload_module.is_loading(stgtrain_module.funcs.get_file_id(obj->file)) != 0 || obj->tex_set == 0 ||
            obj->clut_set == 0) {
            break;
        }
        obj->base.next_state(obj);
        pos = stgtrain_module.funcs.get_file_pos(obj->file);
        stgtrain_module.funcs.parse(obj->part);
        stgtrain_module.funcs.upload(obj->part, obj->pos);
        if (data->sprite == NULL) {
            data->sprite = stgtrain_sprite_create();
        }
        data->sprite->set_data(data->sprite, stgtrain_module.funcs.get_part_data(obj->part),
                              stgtrain_module.funcs.get_part_offset(obj->part));
        data->sprite->set_anim(data->sprite, (StgtrainSpriteAnim *)stgtrain_module.funcs.get_part_anim(obj->part, obj->step));
        data->sprite->set_pos(data->sprite, pos & 0xFFFF, pos >> 16);
        data->sprite->set_tex(data->sprite, obj->pos[0], obj->pos[1]);
        data->sprite->set_clut(data->sprite, obj->pos[2], obj->pos[3]);
        data->sprite->set_layer(data->sprite, obj->layer, obj->ot_depth);
        if (stgtrain_module.funcs.parse(8) != 0) {
            obj->saved_x = obj->pos[0];
            obj->pos[0] += 0x80;
            stgtrain_module.funcs.upload(8, obj->pos);
            obj->pos[0] = obj->saved_x;
            if (data->sprite_2 == NULL) {
                data->sprite_2 = stgtrain_sprite_create();
            }
            data->sprite_2->set_data(data->sprite_2, stgtrain_module.funcs.get_part_data(8), stgtrain_module.funcs.get_part_offset(8));
            data->sprite_2->set_anim(data->sprite_2, (StgtrainSpriteAnim *)stgtrain_module.funcs.get_part_anim(8, obj->step));
            data->sprite_2->set_pos(data->sprite_2, pos & 0xFFFF, pos >> 16);
            data->sprite_2->set_tex(data->sprite_2, obj->pos[0], obj->pos[1]);
            data->sprite_2->set_clut(data->sprite_2, obj->pos[2], obj->pos[3]);
            if (stgtrain_module.funcs.get_file_unk_C(obj->file) == 0) {
                data->sprite_2->set_layer(data->sprite_2, obj->layer, obj->ot_depth - 1);
            } else {
                data->sprite_2->set_layer(data->sprite_2, obj->layer, obj->ot_depth);
            }
        } else if (data->sprite_2 != NULL) {
            data->sprite_2->base.set_state(data->sprite_2, OBJECT_STATE_END);
        }
        obj->result = -1;
        break;
    case OBJECT_STATE_RUN:
        if (obj->scale_set != 0) {
            if (data->sprite != NULL) {
                data->sprite->set_scale(data->sprite, obj->scale, obj->scale, 0x1000);
            }
            if (data->sprite_2 != NULL) {
                data->sprite_2->set_scale(data->sprite_2, obj->scale, obj->scale, 0x1000);
            }
            obj->scale_set = 0;
        }
        switch (obj->state) {
        case 1:
            break;
        case 2:
            obj->scale += obj->scale_step;
            if (obj->scale_step > 0) {
                if (obj->scale > 0x1000) {
                    obj->scale = 0x1000;
                    obj->scale_step = 0;
                    obj->state = 1;
                }
            } else if (obj->scale < 0) {
                obj->scale = 0;
                obj->scale_step = 0;
                obj->state = 1;
            }
            if (data->sprite != NULL) {
                data->sprite->set_scale(data->sprite, obj->scale, obj->scale, 0x1000);
                data->sprite->set_pivot(data->sprite, 0xCF, 0x7F);
            }
            if (data->sprite_2 != NULL) {
                data->sprite_2->set_scale(data->sprite_2, obj->scale, obj->scale, 0x1000);
                data->sprite_2->set_pivot(data->sprite_2, 0xCF, 0x7F);
            }
            break;
        case 4:
            if (data->sprite_2 != NULL) {
                done = data->sprite->is_done(data->sprite) & data->sprite_2->is_done(data->sprite_2);
            } else {
                done = data->sprite->is_done(data->sprite);
            }
            if (done == 0) {
                break;
            }
            if (obj->base.step != 0) {
                goto end;
            }
            obj->step++;
            if (obj->step >= 2) {
                success = obj->chance >= pad_random.next() % 100;
                if (success == 1) {
                    if (data->sprite != NULL) {
                        data->sprite->set_anim(data->sprite, (StgtrainSpriteAnim *)stgtrain_module.funcs.get_part_anim(obj->part, 2));
                    }
                    if (data->sprite_2 != NULL) {
                        data->sprite_2->set_anim(data->sprite_2, (StgtrainSpriteAnim *)stgtrain_module.funcs.get_part_anim(8, 2));
                    }
                    obj->result = 1;
                } else {
                    if (data->sprite != NULL) {
                        data->sprite->set_anim(data->sprite, (StgtrainSpriteAnim *)stgtrain_module.funcs.get_part_anim(obj->part, 3));
                    }
                    if (data->sprite_2 != NULL) {
                        data->sprite_2->set_anim(data->sprite_2, (StgtrainSpriteAnim *)stgtrain_module.funcs.get_part_anim(8, 3));
                    }
                    obj->result = 0;
                }
                obj->base.next_step(obj);
                break;
            }
            if (data->sprite != NULL) {
                data->sprite->set_anim(data->sprite, (StgtrainSpriteAnim *)stgtrain_module.funcs.get_part_anim(obj->part, obj->step));
            }
            if (data->sprite_2 != NULL) {
                data->sprite_2->set_anim(data->sprite_2, (StgtrainSpriteAnim *)stgtrain_module.funcs.get_part_anim(8, obj->step));
            }
            obj->result = -1;
            break;
        case 8:
            if (obj->base.step == 0) {
                if (obj->result != 0) {
                    obj->step = 4;
                } else {
                    obj->step = 5;
                }
                if (data->sprite != NULL) {
                    StgtrainSpriteAnim *anim = (StgtrainSpriteAnim *)stgtrain_module.funcs.get_part_anim(obj->part, obj->step);

                    if (anim != NULL) {
                        data->sprite->set_anim(data->sprite, anim);
                    } else {
                        obj->state = 1;
                    }
                }
                if (data->sprite_2 != NULL) {
                    StgtrainSpriteAnim *anim = (StgtrainSpriteAnim *)stgtrain_module.funcs.get_part_anim(8, obj->step);

                    if (anim != NULL) {
                        data->sprite_2->set_anim(data->sprite_2, anim);
                    } else {
                        obj->state = 1;
                    }
                }
                obj->base.step++;
                break;
            }
            if (data->sprite_2 != NULL) {
                done = data->sprite->is_done(data->sprite) & data->sprite_2->is_done(data->sprite_2);
            } else {
                done = data->sprite->is_done(data->sprite);
            }
            if (done != 0) {
            end:
                if (data->sprite != NULL) {
                    data->sprite->pause(data->sprite, 1);
                }
                if (data->sprite_2 != NULL) {
                    data->sprite_2->pause(data->sprite_2, 1);
                }
                obj->step = 0;
                obj->base.set_step(obj, 0);
                obj->state = 1;
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* StgtrainTrainee.pause: stops the sprites' animations. */
void stgtrain_trainee_pause(StgtrainTrainee *obj) {
    StgtrainTraineeData *data = (StgtrainTraineeData *)obj->base.children;

    if (data->sprite != NULL) {
        data->sprite->pause(data->sprite, 1);
    }
    if (data->sprite_2 != NULL) {
        data->sprite_2->pause(data->sprite_2, 1);
    }
}

/* StgtrainTrainee.start_round. */
void stgtrain_trainee_start_round(StgtrainTrainee *obj) {
    StgtrainTraineeData *data = (StgtrainTraineeData *)obj->base.children;

    if (data->sprite != NULL) {
        data->sprite->pause(data->sprite, 0);
    }
    if (data->sprite_2 != NULL) {
        data->sprite_2->pause(data->sprite_2, 0);
    }
    obj->state = 4;
}

void stgtrain_trainee_set_tex(StgtrainTrainee *obj, s32 x, s32 y) {
    obj->pos[0] = x;
    obj->pos[1] = y;
    obj->tex_set = 1;
}

void stgtrain_trainee_set_clut(StgtrainTrainee *obj, s32 x, s32 y) {
    obj->pos[2] = x;
    obj->pos[3] = y;
    obj->clut_set = 1;
}

void stgtrain_trainee_set_chance(StgtrainTrainee *obj, s32 chance) {
    obj->chance = chance;
}

/* StgtrainTrainee.grow: scales the sprites up from nothing. */
void stgtrain_trainee_grow(StgtrainTrainee *obj) {
    obj->scale_step = 0x199;
    obj->scale = 0;
    obj->state = 2;
}

void stgtrain_trainee_shrink(StgtrainTrainee *obj) {
    obj->scale = 0x1000;
    obj->scale_step = -0x333;
    obj->state = 2;
}

void stgtrain_trainee_set_scale(StgtrainTrainee *obj, s32 scale) {
    obj->scale_set = 1;
    obj->scale = scale;
}

/* StgtrainTrainee.get_result: the round's result once it is over (state odd), else -1. */
s32 stgtrain_trainee_get_result(StgtrainTrainee *obj) {
    if (obj->state & 1) {
        return obj->result;
    }
    return -1;
}

void stgtrain_trainee_end(StgtrainTrainee *obj) {
    obj->state = 8;
    obj->base.step = 0;
}

StgtrainTrainee *stgtrain_trainee_create(s32 part, s32 file, s32 layer, s32 ot_depth) {
    StgtrainTrainee *obj = object_new(stgtrain_trainee_update, sizeof(StgtrainTrainee), sizeof(StgtrainTraineeData));

    obj->set_tex = stgtrain_trainee_set_tex;
    obj->set_clut = stgtrain_trainee_set_clut;
    obj->set_chance = stgtrain_trainee_set_chance;
    obj->grow = stgtrain_trainee_grow;
    obj->shrink = stgtrain_trainee_shrink;
    obj->start_round = stgtrain_trainee_start_round;
    obj->pause = stgtrain_trainee_pause;
    obj->set_scale = stgtrain_trainee_set_scale;
    obj->get_result = stgtrain_trainee_get_result;
    obj->part = part;
    obj->file = file;
    obj->layer = layer;
    obj->ot_depth = ot_depth;
    obj->end = stgtrain_trainee_end;
    return obj;
}

/* The training choice's data block (0x10 bytes). */
typedef struct StgtrainChoiceData {
    /* 0x0 */ MessageWindow *question;
    /* 0x4 */ MessageWindow *hint;  /* the page hint */
    /* 0x8 */ MessageWindow *name;  /* the training's name */
    /* 0xC */ MessageWindow *description; /* its description */
} StgtrainChoiceData; /* size 0x10 */

void stgtrain_choice_create_windows(StgtrainChoice *obj, StgtrainChoiceData *data) {
    data->question = message_create_window(obj->layer, 1, 0xAE, 0x49);
    data->hint = message_create_window(obj->layer, 1, 0xA3, 0xA0);
    data->name = message_create_window(obj->layer, 1, 0x74, 0xC0);
    data->description = message_create_window(obj->layer, 1, 0x74, 0xCE);
}

/* Shows the selected training's name and description (or hides them). */
void stgtrain_choice_show_training(StgtrainChoice *obj, StgtrainChoiceData *data, s32 show) {
    s32 id;

    if (show) {
        id = obj->grid[obj->page][obj->col + obj->row * 4];
        if (id > 0) {
            data->name->set_text(data->name, cdload_module.files.get_file(records_language + 0x10B), stgtrain_module.funcs.trainings[id].name);
            data->description->set_text(data->description, cdload_module.files.get_file(records_language + 0x10B), stgtrain_module.funcs.trainings[id].desc);
            return;
        }
    }
    data->name->set_visible(data->name, 0);
    data->description->set_visible(data->description, 0);
}

void stgtrain_choice_draw(StgtrainChoice *obj) {
    Sprite spr;
    s32 i;
    s32 id;
    s32 x, y;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer, obj->ot_depth);
    spr.set_vram_pos(0x240, 0x100);
    if (obj->anims[0].level != 0) {
        if (obj->anims[0].level != 0x1000) {
            spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x4E);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x22, 0x92, 0x43);
    }
    if (obj->cursor != 0) {
        if (gfx_module.funcs.get_time() - obj->cursor_time >= 11) {
            obj->cursor_time = gfx_module.funcs.get_time();
            obj->cursor_frame++;
            if (obj->cursor_frame >= 4) {
                obj->cursor_frame = 0;
            }
        }
        spr.set_palette(obj->cursor_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x1E, obj->col * 0x28 + 0x94, obj->row * 0x28 + 0x64);
        spr.set_palette(0);
    }
    if (obj->anims[2].level != 0) {
        if (gfx_module.funcs.get_time() - obj->icon_time >= 16) {
            obj->icon_time = gfx_module.funcs.get_time();
            obj->icon_frame++;
            if (obj->icon_frame >= 4) {
                obj->icon_frame = 0;
            }
        }
        if (obj->anims[2].level != 0x1000) {
            spr.set_scale(obj->anims[2].level, obj->anims[2].level, 0x1000);
        }
        for (i = 0; i < 8; i++) {
            id = obj->grid[obj->page][i];
            if (id != 0) {
                x = i % 4 * 0x28;
                y = i / 4 * 0x28;
                if (obj->anims[2].level != 0x1000) {
                    spr.set_pivot(x + 0xA8, y + 0x76);
                }
                if (id == -1) {
                    spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x5F, x + 0x94, y + 0x64);
                } else if (i == obj->col + obj->row * 4) {
                    spr.set_palette(0);
                    spr.draw(cdload_module.get_subfile_by_id(0x028C0000), stgtrain_module.funcs.trainings[id].icons[obj->icon_frame], x + 0x94,
                               y + 0x64);
                } else {
                    spr.set_palette(1);
                    spr.draw(cdload_module.get_subfile_by_id(0x028C0000), stgtrain_module.funcs.trainings[id].icons[0], x + 0x94, y + 0x64);
                }
            }
        }
    }
    if (obj->page_arrow != 0) {
        if (gfx_module.funcs.get_time() - obj->arrow_time >= 11) {
            obj->arrow_time = gfx_module.funcs.get_time();
            obj->arrow_frame++;
            if (obj->arrow_frame >= 4) {
                obj->arrow_frame = 0;
            }
        }
        spr.set_palette(obj->arrow_frame);
        if (obj->page == 0) {
            spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x2C, 0xE4, 0xA0);
        } else {
            spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x2B, 0x94, 0xA0);
        }
    }
    spr.set_palette(0);
    if (obj->anims[1].level != 0) {
        spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
        if (obj->anims[1].level != 0x1000) {
            spr.set_pivot(0x140, 0x8A);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x24, 0x8F, 0x5F);
    }
    if (obj->anims[3].level != 0) {
        if (obj->anims[3].level != 0x1000) {
            spr.set_scale(obj->anims[3].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0xCD);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x23, 0x46, 0xBA);
    }
}

void stgtrain_choice_run(StgtrainChoice *obj, StgtrainChoiceData *data) {
    s32 prev; /* the page, then the loop index, then the column */
    s32 row;

    switch (obj->base.step) {
    case 0:
    default:
        stgtrain_module.funcs.anim_start(&obj->anims[0], 1);
        obj->base.step++;
        break;
    case 1:
        if (stgtrain_module.funcs.anim_update(&obj->anims[0])) {
            data->question->set_text(data->question, cdload_module.files.get_file(records_language + 0x10B), 7);
            stgtrain_module.funcs.anim_start(&obj->anims[1], 1);
            obj->base.step++;
        }
        break;
    case 2:
        if (stgtrain_module.funcs.anim_update(&obj->anims[1])) {
            stgtrain_module.funcs.anim_start(&obj->anims[2], 1);
            obj->base.step++;
        }
        break;
    case 3:
        if (stgtrain_module.funcs.anim_update(&obj->anims[2])) {
            if (stgtrain_module.count >= 6) {
                obj->page_arrow = 1;
                if (obj->page == 0) {
                    data->hint->set_text(data->hint, cdload_module.files.get_file(records_language + 0x10B), 0x45);
                    data->hint->set_pos(data->hint, 0xE4, 0xA0);
                } else {
                    data->hint->set_text(data->hint, cdload_module.files.get_file(records_language + 0x10B), 0x44);
                    data->hint->set_pos(data->hint, 0xA3, 0xA0);
                }
            }
            stgtrain_module.funcs.anim_start(&obj->anims[3], 1);
            obj->base.step++;
        }
        break;
    case 4:
        if (stgtrain_module.funcs.anim_update(&obj->anims[3])) {
            stgtrain_choice_show_training(obj, data, 1);
            obj->cursor = 1;
            obj->base.step = 10;
        }
        break;
    case 10:
        prev = obj->page;
        if (stgtrain_module.count >= 6) {
            if (!PAD_HELD(11) && PAD_PRESSED(10)) {
                obj->page = 0;
            } else if (!PAD_HELD(10) && PAD_PRESSED(11)) {
                obj->page = 1;
            }
        }
        if (prev != obj->page) {
            sound_module.play(0x4001B);
            if (obj->page == 0) {
                data->hint->set_text(data->hint, cdload_module.files.get_file(records_language + 0x10B), 0x45);
                data->hint->set_pos(data->hint, 0xE4, 0xA0);
            } else {
                data->hint->set_text(data->hint, cdload_module.files.get_file(records_language + 0x10B), 0x44);
                data->hint->set_pos(data->hint, 0xA3, 0xA0);
            }
            for (prev = 0; prev < 8; prev++) {
                if (obj->grid[obj->page][prev] > 0) {
                    obj->col = prev % 4;
                    obj->row = prev / 4;
                    break;
                }
            }
            stgtrain_choice_show_training(obj, data, 1);
            obj->icon_frame = 0;
            break;
        }
        prev = obj->col;
        row = obj->row;
        if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
            do {
                obj->col--;
                if (obj->col < 0) {
                    obj->col = 0;
                    break;
                }
            } while (obj->grid[obj->page][obj->col + obj->row * 4] <= 0);
        } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
            do {
                obj->col++;
                if (obj->col >= 4) {
                    obj->col = 3;
                    break;
                }
            } while (obj->grid[obj->page][obj->col + obj->row * 4] <= 0);
        }
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            do {
                obj->row--;
                if (obj->row < 0) {
                    obj->row = 0;
                    break;
                }
            } while (obj->grid[obj->page][obj->col + obj->row * 4] <= 0);
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            do {
                obj->row++;
                if (obj->row >= 2) {
                    obj->row = 1;
                    break;
                }
            } while (obj->grid[obj->page][obj->col + obj->row * 4] <= 0);
        }
        if (prev != obj->col || row != obj->row) {
            if (obj->grid[obj->page][obj->col + obj->row * 4] > 0) {
                sound_module.play(0x4001B);
                stgtrain_choice_show_training(obj, data, 1);
            } else {
                obj->col = prev;
                obj->row = row;
            }
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x4001B);
            obj->main->training = obj->grid[obj->page][obj->col + obj->row * 4];
            if (obj->main->training > 0) {
                obj->base.step = 50;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step = 50;
            obj->base.substep = 1;
        }
        break;
    case 50:
        obj->cursor = 0;
        obj->page_arrow = 0;
        data->hint->set_visible(data->hint, 0);
        stgtrain_module.funcs.anim_start(&obj->anims[2], 0);
        obj->base.step++;
        break;
    case 51:
        if (stgtrain_module.funcs.anim_update(&obj->anims[2])) {
            stgtrain_choice_show_training(obj, data, 0);
            data->question->set_visible(data->question, 0);
            stgtrain_module.funcs.anim_start(&obj->anims[0], 0);
            stgtrain_module.funcs.anim_start(&obj->anims[1], 0);
            stgtrain_module.funcs.anim_start(&obj->anims[3], 0);
            obj->base.step++;
        }
        break;
    case 52:
        stgtrain_module.funcs.anim_update(&obj->anims[0]);
        stgtrain_module.funcs.anim_update(&obj->anims[1]);
        if (stgtrain_module.funcs.anim_update(&obj->anims[3])) {
            if (obj->base.substep != 0) {
                obj->base.set_state(obj, OBJECT_STATE_DONE);
                break;
            }
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}

void stgtrain_choice_update(StgtrainChoice *obj, StgtrainChoiceData *data) {
    StgtrainMenuEntry *menu;
    s32 page;
    s32 row;
    s32 col;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stgtrain_choice_create_windows(obj, data);
        obj->anims[0].duration = 10;
        obj->anims[1].duration = 10;
        obj->anims[2].duration = 10;
        obj->anims[3].duration = 10;
        menu = stgtrain_module.funcs.get_menu(gamestate_data.funcs.get_map_entry());
        for (row = 0; row < 2; row++) {
            for (col = 0; col < 3; col++) {
                if (row == 1 && col == 2) {
                    break;
                }
                obj->grid[0][col + row * 4] = -1;
            }
        }
        for (row = 0; row < 2; row++) {
            for (col = 0; col < 4; col++) {
                if (row == 1 && col == 0) {
                    continue;
                }
                obj->grid[1][col + row * 4] = -1;
            }
        }
        for (i = 0; i < 16; i++) {
            switch (menu[i].id) {
            case 1:
            case 13:
                obj->grid[0][0] = menu[i].id;
                break;
            case 2:
            case 14:
                obj->grid[0][1] = menu[i].id;
                break;
            case 3:
            case 15:
                obj->grid[0][2] = menu[i].id;
                break;
            case 4:
            case 16:
                obj->grid[0][4] = menu[i].id;
                break;
            case 5:
            case 17:
                obj->grid[0][5] = menu[i].id;
                break;
            case 6:
            case 18:
                obj->grid[1][0] = menu[i].id;
                break;
            case 7:
            case 19:
                obj->grid[1][1] = menu[i].id;
                break;
            case 8:
            case 20:
                obj->grid[1][2] = menu[i].id;
                break;
            case 9:
            case 21:
                obj->grid[1][3] = menu[i].id;
                break;
            case 10:
            case 22:
                obj->grid[1][5] = menu[i].id;
                break;
            case 11:
            case 23:
                obj->grid[1][6] = menu[i].id;
                break;
            case 12:
            case 24:
                obj->grid[1][7] = menu[i].id;
                break;
            }
        }
        if (obj->main->training > 0) {
            for (page = 0; page < 2; page++) {
                for (row = 0; row < 2; row++) {
                    for (col = 0; col < 4; col++) {
                        if (obj->grid[page][col + row * 4] == obj->main->training) {
                            obj->page = page;
                            obj->col = col;
                            obj->row = row;
                            break;
                        }
                    }
                }
            }
        }
        break;
    case OBJECT_STATE_RUN:
        stgtrain_choice_run(obj, data);
        stgtrain_choice_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* StgtrainChoice.open. */
void stgtrain_choice_open(StgtrainChoice *obj) {
    obj->base.state = OBJECT_STATE_RUN;
    obj->base.step = 0;
}

/* StgtrainChoice.close. */
void stgtrain_choice_close(StgtrainChoice *obj) {
    obj->base.state = OBJECT_STATE_RUN;
    obj->base.step = 50;
}

/* StgtrainChoice.show_training. */
void stgtrain_choice_show(StgtrainChoice *obj) {
    stgtrain_choice_show_training(obj, (StgtrainChoiceData *)obj->base.children, 1);
}

StgtrainChoice *stgtrain_choice_create(StgtrainMain *main) {
    StgtrainChoice *obj = object_new(stgtrain_choice_update, sizeof(StgtrainChoice), sizeof(StgtrainChoiceData));

    obj->open = stgtrain_choice_open;
    obj->close = stgtrain_choice_close;
    obj->show = stgtrain_choice_show;
    obj->layer = 0x1000;
    obj->ot_depth = 6;
    obj->main = main;
    return obj;
}

/* Loads the overlay's image into VRAM (stgtrain_module.funcs.load). */
void stgtrain_load_image(void) {
    Tim tim;

    tim_init(&tim);
    tim.set_image_pos(0x240, 0x100);
    tim.load_all(cdload_module.get_subfile_by_id(0x028D0000));
}

/* window_anim_start (include/window_anim.h), byte for byte. */
void stgtrain_window_anim_start(WindowAnim *anim, s32 open) {
    anim->running = 1;
    if (open) {
        sound_module.play(0x40019);
        anim->step = 0x1000 / anim->duration;
        anim->level = 0;
    } else {
        sound_module.play(0x4001A);
        anim->level = 0x1000;
        anim->step = -(0x1000 / anim->duration * 2);
    }
}

/* window_anim_update (include/window_anim.h), byte for byte. */
s32 stgtrain_window_anim_update(WindowAnim *anim) {
    if (anim->running == 0) {
        return 1;
    }
    anim->level += anim->step;
    if (anim->step > 0) {
        if (anim->level > 0x1000) {
            anim->level = 0x1000;
            anim->running = 0;
            return 1;
        }
    } else if (anim->level < 0) {
        anim->level = 0;
        anim->running = 0;
        return 1;
    }
    return 0;
}

void stgtrain_tween_start(Tween *obj, s32 from, s32 to, s32 frames) {
    if (from != to) {
        obj->duration = frames;
        obj->acc = from << 8;
        obj->value = from;
        obj->target = to;
        obj->running = 1;
        obj->step = ((to - from) << 8) / obj->duration;
    }
}

s32 stgtrain_tween_update(Tween *obj) {
    if (obj->running == 0) {
        return 1;
    }
    obj->acc += obj->step;
    obj->value = obj->acc >> 8;
    if (obj->step > 0) {
        if (obj->value > obj->target) {
            obj->value = obj->target;
            obj->running = 0;
            return 1;
        }
    } else if (obj->value < obj->target) {
        obj->value = obj->target;
        obj->running = 0;
        return 1;
    }
    return 0;
}

/* Starts loading animation file `file` (stgtrain_module.funcs.file_load). */
s32 stgtrain_anim_load(s32 file) {
    StgtrainModule *state;

    if (file < 0) {
        return 0;
    }
    state = &stgtrain_module;
    if (file != state->file) {
        heap_funcs.bzero(&state->data, 0x2D8); /* data, file and parts */
        state->file = file;
        cdload_module.queue_file(stgtrain_anim_files[file].file);
        state->data = NULL;
    }
    return 1;
}

/* Returns the animation file once it is loaded, else 0 (stgtrain_module.funcs.file_get). */
u8 *stgtrain_anim_get_data(void) {
    StgtrainAnimFile *files = stgtrain_anim_files;
    StgtrainModule *state = &stgtrain_module;

    if (cdload_module.is_loading(files[state->file].file) == 0) {
        state->data = cdload_module.files.get_file(files[state->file].file);
    }
    return state->data;
}

/* Frees the animation file (stgtrain_module.funcs.file_free). */
void stgtrain_anim_free(void) {
    if (stgtrain_module.file != -1) {
        cdload_module.files.free_file(stgtrain_anim_files[stgtrain_module.file].file);
    }
}

s32 stgtrain_anim_parse_header(s32 *hdr, s32 part) {
    StgtrainModule *state = &stgtrain_module;

    stgtrain_header_reader.pos = hdr;
    state->parts[part].sprite_data = state->data + hdr[1];
    stgtrain_header_reader.pos += *stgtrain_header_reader.pos + 1; /* skip the offsets */
    state->parts[part].sprite_offset = *stgtrain_header_reader.pos++;
    state->parts[part].unk_0C = *stgtrain_header_reader.pos;
    return 1;
}

s32 stgtrain_anim_parse_10(StgtrainAnimHeader *hdr, s32 part) {
    s32 i;

    stgtrain_anims_reader.hdr = hdr;
    for (i = 0; i < 6; i++) {
        if (stgtrain_anims_reader.hdr->anims[i] != 0) {
            stgtrain_module.parts[part].anims[i] = stgtrain_module.data + stgtrain_anims_reader.hdr->anims[i];
        } else {
            stgtrain_module.parts[part].anims[i] = NULL;
        }
    }
    return 1;
}

s32 stgtrain_anim_parse_images(StgtrainAnimHeader *hdr, s32 part) {
    s32 i;

    stgtrain_images_reader.hdr = hdr;
    stgtrain_module.parts[part].count = hdr->count - 7;
    for (i = 0; i < stgtrain_module.parts[part].count; i++) {
        stgtrain_module.parts[part].images[i] =
            (u32 *)(stgtrain_module.data + stgtrain_images_reader.hdr->images[i]);
    }
    return 1;
}

/* Reads part `part` (0..8) of the loaded animation file (stgtrain_module.funcs.parse). */
s32 stgtrain_anim_parse(s32 part) {
    StgtrainModule *state = &stgtrain_module;

    if (state->data == NULL || part >= 9) {
        return 0;
    }
    state->parts[part].id = part;
    stgtrain_parse_reader.hdr = (StgtrainAnimHeader *)((s32 *)state->data + part);
    stgtrain_parse_reader.hdr = (StgtrainAnimHeader *)(state->data + stgtrain_parse_reader.hdr->count);
    if (stgtrain_parse_reader.hdr->count == 0) {
        return 0;
    }
    if (stgtrain_anim_parse_header((s32 *)stgtrain_parse_reader.hdr, part) == 0) {
        return 0;
    }
    if (stgtrain_anim_parse_10(stgtrain_parse_reader.hdr, part) == 0) {
        return 0;
    }
    return stgtrain_anim_parse_images(stgtrain_parse_reader.hdr, part) != 0;
}

/* Puts part `part`'s images into VRAM from pos[0], pos[1] (one every 64 columns), their CLUTs at pos[2] (+ the
 * image's own offset), pos[3], decompressing "RLEN" images (stgtrain_module.funcs.upload). */
s32 stgtrain_anim_upload(s32 part, s32 *pos) {
    StgtrainModule *state = &stgtrain_module;
    Tim tim;
    s32 x0, x, y, clut_x, clut_y;
    s32 i, n, k;
    u8 c;
    u8 *buf;
    u8 *src;
    u8 *dst; /* the image to upload, also the decompression's output */
    u32 magic;

    if (part != state->parts[part].id) {
        return 0;
    }
    x0 = pos[0];
    y = pos[1];
    clut_x = pos[2];
    clut_y = pos[3];
    tim_init(&tim);
    state->parts[part].y = y;
    buf = heap_funcs.alloc(0xA800, 2);
    for (i = 0; i < state->parts[part].count; i++) {
        x = x0 + i * 0x40;
        stgtrain_module.parts[part].x[i] = x;
        tim.set_image_pos(x, y);
        src = (u8 *)stgtrain_module.parts[part].images[i];
        magic = 0x4E454C52; /* "RLEN" */
        /* FAKE: the constant in a variable set before an empty loop (a scheduling barrier). The original schedules
         * [lui, lw src, ori, lw *src, nop, bne]: the constant and src are ready before the image word's load,
         * which no single block's schedule gives (sched2 puts the ori next to the bne). */
        LOOP_BARRIER();
        if (*(u32 *)src == magic) {
            src += 8;
            dst = buf;
            c = *src;
            while (c != 0) {
                if (c & 0x80) {
                    n = c & 0x7F;
                    src++;
                    for (k = 0; k < n; k++) {
                        *dst++ = *src;
                    }
                    src++;
                } else {
                    n = *src++;
                    for (k = 0; k < n; k++) {
                        *dst++ = *src++;
                    }
                }
                c = *src;
            }
            dst = buf;
        } else {
            dst = src;
        }
        tim.set_clut_pos(clut_x + ((s16 *)dst)[6], clut_y);
        tim.load((u32 *)dst);
    }
    heap_funcs.free(buf);
    return 1;
}

s32 stgtrain_anim_get_file_id(s32 file) {
    return stgtrain_anim_files[file].file;
}

s32 stgtrain_anim_get_file_pos(s32 file) {
    return (stgtrain_anim_files[file].y << 16) | stgtrain_anim_files[file].x;
}

s32 stgtrain_anim_get_file_unk_C(s32 file) {
    return stgtrain_anim_files[file].unk_C;
}

u8 *stgtrain_anim_get_part_data(s32 part) {
    return stgtrain_module.parts[part].sprite_data;
}

s32 stgtrain_anim_get_part_offset(s32 part) {
    return stgtrain_module.parts[part].sprite_offset;
}

s32 stgtrain_anim_get_part_0C(s32 part) {
    return stgtrain_module.parts[part].unk_0C;
}

u8 *stgtrain_anim_get_part_anim(s32 part, s32 i) {
    return stgtrain_module.parts[part].anims[i];
}

/* Returns training menu `menu` (1..14, else 0) and counts its entries into stgtrain_module.count
 * (stgtrain_module.funcs.get_menu). */
StgtrainMenuEntry *stgtrain_get_menu(s32 menu) {
    s32 i;

    if ((u32)(menu - 1) >= 14) {
        menu = 0;
    }
    stgtrain_module.count = 0;
    for (i = 0; i < 16; i++) {
        if (stgtrain_menus[menu][i].id != 0) {
            stgtrain_module.count++;
        }
    }
    return stgtrain_menus[menu];
}

/* Returns the entry `id` of training menu `menu`, NULL if it has none (stgtrain_module.funcs.find_menu_entry). */
StgtrainMenuEntry *stgtrain_find_menu_entry(s32 menu, s32 id) {
    s32 i;

    if ((u32)(menu - 1) >= 14) {
        menu = 0;
    }
    for (i = 0; i < 16; i++) {
        if (stgtrain_menus[menu][i].id == id) {
            return &stgtrain_menus[menu][i];
        }
    }
    return NULL;
}

s32 stgtrain_tp_costs[3] = { 1, 5, 10 };

StgtrainMenuEntry stgtrain_menus[14][16] = {
    {
        { 1, 1, 0 }, { 2, 2, 0 }, { 3, 3, 0 }, { 4, 4, 0 },
        { 5, 5, 0 },
    },
    {
        { 1, 1, 0 }, { 2, 2, 0 }, { 3, 3, 0 }, { 4, 4, 0 },
        { 5, 5, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 6, 8, 16 }, { 7, 9, 2 }, { 8, 10, 5 }, { 9, 11, 1 },
        { 10, 12, 4 },
    },
    {
        { 1, 1, 0 }, { 2, 2, 0 }, { 3, 3, 0 }, { 4, 4, 0 },
        { 5, 5, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 6, 8, 16 }, { 7, 9, 2 }, { 8, 10, 5 }, { 9, 11, 1 },
        { 10, 12, 4 }, { 12, 14, 3 },
    },
    {
        { 1, 1, 0 }, { 2, 2, 0 }, { 3, 3, 0 }, { 4, 4, 0 },
        { 5, 5, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 6, 8, 16 }, { 7, 9, 2 }, { 8, 10, 5 }, { 9, 11, 1 },
        { 10, 12, 4 }, { 11, 13, 15 }, { 12, 14, 3 },
    },
    {
        { 13, 1, 0 }, { 14, 2, 0 }, { 15, 3, 0 }, { 16, 4, 0 },
        { 5, 5, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 6, 8, 16 }, { 7, 9, 2 }, { 8, 10, 5 }, { 9, 11, 1 },
        { 10, 12, 4 }, { 11, 13, 15 }, { 12, 14, 3 },
    },
    {
        { 13, 1, 0 }, { 14, 2, 0 }, { 15, 3, 0 }, { 16, 4, 0 },
        { 5, 5, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 6, 8, 16 }, { 7, 9, 2 }, { 20, 10, 5 }, { 9, 11, 1 },
        { 10, 12, 4 }, { 11, 13, 15 }, { 12, 14, 3 },
    },
    {
        { 13, 1, 0 }, { 14, 2, 0 }, { 15, 3, 0 }, { 16, 4, 0 },
        { 17, 5, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 6, 8, 16 }, { 7, 9, 2 }, { 20, 10, 5 }, { 9, 11, 1 },
        { 10, 12, 4 }, { 11, 13, 15 }, { 12, 14, 3 },
    },
    {
        { 1, 1, 0 }, { 2, 2, 0 }, { 3, 3, 0 }, { 4, 4, 0 },
        { 5, 5, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 6, 8, 16 }, { 19, 9, 2 }, { 8, 10, 5 }, { 9, 11, 1 },
        { 22, 12, 4 }, { 11, 13, 15 }, { 12, 14, 3 },
    },
    {
        { 13, 1, 0 }, { 14, 2, 0 }, { 15, 3, 0 }, { 16, 4, 0 },
        { 17, 5, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 6, 8, 16 }, { 19, 9, 2 }, { 20, 10, 5 }, { 9, 11, 1 },
        { 22, 12, 4 }, { 11, 13, 15 }, { 12, 14, 3 },
    },
    {
        { 1, 1, 0 }, { 2, 2, 0 }, { 3, 3, 0 }, { 4, 4, 0 },
        { 5, 5, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 6, 8, 16 }, { 7, 9, 2 }, { 8, 10, 5 }, { 21, 11, 1 },
        { 10, 12, 4 }, { 23, 13, 15 }, { 12, 14, 3 },
    },
    {
        { 13, 1, 0 }, { 14, 2, 0 }, { 15, 3, 0 }, { 16, 4, 0 },
        { 17, 5, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 6, 8, 16 }, { 7, 9, 2 }, { 20, 10, 5 }, { 21, 11, 1 },
        { 10, 12, 4 }, { 23, 13, 15 }, { 12, 14, 3 },
    },
    {
        { 13, 1, 0 }, { 14, 2, 0 }, { 15, 3, 0 }, { 16, 4, 0 },
        { 5, 5, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 18, 8, 16 }, { 19, 9, 2 }, { 8, 10, 5 }, { 21, 11, 1 },
        { 22, 12, 4 }, { 23, 13, 15 }, { 12, 14, 3 },
    },
    {
        { 13, 1, 0 }, { 14, 2, 0 }, { 15, 3, 0 }, { 16, 4, 0 },
        { 17, 5, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 18, 8, 16 }, { 19, 9, 2 }, { 20, 10, 5 }, { 21, 11, 1 },
        { 22, 12, 4 }, { 23, 13, 15 }, { 12, 14, 3 },
    },
    {
        { 13, 1, 0 }, { 14, 2, 0 }, { 15, 3, 0 }, { 16, 4, 0 },
        { 17, 5, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 18, 8, 16 }, { 19, 9, 2 }, { 20, 10, 5 }, { 21, 11, 1 },
        { 22, 12, 4 }, { 23, 13, 15 }, { 24, 14, 3 },
    },
};

StgtrainTraining stgtrain_trainings[25] = {
    { 0, 0, 0, 0, 0, 0 },
    { 0x13, 0x2B, 0, 1, 2, 3 },
    { 0x14, 0x2C, 4, 5, 6, 7 },
    { 0x15, 0x2D, 8, 9, 0xA, 0xB },
    { 0x16, 0x2E, 0xC, 0xD, 0xE, 0xF },
    { 0x17, 0x2F, 0x10, 0x11, 0x12, 0x13 },
    { 0x1D, 0x35, 0x14, 0x15, 0x16, 0x17 },
    { 0x1E, 0x36, 0x18, 0x19, 0x1A, 0x1B },
    { 0x1F, 0x37, 0x1C, 0x1D, 0x1E, 0x1F },
    { 0x20, 0x38, 0x20, 0x21, 0x22, 0x23 },
    { 0x21, 0x39, 0x24, 0x25, 0x26, 0x27 },
    { 0x22, 0x3A, 0x28, 0x29, 0x2A, 0x2B },
    { 0x23, 0x3B, 0x5F, 0x60, 0x61, 0x62 },
    { 0x18, 0x30, 0x2D, 0x2E, 0x2F, 0x30 },
    { 0x19, 0x31, 0x31, 0x32, 0x33, 0x34 },
    { 0x1A, 0x32, 0x35, 0x36, 0x37, 0x38 },
    { 0x1B, 0x33, 0x39, 0x3A, 0x3B, 0x3C },
    { 0x1C, 0x34, 0x3D, 0x3E, 0x3F, 0x40 },
    { 0x24, 0x3C, 0x41, 0x42, 0x43, 0x44 },
    { 0x25, 0x3D, 0x47, 0x48, 0x49, 0x4A },
    { 0x26, 0x3E, 0x4B, 0x4C, 0x4D, 0x4E },
    { 0x27, 0x3F, 0x4F, 0x50, 0x51, 0x52 },
    { 0x28, 0x40, 0x53, 0x54, 0x55, 0x56 },
    { 0x29, 0x41, 0x57, 0x58, 0x59, 0x5A },
    { 0x2A, 0x42, 0x5B, 0x5C, 0x5D, 0x5E },
};

StgtrainAnimFile stgtrain_anim_files[25] = {
    { 0x25E, 0x52, 0x1F, 0 },
    { 0x25E, 0x52, 0x1F, 0 },
    { 0x43E, 0x4F, 0x1F, 0 },
    { 0x280, 0x4E, 0x1B, 0 },
    { 0x281, 0x4F, 0x18, 1 },
    { 0x282, 0x4F, 0x1F, 0 },
    { 0x283, 0x4F, 0x1F, 0 },
    { 0x302, 0x4F, 0x1F, 0 },
    { 0x303, 0x51, 0x1F, 1 },
    { 0x43F, 0x4F, 0x1F, 0 },
    { 0x284, 0x2F, 0x23, 1 },
    { 0x285, 0x4F, 0x1F, 0 },
    { 0x304, 0x4F, 0x1F, 1 },
    { 0x25E, 0x52, 0x1F, 0 },
    { 0x43E, 0x4F, 0x1F, 0 },
    { 0x280, 0x4E, 0x1B, 0 },
    { 0x281, 0x4F, 0x18, 1 },
    { 0x282, 0x4F, 0x1F, 0 },
    { 0x283, 0x4F, 0x1F, 0 },
    { 0x302, 0x4F, 0x1F, 0 },
    { 0x303, 0x51, 0x1F, 1 },
    { 0x43F, 0x4F, 0x1F, 0 },
    { 0x284, 0x2F, 0x23, 1 },
    { 0x285, 0x4F, 0x1F, 0 },
    { 0x304, 0x4F, 0x1F, 1 },
};

s32 stgtrain_anim_upload(s32 part, s32 *pos);

StgtrainModule stgtrain_module = {
    .funcs = {
        stgtrain_trainings,
        stgtrain_load_image,
        stgtrain_window_anim_start,
        stgtrain_window_anim_update,
        stgtrain_tween_start,
        stgtrain_tween_update,
        stgtrain_anim_load,
        stgtrain_anim_get_data,
        stgtrain_anim_free,
        stgtrain_anim_parse,
        stgtrain_anim_upload,
        stgtrain_anim_get_file_id,
        stgtrain_anim_get_file_pos,
        stgtrain_anim_get_file_unk_C,
        stgtrain_anim_get_part_data,
        stgtrain_anim_get_part_offset,
        stgtrain_anim_get_part_0C,
        stgtrain_anim_get_part_anim,
        stgtrain_get_menu,
        stgtrain_find_menu_entry,
    },
};
