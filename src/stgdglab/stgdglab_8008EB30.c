#include "common.h"
#include "object.h"
#include "cdload.h"
#include "sound.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "stgdglab.h"

/* STGDGLAB.PRO's stage module: the main object (the lab's menu, then the screen chosen in it, and a fade
 * to leave) and the helper table stgdglab_funcs (the same file ends STCRDDEK, STCRDSHP and other menus). */

/* The screens of the menu's entries (stgdglab_main_run); their creates return their own object types. */
Object *(*stgdglab_screens[3])(StgdglabMain *main) = {
    (Object *(*)(StgdglabMain *))stgdglab_swap_create,
    (Object *(*)(StgdglabMain *))stgdglab_digimon_create,
    (Object *(*)(StgdglabMain *))stgdglab_chart_create,
};

/* Per Digimon (stgdglab_funcs.anims): its animation frames, ended by -1. */
StgdglabAnim stgdglab_anims[8] = {
    { { 7, 8, 9, 10, 9, 8, -1 } },
    { { 14, 15, 16, 15, -1, -1, -1 } },
    { { 11, 12, 13, 12, -1, -1, -1 } },
    { { 3, 4, 5, 6, 5, 4, -1 } },
    { { 25, 26, 27, 28, 27, 26, -1 } },
    { { 0, 1, 2, 1, -1, -1, -1 } },
    { { 17, 18, 19, 20, 19, 18, -1 } },
    { { 21, 22, 23, 24, 23, 22, -1 } },
};

/* The menu's windows (stgdglab_funcs.menu_layout). */
StgdglabWindowPos stgdglab_menu_layout[11] = {
    { 0x15, 0x33, 0x26 },
    { 0x16, 0x33, 0x30 },
    { 0x17, 0x5F, 0x30 },
    { 0xE, 0x33, 0x39 },
    { 0x17, 0x5F, 0x39 },
    { 0x12, 0x50, 0x26 },
    { 0x12, 0x5D, 0x30 },
    { 0x12, 0x80, 0x30 },
    { 0x12, 0x5D, 0x39 },
    { 0x12, 0x80, 0x39 },
    { -1, 0x33, 0x17 },
};

StgdglabIdEntry stgdglab_ids[53] = {
    { 0x17F, 0x2, 0x10C },
    { 0x181, 0x4, 0x11D },
    { 0x180, 0x3, 0x114 },
    { 0x3, 0x1, 0x12A },
    { 0x91, 0x7, 0x12B },
    { 0x16E, 0x0, 0x12C },
    { 0x175, 0x5, 0x115 },
    { 0x1F, 0x6, 0x10D },
    { 0x5, 0x22, 0x127 },
    { 0x6, 0x1D, 0x132 },
    { 0xC, 0x23, 0x125 },
    { 0x13, 0x17, 0x11C },
    { 0x14, 0xB, 0x10A },
    { 0x1A, 0x28, 0x131 },
    { 0x1B, 0x25, 0x108 },
    { 0x38, 0x21, 0x135 },
    { 0x3B, 0x10, 0x123 },
    { 0x42, 0x1E, 0x130 },
    { 0x90, 0xF, 0x119 },
    { 0x94, 0x13, 0x121 },
    { 0x96, 0x2A, 0x11E },
    { 0x97, 0x19, 0x12D },
    { 0xC4, 0x26, 0x117 },
    { 0xD3, 0xC, 0x105 },
    { 0xD5, 0x24, 0x102 },
    { 0xD6, 0xD, 0x134 },
    { 0xE6, 0x18, 0x118 },
    { 0xEA, 0xE, 0x106 },
    { 0xFE, 0x12, 0x124 },
    { 0x103, 0x11, 0x129 },
    { 0x104, 0x16, 0x10B },
    { 0x10B, 0x29, 0x133 },
    { 0x167, 0x14, 0x120 },
    { 0x16F, 0x8, 0x128 },
    { 0x170, 0x9, 0x126 },
    { 0x171, 0xA, 0x11F },
    { 0x174, 0x27, 0x122 },
    { 0x176, 0x1A, 0x112 },
    { 0x177, 0x1B, 0x110 },
    { 0x178, 0x1C, 0x10F },
    { 0x179, 0x20, 0x12F },
    { 0x17A, 0x1F, 0x12E },
    { 0x17D, 0x15, 0x100 },
    { 0x182, 0x31, 0x109 },
    { 0x183, 0x2B, 0x113 },
    { 0x184, 0x2E, 0x11B },
    { 0x185, 0x32, 0x107 },
    { 0x186, 0x2C, 0x111 },
    { 0x187, 0x2F, 0x11A },
    { 0x188, 0x33, 0x101 },
    { 0x189, 0x2D, 0x10E },
    { 0x18A, 0x30, 0x116 },
    { 0x0, 0x0, 0x0 },
};

/* The 8 charts (stgdglab_funcs.charts), 4 x 4 requirements each. */
StgdglabReq stgdglab_chart_0[16] = {
    { 3, { 0x182, 0x185, 0x188, 0x0, 0x0 } },
    { 4, { 0x5, 0xC, 0xD5, 0x96, 0x0 } },
    { 2, { 0x0, 0x1A, 0x10B, 0x0, 0x0 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
    { 5, { 0x103, 0xFE, 0x94, 0x167, 0x17D } },
    { 3, { 0x1B, 0xC4, 0x174, 0x0, 0x0 } },
    { 4, { 0x104, 0x13, 0x0, 0xE6, 0x97 } },
    { 3, { 0x176, 0x177, 0x178, 0x0, 0x0 } },
    { 3, { 0xEA, 0x90, 0x3B, 0x0, 0x0 } },
    { 3, { 0x14, 0xD3, 0xD6, 0x0, 0x0 } },
    { 1, { 0x38, 0x0, 0x0, 0x0, 0x0 } },
    { 4, { 0x6, 0x42, 0x17A, 0x179, 0x0 } },
    { 3, { 0x16F, 0x170, 0x171, 0x0, 0x0 } },
    { 3, { 0x183, 0x186, 0x189, 0x0, 0x0 } },
    { 3, { 0x184, 0x187, 0x18A, 0x0, 0x0 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
};
StgdglabReq stgdglab_chart_1[16] = {
    { 3, { 0x184, 0x187, 0x18A, 0x0, 0x0 } },
    { 3, { 0x176, 0x177, 0x178, 0x0, 0x0 } },
    { 1, { 0x38, 0x0, 0x0, 0x0, 0x0 } },
    { 5, { 0x103, 0xFE, 0x94, 0x167, 0x17D } },
    { 3, { 0x1B, 0xC4, 0x174, 0x0, 0x0 } },
    { 4, { 0x6, 0x42, 0x17A, 0x179, 0x0 } },
    { 3, { 0x16F, 0x170, 0x171, 0x0, 0x0 } },
    { 4, { 0x104, 0x13, 0x0, 0xE6, 0x97 } },
    { 3, { 0x14, 0xD3, 0xD6, 0x0, 0x0 } },
    { 3, { 0xEA, 0x90, 0x3B, 0x0, 0x0 } },
    { 3, { 0x183, 0x186, 0x189, 0x0, 0x0 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
    { 4, { 0x5, 0xC, 0xD5, 0x96, 0x0 } },
    { 2, { 0x0, 0x1A, 0x10B, 0x0, 0x0 } },
    { 3, { 0x0, 0x182, 0x185, 0x188, 0x0 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
};
StgdglabReq stgdglab_chart_2[16] = {
    { 3, { 0x183, 0x186, 0x189, 0x0, 0x0 } },
    { 5, { 0x103, 0xFE, 0x94, 0x167, 0x17D } },
    { 3, { 0x0, 0x1B, 0xC4, 0x174, 0x0 } },
    { 1, { 0x0, 0x38, 0x0, 0x0, 0x0 } },
    { 3, { 0x14, 0xD3, 0xD6, 0x0, 0x0 } },
    { 3, { 0x0, 0xEA, 0x90, 0x3B, 0x0 } },
    { 3, { 0x0, 0x176, 0x177, 0x178, 0x0 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
    { 4, { 0x5, 0xC, 0xD5, 0x96, 0x0 } },
    { 2, { 0x0, 0x1A, 0x10B, 0x0, 0x0 } },
    { 4, { 0x0, 0x6, 0x42, 0x17A, 0x179 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
    { 3, { 0x182, 0x185, 0x188, 0x0, 0x0 } },
    { 4, { 0x104, 0x13, 0x0, 0xE6, 0x97 } },
    { 3, { 0x0, 0x16F, 0x170, 0x171, 0x0 } },
    { 3, { 0x0, 0x184, 0x187, 0x18A, 0x0 } },
};
StgdglabReq stgdglab_chart_3[16] = {
    { 4, { 0x5, 0xC, 0xD5, 0x96, 0x0 } },
    { 2, { 0x0, 0x1A, 0x10B, 0x0, 0x0 } },
    { 3, { 0x16F, 0x170, 0x171, 0x0, 0x0 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
    { 3, { 0x183, 0x186, 0x189, 0x0, 0x0 } },
    { 3, { 0x176, 0x177, 0x178, 0x0, 0x0 } },
    { 5, { 0x103, 0xFE, 0x94, 0x167, 0x17D } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
    { 3, { 0x1B, 0xC4, 0x174, 0x0, 0x0 } },
    { 3, { 0x182, 0x185, 0x188, 0x0, 0x0 } },
    { 4, { 0x104, 0x13, 0x0, 0xE6, 0x97 } },
    { 1, { 0x0, 0x38, 0x0, 0x0, 0x0 } },
    { 3, { 0x184, 0x187, 0x18A, 0x0, 0x0 } },
    { 3, { 0x14, 0xD3, 0xD6, 0x0, 0x0 } },
    { 3, { 0x0, 0xEA, 0x90, 0x3B, 0x0 } },
    { 4, { 0x0, 0x6, 0x42, 0x17A, 0x179 } },
};
StgdglabReq stgdglab_chart_4[16] = {
    { 5, { 0x103, 0xFE, 0x94, 0x167, 0x17D } },
    { 4, { 0x104, 0x13, 0x0, 0xE6, 0x97 } },
    { 3, { 0x0, 0x1B, 0xC4, 0x174, 0x0 } },
    { 4, { 0x0, 0x6, 0x42, 0x17A, 0x179 } },
    { 3, { 0x16F, 0x170, 0x171, 0x0, 0x0 } },
    { 4, { 0x5, 0xC, 0xD5, 0x96, 0x0 } },
    { 2, { 0x0, 0x1A, 0x10B, 0x0, 0x0 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
    { 3, { 0x176, 0x177, 0x178, 0x0, 0x0 } },
    { 3, { 0x14, 0xD3, 0xD6, 0x0, 0x0 } },
    { 3, { 0x0, 0xEA, 0x90, 0x3B, 0x0 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
    { 1, { 0x38, 0x0, 0x0, 0x0, 0x0 } },
    { 3, { 0x182, 0x185, 0x188, 0x0, 0x0 } },
    { 3, { 0x183, 0x186, 0x189, 0x0, 0x0 } },
    { 3, { 0x184, 0x187, 0x18A, 0x0, 0x0 } },
};
StgdglabReq stgdglab_chart_5[16] = {
    { 3, { 0x16F, 0x170, 0x171, 0x0, 0x0 } },
    { 3, { 0x1B, 0xC4, 0x174, 0x0, 0x0 } },
    { 4, { 0x104, 0x13, 0x0, 0xE6, 0x97 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
    { 3, { 0x14, 0xD3, 0xD6, 0x0, 0x0 } },
    { 1, { 0x38, 0x0, 0x0, 0x0, 0x0 } },
    { 3, { 0xEA, 0x90, 0x3B, 0x0, 0x0 } },
    { 3, { 0x176, 0x177, 0x178, 0x0, 0x0 } },
    { 3, { 0x184, 0x187, 0x18A, 0x0, 0x0 } },
    { 5, { 0x103, 0xFE, 0x94, 0x167, 0x17D } },
    { 4, { 0x5, 0xC, 0xD5, 0x96, 0x0 } },
    { 2, { 0x0, 0x1A, 0x10B, 0x0, 0x0 } },
    { 4, { 0x6, 0x42, 0x17A, 0x179, 0x0 } },
    { 3, { 0x182, 0x185, 0x188, 0x0, 0x0 } },
    { 3, { 0x183, 0x186, 0x189, 0x0, 0x0 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
};
StgdglabReq stgdglab_chart_6[16] = {
    { 3, { 0x176, 0x177, 0x178, 0x0, 0x0 } },
    { 4, { 0x104, 0x13, 0x0, 0xE6, 0x97 } },
    { 3, { 0xEA, 0x90, 0x3B, 0x0, 0x0 } },
    { 3, { 0x16F, 0x170, 0x171, 0x0, 0x0 } },
    { 5, { 0x103, 0xFE, 0x94, 0x167, 0x17D } },
    { 3, { 0x1B, 0xC4, 0x174, 0x0, 0x0 } },
    { 4, { 0x6, 0x42, 0x17A, 0x179, 0x0 } },
    { 1, { 0x38, 0x0, 0x0, 0x0, 0x0 } },
    { 3, { 0x14, 0xD3, 0xD6, 0x0, 0x0 } },
    { 3, { 0x184, 0x187, 0x18A, 0x0, 0x0 } },
    { 3, { 0x182, 0x185, 0x188, 0x0, 0x0 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
    { 4, { 0x5, 0xC, 0xD5, 0x96, 0x0 } },
    { 2, { 0x0, 0x1A, 0x10B, 0x0, 0x0 } },
    { 3, { 0x183, 0x186, 0x189, 0x0, 0x0 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
};
StgdglabReq stgdglab_chart_7[16] = {
    { 3, { 0x14, 0xD3, 0xD6, 0x0, 0x0 } },
    { 1, { 0x38, 0x0, 0x0, 0x0, 0x0 } },
    { 3, { 0xEA, 0x90, 0x3B, 0x0, 0x0 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
    { 4, { 0x104, 0x13, 0x0, 0xE6, 0x97 } },
    { 4, { 0x5, 0xC, 0xD5, 0x96, 0x0 } },
    { 2, { 0x0, 0x1A, 0x10B, 0x0, 0x0 } },
    { 0, { 0x0, 0x0, 0x0, 0x0, 0x0 } },
    { 3, { 0x183, 0x186, 0x189, 0x0, 0x0 } },
    { 3, { 0x0, 0x1B, 0xC4, 0x174, 0x0 } },
    { 3, { 0x0, 0x16F, 0x170, 0x171, 0x0 } },
    { 4, { 0x0, 0x6, 0x42, 0x17A, 0x179 } },
    { 3, { 0x176, 0x177, 0x178, 0x0, 0x0 } },
    { 3, { 0x182, 0x185, 0x188, 0x0, 0x0 } },
    { 5, { 0x103, 0xFE, 0x94, 0x167, 0x17D } },
    { 3, { 0x184, 0x187, 0x18A, 0x0, 0x0 } },
};

/* Runs the menu, and the screen chosen in it until it closes. */
void stgdglab_main_run(StgdglabMain *obj, StgdglabMainData *data) {
    switch (obj->base.step) {
    case 0:
    default:
        if (data->menu == NULL) {
            data->menu = stgdglab_menu_create(obj);
        }
        obj->base.step++;
        break;
    case 1:
        if (data->menu != NULL) {
            if (data->menu->chosen != 0) {
                data->screen = stgdglab_screens[data->menu->entry](obj);
                obj->base.step++;
            }
        } else {
            obj->base.step = 3;
        }
        break;
    case 2:
        if (data->screen == NULL) {
            obj->base.step = 1;
            data->menu->chosen = 0;
        }
        break;
    case 3:
        if (data->fade->base.state == OBJECT_STATE_DONE) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    }
}

/* Counts the party members (member_count) and moves them to the first slots of gamestate_data.party. */
void stgdglab_main_pack_party(StgdglabMain *obj) {
    s32 i;
    s32 j;

    obj->member_count = 0;
    for (i = 0; i < 3; i++) {
        if (gamestate_data.party[i] >= 0) {
            obj->member_count++;
        }
    }
    for (i = 0; i < 3; i++) {
        if (gamestate_data.party[i] < 0) {
            for (j = i; j < 3; j++) {
                if (gamestate_data.party[j] >= 0) {
                    gamestate_data.party[i] = gamestate_data.party[j];
                    gamestate_data.party[j] = -1;
                    break;
                }
            }
        }
    }
}

void stgdglab_main_update(StgdglabMain *obj, StgdglabMainData *data) {
    Sprite spr;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            stgdglab_funcs.load_files();
            obj->base.step++;
            break;
        case 1:
            if (stgdglab_funcs.is_loading() == 0) {
                obj->base.next_state(obj);
                obj->slot_count = 3;
                stgdglab_main_pack_party(obj);
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        stgdglab_main_run(obj, data);
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, 7);
        spr.set_vram_pos(0x280, 0x100);
        if (obj->odd_frame != 0) {
            obj->scroll = ++obj->scroll < 0x60 ? obj->scroll : 0;
            obj->odd_frame = 0;
        } else {
            obj->odd_frame = 1;
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x37, obj->scroll, obj->scroll);
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        gamestate_data.funcs.set_next_map(gamestate_data.field_map, 0);
        break;
    }
}

/* StgdglabMain.open_menu: the menu's open, if it is running. */
s32 stgdglab_main_open_menu(StgdglabMain *obj) {
    StgdglabMenu *menu = ((StgdglabMainData *)obj->base.children)->menu;

    if (menu != NULL && menu->base.state == OBJECT_STATE_RUN) {
        menu->open(menu);
        return 1;
    }
    return 0;
}

/* StgdglabMain.close_menu: the menu's close, if it is running. */
s32 stgdglab_main_close_menu(StgdglabMain *obj) {
    StgdglabMenu *menu = ((StgdglabMainData *)obj->base.children)->menu;

    if (menu != NULL && menu->base.state == OBJECT_STATE_RUN) {
        menu->close(menu);
        return 1;
    }
    return 0;
}

/* StgdglabMain.is_menu_running: 1 while the menu is running. */
s32 stgdglab_main_is_menu_running(StgdglabMain *obj) {
    StgdglabMenu *menu = ((StgdglabMainData *)obj->base.children)->menu;

    if (menu != NULL && menu->base.state == OBJECT_STATE_RUN) {
        return 1;
    }
    return 0;
}

/* StgdglabMain.fade_out: fades out. */
void stgdglab_main_fade_out(StgdglabMain *obj) {
    StgdglabMainData *data = (StgdglabMainData *)obj->base.children;

    data->fade = stgdglab_fade_create();
    data->fade->start(data->fade, 0, 30);
}

StgdglabMain *stgdglab_main_create(void) {
    StgdglabMain *obj = object_new(stgdglab_main_update, sizeof(StgdglabMain), sizeof(StgdglabMainData));

    obj->open_menu = stgdglab_main_open_menu;
    obj->close_menu = stgdglab_main_close_menu;
    obj->is_menu_running = stgdglab_main_is_menu_running;
    obj->pack_party = stgdglab_main_pack_party;
    obj->fade_out = stgdglab_main_fade_out;
    obj->layer_id = 0x1000;
    return obj;
}

/* Uploads the lab's images (0x02C6, two parts) and starts loading the files the screens need. */
void stgdglab_load_files(void) {
    Tim tim;

    tim_init(&tim);
    tim.set_image_pos(0x280, 0x100);
    tim.load_all(cdload_module.get_subfile_by_id(0x02C60000));
    tim.set_image_pos(0x140, 0x100);
    tim.set_clut_pos(0x280, 0);
    tim.set_buffer_size(0x10000);
    tim.load_all(cdload_module.get_subfile_by_id(0x02C60002));
    cdload_module.queue_file(records_language + 0x39);
    cdload_module.queue_file(records_language + 0x4E);
    cdload_module.queue_file(records_language + 0x47);
    cdload_module.queue_file(records_language + 0xA2);
    cdload_module.queue_file(records_language + 0x9B);
}

/* 1 while one of the language files is still loading. */
s32 stgdglab_is_loading(void) {
    if (cdload_module.is_loading(records_language + 0x39)) {
        return 1;
    }
    if (cdload_module.is_loading(records_language + 0x4E)) {
        return 1;
    }
    if (cdload_module.is_loading(records_language + 0x47)) {
        return 1;
    }
    if (cdload_module.is_loading(records_language + 0xA2)) {
        return 1;
    }
    return cdload_module.is_loading(records_language + 0x9B) != 0;
}

/* window_anim.h's functions, as non-static copies for the table. */
void stgdglab_window_anim_start(WindowAnim *anim, s32 open) {
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

s32 stgdglab_window_anim_update(WindowAnim *anim) {
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

void stgdglab_lerp_start(Tween *lerp, s32 from, s32 to, s32 frames) {
    if (from != to) {
        lerp->duration = frames;
        lerp->acc = from << 8;
        lerp->value = from;
        lerp->target = to;
        lerp->running = 1;
        lerp->step = ((to - from) << 8) / lerp->duration;
    }
}

/* Steps the value; returns 1 once it is at the end (or not running). */
s32 stgdglab_lerp_update(Tween *lerp) {
    if (lerp->running == 0) {
        return 1;
    }
    lerp->acc += lerp->step;
    lerp->value = lerp->acc >> 8;
    if (lerp->step > 0) {
        if (lerp->value > lerp->target) {
            lerp->value = lerp->target;
            lerp->running = 0;
            return 1;
        }
    } else if (lerp->value < lerp->target) {
        lerp->value = lerp->target;
        lerp->running = 0;
        return 1;
    }
    return 0;
}

/* stgdglab_ids's sprite for `id`, 0 if it isn't there. */
s32 stgdglab_get_sprite(s32 id) {
    s32 i;

    for (i = 0; stgdglab_ids[i].id != 0; i++) {
        if (stgdglab_ids[i].id == id) {
            return stgdglab_ids[i].sprite;
        }
    }
    return 0;
}

/* stgdglab_ids's unk_04 for `id`, 0 if it isn't there. */
s32 stgdglab_get_unk_04(s32 id) {
    s32 i;

    for (i = 0; stgdglab_ids[i].id != 0; i++) {
        if (stgdglab_ids[i].id == id) {
            return stgdglab_ids[i].unk_04;
        }
    }
    return 0;
}

StgdglabFuncs stgdglab_funcs = {
    stgdglab_anims,
    stgdglab_menu_layout,
    {
        stgdglab_chart_0, stgdglab_chart_1, stgdglab_chart_2, stgdglab_chart_3,
        stgdglab_chart_4, stgdglab_chart_5, stgdglab_chart_6, stgdglab_chart_7,
    },
    stgdglab_load_files,
    stgdglab_is_loading,
    stgdglab_window_anim_start,
    stgdglab_window_anim_update,
    stgdglab_lerp_start,
    stgdglab_lerp_update,
    stgdglab_get_sprite,
    stgdglab_get_unk_04,
};
