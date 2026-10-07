"""LIBGS's view: GsSetRefView2 and GsGetLw, and the LIBGTE functions they call (MulMatrix, MulMatrix2, ApplyMatrixLV,
TransposeMatrix, SquareRoot0), for the port's port/psyq/libgs.c and libgte.c (issue #7: the battle camera;
tests/host/libgs_replay.py replays these cases).

Every call goes through the gte family's "wrap" routine (tests/golden/families/gte.py: its code is this family's fixture
too): the GTE registers are loaded from a given set first, the function runs, the 64 registers are stored after it and
the GTE state the game had is put back. So each golden is the function's memory output, v0 and the GTE state it leaves.

GsSetRefView2/GsGetLw cases use one scratch buffer, `view` (the first buffer: at SCRATCH_BASE), laid out as
VIEW_LAYOUT says: the GsRVIEW2, up to four GsCOORDINATE2 (their super pointers point into the buffer) and a MATRIX.
Their reads: the world-screen matrix D_80081358 and its copy D_80081338 (the function's result), the buffer (each
coordinate system's flg and workm, which GsGetLw updates). The state they start from is the boot's: GsInitGraph(320,
240) left the view base D_80081398 the identity and PSDCNT (D_800812D8) 1 (the first case reads both).
Cases: the first battle's camera (FIGHTSTG's vp/vr with super rotations taken from the emulator along its swing) and
model view; no super; twists; vp = vr (returns 1); a vertical line of sight (no y turn); coordinates past 15 bits (the
scale-down); random views; hierarchies of 2..4 systems with every flg combination GsGetLw distinguishes (0 = changed,
PSDCNT = up to date, anything else); GsGetLw on its own. The view points stay within +-0x3000 of each other after the
scale-down, so the sum of squares stays below 2^31 (a larger one makes SquareRoot0 read past its table on the PS1).
LIBGTE: random and edge matrices, the aliased calls (MulMatrix(m, m), TransposeMatrix(m, m)), 32-bit vectors with
large components for ApplyMatrixLV, SquareRoot0 of 0, 1, the table's edges and powers of two up to 2^31 - 1.
GsSetFlatLight (issue #19: the battle's lights): each case is a run of calls on a 16-byte GsF_LIGHT buffer per light,
starting from GsInitGraph's zero matrices (the case restores them). Reads: the light matrix D_800812F8 (row id = the
direction normalised to 4096 and negated), the light colour matrix D_80081318 (column id = the colour, (c << 12) / 255)
and the GTE registers (LCM, which the function loads). Cases: every distinct lighting of FIGHTSTG's stage table (disc
file 0x1CB: directions at scale 12800..22170, several zero third lights, which return -1 and leave their row alone),
a zero light over a set row, ids outside 0..2 (nothing changes, LCM still loaded, returns 0), unit and axis vectors,
tiny and large directions (the sum of squares stays below 2^31: SquareRoot0's table), every colour edge, random lights.
"""
import random
import struct

from oracle import SCRATCH_BASE, Call, Case, Read, Write
import gte

COMMENT = ("LIBGS's GsSetRefView2 and GsGetLw (the battle camera's world-screen matrix D_80081358, its copy "
           "D_80081338, the coordinate hierarchy's flg/workm), GsSetFlatLight (the light matrix D_800812F8 and the "
           "light colour matrix D_80081318, the GTE's LCM) and the LIBGTE functions they call, each through "
           "gte.py's wrapper: the GTE registers before and after are part of the golden.")
RESIDENT = "CNTY_SEL loaded; the wrapper routine is the gte family's code in scratch RAM"
SEED = 0x6E5F2003

WS_ADDR = 0x80081358     # D_80081358: the world-screen matrix
WS_COPY_ADDR = 0x80081338  # D_80081338: GsSetRefView2's copy of it
VIEW_BASE_ADDR = 0x80081398  # D_80081398: the view's base (GsInitGraph)
PSDCNT_ADDR = 0x800812D8
LW_STACK_ADDR = 0x800813B8   # GsGetLw's walk (100 pointers)
LIGHT_ADDR = 0x800812F8    # D_800812F8: the light matrix (GsSetFlatLight's directions as rows)
COLOR_ADDR = 0x80081318    # D_80081318: the light colour matrix (the colours as columns), right after it

# FIGHTSTG's stage lighting (FightstgStageRecord.lights, disc file 0x1CB: the first record with each distinct setup):
# three (vx, vy, vz, r, g, b) lights; the ambient colour goes to SetBackColor, not LIBGS.
STAGE_LIGHTS = {
    0: [(0, 12800, 0, 128, 128, 128), (-12800, 0, 0, 55, 55, 55), (12800, 0, 0, 55, 55, 55)],
    2: [(6400, 12800, 6400, 141, 141, 141), (0, 6400, -12800, 77, 77, 77), (-6400, -12800, 0, 55, 53, 43)],
    3: [(6400, 12800, 6400, 128, 128, 128), (0, 6400, -12800, 55, 55, 55), (-6400, -12800, 0, 55, 50, 43)],
    4: [(6400, 12800, 6400, 90, 90, 90), (-12800, -12800, 0, 55, 50, 50), (12800, 6400, -12800, 31, 34, 38)],
    5: [(-12800, 12800, 0, 204, 204, 204), (6400, 6400, -12800, 102, 102, 102), (6400, -12800, 12800, 51, 43, 31)],
    6: [(6400, 12800, 6400, 90, 90, 90), (-12800, 0, -12800, 26, 43, 71), (-6400, -12800, 0, 23, 23, 23)],
    7: [(12800, 12800, -6400, 77, 77, 77), (-12800, -12800, 12800, 23, 15, 26), (0, 0, 0, 0, 0, 0)],
    8: [(-6400, 12800, 6400, 90, 90, 90), (12800, 6400, -12800, 55, 55, 55), (-12800, -12800, 0, 38, 38, 38)],
    9: [(6400, 12800, -6400, 90, 90, 90), (-12800, -12800, 12800, 23, 15, 26), (0, 0, 0, 0, 0, 0)],
    10: [(12800, 12800, 12800, 90, 90, 90), (-12800, -12800, 12800, 25, 50, 50), (0, 6400, -6400, 25, 25, 50)],
    11: [(6400, 12800, 6400, 128, 128, 153), (-6400, -12800, 0, 12, 25, 25), (-6400, 6400, -12800, 50, 100, 87)],
    12: [(-6400, 12800, 6400, 90, 90, 90), (6400, 12800, -6400, 71, 71, 71), (-12800, -12800, 0, 38, 38, 38)],
    14: [(-6400, 12800, 6400, 128, 128, 128), (6400, 12800, -6400, 38, 38, 38), (-12800, -12800, 0, 55, 55, 55)],
    15: [(-12800, 12800, 12800, 55, 55, 55), (6400, 12800, -6400, 102, 102, 102), (0, -12800, 0, 55, 55, 55)],
    16: [(-6400, 12800, 6400, 90, 90, 90), (-12800, -12800, -12800, 23, 23, 23), (0, 0, 0, 0, 0, 0)],
    17: [(12800, 12800, 0, 242, 242, 242), (-12800, -12800, 0, 25, 50, 102), (0, 0, 0, 0, 0, 0)],
    18: [(0, 12800, 12800, 90, 90, 90), (6400, -12800, 0, 77, 41, 102), (-6400, 12800, -12800, 71, 69, 102)],
    19: [(12800, 12800, 12800, 90, 90, 90), (-6400, 6400, -12800, 85, 61, 102), (-12800, -12800, 6400, 69, 69, 69)],
    20: [(-6400, 12800, 6400, 102, 102, 102), (6400, 12800, -6400, 71, 71, 71), (-12800, -12800, 0, 38, 38, 38)],
    22: [(12800, 12800, -6400, 242, 242, 242), (-12800, -12800, 6400, 25, 50, 102), (0, 0, 0, 0, 0, 0)],
    23: [(12800, 12800, 12800, 128, 128, 128), (0, 0, -12800, 55, 55, 55), (-12800, -6400, 0, 20, 40, 51)],
    24: [(6400, -12800, -12800, 178, 90, 90), (0, -6400, 12800, 77, 77, 77), (-6400, 12800, 0, 77, 69, 46)],
    25: [(6400, -12800, 6400, 90, 90, 128), (-12800, 12800, -6400, 55, 55, 55), (6400, -6400, -12800, 38, 38, 38)],
    26: [(6400, -12800, -6400, 128, 90, 90), (-6400, 12800, 0, 15, 15, 31), (6400, -6400, 12800, 69, 43, 69)],
    27: [(6400, -12800, 6400, 90, 90, 128), (-6400, 12800, 0, 12, 25, 25), (6400, -6400, -12800, 38, 77, 69)],
    55: [(-6400, -12800, 6400, 55, 77, 90), (6400, 12800, 6400, 128, 132, 128), (0, 0, -12800, 55, 55, 55)],
    57: [(0, 0, 0, 0, 0, 0), (0, 0, 0, 0, 0, 0), (0, 0, 0, 0, 0, 0)],
}

# The view buffer: GsRVIEW2 at 0, GsCOORDINATE2 i at COORD_OFF + i * 0x50, a MATRIX at MATRIX_OFF.
COORD_OFF, COORD_SIZE, COORDS, MATRIX_OFF, VIEW_SIZE = 0x20, 0x50, 4, 0x160, 0x180
VIEW_ADDR = SCRATCH_BASE


def coord_addr(i):
    return VIEW_ADDR + COORD_OFF + COORD_SIZE * i


def matrix(m, t, pad=0x5A5A):
    return struct.pack("<9h", *[x for row in m for x in row]) + struct.pack("<H", pad) + struct.pack("<3i", *t)


IDENTITY = [[4096, 0, 0], [0, 4096, 0], [0, 0, 4096]]
JUNK = matrix([[0x1111, 0x2222, 0x3333], [0x4444, 0x5555, 0x6666], [0x7777, -0x7777, -0x6666]], [0x11223344, -5, 7])

# FIGHTSTG's battle camera (fightstg_8008D3B4.c, the camera mover: a stack GsCOORDINATE2 from RotMatrixYXZ_gte of the
# setting's rotation, translation 0) along its swing in first_battle_save, and the model view (fightstg_model_view_set).
BATTLE_VIEW = (-2432, -1920, -11136, 1408, -2496, 4864)
BATTLE_SUPERS = [
    [[-4096, 0, 0], [0, 3548, 2046], [0, 2046, -3548]],
    [[-3920, -543, 1057], [0, 3644, 1870], [-1189, 1789, -3488]],
    [[-2440, -1180, 3071], [0, 3824, 1468], [-3290, 874, -2278]],
    [[0, -1056, 3958], [0, 3958, 1056], [-4096, 0, 0]],
    [[2598, -460, 3132], [0, 4053, 595], [-3166, -378, 2570]],
    [[4076, -7, 401], [0, 4096, 63], [-401, -63, 4076]],
    IDENTITY,
]
MODEL_VIEW = (-1972, -1066, -2767, 0, -960, -5120)


def coord(flg, m, t, workm=JUNK, super_index=None, pad=0x5A5A):
    """A GsCOORDINATE2's bytes: flg, coord, workm, param (a dummy non-NULL), super (an index into the buffer or None),
    sub (NULL)."""
    sup = coord_addr(super_index) if super_index is not None else 0
    return struct.pack("<I", flg) + matrix(m, t, pad) + workm + struct.pack("<3I", 0x13572468, sup, 0)


def view_buffer(vp_vr, rz, super_index, coords, m=JUNK):
    """The view buffer: the GsRVIEW2 (vp, vr, rz, super), the coordinate systems (bytes, or None), the MATRIX."""
    buf = bytearray(VIEW_SIZE)
    sup = coord_addr(super_index) if super_index is not None else 0
    buf[0:0x20] = struct.pack("<7iI", *vp_vr, rz, sup)
    for i, c in enumerate(coords):
        if c is not None:
            buf[COORD_OFF + COORD_SIZE * i:COORD_OFF + COORD_SIZE * (i + 1)] = c
    buf[MATRIX_OFF:MATRIX_OFF + 0x20] = m
    return bytes(buf)


class Ctx:
    def __init__(self, sym):
        self.code, self.labels = gte.assemble()
        self.gen = gte.Gen(SEED)
        self.r = random.Random(SEED)
        self.sym = sym

    def wrap(self, args, fn, reads, ret_type="void", comment=""):
        d, c = self.gen.typical()
        return Call(f"0x{self.labels['wrap']:08X}", args + [self.sym[fn]], ret_type,
                    reads + [Read(f"0x{gte.OUT_ADDR:08X}", 0, 256, "the 64 GTE registers after the call")],
                    comment=f"{fn} through the wrapper" + (f": {comment}" if comment else ""),
                    writes=[Write(f"0x{gte.GTE_IN_ADDR:08X}", 0, gte.regs_bytes(d, c),
                                  "the GTE registers before the call")])

    def s16(self):
        return self.gen.s16()

    def rot(self):
        """A rotation matrix (the product of three axis rotations, in floats rounded: near-orthonormal)."""
        import math
        a, b, c = (self.r.uniform(0, 2 * math.pi) for _ in range(3))
        ca, sa, cb, sb, cc, sc = math.cos(a), math.sin(a), math.cos(b), math.sin(b), math.cos(c), math.sin(c)
        ry = [[ca, 0, sa], [0, 1, 0], [-sa, 0, ca]]
        rx = [[1, 0, 0], [0, cb, -sb], [0, sb, cb]]
        rz = [[cc, -sc, 0], [sc, cc, 0], [0, 0, 1]]

        def mul(p, q):
            return [[sum(p[i][k] * q[k][j] for k in range(3)) for j in range(3)] for i in range(3)]
        m = mul(mul(ry, rx), rz)
        return [[int(round(4096 * x)) for x in row] for row in m]


def view_reads(first=False):
    reads = [Read(f"0x{WS_ADDR:08X}", 0, 32, "D_80081358: the world-screen matrix"),
             Read(f"0x{WS_COPY_ADDR:08X}", 0, 32, "D_80081338: its copy"),
             Read("buf:view", 0, VIEW_SIZE, "the view buffer: the coordinate systems' flg and workm, the MATRIX")]
    if first:
        reads = [Read(f"0x{VIEW_BASE_ADDR:08X}", 0, 32, "D_80081398: the view base GsInitGraph left"),
                 Read(f"0x{PSDCNT_ADDR:08X}", 0, 4, "PSDCNT")] + reads
    return reads


def view_case(ctx, name, vp_vr, rz, super_index, coords, comment, first=False):
    call = ctx.wrap([VIEW_ADDR, 0, 0], "GsSetRefView2", view_reads(first), "s32", comment)
    return Case(name, [call], buffers={"view": view_buffer(vp_vr, rz, super_index, coords)}, comment=comment)


def view_cases(ctx):
    out, r = [], ctx.r
    for k, m in enumerate(BATTLE_SUPERS):
        out.append(view_case(ctx, f"battle_camera_{k}", BATTLE_VIEW, 0, 0, [coord(0, m, [0, 0, 0])],
                             "FIGHTSTG's battle camera, super rotated along its swing", first=(k == 0)))
    out.append(view_case(ctx, "battle_model_view", MODEL_VIEW, 0, 0, [coord(0, IDENTITY, [0, 0, 0])],
                         "FIGHTSTG's model view (fightstg_model_view_set)"))
    out.append(view_case(ctx, "no_super", BATTLE_VIEW, 0, None, [], "super NULL: the world's coordinates"))
    for k, rz in enumerate([4096, -4096, 90 << 12, 45 << 12, 359 << 12, 1000, -123456, 360 << 12, 720 << 12 | 5]):
        out.append(view_case(ctx, f"twist_{k}", BATTLE_VIEW, rz, 0, [coord(0, BATTLE_SUPERS[2], [100, -200, 300])],
                             f"rz = {rz}"))
    out.append(view_case(ctx, "same_point", (100, 200, 300, 100, 200, 300), 0, 0, [coord(0, IDENTITY, [0, 0, 0])],
                         "vp = vr: returns 1, the matrix half built"))
    out.append(view_case(ctx, "same_point_twist", (5, 5, 5, 5, 5, 5), 30 << 12, None, [], "vp = vr with a twist"))
    out.append(view_case(ctx, "look_down", (0, -2000, 0, 0, 0, 0), 0, None, [], "straight down: no y turn"))
    out.append(view_case(ctx, "look_up", (300, 0, -400, 300, -5000, -400), 0, 0, [coord(0, IDENTITY, [7, 8, 9])],
                         "straight up: no y turn"))
    big = [(0x40000, 0x20000, -0x30000, 0x42000, 0x21000, -0x31000), (0x7FFF, 0, 0, 0x4000, 0x1000, -0x2000),
           (0x8000, 0, 0, 0x6000, 0, 0x1000), (-0x100000, 0x80000, 0x10000, -0xFE000, 0x81000, 0x12000),
           (0x7FFFFF0, -0x7FFFFF0, 0x1000000, 0x7FFF000, -0x7FFF000, 0x1001000)]
    for k, v in enumerate(big):
        m = ctx.rot()
        t = [r.randint(-5000, 5000) for _ in range(3)]
        out.append(view_case(ctx, f"scaled_{k}", v, 0, 0, [coord(0, m, t)],
                             "coordinates past 15 bits: scaled down together"))
    for k in range(40):
        vp = [r.randint(-0x1800, 0x1800) for _ in range(3)]
        vr = [r.randint(-0x1800, 0x1800) for _ in range(3)]
        rz = r.choice([0, 0, r.randint(-360 << 12, 360 << 12), r.randint(-0x8000000, 0x8000000)])
        t = [r.choice([0, r.randint(-0x8000, 0x8000), r.randint(-0x400000, 0x400000)]) for _ in range(3)]
        m = ctx.rot() if k % 4 else [[ctx.s16() for _ in range(3)] for _ in range(3)]
        sup = None if k % 7 == 3 else 0
        out.append(view_case(ctx, f"random_{k}", vp + vr, rz, sup, [coord(0, m, t)],
                             "random view" + ("" if k % 4 else ", a super that is not a rotation")))
    return out


def hierarchy_cases(ctx):
    """Chains of coordinate systems: 0 -> 1 -> ... (super = the next), every flg pattern GsGetLw tells apart."""
    out, r = [], ctx.r
    patterns = [(0, 0), (0, 1), (1, 0), (1, 1), (0, 5), (5, 0), (5, 5), (0, 0, 0), (0, 1, 0), (0, 5, 0), (5, 0, 0),
                (5, 5, 0), (0, 0, 5), (5, 5, 5), (1, 0, 0), (0, 0, 0, 0), (5, 0, 5, 0), (0, 5, 0, 5), (5, 5, 5, 5),
                (0, 0, 1, 0)]
    for k, flgs in enumerate(patterns):
        n = len(flgs)
        coords = []
        for i, f in enumerate(flgs):
            workm = matrix(ctx.rot(), [r.randint(-3000, 3000) for _ in range(3)])
            coords.append(coord(f, ctx.rot(), [r.randint(-3000, 3000) for _ in range(3)], workm,
                                i + 1 if i + 1 < n else None))
        vp = [r.randint(-0x1800, 0x1800) for _ in range(3)]
        vr = [r.randint(-0x1800, 0x1800) for _ in range(3)]
        text = f"flg {'/'.join(str(f) for f in flgs)} from the view's super up to the root"
        out.append(view_case(ctx, f"hierarchy_{k}", vp + vr, 0, 0, coords, text))
        call = ctx.wrap([coord_addr(0), VIEW_ADDR + MATRIX_OFF, 0], "GsGetLw",
                        [Read("buf:view", 0, VIEW_SIZE, "the coordinate systems and the MATRIX")], comment=text)
        out.append(Case(f"getlw_{k}", [call], buffers={"view": view_buffer((0,) * 6, 0, None, coords)}, comment=text))
    return out


def library_cases(ctx):
    out, r = [], ctx.r

    def entry():
        return r.randint(-4096, 4096) if r.random() < 0.6 else ctx.s16()

    def mat():
        m = [[entry() for _ in range(3)] for _ in range(3)]
        return matrix(m, [ctx.gen.s32() for _ in range(3)], r.getrandbits(16))

    for fn in ("MulMatrix", "MulMatrix2"):
        for k in range(24):
            m0 = matrix(ctx.rot(), [1, 2, 3]) if k % 3 == 0 else mat()
            m1 = matrix(ctx.rot(), [4, 5, 6]) if k % 3 == 0 else mat()
            out.append(Case(f"{fn}_{k}", [ctx.wrap([("buf", "m0"), ("buf", "m1"), 0], fn,
                                                   [Read("buf:m0", 0, 32, "m0"), Read("buf:m1", 0, 32, "m1")])],
                            buffers={"m0": m0, "m1": m1}))
        for k in range(4):
            out.append(Case(f"{fn}_alias_{k}", [ctx.wrap([("buf", "m"), ("buf", "m"), 0], fn,
                                                         [Read("buf:m", 0, 32, "m")], comment="m0 = m1")],
                            buffers={"m": mat()}))
    edges = [0, 1, -1, 0x7FFF, -0x8000, 0x8000, -0x8001, 0x7FFFFFFF, -0x80000000, 0x12345678, -0x12345678]
    for k in range(40):
        comps = [r.choice(edges) if k % 4 == 0 else ctx.gen.s32() for _ in range(3)]
        v = struct.pack("<4i", *comps, 0x2468)
        alias = k % 10 == 9
        args = [("buf", "m"), ("buf", "v0"), ("buf", "v0" if alias else "v1")]
        reads = [Read("buf:v0", 0, 16, "v0")] + ([] if alias else [Read("buf:v1", 0, 16, "v1")])
        call = ctx.wrap(args, "ApplyMatrixLV", reads, comment="v1 = v0" if alias else "")
        out.append(Case(f"ApplyMatrixLV_{k}", [call],
                        buffers={"m": mat(), "v0": v, "v1": struct.pack("<4I", 0x11111111, 0x22222222, 0x33333333,
                                                                        0x44444444)}))
    for k in range(8):
        out.append(Case(f"TransposeMatrix_{k}", [ctx.wrap([("buf", "m0"), ("buf", "m1"), 0], "TransposeMatrix",
                                                          [Read("buf:m0", 0, 32, "m0"), Read("buf:m1", 0, 32, "m1")])],
                        buffers={"m0": mat(), "m1": mat()}))
    for k in range(2):
        out.append(Case(f"TransposeMatrix_alias_{k}", [ctx.wrap([("buf", "m"), ("buf", "m"), 0], "TransposeMatrix",
                                                                [Read("buf:m", 0, 32, "m")], comment="m0 = m1")],
                        buffers={"m": mat()}))
    values = [0, 1, 2, 3, 4, 63, 64, 65, 255, 256, 257, 1023, 1024, 4095, 4096, 0x10000, 0xFFFFFF, 0x1000000,
              0x7FFFFFFF, 0x40000000, 0x3FFFFFFF]
    values += [1 << b for b in range(31)] + [(1 << b) - 1 for b in range(2, 31)]
    values += [r.randint(0, 0x7FFFFFFF) for _ in range(30)] + [r.randint(0, 0xFFFF) for _ in range(20)]
    for k in range(0, len(values), 8):
        chunk = values[k:k + 8]
        out.append(Case(f"SquareRoot0_{k // 8}", [ctx.wrap([v, 0, 0], "SquareRoot0", [], "s32", comment=f"a = {v}")
                                                  for v in chunk]))
    return out


def light_bytes(vx, vy, vz, r, g, b):
    """A GsF_LIGHT: s32 vx, vy, vz; u8 r, g, b and a pad byte."""
    return struct.pack("<3i3Bx", vx, vy, vz, r, g, b)


def light_case(ctx, name, lights, comment):
    """GsSetFlatLight for each (id, light) in order, one buffer per call, the matrices read after each call."""
    reads = [Read(f"0x{LIGHT_ADDR:08X}", 0, 32, "D_800812F8: the light matrix"),
             Read(f"0x{COLOR_ADDR:08X}", 0, 32, "D_80081318: the light colour matrix")]
    calls, buffers = [], {}
    for k, (lid, light) in enumerate(lights):
        buffers[f"l{k}"] = light_bytes(*light)
        calls.append(ctx.wrap([lid, ("buf", f"l{k}"), 0], "GsSetFlatLight", reads, "s32",
                              f"id {lid}, direction {light[:3]}, colour {light[3:]}"))
    return Case(name, calls, buffers=buffers, comment=comment)


def light_cases(ctx):
    out, r = [], ctx.r
    for idx, lights in STAGE_LIGHTS.items():
        out.append(light_case(ctx, f"stage_lights_{idx}", list(enumerate(lights)),
                              f"FIGHTSTG's stage record {idx}: its three lights in order"
                              + (" (a zero light leaves its row alone, returns -1)" if (0, 0, 0) in
                                 [l[:3] for l in lights] else "")))
    set_row = (6400, 12800, 6400, 90, 90, 90)
    out.append(light_case(ctx, "zero_over_set_row", [(1, set_row), (1, (0, 0, 0, 200, 100, 50))],
                          "a zero direction after a set row: returns -1, row and column stay"))
    out.append(light_case(ctx, "id_out_of_range", [(0, set_row), (3, (100, 200, 300, 10, 20, 30)),
                                                   (-1, (100, 200, 300, 10, 20, 30)), (7, (0, 0, 0, 1, 1, 1))],
                          "ids 3 and -1 change nothing but load LCM and return 0; a zero light with id 7 returns -1"))
    units = [(4096, 0, 0), (0, 4096, 0), (0, 0, 4096), (-4096, 0, 0), (0, -4096, 0), (0, 0, -4096), (1, 0, 0),
             (0, 0, -1), (3, 4, 0), (-3, 0, 4), (1, 1, 1), (-1, -1, -1), (2365, 2365, 2365), (26000, 26000, 26000),
             (-26000, 26000, -26000), (32767, 0, 0), (-32768, 0, 0), (0, 32767, 32767), (12800, 12800, 12800),
             (0, 0, 7)]
    out.append(light_case(ctx, "axes_and_sizes", [(k % 3, v + (255, 128, 0)) for k, v in enumerate(units)],
                          "unit and axis vectors, tiny and large directions (the squares' sum below 2^31)"))
    colours = [(0, 0, 0), (255, 255, 255), (1, 2, 3), (127, 128, 129), (254, 255, 0), (64, 32, 16), (200, 100, 50),
               (255, 0, 255), (85, 170, 255)]
    out.append(light_case(ctx, "colours", [(k % 3, (0, -12800, 0) + c) for k, c in enumerate(colours)],
                          "every colour edge: (c << 12) / 255 into column id"))
    for k in range(6):
        lights = []
        for j in range(6):
            v = tuple(r.randint(-26000, 26000) if r.random() < 0.7 else r.choice([0, 1, -1, 12800, -12800])
                      for _ in range(3))
            lights.append((r.choice([0, 1, 2, 0, 1, 2, 3, -1]), v + tuple(r.randint(0, 255) for _ in range(3))))
        out.append(light_case(ctx, f"random_lights_{k}", lights, "random lights, ids and colours"))
    return out


def cases(sym):
    ctx = Ctx(sym)
    out = view_cases(ctx) + hierarchy_cases(ctx) + library_cases(ctx) + light_cases(ctx)
    fixture = [Write(f"0x{gte.CODE_ADDR:08X}", 0, ctx.code, "the gte family's routines (gte.py assemble: the wrapper)")]
    saves = [(f"0x{gte.OUT_ADDR:08X}", 0x200), (f"0x{WS_COPY_ADDR:08X}", 0x40), (f"0x{LW_STACK_ADDR:08X}", 400),
             (f"0x{LIGHT_ADDR:08X}", 0x40)]
    for case in out:
        case.fixture = fixture
        case.saves = saves + case.saves
    return out
