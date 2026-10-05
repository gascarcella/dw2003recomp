#ifndef PSYQ_LIBPAD_H
#define PSYQ_LIBPAD_H

/* Our own declarations of the Psy-Q 4.7 LIBPAD interface, added as the game needs them. */

#include "common.h"

void PadInitDirect(u8 *, u8 *);
void PadInitMtap(u8 *, u8 *);
int PadStartCom(void);
void PadStopCom(void);
int PadChkVsync(void);
int PadGetState(int port);
int PadInfoMode(int port, int term, int offs);
int PadInfoAct(int port, int actno, int term);
void PadSetAct(int port, u8 *data, int len);
int PadSetActAlign(int port, u8 *data);
int PadSetMainMode(int socket, int offs, int lock);

#endif /* PSYQ_LIBPAD_H */
