#include "common.h"
#include "object.h"
#include "heap.h"
#include "gfx.h"
#include "cdload.h"
#include "gamestate.h"
#include "sound.h"
#include "pad.h"
#include "message.h"
#include "memcard.h"
#include "records.h"
#include "psyq/libgpu.h"
#include "psyq/libgte.h"
#include "psyq/libc2.h"
#include "psyq/libmcrd.h"
#include "stgmcard.h"

/* Data block of the root object (stgmcard_root_update). */
typedef struct StgmcardRootData {
    /* 0x0 */ Object *main;  /* stgmcard_main_create's object */
} StgmcardRootData;

/* Object of stgmcard_contents_update, created by stgmcard_contents_create: the windows of a card's contents. */
typedef struct StgmcardContents {
    /* 0x00 */ Object base;
    /* 0x50 */ struct StgmcardScreen *screen; /* the screen */
    /* 0x54 */ s32 layer_id;
    /* 0x58 */ s32 ot_depth;
    /* 0x5C */ s32 shown;                  /* shown */
    /* 0x60 */ s32 show_slot;
    /* 0x64 */ s32 anim_time;              /* time of the last animation step */
    /* 0x68 */ s32 anim_frames[3];         /* animation frames */
    /* 0x74 */ WindowAnim fade;           /* fade */
    /* 0x84 */ void (*open)(Object *);
    /* 0x88 */ void (*close)(Object *);
    /* 0x8C */ void (*fill)();
} StgmcardContents; /* size 0x90 */

/* Object of stgmcard_frame_update, created by stgmcard_frame_create: a window frame that grows open. */
typedef struct StgmcardFrame {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id;
    /* 0x54 */ s32 ot_depth; /* ordering table depth */
    /* 0x58 */ s32 x;      /* x */
    /* 0x5C */ s32 y;      /* y */
    /* 0x60 */ s32 w;      /* width */
    /* 0x64 */ s32 h;      /* height */
    /* 0x68 */ s32 is_open; /* fully open */
    /* 0x6C */ u8 color_left[4]; /* colour of the left corners (POLY_G4 vertices 0 and 2) */
    /* 0x70 */ u8 color_right[4]; /* colour of the right corners (1 and 3) */
    /* 0x74 */ s32 frames; /* frames to open */
    /* 0x78 */ s32 step;   /* step */
    /* 0x7C */ u8 unk_7C[0x8];
    /* 0x84 */ s32 center_x; /* centre */
    /* 0x88 */ s32 center_y;
    /* 0x8C */ VECTOR scale;  /* scale */
    /* 0x9C */ SVECTOR rotation; /* rotation */
    /* 0xA4 */ MATRIX matrix;
    /* 0xC4 */ void (*close)(struct StgmcardFrame *);
    /* 0xC8 */ void (*open)(struct StgmcardFrame *, s32, s32);
    /* 0xCC */ void (*set_left_color)(struct StgmcardFrame *, u8, u8, u8);
    /* 0xD0 */ void (*set_right_color)(struct StgmcardFrame *, u8, u8, u8);
    /* 0xD4 */ void (*set_pos)(struct StgmcardFrame *, s32 x, s32 y);
} StgmcardFrame; /* size 0xD8 */

/* Data block of stgmcard_slots_update's object. */
typedef struct StgmcardSlotsData {
    /* 0x0 */ MessageWindow *title;
} StgmcardSlotsData;

/* Object of stgmcard_slots_update, created by stgmcard_slots_create: the card slots and the cursor moving
 * between them. */
typedef struct StgmcardSlots {
    /* 0x00 */ Object base;
    /* 0x50 */ struct StgmcardScreen *screen; /* the screen */
    /* 0x54 */ s32 layer_id;
    /* 0x58 */ s32 ot_depth;
    /* 0x5C */ Tween icons_x;
    /* 0x78 */ Tween bar_y;
    /* 0x94 */ Tween cursor_x;
    /* 0xB0 */ Tween cursor_move;
    /* 0xCC */ s32 done;   /* what was done last (1-3) */
    /* 0xD0 */ s32 blink_time; /* time of the last cursor animation step */
    /* 0xD4 */ s32 blink;  /* cursor frame */
    /* 0xD8 */ s32 blink_back; /* cursor animation going back */
    /* 0xDC */ void (*reset)(struct StgmcardSlots *);
    /* 0xE0 */ void (*show)(struct StgmcardSlots *);
    /* 0xE4 */ void (*select)(struct StgmcardSlots *, s32);
    /* 0xE8 */ void (*hide)(struct StgmcardSlots *);
    /* 0xEC */ void (*open)(struct StgmcardSlots *);
    /* 0xF0 */ void (*close)(struct StgmcardSlots *);
} StgmcardSlots; /* size 0xF4 */

/* A copy of gamestate_data.playtime_frames and playtime (the 12 bytes are copied whole). */
typedef struct StgmcardPlayInfo {
    /* 0x0 */ s32 frames;
    /* 0x4 */ s16 playtime[4];
} StgmcardPlayInfo; /* size 0xC */

/* What the screen shows of a save file (StgmcardSaveHeader.slots). */
typedef struct StgmcardSlotSummary {
    /* 0x00 */ u8 name[0x18];   /* the player's name; empty: no file */
    /* 0x18 */ s32 area;     /* StgmcardMain.area: a ?STAREA entry */
    /* 0x1C */ s32 map_name; /* StgmcardMain.map_name: a ?SHPNAM entry (file 0x94) */
    /* 0x20 */ s32 money;
    /* 0x24 */ StgmcardPlayInfo playtime; /* the save's gamestate_data.playtime_frames..playtime */
    /* 0x30 */ s32 party[3];  /* the party's GamestateDigimon.joined (3..: sprite animation, stgmcard_party_anims) */
    /* 0x3C */ s16 levels[3]; /* the party's levels */
    /* 0x42 */ u8 unk_42[0x2];
} StgmcardSlotSummary; /* size 0x44 */

/* The save file's header (stgmcard_module.header; part 1 of the save file, 0xD4 bytes): what the screen
 * shows of the three save slots. */
typedef struct StgmcardSaveHeader {
    /* 0x00 */ u8 checksum; /* checksum of the bytes from magic on (memcard_funcs.get_checksum) */
    /* 0x01 */ u8 last_slot; /* the slot saved last */
    /* 0x02 */ u8 version; /* 4 */
    /* 0x03 */ u8 unk_03;
    /* 0x04 */ s32 magic;  /* "DMW3" */
    /* 0x08 */ StgmcardSlotSummary slots[3];
} StgmcardSaveHeader; /* size 0xD4 */

/* A save slot (parts 2-4 of the save file): the first 0x26C4 bytes of gamestate_data, copied whole. */
typedef struct StgmcardSaveData {
    /* 0x0000 */ s32 unk_0000[0x26C4 / 4];
} StgmcardSaveData; /* size 0x26C4 */

/* The screen's object (stgmcard_screen_update), created by stgmcard_screen_create. */
typedef struct StgmcardScreen {
    /* 0x0000 */ Object base;
    /* 0x0050 */ struct StgmcardMain *parent; /* the parent */
    /* 0x0054 */ s32 layer_id;
    /* 0x0058 */ s32 x;               /* x */
    /* 0x005C */ s32 y;               /* y */
    /* 0x0060 */ s32 unk_60;
    /* 0x0064 */ s32 port;
    /* 0x0068 */ s32 status;
    /* 0x006C */ StgmcardSaveHeader header; /* the card's save file header */
    /* 0x0140 */ u8 unk_140[0x26C4];
    /* 0x2804 */ s32 question_cursor;
    /* 0x2808 */ u8 unk_2808[0x8];
    /* 0x2810 */ s32 title_x;
    /* 0x2814 */ u8 unk_2814[0x10];
    /* 0x2824 */ Tween title_y;
    /* 0x2840 */ s32 arrow_shown;
    /* 0x2844 */ s32 arrow_time;
    /* 0x2848 */ s32 arrow_frame;
    /* 0x284C */ s32 question_shown;
    /* 0x2850 */ s32 from_title; /* loading from the title screen */
    /* 0x2854 */ void (*refill)(struct StgmcardScreen *);
    /* 0x2858 */ void (*close)(struct StgmcardScreen *);
} StgmcardScreen; /* size 0x285C */

/* A window of the card's contents: text, font, position, shown at once. */
typedef struct StgmcardContentsWindow {
    /* 0x00 */ s32 text;   /* text */
    /* 0x04 */ s32 font;
    /* 0x08 */ s32 x;      /* x */
    /* 0x0C */ s32 y;      /* y */
    /* 0x10 */ s32 is_number;
} StgmcardContentsWindow; /* size 0x14 */

/* Data block of the screen's object. */
typedef struct StgmcardScreenData {
    /* 0x00 */ MessageWindow *title;
    /* 0x04 */ MessageWindow *message;
    /* 0x08 */ MessageWindow *prompt;
    /* 0x0C */ MessageWindow *yes;
    /* 0x10 */ MessageWindow *no;
    /* 0x14 */ MessageWindow *status;
    /* 0x18 */ MessageCursor *cursor; /* cursor */
    /* 0x1C */ StgmcardFrame *frame; /* frame */
    /* 0x20 */ StgmcardSlots *slots; /* card slots */
    /* 0x24 */ StgmcardContents *contents; /* a card's contents */
} StgmcardScreenData;

/* Data block of the memory card screen's root (stgmcard_main_update). */
typedef struct StgmcardMainData {
    /* 0x0 */ StgmcardScreen *screen; /* the screen */
    /* 0x4 */ Fade *fade;  /* the fade */
} StgmcardMainData;

/* Object of stgmcard_main_update, created by stgmcard_main_create. */
typedef struct StgmcardMain {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 loading;
    /* 0x54 */ s32 layer_id;
    /* 0x58 */ s32 scroll;   /* background scroll */
    /* 0x5C */ s32 odd_frame;
    /* 0x60 */ u8 unk_60[0x8];
    /* 0x68 */ s32 area;     /* of the previous map (stgmcard_map_areas) */
    /* 0x6C */ s32 map_name; /* stgmcard_map_names[map & 0xFF] */
} StgmcardMain; /* size 0x70 */

/* An entry of stgmcard_map_areas (ended by map == 0): the area (a ?STAREA entry) the save screen shows for the
 * map the player came from. */
typedef struct StgmcardMapArea {
    /* 0x0 */ u8 area;
    /* 0x1 */ u8 unk_1;
    /* 0x2 */ s16 map;
} StgmcardMapArea; /* size 0x4 */

/* The memory card module as this overlay reaches it: memcard.c's state (memcard_state) and its function table
 * (memcard_funcs, memcard.h) as one object. stgmcard_screen_run keeps one base address of 0x80048750 for two
 * table entries (offsets 0x33C, 0x358), which only one symbol gives. */
typedef struct StgmcardMemcard {
    /* 0x000 */ u8 unk_000[0x10];
    /* 0x010 */ u8 header[0x80];  /* the save file's 0x80-byte header (written as part 0) */
    /* 0x090 */ u8 unk_090[0x14];
    /* 0x0A4 */ s32 file_count; /* entries in files (memcard_list_files) */
    /* 0x0A8 */ DIRENTRY files[15];
    /* 0x300 */ u8 unk_300[0x14];
    /* 0x314 */ s32 icon_frames; /* icon frames (1-3) */
    /* 0x318 */ void *icons[3];   /* icon images */
    /* 0x324 */ s32 icon_frame; /* the icon frame being written */
    /* 0x328 */ MemcardFuncs funcs;   /* memcard_funcs */
} StgmcardMemcard; /* size 0x35C */

extern StgmcardMemcard memcard_state;
extern StgmcardContentsWindow stgmcard_contents_windows[19];
extern s32 stgmcard_party_anims[8][7]; /* frames of the save files' sprite animations, ended by -1 */
extern s32 stgmcard_slot_icons[]; /* per card status - 3: sprite */
extern s32 stgmcard_slot_x[3];     /* x of each slot */
extern StgmcardMapArea stgmcard_map_areas[];
extern s32 stgmcard_map_names[];
extern s32 stgmcard_status_messages[]; /* message per error (StgmcardScreen.status) */

void stgmcard_contents_fill(StgmcardContents *obj);
Object *stgmcard_main_create(void);

void stgmcard_root_update(Object *obj, StgmcardRootData *data) {
    RECT rect;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0x5000);
        gfx_module.funcs.init_display(0x140, 0xF0, 0, 0);
        rect.x = 0;
        rect.y = 0;
        rect.w = 0x140;
        rect.h = 0xF0;
        layer = gfx_module.funcs.create_layer(&rect, 2, 0x1000);
        layer->set_bg_color(layer, 0, 0, 0);
        data->main = stgmcard_main_create();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The overlay's entry point (overlay_entries). */
void stgmcard_create_root(void) {
    object_new(stgmcard_root_update, 0x58, sizeof(StgmcardRootData));
}

/* Starts the fade. */
void stgmcard_fade_start(Fade *obj, s32 from_black, s32 frames) {
    obj->base.set_state(obj, OBJECT_STATE_RUN);
    obj->base.step = 1;
    obj->from_black = from_black;
    if (from_black == 0) {
        obj->level = 0;
        obj->step = 0xFF00 / frames;
    } else {
        obj->level = 0xFF00;
        obj->step = -(0xFF00 / frames);
    }
}

void stgmcard_fade_draw(Fade *obj) {
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

void stgmcard_fade_update(Fade *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
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
        stgmcard_fade_draw(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Fade *stgmcard_fade_create(void) {
    Fade *obj = object_new(stgmcard_fade_update, sizeof(Fade), 0);

    obj->start = stgmcard_fade_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 0;
    return obj;
}

void stgmcard_contents_open(Object *obj) {
    obj->set_step(obj, 1);
}

void stgmcard_contents_close(Object *obj) {
    s32 i;
    MessageWindow **windows;

    obj->set_step(obj, 2);
    windows = (MessageWindow **)obj->children;
    for (i = 0; i < obj->child_count; i++, windows++) {
        if (*windows != NULL) {
            (*windows)->set_visible(*windows, 0);
        }
    }
}

/* Fills the windows with the selected card's save file (stgmcard_module.slot), or the empty-slot texts. */
void stgmcard_contents_fill(StgmcardContents *obj) {
    StgmcardSlotSummary *card = &obj->screen->header.slots[stgmcard_module.slot];
    MessageWindow **w = (MessageWindow **)obj->base.children;
    s32 *status;
    s32 i;
    MessageWindow **p;

    if (obj->show_slot == 0) {
        p = w;
        for (i = 0; i < obj->base.child_count; i++, p++) {
            (*p)->set_visible(*p, 0);
        }
    } else if (card->name[0] == 0) {
        p = w;
        for (i = 0; i < obj->base.child_count; i++, p++) {
            (*p)->set_text(*p, cdload_module.files.get_file(records_language + 0x78), stgmcard_contents_windows[i].text);
            if (stgmcard_contents_windows[i].is_number != 0) {
                (*p)->set_line_number(*p, 1, 0);
                (*p)->measure(*p, 1);
                (*p)->set_pos(*p, stgmcard_contents_windows[i].x, stgmcard_contents_windows[i].y);
            }
        }
    } else {
        w[0]->set_text(w[0], card->name, -1);
        w[1]->set_text(w[1], cdload_module.files.get_file(records_language + 0xA9), card->area);
        w[2]->set_text(w[2], cdload_module.files.get_file(records_language + 0x94), card->map_name);
        for (i = 0; i < 2; i++) {
            w[3 + i]->set_text(w[3 + i], cdload_module.files.get_file(records_language + 0x78), 0x15);
        }
        w[5]->set_line_number(w[5], 0, card->money);
        w[5]->measure(w[5], 1);
        w[12]->set_line_number(w[12], 0, card->playtime.playtime[0]);
        w[13]->set_line_number(w[13], 0, card->playtime.playtime[1]);
        w[14]->set_line_number(w[14], 0, card->playtime.playtime[2]);
        p = &w[12];
        for (i = 0; i < 3; i++, p++) {
            (*p)->measure(*p, 1);
        }
        if (card->playtime.playtime[1] < 10) {
            w[17]->set_line_number(w[17], 0, 0);
            w[17]->measure(w[17], 1);
            w[17]->set_pos(w[17], 0x111, 0xB2);
        } else {
            w[17]->set_visible(w[17], 0);
        }
        if (card->playtime.playtime[2] < 10) {
            w[18]->set_line_number(w[18], 0, 0);
            w[18]->measure(w[18], 1);
            w[18]->set_pos(w[18], 0x124, 0xB2);
        } else {
            w[18]->set_visible(w[18], 0);
        }
        for (i = 0; i < 2; i++) {
            w[15 + i]->set_text(w[15 + i], cdload_module.files.get_file(records_language + 0x78), 0x13);
        }
        status = obj->screen->header.slots[stgmcard_module.slot].party;
        for (i = 0; i < 3; i++) {
            w[6 + i]->set_text(w[6 + i], cdload_module.files.get_file(records_language + 0x78), 0x14);
            if (status[i] - 3 < 0) {
                w[9 + i]->set_text(w[9 + i], cdload_module.files.get_file(records_language + 0x78), 0x23);
            } else {
                w[9 + i]->set_line_number(w[9 + i], 0, card->levels[i]);
            }
            w[9 + i]->measure(w[9 + i], 1);
            obj->anim_frames[i] = 0;
        }
        obj->anim_time = 0;
    }
}

void stgmcard_contents_update(StgmcardContents *obj) {
    Sprite spr;
    MessageWindow **windows;
    s32 *status;
    s32 i;
    s32 j;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stgmcard_contents_windows[0].text = records_language != 0 ? 0x21 : 0x20;
        windows = (MessageWindow **)obj->base.children;
        for (i = 0; i < obj->base.child_count; i++, windows++) {
            if (*windows == NULL) {
                *windows = message_create_window(obj->layer_id, stgmcard_contents_windows[i].font, stgmcard_contents_windows[i].x,
                                                 stgmcard_contents_windows[i].y);
            }
            if ((u32)i < 3) {
                (*windows)->set_palette(*windows, 1);
            }
        }
        obj->fade.duration = 8;
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            switch (obj->base.substep) {
            case 0:
            default:
                stgmcard_module.start_fade(&obj->fade, 1);
                obj->base.substep++;
                break;
            case 1:
                if (stgmcard_module.update_fade(&obj->fade) != 0) {
                    obj->base.set_step(obj, 0);
                }
                break;
            }
            break;
        case 2:
            switch (obj->base.substep) {
            case 0:
            default:
                stgmcard_module.start_fade(&obj->fade, 0);
                obj->show_slot = 0;
                stgmcard_contents_fill(obj);
                obj->base.substep++;
                break;
            case 1:
                if (stgmcard_module.update_fade(&obj->fade) != 0) {
                    obj->shown = 0;
                    obj->base.set_step(obj, 0);
                }
                break;
            }
            break;
        }
        if (obj->shown != 0) {
            sprite_init(&spr);
            spr.set_vram_pos(0x280, 0);
            spr.set_layer_id(obj->layer_id, obj->ot_depth);
            if (obj->fade.level != 0x1000) {
                spr.set_scale(0x1000, obj->fade.level, 0x1000);
                spr.set_pivot(0xA0, 0x94);
            }
            status = obj->screen->header.slots[stgmcard_module.slot].party;
            if (gfx_module.funcs.get_time() - obj->anim_time >= 13) {
                obj->anim_time = gfx_module.funcs.get_time();
                for (j = 0; j < 3; j++) {
                    if (status[j] - 3 >= 0) {
                        if (++obj->anim_frames[j] >= 8) {
                            obj->anim_frames[j] = 0;
                        }
                        if (stgmcard_party_anims[status[j] - 3][obj->anim_frames[j]] == -1) {
                            obj->anim_frames[j] = 0;
                        }
                    }
                }
            }
            for (j = 0; j < 3; j++) {
                if (status[j] - 3 >= 0) {
                    spr.draw(cdload_module.get_subfile_by_id(0x029D0000), stgmcard_party_anims[status[j] - 3][obj->anim_frames[j]],
                               j * 0x34 + 0x9A, 0x85);
                }
            }
            spr.draw(cdload_module.get_subfile_by_id(0x029D0000), 0x24, 0x10, 0x74);
            spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
            spr.draw(cdload_module.get_subfile_by_id(0x029D0000), 0x25, 0x10, 0x74);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}


StgmcardContents *stgmcard_contents_create(struct StgmcardScreen *screen) {
    StgmcardContents *obj = object_new(stgmcard_contents_update, sizeof(StgmcardContents), 0x4C);

    obj->open = stgmcard_contents_open;
    obj->close = stgmcard_contents_close;
    obj->fill = stgmcard_contents_fill;
    obj->layer_id = 0x1000;
    obj->screen = screen;
    obj->ot_depth = 1;
    return obj;
}

/* Closes the frame. */
void stgmcard_frame_close(StgmcardFrame *obj) {
    obj->base.step = 0;
    obj->frames = 0;
    obj->step = 0;
    obj->is_open = 0;
    obj->scale.vx = 0;
    obj->scale.vz = 0x1000;
    obj->scale.vy = 0x1000;
    obj->center_x = obj->x;
    obj->center_y = obj->y;
}

/* Starts opening it (`mode`) over `frames` frames. */
void stgmcard_frame_open(StgmcardFrame *obj, s32 mode, s32 frames) {
    obj->base.step = mode;
    obj->frames = frames;
    obj->step = 0;
}

void stgmcard_frame_set_left_color(StgmcardFrame *obj, u8 r, u8 g, u8 b) {
    obj->color_left[0] = r;
    obj->color_left[1] = g;
    obj->color_left[2] = b;
    obj->color_left[3] = 0;
}

void stgmcard_frame_set_right_color(StgmcardFrame *obj, u8 r, u8 g, u8 b) {
    obj->color_right[0] = r;
    obj->color_right[1] = g;
    obj->color_right[2] = b;
    obj->color_right[3] = 0;
}

void stgmcard_frame_set_pos(StgmcardFrame *obj, s32 x, s32 y) {
    obj->x = x;
    obj->y = y;
}

void stgmcard_frame_update(StgmcardFrame *obj) {
    SVECTOR out[4];
    SVECTOR in[4];
    Sprite spr;
    GfxLayer *layer;
    u32 *ot;
    POLY_G4 *poly;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stgmcard_frame_close(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->frames == 0) {
            break;
        }
        switch (obj->base.step) {
        case 0:
            return;
        case 1:
            if (obj->step == 0) {
                obj->step = 0x1000 / obj->frames;
            }
            obj->scale.vx += obj->step;
            if (obj->scale.vx >= 0xF34) {
                obj->scale.vx = 0xF33;
            }
            break;
        case 2:
            if (obj->step == 0) {
                obj->step = 0x1000 / obj->frames;
            }
            obj->scale.vx += obj->step;
            if (obj->scale.vx > 0x1000) {
                obj->scale.vx = 0x1000;
                obj->is_open = 1;
            }
            break;
        default:
            return;
        }
        if (obj->scale.vx != 0) {
            layer = gfx_module.funcs.get_layer(obj->layer_id);
            ot = layer->get_ot_entry(layer, obj->ot_depth);
            RotMatrixYXZ_gte(&obj->rotation, &obj->matrix);
            ScaleMatrix(&obj->matrix, &obj->scale);
            poly = gfx_module.funcs.get_packet();
            setPolyG4(poly);
            poly->r0 = obj->color_left[0];
            poly->g0 = obj->color_left[1];
            poly->b0 = obj->color_left[2];
            poly->r1 = obj->color_right[0];
            poly->g1 = obj->color_right[1];
            poly->b1 = obj->color_right[2];
            poly->r2 = obj->color_left[0];
            poly->g2 = obj->color_left[1];
            poly->b2 = obj->color_left[2];
            poly->r3 = obj->color_right[0];
            poly->g3 = obj->color_right[1];
            poly->b3 = obj->color_right[2];
            in[0].vx = in[2].vx = obj->x - obj->center_x;
            in[1].vx = in[3].vx = in[0].vx + obj->w;
            in[0].vy = in[1].vy = obj->y - obj->center_y;
            in[2].vy = in[3].vy = in[0].vy + obj->h;
            in[0].vz = in[1].vz = in[2].vz = in[3].vz = 0;
            for (i = 0; i < 4; i++) {
                ApplyMatrixSV(&obj->matrix, &in[i], &out[i]);
                out[i].vx += obj->center_x;
                out[i].vy += obj->center_y;
            }
            poly->x0 = out[0].vx;
            poly->y0 = out[0].vy;
            poly->x1 = out[1].vx;
            poly->y1 = out[1].vy;
            poly->x2 = out[2].vx;
            poly->y2 = out[2].vy;
            poly->x3 = out[3].vx;
            poly->y3 = out[2].vy; /* sic: out[3].vy */
            addPrim(ot, poly);
            gfx_module.funcs.set_packet(poly + 1);
            sprite_init(&spr);
            spr.set_layer_id(obj->layer_id, obj->ot_depth);
            spr.set_vram_pos(0x280, 0);
            spr.draw(cdload_module.get_subfile_by_id(0x029D0000), 0x26, 0xCC, 0xC0);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StgmcardFrame *stgmcard_frame_create(s32 x, s32 y, s32 w, s32 h) {
    StgmcardFrame *obj = object_new(stgmcard_frame_update, sizeof(StgmcardFrame), 0);

    obj->close = stgmcard_frame_close;
    obj->open = stgmcard_frame_open;
    obj->set_left_color = stgmcard_frame_set_left_color;
    obj->set_right_color = stgmcard_frame_set_right_color;
    obj->set_pos = stgmcard_frame_set_pos;
    obj->layer_id = 0x1000;
    obj->ot_depth = 1;
    obj->x = x;
    obj->y = y;
    obj->w = w;
    obj->h = h;
    return obj;
}


void stgmcard_slots_show(StgmcardSlots *obj) {
    StgmcardSlotsData *data = (StgmcardSlotsData *)obj->base.children;

    stgmcard_module.start_value(&obj->bar_y, -0x55, 0, 10);
    data->title->set_line_number(data->title, 1, obj->screen->port + 1);
    data->title->set_visible(data->title, 0);
    obj->base.step = 1;
}

void stgmcard_slots_select(StgmcardSlots *obj, s32 arg1) {
    stgmcard_module.start_value(&obj->cursor_x, 0, 0x2F, 8);
    obj->base.step = 5;
    stgmcard_module.slot = arg1;
    stgmcard_module.prev_slot = 0;
}

void stgmcard_slots_hide(StgmcardSlots *obj) {
    stgmcard_module.start_value(&obj->bar_y, 0, -0x55, 5);
    obj->base.step = 4;
}

void stgmcard_slots_move_cursor(StgmcardSlots *obj) {
    stgmcard_module.start_value(&obj->cursor_move, stgmcard_module.prev_slot * 0x44, stgmcard_module.slot * 0x44, 5);
    obj->base.step = 7;
}

void stgmcard_slots_open(StgmcardSlots *obj) {
    stgmcard_module.start_value(&obj->icons_x, 0xDE, 0, 10);
    obj->base.step = 2;
}

void stgmcard_slots_close(StgmcardSlots *obj) {
    stgmcard_module.start_value(&obj->icons_x, 0, 0xDE, 5);
    obj->base.step = 3;
}

void stgmcard_slots_reset(StgmcardSlots *obj) {
    obj->base.step = 0;
    obj->done = 0;
    stgmcard_module.slot = 0;
    stgmcard_module.prev_slot = 0;
    stgmcard_module.start_value(&obj->icons_x, 0xDE, 0, 10);
    stgmcard_module.start_value(&obj->bar_y, -0x55, 0, 10);
    stgmcard_module.start_value(&obj->cursor_x, 0, 0x2F, 8);
    stgmcard_module.start_value(&obj->cursor_move, -1, 0, 1);
    obj->cursor_move.value = 0;
}

void stgmcard_slots_update(StgmcardSlots *obj, StgmcardSlotsData *data) {
    Sprite spr;
    s32 prev;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stgmcard_slots_reset(obj);
        if (data->title == NULL) {
            data->title = message_create_window(obj->layer_id, 1, 0x15, obj->bar_y.value + 0x18);
        }
        data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x78), 3);
        data->title->set_line_number(data->title, 1, obj->screen->port + 1);
        data->title->set_visible(data->title, 0);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            if (stgmcard_module.step_value(&obj->bar_y) != 0) {
                obj->base.step = 0;
                obj->done = 1;
            }
            break;
        case 2:
            if (stgmcard_module.step_value(&obj->icons_x) != 0) {
                obj->base.step = 0;
                obj->done = 2;
            }
            break;
        case 3:
            if (stgmcard_module.step_value(&obj->icons_x) != 0) {
                obj->base.step = 0;
                obj->done = 3;
            }
            break;
        case 4:
            if (stgmcard_module.step_value(&obj->bar_y) != 0) {
                stgmcard_slots_reset(obj);
            }
            break;
        case 5:
            if (stgmcard_module.step_value(&obj->cursor_x) != 0) {
                stgmcard_slots_move_cursor(obj);
            }
            break;
        case 6:
            if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
                prev = stgmcard_module.slot;
                stgmcard_module.prev_slot = prev;
                stgmcard_module.slot = prev - 1;
                if (stgmcard_module.slot < 0) {
                    stgmcard_module.slot = 0;
                }
                if (stgmcard_module.slot != prev) {
                    obj->screen->refill(obj->screen);
                    stgmcard_slots_move_cursor(obj);
                    sound_module.play(0x4001B);
                }
            } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
                prev = stgmcard_module.slot;
                stgmcard_module.slot = prev + 1;
                stgmcard_module.prev_slot = prev;
                if (stgmcard_module.slot >= 3) {
                    stgmcard_module.slot = 2;
                }
                if (stgmcard_module.slot != prev) {
                    obj->screen->refill(obj->screen);
                    stgmcard_slots_move_cursor(obj);
                    sound_module.play(0x4001B);
                }
            }
            break;
        case 7:
            if (stgmcard_module.step_value(&obj->cursor_move) != 0) {
                obj->base.step = 6;
            }
            break;
        }
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        spr.set_vram_pos(0x280, 0);
        if (gfx_module.funcs.get_time() - obj->blink_time >= 3) {
            obj->blink_time = gfx_module.funcs.get_time();
            if (obj->blink_back == 0) {
                if (++obj->blink >= 12) {
                    obj->blink = 10;
                    obj->blink_back = 1;
                }
            } else {
                if (--obj->blink <= 0) {
                    obj->blink = 0;
                    obj->blink_back = 0;
                }
            }
        }
        spr.set_palette(obj->blink);
        spr.draw(cdload_module.get_subfile_by_id(0x029D0000), 0x23, obj->cursor_x.value + 0x30 + obj->cursor_move.value,
                   obj->bar_y.value + 0x12);
        spr.set_palette(0);
        spr.draw(cdload_module.get_subfile_by_id(0x029D0000), 0x20, 0, obj->bar_y.value + 0x10);
        data->title->set_pos(data->title, 0x15, obj->bar_y.value + 0x18);
        for (i = 0; i < 3; i++) {
            if (obj->screen->header.slots[i].name[0] != 0) {
                spr.draw(cdload_module.get_subfile_by_id(0x029D0000), stgmcard_slot_icons[obj->screen->header.slots[i].party[0] - 3],
                           stgmcard_slot_x[i] + obj->icons_x.value, 0x23);
            }
        }
        spr.draw(cdload_module.get_subfile_by_id(0x029D0000), 0x22, obj->icons_x.value + 0x62, 0x20);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StgmcardSlots *stgmcard_slots_create(StgmcardScreen *screen) {
    StgmcardSlots *obj = object_new(stgmcard_slots_update, sizeof(StgmcardSlots), sizeof(StgmcardSlotsData));

    obj->reset = stgmcard_slots_reset;
    obj->show = stgmcard_slots_show;
    obj->select = stgmcard_slots_select;
    obj->hide = stgmcard_slots_hide;
    obj->open = stgmcard_slots_open;
    obj->close = stgmcard_slots_close;
    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->screen = screen;
    return obj;
}

void stgmcard_screen_show_port(StgmcardScreen *obj, StgmcardScreenData *data, s32 show) {
    if (show) {
        data->status->set_text(data->status, cdload_module.files.get_file(records_language + 0x78), 0x28);
        data->status->set_line_number(data->status, 1, obj->port + 1);
    } else {
        data->status->set_visible(data->status, 0);
    }
}

void stgmcard_screen_report(StgmcardScreen *obj, StgmcardScreenData *data) {
    obj->base.step = 0x64;
    obj->question_cursor = 0;
    obj->status--;
    if (data->prompt != NULL) {
        data->prompt->set_visible(data->prompt, 0);
    }
    if (data->yes != NULL) {
        data->yes->set_visible(data->yes, 0);
    }
    if (data->no != NULL) {
        data->no->set_visible(data->no, 0);
    }
    if (data->cursor != NULL) {
        data->cursor->show(data->cursor, 0);
    }
    if (data->frame != NULL) {
        data->frame->close(data->frame);
    }
}

void stgmcard_screen_report_after_slots(StgmcardScreen *obj, StgmcardScreenData *data) {
    data->prompt->set_visible(data->prompt, 0);
    data->frame->close(data->frame);
    obj->base.step = 0x5A;
    obj->base.substep = 0x64;
    data->slots->base.step = 0;
}

void stgmcard_screen_refill(StgmcardScreen *obj) {
    StgmcardScreenData *data = (StgmcardScreenData *)obj->base.children;

    data->contents->fill(data->contents);
}

void stgmcard_screen_close(StgmcardScreen *obj) {
    StgmcardScreenData *data = (StgmcardScreenData *)obj->base.children;

    if (data->message != NULL) {
        data->message->set_visible(data->message, 0);
    }
    if (data->status != NULL) {
        data->status->set_visible(data->status, 0);
    }
    if (data->prompt != NULL) {
        data->prompt->set_visible(data->prompt, 0);
    }
    if (data->yes != NULL) {
        data->yes->set_visible(data->yes, 0);
    }
    if (data->no != NULL) {
        data->no->set_visible(data->no, 0);
    }
    if (data->cursor != NULL) {
        data->cursor->show(data->cursor, 0);
    }
    if (data->frame != NULL) {
        data->frame->close(data->frame);
    }
    obj->base.set_state(obj, OBJECT_STATE_DONE);
}

/* The memory card screen's state machine (base.step): pick a card (2), check it (0xA-0x16), pick a slot (0x1F-0x21),
 * then save (0x28-0x35), load (0x46-0x47) or format and create the file (0x64-0x7D); 0x190/0x191 wait for the
 * card after an error, 0x1F4-0x1F6 show the result. */
void stgmcard_screen_run(StgmcardScreen *obj, StgmcardScreenData *data) {
    s32 prev;

    switch (obj->base.step) {
    case 0:
    default:
        data->title->set_ot_depth(data->title, 1);
        if (obj->parent->loading == 0) {
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x78), 1);
        } else {
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x78), 0xE);
        }
        if (data->contents == NULL) {
            data->contents = stgmcard_contents_create(obj);
        }
        obj->base.step++;
        /* fallthrough */
    case 1:
        if (stgmcard_module.step_value(&obj->title_y) != 0) {
            data->message->set_visible(data->message, 0);
            if (obj->parent->loading == 0) {
                data->status->set_text(data->status, cdload_module.files.get_file(records_language + 0x78), 2);
            } else {
                data->status->set_text(data->status, cdload_module.files.get_file(records_language + 0x78), 0xF);
            }
            data->prompt->set_ot_depth(data->prompt, 1);
            data->prompt->set_text(data->prompt, cdload_module.files.get_file(records_language + 0x78), 0x1D);
            data->yes->set_text(data->yes, cdload_module.files.get_file(records_language + 0x78), 3);
            data->yes->set_line_number(data->yes, 1, 1);
            data->no->set_text(data->no, cdload_module.files.get_file(records_language + 0x78), 3);
            data->no->set_line_number(data->no, 1, 2);
            data->cursor->show(data->cursor, 1);
            data->cursor->set_pos(data->cursor, 0xC2, obj->port * 14 + 0xBD);
            obj->question_shown = 1;
            obj->base.step++;
        }
        data->title->set_pos(data->title, obj->x + 0xC8 + obj->title_x,
                              obj->y + 9 + obj->title_y.value);
        break;
    case 2:
        prev = obj->port;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            obj->port = 0;
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            obj->port = 1;
        }
        if (prev != obj->port) {
            data->cursor->set_pos(data->cursor, 0xC2, obj->port * 14 + 0xBD);
            sound_module.play(0x8004513E);
        }
        if (PAD_PRESSED(13)) {
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x78), 4);
            stgmcard_screen_show_port(obj, data, 1);
            data->prompt->set_visible(data->prompt, 0);
            data->yes->set_visible(data->yes, 0);
            data->no->set_visible(data->no, 0);
            data->cursor->show(data->cursor, 0);
            if (data->frame == NULL) {
                data->frame = stgmcard_frame_create(0xCD, 0xC1, 0x62, 0xA);
            }
            data->frame->set_left_color(data->frame, 0x7F, 0x32, 0xF2);
            data->frame->set_right_color(data->frame, 0xD1, 0x2F, 0xDE);
            obj->base.step = 0xA;
            sound_module.play(0x8004503C);
            obj->question_shown = 0;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->close(obj);
            obj->question_shown = 0;
            obj->parent->base.substep = 1;
        }
        break;
    case 0xA: {
        s32 ret;

        if (data->frame->base.step == 0) {
            data->frame->open(data->frame, 1, 0x4C);
        }
        ret = memcard_state.funcs.wait_accept(obj->port);
        obj->status = ret;
        if (ret != 0) {
            if (ret == 1 || ret - 1 == 3) {
                obj->base.step++;
            } else {
                data->frame->open(data->frame, 2, 0x14);
                obj->base.step += 2;
            }
        }
        break;
    }
    case 0xB:
        obj->status = memcard_state.funcs.list_files(obj->port);
        if (obj->status != 0) {
            data->frame->open(data->frame, 2, 0x14);
            obj->base.step++;
        }
        break;
    case 0xC:
        if (data->frame->is_open != 0) {
            if (obj->status == 1) {
                obj->base.step = 0x14;
            } else {
                stgmcard_screen_report(obj, data);
            }
        }
        break;
    case 0x14:
        if (data->frame->is_open != 0) {
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x78), 5);
            stgmcard_screen_show_port(obj, data, 1);
            data->frame->close(data->frame);
            data->frame->open(data->frame, 1, 0x4C);
            obj->base.step++;
        }
        /* fallthrough */
    case 0x15:
        obj->status = memcard_state.funcs.read(obj->port, stgmcard_module.header, 0xD4, 1);
        if (obj->status != 0) {
            data->frame->open(data->frame, 2, 0x14);
            obj->base.step++;
        }
        break;
    case 0x16:
        if (data->frame->is_open != 0) {
            if (obj->status == 1) {
                if (stgmcard_module.header->magic != 0x33574D44) {
                    heap_funcs.bzero(stgmcard_module.header, 0x44);
                    stgmcard_module.header->magic = 0x33574D44;
                    stgmcard_module.header->version = 4;
                } else if (memcard_state.funcs.get_checksum((u8 *)stgmcard_module.header + 4, 0xD0) &
                           ~stgmcard_module.header->checksum) {
                    obj->status = 9;
                    stgmcard_screen_report(obj, data);
                    break;
                } else {
                    obj->header = *stgmcard_module.header;
                    stgmcard_module.cursor_slot = stgmcard_module.header->last_slot;
                }
                data->message->set_visible(data->message, 0);
                data->status->set_visible(data->status, 0);
                data->frame->close(data->frame);
                obj->base.step = 0x1E;
            } else {
                stgmcard_screen_report(obj, data);
            }
        }
        break;
    case 0x1E:
        data->message->set_visible(data->message, 0);
        data->status->set_visible(data->status, 0);
        if (data->slots->done != 5) {
            data->slots->reset(data->slots);
        }
        if (data->contents != NULL) {
            data->contents->open(&data->contents->base);
        }
        obj->base.step++;
        break;
    case 0x1F:
        if (data->slots->base.step == 0) {
            if (data->slots->done == 0) {
                data->slots->show(data->slots);
            } else if (data->slots->done == 1) {
                data->slots->open(data->slots);
            } else if (data->slots->done == 2) {
                data->slots->select(data->slots, stgmcard_module.cursor_slot);
                obj->base.step++;
            }
        }
        break;
    case 0x20:
        if (data->slots->base.step == 6) {
            if (obj->parent->loading == 0) {
                data->prompt->set_text(data->prompt, cdload_module.files.get_file(records_language + 0x78), 6);
            } else {
                data->prompt->set_text(data->prompt, cdload_module.files.get_file(records_language + 0x78), 0x10);
            }
            data->contents->shown = 1;
            data->contents->show_slot = 1;
            data->contents->fill(data->contents);
            obj->base.next_step(obj);
            data->slots->base.step = 6;
        }
        break;
    case 0x21: {
        s32 ret;

        if (data->slots->base.step == 6) {
            if (PAD_PRESSED(13)) {
                sound_module.play(0x4001C);
                obj->base.step = 0x190;
                data->slots->base.step = 0;
                if (obj->parent->loading == 0) {
                    if (stgmcard_module.header->slots[stgmcard_module.slot].name[0] != 0) {
                        obj->base.substep = 0x28;
                        obj->question_cursor = 0;
                        data->prompt->set_text(data->prompt, cdload_module.files.get_file(records_language + 0x78), 7);
                        data->yes->set_text(data->yes, cdload_module.files.get_file(records_language + 0x78), 0x16);
                        data->no->set_text(data->no, cdload_module.files.get_file(records_language + 0x78), 0x17);
                        data->cursor->show(data->cursor, 1);
                        obj->question_shown = 1;
                        data->cursor->set_pos(data->cursor, 0xC2, obj->question_cursor * 14 + 0xBD);
                    } else {
                        obj->base.substep = 0x32;
                    }
                } else {
                    obj->base.substep = 0x46;
                }
            } else if (PAD_PRESSED(14)) {
                sound_module.play(0x800450BD);
                data->slots->base.step = 0;
                obj->base.step = 0x5A;
                obj->base.substep = 1;
            } else {
                ret = memcard_state.funcs.wait_exist(obj->port);
                if (ret != 0) {
                    if (ret != 1) {
                        obj->status = ret - 1;
                        stgmcard_screen_report_after_slots(obj, data);
                    }
                }
            }
        }
        break;
    }
    case 0x5A:
        if (data->slots->base.step == 0) {
            if (data->slots->done == 2) {
                data->contents->close(&data->contents->base);
                data->slots->close(data->slots);
            } else if (data->slots->done == 3) {
                data->slots->hide(data->slots);
                obj->base.step = obj->base.substep;
                obj->base.substep = obj->base.timer;
                obj->base.timer = 0;
            }
        }
        break;
    case 0x258:
        if (data->slots->base.step == 0) {
            if (data->slots->done == 2) {
                data->contents->close(&data->contents->base);
                data->slots->close(data->slots);
            } else if (data->slots->done == 3) {
                data->slots->hide(data->slots);
                obj->base.step++;
            }
        }
        break;
    case 0x259:
        obj->close(obj);
        break;
    case 0x46:
        if (stgmcard_module.header->slots[stgmcard_module.slot].name[0] == 0) {
            data->prompt->set_text(data->prompt, cdload_module.files.get_file(records_language + 0x78), 0x18);
            obj->arrow_shown = 1;
            obj->base.step = 0x1F5;
            obj->base.substep = 0x20;
            obj->from_title = 1;
        } else {
            data->prompt->set_text(data->prompt, cdload_module.files.get_file(records_language + 0x78), 0x11);
            data->frame->open(data->frame, 1, 0x4D8);
            obj->base.step++;
        }
        break;
    case 0x47: {
        s32 ret;

        ret = memcard_state.funcs.read(obj->port, stgmcard_module.save, 0x26C4,
                                        stgmcard_module.slot + 2);
        obj->status = ret;
        if (ret != 0) {
            if (ret != 1) {
                obj->status = ret - 1;
                stgmcard_screen_report_after_slots(obj, data);
            } else if (memcard_state.funcs.get_checksum((u8 *)stgmcard_module.save + 4, 0x26C0) &
                       ~stgmcard_module.save->checksum) {
                obj->status = 8;
                stgmcard_screen_report_after_slots(obj, data);
            } else if (stgmcard_module.save->version != 4 && obj->parent->loading != 0) {
                obj->status = 8;
                stgmcard_screen_report_after_slots(obj, data);
            } else {
                *(StgmcardSaveData *)&gamestate_data = *(StgmcardSaveData *)stgmcard_module.save;
                data->frame->open(data->frame, 2, 0x14);
                obj->base.step = 0x1F4;
                data->slots->base.step = 0;
                obj->from_title = 0;
            }
        }
        break;
    }
    case 0x28: {
        s32 ret;

        prev = obj->question_cursor;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            obj->question_cursor = 0;
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            obj->question_cursor = 1;
        }
        if (prev != obj->question_cursor) {
            data->cursor->set_pos(data->cursor, 0xC2, obj->question_cursor * 14 + 0xBD);
            sound_module.play(0x8004513E);
        }
        if (PAD_PRESSED(13)) {
            data->yes->set_visible(data->yes, 0);
            data->no->set_visible(data->no, 0);
            data->cursor->show(data->cursor, 0);
            sound_module.play(0x8004503C);
            obj->question_shown = 0;
            obj->base.step = 0x190;
            if (obj->question_cursor == 0) {
                obj->base.substep = 0x32;
                /* FAKE: a repeated store (reorg deletes it, redundant with the branch's delay-slot store). Its use of
                 * the 0x190 register makes that register live into this branch, so global-alloc (not local-alloc)
                 * places it, after unk_2804's load has taken v0 (the original's v0/v1). Without it the constant's
                 * shorter life wins v0 (99.9%).
                 * No natural form found: store orders, `!obj->unk_2804`, swapped branches, a switch, unk_10 per
                 * branch, the test value or the state in a local (wip-7, final-rest); found by the permuter on
                 * this block alone. */
                obj->base.step = 0x190;
            } else {
                obj->base.substep = 0x20;
                data->slots->base.step = 6;
            }
        } else if (PAD_PRESSED(14)) {
            data->yes->set_visible(data->yes, 0);
            data->no->set_visible(data->no, 0);
            data->cursor->show(data->cursor, 0);
            sound_module.play(0x800450BD);
            obj->base.step = 0x190;
            obj->question_shown = 0;
            obj->base.substep = 0x20;
            data->slots->base.step = 6;
        } else {
            ret = memcard_state.funcs.wait_exist(obj->port);
            if (ret != 0) {
                if (ret != 1) {
                    data->yes->set_visible(data->yes, 0);
                    data->no->set_visible(data->no, 0);
                    data->cursor->show(data->cursor, 0);
                    obj->question_shown = 0;
                    obj->status = ret - 1;
                    stgmcard_screen_report_after_slots(obj, data);
                }
            }
        }
        break;
    }
    case 0x32:
        data->prompt->set_text(data->prompt, cdload_module.files.get_file(records_language + 0x78), 8);
        data->frame->open(data->frame, 1, 0x4F3);
        obj->base.step++;
        break;
    case 0x33: {
        StgmcardSlotSummary *card;
        s32 i;
        s32 ret;

        card = &stgmcard_module.header->slots[stgmcard_module.slot];
        *(StgmcardSaveData *)stgmcard_module.save = *(StgmcardSaveData *)&gamestate_data;
        stgmcard_module.save->checksum =
            memcard_state.funcs.get_checksum((u8 *)stgmcard_module.save + 4, 0x26C0);
        stgmcard_module.save->version = 4;
        strcpy(card->name, stgmcard_module.save->name);
        card->area = obj->parent->area;
        card->map_name = obj->parent->map_name;
        card->money = stgmcard_module.save->money;
        card->playtime = *(StgmcardPlayInfo *)&stgmcard_module.save->playtime_frames;
        for (i = 0; i < 3; i++) {
            ret = gamestate_data.funcs.get_party_member(i);
            card->levels[i] = stgmcard_module.save->digimon[ret].record.stats.values[0];
            card->party[i] = stgmcard_module.save->digimon[ret].joined;
        }
        stgmcard_module.header->last_slot = stgmcard_module.slot;
        stgmcard_module.header->checksum = memcard_state.funcs.get_checksum((u8 *)stgmcard_module.header + 4, 0xD0);
        obj->base.step++;
        break;
    }
    case 0x34: {
        s32 ret;

        ret = memcard_state.funcs.write(obj->port, stgmcard_module.header, 0xD4, 1);
        obj->status = ret;
        if (ret != 0) {
            if (ret == 1) {
                obj->base.step++;
            } else {
                obj->status = ret - 1;
                stgmcard_screen_report_after_slots(obj, data);
            }
        }
        break;
    }
    case 0x35: {
        s32 ret;

        ret = memcard_state.funcs.write(obj->port, stgmcard_module.save, 0x26C4,
                                        stgmcard_module.slot + 2);
        obj->status = ret;
        if (ret != 0) {
            if (ret == 1) {
                data->frame->open(data->frame, 2, 0x14);
                obj->base.step = 0x1F4;
            } else {
                obj->status = ret - 1;
                stgmcard_screen_report_after_slots(obj, data);
            }
        }
        break;
    }
    case 0x1F4:
        if (data->frame->is_open != 0) {
            if (obj->parent->loading == 0) {
                obj->header = *stgmcard_module.header;
                obj->refill(obj);
                data->prompt->set_text(data->prompt, cdload_module.files.get_file(records_language + 0x78), 9);
                obj->base.substep = 0x20;
            } else {
                data->prompt->set_text(data->prompt, cdload_module.files.get_file(records_language + 0x78), 0x12);
                obj->base.substep = 0x190;
                obj->base.step++;
            }
            obj->base.step++;
            data->frame->close(data->frame);
            obj->arrow_shown = 1;
        }
        break;
    case 0x1F5: {
        s32 ret;

        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            obj->arrow_shown = 0;
            obj->base.step = obj->base.substep;
            if (obj->parent->loading == 0) {
                obj->base.set_substep(obj, 0);
                data->slots->base.step = 6;
            } else {
                data->prompt->set_visible(data->prompt, 0);
                obj->base.substep = 0x258;
                if (obj->from_title != 0) {
                    data->slots->base.step = 6;
                } else {
                    data->slots->base.step = 0;
                }
            }
        } else {
            ret = memcard_state.funcs.wait_exist(obj->port);
            if (ret != 0) {
                if (ret != 1) {
                    obj->arrow_shown = 0;
                    obj->status = 1;
                    stgmcard_screen_report_after_slots(obj, data);
                }
            }
        }
        break;
    }
    case 0x1F6:
        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            obj->arrow_shown = 0;
            obj->base.step = obj->base.substep;
            data->prompt->set_visible(data->prompt, 0);
            obj->base.substep = 0x258;
            data->slots->base.step = 0;
        }
        break;
    case 0x64: {
        s32 ok;
        s32 i;
        s32 blocks;

        data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x78),
                              stgmcard_status_messages[obj->status]);
        stgmcard_screen_show_port(obj, data, 1);
        if (obj->parent->loading == 0) {
            if (obj->status == 4) {
                obj->question_cursor = 1;
                ok = 1;
            } else if (obj->status == 5) {
                if (memcard_state.file_count != 0) {
                    blocks = 0;
                    for (i = 0; i < memcard_state.file_count; i++) {
                        blocks += memcard_state.files[i].size / 8192;
                    }
                    if (blocks + 4 >= 16) {
                        data->message->set_visible(data->message, 0);
                        obj->status = 7;
                        obj->base.step = 0x64;
                        return;
                    }
                }
                ok = 1;
            } else {
                obj->arrow_shown = 1;
                ok = 0;
                if (obj->status == 7) {
                    data->message->set_line_number(data->message, 1, 4);
                }
            }
            if (ok) {
                if (obj->status == 4) {
                    data->prompt->set_text(data->prompt, cdload_module.files.get_file(records_language + 0x78), 0x19);
                } else {
                    data->prompt->set_text(data->prompt, cdload_module.files.get_file(records_language + 0x78), 0x1A);
                }
                data->yes->set_text(data->yes, cdload_module.files.get_file(records_language + 0x78), 0x16);
                data->no->set_text(data->no, cdload_module.files.get_file(records_language + 0x78), 0x17);
                data->cursor->show(data->cursor, 1);
                obj->question_shown = 1;
                data->cursor->set_pos(data->cursor, 0xC2, obj->question_cursor * 14 + 0xBD);
            }
        } else {
            obj->arrow_shown = 1;
        }
        obj->base.step++;
        break;
    }
    case 0x65: {
        s32 ret;

        if (obj->parent->loading == 0 && (obj->status == 4 || obj->status == 5)) {
            prev = obj->question_cursor;
            if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                obj->question_cursor = 0;
            } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                obj->question_cursor = 1;
            }
            if (prev != obj->question_cursor) {
                sound_module.play(0x8004513E);
                data->cursor->set_pos(data->cursor, 0xC2, obj->question_cursor * 14 + 0xBD);
            }
            if (PAD_PRESSED(13)) {
                sound_module.play(0x8004503C);
                data->message->set_visible(data->message, 0);
                data->status->set_visible(data->status, 0);
                data->prompt->set_visible(data->prompt, 0);
                data->yes->set_visible(data->yes, 0);
                data->no->set_visible(data->no, 0);
                data->cursor->show(data->cursor, 0);
                obj->base.step = 0x190;
                obj->question_shown = 0;
                if (obj->status == 4) {
                    if (obj->question_cursor == 0) {
                        obj->base.substep = 0x6E;
                        data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x78), 0x1E);
                        stgmcard_screen_show_port(obj, data, 1);
                        data->frame->open(data->frame, 1, 0x90);
                    } else {
                        obj->base.substep = 1;
                        data->frame->close(data->frame);
                    }
                } else {
                    if (obj->question_cursor == 0) {
                        obj->base.substep = 0x78;
                        data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x78), 0x1F);
                        stgmcard_screen_show_port(obj, data, 1);
                        data->frame->open(data->frame, 1, 0x90);
                    } else {
                        obj->base.substep = 1;
                        data->frame->close(data->frame);
                    }
                }
            } else if (PAD_PRESSED(14)) {
                sound_module.play(0x800450BD);
                data->message->set_visible(data->message, 0);
                data->status->set_visible(data->status, 0);
                data->prompt->set_visible(data->prompt, 0);
                data->yes->set_visible(data->yes, 0);
                data->no->set_visible(data->no, 0);
                data->cursor->show(data->cursor, 0);
                obj->base.step = 0x190;
                obj->question_shown = 0;
                obj->base.substep = 1;
                data->frame->close(data->frame);
            }
            ret = memcard_state.funcs.wait_exist(obj->port);
            if (ret != 0) {
                if (ret != 1) {
                    obj->status = ret;
                    obj->question_shown = 0;
                    stgmcard_screen_report(obj, data);
                }
            }
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            obj->base.step = 0x190;
            obj->arrow_shown = 0;
            obj->base.substep = 1;
        }
        break;
    }
    case 0x190:
        obj->status = memcard_state.funcs.wait_exist(obj->port);
        if (obj->status != 0) {
            obj->base.step++;
        }
        break;
    case 0x191:
        if (obj->status != 1) {
            stgmcard_screen_report(obj, data);
        }
        obj->base.step = obj->base.substep;
        obj->base.substep = obj->base.timer;
        obj->base.timer = 0;
        break;
    case 0x6E: {
        s32 ret;

        ret = memcard_state.funcs.format(obj->port);
        obj->status = ret;
        if (ret != 0) {
            if (ret == 1) {
                data->frame->open(data->frame, 2, 0x14);
                obj->base.step++;
            } else {
                stgmcard_screen_report(obj, data);
            }
        }
        break;
    }
    case 0x6F:
        if (data->frame->is_open != 0) {
            stgmcard_screen_report(obj, data);
            obj->status = 5;
        }
        break;
    case 0x78:
        obj->base.step = 0x79;
        break;
    case 0x79: {
        s32 ret;

        ret = memcard_state.funcs.create_file(obj->port);
        obj->status = ret;
        if (ret != 0) {
            if (ret == 1) {
                obj->base.step++;
            } else {
                stgmcard_screen_report(obj, data);
            }
        }
        break;
    }
    case 0x7A: {
        s32 ret;

        ret = memcard_state.funcs.write(obj->port, &memcard_state.header, 0x80, 0);
        obj->status = ret;
        if (ret != 0) {
            if (ret == 1) {
                if (memcard_state.icon_frames < 1 || memcard_state.icon_frames > 3) {
                    obj->status = 3;
                    stgmcard_screen_report(obj, data);
                } else {
                    memcard_state.icon_frame = 0;
                    obj->base.step++;
                }
            } else {
                obj->status = 0xA;
                stgmcard_screen_report(obj, data);
            }
        }
        break;
    }
    case 0x7B: {
        s32 ret;

        ret = memcard_state.funcs.write(obj->port, memcard_state.icons[memcard_state.icon_frame], 0x80,
                                        (memcard_state.icon_frame * 0x80 + 0x80) << 8);
        obj->status = ret;
        if (ret != 0) {
            if (ret == 1) {
                if (++memcard_state.icon_frame > memcard_state.icon_frames - 1) {
                    heap_funcs.bzero(stgmcard_module.header, 0xD4);
                    stgmcard_module.header->magic = 0x33574D44;
                    stgmcard_module.header->version = 4;
                    stgmcard_module.header->checksum =
                        memcard_state.funcs.get_checksum((u8 *)stgmcard_module.header + 4, 0xD0);
                    obj->header = *stgmcard_module.header;
                    stgmcard_module.cursor_slot = stgmcard_module.header->last_slot;
                    obj->base.step++;
                }
            } else {
                obj->status = 0xB;
                stgmcard_screen_report(obj, data);
            }
        }
        break;
    }
    case 0x7C:
        obj->status = memcard_state.funcs.write(obj->port, stgmcard_module.header, 0xD4, 1);
        if (obj->status != 0) {
            data->frame->open(data->frame, 2, 0x14);
            obj->base.step++;
        }
        break;
    case 0x7D:
        if (data->frame->is_open != 0) {
            data->frame->close(data->frame);
            if (obj->status == 1) {
                obj->base.step = 0x1E;
            } else {
                stgmcard_screen_report(obj, data);
            }
        }
        break;
    }
}

void stgmcard_screen_draw(StgmcardScreen *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, 2);
    spr.set_vram_pos(0x140, 0);
    if (obj->arrow_shown != 0) {
        if ((gfx_module.funcs.get_time() - obj->arrow_time) / 3 != 0) {
            obj->arrow_time = gfx_module.funcs.get_time();
            if (++obj->arrow_frame >= 5) {
                obj->arrow_frame = 0;
            }
        }
        spr.set_palette(obj->arrow_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xA, 0x126, 0xD0);
    }
    spr.set_vram_pos(0x280, 0);
    spr.set_palette(0);
    spr.draw(cdload_module.get_subfile_by_id(0x029D0000), 0x21, obj->x + obj->title_x, obj->y + obj->title_y.value);
    if (obj->question_shown != 0) {
        spr.set_layer_id(obj->layer_id, 1);
        spr.draw(cdload_module.get_subfile_by_id(0x029D0000), 0x1D, 0xBC, 0xB9);
    }
}

void stgmcard_screen_update(StgmcardScreen *obj, StgmcardScreenData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->x = 5;
        obj->y = 0x59;
        stgmcard_module.start_value(&obj->title_y, 0x97, 0, 10);
        data->title = message_create_window(obj->layer_id, 1, obj->x + 0xC8, obj->y + 9 + obj->title_y.value);
        data->status = message_create_window(obj->layer_id, 1, obj->x + 0xF, obj->y + 0x18);
        data->message = message_create_window(obj->layer_id, 1, obj->x + 0xF, obj->y + 0x28);
        data->message->set_page_lines(data->message, 5);
        data->message->set_ot_depth(data->message, 1);
        data->prompt = message_create_window(obj->layer_id, 1, obj->x + 0xF, obj->y + 0x67);
        data->prompt->set_page_lines(data->prompt, 2);
        data->yes = message_create_window(obj->layer_id, 1, 0xCF, 0xBD);
        data->no = message_create_window(obj->layer_id, 1, 0xCF, 0xCB);
        data->cursor = message_create_cursor(obj->layer_id, 0, 0xCF, 0xBD);
        data->cursor->show(data->cursor, 0);
        data->slots = stgmcard_slots_create(obj);
        break;
    case OBJECT_STATE_RUN:
        stgmcard_screen_run(obj, data);
        stgmcard_screen_draw(obj);
        break;
    case OBJECT_STATE_DONE:
        switch (obj->base.step) {
        case 0:
        default:
            data->title->set_visible(data->title, 0);
            obj->base.step++;
            break;
        case 1:
            break;
        }
        stgmcard_screen_draw(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

StgmcardScreen *stgmcard_screen_create(struct StgmcardMain *parent) {
    StgmcardScreen *obj = object_new(stgmcard_screen_update, sizeof(StgmcardScreen), sizeof(StgmcardScreenData));

    obj->refill = stgmcard_screen_refill;
    obj->close = stgmcard_screen_close;
    obj->parent = parent;
    obj->layer_id = 0x1000;
    return obj;
}

void stgmcard_main_update(StgmcardMain *obj, StgmcardMainData *data) {
    Sprite spr;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            stgmcard_module.init_save_header();
            obj->base.step++;
            break;
        case 1:
            if (stgmcard_module.is_loading() == 0 && sound_module.is_loading() == 0) {
                sound_module.play(0x60800000);
                obj->base.next_state(obj);
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            if (data->screen == NULL) {
                data->screen = stgmcard_screen_create(obj);
            }
            obj->base.next_step(obj);
            break;
        case 1:
            if (data->screen->base.state == OBJECT_STATE_DONE) {
                data->fade = stgmcard_fade_create();
                data->fade->start(data->fade, 0, 0x1E);
                obj->base.step++;
            }
            break;
        case 2:
            if (data->fade->base.state == OBJECT_STATE_DONE) {
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        }
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, 3);
        spr.set_vram_pos(0x280, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x029D0000), 0x1F, 0x19, 0);
        if (obj->odd_frame != 0) {
            obj->scroll = ++obj->scroll < 0x60 ? obj->scroll : 0;
            obj->odd_frame = 0;
        } else {
            obj->odd_frame = 1;
        }
        spr.draw(cdload_module.get_subfile_by_id(0x029D0000), 0x1E, obj->scroll, obj->scroll);
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        sound_module.stop(0x60800000);
        if (obj->base.substep != 0) {
            if (obj->loading == 0) {
                gamestate_data.funcs.set_next_map(gamestate_data.funcs.get_prev_map(), 0);
            } else {
                gamestate_data.funcs.set_next_map(0xE00, 0);
            }
        } else {
            gamestate_data.funcs.set_next_map(gamestate_data.field_map, 0);
        }
        stgmcard_module.free_buffers();
        break;
    }
}

Object *stgmcard_main_create(void) {
    StgmcardMain *obj = object_new(stgmcard_main_update, sizeof(StgmcardMain), sizeof(StgmcardMainData));
    s32 i;
    s32 map;
    s32 j;

    obj->layer_id = 0x1000;
    i = gamestate_data.funcs.get_map() & 0xFF;
    obj->loading = (u32)gamestate_data.funcs.get_map_entry() >> 31 ^ 1;
    map = gamestate_data.funcs.get_prev_map();
    for (j = 0; stgmcard_map_areas[j].map != 0; j++) {
        if (stgmcard_map_areas[j].map == map) {
            obj->area = stgmcard_map_areas[j].area;
        }
    }
    obj->map_name = stgmcard_map_names[i];
    sound_module.load_extra_bank(0x20);
    return &obj->base;
}

StgmcardContentsWindow stgmcard_contents_windows[19] = {
    { 32, 1, 24, 109, 0 },
    { 33, 1, 24, 123, 0 },
    { 33, 1, 24, 138, 0 },
    { 21, 3, 29, 155, 0 },
    { 21, 3, 118, 163, 0 },
    { 34, 3, 115, 163, 1 },
    { 20, 3, 156, 125, 0 },
    { 20, 3, 208, 125, 0 },
    { 20, 3, 260, 125, 0 },
    { 35, 3, 185, 125, 1 },
    { 35, 3, 237, 125, 1 },
    { 35, 3, 289, 125, 1 },
    { 35, 3, 261, 178, 1 },
    { 35, 3, 280, 178, 1 },
    { 35, 3, 299, 178, 1 },
    { 19, 3, 261, 178, 0 },
    { 19, 3, 280, 178, 0 },
    { 35, 3, 280, 178, 1 },
    { 35, 3, 299, 178, 1 },
};
s32 stgmcard_party_anims[8][7] = {
    { 7, 8, 9, 10, 9, 8, -1 },
    { 14, 15, 16, 15, -1, -1, -1 },
    { 11, 12, 13, 12, -1, -1, -1 },
    { 3, 4, 5, 6, 5, 4, -1 },
    { 25, 26, 27, 28, 27, 26, -1 },
    { 0, 1, 2, 1, -1, -1, -1 },
    { 17, 18, 19, 20, 19, 18, -1 },
    { 21, 22, 23, 24, 23, 22, -1 },
};
s32 stgmcard_slot_icons[8] = { 42, 44, 43, 41, 47, 40, 45, 46 };
s32 stgmcard_slot_x[3] = { 109, 177, 245 };
s32 stgmcard_status_messages[11] = {
    1, 0xA, 0x1B, 0xA, 0xB, 0xC, 0x1C, 0xD, 0x26, 0x24, 0x25,
};
s32 stgmcard_map_names[26] = {
    0x2B, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43,
};
StgmcardMapArea stgmcard_map_areas[] = {
    { 11, 1, 512 }, { 11, 118, 624 }, { 11, 1, 513 }, { 11, 118, 625 }, { 11, 2, 514 }, { 11, 119, 626 },
    { 1, 3, 515 }, { 6, 3, 627 }, { 1, 3, 516 }, { 11, 1, 517 }, { 11, 118, 628 }, { 1, 4, 518 },
    { 6, 4, 629 }, { 1, 5, 519 }, { 6, 5, 630 }, { 1, 6, 520 }, { 6, 6, 631 }, { 1, 7, 521 },
    { 6, 7, 632 }, { 1, 8, 522 }, { 6, 120, 633 }, { 1, 135, 523 }, { 6, 135, 634 }, { 1, 9, 524 },
    { 6, 121, 635 }, { 1, 10, 525 }, { 6, 122, 636 }, { 1, 11, 526 }, { 6, 11, 637 }, { 1, 12, 527 },
    { 6, 12, 638 }, { 1, 13, 528 }, { 6, 13, 639 }, { 1, 14, 529 }, { 6, 14, 640 }, { 1, 15, 530 },
    { 6, 15, 641 }, { 1, 16, 531 }, { 6, 16, 642 }, { 1, 17, 532 }, { 6, 17, 643 }, { 1, 18, 533 },
    { 6, 18, 644 }, { 1, 19, 534 }, { 6, 19, 645 }, { 1, 20, 535 }, { 6, 20, 646 }, { 1, 21, 536 },
    { 6, 21, 647 }, { 1, 22, 537 }, { 6, 22, 648 }, { 1, 23, 538 }, { 6, 23, 649 }, { 1, 24, 539 },
    { 6, 123, 650 }, { 1, 25, 540 }, { 6, 25, 651 }, { 11, 26, 541 }, { 11, 26, 652 }, { 11, 27, 542 },
    { 11, 27, 653 }, { 11, 28, 543 }, { 11, 28, 654 }, { 11, 29, 544 }, { 11, 29, 655 }, { 12, 30, 545 },
    { 12, 30, 656 }, { 12, 31, 546 }, { 12, 31, 657 }, { 12, 32, 547 }, { 12, 32, 658 }, { 12, 33, 548 },
    { 12, 33, 659 }, { 12, 34, 549 }, { 12, 34, 660 }, { 12, 35, 550 }, { 12, 35, 661 }, { 12, 36, 551 },
    { 12, 36, 662 }, { 12, 37, 552 }, { 12, 37, 663 }, { 12, 38, 553 }, { 12, 38, 664 }, { 12, 39, 554 },
    { 12, 39, 665 }, { 12, 40, 555 }, { 12, 40, 666 }, { 12, 41, 556 }, { 12, 41, 667 }, { 12, 42, 557 },
    { 12, 43, 558 }, { 12, 124, 668 }, { 2, 44, 559 }, { 7, 44, 669 }, { 2, 45, 560 }, { 7, 125, 670 },
    { 2, 46, 561 }, { 7, 46, 671 }, { 13, 47, 562 }, { 13, 47, 672 }, { 13, 48, 563 }, { 13, 48, 673 },
    { 13, 49, 564 }, { 13, 49, 674 }, { 13, 50, 565 }, { 13, 50, 675 }, { 13, 51, 566 }, { 13, 52, 567 },
    { 13, 52, 676 }, { 13, 53, 568 }, { 13, 53, 677 }, { 13, 54, 569 }, { 13, 54, 678 }, { 13, 55, 570 },
    { 13, 55, 679 }, { 13, 56, 571 }, { 13, 56, 680 }, { 13, 57, 572 }, { 13, 57, 681 }, { 13, 58, 573 },
    { 13, 58, 682 }, { 13, 59, 574 }, { 13, 126, 683 }, { 3, 60, 575 }, { 8, 127, 684 }, { 3, 61, 576 },
    { 8, 128, 685 }, { 13, 62, 577 }, { 13, 129, 686 }, { 13, 63, 578 }, { 13, 63, 687 }, { 13, 64, 579 },
    { 13, 64, 580 }, { 13, 64, 688 }, { 16, 66, 581 }, { 16, 67, 582 }, { 14, 68, 583 }, { 14, 68, 689 },
    { 14, 69, 584 }, { 14, 69, 690 }, { 14, 70, 585 }, { 14, 70, 691 }, { 14, 71, 586 }, { 14, 71, 692 },
    { 14, 72, 587 }, { 14, 72, 693 }, { 14, 73, 588 }, { 14, 73, 694 }, { 14, 74, 589 }, { 14, 74, 695 },
    { 14, 75, 590 }, { 14, 75, 696 }, { 14, 76, 591 }, { 14, 76, 697 }, { 14, 77, 592 }, { 14, 77, 698 },
    { 14, 78, 593 }, { 14, 78, 699 }, { 14, 79, 594 }, { 14, 79, 700 }, { 14, 80, 595 }, { 14, 80, 701 },
    { 14, 81, 596 }, { 14, 81, 702 }, { 14, 83, 597 }, { 14, 83, 703 }, { 14, 83, 598 }, { 14, 84, 599 },
    { 14, 84, 704 }, { 14, 85, 600 }, { 14, 86, 705 }, { 14, 86, 601 }, { 14, 86, 706 }, { 14, 87, 602 },
    { 14, 87, 707 }, { 14, 88, 603 }, { 14, 88, 708 }, { 14, 89, 604 }, { 14, 89, 709 }, { 14, 90, 605 },
    { 14, 130, 710 }, { 4, 91, 606 }, { 9, 131, 711 }, { 4, 92, 607 }, { 4, 93, 608 }, { 9, 93, 712 },
    { 15, 94, 609 }, { 15, 94, 713 }, { 15, 95, 610 }, { 15, 95, 714 }, { 15, 96, 611 }, { 15, 96, 715 },
    { 15, 97, 612 }, { 15, 97, 716 }, { 15, 98, 613 }, { 15, 98, 717 }, { 15, 99, 614 }, { 15, 99, 718 },
    { 15, 100, 615 }, { 15, 100, 719 }, { 15, 101, 616 }, { 15, 101, 720 }, { 15, 102, 617 }, { 15, 102, 721 },
    { 15, 103, 618 }, { 15, 103, 722 }, { 15, 104, 619 }, { 15, 104, 723 }, { 15, 105, 620 }, { 15, 105, 724 },
    { 15, 106, 621 }, { 15, 107, 622 }, { 15, 107, 725 }, { 15, 108, 623 }, { 15, 132, 726 }, { 17, 109, 727 },
    { 17, 110, 728 }, { 17, 106, 729 }, { 18, 112, 730 }, { 18, 113, 731 }, { 18, 114, 732 }, { 19, 115, 733 },
    { 19, 116, 734 }, { 19, 117, 735 }, { 21, 133, 736 }, { 21, 133, 737 }, { 21, 133, 738 }, { 21, 133, 739 },
    { 21, 133, 740 }, { 21, 133, 741 }, { 21, 133, 742 }, { 21, 133, 743 }, { 21, 134, 744 }, { 21, 134, 745 },
    { 21, 134, 746 }, { 21, 134, 747 }, { 21, 134, 748 }, { 21, 134, 749 }, { 21, 134, 750 }, { 1, 3, 5376 },
    { 0, 0, 0 },
};
