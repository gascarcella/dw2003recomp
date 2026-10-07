"""PPM (P6) -> PNG with the standard library only (zlib), for the debug channel's screenshots."""

from __future__ import annotations

import struct
import zlib


def parse_ppm(data: bytes) -> tuple[int, int, bytes]:
    """A binary PPM (P6, maxval 255) -> (width, height, rgb bytes)."""
    pos = 0
    fields = []

    def token():
        nonlocal pos
        while True:
            while pos < len(data) and data[pos] in b" \t\r\n":
                pos += 1
            if pos < len(data) and data[pos] == ord("#"):
                while pos < len(data) and data[pos] not in b"\r\n":
                    pos += 1
                continue
            break
        start = pos
        while pos < len(data) and data[pos] not in b" \t\r\n":
            pos += 1
        return data[start:pos]

    while len(fields) < 4:
        t = token()
        if not t:
            raise ValueError("truncated PPM header")
        fields.append(t)
    if fields[0] != b"P6":
        raise ValueError(f"not a binary PPM (magic {fields[0]!r})")
    w, h, maxval = int(fields[1]), int(fields[2]), int(fields[3])
    if maxval != 255:
        raise ValueError(f"PPM maxval {maxval} unsupported (255 only)")
    pos += 1  # the single whitespace after maxval
    need = w * h * 3
    pixels = data[pos:pos + need]
    if len(pixels) != need:
        raise ValueError(f"truncated PPM: {len(pixels)} of {need} pixel bytes")
    return w, h, pixels


def _chunk(kind: bytes, body: bytes) -> bytes:
    return struct.pack(">I", len(body)) + kind + body + struct.pack(">I", zlib.crc32(kind + body) & 0xFFFFFFFF)


def rgb_to_png(w: int, h: int, pixels: bytes, level: int = 6) -> bytes:
    """8-bit RGB rows -> PNG bytes (filter 0 on every row)."""
    stride = w * 3
    raw = b"".join(b"\x00" + pixels[y * stride:(y + 1) * stride] for y in range(h))
    ihdr = struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)
    return (b"\x89PNG\r\n\x1a\n" + _chunk(b"IHDR", ihdr) + _chunk(b"IDAT", zlib.compress(raw, level))
            + _chunk(b"IEND", b""))


def ppm_to_png(data: bytes) -> bytes:
    w, h, pixels = parse_ppm(data)
    return rgb_to_png(w, h, pixels)


if __name__ == "__main__":
    import sys
    src, dst = sys.argv[1], sys.argv[2]
    with open(src, "rb") as f:
        png = ppm_to_png(f.read())
    with open(dst, "wb") as f:
        f.write(png)
