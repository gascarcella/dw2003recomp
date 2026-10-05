#ifndef PSYQ_LIBGS_H
#define PSYQ_LIBGS_H

/* Our own declarations of the Psy-Q 4.7 LIBGS interface, added as the game needs them. */

#include "common.h"
#include "psyq/libgte.h"

/* GsGetTimInfo's result. */
typedef struct {
    /* 0x00 */ u32 pmode;
    /* 0x04 */ s16 px;
    /* 0x06 */ s16 py;
    /* 0x08 */ u16 pw;
    /* 0x0A */ u16 ph;
    /* 0x0C */ u32 *pixel;
    /* 0x10 */ s16 cx;
    /* 0x12 */ s16 cy;
    /* 0x14 */ u16 cw;
    /* 0x16 */ u16 ch;
    /* 0x18 */ u32 *clut;
} GsIMAGE; /* size 0x1C */

/* A flat (directional) light. */
typedef struct {
    s32 vx, vy, vz;
    u8 r, g, b;
} GsF_LIGHT;

void GsInitGraph(u16 w, u16 h, u16 intl, u16 dither, u16 vram);
void GsGetTimInfo(u32 *tim, GsIMAGE *image);
void GsSetProjection(s32 h);
void GsInit3D(void);
s32 GsSetFlatLight(s32 id, GsF_LIGHT *lt);
void GsSetLightMode(s32 mode);

/* A coordinate system (a node of a hierarchy). */
typedef struct _GsCOORDINATE2 {
    /* 0x00 */ u32 flg;
    /* 0x04 */ MATRIX coord;
    /* 0x24 */ MATRIX workm;
    /* 0x44 */ void *param; /* GsCOORD2PARAM */
    /* 0x48 */ struct _GsCOORDINATE2 *super;
    /* 0x4C */ struct _GsCOORDINATE2 *sub;
} GsCOORDINATE2; /* size 0x50 */

/* A view: viewpoint, reference point, twist, and the coordinate system they are in. */
typedef struct {
    /* 0x00 */ s32 vpx, vpy, vpz;
    /* 0x0C */ s32 vrx, vry, vrz;
    /* 0x18 */ s32 rz;
    /* 0x1C */ GsCOORDINATE2 *super;
} GsRVIEW2; /* size 0x20 */

s32 GsSetRefView2(GsRVIEW2 *pv);

#endif /* PSYQ_LIBGS_H */
