/* SHA-1 (FIPS 180-4), written for the port (MIT, like the repo). */
#include <string.h>

#include "sha1.h"

#define ROL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static void port_sha1_block(PortSha1 *c, const uint8_t *p) {
    uint32_t w[80], a, b, d, e, f, k, t, cc;
    int i;
    for (i = 0; i < 16; i++) {
        w[i] = (uint32_t)p[4 * i] << 24 | (uint32_t)p[4 * i + 1] << 16 | (uint32_t)p[4 * i + 2] << 8 | p[4 * i + 3];
    }
    for (; i < 80; i++) {
        w[i] = ROL(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }
    a = c->h[0];
    b = c->h[1];
    cc = c->h[2];
    d = c->h[3];
    e = c->h[4];
    for (i = 0; i < 80; i++) {
        if (i < 20) {
            f = (b & cc) | (~b & d);
            k = 0x5A827999u;
        } else if (i < 40) {
            f = b ^ cc ^ d;
            k = 0x6ED9EBA1u;
        } else if (i < 60) {
            f = (b & cc) | (b & d) | (cc & d);
            k = 0x8F1BBCDCu;
        } else {
            f = b ^ cc ^ d;
            k = 0xCA62C1D6u;
        }
        t = ROL(a, 5) + f + e + k + w[i];
        e = d;
        d = cc;
        cc = ROL(b, 30);
        b = a;
        a = t;
    }
    c->h[0] += a;
    c->h[1] += b;
    c->h[2] += cc;
    c->h[3] += d;
    c->h[4] += e;
}

void port_sha1_init(PortSha1 *c) {
    static const uint32_t h0[5] = { 0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u };
    memcpy(c->h, h0, sizeof(h0));
    c->length = 0;
    c->used = 0;
}

void port_sha1_update(PortSha1 *c, const void *data, size_t n) {
    const uint8_t *p = data;
    c->length += n;
    if (c->used > 0) {
        size_t take = 64 - c->used < n ? 64 - c->used : n;
        memcpy(c->block + c->used, p, take);
        c->used += take;
        p += take;
        n -= take;
        if (c->used < 64) {
            return;
        }
        port_sha1_block(c, c->block);
        c->used = 0;
    }
    for (; n >= 64; p += 64, n -= 64) {
        port_sha1_block(c, p);
    }
    memcpy(c->block, p, n);
    c->used = n;
}

void port_sha1_final(PortSha1 *c, uint8_t digest[20]) {
    uint64_t bits = c->length * 8;
    uint8_t pad[72];
    size_t padlen = (c->used < 56 ? 56 : 120) - c->used;
    int i;
    memset(pad, 0, sizeof(pad));
    pad[0] = 0x80;
    for (i = 0; i < 8; i++) {
        pad[padlen + i] = (uint8_t)(bits >> (56 - 8 * i));
    }
    port_sha1_update(c, pad, padlen + 8);
    for (i = 0; i < 20; i++) {
        digest[i] = (uint8_t)(c->h[i / 4] >> (24 - 8 * (i % 4)));
    }
}

void port_sha1_hex(const uint8_t digest[20], char hex[41]) {
    static const char digits[] = "0123456789abcdef";
    int i;
    for (i = 0; i < 20; i++) {
        hex[2 * i] = digits[digest[i] >> 4];
        hex[2 * i + 1] = digits[digest[i] & 0xF];
    }
    hex[40] = '\0';
}
