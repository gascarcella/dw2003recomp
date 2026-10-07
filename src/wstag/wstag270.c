#include "wstag.h"
#include "pad.h"

/* WSTAG270: stage 0x211 (fieldstg_stages). */

extern WstagFuncs wstag270_funcs;
extern FieldstgVramPlace wstag270_vram_places[];
extern FieldstgPlacedActor *wstag270_actors[];
extern FieldstgSprite wstag270_sprites[];
extern FieldstgMapEvent wstag270_map_events[];
extern FieldstgEventDef wstag270_events[];
extern void (*D_WSTAG270_800A87A8)(WindowAnim *fade, s32 in);
extern s32 (*D_WSTAG270_800A87AC)(WindowAnim *fade);
void wstag270_choice_1518_update();
void wstag270_choice_1520_update();

void wstag270_choice_1518_update(WstagChoice *obj, WstagChoiceData *data) {
    Sprite spr;
    s32 prev;
    s32 i;
    s32 j;
    s32 k;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->fade.duration = 10;
        for (j = 0; j < 2; j++) {
            data->answers[j] = message_create_window(0x1002, 1, 0x1C, 0xBE + j * 14);
            data->answers[j]->set_ot_depth(data->answers[j], 1);
        }
        data->cursor = message_create_cursor(0x1002, 1, 0x12, 0xBE);
        data->cursor->show(data->cursor, 0);
        data->question = message_create_window(0x1002, 1, 0x12, 0xB0);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            D_WSTAG270_800A87A8(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (D_WSTAG270_800A87AC(&obj->fade) != 0) {
                for (i = 0; i < 2; i++) {
                    data->answers[i]->set_text(data->answers[i], cdload_module.get_subfile_by_id(0x1200005 + (records_language << 16)),
                                              i + 2);
                }
                data->cursor->show(data->cursor, 1);
                data->question->set_text(data->question, cdload_module.get_subfile_by_id(0x1200005 + (records_language << 16)), 1);
                obj->base.step++;
            }
            break;
        case 2:
            prev = obj->cursor;
            if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                if (--obj->cursor < 0) {
                    obj->cursor = 0;
                }
            } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                if (++obj->cursor >= 2) {
                    obj->cursor = 1;
                }
            }
            if (prev != obj->cursor) {
                sound_module.play(0x8004513E);
                data->cursor->set_pos(data->cursor, 0x12, obj->cursor * 14 + 0xBE);
            } else if (PAD_PRESSED(13)) {
                sound_module.play(0x8004503C);
                obj->base.step = 10;
                obj->base.substep = 1;
            }
            break;
        case 3:
            if (obj->cursor == 0) {
                data->event = fieldstg_event_start(0x3B);
            } else {
                data->event = fieldstg_event_start(0x5EF);
            }
            obj->base.step++;
            break;
        case 4:
            if (data->event == NULL) {
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        case 10:
            for (k = 0; k < 2; k++) {
                data->answers[k]->set_visible(data->answers[k], 0);
            }
            data->cursor->show(data->cursor, 0);
            data->question->set_visible(data->question, 0);
            D_WSTAG270_800A87A8(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (D_WSTAG270_800A87AC(&obj->fade) != 0) {
                if (obj->base.substep == 1) {
                    obj->base.step = 3;
                } else {
                    obj->base.state = OBJECT_STATE_END;
                }
            }
            break;
        }
        sprite_init(&spr);
        spr.set_layer_id(0x1002, 2);
        spr.set_vram_pos(0x140, 0);
        spr.set_follow_scroll(0);
        if (obj->fade.level != 0) {
            if (obj->fade.level != 0x1000) {
                spr.set_scale(obj->fade.level, 0x1000, 0x1000);
                spr.set_pivot(0, 0xC3);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x45, 0, 0xAC);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag270_event_1518_start(void) {
    return object_new(wstag270_choice_1518_update, sizeof(WstagChoice), sizeof(WstagChoiceData));
}

void wstag270_choice_1520_update(WstagChoice *obj, WstagChoiceData *data) {
    Sprite spr;
    s32 prev;
    s32 i;
    s32 j;
    s32 k;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->fade.duration = 10;
        for (j = 0; j < 2; j++) {
            data->answers[j] = message_create_window(0x1002, 1, 0x1C, 0xBE + j * 14);
            data->answers[j]->set_ot_depth(data->answers[j], 1);
        }
        data->cursor = message_create_cursor(0x1002, 1, 0x12, 0xBE);
        data->cursor->show(data->cursor, 0);
        data->question = message_create_window(0x1002, 1, 0x12, 0xB0);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            D_WSTAG270_800A87A8(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (D_WSTAG270_800A87AC(&obj->fade) != 0) {
                for (i = 0; i < 2; i++) {
                    data->answers[i]->set_text(data->answers[i], cdload_module.get_subfile_by_id(0x1200006 + (records_language << 16)),
                                              i + 2);
                }
                data->cursor->show(data->cursor, 1);
                data->question->set_text(data->question, cdload_module.get_subfile_by_id(0x1200006 + (records_language << 16)), 1);
                obj->base.step++;
            }
            break;
        case 2:
            prev = obj->cursor;
            if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                if (--obj->cursor < 0) {
                    obj->cursor = 0;
                }
            } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                if (++obj->cursor >= 2) {
                    obj->cursor = 1;
                }
            }
            if (prev != obj->cursor) {
                sound_module.play(0x8004513E);
                data->cursor->set_pos(data->cursor, 0x12, obj->cursor * 14 + 0xBE);
            } else if (PAD_PRESSED(13)) {
                sound_module.play(0x8004503C);
                obj->base.step = 10;
                obj->base.substep = 1;
            }
            break;
        case 3:
            if (obj->cursor == 0) {
                data->event = fieldstg_event_start(0x41);
            } else {
                data->event = fieldstg_event_start(0x5F1);
            }
            obj->base.step++;
            break;
        case 4:
            if (data->event == NULL) {
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        case 10:
            for (k = 0; k < 2; k++) {
                data->answers[k]->set_visible(data->answers[k], 0);
            }
            data->cursor->show(data->cursor, 0);
            data->question->set_visible(data->question, 0);
            D_WSTAG270_800A87A8(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (D_WSTAG270_800A87AC(&obj->fade) != 0) {
                if (obj->base.substep == 1) {
                    obj->base.step = 3;
                } else {
                    obj->base.state = OBJECT_STATE_END;
                }
            }
            break;
        }
        sprite_init(&spr);
        spr.set_layer_id(0x1002, 2);
        spr.set_vram_pos(0x140, 0);
        spr.set_follow_scroll(0);
        if (obj->fade.level != 0) {
            if (obj->fade.level != 0x1000) {
                spr.set_scale(obj->fade.level, 0x1000, 0x1000);
                spr.set_pivot(0, 0xC3);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x45, 0, 0xAC);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag270_event_1520_start(void) {
    return object_new(wstag270_choice_1520_update, sizeof(WstagChoice), sizeof(WstagChoiceData));
}

void wstag270_update(WstagObject *obj) {
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

WstagObject *wstag270_start(void *arg0) {
    WstagObject *obj = object_new(wstag270_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag270_funcs.setup();
    return obj;
}

void wstag270_event_1415_end(void) {
    gamestate_flags.set_flag(0x8192, 1);
    gamestate_flags.set_flag(0x1A1C, 1);
    gamestate_flags.set_flag(0x40A1, 1);
}

void wstag270_setup(void) {
    fieldstg_stage.background_file = 0x1AE;
    fieldstg_stage.sprite_file = 0x01AF0000;
    fieldstg_stage.sprites = wstag270_sprites;
    fieldstg_stage.map_events = wstag270_map_events;
    fieldstg_stage.mask_file = 0x3D2;
    fieldstg_stage.talk_file = records_language + 0xC5;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1A400, 0xC800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag270_vram_places;
    fieldstg_stage.music = 8;
    fieldstg_stage.sound = 0x60200000;
    fieldstg_stage.actors = wstag270_actors;
    fieldstg_stage.events = wstag270_events;
    fieldstg_attr.set_file(0, 0x01AF0001);
    fieldstg_attr.set_file(7, 0x01AF0002);
    fieldstg_attr.init_layer(0);
}

void wstag270_fade_start(WindowAnim *fade, s32 in) {
    fade->running = 1;
    if (in) {
        sound_module.play(0x40019);
        fade->step = 0x1000 / fade->duration;
        fade->level = 0;
    } else {
        sound_module.play(0x4001A);
        fade->level = 0x1000;
        fade->step = -(0x1000 / fade->duration * 2);
    }
}

s32 wstag270_fade_update(WindowAnim *fade) {
    if (fade->running == 0) {
        return 1;
    }
    fade->level += fade->step;
    if (fade->step > 0) {
        if (fade->level > 0x1000) {
            fade->level = 0x1000;
            fade->running = 0;
            return 1;
        }
    } else if (fade->level < 0) {
        fade->level = 0;
        fade->running = 0;
        return 1;
    }
    return 0;
}

/* The stage's .data (tools/wstag_data.py). */
void wstag270_setup(void);

s16 D_WSTAG270_800A6D08[84] = {
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 51, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 824, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 51, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 825, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 51, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x104, /* padding, not read */
};
s16 D_WSTAG270_800A6DB0[78] = {
    FIELDSTG_EVENT_WALK(2, 279, 189, 3),
    FIELDSTG_EVENT_ANIM(45, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 824, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 45, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 825, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG270_800A6E4C[354] = {
    FIELDSTG_EVENT_WALK(2, 411, 163, 3),
    FIELDSTG_EVENT_ANIM(47, 1, 7),
    FIELDSTG_EVENT_ANIM(50, 1, 7),
    FIELDSTG_EVENT_ANIM(52, 1, 3),
    FIELDSTG_EVENT_ANIM(55, 1, 3),
    FIELDSTG_EVENT_ANIM(206, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_DIALOG(0, 1, 206, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 206, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 206),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 206),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 206, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 208, 225),
    FIELDSTG_EVENT_WAIT(150),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 52),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 52),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 52, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 52, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 47, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 52, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 47, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(1, 12, 47, 2),
    FIELDSTG_EVENT_DIALOG(0, 11, 52, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 52),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 52),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 13, 52, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 14, 47, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 807, 52),
    FIELDSTG_EVENT_WAIT(180),
    FIELDSTG_EVENT_DIALOG(0, 15, 50, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 806, 52),
    FIELDSTG_EVENT_ANIM(0x324, 805, 55),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x324, 806, 55),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 16, 55, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 17, 50, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 18, 55, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 807, 55),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 19, 50, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 55),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 20, 55, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 21, 50, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_DIALOG(0, 22, 206, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 23, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 24, 206, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 25, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 26, 206, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG270_800A7110[11] = {
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 51, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG270_800A7128[32] = {
    FIELDSTG_EVENT_WALK(2, 279, 189, 3),
    FIELDSTG_EVENT_ANIM(45, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag270_vram_places[22] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 440, 345, 480, 89, 352, 507 }, { 384, 256, 432, 345, 448, 89, 368, 507 },
    { 384, 256, 416, 349, 384, 93, 336, 506 }, { 384, 256, 424, 349, 416, 93, 352, 506 },
    { 384, 256, 400, 369, 320, 113, 368, 506 }, { 384, 256, 408, 369, 352, 113, 320, 505 },
    { 320, 256, 320, 472, 0, 216, 336, 505 }, { 384, 256, 384, 370, 256, 114, 352, 505 },
    { 384, 256, 392, 370, 288, 114, 368, 505 }, { 384, 256, 432, 385, 448, 129, 320, 504 },
    { 384, 256, 416, 389, 384, 133, 336, 504 }, { 384, 256, 424, 389, 416, 133, 352, 504 },
    { 384, 256, 436, 256, 464, 0, 368, 504 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG270_800A72C8[4] = { 0x901C, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A72D0[4] = { 0x901C, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A72D8[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A72E0[8] = { 0x8192, 1, 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A72F0[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A72FC[8] = { 0x8192, 1, 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A730C[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7314[8] = { 2, 0, 0x8192, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7324[4] = { 2, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A732C[10] = {
    2, 1, 0x7200, 0, 0x8192, 1, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG270_800A7340[12] = {
    0x7204, 0, 0x7200, 1, 2, 1, 0x8192, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG270_800A7358[4] = { 0x7603, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7360[12] = {
    2, 1, 0x7200, 1, 0x7204, 1, 0x8192, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG270_800A7378[4] = { 0x7803, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7380[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7388[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7394[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A739C[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A73A8[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A73B4[4] = { 2, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A73BC[4] = { 2, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A73C4[6] = { 2, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A73D0[8] = { 2, 1, 0x7200, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A73E0[4] = { 0x7603, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A73E8[8] = { 2, 1, 0x7200, 1, 0x7204, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A73F8[4] = { 0x7803, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7400[4] = { 2, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7408[4] = { 2, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7410[6] = { 2, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A741C[8] = { 2, 1, 0x7200, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A742C[4] = { 0x7603, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7434[8] = { 2, 1, 0x7200, 1, 0x7204, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7444[4] = { 0x7803, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A744C[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7454[8] = { 0x8192, 1, 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7464[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7470[8] = { 0x8192, 1, 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7480[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7488[8] = { 3, 0, 0x8192, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7498[4] = { 3, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A74A0[10] = {
    3, 1, 0x7200, 0, 0x8192, 1, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG270_800A74B4[12] = {
    3, 1, 0x7200, 1, 0x7204, 0, 0x8192, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG270_800A74CC[4] = { 0x7604, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A74D4[12] = {
    3, 1, 0x7200, 1, 0x7204, 1, 0x8192, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG270_800A74EC[4] = { 0x7804, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A74F4[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A74FC[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7508[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7510[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A751C[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7528[4] = { 3, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7530[4] = { 3, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7538[6] = { 0x7200, 0, 3, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7544[8] = { 3, 1, 0x7200, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7554[4] = { 0x7604, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A755C[8] = { 0x7204, 1, 3, 1, 0x7200, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A756C[4] = { 0x7804, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7574[4] = { 3, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A757C[4] = { 3, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7584[6] = { 3, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7590[8] = { 3, 1, 0x7200, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A75A0[4] = { 0x7604, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A75A8[8] = { 3, 1, 0x7200, 1, 0x7204, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A75B8[4] = { 0x7804, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A75C0[4] = { 0x901B, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A75C8[4] = { 0x901B, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A75D0[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A75D8[8] = { 0x8192, 1, 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A75E8[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A75F4[8] = { 0x8192, 1, 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7604[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A760C[8] = { 0, 0, 0x8192, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A761C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7624[10] = {
    0, 1, 0x7200, 0, 0x8192, 1, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG270_800A7638[12] = {
    0, 1, 0x7204, 0, 0x7200, 1, 0x8192, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG270_800A7650[4] = { 0x7601, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7658[12] = {
    0x7200, 1, 0, 1, 0x7204, 1, 0x8192, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG270_800A7670[4] = { 0x7801, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7678[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7680[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A768C[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7694[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A76A0[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A76AC[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A76B4[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A76BC[6] = { 0, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A76C8[8] = { 0, 1, 0x7200, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A76D8[4] = { 0x7601, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A76E0[8] = { 0, 1, 0x7200, 1, 0x7204, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A76F0[4] = { 0x7801, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A76F8[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7700[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7708[6] = { 0, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7714[8] = { 0, 1, 0x7204, 0, 0x7200, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7724[4] = { 0x7601, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A772C[8] = { 0, 1, 0x7200, 1, 0x7204, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A773C[4] = { 0x7801, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7744[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A774C[8] = { 0x8192, 1, 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A775C[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7768[8] = { 0x8192, 1, 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7778[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7780[8] = { 1, 0, 0x8192, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7790[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7798[10] = {
    1, 1, 0x7200, 0, 0x8192, 1, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG270_800A77AC[12] = {
    1, 1, 0x7200, 1, 0x7204, 0, 0x8192, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG270_800A77C4[4] = { 0x7602, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A77CC[12] = {
    1, 1, 0x7200, 1, 0x7204, 1, 0x8192, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG270_800A77E4[4] = { 0x7802, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A77EC[4] = { 1, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A77F4[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A77FC[6] = { 1, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7808[8] = { 1, 1, 0x7200, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7818[4] = { 0x7602, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7820[8] = { 1, 1, 0x7200, 1, 0x7204, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7830[4] = { 0x7802, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7838[4] = { 1, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7840[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7848[6] = { 1, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7854[8] = { 1, 1, 0x7200, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7864[4] = { 0x7602, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A786C[8] = { 1, 1, 0x7204, 1, 0x7200, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A787C[4] = { 0x7802, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7884[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A788C[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7898[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A78A0[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A78AC[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A78B8[4] = { 2, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A78C0[6] = { 2, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A78CC[8] = { 2, 1, 0x7200, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A78DC[4] = { 0x7603, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A78E4[8] = { 2, 1, 0x7200, 1, 0x7204, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A78F4[4] = { 0x7803, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A78FC[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7904[6] = { 0, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7910[8] = { 0, 1, 0x7200, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7920[4] = { 0x7601, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7928[8] = { 0, 1, 0x7200, 1, 0x7204, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7938[4] = { 0x7801, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7940[4] = { 3, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7948[6] = { 3, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7954[8] = { 3, 1, 0x7200, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7964[4] = { 0x7604, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A796C[8] = { 3, 1, 0x7200, 1, 0x7204, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A797C[4] = { 0x7804, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7984[4] = { 1, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A798C[6] = { 1, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7998[8] = { 1, 1, 0x7200, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A79A8[4] = { 0x7602, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A79B0[8] = { 1, 1, 0x7200, 1, 0x7204, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A79C0[4] = { 0x7802, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A79C8[6] = { 0x6004, 1, 0x1A1C, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A79D4[4] = { 0x1A1C, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A79DC[6] = { 0x6004, 1, 0x1A1C, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A79E8[4] = { 0x7A31, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A79F0[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A79F8[4] = { 0x7A31, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A00[4] = { 0x703E, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A08[4] = { 0x7A32, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A10[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A18[4] = { 0x7A33, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A20[4] = { 0x7020, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A28[4] = { 0x7A34, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A30[6] = { 0x7021, 1, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A3C[4] = { 0x7A35, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A44[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A4C[4] = { 0x7A36, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A54[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A5C[4] = { 0x7A36, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A64[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A6C[4] = { 0x7A36, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A74[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A7C[6] = { 0x1C48, 1, 0x905C, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A7A88[4] = { 0x8192, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG270_800A7A90[2] = { { NULL, D_WSTAG270_800A72C8, 584 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG270_800A7AA8[2] = { { NULL, D_WSTAG270_800A72D0, 49 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG270_800A7AC0[8] = {
    { D_WSTAG270_800A72D8, NULL, 1053 }, { D_WSTAG270_800A72E0, D_WSTAG270_800A72F0, 151 },
    { D_WSTAG270_800A72FC, D_WSTAG270_800A730C, 150 }, { D_WSTAG270_800A7314, D_WSTAG270_800A7324, 122 },
    { D_WSTAG270_800A732C, NULL, 124 }, { D_WSTAG270_800A7340, D_WSTAG270_800A7358, 125 },
    { D_WSTAG270_800A7360, D_WSTAG270_800A7378, 126 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7B20[4] = {
    { D_WSTAG270_800A7380, NULL, 122 }, { D_WSTAG270_800A7388, D_WSTAG270_800A7394, 150 },
    { D_WSTAG270_800A739C, D_WSTAG270_800A73A8, 151 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7B50[5] = {
    { D_WSTAG270_800A73B4, D_WSTAG270_800A73BC, 123 }, { D_WSTAG270_800A73C4, NULL, 124 },
    { D_WSTAG270_800A73D0, D_WSTAG270_800A73E0, 125 }, { D_WSTAG270_800A73E8, D_WSTAG270_800A73F8, 126 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7B8C[5] = {
    { D_WSTAG270_800A7400, D_WSTAG270_800A7408, 147 }, { D_WSTAG270_800A7410, NULL, 124 },
    { D_WSTAG270_800A741C, D_WSTAG270_800A742C, 125 }, { D_WSTAG270_800A7434, D_WSTAG270_800A7444, 126 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7BC8[2] = { { NULL, NULL, 1053 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG270_800A7BE0[8] = {
    { D_WSTAG270_800A744C, NULL, 1054 }, { D_WSTAG270_800A7454, D_WSTAG270_800A7464, 161 },
    { D_WSTAG270_800A7470, D_WSTAG270_800A7480, 160 }, { D_WSTAG270_800A7488, D_WSTAG270_800A7498, 152 },
    { D_WSTAG270_800A74A0, NULL, 157 }, { D_WSTAG270_800A74B4, D_WSTAG270_800A74CC, 158 },
    { D_WSTAG270_800A74D4, D_WSTAG270_800A74EC, 159 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7C40[4] = {
    { D_WSTAG270_800A74F4, NULL, 152 }, { D_WSTAG270_800A74FC, D_WSTAG270_800A7508, 160 },
    { D_WSTAG270_800A7510, D_WSTAG270_800A751C, 161 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7C70[5] = {
    { D_WSTAG270_800A7528, D_WSTAG270_800A7530, 153 }, { D_WSTAG270_800A7538, NULL, 157 },
    { D_WSTAG270_800A7544, D_WSTAG270_800A7554, 158 }, { D_WSTAG270_800A755C, D_WSTAG270_800A756C, 159 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7CAC[5] = {
    { D_WSTAG270_800A7574, D_WSTAG270_800A757C, 154 }, { D_WSTAG270_800A7584, NULL, 157 },
    { D_WSTAG270_800A7590, D_WSTAG270_800A75A0, 158 }, { D_WSTAG270_800A75A8, D_WSTAG270_800A75B8, 159 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7CE8[2] = { { NULL, NULL, 1054 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG270_800A7D00[2] = { { NULL, D_WSTAG270_800A75C0, 48 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG270_800A7D18[2] = { { NULL, D_WSTAG270_800A75C8, 48 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG270_800A7D30[8] = {
    { D_WSTAG270_800A75D0, NULL, 1051 }, { D_WSTAG270_800A75D8, D_WSTAG270_800A75E8, 136 },
    { D_WSTAG270_800A75F4, D_WSTAG270_800A7604, 135 }, { D_WSTAG270_800A760C, D_WSTAG270_800A761C, 112 },
    { D_WSTAG270_800A7624, NULL, 114 }, { D_WSTAG270_800A7638, D_WSTAG270_800A7650, 115 },
    { D_WSTAG270_800A7658, D_WSTAG270_800A7670, 116 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7D90[4] = {
    { D_WSTAG270_800A7678, NULL, 112 }, { D_WSTAG270_800A7680, D_WSTAG270_800A768C, 135 },
    { D_WSTAG270_800A7694, D_WSTAG270_800A76A0, 136 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7DC0[5] = {
    { D_WSTAG270_800A76AC, D_WSTAG270_800A76B4, 113 }, { D_WSTAG270_800A76BC, NULL, 114 },
    { D_WSTAG270_800A76C8, D_WSTAG270_800A76D8, 115 }, { D_WSTAG270_800A76E0, D_WSTAG270_800A76F0, 116 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7DFC[5] = {
    { D_WSTAG270_800A76F8, D_WSTAG270_800A7700, 132 }, { D_WSTAG270_800A7708, NULL, 114 },
    { D_WSTAG270_800A7714, D_WSTAG270_800A7724, 115 }, { D_WSTAG270_800A772C, D_WSTAG270_800A773C, 116 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7E38[2] = { { NULL, NULL, 1051 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG270_800A7E50[8] = {
    { D_WSTAG270_800A7744, NULL, 1052 }, { D_WSTAG270_800A774C, D_WSTAG270_800A775C, 146 },
    { D_WSTAG270_800A7768, D_WSTAG270_800A7778, 145 }, { D_WSTAG270_800A7780, D_WSTAG270_800A7790, 117 },
    { D_WSTAG270_800A7798, NULL, 119 }, { D_WSTAG270_800A77AC, D_WSTAG270_800A77C4, 120 },
    { D_WSTAG270_800A77CC, D_WSTAG270_800A77E4, 121 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7EB0[5] = {
    { D_WSTAG270_800A77EC, D_WSTAG270_800A77F4, 118 }, { D_WSTAG270_800A77FC, NULL, 119 },
    { D_WSTAG270_800A7808, D_WSTAG270_800A7818, 120 }, { D_WSTAG270_800A7820, D_WSTAG270_800A7830, 121 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7EEC[5] = {
    { D_WSTAG270_800A7838, D_WSTAG270_800A7840, 141 }, { D_WSTAG270_800A7848, NULL, 119 },
    { D_WSTAG270_800A7854, D_WSTAG270_800A7864, 120 }, { D_WSTAG270_800A786C, D_WSTAG270_800A787C, 121 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7F28[4] = {
    { D_WSTAG270_800A7884, NULL, 117 }, { D_WSTAG270_800A788C, D_WSTAG270_800A7898, 145 },
    { D_WSTAG270_800A78A0, D_WSTAG270_800A78AC, 146 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7F58[2] = { { NULL, NULL, 1052 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG270_800A7F70[2] = { { NULL, NULL, 593 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG270_800A7F88[5] = {
    { D_WSTAG270_800A78B8, NULL, 148 }, { D_WSTAG270_800A78C0, NULL, 124 },
    { D_WSTAG270_800A78CC, D_WSTAG270_800A78DC, 125 }, { D_WSTAG270_800A78E4, D_WSTAG270_800A78F4, 126 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A7FC4[5] = {
    { D_WSTAG270_800A78FC, NULL, 133 }, { D_WSTAG270_800A7904, NULL, 114 },
    { D_WSTAG270_800A7910, D_WSTAG270_800A7920, 115 }, { D_WSTAG270_800A7928, D_WSTAG270_800A7938, 116 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A8000[5] = {
    { D_WSTAG270_800A7940, NULL, 155 }, { D_WSTAG270_800A7948, NULL, 157 },
    { D_WSTAG270_800A7954, D_WSTAG270_800A7964, 158 }, { D_WSTAG270_800A796C, D_WSTAG270_800A797C, 159 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A803C[5] = {
    { D_WSTAG270_800A7984, NULL, 143 }, { D_WSTAG270_800A798C, NULL, 119 },
    { D_WSTAG270_800A7998, D_WSTAG270_800A79A8, 120 }, { D_WSTAG270_800A79B0, D_WSTAG270_800A79C0, 121 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A8078[2] = { { NULL, NULL, 583 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG270_800A8090[11] = {
    { D_WSTAG270_800A79C8, D_WSTAG270_800A79D4, 47 }, { D_WSTAG270_800A79DC, D_WSTAG270_800A79E8, 677 },
    { D_WSTAG270_800A79F0, D_WSTAG270_800A79F8, 677 }, { D_WSTAG270_800A7A00, D_WSTAG270_800A7A08, 677 },
    { D_WSTAG270_800A7A10, D_WSTAG270_800A7A18, 677 }, { D_WSTAG270_800A7A20, D_WSTAG270_800A7A28, 677 },
    { D_WSTAG270_800A7A30, D_WSTAG270_800A7A3C, 677 }, { D_WSTAG270_800A7A44, D_WSTAG270_800A7A4C, 677 },
    { D_WSTAG270_800A7A54, D_WSTAG270_800A7A5C, 677 }, { D_WSTAG270_800A7A64, D_WSTAG270_800A7A6C, 677 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A8114[3] = {
    { D_WSTAG270_800A7A74, D_WSTAG270_800A7A7C, 99 }, { D_WSTAG270_800A7A88, NULL, 100 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG270_800A8138[2] = { { NULL, NULL, 1186 }, { NULL, NULL, 0 } };
u16 D_WSTAG270_800A8150[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8158[4] = { 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8160[4] = { 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8168[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8174[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8180[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A818C[6] = { 0x8192, 0, 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8198[4] = { 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A81A0[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A81AC[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A81B8[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A81C4[6] = { 0x8192, 0, 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A81D0[4] = { 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A81D8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A81E0[4] = { 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A81E8[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A81F4[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8200[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A820C[6] = { 0x8192, 0, 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8218[4] = { 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8220[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A822C[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8238[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8244[6] = { 0x8192, 0, 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8250[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8258[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8260[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8268[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8270[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8278[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A8280[6] = { 0x8192, 1, 0x7009, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A828C[8] = { 0x8192, 0, 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG270_800A829C[6] = { 0x8192, 0, 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A82A8[4] = { 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG270_800A82B0[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG270_800A82B8 = { D_WSTAG270_800A8150, D_WSTAG270_800A7A90, 45, 4, 255, 176, 3 };
FieldstgPlacedActor D_WSTAG270_800A82CC = { D_WSTAG270_800A8158, D_WSTAG270_800A7AA8, 45, 4, 255, 176, 3 };
FieldstgPlacedActor D_WSTAG270_800A82E0 = { D_WSTAG270_800A8160, D_WSTAG270_800A7AC0, 47, 5, 198, 219, 7 };
FieldstgPlacedActor D_WSTAG270_800A82F4 = { D_WSTAG270_800A8168, D_WSTAG270_800A7B20, 47, 5, 198, 219, 7 };
FieldstgPlacedActor D_WSTAG270_800A8308 = { D_WSTAG270_800A8174, D_WSTAG270_800A7B50, 47, 5, 198, 219, 7 };
FieldstgPlacedActor D_WSTAG270_800A831C = { D_WSTAG270_800A8180, D_WSTAG270_800A7B8C, 47, 5, 198, 219, 7 };
FieldstgPlacedActor D_WSTAG270_800A8330 = { D_WSTAG270_800A818C, D_WSTAG270_800A7BC8, 47, 5, 198, 219, 7 };
FieldstgPlacedActor D_WSTAG270_800A8344 = { D_WSTAG270_800A8198, D_WSTAG270_800A7BE0, 50, 6, 173, 231, 7 };
FieldstgPlacedActor D_WSTAG270_800A8358 = { D_WSTAG270_800A81A0, D_WSTAG270_800A7C40, 50, 6, 173, 231, 7 };
FieldstgPlacedActor D_WSTAG270_800A836C = { D_WSTAG270_800A81AC, D_WSTAG270_800A7C70, 50, 6, 173, 231, 7 };
FieldstgPlacedActor D_WSTAG270_800A8380 = { D_WSTAG270_800A81B8, D_WSTAG270_800A7CAC, 50, 6, 173, 231, 7 };
FieldstgPlacedActor D_WSTAG270_800A8394 = { D_WSTAG270_800A81C4, D_WSTAG270_800A7CE8, 50, 6, 173, 231, 7 };
FieldstgPlacedActor D_WSTAG270_800A83A8 = { D_WSTAG270_800A81D0, D_WSTAG270_800A7D00, 51, 7, 348, 147, 1 };
FieldstgPlacedActor D_WSTAG270_800A83BC = { D_WSTAG270_800A81D8, D_WSTAG270_800A7D18, 51, 7, 348, 147, 1 };
FieldstgPlacedActor D_WSTAG270_800A83D0 = { D_WSTAG270_800A81E0, D_WSTAG270_800A7D30, 52, 8, 250, 244, 3 };
FieldstgPlacedActor D_WSTAG270_800A83E4 = { D_WSTAG270_800A81E8, D_WSTAG270_800A7D90, 52, 8, 250, 244, 3 };
FieldstgPlacedActor D_WSTAG270_800A83F8 = { D_WSTAG270_800A81F4, D_WSTAG270_800A7DC0, 52, 8, 250, 244, 3 };
FieldstgPlacedActor D_WSTAG270_800A840C = { D_WSTAG270_800A8200, D_WSTAG270_800A7DFC, 52, 8, 250, 244, 3 };
FieldstgPlacedActor D_WSTAG270_800A8420 = { D_WSTAG270_800A820C, D_WSTAG270_800A7E38, 52, 8, 250, 244, 3 };
FieldstgPlacedActor D_WSTAG270_800A8434 = { D_WSTAG270_800A8218, D_WSTAG270_800A7E50, 55, 9, 224, 257, 3 };
FieldstgPlacedActor D_WSTAG270_800A8448 = { D_WSTAG270_800A8220, D_WSTAG270_800A7EB0, 55, 9, 224, 257, 3 };
FieldstgPlacedActor D_WSTAG270_800A845C = { D_WSTAG270_800A822C, D_WSTAG270_800A7EEC, 55, 9, 224, 257, 3 };
FieldstgPlacedActor D_WSTAG270_800A8470 = { D_WSTAG270_800A8238, D_WSTAG270_800A7F28, 55, 9, 224, 257, 3 };
FieldstgPlacedActor D_WSTAG270_800A8484 = { D_WSTAG270_800A8244, D_WSTAG270_800A7F58, 55, 9, 224, 257, 3 };
FieldstgPlacedActor D_WSTAG270_800A8498 = { D_WSTAG270_800A8250, D_WSTAG270_800A7F70, 157, 10, 348, 147, 1 };
FieldstgPlacedActor D_WSTAG270_800A84AC = { D_WSTAG270_800A8258, D_WSTAG270_800A7F88, 158, 11, 198, 219, 7 };
FieldstgPlacedActor D_WSTAG270_800A84C0 = { D_WSTAG270_800A8260, D_WSTAG270_800A7FC4, 159, 12, 250, 244, 3 };
FieldstgPlacedActor D_WSTAG270_800A84D4 = { D_WSTAG270_800A8268, D_WSTAG270_800A8000, 160, 13, 173, 231, 7 };
FieldstgPlacedActor D_WSTAG270_800A84E8 = { D_WSTAG270_800A8270, D_WSTAG270_800A803C, 161, 14, 224, 257, 3 };
FieldstgPlacedActor D_WSTAG270_800A84FC = { D_WSTAG270_800A8278, D_WSTAG270_800A8078, 162, 15, 255, 176, 3 };
FieldstgPlacedActor D_WSTAG270_800A8510 = { D_WSTAG270_800A8280, D_WSTAG270_800A8090, 206, 16, 379, 147, 7 };
FieldstgPlacedActor D_WSTAG270_800A8524 = { D_WSTAG270_800A828C, D_WSTAG270_800A8114, 206, 16, 379, 147, 7 };
FieldstgPlacedActor D_WSTAG270_800A8538 = { D_WSTAG270_800A829C, D_WSTAG270_800A8138, 206, 16, 379, 147, 7 };
FieldstgPlacedActor D_WSTAG270_800A854C = { NULL, NULL, 220, 17, 379, 163, 7 };
FieldstgPlacedActor D_WSTAG270_800A8560 = { D_WSTAG270_800A82A8, NULL, 225, 18, 347, 158, 1 };
FieldstgPlacedActor D_WSTAG270_800A8574 = { D_WSTAG270_800A82B0, NULL, 270, 19, 347, 158, 1 };
FieldstgPlacedActor *wstag270_actors[37] = {
    &D_WSTAG270_800A82B8, &D_WSTAG270_800A82CC, &D_WSTAG270_800A82E0, &D_WSTAG270_800A82F4, &D_WSTAG270_800A8308,
    &D_WSTAG270_800A831C, &D_WSTAG270_800A8330, &D_WSTAG270_800A8344, &D_WSTAG270_800A8358, &D_WSTAG270_800A836C,
    &D_WSTAG270_800A8380, &D_WSTAG270_800A8394, &D_WSTAG270_800A83A8, &D_WSTAG270_800A83BC, &D_WSTAG270_800A83D0,
    &D_WSTAG270_800A83E4, &D_WSTAG270_800A83F8, &D_WSTAG270_800A840C, &D_WSTAG270_800A8420, &D_WSTAG270_800A8434,
    &D_WSTAG270_800A8448, &D_WSTAG270_800A845C, &D_WSTAG270_800A8470, &D_WSTAG270_800A8484, &D_WSTAG270_800A8498,
    &D_WSTAG270_800A84AC, &D_WSTAG270_800A84C0, &D_WSTAG270_800A84D4, &D_WSTAG270_800A84E8, &D_WSTAG270_800A84FC,
    &D_WSTAG270_800A8510, &D_WSTAG270_800A8524, &D_WSTAG270_800A8538, &D_WSTAG270_800A854C, &D_WSTAG270_800A8560,
    &D_WSTAG270_800A8574, NULL,
};
FieldstgSprite wstag270_sprites[19] = {
    { 1, 0, 0x40, 6, 0x3B, 2, 0, 5, 8, 0, 428, 75, 0, 0 }, { 1, 0, 0x40, 6, 0x3D, 2, 0, 3, 8, 0, 65, 176, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 8, 0, 425, 67, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x39, 8, 0, 428, 75, 0, 0 },
    { 1, 0, 0x40, 4, 0x3C, 2, 0, 3, 8, 0, 470, 149, 176, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 86, 239, 258, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 160, 220, 248, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 192, 204, 232, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 320, 201, 231, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 291, 185, 209, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 352, 147, 168, 0 }, { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 371, 140, 160, 0 },
    { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 336, 139, 160, 0 }, { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 385, 131, 153, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 320, 131, 153, 0 }, { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 401, 123, 144, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 304, 117, 144, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 464, 149, 176, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag270_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x201, 0x1F0, 0xA0, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag270_funcs = { wstag270_setup };
void (*D_WSTAG270_800A87A8)(WindowAnim *fade, s32 in) = wstag270_fade_start;
s32 (*D_WSTAG270_800A87AC)(WindowAnim *fade) = wstag270_fade_update;
FieldstgEventDef wstag270_events[8] = {
    { 59, D_WSTAG270_800A6D08, 0x01200000, NULL, NULL }, { 65, D_WSTAG270_800A6DB0, 0x01200001, NULL, NULL },
    { 1415, D_WSTAG270_800A6E4C, 0x01200002, NULL, wstag270_event_1415_end },
    { 1518, NULL, 0x01200005, (Object *(*)(void))wstag270_event_1518_start, NULL },
    { 1519, D_WSTAG270_800A7110, 0x01200003, NULL, NULL },
    { 1520, NULL, 0x01200006, (Object *(*)(void))wstag270_event_1520_start, NULL },
    { 1521, D_WSTAG270_800A7128, 0x01200004, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
