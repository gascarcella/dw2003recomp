#include "common.h"
#include "gfx.h"
#include "cdload.h"
#include "sound.h"
#include "gamestate.h"
#include "records.h"
#include "ststatus.h"

/* The stage module: the overlay's helper table ststatus_module (the same tail as STCRDDEK's
 * stcrddek_8008A100.c, with item-list helpers added), then the map-area table ststatus_map. */

/* Per Digimon (ststatus_module.anims): its animation frames, ended by -1. */
StstatusAnim ststatus_anims[8] = {
    { { 7, 8, 9, 10, 9, 8, -1 } },
    { { 14, 15, 16, 15, -1, -1, -1 } },
    { { 11, 12, 13, 12, -1, -1, -1 } },
    { { 3, 4, 5, 6, 5, 4, -1 } },
    { { 25, 26, 27, 28, 27, 26, -1 } },
    { { 0, 1, 2, 1, -1, -1, -1 } },
    { { 17, 18, 19, 20, 19, 18, -1 } },
    { { 21, 22, 23, 24, 23, 22, -1 } },
};

/* The party page's windows (ststatus_module.party_layout). */
StstatusLayout ststatus_party_layout[16] = {
    { 12, 55, 19 },
    { 1, 16, 28 },
    { 2, 16, 37 },
    { 4, 61, 37 },
    { 3, 16, 46 },
    { 4, 61, 46 },
    { 13, 44, 28 },
    { 14, 59, 37 },
    { 14, 94, 37 },
    { 14, 59, 46 },
    { 14, 94, 46 },
    { 14, 152, 19 },
    { 14, 20, 198 },
    { 3, 265, 212 },
    { 14, 300, 212 },
    { 22, 189, 49 },
};

/* The map's areas (ststatus_module.area_layout, from 1): sprite, position. */
StstatusLayout ststatus_area_layout[47] = {
    { 0, 0, 0 },
    { 1, 60, 53 },
    { 2, 100, 59 },
    { 3, 126, 74 },
    { 4, 174, 79 },
    { 4, 208, 61 },
    { 5, 209, 36 },
    { 6, 288, 52 },
    { 7, 59, 98 },
    { 8, 116, 102 },
    { 9, 149, 104 },
    { 10, 204, 85 },
    { 10, 251, 152 },
    { 10, 253, 220 },
    { 11, 259, 99 },
    { 12, 303, 115 },
    { 13, 340, 105 },
    { 14, 57, 123 },
    { 15, 162, 138 },
    { 15, 158, 223 },
    { 16, 186, 119 },
    { 17, 251, 126 },
    { 17, 224, 127 },
    { 18, 279, 130 },
    { 19, 304, 141 },
    { 20, 330, 130 },
    { 21, 60, 169 },
    { 22, 122, 162 },
    { 22, 109, 205 },
    { 22, 89, 163 },
    { 23, 185, 143 },
    { 24, 332, 153 },
    { 24, 294, 190 },
    { 25, 199, 167 },
    { 26, 47, 199 },
    { 27, 72, 213 },
    { 28, 185, 221 },
    { 29, 265, 194 },
    { 30, 319, 234 },
    { 31, 35, 254 },
    { 32, 69, 248 },
    { 33, 191, 247 },
    { 34, 224, 227 },
    { 35, 297, 215 },
    { 35, 276, 233 },
    { 36, 296, 253 },
    { 37, 225, 253 },
};

/* The map marks' positions (ststatus_module.mark_positions). */
StstatusPoint ststatus_mark_positions[23] = {
    { 0, 0 },
    { 171, 65 },
    { 154, 123 },
    { 230, 104 },
    { 182, 174 },
    { 286, 129 },
    { 285, 56 },
    { 316, 133 },
    { 262, 161 },
    { 249, 193 },
    { 253, 251 },
    { 172, 221 },
    { 160, 221 },
    { 21, 242 },
    { 25, 160 },
    { 67, 151 },
    { 30, 106 },
    { 32, 45 },
    { 114, 78 },
    { 55, 34 },
    { 128, 88 },
    { 165, 43 },
    { 198, 36 },
};

/* Item kinds (ended by 0), by list and slot (ststatus_kind_lists, ststatus_get_kind_list). */
s32 ststatus_kinds_6[7] = { 1, 2, 3, 4, 5, 7, 0 };
s32 ststatus_kinds_11[12] = { 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 0 };
s32 ststatus_kinds_17[18] = { 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 6, 13, 14, 15, 16, 17, 0 };
s32 ststatus_kinds_22[23] = { 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 6, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 0 };
s32 ststatus_kinds_none[1] = { 0 };
s32 *ststatus_kind_lists[2][5] = {
    { ststatus_kinds_6, ststatus_kinds_11, ststatus_kinds_17, ststatus_kinds_17, ststatus_kinds_22 },
    { ststatus_kinds_none, ststatus_kinds_none, ststatus_kinds_none, ststatus_kinds_17, ststatus_kinds_22 },
};

void ststatus_load_files(void);
s32 ststatus_is_loading(void);
void ststatus_window_anim_start(WindowAnim *anim, s32 open);
s32 ststatus_window_anim_update(WindowAnim *anim);
void ststatus_lerp_start(Tween *lerp, s32 from, s32 to, s32 frames);
s32 ststatus_lerp_update(Tween *lerp);
void *ststatus_get_kind_list(s32 arg0, s32 arg1);
s32 ststatus_list_items(s32 category, u16 *out);
s32 ststatus_can_equip(s32 slot, s32 kind, s32 item);
void ststatus_equip_item(s32 member, s32 slot, s32 item);

StstatusModule ststatus_module = {
    ststatus_anims,
    ststatus_party_layout,
    ststatus_area_layout,
    ststatus_mark_positions,
    { 0 },
    0,
    { 0 },
    0,
    ststatus_load_files,
    ststatus_is_loading,
    ststatus_window_anim_start,
    ststatus_window_anim_update,
    ststatus_lerp_start,
    ststatus_lerp_update,
    ststatus_get_kind_list,
    ststatus_list_items,
    ststatus_can_equip,
    (s32 (*)())ststatus_equip_item,
};

u8 ststatus_hand_kinds[4] = { 1, 2, 3, 7 };
/* Per map (gamestate_data.field_map & 0xFF): its area; bit 7 set in the second region. */
u8 ststatus_map_areas[0xF0] = {
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x1D, 0x15, 0x20,
    0x11, 0x14, 0x14, 0x0B, 0x0B, 0x0D, 0x0D, 0x16, 0x06, 0x17, 0x18, 0x0F, 0x1E, 0x1E, 0x0E, 0x0E,
    0x0E, 0x0E, 0x1F, 0x2A, 0x2A, 0x25, 0x25, 0x2B, 0x0C, 0x24, 0x2C, 0x2D, 0x28, 0x12, 0x29, 0x29,
    0x29, 0x29, 0x23, 0x23, 0x23, 0x23, 0x23, 0x1B, 0x22, 0x19, 0x1C, 0x1A, 0x10, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x22, 0x27, 0x27, 0x26, 0x26, 0x26, 0x21, 0x21, 0x21,
    0x21, 0x09, 0x03, 0x0A, 0x04, 0x02, 0x01, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x05, 0x05,
    0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93,
    0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x9D, 0x95, 0xA0, 0x91,
    0x94, 0x94, 0x8B, 0x8B, 0x8D, 0x8D, 0x96, 0x86, 0x97, 0x98, 0x8F, 0x9E, 0x8E, 0x8E, 0x8E, 0x8E,
    0x9F, 0xAA, 0xAA, 0xA5, 0xAB, 0x8C, 0xA4, 0xAC, 0xAD, 0xA8, 0x92, 0xA9, 0xA9, 0xA9, 0xA9, 0xA3,
    0xA3, 0x9B, 0xA2, 0x99, 0x9C, 0x9A, 0x90, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87,
    0xA2, 0xA7, 0xA7, 0xA6, 0xA6, 0xA6, 0xA1, 0xA1, 0xA1, 0x89, 0x83, 0x8A, 0x84, 0x82, 0x81, 0x80,
    0x88, 0x88, 0x88, 0x88, 0x88, 0x85, 0x85, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
};

s32 ststatus_map_get_region(void);
s32 ststatus_map_get_area(void);
void ststatus_map_mark_visited(s32 *out);

StstatusMap ststatus_map = { ststatus_map_get_region, ststatus_map_get_area, ststatus_map_mark_visited };

/* Uploads the screen's image and starts loading the files the status menu needs. */
void ststatus_load_files(void) {
    Tim tim;

    tim_init(&tim);
    tim.set_image_pos(0x280, 0x100);
    tim.load_all(cdload_module.get_subfile_by_id(0x04050000));
    cdload_module.queue_file(records_language + 0xB0);
    cdload_module.queue_file(records_language + 0x6A);
    cdload_module.queue_file(records_language + 0x63);
    cdload_module.queue_file(records_language + 0x4E);
    cdload_module.queue_file(records_language + 0x47);
    cdload_module.queue_file(records_language + 0xA2);
    cdload_module.queue_file(records_language + 0x9B);
}

/* 1 while one of the files is still loading. */
s32 ststatus_is_loading(void) {
    if (cdload_module.is_loading(records_language + 0xB0)) {
        return 1;
    }
    if (cdload_module.is_loading(records_language + 0x6A)) {
        return 1;
    }
    if (cdload_module.is_loading(records_language + 0x63)) {
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
void ststatus_window_anim_start(WindowAnim *anim, s32 open) {
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

s32 ststatus_window_anim_update(WindowAnim *anim) {
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

/* Starts moving the value (with the cursor sound) unless it is already there. */
void ststatus_lerp_start(Tween *lerp, s32 from, s32 to, s32 frames) {
    if (from != to) {
        sound_module.play(0x40019);
        lerp->duration = frames;
        lerp->acc = from << 8;
        lerp->value = from;
        lerp->target = to;
        lerp->running = 1;
        lerp->step = ((to - from) << 8) / lerp->duration;
    }
}

/* Steps the value; returns 1 once it is at the end (or not running). */
s32 ststatus_lerp_update(Tween *lerp) {
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

void *ststatus_get_kind_list(s32 arg0, s32 arg1) {
    return ststatus_kind_lists[arg0][arg1];
}

s32 ststatus_list_hand_items(u16 *out);
s32 ststatus_list_items_of_kind(s32 kind, u16 *out);

/* Lists the items of a category into out; returns how many. */
s32 ststatus_list_items(s32 category, u16 *out) {
    s32 n;

    if (category < 5) {
        n = records_funcs.list_items(category, out);
    } else {
        switch (category) {
        case 5:
        default:
            n = ststatus_list_hand_items(out);
            break;
        case 6:
            n = ststatus_list_items_of_kind(4, out);
            break;
        case 7:
            n = ststatus_list_items_of_kind(5, out);
            break;
        }
    }
    return n;
}

/* The items of lists 2 and 3 whose kind is in ststatus_hand_kinds. */
s32 ststatus_list_hand_items(u16 *out) {
    s32 i;
    s32 j;
    s32 n;
    RecordsEquip *item;

    n = 0;
    ststatus_module.list_2_count = records_funcs.list_items(2, ststatus_module.list_2);
    ststatus_module.list_3_count = records_funcs.list_items(3, ststatus_module.list_3);
    for (i = 0; i < ststatus_module.list_2_count; i++) {
        item = records_funcs.get_item(ststatus_module.list_2[i])->data;
        for (j = 0; j < 4; j++) {
            if (item->slot == ststatus_hand_kinds[j]) {
                out[n++] = ststatus_module.list_2[i];
            }
        }
    }
    for (i = 0; i < ststatus_module.list_3_count; i++) {
        item = records_funcs.get_item(ststatus_module.list_3[i])->data;
        for (j = 0; j < 4; j++) {
            if (item->slot == ststatus_hand_kinds[j]) {
                out[n++] = ststatus_module.list_3[i];
            }
        }
    }
    return n;
}

/* The items of lists 2 and 3 of one kind. */
s32 ststatus_list_items_of_kind(s32 kind, u16 *out) {
    s32 i;
    s32 n;

    n = 0;
    ststatus_module.list_2_count = records_funcs.list_items(2, ststatus_module.list_2);
    ststatus_module.list_3_count = records_funcs.list_items(3, ststatus_module.list_3);
    for (i = 0; i < ststatus_module.list_2_count; i++) {
        if (((RecordsEquip *)records_funcs.get_item(ststatus_module.list_2[i])->data)->slot == kind) {
            out[n++] = ststatus_module.list_2[i];
        }
    }
    for (i = 0; i < ststatus_module.list_3_count; i++) {
        if (((RecordsEquip *)records_funcs.get_item(ststatus_module.list_3[i])->data)->slot == kind) {
            out[n++] = ststatus_module.list_3[i];
        }
    }
    return n;
}

/* 1 if party slot `slot` may equip `item` (-1: nothing) in equipment slot `kind`. */
s32 ststatus_can_equip(s32 slot, s32 kind, s32 item) {
    RecordsEquip *e;

    if (item != -1) {
        e = records_funcs.get_item(item)->data;
        if (!((e->members >> slot) & 1)) {
            return 0;
        }
        if (e->slot == 1) {
            if (kind == 3) {
                return 0;
            }
        } else if (e->slot == 2) {
            if (kind == 2) {
                return 0;
            }
        }
    }
    return 1;
}

/* Equips party member `member` with `item` in equipment slot `slot` (item 0: just take off what is there),
 * keeping the stock counts (gamestate_data.items free, unk_020F equipped) in step. Kind 7 takes slots 2 and 3;
 * kind 8 replaces an item of the same group (unk_3) in slots 4 and 5. */
void ststatus_equip_item(s32 member, s32 slot, s32 item) {
    GamestateRecord *rec;
    s16 *p;
    s32 i;
    RecordsEquip *e;
    s32 group;
    s16 old;
    s32 id;

    rec = gamestate_data.funcs.get_record(member);
    old = *(rec->equipment + slot);
    id = item;
    if (old != 0) {
        gamestate_data.items_equipped[old]--;
        gamestate_data.items[old]++;
        e = records_funcs.get_item(old)->data;
        if (e->slot == 7) {
            rec->equipment[2] = 0;
            rec->equipment[3] = 0;
        } else {
            *(rec->equipment + slot) = 0;
        }
    }
    if (id > 0) {
        e = records_funcs.get_item(id)->data;
        if (e->slot == 7) {
            s16 *hand = &rec->equipment[2];

            if (*hand == 0) {
                hand = NULL;
                if (rec->equipment[3] != 0) {
                    hand = &rec->equipment[3];
                }
            }
            if (hand != NULL) {
                gamestate_data.items_equipped[*hand]--;
                gamestate_data.items[*hand]++;
                *hand = 0;
            }
        } else if (e->slot == 8) {
            group = e->group;
            for (i = 0; i < 2; i++) {
                p = &rec->equipment[4 + i];
                if (*p != 0) {
                    e = records_funcs.get_item(*p)->data;
                    if (e->group == group) {
                        gamestate_data.items_equipped[*p]--;
                        gamestate_data.items[*p]++;
                        *p = 0;
                    }
                }
            }
        }
        gamestate_data.items_equipped[id]++;
        gamestate_data.items[id]--;
        e = records_funcs.get_item(id)->data;
        if (e->slot == 7) {
            rec->equipment[2] = id;
            rec->equipment[3] = id;
            return;
        }
        *(rec->equipment + slot) = id;
    }
}

/* The map table ststatus_map's functions. gamestate_data.field_map is the map: 0x200-0x26F one region,
 * 0x270-0x2D6 the other. */

/* 1 in the second region, 0 in the first, -1 past it. */
s32 ststatus_map_get_region(void) {
    if (gamestate_data.field_map >= 0x2D7) {
        return -1;
    }
    return gamestate_data.field_map >= 0x270;
}

/* The map's area (ststatus_map_areas, bit 7 masked). */
s32 ststatus_map_get_area(void) {
    return ststatus_map_areas[gamestate_data.field_map & 0xFF] & 0x7F;
}

/* Marks in out[area] the areas of the current region that have a map whose flag 0x20xx is set. */
void ststatus_map_mark_visited(s32 *out) {
    s32 first;
    s32 last;
    s32 i;
    s32 area;
    s32 set;

    if (ststatus_map_get_region() == 0) {
        first = 0x200;
        last = 0x26F;
    } else {
        first = 0x270;
        last = 0x2D6;
    }
    for (i = first; i <= last; i++) {
        area = ststatus_map_areas[i & 0xFF] & 0x7F;
        set = gamestate_flags.get_flag((i & 0xFF) | 0x2000, 1);
        if (set == 1) {
            out[area] = set;
        }
    }
}
