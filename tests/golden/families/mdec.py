"""LIBPRESS and the MDEC (the PS1's macroblock decoder), for the port's movie decoder (port/psyq/libpress.c,
port/psyq/mdec.c; tests/host/mdec_replay.py replays these cases through it).

STDWTITL.PRO (the title overlay, which links LIBPRESS: stdwtitl_80082D70.c's movie player) is put in the overlay slot by
a kept setup case. Then:
- vlc cases: a frame of a movie on the disc (AAA/STR/MOVIE*.STR: its video sectors' 2016-byte payloads in sector order,
  the bytes StGetNext hands the player) goes into RAM; DecDCTvlcBuild builds LIBPRESS's lookup table and DecDCTvlc2
  turns the bit stream into the MDEC's run-level words: the golden is those words (a SHA-1: they are the frame's data)
  and the bytes after them (a fill of 0xA5 shows how far the decoder wrote).
- the same cases then decode that frame with the MDEC as the player does: DecDCTReset(0), DecDCTin(words, mode),
  then per 16-pixel column of macroblocks DecDCTout(out, words) and DecDCToutSync(0): the golden is each column's
  pixels (a SHA-1 each). Mode 3 is the game's (24-bit); one frame is also decoded in mode 2 and 0 (15-bit, bit 15
  set or clear) for a few columns.
- rl cases: run-level words made up here (no disc data): random coefficients with every quantisation scale, q 0
  (the MDEC's "no quant table" form), saturating values, runs past the 64th coefficient, 0xFE00 padding between blocks.

The frames are not in the golden (no game data in the repo): their fixture writes keep a `source` text ("MOVIEE03.STR
frame 100"), the size and the SHA-1; frame_bitstream() rebuilds the bytes from the disc for the oracle and the host.
The MDEC is PCSX-Redux's emulation: its IDCT and colour conversion are not the hardware's bit for bit (psx-spx
"MDEC"), so the pixels are compared by tests/host/mdec_replay.py with the differences it accepts listed there.
"""
import random
import struct
from pathlib import Path

from oracle import ROOT, Call, Case, Read, Write

COMMENT = ("LIBPRESS and the MDEC: DecDCTvlc2 on real movie frames from the disc (the run-level words), then "
           "DecDCTReset/DecDCTin/DecDCTout/DecDCToutSync column by column (24-bit as the game, 15-bit modes 0 and 2); "
           "made-up run-level data for the MDEC's edge cases.")
RESIDENT = "CNTY_SEL loaded, then STDWTITL.PRO (LIBPRESS) put in the overlay slot by the setup case"

OVERLAY_SLOT = 0x80082CB0
STDWTITL_PRO = "extracted/disc/AAA/PRO/STDWTITL.PRO"
STR_DIR = ROOT / "extracted/disc/AAA/STR"
# RAM the cases use (saved and restored by the oracle: heap far above STDWTITL's 26 KB, below the oracle's scratch).
TABLE_ADDR = 0x80100000    # DecDCTvlcBuild's table (the game allocates 0x11000 bytes)
TABLE_SIZE = 0x11000
BS_ADDR = 0x80112000       # the frame's bit stream (at most 9 sectors x 2016 bytes)
BS_SIZE = 0x4800
RL_ADDR = 0x80118000       # the run-level words (the game allocates 0x28000 bytes)
RL_SIZE = 0x14000
OUT_ADDR = 0x8012C000      # one column of pixels (the game's image buffers are 0x4E00 bytes: 16 x 416 x 3)
OUT_SIZE = 0x4E00
TAIL = 0x100               # bytes after the run-level words filled with 0xA5 and read back
SECTOR = 2336              # dumpsxiso's MODE2 sectors: 8-byte subheader + 2328 bytes
ST_MAGIC = 0x80010160
ST_DATA = 2016             # MDEC bytes per video sector, after the 32-byte StHEADER

# (file, frame): the opening (MOVIEOPN: the first frames, quantiser 1), the English opening movie (MOVIEE03, what
# new_game plays) at an ordinary frame, its largest frame and a frame with the largest quantiser, and an event movie's
# largest frame.
FRAMES = [("MOVIEOPN.STR", 3), ("MOVIEE03.STR", 100), ("MOVIEE03.STR", 1383), ("MOVIEE03.STR", 1480),
          ("MOVIED01.STR", 397)]
MODE_FRAME = ("MOVIEE03.STR", 100)   # also decoded in 15-bit modes 0 and 2
MODE_COLUMNS = 3
SEED = 0x3DEC2003


def frame_sectors(name):
    """{frame number: (StHEADER fields, [payload of sector 0, 1, ...])} for every video frame of a movie."""
    data = (STR_DIR / name).read_bytes()
    frames = {}
    for i in range(len(data) // SECTOR):
        d = data[i * SECTOR + 8:(i + 1) * SECTOR]
        if struct.unpack_from("<I", d, 0)[0] != ST_MAGIC:
            continue
        idx, cnt, fr, size, w, h = struct.unpack_from("<HHIIHH", d, 4)
        hdr, parts = frames.setdefault(fr, ({"sectors": cnt, "size": size, "width": w, "height": h}, {}))
        parts[idx] = d[32:32 + ST_DATA]
    return {fr: (hdr, [parts[k] for k in sorted(parts)]) for fr, (hdr, parts) in frames.items()}


def frame_bitstream(name, frame):
    """A frame's MDEC data as StGetNext hands it to the player (port/psyq/libcd.c does the same), and its header."""
    hdr, parts = frame_sectors(name)[frame]
    assert len(parts) == hdr["sectors"], (name, frame)
    return b"".join(parts), hdr


def source_text(name, frame):
    return f"{name} frame {frame}: the video sectors' 2016-byte payloads in sector order"


def setup_case():
    return Case("setup_stdwtitl", [Call("DecDCTReset", [0], "void", comment="smoke: LIBPRESS answers")],
                fixture=[Write(OVERLAY_SLOT, 0, b"", "overlay slot: STDWTITL.PRO", file=STDWTITL_PRO)],
                keep=True, comment="setup, kept for the family: STDWTITL.PRO (LIBPRESS) in the overlay slot")


def saves():
    return [(f"0x{TABLE_ADDR:08X}", TABLE_SIZE), (f"0x{BS_ADDR:08X}", BS_SIZE), (f"0x{RL_ADDR:08X}", RL_SIZE),
            (f"0x{OUT_ADDR:08X}", OUT_SIZE)]


def column_words(height, mode):
    """DecDCTout's size for one 16-pixel column: 24-bit (mode bit 0) 16 x h x 3 bytes, else 16 x h x 2."""
    return 16 * height * (3 if mode & 1 else 2) // 4


def decode_calls(width, height, mode, columns=None):
    """DecDCTReset(0), DecDCTin(RL, mode), then per column DecDCTout + DecDCToutSync, each column read back."""
    words = column_words(height, mode)
    calls = [Call("DecDCTReset", [0], "void"),
             Call("DecDCTin", [RL_ADDR, mode], "void", reads=[Read(f"0x{RL_ADDR:08X}", 0, 4, "the MDEC command word")])]
    for col in range(columns if columns is not None else width // 16):
        calls.append(Call("DecDCTout", [OUT_ADDR, words], "void", comment=f"column {col}"))
        calls.append(Call("DecDCToutSync", [0], "s32",
                          reads=[Read(f"0x{OUT_ADDR:08X}", 0, words * 4, f"column {col}: {words} words")]))
    return calls


def frame_case(name, frame, mode=3, columns=None):
    bs, hdr = frame_bitstream(name, frame)
    rl_words = struct.unpack_from("<H", bs, 0)[0]
    rl_bytes = 4 + 4 * rl_words
    calls = [Call("DecDCTvlcBuild", [TABLE_ADDR], "void"),
             Call("DecDCTvlc2", [BS_ADDR, RL_ADDR, TABLE_ADDR], "s32",
                  reads=[Read(f"0x{RL_ADDR:08X}", 0, rl_bytes, "the run-level words (header word + size words)"),
                         Read(f"0x{RL_ADDR:08X}", rl_bytes, TAIL, "after them (filled with 0xA5)")])]
    calls += decode_calls(hdr["width"], hdr["height"], mode, columns)
    tag = name.split(".")[0].lower()
    return Case(f"{tag}_f{frame}_m{mode}", calls,
                fixture=[Write(BS_ADDR, 0, bs, f"{name} frame {frame}", source=source_text(name, frame)),
                         Write(RL_ADDR, rl_bytes, b"\xA5" * TAIL, "fill after the run-level words")],
                saves=saves(),
                comment=f"{name} frame {frame}: {hdr['width']}x{hdr['height']}, q {struct.unpack_from('<H', bs, 4)[0]}, "
                        f"{rl_words} run-level words; mode {mode}")


# ------------------------------------------------------------------------------------------- made-up run-level data
# LIBPRESS's quantisation table (zig-zag order), to keep made-up coefficients in range.
QUANT = [2, 16, 16, 19, 16, 19, 22, 22, 22, 22, 22, 22, 26, 24, 26, 27, 27, 27, 26, 26, 26, 26, 27, 27, 27, 29, 29, 29,
         34, 34, 34, 29, 29, 29, 27, 27, 29, 29, 32, 32, 34, 34, 37, 38, 37, 35, 35, 34, 35, 38, 38, 40, 40, 40, 48, 48,
         46, 46, 56, 56, 58, 69, 69, 83]
RL_HEIGHT = 128            # the made-up cases decode one column of 8 macroblocks


def rl_block(r, q, chroma, kind="random"):
    """One block's halfwords: the DC word (q << 10 | dc), AC words (run << 10 | level), the end (0xFE00). The values
    keep the pixels inside the colour range (the IDCT of a coefficient F has an amplitude of at most |F| / 4, the DC's
    is F / 8): a luminance block within about +-100, a colour block within +-40, so that the result does not depend on
    how an out-of-range colour is clamped (PCSX-Redux's MDEC does not clamp as psx-spx describes: the "extreme" case)."""
    dc_max, ac_budget = (120, 40) if chroma else (240, 160)
    if kind == "extreme":
        out = [(q << 10) | r.choice([0x1FF, 0x200, 0x3FF, 0x001])]
        k = 0
        while k < 63:
            run = r.choice([0, 0, 0, 1, 5])
            if k + run + 1 > 63:
                break
            k += run + 1
            out.append((run << 10) | r.choice([0x1FF, 0x200, 0x201, 0x3FF, 0x100, 0x2FF]))
        return out + [0xFE00]
    if kind == "dc":
        return [(q << 10) | (r.randrange(-dc_max, dc_max + 1) & 0x3FF), 0xFE00]
    out = [(q << 10) | (r.randrange(-dc_max // 2, dc_max // 2 + 1) & 0x3FF)]
    k, budget = 0, ac_budget
    while True:
        run = min(int(r.expovariate(0.4)), 62)
        if k + run + 1 > 63 or r.random() < 0.06:
            break
        unit = 2 if q == 0 else (QUANT[k + run + 1] * q + 4) >> 3
        if unit == 0 or unit > budget:
            if r.random() < 0.5:
                break
            continue
        k += run + 1
        level = r.randrange(1, min(511, budget // unit) + 1)
        budget -= level * unit
        out.append((run << 10) | ((-level if r.random() < 0.5 else level) & 0x3FF))
    if kind == "overrun" and k < 50:
        # A run that takes the index past 63 instead of 0xFE00: the block ends there (psx-spx rl_decode_block).
        return out + [((63 - k) << 10) | 1]
    return out + [0xFE00]


def rl_data(r, qs, kind="random", padding=False, macroblocks=RL_HEIGHT // 16):
    """Header word 0x3800xxxx (xxxx = words) and the blocks of the macroblocks (Cr, Cb, Y1..Y4), quantisers in turn
    from qs, padded with 0xFE00 to a 32-word multiple as the movies are; padding: 0xFE00s between some blocks too."""
    hw, n = [], 0
    for _ in range(macroblocks):
        for b in range(6):
            if padding and r.random() < 0.25:
                hw += [0xFE00] * r.randrange(1, 4)
            k = kind if kind != "overrun" or r.random() < 0.5 else "random"
            hw += rl_block(r, qs[n % len(qs)], b < 2, k)
            n += 1
    if len(hw) % 2:
        hw.append(0xFE00)
    while (len(hw) // 2) % 32:
        hw += [0xFE00, 0xFE00]
    return struct.pack("<I", 0x38000000 | (len(hw) // 2)) + struct.pack(f"<{len(hw)}H", *hw)


def rl_case(name, data, mode=3, comment=""):
    """Decodes one column; the pixels come from made-up data, so the golden keeps them (reads of 256 bytes)."""
    words = column_words(RL_HEIGHT, mode)
    calls = [Call("DecDCTReset", [0], "void"),
             Call("DecDCTin", [RL_ADDR, mode], "void", reads=[Read(f"0x{RL_ADDR:08X}", 0, 4, "the MDEC command word")]),
             Call("DecDCTout", [OUT_ADDR, words], "void", comment="column 0"),
             Call("DecDCToutSync", [0], "s32",
                  reads=[Read(f"0x{OUT_ADDR:08X}", o, 256, f"column 0 bytes {o:#x}") for o in range(0, words * 4, 256)])]
    return Case(name, calls, fixture=[Write(RL_ADDR, 0, data, "run-level words (made up)")], saves=saves(),
                comment=comment)


def rl_cases(r):
    every_q = list(range(1, 64))
    return [
        rl_case("rl_q_1_31", rl_data(r, every_q[:31] + every_q[:17]), comment="quantisers 1..31 (and 1..17)"),
        rl_case("rl_q_32_63", rl_data(r, every_q[31:] + every_q[:16]), comment="quantisers 32..63 (and 1..16)"),
        rl_case("rl_q0", rl_data(r, [0]), comment="quantiser 0: the coefficients x 2, no table, in index order"),
        rl_case("rl_dc", rl_data(r, [1, 2, 7, 31, 63], "dc"), comment="DC only: flat blocks over the colour range"),
        rl_case("rl_pad", rl_data(r, [2, 10, 0], padding=True), comment="0xFE00 padding between blocks (skipped)"),
        rl_case("rl_overrun", rl_data(r, [2, 10, 0], "overrun"),
                comment="runs past the 64th coefficient (no 0xFE00): psx-spx ends the block there; PCSX-Redux does "
                        "otherwise (a known difference)"),
        rl_case("rl_m0", rl_data(r, every_q[:48]), 0, comment="15-bit output, bit 15 clear (mode 0)"),
        rl_case("rl_m2", rl_data(r, every_q[15:]), 2, comment="15-bit output, bit 15 set (mode 2)"),
        rl_case("rl_extreme", rl_data(r, [1, 8, 63], "extreme"),
                comment="saturating coefficients and out-of-range colours (a known difference: PCSX-Redux)"),
    ]


def cases(sym):
    r = random.Random(SEED)
    out = [setup_case()]
    out += [frame_case(name, frame) for name, frame in FRAMES]
    out += [frame_case(*MODE_FRAME, mode=mode, columns=MODE_COLUMNS) for mode in (0, 2)]
    out += rl_cases(r)
    return out
