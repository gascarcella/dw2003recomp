#include "common.h"

#include "cdload.h"
#include "gfx.h"
#include "heap.h"
#include "object.h"
#include "psyq/libgpu.h"
#include "psyq/libgte.h"
#include "psyq/gtemac.h"
#include "fightstg.h"

/* A mesh of a model (fightstg_model_mesh_create): sub-files 0, 1, 2 and 5 of its file, the buffers
 * fightstg_model_mesh_project fills (screen coordinates, depths) and its matrix. */
typedef struct FightstgMesh {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 unclipped;
    /* 0x54 */ s32 use_color; /* non-zero: draw in colour color */
    /* 0x58 */ CVECTOR color;
    /* 0x5C */ void *file; /* the file */
    /* 0x60 */ void *vertices; /* its sub-file 0 */
    /* 0x64 */ void *normals; /* sub-file 1 */
    /* 0x68 */ void *faces; /* sub-file 2: the faces (fightstg_model_mesh_draw) */
    /* 0x6C */ void *bounds; /* sub-file 5 */
    /* 0x70 */ FightstgPos texture_pos; /* texture position in VRAM */
    /* 0x78 */ s32 *screen_xy;
    /* 0x7C */ s32 *screen_z;
    /* 0x80 */ s32 *colors;
    /* 0x84 */ MATRIX matrix;
    /* 0xA4 */ void (*queue_draw)(struct FightstgMesh *, s32, MATRIX *); /* fightstg_model_mesh_queue_draw */
    /* 0xA8 */ void (*queue_edges)(struct FightstgMesh *, s32, MATRIX *); /* fightstg_model_mesh_queue_edges */
} FightstgMesh; /* size 0xAC */

/* A model's data block (base.children). */
typedef struct FightstgModelData {
    /* 0x0 */ void *texture_anim;            /* fightstg_texture_anim_create's */
    /* 0x4 */ FightstgMesh *meshes[1]; /* mesh of each part (from 1), part_count of them */
} FightstgModelData;

/* An animation's key (sub-files of the file anim_file of a model), 0x7FFF-terminated. */
typedef struct FightstgAnimKey {
    /* 0x0 */ s16 index;
    /* 0x2 */ s16 frames; /* frames (0: hold key and stop) */
    /* 0x4 */ s16 key; /* key frame */
    /* 0x6 */ s16 blend_key; /* 0: blend from the previous key's blend_key to the next key's key */
} FightstgAnimKey; /* size 0x8 */

void fightstg_model_blend_part(FightstgModel *obj, FightstgModelPart *part);
void fightstg_model_play_anim(FightstgModel *obj, s32 anim, s32 force);
void fightstg_model_update();
void fightstg_model_pose(FightstgModel *obj, FightstgModelData *data);
void fightstg_model_step_anim(FightstgModel *obj);
FightstgMesh *fightstg_model_mesh_create(void *file, FightstgPos pos);
void fightstg_model_pose_part(FightstgModel *obj, FightstgModelPart *part, s32 frame);
void fightstg_model_save_pose(FightstgModel *obj);
s32 fightstg_model_is_anim_done(FightstgModel *obj);
void fightstg_model_set_color(FightstgModel *obj, s32 use_color, CVECTOR *color);
void fightstg_model_set_part_unclipped(FightstgModel *obj, s32 i, s32 unclipped);
void fightstg_model_mesh_draw(void *data, GfxLayer *layer, s32 arg);
void fightstg_model_mesh_draw_edges(void *data, GfxLayer *layer, s32 arg);
s32 fightstg_model_mesh_is_visible(FightstgMesh *obj, GfxLayer *layer);
void fightstg_model_mesh_project(FightstgMesh *obj, GfxLayer *layer);
void fightstg_model_mesh_light(FightstgMesh *obj);

/* Where the next primitive goes. */
typedef union FightstgPrim {
    POLY_FT3 *ft3;
    POLY_FT4 *ft4;
    POLY_GT3 *gt3;
    POLY_GT4 *gt4;
    LINE_F2 *f2;
    LINE_F4 *f4;
} FightstgPrim;

/* The drawing state of fightstg_model_mesh_draw (a local of it), which its helpers fill a primitive
 * from: one face of a model, its screen coordinates, texture and colours. */
typedef struct FightstgDrawState {
    /* 0x00 */ s32 textured; /* textured */
    /* 0x04 */ s32 unk_04;
    /* 0x08 */ s32 unk_08;
    /* 0x0C */ s32 quad;   /* quad (else triangle) */
    /* 0x10 */ s32 gouraud; /* gouraud */
    /* 0x14 */ s32 unk_14;
    /* 0x18 */ s32 semi_trans; /* semi-transparent (blend mode + 1) */
    /* 0x1C */ u8 *stream; /* command stream */
    /* 0x20 */ s32 *screen_xy; /* screen coordinates of the vertices */
    /* 0x24 */ s32 *screen_z;
    /* 0x28 */ u32 *ot;     /* ordering table */
    /* 0x2C */ s32 *vertex_colors;
    /* 0x30 */ s32 texture_x;
    /* 0x34 */ s32 texture_y;
    /* 0x38 */ s32 u_offset;
    /* 0x3C */ s32 v_offset;
    /* 0x40 */ u16 tpage;  /* tpage */
    /* 0x42 */ u16 clut;   /* clut */
    /* 0x44 */ CVECTOR face_colors[4];
    /* 0x54 */ FightstgPrim prim;   /* next primitive */
    /* 0x58 */ s32 corners_xy[4]; /* x, y of each corner */
    /* 0x68 */ u32 *ot_entry; /* ordering table entry */
    /* 0x6C */ u16 corners_uv[4]; /* u, v of each corner */
    /* 0x74 */ CVECTOR corners_color[4]; /* colour of each corner */
} FightstgDrawState;

/* Poses `part` between its saved transform (saved_trans..) and key frame `key` of its animation, by the
 * blend factor `blend` (a scale with a zero component is not blended). */
void fightstg_model_blend_part(FightstgModel *obj, FightstgModelPart *part) {
    SVECTOR to;
    SVECTOR from;
    SVECTOR diff;
    SVECTOR step;
    void *file = cdload_module.get_subfile_by_id(part->keys_file);
    SVECTOR *key;
    SVECTOR *dst;
    s32 i;

    for (i = 0; i < 9; i += 3) {
        key = cdload_module.get_subfile(i / 3, file);
        switch (i) {
        case 0:
        default:
            dst = &part->trans;
            break;
        case 3:
            dst = &part->rot;
            break;
        case 6:
            dst = &part->scale;
            break;
        }
        for (; key->pad < obj->key; key++) {
            /* the first key at or after the frame */
        }
        to = *key;
        switch (i) {
        case 0:
        default:
            from = part->saved_trans;
            break;
        case 3:
            from = part->saved_rot;
            break;
        case 6:
            from = part->saved_scale;
            break;
        }
        if (i == 6 && (from.vx == 0 || from.vy == 0 || from.vz == 0 || to.vx == 0 || to.vy == 0 || to.vz == 0)) {
            *dst = to;
            return;
        }
        gte_lddp(obj->blend);
        diff.vx = to.vx - from.vx;
        diff.vy = to.vy - from.vy;
        diff.vz = to.vz - from.vz;
        gte_ldsv(&diff);
        gte_gpf12();
        *dst = from;
        gte_stsv(&step);
        dst->vx += step.vx;
        dst->vy += step.vy;
        dst->vz += step.vz;
    }
}

/* Poses `part` at key frame `frame` of its animation (sub-file keys_file: translation, rotation and
 * scale keys, each an SVECTOR whose pad is its frame): a key's value, or the interpolation between
 * the keys around the frame. */
void fightstg_model_pose_part(FightstgModel *obj, FightstgModelPart *part, s32 frame) {
    SVECTOR next;
    SVECTOR prev;
    SVECTOR diff;
    SVECTOR step;
    void *file = cdload_module.get_subfile_by_id(part->keys_file);
    SVECTOR *key;
    SVECTOR *dst;
    s32 found;
    s32 n;
    s32 at;
    s32 i;

    for (i = 0; i < 9; i += 3) {
        key = cdload_module.get_subfile(i / 3, file);
        found = 0;
        switch (i) {
        case 0:
        default:
            dst = &part->trans;
            break;
        case 3:
            dst = &part->rot;
            break;
        case 6:
            dst = &part->scale;
            break;
        }
        /* Find the key at `frame`, or the first after it (a goto: the break skips found = 1). */
        for (; (at = key->pad) != frame; key++) {
            if (at >= frame) {
                goto interpolate;
            }
        }
        found = 1;
    interpolate:
        if (found) {
            *dst = *key;
        } else {
            next = key[0];
            prev = key[-1];
            n = next.pad - prev.pad;
            gte_lddp(((frame - prev.pad) << 12) / n);
            diff.vx = next.vx - prev.vx;
            diff.vy = next.vy - prev.vy;
            diff.vz = next.vz - prev.vz;
            gte_ldsv(&diff);
            gte_gpf12();
            *dst = prev;
            gte_stsv(&step);
            dst->vx += step.vx;
            dst->vy += step.vy;
            dst->vz += step.vz;
        }
    }
}

/* Applies `move` (a move in the root part's axes) to the root, then poses every part: blended
 * (fightstg_model_blend_part) or at key frame `key` (fightstg_model_pose_part). */
void fightstg_model_pose(FightstgModel *obj, FightstgModelData *data) {
    FightstgModelPart *part = obj->parts;
    VECTOR move;
    s32 i;

    if (*(s32 *)&obj->move != 0 || obj->move.vz != 0) {
        gte_SetRotMatrix(&part->local);
        gte_ldv0(&obj->move);
        gte_rtv0();
        gte_stlvnl(&move);
        obj->parts->trans.vx += move.vx;
        obj->parts->trans.vy += move.vy;
        obj->parts->trans.vz += move.vz;
        obj->move.vx = obj->move.vy = obj->move.vz = 0;
    }
    part = obj->parts;
    if (obj->blending == 0) {
        for (i = 1, part++; i < obj->part_count; i++, part++) {
            fightstg_model_pose_part(obj, part, obj->key);
        }
    } else {
        for (i = 1, part++; i < obj->part_count; i++, part++) {
            fightstg_model_blend_part(obj, part);
        }
    }
}

/* Saves every part's transform. */
void fightstg_model_save_pose(FightstgModel *obj) {
    FightstgModelPart *part = obj->parts;
    s32 i;

    for (i = 0; i < obj->part_count; i++, part++) {
        part->saved_trans = part->trans;
        part->saved_rot = part->rot;
        part->saved_scale = part->scale;
    }
}

/* Starts animation `anim` (unless it is the current one and !force): expands its keys into per-frame
 * entries. */
void fightstg_model_play_anim(FightstgModel *obj, s32 anim, s32 force) {
    FightstgAnimKey *key;
    s32 n;
    s32 at;
    s32 i;
    s32 count;
    s32 to;
    s32 from;

    if (force == 1 || obj->anim != anim) {
        obj->anim = anim;
        obj->frame = 1;
        obj->anim_done = 0;
        obj->to_idle = 0;
        key = cdload_module.get_subfile(anim - 1, cdload_module.get_subfile_by_id(obj->anim_file));
        n = 0;
        while (key->index != 0x7FFF) {
            count = key->frames;
            if (count == 0) {
                obj->frames[n] = key->key;
                obj->frames_to[n] = key->blend_key;
                n++;
                break;
            }
            if (key->blend_key == 0) {
                to = key[1].key;
                if (to == -1) {
                    to = obj->idle_keys[obj->params->idle_anim];
                    obj->to_idle = 1;
                }
                from = 0xFFFF;
                if (key->index != 0) {
                    from = key[-1].blend_key;
                }
                for (i = 0; i < count; i++) {
                    obj->frames_from[n] = from;
                    obj->frames_to[n] = to;
                    obj->frames[n] = ((i + 1) << 12) / (count + 1) | 0x8000;
                    n++;
                }
            } else {
                for (i = 0; i < count; i++) {
                    obj->frames[n] = key->key + i;
                    obj->frames_to[n] = 0;
                    n++;
                }
            }
            key++;
        }
        obj->frame_count = n;
        obj->frame = 1;
    }
}

/* Advances the animation. */
void fightstg_model_step_anim(FightstgModel *obj) {
    s32 i;
    FightstgModelPart *part;
    s32 cur = obj->frame;

    if (obj->anim_done == 0) {
        if (obj->frames[cur] & 0x8000) {
            if (obj->blend_to != obj->frames_to[cur]) {
                obj->blend_to = obj->frames_to[cur];
                if (obj->frames_from[cur] != 0xFFFF) {
                    part = obj->parts;
                    for (i = 1, part++; i < obj->part_count; i++, part++) {
                        fightstg_model_pose_part(obj, part, obj->frames_from[cur]);
                    }
                }
                fightstg_model_save_pose(obj);
            }
            obj->key = obj->frames_to[cur];
            obj->blend = obj->frames[cur] & 0x7FFF;
            obj->blending = 1;
        } else {
            obj->blend_to = 0;
            obj->key = obj->frames[cur];
            obj->blending = 0;
        }
        obj->frame += fightstg_battle.frames;
        if (obj->frame >= obj->frame_count) {
            obj->frame = obj->frame_count - 1;
        }
        cur = obj->frame;
        switch (obj->frames[cur]) {
        case 0x8000:
            obj->frame = obj->frames_to[cur];
            break;
        case 0xFFFF:
            obj->anim_done = 1;
            obj->params->anim_done = 1;
            break;
        }
    } else {
        obj->blend_to = 0;
        obj->blending = 0;
        if (obj->has_idle != 0 && obj->to_idle != 0) {
            obj->params->anim = obj->params->idle_anim + 1;
            fightstg_model_play_anim(obj, obj->params->idle_anim + 1, 0);
        }
    }
}

/* The model object: loads its texture, creates its meshes and starts animation idle_anim + 1 of its
 * parameters (state 0); then each frame follows the parameters' animation, poses the parts,
 * computes their matrices and draws the meshes on the parameters' layers (1); frees its parts (3). */
void fightstg_model_update(FightstgModel *obj, FightstgModelData *data) {
    Tim tim;
    VECTOR scale;
    FightstgModelPart *part;
    FightstgModelPart *child;
    FightstgModelPart *parts;
    void *file;
    s32 k;
    s32 n;
    s32 m;
    s32 i;
    s32 j;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->texture_file != 0) {
            tim_init(&tim);
            tim.set_image_pos(obj->texture_pos.x, obj->texture_pos.y);
            tim.load_all(cdload_module.get_subfile_by_id(obj->texture_file));
        }
        child = obj->parts;
        for (k = 0; k < obj->part_count; k++, child++) {
            if (k != 0) {
                child->parent_world = &obj->parts[child->parent].world;
            }
        }
        for (i = 0; i < obj->part_count; i++) {
            if (i != 0) {
                data->meshes[i] = fightstg_model_mesh_create(cdload_module.get_subfile_by_id(obj->parts[i].mesh_file), obj->texture_pos);
            }
        }
        data->texture_anim = fightstg_texture_anim_create(obj, obj->params->model_id);
        obj->anim = 1;
        if (obj->has_idle != 0) {
            file = cdload_module.get_subfile_by_id(obj->anim_file);
            obj->idle_keys[0] = ((FightstgAnimKey *)cdload_module.get_subfile(0, file))->key;
            obj->idle_keys[1] = ((FightstgAnimKey *)cdload_module.get_subfile(1, file))->key;
        }
        fightstg_model_play_anim(obj, obj->params->idle_anim + 1, 1);
        obj->params->anim = obj->params->idle_anim + 1;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->params->restart != 0 || obj->anim != obj->params->anim) {
            obj->params->restart = 0;
            fightstg_model_play_anim(obj, obj->params->anim, 1);
            switch (obj->params->anim) {
            case 1:
                obj->params->idle_anim = 0;
                break;
            case 2:
                obj->params->idle_anim = 1;
                break;
            }
        }
        if (obj->part_count == 0) {
            break;
        }
        fightstg_model_step_anim(obj);
        fightstg_model_pose(obj, data);
        obj->parts->trans.vx = obj->params->pos.x;
        obj->parts->trans.vy = obj->params->pos.y;
        obj->parts->trans.vz = obj->params->pos.z;
        obj->parts->rot.vx = obj->params->rot.x;
        obj->parts->rot.vy = obj->params->rot.y;
        obj->parts->rot.vz = obj->params->rot.z;
        part = obj->parts;
        for (n = 0; n < obj->part_count; n++, part++) {
            scale.vx = part->scale.vx;
            scale.vy = part->scale.vy;
            scale.vz = part->scale.vz;
            if ((scale.vx | scale.vy | scale.vz) < 0x31) {
                part->visible = 0;
                continue;
            }
            part->visible = 1;
            RotMatrixZYX_gte(&part->rot, &part->local);
            ScaleMatrix(&part->local, &scale);
            part->local.t[0] = part->trans.vx;
            part->local.t[1] = part->trans.vy;
            part->local.t[2] = part->trans.vz;
            gte_CompMatrix(part->parent_world, &part->local, &part->world);
        }
        for (j = 0; j < 2; j++) {
            if (obj->params->layers[j].shown != 0) {
                parts = obj->parts;
                for (m = 0; m < obj->part_count; m++, parts++) {
                    if (m != 0 && parts->visible != 0) {
                        if (obj->params->layers[j].edges != 0) {
                            data->meshes[m]->queue_edges(data->meshes[m], obj->params->layers[j].layer_id, &parts->world);
                        } else {
                            data->meshes[m]->queue_draw(data->meshes[m], obj->params->layers[j].layer_id, &parts->world);
                        }
                    }
                }
            }
        }
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        if (obj->parts != NULL) {
            heap_funcs.free(obj->parts);
        }
        break;
    }
}

/* Sets every mesh's colour override (use_color, and color = *color when given). */
void fightstg_model_set_color(FightstgModel *obj, s32 use_color, CVECTOR *color) {
    FightstgModelData *data = (FightstgModelData *)obj->base.children;
    s32 j;
    s32 i;

    for (j = 0; j < 2; j++) {
        if (obj->params->layers[j].shown != 0) {
            for (i = 0; i < obj->part_count; i++) {
                if (i != 0) {
                    data->meshes[i]->use_color = use_color;
                    if (color != NULL) {
                        data->meshes[i]->color = *color;
                    }
                }
            }
        }
    }
}

void fightstg_model_set_part_unclipped(FightstgModel *obj, s32 i, s32 unclipped) {
    FightstgModelData *data = (FightstgModelData *)obj->base.children;
    s32 j;

    for (j = 0; j < 2; j++) {
        if (obj->params->layers[j].shown != 0 && i != 0) {
            data->meshes[i]->unclipped = unclipped;
        }
    }
}

s32 fightstg_model_is_anim_done(FightstgModel *obj) {
    return obj->anim_done;
}

/* Creates a model from sub-file `id`: its texture's sub-file ID, the number of parts after the root,
 * then per part its parent and the sub-file IDs of its mesh (IDs relative to id's file). */
FightstgModel *fightstg_model_new(s32 id, s32 anim_file, FightstgPos pos, FightstgModelParams *params, s32 has_idle) {
    s32 file = id & 0xFFFF0000;
    s32 *data = cdload_module.get_subfile_by_id(id);
    FightstgModel *obj;
    s32 count = data[1] + 1;
    s32 i;

    obj = object_create(fightstg_model_update, sizeof(FightstgModel), (data[1] + 2) * 4, 0x11);
    obj->parts = heap_funcs.alloc(count * sizeof(FightstgModelPart), 2);
    obj->params = params;
    obj->has_idle = has_idle;
    obj->texture_pos = pos;
    obj->texture_file = *data++;
    if (obj->texture_file != 0) {
        obj->texture_file |= file;
    }
    obj->part_count = count;
    obj->parts[0].parent = 0;
    obj->parts[0].mesh_file = 0;
    obj->parts[0].keys_file = 0;
    obj->parts[0].parent_world = &D_8004DC20;
    obj->parts[0].trans.vx = 0;
    obj->parts[0].trans.vy = 0;
    obj->parts[0].trans.vz = 0;
    obj->parts[0].rot.vx = 0;
    obj->parts[0].rot.vy = 0;
    obj->parts[0].rot.vz = 0;
    obj->parts[0].scale.vx = 0x1000;
    obj->parts[0].scale.vy = 0x1000;
    obj->parts[0].scale.vz = 0x1000;
    data++;
    for (i = 1; i < count; i++) {
        obj->parts[i].parent = *data++;
        obj->parts[i].mesh_file = *data++ | file;
        obj->parts[i].keys_file = *data++ | file;
    }
    obj->play_anim = fightstg_model_play_anim;
    obj->is_anim_done = fightstg_model_is_anim_done;
    obj->set_color = fightstg_model_set_color;
    obj->anim_file = anim_file;
    obj->set_part_unclipped = fightstg_model_set_part_unclipped;
    return obj;
}

FightstgModel *fightstg_model_create_idle(s32 id, s32 anim_file, FightstgPos pos, FightstgModelParams *params) {
    return fightstg_model_new(id, anim_file, pos, params, 1);
}

FightstgModel *fightstg_model_create(s32 id, s32 anim_file, FightstgPos pos, FightstgModelParams *params) {
    return fightstg_model_new(id, anim_file, pos, params, 0);
}

/* Lights the mesh: the colour of each normal of sub-file 1 (a count, then the normals) with the
 * mesh's matrix in world-screen space as the light matrix, into `colors`. */
void fightstg_model_mesh_light(FightstgMesh *obj) {
    MATRIX m;
    FightstgVec *v = obj->normals;
    s32 n = v->x;
    s32 *c;
    s32 i;

    v++;
    if (n == 0) {
        return;
    }
    gte_CompMatrix(&D_800812F8, &obj->matrix, &m);
    gte_SetLightMatrix(&m);
    if (obj->colors == NULL) {
        obj->colors = heap_funcs.alloc(n * 4, 2);
    }
    c = obj->colors;
    gte_ldv0_u(v);
    gte_ncs();
    gte_strgb(c);
    v++;
    i = 1;
    while (i < n) {
        gte_ldv0_u(v);
        gte_ncs();
        v++;
        c++;
        i++;
        gte_strgb(c);
    }
}

/* Projects the mesh's vertices (sub-file 0: a count, then the vertices): screen coordinates into
 * screen_xy, depths (OTZ >> (14 - the layer's ordering table depth)) into screen_z. */
void fightstg_model_mesh_project(FightstgMesh *obj, GfxLayer *layer) {
    s32 otz;
    FightstgVec *v = obj->vertices;
    s32 shift = 14 - layer->get_ot_bits(layer);
    s32 n = v->x;
    s32 *xy;
    s32 *z;
    s32 i;

    v++;
    if (obj->screen_xy == NULL) {
        obj->screen_xy = heap_funcs.alloc(n * 4, 2);
    }
    xy = obj->screen_xy;
    if (obj->screen_z == NULL) {
        obj->screen_z = heap_funcs.alloc(n * 4, 2);
    }
    z = obj->screen_z;
    gte_ldv0_u(v);
    gte_rtps();
    gte_stsxy(xy);
    gte_stszotz(&otz);
    v++;
    i = 1;
    while (i < n) {
        gte_ldv0_u(v);
        gte_rtps();
        v++;
        xy++;
        i++;
        *z++ = otz >> shift;
        gte_stsxy(xy);
        gte_stszotz(&otz);
    }
    *z = otz >> shift;
}

/* Adds a gouraud textured triangle or quad (POLY_GT3/POLY_GT4). */
void fightstg_model_add_poly_gt(FightstgDrawState *ctx) {
    *(CVECTOR *)&ctx->prim.gt4->r0 = ctx->corners_color[0];
    *(CVECTOR *)&ctx->prim.gt4->r1 = ctx->corners_color[1];
    *(CVECTOR *)&ctx->prim.gt4->r2 = ctx->corners_color[2];
    if (ctx->quad != 0) {
        *(CVECTOR *)&ctx->prim.gt4->r3 = ctx->corners_color[3];
    }
    *(s32 *)&ctx->prim.gt4->x0 = ctx->corners_xy[0];
    *(s32 *)&ctx->prim.gt4->x1 = ctx->corners_xy[1];
    *(s32 *)&ctx->prim.gt4->x2 = ctx->corners_xy[2];
    *(u16 *)&ctx->prim.gt4->u0 = ctx->corners_uv[0];
    *(u16 *)&ctx->prim.gt4->u1 = ctx->corners_uv[1];
    *(u16 *)&ctx->prim.gt4->u2 = ctx->corners_uv[2];
    ctx->prim.gt4->clut = ctx->clut;
    ctx->prim.gt4->tpage = ctx->tpage;
    if (ctx->quad != 0) {
        setPolyGT4(ctx->prim.gt4);
        if (ctx->semi_trans != 0) {
            setSemiTrans(ctx->prim.gt4, 1);
        }
        *(s32 *)&ctx->prim.gt4->x3 = ctx->corners_xy[3];
        *(u16 *)&ctx->prim.gt4->u3 = ctx->corners_uv[3];
        addPrim(ctx->ot_entry, ctx->prim.gt4);
        ctx->prim.gt4++;
    } else {
        setPolyGT3(ctx->prim.gt3);
        if (ctx->semi_trans != 0) {
            setSemiTrans(ctx->prim.gt3, 1);
        }
        addPrim(ctx->ot_entry, ctx->prim.gt3);
        ctx->prim.gt3++;
    }
}

/* Adds a flat textured triangle or quad (POLY_FT3/POLY_FT4). */
void fightstg_model_add_poly_ft(FightstgDrawState *ctx) {
    *(CVECTOR *)&ctx->prim.ft4->r0 = ctx->corners_color[0];
    *(s32 *)&ctx->prim.ft4->x0 = ctx->corners_xy[0];
    *(s32 *)&ctx->prim.ft4->x1 = ctx->corners_xy[1];
    *(s32 *)&ctx->prim.ft4->x2 = ctx->corners_xy[2];
    *(u16 *)&ctx->prim.ft4->u0 = ctx->corners_uv[0];
    *(u16 *)&ctx->prim.ft4->u1 = ctx->corners_uv[1];
    *(u16 *)&ctx->prim.ft4->u2 = ctx->corners_uv[2];
    ctx->prim.ft4->clut = ctx->clut;
    ctx->prim.ft4->tpage = ctx->tpage;
    if (ctx->quad != 0) {
        setPolyFT4(ctx->prim.ft4);
        if (ctx->semi_trans != 0) {
            setSemiTrans(ctx->prim.ft4, 1);
        }
        *(s32 *)&ctx->prim.ft4->x3 = ctx->corners_xy[3];
        *(u16 *)&ctx->prim.ft4->u3 = ctx->corners_uv[3];
        addPrim(ctx->ot_entry, ctx->prim.ft4);
        ctx->prim.ft4++;
    } else {
        setPolyFT3(ctx->prim.ft3);
        if (ctx->semi_trans != 0) {
            setSemiTrans(ctx->prim.ft3, 1);
        }
        addPrim(ctx->ot_entry, ctx->prim.ft3);
        ctx->prim.ft3++;
    }
}

/* Whether the mesh may be on screen: unclipped meshes count as visible; else whether one of
 * the 9 points of sub-file 5 projects into the layer's clip area (with a 0x40 margin). */
s32 fightstg_model_mesh_is_visible(FightstgMesh *obj, GfxLayer *layer) {
    u32 sxy[2];
    s32 flag;
    u8 *v;
    s32 xmin, xmax, ymin, ymax;
    s32 x, y;
    s32 i;

    if (obj->unclipped != 0) {
        return 1;
    }
    v = obj->bounds;
    ymax = ymin = xmax = xmin = 0;
    for (i = 0; i < 9;) {
        gte_ldv0_u(v);
        gte_rtps();
        if (i == 0) {
            /* The clip area with the margin, then in the layer's offset coordinates (own block-local values:
             * written inline, fold reassociates the constants; reusing x/y changes the allocation). */
            s32 left, width, top, height;

            left = layer->env.clip.x - 0x40;
            width = layer->env.clip.w + 0x80;
            xmin = left - layer->draw_x;
            xmax = left + width - layer->draw_x;
            top = layer->env.clip.y - 0x40;
            height = layer->env.clip.h + 0x80;
            ymin = top - layer->draw_y;
            ymax = top + height - layer->draw_y;
        }
        i++;
        v += 6;
        gte_stsxy(sxy);
        gte_stflg(&flag);
        x = (s16)sxy[0];
        if (x >= xmin && x <= xmax) {
            y = (s16)(sxy[0] >> 16);
            if (y >= ymin && y <= ymax) {
                return 1;
            }
        }
    }
    return 0;
}

/* Draws the mesh on `layer` (a layer callback): its screen matrix, then the command stream of
 * sub-file 2: 0x8n-0xEn set the drawing state, 1 a texture page, 2-5 a colour, 0 a run of faces
 * (vertex indices, then colours and UVs as the state says); 0xFF ends. */
#ifdef NON_MATCHING
/* 99.2% (last-fight; was 97.3%): register allocation of the face's vertex values and depths and of the clut's two halves.
 * wip-14: the texture command through getClut/getTPage (97.3). wip-10: the original loads cmd[6] (tpage) before storing
 * unk_42 (clut), and keeps the two UV sums' adds per branch; argument orders, per-branch `p = ...`, a ternary: no better.
 * final-fight: draw_edges' if/else skeleton instead of `continue`, the fields instead of x/y locals, block-local x/y,
 * the clut written out: no better (the original loads x into a1 and y into a2, here y first).
 * last-fight: what moved it to 99.2: cmd[6] read into tp before the clut store (the original's load order; it fixes
 * the command pointer's a3 and x/y), cx/cy locals for the clut's coordinates, an own k for the UV block (n was one
 * pseudo with the step at the end of the face loop), and in the UV block uv = k + 3 computed before the branch with
 * p = q + (k + (k + 6)) / q + uv from a once-assigned q (the original's per-branch adds and load order; these last
 * temporaries are permuter-style shapes, kept only in this WIP). Left: the vertex base (here v1, original a2), the
 * depth base/z1 (a1/a0 swapped), the clut halves (v0/v1 swapped, store one slot earlier). xy0..2 locals, a pointer
 * for unk_20/unk_24, depth load orders, clut forms (u16 local, shifts first): no better; the permuter (30 min, gte
 * macros preserved): only dummy copies (9055 -> 8755). */
void fightstg_model_mesh_draw(void *data, GfxLayer *layer, s32 arg) {
    FightstgMesh *obj = data;
    MATRIX m;
    FightstgDrawState ctx;
    s32 opz;
    s32 otz;
    u8 *cmd;
    u8 *p;
    s32 hi;
    s32 lo;
    s32 i0, i1, i2, i3;
    s32 xy0, xy1, xy2;
    s32 z1, z2;
    s32 n;
    s32 k;
    s32 uv;
    u8 *q;
    s32 x, y;
    s32 tp;
    s32 cx, cy;

    gte_CompMatrix(&D_80081358, &obj->matrix, &m);
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    if (fightstg_model_mesh_is_visible(obj, layer) == 0) {
        return;
    }
    fightstg_model_mesh_project(obj, layer);
    fightstg_model_mesh_light(obj);
    ctx.stream = obj->faces;
    *(FightstgPos *)&ctx.texture_x = obj->texture_pos;
    ctx.screen_xy = obj->screen_xy;
    ctx.screen_z = obj->screen_z;
    ctx.vertex_colors = obj->colors;
    ctx.ot = layer->get_ot(layer);
    ctx.prim.ft3 = gfx_module.funcs.get_packet();
    while (*ctx.stream != 0xFF) {
        hi = *ctx.stream >> 4;
        lo = *ctx.stream & 0xF;
        if (hi != 0) {
            switch (hi) {
            case 8:
                ctx.quad = lo;
                break;
            case 9:
                ctx.textured = lo;
                break;
            case 10:
                ctx.unk_08 = lo;
                break;
            case 11:
                ctx.unk_04 = lo;
                break;
            case 12:
                ctx.gouraud = lo;
                break;
            case 13:
                ctx.unk_14 = lo;
                break;
            case 14:
                ctx.semi_trans = lo;
                break;
            }
            ctx.stream++;
            continue;
        }
        switch (lo) {
        case 1:
            x = ctx.texture_x;
            y = ctx.texture_y;
            ctx.u_offset = ctx.stream[1] + (ctx.stream[2] << 8);
            ctx.v_offset = ctx.stream[3];
            tp = ctx.stream[6];
            cy = ctx.stream[5] + y;
            cx = ctx.stream[4] + x;
            ctx.clut = getClut(cx, cy);
            ctx.tpage = getTPage(tp, ctx.semi_trans != 0 ? ctx.semi_trans - 1 : 0, x + (ctx.stream[2] << 6), y);
            ctx.stream += 7;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            if (obj->use_color != 0) {
                ctx.face_colors[lo - 2].r = obj->color.r;
                ctx.face_colors[lo - 2].g = obj->color.g;
                ctx.face_colors[lo - 2].b = obj->color.b;
            } else {
                ctx.face_colors[lo - 2].r = ctx.stream[1];
                ctx.face_colors[lo - 2].g = ctx.stream[2];
                ctx.face_colors[lo - 2].b = ctx.stream[3];
            }
            ctx.stream += 4;
            break;
        case 0:
            do {
                ctx.stream++;
                i0 = ctx.stream[0];
                i1 = ctx.stream[1];
                i2 = ctx.stream[2];
                i3 = 0;
                if (ctx.quad != 0) {
                    i3 = ctx.stream[3];
                }
                ctx.corners_xy[0] = ctx.screen_xy[i0];
                ctx.corners_xy[1] = ctx.screen_xy[i1];
                ctx.corners_xy[2] = ctx.screen_xy[i2];
                if (ctx.quad != 0) {
                    ctx.corners_xy[3] = ctx.screen_xy[i3];
                }
                gte_ldsxy3(ctx.corners_xy[0], ctx.corners_xy[1], ctx.corners_xy[2]);
                gte_nclip();
                if (ctx.corners_xy[0] != ctx.corners_xy[1] && ctx.corners_xy[0] != ctx.corners_xy[2] && ctx.corners_xy[1] != ctx.corners_xy[2] &&
                    (ctx.quad == 0 || (ctx.corners_xy[0] != ctx.corners_xy[3] && ctx.corners_xy[1] != ctx.corners_xy[3] &&
                                         ctx.corners_xy[2] != ctx.corners_xy[3]))) {
                    gte_stopz(&opz);
                    if (opz > 0) {
                        if (ctx.gouraud != 0) {
                            p = ctx.stream + (ctx.quad + 3);
                            ctx.corners_color[0] = ((CVECTOR *)ctx.vertex_colors)[p[0]];
                            ctx.corners_color[1] = ((CVECTOR *)ctx.vertex_colors)[p[1]];
                            ctx.corners_color[2] = ((CVECTOR *)ctx.vertex_colors)[p[2]];
                            if (ctx.quad != 0) {
                                ctx.corners_color[3] = ((CVECTOR *)ctx.vertex_colors)[p[3]];
                            }
                        }
                        if (ctx.textured != 0) {
                            k = ctx.quad;
                            uv = k + 3;
                            q = ctx.stream;
                            if (ctx.gouraud != 0) {
                                p = q + (k + (k + 6));
                            } else {
                                p = q + uv;
                            }
                            ((u8 *)ctx.corners_uv)[0] = p[0] + (u8)ctx.u_offset;
                            ((u8 *)ctx.corners_uv)[1] = p[1] + (u8)ctx.v_offset;
                            ((u8 *)ctx.corners_uv)[2] = p[2] + (u8)ctx.u_offset;
                            ((u8 *)ctx.corners_uv)[3] = p[3] + (u8)ctx.v_offset;
                            ((u8 *)ctx.corners_uv)[4] = p[4] + (u8)ctx.u_offset;
                            ((u8 *)ctx.corners_uv)[5] = p[5] + (u8)ctx.v_offset;
                            if (ctx.quad != 0) {
                                ((u8 *)ctx.corners_uv)[6] = p[6] + (u8)ctx.u_offset;
                                ((u8 *)ctx.corners_uv)[7] = p[7] + (u8)ctx.v_offset;
                            }
                        }
                        otz = ctx.screen_z[i0];
                        z1 = ctx.screen_z[i1];
                        z2 = ctx.screen_z[i2];
                        if (ctx.quad != 0) {
                            gte_ldsz4(otz, z1, z2, ctx.screen_z[i3]);
                            gte_avsz4();
                            gte_stotz(&otz);
                        } else {
                            gte_ldsz3(otz, z1, z2);
                            gte_avsz3();
                            gte_stotz(&otz);
                        }
                        ctx.ot_entry = ctx.ot + otz;
                        if (ctx.unk_14 != 0) {
                            fightstg_model_add_poly_gt(&ctx);
                        } else {
                            if (ctx.gouraud == 0) {
                                ctx.corners_color[0] = ctx.face_colors[0];
                            }
                            fightstg_model_add_poly_ft(&ctx);
                        }
                    }
                }
                n = ctx.quad + 3;
                ctx.stream += n;
                if (ctx.gouraud != 0) {
                    ctx.stream += n;
                }
                if (ctx.textured != 0) {
                    ctx.stream += n * 2;
                }
            } while (*ctx.stream == 0);
            break;
        }
    }
    gfx_module.funcs.set_packet(ctx.prim.ft3);
}
#else
INCLUDE_ASM("asm/fightstg/nonmatchings/fightstg_model", fightstg_model_mesh_draw);
#endif

/* Draws the mesh's edges (green lines) on `layer`: fightstg_model_mesh_draw's command stream, faces
 * only. */
void fightstg_model_mesh_draw_edges(void *data, GfxLayer *layer, s32 arg) {
    FightstgMesh *obj = data;
    MATRIX m;
    FightstgDrawState ctx;
    s32 hi;
    s32 lo;
    s32 i0, i1, i2, i3;
    s32 n;

    gte_CompMatrix(&D_80081358, &obj->matrix, &m);
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    if (fightstg_model_mesh_is_visible(obj, layer) == 0) {
        return;
    }
    fightstg_model_mesh_project(obj, layer);
    ctx.stream = obj->faces;
    *(FightstgPos *)&ctx.texture_x = obj->texture_pos;
    ctx.screen_xy = obj->screen_xy;
    ctx.screen_z = obj->screen_z;
    ctx.vertex_colors = obj->colors;
    ctx.ot_entry = ctx.ot = layer->get_ot(layer);
    ctx.prim.ft3 = gfx_module.funcs.get_packet();
    while (*ctx.stream != 0xFF) {
        hi = *ctx.stream >> 4;
        lo = *ctx.stream & 0xF;
        if (hi != 0) {
            switch (hi) {
            case 8:
                ctx.quad = lo;
                break;
            case 9:
                ctx.textured = lo;
                break;
            case 12:
                ctx.gouraud = lo;
                break;
            }
            ctx.stream++;
        } else {
            switch (lo) {
            case 1:
                ctx.stream += 7;
                break;
            case 2:
            case 3:
            case 4:
            case 5:
                ctx.stream += 4;
                break;
            case 0:
                do {
                    ctx.stream++;
                    i0 = ctx.stream[0];
                    i1 = ctx.stream[1];
                    i2 = ctx.stream[2];
                    i3 = 0;
                    if (ctx.quad != 0) {
                        i3 = ctx.stream[3];
                    }
                    ctx.corners_xy[0] = ctx.screen_xy[i0];
                    ctx.corners_xy[1] = ctx.screen_xy[i1];
                    ctx.corners_xy[2] = ctx.screen_xy[i2];
                    if (ctx.quad != 0) {
                        ctx.corners_xy[3] = ctx.screen_xy[i3];
                    }
                    setLineF4(ctx.prim.f4);
                    setRGB0(ctx.prim.f4, 0, 0xFF, 0);
                    *(s32 *)&ctx.prim.f4->x0 = ctx.corners_xy[0];
                    *(s32 *)&ctx.prim.f4->x1 = ctx.corners_xy[1];
                    if (ctx.quad == 0) {
                        *(s32 *)&ctx.prim.f4->x2 = ctx.corners_xy[2];
                        *(s32 *)&ctx.prim.f4->x3 = ctx.corners_xy[0];
                    } else {
                        *(s32 *)&ctx.prim.f4->x2 = ctx.corners_xy[3];
                        *(s32 *)&ctx.prim.f4->x3 = ctx.corners_xy[2];
                    }
                    /* Evidence (class A1, sched1 barrier; DECISIONS "LOOP_BLOCK audit"): the original's addPrim
                     * registers are what sched1 gives when it can't move code across either addPrim (without sched1
                     * both forms are the same). */
                    LOOP_BLOCK(addPrim(ctx.ot_entry, ctx.prim.f4););
                    ctx.prim.f4++;
                    if (ctx.quad != 0) {
                        setLineF2(ctx.prim.f2);
                        setRGB0(ctx.prim.f2, 0, 0xFF, 0);
                        *(s32 *)&ctx.prim.f2->x0 = ctx.corners_xy[2];
                        *(s32 *)&ctx.prim.f2->x1 = ctx.corners_xy[0];
                        LOOP_BLOCK(addPrim(ctx.ot_entry, ctx.prim.f2););
                        ctx.prim.f2++;
                    }
                    n = ctx.quad + 3;
                    ctx.stream += n;
                    if (ctx.gouraud != 0) {
                        ctx.stream += n;
                    }
                    if (ctx.textured != 0) {
                        ctx.stream += n * 2;
                    }
                } while (*ctx.stream == 0);
                break;
            }
        }
    }
    gfx_module.funcs.set_packet(ctx.prim.ft3);
}

/* Draws the mesh (fightstg_model_mesh_draw) on layer `id` with matrix `m`. */
void fightstg_model_mesh_queue_draw(FightstgMesh *obj, s32 id, MATRIX *m) {
    GfxLayer *layer = gfx_module.funcs.get_layer(id);

    layer->append_callback(layer, fightstg_model_mesh_draw, obj);
    obj->matrix = *m;
}

/* Draws the mesh (fightstg_model_mesh_draw_edges) on layer `id` with matrix `m`. */
void fightstg_model_mesh_queue_edges(FightstgMesh *obj, s32 id, MATRIX *m) {
    GfxLayer *layer = gfx_module.funcs.get_layer(id);

    layer->append_callback(layer, fightstg_model_mesh_draw_edges, obj);
    obj->matrix = *m;
}

void fightstg_model_mesh_update(FightstgMesh *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        if (obj->screen_xy != NULL) {
            heap_funcs.free(obj->screen_xy);
        }
        if (obj->screen_z != NULL) {
            heap_funcs.free(obj->screen_z);
        }
        if (obj->colors != NULL) {
            heap_funcs.free(obj->colors);
        }
        break;
    }
}

FightstgMesh *fightstg_model_mesh_create(void *file, FightstgPos pos) {
    FightstgMesh *obj = object_new(fightstg_model_mesh_update, sizeof(FightstgMesh), 0);

    obj->file = file;
    obj->vertices = cdload_module.get_subfile(0, file);
    obj->normals = cdload_module.get_subfile(1, file);
    obj->faces = cdload_module.get_subfile(2, file);
    obj->bounds = cdload_module.get_subfile(5, file);
    obj->texture_pos = pos;
    obj->queue_draw = fightstg_model_mesh_queue_draw;
    obj->queue_edges = fightstg_model_mesh_queue_edges;
    return obj;
}
