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
"""
import random
import struct

from oracle import SCRATCH_BASE, Call, Case, Read, Write
import gte

COMMENT = ("LIBGS's GsSetRefView2 and GsGetLw (the battle camera's world-screen matrix D_80081358, its copy "
           "D_80081338, the coordinate hierarchy's flg/workm) and the LIBGTE functions they call, each through "
           "gte.py's wrapper: the GTE registers before and after are part of the golden.")
RESIDENT = "CNTY_SEL loaded; the wrapper routine is the gte family's code in scratch RAM"
SEED = 0x6E5F2003

WS_ADDR = 0x80081358     # D_80081358: the world-screen matrix
WS_COPY_ADDR = 0x80081338  # D_80081338: GsSetRefView2's copy of it
VIEW_BASE_ADDR = 0x80081398  # D_80081398: the view's base (GsInitGraph)
PSDCNT_ADDR = 0x800812D8
LW_STACK_ADDR = 0x800813B8   # GsGetLw's walk (100 pointers)

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


def cases(sym):
    ctx = Ctx(sym)
    out = view_cases(ctx) + hierarchy_cases(ctx) + library_cases(ctx)
    fixture = [Write(f"0x{gte.CODE_ADDR:08X}", 0, ctx.code, "the gte family's routines (gte.py assemble: the wrapper)")]
    saves = [(f"0x{gte.OUT_ADDR:08X}", 0x200), (f"0x{WS_COPY_ADDR:08X}", 0x40), (f"0x{LW_STACK_ADDR:08X}", 400)]
    for case in out:
        case.fixture = fixture
        case.saves = saves + case.saves
    return out
