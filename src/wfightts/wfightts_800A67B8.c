#include "common.h"
#include "object.h"
#include "heap.h"
#include "gfx.h"
#include "cdload.h"
#include "pad.h"
#include "message.h"
#include "sound.h"
#include "gamestate.h"
#include "records.h"
#include "fightstg.h"
#include "wfightts.h"

/* WFIGHTTS: the battle overlay FIGHTSTG loads (file 0x209) when gamestate_data.funcs.get_map_entry() != 0. */

void wfightts_main_update();

extern RECT wfightts_screen_rect; /* the screen: 0, 0, 320, 240 */

/* Sets up the display and the battle's seven layers (0x1000-0x1006); WFIGHTMN's wfightmn_init_layers with
 * 5 instead of 10 for layer 0x1006. */
void wfightts_init_layers(void) {
    GfxLayer *layer;

    gfx_module.reset();
    gfx_module.alloc_packet_buffers(0x19000);
    gfx_module.funcs.init_display(320, 240, 0, 0);
    layer = gfx_module.funcs.create_layer(&wfightts_screen_rect, 1, 0x1000);
    layer->set_draw_offset(layer, 160, 120);
    layer = gfx_module.funcs.create_layer(&wfightts_screen_rect, 1, 0x1001);
    layer->set_draw_offset(layer, 160, 120);
    layer->alloc_callbacks(layer, 100);
    layer = gfx_module.funcs.create_layer(&wfightts_screen_rect, 8, 0x1002);
    layer->set_draw_offset(layer, wfightts_screen_rect.w / 2, wfightts_screen_rect.h / 2);
    layer->alloc_callbacks(layer, 40);
    layer = gfx_module.funcs.create_layer(&wfightts_screen_rect, 1, 0x1003);
    layer->set_draw_offset(layer, 160, 120);
    layer->alloc_callbacks(layer, 100);
    layer = gfx_module.funcs.create_layer(&wfightts_screen_rect, 12, 0x1004);
    layer->set_draw_offset(layer, 160, 120);
    layer->alloc_callbacks(layer, 100);
    layer = gfx_module.funcs.create_layer(&wfightts_screen_rect, 1, 0x1005);
    layer->set_draw_offset(layer, 0, 0);
    layer = gfx_module.funcs.create_layer(&wfightts_screen_rect, 1, 0x1006);
    layer->set_draw_offset(layer, 0, 0);
    layer->alloc_callbacks(layer, 5);
}

/* Loads the battle's images into VRAM (the files WFIGHTMN's wfightmn_loader_update loads one by one). */
void wfightts_load_images(void) {
    Tim tim;

    tim_init(&tim);
    tim.set_image_pos(0x200, 0);
    tim.load_all(cdload_module.get_subfile_by_id(0x4560001));
    tim.set_image_pos(0, 0xF4);
    tim.load(cdload_module.get_subfile_by_id(0x4560000));
    tim.set_image_pos(0x140, 0x100);
    tim.load_all((s32 *)cdload_module.files.get_file(0x79E));
    tim.set_image_pos(0x1C0, 0x100);
    tim.load_all((s32 *)cdload_module.files.get_file(0x7AE));
    tim.set_image_pos(0x200, 0x100);
    tim.load_all((s32 *)cdload_module.files.get_file(0x7E1));
    tim.set_image_pos(0x240, 0x100);
    tim.load_all((s32 *)cdload_module.files.get_file(0x7E3));
}

/* wfightts_main_update's object: a test battle's menus (menu: the last menu, 1..5) and their results. */
typedef struct WfighttsMain {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 menu; /* the menu open (or last open) */
    /* 0x54 */ s32 column; /* the column (side) chosen, -1 = cancelled */
    /* 0x58 */ s32 digimon; /* menu 1: the Digimon */
    /* 0x5C */ s32 camera; /* menu 2: the camera entry */
    /* 0x60 */ s32 stage; /* menu 3: the stage */
    /* 0x64 */ s32 technique; /* menu 4: the technique */
    /* 0x68 */ s32 script; /* menu 5: the script */
} WfighttsMain; /* size 0x6C */

/* Its data block: FIGHTSTG's objects and the menus it creates. */
typedef struct WfighttsMainData {
    /* 0x00 */ Object *action;  /* the action shown (a scene, a script, ...) */
    /* 0x04 */ Object *command;  /* the command menu */
    /* 0x08 */ FightstgCamera *camera;  /* the camera */
    /* 0x0C */ FightstgLighting *lighting;  /* the lighting */
    /* 0x10 */ FightstgStage *stage;  /* the stage */
    /* 0x14 */ WfighttsColumnMenu *digimon_menu;  /* menu 1 */
    /* 0x18 */ WfighttsColumnMenu *camera_menu;  /* menu 2 */
    /* 0x1C */ WfighttsStageMenu *stage_menu;  /* menu 3 */
    /* 0x20 */ WfighttsTechniqueMenu *technique_menu;  /* menu 4 */
    /* 0x24 */ WfighttsScriptMenu *script_menu;  /* menu 5 */
    /* 0x28 */ FightstgSlots *slots;  /* the models */
} WfighttsMainData; /* size 0x2C */

/* Per test battle (gamestate_data.funcs.get_map_entry()): the two Digimon. */
typedef struct WfighttsTestBattle {
    /* 0x0 */ s16 player;
    /* 0x2 */ s16 enemy;
} WfighttsTestBattle; /* size 0x4 */

extern WfighttsTestBattle wfightts_test_battles[7]; /* indexed by the test battle */
extern DVECTOR wfightts_screen_corners[4]; /* the screen's corners */
extern CVECTOR wfightts_corner_colors[4];
extern s32 wfightts_speed;          /* the battle speed (fightstg_battle.set_speed) */
/* wfightts_camera_0: the camera of column 0's entry, with 0xC bytes after it that nothing uses (an unreferenced
 * variable would come last in this file's .bss: GCC outputs referenced tentative definitions first). */
typedef struct WfighttsCamera {
    /* 0x00 */ FightstgCameraSetting camera;
    /* 0x34 */ u8 unk_34[0xC];
} WfighttsCamera; /* size 0x40 */

extern WfighttsCamera wfightts_camera_0;
extern FightstgCameraSetting wfightts_camera_1;  /* camera of column 1's entry */
extern s32 wfightts_display_x;          /* display offset x */
extern s32 wfightts_display_y;          /* display offset y */

Object *fightstg_command_create();
FightstgCamera *fightstg_camera_create(s32 layer);
FightstgStage *fightstg_stage_create(s32 stage, s32 frames);
FightstgSlots *fightstg_slots_create(void);
FightstgLighting *fightstg_lights_create(s32 layer);
void fightstg_command_set_phase(s32 phase);
Object *fightstg_entrance_create(s32 id, s32 enemy, s32 weak);
Object *fightstg_digivolve_create(s32 id, s32 blast);
Object *fightstg_defeat_camera_create(void);
Object *fightstg_player_reaction_create(s32 arg0, s32 arg1);

/* The test battle's main object: sets the battle up (display, sound, images, FIGHTSTG's objects, the two Digimon of
 * test battle gamestate_data.funcs.get_map_entry()), then runs five menus (0x400/0x800 switch between them): the Digimon,
 * a camera entry, the stage, a technique and a script to play; 0x2000 leaves, 0x800/0x200/0x100 on the first screen
 * play an action. 0x1 steps the battle speed; with button 9 held, buttons 7/5/4/6 move the display (3 resets). */
void wfightts_main_update(WfighttsMain *obj, WfighttsMainData *data) {
    FightstgCameraSetting unused; /* unused: the original's frame keeps 0x38 bytes for a local of this size */
    s32 i;
    FightstgModelRecordA *rec;
    FightstgModelRecordB *rec_b;
    s32 pressed;
    s32 draw = 0;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            cdload_module.files.free_all();
            wfightts_init_layers();
            obj->base.next_step(obj);
            /* fallthrough */
        case 1:
            switch (obj->base.substep) {
            case 0:
            default:
                sound_module.load_extra_bank(2);
                obj->base.next_substep(obj);
                /* fallthrough */
            case 1:
                if (sound_module.is_loading() == 0) {
                    sound_module.play(0x60080000);
                    records_state.music = 0x60080000;
                    obj->base.next_step(obj);
                }
                break;
            }
            break;
        case 2:
            wfightts_load_images();
            obj->base.next_step(obj);
            /* fallthrough */
        case 3:
            switch (obj->base.substep) {
            case 0:
            default:
                data->command = fightstg_command_create();
                data->camera = fightstg_camera_create(0x1001);
                data->stage = fightstg_stage_create(records_state.stage, 60);
                data->slots = fightstg_slots_create();
                data->lighting = fightstg_lights_create(0x1001);
                obj->base.next_substep(obj);
                break;
            case 1:
                i = gamestate_data.funcs.get_map_entry();
                data->slots->add(data->slots, 0, wfightts_test_battles[i].player, 1);
                data->slots->reset_pos(data->slots, 0);
                data->slots->add(data->slots, 0x10, wfightts_test_battles[i].enemy, 1);
                data->slots->reset_pos(data->slots, 0x10);
                data->camera->set(data->camera, data->camera->get_default(data->camera));
                fightstg_command_set_phase(0);
                obj->menu = 1;
                obj->base.next_state(obj);
                break;
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
                fightstg_command_set_phase(0);
                obj->base.next_substep(obj);
                /* fallthrough */
            case 1:
                pressed = pad_state.get_pressed(0);
                if (pressed & 0x2000) {
                    fightstg_command_set_phase(0);
                    obj->base.set_step(obj, obj->menu);
                }
                if ((pressed & 0x800) && data->action == NULL) {
                    data->action = fightstg_entrance_create(0x94, 0, 0);
                }
                if ((pressed & 0x200) && data->action == NULL) {
                    data->action = fightstg_digivolve_create(0x3B, 0);
                }
                if ((pressed & 0x100) && data->action == NULL) {
                    data->action = fightstg_defeat_camera_create();
                }
                break;
            }
            break;
        case 1:
            draw = 1;
            switch (obj->base.substep) {
            case 0:
            default:
                obj->menu = 1;
                data->digimon_menu = wfightts_digimon_menu_create(&obj->column, &obj->digimon);
                obj->base.next_substep(obj);
                break;
            case 1:
                if (pad_state.get_pressed(0) & 0x400) {
                    obj->base.set_step(obj, 2);
                    data->digimon_menu->base.set_state(data->digimon_menu, OBJECT_STATE_END);
                } else if (pad_state.get_pressed(0) & 0x800) {
                    obj->base.set_step(obj, 5);
                    data->digimon_menu->base.set_state(data->digimon_menu, OBJECT_STATE_END);
                } else if (data->digimon_menu == NULL) {
                    if (obj->column != -1) {
                        if (obj->column == 0) {
                            data->slots->add(data->slots, 0, obj->digimon, 1);
                            data->slots->reset_pos(data->slots, 0);
                        } else {
                            data->slots->add(data->slots, 0x10, obj->digimon, 1);
                            data->slots->reset_pos(data->slots, 0x10);
                        }
                        data->camera->set(data->camera, data->camera->get_default(data->camera));
                    }
                    obj->base.set_step(obj, 0);
                }
                break;
            }
            break;
        case 2:
            draw = 1;
            switch (obj->base.substep) {
            case 0:
            default:
                obj->menu = 2;
                data->camera_menu = wfightts_camera_menu_create(&obj->column, &obj->camera);
                obj->base.next_substep(obj);
                break;
            case 1:
                if (pad_state.get_pressed(0) & 0x400) {
                    data->camera_menu->base.set_state(data->camera_menu, OBJECT_STATE_END);
                    obj->base.set_step(obj, 3);
                } else if (pad_state.get_pressed(0) & 0x800) {
                    data->camera_menu->base.set_state(data->camera_menu, OBJECT_STATE_END);
                    obj->base.set_step(obj, 1);
                } else if (data->camera_menu == NULL) {
                    if (obj->column != -1) {
                        if (obj->column == 0) {
                            rec = fightstg_models.get(data->slots->get_model_id(data->slots, 0));
                            wfightts_camera_0.camera.eye[0] = rec->camera_eyes[obj->camera][0];
                            wfightts_camera_0.camera.eye[1] = -rec->camera_eyes[obj->camera][1];
                            wfightts_camera_0.camera.eye[2] = -rec->camera_eyes[obj->camera][2];
                            wfightts_camera_0.camera.target[0] = rec->camera_targets[obj->camera][0];
                            wfightts_camera_0.camera.target[1] = -rec->camera_targets[obj->camera][1];
                            wfightts_camera_0.camera.target[2] = -rec->camera_targets[obj->camera][2];
                            wfightts_camera_0.camera.roll = 0;
                            wfightts_camera_0.camera.projection = rec->camera_projections[obj->camera];
                            data->camera->move(data->camera, NULL, &wfightts_camera_0.camera, 60);
                        } else {
                            rec_b = fightstg_models.get(data->slots->get_model_id(data->slots, 0x10));
                            wfightts_camera_1.eye[0] = rec_b->camera_eyes[obj->camera][0];
                            wfightts_camera_1.eye[1] = -rec_b->camera_eyes[obj->camera][1];
                            wfightts_camera_1.eye[2] = -rec_b->camera_eyes[obj->camera][2];
                            wfightts_camera_1.target[0] = rec_b->camera_targets[obj->camera][0];
                            wfightts_camera_1.target[1] = -rec_b->camera_targets[obj->camera][1];
                            wfightts_camera_1.target[2] = -rec_b->camera_targets[obj->camera][2];
                            wfightts_camera_1.roll = 0;
                            wfightts_camera_1.projection = rec_b->camera_projections[obj->camera];
                            data->camera->move(data->camera, NULL, &wfightts_camera_1, 60);
                        }
                    }
                    obj->base.set_step(obj, 0);
                }
                break;
            }
            break;
        case 3:
            draw = 1;
            switch (obj->base.substep) {
            case 0:
            default:
                obj->menu = 3;
                data->stage_menu = wfightts_stage_menu_create(&obj->stage);
                obj->base.next_substep(obj);
                break;
            case 1:
                if (pad_state.get_pressed(0) & 0x400) {
                    data->stage_menu->base.set_state(data->stage_menu, OBJECT_STATE_END);
                    obj->base.set_step(obj, 4);
                } else if (pad_state.get_pressed(0) & 0x800) {
                    data->stage_menu->base.set_state(data->stage_menu, OBJECT_STATE_END);
                    obj->base.set_step(obj, 2);
                } else if (data->stage_menu == NULL) {
                    if (obj->stage != -1) {
                        records_state.stage = obj->stage;
                        data->stage->change(data->stage, obj->stage, 60, 60);
                    }
                    obj->base.set_step(obj, 0);
                }
                break;
            }
            break;
        case 4:
            draw = 1;
            switch (obj->base.substep) {
            case 0:
            default:
                obj->menu = 4;
                data->technique_menu = wfightts_technique_menu_create(&obj->column, &obj->technique);
                obj->base.next_substep(obj);
                break;
            case 1:
                if (pad_state.get_pressed(0) & 0x400) {
                    data->technique_menu->base.set_state(data->technique_menu, OBJECT_STATE_END);
                    obj->base.set_step(obj, 5);
                } else if (pad_state.get_pressed(0) & 0x800) {
                    data->technique_menu->base.set_state(data->technique_menu, OBJECT_STATE_END);
                    obj->base.set_step(obj, 3);
                } else if (data->technique_menu == NULL) {
                    if (obj->column == -1) {
                        obj->base.set_step(obj, 0);
                    } else {
                        data->slots->get_params(data->slots, obj->column != 0 ? 0x10 : 0)->anim = obj->technique;
                        obj->base.set_step(obj, 0);
                    }
                }
                break;
            }
            break;
        case 5:
            draw = 1;
            switch (obj->base.substep) {
            case 0:
            default:
                obj->menu = 5;
                data->script_menu = wfightts_script_menu_create(&obj->column, &obj->script);
                obj->base.next_substep(obj);
                break;
            case 1:
                if (pad_state.get_pressed(0) & 0x400) {
                    data->script_menu->base.set_state(data->script_menu, OBJECT_STATE_END);
                    obj->base.set_step(obj, 1);
                } else if (pad_state.get_pressed(0) & 0x800) {
                    data->script_menu->base.set_state(data->script_menu, OBJECT_STATE_END);
                    obj->base.set_step(obj, 4);
                } else if (data->script_menu == NULL) {
                    if (obj->column != -1) {
                        if (obj->script != 0x12) {
                            data->action = (Object *)fightstg_script_create();
                            ((FightstgScript *)data->action)->script = obj->script;
                            ((FightstgScript *)data->action)->side = obj->column;
                            ((FightstgScript *)data->action)->results[0] = 3;
                            ((FightstgScript *)data->action)->results[1] = 3;
                            ((FightstgScript *)data->action)->results[2] = 3;
                            ((FightstgScript *)data->action)->results[3] = 2;
                            ((FightstgScript *)data->action)->stage = -1;
                            ((FightstgScript *)data->action)->hit_sound = 0x39;
                        } else {
                            data->action = fightstg_player_reaction_create(1, 0);
                        }
                        obj->base.next_substep(obj);
                    } else {
                        obj->base.set_step(obj, 0);
                    }
                }
                break;
            case 2:
                draw = 0;
                if (data->action == NULL) {
                    obj->base.set_step(obj, 0);
                }
                break;
            }
            break;
        }
        if (draw) {
            fightstg_battle.draw_quad_semi(0x1005, 1, wfightts_screen_corners, wfightts_corner_colors);
        }
        if (pad_state.get_pressed(0) & 1) {
            if (++wfightts_speed == 4) {
                wfightts_speed = 0;
            }
            fightstg_battle.set_speed(wfightts_speed);
        }
        if (PAD_HELD(9)) {
            if (PAD_HELD(7)) {
                wfightts_display_x -= 10;
            } else if (PAD_HELD(5)) {
                wfightts_display_x += 10;
            } else if (PAD_HELD(4)) {
                wfightts_display_y -= 10;
            } else if (PAD_HELD(6)) {
                wfightts_display_y += 10;
            } else if (PAD_PRESSED(3)) {
                wfightts_display_y = 0;
                wfightts_display_x = 0;
            }
            gfx_module.funcs.set_display_area(wfightts_display_x, wfightts_display_y, 320, 240);
        } else {
            gfx_module.funcs.init_display(320, 240, 0, 0);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* This file's data: .data, then .bss (the zeros from 0x800A9638). */
RECT wfightts_screen_rect = { 0, 0, 320, 240 };
WfighttsTestBattle wfightts_test_battles[7] = {
    { 378, 441 }, { 366, 115 }, { 259, 202 }, { 366, 436 }, { 145, 197 }, { 31, 137 }, { 151, 108 },
};
DVECTOR wfightts_screen_corners[4] = { { 0, 0 }, { 320, 0 }, { 0, 240 }, { 320, 240 } };
CVECTOR wfightts_corner_colors[4] = { { 128, 128, 128, 0 }, { 128, 128, 128, 0 }, { 128, 128, 128, 0 }, { 128, 128, 128, 0 } };
s32 wfightts_speed = 0;
WfighttsCamera wfightts_camera_0;
FightstgCameraSetting wfightts_camera_1;
s32 wfightts_display_x;
s32 wfightts_display_y;

/* The overlay's entry (FIGHTSTG's fightstg_main_update keeps the object it returns). */
Object *wfightts_main_create(void) {
#ifndef PC_PORT
    return object_create(wfightts_main_update, 0, 0, 0);
#else
    /* PC_PORT: FINDINGS 9a: the original's sizes 0 write the object over the next heap block and its data at 0 */
    return object_create(wfightts_main_update, sizeof(WfighttsMain), sizeof(WfighttsMainData), 0);
#endif
}
