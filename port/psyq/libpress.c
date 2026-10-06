/* port/psyq/libpress.c: LIBPRESS (the MDEC), linked into STDWTITL's movie player only. The decoding is mdec.c's: the
 * bit stream to run-level words on the CPU (DecDCTvlc2), and the MDEC (DecDCTin hands it the words, DecDCTout takes
 * `size` words of pixels). On the PS1 DecDCTout starts a DMA and the DecDCToutCallback handler runs at its end; here
 * the words are written at once and the handler runs before DecDCTout returns: STDWTITL's stdwtitl_wait_decode spins
 * on a flag only that handler sets (0x800000 iterations otherwise), and the handler calls DecDCTout again for the next
 * 16-pixel column of the frame (20 for the 320-pixel movies), which DecDCTout runs after the handler returns (a loop,
 * no recursion). The behaviours below were checked against the PS1 (tests/golden/families/mdec.py), and the opening
 * movie's VRAM against the emulator's at the same movie frames (port/psyq/README.md "LIBPRESS"). */
#include "psyq_internal.h"
#include "psyq/libpress.h"

static void (*psyq_press_out_handler)(void);
static int psyq_press_out_in_handler; /* the handler is running */
static int psyq_press_out_pending;    /* a DecDCTout to run when it returns */
static u32 *psyq_press_out_buf;
static u32 psyq_press_out_size;

/* The console's reset (psyq.c psyq_reset): the MDEC's default tables, no DecDCTout handler. */
void psyq_press_reset(void) {
    psyq_press_out_handler = NULL;
    psyq_press_out_in_handler = 0;
    psyq_press_out_pending = 0;
    psyq_press_out_buf = NULL;
    psyq_press_out_size = 0;
    mdec_reset();
}

/* 0: resets the MDEC and loads LIBPRESS's quantisation and IDCT tables; else only ends the current decode. */
void DecDCTReset(int mode) {
    PSYQ_TRACE("DecDCTReset %d", mode);
    if (mode == 0) {
        mdec_reset();
    } else {
        mdec_decode_start(NULL);
    }
}

/* Starts decoding the run-level words at buf (DecDCTvlc2's output: the command word 0x3800xxxx, then xxxx words).
 * The mode is written into the command word as the PS1 does: bit 0 = 24-bit output (clears the command's bit 27),
 * bit 1 = bit 15 set in 15-bit pixels (sets bit 25). STDWTITL passes 3. */
void DecDCTin(u32 *buf, int mode) {
    PSYQ_TRACE("DecDCTin %u mode %d", PSYQ_PTR(buf), mode);
    if (mode & 1) {
        buf[0] &= ~(1u << 27);
    }
    if (mode & 2) {
        buf[0] |= 1u << 25;
    }
    mdec_decode_start(buf);
}

/* Writes `size` words of pixels (whole 16x16 macroblocks, in decode order) to buf, then runs the handler, as the end
 * of the PS1's DMA would. A DecDCTout the handler makes (the next column) is not decoded inside the handler but after
 * it returns: on the PS1 the handler finishes (STDWTITL's uploads the column just decoded with LoadImage) while the
 * MDEC works on the next column; decoding at once would overwrite the image buffer the handler is about to upload
 * (the player's two buffers alternate). So a frame's columns run as a loop here, not as a recursion. */
void DecDCTout(u32 *buf, int size) {
    PSYQ_TRACE("DecDCTout %u words %d", PSYQ_PTR(buf), size);
    psyq_press_out_buf = buf;
    psyq_press_out_size = size > 0 ? (u32)size : 0;
    psyq_press_out_pending = 1;
    if (psyq_press_out_in_handler) {
        return;
    }
    while (psyq_press_out_pending) {
        u32 done;

        psyq_press_out_pending = 0;
        done = mdec_decode_out(psyq_press_out_buf, psyq_press_out_size);
        if (done < psyq_press_out_size) {
            PSYQ_TRACE("DecDCTout: the run-level words ended after %u words", done);
        }
        if (psyq_press_out_handler != NULL) {
            psyq_press_out_in_handler = 1;
            psyq_press_out_handler();
            psyq_press_out_in_handler = 0;
        }
    }
}

void DecDCToutCallback(void (*func)()) {
    PSYQ_TRACE("DecDCToutCallback %u", PSYQ_PTR(func));
    psyq_press_out_handler = (void (*)(void))func;
}

/* Expands the frame's bit stream `bs` (a .STR version 2 frame: header and stream) into the MDEC run-level words at
 * buf. Returns 0 (the whole frame decoded). The table is not read: the decoder has its own (mdec.c). */
int DecDCTvlc2(u32 *bs, u32 *buf, u16 *table) {
    PSYQ_TRACE("DecDCTvlc2 %u -> %u table %u", PSYQ_PTR(bs), PSYQ_PTR(buf), PSYQ_PTR(table));
    return mdec_vlc_decode((const u8 *)bs, buf);
}

/* On the PS1, fills the 0x11000-byte lookup table STDWTITL allocates for DecDCTvlc2. mdec.c's decoder builds its own,
 * so the game's table is left as it is (nothing reads it). */
void DecDCTvlcBuild(u16 *table) {
    PSYQ_TRACE("DecDCTvlcBuild %u", PSYQ_PTR(table));
}
