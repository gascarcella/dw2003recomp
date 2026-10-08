"""The GPU (GP0 drawing into VRAM) and LIBGPU's VRAM functions, for the port's software GPU (psxstack/psyq/gpu.c,
psxstack/psyq/libgpu.c; tests/host/gpu_replay.py replays these cases through it).

A case fills VRAM rectangles with a known pattern (a MIPS routine of this module writes xorshift32 pixels into scratch
RAM, the EXE's own LoadImage sends them), runs a GP0 list through the EXE's DrawOTag (or calls MoveImage, ClearImage,
ClearImage2), waits with DrawSync(0), and reads rectangles back with StoreImage: the golden is the pixels the PS1 (the
emulator's software GPU) left. Every pixel a case reads was loaded or drawn by the case itself (VRAM is not restored
between cases), every list starts by setting the whole drawing state (E1..E6) and ends with E6 0 (LoadImage obeys the
mask bit), so cases are independent. The LIBGPU packet builders (SetDrawEnv, SetDefDrawEnv, SetDrawMove) are called
too and their words read back.

Cases (fixed seed): fills (alignment and size rounding, the VRAM edge), VRAM copies (overlaps, the edge, the mask bit),
flat/Gouraud triangles and quads, textured ones in 4/8/15-bit mode, raw and modulated, every semi-transparency mode,
dithering on/off, mask set/check, the texture window, sprites and tiles of every size, lines and polylines, the draw
area and offset, edge rules (shared edges, thin and degenerate triangles, negative coordinates, the size limit).
"""
import math
import random
import struct

from oracle import SCRATCH_BASE, Call, Case, Read, Write

COMMENT = ("The GPU: VRAM patterns loaded with LoadImage, GP0 lists drawn with DrawOTag, the result read back with "
           "StoreImage (fills, copies, every polygon/sprite/tile/line type, texture modes, semi-transparency, dithering, "
           "mask, texture window, draw area/offset, edge rules); MoveImage, ClearImage(2); SetDrawEnv/SetDefDrawEnv/"
           "SetDrawMove packets.")
RESIDENT = "CNTY_SEL loaded; the pattern routine is this family's own code in scratch RAM"

# Scratch RAM (tests/golden/oracle.py SCRATCH_BASE..+0x8000): the case's buffers (the list first, so at SCRATCH_BASE),
# then fixed areas saved and restored by the oracle.
LIST_ADDR = SCRATCH_BASE
BUFFERS_END = 0x80183800
PAT_ADDR = 0x80183800      # the pattern routine's output, LoadImage's source
PAT_SIZE = 0x2000
OUT_ADDR = 0x80185800      # StoreImage's destination
OUT_SIZE = 0x2000
CODE_ADDR = 0x80187800     # the pattern routine
CALL5_ADDR = 0x80187900    # call5: a function with five arguments
CHUNK = PAT_SIZE // 2      # pixels per LoadImage / StoreImage
SEED = 0x6E7E2003

PAT_ZEROS = 0x10000        # pattern flag: one pixel in 8 becomes 0x0000 or 0x8000
PAT_RAMP = 0x20000         # pattern flag: a counter (texel = its index | 0x8000)


# --------------------------------------------------------------------------------------------------- the pattern routine
def assemble():
    """pattern(a0 = dst, a1 = count of halfwords (> 0), a2 = state (xorshift32, not 0), a3 = flags): per halfword,
    state ^= state << 13, >> 17, << 5; pixel = (state >> 8) & (flags & 0xFFFF); with PAT_ZEROS, when state & 7 == 0
    the pixel is (state & 0x80) << 8 instead (0x0000 or 0x8000). With PAT_RAMP the pixel is (state & mask) | 0x8000
    and the state counts up by one (a 15-bit texture whose texel at (u, v) of a 256-wide page is u | v << 8: drawn raw,
    each pixel shows the texture coordinates the GPU interpolated). Returns v0 = 0."""
    def i_type(op, rs, rt, imm):
        return (op << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)

    def r_type(rs, rt, rd, sa, fn):
        return (rs << 21) | (rt << 16) | (rd << 11) | (sa << 6) | fn

    A0, A1, A2, A3, T0, T1, T2, T3, V0, RA, ZERO = 4, 5, 6, 7, 8, 9, 10, 11, 2, 31, 0
    sll = lambda rd, rt, sa: r_type(0, rt, rd, sa, 0x00)
    srl = lambda rd, rt, sa: r_type(0, rt, rd, sa, 0x02)
    xor = lambda rd, rs, rt: r_type(rs, rt, rd, 0, 0x26)
    and_ = lambda rd, rs, rt: r_type(rs, rt, rd, 0, 0x24)
    andi = lambda rt, rs, imm: i_type(0x0C, rs, rt, imm)
    addiu = lambda rt, rs, imm: i_type(0x09, rs, rt, imm)
    sh = lambda rt, off, base: i_type(0x29, base, rt, off)
    bne = lambda rs, rt, off: i_type(0x05, rs, rt, off)
    beq = lambda rs, rt, off: i_type(0x04, rs, rt, off)
    lui = lambda rt, imm: i_type(0x0F, 0, rt, imm)
    jr = lambda rs: r_type(rs, 0, 0, 0, 0x08)
    ori = lambda rt, rs, imm: i_type(0x0D, rs, rt, imm)
    T4 = 12
    # t2 = mask, t3 = PAT_ZEROS, t4 = PAT_RAMP
    code = [andi(T2, A3, 0xFFFF), lui(T3, 1), and_(T3, T3, A3), lui(T4, 2), and_(T4, T4, A3)]
    loop = len(code)
    ramp = len(code)
    code += [beq(T4, ZERO, 0), and_(T1, A2, T2), ori(T1, T1, 0x8000), beq(ZERO, ZERO, 0), addiu(A2, A2, 1)]
    code[ramp] = beq(T4, ZERO, len(code) - (ramp + 1))
    jump_store = len(code) - 2
    code += [sll(T0, A2, 13), xor(A2, A2, T0), srl(T0, A2, 17), xor(A2, A2, T0), sll(T0, A2, 5), xor(A2, A2, T0),
             srl(T1, A2, 8), and_(T1, T1, T2)]
    skip = len(code)
    code += [beq(T3, ZERO, 0), andi(T0, A2, 7), bne(T0, ZERO, 0), andi(T0, A2, 0x80), sll(T1, T0, 8)]
    store = len(code)
    code[jump_store] = beq(ZERO, ZERO, store - (jump_store + 1))
    code[skip] = beq(T3, ZERO, store - (skip + 1))
    code[skip + 2] = bne(T0, ZERO, store - (skip + 3))
    code += [sh(T1, 0, A0), addiu(A1, A1, -1), bne(A1, ZERO, 0), addiu(A0, A0, 2)]
    code[-2] = bne(A1, ZERO, loop - (len(code) - 1))
    code += [jr(RA), addiu(V0, ZERO, 0)]
    # call5(a0 = function, a1 = five argument words): the function with a0..a3 and the fifth on the stack (o32), for
    # LIBGPU functions with five arguments (the oracle passes four); returns the function's v0.
    assert len(code) <= (CALL5_ADDR - CODE_ADDR) // 4
    code += [0] * ((CALL5_ADDR - CODE_ADDR) // 4 - len(code))
    lw = lambda rt, off, base: i_type(0x23, base, rt, off)
    sw = lambda rt, off, base: i_type(0x2B, base, rt, off)
    jalr = lambda rs: r_type(rs, 0, RA, 0, 0x09)
    SP = 29
    code += [addiu(SP, SP, -32), sw(RA, 28, SP), r_type(A0, ZERO, T0, 0, 0x21), r_type(A1, ZERO, T1, 0, 0x21),
             lw(A0, 0, T1), lw(A1, 4, T1), lw(A2, 8, T1), lw(A3, 12, T1), lw(T2, 16, T1), 0, sw(T2, 16, SP),
             jalr(T0), 0, lw(RA, 28, SP), 0, jr(RA), addiu(SP, SP, 32)]
    return b"".join(struct.pack("<I", w) for w in code)


def pattern(count, state, flags):
    """The pattern routine in Python: `count` halfwords."""
    out, mask = [], flags & 0xFFFF
    if flags & PAT_RAMP:
        return [((state + i) & mask) | 0x8000 for i in range(count)]
    for _ in range(count):
        state ^= (state << 13) & 0xFFFFFFFF
        state ^= state >> 17
        state ^= (state << 5) & 0xFFFFFFFF
        px = (state >> 8) & mask
        if flags & PAT_ZEROS and state & 7 == 0:
            px = (state & 0x80) << 8
        out.append(px)
    return out


def chunk_state(seed, k):
    return ((seed * 0x9E3779B1 + k * 0x7F4A7C15 + 0x1234567) & 0xFFFFFFFF) | 1


# ----------------------------------------------------------------------------------------------------- GP0 words
def rgb(r, g, b):
    return (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16)


def xy(x, y):
    return (x & 0xFFFF) | ((y & 0xFFFF) << 16)


def uvw(u, v, hi=0):
    return (u & 0xFF) | ((v & 0xFF) << 8) | ((hi & 0xFFFF) << 16)


def tpage(tx, ty, abr=0, tp=0):
    """The texpage attribute: base x = tx * 64, base y = ty * 256, semi-transparency mode, depth (0 4-bit, 1 8-bit,
    2 15-bit)."""
    return (tx & 0xF) | ((ty & 1) << 4) | ((abr & 3) << 5) | ((tp & 3) << 7)


def clut(x, y):
    return ((x >> 4) & 0x3F) | ((y & 0x1FF) << 6)


def e1(page=0, dither=0, dfe=1):
    return 0xE1000000 | (page & 0x1FF) | (dither << 9) | (dfe << 10)


def e2(mask_x=0, mask_y=0, off_x=0, off_y=0):
    return 0xE2000000 | (mask_x & 0x1F) | ((mask_y & 0x1F) << 5) | ((off_x & 0x1F) << 10) | ((off_y & 0x1F) << 15)


def e3(x, y):
    return 0xE3000000 | (x & 0x3FF) | ((y & 0x3FF) << 10)


def e4(x, y):
    return 0xE4000000 | (x & 0x3FF) | ((y & 0x3FF) << 10)


def e5(x, y):
    return 0xE5000000 | (x & 0x7FF) | ((y & 0x7FF) << 11)


def e6(set_mask=0, check=0):
    return 0xE6000000 | set_mask | (check << 1)


def state(area, ofs=(0, 0), page=0, dither=0, tw=(0, 0, 0, 0), mask=(0, 0), dfe=1):
    """The whole drawing state: E1..E6 (area = x0, y0, x1, y1 inclusive)."""
    return [e1(page, dither, dfe), e2(*tw), e3(area[0], area[1]), e4(area[2], area[3]), e5(*ofs), e6(*mask)]


def poly(code, verts, color=(128, 128, 128), clut_=0, page=0):
    """A polygon packet (code 0x20..0x3F); verts: [(x, y[, u, v][, (r, g, b)])]: the colour of vertex 0 is `color`
    (Gouraud: each vertex's own colour, verts[i][-1], replaces it)."""
    tex, gour = code & 4, code & 0x10
    words = []
    for i, vtx in enumerate(verts):
        c = vtx[-1] if gour else color
        if i == 0:
            words.append((code << 24) | rgb(*c))
        elif gour:
            words.append(rgb(*c))
        words.append(xy(vtx[0], vtx[1]))
        if tex:
            words.append(uvw(vtx[2], vtx[3], clut_ if i == 0 else page if i == 1 else 0))
    return words


def line(code, points, color=(128, 128, 128)):
    """A line or polyline packet (code 0x40..0x5F); points: [(x, y[, (r, g, b)])]. A polyline ends with 0x55555555."""
    gour = code & 0x10
    words = []
    for i, p in enumerate(points):
        c = p[2] if gour else color
        if i == 0:
            words.append((code << 24) | rgb(*c))
        elif gour:
            words.append(rgb(*c))
        words.append(xy(p[0], p[1]))
    if code & 8:
        words.append(0x55555555)
    return words


def rect(code, x, y, w=0, h=0, color=(128, 128, 128), u=0, v=0, clut_=0):
    """A rectangle packet (code 0x60..0x7F): variable size (bits 3-4 = 0) takes w, h."""
    words = [(code << 24) | rgb(*color), xy(x, y)]
    if code & 4:
        words.append(uvw(u, v, clut_))
    if (code >> 3) & 3 == 0:
        words.append(xy(w, h))
    return words


def fill(x, y, w, h, color):
    return [0x02000000 | rgb(*color), xy(x, y), xy(w, h)]


def copy(sx, sy, dx, dy, w, h):
    return [0x80000000, xy(sx, sy), xy(dx, dy), xy(w, h)]


def link(packets, base=LIST_ADDR):
    """The DMA list: each packet {tag: len << 24 | next, words}, the last one's next 0xFFFFFF. Packets longer than
    the 8-bit length go whole (the GPU sees a word stream: a polyline or a transfer may span packets)."""
    out, addr = [], base
    sizes = [len(p) for p in packets]
    for i, p in enumerate(packets):
        assert len(p) < 256
        nxt = 0xFFFFFF if i == len(packets) - 1 else (addr + 4 * (1 + len(p))) & 0xFFFFFF
        out.append((len(p) << 24) | nxt)
        out += p
        addr += 4 * (1 + sizes[i])
    return struct.pack(f"<{len(out)}I", *out)


# ----------------------------------------------------------------------------------------------------------- cases
class Builder:
    """One case: pattern loads, then one or more steps (a GP0 list, or a LIBGPU call), then the reads."""

    def __init__(self, name, comment=""):
        self.name, self.comment = name, comment
        self.rects = []          # RECTs (x, y, w, h) in the case's "rects" buffer
        self.calls = []
        self.lists = []          # GP0 lists, in order (each its own buffer)
        self.bufs = {}

    def _rect(self, x, y, w, h):
        self.rects.append((x, y, w, h))
        return len(self.rects) - 1

    def load(self, x, y, w, h, seed, mask=0xFFFF, flags=0):
        """A pattern into VRAM (x, y, w, h), in chunks of at most CHUNK pixels (whole rows)."""
        rows = max(1, CHUNK // w)
        for k, y0 in enumerate(range(0, h, rows)):
            n = min(rows, h - y0)
            r = self._rect(x, y + y0, w, n)
            st = (seed + y0 * w) & 0xFFFFFFFF if flags & PAT_RAMP else chunk_state(seed, k)
            self.calls.append(Call(f"0x{CODE_ADDR:08X}", [PAT_ADDR, w * n, st, (flags | mask)],
                                   "void", comment=f"pattern {w}x{n} seed {seed:#x} chunk {k}"))
            self.calls.append(Call("LoadImage", [("rect", r), PAT_ADDR], "void"))
            self.calls.append(Call("DrawSync", [0]))

    def draw(self, packets):
        self.lists.append(packets)
        self.calls.append(Call("DrawOTag", [("list", len(self.lists) - 1)], "void"))
        self.calls.append(Call("DrawSync", [0]))

    def call(self, func, args, ret_type="s32", reads=(), comment=""):
        self.calls.append(Call(func, list(args), ret_type, list(reads), comment=comment))
        self.calls.append(Call("DrawSync", [0]))

    def read(self, x, y, w, h):
        rows = max(1, CHUNK // w)
        for y0 in range(0, h, rows):
            n = min(rows, h - y0)
            r = self._rect(x, y + y0, w, n)
            self.calls.append(Call("StoreImage", [("rect", r), OUT_ADDR], "void"))
            self.calls.append(Call("DrawSync", [0], reads=[Read(f"0x{OUT_ADDR:08X}", 0, 2 * w * n,
                                                                 f"VRAM {x},{y + y0} {w}x{n}")]))

    def case(self):
        # Buffer layout (oracle.layout_buffers: in order, 16-aligned from SCRATCH_BASE): the lists, then the RECTs.
        bufs, addr = {}, SCRATCH_BASE
        list_addr = []
        for i, packets in enumerate(self.lists):
            data = link(packets, addr)
            bufs[f"list{i}"] = data
            list_addr.append(addr)
            addr += (len(data) + 15) & ~15
        bufs["rects"] = b"".join(struct.pack("<4h", *r) for r in self.rects) or b"\0" * 8
        bufs.update(self.bufs)
        end = addr + sum((len(d) + 15) & ~15 for n, d in bufs.items() if not n.startswith("list"))
        assert end <= BUFFERS_END, (self.name, hex(end))
        calls = []
        for c in self.calls:
            args = []
            for a in c.args:
                if isinstance(a, tuple) and a[0] == "rect":
                    args.append(("buf_ofs", "rects", 8 * a[1]))
                elif isinstance(a, tuple) and a[0] == "list":
                    args.append(list_addr[a[1]])
                else:
                    args.append(a)
            calls.append(Call(c.func, args, c.ret_type, c.reads, c.comment, c.writes))
        return Case(self.name, calls, buffers=bufs, comment=self.comment)


def rects_addr(case):
    """The address of a case's "rects" buffer (after the lists)."""
    addr = SCRATCH_BASE
    for n, d in case.buffers.items():
        if n == "rects":
            return addr
        addr += (max(len(d), 1) + 15) & ~15
    raise KeyError("rects")


def finish(case):
    """Resolves ("buf_ofs", buffer, offset) arguments to raw addresses (the oracle passes buffers without offsets);
    an "args" buffer (call5's arguments) gets the "env" buffer's address as its first word."""
    if "args" in case.buffers:
        addr = SCRATCH_BASE
        for n, d in case.buffers.items():
            if n == "env":
                break
            addr += (max(len(d), 1) + 15) & ~15
        case.buffers["args"] = struct.pack("<I", addr) + case.buffers["args"][4:]
    base = rects_addr(case)
    for c in case.calls:
        c.args = [base + a[2] if isinstance(a, tuple) and a[0] == "buf_ofs" else a for a in c.args]
    return case


# --------------------------------------------------------------------------------------------------- case generators
T = (512, 0)              # the target area's corner (64x64): the draw area and offset of most cases
TS = 64
TEX_PAGE = (10, 1)        # the texture page: VRAM (640, 256); the cases' texture coordinates stay in 0..63
TEX = (640, 256)
CLUT_X, CLUT_Y = 768, 480  # CLUT rows (256 entries each) from here down
DEPTH_W = {0: 16, 1: 32, 2: 64, 3: 64}   # VRAM pixels holding 64 texels at each depth


def target_state(ofs=None, area=None, **kw):
    """E1..E6 drawing into the target area (the draw area = the target, the offset = its corner unless given)."""
    x, y = T
    return state(area or (x, y, x + TS - 1, y + TS - 1), ofs if ofs is not None else (x, y), **kw)


def load_target(b, r, mask=0xFFFF):
    b.load(T[0], T[1], TS, TS, r.getrandbits(31) | 1, mask)


def load_texture(b, r, depth, zeros=True):
    """The texture page's first 64x64 texels and two CLUT rows (with transparent and STP texels when `zeros`)."""
    flags = PAT_ZEROS if zeros else 0
    b.load(TEX[0], TEX[1], DEPTH_W[depth], 64, r.getrandbits(31) | 1, 0xFFFF, flags if depth >= 2 else 0)
    if depth < 2:
        b.load(CLUT_X, CLUT_Y, 256, 2, r.getrandbits(31) | 1, 0xFFFF, flags)


def read_target(b):
    b.read(T[0], T[1], TS, TS)


def rand_color(r):
    k = r.random()
    if k < 0.15:
        return (128, 128, 128)
    if k < 0.25:
        return tuple(r.choice([0, 255, 127, 128, 129, 8, 7]) for _ in range(3))
    return (r.randrange(256), r.randrange(256), r.randrange(256))


def rand_xy(r, lo=-12, hi=TS + 12):
    return (r.randint(lo, hi), r.randint(lo, hi))


def rand_uv(r):
    return (r.randint(2, 61), r.randint(2, 61))


class Mode:
    def __init__(self, r, semi=None, dither=None):
        self.semi = (r.random() < 0.4) if semi is None else semi
        self.abr = r.randrange(4)
        self.dither = (r.random() < 0.5) if dither is None else dither
        self.mask = (int(r.random() < 0.2), int(r.random() < 0.2))
        self.depth = r.choice([0, 0, 1, 1, 2, 3])
        self.raw = r.random() < 0.3

    def page(self):
        return tpage(*TEX_PAGE, self.abr, self.depth)

    def desc(self):
        return (f"semi {int(self.semi)} abr {self.abr} dither {int(self.dither)} mask {self.mask} depth {self.depth} "
                f"raw {int(self.raw)}")


def clut_for(r, depth):
    if depth == 0:
        return clut(CLUT_X + 16 * r.randrange(16), CLUT_Y + r.randrange(2))
    return clut(CLUT_X, CLUT_Y + r.randrange(2))


def cross(a, b, c):
    """Twice the signed area of a triangle: an interpolated attribute is a multiple of 1/cross away from its vertex
    values, so with an odd cross product no pixel's exact value ends in .5."""
    return (b[0] - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (b[1] - a[1])


def poly_cases(r):
    """Random polygons of every type and mode. The interpolated ones (Gouraud, textured) have an odd cross product:
    no exact interpolation ties, where the emulator's rounding is not modelled (see the uv_ramp/uv_axis probes)."""
    out = []
    names = {0x20: "f3", 0x28: "f4", 0x30: "g3", 0x38: "g4", 0x24: "ft3", 0x2C: "ft4", 0x34: "gt3", 0x3C: "gt4"}
    for code, name in names.items():
        for k in range(32):
            m = Mode(r)
            tex, gour, nv = code & 4, code & 0x10, 4 if code & 8 else 3
            c = code | (2 if m.semi else 0) | (1 if tex and m.raw else 0)
            b = Builder(f"poly_{name}_{k}", f"{name}: {m.desc()}")
            load_target(b, r)
            if tex:
                load_texture(b, r, m.depth)
            prims = []
            for _ in range(r.choice([1, 1, 2, 3])):
                small = r.random() < 0.3
                base = rand_xy(r, 0, TS - 16)
                while True:
                    verts = []
                    for _ in range(nv):
                        p = (base[0] + r.randint(-6, 16), base[1] + r.randint(-6, 16)) if small else rand_xy(r)
                        if tex:
                            p += rand_uv(r)
                        if gour:
                            p += (rand_color(r),)
                        verts.append(p)
                    if not (tex or gour) or all(cross(*verts[i:i + 3]) % 2 for i in range(nv - 2)):
                        break   # interpolated: no exact .5 ties (tie_free)
                prims.append(poly(c, verts, rand_color(r), clut_for(r, m.depth), m.page()))
            page = tpage(*TEX_PAGE, m.abr, m.depth)
            b.draw([[0x01000000] + target_state(page=page, dither=int(m.dither), mask=m.mask)] + prims + [[e6()]])
            read_target(b)
            out.append(b)
    return out


def rect_cases(r):
    out = []
    for textured in (0, 1):
        for size in range(4):
            for k in range(10):
                m = Mode(r)
                code = 0x60 | (size << 3) | (4 if textured else 0) | (2 if m.semi else 0) | (1 if textured and m.raw else 0)
                b = Builder(f"rect_{'sprt' if textured else 'tile'}{['var', '1', '8', '16'][size]}_{k}",
                            f"rectangle {code:#x}: {m.desc()}")
                load_target(b, r)
                if textured:
                    load_texture(b, r, m.depth)
                prims = []
                for _ in range(r.choice([1, 2, 3])):
                    x, y = rand_xy(r, -10, TS - 4)
                    w, h = r.randint(0, 40), r.randint(0, 40)
                    u, v = r.randint(0, 20), r.randint(0, 20)
                    prims.append(rect(code, x, y, w, h, rand_color(r), u, v, clut_for(r, m.depth)))
                b.draw([[0x01000000] + target_state(page=m.page(), dither=int(m.dither), mask=m.mask)] + prims + [[e6()]])
                read_target(b)
                out.append(b)
    return out


def line_cases(r):
    out = []
    for code, name in ((0x40, "f2"), (0x50, "g2"), (0x48, "fpoly"), (0x58, "gpoly")):
        for k in range(14):
            m = Mode(r)
            c = code | (2 if m.semi else 0)
            b = Builder(f"line_{name}_{k}", f"line {c:#x}: {m.desc()}")
            load_target(b, r)
            prims = []
            for _ in range(r.choice([1, 2, 3])):
                n = r.randint(3, 6) if code & 8 else 2
                pts = []
                for _ in range(n):
                    p = rand_xy(r, -8, TS + 8)
                    if r.random() < 0.3 and pts:   # horizontal, vertical, diagonal and short segments
                        q = pts[-1]
                        p = r.choice([(p[0], q[1]), (q[0], p[1]), (q[0] + (p[1] - q[1]), p[1]), (q[0] + 1, q[1] + 2), q])
                    pts.append(p + (rand_color(r),))
                if code & 0x18 == 0x18 and (pts[0][2][1] >> 4) == 5:
                    # The emulator never ends a Gouraud polyline whose first word (code and colour) has the end
                    # marker's pattern (word & 0xF000F000 == 0x50005000: a green of 0x5X); the commands after it
                    # vanish into it (the PS1 itself is not affected: it looks for the marker after the first vertex).
                    pts[0] = pts[0][:2] + ((pts[0][2][0], pts[0][2][1] ^ 0x80, pts[0][2][2]),)
                prims.append(line(c, pts, rand_color(r)))
            b.draw([[0x01000000] + target_state(page=m.page(), dither=int(m.dither), mask=m.mask)] + prims + [[e6()]])
            read_target(b)
            out.append(b)
    return out


def fill_cases(r):
    out = []
    # A 128x32 region: fills inside it (x rounded down to 16, the width up), then at the VRAM's right and bottom edges.
    R = (512, 64, 128, 32)
    for k in range(16):
        b = Builder(f"fill_{k}", "fills in a 128x32 region")
        b.load(*R, r.getrandbits(31) | 1)
        prims = []
        for _ in range(r.choice([1, 2, 3])):
            x = R[0] + r.randint(0, 60)
            y = R[1] + r.randint(0, 24)
            w = r.choice([0, 1, 15, 16, 17, 31, 33, r.randint(0, 60)])
            h = r.choice([0, 1, r.randint(0, 32 - (y - R[1]))])
            h = min(h, R[1] + R[3] - y)
            prims.append(fill(x, y, w, h, rand_color(r)))
        # garbage in the unused bits of the coordinates
        if k % 4 == 3:
            prims.append(fill(R[0] + 32 + 0x8000, R[1] + 4 + 0x4000, 16 + 0x4000, 4 + 0x2000, (1, 2, 3)))
        b.draw([target_state(mask=(1, 1))] + prims + [[e6()]])
        b.read(*R)
        out.append(b)
    for k, (x, y, w, h) in enumerate([(1000, 128, 48, 8), (1008, 140, 32, 4), (512, 504, 32, 16), (1016, 508, 16, 8)]):
        b = Builder(f"fill_edge_{k}", "a fill across the VRAM's edge")
        regions = [(960, 128, 64, 24), (0, 128, 64, 24), (512, 496, 64, 16), (512, 0, 64, 16), (960, 496, 64, 16),
                   (0, 496, 64, 16), (960, 0, 64, 16), (0, 0, 64, 16)]
        for reg in regions:
            b.load(*reg, r.getrandbits(31) | 1)
        b.draw([target_state(), fill(x, y, w, h, rand_color(r))])
        for reg in regions:
            b.read(*reg)
        out.append(b)
    return out


def copy_cases(r):
    out = []
    R = (512, 64, 128, 32)
    for k in range(20):
        b = Builder(f"copy_{k}", "VRAM copies in a 128x32 region (overlaps, the mask bit)")
        b.load(*R, r.getrandbits(31) | 1)
        mask = (int(r.random() < 0.3), int(r.random() < 0.3))
        prims = [target_state(mask=mask)]
        for _ in range(r.choice([1, 2])):
            w, h = r.randint(1, 40), r.randint(1, 16)
            sx, sy = R[0] + r.randint(0, R[2] - w), R[1] + r.randint(0, R[3] - h)
            if r.random() < 0.6:   # overlapping
                dx, dy = sx + r.randint(-4, 4), sy + r.randint(-3, 3)
                dx, dy = min(max(dx, R[0]), R[0] + R[2] - w), min(max(dy, R[1]), R[1] + R[3] - h)
            else:
                dx, dy = R[0] + r.randint(0, R[2] - w), R[1] + r.randint(0, R[3] - h)
            prims.append(copy(sx, sy, dx, dy, w, h))
        b.draw(prims + [[e6()]])
        b.read(*R)
        out.append(b)
    regions = [(960, 128, 64, 24), (0, 128, 64, 24), (512, 496, 64, 16), (512, 0, 64, 16)]
    for k, (sx, sy, dx, dy, w, h) in enumerate([(1000, 130, 980, 140, 40, 6), (980, 130, 1010, 140, 30, 6),
                                                (520, 500, 530, 504, 16, 20), (1016, 132, 20, 140, 16, 4)]):
        b = Builder(f"copy_edge_{k}", "a copy across the VRAM's edge")
        for reg in regions:
            b.load(*reg, r.getrandbits(31) | 1)
        b.draw([target_state(), copy(sx, sy, dx, dy, w, h)])
        for reg in regions:
            b.read(*reg)
        out.append(b)
    return out


def window_cases(r):
    out = []
    for k in range(16):
        m = Mode(r, semi=False)
        tw = (r.choice([0, 1, 2, 3, 4, 7, 0x1F, r.randrange(32)]), r.choice([0, 1, 3, 7, r.randrange(32)]),
              r.randrange(8), r.randrange(8))
        code = r.choice([0x2C, 0x3C, 0x64, 0x24]) | (1 if m.raw else 0)
        b = Builder(f"window_{k}", f"texture window {tw} code {code:#x}: {m.desc()}")
        load_target(b, r)
        load_texture(b, r, m.depth)
        if code & 0x40:
            prim = rect(code, r.randint(-4, 20), r.randint(-4, 20), r.randint(20, 60), r.randint(20, 60),
                        rand_color(r), r.randint(0, 8), r.randint(0, 8), clut_for(r, m.depth))
        else:
            nv = 4 if code & 8 else 3
            verts = [(x, y, u, v, rand_color(r)) for (x, y), (u, v) in
                     zip([(-4, -4), (70, -2), (-2, 68), (66, 66)][:nv], [(2, 2), (61, 2), (2, 61), (61, 61)])]
            prim = poly(code, verts, rand_color(r), clut_for(r, m.depth), m.page())
        b.draw([[0x01000000] + target_state(page=m.page(), dither=int(m.dither), tw=tw), prim, [e6()]])
        read_target(b)
        out.append(b)
    return out


def edge_cases(r):
    """Hand-picked: shared edges, thin and degenerate triangles, negative coordinates, the size limit, gradients, the
    draw area and offset."""
    out = []

    def case(name, comment, prims, st=None, tex_depth=None, mask=0xFFFF):
        b = Builder(name, comment)
        load_target(b, r, mask)
        if tex_depth is not None:
            load_texture(b, r, tex_depth, zeros=False)
        b.draw([[0x01000000] + (st or target_state())] + prims + [[e6()]])
        read_target(b)
        out.append(b)

    add = target_state(page=tpage(0, 0, 1))    # abr 1 (additive): a pixel drawn twice shows
    white = (64, 64, 64)
    # Shared edges: two triangles of a quad, a fan around a centre, a strip: additive, so overlaps and gaps show.
    case("edge_shared_quad", "two triangles sharing a diagonal, additive",
         [poly(0x22, [(3, 5), (60, 9), (7, 58)], white), poly(0x22, [(60, 9), (7, 58), (58, 61)], white)], add, mask=0)
    fan = [(32 + int(28 * math.cos(a / 10 * 6.2832)), 30 + int(26 * math.sin(a / 10 * 6.2832)))
           for a in range(10)]
    case("edge_shared_fan", "a fan of 10 triangles around (32, 30), additive",
         [poly(0x22, [(32, 30), fan[i], fan[(i + 1) % 10]], white) for i in range(10)], add, mask=0)
    case("edge_shared_strip", "a strip of thin triangles, additive",
         [poly(0x22, [(i * 6, 2), (i * 6 + 6, 2), (i * 6 + 3, 62)], white) for i in range(10)]
         + [poly(0x22, [(i * 6 + 6, 2), (i * 6 + 3, 62), (i * 6 + 9, 62)], white) for i in range(9)], add, mask=0)
    case("edge_quads_grid", "a 4x4 grid of quads sharing edges (non-axis-aligned), additive",
         [poly(0x2A, [(4 + 14 * i + j, 3 + 14 * j - i), (18 + 14 * i + j, 3 + 14 * j - i + 1),
                      (4 + 14 * i + j + 1, 17 + 14 * j - i), (18 + 14 * i + j + 1, 17 + 14 * j - i + 1)], white)
          for i in range(4) for j in range(4)], add, mask=0)
    # Thin and degenerate.
    case("edge_thin", "thin triangles (1 pixel and less)",
         [poly(0x20, [(2, 2), (60, 3), (2, 3)], (255, 0, 0)), poly(0x20, [(2, 10), (3, 60), (3, 10)], (0, 255, 0)),
          poly(0x20, [(10, 10), (50, 50), (11, 10)], (0, 0, 255)), poly(0x20, [(20, 5), (60, 30), (20, 6)], (255, 255, 0))])
    case("edge_degenerate", "collinear and point triangles, a quad folded on itself",
         [poly(0x20, [(5, 5), (30, 30), (55, 55)], (255, 0, 0)), poly(0x20, [(9, 9), (9, 9), (9, 9)], (0, 255, 0)),
          poly(0x20, [(5, 40), (60, 40), (30, 40)], (0, 0, 255)),
          poly(0x28, [(10, 50), (50, 50), (50, 60), (10, 60)], (255, 0, 255)),
          poly(0x28, [(40, 5), (60, 25), (40, 25), (60, 5)], (0, 255, 255))])
    case("edge_bowtie", "non-convex and bow-tie quads",
         [poly(0x28, [(4, 4), (60, 4), (60, 60), (4, 60)], (200, 0, 0)), poly(0x2A, [(4, 30), (30, 4), (58, 58), (30, 34)], white)],
         target_state(page=tpage(0, 0, 1)))
    # Negative coordinates and offsets.
    case("edge_negative", "vertices left of and above the offset, the offset at the target's middle",
         [poly(0x30, [(-40, -40, (255, 0, 0)), (31, -32, (0, 255, 0)), (-32, 31, (0, 0, 255))])],
         target_state(ofs=(T[0] + 32, T[1] + 32)))
    case("edge_neg_area", "an offset far left: vertex -1000 + offset",
         [poly(0x20, [(-1000, 0), (-950, 10), (-990, 60)], (10, 200, 30))], target_state(ofs=(T[0] + 1000, T[1])))
    case("edge_11bit", "vertex coordinates with bits above the 11th (sign-extended from bit 10)",
         [poly(0x20, [(0x0805, 3), (0x1830, 0x2810), (0xF80A, 0x0838)], (200, 100, 50))], target_state(ofs=(T[0], T[1])))
    # The size limit: 1023 / 1024 wide, 511 / 512 tall (the visible part is clipped to the target).
    for k, (w, h) in enumerate([(1023, 40), (1024, 40), (40, 511), (40, 512), (1023, 511), (1025, 513)]):
        case(f"edge_limit_{k}", f"a triangle {w} wide and {h} tall",
             [poly(0x20, [(10, 10), (10 + w, 12), (12, 10 + h)], (250, 250, 0)),
              poly(0x28, [(30, 30), (30 + w, 30), (30, 30 + h), (30 + w, 30 + h)], (0, 250, 250))],
             target_state(ofs=(T[0], T[1])))
    for k, (w, h) in enumerate([(1023, 0), (1024, 0), (0, 511), (0, 512)]):
        case(f"edge_line_limit_{k}", f"a line {w} wide and {h} tall",
             [line(0x40, [(5, 5), (5 + w, 5 + h)], (250, 250, 0)), line(0x40, [(40, 40), (40 - w, 40 - h)], (0, 250, 250))])
    # Gouraud gradients: full range along x, along y, diagonal; dithered and not.
    for d in (0, 1):
        case(f"edge_grad_x_d{d}", "a red gradient 0..255 along x (two triangles)",
             [poly(0x38, [(0, 0, (0, 0, 0)), (63, 0, (255, 255, 255)), (0, 64, (0, 0, 0)), (63, 64, (255, 255, 255))])],
             target_state(dither=d))
        case(f"edge_grad_y_d{d}", "a gradient along y",
             [poly(0x38, [(0, 0, (255, 0, 40)), (64, 0, (255, 0, 40)), (0, 63, (0, 255, 200)), (64, 63, (0, 255, 200))])],
             target_state(dither=d))
        case(f"edge_grad_tri_d{d}", "a three-colour triangle",
             [poly(0x30, [(0, 0, (255, 0, 0)), (63, 20, (0, 255, 0)), (10, 63, (0, 0, 255))])], target_state(dither=d))
        case(f"edge_grad_big_d{d}", "a large triangle, clipped, colours far apart",
             [poly(0x30, [(-300, -100, (255, 0, 0)), (400, 10, (0, 255, 0)), (20, 300, (0, 0, 255))])], target_state(dither=d))
        case(f"edge_uv_big_d{d}", "a large textured triangle (15-bit), clipped",
             [poly(0x34, [(-30, -30, 0, 0, (128, 128, 128)), (120, -10, 63, 0, (255, 64, 0)), (0, 140, 0, 63, (0, 128, 255))],
                   page=tpage(*TEX_PAGE, 0, 2))], target_state(dither=d, page=tpage(*TEX_PAGE, 0, 2)), tex_depth=2)
    # Sprites on the ramp texture: u + x wrapping at 256, v + y past the page's 128 loaded rows is avoided.
    for k, (x, y, w, h, u, v) in enumerate([(0, 0, 64, 32, 230, 10), (-5, 3, 60, 40, 251, 77), (7, -9, 33, 50, 0, 0)]):
        b = Builder(f"edge_sprite_uwrap_{k}", "a raw 15-bit sprite on the ramp texture: u runs past 255 (wraps)")
        load_target(b, r)
        b.load(768, 256, 256, 128, 0, 0x7FFF, PAT_RAMP)
        page = tpage(*RAMP_PAGE, 0, 2)
        b.draw([[0x01000000] + target_state(page=page), rect(0x65, x, y, w, h, u=u, v=v), [e6()]])
        read_target(b)
        out.append(b)
    # Draw area: smaller than the target, an empty one (x1 < x0), a 1x1 one.
    for k, area in enumerate([(T[0] + 10, T[1] + 5, T[0] + 50, T[1] + 40), (T[0] + 30, T[1] + 30, T[0] + 20, T[1] + 40),
                              (T[0] + 31, T[1] + 31, T[0] + 31, T[1] + 31)]):
        case(f"edge_area_{k}", f"draw area {area}",
             [poly(0x30, [(-5, -5, (255, 0, 0)), (70, 0, (0, 255, 0)), (0, 70, (0, 0, 255))]),
              rect(0x60, 0, 50, 64, 14, (9, 99, 199)), line(0x40, [(0, 63), (63, 0)], (255, 255, 255)),
              fill(T[0] + 16, T[1] + 48, 32, 8, (250, 0, 250))],
             target_state(area=area))
    # Semi-transparency of each mode with colours at the extremes (saturation both ways), untextured and textured.
    for abr in range(4):
        case(f"edge_semi_{abr}", f"mode {abr} over a random background: white, black, mid grey, with and without the STP texels",
             [rect(0x62, 0, 0, 64, 16, (255, 255, 255)), rect(0x62, 0, 16, 64, 16, (0, 0, 0)),
              rect(0x62, 0, 32, 64, 16, (128, 64, 200)), rect(0x67, 0, 48, 64, 16, u=0, v=0)],
             target_state(page=tpage(*TEX_PAGE, abr, 2)), tex_depth=2)
    # Modulation saturation: colour 255 doubles a texel (clamped at 31).
    for d in (0, 1):
        case(f"edge_modulate_d{d}", "textured quads modulated by 0, 64, 127, 128, 129, 255",
             [poly(0x2C, [(0, 10 * i, 0, 10 * i), (64, 10 * i, 63, 10 * i), (0, 10 * i + 10, 0, 10 * i + 10),
                          (64, 10 * i + 10, 63, 10 * i + 10)], (c, c, c), page=tpage(*TEX_PAGE, 0, 2))
              for i, c in enumerate([0, 64, 127, 128, 129, 255])],
             target_state(dither=d, page=tpage(*TEX_PAGE, 0, 2)), tex_depth=2)
    # The texpage of a textured polygon stays in force for later rectangles (E1's bits 0-8).
    case("edge_texpage_sticks", "a polygon's texpage, then a sprite without E1",
         [poly(0x2D, [(0, 0, 0, 0), (20, 0, 20, 0), (0, 20, 0, 20)], page=tpage(*TEX_PAGE, 2, 2)),
          rect(0x67, 20, 20, 40, 40, u=5, v=5)], target_state(page=tpage(0, 0, 0, 0)), tex_depth=2)
    # dfe 0 (drawing to the displayed area not allowed): no effect outside interlace.
    case("edge_dfe0", "E1 dfe 0", [poly(0x30, [(0, 0, (255, 0, 0)), (63, 20, (0, 255, 0)), (10, 63, (0, 0, 255))])],
         target_state(dfe=0, dither=1))
    # GP0 commands without effect between primitives: NOPs, cache clear, unused E words.
    case("edge_nops", "NOP, 01, 03..1E, E0, E7..EF between primitives",
         [[0x00000000, 0x01000000, 0x03000000, 0x1E000000, 0xE0000000, 0xE7000000, 0xEF123456],
          poly(0x20, [(0, 0), (63, 20), (10, 63)], (0, 200, 0))])
    return out


def api_cases(r):
    """LIBGPU's own functions in the EXE: MoveImage, ClearImage, ClearImage2 into VRAM; LoadImage with odd sizes;
    SetDrawEnv, SetDefDrawEnv and SetDrawMove packets."""
    out = []
    R = (512, 64, 128, 32)
    for k, (sx, sy, w, h, dx, dy) in enumerate([(520, 66, 16, 8, 560, 80), (530, 70, 33, 5, 531, 71), (600, 64, 7, 3, 512, 90),
                                                (512, 64, 128, 1, 512, 65), (515, 67, 0, 4, 530, 70), (520, 70, 4, 0, 530, 70)]):
        b = Builder(f"api_moveimage_{k}", f"MoveImage {sx},{sy} {w}x{h} -> {dx},{dy}")
        b.load(*R, r.getrandbits(31) | 1)
        b.bufs["rect"] = struct.pack("<4h", sx, sy, w, h)
        b.call("MoveImage", [("buf", "rect"), dx, dy])
        b.read(*R)
        out.append(b)
    for fn in ("ClearImage", "ClearImage2"):
        for k, (x, y, w, h, c) in enumerate([(520, 66, 16, 8, (255, 0, 0)), (530, 70, 33, 5, (7, 8, 9)),
                                             (517, 64, 1, 30, (128, 129, 130)), (512, 90, 128, 2, (255, 255, 255)),
                                             (600, 70, 0, 4, (50, 60, 70)), (0, 0, 1024, 512, (8, 16, 24)),
                                             (0, 70, 1024, 8, (40, 50, 60)), (520, 0, 64, 512, (70, 80, 90))]):
            b = Builder(f"api_{fn.lower()}_{k}", f"{fn} {x},{y} {w}x{h} rgb {c}")
            b.load(*R, r.getrandbits(31) | 1)
            b.bufs["rect"] = struct.pack("<4h", x, y, w, h)
            b.call(fn, [("buf", "rect")] + list(c))
            b.read(*R)
            out.append(b)
    for k, (w, h) in enumerate([(3, 3), (5, 1), (1, 7)]):
        b = Builder(f"api_loadimage_odd_{k}", f"LoadImage of {w}x{h} (an odd number of pixels)")
        b.load(*R, r.getrandbits(31) | 1)
        b.load(530, 70, w, h, r.getrandbits(31) | 1)
        b.read(*R)
        out.append(b)
    # Packets: SetDrawEnv (with and without a background fill, a texture window, odd sizes), SetDefDrawEnv, SetDrawMove.
    for k, env in enumerate([
            dict(clip=(0, 0, 320, 240), ofs=(0, 0), tw=(0, 0, 0, 0), tpage=0x0A, dtd=1, dfe=1, isbg=0, rgb=(0, 0, 0)),
            dict(clip=(0, 256, 320, 240), ofs=(0, 256), tw=(0, 0, 0, 0), tpage=0x1F, dtd=0, dfe=0, isbg=1, rgb=(10, 20, 30)),
            dict(clip=(13, 7, 101, 55), ofs=(-20, 300), tw=(16, 32, 64, 128), tpage=0x1FF, dtd=1, dfe=0, isbg=1,
                 rgb=(255, 128, 1)),
            dict(clip=(1000, 500, 100, 100), ofs=(1023, -1024), tw=(8, 8, 8, 8), tpage=0x9FF, dtd=2, dfe=3, isbg=2,
                 rgb=(1, 2, 3)),
            dict(clip=(0, 0, 640, 480), ofs=(0, 0), tw=(0, 0, 256, 256), tpage=0, dtd=1, dfe=0, isbg=0, rgb=(0, 0, 0)),
            dict(clip=(0, 0, 0, 0), ofs=(5, 5), tw=(255, 255, 248, 8), tpage=0x200, dtd=0, dfe=1, isbg=1, rgb=(0, 0, 0)),
            dict(clip=(-8, -3, 1100, 600), ofs=(0, 0), tw=(0, 0, 0, 0), tpage=0, dtd=1, dfe=1, isbg=1, rgb=(9, 9, 9)),
            dict(clip=(1030, 520, -5, -7), ofs=(-3, 4), tw=(0, 0, 0, 0), tpage=0, dtd=1, dfe=1, isbg=1, rgb=(9, 9, 9))]):
        b = Builder(f"api_setdrawenv_{k}", f"SetDrawEnv {env}")
        drawenv = (struct.pack("<4h2h4hH6B", *env["clip"], *env["ofs"], *env["tw"], env["tpage"], env["dtd"], env["dfe"],
                               env["isbg"], *env["rgb"]) + b"\xEE" * 64)
        b.bufs["env"] = drawenv
        b.bufs["dr"] = bytes([0x44, 0x33, 0x22, 0x11]) + b"\xEE" * 60
        b.call("SetDrawEnv", [("buf", "dr"), ("buf", "env")], "void",
               [Read("buf:dr", 0, 64, "the DR_ENV")])
        out.append(b)
    for k, (x, y, w, h) in enumerate([(0, 0, 320, 240), (0, 256, 320, 240), (0, 0, 320, 256), (0, 0, 320, 257),
                                      (0, 0, 320, 288), (0, 0, 320, 289),
                                      (0, 0, 640, 480), (17, 33, 100, 1)]):
        b = Builder(f"api_setdefdrawenv_{k}", f"SetDefDrawEnv {x},{y} {w}x{h} (through call5)")
        b.bufs["env"] = b"\xEE" * 92
        b.bufs["args"] = struct.pack("<5i", 0, x, y, w, h)    # word 0: the DRAWENV's address, written by finish()
        b.call(f"0x{CALL5_ADDR:08X}", [("sym", "SetDefDrawEnv"), ("buf", "args")], "u32",
               [Read("buf:env", 0, 92, "the DRAWENV")])
        out.append(b)
    b = Builder("api_breakdraw_idle", "BreakDraw with the GPU idle (after DrawSync): what it returns")
    b.call("BreakDraw", [], "s32")
    out.append(b)
    for k, (x, y, w, h, dx, dy) in enumerate([(320, 0, 16, 16, 640, 256), (1000, 500, 1, 1, -1, -2)]):
        b = Builder(f"api_setdrawmove_{k}", f"SetDrawMove {x},{y} {w}x{h} -> {dx},{dy}")
        b.bufs["rect"] = struct.pack("<4h", x, y, w, h)
        b.bufs["mv"] = bytes([0x44, 0x33, 0x22, 0x11]) + b"\xEE" * 20
        b.call("SetDrawMove", [("buf", "mv"), ("buf", "rect"), dx, dy], "void", [Read("buf:mv", 0, 24, "the DR_MOVE")])
        out.append(b)
    return out


RAMP_PAGE = (12, 1)        # the ramp texture: VRAM (768, 256), 256 x 128 texels (u | v << 8)


def ramp_cases(r):
    """Raw 15-bit textured polygons on the ramp texture: every pixel shows the (u, v) the GPU interpolated."""
    out = []
    page = tpage(*RAMP_PAGE, 0, 2)
    for k in range(40):
        quad = k % 4 == 3
        code = 0x2D if quad else 0x25
        b = Builder(f"uv_ramp_{k}", f"{'quad' if quad else 'triangle'}: raw 15-bit texels u | v << 8")
        load_target(b, r)
        b.load(768, 256, 256, 128, 0, 0x7FFF, PAT_RAMP)
        verts = []
        for _ in range(4 if quad else 3):
            if k < 8:
                p = rand_xy(r, -4, TS + 4)
            else:
                p = rand_xy(r, -40, TS + 40)
            verts.append(p + (r.randrange(256), r.randrange(128)))
        b.draw([[0x01000000] + target_state(page=page), poly(code, verts, page=page), [e6()]])
        read_target(b)
        out.append(b)
    return out


def axis_cases(r):
    """Right triangles on the ramp texture whose u changes along x only and v along y only (the right angle at one of
    the four corners): the interpolation's precision and rounding, one axis at a time."""
    out = []
    page = tpage(*RAMP_PAGE, 0, 2)
    for k in range(32):
        b = Builder(f"uv_axis_{k}", "four right triangles: u along x, v along y")
        load_target(b, r)
        b.load(768, 256, 256, 128, 0, 0x7FFF, PAT_RAMP)
        prims = []
        for q in range(4):
            ox, oy = 32 * (q & 1), 32 * (q >> 1)
            w, h = r.randint(2, 31), r.randint(2, 31)
            u0, v0 = r.randint(64, 190), r.randint(32, 90)
            du = r.randint(1, 60) * r.choice([1, -1])
            corner = r.randrange(4) if k >= 8 else 0
            cx, cy = (w if corner & 1 else 0), (h if corner & 2 else 0)
            sx, sy = (-1 if corner & 1 else 1), (-1 if corner & 2 else 1)
            dv = sy * (r.choice([v for v in range(max(1, v0 - 60), min(126, v0 + 60) + 1) if v != v0]) - v0)  # in 128 rows
            verts = [(ox + cx, oy + cy, u0, v0), (ox + cx + sx * w, oy + cy, u0 + sx * du, v0),
                     (ox + cx, oy + cy + sy * h, u0, v0 + sy * dv)]
            r.shuffle(verts)
            prims.append(poly(0x25, verts, page=page))
        b.draw([[0x01000000] + target_state(page=page)] + prims + [[e6()]])
        read_target(b)
        out.append(b)
    return out


def dither_cases(r):
    """Dithering, measured: a grid of 8x8 quads, each of one colour (0, 4, .. 252), over a texture of one texel value
    (a fill), modulated and dithered; and Gouraud quads whose four colours are equal."""
    out = []
    page = tpage(*TEX_PAGE, 0, 2)
    for k, texel in enumerate([(248, 136, 8), (128, 64, 200), (40, 255, 96), (8, 16, 24)]):
        b = Builder(f"dither_tex_{k}", f"modulated texel {texel} by 64 colours, dithered")
        load_target(b, r)
        prims = [fill(TEX[0], TEX[1], 64, 64, texel)]
        for i in range(64):
            x, y, c = 8 * (i % 8), 8 * (i // 8), 4 * i + k
            prims.append(poly(0x2C, [(x, y, 0, 0), (x + 8, y, 7, 0), (x, y + 8, 0, 7), (x + 8, y + 8, 7, 7)],
                              (c, (c + 85) & 0xFF, (c + 170) & 0xFF), page=page))
        b.draw([[0x01000000] + target_state(page=page, dither=1)] + prims + [[e6()]])
        read_target(b)
        out.append(b)
    for k, texel in enumerate([(248, 136, 8), (128, 64, 200), (40, 255, 96), (8, 16, 24)]):
        b = Builder(f"gt_modulate_{k}", f"Gouraud-textured quads of one colour each over texels {texel}, no dither")
        load_target(b, r)
        prims = [fill(TEX[0], TEX[1], 64, 64, texel)]
        for i in range(64):
            x, y, c = 8 * (i % 8), 8 * (i // 8), 4 * i + k
            col = (c, (c + 85) & 0xFF, (c + 170) & 0xFF)
            prims.append(poly(0x3C, [(x, y, 0, 0, col), (x + 8, y, 7, 0, col), (x, y + 8, 0, 7, col),
                                     (x + 8, y + 8, 7, 7, col)], page=page))
        b.draw([[0x01000000] + target_state(page=page)] + prims + [[e6()]])
        read_target(b)
        out.append(b)
    for k in range(12):
        # Gouraud-textured triangles over one texel value 16 (a 128 fill): (16 * c) >> 7 = c >> 3, the colour itself.
        b = Builder(f"gt_colour_{k}", "Gouraud-textured triangles over texels of 16: the interpolated colour")
        load_target(b, r)
        prims = [fill(TEX[0], TEX[1], 64, 64, (128, 128, 128) if k < 8 else (255, 255, 255))]
        for _ in range(3):
            verts = [rand_xy(r, -20, TS + 20) + rand_uv(r) + (rand_color(r),) for _ in range(3 if k % 2 == 0 else 4)]
            prims.append(poly(0x34 if k % 2 == 0 else 0x3C, verts, page=page))
        b.draw([[0x01000000] + target_state(page=page, dither=int(k % 4 >= 2))] + prims + [[e6()]])
        read_target(b)
        out.append(b)
    for abr in range(4):
        for tex in (0, 1):
            b = Builder(f"dither_semi_{abr}_{'gt' if tex else 'g'}",
                        f"semi-transparent (mode {abr}) dithered {'Gouraud-textured (texel 16, STP)' if tex else 'Gouraud'}"
                        " quads of one colour each, over a random background")
            load_target(b, r)
            prims = [fill(TEX[0], TEX[1], 64, 64, (128, 128, 128))] if tex else []
            pg = tpage(*TEX_PAGE, abr, 2)
            for i in range(64):
                x, y, c = 8 * (i % 8), 8 * (i // 8), 4 * i + 1
                col = (c, (c + 85) & 0xFF, (c + 170) & 0xFF)
                if tex:
                    prims.append(poly(0x3E, [(x, y, 0, 0, col), (x + 8, y, 7, 0, col), (x, y + 8, 0, 7, col),
                                             (x + 8, y + 8, 7, 7, col)], page=pg))
                else:
                    prims.append(poly(0x3A, [(x, y, col), (x + 8, y, col), (x, y + 8, col), (x + 8, y + 8, col)]))
            if tex:   # a fill has no STP bit: the texels are an opaque tile drawn with the mask bit set
                prims = [e3(*TEX), e4(TEX[0] + 63, TEX[1] + 63), e5(0, 0), e6(1, 0),
                         rect(0x60, TEX[0], TEX[1], 64, 64, (128, 128, 128)), e6(), e3(*T),
                         e4(T[0] + TS - 1, T[1] + TS - 1), e5(*T)] + prims[1:]
            b.draw([[0x01000000] + target_state(page=pg, dither=1)] + [[p] if isinstance(p, int) else p for p in prims]
                   + [[e6()]])
            read_target(b)
            out.append(b)
    for k in range(2):
        b = Builder(f"dither_gouraud_{k}", "Gouraud quads of one colour each (0..255), dithered")
        load_target(b, r)
        prims = []
        for i in range(64):
            x, y, c = 8 * (i % 8), 8 * (i // 8), 4 * i + 2 * k + 1
            col = (c, (c + 85) & 0xFF, (c + 170) & 0xFF)
            prims.append(poly(0x38, [(x, y, col), (x + 8, y, col), (x, y + 8, col), (x + 8, y + 8, col)]))
        b.draw([[0x01000000] + target_state(dither=1)] + prims + [[e6()]])
        read_target(b)
        out.append(b)
    return out


def sprite_probe_cases(r):
    """Semi-transparent sprites in rows, odd and even widths, some clipped at the left: the emulator draws a textured
    span in pairs of pixels and its last odd pixel alone."""
    out = []
    for abr in range(4):
        for d in (0, 1):
            for raw in (0, 1):
                b = Builder(f"sprite_probe_{abr}_{d}_{raw}", f"mode {abr}, dither {d}, raw {raw}: 15-bit sprites")
                load_target(b, r)
                load_texture(b, r, 2)
                prims = []
                for i in range(8):
                    x = r.choice([-3, -2, -1, 0, 1, 2, 5, 9, 24])
                    w = r.randint(1, 15)
                    col = rand_color(r) if not raw else (128, 128, 128)
                    prims.append(rect(0x66 | raw, x, 8 * i, w, 7, col, r.randint(2, 40), r.randint(2, 50)))
                    prims.append(rect(0x66 | raw, 40, 8 * i, w, 7, col, r.randint(2, 40), r.randint(2, 50)))
                b.draw([[0x01000000] + target_state(page=tpage(*TEX_PAGE, abr, 2), dither=d)] + prims + [[e6()]])
                read_target(b)
                out.append(b)
    return out


def semi_probe_cases(r):
    """Semi-transparent modulated quads (flat and Gouraud, one colour each, 15-bit random texels), every mode, no
    dither: the blend of a modulated texel."""
    out = []
    page_of = lambda abr: tpage(*TEX_PAGE, abr, 2)
    for abr in range(4):
        for code in (0x2E, 0x3E):
            b = Builder(f"semi_probe_{abr}_{'gt' if code & 0x10 else 'ft'}", f"mode {abr} modulated quads")
            load_target(b, r)
            load_texture(b, r, 2)
            prims = []
            for i in range(64):
                x, y = 8 * (i % 8), 8 * (i // 8)
                col = rand_color(r)
                u, v = r.randint(0, 50), r.randint(0, 50)
                prims.append(poly(code, [(x, y, u, v, col), (x + 8, y, u + 8, v, col), (x, y + 8, u, v + 8, col),
                                         (x + 8, y + 8, u + 8, v + 8, col)], col, page=page_of(abr)))
            b.draw([[0x01000000] + target_state(page=page_of(abr))] + prims + [[e6()]])
            read_target(b)
            out.append(b)
    return out


def line_probe_cases(r):
    """Single opaque segments, eight per case in separate bands (x-major in 8-row bands, y-major in 8-column bands),
    both directions, all slopes: which pixels a segment covers."""
    out = []
    for k in range(24):
        b = Builder(f"line_probe_{k}", "eight separate segments: the covered pixels")
        load_target(b, r)
        prims = []
        for i in range(8):
            if k % 2 == 0:   # x-major, in rows 8i..8i+7
                x0, x1 = r.randint(-6, 30), r.randint(34, 70)
                y0, y1 = 8 * i + r.randint(0, 7), 8 * i + r.randint(0, 7)
            else:            # y-major, in columns 8i..8i+7
                y0, y1 = r.randint(-6, 30), r.randint(34, 70)
                x0, x1 = 8 * i + r.randint(0, 7), 8 * i + r.randint(0, 7)
            if r.random() < 0.5:
                x0, y0, x1, y1 = x1, y1, x0, y0
            prims.append(line(0x40, [(x0, y0), (x1, y1)], (255, 255, 255)))
        b.draw([[0x01000000] + target_state()] + prims + [[e6()]])
        read_target(b)
        out.append(b)
    for k in range(24):
        b = Builder(f"line_gprobe_{k}", "eight separate Gouraud segments, (0, 255, 128) to (255, 0, 64): the colours")
        load_target(b, r)
        prims = []
        for i in range(8):
            if k % 2 == 0:
                x0, x1 = r.randint(-6, 30), r.randint(34, 70)
                y0, y1 = 8 * i + r.randint(0, 7), 8 * i + r.randint(0, 7)
            else:
                y0, y1 = r.randint(-6, 30), r.randint(34, 70)
                x0, x1 = 8 * i + r.randint(0, 7), 8 * i + r.randint(0, 7)
            if r.random() < 0.5:
                x0, y0, x1, y1 = x1, y1, x0, y0
            prims.append(line(0x50, [(x0, y0, (0, 255, 128)), (x1, y1, (255, 0, 64))]))
        b.draw([[0x01000000] + target_state()] + prims + [[e6()]])
        read_target(b)
        out.append(b)
    return out


def finalize(builders):
    fixture = [Write(f"0x{CODE_ADDR:08X}", 0, assemble(), "the pattern routine and call5 (tests/golden/families/gpu.py)")]
    saves = [(f"0x{PAT_ADDR:08X}", PAT_SIZE), (f"0x{OUT_ADDR:08X}", OUT_SIZE)]
    out = []
    for b in builders:
        case = finish(b.case())
        case.fixture = fixture
        case.saves = saves
        out.append(case)
    return out


def cases(sym):
    r = random.Random(SEED)
    builders = (ramp_cases(random.Random(SEED + 1)) + axis_cases(random.Random(SEED + 2))
                + dither_cases(random.Random(SEED + 3)) + line_probe_cases(random.Random(SEED + 4))
                + sprite_probe_cases(random.Random(SEED + 5)) + semi_probe_cases(random.Random(SEED + 6)) + poly_cases(r)
                + rect_cases(r) + line_cases(r) + fill_cases(r) + copy_cases(r) + window_cases(r) + edge_cases(r)
                + api_cases(r))
    return finalize(builders)
