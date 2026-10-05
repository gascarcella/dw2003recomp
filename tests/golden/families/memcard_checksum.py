"""memcard_get_checksum / memcard_check_checksum (src/main/memcard.c): the save slot's XOR checksum.

STGMCARD checksums bytes 4..0x26C3 of a slot (0x26C0 bytes) into gamestate_data.checksum (docs/FORMATS.md "Save data";
docs/MECHANICS.md section 12). memcard_check_checksum(data, size, sum) is the lenient load check.
"""
from oracle import Call, Case

COMMENT = ("memcard_get_checksum(data, size) over fixed byte patterns, and memcard_check_checksum(data, size, sum) with the "
           "right sum, a wrong one, 0 and 0xFF.")
SLOT_SIZE = 0x26C0


def lcg(n, seed=0x1234567):
    out = bytearray()
    x = seed
    for _ in range(n):
        x = (x * 1103515245 + 12345) & 0xFFFFFFFF
        out.append((x >> 16) & 0xFF)
    return bytes(out)


PATTERNS = {
    "empty": b"",
    "one_byte_5a": bytes([0x5A]),
    "two_bytes": bytes([0x5A, 0xA5]),
    "zeros_16": bytes(16),
    "ff_16": bytes([0xFF] * 16),
    "inc_256": bytes(range(256)),
    "inc_255": bytes(range(255)),
    "lcg_1000": lcg(1000),
    "lcg_slot": lcg(SLOT_SIZE),
}


def cases(sym):
    out = []
    for name, data in PATTERNS.items():
        n = len(data)
        xor = 0
        for b in data:
            xor ^= b
        calls = [Call("memcard_get_checksum", [("buf", "data"), n], "u8", comment="XOR of the bytes")]
        for sum_, label in ((xor, "right sum"), (xor ^ 0x01, "one bit off"), (0, "zero"), (0xFF, "0xFF")):
            calls.append(Call("memcard_check_checksum", [("buf", "data"), n, sum_], "s32", comment=label))
        out.append(Case(name, calls, buffers={"data": data}, comment=f"{n} bytes; Python's XOR is {xor:#x}"))
    return out
