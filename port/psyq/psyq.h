/* port/psyq/psyq.h: the Psy-Q shim's interface to the port runtime (port/src/, T8). */
#ifndef PORT_PSYQ_H
#define PORT_PSYQ_H

#include <stdio.h>             /* FILE */
#include "common.h"            /* the game's types (u8, s32, ...), include/port.h's port_* declarations */

/* Runs the handler VSyncCallback registered (if any) once and advances the frame counter VSync() reports. */
void psyq_vsync_tick(void);
/* Completes the pending CD command: calls the CdSyncCallback / CdReadyCallback handlers as the PS1 would for a
 * command that finished (the stub has no data: a read completes with no sectors). Returns 1 if a handler ran. */
int psyq_cd_tick(void);
/* Tracing: on/off and the stream (stderr by default). */
void psyq_set_trace(int on, FILE *stream);

/* Provided by the port runtime (port/src/), not by the shim: a function the game needs that the skeleton does
 * not implement: prints `fn` and exits with status 3. A stub calls it only when it cannot fake a result. */
void port_unimplemented(const char *fn);

/* ------------------------------------------------------------------------------------------------------------------
 * Optional extras (the runtime may ignore every one of them; none is needed to link or to run the skeleton).
 * ---------------------------------------------------------------------------------------------------------------- */

/* The window DrawOTag may walk. A 24-bit tag is resolved as `(ot & ~0xFFFFFF) + tag` (PC_PORT_PLAN 2.4) and followed
 * only while it stays inside [base, base + size); the default window is [port_heap_start, port_heap_end). Call it
 * once with the whole arena so that primitives outside the heap (FIGHTSTG's cursor OT) are walked too. */
void psyq_set_arena(const void *base, unsigned long size);

/* A sector source for LIBCD (M1 "LIBCD over the BIN"). `read` copies the raw 2352-byte sector `lba` (0 = the
 * first sector of the data track, as CdIntToPos counts it) into `sector` and returns 1, or returns 0 when the
 * sector does not exist. Without a source every read ends at once with CdlDataEnd (the game retries forever). */
void psyq_cd_set_reader(int (*read)(unsigned lba, u8 *sector));

/* The pad the shim reports on `port` (0 or 1): a digital pad whose buttons are `buttons` (PS1 bit order,
 * active high: bit 3 START, bit 4..7 up/right/down/left, bit 12..15 triangle/circle/cross/square);
 * `connected` 0 reports no controller. Default: port 0 connected with nothing pressed, port 1 empty. */
void psyq_pad_set(int port, int connected, u16 buttons);

/* The primitive stream recorder (LIBGPU): the FNV-1a hash of every primitive DrawOTag/ContinueDraw walked since
 * the previous call (and the number of them in *count, if not NULL), then resets both. */
u32 psyq_gpu_take_hash(u32 *count);

/* LIBC2 (strlen, strcpy, strncpy, memcpy, strcspn, atoi) and LIBAPI (open, read, write, close) are the host libc:
 * the shim defines none of them, on purpose (a definition in the executable would replace libc's for every shared
 * library in the process, SDL included). Our include/psyq/libc2.h declares strlen/strcspn as returning s32 and
 * memcpy's length as u32, libapi.h's read/write take an s32 length; on the LP64 host ABIs (x86-64, AArch64) the low
 * 32 bits of a size_t return and a 32-bit length register agree with the real prototypes, so the game's calls
 * resolve to libc unchanged. SHOCKTST's `sim:C:\...` paths do not exist on the host, so its open() fails (-1) as
 * the skeleton wants (README.md "LIBC2 and LIBAPI"). */

#endif /* PORT_PSYQ_H */
