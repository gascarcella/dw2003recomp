#ifndef PSYQ_LIBGPU_H
#define PSYQ_LIBGPU_H

/* Our own declarations of the Psy-Q 4.7 LIBGPU interface, added as the game needs them. */

#include "common.h"

typedef struct {
    s16 x, y, w, h;
} RECT;

typedef struct {
    RECT disp;
    RECT screen;
    u8 isinter;
    u8 isrgb24;
    u8 pad0;
    u8 pad1;
} DISPENV;

DISPENV *SetDefDispEnv(DISPENV *env, s32 x, s32 y, s32 w, s32 h);
u32 *ClearOTagR(u32 *ot, s32 n);
s32 DrawSync(s32 mode);
int ResetGraph(int mode);
int SetGraphDebug(int level);
void SetDispMask(int mask);
int ClearImage(RECT *rect, u8 r, u8 g, u8 b);
int ClearImage2(RECT *rect, u8 r, u8 g, u8 b);

typedef struct {
    u32 tag;
    u32 code[15];
} DR_ENV;

typedef struct {
    RECT clip;
    s16 ofs[2];
    RECT tw;
    u16 tpage;
    u8 dtd;
    u8 dfe;
    u8 isbg;
    u8 r0, g0, b0;
    DR_ENV dr_env;
} DRAWENV;

typedef struct {
    unsigned addr : 24;
    unsigned len : 8;
    u8 r0, g0, b0, code;
} P_TAG;

#define setaddr(p, _addr) (((P_TAG *)(p))->addr = PTR_TO_U32(_addr))
#define getaddr(p) (u32)(((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)
#define setlen(p, _len) (((P_TAG *)(p))->len = (u8)(_len))
#define setcode(p, _code) (((P_TAG *)(p))->code = (u8)(_code))
#define getcode(p) (u8)(((P_TAG *)(p))->code)
#define setSemiTrans(p, abe) ((abe) ? setcode(p, getcode(p) | 0x02) : setcode(p, getcode(p) & ~0x02))
#define setSprt(p) setlen(p, 4), setcode(p, 0x64)
#define setRGB0(p, _r0, _g0, _b0) (p)->r0 = _r0, (p)->g0 = _g0, (p)->b0 = _b0
#define setXY0(p, _x0, _y0) (p)->x0 = (_x0), (p)->y0 = (_y0)
#define setUV0(p, _u0, _v0) (p)->u0 = (_u0), (p)->v0 = (_v0)
#define setWH(p, _w, _h) (p)->w = _w, (p)->h = _h
#define getTPage(tp, abr, x, y)                                                                  \
    ((((tp) & 0x3) << 7) | (((abr) & 0x3) << 5) | (((y) & 0x100) >> 4) | (((x) & 0x3ff) >> 6) | \
     (((y) & 0x200) << 2))
#define getClut(x, y) (((y) << 6) | (((x) >> 4) & 0x3f))
#define setXY4(p, _x0, _y0, _x1, _y1, _x2, _y2, _x3, _y3)                   \
    (p)->x0 = (_x0), (p)->y0 = (_y0), (p)->x1 = (_x1), (p)->y1 = (_y1), \
    (p)->x2 = (_x2), (p)->y2 = (_y2), (p)->x3 = (_x3), (p)->y3 = (_y3)
#define setUV4(p, _u0, _v0, _u1, _v1, _u2, _v2, _u3, _v3)                   \
    (p)->u0 = (_u0), (p)->v0 = (_v0), (p)->u1 = (_u1), (p)->v1 = (_v1), \
    (p)->u2 = (_u2), (p)->v2 = (_v2), (p)->u3 = (_u3), (p)->v3 = (_v3)
#define setXYWH(p, _x0, _y0, _w, _h)                                                     \
    (p)->x0 = (p)->x2 = (_x0), (p)->y0 = (p)->y1 = (_y0), (p)->x1 = (p)->x3 = (_x0) + (_w), \
    (p)->y2 = (p)->y3 = (_y0) + (_h)
#define setPolyF4(p) setlen(p, 5), setcode(p, 0x28)
#define setPolyFT3(p) setlen(p, 7), setcode(p, 0x24)
#define setPolyFT4(p) setlen(p, 9), setcode(p, 0x2C)
#define setPolyGT3(p) setlen(p, 9), setcode(p, 0x34)
#define setPolyGT4(p) setlen(p, 12), setcode(p, 0x3C)
#define setLineF2(p) setlen(p, 3), setcode(p, 0x40)
#define setLineF4(p) setlen(p, 6), setcode(p, 0x4C), ((p)->pad = 0x55555555)
#define _get_mode(dfe, dtd, tpage) \
    ((0xE1000000) | ((dtd) ? 0x0200 : 0) | ((dfe) ? 0x0400 : 0) | ((tpage) & 0x9FF))
#define setDrawTPage(p, dfe, dtd, tpage) setlen(p, 1), ((u32 *)(p))[1] = _get_mode(dfe, dtd, tpage)

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
} LINE_F2;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    s16 x3, y3;
    u32 pad;
} LINE_F4;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    s16 x3, y3;
} POLY_F4;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
} POLY_FT3;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad2;
} POLY_FT4;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    u8 r2, g2, b2, p2;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad2;
} POLY_GT3;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    u8 r2, g2, b2, p2;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad2;
    u8 r3, g3, b3, p3;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad3;
} POLY_GT4;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 w, h;
} SPRT;

typedef struct {
    u32 tag;
    u32 code[1];
} DR_TPAGE;

typedef struct {
    u32 tag;
    u32 code[5];
} DR_MOVE;

void SetDrawTPage(DR_TPAGE *p, s32 dfe, s32 dtd, s32 tpage);
void SetDrawMove(DR_MOVE *p, RECT *rect, s32 x, s32 y);
u16 GetTPage(s32 tp, s32 abr, s32 x, s32 y);
u16 GetClut(s32 x, s32 y);
void SetSemiTrans(void *p, s32 abe);
void SetSprt(SPRT *p);

DRAWENV *SetDefDrawEnv(DRAWENV *env, s32 x, s32 y, s32 w, s32 h);
void SetDrawEnv(DR_ENV *dr_env, DRAWENV *env);
DISPENV *PutDispEnv(DISPENV *env);
void LoadImage(RECT *rect, u32 *p);
s32 MoveImage(RECT *rect, s32 x, s32 y);
void DrawOTag(u32 *p);
u32 *ClearOTag(u32 *ot, s32 n);
u32 *BreakDraw(void);
s32 IsIdleGPU(s32 max_count);
void ContinueDraw(u32 *insaddr, u32 *contaddr);


typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} POLY_G4;

#define setPolyG4(p) setlen(p, 8), setcode(p, 0x38)
#define setRGB1(p, _r1, _g1, _b1) (p)->r1 = _r1, (p)->g1 = _g1, (p)->b1 = _b1
#define setRGB2(p, _r2, _g2, _b2) (p)->r2 = _r2, (p)->g2 = _g2, (p)->b2 = _b2
#define setRGB3(p, _r3, _g3, _b3) (p)->r3 = _r3, (p)->g3 = _g3, (p)->b3 = _b3

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
} POLY_G3;

#define setPolyG3(p) setlen(p, 6), setcode(p, 0x30)

#endif /* PSYQ_LIBGPU_H */
