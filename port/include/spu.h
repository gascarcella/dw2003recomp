/* The SPU core's interface (M3; docs/SOUND.md section 6): port/runtime/spu.c implements it (T-spu), LIBSND
 * (port/psyq/libsnd*.c, T-libsnd) and LIBCD's CdInit drive it, the audio output (SDL3) and the tests read it.
 * Registers are addressed by their offset from 0x1F801C00 (0x000..0x1FF), 16 bits wide, as on the PS1. */
#ifndef PORT_SPU_H
#define PORT_SPU_H

#include <stdint.h>

#define SPU_RAM_SIZE 0x80000 /* 512 KB */
#define SPU_RATE 44100       /* output samples per second */

/* Power-on state (spu_init once; spu_reset at the console's reset: registers, voices and SPU RAM cleared). */
void spu_init(void);
void spu_reset(void);

/* A 16-bit register write or read at `offset` (0x000..0x1FF). A write goes to the write hook first (the trace). */
void spu_write16(uint32_t offset, uint16_t value);
uint16_t spu_read16(uint32_t offset);

/* A DMA4 block (RAM -> SPU): `halfwords` 16-bit words written into SPU RAM from the transfer address register (0x1A6,
 * in units of 8 bytes), as the SPU's transfer mode does; the hook sees it as one block. */
void spu_dma_write(const uint16_t *data, uint32_t halfwords);

/* Renders `frames` stereo frames (interleaved left, right; signed 16-bit) at SPU_RATE, advancing the voices. */
void spu_render(int16_t *out, int frames);

/* CD audio into the SPU's CD input (M5: XA from the movies): `frames` stereo frames at SPU_RATE, mixed by the CD volume. */
void spu_cd_input(const int16_t *samples, int frames);

/* The trace hook (port/runtime/spu_trace.c): every register write (dma = NULL) and every DMA block (offset = the transfer
 * address in bytes, data/halfwords the block). NULL: none. */
typedef void (*SpuWriteHook)(uint32_t offset, uint16_t value, const uint16_t *dma, uint32_t halfwords);
void spu_set_write_hook(SpuWriteHook hook);

#endif /* PORT_SPU_H */
