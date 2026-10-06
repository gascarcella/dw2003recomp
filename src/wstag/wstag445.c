#include "wstag.h"

/* WSTAG445: stage 0x233 (fieldstg_stages). */

extern WstagFuncs wstag445_funcs;
const CVECTOR wstag445_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag445_battle_lists[];
extern FieldstgVramPlace wstag445_vram_places[];
extern FieldstgPlacedActor *wstag445_actors[];
extern FieldstgSprite wstag445_sprites[];
extern FieldstgMapEvent wstag445_map_events[];
extern FieldstgEventDef wstag445_events[];
void wstag445_update();

void wstag445_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x4023, 1) && gamestate_flags.get_flag(0x4024, 0)) {
            data->event = fieldstg_event_start(0x4F2);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag445_start(void *arg0) {
    WstagObject *obj = object_new(wstag445_update, sizeof(WstagObject), sizeof(FieldstgEvent *));

    obj->manager = arg0;
    wstag445_funcs.setup();
    return obj;
}

void wstag445_event_1265_end(void) {
    gamestate_flags.set_flag(0x4023, 1);
    gamestate_flags.set_flag(0x7401, 1);
}

void wstag445_event_1266_end(void) {
    gamestate_flags.set_flag(0x4024, 1);
    gamestate_flags.set_flag(0x8006, 1);
}

void wstag445_setup(void) {
    fieldstg_stage.background_file = 0x510;
    fieldstg_stage.sprite_file = 0x05110000;
    fieldstg_stage.sprites = wstag445_sprites;
    fieldstg_stage.map_events = wstag445_map_events;
    fieldstg_stage.mask_file = 0x50F;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x14C00, 0x14400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag445_vram_places;
    fieldstg_stage.music = 0x34;
    fieldstg_stage.sound = 0x60D00000;
    fieldstg_stage.actors = wstag445_actors;
    fieldstg_stage.color = wstag445_color;
    fieldstg_stage.battle_lists = wstag445_battle_lists;
    fieldstg_stage.events = wstag445_events;
    fieldstg_attr.set_file(0, 0x05110001);
    fieldstg_attr.set_file(7, 0x05110002);
    fieldstg_attr.set_file(4, 0x05110003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress < 0x18) {
        fieldstg_stage.battle_lists = wstag445_battle_lists;
    } else {
        fieldstg_stage.battle_lists = &wstag445_battle_lists[1];
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag445_setup(void);

s16 D_WSTAG445_800A60E4[55] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 260, 330, 3),
    FIELDSTG_EVENT_PLACE(123, 232, 316),
    FIELDSTG_EVENT_ANIM(123, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 123, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG445_800A6154[66] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 260, 330),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(123, 232, 316),
    FIELDSTG_EVENT_ANIM(123, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 123, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG445_800A61D8 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A61E4 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A61F0 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A61FC = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A6208 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A6214 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A6220 = { 54, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A622C = { 54, 8, 0x60080000 };
FieldstgBattleList D_WSTAG445_800A6238 = {
    3,
    { &D_WSTAG445_800A61D8, &D_WSTAG445_800A61E4, &D_WSTAG445_800A61F0, &D_WSTAG445_800A61FC, &D_WSTAG445_800A6208,
        &D_WSTAG445_800A6214, &D_WSTAG445_800A6220, &D_WSTAG445_800A622C },
};
FieldstgListedBattle D_WSTAG445_800A625C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6268 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6274 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6280 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A628C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6298 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A62A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A62B0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG445_800A62BC = {
    0,
    { &D_WSTAG445_800A625C, &D_WSTAG445_800A6268, &D_WSTAG445_800A6274, &D_WSTAG445_800A6280, &D_WSTAG445_800A628C,
        &D_WSTAG445_800A6298, &D_WSTAG445_800A62A4, &D_WSTAG445_800A62B0 },
};
FieldstgListedBattle D_WSTAG445_800A62E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A62EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A62F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6304 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6310 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A631C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6328 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6334 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG445_800A6340 = {
    0,
    { &D_WSTAG445_800A62E0, &D_WSTAG445_800A62EC, &D_WSTAG445_800A62F8, &D_WSTAG445_800A6304, &D_WSTAG445_800A6310,
        &D_WSTAG445_800A631C, &D_WSTAG445_800A6328, &D_WSTAG445_800A6334 },
};
FieldstgListedBattle D_WSTAG445_800A6364 = { 211, 8, 0x600C0000 };
FieldstgListedBattle D_WSTAG445_800A6370 = { 264, 8, 0x60880000 };
FieldstgListedBattle D_WSTAG445_800A637C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6388 = { 329, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A6394 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A63A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A63AC = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A63B8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG445_800A63C4 = {
    0,
    { &D_WSTAG445_800A6364, &D_WSTAG445_800A6370, &D_WSTAG445_800A637C, &D_WSTAG445_800A6388, &D_WSTAG445_800A6394,
        &D_WSTAG445_800A63A0, &D_WSTAG445_800A63AC, &D_WSTAG445_800A63B8 },
};
FieldstgListedBattle D_WSTAG445_800A63E8 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A63F4 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A6400 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A640C = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A6418 = { 60, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A6424 = { 60, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A6430 = { 60, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A643C = { 60, 8, 0x60080000 };
FieldstgBattleList D_WSTAG445_800A6448 = {
    3,
    { &D_WSTAG445_800A63E8, &D_WSTAG445_800A63F4, &D_WSTAG445_800A6400, &D_WSTAG445_800A640C, &D_WSTAG445_800A6418,
        &D_WSTAG445_800A6424, &D_WSTAG445_800A6430, &D_WSTAG445_800A643C },
};
FieldstgListedBattle D_WSTAG445_800A646C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6478 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6484 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6490 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A649C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A64A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A64B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A64C0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG445_800A64CC = {
    0,
    { &D_WSTAG445_800A646C, &D_WSTAG445_800A6478, &D_WSTAG445_800A6484, &D_WSTAG445_800A6490, &D_WSTAG445_800A649C,
        &D_WSTAG445_800A64A8, &D_WSTAG445_800A64B4, &D_WSTAG445_800A64C0 },
};
FieldstgListedBattle D_WSTAG445_800A64F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A64FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6508 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6514 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6520 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A652C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6538 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6544 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG445_800A6550 = {
    0,
    { &D_WSTAG445_800A64F0, &D_WSTAG445_800A64FC, &D_WSTAG445_800A6508, &D_WSTAG445_800A6514, &D_WSTAG445_800A6520,
        &D_WSTAG445_800A652C, &D_WSTAG445_800A6538, &D_WSTAG445_800A6544 },
};
FieldstgListedBattle D_WSTAG445_800A6574 = { 211, 8, 0x600C0000 };
FieldstgListedBattle D_WSTAG445_800A6580 = { 264, 8, 0x60880000 };
FieldstgListedBattle D_WSTAG445_800A658C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A6598 = { 329, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A65A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A65B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG445_800A65BC = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG445_800A65C8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG445_800A65D4 = {
    0,
    { &D_WSTAG445_800A6574, &D_WSTAG445_800A6580, &D_WSTAG445_800A658C, &D_WSTAG445_800A6598, &D_WSTAG445_800A65A4,
        &D_WSTAG445_800A65B0, &D_WSTAG445_800A65BC, &D_WSTAG445_800A65C8 },
};
FieldstgBattleLists wstag445_battle_lists[2] = {
    { 13, 0, 0, { &D_WSTAG445_800A6238, &D_WSTAG445_800A62BC, &D_WSTAG445_800A6340 }, &D_WSTAG445_800A63C4 },
    { 54, 1, 0, { &D_WSTAG445_800A6448, &D_WSTAG445_800A64CC, &D_WSTAG445_800A6550 }, &D_WSTAG445_800A65D4 },
};
FieldstgVramPlace wstag445_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 421, 216, 165, 352, 511 }, { 320, 256, 320, 437, 0, 181, 368, 511 },
    { 320, 256, 358, 256, 152, 0, 320, 510 }, { 320, 256, 328, 437, 32, 181, 336, 510 },
    { 320, 256, 336, 437, 64, 181, 352, 510 }, { 320, 256, 344, 445, 96, 189, 368, 510 },
};
u16 D_WSTAG445_800A66F0[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A66F8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6700[6] = { 0, 1, 0x7202, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A670C[8] = { 0, 1, 0x7202, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A671C[4] = { 0x7615, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6724[10] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0B, 0,
    0xFFFF, 0,
};
u16 D_WSTAG445_800A6738[6] = { 0x7400, 1, 0xE0B, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6744[12] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0B, 1,
    0x8012, 0, 0xFFFF, 0,
};
u16 D_WSTAG445_800A675C[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0B, 1,
    0x8012, 1, 0x7206, 0, 0xFFFF, 0,
};
u16 D_WSTAG445_800A6778[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0B, 1,
    0x8012, 1, 0x7206, 1, 0xFFFF, 0,
};
u16 D_WSTAG445_800A6794[4] = { 0x7815, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A679C[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A67A4[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A67AC[6] = { 0, 1, 0x7202, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A67B8[8] = { 0, 1, 0x7202, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A67C8[4] = { 0x7615, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A67D0[10] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0B, 0,
    0xFFFF, 0,
};
u16 D_WSTAG445_800A67E4[6] = { 0x7400, 1, 0xE0B, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A67F0[12] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0B, 1,
    0x8012, 0, 0xFFFF, 0,
};
u16 D_WSTAG445_800A6808[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0B, 1,
    0x8012, 1, 0x7206, 0, 0xFFFF, 0,
};
u16 D_WSTAG445_800A6824[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0B, 1,
    0x8012, 1, 0x7206, 1, 0xFFFF, 0,
};
u16 D_WSTAG445_800A6840[4] = { 0x7815, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6848[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A6850[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6858[6] = { 0, 1, 0x7202, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A6864[8] = { 0, 1, 0x7202, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A6874[4] = { 0x7615, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A687C[10] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0B, 0,
    0xFFFF, 0,
};
u16 D_WSTAG445_800A6890[6] = { 0x7400, 1, 0xE0B, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A689C[12] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0B, 1,
    0x8012, 0, 0xFFFF, 0,
};
u16 D_WSTAG445_800A68B4[14] = {
    0, 1, 0x7202, 1, 0x7206, 0, 0x7204, 1,
    0x8012, 1, 0xE0B, 1, 0xFFFF, 0,
};
u16 D_WSTAG445_800A68D0[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0B, 1,
    0x8012, 1, 0x7206, 1, 0xFFFF, 0,
};
u16 D_WSTAG445_800A68EC[4] = { 0x7815, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A68F4[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A68FC[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6908[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A6910[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A691C[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A6928[4] = { 0x1C12, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A6930[6] = { 0x1C12, 1, 0x9025, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A693C[4] = { 0x1C12, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6944[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A694C[6] = { 0, 1, 0x7202, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6958[8] = { 0, 1, 0x7202, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A6968[4] = { 0x7615, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6970[10] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0B, 0,
    0xFFFF, 0,
};
u16 D_WSTAG445_800A6984[4] = { 0xE0B, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A698C[12] = {
    0x7204, 1, 0x8012, 0, 0, 1, 0x7202, 1,
    0xE0B, 1, 0xFFFF, 0,
};
u16 D_WSTAG445_800A69A4[14] = {
    0x7204, 1, 0x8012, 1, 0, 1, 0x7202, 1,
    0xE0B, 1, 0x7206, 0, 0xFFFF, 0,
};
u16 D_WSTAG445_800A69C0[14] = {
    0, 1, 0x7202, 1, 0xE0B, 1, 0x7204, 1,
    0x8012, 1, 0x7206, 1, 0xFFFF, 0,
};
u16 D_WSTAG445_800A69DC[4] = { 0x7815, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG445_800A69E4[8] = {
    { D_WSTAG445_800A66F0, D_WSTAG445_800A66F8, 151 }, { D_WSTAG445_800A6700, NULL, 156 },
    { D_WSTAG445_800A670C, D_WSTAG445_800A671C, 157 }, { D_WSTAG445_800A6724, D_WSTAG445_800A6738, 158 },
    { D_WSTAG445_800A6744, NULL, 159 }, { D_WSTAG445_800A675C, NULL, 160 },
    { D_WSTAG445_800A6778, D_WSTAG445_800A6794, 161 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG445_800A6A44[2] = { { NULL, NULL, 639 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG445_800A6A5C[8] = {
    { D_WSTAG445_800A679C, D_WSTAG445_800A67A4, 152 }, { D_WSTAG445_800A67AC, NULL, 156 },
    { D_WSTAG445_800A67B8, D_WSTAG445_800A67C8, 157 }, { D_WSTAG445_800A67D0, D_WSTAG445_800A67E4, 158 },
    { D_WSTAG445_800A67F0, NULL, 159 }, { D_WSTAG445_800A6808, NULL, 160 },
    { D_WSTAG445_800A6824, D_WSTAG445_800A6840, 161 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG445_800A6ABC[8] = {
    { D_WSTAG445_800A6848, D_WSTAG445_800A6850, 153 }, { D_WSTAG445_800A6858, NULL, 156 },
    { D_WSTAG445_800A6864, D_WSTAG445_800A6874, 157 }, { D_WSTAG445_800A687C, D_WSTAG445_800A6890, 158 },
    { D_WSTAG445_800A689C, NULL, 159 }, { D_WSTAG445_800A68B4, NULL, 160 },
    { D_WSTAG445_800A68D0, D_WSTAG445_800A68EC, 161 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG445_800A6B1C[4] = {
    { D_WSTAG445_800A68F4, NULL, 151 }, { D_WSTAG445_800A68FC, D_WSTAG445_800A6908, 162 },
    { D_WSTAG445_800A6910, D_WSTAG445_800A691C, 163 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG445_800A6B4C[2] = { { NULL, NULL, 31 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG445_800A6B64[2] = { { NULL, NULL, 31 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG445_800A6B7C[2] = { { NULL, NULL, 284 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG445_800A6B94[2] = { { NULL, NULL, 285 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG445_800A6BAC[2] = { { NULL, NULL, 299 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG445_800A6BC4[3] = {
    { D_WSTAG445_800A6928, D_WSTAG445_800A6930, 701 }, { D_WSTAG445_800A693C, NULL, 702 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG445_800A6BE8[2] = { { NULL, NULL, 298 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG445_800A6C00[8] = {
    { D_WSTAG445_800A6944, NULL, 154 }, { D_WSTAG445_800A694C, NULL, 154 },
    { D_WSTAG445_800A6958, D_WSTAG445_800A6968, 154 }, { D_WSTAG445_800A6970, D_WSTAG445_800A6984, 154 },
    { D_WSTAG445_800A698C, NULL, 154 }, { D_WSTAG445_800A69A4, NULL, 154 },
    { D_WSTAG445_800A69C0, D_WSTAG445_800A69DC, 154 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG445_800A6C60[2] = { { NULL, NULL, 817 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG445_800A6C78[2] = { { NULL, NULL, 817 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG445_800A6C90[2] = { { NULL, NULL, 817 }, { NULL, NULL, 0 } };
u16 D_WSTAG445_800A6CA8[8] = { 0x7003, 1, 0x11, 0, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6CB8[8] = { 0x8192, 0, 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A6CC8[8] = { 0x7004, 1, 0x11, 0, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6CD8[8] = { 0x6026, 1, 0x11, 0, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6CE8[8] = { 0x7009, 1, 0x11, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6CF8[4] = { 0x6019, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6D00[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6D08[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6D10[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6D18[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6D20[6] = { 0x600, 1, 0x8006, 0, 0xFFFF, 0 };
u16 D_WSTAG445_800A6D2C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6D34[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6D3C[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6D44[4] = { 0x6009, 1, 0xFFFF, 0 };
u16 D_WSTAG445_800A6D4C[4] = { 0x600A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG445_800A6D54 = { D_WSTAG445_800A6CA8, D_WSTAG445_800A69E4, 49, 4, 595, 475, 1 };
FieldstgPlacedActor D_WSTAG445_800A6D68 = { D_WSTAG445_800A6CB8, D_WSTAG445_800A6A44, 49, 4, 595, 475, 1 };
FieldstgPlacedActor D_WSTAG445_800A6D7C = { D_WSTAG445_800A6CC8, D_WSTAG445_800A6A5C, 49, 4, 595, 475, 1 };
FieldstgPlacedActor D_WSTAG445_800A6D90 = { D_WSTAG445_800A6CD8, D_WSTAG445_800A6ABC, 49, 4, 595, 475, 1 };
FieldstgPlacedActor D_WSTAG445_800A6DA4 = { D_WSTAG445_800A6CE8, D_WSTAG445_800A6B1C, 49, 4, 595, 475, 1 };
FieldstgPlacedActor D_WSTAG445_800A6DB8 = { D_WSTAG445_800A6CF8, D_WSTAG445_800A6B4C, 57, 5, 611, 171, 1 };
FieldstgPlacedActor D_WSTAG445_800A6DCC = { D_WSTAG445_800A6D00, D_WSTAG445_800A6B64, 57, 5, 611, 171, 1 };
FieldstgPlacedActor D_WSTAG445_800A6DE0 = { D_WSTAG445_800A6D08, D_WSTAG445_800A6B7C, 57, 5, 611, 171, 1 };
FieldstgPlacedActor D_WSTAG445_800A6DF4 = { D_WSTAG445_800A6D10, D_WSTAG445_800A6B94, 57, 5, 611, 171, 1 };
FieldstgPlacedActor D_WSTAG445_800A6E08 = { D_WSTAG445_800A6D18, D_WSTAG445_800A6BAC, 57, 5, 207, 311, 3 };
FieldstgPlacedActor D_WSTAG445_800A6E1C = { D_WSTAG445_800A6D20, D_WSTAG445_800A6BC4, 123, 6, 232, 316, 7 };
FieldstgPlacedActor D_WSTAG445_800A6E30 = { D_WSTAG445_800A6D2C, D_WSTAG445_800A6BE8, 157, 7, 611, 171, 1 };
FieldstgPlacedActor D_WSTAG445_800A6E44 = { D_WSTAG445_800A6D34, D_WSTAG445_800A6C00, 158, 8, 595, 475, 1 };
FieldstgPlacedActor D_WSTAG445_800A6E58 = { D_WSTAG445_800A6D3C, D_WSTAG445_800A6C60, 178, 9, 288, 496, 3 };
FieldstgPlacedActor D_WSTAG445_800A6E6C = { D_WSTAG445_800A6D44, D_WSTAG445_800A6C78, 178, 9, 288, 496, 3 };
FieldstgPlacedActor D_WSTAG445_800A6E80 = { D_WSTAG445_800A6D4C, D_WSTAG445_800A6C90, 178, 9, 288, 496, 3 };
FieldstgPlacedActor *wstag445_actors[17] = {
    &D_WSTAG445_800A6D54, &D_WSTAG445_800A6D68, &D_WSTAG445_800A6D7C, &D_WSTAG445_800A6D90, &D_WSTAG445_800A6DA4,
    &D_WSTAG445_800A6DB8, &D_WSTAG445_800A6DCC, &D_WSTAG445_800A6DE0, &D_WSTAG445_800A6DF4, &D_WSTAG445_800A6E08,
    &D_WSTAG445_800A6E1C, &D_WSTAG445_800A6E30, &D_WSTAG445_800A6E44, &D_WSTAG445_800A6E58, &D_WSTAG445_800A6E6C,
    &D_WSTAG445_800A6E80, NULL,
};
FieldstgSprite wstag445_sprites[77] = {
    { 1, 0, 0x40, 2, 2, 1, 2, 7, 8, 0, 95, 22, 0, 0 }, { 1, 0, 0x40, 2, 2, 1, 2, 7, 8, 0, 700, 48, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 235, 430, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 273, 238, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 363, 254, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 696, 424, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 94, 336, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 662, 456, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 367, 482, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 476, 345, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 669, 498, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 417, 446, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 421, 278, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 596, 526, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 166, 432, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 229, 375, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 436, 275, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 483, 489, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 172, 351, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 283, 392, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 451, 287, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 550, 507, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 127, 287, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 133, 280, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 185, 434, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 219, 401, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 390, 474, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 417, 508, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 669, 570, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 672, 562, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 683, 497, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 690, 430, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 709, 426, 0, 0 },
    { 1, 0x64, 0x40, 6, 1, 0, 0, 0, 0, 0, 490, 108, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, -4, 274, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, -2, 366, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 65, 485, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 85, 170, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 129, 227, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 159, 389, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 176, 334, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 273, 212, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 355, 284, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 358, 517, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 378, 421, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 389, 573, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 471, 540, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 498, 475, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 500, 291, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 556, 625, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 650, 485, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 687, 407, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 750, 536, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 104, 604, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 133, 404, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 136, 302, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 142, 447, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 212, 399, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 290, 266, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 300, 550, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 341, 568, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 377, 471, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 420, 254, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 480, 327, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 576, 564, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 623, 521, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 750, 426, 0, 0 },
    { 1, 0, 0x58, 4, 0, 0, 0, 0, 0, 0, 464, 77, 161, 0 }, { 1, 0, 0x80, 4, 0xD, 0, 0, 0, 0, 0, 358, 119, 155, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 223, 167, 167, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 248, 140, 140, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 295, 115, 115, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 304, 151, 151, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 599, 123, 123, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 656, 313, 313, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 690, 375, 375, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag445_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x234, 0x668, 0xB4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x232, 0x190, 0x218, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag445_funcs = { wstag445_setup };
FieldstgEventDef wstag445_events[3] = {
    { 1265, D_WSTAG445_800A60E4, 0x01350013, NULL, wstag445_event_1265_end },
    { 1266, D_WSTAG445_800A6154, 0x01350014, NULL, wstag445_event_1266_end }, { -1, NULL, 0, NULL, NULL },
};
