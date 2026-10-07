/* port/psyq/libpad.c: LIBPAD. One digital pad on port 0 (pad.c reads `buffers[port]`: byte 0 the status, 0 = ok,
 * 0xFF = nothing connected; byte 1 the id, 0x41 = digital pad, 0x80 = multitap; bytes 2-3 the buttons, active
 * low). The port's input script sets the buttons with psyq_pad_set; every vsync tick makes them "new data" for
 * PadChkVsync.
 *
 * Assumption to verify (M3, against the emulator): the id/mode numbers (PadInfoMode term 2 = the current mode id,
 * 4 for a digital pad, 7 for a DualShock in analog mode; term 3 = the id table, 0 here), PadGetState 6 = stable,
 * and that a digital pad reports no actuators (PadInfoAct -1 -> 0). */
#include "psyq_internal.h"
#include "psyq/libpad.h"

#define PAD_STATE_STABLE 6
#define PAD_ID_DIGITAL 0x41
#define PAD_MODE_DIGITAL 4

static u8 *psyq_pad_buffer[2];
static int psyq_pad_connected[2] = { 1, 0 };
static u16 psyq_pad_buttons[2];
static int psyq_pad_started;
static int psyq_pad_fresh; /* data arrived since the previous PadChkVsync */

static void psyq_pad_fill(int port) {
    u8 *buf = psyq_pad_buffer[port];

    if (buf == NULL) {
        return;
    }
    if (!psyq_pad_connected[port]) {
        buf[0] = 0xFF;
        buf[1] = 0;
        buf[2] = 0xFF;
        buf[3] = 0xFF;
        return;
    }
    buf[0] = 0;
    buf[1] = PAD_ID_DIGITAL;
    buf[2] = (u8)~psyq_pad_buttons[port];
    buf[3] = (u8)(~psyq_pad_buttons[port] >> 8);
}

/* The script's buttons are latched here and reach the game's buffers at the next vsync's poll (psyq_pad_vsync), as
 * on the console: LIBPAD reads the controllers in the vsync interrupt, so buttons set at a vsync (the emulator's
 * replay runner applies its pad override in its vsync listener, after that read) are what the game sees one frame
 * later. Filling the buffers at once made the port react a frame earlier than the emulator, which moved every
 * `walk` of a layer-2 script by a few pixels. Before PadStartCom the buffers are filled at once (nothing polls). */
void psyq_pad_set(int port, int connected, u16 buttons) {
    if (port < 0 || port > 1) {
        return;
    }
    psyq_pad_connected[port] = connected;
    psyq_pad_buttons[port] = buttons;
    if (!psyq_pad_started) {
        psyq_pad_fill(port);
    }
}

u16 psyq_pad_get(int port) {
    return (port == 0 || port == 1) ? psyq_pad_buttons[port] : 0;
}

/* The console's reset (psyq.c psyq_reset): no buffers, not started, the default pads (port 0 connected with nothing
 * pressed, port 1 empty). */
void psyq_pad_reset(void) {
    psyq_pad_buffer[0] = NULL;
    psyq_pad_buffer[1] = NULL;
    psyq_pad_connected[0] = 1;
    psyq_pad_connected[1] = 0;
    psyq_pad_buttons[0] = 0;
    psyq_pad_buttons[1] = 0;
    psyq_pad_started = 0;
    psyq_pad_fresh = 0;
}

void PadInitDirect(u8 *buf0, u8 *buf1) {
    PSYQ_TRACE("PadInitDirect %u %u", PSYQ_PTR(buf0), PSYQ_PTR(buf1));
    psyq_pad_buffer[0] = buf0;
    psyq_pad_buffer[1] = buf1;
    psyq_pad_fill(0);
    psyq_pad_fill(1);
}

/* The multitap mode: the game only asks for it with pad_init(multitap != 0), which nothing does; the pads answer
 * as plain pads (buffer byte 1 is not 0x80), which pad.c handles. */
void PadInitMtap(u8 *buf0, u8 *buf1) {
    PSYQ_TRACE("PadInitMtap %u %u", PSYQ_PTR(buf0), PSYQ_PTR(buf1));
    PadInitDirect(buf0, buf1);
}

int PadStartCom(void) {
    PSYQ_TRACE("PadStartCom");
    psyq_pad_started = 1;
    psyq_pad_fresh = 1;
    return 0;
}

void PadStopCom(void) {
    PSYQ_TRACE("PadStopCom");
    psyq_pad_started = 0;
}

/* 1 when the buffers hold data not yet seen (every vsync while started), else 0. */
int PadChkVsync(void) {
    int fresh = psyq_pad_started && psyq_pad_fresh;

    psyq_pad_fresh = 0;
    return fresh;
}

/* Called from the vsync tick (libetc.c): the controllers were polled again. */
void psyq_pad_vsync(void) {
    if (psyq_pad_started) {
        psyq_pad_fill(0);
        psyq_pad_fill(1);
        psyq_pad_fresh = 1;
    }
}

/* 6 = stable (a controller there and identified), 0 = nothing connected. */
int PadGetState(int port) {
    int p = (port >> 4) & 1;

    return psyq_pad_connected[p] ? PAD_STATE_STABLE : 0;
}

int PadInfoMode(int port, int term, int offs) {
    int p = (port >> 4) & 1;

    (void)offs;
    if (!psyq_pad_connected[p]) {
        return 0;
    }
    switch (term) {
    case 2: /* the current mode id */
        return PAD_MODE_DIGITAL;
    default: /* 1 the extended id, 3 the id table, 4 the table offset: nothing a digital pad answers */
        return 0;
    }
}

/* A digital pad has no actuators: 0 of them (actno -1), 0 for each. */
int PadInfoAct(int port, int actno, int term) {
    (void)port;
    (void)actno;
    (void)term;
    return 0;
}

void PadSetAct(int port, u8 *data, int len) {
    PSYQ_TRACE("PadSetAct %x %02x %02x len %d", port, data ? data[0] : 0, data ? data[1] : 0, len);
}

/* 0 = refused (no actuators to align). */
int PadSetActAlign(int port, u8 *data) {
    PSYQ_TRACE("PadSetActAlign %x", port);
    (void)data;
    return 0;
}

/* 0 = refused (a digital pad has one mode). */
int PadSetMainMode(int socket, int offs, int lock) {
    PSYQ_TRACE("PadSetMainMode %x offs %d lock %d", socket, offs, lock);
    return 0;
}
