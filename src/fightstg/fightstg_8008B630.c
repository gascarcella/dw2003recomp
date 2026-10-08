#include "common.h"

#include "cdload.h"
#include "gfx.h"
#include "records.h"
#include "heap.h"
#include "object.h"
#include "sound.h"
#include "fightstg.h"

extern s32 fightstg_script_saved_idle_anim;

/* Data block of fightstg_script_create's object. */
typedef struct FightstgScriptData {
    /* 0x00 */ struct FightstgFlash *flash; /* command 7: a fade (fightstg_flash_create) */
    /* 0x04 */ Object *jump;
    /* 0x08 */ void *move;
    /* 0x0C */ void *sound;        /* command 10 */
    /* 0x10 */ Object *effects[8];
    /* 0x30 */ void *props[3];     /* command 6: models (fightstg_prop_create) */
    /* 0x3C */ FightstgScript *child; /* command 1: a nested script */
} FightstgScriptData; /* size 0x40 */

void fightstg_script_run_child(FightstgScript *obj, FightstgScriptData *data);
s32 fightstg_script_run_model(FightstgScript *obj, FightstgScriptData *data);
s32 fightstg_script_run_effect(FightstgScript *obj, FightstgScriptData *data);
s32 fightstg_script_run_prop(FightstgScript *obj, FightstgScriptData *data);
void fightstg_script_run_camera(FightstgScript *obj, FightstgScriptData *data);
s32 fightstg_script_run_wait(FightstgScript *obj, FightstgScriptData *data);
s32 fightstg_script_run_stage(FightstgScript *obj, FightstgScriptData *data);
void fightstg_script_run_sound(FightstgScript *obj, FightstgScriptData *data);
void fightstg_script_run_flash(FightstgScript *obj, FightstgScriptData *data);

/* In fightstg_8008D3B4 (defined there as void; the object stays in v0 and command 2 keeps it; OBJECT_V0). */
Object *fightstg_jump_create(FightstgModelParams *params, s32 kind, s32 distance);
void *fightstg_move_create(FightstgModelParams *params, void *pos, s32 frames);

/* fightstg_attack_create's object: a side's attack, shown in steps (message, damage, knock-out). */
typedef struct FightstgAttack {
    /* 0x00 */ Object base;
    /* 0x50 */ u8 side; /* side: 0 or 0x10 */
    /* 0x51 */ u8 pad_51[0x3];
    /* 0x54 */ s32 tech; /* record of records_techniques (from 1) */
    /* 0x58 */ s32 damage;
    /* 0x5C */ s32 target_asleep;
    /* 0x60 */ s32 args[3]; /* the message's arguments (fightstg_message_show's args) */
    /* 0x6C */ u8 unk_6C[0x14];
} FightstgAttack; /* size 0x80 */

/* fightstg_message_create's object (fightstg_8008D3B4), the part used here. */
typedef struct FightstgMessage {
    /* 0x00 */ Object base;
    /* 0x50 */ u8 unk_50[0x5C];
    /* 0xAC */ void (*show)(struct FightstgMessage *, s32, s32 *); /* fightstg_message_show */
    /* 0xB0 */ void (*close)();
} FightstgMessage; /* size 0xB4 */

/* In records's .sdata: records_get_digimon (gamestate.c declares it void). */
extern RecordsDigimon *(*records_get_digimon_func)(s32 id);

/* In fightstg_8008D3B4 (defined there as void; their objects stay in v0 and the callers keep them; OBJECT_V0). */
FightstgMessage *fightstg_message_create(void);
void *fightstg_counter_create(s32 side, s32 damage_taken, s32 no_knockout);
void *fightstg_boss_turn_create(s32 arg0, s32 arg1);
void fightstg_events_add_knockout(u8 side);

void fightstg_script_update(FightstgScript *obj, FightstgScriptData *data);

/* fightstg_defeat_camera_create's object. */
typedef struct FightstgDefeatCamera {
    /* 0x00 */ Object base;
    /* 0x50 */ FightstgCamera *camera; /* the camera */
} FightstgDefeatCamera; /* size 0x54 */

void fightstg_defeat_camera_update(FightstgDefeatCamera *obj, FightstgScript **data);
void fightstg_attack_update();

/* The slot of role `role` in a script: 1-3 = slots 0-2, 4-6 = slots 0x10-0x12, else the script's own. */
s32 fightstg_script_get_slot(FightstgScript *obj, s32 role) {
    switch (role) {
    case 0:
    default:
        return (obj->side != 0) << 4;
    case 1:
        return 0;
    case 2:
        return 1;
    case 3:
        return 2;
    case 4:
        return 0x10;
    case 5:
        return 0x11;
    case 6:
        return 0x12;
    }
}

/* Command 1 (op): 0 = start script results[3] + 1 for the other side, 5 = start the next of scripts
 * results[0..2] + 1 for the other side. */
void fightstg_script_run_child(FightstgScript *obj, FightstgScriptData *data) {
    s32 script;

    switch (*obj->next++) {
    case 0:
        if (data->child != NULL) {
            data->child->base.destroy(data->child);
        }
        data->child = fightstg_script_create();
        data->child->side = obj->side == 0;
        data->child->script = obj->results[3] + 1;
        break;
    case 1: /* commands 1-4 do nothing (their cases shape the compare tree) */
    case 2:
    case 3:
    case 4:
        break;
    case 5:
        if (data->child != NULL) {
            data->child->base.destroy(data->child);
        }
        data->child = fightstg_script_create();
        data->child->side = obj->side == 0;
        switch (obj->child) {
        case 0:
        default:
            script = obj->results[0];
            break;
        case 1:
            script = obj->results[1];
            break;
        case 2:
            script = obj->results[2];
            break;
        }
        data->child->script = script + 1;
        obj->child++;
        break;
    }
}

/* Command 2 (op, role, [arg], ...): acts on the model of role `role` (its 0x14 object slot). The move cases read
 * their frame count into a block-local first (case 3 before the position: sched1 boosts the once-assigned load). */
s32 fightstg_script_run_model(FightstgScript *obj, FightstgScriptData *data) {
    s32 arg = 0;
    FightstgModelParams *params = NULL;
    s32 op = *obj->next++;
    s32 slot = fightstg_script_get_slot(obj, *obj->next++);
    FightstgSlots *slots;
    SVECTOR pos;
    s32 arg2;

    switch (op) {
    case 1 ... 4:
    case 7 ... 9:
        break;
    default:
        arg = *obj->next++;
        break;
    }
    if (op != 6 && op != 9) {
        params = obj->slots->get_params(obj->slots, slot);
    }
    switch (op) {
    case 0:
        switch (arg) {
        case 0:
            if (params->anim_done == 0) {
                obj->next -= 4;
                return 0;
            }
            return 1;
        case 1:
            params->anim_done = 0;
            params->anim = params->idle_anim + 1;
            break;
        default:
            params->anim = arg + 1;
            params->restart = 1;
            params->anim_done = 0;
            break;
        }
        break;
    case 5:
        switch (arg) {
        case 0:
            if (data->jump == NULL) {
                return 1;
            }
            if (data->jump->state < OBJECT_STATE_DONE) {
                obj->next -= 4;
                return 0;
            }
            return 1;
        case 1:
        case 2:
        case 3:
        case 5:
        case 6:
            if (data->jump != NULL) {
                data->jump->destroy(data->jump);
            }
            data->jump = fightstg_jump_create(params, arg, 0);
            break;
        case 4:
            arg2 = *obj->next++;
            if (data->jump != NULL) {
                data->jump->destroy(data->jump);
            }
            data->jump = fightstg_jump_create(params, arg, arg2);
            break;
        }
        break;
    case 1:
        params->layers[0].shown = 1;
        return 1;
    case 2:
        params->layers[0].shown = 0;
        break;
    case 3: {
        s32 frames = obj->next[3];

        pos.vx = obj->next[0];
        pos.vy = -obj->next[1];
        pos.vz = -obj->next[2];
        obj->next += 4;
        if (frames != 0) {
            data->move = fightstg_move_create(params, &pos, frames);
        } else {
            params->pos.x = pos.vx;
            params->pos.y = pos.vy;
            params->pos.z = pos.vz;
        }
        break;
    }
    case 4:
        params->rot.x = *obj->next++;
        params->rot.y = -*obj->next++;
        params->rot.z = -*obj->next++;
        obj->next++;
        break;
    case 6:
        if (obj->script == 12 &&
            cdload_module.is_loading((s16)(((FightstgModelRecordA *)fightstg_models.get(arg))->model_file >> 16)) != 0) {
            obj->next -= 4;
            return 0;
        }
        slots = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
        slots->add(slots, slot, arg, 0);
        return 1;
    case 7: {
        s32 frames = *obj->next++;

        if (frames != 0) {
            data->move = fightstg_move_create(params, &params->start_pos, frames);
        } else {
            params->pos.x = params->start_pos.x;
            params->pos.y = params->start_pos.y;
            params->pos.z = params->start_pos.z;
        }
        break;
    }
    case 8: {
        s32 frames = *obj->next++;

        pos.vx = params->start_pos.x;
        pos.vy = params->start_pos.y;
        if (slot & 0xF0) {
            pos.vz = params->start_pos.z - 0x2800;
        } else {
            pos.vz = params->start_pos.z + 0x2800;
        }
        if (frames != 0) {
            data->move = fightstg_move_create(params, &pos, frames);
        } else {
            params->pos.x = pos.vx;
            params->pos.y = pos.vy;
            params->pos.z = pos.vz;
        }
        break;
    }
    case 9:
        slots = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
        slots->remove(slots, slot);
        break;
    }
    return 1;
}

/* Command 3 (op, id, ...; id 9999 = the script's stage): 0 = start effect `id` at the position that
 * follows; 1 = load effect `id`'s image (into VRAM) and model, waiting for each. */
s32 fightstg_script_run_effect(FightstgScript *obj, FightstgScriptData *data) {
    s32 op = *obj->next++;
    s32 id = *obj->next++;
    SVECTOR pos;
    Tim tim;
    s32 i;

    if (id == 9999) {
        id = obj->effect;
    }
    switch (op) {
    case 0:
    default:
        pos.vx = *obj->next++;
        pos.vy = *obj->next++;
        pos.vz = *obj->next++;
        for (i = 0; i < 8; i++) {
            if (data->effects[i] == NULL) {
                data->effects[i] = (Object *)fightstg_effect_create(id, &pos);
                break;
            }
        }
        break;
    case 1:
        switch (obj->load_state) {
        case 0:
        default:
            if (fightstg_effect_get_files(id, &obj->image_file, &obj->model_file, &obj->image_pos) == 0) {
                break;
            }
            obj->load_state++;
            /* fallthrough */
        case 1:
            if (obj->image_file == 0 || cdload_module.is_loading(obj->image_file >> 16) == 0) {
                obj->load_state++;
            }
            obj->next -= 3;
            return 0;
        case 2:
            if (obj->image_file != 0) {
                tim_init(&tim);
                tim.set_image_pos(obj->image_pos.x, obj->image_pos.y);
                tim.load_all(cdload_module.get_subfile_by_id(obj->image_file));
            }
            obj->load_state++;
            /* fallthrough */
        case 3:
            if (obj->model_file != 0 && cdload_module.is_loading(obj->model_file >> 16) != 0) {
                obj->next -= 3;
            } else {
                obj->load_state = 0;
            }
            return 0;
        }
        break;
    }
    return 1;
}

/* Command 6 (op, id, ...): 0 = show model `id` at the position and rotation that follow (y and z
 * negated); 1 = wait until model `id`'s file is loaded. */
s32 fightstg_script_run_prop(FightstgScript *obj, FightstgScriptData *data) {
    s32 op = *obj->next++;
    s32 id = *obj->next++;
    SVECTOR pos;
    SVECTOR rot;
    s32 i;

    switch (op) {
    case 0:
    default:
        pos.vx = *obj->next++;
        pos.vy = -*obj->next++;
        pos.vz = -*obj->next++;
        rot.vx = *obj->next++;
        rot.vy = -*obj->next++;
        rot.vz = -*obj->next++;
        for (i = 0; i < 3; i++) {
            if (data->props[i] == NULL) {
                data->props[i] = fightstg_prop_create(id, &pos, &rot);
                break;
            }
        }
        break;
    case 1:
        id = fightstg_prop_get_file(id);
        if (id != 0 && cdload_module.is_loading(id) != 0) {
            obj->next -= 3;
            return 0;
        }
        break;
    }
    return 1;
}

/* Command 5 (frames, op, ...): sets the camera (0x12 object), over `frames` frames: op 0 = preset
 * (0: the current one, 1-4: 8-11, 5-7: 0x10-0x12), op 1 = the setting that follows (y and z negated). */
void fightstg_script_run_camera(FightstgScript *obj, FightstgScriptData *data) {
    FightstgCamera *camera = (FightstgCamera *)heap_objects.find(0x12, -1, -1);
    s32 frames = *obj->next++;
    s16 *args = obj->next;
    s32 op = *obj->next++;
    FightstgCameraSetting *setting = &fightstg_camera_setting;
    s32 preset;

    switch (op) {
    case 0:
    default:
        preset = *obj->next++;
        switch (preset) {
        case 0:
        default:
            camera->get_default(camera);
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            camera->get_preset(camera, 0, preset + 7);
            break;
        case 5:
        case 6:
        case 7:
            camera->get_preset(camera, 0x10, preset - 5);
            break;
        }
        break;
    case 1:
        setting->eye[0] = args[1];
        setting->eye[1] = -args[2];
        setting->eye[2] = -args[3];
        setting->target[0] = args[4];
        setting->target[1] = -args[5];
        setting->target[2] = -args[6];
        setting->trans[0] = args[7];
        setting->trans[1] = -args[8];
        setting->trans[2] = -args[9];
        setting->rot[0] = args[10];
        setting->rot[1] = -args[11];
        setting->rot[2] = -args[12];
        setting->roll = args[13];
        setting->projection = args[14];
        obj->next = args + 15;
        break;
    }
    if (frames != 0) {
        camera->move(camera, 0, setting, frames);
    } else {
        camera->set(camera, setting);
    }
}

/* Command 11: wait the frames given by the argument. Returns 1 when done. */
s32 fightstg_script_run_wait(FightstgScript *obj, FightstgScriptData *data) {
    switch (obj->waiting) {
    case 0:
    default:
        obj->wait_frames = *obj->next;
        obj->waiting = 1;
        obj->next--;
        return 0;
    case 1:
        obj->wait_frames -= gfx_module.funcs.get_frame_ticks();
        obj->next--;
        if (obj->wait_frames > 0) {
            return 0;
        }
        obj->waiting = 0;
        obj->wait_frames = 0;
        obj->next += 2;
        return 1;
    }
}

/* Command 4 (op, stage, ...): 0 = switch to stage `stage` (0x38: obj->stage, 0: records_state's random one)
 * with the fade times that follow; 1 = wait until the stage's file is loaded. */
s32 fightstg_script_run_stage(FightstgScript *obj, FightstgScriptData *data) {
    FightstgStage *stage = (FightstgStage *)heap_objects.find(0x15, -1, -1);
    s32 op = *obj->next++;
    s32 id = *obj->next++;
    s32 fade_in;
    s32 fade_out;
    s32 file;
    FightstgModelParams *params;

    if (id == 0x38) {
        id = obj->stage;
    }
    if (id == -1) {
        return 1;
    }
    switch (op) {
    case 0:
    default:
        fade_in = *obj->next++;
        fade_out = *obj->next++;
        if (id == 0) {
            id = records_state.stage;
        }
        stage->change(stage, id, fade_in, fade_out);
        switch (id) {
        case 0x1D:
            params = obj->slots->get_params(obj->slots, 0);
            fightstg_script_saved_idle_anim = params->idle_anim;
            params->idle_anim = 0;
            break;
        case 0x1E:
            if (fightstg_script_saved_idle_anim != 0) {
                obj->slots->get_params(obj->slots, 0)->anim = 2;
            }
            break;
        }
        break;
    case 1:
        if (id == 0) {
            break;
        }
        file = fightstg_stage_get_file(id);
        if (file != 0 && cdload_module.is_loading(file) != 0) {
            obj->next -= 3;
            return 0;
        }
        break;
    }
    return 1;
}

/* Command 10 (id, arg): starts effect `id` (0x62: per results[sound_index], 0x63: per results[3]) unless one runs. */
void fightstg_script_run_sound(FightstgScript *obj, FightstgScriptData *data) {
    s32 id = *obj->next++;
    s32 arg = *obj->next++;

    if (id == 0x62) {
        id = obj->results[obj->sound_index] == 3 ? 0x38 : obj->hit_sound;
        obj->sound_index++;
    } else if (id == 0x63) {
        id = obj->results[3] == 3 ? 0x38 : obj->hit_sound;
        obj->sound_index++;
    }
    if (data->sound == NULL) {
        data->sound = fightstg_sound_play(id, arg);
    }
}

void fightstg_script_run_flash(FightstgScript *obj, FightstgScriptData *data) {
    s32 op = *obj->next++;
    s32 arg = *obj->next++;

    switch ((s16)op) { /* the original compares the halfword: the cast is what matches */
    case 0:
        data->flash = fightstg_flash_create(arg);
        break;
    case 1:
        if (data->flash != NULL) {
            fightstg_flash_end(data->flash, arg);
        }
        break;
    }
}

void fightstg_script_update(FightstgScript *obj, FightstgScriptData *data) {
    FightstgSlots *slots;
    s32 more;
    s32 busy;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            obj->slot = (obj->side != 0) << 4;
            slots = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
            obj->slots = slots;
            obj->model_id = slots->get_params(slots, obj->slot)->model_id;
            fightstg_models.get(obj->model_id);
            obj->file = fightstg_models.record->script_file;
            obj->next = cdload_module.get_subfile(obj->script, cdload_module.get_subfile_by_id(obj->file));
#ifdef PC_PORT
            if (port_mod_battle_animations != 0 && obj->script >= 5) {
                s32 sound_id;
                s32 sound_arg;
                s32 cut = port_battle_cut(obj->next, obj->stage, obj->results, obj->hit_sound, &sound_id, &sound_arg);

                if (cut >= 0) {
                    /* battle_animations: the animation cut before it starts (and before script 12's bank load) */
                    if (sound_id != 0 && data->sound == NULL) {
                        data->sound = fightstg_sound_play(sound_id, sound_arg);
                    }
                    if (cut == 0) {
                        obj->base.set_state(obj, OBJECT_STATE_END);
                    } else {
                        /* the target's reaction, as the child command would have started it (a new object: no
                         * results, stage, effect or hit sound); step 0 runs again for it */
                        obj->side = obj->side == 0;
                        obj->script = cut;
                        for (i = 0; i < 4; i++) {
                            obj->results[i] = 0;
                        }
                        obj->stage = 0;
                        obj->effect = 0;
                        obj->hit_sound = 0;
                    }
                    break;
                }
            }
#endif
            if (obj->script != 12) {
                obj->base.next_state(obj);
                break;
            }
            obj->base.next_step(obj);
            /* fallthrough */
        case 1:
            switch (obj->base.substep) {
            case 0:
            default:
                sound_module.load_extra_bank(0x46);
                obj->base.next_substep(obj);
                /* fallthrough */
            case 1:
                if (sound_module.is_loading() == 0) {
                    obj->base.next_state(obj);
                }
                break;
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        do {
            more = 1;
            switch (*obj->next++) {
            case 1:
                fightstg_script_run_child(obj, data);
                break;
            case 2:
                more = fightstg_script_run_model(obj, data);
                break;
            case 3:
                more = fightstg_script_run_effect(obj, data);
                break;
            case 4:
                more = fightstg_script_run_stage(obj, data);
                break;
            case 5:
                fightstg_script_run_camera(obj, data);
                break;
            case 6:
                more = fightstg_script_run_prop(obj, data);
                break;
            case 7:
                fightstg_script_run_flash(obj, data);
                break;
            case 8: /* commands 8 and 9 do nothing (their cases shape the compare tree) */
            case 9:
                break;
            case 10:
                fightstg_script_run_sound(obj, data);
                break;
            case 11:
                more = fightstg_script_run_wait(obj, data);
                break;
            case 0:
            case 0xFF:
                busy = 0;
                if (data->jump != NULL) {
                    busy = data->jump->state < OBJECT_STATE_DONE;
                }
                if (data->child != NULL) {
                    busy = 1;
                }
                for (i = 0; i < 8; i++) {
                    if (data->effects[i] != NULL) {
                        busy = 1;
                        break;
                    }
                }
                if (busy) {
                    obj->next--;
                } else {
                    obj->base.set_state(obj, OBJECT_STATE_END);
                }
                more = 0;
                break;
            }
        } while (more);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

FightstgScript *fightstg_script_create(void) {
    return object_new(fightstg_script_update, sizeof(FightstgScript), sizeof(FightstgScriptData));
}

/* Turns the camera round the scene for 240 frames while script 3 runs on stage 0x17. */
void fightstg_defeat_camera_update(FightstgDefeatCamera *obj, FightstgScript **data) {
    FightstgCamera *camera = obj->camera;
    FightstgStage *stage;
    FightstgSlots *slots;
    s32 z;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        camera = (FightstgCamera *)heap_objects.find(0x12, -1, -1);
        obj->camera = camera;
        camera->get_preset(camera, 0, 11);
        z = fightstg_camera_setting.target[2];
        fightstg_camera_setting.target[2] = 0;
        fightstg_camera_setting.eye[2] -= z;
        fightstg_camera_setting.trans[2] = z;
        stage = (FightstgStage *)heap_objects.find(0x15, -1, -1);
        stage->change(stage, 0x17, 30, 30);
        slots = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
        slots->get_params(slots, 0x10)->layers[0].shown = 0;
        fightstg_battle.set_speed(2);
        *data = fightstg_script_create();
        (*data)->script = 3;
        (*data)->side = 0;
        obj->base.next_state(obj);
        /* fallthrough */
    case OBJECT_STATE_RUN:
        obj->base.step += gfx_module.funcs.get_frame_ticks();
        if (obj->base.step >= 240) {
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        /* fallthrough */
    case OBJECT_STATE_DONE:
        fightstg_camera_setting.rot[1] += gfx_module.funcs.get_frame_ticks() * 2;
        camera->set(camera, &fightstg_camera_setting);
        break;
    case OBJECT_STATE_END:
        fightstg_battle.set_speed(0);
        break;
    }
}

OBJECT_V0(FightstgDefeatCamera *) fightstg_defeat_camera_create(void) {
    /* PC_PORT: FINDINGS 8: callers use the object (v0) */
    OBJECT_V0_TAIL(object_new(fightstg_defeat_camera_update, sizeof(FightstgDefeatCamera), sizeof(FightstgScript *)))
}

/* Side `side`'s attack: the message (8 or 0x8D), the effect (WFIGHTMN), then per fightstg_action's
 * result the damage message and HP loss, a knock-out (fightstg_events_add_knockout) and the follow-ups
 * (steps 2-8). */
void fightstg_attack_update(FightstgAttack *obj, FightstgMessage **data) {
    FightstgMember *enemy;
    FightstgMember *victim;
    FightstgMember *player;
    s32 other;
    FightstgMember *target;
    FightstgMember *members;
    RecordsTechnique *rec;
    FightstgMember *member;
    s32 side;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->side == 0) {
            obj->tech = records_get_digimon_func(fightstg_battle.state.members[0][fightstg_battle.state.current[0]].digimon)->techniques[0];
        } else {
            obj->tech = fightstg_enemy_records.get(fightstg_battle.state.members[1][fightstg_battle.state.current[1]].digimon)->tech;
        }
        enemy = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
        rec = &records_techniques[obj->tech - 1];
        if (obj->side != 0 && rec->mp_cost > enemy->mp) {
            *data = fightstg_message_create();
            obj->args[0] = 0x8D;
            obj->args[1] = obj->side;
            (*data)->show(*data, 2, obj->args);
            obj->base.step = 2;
        } else {
            *data = fightstg_message_create();
            obj->args[0] = 8;
            obj->args[1] = obj->side;
            (*data)->show(*data, 2, obj->args);
            fightstg_action.run(obj->side, obj->tech);
        }
        enemy = &fightstg_battle.state.members[1 - (obj->side >> 4)][fightstg_battle.state.current[1 - (obj->side >> 4)]];
        if (enemy->status & 8) {
            obj->target_asleep = 1;
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            if (*data != NULL) {
                break;
            }
            *data = wfightmn_tech_script_create(obj->side, obj->tech);
            members = obj->side != 0 ? fightstg_battle.state.members[1] : fightstg_battle.state.members[0];
            members[fightstg_battle.state.current[obj->side != 0]].power_up = 0;
            obj->base.step++;
            break;
        case 1:
            if (*data != NULL) {
                break;
            }
            *data = fightstg_message_create();
            if (fightstg_action.effects[9]) {
                obj->args[0] = 0x10 - obj->side;
                obj->args[1] = fightstg_action.damage;
                obj->args[2] = (s16)fightstg_action.hit_count;
                (*data)->show(*data, 0x10, obj->args);
                obj->damage = (s16)fightstg_action.hit_count * fightstg_action.damage;
                if (obj->damage >= 10000) {
                    obj->damage = 9999;
                }
            } else if (fightstg_action.effects[6]) {
                other = obj->side == 0;
                target = &fightstg_battle.state.members[other][fightstg_battle.state.current[other]];
                target->hp = 0;
                fightstg_events_add_knockout(other << 4);
                (*data)->base.state = OBJECT_STATE_END;
            } else if (fightstg_action.effects[0x23]) {
                player = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                obj->damage = player->hp * 7 / 10;
                obj->args[0] = 0;
                obj->args[1] = obj->damage;
                (*data)->show(*data, 4, obj->args);
            } else if (fightstg_action.hits[0]) {
                obj->args[0] = (obj->side == 0) << 4;
                obj->args[1] = fightstg_action.damage;
                (*data)->show(*data, 4, obj->args);
                obj->damage = fightstg_action.damage;
            } else {
                obj->args[0] = 0x1D;
                obj->args[1] = (obj->side == 0) << 4;
                (*data)->show(*data, 2, obj->args);
            }
            if (obj->damage != 0) {
                other = obj->side == 0;
                victim = &fightstg_battle.state.members[other][fightstg_battle.state.current[other]];
                victim->hp -= obj->damage;
                if (victim->hp <= 0) {
                    victim->hp = 0;
                    fightstg_events_add_knockout((obj->side == 0) << 4);
                    obj->base.substep = 1;
                }
            }
            obj->base.step++;
            break;
        case 2:
            if (*data != NULL) {
                break;
            }
            if (fightstg_action.effects[0x23]) {
                *data = (FightstgMessage *)fightstg_results_create(obj->side);
                obj->base.next_step(obj);
                break;
            }
            if (obj->base.substep == 0) {
                if (obj->damage == 0) {
                    if (obj->side == 0 && fightstg_battle.state.type == 6) {
                        obj->base.set_step(obj, 6);
                        break;
                    }
                } else {
                    *data = (FightstgMessage *)fightstg_results_create(obj->side);
                    obj->base.next_step(obj);
                    break;
                }
            }
            obj->base.state = OBJECT_STATE_END;
            break;
        case 3:
            if (*data == NULL) {
                obj->base.next_step(obj);
            }
            break;
        case 4:
            side = obj->side == 0;
            member = &fightstg_battle.state.members[side][fightstg_battle.state.current[side]];
            if (member->status & 8) {
                if (obj->target_asleep == 0 || fightstg_rules.roll_wake(0x10 - obj->side, obj->damage) == 0) {
                    obj->base.state = OBJECT_STATE_END;
                    break;
                }
                obj->args[0] = 0x2B;
                obj->args[1] = 0x10 - obj->side;
                obj->args[2] = fightstg_battle.state.current[side];
                *data = fightstg_message_create();
                (*data)->show(*data, 7, obj->args);
                member->status &= ~8;
                i = fightstg_events.find_member(0xC, 0x10 - obj->side, fightstg_battle.state.current[side]);
                if (i >= 0) {
                    fightstg_events.events[i].type = 0;
                }
                obj->base.step = 8;
            } else {
                *data = fightstg_counter_create(side << 4, obj->damage, 0);
                obj->base.step++;
            }
            break;
        case 5:
            if (*data != NULL) {
                if ((*data)->base.state != OBJECT_STATE_DONE) {
                    break;
                }
                wfightmn_add_gauge(obj->side, obj->damage);
            }
            obj->base.state = OBJECT_STATE_END;
            break;
        case 6:
            if (*data != NULL) {
                break;
            }
            *data = fightstg_boss_turn_create(1, 0);
            obj->base.next_step(obj);
            break;
        case 7:
            if (*data != NULL) {
                break;
            }
            obj->base.set_state(obj, OBJECT_STATE_END);
            break;
        case 8:
            if (*data != NULL) {
                break;
            }
            *data = fightstg_counter_create((obj->side == 0) << 4, obj->damage, 0);
            obj->base.step = 5;
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

OBJECT_V0(FightstgAttack *) fightstg_attack_create(s32 side) {
    FightstgAttack *obj = object_new(fightstg_attack_update, sizeof(FightstgAttack), sizeof(FightstgMessage *));

    obj->side = side;
    OBJECT_V0_RETURN(obj) /* PC_PORT: FINDINGS 8: callers use the object (v0) */
}

/* .bss */
s32 fightstg_script_saved_idle_anim;
s32 fightstg_unused_1; /* unreferenced */
