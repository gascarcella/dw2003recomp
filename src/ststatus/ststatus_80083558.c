#include "common.h"
#include "object.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "ststatus.h"

/* The overlay's entry point and first object, then the party page (ststatus_create_party_page) and its fade. */

/* The party page's data block (ststatus_create_party_page, 0xA0 bytes): its text windows and its fade. */
typedef struct StstatusPartyPageData {
    /* 0x00 */ StstatusPanelWindows panels[3]; /* per party member: its panel */
    /* 0x84 */ MessageWindow *help;
    /* 0x88 */ MessageWindow *back_hint;
    /* 0x8C */ MessageWindow *menu_title; /* menu title */
    /* 0x90 */ MessageWindow *menu_choices[2]; /* menu choices */
    /* 0x98 */ MessageCursor *menu_cursor; /* menu cursor */
    /* 0x9C */ Fade *fade;   /* fade */
} StstatusPartyPageData; /* size 0xA0 */

/* The party page (ststatus_create_party_page, size 0xD4): a panel per party member, then a two-choice menu. */
typedef struct StstatusPartyPage {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 parent;
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 ot_depth; /* ordering table depth */
    /* 0x5C */ s32 member_count; /* party members */
    /* 0x60 */ s32 frames[3]; /* per member: sprite frame */
    /* 0x6C */ s32 frame_time; /* time of the last frame */
    /* 0x70 */ s32 menu_cursor; /* menu cursor, 0..1 */
    /* 0x74 */ WindowAnim member_panels[3]; /* member panels */
    /* 0xA4 */ WindowAnim bars[2];
    /* 0xC4 */ WindowAnim menu_anim;
} StstatusPartyPage; /* size 0xD4 */

s32 ststatus_party_panel_stats[5] = { 0, 2, 3, 4, 5 };

Object *ststatus_create_root(void);
Fade *ststatus_create_fade(void);

/* The overlay's first object: sets up the display and creates the root object. */
void ststatus_update_main(Object *obj, Object **data) {
    RECT rect;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0x14000);
        gfx_module.funcs.init_display(320, 240, 0, 0);
        rect.x = 0;
        rect.y = 0;
        rect.w = 320;
        rect.h = 240;
        layer = gfx_module.funcs.create_layer(&rect, 3, 0x1000);
        layer->set_bg_color(layer, 0, 0, 0);
        *data = ststatus_create_root();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The overlay's entry point (overlay_entries). */
Object *ststatus_start(void) {
    return object_new(ststatus_update_main, sizeof(Object), 4);
}

/* Creates the page's text windows. */
void ststatus_party_create_windows(StstatusPartyPage *obj, StstatusPartyPageData *data) {
    s32 i;
    s32 j;
    StstatusLayout *l;
    MessageWindow **child;

    for (i = 0; i < 3; i++) {
        l = ststatus_module.party_layout;
        data->panels[i].name = message_create_window(obj->layer_id, 1, l->x, l->y + i * 0x2E);
        l = &ststatus_module.party_layout[1];
        for (j = 0; j < 5; j++, l++) {
            data->panels[i].labels[j] = message_create_window(obj->layer_id, 3, l->x, l->y + i * 0x2E);
        }
        l = &ststatus_module.party_layout[6];
        for (j = 0; j < 5; j++, l++) {
            data->panels[i].values[j] = message_create_window(obj->layer_id, 3, l->x, l->y + i * 0x2E);
        }
    }
    l = &ststatus_module.party_layout[12];
    data->help = message_create_window(obj->layer_id, 1, l->x, l->y);
    data->help->set_page_lines(data->help, 2);
    data->back_hint = message_create_window(obj->layer_id, 1, l->x, l->y + 0xE);
    data->menu_title = message_create_window(obj->layer_id, 1, 0xB2, 0x2A);
    for (j = 0; j < 2; j++) {
        data->menu_choices[j] = message_create_window(obj->layer_id, 1, 0xC5, j * 0xE + 0x3A);
    }
    data->menu_cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0xB8, 0x3A);
    data->menu_cursor->show(data->menu_cursor, 0);
    child = (MessageWindow **)obj->base.children;
    for (j = 0; j < obj->base.child_count - 2; j++, child++) {
        (*child)->set_ot_depth(*child, obj->ot_depth - 1);
    }
}

/* Shows (fills in) or hides a member's panel. */
void ststatus_party_show_member(StstatusPartyPage *obj, StstatusPartyPageData *data, s32 member, s32 show) {
    GamestateStats stats;
    s32 digimon;
    s32 i;
    MessageWindow *win;
    StstatusLayout *l;

    if (show) {
        digimon = gamestate_data.funcs.get_party_member(member);
        gamestate_data.funcs.get_stats(digimon, &stats);
        data->panels[member].name->set_text(data->panels[member].name,
                                             (u8 *)gamestate_data.funcs.get_record(digimon), -1);
        l = &ststatus_module.party_layout[1];
        for (i = 0; i < 5; i++, l++) {
            data->panels[member].labels[i]->set_text(data->panels[member].labels[i],
                                                    cdload_module.files.get_file(records_language + 0xB0), l->text);
        }
        for (i = 0; i < 5; i++) {
            win = data->panels[member].values[i];
            win->set_line_number(win, 0, stats.values[ststatus_party_panel_stats[i]]);
            data->panels[member].values[i]->measure(data->panels[member].values[i], 1);
        }
    } else {
        data->panels[member].name->set_visible(data->panels[member].name, 0);
        for (i = 0; i < 5; i++) {
            data->panels[member].labels[i]->set_visible(data->panels[member].labels[i], 0);
        }
        for (i = 0; i < 5; i++) {
            data->panels[member].values[i]->set_visible(data->panels[member].values[i], 0);
        }
    }
}

/* Shows or hides the menu. */
void ststatus_party_show_menu(StstatusPartyPage *obj, StstatusPartyPageData *data, s32 show) {
    s32 i;

    if (show) {
        data->menu_title->set_text(data->menu_title, cdload_module.files.get_file(records_language + 0xB0), 0xB);
        for (i = 0; i < 2; i++) {
            data->menu_choices[i]->set_text(data->menu_choices[i], cdload_module.files.get_file(records_language + 0xB0), i + 0x4A);
        }
        data->menu_cursor->show(data->menu_cursor, 1);
    } else {
        data->menu_title->set_visible(data->menu_title, 0);
        for (i = 0; i < 2; i++) {
            data->menu_choices[i]->set_visible(data->menu_choices[i], 0);
        }
        data->menu_cursor->show(data->menu_cursor, 0);
    }
}

/* Draws the members' sprites and the panels' frames. */
void ststatus_party_draw(StstatusPartyPage *obj) {
    Sprite spr;
    s32 i;
    s32 digimon;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (gfx_module.funcs.get_time() - obj->frame_time >= 13) {
        obj->frame_time = gfx_module.funcs.get_time();
        for (i = 0; i < obj->member_count; i++) {
            digimon = gamestate_data.funcs.get_party_member(i);
            obj->frames[i]++;
            if (ststatus_module.anims[digimon].frame[obj->frames[i]] == -1 ||
                obj->frames[i] >= 7) {
                obj->frames[i] = 0;
            }
        }
    }
    for (i = 0; i < obj->member_count; i++) {
        if (obj->member_panels[i].level != 0) {
            if (obj->member_panels[i].level != 0x1000) {
                spr.set_scale(obj->member_panels[i].level, obj->member_panels[i].level, 0x1000);
                spr.set_pivot(0x7C, i * 0x2E + 0x27);
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
            }
            digimon = gamestate_data.funcs.get_party_member(i);
            spr.set_vram_pos(0x280, 0x100);
            spr.draw(cdload_module.get_subfile_by_id(0x04040000), ststatus_module.anims[digimon].frame[obj->frames[i]], 0x6B,
                       i * 0x2E + 0x13);
        }
    }
    spr.set_vram_pos(0x140, 0);
    for (i = 0; i < obj->member_count; i++) {
        if (obj->member_panels[i].level != 0) {
            if (obj->member_panels[i].level != 0x1000) {
                spr.set_scale(obj->member_panels[i].level, 0x1000, 0x1000);
                spr.set_pivot(0, i * 0x2E + 0x25);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x15, 0, i * 0x2E + 0x11);
                spr.set_scale(obj->member_panels[i].level, obj->member_panels[i].level, 0x1000);
                spr.set_pivot(0x7C, i * 0x2E + 0x27);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x16, 0x67, i * 0x2E + 0x13);
                spr.set_scale(obj->member_panels[i].level, 0x1000, 0x1000);
                spr.set_pivot(0, i * 0x2E + 0x25);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x17, 0, i * 0x2E + 0x11);
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x15, 0, i * 0x2E + 0x11);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x16, 0x67, i * 0x2E + 0x13);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x17, 0, i * 0x2E + 0x11);
            }
        }
    }
    if (obj->bars[1].level != 0) {
        if (obj->bars[1].level != 0x1000) {
            spr.set_scale(obj->bars[1].level, 0x1000, 0x1000);
            spr.set_pivot(0, 0xD3);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x20, 0, 0xC2);
    }
    if (obj->menu_anim.level != 0) {
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        spr.set_vram_pos(0x280, 0x100);
        if (obj->menu_anim.level != 0x1000) {
            spr.set_scale(obj->menu_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x40);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x3A, 0xA8, 0x28);
    }
}

/* The page's states: open the panels one by one, run the menu, close them again, then leave. */
void ststatus_party_run(StstatusPartyPage *obj, StstatusPartyPageData *data) {
    s32 old;

    switch (obj->base.step) {
    case 0:
    default:
        ststatus_module.window_anim_start(&obj->member_panels[0], 1);
        if (obj->member_count == 1) {
            ststatus_module.window_anim_start(&obj->bars[1], 1);
            ststatus_module.window_anim_start(&obj->menu_anim, 1);
        }
        obj->base.step = obj->member_count;
        break;
    case 1:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        ststatus_module.window_anim_update(&obj->menu_anim);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_party_show_member(obj, data, 0, 1);
            data->help->set_text(data->help, cdload_module.files.get_file(records_language + 0xB0), 0x4C);
            data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0xB0), 0x15);
            ststatus_party_show_menu(obj, data, 1);
            obj->base.step = 10;
        }
        break;
    case 2:
        if (ststatus_module.window_anim_update(&obj->member_panels[0])) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 1);
            ststatus_module.window_anim_start(&obj->bars[1], 1);
            ststatus_party_show_member(obj, data, 0, 1);
            obj->base.step = 4;
        }
        break;
    case 4:
        ststatus_module.window_anim_update(&obj->member_panels[1]);
        ststatus_module.window_anim_update(&obj->menu_anim);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_party_show_member(obj, data, 1, 1);
            data->help->set_text(data->help, cdload_module.files.get_file(records_language + 0xB0), 0x4C);
            data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0xB0), 0x15);
            ststatus_party_show_menu(obj, data, 1);
            obj->base.step = 10;
        }
        break;
    case 3:
        if (ststatus_module.window_anim_update(&obj->member_panels[0])) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 1);
            ststatus_party_show_member(obj, data, 0, 1);
            obj->base.step = 5;
        }
        break;
    case 5:
        if (ststatus_module.window_anim_update(&obj->member_panels[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[2], 1);
            ststatus_module.window_anim_start(&obj->bars[1], 1);
            ststatus_party_show_member(obj, data, 1, 1);
            obj->base.step++;
        }
        break;
    case 6:
        ststatus_module.window_anim_update(&obj->member_panels[2]);
        ststatus_module.window_anim_update(&obj->menu_anim);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_party_show_member(obj, data, 2, 1);
            data->help->set_text(data->help, cdload_module.files.get_file(records_language + 0xB0), 0x4C);
            data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0xB0), 0x15);
            ststatus_party_show_menu(obj, data, 1);
            obj->base.step = 10;
        }
        break;
    case 10:
        old = obj->menu_cursor;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            obj->menu_cursor = 0;
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            obj->menu_cursor = 1;
        }
        if (old != obj->menu_cursor) {
            sound_module.play(0x8004513E);
            data->menu_cursor->set_pos(data->menu_cursor, 0xB8, obj->menu_cursor * 0xE + 0x3A);
        }
        if (PAD_PRESSED(13)) {
            sound_module.play(0x8004503C);
            obj->base.set_step(obj, 100);
            if (obj->menu_cursor == 0) {
                obj->base.substep = 1;
            } else {
                obj->base.substep = 2;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.set_step(obj, 50);
        }
        break;
    case 100:
        if (data->fade == NULL) {
            data->fade = ststatus_create_fade();
        }
        obj->base.step++;
        break;
    case 101:
        data->fade->start(data->fade, 0, 10);
        obj->base.step++;
        break;
    case 102:
        if (data->fade->base.state == OBJECT_STATE_DONE) {
            obj->base.step = 57;
        }
        break;
    case 50:
        ststatus_module.window_anim_start(&obj->member_panels[obj->member_count - 1], 0);
        ststatus_party_show_member(obj, data, obj->member_count - 1, 0);
        ststatus_module.window_anim_start(&obj->bars[1], 0);
        data->help->set_visible(data->help, 0);
        data->back_hint->set_visible(data->back_hint, 0);
        ststatus_module.window_anim_start(&obj->menu_anim, 0);
        ststatus_party_show_menu(obj, data, 0);
        obj->base.step = obj->member_count + 50;
        break;
    case 51:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        ststatus_module.window_anim_update(&obj->menu_anim);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            obj->base.step = 57;
        }
        break;
    case 52:
        ststatus_module.window_anim_update(&obj->member_panels[1]);
        ststatus_module.window_anim_update(&obj->menu_anim);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_party_show_member(obj, data, 0, 0);
            obj->base.step = 54;
        }
        break;
    case 53:
        ststatus_module.window_anim_update(&obj->member_panels[2]);
        ststatus_module.window_anim_update(&obj->menu_anim);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 0);
            ststatus_party_show_member(obj, data, 1, 0);
            obj->base.step = 55;
        }
        break;
    case 55:
        if (ststatus_module.window_anim_update(&obj->member_panels[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_party_show_member(obj, data, 0, 0);
            obj->base.step++;
        }
        break;
    case 54:
    case 56:
        if (ststatus_module.window_anim_update(&obj->member_panels[0])) {
            obj->base.step = 57;
        }
        break;
    case 57:
        if (obj->base.substep == 1) {
            gamestate_data.funcs.set_next_map(0x1200, 0);
        } else if (obj->base.substep == 2) {
            gamestate_data.funcs.set_next_map(0x400, 0);
        } else {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}

void ststatus_party_update(StstatusPartyPage *obj, StstatusPartyPageData *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        for (i = 0; i < 3; i++) {
            if (gamestate_data.funcs.get_party_member(i) >= 0) {
                obj->member_count++;
            }
        }
        for (i = 0; i < obj->member_count; i++) {
            obj->member_panels[i].duration = 10;
            ststatus_module.window_anim_start(&obj->member_panels[i], 1);
        }
        for (i = 0; i < 2; i++) {
            obj->bars[i].duration = 10;
            ststatus_module.window_anim_start(&obj->bars[i], 1);
        }
        obj->menu_anim.duration = 10;
        ststatus_module.window_anim_start(&obj->menu_anim, 1);
        ststatus_party_create_windows(obj, data);
        break;
    case OBJECT_STATE_RUN:
        ststatus_party_run(obj, data);
        ststatus_party_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Creates the party page. */
StstatusPartyPage *ststatus_create_party_page(s32 arg0) {
    StstatusPartyPage *obj = object_new(ststatus_party_update, sizeof(StstatusPartyPage), sizeof(StstatusPartyPageData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->parent = arg0;
    return obj;
}

void ststatus_fade_start(Fade *obj, s32 dir, s32 frames) {
    obj->base.set_state(obj, OBJECT_STATE_RUN);
    obj->base.step = 1;
    obj->from_black = dir;
    if (dir == 0) {
        obj->level = 0;
        obj->step = 0xFF00 / frames;
    } else {
        obj->level = 0xFF00;
        obj->step = -(0xFF00 / frames);
    }
}

void ststatus_fade_draw(Fade *obj) {
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
    setXYWH(poly, 0, 0, 320, 256);
    addPrim(ot, poly);
    tpage = (DR_TPAGE *)(poly + 1);
    setDrawTPage(tpage, 0, 1, getTPage(0, 2, 320, 0));
    addPrim(ot, tpage);
    gfx_module.funcs.set_packet(tpage + 1);
}

void ststatus_fade_update(Fade *obj) {
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
        ststatus_fade_draw(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Fade *ststatus_create_fade(void) {
    Fade *obj = object_new(ststatus_fade_update, sizeof(Fade), 0);

    obj->start = ststatus_fade_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 0;
    return obj;
}
