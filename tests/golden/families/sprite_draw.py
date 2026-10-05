"""sprite_draw (src/main/sprite.c): the EXE's 2D sprite renderer, on its untransformed path (scale 0x1000, no rotation:
one SPRT per cell; the scaled/rotated path goes through the GTE library, RotMatrixYXZ_gte/ScaleMatrix/ApplyMatrixSV,
which the port replaces, and is not a case).

The bank: words {cells offset, frame IDs offset, frame offsets...}; the frame IDs are bytes, searched from the first
for `id` (no end: an ID that is not there reads past the list), and frame i is at bank[i + 2] (the frame's position in
the ID list, not the ID). A frame is s16 {count, CLUT row, blend mode (-1: opaque; 0-3: semi-transparent with that
mode)} and `count` s16 {cell, x, y}. Each cell (SpriteCell) is drawn last to first as a SPRT: colour sprite_current.color,
code 0x64 (| 2 semi-transparent), x0 = frame x + x, y0 = frame y + y (follow_scroll 0 here: no layer scroll), u0 = cell u
(& 0x7F for an 8-bit cell, else truncated to u8), v0, w, h, clut = getClut(base x + cell clut_x, base y + cell clut_y +
frame CLUT row + palette) with the 4-bit base (clut_x, clut_y) or the 8-bit base (clut8_x, clut8_y). The cell's texture
page is getTPage(8-bit, mode, vram_x + u / 4 (4-bit) or u / 2 (8-bit), vram_y); when it differs from the page of the cell
drawn before, a DR_TPAGE of the previous page goes in first; a DR_TPAGE of the last cell's page ends the frame (the
ordering table runs backwards: the page command is executed before the cells that follow it in the packet buffer).
DR_TPAGE = SetDrawTPage(dfe 0, dtd 1, page): 0xE1000000 | 0x200 | page.

The packets go to a scratch `packet` buffer (gfx_module.packet points at it; filled with 0xEE, so the bytes after the
last primitive show where it stopped) and are linked into a scratch ordering-table word `ot`. The reads skip each
primitive's 24-bit link (an address: the host's differs) and keep its length byte.

Hand check (one_cell_opaque): cell u 8, v 16, 32x24, CLUT +16/+1 on base (0, 480); vram (640, 256); drawn at (100, 50)
with offset (10, 20): SPRT 80 80 80 64, x0 110, y0 70, u0 8, v0 16, clut (481 << 6) | 1 = 0x7841, w 32, h 24; then
DR_TPAGE 0xE100021A (page x 642 >> 6 = 10, y 256 -> 0x10).
"""
import struct

from oracle import SCRATCH_BASE, Call, Case, Read, Write

COMMENT = ("sprite_draw without scale or rotation into a scratch packet buffer: one opaque 4-bit cell (the hand check); "
           "the frame search (ID 2 at position 2 of the list) with three cells drawn last to first, negative offsets, the "
           "frame's CLUT row and the palette, blend mode 0; a frame whose cells change texture page (4-bit u 0 and 256, "
           "8-bit u 200: u & 0x7F, the 8-bit CLUT base) with a DR_TPAGE between them; blend mode 2.")

SPRT, DR_TPAGE = 0x14, 0x8
SPRITE_SIZE = 0x70                    # Sprite up to its function table (sprite_draw reads nothing past .matrix)
PACKET_SIZE = 0x80


def cell(u, v, w, h, clut_x, clut_y, is_8bit=0):
    return struct.pack("<hBBhhhhh", u, v, 0, w, h, clut_x, clut_y, is_8bit)


def bank(cells, ids, frames):
    """cells: [SpriteCell bytes]; ids: frame IDs; frames: [(clut_row, mode, [(cell, x, y), ...])]."""
    header = 4 * (2 + len(frames))
    cells_b = b"".join(cells)
    ids_b = bytes(ids) + bytes((-len(ids)) % 4)
    frames_b, offs = b"", []
    base = header + len(cells_b) + (-len(cells_b)) % 4 + len(ids_b)
    for clut_row, mode, parts in frames:
        offs.append(base + len(frames_b))
        frames_b += struct.pack("<3h", len(parts), clut_row, mode) + b"".join(struct.pack("<3h", *p) for p in parts)
        frames_b += bytes((-len(frames_b)) % 4)
    cells_off = header
    ids_off = header + len(cells_b) + (-len(cells_b)) % 4
    return (struct.pack(f"<{2 + len(frames)}I", cells_off, ids_off, *offs) + cells_b + bytes((-len(cells_b)) % 4)
            + ids_b + frames_b)


def sprite(ot_addr, vram=(640, 256), clut=(0, 480), clut8=(256, 490), palette=0, color=(0x80, 0x80, 0x80)):
    """A Sprite up to .matrix: layer 0, ot_entry -> `ot`, follow_scroll 0, scale 0x1000 (x3), rotation 0."""
    b = bytearray(SPRITE_SIZE)
    struct.pack_into("<II7ii", b, 0, 0, ot_addr, vram[0], vram[1], clut[0], clut[1], clut8[0], clut8[1], palette, 0)
    struct.pack_into("<4B", b, 0x28, *color, 0)
    struct.pack_into("<3i", b, 0x38, 0x1000, 0x1000, 0x1000)
    return bytes(b)


def draw_case(name, bank_bytes, frame_id, x, y, prims, comment, **sprite_kw):
    """prims: the primitive kinds in packet order (SPRT, DR_TPAGE), for the reads."""
    sizes = {"sprite": SPRITE_SIZE, "ot": 4, "bank": len(bank_bytes)}
    places, addr = {}, SCRATCH_BASE
    for n in ("sprite", "ot", "bank", "packet"):
        places[n] = addr
        addr += (sizes.get(n, PACKET_SIZE) + 15) & ~15
    buffers = {"sprite": sprite(places["ot"], **sprite_kw), "ot": struct.pack("<I", 0x00FFFFFF), "bank": bank_bytes,
               "packet": b"\xEE" * PACKET_SIZE}
    fixture = [Write("sprite_current", 0, struct.pack("<I", places["sprite"]), "sprite_current = the `sprite` buffer"),
               Write("gfx_module", 0x20, struct.pack("<I", places["packet"]), "gfx_module.packet = the `packet` buffer")]
    reads, off = [], 0
    for k, size in enumerate(prims):
        kind = "SPRT" if size == SPRT else "DR_TPAGE"
        reads.append(Read("buf:packet", off + 3, size - 3, f"primitive {k}: {kind} (length byte, then its words)"))
        off += size
    reads.append(Read("buf:packet", off, 8, "the 8 bytes after the last primitive (0xEE: nothing more written)"))
    return Case(name, [Call("sprite_draw", [("buf", "bank"), frame_id, x, y], "void", reads, comment=comment)],
                fixture=fixture, buffers=buffers, saves=[("sprite_current", 4), ("gfx_module", 0x24)], comment=comment)


def cases(sym):
    c_small = cell(8, 16, 32, 24, 16, 1)
    one = bank([c_small], [0], [(0, -1, [(0, 10, 20)])])
    blend2 = bank([c_small], [0], [(0, 2, [(0, 10, 20)])])
    three = bank([c_small, cell(40, 0, 16, 16, 32, 0), cell(252, 200, 8, 8, 0, 3)], [5, 9, 2],
                 [(0, -1, [(0, 0, 0)]), (1, -1, [(1, 4, 4)]),
                  (2, 0, [(0, -16, -8), (1, 0, 0), (2, 30, -40)])])
    pages = bank([cell(0, 0, 16, 16, 0, 0), cell(256, 32, 16, 16, 16, 0), cell(200, 64, 24, 24, 32, 2, 1)], [7],
                 [(0, -1, [(0, 0, 0), (1, 16, 0), (2, 32, 0)])])
    return [
        draw_case("one_cell_opaque", one, 0, 100, 50, [SPRT, DR_TPAGE],
                  "one 4-bit cell, opaque: SPRT 0x64 at (110, 70), clut 0x7841, then DR_TPAGE 0xE100021A"),
        draw_case("search_third_frame", three, 2, 50, 60, [SPRT, SPRT, SPRT, DR_TPAGE],
                  "ID 2 is third in the list: frame 2's three cells, last first (u 252, 40, 8: one page), offsets -16/-8 "
                  "and 30/-40, CLUT row 2 + palette 3 + cell rows, blend mode 0 (semi-transparent, code 0x66)",
                  palette=3, color=(0x40, 0x60, 0x80)),
        draw_case("page_change", pages, 7, 0, 0, [SPRT, DR_TPAGE, SPRT, DR_TPAGE, SPRT, DR_TPAGE],
                  "cells drawn 8-bit u 200 (u0 72, page x 740 -> 11, 8-bit, CLUT base 256/490), 4-bit u 256 (u0 0, x 704 "
                  "-> 11), 4-bit u 0 (x 640 -> 10): a DR_TPAGE of the previous page at each change, the last one at the end"),
        draw_case("blend_mode2", blend2, 0, -20, 300, [SPRT, DR_TPAGE],
                  "blend mode 2: code 0x66, page abr bits 0x40; x0 -10, y0 320 (no clipping)"),
    ]
