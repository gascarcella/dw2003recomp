#include "common.h"

#include "cdload.h"
#include "heap.h"
#include "gamestate.h"
#include "records.h"
#include "object.h"
#include "gfx.h"
#include "sound.h"
#include "pad.h"
#include "overlay.h"
#include "overlay_common.h"
#include "fightstg.h"

extern FightstgPos fightstg_slots_vram[4];

/* fightstg_entrance_create's object: a Digimon's entrance (record base.key1 of file 0x1CC on side
 * base.key2): loads its model, shows it with the camera, a fade and, for 0x1D2/0x1D3, a stage or a
 * sound. */
typedef struct FightstgEntrance {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 done;
    /* 0x54 */ FightstgSlots *slots; /* the 0x14 object */
    /* 0x58 */ FightstgCamera *camera;
    /* 0x5C */ FightstgStage *stage;
    /* 0x60 */ s32 file; /* the model's file */
    /* 0x64 */ s32 weak;
} FightstgEntrance; /* size 0x68 */

/* Data block of fightstg_entrance_create's object. */
typedef struct FightstgEntranceData {
    /* 0x0 */ struct FightstgFlash *flash; /* a fade */
    /* 0x4 */ Object *unk_4[2]; /* never written */
} FightstgEntranceData; /* size 0xC */

/* A step of a camera sequence (fightstg_idle_camera_steps): frames, then the camera mode (base.step). */
typedef struct FightstgCameraStep {
    /* 0x0 */ s16 frames; /* frames; -1 ends the sequence */
    /* 0x2 */ s16 mode;
} FightstgCameraStep; /* size 0x4 */

extern FightstgCameraStep fightstg_idle_camera_steps[][6];
extern u8 fightstg_idle_camera_next[][3]; /* per sequence: the three that may follow */

/* fightstg_idle_camera_create's object: the idle camera. */
typedef struct FightstgIdleCamera {
    /* 0x00 */ Object base;
    /* 0x50 */ FightstgCamera *camera; /* the camera */
    /* 0x54 */ FightstgCameraSetting *setting; /* its setting */
    /* 0x58 */ FightstgSlots *slots; /* the 0x14 object */
    /* 0x5C */ s16 sequence;
    /* 0x5E */ s16 step;
    /* 0x60 */ s16 angle; /* the camera's angle */
    /* 0x62 */ s16 turned; /* angle turned */
    /* 0x64 */ s32 frames; /* frames left in the step */
    /* 0x68 */ s32 speed; /* turning speed */
} FightstgIdleCamera; /* size 0x6C */

/* fightstg_sprite_anim_create's object: plays a sprite animation (a stream of s16: flags, values per frame,
 * frames, the 9 initial values, then per frame the values whose flag bit is set). */
typedef struct FightstgSpriteAnim {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 *stream; /* the stream */
    /* 0x54 */ SVECTOR pos;
    /* 0x5C */ s32 file;
    /* 0x60 */ FightstgPos vram;
    /* 0x68 */ s32 layer_id;
    /* 0x6C */ s32 frame;
    /* 0x70 */ s32 flags; /* which of values each frame sets */
    /* 0x74 */ s32 values_per_frame;
    /* 0x78 */ s32 frames;
    /* 0x7C */ s16 values[9];
} FightstgSpriteAnim; /* size 0x90 */

/* fightstg_effect_files: models of the animations (fightstg_effects). */
typedef struct FightstgEffectFiles {
    /* 0x0 */ s32 image_file;
    /* 0x4 */ s32 file; /* sub-file ID */
    /* 0x8 */ FightstgPos vram;
} FightstgEffectFiles; /* size 0x10 */

/* fightstg_effects: effect ID -> its animations (sub-file `file`, a list of animation sub-files) and
 * entry of fightstg_effect_files, -1-terminated. */
typedef struct FightstgEffectEntry {
    /* 0x0 */ s16 id;
    /* 0x2 */ s16 files; /* entry of fightstg_effect_files */
    /* 0x4 */ s32 file; /* sub-file ID */
} FightstgEffectEntry; /* size 0x8 */

/* fightstg_props: model ID -> the model's sub-file IDs, 0-terminated. */
typedef struct FightstgPropEntry {
    /* 0x0 */ s32 id;
    /* 0x4 */ s32 model_file; /* the model's sub-file ID */
    /* 0x8 */ s32 anim_file;
} FightstgPropEntry; /* size 0xC */

/* fightstg_prop_create's object: a model (fightstg_props) at a position. */
typedef struct FightstgProp {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 id; /* ID, 0 if unknown */
    /* 0x54 */ s32 model_file;
    /* 0x58 */ s32 anim_file;
    /* 0x5C */ SVECTOR pos;
    /* 0x64 */ SVECTOR rot;
    /* 0x6C */ FightstgPos vram;
    /* 0x74 */ FightstgModelParams params;
} FightstgProp; /* size 0xC0 */

/* fightstg_effect_create's object: an effect (fightstg_effects), one fightstg_sprite_anim_create object
 * per animation (in base.children). */
typedef struct FightstgEffect {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 id; /* ID, -1 if unknown */
    /* 0x54 */ s32 files; /* entry of fightstg_effect_files */
    /* 0x58 */ s32 file; /* sub-file ID */
    /* 0x5C */ SVECTOR pos;
    /* 0x64 */ s32 anim_count; /* animations */
} FightstgEffect; /* size 0x68 */

/* fightstg_digivolve_create's object: a model's scene. */
typedef struct FightstgDigivolve {
    /* 0x00 */ Object base;
    /* 0x50 */ FightstgSlots *slots; /* the 0x14 object */
    /* 0x54 */ FightstgCamera *camera;
    /* 0x58 */ FightstgStage *stage;
    /* 0x5C */ s32 idle_anim; /* slot 0's idle_anim */
    /* 0x60 */ RECT top_bar; /* the bar on layer 0x1004 */
    /* 0x68 */ RECT bottom_bar; /* the bar on layer 0x1003 */
    /* 0x70 */ s32 file; /* the model's file */
} FightstgDigivolve; /* size 0x74 */

/* Data block of fightstg_digivolve_create's object. */
typedef struct FightstgDigivolveData {
    /* 0x00 */ Object *flash; /* the fade (fightstg_flash_create) */
    /* 0x04 */ s32 unk_4;
    /* 0x08 */ FightstgEffect *effects[4]; /* effects (fightstg_effect_create): [0] and (key2 0) [1] at step 2, [2]
                                            * at the end of step 3, [3] at the end of step 8 */
} FightstgDigivolveData; /* size 0x18 */

extern FightstgEffectFiles fightstg_effect_files[];
extern FightstgEffectEntry fightstg_effects[];
extern FightstgPropEntry fightstg_props[];

/* fightstg_slots_create's object's data block (base.children). */
typedef struct FightstgSlotsData {
    /* 0x00 */ FightstgModel *models[4]; /* model of each slot */
    /* 0x10 */ FightstgModel *ending[4]; /* model being ended */
} FightstgSlotsData; /* size 0x20 */

/* fightstg_player_reaction_create's object: effect 0x33 over the screen, then side 0's reaction script
 * (reaction + 1: 2 hit, 3 knocked out; FIGHTSTG's poison damage and a WFIGHTTS lesson). */
typedef struct FightstgPlayerReaction {
    /* 0x00 */ Object base;
    /* 0x50 */ FightstgSlots *slots;  /* the 0x14 object */
    /* 0x54 */ FightstgCamera *camera;  /* the 0x12 object */
    /* 0x58 */ s32 reaction;
    /* 0x5C */ s32 unk_5C; /* create's arg1 (1 from WFIGHTMN's poison damage, 0 from WFIGHTTS): the script's unk_74 */
    /* 0x60 */ s32 image_file;            /* animation 0x33's model (fightstg_effect_get_files) */
    /* 0x64 */ s32 file;
    /* 0x68 */ FightstgPos vram;
} FightstgPlayerReaction; /* size 0x70 */

/* fightstg_enemy_turn_create's object: the enemy's turn. */
typedef struct FightstgEnemyTurn {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 args[3]; /* the message's arguments (fightstg_message_show's args) */
    /* 0x5C */ u8 unk_5C[0x1C];
    /* 0x78 */ s32 action; /* chosen action (fightstg_enemy_get_action) */
    /* 0x7C */ s32 called; /* member called in */
} FightstgEnemyTurn; /* size 0x80 */

/* fightstg_message_create's object (fightstg_8008D3B4), the part used here. */
typedef struct FightstgMessage {
    /* 0x00 */ Object base;
    /* 0x50 */ u8 unk_50[0x5C];
    /* 0xAC */ void (*show)(struct FightstgMessage *, s32, s32 *); /* fightstg_message_show */
    /* 0xB0 */ void (*close)();
} FightstgMessage; /* size 0xB4 */

/* Data block of fightstg_enemy_turn_create's object. */
typedef struct FightstgEnemyTurnData {
    /* 0x0 */ FightstgMessage *child; /* the object waited for (a message, an attack, an entrance) */
    /* 0x4 */ u8 unk_4[0x4];
} FightstgEnemyTurnData; /* size 0x8 */

/* In fightstg_8008D3B4 (defined there as void, or with other parameters; their objects stay in v0; OBJECT_V0). */
FightstgMessage *fightstg_message_create(void);
FightstgMessage *fightstg_attack_create();
FightstgMessage *fightstg_tech_create(s32 side, s32 action);
void fightstg_events_add_escape(u8 side);
void fightstg_events_add_enemy_turn(s32 delay);

/* Data block of fightstg_player_reaction_create's object. */
typedef struct FightstgPlayerReactionData {
    /* 0x0 */ Object *effect; /* effect 0x33 (fightstg_effect_create) */
    /* 0x4 */ FightstgScript *script;
} FightstgPlayerReactionData; /* size 0x8 */

extern const SVECTOR fightstg_player_reaction_pos; /* {0, 120, 0x7FFF}: a screen position */

void fightstg_entrance_update(FightstgEntrance *obj, FightstgEntranceData *data);
void fightstg_main_update(Object *obj, Object **data);
void fightstg_idle_camera_update(FightstgIdleCamera *obj);
void fightstg_slots_update();
s32 fightstg_slots_find(FightstgSlots *obj, s32 id);
void fightstg_slots_remove(FightstgSlots *obj, s32 id);
void fightstg_slots_add(FightstgSlots *obj, s32 id, s32 model_id, s32 shown);
FightstgModelParams *fightstg_slots_get_params(FightstgSlots *obj, s32 id);
void fightstg_slots_set_id(FightstgSlots *obj, s32 id, s32 new_id);
s32 fightstg_slots_get_model_id(FightstgSlots *obj, s32 id);
void fightstg_slots_reset_pos(FightstgSlots *obj, s32 id);
void fightstg_slots_set_idle_anim(FightstgSlots *obj, s32 id, s32 idle_anim);
void fightstg_player_reaction_update(FightstgPlayerReaction *obj, FightstgPlayerReactionData *data);
void fightstg_enemy_turn_update(FightstgEnemyTurn *obj, FightstgEnemyTurnData *data);
s32 fightstg_enemy_get_action(u8 type);
void fightstg_digivolve_update(FightstgDigivolve *obj, FightstgDigivolveData *data);
void fightstg_sprite_anim_draw(void *data, GfxLayer *layer, s32 arg);
void fightstg_sprite_anim_update(FightstgSpriteAnim *obj);
FightstgSpriteAnim *fightstg_sprite_anim_create(s16 *data, SVECTOR *screen_pos, s32 file, FightstgPos *pos, s32 layer);
void fightstg_prop_update(FightstgProp *obj, FightstgModel **data);
void fightstg_effect_update(FightstgEffect *obj, FightstgSpriteAnim **anims);
/* FIGHTSTG's copy of the fade (overlay_common.h's Fade; fightstg_fade_create creates it). */
void fightstg_fade_start(Fade *obj, s32 down, s32 frames);
void fightstg_fade_draw(Fade *obj);
void fightstg_fade_update(Fade *obj);
void fightstg_lights_update(FightstgLighting *obj);
void fightstg_lights_set(FightstgLighting *obj, FightstgLights *lights);
void fightstg_lights_fade(FightstgLighting *obj, FightstgLights *from, FightstgLights *to, s32 frames);
FightstgLights *fightstg_lights_get_stage(FightstgLighting *obj, s32 index);

void fightstg_entrance_update(FightstgEntrance *obj, FightstgEntranceData *data) {
    FightstgSlots *slots = obj->slots;
    FightstgCamera *camera = obj->camera;
    FightstgStage *stage = obj->stage;
    s32 is_1D2 = obj->base.key1 == 0x1D2;
    s32 is_1D3 = obj->base.key1 == 0x1D3;
    FightstgModelParams *params;
    FightstgModelParams *own;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            obj->slots = slots = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
            obj->camera = (FightstgCamera *)heap_objects.find(0x12, -1, -1);
            obj->stage = (FightstgStage *)heap_objects.find(0x15, -1, -1);
            obj->file = (s16)(((FightstgModelRecordA *)fightstg_models.get(obj->base.key1))->model_file >> 16);
            cdload_module.queue_file(obj->file);
            obj->base.next_step(obj);
            /* fallthrough */
        case 1:
            if (cdload_module.is_loading(obj->file) != 0) {
                break;
            }
            slots->add(slots, obj->base.key2 + 1, obj->base.key1, 0);
            if (is_1D2 == 0 && is_1D3 == 0) {
                obj->base.next_state(obj);
                break;
            }
            obj->base.next_step(obj);
            /* fallthrough */
        case 2:
            switch (obj->base.substep) {
            case 0:
            default:
                if (is_1D2) {
                    cdload_module.queue_file(0x6E2);
                } else {
                    sound_module.fade_out(0x60900000);
                    sound_module.load_extra_bank(0x26);
                }
                obj->base.next_substep(obj);
                break;
            case 1:
                if (is_1D2) {
                    if (cdload_module.is_loading(0x6E2) == 0) {
                        obj->base.next_state(obj);
                    }
                } else if (sound_module.is_loading() == 0) {
                    obj->base.next_state(obj);
                }
                break;
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            camera->move(camera, 0, camera->get_preset(camera, obj->base.key2, obj->base.key2 != 0 ? 2 : 10), 60);
            obj->base.next_step(obj);
            /* fallthrough */
        case 1:
            obj->base.timer += gfx_module.funcs.get_frame_ticks();
            switch (obj->base.substep) {
            case 0:
            default:
                if (obj->base.timer < 30) {
                    break;
                }
                slots->get_params(slots, (obj->base.key2 == 0) << 4)->layers[0].shown = 0;
                obj->base.substep++;
                /* fallthrough */
            case 1:
                if (obj->base.timer < 60) {
                    break;
                }
                obj->base.next_step(obj);
                break;
            }
            break;
        case 2:
            sound_module.play(0xA0045EC9);
            data->flash = fightstg_flash_create(60);
            obj->base.next_step(obj);
            /* fallthrough */
        case 3:
            if (((Object *)data->flash)->step == 0) {
                break;
            }
            obj->base.next_step(obj);
            obj->done = 1;
            slots->set_id(slots, obj->base.key2 + 1, obj->base.key2);
            slots->reset_pos(slots, obj->base.key2);
            params = slots->get_params(slots, obj->base.key2);
            params->layers[0].layer_id = 0x1004;
            params->layers[0].shown = 1;
            params->layers[0].edges = 0;
            params->anim = 13;
            camera->set(camera, camera->get_preset(camera, obj->base.key2, obj->base.key2 != 0 ? 2 : 10));
            if (is_1D2) {
                records_state.stage = 0x16;
                stage->change(stage, 0x16, 1, 1);
            }
            if (is_1D3) {
                records_state.music = 0x60980000;
                sound_module.play(0x60980000);
            }
            break;
        case 4:
            fightstg_flash_end(data->flash, 60);
            obj->base.next_step(obj);
            /* fallthrough */
        case 5:
            if (data->flash != NULL) {
                break;
            }
            obj->base.next_step(obj);
            /* fallthrough */
        case 6:
            obj->base.substep += gfx_module.funcs.get_frame_ticks();
            if (obj->base.substep < (is_1D2 ? 10 : 120)) {
                break;
            }
            obj->base.next_step(obj);
            /* fallthrough */
        case 7:
            camera->move(camera, 0, camera->get_default(camera), obj->base.key2 != 0 ? 1 : 60);
            own = slots->get_params(slots, obj->base.key2);
            if (obj->weak != 0) {
                own->anim = 2;
            } else {
                own->anim = 1;
            }
            slots->get_params(slots, (obj->base.key2 == 0) << 4)->layers[0].shown = 1;
            obj->base.set_state(obj, OBJECT_STATE_END);
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

FightstgEntrance *fightstg_entrance_create(s32 id, s32 enemy, s32 weak) {
    FightstgEntrance *obj = object_new(fightstg_entrance_update, sizeof(FightstgEntrance), sizeof(FightstgEntranceData));

    obj->base.key1 = id;
    if (enemy != 0) {
        obj->base.key2 = 0x10;
    } else {
        obj->base.key2 = 0;
    }
    obj->weak = weak;
    return obj;
}

/* The battle's main object (fightstg_entry): loads the battle overlay 0x209 or 0x208 and
 * creates its object (in base.children). */
void fightstg_main_update(Object *obj, Object **data) {
    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_data.funcs.get_map_entry() != 0) {
            overlay_module.load_file(0x209);
            records_state.stage = fightstg_stage_get_random();
            *data = wfightts_main_create();
        } else {
            overlay_module.load_file(0x208);
            *data = wfightmn_main_create();
        }
        fightstg_battle.set_speed(0);
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        fightstg_battle.update_speed();
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The overlay's entry point: creates the battle's main object, which overlay_run_object keeps (v0; PC_PORT:
 * FINDINGS 8). */
OBJECT_V0(Object *) fightstg_entry(void) {
    OBJECT_V0_TAIL(object_new(fightstg_main_update, sizeof(Object), sizeof(Object *)))
}

/* The idle camera: plays camera sequences (fightstg_idle_camera_steps: frames and a mode per step), each
 * followed by one of three others (fightstg_idle_camera_next), starting with sequence 3. Modes turn the
 * camera round the scene or show either side. */
void fightstg_idle_camera_update(FightstgIdleCamera *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->sequence = 3;
        obj->step = 0;
        obj->frames = fightstg_idle_camera_steps[obj->sequence][0].frames;
        obj->base.set_step(obj, fightstg_idle_camera_steps[obj->sequence][obj->step].mode);
        obj->camera = (FightstgCamera *)heap_objects.find(0x12, -1, -1);
        obj->slots = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
        obj->setting = obj->camera->get_default(obj->camera);
        obj->angle = obj->setting->rot[1];
        break;
    case OBJECT_STATE_RUN:
        if (obj->frames <= 0) {
            obj->slots->get_params(obj->slots, 0)->layers[0].shown = 1;
            obj->slots->get_params(obj->slots, 0x10)->layers[0].shown = 1;
            obj->step++;
            if (fightstg_idle_camera_steps[obj->sequence][obj->step].frames == -1) {
                obj->sequence = fightstg_idle_camera_next[obj->sequence][pad_random.next() % 3];
                obj->step = 0;
            }
            obj->frames = fightstg_idle_camera_steps[obj->sequence][obj->step].frames;
            obj->base.set_step(obj, fightstg_idle_camera_steps[obj->sequence][obj->step].mode);
        }
        switch (obj->base.step) {
        case 1:
        default:
            if (obj->base.substep == 0) {
                obj->setting = obj->camera->get_default(obj->camera);
                obj->turned = 0;
                obj->base.substep++;
                if ((records_state.stage & 0xF) != 4) {
                    obj->setting->rot[1] = obj->angle;
                }
            }
            obj->setting->rot[1] += gfx_module.funcs.get_frame_ticks() * 2;
            obj->turned += gfx_module.funcs.get_frame_ticks() * 2;
            if (obj->setting->rot[1] >= 0x1000) {
                obj->setting->rot[1] -= 0x1000;
            }
            obj->angle = obj->setting->rot[1];
            if ((records_state.stage & 0xF) == 4 && obj->turned > 0x800) {
                obj->frames = 0;
            }
            break;
        case 2:
            if (obj->base.substep == 0) {
                obj->setting = obj->camera->get_default(obj->camera);
                obj->setting->eye[0] = -0x1E80;
                obj->setting->eye[1] = -0x500;
                obj->setting->eye[2] = 0;
                obj->setting->target[0] = 0;
                obj->setting->target[1] = 0x500;
                obj->setting->target[2] = 0;
                obj->setting->projection = 0x98;
                obj->base.substep++;
            }
            break;
        case 3:
            if (obj->base.substep == 0) {
                obj->slots->get_params(obj->slots, 0x10)->layers[0].shown = 0;
                obj->setting = obj->camera->get_preset(obj->camera, 0, 8);
                obj->setting->trans[2] -= 0x1400;
                obj->setting->eye[2] += 0x1400;
                obj->setting->target[2] += 0x1400;
                obj->speed = 1;
                obj->base.substep++;
            }
            obj->setting->rot[1] += gfx_module.funcs.get_frame_ticks() * obj->speed;
            if (obj->setting->rot[1] >= 0x1000) {
                obj->setting->rot[1] -= 0x1000;
            }
            break;
        case 4:
            if (obj->base.substep == 0) {
                obj->slots->get_params(obj->slots, 0)->layers[0].shown = 0;
                obj->setting = obj->camera->get_preset(obj->camera, 0x10, 0);
                obj->setting->trans[2] += 0x1400;
                obj->setting->eye[2] -= 0x1400;
                obj->setting->target[2] -= 0x1400;
                obj->speed = 1;
                obj->base.substep++;
            }
            obj->setting->rot[1] -= gfx_module.funcs.get_frame_ticks() * obj->speed;
            if (obj->setting->rot[1] < 0) {
                obj->setting->rot[1] += 0x1000;
            }
            break;
        case 5:
            if (obj->base.substep == 0) {
                obj->slots->get_params(obj->slots, 0x10)->layers[0].shown = 0;
                obj->setting = obj->camera->get_preset(obj->camera, 0, 8);
                obj->setting->trans[2] -= 0x1400;
                obj->setting->eye[2] += 0x1400;
                obj->setting->target[2] += 0x1400;
                obj->speed = 1;
                obj->base.substep++;
                obj->setting->rot[1] += 0xE3;
            }
            obj->setting->rot[1] -= gfx_module.funcs.get_frame_ticks() * obj->speed;
            if (obj->setting->rot[1] < 0) {
                obj->setting->rot[1] += 0x1000;
            }
            break;
        case 6:
            if (obj->base.substep == 0) {
                obj->slots->get_params(obj->slots, 0)->layers[0].shown = 0;
                obj->setting = obj->camera->get_preset(obj->camera, 0x10, 0);
                obj->setting->trans[2] += 0x1400;
                obj->setting->eye[2] -= 0x1400;
                obj->setting->target[2] -= 0x1400;
                obj->speed = 1;
                obj->base.substep++;
                obj->setting->rot[1] -= 0xE3;
            }
            obj->setting->rot[1] += gfx_module.funcs.get_frame_ticks() * obj->speed;
            if (obj->setting->rot[1] >= 0x1000) {
                obj->setting->rot[1] -= 0x1000;
            }
            break;
        case 7:
            if (obj->base.substep == 0) {
                obj->slots->get_params(obj->slots, 0x10)->layers[0].shown = 0;
                obj->setting = obj->camera->get_preset(obj->camera, 0, 10);
                obj->base.substep++;
            }
            break;
        case 8:
            if (obj->base.substep == 0) {
                obj->slots->get_params(obj->slots, 0x10)->layers[0].shown = 0;
                obj->setting = obj->camera->get_preset(obj->camera, 0, 8);
                obj->base.substep++;
            }
            break;
        case 9:
            if (obj->base.substep == 0) {
                obj->setting = obj->camera->get_default(obj->camera);
                obj->setting->eye[0] = -0x1400;
                obj->setting->eye[1] = -0x2800;
                obj->setting->eye[2] = 0;
                obj->setting->target[0] = 0;
                obj->setting->target[1] = 0;
                obj->setting->target[2] = 0;
                obj->setting->projection = 0xC8;
                obj->speed = 1;
                obj->setting->rot[1] -= 0x155;
                obj->base.substep++;
            }
            obj->setting->rot[1] += gfx_module.funcs.get_frame_ticks() * obj->speed;
            if (obj->setting->rot[1] >= 0x1000) {
                obj->setting->rot[1] -= 0x1000;
            }
            break;
        case 10:
            if (obj->base.substep == 0) {
                obj->setting = obj->camera->get_default(obj->camera);
                obj->base.substep++;
            }
            break;
        }
        if (obj->setting != NULL) {
            obj->camera->set(obj->camera, obj->setting);
        }
        obj->frames -= gfx_module.funcs.get_frame_ticks();
        break;
    case OBJECT_STATE_END:
        obj->slots->get_params(obj->slots, 0)->layers[0].shown = 1;
        obj->slots->get_params(obj->slots, 0x10)->layers[0].shown = 1;
        obj->setting = obj->camera->get_default(obj->camera);
        obj->camera->set(obj->camera, obj->setting);
        break;
    case OBJECT_STATE_DONE:
        break;
    }
}

OBJECT_V0(FightstgIdleCamera *) fightstg_idle_camera_create(void) {
    /* PC_PORT: FINDINGS 8: callers use the object (v0) */
    OBJECT_V0_TAIL(object_new(fightstg_idle_camera_update, sizeof(FightstgIdleCamera), 0))
}

void fightstg_slots_update(FightstgSlots *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The slot in use with this ID, or -1. */
s32 fightstg_slots_find(FightstgSlots *obj, s32 id) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (obj->slots[i].in_use != 0 && obj->slots[i].id == id) {
            return i;
        }
    }
    return -1;
}

/* A free slot, or -1. */
s32 fightstg_slots_find_free(FightstgSlots *obj) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (obj->slots[i].in_use == 0) {
            return i;
        }
    }
    return -1;
}

/* Ends the model of the slot with this ID and frees the slot. */
void fightstg_slots_remove(FightstgSlots *obj, s32 id) {
    FightstgSlotsData *data = (FightstgSlotsData *)obj->base.children;
    s32 i = fightstg_slots_find(obj, id);

    if (i != -1) {
        data->ending[i] = data->models[i];
        data->models[i] = NULL;
        data->ending[i]->base.set_state(data->ending[i], OBJECT_STATE_END);
        obj->slots[i].in_use = 0;
    }
}

/* Puts a new model (fightstg_models.get(arg2)) in a free slot with this ID, ending the slot that had
 * the ID. */
void fightstg_slots_add(FightstgSlots *obj, s32 id, s32 model_id, s32 shown) {
    FightstgSlotsData *data = (FightstgSlotsData *)obj->base.children;
    FightstgModelRecordA *model;
    FightstgModelParams *params;
    s32 i;

    fightstg_slots_remove(obj, id);
    i = fightstg_slots_find_free(obj);
    if (i != -1) {
        model = fightstg_models.get(model_id);
        params = &obj->slots[i];
        data->models[i] = fightstg_model_create_idle(model->model_file, model->anim_file, fightstg_slots_vram[i], params);
        heap_funcs.bzero(params, sizeof(FightstgModelParams));
        obj->slots[i].in_use = 1;
        obj->slots[i].anim = 1;
        obj->slots[i].model_id = model_id;
        obj->slots[i].id = id;
        obj->slots[i].layers[0].shown = shown;
        obj->slots[i].layers[0].edges = 0;
        obj->slots[i].layers[0].layer_id = 0x1004;
    }
}

FightstgModelParams *fightstg_slots_get_params(FightstgSlots *obj, s32 id) {
    s32 i = fightstg_slots_find(obj, id);

    if (i != -1) {
        return &obj->slots[i];
    }
    return NULL;
}

/* Gives the slot `id` the ID `new_id`, ending the slot that had it. */
void fightstg_slots_set_id(FightstgSlots *obj, s32 id, s32 new_id) {
    fightstg_slots_remove(obj, new_id);
    obj->slots[fightstg_slots_find(obj, id)].id = new_id;
}

s32 fightstg_slots_get_model_id(FightstgSlots *obj, s32 id) {
    s32 i = fightstg_slots_find(obj, id);

    if (i != -1) {
        return obj->slots[i].model_id;
    }
    return 0;
}

/* Puts the slot's model at its start position: IDs below 0x10 on one side facing the other, the rest
 * on the other side. */
void fightstg_slots_reset_pos(FightstgSlots *obj, s32 id) {
    FightstgModelParams *params = fightstg_slots_get_params(obj, id);
    FightstgModelRecordA *model;

    if (params != NULL) {
        model = fightstg_models.get(params->model_id);
        if (id < 0x10) {
            params->pos.x = 0;
            params->pos.y = -model->height;
            params->pos.z = -0x1400 - model->depth;
            params->rot.x = 0;
            params->rot.y = 0x800;
            params->rot.z = 0;
        } else {
            params->pos.x = 0;
            params->pos.y = -model->height;
            params->pos.z = model->depth + 0x1400;
            params->rot.x = 0;
            params->rot.y = 0;
            params->rot.z = 0;
        }
        params->start_pos = params->pos;
        params->start_rot = params->rot;
    }
}

void fightstg_slots_set_idle_anim(FightstgSlots *obj, s32 id, s32 idle_anim) {
    FightstgModelParams *params = fightstg_slots_get_params(obj, id);

    if (params != NULL) {
        params->idle_anim = idle_anim;
    }
}

FightstgSlots *fightstg_slots_create(void) {
    FightstgSlots *obj;
    s32 i;

    obj = object_create(fightstg_slots_update, sizeof(FightstgSlots), sizeof(FightstgSlotsData), 0x14);
    for (i = 0; i < 4; i++) {
        obj->slots[i].in_use = 0;
    }
    obj->add = fightstg_slots_add;
    obj->get_model_id = fightstg_slots_get_model_id;
    obj->remove = fightstg_slots_remove;
    obj->get_params = fightstg_slots_get_params;
    obj->set_id = fightstg_slots_set_id;
    obj->reset_pos = fightstg_slots_reset_pos;
    obj->set_idle_anim = fightstg_slots_set_idle_anim;
    return obj;
}

void fightstg_player_reaction_update(FightstgPlayerReaction *obj, FightstgPlayerReactionData *data) {
    FightstgCamera *other = obj->camera;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            obj->slots = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
            obj->camera = (FightstgCamera *)heap_objects.find(0x12, -1, -1);
            obj->base.next_step(obj);
            /* fallthrough */
        case 1:
            switch (obj->base.substep) {
            case 0:
            default:
                fightstg_effect_get_files(0x33, &obj->image_file, &obj->file, &obj->vram);
                obj->base.next_substep(obj);
                /* fallthrough */
            case 1:
                if (cdload_module.is_loading(obj->file >> 16) == 0) {
                    obj->base.next_state(obj);
                }
                break;
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            other->set(other, other->get_default(other));
            data->effect = (Object *)fightstg_effect_create(0x33, (SVECTOR *)&fightstg_player_reaction_pos);
            sound_module.play(0x800429BF);
            obj->base.next_step(obj);
            /* fallthrough */
        case 1:
            obj->base.timer += gfx_module.funcs.get_frame_ticks();
            if (obj->base.timer < 10) {
                break;
            }
            data->script = fightstg_script_create();
            data->script->side = 0;
            data->script->script = obj->reaction + 1;
            data->script->unk_74 = obj->unk_5C;
            obj->base.next_step(obj);
            /* fallthrough */
        case 2:
            if (data->script == NULL) {
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

OBJECT_V0(FightstgPlayerReaction *) fightstg_player_reaction_create(s32 arg0, s32 arg1) {
    FightstgPlayerReaction *obj = object_new(fightstg_player_reaction_update, sizeof(FightstgPlayerReaction), sizeof(FightstgPlayerReactionData));

    obj->reaction = arg0;
    obj->unk_5C = arg1;
    OBJECT_V0_RETURN(obj) /* PC_PORT: FINDINGS 8: callers use the object (v0) */
}

/* The target of action obj->action, a member of side 1: -2..-4 = member 0..2 if it is in the battle and
 * not side 1's acting member; otherwise one of the other members still in the battle (random if two).
 * -1 if none. */
s32 fightstg_enemy_turn_find_target(FightstgEnemyTurn *obj) {
    FightstgMember *team = fightstg_battle.state.members[1];
    FightstgMember *members;
    s32 count;
    s32 i;
    s32 found[2];
    s32 cur;

    switch (obj->action) {
    case -2:
        if (team[0].digimon == team[fightstg_battle.state.current[1]].digimon || team[0].hp == 0) {
            return -1;
        }
        return 0;
    case -3:
        if (team[1].digimon == team[fightstg_battle.state.current[1]].digimon || team[1].hp == 0) {
            return -1;
        }
        return 1;
    case -4:
        if (team[2].digimon == team[fightstg_battle.state.current[1]].digimon || team[2].hp == 0) {
            return -1;
        }
        return 2;
    }
    count = 0;
    cur = fightstg_battle.state.current[1];
    found[0] = -1;
    found[1] = -1;
    members = fightstg_battle.state.members[1];
    for (i = 0; i < 3; i++) {
        if (cur != i && members[i].digimon != 0 && members[i].hp != 0) {
            found[count++] = i;
        }
    }
    switch (count) {
    case 0:
        return -1;
    case 1:
        return found[0];
    case 2:
        return found[pad_random.next() & 1];
    }
    return -1;
}

const SVECTOR fightstg_player_reaction_pos = { 0, 120, 0x7FFF, 0 };

s32 fightstg_enemy_check_condition(u8 type, s16 value);

/* The enemy's turn (side 0x10): flees at low HP, then shows its state (flags 8/2/4), or picks an action
 * (the first of its Digimon's three whose condition holds) and runs it: an attack, an item, a call for
 * help, a switch of members. */
void fightstg_enemy_turn_update(FightstgEnemyTurn *obj, FightstgEnemyTurnData *data) {
    FightstgMember *enemy;
    FightstgMember *members;
    FightstgMember *member;
    RecordsTechnique *rec;
    FightstgEnemyRecord *digimon;
    FightstgEnemyRecord *info;
    s32 i;
    s32 target;
    s32 event;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            if (fightstg_battle.state.type == 1) {
                enemy = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
                if (enemy->hp < (s16)(enemy->max_hp / 10)) {
                    data->child = fightstg_message_create();
                    obj->args[0] = 0x5F;
                    obj->args[1] = 0x10;
                    data->child->show(data->child, 2, obj->args);
                    fightstg_events_end_battle(0);
                    obj->base.step++;
                } else {
                    obj->base.next_state(obj);
                }
            } else {
                obj->base.next_state(obj);
            }
            break;
        case 1:
            if (data->child == NULL) {
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            switch (obj->base.substep) {
            case 0:
            default:
                event = fightstg_events.find_member(4, 0x10, fightstg_battle.state.current[1]);
                if (event < 0) {
                    obj->base.next_step(obj);
                    break;
                }
                data->child = fightstg_message_create();
                obj->args[0] = 0x60;
                obj->args[1] = 0x10;
                data->child->show(data->child, 2, obj->args);
                fightstg_events.events[event].type = 0;
                obj->base.substep++;
                break;
            case 1:
                if (data->child == NULL) {
                    obj->base.next_step(obj);
                }
                break;
            }
            break;
        case 1:
            member = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
            if (member->status & 8) {
                data->child = fightstg_message_create();
                obj->args[0] = 0x90;
                obj->args[1] = 0x10;
                data->child->show(data->child, 2, obj->args);
                obj->base.set_step(obj, 3);
            } else if ((member->status & 2) && fightstg_rules.roll_paralyzed(0x10) != 0) {
                data->child = fightstg_message_create();
                obj->args[0] = 0x56;
                obj->args[1] = 0x10;
                data->child->show(data->child, 2, obj->args);
                obj->base.set_step(obj, 3);
            } else if (member->status & 4) {
                data->child = fightstg_message_create();
                obj->args[0] = pad_random.next() % 8 + 0x83;
                obj->args[1] = 0x10;
                data->child->show(data->child, 2, obj->args);
                obj->base.set_step(obj, 3);
            } else {
                digimon = fightstg_enemy_records.get(member->digimon);
                for (i = 0; i < 3; i++) {
                    if (fightstg_enemy_check_condition(digimon->actions[i].condition, digimon->actions[i].value) != 0) {
                        break;
                    }
                }
                obj->action = fightstg_enemy_get_action(digimon->actions[i].type);
                obj->base.step++;
            }
            break;
        case 2:
            if (obj->action > 0) {
                if (obj->action == 1) {
                    data->child = fightstg_attack_create(0x10);
                } else {
                    member = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
                    rec = &records_techniques[obj->action - 1];
                    if (member->mp >= rec->mp_cost) {
                        data->child = fightstg_tech_create(0x10, obj->action);
                        member->mp -= rec->mp_cost;
                    } else {
                        data->child = fightstg_message_create();
                        obj->args[0] = 0x8D;
                        obj->args[1] = 0x10;
                        data->child->show(data->child, 2, obj->args);
                    }
                }
                obj->base.step = 3;
            } else if (obj->action < 0) {
                switch (obj->action) {
                case -1:
                default:
                    fightstg_events_add_escape(0x10);
                    data->child = fightstg_message_create();
                    obj->args[0] = 0x5E;
                    obj->args[1] = 0x10;
                    data->child->show(data->child, 2, obj->args);
                    obj->base.step = 3;
                    break;
                case -5:
                case -4:
                case -3:
                case -2:
                    target = fightstg_enemy_turn_find_target(obj);
                    if (target != -1) {
                        obj->called = target;
                        data->child = fightstg_message_create();
                        obj->args[0] = 0x4D;
                        data->child->show(data->child, 1, obj->args);
                        obj->base.step = 5;
                    } else {
                        data->child = fightstg_message_create();
                        obj->args[0] = 0x8E;
                        obj->args[1] = 0x10;
                        data->child->show(data->child, 2, obj->args);
                        obj->base.step = 3;
                    }
                    break;
                }
            }
            break;
        case 3:
            if (data->child != NULL) {
                break;
            }
            obj->base.state = OBJECT_STATE_END;
            fightstg_events_add_enemy_turn(fightstg_events.get_delay(0x10, 0));
            break;
        case 4:
            if (data->child != NULL) {
                break;
            }
            members = fightstg_battle.state.members[1];
            info = fightstg_enemy_records.get(members[fightstg_battle.state.current[1]].digimon);
            data->child = fightstg_message_create();
            obj->args[0] = info->name;
            data->child->show(data->child, 13, obj->args);
            obj->base.step = 3;
            break;
        case 5:
            if (data->child != NULL) {
                break;
            }
            data->child = (FightstgMessage *)fightstg_entrance_create((fightstg_battle.state.members[1] + obj->called)->digimon, 1, 0);
            wfightmn_update_idle_anim(0x10, 0);
            obj->base.step = 6;
            break;
        case 6:
            if (((FightstgEntrance *)data->child)->done == 0) {
                break;
            }
            fightstg_battle.state.current[1] = obj->called;
            obj->base.step = 4;
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

OBJECT_V0(FightstgEnemyTurn *) fightstg_enemy_turn_create(void) {
    /* PC_PORT: FINDINGS 8: callers use the object (v0) */
    OBJECT_V0_TAIL(object_new(fightstg_enemy_turn_update, sizeof(FightstgEnemyTurn), sizeof(FightstgEnemyTurnData)))
}

/* Whether condition `type` holds with `value` (an action's condition): a random chance out of 128,
 * the enemy's or the party's HP under/over value/128 of its maximum, the enemy's MP under/over
 * value, the party member's Digimon among records_digimon, the flags, another enemy member (Digimon
 * `value`) in the battle, the battle's and game's state... 0: always. */
/* The default's `result = 1` comes first: jump2 then cross-jumps every case's `result = 1` into case 18's under a
 * new label, which stops it from merging the cases' identical tails (as the original does). */
s32 fightstg_enemy_check_condition(u8 type, s16 value) {
    FightstgMember *member = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
    s32 result = 0;
    s32 rate;
    s32 i;
    s32 j;
    s32 roll;

    switch (type) {
    case 0:
    default:
        result = 1;
        break;
    case 1:
        roll = pad_random.next() % 128;
        if (roll < value) {
            result = 1;
        }
        break;
    case 2:
        rate = value * 100 / 128;
        if ((s16)(member->max_hp / 100) * rate > member->hp) {
            result = 1;
        }
        break;
    case 3:
        rate = value * 100 / 128;
        if ((s16)(member->max_hp / 100) * rate <= member->hp) {
            result = 1;
        }
        break;
    case 4:
        if (member->mp < value) {
            result = 1;
        }
        break;
    case 5:
        if (member->mp >= value) {
            result = 1;
        }
        break;
    case 6:
        member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
        rate = value * 100 / 128;
        if ((s16)(member->max_hp / 100) * rate > member->hp) {
            result = 1;
        }
        break;
    case 7:
        member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
        rate = value * 100 / 128;
        if ((s16)(member->max_hp / 100) * rate <= member->hp) {
            result = 1;
        }
        break;
    case 8:
        member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
        for (i = 0; i < 8; i++) {
            if (member->digimon == records_digimon[i].id) {
                result = 1;
                break;
            }
        }
        break;
    case 9:
        member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
        if (member->status & 8) {
            result = 1;
        }
        break;
    case 10:
        member = fightstg_battle.state.members[1];
        if (value == 0) {
            for (j = 0; j < 3; j++) {
                if (j != fightstg_battle.state.current[1] && member[j].digimon != 0 && member[j].hp != 0) {
                    result = 1;
                    break;
                }
            }
        } else {
            for (j = 0; j < 3; j++) {
                if (j != fightstg_battle.state.current[1] && member[j].digimon == value && member[j].hp != 0) {
                    result = 1;
                    break;
                }
            }
        }
        break;
    case 11:
        if (fightstg_battle.state.field.element == value) {
            result = 1;
        }
        break;
    case 12:
        if (records_state.battle == value) {
            result = 1;
        }
        break;
    case 13:
        if (records_state.battle_kind == value) {
            result = 1;
        }
        break;
    case 14:
        if (member->power_up != 0) {
            result = 1;
        }
        break;
    case 15:
        if (member->modifiers[0] < 0) {
            result = 1;
        }
        break;
    case 16:
        if (member->modifiers[1] < 0) {
            result = 1;
        }
        break;
    case 17:
        if (member->modifiers[2] < 0) {
            result = 1;
        }
        break;
    case 18:
        if (member->turns % value == 0) {
            result = 1;
        }
        break;
    }
    return result;
}

s32 fightstg_enemy_get_action(u8 type) {
    FightstgMember *member = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
    s32 result = 0;
    FightstgEnemyRecord *digimon = fightstg_enemy_records.get(member->digimon);

    switch (type) {
    case 1:
        result = 1;
        break;
    case 2:
        result = digimon->tech_2;
        break;
    case 3:
        result = digimon->tech_3;
        break;
    case 4:
        result = -1;
        break;
    case 5:
        result = -2;
        break;
    case 6:
        result = -3;
        break;
    case 7:
        result = -4;
        break;
    case 8:
        result = -5;
        break;
    }
    return result;
}

void fightstg_nop(void) {
}

/* Draws the animation's current frame (a layer callback, `data` is the object). */
void fightstg_sprite_anim_draw(void *data, GfxLayer *layer, s32 arg) {
    FightstgSpriteAnim *obj = data;
    SVECTOR pos;
    Sprite sprite;

    if (obj->pos.vz != 0x7FFF && obj->pos.vz != -1) {
        fightstg_battle.to_screen(layer, &obj->pos, &pos);
    } else {
        pos.vx = obj->pos.vx;
        pos.vy = obj->pos.vy;
        if (obj->pos.vz != 0x7FFF) {
            pos.vz = 0xFFF;
        } else {
            pos.vz = 0;
        }
    }
    pos.vx += obj->values[2];
    pos.vy += obj->values[3];
    sprite_init(&sprite);
    sprite.set_layer(layer, pos.vz);
    sprite.set_vram_pos(obj->vram.x, obj->vram.y);
    if (obj->values[4] != 0x1000 || obj->values[5] != 0x1000) {
        sprite.set_scale(obj->values[4], obj->values[5], 0);
    }
    if (obj->values[1] != 0) {
        sprite.set_palette(obj->values[1]);
    }
    if (obj->values[6] != 0 || obj->values[7] != 0 || obj->values[8] != 0) {
        sprite.set_rotation(obj->values[6], obj->values[7], obj->values[8]);
    }
    sprite.set_pivot(pos.vx, pos.vy);
    sprite.draw(cdload_module.get_subfile_by_id(obj->file), obj->values[0], pos.vx, pos.vy);
}

void fightstg_sprite_anim_update(FightstgSpriteAnim *obj) {
    s16 *p;
    s32 i;
    GfxLayer *layer;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->flags = *obj->stream++;
        obj->values_per_frame = *obj->stream++;
        obj->frames = *obj->stream++;
        for (i = 0; i < 9; i++) {
            obj->values[i] = *obj->stream++;
        }
        obj->base.next_state(obj);
        /* fallthrough */
    case OBJECT_STATE_RUN:
        p = obj->stream + obj->frame * obj->values_per_frame;
        if (obj->flags & 0x1) {
            obj->values[0] = *p++;
        }
        if (obj->flags & 0x2) {
            obj->values[1] = *p++;
        }
        if (obj->flags & 0x4) {
            obj->values[2] = *p++;
        }
        if (obj->flags & 0x8) {
            obj->values[3] = *p++;
        }
        if (obj->flags & 0x10) {
            obj->values[4] = *p++;
        }
        if (obj->flags & 0x20) {
            obj->values[5] = *p++;
        }
        if (obj->flags & 0x40) {
            obj->values[6] = *p++;
        }
        if (obj->flags & 0x80) {
            obj->values[7] = *p++;
        }
        if (obj->flags & 0x100) {
            obj->values[8] = *p++;
        }
        if (obj->values[4] != 0 && obj->values[5] != 0) {
            layer = gfx_module.funcs.get_layer(obj->layer_id);
            layer->append_callback(layer, fightstg_sprite_anim_draw, obj);
        }
        obj->frame += fightstg_battle.frames;
        if (obj->frame >= obj->frames) {
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        break;
    case OBJECT_STATE_DONE:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

FightstgSpriteAnim *fightstg_sprite_anim_create(s16 *data, SVECTOR *screen_pos, s32 file, FightstgPos *pos, s32 layer) {
    FightstgSpriteAnim *obj = object_new(fightstg_sprite_anim_update, sizeof(FightstgSpriteAnim), 0);

    obj->stream = data;
    obj->pos = *screen_pos;
    obj->file = file;
    obj->vram = *pos;
    obj->layer_id = layer;
    return obj;
}

void fightstg_prop_update(FightstgProp *obj, FightstgModel **data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->params.layers[0].shown = 1;
        obj->params.layers[0].layer_id = 0x1004;
        obj->params.model_id = 0;
        obj->params.layers[0].edges = 0;
        obj->params.pos.x = obj->pos.vx;
        obj->params.pos.y = obj->pos.vy;
        obj->params.pos.z = obj->pos.vz;
        obj->params.rot.x = obj->rot.vx;
        obj->params.rot.y = obj->rot.vy;
        obj->params.rot.z = obj->rot.vz;
        *data = fightstg_model_create(obj->model_file, obj->anim_file, obj->vram, &obj->params);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->params.anim_done != 0) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

s32 fightstg_prop_get_file(s32 id) {
    FightstgPropEntry *entry;

    for (entry = fightstg_props; entry->id != 0; entry++) {
        if (entry->id == id) {
            return entry->model_file >> 16;
        }
    }
    return 0;
}

FightstgProp *fightstg_prop_create(s32 id, SVECTOR *pos, SVECTOR *rot) {
    FightstgProp *obj = object_new(fightstg_prop_update, sizeof(FightstgProp), sizeof(FightstgModel *));
    FightstgPropEntry *entry;

    obj->id = 0;
    for (entry = fightstg_props; entry->id != 0; entry++) {
        if (entry->id == id) {
            obj->id = id;
            obj->anim_file = entry->anim_file;
            obj->model_file = entry->model_file;
            /* Evidence (class A2, sched2 barrier; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): the original stores unk_6C only after
             * unk_54, never interleaved. */
            LOOP_BARRIER();
            obj->vram.x = 0x280;
            obj->vram.y = 0x100;
            obj->pos = *pos;
            obj->rot = *rot;
            break;
        }
    }
    if (obj->id == 0) {
        obj->base.set_state(obj, OBJECT_STATE_END);
    }
    return obj;
}

/* The model of animation `id`: *arg1, *arg2 and *pos from its fightstg_effect_files entry. 0 if none. */
s32 fightstg_effect_get_files(s32 id, s32 *image_file, s32 *file, FightstgPos *pos) {
    FightstgEffectEntry *entry;

    for (entry = fightstg_effects; entry->id != -1; entry++) {
        if (entry->id == id) {
            *image_file = fightstg_effect_files[entry->files].image_file;
            *file = fightstg_effect_files[entry->files].file;
            *pos = fightstg_effect_files[entry->files].vram;
            return 1;
        }
    }
    return 0;
}

void fightstg_effect_update(FightstgEffect *obj, FightstgSpriteAnim **anims) {
    s32 *data;
    s32 layer;
    s32 count;
    s32 i;
    s32 j;
    s32 done;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data = cdload_module.get_subfile_by_id(obj->file);
        layer = 0x1004;
        if (obj->id >= 1000 && obj->id < 1003) {
            layer = 0x1006;
        }
        if (obj->id == 1007) {
            layer = 0x1006;
        }
        for (count = 0; count < 30; count++) {
            if (data[count] == 0) {
                break;
            }
        }
        for (i = 0; i < count; i++) {
            *anims++ = fightstg_sprite_anim_create(cdload_module.get_subfile(i, data), &obj->pos,
                                              fightstg_effect_files[obj->files].file,
                                              &fightstg_effect_files[obj->files].vram, layer);
        }
        obj->anim_count = count;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        done = 1;
        for (j = 0; j < obj->anim_count; j++) {
            if (anims[j] != NULL) {
                done = 0;
                break;
            }
        }
        if (done) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

FightstgEffect *fightstg_effect_create(s32 id, SVECTOR *pos) {
    FightstgEffect *obj = object_new(fightstg_effect_update, sizeof(FightstgEffect), 30 * sizeof(FightstgSpriteAnim *));
    FightstgEffectEntry *entry;

    obj->id = -1;
    for (entry = fightstg_effects; entry->id != -1; entry++) {
        if (entry->id == id) {
            obj->id = id;
            obj->files = entry->files;
            obj->file = entry->file;
            obj->pos = *pos;
        }
    }
    if (obj->id == -1) {
        obj->base.set_state(obj, OBJECT_STATE_END);
    }
    return obj;
}

/* A model's scene (fightstg_digivolve_create): loads model base.key1 into slot 1 and its files (0x8A2: an
 * image), then shows it between two black bars (layers 0x1004 and 0x1003, closing and opening),
 * with effects 0x3E8... (fightstg_effect_create) and a fade; base.key2 picks the variant. */
void fightstg_digivolve_update(FightstgDigivolve *obj, FightstgDigivolveData *data) {
    FightstgSlots *slots = obj->slots;
    FightstgCamera *camera = obj->camera;
    FightstgStage *stage = obj->stage;
    /* Effect positions (.rodata 0x80082DC8, 0x80082DD0, 0x80082DD8). */
    static const SVECTOR pos_origin = { 0, 0, 0x7FFF, 0 };
    static const SVECTOR pos_center = { 160, 120, 0x7FFF, 0 };
    static const SVECTOR pos_corner = { -160, -120, -1, 0 };
    FightstgModelParams *params;
    FightstgModelParams *model;
    GfxLayer *layer;
    GfxLayer *bar;
    Tim tim;
    s16 step;
    s32 z;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            switch (obj->base.substep) {
            case 0:
            default:
                sound_module.load_extra_bank(0x45);
                obj->base.next_substep(obj);
                /* fallthrough */
            case 1:
                if (sound_module.is_loading() != 0) {
                    break;
                }
                obj->base.next_step(obj);
                break;
            }
            break;
        case 1:
            obj->slots = slots = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
            obj->camera = (FightstgCamera *)heap_objects.find(0x12, -1, -1);
            obj->stage = (FightstgStage *)heap_objects.find(0x15, -1, -1);
            obj->file = (s16)(((FightstgModelRecordA *)fightstg_models.get(obj->base.key1))->model_file >> 16);
            cdload_module.queue_file(obj->file);
            obj->idle_anim = slots->get_params(slots, 0)->idle_anim;
            obj->base.next_step(obj);
            /* fallthrough */
        case 2:
            if (cdload_module.is_loading(obj->file) != 0) {
                break;
            }
            cdload_module.queue_file(0x8A2);
            obj->base.next_step(obj);
            /* fallthrough */
        case 3:
            if (cdload_module.is_loading(0x8A2) != 0) {
                break;
            }
            tim_init(&tim);
            tim.set_image_pos(0x300, 0x100);
            tim.load_all(cdload_module.get_subfile_by_id(0x08A20005));
            obj->base.next_state(obj);
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            stage->change(stage, obj->base.key2 != 0 ? 0x1F : 0x1C, 0x20, 0x20);
            obj->base.next_step(obj);
            /* fallthrough */
        case 1:
            switch (obj->base.substep) {
            case 0:
            default:
                if (stage->base.state == OBJECT_STATE_DONE) {
                    break;
                }
                sound_module.play(0x41140000);
                slots->add(slots, 1, obj->base.key1, 0);
                obj->base.next_substep(obj);
                /* fallthrough */
            case 1:
                obj->base.timer += gfx_module.funcs.get_frame_ticks();
                if (obj->base.timer < 180) {
                    break;
                }
                obj->base.next_step(obj);
                break;
            }
            break;
        case 2:
            obj->top_bar.x = 0;
            obj->top_bar.y = 0;
            obj->top_bar.w = 320;
            obj->top_bar.h = 240;
            obj->bottom_bar.x = 0;
            obj->bottom_bar.y = 240;
            obj->bottom_bar.w = 320;
            obj->bottom_bar.h = 0;
            params = slots->get_params(slots, 0);
            camera->get_preset(camera, 0, 9);
            z = params->start_pos.z;
            z = -z;
            fightstg_camera_setting.eye[2] += z;
            fightstg_camera_setting.target[2] += z;
            camera->set(camera, &fightstg_camera_setting);
            params->layers[1].shown = 1;
            params->layers[1].edges = 1;
            params->layers[1].layer_id = 0x1003;
            params->pos.x = 0;
            params->pos.z = 0;
            params->rot.x = 0;
            params->rot.y = 0x800;
            params->rot.z = 0;
            params->pos.y = params->start_pos.y;
            if (obj->base.key2 == 0) {
                params->anim = 13;
            } else {
                params->anim = 14;
            }
            slots->get_params(slots, 0x10)->layers[0].shown = 0;
            if (obj->base.key2 == 0) {
                data->effects[0] = fightstg_effect_create(0x3E8, (SVECTOR *)&pos_origin);
                data->effects[1] = fightstg_effect_create(0x3E9, (SVECTOR *)&pos_origin);
            } else {
                data->effects[0] = fightstg_effect_create(0x3EF, (SVECTOR *)&pos_origin);
            }
            obj->base.next_step(obj);
            /* fallthrough */
        case 3:
            step = gfx_module.funcs.get_frame_ticks() * 2;
            obj->top_bar.h -= step;
            obj->bottom_bar.h += step;
            obj->bottom_bar.y -= step;
            if (obj->bottom_bar.y <= 0) {
                obj->top_bar.h = 0;
                obj->bottom_bar.h = 240;
                obj->bottom_bar.y = 0;
                slots->get_params(slots, 0)->layers[0].shown = 0;
                data->effects[2] = fightstg_effect_create(0x3EA, (SVECTOR *)&pos_center);
                obj->base.next_step(obj);
            }
            layer = gfx_module.funcs.get_layer(0x1004);
            layer->set_clip_pos(layer, obj->top_bar.x, obj->top_bar.y);
            layer->set_clip_size(layer, obj->top_bar.w, obj->top_bar.h);
            layer = gfx_module.funcs.get_layer(0x1003);
            layer->set_clip_pos(layer, obj->bottom_bar.x, obj->bottom_bar.y);
            layer->set_clip_size(layer, obj->bottom_bar.w, obj->bottom_bar.h);
            break;
        case 4:
            obj->base.timer += gfx_module.funcs.get_frame_ticks();
            if (obj->base.timer < 120) {
                break;
            }
            obj->base.next_step(obj);
            break;
        case 5:
            switch (obj->base.substep) {
            case 0:
            default:
                data->flash = (Object *)fightstg_flash_create(0x20);
                obj->base.next_substep(obj);
                break;
            case 1:
                if (data->flash->step == 0) {
                    break;
                }
                obj->base.next_step(obj);
                break;
            }
            break;
        case 6:
            slots->set_id(slots, 1, 0);
            model = slots->get_params(slots, 0);
            model->layers[0].shown = 1;
            model->layers[0].layer_id = 0x1004;
            model->layers[0].edges = 0;
            model->layers[1].shown = 1;
            model->layers[1].edges = 1;
            model->layers[1].layer_id = 0x1003;
            slots->reset_pos(slots, 0);
            camera->get_preset(camera, 0, 9);
            z = model->start_pos.z;
            z = -z;
            fightstg_camera_setting.eye[2] += z;
            fightstg_camera_setting.target[2] += z;
            camera->set(camera, &fightstg_camera_setting);
            model->pos.x = 0;
            model->pos.y = model->start_pos.y;
            model->pos.z = 0;
            model->rot.x = 0;
            model->rot.y = 0x800;
            model->rot.z = 0;
            obj->top_bar.x = 0;
            obj->top_bar.y = 0;
            obj->top_bar.w = 320;
            obj->top_bar.h = 0;
            obj->bottom_bar.x = 0;
            obj->bottom_bar.y = 0;
            obj->bottom_bar.w = 320;
            obj->bottom_bar.h = 240;
            obj->base.next_step(obj);
            /* fallthrough */
        case 7:
            switch (obj->base.substep) {
            case 0:
            default:
                data->flash->set_state(data->flash, OBJECT_STATE_DONE);
                obj->base.next_substep(obj);
                break;
            case 1:
                if (data->flash != NULL) {
                    break;
                }
                obj->base.next_step(obj);
                break;
            }
            break;
        case 8:
            step = gfx_module.funcs.get_frame_ticks() * 2;
            obj->top_bar.h += step;
            obj->bottom_bar.y += step;
            obj->bottom_bar.h -= step;
            if (obj->top_bar.h >= 240) {
                obj->top_bar.h = 240;
                obj->bottom_bar.y = 240;
                obj->bottom_bar.h = 0;
                slots->get_params(slots, 0)->layers[1].shown = 0;
                data->effects[3] = fightstg_effect_create(obj->base.key2 == 0 ? 0x3EB : 0x3F0,
                                                         (SVECTOR *)&pos_corner);
                obj->base.next_step(obj);
            }
            bar = gfx_module.funcs.get_layer(0x1004);
            bar->set_clip_pos(bar, obj->top_bar.x, obj->top_bar.y);
            bar->set_clip_size(bar, obj->top_bar.w, obj->top_bar.h);
            bar = gfx_module.funcs.get_layer(0x1003);
            bar->set_clip_pos(bar, obj->bottom_bar.x, obj->bottom_bar.y);
            bar->set_clip_size(bar, obj->bottom_bar.w, obj->bottom_bar.h);
            break;
        case 9:
            slots->get_params(slots, 0)->anim = 13;
            obj->base.next_step(obj);
            /* fallthrough */
        case 10:
            obj->base.timer += gfx_module.funcs.get_frame_ticks();
            if (obj->base.timer < 180) {
                break;
            }
            obj->base.next_step(obj);
            break;
        case 11:
            stage->change(stage, records_state.stage, 0x20, 0x20);
            obj->base.next_step(obj);
            /* fallthrough */
        case 12:
            if (stage->base.state == OBJECT_STATE_DONE) {
                break;
            }
            slots->reset_pos(slots, 0);
            camera->set(camera, camera->get_preset(camera, 0, 9));
            obj->base.next_step(obj);
            break;
        case 13:
            obj->base.next_step(obj);
            break;
        case 14:
            switch (obj->base.substep) {
            case 0:
            default:
                slots->get_params(slots, 0x10)->layers[0].shown = 1;
                slots->get_params(slots, 0)->anim = (obj->base.key2 != 0 || obj->idle_anim == 0) ? 1 : 2;
                camera->move(camera, 0, camera->get_default(camera), 60);
                sound_module.play(records_state.music);
                obj->base.next_substep(obj);
                break;
            case 1:
                obj->base.timer += gfx_module.funcs.get_frame_ticks();
                if (obj->base.timer < 60) {
                    break;
                }
                obj->base.set_state(obj, OBJECT_STATE_END);
                break;
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

OBJECT_V0(Object *) fightstg_digivolve_create(s32 digimon, s32 blast) {
    Object *obj = object_new(fightstg_digivolve_update, sizeof(FightstgDigivolve), sizeof(FightstgDigivolveData));

    obj->key1 = digimon;
    obj->key2 = blast;
    OBJECT_V0_RETURN(obj) /* PC_PORT: FINDINGS 8: callers use the object (v0) */
}

/* Starts a fade up (down == 0) or down over `frames` frames. */
void fightstg_fade_start(Fade *obj, s32 down, s32 frames) {
    obj->base.set_state(obj, OBJECT_STATE_RUN);
    obj->base.step = 1;
    obj->from_black = down;
    if (down == 0) {
        obj->level = 0;
        obj->step = 0xFF00 / frames;
    } else {
        obj->level = 0xFF00;
        obj->step = -(0xFF00 / frames);
    }
}

/* Draws a full-screen (320x256) quad with subtractive blending (abr 2), each channel level >> 8
 * (as inn.c's inn_fade_draw). */
void fightstg_fade_draw(Fade *obj) {
    GfxLayer *layer;
    u32 *ot;
    POLY_F4 *poly;
    DR_TPAGE *tpage;

    layer = gfx_module.funcs.get_layer(obj->layer_id);
    ot = layer->get_ot_entry(layer, obj->ot_depth);
    poly = gfx_module.funcs.get_packet();
    setPolyF4(poly);
    setSemiTrans(poly, 1);
    poly->r0 = poly->g0 = poly->b0 = obj->level >> 8;
    poly->x0 = poly->x2 = 0;
    poly->x1 = poly->x3 = 320;
    poly->y0 = poly->y1 = 0;
    poly->y2 = poly->y3 = 256;
    addPrim(ot, poly);
    tpage = (DR_TPAGE *)(poly + 1);
    setDrawTPage(tpage, 0, 1, getTPage(0, 2, 320, 0));
    addPrim(ot, tpage);
    gfx_module.funcs.set_packet(tpage + 1);
}

void fightstg_fade_update(Fade *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.state = OBJECT_STATE_RUN;
        break;
    case OBJECT_STATE_RUN:
        if (obj->base.step == 0) {
            break;
        }
        obj->level += obj->step;
        if (obj->from_black == 0) {
            if (obj->level > 0xFF00) {
                obj->level = 0xFF00;
                obj->base.state = OBJECT_STATE_DONE;
            }
        } else if (obj->level < 0) {
            obj->level = 0;
            obj->base.state = OBJECT_STATE_DONE;
        }
        /* fallthrough */
    case OBJECT_STATE_DONE:
        fightstg_fade_draw(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

OBJECT_V0(Fade *) fightstg_fade_create(void) {
    Fade *obj = object_new(fightstg_fade_update, sizeof(Fade), 0);

    obj->start = fightstg_fade_start;
    obj->layer_id = 0x1006;
    obj->ot_depth = 0;
    OBJECT_V0_RETURN(obj) /* PC_PORT: FINDINGS 8: callers use the object (v0) */
}

void fightstg_lights_update(FightstgLighting *obj) {
    SVECTOR from;
    SVECTOR to;
    SVECTOR out;
    s32 t;
    s32 i;
    s32 j;
    GfxLayer *layer;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->fade_pos = 0x1000;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_DONE:
        obj->base.set_state(obj, OBJECT_STATE_RUN);
        obj->base.set_step(obj, 1);
        /* fallthrough */
    case OBJECT_STATE_RUN:
        if (obj->base.step >= 3) {
            break;
        }
        if (obj->base.step > 0) {
            if (obj->fade_pos != 0x1000) {
                obj->fade_pos += obj->fade_step * gfx_module.funcs.get_frame_ticks();
                if (obj->fade_pos < 0x1000) {
                    t = obj->fade_pos;
                    for (i = 0; i < 3; i++) {
                        from.vx = obj->fade_from.lights[i].vx;
                        from.vy = obj->fade_from.lights[i].vy;
                        from.vz = obj->fade_from.lights[i].vz;
                        to.vx = obj->fade_to.lights[i].vx;
                        to.vy = obj->fade_to.lights[i].vy;
                        to.vz = obj->fade_to.lights[i].vz;
                        fightstg_math.lerp(&from, &to, t, &out);
                        obj->lights.lights[i].vx = out.vx;
                        obj->lights.lights[i].vy = out.vy;
                        obj->lights.lights[i].vz = out.vz;
                        from.vx = obj->fade_from.lights[i].r;
                        from.vy = obj->fade_from.lights[i].g;
                        from.vz = obj->fade_from.lights[i].b;
                        to.vx = obj->fade_to.lights[i].r;
                        to.vy = obj->fade_to.lights[i].g;
                        to.vz = obj->fade_to.lights[i].b;
                        fightstg_math.lerp(&from, &to, t, &out);
                        obj->lights.lights[i].r = out.vx;
                        obj->lights.lights[i].g = out.vy;
                        obj->lights.lights[i].b = out.vz;
                    }
                    from.vx = obj->fade_from.ambient_r;
                    from.vy = obj->fade_from.ambient_g;
                    from.vz = obj->fade_from.ambient_b;
                    to.vx = obj->fade_to.ambient_r;
                    to.vy = obj->fade_to.ambient_g;
                    to.vz = obj->fade_to.ambient_b;
                    fightstg_math.lerp(&from, &to, t, &out);
                    obj->lights.ambient_r = out.vx;
                    obj->lights.ambient_g = out.vy;
                    obj->lights.ambient_b = out.vz;
                } else {
                    obj->fade_pos = 0x1000;
                    obj->lights = obj->fade_to;
                }
            }
            GsSetLightMode(0);
            for (j = 0; j < 3; j++) {
                GsSetFlatLight(j, &obj->lights.lights[j]);
            }
            SetBackColor(obj->lights.ambient_r, obj->lights.ambient_g, obj->lights.ambient_b);
            layer = gfx_module.funcs.get_layer(obj->layer_id);
            layer->save_light(layer, 1);
            if (obj->fade_pos == 0x1000) {
                obj->base.next_step(obj);
            }
        }
        break;
    case OBJECT_STATE_END:
        layer = gfx_module.funcs.get_layer(obj->layer_id);
        layer->save_light(layer, 0);
        break;
    }
}

void fightstg_lights_set(FightstgLighting *obj, FightstgLights *lights) {
    obj->lights = *lights;
    obj->base.set_state(obj, OBJECT_STATE_DONE);
}

/* Fades the lighting from `from` (NULL: the current one) to `to` over `frames` frames. */
void fightstg_lights_fade(FightstgLighting *obj, FightstgLights *from, FightstgLights *to, s32 frames) {
    if (from != NULL) {
        obj->fade_from = *from;
    } else {
        obj->fade_from = obj->lights;
    }
    obj->fade_to = *to;
    obj->fade_step = 0x1000 / frames;
    obj->fade_pos = obj->fade_step * gfx_module.funcs.get_frame_ticks();
    obj->base.set_state(obj, OBJECT_STATE_DONE);
}

FightstgLights *fightstg_lights_get_stage(FightstgLighting *obj, s32 index) {
    FightstgStageRecord *recs = (FightstgStageRecord *)cdload_module.files.get_file(0x1CB);

    return &recs[index].lights;
}

OBJECT_V0(FightstgLighting *) fightstg_lights_create(s32 layer_id) {
    FightstgLighting *obj = object_create(fightstg_lights_update, sizeof(FightstgLighting), 0, 0x13);

    obj->set = fightstg_lights_set;
    obj->fade = fightstg_lights_fade;
    obj->layer_id = layer_id;
    obj->get_stage = fightstg_lights_get_stage;
    OBJECT_V0_RETURN(obj) /* PC_PORT: FINDINGS 8: callers use the object (v0) */
}

/* .data (address order) */

FightstgCameraStep fightstg_idle_camera_steps[4][6] = {
    { { 1800, 1 }, { 480, 9 }, { 360, 3 }, { 300, 4 }, { 480, 9 }, { -1, 0 } },
    { { 360, 6 }, { 900, 1 }, { 300, 5 }, { 480, 9 }, { -1, 0 }, { 0, 0 } },
    { { 1920, 1 }, { 360, 6 }, { 180, 7 }, { 480, 9 }, { -1, 0 }, { 0, 0 } },
    { { 90, 10 }, { 90, 5 }, { -1, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
};

u8 fightstg_idle_camera_next[8][3] = {
    { 1, 2, 1 }, { 0, 2, 2 }, { 0, 1, 0 }, { 0, 1, 2 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
};

FightstgPos fightstg_slots_vram[4] = { { 832, 0 }, { 896, 0 }, { 960, 0 }, { 960, 256 } };

FightstgPropEntry fightstg_props[131] = {
    { 39, 0x080D001A, 0x080D0000 }, { 65, 0x076F0004, 0x076F0000 }, { 66, 0x08140004, 0x08140001 },
    { 85, 0x083A0006, 0x083A0000 }, { 86, 0x083B0007, 0x083B0000 }, { 87, 0x07D80016, 0x07D80000 },
    { 88, 0x077D0006, 0x077D0000 }, { 89, 0x07FB0006, 0x07FB0000 }, { 90, 0x0807000A, 0x08070000 },
    { 91, 0x08080008, 0x08080001 }, { 92, 0x08090006, 0x08090000 }, { 93, 0x075A001A, 0x075A0000 },
    { 94, 0x075C002C, 0x075C0000 }, { 95, 0x0772000A, 0x07720000 }, { 96, 0x07E4001A, 0x07E40004 },
    { 97, 0x08040003, 0x08040000 }, { 98, 0x08060007, 0x08060000 }, { 99, 0x080A0008, 0x080A0001 },
    { 100, 0x080B0005, 0x080B0006 }, { 101, 0x07FC0003, 0x07FC0001 }, { 102, 0x07FD0006, 0x07FD0001 },
    { 103, 0x07FE0006, 0x07FE0001 }, { 106, 0x07FF000E, 0x07FF0002 }, { 107, 0x0800000A, 0x08000001 },
    { 108, 0x07D9000C, 0x07D90000 }, { 109, 0x07DA001C, 0x07DA0005 }, { 113, 0x080C0005, 0x080C0006 },
    { 114, 0x0885000C, 0x08850002 }, { 115, 0x088E0001, 0x088E0000 }, { 116, 0x088F000A, 0x088F0000 },
    { 201, 0x081E0004, 0x081E0000 }, { 202, 0x081F0004, 0x081F0000 }, { 203, 0x08200004, 0x08200000 },
    { 204, 0x08210004, 0x08210000 }, { 205, 0x08220004, 0x08220000 }, { 206, 0x08230004, 0x08230000 },
    { 207, 0x08240004, 0x08240000 }, { 208, 0x08250004, 0x08250000 }, { 209, 0x08260004, 0x08260000 },
    { 210, 0x08270004, 0x08270000 }, { 211, 0x08280004, 0x08280000 }, { 212, 0x08290004, 0x08290000 },
    { 213, 0x082A0004, 0x082A0000 }, { 214, 0x082B0004, 0x082B0000 }, { 215, 0x082C0004, 0x082C0000 },
    { 216, 0x082D0004, 0x082D0000 }, { 217, 0x082E0004, 0x082E0000 }, { 218, 0x082F0004, 0x082F0000 },
    { 219, 0x08300004, 0x08300000 }, { 220, 0x08310004, 0x08310000 }, { 221, 0x07CF0004, 0x07CF0000 },
    { 222, 0x08320004, 0x08320000 }, { 223, 0x08330004, 0x08330000 }, { 224, 0x08340004, 0x08340000 },
    { 225, 0x08350004, 0x08350000 }, { 226, 0x08360004, 0x08360000 }, { 301, 0x083C000E, 0x083C0000 },
    { 302, 0x083E0015, 0x083E0016 }, { 303, 0x083F0004, 0x083F0001 }, { 306, 0x08400004, 0x08400000 },
    { 307, 0x08410006, 0x08410000 }, { 308, 0x08420008, 0x08420000 }, { 309, 0x08430004, 0x08430000 },
    { 310, 0x0845000A, 0x08450000 }, { 310, 0x0845000A, 0x08450000 }, { 311, 0x08460004, 0x08460000 },
    { 312, 0x08470004, 0x08470000 }, { 313, 0x08480004, 0x08480000 }, { 314, 0x08490004, 0x08490000 },
    { 315, 0x084A000A, 0x084A0000 }, { 316, 0x084B0006, 0x084B0001 }, { 317, 0x084C0001, 0x084C0000 },
    { 318, 0x084D0004, 0x084D0001 }, { 319, 0x084E000A, 0x084E0006 }, { 320, 0x084F000A, 0x084F0000 },
    { 321, 0x08500004, 0x08500000 }, { 322, 0x08510004, 0x08510000 }, { 323, 0x08520004, 0x08520000 },
    { 324, 0x08530004, 0x08530000 }, { 325, 0x08540004, 0x08540000 }, { 326, 0x08550006, 0x08550000 },
    { 327, 0x0856000D, 0x0856000E }, { 328, 0x08570006, 0x08570000 }, { 329, 0x08580006, 0x08580000 },
    { 330, 0x08590008, 0x08590000 }, { 331, 0x085A0008, 0x085A0000 }, { 332, 0x085B0008, 0x085B0000 },
    { 333, 0x085C000A, 0x085C0000 }, { 334, 0x085D000A, 0x085D0000 }, { 335, 0x085E0006, 0x085E0000 },
    { 336, 0x085F0005, 0x085F0006 }, { 337, 0x0860000C, 0x08600000 }, { 338, 0x08610006, 0x08610000 },
    { 339, 0x08620007, 0x08620008 }, { 340, 0x08630005, 0x08630006 }, { 341, 0x0864000C, 0x08640000 },
    { 342, 0x08650001, 0x08650000 }, { 343, 0x08660001, 0x08660000 }, { 344, 0x08670000, 0x08670008 },
    { 345, 0x08680000, 0x08680008 }, { 346, 0x08690015, 0x08690016 }, { 347, 0x086A0004, 0x086A0000 },
    { 348, 0x086B0001, 0x086B0000 }, { 349, 0x086C000A, 0x086C0000 }, { 350, 0x086D0019, 0x086D001A },
    { 351, 0x086E000A, 0x086E0000 }, { 352, 0x086F0008, 0x086F0000 }, { 353, 0x08700008, 0x08700000 },
    { 354, 0x08710009, 0x0871000A }, { 355, 0x08720001, 0x08720000 }, { 356, 0x08730008, 0x08730000 },
    { 357, 0x08740001, 0x08740000 }, { 358, 0x0875000A, 0x08750000 }, { 359, 0x08760008, 0x08760000 },
    { 360, 0x0877000C, 0x08770002 }, { 361, 0x08780011, 0x08780012 }, { 362, 0x0879000C, 0x08790000 },
    { 363, 0x087A0011, 0x087A0012 }, { 364, 0x087B0031, 0x087B0032 }, { 365, 0x087C001A, 0x087C0000 },
    { 366, 0x087D0006, 0x087D0000 }, { 367, 0x087E0006, 0x087E0000 }, { 368, 0x087F0004, 0x087F0001 },
    { 369, 0x08800001, 0x08800000 }, { 370, 0x08810012, 0x08810000 }, { 371, 0x08820008, 0x08820000 },
    { 372, 0x0883000A, 0x08830000 }, { 374, 0x08840012, 0x08840000 }, { 375, 0x0844000E, 0x08440003 },
    { 376, 0x08A10012, 0x08A1000C }, { 0, 0, 0 },
};

FightstgEffectFiles fightstg_effect_files[51] = {
    { 0, 0x079D0026, { 320, 256 } }, { 0, 0x07A30000, { 448, 256 } }, { 0, 0x07E00000, { 512, 256 } },
    { 0, 0x07E20000, { 576, 256 } }, { 0, 0, { 576, 256 } }, { 0x07F50006, 0x07F50000, { 768, 256 } },
    { 0x08A20005, 0x08A20000, { 768, 256 } }, { 0x080F0002, 0x080F0000, { 896, 256 } },
    { 0x078B0002, 0x078B0000, { 896, 256 } }, { 0x080E0003, 0x080E0000, { 896, 256 } },
    { 0x07890003, 0x07890000, { 896, 256 } }, { 0x081C0002, 0x081C0001, { 896, 256 } },
    { 0x07E50003, 0x07E50000, { 896, 256 } }, { 0x06D90002, 0x06D90000, { 896, 256 } },
    { 0x07E60003, 0x07E60000, { 896, 256 } }, { 0x08110002, 0x08110000, { 896, 256 } },
    { 0x081D0002, 0x081D0001, { 896, 256 } }, { 0x078C0002, 0x078C0000, { 896, 256 } },
    { 0x07DB0002, 0x07DB0000, { 896, 256 } }, { 0x07E70002, 0x07E70000, { 896, 256 } },
    { 0x08120002, 0x08120000, { 896, 256 } }, { 0x07A50002, 0x07A50000, { 896, 256 } },
    { 0x08190002, 0x08190000, { 896, 256 } }, { 0x078D0003, 0x078D0000, { 896, 256 } },
    { 0x078E0005, 0x078E0000, { 896, 256 } }, { 0x089D0002, 0x089D0000, { 896, 256 } },
    { 0x078F0004, 0x078F0000, { 896, 256 } }, { 0x07DC0002, 0x07DC0000, { 896, 256 } },
    { 0x08130003, 0x08130000, { 896, 256 } }, { 0x08180002, 0x08180000, { 896, 256 } },
    { 0x081B0002, 0x081B0000, { 896, 256 } }, { 0x078A0003, 0x078A0000, { 896, 256 } },
    { 0x07730002, 0x07730000, { 896, 256 } }, { 0x083D0002, 0x083D0000, { 896, 256 } },
    { 0x07E80003, 0x07E80000, { 896, 256 } }, { 0x07DD0003, 0x07DD0000, { 896, 256 } },
    { 0x07DE0002, 0x07DE0000, { 896, 256 } }, { 0x07E90004, 0x07E90000, { 896, 256 } },
    { 0x08010002, 0x08010000, { 896, 256 } }, { 0x07DF0002, 0x07DF0000, { 896, 256 } },
    { 0x08030002, 0x08030000, { 896, 256 } }, { 0x07900001, 0x07900000, { 896, 256 } },
    { 0x08050002, 0x08050000, { 896, 256 } }, { 0x08020003, 0x08020000, { 896, 256 } },
    { 0x08900002, 0x08900000, { 896, 256 } }, { 0x08890002, 0x08890000, { 896, 256 } },
    { 0x088A0004, 0x088A0000, { 896, 256 } }, { 0x08880002, 0x08880000, { 896, 256 } },
    { 0x088B0004, 0x088B0000, { 896, 256 } }, { 0x08100002, 0x08100000, { 896, 256 } },
    { 0x081A0002, 0x081A0000, { 896, 256 } },
};

FightstgEffectEntry fightstg_effects[149] = {
    { 0, 0, 0x079D0000 }, { 1, 0, 0x079D0001 }, { 2, 0, 0x079D0002 }, { 19, 0, 0x079D0003 }, { 20, 0, 0x079D0028 },
    { 21, 0, 0x079D0004 }, { 22, 0, 0x079D0005 }, { 23, 0, 0x079D0006 }, { 24, 0, 0x079D0007 },
    { 25, 0, 0x079D0027 }, { 26, 0, 0x079D0008 }, { 27, 0, 0x079D0009 }, { 28, 0, 0x079D000A },
    { 29, 0, 0x079D000B }, { 31, 0, 0x079D000C }, { 32, 0, 0x079D000D }, { 42, 0, 0x079D0017 },
    { 43, 0, 0x079D0018 }, { 44, 0, 0x079D0019 }, { 45, 0, 0x079D001A }, { 46, 0, 0x079D001B },
    { 47, 0, 0x079D001C }, { 57, 0, 0x079D0024 }, { 58, 0, 0x079D0025 }, { 33, 0, 0x079D000E },
    { 34, 0, 0x079D000F }, { 35, 0, 0x079D0010 }, { 36, 0, 0x079D0011 }, { 37, 0, 0x079D0012 },
    { 38, 0, 0x079D0013 }, { 39, 0, 0x079D0014 }, { 40, 0, 0x079D0015 }, { 41, 0, 0x079D0016 },
    { 50, 0, 0x079D001D }, { 51, 0, 0x079D001E }, { 52, 0, 0x079D001F }, { 53, 0, 0x079D0020 },
    { 54, 0, 0x079D0021 }, { 55, 0, 0x079D0022 }, { 56, 0, 0x079D0023 }, { 13, 1, 0x07A30001 },
    { 14, 1, 0x07A30002 }, { 15, 1, 0x07A30003 }, { 16, 1, 0x07A30004 }, { 17, 1, 0x07A30005 },
    { 18, 1, 0x07A30006 }, { 64, 1, 0x07A30008 }, { 65, 1, 0x07A30009 }, { 3, 2, 0x07E00001 }, { 4, 2, 0x07E00002 },
    { 5, 2, 0x07E00003 }, { 6, 2, 0x07E00004 }, { 7, 2, 0x07E00005 }, { 8, 2, 0x07E00006 }, { 9, 2, 0x07E00007 },
    { 10, 2, 0x07E00008 }, { 11, 2, 0x07E00009 }, { 12, 2, 0x07E0000A }, { 63, 2, 0x07E0000B },
    { 62, 3, 0x07E2000B }, { 1009, 3, 0x07E20009 }, { 1012, 3, 0x07E20003 }, { 1013, 3, 0x07E20004 },
    { 1014, 3, 0x07E20005 }, { 1017, 3, 0x07E20008 }, { 1020, 3, 0x07E2000A }, { 1004, 5, 0x07F50001 },
    { 1005, 5, 0x07F50002 }, { 1006, 5, 0x07F50003 }, { 1000, 6, 0x08A20001 }, { 1001, 6, 0x08A20002 },
    { 1002, 6, 0x08A20003 }, { 1003, 6, 0x08A20004 }, { 1007, 6, 0x08A20006 }, { 1008, 6, 0x08A20007 },
    { 1010, 7, 0x080F0001 }, { 59, 9, 0x080E0001 }, { 60, 9, 0x080E0002 }, { 1018, 9, 0x080E0006 },
    { 1019, 9, 0x080E0007 }, { 2040, 8, 0x078B0001 }, { 2036, 10, 0x07890001 }, { 2037, 10, 0x07890002 },
    { 2058, 11, 0x081C0000 }, { 1021, 12, 0x07E50004 }, { 2027, 12, 0x07E50001 }, { 2028, 12, 0x07E50002 },
    { 2043, 13, 0x06D90001 }, { 2011, 14, 0x07E60001 }, { 2012, 14, 0x07E60002 }, { 1015, 15, 0x08110001 },
    { 2059, 15, 0x08110003 }, { 2060, 16, 0x081D0000 }, { 2056, 17, 0x078C0003 }, { 2031, 17, 0x078C0001 },
    { 2014, 18, 0x07DB0001 }, { 2026, 19, 0x07E70001 }, { 1016, 20, 0x08120001 }, { 2044, 21, 0x07A50001 },
    { 1022, 22, 0x08190001 }, { 2019, 23, 0x078D0001 }, { 2020, 23, 0x078D0002 }, { 2008, 24, 0x078E0002 },
    { 2009, 24, 0x078E0003 }, { 2010, 24, 0x078E0004 }, { 2062, 25, 0x089D0001 }, { 2000, 26, 0x078F0001 },
    { 2001, 26, 0x078F0002 }, { 2002, 26, 0x078F0003 }, { 2004, 27, 0x07DC0001 }, { 2046, 28, 0x08130001 },
    { 2047, 28, 0x08130002 }, { 2061, 29, 0x08180001 }, { 2057, 30, 0x081B0001 }, { 2029, 31, 0x078A0001 },
    { 2030, 31, 0x078A0002 }, { 2032, 32, 0x07730001 }, { 2007, 33, 0x083D0001 }, { 2041, 34, 0x07E80001 },
    { 2042, 34, 0x07E80002 }, { 2024, 35, 0x07DD0001 }, { 2025, 35, 0x07DD0002 }, { 2005, 36, 0x07DE0001 },
    { 2006, 36, 0x07DE0003 }, { 2016, 37, 0x07E90001 }, { 2017, 37, 0x07E90002 }, { 2018, 37, 0x07E90003 },
    { 2038, 38, 0x08010003 }, { 2039, 38, 0x08010004 }, { 2003, 39, 0x07DF0001 }, { 2015, 40, 0x08030001 },
    { 2021, 41, 0x07900002 }, { 2022, 41, 0x07900003 }, { 2023, 41, 0x07900004 }, { 2013, 42, 0x08050001 },
    { 2033, 43, 0x08020001 }, { 2034, 43, 0x08020002 }, { 2035, 44, 0x08900001 }, { 2049, 45, 0x08890001 },
    { 2050, 46, 0x088A0001 }, { 2051, 46, 0x088A0002 }, { 2052, 46, 0x088A0003 }, { 2048, 47, 0x08880001 },
    { 2053, 48, 0x088B0001 }, { 2054, 48, 0x088B0002 }, { 2055, 48, 0x088B0003 }, { 1011, 49, 0x08100001 },
    { 2045, 50, 0x081A0001 }, { -1, 0, 0 },
};
