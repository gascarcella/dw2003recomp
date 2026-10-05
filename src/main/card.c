#include "common.h"

#include "cdload.h"
#include "gfx.h"
#include "heap.h"
#include "card.h"

/* $gp variable: this file is built with -G8, and ASPSX only uses $gp for variables defined in the
 * same file. It is in this object's .sbss (configure.py DATA_IN_C). */
CardPicture *card_module;

s32 card_files[] = { 0x7F6, 0x7F7, 0x7F8, 0x7F9, 0x7FA };
/* The class of each card kind (record byte 3; card_get_class): 0 a Digimon card (kind 16), 1 an option card, 2 an
 * option card that can also be played in phase 5 (cardgame_is_card_playable); STCRDABM draws icon 0x12/0x13 for 1/2. */
s32 card_classes[] = { 0, 1, 1, 1, 2, 1, 1, 2, 2, 1, 1, 2, 1, 1, 2, 0, 0 };

void card_set_module(CardPicture *obj);
void card_select(s32 arg0);
void card_load_image(void);
void card_set_layer(s32 arg0, s32 arg1);
void card_set_image_pos(s32 arg0, s32 arg1);
void card_set_clut_pos(s32 arg0, s32 arg1);
void card_set_cell(s32 arg0, s32 arg1);
void card_set_clut_stride(s32 arg0);
void card_set_semi_trans(s32 arg0);
void card_draw(s32 x, s32 y);
s32 card_get_class(void);

void card_set_module(CardPicture *obj) {
    card_module = obj;
}

void card_select(s32 arg0) {
    s32 i;

    if (arg0 > 0) {
        i = arg0 - 1;
        card_module->record = cdload_module.files.get_file(*(card_files + (i >> 6))) + (i & 0x3F) * 0x62C;
    } else {
        card_module->record = cdload_module.files.get_file(card_files[0]);
    }
}

void card_load_image(void) {
    Tim obj;

    tim_init(&obj);
    obj.set_image_pos(card_module->image_x + card_module->cell_x * 16, card_module->image_y + card_module->cell_y * 32);
    obj.set_clut_pos(card_module->clut_x,
               card_module->clut_y + card_module->cell_x * card_module->clut_stride + card_module->cell_y);
    obj.load((u32 *)(card_module->record + 12));
}

void card_set_layer(s32 arg0, s32 arg1) {
    card_module->layer = gfx_module.funcs.get_layer(arg0);
    card_module->ot_entry = card_module->layer->get_ot_entry(card_module->layer, arg1);
}

void card_set_image_pos(s32 arg0, s32 arg1) {
    card_module->image_x = arg0;
    card_module->image_y = arg1;
}

void card_set_clut_pos(s32 arg0, s32 arg1) {
    card_module->clut_x = arg0;
    card_module->clut_y = arg1;
}

void card_set_cell(s32 arg0, s32 arg1) {
    card_module->cell_x = arg0;
    card_module->cell_y = arg1;
}

void card_set_clut_stride(s32 arg0) {
    card_module->clut_stride = arg0;
}

void card_set_semi_trans(s32 arg0) {
    card_module->semi_trans = arg0;
}

/* Draws a 32x32 sprite (SPRT + DR_TPAGE) at (x, y) from the texture page and CLUT in card_module.
 * The original masks the texture page's x with 0x3C0, not getTPage's 0x3FF: hence the & ~0x3F. */
void card_draw(s32 x, s32 y) {
    SPRT *packet;
    u8 *next;
    SPRT *sprt;
    u16 tpage;
    u16 clut;

    packet = gfx_module.funcs.get_packet();
    sprt = packet;
    clut = getClut(card_module->clut_x,
                   card_module->clut_y + card_module->cell_x * card_module->clut_stride + card_module->cell_y);
    tpage = getTPage(1, 1, (card_module->image_x + card_module->cell_x * 16) & ~0x3F, card_module->image_y);
    setSprt(sprt);
    if (card_module->semi_trans != 0) {
        setSemiTrans(sprt, 1);
    }
    setXY0(sprt, x, y);
    setRGB0(sprt, 128, 128, 128);
    setUV0(sprt, (card_module->cell_x & 3) * 32, card_module->cell_y * 32);
    setWH(sprt, 32, 32);
    sprt->clut = clut;
    addPrim(card_module->ot_entry, sprt);
    sprt++;
    SetDrawTPage((DR_TPAGE *)sprt, 0, 1, tpage);
    next = (u8 *)packet + sizeof(SPRT) + sizeof(DR_TPAGE);
    addPrim(card_module->ot_entry, (DR_TPAGE *)sprt);
    gfx_module.funcs.set_packet(next);
}

s32 card_get_class(void) {
    return card_classes[card_module->record[3]];
}

void card_init(CardPicture *obj) {
    heap_funcs.bzero(obj, sizeof(CardPicture));
    obj->select = card_select;
    obj->load_image = card_load_image;
    obj->set_layer = card_set_layer;
    obj->set_image_pos = card_set_image_pos;
    obj->set_clut_pos = card_set_clut_pos;
    obj->set_cell = card_set_cell;
    obj->set_clut_stride = card_set_clut_stride;
    obj->set_semi_trans = card_set_semi_trans;
    obj->draw = card_draw;
    obj->get_class = card_get_class;
    card_set_module(obj);
    obj->clut_stride = 8;
}
