#include "common.h"
#include "object.h"
#include "heap.h"
#include "gfx.h"
#include "cdload.h"
#include "sound.h"
#include "gamestate.h"
#include "psyq/libgpu.h"
#include "message.h"
#include "records.h"
#include "fieldstg.h"

void fieldstg_warp_picture_update();
void fieldstg_warp_effect_update();
void fieldstg_background_update();
s32 *fieldstg_background_get_size();
void fieldstg_update_main();
void fieldstg_map_title_update();

/* Entry of fieldstg_map_titles (ended by map == 0): the two texts shown for a map. */
typedef struct FieldstgMapTitleText {
    /* 0x0 */ u8 text_0; /* text of the first window */
    /* 0x1 */ u8 text_1; /* text of the second window */
    /* 0x2 */ s16 map; /* map */
} FieldstgMapTitleText; /* size 0x4 */

extern FieldstgMapTitleText fieldstg_map_titles[];

/* Data block of the object of fieldstg_map_title_update. */
typedef struct FieldstgMapTitleData {
    /* 0x0 */ MessageWindow *window_0;
    /* 0x4 */ MessageWindow *window_1;
} FieldstgMapTitleData; /* size 0x8 */

/* A rectangle of the object of fieldstg_map_title_update (10 of them at 0x50), drawn by
 * fieldstg_map_title_draw_rect and opened/closed by fieldstg_map_title_move_rect. */
typedef struct FieldstgMapTitleRect {
    /* 0x00 */ s32 shown; /* shown */
    /* 0x04 */ RECT rect;
    /* 0x0C */ u32 color; /* colour and primitive code */
    /* 0x10 */ s32 moving; /* moving: 1 = left/right edges, 2 = top/bottom edges, 0 = done */
    /* 0x14 */ s32 target_0; /* target of the first edge */
    /* 0x18 */ s32 target_1; /* target of the second edge */
    /* 0x1C */ s32 speed; /* speed */
    /* 0x20 */ s32 unk_20;
} FieldstgMapTitleRect; /* size 0x24 */

extern FieldstgMapTitleRect fieldstg_map_title_rects[10];

/* The map title (fieldstg_map_title_update, created by fieldstg_map_title_create; base.key1: its argument,
 * 0 = nothing to show): two text windows with the map's name (fieldstg_map_titles) over rectangles that slide in.
 * States: 0 create, 1 open (the rectangles in four steps, then wait for the windows), 2 close (wait 60 frames,
 * then shrink the layer), 3 end (clears fieldstg_stage.title_shown). */
typedef struct FieldstgMapTitle {
    /* 0x000 */ Object base;
    /* 0x050 */ FieldstgMapTitleRect rects[10];
    /* 0x1B8 */ RECT clip; /* the layer's area while it closes */
} FieldstgMapTitle; /* size 0x1C0 */

void fieldstg_map_title_move_rect(Object *obj, FieldstgMapTitleRect *rect);
void fieldstg_map_title_draw_rect(Object *obj, u32 *ot, RECT rect, u32 color);

/* A sprite of FieldstgTile (fieldstg_tile_upload fills them from the tile's data). */
typedef struct FieldstgTileSprite {
    /* 0x00 */ s32 used; /* used */
    /* 0x04 */ s32 x; /* x */
    /* 0x08 */ s32 y; /* y */
    /* 0x0C */ s32 u; /* u */
    /* 0x10 */ s32 v; /* v (added to the tile's tpage_y) */
    /* 0x14 */ s32 w; /* width */
    /* 0x18 */ s32 h; /* height */
} FieldstgTileSprite; /* size 0x1C */

/* Object of fieldstg_tile_update, created by fieldstg_tile_create: one tile of the field
 * background, read from `file` into `buffer`, with its methods at 0x234. */
typedef struct FieldstgTile {
    /* 0x000 */ Object base;
    /* 0x050 */ s32 last_used; /* gfx_module's vsync count when last used */
    /* 0x054 */ s32 loaded_time; /* vsync count when the read finished */
    /* 0x058 */ s32 number; /* tile number, -1: none */
    /* 0x05C */ s32 sector; /* its sector offset in the file */
    /* 0x060 */ s32 file; /* file ID */
    /* 0x064 */ s32 sectors; /* sectors per tile */
    /* 0x068 */ void *buffer; /* buffer */
    /* 0x06C */ s32 read_done; /* read done (cdload_read's flag) */
    /* 0x070 */ s32 slot; /* VRAM slot holding its image, -1: none */
    /* 0x074 */ s32 tpage_x; /* the slot's VRAM position (fieldstg_tile_vram_pos) */
    /* 0x078 */ s32 tpage_y;
    /* 0x07C */ s32 clut_x;
    /* 0x080 */ s32 clut_y;
    /* 0x084 */ FieldstgTileSprite sprites[3][5];
    /* 0x228 */ s32 upload_slot; /* slot to unpack into (fieldstg_tile_unpack) */
    /* 0x22C */ MessageRlen *unpacker;
    /* 0x230 */ s32 *data; /* the unpacked tile */
    /* 0x234 */ void (*load)(struct FieldstgTile *obj, s32 tile, s32 sectors); /* fieldstg_tile_load */
    /* 0x238 */ s32 (*is_loaded)(struct FieldstgTile *obj);                         /* fieldstg_tile_is_loaded */
    /* 0x23C */ void (*draw)(struct FieldstgTile *obj, GfxLayer *layer, s32 x, s32 y); /* fieldstg_tile_draw */
    /* 0x240 */ void (*unpack)(struct FieldstgTile *obj, s32 arg1, MessageRlen *unpacker); /* fieldstg_tile_unpack */
    /* 0x244 */ s32 (*get_number)(struct FieldstgTile *obj);                         /* fieldstg_tile_get_number */
    /* 0x248 */ s32 (*get_slot)(struct FieldstgTile *obj);                         /* fieldstg_tile_get_slot */
    /* 0x24C */ void (*free_slot)(struct FieldstgTile *obj);                        /* fieldstg_tile_free_slot */
    /* 0x250 */ void (*touch)(struct FieldstgTile *obj);                        /* fieldstg_tile_touch */
} FieldstgTile; /* size 0x254 */

void fieldstg_tile_touch(FieldstgTile *obj);

/* VRAM position of each tile slot's image (FieldstgTile.upload_slot). */
typedef struct FieldstgTileVramPos {
    /* 0x0 */ s32 x; /* x */
    /* 0x4 */ s32 y; /* y */
} FieldstgTileVramPos; /* size 0x8 */

extern FieldstgTileVramPos fieldstg_tile_vram_pos[];

/* Per FieldstgActor.dir: which tile order table to use, and its flips (1: rows, 2: columns). */
typedef struct FieldstgTileOrder {
    /* 0x0 */ u8 table;
    /* 0x1 */ u8 flip;
} FieldstgTileOrder; /* size 0x2 */

extern FieldstgTileOrder fieldstg_background_order_by_dir[];
extern u8 fieldstg_background_tile_orders[][4][5][6]; /* [table][quarter][row][column]: slot of each visible tile */
extern u8 fieldstg_background_first_cells[4][2][2]; /* [quarter]: first column (2 choices), first row (2 choices) */

/* Ordering-table depth of each of a tile's three sprite groups. */
extern s32 fieldstg_tile_ot_depths[3];
void fieldstg_tile_load(FieldstgTile *obj, s32 tile, s32 sectors);
s32 fieldstg_tile_is_loaded(FieldstgTile *obj);
void fieldstg_tile_draw(FieldstgTile *obj, GfxLayer *layer, s32 x, s32 y);
void fieldstg_tile_unpack(FieldstgTile *obj, s32 slot, MessageRlen *unpacker);
void fieldstg_tile_upload(FieldstgTile *obj);
void fieldstg_tile_free_slot(FieldstgTile *obj);
s32 fieldstg_tile_get_slot(FieldstgTile *obj);
s32 fieldstg_tile_get_number(FieldstgTile *obj);
void fieldstg_tile_update(FieldstgTile *obj);

/* The warp picture (fieldstg_warp_picture_update, created by fieldstg_warp_picture_create): a full-screen animation
 * from file 0x88C (type 0) or 0x88D (type 1, two layers) shown while a warp changes the map. States: 0 load,
 * 1 play, 2 hold. */
typedef struct FieldstgWarpPicture {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 type;
    /* 0x54 */ s32 image;
    /* 0x58 */ s32 frame;
    /* 0x5C */ s32 time;
    /* 0x60 */ s32 image_2;
    /* 0x64 */ s32 frame_2;
    /* 0x68 */ s32 time_2;
} FieldstgWarpPicture; /* size 0x6C */

/* Animation track of FieldstgWarpEffect (fieldstg_warp_effect_step steps it). */
typedef struct FieldstgAnimTrack {
    /* 0x0 */ s32 active; /* active */
    /* 0x4 */ s16 frame; /* frame index */
    /* 0x6 */ s16 time; /* time left in the frame */
} FieldstgAnimTrack; /* size 0x8 */

/* A frame of an animation of FieldstgWarpEffect (tables at fieldstg_warp_effect_anims). */
typedef struct FieldstgAnimFrame {
    /* 0x0 */ s16 image; /* image; 0xFF: end, 0x12C: none */
    /* 0x2 */ s16 time; /* duration */
} FieldstgAnimFrame; /* size 0x4 */

/* Animations of FieldstgWarpPicture: image (-1: go back to the frame in time) and duration. */
extern FieldstgAnimFrame D_FIELDSTG_80096CC8[];
extern FieldstgAnimFrame D_FIELDSTG_80096D14[];
extern FieldstgAnimFrame D_FIELDSTG_80096D40[];

/* Animations of FieldstgWarpEffect: [its type][track]. */
extern FieldstgAnimFrame *fieldstg_warp_effect_anims[][4];

/* The warp effect (fieldstg_warp_effect_update, created by fieldstg_warp_effect_create): four animation tracks at a
 * map position (sprite file 0x160), ended when all four are. */
typedef struct FieldstgWarpEffect {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 x;
    /* 0x54 */ s32 y;
    /* 0x58 */ s16 type;
    /* 0x5A */ u8 unk_5A[0x2];
    /* 0x5C */ FieldstgAnimTrack tracks[4];
} FieldstgWarpEffect; /* size 0x7C */

/* A tile slot of FieldstgBackground (12 of them). */
typedef struct FieldstgBackgroundSlot {
    /* 0x0 */ s32 tile; /* tile number, -1: free */
    /* 0x4 */ s32 last_used; /* gfx_module's vsync count when last used */
    /* 0x8 */ s32 fade; /* fade (0xFFFF down to 0) */
} FieldstgBackgroundSlot; /* size 0xC */

/* Entry of FieldstgBackground.tiles, one per map tile. */
typedef struct FieldstgBackgroundTile {
    /* 0x0 */ s32 size;    /* bytes, 0: no tile */
    /* 0x4 */ s32 sectors; /* sectors */
} FieldstgBackgroundTile; /* size 0x8 */

/* Header of the field background file (its first sector). */
typedef struct FieldstgBackgroundHeader {
    /* 0x00 */ s32 tile_count; /* non-empty tiles (the code ignores it) */
    /* 0x04 */ s32 width; /* width in tiles */
    /* 0x08 */ s32 height; /* height in tiles */
    /* 0x0C */ s32 tile_size; /* bytes per tile */
    /* 0x10 */ u16 sizes[1]; /* per tile: its size in bytes (0: none) */
} FieldstgBackgroundHeader;

/* Object of fieldstg_background_update (type 4), created by fieldstg_background_create: the field background,
 * a map of width x height tiles of 128x128 read from `file`, streamed: the 30 tile objects hold the packed tiles
 * around the view (fieldstg_background_update_visible orders them by the player's direction,
 * fieldstg_background_load_tiles reads them), 12 VRAM slots their images (fieldstg_background_draw unpacks one per
 * frame into the least recently used slot). A tile not in VRAM yet is drawn blank, then fades in. States: 0 read
 * the header and create the tile objects, 1 stream and draw, 3 free. */
typedef struct FieldstgBackground {
    /* 0x000 */ Object base;
    /* 0x050 */ u8 unk_050[0x4];
    /* 0x054 */ FieldstgBackgroundHeader *header; /* the file's header (heap, while it loads) */
    /* 0x058 */ s32 x;  /* position (the layer's, gfx_get_scroll) */
    /* 0x05C */ s32 y;
    /* 0x060 */ u8 unk_060[0x4];
    /* 0x064 */ s32 file;  /* file ID */
    /* 0x068 */ s32 width;  /* width in tiles */
    /* 0x06C */ s32 height;  /* height in tiles */
    /* 0x070 */ s32 sectors;  /* sectors per tile */
    /* 0x074 */ FieldstgBackgroundSlot slots[12];
    /* 0x104 */ FieldstgBackgroundTile *tiles;
    /* 0x108 */ u8 visible[30]; /* visible tiles, 0xFF: none */
    /* 0x126 */ u8 unk_126[0x2];
    /* 0x128 */ s32 first_x; /* tile of visible's first column */
    /* 0x12C */ s32 first_y; /* and first row */
    /* 0x130 */ s32 *(*get_size)(struct FieldstgBackground *obj); /* fieldstg_background_get_size */
} FieldstgBackground; /* size 0x134 */

/* Data block of FieldstgBackground (base.children). */
typedef struct FieldstgBackgroundData {
    /* 0x00 */ MessageRlen *unpacker;
    /* 0x04 */ FieldstgTile *tiles[30];
} FieldstgBackgroundData; /* size 0x7C */

/* A visible map cell of fieldstg_background_draw (up to 4 x 3). */
typedef struct FieldstgBackgroundCell {
    /* 0x00 */ s16 loaded; /* its tile is loaded in a tile object */
    /* 0x02 */ s16 exists; /* the map has a tile there */
    /* 0x04 */ FieldstgTile *tile;
    /* 0x08 */ s32 slot; /* the tile object's slot (FieldstgTile.slot), -1: none */
    /* 0x0C */ s32 number; /* tile number */
    /* 0x10 */ s32 x; /* x */
    /* 0x14 */ s32 y; /* y */
} FieldstgBackgroundCell; /* size 0x18 */

/* Size of the field map in pixels (fieldstg_background_get_size). */
extern s32 fieldstg_background_size[2];

void fieldstg_background_draw_blank(GfxLayer *layer, s32 x, s32 y, s32 alpha);
void fieldstg_background_load_tiles(FieldstgBackground *obj, FieldstgBackgroundData *data);
void fieldstg_background_draw(FieldstgBackground *obj, FieldstgBackgroundData *data);
void fieldstg_background_update_visible(FieldstgBackground *obj);
FieldstgTile *fieldstg_tile_create(s32 size, s32 file);
FieldstgTile *fieldstg_background_find_tile(FieldstgBackgroundData *data, s32 tile_no);

void fieldstg_warp_picture_update(FieldstgWarpPicture *obj) {
    Tim tim;
    Sprite spr;
    s32 loading;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            if (obj->type != 0) {
                cdload_module.queue_file(0x88D);
            } else {
                cdload_module.queue_file(0x88C);
            }
            obj->base.next_step(obj);
            /* fallthrough */
        case 1:
            if (obj->type != 0) {
                loading = cdload_module.is_loading(0x88D);
            } else {
                loading = cdload_module.is_loading(0x88C);
            }
            if (loading == 0) {
                tim_init(&tim);
                tim.set_image_pos(0x280, 0);
                tim.set_clut_pos(0, 0xF0);
                if (obj->type != 0) {
                    tim.load_all(cdload_module.get_subfile_by_id(0x088D0001));
                    sound_module.play(0x40003);
                } else {
                    tim.load_all(cdload_module.get_subfile_by_id(0x088C0001));
                    sound_module.play(0x40018);
                }
                obj->base.next_state(obj);
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        if (obj->type == 0) {
            if (obj->time <= 0) {
                obj->frame++;
                obj->image = D_FIELDSTG_80096CC8[obj->frame].image;
                obj->time = D_FIELDSTG_80096CC8[obj->frame].time;
            } else {
                obj->time -= gfx_module.funcs.get_frame_ticks();
            }
            if (obj->image == -1) {
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
    if (obj->base.state == OBJECT_STATE_RUN || obj->base.state == OBJECT_STATE_DONE) {
        if (obj->type != 0) {
            while (1) {
                if (obj->time <= 0) {
                    obj->frame++;
                    obj->image = D_FIELDSTG_80096D14[obj->frame].image;
                    obj->time = D_FIELDSTG_80096D14[obj->frame].time;
                } else {
                    obj->time -= gfx_module.funcs.get_frame_ticks();
                }
                if (obj->image != -1) {
                    break;
                }
                obj->frame = obj->time;
                obj->time = -1;
            }
            while (1) {
                if (obj->time_2 <= 0) {
                    obj->frame_2++;
                    obj->image_2 = D_FIELDSTG_80096D40[obj->frame_2].image;
                    obj->time_2 = D_FIELDSTG_80096D40[obj->frame_2].time;
                } else {
                    obj->time_2 -= gfx_module.funcs.get_frame_ticks();
                }
                if (obj->image_2 != -1) {
                    break;
                }
                obj->frame_2 = obj->time_2;
                obj->time_2 = -1;
            }
            obj->base.timer += gfx_module.funcs.get_frame_ticks();
            if (obj->base.timer > 160) {
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
        }
        sprite_init(&spr);
        spr.set_follow_scroll(0);
        spr.set_layer_id(0x1002, 7);
        spr.set_vram_pos(0x280, 0);
        spr.set_clut8_pos(0, 0xF0);
        if (obj->type != 0) {
            spr.draw(cdload_module.get_subfile_by_id(0x088D0000), obj->image, 0, 0);
            if (obj->image_2 != 0) {
                spr.draw(cdload_module.get_subfile_by_id(0x088D0000), obj->image_2, 0, 0);
            }
        } else if (obj->base.state == OBJECT_STATE_RUN) {
            spr.draw(cdload_module.get_subfile_by_id(0x088C0000), obj->image, 0, 0);
        }
        if (obj->type != 0) {
            spr.draw(cdload_module.get_subfile_by_id(0x088D0000), 0, 0, 0);
        } else {
            spr.draw(cdload_module.get_subfile_by_id(0x088C0000), 10, 0, 0);
        }
    }
}

FieldstgWarpPicture *fieldstg_warp_picture_create(s32 type) {
    FieldstgWarpPicture *obj = object_new(fieldstg_warp_picture_update, sizeof(FieldstgWarpPicture), 0);

    obj->type = type;
    return obj;
}

s32 fieldstg_warp_effect_step(FieldstgAnimTrack *track, FieldstgAnimFrame *frames, s32 depth) {
    FieldstgAnimFrame *frame = &frames[track->frame];
    s32 ticks = gfx_module.funcs.get_frame_ticks();

    if (ticks > 4) {
        ticks = 4;
    }
    if (depth == 0) {
        track->time -= ticks;
    }
    if (track->time <= 0) {
        frame++;
        track->frame++;
        track->time += frame->time;
        if (frame->image == 0xFF) {
            return 0xFF;
        }
        fieldstg_warp_effect_step(track, frames, depth + 1);
    }
    return frame->image;
}

void fieldstg_warp_effect_update(FieldstgWarpEffect *obj) {
    Sprite spr;
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 image;
    s32 i;
    s32 j;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (j = 0; j < 4; j++) {
            obj->tracks[j].active = 1;
            obj->tracks[j].frame = 0;
            obj->tracks[j].time = fieldstg_warp_effect_anims[obj->type][j]->time;
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (i = 0; i < 4; i++) {
            if (obj->tracks[i].active != 0) {
                image = fieldstg_warp_effect_step(&obj->tracks[i], fieldstg_warp_effect_anims[obj->type][i], 0);
                switch (image) {
                case 0x12C:
                    break;
                case 0xFF:
                    obj->tracks[i].active = 0;
                    break;
                default:
                    sprite_init(&spr);
                    spr.set_vram_pos(0x240, 0x100);
                    spr.set_layer(layer, 0);
                    spr.set_palette(0);
                    spr.draw(cdload_module.get_subfile_by_id(0x01600001), image, obj->x, obj->y);
                    break;
                }
            }
        }
        if (obj->tracks[0].active == 0 && obj->tracks[1].active == 0 && obj->tracks[2].active == 0
            && obj->tracks[3].active == 0) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

FieldstgWarpEffect *fieldstg_warp_effect_create(s32 x, s32 y, s16 type) {
    FieldstgWarpEffect *obj = object_new(fieldstg_warp_effect_update, sizeof(FieldstgWarpEffect), 0);

    obj->x = x;
    obj->y = y;
    obj->type = type;
    return obj;
}

FieldstgTile *fieldstg_background_get_oldest_tile(FieldstgBackgroundData *data) {
    s32 oldest = gfx_module.funcs.get_time();
    s32 i;
    FieldstgTile *found = data->tiles[0];

    for (i = 0; i < 30; i++) {
        if (data->tiles[i]->last_used <= oldest) {
            oldest = data->tiles[i]->last_used;
            found = data->tiles[i];
        }
    }
    return found;
}

void fieldstg_background_load_tiles(FieldstgBackground *obj, FieldstgBackgroundData *data) {
    FieldstgTile *tile;
    s32 loaded;
    s32 tile_no;
    s32 j;
    s32 i;
    s32 k;

    if (cdload_reader.is_busy() != 0) {
        return;
    }
    for (i = 0; i < 30; i++) {
        tile = data->tiles[i];
        loaded = tile->get_number(tile);
        for (j = 0; j < 30; j++) {
            if (obj->visible[j] != 0xFF && obj->visible[j] == loaded) {
                obj->visible[j] = 0xFF;
                tile->touch(tile);
                break;
            }
        }
    }
    for (k = 0; k < 30; k++) {
        if (obj->visible[k] != 0xFF) {
            tile_no = obj->visible[k];
            if (obj->tiles[tile_no].size != 0) {
                tile = fieldstg_background_get_oldest_tile(data);
                if (tile->is_loaded(tile) != 0) {
                    tile->load(tile, tile_no, obj->tiles[tile_no].sectors);
                }
            }
            if (cdload_reader.is_busy() != 0) {
                break;
            }
        }
    }
}

void fieldstg_background_draw_blank(GfxLayer *layer, s32 x, s32 y, s32 alpha) {
    u32 *ot = layer->get_ot_entry(layer, 0);
    s32 pos[2];
    s32 a = alpha >> 8;
    SPRT *sprt;
    DR_TPAGE *tpage;
    s32 i;

    layer->get_scroll(layer, pos);
    sprt = gfx_module.funcs.get_packet();
    for (i = 0; i < 4; i++) {
        SetSprt(sprt);
        if (a != 0xFF) {
            SetSemiTrans(sprt, 1);
        }
        sprt->r0 = sprt->g0 = sprt->b0 = a;
        sprt->x0 = x - pos[0] + ((i & 1) << 6);
        sprt->y0 = y - pos[1] + ((i >> 1 & 1) << 6);
        sprt->u0 = fieldstg_stage.vram_places[1].u;
        sprt->v0 = fieldstg_stage.vram_places[1].v;
        sprt->w = 0x40;
        sprt->h = 0x40;
        sprt->clut = GetClut(fieldstg_stage.vram_places[1].clut_x, fieldstg_stage.vram_places[1].clut_y);
        addPrim(ot, sprt);
        tpage = (DR_TPAGE *)++sprt;
        SetDrawTPage(tpage, 0, 1, GetTPage(0, 1, fieldstg_stage.vram_places[1].tpage_x, fieldstg_stage.vram_places[1].tpage_y));
        addPrim(ot, tpage);
        sprt = (SPRT *)(tpage + 1);
    }
    gfx_module.funcs.set_packet(sprt);
}

FieldstgTile *fieldstg_background_find_tile(FieldstgBackgroundData *data, s32 tile_no) {
    s32 i;
    FieldstgTile *tile;

    for (i = 0; i < 30; i++) {
        tile = data->tiles[i];
        if (tile->get_number(tile) == tile_no) {
            return tile;
        }
    }
    return NULL;
}

void fieldstg_background_draw(FieldstgBackground *obj, FieldstgBackgroundData *data) {
    s32 origin[2];
    FieldstgBackgroundCell cells[12];
    FieldstgTile *tile;
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 count = 0;
    s32 row;
    s32 col;
    s32 tx;
    s32 tile_no;
    s32 fade;
    s32 best;
    s32 oldest;
    s32 i;
    s32 j;

    origin[0] = obj->x < 0x20 ? 0 : (obj->x - 0x20) / 128;
    origin[1] = obj->y < 8 ? 0 : (obj->y - 8) / 128;
    for (row = 0; row < 3; row++) {
        if (row + origin[1] >= obj->height) {
            break;
        }
        for (col = 0; col < 4; col++) {
            tx = col + origin[0];
            if (tx >= obj->width) {
                break;
            }
            tile_no = tx + (row + origin[1]) * obj->width;
            tile = fieldstg_background_find_tile(data, tile_no);
            cells[count].exists = obj->tiles[tile_no].size;
            if (tile != NULL) {
                cells[count].tile = tile;
                cells[count].number = tile_no;
                cells[count].slot = tile->get_slot(tile);
                cells[count].loaded = 1;
            } else {
                cells[count].loaded = 0;
            }
            cells[count].x = (col + origin[0]) << 7;
            cells[count].y = (row + origin[1]) << 7;
            count++;
        }
    }
    for (i = 0; i < count; i++) {
        if (cells[i].loaded != 0 && cells[i].slot != -1) {
            fade = obj->slots[cells[i].slot].fade;
            if (fade != 0) {
                if (fade == 0xFFFF && cells[i].tile->loaded_time < gfx_module.funcs.get_time() - 10) {
                    obj->slots[cells[i].slot].fade = 0;
                } else {
                    fieldstg_background_draw_blank(layer, cells[i].x, cells[i].y, fade);
                    fade -= 0x2AAA;
                    if (fade <= 0) {
                        fade = 0;
                    }
                    obj->slots[cells[i].slot].fade = fade;
                }
            }
            cells[i].tile->draw(cells[i].tile, layer, cells[i].x, cells[i].y);
            obj->slots[cells[i].slot].last_used = gfx_module.funcs.get_time();
        } else if (cells[i].exists != 0) {
            fieldstg_background_draw_blank(layer, cells[i].x, cells[i].y, 0xFFFF);
        }
    }
    if (data->unpacker->base.step == 0) {
        for (i = 0; i < count; i++) {
                if (cells[i].loaded != 0 && cells[i].slot == -1 && cells[i].tile->is_loaded(cells[i].tile) != 0) {
                best = 0;
                oldest = gfx_module.funcs.get_time();
                for (j = 0; j < 12; j++) {
                    if (obj->slots[j].last_used < oldest) {
                        oldest = obj->slots[j].last_used;
                        best = j;
                    }
                }
                cells[i].tile->unpack(cells[i].tile, best, data->unpacker);
                if (obj->slots[best].tile != -1) {
                    tile = fieldstg_background_find_tile(data, obj->slots[best].tile);
                    if (tile != NULL) {
                        tile->free_slot(tile);
                    }
                }
                obj->slots[best].tile = cells[i].number;
                obj->slots[best].last_used = gfx_module.funcs.get_time();
                obj->slots[best].fade = 0xFFFF;
                break;
            }
        }
    }
}

#ifdef NON_MATCHING
/* 88.7%: first_cells as [4][2][2] (the original's folded lbu 2(v0)), an unsigned flip (srl) and the two first-cell
 * indexes computed before the divisions (cx, cy) are the original's. The rest: the original computes the tile-order
 * value before order[row][col]'s address (so loop.c doesn't hoist sp+16 and flip stays in t7): a value temporary or
 * a static inline accessor for the tile order gives that (90.45%, not adopted: judgement call); then sched2 places the
 * order_by_dir address first and the t-registers of quarter, quarter*30 and the row giv differ. wip-13: 30 min of
 * permuter gave only a quarter copy (87.4 alone) plus a semantic change; *(order[row] + col) 90.08 (flip in t7, the row
 * giv then includes sp); 1D order indexes, a dir local, statement orders of quarter/table/flip: no gain.
 * US decomp (func_80085EEC): still INCLUDE_ASM.
 * final-rest: a value temporary `v` for the tile order plus `cx = obj->y & 0x7F` reused for the y test: 94.59
 * (both forced; left: the first block's schedule and the t-registers of quarter, flip and the first-cell bases).
 * last-rest (final): the permuter (25 min, boosted weights) on that form: an s16 quarter gives 96.00, the rest only
 * with dummy memory writes. Not adopted: the forced forms don't reach 100. */
void fieldstg_background_update_visible(FieldstgBackground *obj) {
    u8 order[5][6];
    FieldstgActor *player = (FieldstgActor *)heap_objects.find(5, -1, 0);
    s32 quarter = (obj->x & 0x7F) > 0x40;
    s32 table = fieldstg_background_order_by_dir[player->dir].table;
    u32 flip = fieldstg_background_order_by_dir[player->dir].flip;
    s32 row;
    s32 col;
    s32 r;
    s32 c;
    s32 tx;
    s32 ty;
    s32 i;
    s32 cx;
    s32 cy;

    if ((obj->y & 0x7F) > 0x10) {
        quarter |= 2;
    }
    for (row = 0; row < 5; row++) {
        for (col = 0; col < 6; col++) {
            c = col;
            if (flip & 2) {
                c = 5 - col;
            }
            r = row;
            if (flip & 1) {
                r = 4 - row;
            }
            order[row][col] = fieldstg_background_tile_orders[table][quarter][r][c];
        }
    }
    cx = (flip >> 1) & 1;
    cy = flip & 1;
    obj->first_x = obj->x / 128 - fieldstg_background_first_cells[quarter][0][cx];
    obj->first_y = obj->y / 128 - fieldstg_background_first_cells[quarter][1][cy];
    for (i = 0; i < 30; i++) {
        obj->visible[i] = 0xFF;
    }
    for (row = 0; row < 5; row++) {
        ty = row + obj->first_y;
        if (ty >= 0 && ty < obj->height) {
            for (col = 0; col < 6; col++) {
                tx = col + obj->first_x;
                if (tx >= 0 && tx < obj->width) {
                    obj->visible[order[row][col]] = tx + ty * obj->width;
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/fieldstg/nonmatchings/fieldstg_80085590", fieldstg_background_update_visible);
#endif

void fieldstg_background_update(FieldstgBackground *obj, FieldstgBackgroundData *data) {
    GfxLayer *layer;
    FieldstgBackgroundHeader *header;
    s32 count;
    s32 i;
    s32 j;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            if (cdload_reader.is_busy() == 0) {
                obj->header = heap_funcs.alloc(0x800, 2);
                cdload_reader.read(obj->file, 0, 1, obj->header, NULL);
                obj->base.next_step(obj);
            }
            break;
        case 1:
            if (cdload_reader.is_busy() != 1) {
                header = obj->header;
                obj->width = header->width;
                obj->height = header->height;
                obj->sectors = header->tile_size / 0x800;
                count = obj->width * obj->height;
                obj->tiles = heap_funcs.alloc(count * sizeof(FieldstgBackgroundTile), 2);
                for (j = 0; j < count; j++) {
                    obj->tiles[j].size = header->sizes[j];
                    obj->tiles[j].sectors = header->sizes[j] >> 11;
                }
                heap_funcs.free(obj->header);
                obj->header = NULL;
                for (i = 0; i < 30; i++) {
                    data->tiles[i] = fieldstg_tile_create(obj->sectors << 11, obj->file);
                }
                for (i = 0; i < 12; i++) {
                    obj->slots[i].tile = -1;
                    obj->slots[i].last_used = 0;
                }
                data->unpacker = message_rlen_create();
                obj->base.next_state(obj);
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        layer = gfx_module.funcs.get_layer(0x1002);
        layer->get_scroll(layer, &obj->x);
        fieldstg_background_update_visible(obj);
        fieldstg_background_draw(obj, data);
        fieldstg_background_load_tiles(obj, data);
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        if (obj->header != NULL) {
            heap_funcs.free(obj->header);
        }
        if (obj->tiles != NULL) {
            heap_funcs.free(obj->tiles);
        }
        break;
    }
}

s32 *fieldstg_background_get_size(FieldstgBackground *obj) {
    fieldstg_background_size[0] = obj->width << 7;
    fieldstg_background_size[1] = obj->height << 7;
    return fieldstg_background_size;
}

FieldstgBackground *fieldstg_background_create(s32 file) {
    FieldstgBackground *obj = object_create(fieldstg_background_update, sizeof(FieldstgBackground), sizeof(FieldstgBackgroundData), 4);

    obj->file = file;
    obj->get_size = fieldstg_background_get_size;
    return obj;
}

void fieldstg_background_fill_layer(s32 layer_id, s32 alpha) {
    GfxLayer *layer = gfx_module.funcs.get_layer(layer_id);
    s32 x;
    s32 y;

    if (layer != NULL) {
        for (y = 0; y < 0xF0; y += 0x80) {
            for (x = 0; x < 0x140; x += 0x80) {
                fieldstg_background_draw_blank(layer, x, y, alpha);
            }
        }
    }
}

void fieldstg_tile_touch(FieldstgTile *obj) {
    obj->last_used = gfx_module.funcs.get_time();
}

void fieldstg_tile_load(FieldstgTile *obj, s32 tile, s32 sectors) {
    if (obj->number != tile) {
        obj->number = tile;
        obj->read_done = 0;
        obj->slot = -1;
        obj->sector = tile * obj->sectors + 1;
        cdload_reader.read(obj->file, obj->sector, sectors, obj->buffer, &obj->read_done);
        obj->loaded_time = 0;
    }
    fieldstg_tile_touch(obj);
}

s32 fieldstg_tile_is_loaded(FieldstgTile *obj) {
    return obj->read_done;
}

void fieldstg_tile_draw(FieldstgTile *obj, GfxLayer *layer, s32 x, s32 y) {
    s32 pos[2];
    SPRT *sprt;
    u32 *ot;
    s32 i;
    s32 j;

    layer->get_scroll(layer, pos);
    sprt = gfx_module.funcs.get_packet();
    for (i = 0; i < 3; i++) {
        ot = layer->get_ot_entry(layer, fieldstg_tile_ot_depths[i]);
        for (j = 0; j < 5; j++) {
            if (obj->sprites[i][j].used != 0) {
                SetSprt(sprt);
                if (i == 2 || fieldstg_stage.color.cd != 0) {
                    setRGB0(sprt, fieldstg_stage.color.r, fieldstg_stage.color.g, fieldstg_stage.color.b);
                } else {
                    setRGB0(sprt, 0x80, 0x80, 0x80);
                }
                sprt->x0 = obj->sprites[i][j].x + x - pos[0];
                sprt->y0 = obj->sprites[i][j].y + y - pos[1];
                sprt->w = obj->sprites[i][j].w;
                sprt->h = obj->sprites[i][j].h;
                sprt->clut = getClut(obj->clut_x, obj->clut_y);
                sprt->u0 = obj->sprites[i][j].u;
                sprt->v0 = obj->tpage_y + obj->sprites[i][j].v;
                addPrim(ot, sprt);
                sprt++;
            }
        }
        SetDrawTPage((DR_TPAGE *)sprt, 0, 1, GetTPage(1, 0, obj->tpage_x, obj->tpage_y));
        addPrim(ot, (DR_TPAGE *)sprt);
        sprt = (SPRT *)((DR_TPAGE *)sprt + 1);
    }
    gfx_module.funcs.set_packet(sprt);
    fieldstg_tile_touch(obj);
}

void fieldstg_tile_unpack(FieldstgTile *obj, s32 slot, MessageRlen *unpacker) {
    obj->upload_slot = slot;
    obj->unpacker = unpacker;
    unpacker->unpack_start(unpacker, obj->buffer, 0x2800);
    obj->base.set_step(obj, 1);
}

void fieldstg_tile_upload(FieldstgTile *obj) {
    Tim tim;
    s32 *data = obj->data;
    s32 slot = obj->upload_slot;
    s16 *p = (s16 *)(data + 1);
    s32 count;
    s32 i;
    s32 j;

    for (i = 0; i < 3; i++) {
        count = *(s32 *)p;
        p += 2;
        for (j = 0; j < count; j++) {
            obj->sprites[i][j].used = 1;
            obj->sprites[i][j].x = *p++;
            obj->sprites[i][j].y = *p++;
            obj->sprites[i][j].u = *p++;
            obj->sprites[i][j].v = *p++;
            obj->sprites[i][j].w = *p++;
            obj->sprites[i][j].h = *p++;
        }
        for (; j < 5; j++) {
            obj->sprites[i][j].used = 0;
        }
    }
    obj->slot = slot;
    obj->tpage_x = fieldstg_tile_vram_pos[slot].x;
    obj->tpage_y = fieldstg_tile_vram_pos[slot].y;
    obj->clut_x = 0;
    obj->clut_y = slot + 0xF0;
    tim_init(&tim);
    tim.set_image_pos(obj->tpage_x, obj->tpage_y);
    tim.set_clut_pos(obj->clut_x, obj->clut_y);
    tim.load(cdload_module.get_subfile(0, data));
    fieldstg_tile_touch(obj);
}

void fieldstg_tile_free_slot(FieldstgTile *obj) {
    obj->slot = -1;
}

s32 fieldstg_tile_get_slot(FieldstgTile *obj) {
    return obj->slot;
}

s32 fieldstg_tile_get_number(FieldstgTile *obj) {
    return obj->number;
}

void fieldstg_tile_update(FieldstgTile *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            obj->data = obj->unpacker->get_data(obj->unpacker);
            if (obj->data != NULL) {
                fieldstg_tile_upload(obj);
                obj->base.set_step(obj, 0);
            }
            break;
        }
        if (obj->loaded_time == 0 && obj->read_done != 0) {
            obj->loaded_time = gfx_module.funcs.get_time();
        }
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        heap_funcs.free(obj->buffer);
        break;
    }
}

FieldstgTile *fieldstg_tile_create(s32 size, s32 file) {
    FieldstgTile *obj = object_new(fieldstg_tile_update, sizeof(FieldstgTile), 0);

    obj->load = fieldstg_tile_load;
    obj->draw = fieldstg_tile_draw;
    obj->unpack = fieldstg_tile_unpack;
    obj->get_number = fieldstg_tile_get_number;
    obj->is_loaded = fieldstg_tile_is_loaded;
    obj->get_slot = fieldstg_tile_get_slot;
    obj->free_slot = fieldstg_tile_free_slot;
    obj->touch = fieldstg_tile_touch;
    obj->file = file;
    obj->sectors = size / 0x800;
    obj->buffer = heap_funcs.alloc_top(size, 2);
    obj->number = -1;
    obj->slot = -1;
    obj->read_done = 1;
    return obj;
}

void fieldstg_update_main(Object *obj, struct FieldstgManager **data) {
    s32 value;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        value = gamestate_data.funcs.get_map();
        if (gamestate_data.field_last_map != value) {
            gamestate_data.field_last_map = value;
            gamestate_data.map_is_new = 1;
            gamestate_data.meter_random_count = 0x10;
        } else {
            gamestate_data.map_is_new = 0;
        }
        *data = fieldstg_manager_create();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The overlay's entry point (overlay_entries): overlay_run_object keeps the object (v0; PC_PORT: FINDINGS 8). */
OBJECT_V0(Object *) fieldstg_start(void) {
    OBJECT_V0_TAIL(object_new(fieldstg_update_main, sizeof(Object), 3 * sizeof(struct FieldstgManager *))) /* only [0] is used */
}

void fieldstg_map_title_create_windows(Object *obj, FieldstgMapTitleData *data) {
    s16 map = gamestate_data.funcs.get_map();
    s32 i;

    for (i = 0; fieldstg_map_titles[i].map != 0; i++) {
        if (fieldstg_map_titles[i].map == map) {
            data->window_0 = message_create_window(0x1003, 1, 0x80, 0x1A);
            data->window_0->set_text(data->window_0, cdload_module.files.get_file(records_language + 0xA9), fieldstg_map_titles[i].text_0);
            data->window_0->set_speed(data->window_0, 5);
            data->window_1 = message_create_window(0x1003, 1, 0x28, 0x44);
            data->window_1->set_text(data->window_1, cdload_module.files.get_file(records_language + 0xB7), fieldstg_map_titles[i].text_1);
            data->window_1->set_speed(data->window_1, 5);
            break;
        }
    }
}

/* Moves the two edges of one axis (moving 1: left/right, 2: top/bottom) towards their targets by speed. Each
 * case does its own done test; the compiler merges the two copies. */
void fieldstg_map_title_move_rect(Object *obj, FieldstgMapTitleRect *rect) {
    s32 first;
    s32 second;
    s32 target2;
    s32 target1;
    s32 speed;
    s32 size;

    switch (rect->moving) {
    case 1:
        first = rect->rect.x;
        size = rect->rect.w;
        second = first + size;
        target1 = rect->target_0;
        target2 = rect->target_1;
        speed = rect->speed;
        if (first < target1) {
            first += speed;
            if (first > target1) {
                first = target1;
            }
        } else if (first > target1) {
            first -= speed;
            if (first < target1) {
                first = target1;
            }
        }
        if (second < target2) {
            second += speed;
            if (second > target2) {
                second = target2;
            }
        } else if (second > target2) {
            second -= speed;
            if (second < target2) {
                second = target2;
            }
        }
        rect->rect.x = first;
        rect->rect.w = second - first;
        if (first == target1 && second == target2) {
            rect->moving = 0;
        }
        break;
    case 2:
        first = rect->rect.y;
        size = rect->rect.h;
        second = first + size;
        target1 = rect->target_0;
        target2 = rect->target_1;
        speed = rect->speed;
        if (first < target1) {
            first += speed;
            if (first > target1) {
                first = target1;
            }
        } else if (first > target1) {
            first -= speed;
            if (first < target1) {
                first = target1;
            }
        }
        if (second < target2) {
            second += speed;
            if (second > target2) {
                second = target2;
            }
        } else if (second > target2) {
            second -= speed;
            if (second < target2) {
                second = target2;
            }
        }
        rect->rect.y = first;
        rect->rect.h = second - first;
        if (first == target1 && second == target2) {
            rect->moving = 0;
        }
        break;
    }
}

void fieldstg_map_title_draw_rect(Object *obj, u32 *ot, RECT rect, u32 color) {
    POLY_F4 *poly = gfx_module.funcs.get_packet();

    setlen(poly, 5);
    *(u32 *)&poly->r0 = color;
    setcode(poly, 0x28);
    poly->x0 = rect.x;
    poly->x1 = rect.x + rect.w;
    poly->x2 = rect.x;
    poly->x3 = rect.x + rect.w;
    poly->y0 = rect.y;
    poly->y1 = rect.y;
    poly->y2 = rect.y + rect.h;
    poly->y3 = rect.y + rect.h;
    addPrim(ot, poly);
    gfx_module.funcs.set_packet(poly + 1);
}

void fieldstg_map_title_update(FieldstgMapTitle *obj, FieldstgMapTitleData *data) {
    GfxLayer *layer;
    s32 j;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->base.key1 == 0) {
            obj->base.set_state(obj, OBJECT_STATE_DONE);
            break;
        }
        for (j = 0; j < 10; j++) {
            obj->rects[j] = fieldstg_map_title_rects[j];
        }
        fieldstg_map_title_create_windows(&obj->base, data);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            obj->rects[0].shown = 1;
            obj->rects[5].shown = 1;
            if (obj->rects[5].moving != 0) {
                break;
            }
            obj->base.next_step(obj);
            /* fallthrough */
        case 1:
            obj->rects[8].shown = 1;
            obj->rects[9].shown = 1;
            obj->rects[6].shown = 1;
            obj->rects[1].shown = 1;
            obj->rects[2].shown = 1;
            obj->rects[7].shown = 1;
            obj->rects[3].shown = 1;
            if (obj->rects[9].moving != 0) {
                break;
            }
            obj->base.next_step(obj);
            /* fallthrough */
        case 2:
            obj->base.next_step(obj);
            /* fallthrough */
        case 3:
            obj->rects[4].shown = 1;
            if (obj->rects[4].moving != 0) {
                break;
            }
            obj->base.next_step(obj);
            break;
        case 4:
            if (data->window_0->is_done(data->window_0) != 0 && data->window_1->is_done(data->window_1) != 0) {
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        layer = gfx_module.funcs.get_layer(0x1003);
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            obj->base.substep += gfx_module.funcs.get_frame_ticks();
            if (obj->base.substep < 60) {
                break;
            }
            obj->base.next_step(obj);
            /* fallthrough */
        case 2:
            obj->clip.w = 0x140;
            obj->clip.x = 0;
            obj->clip.y = 0;
            obj->clip.h = 0xF0;
            obj->base.next_step(obj);
            /* fallthrough */
        case 3:
            obj->clip.y += 8;
            obj->clip.h -= 16;
            layer->set_clip_pos(layer, obj->clip.x, obj->clip.y);
            layer->set_clip_size(layer, obj->clip.w, obj->clip.h);
            if (obj->clip.h == 0) {
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
            break;
        }
        break;
    case OBJECT_STATE_END:
        fieldstg_stage.title_shown = 0;
        break;
    }
    if ((obj->base.state == OBJECT_STATE_RUN || obj->base.state == OBJECT_STATE_DONE) && obj->base.key1 != 0) {
        /* Draw the rectangles. */
        GfxLayer *layer = gfx_module.funcs.get_layer(0x1003);
        u32 *ot = layer->get_ot_entry(layer, 1);
        s32 i;

        for (i = 0; i < 10; i++) {
            if (obj->rects[i].shown != 0) {
                fieldstg_map_title_move_rect(&obj->base, &obj->rects[i]);
                fieldstg_map_title_draw_rect(&obj->base, ot, obj->rects[i].rect, obj->rects[i].color);
            }
        }
    }
}

Object *fieldstg_map_title_create(s32 show) {
    Object *obj = object_create(fieldstg_map_title_update, sizeof(FieldstgMapTitle), sizeof(FieldstgMapTitleData), 9);

    obj->key1 = show;
    fieldstg_stage.title_shown = 1;
    return obj;
}

/* .data (address order) */

FieldstgAnimFrame D_FIELDSTG_80096CC8[19] = {
    { 0, 1 }, { 0, 13 }, { 1, 14 }, { 2, 15 }, { 3, 14 }, { 0, 13 }, { 1, 14 }, { 2, 15 }, { 3, 14 }, { 0, 6 },
    { 1, 6 }, { 2, 6 }, { 4, 7 }, { 5, 5 }, { 6, 4 }, { 7, 4 }, { 8, 4 }, { 9, 4 }, { -1, -1 },
};

FieldstgAnimFrame D_FIELDSTG_80096D14[11] = {
    { 0, 1 }, { 1, 8 }, { 2, 6 }, { 3, 4 }, { 4, 4 }, { 3, 4 }, { 4, 4 }, { 3, 6 }, { 4, 4 }, { 3, 4 }, { -1, 7 },
};

FieldstgAnimFrame D_FIELDSTG_80096D40[10] = {
    { 0, 1 }, { 9, 18 }, { 5, 4 }, { 6, 5 }, { 7, 4 }, { 8, 4 }, { 6, 4 }, { 7, 4 }, { 8, 4 }, { -1, 5 },
};

FieldstgAnimFrame D_FIELDSTG_80096D68[33] = {
    { 0, 4 }, { 1, 4 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 8, 4 }, { 9, 4 }, { 10, 4 },
    { 11, 4 }, { 12, 4 }, { 13, 4 }, { 14, 4 }, { 15, 4 }, { 16, 4 }, { 17, 4 }, { 18, 4 }, { 19, 4 }, { 20, 4 },
    { 21, 4 }, { 22, 4 }, { 23, 4 }, { 24, 4 }, { 25, 4 }, { 26, 4 }, { 27, 4 }, { 28, 4 }, { 29, 4 }, { 30, 4 },
    { 31, 4 }, { 255, 999 },
};

FieldstgAnimFrame D_FIELDSTG_80096DEC[5] = { { 300, 28 }, { 32, 4 }, { 33, 24 }, { 34, 4 }, { 255, 999 } };
FieldstgAnimFrame D_FIELDSTG_80096E00[5] = { { 300, 28 }, { 35, 4 }, { 36, 24 }, { 37, 4 }, { 255, 999 } };

FieldstgAnimFrame D_FIELDSTG_80096E14[8] = {
    { 300, 104 }, { 38, 4 }, { 39, 4 }, { 40, 4 }, { 41, 4 }, { 42, 4 }, { 43, 4 }, { 255, 999 },
};

FieldstgAnimFrame D_FIELDSTG_80096E34[8] = {
    { 300, 104 }, { 44, 4 }, { 45, 4 }, { 46, 4 }, { 47, 4 }, { 48, 4 }, { 49, 4 }, { 255, 0 },
};

FieldstgAnimFrame D_FIELDSTG_80096E54[12] = {
    { 300, 128 }, { 50, 4 }, { 51, 4 }, { 52, 4 }, { 53, 4 }, { 54, 4 }, { 55, 4 }, { 56, 4 }, { 57, 4 }, { 58, 4 },
    { 59, 4 }, { 255, 999 },
};

FieldstgAnimFrame *fieldstg_warp_effect_anims[2][4] = {
    { D_FIELDSTG_80096D68, D_FIELDSTG_80096DEC, D_FIELDSTG_80096E14, D_FIELDSTG_80096E54 },
    { D_FIELDSTG_80096D68, D_FIELDSTG_80096E00, D_FIELDSTG_80096E34, D_FIELDSTG_80096E54 },
};

u8 fieldstg_background_tile_orders[3][4][5][6] = {
    { { { 0x19, 0xE, 9, 0xA, 0xB, 0xF }, { 0x1A, 0xC, 6, 7, 8, 0xD }, { 0x1B, 0x10, 2, 0, 3, 0x11 }, { 0x1C, 0x12, 4, 1, 5, 0x13 }, { 0x1D, 0x14, 0x15, 0x16, 0x17, 0x18 } }, { { 0x14, 0xC, 0xD, 0xE, 0xF, 0x15 }, { 0x10, 8, 9, 0xA, 0xB, 0x11 }, { 0x12, 4, 0, 1, 5, 0x13 }, { 0x16, 6, 2, 3, 7, 0x17 }, { 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D } }, { { 0x19, 0xC, 9, 0xA, 0xB, 0xD }, { 0x1A, 0xE, 2, 1, 3, 0xF }, { 0x1B, 0x10, 4, 0, 5, 0x11 }, { 0x1C, 0x12, 6, 7, 8, 0x13 }, { 0x1D, 0x14, 0x15, 0x16, 0x17, 0x18 } }, { { 0x10, 0xC, 0xD, 0xE, 0xF, 0x11 }, { 0x12, 4, 2, 3, 5, 0x13 }, { 0x14, 6, 0, 1, 7, 0x15 }, { 0x16, 8, 9, 0xA, 0xB, 0x17 }, { 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D } } },
    { { { 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D }, { 0xC, 0xA, 0xE, 0x10, 0x12, 0x14 }, { 8, 6, 2, 0, 4, 0x15 }, { 9, 7, 3, 1, 5, 0x16 }, { 0xD, 0xB, 0xF, 0x11, 0x13, 0x17 } }, { { 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D }, { 0xC, 0xA, 0xE, 0x10, 0x12, 0x14 }, { 8, 4, 0, 1, 6, 0x15 }, { 9, 5, 2, 3, 7, 0x16 }, { 0xD, 0xB, 0xF, 0x11, 0x13, 0x17 } }, { { 0x17, 0xF, 0x11, 0x13, 0x15, 0x19 }, { 0xC, 9, 3, 1, 6, 0x1A }, { 0xD, 0xA, 4, 0, 7, 0x1B }, { 0xE, 0xB, 5, 2, 8, 0x1C }, { 0x18, 0x10, 0x12, 0x14, 0x16, 0x1D } }, { { 0xF, 0x11, 0x13, 0x15, 0x17, 0x19 }, { 0xC, 4, 2, 7, 9, 0x1A }, { 0xD, 5, 0, 1, 0xA, 0x1B }, { 0xE, 6, 3, 8, 0xB, 0x1C }, { 0x10, 0x12, 0x14, 0x16, 0x18, 0x1D } } },
    { { { 0xC, 0xF, 0x10, 0x11, 0x13, 0x15 }, { 0xD, 6, 8, 0xA, 0xB, 0x17 }, { 0xE, 7, 2, 0, 4, 0x19 }, { 0x12, 9, 3, 1, 5, 0x1B }, { 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1D } }, { { 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D }, { 8, 0xA, 0xC, 0xD, 0xE, 0x10 }, { 9, 4, 0, 1, 6, 0x12 }, { 0xB, 5, 2, 3, 7, 0x14 }, { 0xF, 0x11, 0x13, 0x15, 0x16, 0x17 } }, { { 0x19, 9, 0xC, 0xD, 0xF, 0x11 }, { 0x1A, 0xA, 3, 2, 5, 0x13 }, { 0x1B, 0xB, 1, 0, 7, 0x15 }, { 0x1C, 0xE, 4, 6, 8, 0x17 }, { 0x1D, 0x10, 0x12, 0x14, 0x16, 0x18 } }, { { 0xC, 0xF, 0x10, 0x12, 0x13, 0x15 }, { 0xD, 4, 3, 5, 8, 0x17 }, { 0xE, 2, 0, 1, 0xA, 0x19 }, { 0x11, 6, 7, 9, 0xB, 0x1C }, { 0x14, 0x16, 0x18, 0x1A, 0x1B, 0x1D } } },
};

FieldstgTileOrder fieldstg_background_order_by_dir[8] = {
    { 0, 1 }, { 2, 1 }, { 1, 0 }, { 2, 0 }, { 0, 0 }, { 2, 2 }, { 1, 2 }, { 2, 3 },
};

u8 fieldstg_background_first_cells[4][2][2] = { { { 2, 1 }, { 2, 1 } }, { { 1, 1 }, { 2, 1 } }, { { 2, 1 }, { 1, 1 } }, { { 1, 1 }, { 1, 1 } } };
s32 fieldstg_tile_ot_depths[3] = { 7, 3, 11 };

FieldstgTileVramPos fieldstg_tile_vram_pos[12] = {
    { 640, 0 }, { 640, 128 }, { 768, 0 }, { 768, 128 }, { 896, 0 }, { 896, 128 }, { 640, 256 }, { 640, 384 },
    { 768, 256 }, { 768, 384 }, { 896, 256 }, { 896, 384 },
};

FieldstgMapTitleText fieldstg_map_titles[240] = {
    { 0xB, 1, 512 }, { 0xB, 0x76, 624 }, { 0xB, 1, 513 }, { 0xB, 0x76, 625 }, { 0xB, 2, 514 }, { 0xB, 0x77, 626 },
    { 1, 3, 515 }, { 6, 3, 627 }, { 1, 3, 516 }, { 0xB, 1, 517 }, { 0xB, 0x76, 628 }, { 1, 4, 518 }, { 6, 4, 629 },
    { 1, 5, 519 }, { 6, 5, 630 }, { 1, 6, 520 }, { 6, 6, 631 }, { 1, 7, 521 }, { 6, 7, 632 }, { 1, 8, 522 },
    { 6, 0x78, 633 }, { 1, 0x87, 523 }, { 6, 0x87, 634 }, { 1, 9, 524 }, { 6, 0x79, 635 }, { 1, 0xA, 525 },
    { 6, 0x7A, 636 }, { 1, 0xB, 526 }, { 6, 0xB, 637 }, { 1, 0xC, 527 }, { 6, 0xC, 638 }, { 1, 0xD, 528 },
    { 6, 0xD, 639 }, { 1, 0xE, 529 }, { 6, 0xE, 640 }, { 1, 0xF, 530 }, { 6, 0xF, 641 }, { 1, 0x10, 531 },
    { 6, 0x10, 642 }, { 1, 0x11, 532 }, { 6, 0x11, 643 }, { 1, 0x12, 533 }, { 6, 0x12, 644 }, { 1, 0x13, 534 },
    { 6, 0x13, 645 }, { 1, 0x14, 535 }, { 6, 0x14, 646 }, { 1, 0x15, 536 }, { 6, 0x15, 647 }, { 1, 0x16, 537 },
    { 6, 0x16, 648 }, { 1, 0x17, 538 }, { 6, 0x17, 649 }, { 1, 0x18, 539 }, { 6, 0x7B, 650 }, { 1, 0x19, 540 },
    { 6, 0x19, 651 }, { 0xB, 0x1A, 541 }, { 0xB, 0x1A, 652 }, { 0xB, 0x1B, 542 }, { 0xB, 0x1B, 653 },
    { 0xB, 0x1C, 543 }, { 0xB, 0x1C, 654 }, { 0xB, 0x1D, 544 }, { 0xB, 0x1D, 655 }, { 0xC, 0x1E, 545 },
    { 0xC, 0x1E, 656 }, { 0xC, 0x1F, 546 }, { 0xC, 0x1F, 657 }, { 0xC, 0x20, 547 }, { 0xC, 0x20, 658 },
    { 0xC, 0x21, 548 }, { 0xC, 0x21, 659 }, { 0xC, 0x22, 549 }, { 0xC, 0x22, 660 }, { 0xC, 0x23, 550 },
    { 0xC, 0x23, 661 }, { 0xC, 0x24, 551 }, { 0xC, 0x24, 662 }, { 0xC, 0x25, 552 }, { 0xC, 0x25, 663 },
    { 0xC, 0x26, 553 }, { 0xC, 0x26, 664 }, { 0xC, 0x27, 554 }, { 0xC, 0x27, 665 }, { 0xC, 0x28, 555 },
    { 0xC, 0x28, 666 }, { 0xC, 0x29, 556 }, { 0xC, 0x29, 667 }, { 0xC, 0x2A, 557 }, { 0xC, 0x2B, 558 },
    { 0xC, 0x7C, 668 }, { 2, 0x2C, 559 }, { 7, 0x2C, 669 }, { 2, 0x2D, 560 }, { 7, 0x7D, 670 }, { 2, 0x2E, 561 },
    { 7, 0x2E, 671 }, { 0xD, 0x2F, 562 }, { 0xD, 0x2F, 672 }, { 0xD, 0x30, 563 }, { 0xD, 0x30, 673 },
    { 0xD, 0x31, 564 }, { 0xD, 0x31, 674 }, { 0xD, 0x32, 565 }, { 0xD, 0x32, 675 }, { 0xD, 0x33, 566 },
    { 0xD, 0x34, 567 }, { 0xD, 0x34, 676 }, { 0xD, 0x35, 568 }, { 0xD, 0x35, 677 }, { 0xD, 0x36, 569 },
    { 0xD, 0x36, 678 }, { 0xD, 0x37, 570 }, { 0xD, 0x37, 679 }, { 0xD, 0x38, 571 }, { 0xD, 0x38, 680 },
    { 0xD, 0x39, 572 }, { 0xD, 0x39, 681 }, { 0xD, 0x3A, 573 }, { 0xD, 0x3A, 682 }, { 0xD, 0x3B, 574 },
    { 0xD, 0x7E, 683 }, { 3, 0x3C, 575 }, { 8, 0x7F, 684 }, { 3, 0x3D, 576 }, { 8, 0x80, 685 }, { 0xD, 0x3E, 577 },
    { 0xD, 0x81, 686 }, { 0xD, 0x3F, 578 }, { 0xD, 0x3F, 687 }, { 0xD, 0x40, 579 }, { 0xD, 0x40, 580 },
    { 0xD, 0x40, 688 }, { 0x10, 0x42, 581 }, { 0x10, 0x43, 582 }, { 0xE, 0x44, 583 }, { 0xE, 0x44, 689 },
    { 0xE, 0x45, 584 }, { 0xE, 0x45, 690 }, { 0xE, 0x46, 585 }, { 0xE, 0x46, 691 }, { 0xE, 0x47, 586 },
    { 0xE, 0x47, 692 }, { 0xE, 0x48, 587 }, { 0xE, 0x48, 693 }, { 0xE, 0x49, 588 }, { 0xE, 0x49, 694 },
    { 0xE, 0x4A, 589 }, { 0xE, 0x4A, 695 }, { 0xE, 0x4B, 590 }, { 0xE, 0x4B, 696 }, { 0xE, 0x4C, 591 },
    { 0xE, 0x4C, 697 }, { 0xE, 0x4D, 592 }, { 0xE, 0x4D, 698 }, { 0xE, 0x4E, 593 }, { 0xE, 0x4E, 699 },
    { 0xE, 0x4F, 594 }, { 0xE, 0x4F, 700 }, { 0xE, 0x50, 595 }, { 0xE, 0x50, 701 }, { 0xE, 0x51, 596 },
    { 0xE, 0x51, 702 }, { 0xE, 0x53, 597 }, { 0xE, 0x53, 703 }, { 0xE, 0x53, 598 }, { 0xE, 0x54, 599 },
    { 0xE, 0x54, 704 }, { 0xE, 0x55, 600 }, { 0xE, 0x56, 705 }, { 0xE, 0x56, 601 }, { 0xE, 0x56, 706 },
    { 0xE, 0x57, 602 }, { 0xE, 0x57, 707 }, { 0xE, 0x58, 603 }, { 0xE, 0x58, 708 }, { 0xE, 0x59, 604 },
    { 0xE, 0x59, 709 }, { 0xE, 0x5A, 605 }, { 0xE, 0x82, 710 }, { 4, 0x5B, 606 }, { 9, 0x83, 711 },
    { 4, 0x5C, 607 }, { 4, 0x5D, 608 }, { 9, 0x5D, 712 }, { 0xF, 0x5E, 609 }, { 0xF, 0x5E, 713 },
    { 0xF, 0x5F, 610 }, { 0xF, 0x5F, 714 }, { 0xF, 0x60, 611 }, { 0xF, 0x60, 715 }, { 0xF, 0x61, 612 },
    { 0xF, 0x61, 716 }, { 0xF, 0x62, 613 }, { 0xF, 0x62, 717 }, { 0xF, 0x63, 614 }, { 0xF, 0x63, 718 },
    { 0xF, 0x64, 615 }, { 0xF, 0x64, 719 }, { 0xF, 0x65, 616 }, { 0xF, 0x65, 720 }, { 0xF, 0x66, 617 },
    { 0xF, 0x66, 721 }, { 0xF, 0x67, 618 }, { 0xF, 0x67, 722 }, { 0xF, 0x68, 619 }, { 0xF, 0x68, 723 },
    { 0xF, 0x69, 620 }, { 0xF, 0x69, 724 }, { 0xF, 0x6A, 621 }, { 0xF, 0x6B, 622 }, { 0xF, 0x6B, 725 },
    { 0xF, 0x6C, 623 }, { 0xF, 0x84, 726 }, { 0x11, 0x6D, 727 }, { 0x11, 0x6E, 728 }, { 0x11, 0x6A, 729 },
    { 0x12, 0x70, 730 }, { 0x12, 0x71, 731 }, { 0x12, 0x72, 732 }, { 0x13, 0x73, 733 }, { 0x13, 0x74, 734 },
    { 0x13, 0x75, 735 }, { 0x15, 0x85, 736 }, { 0x15, 0x85, 737 }, { 0x15, 0x85, 738 }, { 0x15, 0x85, 739 },
    { 0x15, 0x85, 740 }, { 0x15, 0x85, 741 }, { 0x15, 0x85, 742 }, { 0x15, 0x85, 743 }, { 0x15, 0x86, 744 },
    { 0x15, 0x86, 745 }, { 0x15, 0x86, 746 }, { 0x15, 0x86, 747 }, { 0x15, 0x86, 748 }, { 0x15, 0x86, 749 },
    { 0x15, 0x86, 750 }, { 0, 0, 0 },
};

FieldstgMapTitleRect fieldstg_map_title_rects[10] = {
    { 0, { 320, 41, 0, 1 }, 0xFFE400, 1, 124, 320, 8, 0 }, { 0, { 0, 87, 320, 0 }, 0xFFE400, 2, 86, 88, 2, 0 },
    { 0, { 0, 0, 2, 240 }, 0xFFE400, 1, 31, 32, 2, 0 }, { 0, { 0, 0, 1, 240 }, 0xFFE400, 1, 13, 14, 2, 0 },
    { 0, { 227, 0, 1, 0 }, 0xFFE400, 2, 0, 240, 16, 0 }, { 0, { 320, 40, 0, 3 }, 0xC83E3E, 1, 124, 320, 8, 0 },
    { 0, { 0, 87, 320, 0 }, 0xC83E3E, 2, 81, 93, 2, 0 }, { 0, { 0, 0, 8, 240 }, 0xC83E3E, 1, 27, 36, 2, 0 },
    { 0, { 125, 16, 116, 0 }, 0x800000, 2, 16, 47, 2, 0 }, { 0, { 0, 87, 320, 0 }, 0x800000, 2, 66, 97, 2, 0 },
};
