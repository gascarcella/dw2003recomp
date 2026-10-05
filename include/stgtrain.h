#ifndef STGTRAIN_H
#define STGTRAIN_H

/* STGTRAIN.PRO (training): types, module structs and functions shared by its files. */

#include "common.h"
#include "object.h"
#include "heap.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "overlay_common.h"

/* A sprite-animation file of the trainer menu (stgtrain_anim_files, 25 of them). */
typedef struct StgtrainAnimFile {
    /* 0x0 */ s32 file; /* file ID */
    /* 0x4 */ s32 x;
    /* 0x8 */ s32 y;
    /* 0xC */ s32 unk_C;
} StgtrainAnimFile; /* size 0x10 */

/* A part of the loaded animation file (stgtrain_anim_parse): pointers into the file. */
typedef struct StgtrainAnimPart {
    /* 0x00 */ s32 id; /* the part's index, set by stgtrain_anim_parse */
    /* 0x04 */ u8 *sprite_data;
    /* 0x08 */ s32 sprite_offset;
    /* 0x0C */ s32 unk_0C;
    /* 0x10 */ u8 *anims[6];
    /* 0x28 */ u32 *images[4]; /* TIM images, "RLEN" compressed or not */
    /* 0x38 */ s32 count;      /* images */
    /* 0x3C */ s32 x[4];       /* VRAM position of each image */
    /* 0x4C */ s32 y;
} StgtrainAnimPart; /* size 0x50 */


/* An entry of a training menu (stgtrain_get_menu: 16 per menu). */
typedef struct StgtrainMenuEntry {
    /* 0x0 */ s32 id; /* 0: none */
    /* 0x4 */ s16 stat;
    /* 0x6 */ s16 stat_2;
} StgtrainMenuEntry; /* size 0x8 */

/* A training (stgtrain_trainings, 25 of them; StgtrainMain.training indexes it). */
typedef struct StgtrainTraining {
    /* 0x00 */ s32 name;     /* text IDs */
    /* 0x04 */ s32 desc;
    /* 0x08 */ s32 icons[4]; /* the icon's animation frames */
} StgtrainTraining; /* size 0x18 */

/* The overlay's helpers (the end of stgtrain_80088100.c), called through this table (stgtrain_module.funcs). */
typedef struct StgtrainFuncs {
    /* 0x00 */ StgtrainTraining *trainings; /* stgtrain_trainings */
    /* 0x04 */ void (*load)(void);                                              /* stgtrain_load_image */
    /* 0x08 */ void (*anim_start)(WindowAnim *anim, s32 open);          /* stgtrain_window_anim_start */
    /* 0x0C */ s32 (*anim_update)(WindowAnim *anim);                    /* stgtrain_window_anim_update */
    /* 0x10 */ void (*tween_start)(Tween *obj, s32 from, s32 to, s32 frames); /* stgtrain_tween_start */
    /* 0x14 */ s32 (*tween_update)(Tween *obj);                         /* stgtrain_tween_update */
    /* 0x18 */ s32 (*file_load)(s32 file);                                      /* stgtrain_anim_load */
    /* 0x1C */ u8 *(*file_get)(void);                                           /* stgtrain_anim_get_data */
    /* 0x20 */ void (*file_free)(void);                                         /* stgtrain_anim_free */
    /* 0x24 */ s32 (*parse)(s32 part);                                          /* stgtrain_anim_parse */
    /* 0x28 */ s32 (*upload)(s32 part, s32 *pos);                               /* stgtrain_anim_upload */
    /* 0x2C */ s32 (*get_file_id)(s32 file);                                    /* stgtrain_anim_get_file_id */
    /* 0x30 */ s32 (*get_file_pos)(s32 file);                                   /* stgtrain_anim_get_file_pos */
    /* 0x34 */ s32 (*get_file_unk_C)(s32 file);                                 /* stgtrain_anim_get_file_unk_C */
    /* 0x38 */ u8 *(*get_part_data)(s32 part);                                    /* stgtrain_anim_get_part_data */
    /* 0x3C */ s32 (*get_part_offset)(s32 part);                                    /* stgtrain_anim_get_part_offset */
    /* 0x40 */ s32 (*get_part_0C)(s32 part);                                    /* stgtrain_anim_get_part_0C */
    /* 0x44 */ u8 *(*get_part_anim)(s32 part, s32 i);                             /* stgtrain_anim_get_part_anim */
    /* 0x48 */ StgtrainMenuEntry *(*get_menu)(s32 menu);                        /* stgtrain_get_menu */
    /* 0x4C */ StgtrainMenuEntry *(*find_menu_entry)(s32 menu, s32 id);         /* stgtrain_find_menu_entry */
} StgtrainFuncs; /* size 0x50 */

/* stgtrain_module: the module of the overlay's helpers, its state and its function table. One object: code
 * reaches the table through the state's address (stgtrain_main_run). */
typedef struct StgtrainModule {
    /* 0x000 */ s32 count;    /* menu entries (stgtrain_get_menu) */
    /* 0x004 */ u8 *data;     /* the loaded animation file, NULL while loading */
    /* 0x008 */ s32 file;     /* index into stgtrain_anim_files, -1: none */
    /* 0x00C */ StgtrainAnimPart parts[9];
    /* 0x2DC */ StgtrainFuncs funcs;
} StgtrainModule; /* size 0x32C */

extern StgtrainModule stgtrain_module;

/* A frame of a sprite animation (StgtrainSpriteAnim). */
typedef struct StgtrainSpriteFrame {
    /* 0x0 */ s16 list;     /* index of the frame's part list in the sprite data */
    /* 0x2 */ u8 duration;  /* in frames */
    /* 0x3 */ u8 unk_3;
    /* 0x4 */ s16 x;
    /* 0x6 */ s16 y;
} StgtrainSpriteFrame; /* size 0x8 */

/* A sprite animation: this header, then `count` StgtrainSpriteFrame. */
typedef struct StgtrainSpriteAnim {
    /* 0x0 */ s16 unk_0;
    /* 0x2 */ s16 count; /* frames */
    /* 0x4 */ StgtrainSpriteFrame frames[0];
} StgtrainSpriteAnim; /* size 0x4 */

/* An animated sprite drawn from part lists (stgtrain_sprite_create, size 0x10C). */
typedef struct StgtrainSprite {
    /* 0x000 */ Object base;
    /* 0x050 */ GfxLayer *layer;
    /* 0x054 */ u32 *ot;
    /* 0x058 */ s32 layer_id;
    /* 0x05C */ s32 ot_depth;
    /* 0x060 */ s32 x;
    /* 0x064 */ s32 y;
    /* 0x068 */ s32 tex_x; /* VRAM position of the texture */
    /* 0x06C */ s32 tex_y;
    /* 0x070 */ s32 clut_x;
    /* 0x074 */ s32 clut_y;
    /* 0x078 */ s32 done; /* the animation reached its last frame; bit 31: paused */
    /* 0x07C */ s32 unk_7C;
    /* 0x080 */ u8 *data;  /* part lists */
    /* 0x084 */ u16 unk_84;
    /* 0x086 */ u16 unk_86;
    /* 0x088 */ s32 offset; /* of the part lists in data */
    /* 0x08C */ StgtrainSpriteAnim *anim;
    /* 0x090 */ s32 frame;
    /* 0x094 */ s32 frames;
    /* 0x098 */ s32 time; /* of the frame's start */
    /* 0x09C */ s32 transform; /* rotate and scale (matrix) */
    /* 0x0A0 */ s32 pivot_x;
    /* 0x0A4 */ s32 pivot_y;
    /* 0x0A8 */ VECTOR scale;
    /* 0x0B8 */ SVECTOR rotation;
    /* 0x0C0 */ MATRIX matrix;
    /* 0x0E0 */ void (*set_data)(struct StgtrainSprite *obj, u8 *data, s32 offset);
    /* 0x0E4 */ void (*set_anim)(struct StgtrainSprite *obj, StgtrainSpriteAnim *anim);
    /* 0x0E8 */ void (*set_pos)(struct StgtrainSprite *obj, s32 x, s32 y);
    /* 0x0EC */ void (*set_tex)(struct StgtrainSprite *obj, s32 x, s32 y);
    /* 0x0F0 */ void (*set_layer)(struct StgtrainSprite *obj, s32 id, s32 depth);
    /* 0x0F4 */ void (*set_clut)(struct StgtrainSprite *obj, s32 x, s32 y);
    /* 0x0F8 */ void (*set_scale)(struct StgtrainSprite *obj, s32 x, s32 y, s32 z);
    /* 0x0FC */ void (*set_pivot)(struct StgtrainSprite *obj, s32 x, s32 y);
    /* 0x100 */ void (*set_rotation)(struct StgtrainSprite *obj, s16 x, s16 y, s16 z);
    /* 0x104 */ s32 (*is_done)(struct StgtrainSprite *obj);
    /* 0x108 */ void (*pause)(struct StgtrainSprite *obj, s32 paused);
} StgtrainSprite; /* size 0x10C */

/* The training screen (stgtrain_main_create, size 0x114). */
typedef struct StgtrainMain {
    /* 0x000 */ Object base;
    /* 0x050 */ s32 layer;
    /* 0x054 */ s32 ot_depth;
    /* 0x058 */ s32 background; /* image of the background, by gym */
    /* 0x05C */ s32 scroll;     /* background scroll */
    /* 0x060 */ s32 scroll_wait; /* the scroll moves every other frame */
    /* 0x064 */ s32 members;    /* party members */
    /* 0x068 */ s32 member;     /* party member shown */
    /* 0x06C */ s32 cursor;     /* the arrows are shown */
    /* 0x070 */ s32 cursor_frame;
    /* 0x074 */ s32 cursor_time;
    /* 0x078 */ s32 training;   /* the training chosen: its animation file, row of stgtrain_module.funcs.trainings */
    /* 0x07C */ s32 level;     /* the trainer's level? (0..2, StgtrainSession's gains) */
    /* 0x080 */ struct {
        s32 frame;
        s32 time;
    } member_anims[3]; /* each member's animation */
    /* 0x098 */ s32 frame;      /* the shown member's animation */
    /* 0x09C */ s32 time;
    /* 0x0A0 */ WindowAnim anims[7];
    /* 0x110 */ void (*show_change)(struct StgtrainMain *obj, GamestateStats *old); /* stgtrain_main_show_change */
} StgtrainMain; /* size 0x114 */

/* The trainer level choice (stgtrain_level_create, size 0xFC): three levels, each with a cost. */
typedef struct StgtrainLevel {
    /* 0x00 */ Object base;
    /* 0x50 */ StgtrainMain *main;
    /* 0x54 */ s32 layer;
    /* 0x58 */ s32 ot_depth;
    /* 0x5C */ s32 level;
    /* 0x60 */ s32 cursor_frame;
    /* 0x64 */ s32 cursor_time;
    /* 0x68 */ s32 cursor; /* the cursor is shown */
    /* 0x6C */ s32 yes;    /* the confirmation: 0 yes, 1 no */
    /* 0x70 */ s32 trainer_frame;
    /* 0x74 */ s32 trainer_time;
    /* 0x78 */ WindowAnim anims[8];
    /* 0xF8 */ void (*close)(struct StgtrainLevel *obj); /* stgtrain_level_close */
} StgtrainLevel; /* size 0xFC */

/* The training choice (stgtrain_choice_create, size 0x114): the menu's trainings on two pages of 2 x 4. */
typedef struct StgtrainChoice {
    /* 0x000 */ Object base;
    /* 0x050 */ StgtrainMain *main;
    /* 0x054 */ s32 layer;
    /* 0x058 */ s32 ot_depth;
    /* 0x05C */ s32 cursor; /* the cursor is shown */
    /* 0x060 */ s32 cursor_frame;
    /* 0x064 */ s32 cursor_time;
    /* 0x068 */ s32 col;
    /* 0x06C */ s32 row;
    /* 0x070 */ s32 page;
    /* 0x074 */ s32 page_arrow; /* the page arrow is shown (6 trainings or more) */
    /* 0x078 */ s32 arrow_frame;
    /* 0x07C */ s32 arrow_time;
    /* 0x080 */ s32 icon_frame;
    /* 0x084 */ s32 icon_time;
    /* 0x088 */ s32 grid[2][8]; /* menu entry IDs; -1: locked, 0: no slot */
    /* 0x0C8 */ WindowAnim anims[4];
    /* 0x108 */ void (*open)(struct StgtrainChoice *obj);
    /* 0x10C */ void (*close)(struct StgtrainChoice *obj);
    /* 0x110 */ void (*show)(struct StgtrainChoice *obj);
} StgtrainChoice; /* size 0x114 */

/* A Digimon training (stgtrain_trainee_create, size 0xD4): its two sprites play the training's animation. */
typedef struct StgtrainTrainee {
    /* 0x00 */ Object base;
    /* 0x50 */ u32 state;   /* 1 idle, 2 scaling, 4 a round, 8 the end; odd: ready */
    /* 0x54 */ s32 part;    /* the Digimon's part of the animation file */
    /* 0x58 */ s32 file;    /* the training's animation file */
    /* 0x5C */ s32 step;
    /* 0x60 */ s32 result;  /* of the round: 1 success, 0 failure, -1 running */
    /* 0x64 */ s32 scale;
    /* 0x68 */ s32 scale_step;
    /* 0x6C */ s32 unk_6C;
    /* 0x70 */ s32 scale_set;
    /* 0x74 */ s32 layer;
    /* 0x78 */ s32 ot_depth;
    /* 0x7C */ s32 unk_7C;
    /* 0x80 */ s32 pos[4];  /* VRAM position of the images and of their CLUTs (stgtrain_anim_upload) */
    /* 0x90 */ s32 saved_x;
    /* 0x94 */ s32 tex_set;
    /* 0x98 */ s32 clut_set;
    /* 0x9C */ s32 chance;  /* of success, in % */
    /* 0xA0 */ s32 unk_A0;
    /* 0xA4 */ void (*set_tex)(struct StgtrainTrainee *obj, s32 x, s32 y);
    /* 0xA8 */ void (*set_clut)(struct StgtrainTrainee *obj, s32 x, s32 y);
    /* 0xAC */ void (*set_chance)(struct StgtrainTrainee *obj, s32 chance);
    /* 0xB0 */ void (*grow)(struct StgtrainTrainee *obj);
    /* 0xB4 */ void (*shrink)(struct StgtrainTrainee *obj);
    /* 0xB8 */ void (*start_round)(struct StgtrainTrainee *obj);
    /* 0xBC */ void (*pause)(struct StgtrainTrainee *obj);
    /* 0xC0 */ void (*set_scale)(struct StgtrainTrainee *obj, s32 scale);
    /* 0xC4 */ s32 (*get_result)(struct StgtrainTrainee *obj);
    /* 0xC8 */ void (*end)(struct StgtrainTrainee *obj);
    /* 0xCC */ s32 unk_CC;
    /* 0xD0 */ s32 unk_D0;
} StgtrainTrainee; /* size 0xD4 */

/* A training session (stgtrain_session_create, size 0x134). */
typedef struct StgtrainSession {
    /* 0x000 */ Object base;
    /* 0x050 */ StgtrainMain *main;
    /* 0x054 */ s32 layer;
    /* 0x058 */ s32 ot_depth;
    /* 0x05C */ s32 digimon;
    /* 0x060 */ s32 training; /* the menu entry */
    /* 0x064 */ s32 results[5]; /* per round: 1 success, 0 failure, -1 not played */
    /* 0x078 */ s32 gains[5];
    /* 0x08C */ s32 losses[5];
    /* 0x0A0 */ s32 map;
    /* 0x0A4 */ GamestateStats old; /* the stats before the session */
    /* 0x0D0 */ s32 choice;  /* the bonus round's question: 0 yes, 1 no */
    /* 0x0D4 */ s32 bonus_running; /* the bonus round is running */
    /* 0x0D8 */ s32 bonus;   /* the bonus round was won */
    /* 0x0DC */ s32 blink;
    /* 0x0E0 */ s32 blink_time;
    /* 0x0E4 */ s32 arrow;   /* the "next" arrow is shown */
    /* 0x0E8 */ s32 arrow_frame;
    /* 0x0EC */ s32 arrow_time;
    /* 0x0F0 */ s16 sound;
    /* 0x0F4 */ WindowAnim anims[4];
} StgtrainSession; /* size 0x134 */

/* stgtrain_80083058.c */
StgtrainSprite *stgtrain_sprite_create(void);
Fade *stgtrain_fade_create(void);

/* stgtrain_800861BC.c */
StgtrainSession *stgtrain_session_create(StgtrainMain *main, s32 digimon, s32 training);

/* stgtrain_80088100.c */
StgtrainLevel *stgtrain_level_create(StgtrainMain *main);
StgtrainChoice *stgtrain_choice_create(StgtrainMain *main);
StgtrainTrainee *stgtrain_trainee_create(s32 part, s32 file, s32 layer, s32 ot_depth);

#endif /* STGTRAIN_H */
