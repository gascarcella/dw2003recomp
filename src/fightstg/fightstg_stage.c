#include "common.h"

#include "cdload.h"
#include "gfx.h"
#include "object.h"
#include "pad.h"
#include "heap.h"
#include "sound.h"
#include "fightstg.h"

extern s32 fightstg_stage_sounds[]; /* sound keys */

void fightstg_stage_update(FightstgStage *obj, FightstgModel **data);
void fightstg_stage_change(FightstgStage *obj, s32 index, s32 fade_out, s32 fade_in);

/* Applies the fade: the model's colour and the background colour of layer 0x1000. */
void fightstg_stage_apply_fade(FightstgStage *obj, FightstgModel **data) {
    CVECTOR color;
    GfxLayer *layer;

    fightstg_math.lerp(&obj->color_from, &obj->color_to, obj->fade_pos, &obj->color);
    color.r = obj->color.vx;
    color.g = obj->color.vy;
    color.b = obj->color.vz;
    (*data)->set_color(*data, 1, &color);
    fightstg_math.lerp(&obj->background_from, &obj->background_to, obj->fade_pos, &obj->background);
    layer = gfx_module.funcs.get_layer(0x1000);
    if (obj->background.vx != 0 || obj->background.vy != 0 || obj->background.vz != 0) {
        layer->set_bg_color(layer, obj->background.vx, obj->background.vy, obj->background.vz);
    } else {
        layer->set_bg_color(layer, 1, 1, 1);
    }
}

/* The stage object: shows the model of record `stage` of file 0x1CB, fading it and the background in
 * (state 1) and out (2). */
void fightstg_stage_update(FightstgStage *obj, FightstgModel **data) {
    FightstgStageRecord *recs = (FightstgStageRecord *)cdload_module.files.get_file(0x1CB);
    FightstgLighting *other;
    GfxLayer *layer;
    CVECTOR black;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        /* fallthrough */
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0: {
            static const FightstgPos pos = { 0x280, 0 };

            obj->params.layers[0].shown = 1;
            obj->params.layers[0].layer_id = 0x1002;
            obj->params.model_id = 0;
            obj->params.layers[0].edges = 0;
            *data = fightstg_model_create(recs[obj->stage].model_file, recs[obj->stage].anim_file,
                                           pos, &obj->params);
            if (recs[obj->stage].sound != -1) {
                obj->sound_key = sound_module.play(fightstg_stage_sounds[recs[obj->stage].sound]);
            }
            other = (FightstgLighting *)heap_objects.find(0x13, -1, -1);
            if (other != NULL) {
                other->fade(other, NULL, other->get_stage(other, obj->stage), obj->fade_in);
            }
            obj->base.next_step(obj);
        }
            /* fallthrough */
        case 1:
            if ((*data)->base.state == OBJECT_STATE_RUN) {
                for (i = 0; i < 8; i++) {
                    if (recs[obj->stage].unclipped_parts[i] == 0) {
                        break;
                    }
                    (*data)->set_part_unclipped(*data, recs[obj->stage].unclipped_parts[i], 1);
                }
                black.r = 0;
                black.g = 0;
                black.b = 0;
                (*data)->set_color(*data, 1, &black);
                layer = gfx_module.funcs.get_layer(0x1000);
                layer->set_bg_color(layer, 1, 1, 1);
                obj->base.next_step(obj);
            }
            break;
        case 2:
            obj->color_to.vz = 0x80;
            obj->color_to.vy = 0x80;
            obj->color_to.vx = 0x80;
            obj->color_from.vz = 0;
            obj->color_from.vy = 0;
            obj->color_from.vx = 0;
            obj->background_to.vx = recs[obj->stage].background[0];
            obj->background_to.vy = recs[obj->stage].background[1];
            obj->background_to.vz = recs[obj->stage].background[2];
            obj->background_from.vz = 0;
            obj->background_from.vy = 0;
            obj->background_from.vx = 0;
            obj->fade_pos = 0;
            obj->fade_step = 0x1000 / obj->fade_in;
            obj->base.next_step(obj);
            /* fallthrough */
        case 3:
            obj->fade_pos += obj->fade_step * gfx_module.funcs.get_frame_ticks();
            if (obj->fade_pos > 0x1000) {
                obj->fade_pos = 0x1000;
            }
            fightstg_stage_apply_fade(obj, data);
            if (obj->fade_pos == 0x1000) {
                obj->base.next_step(obj);
            }
            break;
        case 4:
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        switch (obj->base.step) {
        case 0:
        default:
            cdload_module.queue_file(recs[obj->stage].model_file >> 16);
            obj->color_from.vz = 0x80;
            obj->color_from.vy = 0x80;
            obj->color_from.vx = 0x80;
            obj->color_to.vz = 0;
            obj->color_to.vy = 0;
            obj->color_to.vx = 0;
            obj->background_to.vz = 0;
            obj->background_to.vy = 0;
            obj->background_to.vx = 0;
            obj->background_from.vx = recs[obj->prev_stage].background[0];
            obj->background_from.vy = recs[obj->prev_stage].background[1];
            obj->background_from.vz = recs[obj->prev_stage].background[2];
            obj->fade_pos = 0;
            obj->fade_step = 0x1000 / obj->fade_out;
            obj->base.next_step(obj);
            /* fallthrough */
        case 1:
            obj->fade_pos += obj->fade_step * gfx_module.funcs.get_frame_ticks();
            if (obj->fade_pos > 0x1000) {
                obj->fade_pos = 0x1000;
            }
            fightstg_stage_apply_fade(obj, data);
            if (obj->fade_pos == 0x1000) {
                (*data)->base.set_state(*data, OBJECT_STATE_END);
                obj->base.set_state(obj, OBJECT_STATE_RUN);
                if (recs[obj->prev_stage].sound != -1) {
                    sound_module.key_off(fightstg_stage_sounds[recs[obj->prev_stage].sound], obj->sound_key);
                }
            }
            break;
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

/* Switches to record `index` (fading over arg2/arg3 frames) unless it is the current one. */
void fightstg_stage_change(FightstgStage *obj, s32 index, s32 fade_out, s32 fade_in) {
    if (obj->stage != index) {
        obj->prev_stage = obj->stage;
        obj->stage = index;
        obj->fade_out = fade_out;
        obj->fade_in = fade_in;
        obj->base.set_state(obj, OBJECT_STATE_DONE);
    }
}

/* A random number from 1 to 27. */
s32 fightstg_stage_get_random(void) {
    return pad_random.next() % 27 + 1;
}

s32 fightstg_stage_get_file(s32 index) {
    FightstgStageRecord *recs = (FightstgStageRecord *)cdload_module.files.get_file(0x1CB);

    return (s16)(recs[index].anim_file >> 16);
}

void fightstg_stage_create(s32 stage, s32 fade_in) {
    FightstgStage *obj = object_create(fightstg_stage_update, sizeof(FightstgStage), sizeof(FightstgModel *), 0x15);

    obj->change = fightstg_stage_change;
    obj->stage = stage;
    obj->fade_in = fade_in;
}

/* .data (address order) */

s32 fightstg_stage_sounds[8] = {
    0xA004E03C, 0xA004E0BD, 0xA004E13E, 0xA004E1BF,
    0xA004E240, 0xA004E2C1, 0xA004E342, 0xA004E3C3,
};
