#include "common.h"
#include "object.h"
#include "heap.h"
#include "gfx.h"
#include "cdload.h"
#include "filetable.h"
#include "gamestate.h"
#include "sound.h"
#include "pad.h"
#include "message.h"
#include "records.h"
#include "psyq/libcd.h"
#include "psyq/libgpu.h"
#include "psyq/libpress.h"
#include "stdwtitl.h"

/* Data block of stdwtitl_picture_screen_update's object. */
typedef struct StdwtitlPictureScreenData {
    /* 0x0 */ Object *picture; /* stdwtitl_picture_create's object */
} StdwtitlPictureScreenData;

/* Data block of the title's root object (stdwtitl_root_update). */
typedef struct StdwtitlRootData {
    /* 0x0 */ Object *title; /* stdwtitl_title_screen_create's object */
    /* 0x4 */ Object *movie; /* the movie (stdwtitl_movie_screen_create) */
    /* 0x8 */ Object *picture; /* stdwtitl_picture_screen_create's object */
} StdwtitlRootData;

extern RECT stdwtitl_screen_rect;

void stdwtitl_picture_screen_update(Object *obj, StdwtitlPictureScreenData *data) {
    Tim tim;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0xA000);
        gfx_module.funcs.init_display(0x140, 0xF0, 0, 0);
        tim_init(&tim);
        tim.set_image_pos(0x280, 0);
        tim.load_all(cdload_module.get_subfile_by_id(0x08A60001));
        layer = gfx_module.funcs.create_layer(&stdwtitl_screen_rect, 2, 0x100);
        layer->set_bg_color(layer, 0x1F, 0x1F, 0x1F);
        data->picture = stdwtitl_picture_create();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *stdwtitl_picture_screen_create(void) {
    return object_new(stdwtitl_picture_screen_update, sizeof(Object), sizeof(StdwtitlPictureScreenData));
}

/* The title's root object: by the story state, the title screen, a movie, or stdwtitl_picture_screen_update's object. */
void stdwtitl_root_update(Object *obj, StdwtitlRootData *data) {
    RECT rect;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        switch (gamestate_data.funcs.get_map() & 0xFF) {
        case 0:
            gfx_module.reset();
            gfx_module.alloc_packet_buffers(0x14000);
            gfx_module.funcs.init_display(0x140, 0xF0, 0, 0);
            rect.x = 0;
            rect.y = 0;
            rect.w = 0x140;
            rect.h = 0xF0;
            layer = gfx_module.funcs.create_layer(&rect, 2, 0x1000);
            layer->set_bg_color(layer, 0, 0, 0);
            data->title = stdwtitl_title_screen_create();
            break;
        case 13:
            data->picture = stdwtitl_picture_screen_create();
            break;
        default:
            data->movie = stdwtitl_movie_screen_create((gamestate_data.funcs.get_map() & 0xFF) - 1);
            break;
        }
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The overlay's entry point (overlay_entries): overlay_run_object keeps the object (v0; PC_PORT: FINDINGS 8). */
OBJECT_V0(Object *) stdwtitl_create_root(void) {
    OBJECT_V0_TAIL(object_new(stdwtitl_root_update, sizeof(Object), sizeof(StdwtitlRootData)))
}

extern s16 stdwtitl_glow_frames[2][13]; /* frames of the two animations, ended by -1 */

void stdwtitl_glow_draw(StdwtitlGlow *obj) {
    Sprite spr;

    if (obj->shown != 0) {
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, 0);
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(stdwtitl_module.bank), 6, 0x1D, 0xD1);
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, 0);
        spr.set_vram_pos(0x280, 0x100);
        spr.set_palette(obj->anims[0].frame);
        spr.draw(cdload_module.get_subfile_by_id(stdwtitl_module.bank), 3, 8, 0x1C);
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, 0);
        spr.set_vram_pos(0x280, 0x100);
        spr.set_palette(obj->anims[1].frame);
        spr.draw(cdload_module.get_subfile_by_id(stdwtitl_module.bank), 5, 0x14, 0xCA);
    }
}

void stdwtitl_glow_update(StdwtitlGlow *obj) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->skip == 0) {
            obj->base.next_state(obj);
        } else {
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            obj->shown = 1;
            obj->base.next_step(obj);
            obj->anims[0].index = 0;
            obj->anims[1].index = 0;
            /* fallthrough */
        case 2:
            for (i = 0; i < 2; i++) {
                if (obj->anims[i].done == 0) {
                    if (stdwtitl_glow_frames[i][obj->anims[i].index] == -1) {
                        obj->anims[i].done = 1;
                        obj->anims[i].index--;
                    }
                    obj->anims[i].frame = stdwtitl_glow_frames[i][obj->anims[i].index];
                    obj->anims[i].index++;
                }
            }
            if (obj->anims[0].done + obj->anims[1].done == 2) {
                obj->base.set_step(obj, 2);
            }
            stdwtitl_glow_draw(obj);
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->anims[0].frame = 0xB;
            obj->anims[1].frame = 7;
            obj->shown = 1;
            obj->base.next_step(obj);
        }
        stdwtitl_glow_draw(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void stdwtitl_glow_start(StdwtitlGlow *obj) {
    if (obj->base.state == OBJECT_STATE_RUN) {
        obj->base.set_step(obj, 1);
    }
}

StdwtitlGlow *stdwtitl_glow_create(s32 arg0) {
    StdwtitlGlow *obj = object_new(stdwtitl_glow_update, sizeof(StdwtitlGlow), 0);

    obj->start = stdwtitl_glow_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->skip = arg0;
    return obj;
}

/* The movie player: Sony's MDEC streaming sample (DECENV, strNext, strCallback, ...), 24-bit. */

/* The decoder's state (DECENV in Sony's sample). */
typedef struct StdwtitlDecEnv {
    /* 0x00 */ u32 *vlcbuf[2];
    /* 0x08 */ s32 vlcid;
    /* 0x0C */ u32 *imgbuf[2];
    /* 0x14 */ s32 imgid;
    /* 0x18 */ RECT rect[2];
    /* 0x28 */ s32 rectid;
    /* 0x2C */ RECT slice;
    /* 0x34 */ s32 isdone;
} StdwtitlDecEnv; /* size 0x38 */

extern s32 stdwtitl_frame_width; /* frame width */
extern s32 stdwtitl_frame_height; /* frame height */
extern RECT stdwtitl_vram_rect;
StdwtitlDecEnv stdwtitl_decoder;
u32 *stdwtitl_ring_buffer; /* ring buffer */
u16 *stdwtitl_vlc_table;  /* VLC table (DecDCTvlcBuild) */
u32 *stdwtitl_vlc_buffer_0; /* VLC buffers */
u32 *stdwtitl_vlc_buffer_1;
u32 *stdwtitl_image_buffer_0; /* image buffers */
u32 *stdwtitl_image_buffer_1;
s32 stdwtitl_movie_done; /* reached the last frame */
s32 stdwtitl_movie_file; /* movie file ID */
u32 stdwtitl_last_frame; /* last frame */

void stdwtitl_clear_screen(void) {
    ResetGraph(1);
    ClearImage2(&stdwtitl_vram_rect, 0, 0, 0);
    DrawSync(0);
}

void stdwtitl_init_decoder(StdwtitlDecEnv *dec, s16 x0, s16 y0, s16 x1, s16 y1) {
    dec->vlcbuf[0] = stdwtitl_vlc_buffer_0;
    dec->vlcbuf[1] = stdwtitl_vlc_buffer_1;
    dec->vlcid = gfx_module.buffer ^ 1;
    dec->imgbuf[0] = stdwtitl_image_buffer_0;
    dec->imgbuf[1] = stdwtitl_image_buffer_1;
    dec->imgid = gfx_module.buffer ^ 1;
    dec->rect[0].x = x0;
    dec->rect[0].y = y0;
    dec->rect[1].x = x1;
    dec->rect[1].y = y1;
    dec->rectid = gfx_module.buffer ^ 1;
    dec->slice.x = x0;
    dec->slice.y = y0;
    dec->slice.w = 0x18;
    dec->isdone = 0;
}

void stdwtitl_start_cd_stream(CdlLOC *loc) {
    u8 param;

    param = 0x80;
    do {
        while (CdControl(2, (u8 *)loc, 0) == 0) {
            PLATFORM_WAIT();
        }
        while (CdControl(0xE, &param, 0) == 0) {
            PLATFORM_WAIT();
        }
    } while (CdRead2(0x1E0) == 0);
}

void stdwtitl_init_stream(CdlLOC *loc, void (*callback)()) {
    DecDCTReset(0);
    DecDCToutCallback(callback);
    StSetRing(stdwtitl_ring_buffer, 0x20);
    StSetStream(1, 1, -1, 0, 0);
    stdwtitl_start_cd_stream(loc);
}

u32 *stdwtitl_get_next_frame(StdwtitlDecEnv *dec) {
    u32 *addr;
    StHEADER *sector;
    s32 cnt = 2000;

    while (StGetNext(&addr, (u32 **)&sector) != 0) {
        if (--cnt == 0) {
            return NULL;
        }
    }
    if (sector->frameCount >= stdwtitl_last_frame) {
        stdwtitl_movie_done = 1;
    }
    if (stdwtitl_frame_width != sector->width || stdwtitl_frame_height != sector->height) {
        stdwtitl_clear_screen();
        stdwtitl_frame_width = sector->width;
        stdwtitl_frame_height = sector->height;
    }
    dec->rect[0].w = dec->rect[1].w = stdwtitl_frame_width * 3 / 2;
    dec->rect[0].h = dec->rect[1].h = stdwtitl_frame_height;
    dec->slice.h = stdwtitl_frame_height;
    return addr;
}

s32 stdwtitl_decode_next_vlc(StdwtitlDecEnv *dec) {
    s32 cnt = 2000;
    u32 *next;

    while ((next = stdwtitl_get_next_frame(dec)) == NULL) {
        if (--cnt == 0) {
            return -1;
        }
    }
    dec->vlcid = dec->vlcid ? 0 : 1;
    DecDCTvlc2(next, dec->vlcbuf[dec->vlcid], stdwtitl_vlc_table);
    StFreeRing(next);
    return 0;
}

void stdwtitl_decode_callback(void) {
    RECT snap_rect;
    s32 id;

    if (D_80081454) {
        StCdInterrupt();
        D_80081454 = 0;
    }
    id = stdwtitl_decoder.imgid;
    snap_rect = stdwtitl_decoder.slice;
    stdwtitl_decoder.imgid = stdwtitl_decoder.imgid ? 0 : 1;
    stdwtitl_decoder.slice.x += stdwtitl_decoder.slice.w;
    if (stdwtitl_decoder.rectid != 0) {
        snap_rect.x += 0x1E0;
    }
    snap_rect.y = 0x24;
    if (stdwtitl_decoder.slice.x
        < stdwtitl_decoder.rect[stdwtitl_decoder.rectid].x + stdwtitl_decoder.rect[stdwtitl_decoder.rectid].w) {
        DecDCTout(stdwtitl_decoder.imgbuf[stdwtitl_decoder.imgid],
                  stdwtitl_decoder.slice.w * stdwtitl_decoder.slice.h / 2);
    } else {
        stdwtitl_decoder.isdone = 1;
        stdwtitl_decoder.rectid = stdwtitl_decoder.rectid ? 0 : 1;
        stdwtitl_decoder.slice.x = stdwtitl_decoder.rect[stdwtitl_decoder.rectid].x;
        stdwtitl_decoder.slice.y = stdwtitl_decoder.rect[stdwtitl_decoder.rectid].y;
    }
    DrawSync(0);
    LoadImage(&snap_rect, stdwtitl_decoder.imgbuf[id]);
}

void stdwtitl_wait_decode(StdwtitlDecEnv *dec, s32 mode) {
    volatile s32 cnt = 0x800000;

    while (dec->isdone == 0) {
        if (--cnt == 0) {
            dec->isdone = 1;
            dec->rectid = dec->rectid ? 0 : 1;
            dec->slice.x = dec->rect[dec->rectid].x;
            dec->slice.y = dec->rect[dec->rectid].y;
        }
    }
    dec->isdone = 0;
}

/* Object of stdwtitl_update_movie (the movie player), created by stdwtitl_create_movie. */
typedef struct StdwtitlMoviePlayer {
    /* 0x00 */ Object base;
    /* 0x50 */ CdlLOC loc;    /* the movie's position */
} StdwtitlMoviePlayer; /* size 0x54 */

void stdwtitl_update_movie(StdwtitlMoviePlayer *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        stdwtitl_clear_screen();
        stdwtitl_ring_buffer = heap_funcs.alloc(0x10000, 2);
        stdwtitl_vlc_buffer_0 = heap_funcs.alloc(0x28000, 2);
        stdwtitl_vlc_buffer_1 = heap_funcs.alloc(0x28000, 2);
        stdwtitl_image_buffer_0 = heap_funcs.alloc(0x4E00, 2);
        stdwtitl_image_buffer_1 = heap_funcs.alloc(0x4E00, 2);
        stdwtitl_vlc_table = heap_funcs.alloc(0x11000, 2);
        stdwtitl_init_decoder(&stdwtitl_decoder, 0, 0, 0, 0x1A0);
        filetable_funcs.get_cdloc(stdwtitl_movie_file, 0, &obj->loc);
        stdwtitl_init_stream(&obj->loc, stdwtitl_decode_callback);
        DecDCTvlcBuild(stdwtitl_vlc_table);
        stdwtitl_decode_next_vlc(&stdwtitl_decoder);
        stdwtitl_movie_done = 0;
        obj->base.next_state(obj);
        /* fallthrough */
    case OBJECT_STATE_RUN:
        DecDCTin(stdwtitl_decoder.vlcbuf[stdwtitl_decoder.vlcid], 3);
        DecDCTout(stdwtitl_decoder.imgbuf[stdwtitl_decoder.imgid],
                  stdwtitl_decoder.slice.w * stdwtitl_decoder.slice.h / 2);
        stdwtitl_decode_next_vlc(&stdwtitl_decoder);
        stdwtitl_wait_decode(&stdwtitl_decoder, 0);
        if (stdwtitl_movie_done == 1 || (pad_state.get_pressed(0) & 8)) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        CdControlB(9, 0, 0);
        DecDCToutCallback(0);
        StUnSetRing();
        heap_funcs.free(stdwtitl_ring_buffer);
        heap_funcs.free(stdwtitl_vlc_buffer_0);
        heap_funcs.free(stdwtitl_vlc_buffer_1);
        heap_funcs.free(stdwtitl_image_buffer_0);
        heap_funcs.free(stdwtitl_image_buffer_1);
        heap_funcs.free(stdwtitl_vlc_table);
        DrawSync(0);
        stdwtitl_clear_screen();
        gfx_module.funcs.set_display_area(0, 0, 0x140, 0xF0);
        break;
    }
}

Object *stdwtitl_create_movie(s32 file, s32 last_frame) {
    Object *obj = object_new(stdwtitl_update_movie, sizeof(StdwtitlMoviePlayer), 0);

    stdwtitl_movie_file = file;
    stdwtitl_last_frame = last_frame;
    return obj;
}

/* A movie of stdwtitl_movies. */
typedef struct StdwtitlMovie {
    /* 0x0 */ s32 file;       /* file ID */
    /* 0x4 */ s32 last_frame;
    /* 0x8 */ s32 next_map;   /* passed to gamestate_data.funcs.set_next_map when it ends */
} StdwtitlMovie; /* size 0xC */

extern StdwtitlMovie stdwtitl_movies[14];   /* [11..13]: movie 11 in Japanese, English, the European languages */

/* Data block of stdwtitl_movie_screen_update's object. */
typedef struct StdwtitlMovieScreenData {
    /* 0x0 */ Object *player; /* the movie player */
} StdwtitlMovieScreenData;

/* Object of stdwtitl_movie_screen_update, created by stdwtitl_movie_screen_create. */
typedef struct StdwtitlMovieScreen {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 movie;  /* movie (index into stdwtitl_movies) */
    /* 0x54 */ s32 next_map;
} StdwtitlMovieScreen; /* size 0x58 */

void stdwtitl_movie_screen_update(StdwtitlMovieScreen *obj, StdwtitlMovieScreenData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0x2800);
        gfx_module.funcs.init_display(0x140, 0x1E0, 1, 1);
        if (obj->movie != 11) {
            data->player = stdwtitl_create_movie(stdwtitl_movies[obj->movie].file,
                                                 stdwtitl_movies[obj->movie].last_frame);
        } else {
            switch (records_language) {
            case 0:
                data->player = stdwtitl_create_movie(stdwtitl_movies[11].file, stdwtitl_movies[11].last_frame);
                break;
            case 1:
                data->player = stdwtitl_create_movie(stdwtitl_movies[12].file, stdwtitl_movies[12].last_frame);
                break;
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                data->player = stdwtitl_create_movie(stdwtitl_movies[13].file, stdwtitl_movies[13].last_frame);
                break;
            }
        }
        if (obj->movie != 11) {
            obj->next_map = stdwtitl_movies[obj->movie].next_map;
            if (obj->movie == 2 && gamestate_data.progress == 0x2D) {
                obj->next_map = 0x276;
            }
        } else {
            switch (records_language) {
            case 0:
                obj->next_map = stdwtitl_movies[11].next_map;
                break;
            case 1:
                obj->next_map = stdwtitl_movies[12].next_map;
                break;
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                obj->next_map = stdwtitl_movies[13].next_map;
                break;
            }
        }
        sound_module.stop_all();
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (data->player == NULL) {
            message_module.load_font();
            gamestate_data.funcs.set_next_map(obj->next_map, 0);
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *stdwtitl_movie_screen_create(s32 movie) {
    StdwtitlMovieScreen *obj = object_new(stdwtitl_movie_screen_update, sizeof(StdwtitlMovieScreen), sizeof(StdwtitlMovieScreenData));

    obj->movie = movie;
    return &obj->base;
}

RECT stdwtitl_screen_rect = { 0, 0, 320, 240 };

s16 stdwtitl_glow_frames[2][13] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, -1 },
    { 0, 1, 2, 3, 4, 5, 6, 7, -1, 0, 0, 0, 0 },
};

s32 stdwtitl_frame_width = 0;
s32 stdwtitl_frame_height = 0;

RECT stdwtitl_vram_rect = { 0, 0, 1024, 512 };

StdwtitlMovie stdwtitl_movies[14] = {
    { 0x94C, 0x6F0, 0xE00 },
    { 0x7F4, 0x6F0, 0xE00 },
    { 0x7EA, 0x1D1, 0x207 },
    { 0x7F2, 0x166, 0x26D },
    { 0x7EB, 0x3C0, 0x216 },
    { 0x7EC, 0x231, 0x272 },
    { 0x7ED, 0x294, 0x288 },
    { 0x7EE, 0xE0, 0x288 },
    { 0x7EF, 0x151, 0x2DE },
    { 0x7F0, 0x1B2, 0x600 },
    { 0x7F3, 0x28F, 0xE0C },
    { 0x8A5, 0x671, 0x2D7 },
    { 0x817, 0x671, 0x2D7 },
    { 0x816, 0x671, 0x2D7 },
};
