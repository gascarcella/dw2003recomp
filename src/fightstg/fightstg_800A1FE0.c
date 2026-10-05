#include "common.h"

#include "object.h"
#include "fightstg.h"
#include "heap.h"
#include "pad.h"

/* A step of a camera sequence (fightstg_intro_camera_steps): frames, then the camera mode (`mode`). */
typedef struct FightstgIntroCameraStep {
    /* 0x0 */ s16 frames; /* frames; -1 ends the sequence */
    /* 0x2 */ s16 mode;
} FightstgIntroCameraStep; /* size 0x4 */

extern FightstgIntroCameraStep fightstg_intro_camera_steps[][3];

/* fightstg_intro_camera_create's object: plays one of three camera sequences, picked at random. */
typedef struct FightstgIntroCamera {
    /* 0x00 */ Object base;
    /* 0x50 */ FightstgCamera *camera; /* the camera */
    /* 0x54 */ FightstgCameraSetting *setting; /* its setting */
    /* 0x58 */ Object *slots; /* the 0x14 object */
    /* 0x5C */ s16 sequence;
    /* 0x5E */ s16 step;
    /* 0x60 */ s16 start_angle_y;
    /* 0x62 */ s16 start_angle_x;
    /* 0x64 */ u8 unk_64[0x4];
    /* 0x68 */ s32 frames; /* frames left in the step */
    /* 0x6C */ u8 unk_6C[0x4];
    /* 0x70 */ FightstgCameraSetting setting_copy;
} FightstgIntroCamera; /* size 0xA4 */

void fightstg_intro_camera_update(FightstgIntroCamera *obj) {
    FightstgCameraSetting *setting;

    switch (obj->base.state) {
    default:
    case OBJECT_STATE_INIT:
        obj->base.next_state(obj);
        obj->sequence = pad_random.next() & 3;
        if (obj->sequence == 3) {
            obj->sequence = 0;
        }
        obj->step = 0;
        obj->frames = fightstg_intro_camera_steps[obj->sequence][0].frames;
        obj->base.set_step(obj, fightstg_intro_camera_steps[obj->sequence][obj->step].mode);
        obj->camera = (FightstgCamera *)heap_objects.find(0x12, -1, -1);
        obj->slots = heap_objects.find(0x14, -1, -1);
        obj->setting = obj->camera->get_default(obj->camera);
    case OBJECT_STATE_RUN:
        if (obj->frames <= 0) {
            obj->step++;
            if (fightstg_intro_camera_steps[obj->sequence][obj->step].frames == -1) {
                obj->base.set_state(obj, OBJECT_STATE_END);
                return;
            }
            obj->frames = fightstg_intro_camera_steps[obj->sequence][obj->step].frames;
            obj->base.set_step(obj, fightstg_intro_camera_steps[obj->sequence][obj->step].mode);
        }
        switch (obj->base.step) {
        case 1:
            obj->setting = obj->camera->get_preset(obj->camera, 0x10, 0);
            obj->camera->set(obj->camera, obj->setting);
            obj->base.step = 0;
            break;
        case 2:
            obj->setting = obj->camera->get_preset(obj->camera, 0, 8);
            obj->camera->set(obj->camera, obj->setting);
            obj->base.step = 0;
            break;
        case 3:
            obj->setting = obj->camera->get_default(obj->camera);
            obj->camera->set(obj->camera, obj->setting);
            obj->base.step = 0;
            break;
        case 4:
            obj->setting_copy = *obj->camera->get_default(obj->camera);
            obj->camera->move(obj->camera, 0, &obj->setting_copy, obj->frames);
            obj->base.step = 0;
            break;
        case 5:
            obj->setting_copy = *obj->camera->get_preset(obj->camera, 0, 10);
            obj->camera->move(obj->camera, 0, &obj->setting_copy, obj->frames);
            obj->base.step = 0;
            break;
        case 6:
            if (obj->base.substep == 0) {
                obj->setting = obj->camera->get_default(obj->camera);
                obj->start_angle_x = obj->setting->rot[0];
                obj->start_angle_y = obj->setting->rot[1];
                obj->setting->rot[0] = obj->start_angle_x - 0x155;
                obj->setting->rot[1] = obj->start_angle_y + 0x800;
                obj->base.substep++;
            }
            obj->setting->rot[0] = obj->start_angle_x - obj->frames * 0x155 / 64;
            obj->setting->rot[1] = obj->start_angle_y + obj->frames * 32;
            obj->camera->set(obj->camera, obj->setting);
            break;
        }
        obj->frames -= gfx_module.funcs.get_frame_ticks();
        break;
    case OBJECT_STATE_END:
        obj->setting = obj->camera->get_default(obj->camera);
        obj->camera->set(obj->camera, obj->setting);
        break;
    case OBJECT_STATE_DONE:
        break;
    }
}

void fightstg_intro_camera_create(void) {
    object_new(fightstg_intro_camera_update, 0xA4, 0);
}

/* .data (address order) */

FightstgIntroCameraStep fightstg_intro_camera_steps[3][3] = {
    { { 64, 6 }, { 2, 3 }, { -1, 0 } }, { { 32, 1 }, { 32, 4 }, { -1, 0 } }, { { 32, 2 }, { 32, 4 }, { -1, 0 } },
};

/* unreferenced */
s32 D_FIGHTSTG_800A46CC = 0;
