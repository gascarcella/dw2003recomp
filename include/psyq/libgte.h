#ifndef PSYQ_LIBGTE_H
#define PSYQ_LIBGTE_H

/* Our own declarations of the Psy-Q 4.7 LIBGTE interface, added as the game needs them. */

#include "common.h"

typedef struct {
    u8 r, g, b, cd;
} CVECTOR;

typedef struct {
    s16 m[3][3];
    s32 t[3];
} MATRIX;

typedef struct {
    s32 vx, vy, vz, pad;
} VECTOR;

typedef struct {
    s16 vx, vy, vz, pad;
} SVECTOR;

typedef struct {
    s16 vx, vy;
} DVECTOR;

void InitGeom(void);
void SetGeomOffset(s32 ofx, s32 ofy);
MATRIX *RotMatrixYXZ_gte(SVECTOR *r, MATRIX *m);
MATRIX *RotMatrixZYX_gte(SVECTOR *r, MATRIX *m);
MATRIX *ScaleMatrix(MATRIX *m, VECTOR *v);
SVECTOR *ApplyMatrixSV(MATRIX *m, SVECTOR *v0, SVECTOR *v1);
void SetBackColor(s32 rbk, s32 gbk, s32 bbk);
s32 rsin(s32 a);

#endif /* PSYQ_LIBGTE_H */
