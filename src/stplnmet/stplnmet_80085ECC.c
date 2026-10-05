#include "common.h"
#include "object.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "psyq/libgpu.h"
#include "psyq/libgte.h"
#include "stplnmet.h"

/* The main object: after an opening message, the player enters a name (stplnmet_80083D70.c), chooses one of three
 * partner Digimon and confirms; the choice is saved and the game goes on. Three tabs at the top show the step. */

/* The confirmation (stplnmet_confirm_create, size 0xE4): the name, the partner and "OK?"; on yes a bar fills
 * up while the game is saved. */
typedef struct StplnmetConfirm {
    /* 0x00 */ Object base;
    /* 0x50 */ StplnmetMain *main;
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 ot_depth; /* ordering table depth */
    /* 0x5C */ s32 partner_set; /* the partner set (row of stplnmet_funcs.partners) */
    /* 0x60 */ s32 cursor; /* cursor: 0 yes, 1 no */
    /* 0x64 */ s32 frame_anim_frame; /* the frame's animation frame */
    /* 0x68 */ s32 frame_anim_time; /* time of its last frame */
    /* 0x6C */ u16 *name;   /* the name (StplnmetNameEntry.name) */
    /* 0x70 */ s32 frames[3]; /* per Digimon of the set: its animation frame */
    /* 0x7C */ s32 frame_time; /* time of their last frame */
    /* 0x80 */ s32 bar;    /* the bar, 0..100 */
    /* 0x84 */ VECTOR bar_scale; /* the bar's scale */
    /* 0x94 */ SVECTOR bar_rotation; /* its rotation */
    /* 0x9C */ MATRIX bar_matrix;
    /* 0xBC */ WindowAnim frame_anim; /* the frame */
    /* 0xCC */ WindowAnim window_anim; /* the window */
    /* 0xDC */ void (*pause)(struct StplnmetConfirm *obj, s32 pause); /* stplnmet_confirm_pause */
    /* 0xE0 */ void (*close)(struct StplnmetConfirm *obj);            /* stplnmet_confirm_close */
} StplnmetConfirm; /* size 0xE4 */

/* The confirmation's data block (0x20 bytes). */
typedef struct StplnmetConfirmData {
    /* 0x00 */ MessageWindow *title;  /* title */
    /* 0x04 */ MessageWindow *name;   /* the name */
    /* 0x08 */ MessageWindow *partner_set; /* the partner set */
    /* 0x0C */ MessageWindow *question; /* the question */
    /* 0x10 */ MessageWindow *bar_text; /* the bar's text */
    /* 0x14 */ MessageWindow *yes;    /* yes */
    /* 0x18 */ MessageWindow *no;     /* no */
    /* 0x1C */ MessageCursor *cursor; /* the cursor */
} StplnmetConfirmData; /* size 0x20 */

/* The partner choice (stplnmet_partner_create, size 0x8C). */
typedef struct StplnmetPartner {
    /* 0x00 */ Object base;
    /* 0x50 */ StplnmetMain *main;
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 ot_depth; /* ordering table depth */
    /* 0x5C */ s32 frames[3]; /* per Digimon of the set: its animation frame */
    /* 0x68 */ s32 frame_time; /* time of their last frame */
    /* 0x6C */ s32 set;    /* the set chosen (0..2) */
    /* 0x70 */ s32 cursor_step; /* the cursor's colour step */
    /* 0x74 */ s32 cursor_time; /* time of its last step */
    /* 0x78 */ s32 unk_78;
    /* 0x7C */ s32 unk_7C;
    /* 0x80 */ s32 unk_80;
    /* 0x84 */ s32 unk_84;
    /* 0x88 */ void (*pause)(struct StplnmetPartner *obj, s32 pause); /* stplnmet_partner_pause */
} StplnmetPartner; /* size 0x8C */

/* A Digimon of the partner set shown: its windows. */
typedef struct StplnmetPartnerLine {
    /* 0x0 */ MessageWindow *name; /* the Digimon's name */
    /* 0x4 */ MessageWindow *description; /* its description */
} StplnmetPartnerLine; /* size 0x8 */

/* The partner choice's data block (0x30 bytes). */
typedef struct StplnmetPartnerData {
    /* 0x00 */ MessageWindow *title;  /* title */
    /* 0x04 */ MessageWindow *sets[3];   /* the sets */
    /* 0x10 */ MessageWindow *set_name; /* the set's name */
    /* 0x14 */ MessageWindow *description; /* its description */
    /* 0x18 */ StplnmetPartnerLine digimon[3];
} StplnmetPartnerData; /* size 0x30 */

/* The main object's data block (0x28 bytes). */
typedef struct StplnmetMainData {
    /* 0x00 */ Object *opening; /* the opening message (cleared when it ends) */
    /* 0x04 */ StplnmetNameEntry *name_entry;
    /* 0x08 */ StplnmetPartner *partner;
    /* 0x0C */ StplnmetConfirm *confirm; /* (cleared when it ends) */
    /* 0x10 */ StplnmetBg *bg;
    /* 0x14 */ Object *sparkle;
    /* 0x18 */ Object *figure;
    /* 0x1C */ MessageWindow *tabs[3];   /* the tabs */
} StplnmetMainData; /* size 0x28 */

extern s32 stplnmet_partner_cursor_palettes[6];
extern s32 stplnmet_tab_x[3];

void stplnmet_confirm_show_text(StplnmetConfirm *obj, StplnmetConfirmData *data, s32 show);
void stplnmet_partner_show_text(StplnmetPartner *obj, StplnmetPartnerData *data, s32 show);

/* Creates the confirmation's text windows and cursor. */
void stplnmet_confirm_create_windows(StplnmetConfirm *obj, StplnmetConfirmData *data) {
    MessageWindow *win;
    MessageCursor *cursor;

    data->title = win = message_create_window(obj->layer_id, 1, 0x20, 0x1A);
    win->set_palette(win, 4);
    data->name = win = message_create_window(obj->layer_id, 1, 0x61, 0x42);
    win->set_palette(win, 1);
    data->partner_set = win = message_create_window(obj->layer_id, 1, 0x40, 0x60);
    win->set_palette(win, 1);
    data->question = win = message_create_window(obj->layer_id, 1, 0x5A, 0xA9);
    win->set_palette(win, 1);
    data->yes = win = message_create_window(obj->layer_id, 1, 0x6B, 0xB9);
    win->set_palette(win, 1);
    data->no = win = message_create_window(obj->layer_id, 1, 0x6B, 0xC7);
    win->set_palette(win, 1);
    data->cursor = cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 2, 0x5A, 0xB9);
    cursor->set_palette(cursor, 1);
    data->cursor->show(data->cursor, 0);
    data->bar_text = win = message_create_window(obj->layer_id, 1, 0xE3, 0xA9);
    win->set_palette(win, 1);
}

/* Shows (`show`) or hides the confirmation's texts. */
void stplnmet_confirm_show_text(StplnmetConfirm *obj, StplnmetConfirmData *data, s32 show) {
    u16 *name;
    s32 i;

    if (show) {
        data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x8D), 0x15);
        name = obj->name;
        for (i = 0; name[i] == 0x4081; i++) {
        }
        data->name->copy_text(data->name, (u8 *)&name[i]);
        data->partner_set->set_text(data->partner_set, cdload_module.files.get_file(records_language + 0x8D), obj->partner_set + 0xA);
        data->question->set_text(data->question, cdload_module.files.get_file(records_language + 0x8D), 0x16);
        data->yes->set_text(data->yes, cdload_module.files.get_file(records_language + 0x8D), 0x17);
        data->no->set_text(data->no, cdload_module.files.get_file(records_language + 0x8D), 0x18);
        data->cursor->set_pos(data->cursor, 0x5A, obj->cursor * 14 + 0xB9);
        data->cursor->show(data->cursor, 1);
    } else {
        data->title->set_visible(data->title, 0);
        data->name->set_visible(data->name, 0);
        data->partner_set->set_visible(data->partner_set, 0);
        data->question->set_visible(data->question, 0);
        data->yes->set_visible(data->yes, 0);
        data->no->set_visible(data->no, 0);
        data->cursor->show(data->cursor, 0);
        data->bar_text->set_visible(data->bar_text, 0);
    }
}

/* Draws the confirmation: its frame, the partner set's Digimon, the window and the bar. */
void stplnmet_confirm_draw(StplnmetConfirm *obj) {
    Sprite spr;
    SVECTOR out[4];
    SVECTOR in[4];
    s32 *set;
    StplnmetDigimonAnim *anim;
    s32 i;
    s32 j;
    GfxLayer *layer;
    u32 *ot;
    POLY_G4 *poly;

    set = stplnmet_funcs.partners[obj->partner_set];
    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0x100);
    if (gfx_module.funcs.get_time() - obj->frame_anim_time >= 5) {
        obj->frame_anim_time = gfx_module.funcs.get_time();
        if (++obj->frame_anim_frame >= 14) {
            obj->frame_anim_frame = 0;
        }
    }
    if (obj->frame_anim.level != 0) {
        if (obj->frame_anim.level != 0x1000) {
            spr.set_scale(obj->frame_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x37, 0x42);
        }
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        spr.set_palette(obj->frame_anim_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), 6, 0x37, 0x2F);
        spr.set_palette(0);
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), 2, 0x37, 0x2F);
    }
    if (gfx_module.funcs.get_time() - obj->frame_time >= 13) {
        obj->frame_time = gfx_module.funcs.get_time();
        for (i = 0; i < 3; i++) {
            anim = &stplnmet_funcs.anims[set[i]];
            if (++obj->frames[i] >= 8 || anim->sprites[obj->frames[i]] == -1) {
                obj->frames[i] = 0;
            }
        }
    }
    if (obj->window_anim.level != 0) {
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        if (obj->window_anim.level == 0x1000) {
            spr.set_vram_pos(0x140, 0x100);
            for (i = 0; i < 3; i++) {
                anim = &stplnmet_funcs.anims[set[i]];
                spr.draw(cdload_module.get_subfile_by_id(0x07700000), anim->sprites[obj->frames[i]], i * 0x38 + 0x57, 0x6C);
            }
        }
        spr.set_vram_pos(0x280, 0x100);
        if (obj->window_anim.level != 0x1000) {
            spr.set_scale(0x1000, obj->window_anim.level, 0x1000);
            spr.set_pivot(0xA0, 0x78);
        }
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), 0xA, 0x37, 0x59);
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), 9, 0x37, 0x59);
        if (obj->window_anim.level != 0x1000) {
            spr.set_pivot(0xA0, 0xBE);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), 1, 0, 0xA2);
    }
    if (obj->base.step == 1 || obj->base.step == 2) {
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), 0x37, 0x51, 0xBF);
        obj->bar_scale.vy = obj->bar_scale.vz = 0x1000;
        if (obj->bar == 100) {
            obj->bar_scale.vx = 0x1000;
        } else {
            obj->bar_scale.vx = obj->bar * 41;
        }
        RotMatrixYXZ_gte(&obj->bar_rotation, &obj->bar_matrix);
        ScaleMatrix(&obj->bar_matrix, &obj->bar_scale);
        layer = gfx_module.funcs.get_layer(obj->layer_id);
        ot = layer->get_ot_entry(layer, obj->ot_depth - 1);
        poly = gfx_module.funcs.get_packet();
        setPolyG4(poly);
        setRGB0(poly, 0x7F, 0x32, 0xF2);
        setRGB1(poly, 0xD1, 0x2F, 0xDE);
        setRGB2(poly, 0x7F, 0x32, 0xF2);
        setRGB3(poly, 0xD1, 0x2F, 0xDE);
        in[1].vx = in[3].vx = 0x98;
        in[0].vy = in[1].vy = 0xBC;
        in[0].vx = in[2].vx = 0;
        in[2].vy = in[3].vy = 0xC8;
        in[0].vz = in[1].vz = in[2].vz = in[3].vz = 0;
        for (j = 0; j < 4; j++) {
            ApplyMatrixSV(&obj->bar_matrix, &in[j], &out[j]);
            out[j].vx += 0x54;
            out[j].vy += 6;
        }
        poly->x0 = out[0].vx;
        poly->y0 = out[0].vy;
        poly->x1 = out[1].vx;
        poly->y1 = out[1].vy;
        poly->x2 = out[2].vx;
        poly->y2 = out[2].vy;
        poly->x3 = out[3].vx;
        poly->y3 = out[3].vy;
        addPrim(ot, poly);
        gfx_module.funcs.set_packet(poly + 1);
    }
}

/* The confirmation's input and steps (base.step): yes fills the bar, no or cancel goes back. */
void stplnmet_confirm_run(StplnmetConfirm *obj, StplnmetConfirmData *data) {
    s32 old;

    switch (obj->base.step) {
    case 0:
    default:
        old = obj->cursor;
        if (PAD_PRESSED(4)) {
            obj->cursor = 0;
        } else if (PAD_PRESSED(6)) {
            obj->cursor = 1;
        }
        if (old != obj->cursor) {
            sound_module.play(0x8004513E);
            data->cursor->set_pos(data->cursor, 0x5A, obj->cursor * 14 + 0xB9);
        }
        if (PAD_PRESSED(0xD)) {
            sound_module.play(0x8004503C);
            if (obj->cursor == 0) {
                obj->base.step = 1;
                obj->bar = 0;
                data->yes->set_visible(data->yes, 0);
                data->no->set_visible(data->no, 0);
                data->cursor->show(data->cursor, 0);
                data->question->set_text(data->question, cdload_module.files.get_file(records_language + 0x8D), 0x1D);
                data->bar_text->set_text(data->bar_text, cdload_module.files.get_file(records_language + 0x8D), 0x19);
                data->bar_text->set_line_number(data->bar_text, 1, obj->bar);
                data->bar_text->measure(data->bar_text, 1);
            } else {
                obj->base.step = 200;
            }
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            obj->base.step = 200;
        }
        break;
    case 1:
        if (++obj->bar > 100) {
            obj->bar = 100;
            data->question->set_text(data->question, cdload_module.files.get_file(records_language + 0x8D), 0x1E);
            sound_module.play(0x80045341);
            obj->base.next_step(obj);
        } else {
            sound_module.play(0x800452C6);
        }
        data->bar_text->set_line_number(data->bar_text, 1, obj->bar);
        data->bar_text->measure(data->bar_text, 1);
        break;
    case 2:
        if (++obj->base.substep > 120) {
            obj->base.step = 100;
        }
        break;
    case 50:
        stplnmet_funcs.anim_start(&obj->frame_anim, 0);
        stplnmet_funcs.anim_start(&obj->window_anim, 0);
        stplnmet_confirm_show_text(obj, data, 0);
        obj->base.step++;
        break;
    case 51:
        stplnmet_funcs.anim_update(&obj->frame_anim);
        if (stplnmet_funcs.anim_update(&obj->window_anim)) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 100:
    case 200:
        break;
    }
}

/* Pauses (hides) or resumes the confirmation (StplnmetConfirm.pause). */
void stplnmet_confirm_pause(StplnmetConfirm *obj, s32 pause) {
    StplnmetConfirmData *data = (StplnmetConfirmData *)obj->base.children;
    s32 i;

    if (pause) {
        obj->base.state = OBJECT_STATE_DONE;
        stplnmet_confirm_show_text(obj, data, 0);
    } else {
        obj->base.state = OBJECT_STATE_RUN;
        obj->base.step = 0;
        for (i = 2; i >= 0; i--) {
            obj->frames[i] = 0;
        }
        obj->cursor = 0;
        stplnmet_confirm_show_text(obj, data, 1);
    }
}

/* Closes the confirmation (StplnmetConfirm.close). */
void stplnmet_confirm_close(StplnmetConfirm *obj) {
    obj->base.step = 50;
}

void stplnmet_confirm_update(StplnmetConfirm *obj, StplnmetConfirmData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stplnmet_confirm_create_windows(obj, data);
        obj->frame_anim.duration = 10;
        obj->frame_anim.level = 0x1000;
        obj->window_anim.duration = 10;
        obj->window_anim.level = 0x1000;
        stplnmet_confirm_pause(obj, 1);
        break;
    case OBJECT_STATE_RUN:
        stplnmet_confirm_run(obj, data);
        stplnmet_confirm_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StplnmetConfirm *stplnmet_confirm_create(StplnmetMain *main) {
    StplnmetConfirm *obj = object_new(stplnmet_confirm_update, sizeof(StplnmetConfirm), sizeof(StplnmetConfirmData));

    obj->pause = stplnmet_confirm_pause;
    obj->close = stplnmet_confirm_close;
    obj->layer_id = 0x1001;
    obj->ot_depth = 6;
    obj->main = main;
    return obj;
}

/* Creates the partner choice's text windows. */
void stplnmet_partner_create_windows(StplnmetPartner *obj, StplnmetPartnerData *data) {
    s32 i;
    MessageWindow *win;

    data->title = win = message_create_window(obj->layer_id, 1, 0x20, 0x1A);
    win->set_palette(win, 4);
    for (i = 0; i < 3; i++) {
        data->sets[i] = win = message_create_window(obj->layer_id, 1, 0x1A, i * 0x18 + 0x34);
        win->set_palette(win, 1);
    }
    data->set_name = win = message_create_window(obj->layer_id, 1, 0x3A, 0x32);
    win->set_palette(win, 4);
    data->description = win = message_create_window(obj->layer_id, 1, 0x3A, 0xC2);
    win->set_page_lines(win, 3);
    data->description->set_palette(data->description, 1);
    for (i = 0; i < 3; i++) {
        data->digimon[i].name = win = message_create_window(obj->layer_id, 2, 0x72, i * 0x2A + 0x43);
        win->set_palette(win, 5);
        data->digimon[i].description = win = message_create_window(obj->layer_id, 1, 0x72, i * 0x2A + 0x4F);
        win->set_page_lines(win, 2);
        data->digimon[i].description->set_palette(data->digimon[i].description, 1);
    }
}

/* Shows (`show`) or hides the partner choice's texts. */
void stplnmet_partner_show_text(StplnmetPartner *obj, StplnmetPartnerData *data, s32 show) {
    s32 *set;
    s32 i;

    if (show) {
        set = stplnmet_funcs.partners[obj->set];
        data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x8D), 6);
        for (i = 0; i < 3; i++) {
            data->sets[i]->set_text(data->sets[i], cdload_module.files.get_file(records_language + 0x8D), i + 7);
        }
        data->set_name->set_text(data->set_name, cdload_module.files.get_file(records_language + 0x8D), obj->set + 0xA);
        data->description->set_text(data->description, cdload_module.files.get_file(records_language + 0x8D), obj->set + 0x1A);
        for (i = 0; i < 3; i++) {
            data->digimon[i].name->set_text(data->digimon[i].name, cdload_module.files.get_file(records_language + 0x4E),
                                           set[i] + 1);
            data->digimon[i].description->set_text(data->digimon[i].description, cdload_module.files.get_file(records_language + 0x8D),
                                           set[i] + 0xD);
        }
    } else {
        data->title->set_visible(data->title, 0);
        for (i = 0; i < 3; i++) {
            data->sets[i]->set_visible(data->sets[i], 0);
        }
        data->set_name->set_visible(data->set_name, 0);
        data->description->set_visible(data->description, 0);
        for (i = 0; i < 3; i++) {
            data->digimon[i].name->set_visible(data->digimon[i].name, 0);
            data->digimon[i].description->set_visible(data->digimon[i].description, 0);
        }
    }
}

/* Draws the partner choice: the set's Digimon, the frame and the cursor. */
void stplnmet_partner_draw(StplnmetPartner *obj) {
    Sprite spr;
    s32 *set;
    StplnmetDigimonAnim *anim;
    s32 i;

    set = stplnmet_funcs.partners[obj->set];
    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (gfx_module.funcs.get_time() - obj->frame_time >= 13) {
        obj->frame_time = gfx_module.funcs.get_time();
        for (i = 0; i < 3; i++) {
            anim = &stplnmet_funcs.anims[set[i]];
            if (++obj->frames[i] >= 8 || anim->sprites[obj->frames[i]] == -1) {
                obj->frames[i] = 0;
            }
        }
    }
    spr.set_vram_pos(0x140, 0x100);
    for (i = 0; i < 3; i++) {
        anim = &stplnmet_funcs.anims[set[i]];
        spr.draw(cdload_module.get_subfile_by_id(0x07700000), anim->sprites[obj->frames[i]], 0x41, i * 0x2A + 0x40);
    }
    spr.set_vram_pos(0x280, 0x100);
    spr.draw(cdload_module.get_subfile_by_id(0x028A0000), 7, 0x10, 0x2C);
    spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
    spr.draw(cdload_module.get_subfile_by_id(0x028A0000), 8, 0x10, 0x2C);
    if (gfx_module.funcs.get_time() - obj->cursor_time >= 9) {
        obj->cursor_time = gfx_module.funcs.get_time();
        if (++obj->cursor_step >= 6) {
            obj->cursor_step = 0;
        }
    }
    spr.set_palette(stplnmet_partner_cursor_palettes[obj->cursor_step]);
    spr.draw(cdload_module.get_subfile_by_id(0x028A0000), 0x32, 0x10, obj->set * 0x18 + 0x30);
}

/* The partner choice's input: up/down chooses a set, the button confirms (100), cancel goes back (200). */
void stplnmet_partner_run(StplnmetPartner *obj, StplnmetPartnerData *data) {
    s32 old;
    s32 i;

    old = obj->set;
    if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
        if (--obj->set < 0) {
            obj->set = 0;
        }
    } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
        if (++obj->set >= 3) {
            obj->set = 2;
        }
    }
    if (old != obj->set) {
        sound_module.play(0x4001B);
        stplnmet_partner_show_text(obj, data, 1);
        for (i = 2; i >= 0; i--) {
            obj->frames[i] = 0;
        }
    }
    if (PAD_PRESSED(0xD)) {
        sound_module.play(0x4001C);
        obj->base.step = 100;
    } else if (PAD_PRESSED(0xE)) {
        sound_module.play(0x800450BD);
        obj->base.step = 200;
        obj->set = 0;
    }
}

/* Pauses (hides) or resumes the partner choice (StplnmetPartner.pause). */
void stplnmet_partner_pause(StplnmetPartner *obj, s32 pause) {
    StplnmetPartnerData *data = (StplnmetPartnerData *)obj->base.children;

    if (pause) {
        obj->base.state = OBJECT_STATE_DONE;
        stplnmet_partner_show_text(obj, data, 0);
    } else {
        obj->base.state = OBJECT_STATE_RUN;
        obj->base.step = 0;
        stplnmet_partner_show_text(obj, data, 1);
    }
}

void stplnmet_partner_update(StplnmetPartner *obj, StplnmetPartnerData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stplnmet_partner_create_windows(obj, data);
        stplnmet_partner_pause(obj, 1);
        break;
    case OBJECT_STATE_RUN:
        stplnmet_partner_run(obj, data);
        stplnmet_partner_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StplnmetPartner *stplnmet_partner_create(StplnmetMain *main) {
    StplnmetPartner *obj = object_new(stplnmet_partner_update, sizeof(StplnmetPartner), sizeof(StplnmetPartnerData));

    obj->pause = stplnmet_partner_pause;
    obj->layer_id = 0x1001;
    obj->ot_depth = 6;
    obj->main = main;
    return obj;
}

/* Creates the three tabs. */
void stplnmet_main_create_windows(StplnmetMain *obj, StplnmetMainData *data) {
    s32 i;

    for (i = 0; i < 3; i++) {
        data->tabs[i] = message_create_window(obj->layer_id, 3, stplnmet_tab_x[i], 0x12);
        data->tabs[i]->set_text(data->tabs[i], cdload_module.files.get_file(records_language + 0x8D), i + 3);
        data->tabs[i]->set_palette(data->tabs[i], 1);
        data->tabs[i]->set_visible(data->tabs[i], 0);
    }
}

/* Shows (`show`) or hides the tabs; the current step's tab is highlighted. */
void stplnmet_main_show_tabs(StplnmetMain *obj, StplnmetMainData *data, s32 show) {
    s32 i;

    for (i = 0; i < 3; i++) {
        data->tabs[i]->set_visible(data->tabs[i], show);
        if (obj->tab == i) {
            data->tabs[i]->set_palette(data->tabs[i], 0);
        } else {
            data->tabs[i]->set_palette(data->tabs[i], 1);
        }
    }
}

/* The main object's steps (base.step, then base.substep for the tabs): name, partner, confirmation. */
void stplnmet_main_run(StplnmetMain *obj, StplnmetMainData *data) {
    switch (obj->base.step) {
    case 0:
        data->bg = stplnmet_bg_create();
        data->bg->load(data->bg, 0x1C0, 0x100);
        data->bg->set_layer(data->bg, obj->layer_id, 6);
        data->sparkle = stplnmet_sparkle_create();
        data->figure = stplnmet_figure_create();
        data->opening = stplnmet_message_create(obj);
        obj->base.step++;
        break;
    case 1:
        if (data->opening == NULL) {
            stplnmet_funcs.anim_start(&obj->frame_anim, 1);
            data->name_entry = stplnmet_entry_create(gamestate_data.name);
            data->partner = stplnmet_partner_create(obj);
            data->confirm = stplnmet_confirm_create(obj);
            obj->tab = 0;
            obj->base.next_step(obj);
        }
        break;
    case 2:
        switch (obj->base.substep) {
        case 0:
        default:
            if (stplnmet_funcs.anim_update(&obj->frame_anim)) {
                stplnmet_main_show_tabs(obj, data, 1);
            }
            if (data->name_entry->base.step == 100) {
                data->name_entry->pause(data->name_entry, 1);
                data->partner->pause(data->partner, 0);
                obj->tab = 1;
                stplnmet_main_show_tabs(obj, data, 1);
                obj->base.substep++;
            }
            break;
        case 1:
            if (data->partner->base.step == 100) {
                data->partner->pause(data->partner, 1);
                data->confirm->partner_set = data->partner->set;
                data->confirm->name = data->name_entry->name;
                data->confirm->pause(data->confirm, 0);
                obj->tab = 2;
                stplnmet_main_show_tabs(obj, data, 1);
                obj->base.substep++;
            } else if (data->partner->base.step == 200) {
                data->partner->pause(data->partner, 1);
                data->name_entry->pause(data->name_entry, 0);
                obj->tab = 0;
                stplnmet_main_show_tabs(obj, data, 1);
                obj->base.substep--;
            }
            break;
        case 2:
            if (data->confirm->base.step == 100) {
                data->confirm->close(data->confirm);
                stplnmet_funcs.anim_start(&obj->frame_anim, 0);
                stplnmet_main_show_tabs(obj, data, 0);
                obj->base.substep++;
            } else if (data->confirm->base.step == 200) {
                data->confirm->pause(data->confirm, 1);
                data->partner->pause(data->partner, 0);
                obj->tab = 1;
                stplnmet_main_show_tabs(obj, data, 1);
                obj->base.substep--;
            }
            break;
        case 3:
            if (stplnmet_funcs.anim_update(&obj->frame_anim) && data->confirm == NULL) {
                data->name_entry->get_name(data->name_entry, gamestate_data.name);
                gamestate_data.funcs.set_party(data->partner->set);
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        }
        break;
    }
}

/* Draws the main object's frame. */
void stplnmet_main_draw(StplnmetMain *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0x100);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->frame_anim.level != 0) {
        if (obj->frame_anim.level != 0x1000) {
            spr.set_scale(obj->frame_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x18, 0x20);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), obj->tab + 0xC, 0x18, 0x15);
    }
}

void stplnmet_main_update(StplnmetMain *obj, StplnmetMainData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            stplnmet_funcs.load();
            sound_module.load_extra_bank(0x20);
            obj->base.step++;
            break;
        case 1:
            if (stplnmet_funcs.is_loading() == 0) {
                stplnmet_main_create_windows(obj, data);
                obj->frame_anim.duration = 10;
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        stplnmet_main_run(obj, data);
        stplnmet_main_draw(obj);
        break;
    case OBJECT_STATE_DONE:
        if (sound_module.is_loading() == 0) {
            sound_module.play(0x60800000);
            obj->base.set_state(obj, OBJECT_STATE_RUN);
        }
        break;
    case OBJECT_STATE_END:
        sound_module.stop(0x60800000);
        gamestate_data.funcs.set_next_map(0x2D8, 0);
        break;
    }
}

StplnmetMain *stplnmet_main_create(void) {
    StplnmetMain *obj = object_new(stplnmet_main_update, sizeof(StplnmetMain), sizeof(StplnmetMainData));

    obj->layer_id = 0x1001;
    obj->ot_depth = 5;
    return obj;
}

/* Starts loading the overlay's files (stplnmet_funcs.load). */
void stplnmet_load_files(void) {
    Tim tim;

    tim_init(&tim);
    tim.set_image_pos(0x280, 0x100);
    tim.load_all(cdload_module.get_subfile_by_id(0x028B0000));
    cdload_module.queue_file(records_language + 0x8D);
    cdload_module.queue_file(records_language + 0x4E);
    cdload_module.queue_file(records_language + 0x86);
    cdload_module.queue_file(0x771);
}

/* Returns non-zero while one of the overlay's files is still loading (stplnmet_funcs.is_loading). */
s32 stplnmet_is_loading(void) {
    if (!cdload_module.is_loading(records_language + 0x8D) && !cdload_module.is_loading(records_language + 0x4E) &&
        !cdload_module.is_loading(records_language + 0x86)) {
        return cdload_module.is_loading(0x771) != 0;
    }
    return 1;
}

/* window_anim_start (include/window_anim.h), byte for byte (stplnmet_funcs.anim_start). */
void stplnmet_anim_start(WindowAnim *anim, s32 open) {
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

/* window_anim_update (include/window_anim.h), byte for byte (stplnmet_funcs.anim_update). */
s32 stplnmet_anim_update(WindowAnim *anim) {
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

/* stplnmet_funcs.tween_start (unused here). */
void stplnmet_tween_start(Tween *obj, s32 from, s32 to, s32 frames) {
    if (from != to) {
        obj->duration = frames;
        obj->acc = from << 8;
        obj->value = from;
        obj->target = to;
        obj->running = 1;
        obj->step = ((to - from) << 8) / obj->duration;
    }
}

/* stplnmet_funcs.tween_update (unused here). */
s32 stplnmet_tween_update(Tween *obj) {
    if (obj->running == 0) {
        return 1;
    }
    obj->acc += obj->step;
    obj->value = obj->acc >> 8;
    if (obj->step > 0) {
        if (obj->value > obj->target) {
            obj->value = obj->target;
            obj->running = 0;
            return 1;
        }
    } else if (obj->value < obj->target) {
        obj->value = obj->target;
        obj->running = 0;
        return 1;
    }
    return 0;
}

s32 stplnmet_partner_cursor_palettes[6] = { 0, 1, 2, 3, 2, 1 };

s32 stplnmet_tab_x[3] = { 228, 256, 284 };

StplnmetDigimonAnim stplnmet_digimon_anims[8] = {
    { { 7, 8, 9, 10, 9, 8, -1 } },
    { { 14, 15, 16, 15, -1, -1, -1 } },
    { { 11, 12, 13, 12, -1, -1, -1 } },
    { { 3, 4, 5, 6, 5, 4, -1 } },
    { { 25, 26, 27, 28, 27, 26, -1 } },
    { { 0, 1, 2, 1, -1, -1, -1 } },
    { { 17, 18, 19, 20, 19, 18, -1 } },
    { { 21, 22, 23, 24, 23, 22, -1 } },
};

s32 stplnmet_partner_sets[3][3] = {
    { 0, 6, 7 },
    { 2, 3, 6 },
    { 1, 5, 7 },
};

StplnmetFuncs stplnmet_funcs = {
    stplnmet_digimon_anims,
    stplnmet_partner_sets,
    stplnmet_load_files,
    stplnmet_is_loading,
    stplnmet_anim_start,
    stplnmet_anim_update,
    stplnmet_tween_start,
    stplnmet_tween_update,
};
