#ifndef PSYQ_LIBSND_H
#define PSYQ_LIBSND_H

/* Our own declarations of the Psy-Q 4.7 LIBSND interface, added as the game needs them. */

#include "common.h"

void SsInit(void);
void SsStart2(void);
void SsSetTableSize(u8 *table, s16 s_max, s16 t_max);
void SsSetTickMode(s32 tick_mode);
void SsSeqCalledTbyT(void);
void SsSetMVol(s16 voll, s16 volr);
void SsSetSerialAttr(char s_num, char attr, char mode);
void SsSetSerialVol(char s_num, s16 voll, s16 volr);
s16 SsVabOpenHeadSticky(u8 *addr, s16 vab_id, u32 sbaddr);
s16 SsVabTransBody(u8 *addr, s16 vab_id);
s16 SsVabTransCompleted(s16 immediate_flag);
void SsVabClose(s16 vab_id);
s16 SsSepOpen(u32 *addr, s16 vab_id, s16 seq_num);
void SsSepPlay(s16 access_num, s16 seq_num, char play_mode, s16 l_count);
void SsSepStop(s16 access_num, s16 seq_num);
void SsSepClose(s16 access_num);
void SsSepSetVol(s16 access_num, s16 seq_num, s16 voll, s16 volr);
void SsSepSetDecrescendo(s16 access_num, s16 seq_num, s16 vol, s32 v_time);
s16 SsUtSetReverbType(s16 type);
void SsUtSetReverbDepth(s16 ldepth, s16 rdepth);
void SsUtReverbOn(void);
void SsUtAllKeyOff(s16 mode);
s16 SsUtKeyOn(s16 vabId, s16 prog, s16 tone, s16 note, s16 fine, s16 voll, s16 volr);
s16 SsUtKeyOff(s16 voice, s16 vabId, s16 prog, s16 tone, s16 note);

#endif /* PSYQ_LIBSND_H */
