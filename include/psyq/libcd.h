#ifndef PSYQ_LIBCD_H
#define PSYQ_LIBCD_H

/* Our own declarations of the Psy-Q 4.7 LIBCD interface, added as the game needs them
 * (no Sony headers are used; DECISIONS "Code only: no game data, SDK or BIOS in the repo"). */

#include "common.h"

/* CD position as minute/second/sector (BCD) + track. */
typedef struct CdlLOC {
    u8 minute;
    u8 second;
    u8 sector;
    u8 track;
} CdlLOC;

/* Converts a sector number to a CdlLOC and returns `p`. (Its byte-identical LIBDS twin
 * DsIntToPos isn't linked: docs/TOOLCHAIN.md, "Ambiguous objects".) */
CdlLOC *CdIntToPos(int i, CdlLOC *p);
int CdPosToInt(CdlLOC *p);

int CdGetSector(void *madr, int size);
void *CdReadyCallback(void (*func)());
void *CdSyncCallback(void (*func)());
int CdInit(void);
int CdSetDebug(int level);
int CdControl(u8 com, u8 *param, u8 *result);
int CdControlB(u8 com, u8 *param, u8 *result);
int CdControlF(u8 com, u8 *param);

/* Streaming (LIBCD's st_*.c objects). */
int CdRead2(long mode);
void StSetRing(u32 *ring_addr, u32 ring_size);
void StSetStream(u32 mode, u32 start_frame, u32 end_frame, void (*func1)(), void (*func2)());
u32 StGetNext(u32 **addr, u32 **header);
u32 StFreeRing(u32 *base);
void StUnSetRing(void);
void StCdInterrupt(void);
extern u8 D_80081454; /* StCdIntrFlag (STDWTITL; unnamed: LIBCD's .bss is one block) */

#endif /* PSYQ_LIBCD_H */
