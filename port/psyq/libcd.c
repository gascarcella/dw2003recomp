/* port/psyq/libcd.c: LIBCD over a sector source (port/src/disc.c: the user's BIN). The command model is the PS1's
 * as far as the game's callback chains need it (cdload.c: CdControlF(Setloc) -> sync CdlComplete ->
 * CdControlF(Setmode) -> sync -> CdControlF(ReadN) -> sync, then one CdlDataReady per sector to the ready handler,
 * CdControlF(Pause) -> sync), and the movie stream (CdRead2 and St*) is LIBCD's ring of whole frames.
 *
 * Time is counted in vsync ticks only (psyq_cd_tick, run by the port's pump once per vsync; never the wall clock):
 *  - CdControl/CdControlB (blocking) apply the command at once and return 1; no callback runs for them.
 *  - CdControlF (asynchronous) records the command; the next tick applies it and runs the CdSyncCallback handler
 *    with CdlComplete. A command issued from a handler waits for the following tick.
 *  - A read (ReadN/ReadS) delivers sectors from the sector source, each with its own CdlDataReady call to the
 *    CdReadyCallback handler, the new sector in CdGetSector's window. The tick stops delivering as soon as a handler
 *    issues a command (cdload's Pause after its last sector): the command completes on the next tick, before any
 *    later sector, and a Pause ends the read. A sector the source does not have ends the read with CdlDataEnd
 *    (without a source, at once: the game retries forever).
 *  - Timing "realistic" (the default; psyq_cd_set_timing): the drive's rate from the Setmode byte (bit 0x80 double
 *    speed: 150 sectors/s = 3 per tick at 50 Hz; else 75/s = 1.5 per tick, delivered as 1, 2, 1, 2, ...), starting
 *    the tick after the read's acknowledgement, plus a seek before the first sector when the read follows a Setloc:
 *    PSYQ_CD_SEEK_BASE ticks + one per PSYQ_CD_SEEK_SPAN sectors between the head and the target, at most
 *    PSYQ_CD_SEEK_MAX (60 ms for a short seek up to 800 ms across the whole disc). Real drives (psx-spx "CDROM
 *    Drive"): a Setloc + read first moves the sled and waits for the target sector's address to come by, a few
 *    tens of ms nearby up to about a second over the whole disc, with no fixed formula; this model is a
 *    deterministic stand-in, not a measurement. A read with no Setloc since the last one continues at the head.
 *  - Timing "instant": no seek; sectors start in the same tick as the acknowledgement, up to
 *    PSYQ_CD_INSTANT_SECTORS per tick (as many as the game takes before it pauses or issues a command).
 *
 * The stream (stdwtitl_80082D70.c's player: StSetRing(ring, 32 sectors), StSetStream, CdControl Setloc/Setmode,
 * CdRead2(0x1E0), then StGetNext/StFreeRing per frame, CdControlB(Pause) + StUnSetRing at the end) reads from the
 * Setloc position at the drive's rate. Video sectors (Form 1 data whose first word is the StHEADER magic 0x80010160:
 * id 0x0160, type 0x8001; then secCount, nSectors, frameCount, frameSize, width, height, ...; 32 bytes of header and
 * 2016 bytes of MDEC data per sector, as psx-spx "CDROM File Video Contents" and MOVIE*.STR's sectors have them)
 * are assembled into whole frames; XA audio sectors go to the XA decoder (below), other sectors are skipped. A frame
 * lives in the game's own ring buffer (StSetRing's address and size: 32 sectors = 64 KB here) as one contiguous slot:
 * the first sector's 32-byte StHEADER, then the nSectors * 2016 data bytes in secCount order. StGetNext hands out
 * the oldest complete frame (*addr = its data, *header = its StHEADER), StFreeRing(addr) releases it. This layout is
 * ours (LIBCD's own placement of headers and data in the ring is not documented publicly); it keeps every pointer
 * the game sees inside game memory (the arena) and the data contiguous, which is what DecDCTvlc2 needs.
 * When the ring has no room for a new frame: "realistic" drops the frame's sectors (the drive keeps reading at its
 * rate: a consumer too slow loses frames, as LIBCD's ring overflows; to verify), "instant" waits (the sector is
 * read again on a later tick).
 *
 * StGetNext is polled without a wait hook (stdwtitl_get_next_frame: up to 2000 x 2000 polls); on the PS1 the CD and
 * vsync interrupts keep running while it spins. Here every PSYQ_ST_POLLS_PER_VSYNC consecutive empty polls run one
 * vsync tick (psyq_vsync_tick: the game's VSyncCallback handler, the pump's frame work and the CD tick), so the
 * movie plays at the drive's rate, not at the host's. The constant is an estimate (a PAL vsync is 677,376 CPU
 * cycles at 33.8688 MHz; a poll of the loop is taken as ~135 cycles), not a measurement.
 *
 * XA audio (M5; docs/SOUND.md "CD audio"), as psx-spx "Data/ADPCM Sector Filtering/Delivery" has it: with the Setmode
 * byte's bit 0x40 (XA-ADPCM; the movies' CdRead2(0x1E0)), a Mode 2 sector whose submode has audio and real time (0x44)
 * goes to the drive's XA decoder instead of the CPU, in a read (no CdlDataReady for it) as in the stream; with bit
 * 0x08 (XA-Filter) only one whose file and channel match CdlSetfilter's, the others being dropped. The game sets no
 * filter: the movies have one audio channel (file 1, channel 1, coding 01h: stereo, 37,800 Hz, 4 bits) in every
 * eighth sector, at double speed 18.75 sectors/s = 37,800 frames/s. port/psyq/xa.c decodes a sector and resamples it
 * to 44,100 Hz (2016 frames -> 2352) into this file's FIFO, in the CD tick that delivers the sector. The FIFO feeds
 * the SPU's CD input (spu_cd_input) at the end of every CD tick with PSYQ_XA_PER_TICK = 882 frames: what the next
 * vsync's render consumes at 50 Hz (audio.c renders in the vsync pre-hook, before the tick). The sectors arrive in
 * whole ticks (3 per tick: an audio one after 2, 3, 3 ticks), so a decoder that started playing at once would run dry
 * 294 frames before the next audio sector, where the hardware, whose sectors arrive continuously, has no gap. A run's
 * first sector therefore waits one tick (20 ms of latency) before its samples go out; the FIFO then never runs dry
 * while the drive streams (its low point is 588 frames). When it does run dry (the stream ended, or the reads fell
 * behind), the run ends and the next audio sector starts a new one with its wait. Beyond PSYQ_XA_FIFO frames (instant
 * timing reads ahead) the newest are dropped, counted in the trace. A Pause, a new read, StUnSetRing, CdInit and the
 * reset flush the FIFO and reset the decoder (ADPCM history, resampler ring: psx-spx does not say when the hardware
 * clears them; a movie starts from silence here); at most one tick's frames, already in the SPU's queue, still play.
 * The drive's own volume matrix (ATV0..3, LIBCD's CdMix) stays at its power-on 80h = unity, left to left and right to
 * right (the game never sets it); the SPU applies the CD volume (1B0h/1B2h) and SPUCNT bit 0, which CdInit sets.
 * The drive's rate is per second: psyq_cd_set_vsync_hz (the nominal rate, port/src/pump.c port_rate: 50, or 60 with
 * the 60 Hz setting) turns it into sectors and XA frames per tick (2.5 sectors and 735 frames at 60), and the seeks keep
 * their milliseconds. A --fps other than the nominal rate changes neither: the SPU's queue then fills (and drops) or
 * runs dry.
 *
 * Assumptions to verify (against the emulator where it matters):
 *  - a blocking CdControl never calls the sync handler (the game registers it only around CdControlF reads);
 *  - CdlReadN acknowledges with CdlComplete before its first sector (cdload's state machine needs that order);
 *  - the sector window per mode byte: CdlModeSize1 (0x20) = 2340 bytes from the 4-byte header on (what cdload
 *    uses: mode 0xA0, docs/FORMATS.md), CdlModeSize0 (0x10) = 2328 bytes = data + EDC/ECC, else the 2048 data
 *    bytes; the result buffer's first byte is the status (0x02: motor on);
 *  - StSetStream's callbacks (func1/func2; the game passes none) are not called; frames outside
 *    [start_frame, end_frame] are skipped. */
#include <stdio.h>
#include <string.h>
#include "psyq_internal.h"
#include "psyq/libcd.h"
#include "spu.h"
#include "xa.h"

/* Commands the game sends. */
#define CdlSetloc 0x02
#define CdlReadN 0x06
#define CdlPause 0x09
#define CdlSetfilter 0x0D
#define CdlSetmode 0x0E
#define CdlReadS 0x1B

/* Completion statuses (the first argument of the handlers). */
#define CdlDataReady 1
#define CdlComplete 2
#define CdlDataEnd 4
#define CdlDiskError 5

#define CD_RAW_SECTOR 2352
#define PSYQ_CD_SEEK_BASE 3           /* ticks */
#define PSYQ_CD_SEEK_SPAN 8192        /* sectors per further tick */
#define PSYQ_CD_SEEK_MAX 40           /* ticks */
#define PSYQ_CD_INSTANT_SECTORS 75    /* per tick */
#define PSYQ_ST_POLLS_PER_VSYNC 5000
#define PSYQ_XA_FIFO 16384            /* 44,100 Hz frames decoded, not yet in the SPU (7 sectors' worth) */
#define PSYQ_XA_PER_TICK ((u32)(SPU_RATE / psyq_cd_vsync_hz)) /* 882 frames to the SPU per tick at 50 Hz, 735 at 60 */

#define ST_MAGIC 0x80010160u
#define ST_HEADER_SIZE 32
#define ST_DATA_SIZE 2016             /* MDEC bytes per video sector */
#define ST_MAX_SECTORS 64             /* per frame (a bit mask) */
#define ST_SLOTS 32

typedef void (*PsyqCdHandler)(int status, u8 *result);

u8 D_80081454; /* LIBCD's StCdIntrFlag: set by the CD interrupt while MDEC runs on the PS1 (never here) */

enum { CD_IDLE, CD_READ, CD_STREAM };

static int psyq_cd_timing = PSYQ_CD_REALISTIC;
static int psyq_cd_vsync_hz = 50; /* the vsyncs per second (psyq_cd_set_vsync_hz): 50 (PAL) or 60 */
static PsyqCdHandler psyq_cd_sync_handler;
static PsyqCdHandler psyq_cd_ready_handler;
static int (*psyq_cd_reader)(unsigned lba, u8 *sector);

static int psyq_cd_pending;    /* CdControlF's command, 0 none */
static u8 psyq_cd_param[8];    /* its parameter bytes */
static int psyq_cd_reading;    /* CD_IDLE, CD_READ (ReadN/ReadS) or CD_STREAM (CdRead2) */
static u32 psyq_cd_loc;        /* the Setloc position, as a sector number */
static int psyq_cd_loc_new;    /* a Setloc since the last read started */
static u32 psyq_cd_head;       /* where the head is: the sector after the last one read */
static u32 psyq_cd_next_lba;   /* the next sector a read delivers */
static int psyq_cd_seek_ticks; /* ticks left before a read's first sector */
static int psyq_cd_rate_acc;   /* sectors * psyq_cd_vsync_hz owed to the read (realistic) */
static int psyq_cd_ack_tick;   /* the read was acknowledged in this tick: realistic starts on the next */
static u8 psyq_cd_mode;        /* the Setmode byte */
static u8 psyq_cd_status = 0x02;
static u8 psyq_cd_filter[2];   /* CdlSetfilter's file and channel */

static u8 psyq_cd_raw[CD_RAW_SECTOR]; /* the delivered sector */
static int psyq_cd_have_sector;
static u32 psyq_cd_view_ofs;   /* the window of psyq_cd_raw the mode byte selects ... */
static u32 psyq_cd_view_len;
static u32 psyq_cd_cursor;     /* ... and how far CdGetSector has read it */

/* The stream's ring: frames in FIFO order (st_first, st_count over st_slots), each a contiguous slot of st_ring. */
enum { ST_ASSEMBLING, ST_READY, ST_TAKEN, ST_FREED };
typedef struct PsyqStSlot {
    u32 ofs, len;  /* in st_ring */
    u32 frame;     /* frameCount */
    u32 nsec;      /* nSectors */
    u64 got;       /* the secCounts received */
    int state;
} PsyqStSlot;
static u8 *st_ring;
static u32 st_ring_bytes;
static PsyqStSlot st_slots[ST_SLOTS];
static int st_first, st_count;
static u32 st_start_frame, st_end_frame = 0xFFFFFFFFu;
static u32 st_skip_frame;      /* realistic: the frame being dropped (no room), 0 none */
static int st_polls;           /* consecutive empty StGetNext polls */

/* The XA decoder and its FIFO of 44,100 Hz stereo frames (the header comment). */
enum { XA_IDLE, XA_LEAD, XA_PLAYING };
static XaDecoder psyq_xa;
static int16_t psyq_xa_fifo[PSYQ_XA_FIFO * 2]; /* interleaved left, right */
static u32 psyq_xa_head, psyq_xa_count;
static int psyq_xa_state;   /* XA_IDLE; XA_LEAD: a run's first sector is in, it plays from the next tick; XA_PLAYING */
static u32 psyq_xa_sectors; /* the run's audio sectors */
static u32 psyq_xa_dropped; /* the run's frames dropped (the FIFO full) */

void psyq_cd_set_reader(int (*read)(unsigned lba, u8 *sector)) {
    psyq_cd_reader = read;
}

void psyq_cd_set_timing(int timing) {
    psyq_cd_timing = timing;
}

void psyq_cd_set_vsync_hz(int hz) {
    psyq_cd_vsync_hz = hz;
}

static u32 psyq_le16(const u8 *p) {
    return (u32)p[0] | (u32)p[1] << 8;
}

static u32 psyq_le32(const u8 *p) {
    return (u32)p[0] | (u32)p[1] << 8 | (u32)p[2] << 16 | (u32)p[3] << 24;
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

/* ---- XA audio ---- */

/* Stops the decoder: the FIFO emptied, the decoder's state reset. */
static void psyq_xa_flush(const char *why) {
    if (psyq_xa_state != XA_IDLE || psyq_xa_count > 0) {
        PSYQ_TRACE("xa: flushed (%s) after %u sector(s): %u frame(s) unplayed, %u dropped", why, psyq_xa_sectors,
                   psyq_xa_count, psyq_xa_dropped);
    }
    xa_reset(&psyq_xa);
    psyq_xa_head = 0;
    psyq_xa_count = 0;
    psyq_xa_state = XA_IDLE;
    psyq_xa_sectors = 0;
    psyq_xa_dropped = 0;
}

/* The delivered sector, when it is XA audio the drive keeps from the CPU (the header comment): 1 = taken (decoded into
 * the FIFO, or dropped by the filter), 0 = a data sector. */
static int psyq_xa_take(u32 lba) {
    static int16_t out[2 * XA_MAX_OUT];
    const u8 *sub = psyq_cd_raw + 16;
    int filtered = (psyq_cd_mode & 0x08) && (sub[0] != psyq_cd_filter[0] || sub[1] != psyq_cd_filter[1]);
    int n, i;

    if (psyq_cd_raw[15] != 2 || (sub[2] & 0x44) != 0x44) {
        return 0;
    }
    if (!(psyq_cd_mode & 0x40) || filtered) {
        return (psyq_cd_mode & 0x08) != 0; /* the filter keeps audio sectors from the CPU too */
    }
    n = xa_decode_sector(&psyq_xa, psyq_cd_raw, out);
    if (psyq_xa_state == XA_IDLE) {
        PSYQ_TRACE("xa: sector %u (file %u, channel %u, coding %02x) starts a run, %d frames; plays from the next tick",
                   lba, sub[0], sub[1], sub[3], n);
        psyq_xa_state = XA_LEAD;
    }
    psyq_xa_sectors++;
    for (i = 0; i < n; i++) {
        u32 tail;

        if (psyq_xa_count == PSYQ_XA_FIFO) {
            if (psyq_xa_dropped == 0) {
                PSYQ_TRACE("xa: the FIFO is full at sector %u: frames dropped", lba);
            }
            psyq_xa_dropped += (u32)(n - i);
            break;
        }
        tail = (psyq_xa_head + psyq_xa_count) % PSYQ_XA_FIFO;
        psyq_xa_fifo[2 * tail] = out[2 * i];
        psyq_xa_fifo[2 * tail + 1] = out[2 * i + 1];
        psyq_xa_count++;
    }
    return 1;
}

/* The end of a CD tick: the next vsync's frames into the SPU's CD input (a run's first tick waits). */
static void psyq_xa_feed(void) {
    u32 n, chunk;

    if (psyq_xa_state == XA_LEAD) {
        psyq_xa_state = XA_PLAYING;
        return;
    }
    if (psyq_xa_state != XA_PLAYING) {
        return;
    }
    n = psyq_xa_count < PSYQ_XA_PER_TICK ? psyq_xa_count : PSYQ_XA_PER_TICK;
    while (n > 0) {
        chunk = PSYQ_XA_FIFO - psyq_xa_head < n ? PSYQ_XA_FIFO - psyq_xa_head : n;
        spu_cd_input(psyq_xa_fifo + 2 * psyq_xa_head, (int)chunk);
        psyq_xa_head = (psyq_xa_head + chunk) % PSYQ_XA_FIFO;
        psyq_xa_count -= chunk;
        n -= chunk;
    }
    if (psyq_xa_count == 0) {
        PSYQ_TRACE("xa: ran dry after %u sector(s), %u frame(s) dropped", psyq_xa_sectors, psyq_xa_dropped);
        psyq_xa_state = XA_IDLE;
        psyq_xa_sectors = 0;
        psyq_xa_dropped = 0;
    }
}

/* ---- commands ---- */

static void psyq_cd_set_result(u8 *result) {
    if (result != NULL) {
        memset(result, 0, 8);
        result[0] = psyq_cd_status;
    }
}

/* A read (or the stream) starts at the Setloc position, or at the head without a Setloc since the last one;
 * realistic timing seeks first. */
static void psyq_cd_start_read(int kind) {
    u32 target = psyq_cd_loc_new ? psyq_cd_loc : psyq_cd_head;
    u32 dist = target > psyq_cd_head ? target - psyq_cd_head : psyq_cd_head - target;

    psyq_cd_seek_ticks = 0;
    if (psyq_cd_timing == PSYQ_CD_REALISTIC && psyq_cd_loc_new) {
        psyq_cd_seek_ticks = PSYQ_CD_SEEK_BASE + (int)(dist / PSYQ_CD_SEEK_SPAN);
        if (psyq_cd_seek_ticks > PSYQ_CD_SEEK_MAX) {
            psyq_cd_seek_ticks = PSYQ_CD_SEEK_MAX;
        }
        psyq_cd_seek_ticks = psyq_cd_seek_ticks * psyq_cd_vsync_hz / 50; /* the same time at 60 Hz */
    }
    PSYQ_TRACE("cd: %s from sector %u (head %u, seek %d tick(s)), %s speed", kind == CD_STREAM ? "stream" : "read",
               target, psyq_cd_head, psyq_cd_seek_ticks, (psyq_cd_mode & 0x80) ? "double" : "single");
    psyq_cd_reading = kind;
    psyq_cd_next_lba = target;
    psyq_cd_head = target;
    psyq_cd_loc_new = 0;
    psyq_cd_rate_acc = 0;
    psyq_cd_have_sector = 0;
    psyq_xa_flush("a new read");
}

/* The effect of a command once the drive has taken it. */
static void psyq_cd_apply(int com, const u8 *param) {
    switch (com) {
    case CdlSetloc:
        if (param != NULL) {
            psyq_cd_loc = (u32)CdPosToInt((CdlLOC *)param);
            psyq_cd_loc_new = 1;
        }
        break;
    case CdlSetmode:
        if (param != NULL) {
            psyq_cd_mode = param[0];
        }
        break;
    case CdlReadN:
    case CdlReadS:
        psyq_cd_start_read(CD_READ);
        break;
    case CdlPause:
        if (psyq_cd_reading != CD_IDLE) {
            PSYQ_TRACE("cd: paused at sector %u", psyq_cd_next_lba);
        }
        psyq_cd_reading = CD_IDLE;
        psyq_xa_flush("Pause");
        break;
    case CdlSetfilter:
        if (param != NULL) {
            psyq_cd_filter[0] = param[0];
            psyq_cd_filter[1] = param[1];
        }
        break;
    default:
        break;
    }
}

/* The parameter bytes a command takes (the game's: a CdlLOC for Setloc, the mode byte for Setmode, none else; and
 * Setfilter's file and channel). */
static int psyq_cd_param_len(int com) {
    return com == CdlSetloc ? 4 : com == CdlSetmode ? 1 : com == CdlSetfilter ? 2 : 0;
}

/* One trace line for a command, with only the parameter bytes it takes (a Setmode's is one u8 on the caller's
 * stack: reading more would read past it). */
static void psyq_cd_trace_command(const char *who, int com, const u8 *param) {
    char text[16] = "";
    int i, n = param != NULL ? psyq_cd_param_len(com) : 0;

    for (i = 0; i < n; i++) {
        snprintf(text + i * 3, sizeof(text) - (size_t)i * 3, " %02x", param[i]);
    }
    PSYQ_TRACE("%s %02x%s%s", who, com, n > 0 ? " param" : "", text);
}

static int psyq_cd_blocking(const char *who, u8 com, u8 *param, u8 *result) {
    psyq_cd_trace_command(who, com, param);
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
    psyq_cd_trace_command("CdControlF", com, param);
    if (psyq_cd_pending != 0) {
        PSYQ_TRACE("CdControlF: command %02x replaces the pending %02x", com, psyq_cd_pending);
    }
    psyq_cd_pending = com;
    memset(psyq_cd_param, 0, sizeof(psyq_cd_param));
    if (param != NULL) {
        memcpy(psyq_cd_param, param, (size_t)psyq_cd_param_len(com));
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

/* ---- the stream's ring ---- */

static PsyqStSlot *st_slot(int i) {
    return &st_slots[(st_first + i) % ST_SLOTS];
}

static void st_reset(void) {
    st_first = 0;
    st_count = 0;
    st_skip_frame = 0;
    st_polls = 0;
}

/* Releases the freed frames at the head of the FIFO. */
static void st_pop_freed(void) {
    while (st_count > 0 && st_slot(0)->state == ST_FREED) {
        st_first = (st_first + 1) % ST_SLOTS;
        st_count--;
    }
}

/* A slot of `len` bytes after the newest frame (wrapping to the ring's start when the end has no room), or NULL. */
static PsyqStSlot *st_reserve(u32 len) {
    u32 ofs;
    PsyqStSlot *s;

    if (st_count == ST_SLOTS || len > st_ring_bytes) {
        return NULL;
    }
    if (st_count == 0) {
        ofs = 0;
    } else {
        PsyqStSlot *head = st_slot(0), *tail = st_slot(st_count - 1);
        u32 end = tail->ofs + tail->len;

        if (tail->ofs >= head->ofs) {
            /* used: [head, end) */
            if (end + len <= st_ring_bytes) {
                ofs = end;
            } else if (len <= head->ofs) {
                ofs = 0;
            } else {
                return NULL;
            }
        } else if (end + len <= head->ofs) {
            /* used: [head, ring end) and [0, end) */
            ofs = end;
        } else {
            return NULL;
        }
    }
    s = st_slot(st_count++);
    memset(s, 0, sizeof(*s));
    s->ofs = ofs;
    s->len = len;
    return s;
}

/* One sector of the stream. 1 = consumed; 0 = not taken (instant timing: no room for its frame, read it again). */
static int st_sector(u32 lba) {
    const u8 *d = psyq_cd_raw + 24; /* Form 1 data */
    u32 sc, ns, fc;
    PsyqStSlot *s = st_count > 0 ? st_slot(st_count - 1) : NULL;

    if ((psyq_cd_raw[18] & 0x04) || psyq_le32(d) != ST_MAGIC) {
        return 1; /* XA audio or not video */
    }
    sc = psyq_le16(d + 4);
    ns = psyq_le16(d + 6);
    fc = psyq_le32(d + 8);
    if (ns == 0 || ns > ST_MAX_SECTORS || sc >= ns || fc < st_start_frame || fc > st_end_frame || st_ring == NULL ||
        fc == st_skip_frame) {
        return 1;
    }
    if (s != NULL && s->state == ST_ASSEMBLING && (s->frame != fc || s->nsec != ns)) {
        PSYQ_TRACE("stream: frame %u dropped at sector %u: incomplete (%d of %u sectors)", s->frame, lba,
                   __builtin_popcountll(s->got), s->nsec);
        st_count--;
        s = NULL;
    }
    if (s == NULL || s->state != ST_ASSEMBLING) {
        s = st_reserve(ST_HEADER_SIZE + ns * ST_DATA_SIZE);
        if (s == NULL) {
            if (psyq_cd_timing == PSYQ_CD_INSTANT) {
                return 0;
            }
            PSYQ_TRACE("stream: frame %u dropped at sector %u: the ring is full", fc, lba);
            st_skip_frame = fc;
            return 1;
        }
        s->frame = fc;
        s->nsec = ns;
        s->state = ST_ASSEMBLING;
        st_skip_frame = 0;
    }
    if (sc == 0) {
        memcpy(st_ring + s->ofs, d, ST_HEADER_SIZE);
    }
    memcpy(st_ring + s->ofs + ST_HEADER_SIZE + sc * ST_DATA_SIZE, d + ST_HEADER_SIZE, ST_DATA_SIZE);
    s->got |= (u64)1 << sc;
    if (__builtin_popcountll(s->got) == (int)ns) {
        s->state = ST_READY;
        PSYQ_TRACE("stream: frame %u in (%u sectors to sector %u, ring offset %u)", fc, ns, lba, s->ofs);
    }
    return 1;
}

/* ---- the tick ---- */

/* This tick's sectors of a read or the stream, at most `n`. Stops at a command a handler issued. */
static int psyq_cd_deliver(int n, u8 *result) {
    u32 first = psyq_cd_next_lba;
    int ran = 0, i;

    for (i = 0; i < n && psyq_cd_reading != CD_IDLE && psyq_cd_pending == 0; i++) {
        u32 lba = psyq_cd_next_lba;

        if (!psyq_cd_fetch(lba)) {
            int was = psyq_cd_reading;

            PSYQ_TRACE("cd tick: no sector %u: data end%s", lba,
                       was == CD_READ && psyq_cd_ready_handler ? ", ready handler" : "");
            psyq_cd_reading = CD_IDLE; /* before the handler, which may start another read */
            if (was == CD_READ && psyq_cd_ready_handler != NULL) {
                psyq_cd_ready_handler(CdlDataEnd, result);
                ran = 1;
            }
            break;
        }
        if (psyq_xa_take(lba)) {
            psyq_cd_next_lba++; /* XA audio: played (or filtered out), never delivered */
            psyq_cd_head = psyq_cd_next_lba;
            continue;
        }
        if (psyq_cd_reading == CD_STREAM) {
            if (!st_sector(lba)) {
                break; /* instant: the ring has no room; the drive waits */
            }
            psyq_cd_next_lba++;
            psyq_cd_head = psyq_cd_next_lba;
        } else {
            psyq_cd_next_lba++;
            psyq_cd_head = psyq_cd_next_lba;
            if (psyq_cd_ready_handler != NULL) {
                psyq_cd_ready_handler(CdlDataReady, result);
                ran = 1;
            }
        }
    }
    if (psyq_cd_next_lba != first && psyq_cd_reading != CD_STREAM) {
        PSYQ_TRACE("cd tick: sectors %u..%u", first, psyq_cd_next_lba - 1);
    }
    return ran;
}

static int psyq_cd_tick_drive(void) {
    u8 result[8];
    int ran = 0, n;

    psyq_cd_set_result(result);
    psyq_cd_ack_tick = 0;
    if (psyq_cd_pending != 0) {
        int com = psyq_cd_pending;

        psyq_cd_pending = 0;
        psyq_cd_apply(com, psyq_cd_param);
        psyq_cd_ack_tick = com == CdlReadN || com == CdlReadS;
        PSYQ_TRACE("cd tick: command %02x complete%s", com, psyq_cd_sync_handler ? ", sync handler" : "");
        if (psyq_cd_sync_handler != NULL) {
            psyq_cd_sync_handler(CdlComplete, result);
            ran = 1;
        }
    }
    if (psyq_cd_reading == CD_IDLE) {
        return ran;
    }
    if (psyq_cd_timing == PSYQ_CD_INSTANT) {
        n = PSYQ_CD_INSTANT_SECTORS;
    } else if (psyq_cd_ack_tick) {
        n = 0;
    } else if (psyq_cd_seek_ticks > 0) {
        psyq_cd_seek_ticks--;
        n = 0;
    } else {
        psyq_cd_rate_acc += (psyq_cd_mode & 0x80) ? 150 : 75;
        n = psyq_cd_rate_acc / psyq_cd_vsync_hz;
        psyq_cd_rate_acc %= psyq_cd_vsync_hz;
    }
    if (n > 0 && psyq_cd_deliver(n, result)) {
        ran = 1;
    }
    return ran;
}

/* One vsync of the drive: the pending command, the sectors, then the XA audio for the next vsync's render. 1 = a
 * handler ran. */
int psyq_cd_tick(void) {
    int ran = psyq_cd_tick_drive();

    psyq_xa_feed();
    return ran;
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

/* 1 = ok. LIBCD's CdInit also sets the SPU up for CD audio (its CD_initvol; docs/SOUND.md section 1): the main volume
 * to 3FFFh when the current main volume reads 0, the CD volume to 3FFFh, SPUCNT C001h (on, CD audio in). The emulator
 * reads the current main volume as 0 there, so all five stores happen in its trace; the port's SPU core reads its
 * level (3FFFh x 2 after SsInit), so the port makes them unconditionally: the oracle's stores, and the same main
 * volume either way. */
int CdInit(void) {
    PSYQ_TRACE("CdInit");
    spu_write16(0x180, 0x3FFF);
    spu_write16(0x182, 0x3FFF);
    spu_write16(0x1B0, 0x3FFF);
    spu_write16(0x1B2, 0x3FFF);
    spu_write16(0x1AA, 0xC001);
    psyq_cd_pending = 0;
    psyq_cd_reading = CD_IDLE;
    psyq_cd_sync_handler = NULL;
    psyq_cd_ready_handler = NULL;
    psyq_xa_flush("CdInit");
    return 1;
}

/* The console's reset (psyq.c psyq_reset): the drive as at power-on (no command, no read, no stream, the head at
 * sector 0, mode 0, no sector in the buffer), no handlers, no ring, StCdIntrFlag clear. The disc stays in the drive:
 * the sector source (psyq_cd_set_reader) and the timing model (psyq_cd_set_timing) are kept. */
void psyq_cd_reset(void) {
    psyq_cd_sync_handler = NULL;
    psyq_cd_ready_handler = NULL;
    psyq_cd_pending = 0;
    memset(psyq_cd_param, 0, sizeof(psyq_cd_param));
    psyq_cd_reading = CD_IDLE;
    psyq_cd_loc = 0;
    psyq_cd_loc_new = 0;
    psyq_cd_head = 0;
    psyq_cd_next_lba = 0;
    psyq_cd_seek_ticks = 0;
    psyq_cd_rate_acc = 0;
    psyq_cd_ack_tick = 0;
    psyq_cd_mode = 0;
    psyq_cd_status = 0x02;
    memset(psyq_cd_raw, 0, sizeof(psyq_cd_raw));
    psyq_cd_have_sector = 0;
    psyq_cd_view_ofs = 0;
    psyq_cd_view_len = 0;
    psyq_cd_cursor = 0;
    st_ring = NULL;
    st_ring_bytes = 0;
    memset(st_slots, 0, sizeof(st_slots));
    st_start_frame = 0;
    st_end_frame = 0xFFFFFFFFu;
    st_reset();
    D_80081454 = 0;
    memset(psyq_cd_filter, 0, sizeof(psyq_cd_filter));
    memset(psyq_xa_fifo, 0, sizeof(psyq_xa_fifo));
    psyq_xa_flush("reset");
}

/* Returns the previous level (0). */
int CdSetDebug(int level) {
    PSYQ_TRACE("CdSetDebug %d", level);
    return 0;
}

/* ---- streaming ---- */

/* Sets the mode (CdlModeStream 0x100 is LIBCD's own bit; the low byte is the Setmode byte: 0x1E0 = double speed,
 * XA real-time, 2340-byte sectors) and starts streaming from the Setloc position into the ring. 1 = started. */
int CdRead2(long mode) {
    PSYQ_TRACE("CdRead2 %lx", mode);
    psyq_cd_mode = (u8)mode;
    psyq_cd_pending = 0;
    psyq_cd_start_read(CD_STREAM);
    return 1;
}

void StSetRing(u32 *ring_addr, u32 ring_size) {
    PSYQ_TRACE("StSetRing %u size %u sectors", PSYQ_PTR(ring_addr), ring_size);
    st_ring = (u8 *)ring_addr;
    st_ring_bytes = ring_size * 2048;
    st_reset();
}

void StSetStream(u32 mode, u32 start_frame, u32 end_frame, void (*func1)(), void (*func2)()) {
    PSYQ_TRACE("StSetStream mode %u frames %u..%u cb %u %u", mode, start_frame, end_frame, PSYQ_PTR(func1),
               PSYQ_PTR(func2));
    st_start_frame = start_frame;
    st_end_frame = end_frame;
    st_reset();
}

/* 0 = a frame: *addr its data, *header its first sector's StHEADER; 1 = no complete frame yet. Every
 * PSYQ_ST_POLLS_PER_VSYNC empty polls in a row run one vsync tick (the header comment). */
u32 StGetNext(u32 **addr, u32 **header) {
    int i;

    for (i = 0; i < st_count; i++) {
        PsyqStSlot *s = st_slot(i);

        if (s->state == ST_READY) {
            s->state = ST_TAKEN;
            *header = (u32 *)(st_ring + s->ofs);
            *addr = (u32 *)(st_ring + s->ofs + ST_HEADER_SIZE);
            st_polls = 0;
            PSYQ_TRACE("StGetNext: frame %u (ring offset %u)", s->frame, s->ofs);
            return 0;
        }
        if (s->state == ST_ASSEMBLING) {
            break;
        }
    }
    if (++st_polls >= PSYQ_ST_POLLS_PER_VSYNC) {
        st_polls = 0;
        psyq_vsync_tick();
    }
    return 1;
}

/* Releases the frame whose data is at `base`. 0 = ok, 1 = no such frame. */
u32 StFreeRing(u32 *base) {
    int i;

    for (i = 0; i < st_count; i++) {
        PsyqStSlot *s = st_slot(i);

        if ((s->state == ST_TAKEN || s->state == ST_READY) && (u8 *)base == st_ring + s->ofs + ST_HEADER_SIZE) {
            PSYQ_TRACE("StFreeRing: frame %u", s->frame);
            s->state = ST_FREED;
            st_pop_freed();
            return 0;
        }
    }
    PSYQ_TRACE("StFreeRing %u: no such frame", PSYQ_PTR(base));
    return 1;
}

/* Ends the stream and forgets the ring. */
void StUnSetRing(void) {
    PSYQ_TRACE("StUnSetRing");
    if (psyq_cd_reading == CD_STREAM) {
        psyq_cd_reading = CD_IDLE;
    }
    psyq_xa_flush("StUnSetRing");
    st_ring = NULL;
    st_ring_bytes = 0;
    st_reset();
}

/* The CD interrupt LIBCD deferred while MDEC ran (StCdIntrFlag): nothing is deferred here. */
void StCdInterrupt(void) {
    PSYQ_TRACE("StCdInterrupt");
}
