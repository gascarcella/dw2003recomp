/* port/psyq/psyq.h: the Psy-Q shim's interface to the port runtime (port/src/, T8). */
#ifndef PORT_PSYQ_H
#define PORT_PSYQ_H

#include <stdio.h>             /* FILE */
#include "common.h"            /* the game's types (u8, s32, ...), include/port.h's port_* declarations */

/* Runs the handler VSyncCallback registered (if any) once and advances the frame counter VSync() reports. */
void psyq_vsync_tick(void);
/* The runtime's per-frame hook: called at the end of every vsync tick (from VSync() as from the pump), after the
 * game's VSyncCallback handler. NULL: none. */
void psyq_set_vsync_hook(void (*hook)(void));
/* The CD "interrupt", once per vsync: completes the pending CdControlF command (CdSyncCallback's handler), delivers
 * this tick's sectors of a read (CdReadyCallback's handler, once per sector) or of a stream (CdRead2: into the ring
 * StSetRing gave). Returns 1 if a handler ran. */
int psyq_cd_tick(void);
/* LIBCD's timing model (port_disc_set_speed): PSYQ_CD_REALISTIC (the default) = the drive's speed from the Setmode
 * byte (bit 0x80: 150 sectors/s, else 75) at 50 vsyncs/s and a seek delay before a read's first sector;
 * PSYQ_CD_INSTANT = no seek, and a read delivers up to 75 sectors per tick, as many as the game takes. */
#define PSYQ_CD_REALISTIC 0
#define PSYQ_CD_INSTANT 1
void psyq_cd_set_timing(int timing);
/* The console's reset (port_reset_state): every library back to its power-on state (handlers, the CD, the pad, the
 * GPU recorder, the sound stubs), the memory cards' contents kept; psyq_mcrd_reset is LIBMCRD's part (libmcrd.c),
 * which psyq_reset calls. */
void psyq_reset(void);
void psyq_mcrd_reset(void);
/* LIBMCRD's card store (port/src/memcard.c owns the images and their files): card `slot` (0, 1) is the 128 KB image
 * at `image` (NULL: no card); `written` is called after every change to it (to write the file back). */
void psyq_mcrd_set_card(int slot, u8 *image, void (*written)(int slot));
/* A freshly formatted card, as PCSX-Redux creates one ("MC" frame, 15 free directory frames, ...). */
void psyq_mcrd_format_image(u8 *image);
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

/* ---- The video output (M2: T9's software GPU in libgpu.c, read by T10's window in port/src/video.c) ----
 * psyq_gpu_vram: the 1024x512 VRAM, 16-bit pixels (row-major, 1024 per row), as the GPU leaves it after every command.
 * psyq_gpu_display: what the TV shows: the display area PutDispEnv set (VRAM x, y, width, height in VRAM pixels:
 * a 24-bit area is 1.5 VRAM pixels per screen pixel), 24-bit colour, interlace, and whether SetDispMask enabled it. */
typedef struct PsyqDisplay {
    int x, y, w, h; /* DISPENV.disp */
    int rgb24;      /* DISPENV.isrgb24 */
    int interlace;  /* DISPENV.isinter */
    int enabled;    /* SetDispMask(1) */
} PsyqDisplay;
const u16 *psyq_gpu_vram(void);
void psyq_gpu_display(PsyqDisplay *out);

/* LIBC2 (strlen, strcpy, strncpy, memcpy, strcspn, atoi) and LIBAPI (open, read, write, close) are the host libc:
 * the shim defines none of them, on purpose (a definition in the executable would replace libc's for every shared
 * library in the process, SDL included). Our include/psyq/libc2.h declares strlen/strcspn as returning s32 and
 * memcpy's length as u32, libapi.h's read/write take an s32 length; on the LP64 host ABIs (x86-64, AArch64) the low
 * 32 bits of a size_t return and a 32-bit length register agree with the real prototypes, so the game's calls
 * resolve to libc unchanged. SHOCKTST's `sim:C:\...` paths do not exist on the host, so its open() fails (-1) as
 * the skeleton wants (README.md "LIBC2 and LIBAPI"). */

#endif /* PORT_PSYQ_H */
