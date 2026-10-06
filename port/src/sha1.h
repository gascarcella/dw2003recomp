/* SHA-1 (FIPS 180-4), our own implementation: the disc check (disc.c) and the checkpoint hashes (framelog.c). */
#ifndef PORT_SHA1_H
#define PORT_SHA1_H

#include <stddef.h>
#include <stdint.h>

typedef struct PortSha1 {
    uint32_t h[5];
    uint64_t length; /* bytes so far */
    uint8_t block[64];
    size_t used; /* bytes in block */
} PortSha1;

void port_sha1_init(PortSha1 *c);
void port_sha1_update(PortSha1 *c, const void *data, size_t n);
void port_sha1_final(PortSha1 *c, uint8_t digest[20]);
/* digest -> 40 lowercase hex digits and a NUL */
void port_sha1_hex(const uint8_t digest[20], char hex[41]);

#endif /* PORT_SHA1_H */
