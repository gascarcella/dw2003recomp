/* port/psyq/libcd.c: LIBCD. The command model is the PS1's as far as the game's callback chains need it (cdload.c:
 * CdControlF(Setloc) -> sync CdlComplete -> CdControlF(Setmode) -> sync -> CdControlF(ReadN) -> sync, then one
 * ready CdlDataReady per sector, CdControlF(Pause) -> sync):
 *  - CdControl/CdControlB (blocking) apply the command at once and return 1; no callback runs for them.
 *  - CdControlF (asynchronous) records the command; psyq_cd_tick() (the port's interrupt pump) applies it and runs
 *    the CdSyncCallback handler with CdlComplete. While a read is in progress each later tick delivers one sector to
 *    the CdReadyCallback handler (CdlDataReady), from the sector source psyq_cd_set_reader gave; without one (the
 *    M1 skeleton) the read ends with CdlDataEnd and the game's error path runs (cdload retries forever).
 *  - CdGetSector copies from the delivered sector, through it, in the sector size the mode byte selects.
 *  - CdIntToPos/CdPosToInt are real (BCD, with the 150-sector lead-in).
 *  - Streaming (CdRead2, St*) reports "no data": movies end at once (STDWTITL's players give up after 2000 polls).
 *
 * Assumptions to verify (M1 with the BIN, against the emulator's CD timing where it matters):
 *  - a blocking CdControl never calls the sync handler (the game registers it only around CdControlF reads);
 *  - CdlReadN acknowledges with CdlComplete before its first sector (cdload's state machine needs that order);
 *  - the sector window per mode byte: CdlModeSize1 (0x20) = 2340 bytes from the 4-byte header on (what cdload
 *    uses: mode 0xA0, docs/FORMATS.md), CdlModeSize0 (0x10) = 2328 bytes = data + EDC/ECC, else the 2048 data
 *    bytes; the result buffer's first byte is the status (0x02: motor on). */
#include <string.h>
#include "psyq_internal.h"
#include "psyq/libcd.h"

/* Commands the game sends. */
#define CdlSetloc 0x02
#define CdlReadN 0x06
#define CdlPause 0x09
#define CdlSetmode 0x0E
#define CdlReadS 0x1B

/* Completion statuses (the first argument of the handlers). */
#define CdlDataReady 1
#define CdlComplete 2
#define CdlDataEnd 4
#define CdlDiskError 5

#define CD_RAW_SECTOR 2352

typedef void (*PsyqCdHandler)(int status, u8 *result);

u8 D_80081454; /* LIBCD's StCdIntrFlag: set by the CD interrupt while streaming (never here) */

static PsyqCdHandler psyq_cd_sync_handler;
static PsyqCdHandler psyq_cd_ready_handler;
static int (*psyq_cd_reader)(unsigned lba, u8 *sector);

static int psyq_cd_pending;    /* CdControlF's command, 0 none */
static u8 psyq_cd_param[8];    /* its parameter bytes */
static int psyq_cd_reading;    /* a ReadN/ReadS is in progress */
static u32 psyq_cd_loc;        /* the Setloc position, as a sector number */
static u32 psyq_cd_next_lba;   /* the next sector a read delivers */
static u8 psyq_cd_mode;        /* the Setmode byte */
static u8 psyq_cd_status = 0x02;

static u8 psyq_cd_raw[CD_RAW_SECTOR]; /* the delivered sector */
static int psyq_cd_have_sector;
static u32 psyq_cd_view_ofs;   /* the window of psyq_cd_raw the mode byte selects ... */
static u32 psyq_cd_view_len;
static u32 psyq_cd_cursor;     /* ... and how far CdGetSector has read it */

void psyq_cd_set_reader(int (*read)(unsigned lba, u8 *sector)) {
    psyq_cd_reader = read;
}

/* ---- positions (real) ---- */

static u8 psyq_cd_itob(int i) {
    return (u8)(((i / 10) << 4) | (i % 10));
}

static int psyq_cd_btoi(u8 b) {
    return (b >> 4) * 10 + (b & 0xF);
}

CdlLOC *CdIntToPos(int i, CdlLOC *p) {
    i += 150;
    p->sector = psyq_cd_itob(i % 75);
    p->second = psyq_cd_itob((i / 75) % 60);
    p->minute = psyq_cd_itob(i / 75 / 60);
    return p;
}

int CdPosToInt(CdlLOC *p) {
    return psyq_cd_btoi(p->minute) * 60 * 75 + psyq_cd_btoi(p->second) * 75 + psyq_cd_btoi(p->sector) - 150;
}

/* ---- commands ---- */

static void psyq_cd_set_result(u8 *result) {
    if (result != NULL) {
        memset(result, 0, 8);
        result[0] = psyq_cd_status;
    }
}

/* The effect of a command once the drive has taken it. */
static void psyq_cd_apply(int com, const u8 *param) {
    switch (com) {
    case CdlSetloc:
        if (param != NULL) {
            psyq_cd_loc = (u32)CdPosToInt((CdlLOC *)param);
        }
        break;
    case CdlSetmode:
        if (param != NULL) {
            psyq_cd_mode = param[0];
        }
        break;
    case CdlReadN:
    case CdlReadS:
        psyq_cd_reading = 1;
        psyq_cd_next_lba = psyq_cd_loc;
        psyq_cd_have_sector = 0;
        break;
    case CdlPause:
        psyq_cd_reading = 0;
        break;
    default:
        break;
    }
}

static int psyq_cd_blocking(const char *who, u8 com, u8 *param, u8 *result) {
    PSYQ_TRACE("%s %02x param %02x %02x %02x %02x", who, com, param ? param[0] : 0, param ? param[1] : 0,
               param ? param[2] : 0, param ? param[3] : 0);
    psyq_cd_apply(com, param);
    psyq_cd_set_result(result);
    return 1;
}

int CdControl(u8 com, u8 *param, u8 *result) {
    return psyq_cd_blocking("CdControl", com, param, result);
}

int CdControlB(u8 com, u8 *param, u8 *result) {
    return psyq_cd_blocking("CdControlB", com, param, result);
}

/* Asynchronous: completes on the next psyq_cd_tick. The game issues one at a time (from the previous one's
 * completion handler); a second one before the tick replaces the first. */
int CdControlF(u8 com, u8 *param) {
    PSYQ_TRACE("CdControlF %02x param %02x %02x %02x %02x", com, param ? param[0] : 0, param ? param[1] : 0,
               param ? param[2] : 0, param ? param[3] : 0);
    if (psyq_cd_pending != 0) {
        PSYQ_TRACE("CdControlF: command %02x replaces the pending %02x", com, psyq_cd_pending);
    }
    psyq_cd_pending = com;
    memset(psyq_cd_param, 0, sizeof(psyq_cd_param));
    if (param != NULL) {
        memcpy(psyq_cd_param, param, com == CdlSetloc ? 4 : 1);
    }
    return 1;
}

/* Loads sector `lba` into the sector buffer and sets the window the mode byte selects. */
static int psyq_cd_fetch(u32 lba) {
    if (psyq_cd_reader == NULL || !psyq_cd_reader(lba, psyq_cd_raw)) {
        return 0;
    }
    if (psyq_cd_mode & 0x20) {
        psyq_cd_view_ofs = 12;
        psyq_cd_view_len = 2340;
    } else if (psyq_cd_mode & 0x10) {
        psyq_cd_view_ofs = 24;
        psyq_cd_view_len = 2328;
    } else {
        psyq_cd_view_ofs = 24;
        psyq_cd_view_len = 2048;
    }
    psyq_cd_cursor = 0;
    psyq_cd_have_sector = 1;
    return 1;
}

int psyq_cd_tick(void) {
    u8 result[8];

    if (psyq_cd_pending != 0) {
        int com = psyq_cd_pending;

        psyq_cd_pending = 0;
        psyq_cd_apply(com, psyq_cd_param);
        psyq_cd_set_result(result);
        PSYQ_TRACE("cd tick: command %02x complete%s", com, psyq_cd_sync_handler ? ", sync handler" : "");
        if (psyq_cd_sync_handler != NULL) {
            psyq_cd_sync_handler(CdlComplete, result);
            return 1;
        }
        return 0;
    }
    if (psyq_cd_reading) {
        psyq_cd_set_result(result);
        if (psyq_cd_fetch(psyq_cd_next_lba)) {
            PSYQ_TRACE("cd tick: sector %u ready%s", psyq_cd_next_lba, psyq_cd_ready_handler ? ", ready handler" : "");
            psyq_cd_next_lba++;
            if (psyq_cd_ready_handler != NULL) {
                psyq_cd_ready_handler(CdlDataReady, result);
                return 1;
            }
        } else {
            PSYQ_TRACE("cd tick: no sector %u: data end%s", psyq_cd_next_lba,
                       psyq_cd_ready_handler ? ", ready handler" : "");
            psyq_cd_reading = 0;
            if (psyq_cd_ready_handler != NULL) {
                psyq_cd_ready_handler(CdlDataEnd, result);
                return 1;
            }
        }
    }
    return 0;
}

/* Copies `size` words of the delivered sector, continuing where the previous call stopped (cdload reads the
 * 3-word header, then the 0x200 data words). 1 = data was there; 0 = none (the copy is zero-filled). */
int CdGetSector(void *madr, int size) {
    u32 bytes = (u32)size * 4;
    u32 avail = 0;

    if (psyq_cd_have_sector && psyq_cd_cursor < psyq_cd_view_len) {
        avail = psyq_cd_view_len - psyq_cd_cursor;
        if (avail > bytes) {
            avail = bytes;
        }
        memcpy(madr, psyq_cd_raw + psyq_cd_view_ofs + psyq_cd_cursor, avail);
        psyq_cd_cursor += avail;
    }
    if (avail < bytes) {
        memset((u8 *)madr + avail, 0, bytes - avail);
    }
    return avail != 0;
}

void *CdReadyCallback(void (*func)()) {
    PsyqCdHandler prev = psyq_cd_ready_handler;

    PSYQ_TRACE("CdReadyCallback %u", PSYQ_PTR(func));
    psyq_cd_ready_handler = (PsyqCdHandler)func;
    return (void *)prev;
}

void *CdSyncCallback(void (*func)()) {
    PsyqCdHandler prev = psyq_cd_sync_handler;

    PSYQ_TRACE("CdSyncCallback %u", PSYQ_PTR(func));
    psyq_cd_sync_handler = (PsyqCdHandler)func;
    return (void *)prev;
}

/* 1 = ok. */
int CdInit(void) {
    PSYQ_TRACE("CdInit");
    psyq_cd_pending = 0;
    psyq_cd_reading = 0;
    psyq_cd_sync_handler = NULL;
    psyq_cd_ready_handler = NULL;
    return 1;
}

/* Returns the previous level (0). */
int CdSetDebug(int level) {
    PSYQ_TRACE("CdSetDebug %d", level);
    return 0;
}

/* ---- streaming (stubs: no data) ---- */

/* Starts the streaming read (ReadS with the ring buffer) on the PS1; here nothing streams. 1 = started. */
int CdRead2(long mode) {
    PSYQ_TRACE("CdRead2 %lx", mode);
    psyq_cd_reading = 0;
    return 1;
}

void StSetRing(u32 *ring_addr, u32 ring_size) {
    PSYQ_TRACE("StSetRing %u size %u sectors", PSYQ_PTR(ring_addr), ring_size);
}

void StSetStream(u32 mode, u32 start_frame, u32 end_frame, void (*func1)(), void (*func2)()) {
    PSYQ_TRACE("StSetStream mode %u frames %u..%u cb %u %u", mode, start_frame, end_frame, PSYQ_PTR(func1),
               PSYQ_PTR(func2));
}

/* 1 = no frame is ready (0 would hand one out through addr and header). */
u32 StGetNext(u32 **addr, u32 **header) {
    (void)addr;
    (void)header;
    return 1;
}

/* 0 = ok (the PS1 returns the ring state). */
u32 StFreeRing(u32 *base) {
    PSYQ_TRACE("StFreeRing %u", PSYQ_PTR(base));
    return 0;
}

void StUnSetRing(void) {
    PSYQ_TRACE("StUnSetRing");
}

void StCdInterrupt(void) {
    PSYQ_TRACE("StCdInterrupt");
}
