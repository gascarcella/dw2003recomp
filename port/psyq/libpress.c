/* port/psyq/libpress.c: LIBPRESS (the MDEC), linked into STDWTITL's movie player only. No decoder in the M1
 * skeleton (DECISIONS: our own MDEC + XA decoder, not before M5): nothing is decoded and no output is written.
 * DecDCTout runs the DecDCToutCallback handler at once, as the MDEC would at the end of the transfer: STDWTITL's
 * stdwtitl_wait_decode spins on a flag that only that handler sets (0x800000 iterations otherwise), and the handler
 * calls DecDCTout again for the next slice of the frame, so the recursion is as deep as one frame has slices (none
 * without a stream: the frame is 0 wide). */
#include "psyq_internal.h"
#include "psyq/libpress.h"

static void (*psyq_press_out_handler)(void);

void DecDCTReset(int mode) {
    PSYQ_TRACE("DecDCTReset %d", mode);
}

void DecDCTin(u32 *buf, int mode) {
    PSYQ_TRACE("DecDCTin %u mode %d", PSYQ_PTR(buf), mode);
}

/* Records the request and reports it finished to the handler (the output buffer is not written). */
void DecDCTout(u32 *buf, int size) {
    PSYQ_TRACE("DecDCTout %u words %d", PSYQ_PTR(buf), size);
    if (psyq_press_out_handler != NULL) {
        psyq_press_out_handler();
    }
}

void DecDCToutCallback(void (*func)()) {
    PSYQ_TRACE("DecDCToutCallback %u", PSYQ_PTR(func));
    psyq_press_out_handler = (void (*)(void))func;
}

/* Would expand the frame's bit stream `bs` into the MDEC run-level words at `buf`. 0 = ok; nothing is written. */
int DecDCTvlc2(u32 *bs, u32 *buf, u16 *table) {
    PSYQ_TRACE("DecDCTvlc2 %u -> %u table %u", PSYQ_PTR(bs), PSYQ_PTR(buf), PSYQ_PTR(table));
    return 0;
}

/* Would fill the 0x11000-byte VLC lookup table STDWTITL allocates; the stub decoder never reads it. */
void DecDCTvlcBuild(u16 *table) {
    PSYQ_TRACE("DecDCTvlcBuild %u", PSYQ_PTR(table));
}
