#include "wstag.h"
#include "pad.h"

/* WSTAG280: stage 0x213 (fieldstg_stages). */

extern WstagFuncs wstag280_funcs;
extern FieldstgVramPlace wstag280_vram_places[];
extern FieldstgPlacedActor *wstag280_actors[];
extern FieldstgSprite wstag280_sprites[];
extern FieldstgMapEvent wstag280_map_events[];
extern FieldstgEventDef wstag280_events[];
extern void (*D_WSTAG280_800A7938)(WindowAnim *fade, s32 in);
extern s32 (*D_WSTAG280_800A793C)(WindowAnim *fade);
void wstag280_update();
void wstag280_choice_update();

void wstag280_choice_update(WstagChoice *obj, WstagChoiceData *data) {
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
            D_WSTAG280_800A7938(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (D_WSTAG280_800A793C(&obj->fade) != 0) {
                for (i = 0; i < 2; i++) {
                    data->answers[i]->set_text(data->answers[i], cdload_module.get_subfile_by_id(0x1120039 + (records_language << 16)),
                                              i + 2);
                }
                data->cursor->show(data->cursor, 1);
                data->question->set_text(data->question, cdload_module.get_subfile_by_id(0x1120039 + (records_language << 16)), 1);
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
                data->event = fieldstg_event_start(0x39);
            } else {
                data->event = fieldstg_event_start(0x5F3);
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
            D_WSTAG280_800A7938(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (D_WSTAG280_800A793C(&obj->fade) != 0) {
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

Object *wstag280_event_1522_start(void) {
    return object_new(wstag280_choice_update, sizeof(WstagChoice), sizeof(WstagChoiceData));
}

void wstag280_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_flags.get_flag(0x7201, 0) && gamestate_flags.get_flag(0x8008, 0) && gamestate_flags.get_flag(0x701A, 0)) {
            data->event = fieldstg_event_start(0x42);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag280_start(void *arg0) {
    WstagObject *obj = object_new(wstag280_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag280_funcs.setup();
    return obj;
}

void wstag280_setup(void) {
    fieldstg_stage.background_file = 0x1B2;
    fieldstg_stage.sprite_file = 0x01B30000;
    fieldstg_stage.sprites = wstag280_sprites;
    fieldstg_stage.map_events = wstag280_map_events;
    fieldstg_stage.mask_file = 0x3D3;
    fieldstg_stage.talk_file = records_language + 0xC5;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1BB00, 0xF400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag280_vram_places;
    fieldstg_stage.music = 8;
    fieldstg_stage.sound = 0x60200000;
    fieldstg_stage.actors = wstag280_actors;
    fieldstg_stage.events = wstag280_events;
    fieldstg_attr.set_file(0, 0x01B30002);
    fieldstg_attr.set_file(7, 0x01B30001);
    fieldstg_attr.init_layer(0);
}

void wstag280_fade_start(WindowAnim *fade, s32 in) {
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

s32 wstag280_fade_update(WindowAnim *fade) {
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
void wstag280_setup(void);

s16 D_WSTAG280_800A6708[72] = {
    FIELDSTG_EVENT_WALK(2, 384, 176, 3),
    FIELDSTG_EVENT_ANIM(267, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 267, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 824, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 267, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 825, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 267, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG280_800A6798[60] = {
    FIELDSTG_EVENT_PLACE(2, 536, 244),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(45, 502, 228),
    FIELDSTG_EVENT_ANIM(45, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 528, 241, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 45, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 555, 252, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x201, 384, 216, 7),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG280_800A6810[79] = {
    FIELDSTG_EVENT_WALK(2, 335, 208, 3),
    FIELDSTG_EVENT_PLACE(280, 305, 193),
    FIELDSTG_EVENT_ANIM(280, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 280, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(280, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 360, 196, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WALK(280, 368, 224, 7),
    FIELDSTG_EVENT_WAIT_WALK(280),
    FIELDSTG_EVENT_WALK(2, 335, 208, 3),
    FIELDSTG_EVENT_ANIM(280, 1, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG280_800A68B0[26] = {
    FIELDSTG_EVENT_WALK(2, 384, 176, 3),
    FIELDSTG_EVENT_ANIM(267, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 267, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag280_vram_places[18] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 376, 378, 224, 122, 352, 511 }, { 320, 256, 344, 386, 96, 130, 368, 511 },
    { 320, 256, 368, 378, 192, 122, 352, 510 }, { 320, 256, 320, 386, 0, 130, 368, 510 },
    { 320, 256, 352, 393, 128, 137, 320, 509 }, { 320, 256, 360, 393, 160, 137, 336, 509 },
    { 320, 256, 344, 418, 96, 162, 352, 509 }, { 320, 256, 368, 418, 192, 162, 368, 509 },
    { 320, 256, 328, 386, 32, 130, 320, 508 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 320, 256, 336, 386, 64, 130, 336, 508 }, { 320, 256, 352, 425, 128, 169, 352, 508 },
};
u16 D_WSTAG280_800A6A04[4] = { 0x9000, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A0C[4] = { 0x1A05, 0, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A14[4] = { 0x1A05, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A1C[6] = { 0x1A05, 1, 0x7202, 0, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A28[4] = { 0x9000, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A30[8] = { 0x1A05, 1, 0x7202, 1, 0x8008, 0, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A40[6] = { 0x8008, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A4C[8] = { 0x1A05, 1, 0x7202, 1, 0x8008, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A5C[4] = { 0x9000, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A64[4] = { 0x8008, 0, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A6C[6] = { 0x8008, 1, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A78[6] = { 0x9022, 1, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A84[6] = { 0x8008, 1, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A90[4] = { 0x8008, 0, 0xFFFF, 0 };
u16 D_WSTAG280_800A6A98[6] = { 0, 0, 0x8008, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A6AA4[6] = { 0x9022, 1, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A6AB0[6] = { 0x8008, 1, 0, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG280_800A6ABC[2] = { { NULL, NULL, 138 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6AD4[2] = { { NULL, NULL, 138 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6AEC[2] = { { NULL, NULL, 138 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6B04[2] = { { NULL, NULL, 138 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6B1C[2] = { { NULL, NULL, 138 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6B34[2] = { { NULL, NULL, 138 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6B4C[2] = { { NULL, NULL, 138 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6B64[2] = { { NULL, NULL, 138 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6B7C[2] = { { NULL, NULL, 1129 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6B94[2] = { { NULL, NULL, 1137 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6BAC[2] = { { NULL, NULL, 1130 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6BC4[2] = { { NULL, NULL, 1131 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6BDC[2] = { { NULL, NULL, 1132 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6BF4[2] = { { NULL, NULL, 1133 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6C0C[2] = { { NULL, NULL, 1139 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6C24[2] = { { NULL, NULL, 1134 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6C3C[2] = { { NULL, NULL, 1135 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6C54[2] = { { NULL, NULL, 1136 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6C6C[2] = { { NULL, NULL, 1140 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6C84[2] = { { NULL, NULL, 1148 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6C9C[2] = { { NULL, NULL, 1141 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6CB4[2] = { { NULL, NULL, 1142 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6CCC[2] = { { NULL, NULL, 1143 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6CE4[2] = { { NULL, NULL, 1144 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6CFC[2] = { { NULL, NULL, 1150 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6D14[2] = { { NULL, NULL, 1147 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6D2C[2] = { { NULL, NULL, 1145 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6D44[2] = { { NULL, NULL, 1146 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6D5C[2] = { { NULL, NULL, 1118 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6D74[2] = { { NULL, NULL, 1127 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6D8C[2] = { { NULL, NULL, 1119 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6DA4[2] = { { NULL, NULL, 1120 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6DBC[2] = { { NULL, NULL, 1121 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6DD4[2] = { { NULL, NULL, 1122 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6DEC[2] = { { NULL, NULL, 1128 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6E04[2] = { { NULL, NULL, 1123 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6E1C[2] = { { NULL, NULL, 1125 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6E34[2] = { { NULL, NULL, 1124 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6E4C[2] = { { NULL, NULL, 139 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6E64[2] = { { NULL, NULL, 1126 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6E7C[2] = { { NULL, NULL, 1138 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6E94[2] = { { NULL, NULL, 1149 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6EAC[2] = { { NULL, D_WSTAG280_800A6A04, 312 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6EC4[5] = {
    { D_WSTAG280_800A6A0C, D_WSTAG280_800A6A14, 50 }, { D_WSTAG280_800A6A1C, D_WSTAG280_800A6A28, 312 },
    { D_WSTAG280_800A6A30, D_WSTAG280_800A6A40, 313 }, { D_WSTAG280_800A6A4C, D_WSTAG280_800A6A5C, 312 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG280_800A6F00[4] = {
    { D_WSTAG280_800A6A64, NULL, 163 }, { D_WSTAG280_800A6A6C, D_WSTAG280_800A6A78, 162 },
    { D_WSTAG280_800A6A84, NULL, 164 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG280_800A6F30[4] = {
    { D_WSTAG280_800A6A90, NULL, 163 }, { D_WSTAG280_800A6A98, D_WSTAG280_800A6AA4, 162 },
    { D_WSTAG280_800A6AB0, NULL, 164 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG280_800A6F60[2] = { { NULL, NULL, 164 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6F78[2] = { { NULL, NULL, 164 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG280_800A6F90[2] = { { NULL, NULL, 165 }, { NULL, NULL, 0 } };
u16 D_WSTAG280_800A6FA8[8] = { 0x7022, 1, 0x8008, 0, 0x7201, 0, 0xFFFF, 0 };
u16 D_WSTAG280_800A6FB8[8] = { 0x8008, 1, 0x7022, 1, 0x7201, 0, 0xFFFF, 0 };
u16 D_WSTAG280_800A6FC8[8] = { 0x8008, 0, 0x602B, 1, 0x7201, 0, 0xFFFF, 0 };
u16 D_WSTAG280_800A6FD8[8] = { 0x8008, 1, 0x602B, 1, 0x7201, 0, 0xFFFF, 0 };
u16 D_WSTAG280_800A6FE8[8] = { 0x7022, 1, 0x8008, 0, 0x7201, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A6FF8[8] = { 0x7022, 1, 0x8008, 1, 0x7201, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7008[8] = { 0x602B, 1, 0x8008, 0, 0x7201, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7018[8] = { 0x602B, 1, 0x8008, 1, 0x7201, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7028[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7030[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7038[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7040[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7048[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7050[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7058[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7060[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7068[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7070[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7078[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7080[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7088[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7090[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7098[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A70A0[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A70A8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A70B0[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A70B8[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A70C0[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A70C8[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A70D0[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A70D8[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A70E0[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A70E8[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A70F0[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A70F8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7100[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7108[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7110[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7118[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7120[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7128[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7130[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7138[4] = { 0x8008, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7140[4] = { 0x8008, 0, 0xFFFF, 0 };
u16 D_WSTAG280_800A7148[6] = { 0x7022, 1, 0x8008, 0, 0xFFFF, 0 };
u16 D_WSTAG280_800A7154[6] = { 0x602B, 1, 0x8008, 0, 0xFFFF, 0 };
u16 D_WSTAG280_800A7160[6] = { 0x8008, 1, 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A716C[6] = { 0x8008, 1, 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG280_800A7178[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG280_800A7180 = { D_WSTAG280_800A6FA8, D_WSTAG280_800A6ABC, 45, 4, 502, 228, 7 };
FieldstgPlacedActor D_WSTAG280_800A7194 = { D_WSTAG280_800A6FB8, D_WSTAG280_800A6AD4, 45, 4, 471, 245, 5 };
FieldstgPlacedActor D_WSTAG280_800A71A8 = { D_WSTAG280_800A6FC8, D_WSTAG280_800A6AEC, 45, 4, 502, 228, 7 };
FieldstgPlacedActor D_WSTAG280_800A71BC = { D_WSTAG280_800A6FD8, D_WSTAG280_800A6B04, 45, 4, 471, 245, 5 };
FieldstgPlacedActor D_WSTAG280_800A71D0 = { D_WSTAG280_800A6FE8, D_WSTAG280_800A6B1C, 45, 4, 471, 245, 5 };
FieldstgPlacedActor D_WSTAG280_800A71E4 = { D_WSTAG280_800A6FF8, D_WSTAG280_800A6B34, 45, 4, 471, 245, 5 };
FieldstgPlacedActor D_WSTAG280_800A71F8 = { D_WSTAG280_800A7008, D_WSTAG280_800A6B4C, 45, 4, 471, 245, 5 };
FieldstgPlacedActor D_WSTAG280_800A720C = { D_WSTAG280_800A7018, D_WSTAG280_800A6B64, 45, 4, 471, 245, 5 };
FieldstgPlacedActor D_WSTAG280_800A7220 = { D_WSTAG280_800A7028, D_WSTAG280_800A6B7C, 46, 5, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG280_800A7234 = { D_WSTAG280_800A7030, D_WSTAG280_800A6B94, 46, 5, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG280_800A7248 = { D_WSTAG280_800A7038, D_WSTAG280_800A6BAC, 46, 5, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG280_800A725C = { D_WSTAG280_800A7040, D_WSTAG280_800A6BC4, 46, 5, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG280_800A7270 = { D_WSTAG280_800A7048, D_WSTAG280_800A6BDC, 46, 5, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG280_800A7284 = { D_WSTAG280_800A7050, D_WSTAG280_800A6BF4, 46, 5, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG280_800A7298 = { D_WSTAG280_800A7058, D_WSTAG280_800A6C0C, 46, 5, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG280_800A72AC = { D_WSTAG280_800A7060, D_WSTAG280_800A6C24, 46, 5, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG280_800A72C0 = { D_WSTAG280_800A7068, D_WSTAG280_800A6C3C, 46, 5, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG280_800A72D4 = { D_WSTAG280_800A7070, D_WSTAG280_800A6C54, 46, 5, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG280_800A72E8 = { D_WSTAG280_800A7078, D_WSTAG280_800A6C6C, 51, 6, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG280_800A72FC = { D_WSTAG280_800A7080, D_WSTAG280_800A6C84, 51, 6, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG280_800A7310 = { D_WSTAG280_800A7088, D_WSTAG280_800A6C9C, 51, 6, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG280_800A7324 = { D_WSTAG280_800A7090, D_WSTAG280_800A6CB4, 51, 6, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG280_800A7338 = { D_WSTAG280_800A7098, D_WSTAG280_800A6CCC, 51, 6, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG280_800A734C = { D_WSTAG280_800A70A0, D_WSTAG280_800A6CE4, 51, 6, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG280_800A7360 = { D_WSTAG280_800A70A8, D_WSTAG280_800A6CFC, 51, 6, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG280_800A7374 = { D_WSTAG280_800A70B0, D_WSTAG280_800A6D14, 51, 6, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG280_800A7388 = { D_WSTAG280_800A70B8, D_WSTAG280_800A6D2C, 51, 6, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG280_800A739C = { D_WSTAG280_800A70C0, D_WSTAG280_800A6D44, 51, 6, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG280_800A73B0 = { D_WSTAG280_800A70C8, D_WSTAG280_800A6D5C, 55, 7, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG280_800A73C4 = { D_WSTAG280_800A70D0, D_WSTAG280_800A6D74, 55, 7, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG280_800A73D8 = { D_WSTAG280_800A70D8, D_WSTAG280_800A6D8C, 55, 7, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG280_800A73EC = { D_WSTAG280_800A70E0, D_WSTAG280_800A6DA4, 55, 7, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG280_800A7400 = { D_WSTAG280_800A70E8, D_WSTAG280_800A6DBC, 55, 7, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG280_800A7414 = { D_WSTAG280_800A70F0, D_WSTAG280_800A6DD4, 55, 7, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG280_800A7428 = { D_WSTAG280_800A70F8, D_WSTAG280_800A6DEC, 55, 7, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG280_800A743C = { D_WSTAG280_800A7100, D_WSTAG280_800A6E04, 55, 7, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG280_800A7450 = { D_WSTAG280_800A7108, D_WSTAG280_800A6E1C, 55, 7, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG280_800A7464 = { D_WSTAG280_800A7110, D_WSTAG280_800A6E34, 55, 7, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG280_800A7478 = { D_WSTAG280_800A7118, D_WSTAG280_800A6E4C, 157, 8, 471, 245, 5 };
FieldstgPlacedActor D_WSTAG280_800A748C = { D_WSTAG280_800A7120, D_WSTAG280_800A6E64, 159, 9, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG280_800A74A0 = { D_WSTAG280_800A7128, D_WSTAG280_800A6E7C, 160, 10, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG280_800A74B4 = { D_WSTAG280_800A7130, D_WSTAG280_800A6E94, 161, 11, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG280_800A74C8 = { D_WSTAG280_800A7138, D_WSTAG280_800A6EAC, 267, 12, 353, 160, 7 };
FieldstgPlacedActor D_WSTAG280_800A74DC = { D_WSTAG280_800A7140, D_WSTAG280_800A6EC4, 267, 12, 353, 160, 7 };
FieldstgPlacedActor D_WSTAG280_800A74F0 = { NULL, NULL, 268, 13, 360, 172, 7 };
FieldstgPlacedActor D_WSTAG280_800A7504 = { D_WSTAG280_800A7148, D_WSTAG280_800A6F00, 280, 14, 305, 193, 7 };
FieldstgPlacedActor D_WSTAG280_800A7518 = { D_WSTAG280_800A7154, D_WSTAG280_800A6F30, 280, 14, 305, 193, 7 };
FieldstgPlacedActor D_WSTAG280_800A752C = { D_WSTAG280_800A7160, D_WSTAG280_800A6F60, 280, 14, 368, 224, 3 };
FieldstgPlacedActor D_WSTAG280_800A7540 = { D_WSTAG280_800A716C, D_WSTAG280_800A6F78, 280, 14, 368, 224, 3 };
FieldstgPlacedActor D_WSTAG280_800A7554 = { D_WSTAG280_800A7178, D_WSTAG280_800A6F90, 281, 15, 368, 224, 3 };
FieldstgPlacedActor *wstag280_actors[51] = {
    &D_WSTAG280_800A7180, &D_WSTAG280_800A7194, &D_WSTAG280_800A71A8, &D_WSTAG280_800A71BC, &D_WSTAG280_800A71D0,
    &D_WSTAG280_800A71E4, &D_WSTAG280_800A71F8, &D_WSTAG280_800A720C, &D_WSTAG280_800A7220, &D_WSTAG280_800A7234,
    &D_WSTAG280_800A7248, &D_WSTAG280_800A725C, &D_WSTAG280_800A7270, &D_WSTAG280_800A7284, &D_WSTAG280_800A7298,
    &D_WSTAG280_800A72AC, &D_WSTAG280_800A72C0, &D_WSTAG280_800A72D4, &D_WSTAG280_800A72E8, &D_WSTAG280_800A72FC,
    &D_WSTAG280_800A7310, &D_WSTAG280_800A7324, &D_WSTAG280_800A7338, &D_WSTAG280_800A734C, &D_WSTAG280_800A7360,
    &D_WSTAG280_800A7374, &D_WSTAG280_800A7388, &D_WSTAG280_800A739C, &D_WSTAG280_800A73B0, &D_WSTAG280_800A73C4,
    &D_WSTAG280_800A73D8, &D_WSTAG280_800A73EC, &D_WSTAG280_800A7400, &D_WSTAG280_800A7414, &D_WSTAG280_800A7428,
    &D_WSTAG280_800A743C, &D_WSTAG280_800A7450, &D_WSTAG280_800A7464, &D_WSTAG280_800A7478, &D_WSTAG280_800A748C,
    &D_WSTAG280_800A74A0, &D_WSTAG280_800A74B4, &D_WSTAG280_800A74C8, &D_WSTAG280_800A74DC, &D_WSTAG280_800A74F0,
    &D_WSTAG280_800A7504, &D_WSTAG280_800A7518, &D_WSTAG280_800A752C, &D_WSTAG280_800A7540, &D_WSTAG280_800A7554,
    NULL,
};
FieldstgSprite wstag280_sprites[36] = {
    { 1, 0, 0x40, 2, 0xA, 0, 0, 0, 0, 0, 93, 125, 0, 0 }, { 1, 0, 0x40, 2, 0xB, 0, 0, 0, 0, 0, 126, 109, 0, 0 },
    { 1, 0, 0x40, 2, 0xC, 0, 0, 0, 0, 0, 158, 93, 0, 0 }, { 1, 0, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 235, 67, 0, 0 },
    { 1, 0, 0x40, 2, 0xE, 0, 0, 0, 0, 0, 282, 43, 0, 0 }, { 1, 0, 0x40, 2, 0xF, 0, 0, 0, 0, 0, 379, 34, 0, 0 },
    { 1, 0, 0x40, 2, 0x10, 0, 0, 0, 0, 0, 403, 46, 0, 0 }, { 1, 0, 0x40, 2, 0x11, 0, 0, 0, 0, 0, 427, 90, 0, 0 },
    { 1, 0, 0x40, 2, 0x12, 0, 0, 0, 0, 0, 463, 113, 0, 0 }, { 1, 0, 0x40, 2, 0x13, 0, 0, 0, 0, 0, 482, 109, 0, 0 },
    { 1, 0, 0x40, 2, 0x14, 0, 0, 0, 0, 0, 503, 123, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 84, 130, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 116, 114, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 148, 98, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 272, 44, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 368, 36, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 417, 91, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 453, 113, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 473, 112, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 493, 134, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 497, 124, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 4, 0, 292, 104, 0, 0 },
    { 1, 0, 0x40, 6, 0x15, 0, 0, 0, 0, 0, 416, 59, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 224, 68, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 392, 48, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 416, 60, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 208, 263, 288, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 384, 191, 217, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 320, 169, 184, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 304, 161, 174, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 336, 161, 174, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 288, 153, 166, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 352, 153, 166, 0 }, { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 272, 144, 158, 0 },
    { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 368, 145, 158, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag280_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x201, 0x180, 0xD8, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x212, 0x92, 0x1A2, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0x110, 0xB8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0x100, 0xF0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag280_funcs = { wstag280_setup };
void (*D_WSTAG280_800A7938)(WindowAnim *fade, s32 in) = wstag280_fade_start;
s32 (*D_WSTAG280_800A793C)(WindowAnim *fade) = wstag280_fade_update;
FieldstgEventDef wstag280_events[6] = {
    { 57, D_WSTAG280_800A6708, 0x01120011, NULL, NULL }, { 66, D_WSTAG280_800A6798, 0x01120013, NULL, NULL },
    { 67, D_WSTAG280_800A6810, 0x01120014, NULL, NULL },
    { 1522, NULL, 0x01120039, (s32 (*)(void))wstag280_event_1522_start, NULL },
    { 1523, D_WSTAG280_800A68B0, 0x01120034, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
