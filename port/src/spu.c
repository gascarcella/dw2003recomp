/* The SPU core (port/include/spu.h). M3 step-0 stub: T-spu owns this file and replaces it. */
#include <string.h>

#include "spu.h"

static SpuWriteHook spu_hook;

void spu_init(void) {
}

void spu_reset(void) {
}

void spu_write16(uint32_t offset, uint16_t value) {
    if (spu_hook != NULL) {
        spu_hook(offset, value, NULL, 0);
    }
}

uint16_t spu_read16(uint32_t offset) {
    (void)offset;
    return 0;
}

void spu_dma_write(const uint16_t *data, uint32_t halfwords) {
    if (spu_hook != NULL) {
        spu_hook(0, 0, data, halfwords);
    }
}

void spu_render(int16_t *out, int frames) {
    memset(out, 0, (size_t)frames * 2 * sizeof(*out));
}

void spu_cd_input(const int16_t *samples, int frames) {
    (void)samples;
    (void)frames;
}

void spu_set_write_hook(SpuWriteHook hook) {
    spu_hook = hook;
}
