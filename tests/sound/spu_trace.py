#!/usr/bin/env python3
"""SPU register-write traces of the original game: the oracle for the PC port's LIBSND and SPU (docs/SOUND.md).

Runs a layer-2 pad script (tests/replay/scripts/*.json) in PCSX-Redux under -debugger -interpreter with
tests/sound/spu_trace.lua loaded before tests/replay/run.lua: a write breakpoint over the SPU's registers
(0x1F801C00..0x1F801DFF) and DMA4's (0x1F8010C0..CF) records every CPU store with its vsync tick, in order; a DMA4 start
from RAM records the block's SPU address, length and SHA-1.

Usage:
  tests/sound/spu_trace.py run <script.json> [--until CHECKPOINT] [--extra-frames N] [--repeat N] [--calls] [--detail]
                               [--out DIR] [--bios openbios|retail|FILE]
  tests/sound/spu_trace.py check [--record] # the committed short traces (tests/sound/expected/*.trace) reproduce
  tests/sound/spu_trace.py diff A.trace B.trace [--align MARKER] [--no-ticks] [--max N]

`run` writes <out>/run<i>/spu.trace (and keeps the emulator log and the replay result there); with --repeat N the N
traces must be identical (determinism). --until cuts the script after its checkpoint of that name, --extra-frames
appends a wait. --calls interleaves the game's calls into LIBSND/LIBSPU (the public functions, with a0..a3) as comment
lines and writes per-function call counts (internals included) to calls.txt; --detail adds the storing pc and the DMA
registers as comments. `check` runs the CHECKS cases (OpenBIOS, --calls) and requires the committed trace exactly,
comments included; `check --record` runs each twice and writes the trace when the two are identical.

Trace format (text, one event per line; '#' starts a comment, which comparisons ignore):
  <tick> <reg> <name> <value>                 an SPU register store: reg = address - 0x1F801000 (hex), name per
                                              docs/SOUND.md ("v05.pitch", "kon.lo", ...), value hex; a width other
                                              than 16 bits is a suffix on the name ("/8", "/32")
  <tick> dma4 spu=<addr> len=<bytes> sha1=<hex>   a DMA4 block from RAM into SPU RAM at <addr> (the transfer address the
                                              game set in 0x1F801DA6, x8, advanced by earlier blocks and FIFO writes)
  # mark <frame> <kind> ...                   markers: exe_start (the game's entry point; the BIOS shell's own sound
                                              before it is not traced), reset, and from the replay result the
                                              checkpoints, overlay and map changes
<tick> is the vsync count since boot (the replay runner's frame).

Exit codes: 0 pass, 1 mismatch or emulator failure, 2 usage / missing tool.
"""
import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tests/replay"))
import replay  # noqa: E402  (the layer-2 driver: run_once, bios paths, emulator info)

TRACE_LUA = ROOT / "tests/sound/spu_trace.lua"
SYMBOLS = ROOT / "config/symbol_addrs.txt"
EXPECTED = ROOT / "tests/sound/expected"
EMU_ARGS = ("-debugger", "-interpreter")   # memory and exec breakpoints need both
FORMAT = "dw2003 spu trace v1"

# The committed short traces `check` reproduces (with --calls): (name, script, until checkpoint, extra frames). cnty_sel:
# the boot (SsInit, sound_init, bank 1 = COMMON), CNTY_SEL's bank and music and its first sound effect; ~25 s.
CHECKS = (("cnty_sel", "tests/replay/scripts/new_game.json", "cnty_sel", 300),)

# SPU register names (psx-spx "SPU"): 24 voices of 8 registers, then the control block, then the reverb set.
VOICE_REGS = ("vol.l", "vol.r", "pitch", "addr", "adsr.lo", "adsr.hi", "adsr.vol", "loop")
CONTROL_REGS = {
    0xD80: "mvol.l", 0xD82: "mvol.r", 0xD84: "rvol.l", 0xD86: "rvol.r",
    0xD88: "kon.lo", 0xD8A: "kon.hi", 0xD8C: "koff.lo", 0xD8E: "koff.hi",
    0xD90: "pmon.lo", 0xD92: "pmon.hi", 0xD94: "non.lo", 0xD96: "non.hi",
    0xD98: "eon.lo", 0xD9A: "eon.hi", 0xD9C: "endx.lo", 0xD9E: "endx.hi",
    0xDA0: "unk_da0", 0xDA2: "rev.base", 0xDA4: "irq.addr", 0xDA6: "xfer.addr",
    0xDA8: "xfer.fifo", 0xDAA: "spucnt", 0xDAC: "xfer.ctrl", 0xDAE: "spustat",
    0xDB0: "cdvol.l", 0xDB2: "cdvol.r", 0xDB4: "extvol.l", 0xDB6: "extvol.r",
    0xDB8: "curvol.l", 0xDBA: "curvol.r", 0xDBC: "unk_dbc", 0xDBE: "unk_dbe",
}
REVERB_REGS = ("dAPF1", "dAPF2", "vIIR", "vCOMB1", "vCOMB2", "vCOMB3", "vCOMB4", "vWALL",
               "vAPF1", "vAPF2", "mLSAME", "mRSAME", "mLCOMB1", "mRCOMB1", "mLCOMB2", "mRCOMB2",
               "dLSAME", "dRSAME", "mLDIFF", "mRDIFF", "mLCOMB3", "mRCOMB3", "mLCOMB4", "mRCOMB4",
               "dLDIFF", "dRDIFF", "mLAPF1", "mRAPF1", "mLAPF2", "mRAPF2", "vLIN", "vRIN")
DMA4_REGS = {0x0C0: "dma4.madr", 0x0C4: "dma4.bcr", 0x0C8: "dma4.chcr", 0x0CC: "dma4.unk"}


def reg_name(off):
    """The name of the SPU or DMA4 register at 0x1F801000 + off (off rounded down to a halfword)."""
    off &= ~1
    if 0xC00 <= off < 0xD80:
        voice, r = divmod(off - 0xC00, 0x10)
        return f"v{voice:02d}.{VOICE_REGS[r // 2]}"
    if off in CONTROL_REGS:
        return CONTROL_REGS[off]
    if 0xDC0 <= off < 0xE00:
        return "rev." + REVERB_REGS[(off - 0xDC0) // 2]
    if (off & ~3) in DMA4_REGS:
        return DMA4_REGS[off & ~3] + (".hi" if off & 2 else "")
    return f"unk_{off:03x}"


def libsnd_functions():
    """[(addr, name, public)] of every LIBSND/LIBSPU function in config/symbol_addrs.txt (the object comments say which
    library a function belongs to). Public: called by the game (no leading underscore)."""
    funcs, lib = [], None
    for line in SYMBOLS.read_text().splitlines():
        if line.startswith("//"):
            m = re.search(r"\b(LIB\w+)\.LIB/", line)
            lib = m.group(1) if m else None
            continue
        m = re.match(r"(\w+)\s*=\s*0x([0-9A-Fa-f]+);\s*//\s*type:func", line)
        if m and lib in ("LIBSND", "LIBSPU"):
            funcs.append((int(m.group(2), 16), m.group(1), lib))
    return funcs


def lib_range():
    """[start, end) of the EXE's LIBSND/LIBSPU code (SsSeqCalledTbyT up to MemCardInit, the next library; LIBAPI's
    counter and event stubs in between never call back): a call whose ra is inside it comes from the library."""
    addrs = {name: addr for addr, name, _ in libsnd_functions()}
    end = next(int(l.split("=")[1].split(";")[0], 16) for l in SYMBOLS.read_text().splitlines()
               if l.startswith("MemCardInit ="))
    return addrs["SsSeqCalledTbyT"], end


def write_spec(path, calls):
    """The Lua spec for spu_trace.lua: the public LIBSND functions are logged (but SsSeqCalledTbyT, once a tick, is only
    counted), every other LIBSND/LIBSPU function is counted."""
    logged, counted = [], []
    if calls:
        for addr, name, lib in libsnd_functions():
            entry = f"{{ addr = 0x{addr:08X}, name = '{name}' }}"
            if lib == "LIBSND" and name.startswith("Ss") and name != "SsSeqCalledTbyT":
                logged.append(entry)
            else:
                counted.append(entry)
    path.write_text("return { calls = { " + ", ".join(logged) + " },\n         counts = { " + ", ".join(counted) + " } }\n")


def cut_script(script, until, extra_frames):
    """The script up to (and including) the checkpoint named `until`, plus a wait of `extra_frames`."""
    script = dict(script)
    steps = list(script["steps"])
    if until:
        idx = [i for i, s in enumerate(steps) if s.get("type") == "checkpoint" and s.get("name") == until]
        if not idx:
            raise SystemExit(f"spu_trace: the script has no checkpoint named {until!r}")
        steps = steps[:idx[0] + 1]
        script["name"] = f"{script['name']}@{until}"
    if extra_frames:
        steps.append({"type": "wait_frames", "frames": extra_frames})
        script["name"] = f"{script['name']}+{extra_frames}"
    script["steps"] = steps
    return script


def build_trace(run_dir, header, result, detail):
    """Turns spu_trace.lua's raw.txt + dma.bin and the replay's result.json into the trace text (and deletes dma.bin)."""
    raw = (run_dir / "raw.txt").read_text().splitlines()
    stats = dict(kv.split("=") for kv in raw[0].split()[1:])
    dma_path = run_dir / "dma.bin"
    dma = dma_path.read_bytes()
    # Markers: checkpoints, overlay and map changes, by frame.
    marks = []
    for cp in result.get("checkpoints", []):
        marks.append((cp["frame"], f"# mark {cp['frame']} checkpoint {cp['name']} stage={cp['stage']} map=0x{cp['map']:X}"))
    for o in result.get("overlay_sequence", []):
        marks.append((o["frame"], f"# mark {o['frame']} overlay stage={o['stage']} file={o['file']}"))
    for m in result.get("map_sequence", []):
        marks.append((m["frame"], f"# mark {m['frame']} map 0x{m['map']:X}"))
    marks.sort(key=lambda x: x[0])
    out = [f"# {FORMAT}"] + [f"# {h}" for h in header]
    out.append(f"# ticks {stats['ticks']}, writes {stats['writes']}, dma blocks {stats['dma']} ({stats['dma_bytes']} bytes),"
               f" unknown values {stats['unknown_values']}; BIOS shell writes before the EXE (not traced)"
               f" {stats['boot_writes']}")
    xfer = 0           # the SPU transfer address (bytes), as the game set it
    LIB_RANGE = lib_range()
    mi = 0
    for line in raw[1:]:
        if not line:
            continue
        f = line.split()
        tick = int(f[1])
        while mi < len(marks) and marks[mi][0] <= tick:
            out.append(marks[mi][1])
            mi += 1
        if f[0] == "w":
            addr, width, value, pc = int(f[2], 16), int(f[3]), f[4], f[5]
            off = addr & 0xFFF
            name = reg_name(off)
            if width == 1 and off & 1:
                name += ".b1"
            elif width == 1:
                name += "/8"
            elif width == 4:
                name += "/32"
            if 0xC0 <= off < 0xD0:
                if detail:
                    out.append(f"# {tick} {name} {value} pc={pc}")
                continue
            vtext = value if value == "?" else f"{int(value, 16):04x}" if width == 2 else value
            out.append(f"{tick} {off:03x} {name} {vtext}" + (f"  # pc={pc}" if detail else ""))
            if value != "?":
                v = int(value, 16)
                if off == 0xDA6:
                    xfer = (v & 0xFFFF) * 8
                elif off == 0xDA8:
                    xfer = (xfer + width) & 0x7FFFF
        elif f[0] == "d":
            madr, bcr, chcr, offset, length = f[2], f[3], f[4], int(f[5]), int(f[6])
            sha1 = hashlib.sha1(dma[offset:offset + length]).hexdigest()
            out.append(f"{tick} dma4 spu={xfer:05x} len={length} sha1={sha1}"
                       + (f"  # madr={madr} bcr={bcr} chcr={chcr}" if detail else ""))
            xfer = (xfer + length) & 0x7FFFF
        elif f[0] == "m":
            out.append(f"# mark {tick} {f[2]}")
        elif f[0] == "c":
            name, args, ra = f[2], f[3:7], f[7]
            internal = LIB_RANGE[0] <= int(ra, 16) < LIB_RANGE[1]
            if internal and not detail:
                continue    # LIBSND calling its own public functions (SsPitchFromNote, the NRPN reverb setters, ...)
            text = ", ".join("0x" + a.lstrip("0") if a.strip("0") else "0" for a in args)
            out.append(f"# {tick} call {name}({text}) ra={ra}" + (" (internal)" if internal else ""))
    for _, text in marks[mi:]:
        out.append(text)
    dma_path.unlink()
    return "\n".join(out) + "\n", stats


def run_trace(script_path, script, out_dir, bios, calls=False, detail=False):
    """One emulator run with the tracer; returns (trace text, stats, seconds)."""
    out_dir = Path(out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)
    spec = out_dir / "spu_trace_spec.lua"
    write_spec(spec, calls)
    wrapper = out_dir / "spu_trace_wrapper.lua"
    wrapper.write_text(f"dofile({json.dumps(str(TRACE_LUA))})\ndofile({json.dumps(str(replay.RUN_LUA))})\n")
    os.environ["DW3_SPU_TRACE_OUT"] = str(out_dir)
    os.environ["DW3_SPU_TRACE_SPEC"] = str(spec)
    t0 = time.time()
    replay.run_once(script_path, script, bios, out_dir, lua=wrapper, emu_args=EMU_ARGS)
    elapsed = time.time() - t0
    result = json.loads((out_dir / "result.json").read_text())
    emu = replay.emulator_info()
    header = [f"script {script['name']} ({Path(script_path).name}, sha1 {replay.sha1_file(script_path)[:12]}),"
              f" {result['frames']} frames",
              f"emulator {emu['name']} {emu['version']} build {emu['build_id']} ({emu['changeset'][:8]}), core interpreter,"
              f" bios {bios.name} ({replay.sha1_file(bios)[:12]})"]
    text, stats = build_trace(out_dir, header, result, detail)
    if calls:
        (out_dir / "calls.txt").write_text((out_dir / "counts.txt").read_text())
    (out_dir / "spu.trace").write_text(text)
    return text, stats, elapsed


def events(text, align=None, ticks=True):
    """The comparable lines of a trace: no comments, ticks rebased to the marker `align` (a substring of a '# mark'
    line, e.g. 'checkpoint cnty_sel'; events before it are dropped) or dropped with ticks=False."""
    base = 0
    lines = text.splitlines()
    if align:
        start = None
        for i, line in enumerate(lines):
            if line.startswith("# mark ") and align in line:
                start = i
                break
        if start is None:
            raise SystemExit(f"spu_trace: no marker matching {align!r}")
        base = int(lines[start].split()[2])
        lines = lines[start:]
    out = []
    for line in lines:
        line = line.split("#", 1)[0].strip()
        if not line:
            continue
        tick, rest = line.split(" ", 1)
        out.append(f"{int(tick) - base} {rest}" if ticks else rest)
    return out


def first_difference(a, b):
    for i, (x, y) in enumerate(zip(a, b)):
        if x != y:
            return i
    return None if len(a) == len(b) else min(len(a), len(b))


def report_diff(a, b, label_a, label_b, context=3, max_lines=20):
    i = first_difference(a, b)
    if i is None:
        return []
    out = [f"first difference at event {i} (of {len(a)} / {len(b)}):"]
    for j in range(max(0, i - context), min(max(len(a), len(b)), i + context + 1)):
        x = a[j] if j < len(a) else "<end>"
        y = b[j] if j < len(b) else "<end>"
        out.append(f"  {'*' if x != y else ' '} {label_a}: {x:<40} {label_b}: {y}")
    return out[:max_lines]


def cmd_run(args):
    replay.check_tools()
    script_path = Path(args.script)
    script = cut_script(replay.load_script(script_path), args.until, args.extra_frames)
    bios = replay.bios_path(args.bios)
    base = Path(args.out) if args.out else Path(tempfile.mkdtemp(prefix="dw3_spu_trace_"))
    traces = []
    for i in range(args.repeat):
        print(f"spu_trace {script['name']} (bios {args.bios}), run {i + 1}/{args.repeat}")
        try:
            text, stats, elapsed = run_trace(script_path, script, base / f"run{i + 1}", bios, args.calls, args.detail)
        except (RuntimeError, subprocess.TimeoutExpired) as e:
            print(f"  FAIL: {e}")
            return 1
        size = len(text.encode())
        print(f"  {stats['ticks']} ticks, {stats['writes']} writes, {stats['dma']} DMA blocks ({stats['dma_bytes']} bytes),"
              f" unknown values {stats['unknown_values']}; trace {size} bytes; {elapsed:.0f} s wall")
        traces.append(text)
    status = 0
    for i, t in enumerate(traces[1:], start=2):
        # The whole text, comments (calls, markers, pcs) included: every line must repeat.
        diff = report_diff(traces[0].splitlines(), t.splitlines(), "run1", f"run{i}")
        if diff:
            status = 1
            print(f"  run 1 vs run {i} DIFFER (non-deterministic):\n    " + "\n    ".join(diff))
    if args.repeat > 1 and status == 0:
        print(f"  determinism: {args.repeat} traces identical")
    print(f"  outputs: {base}")
    return status


def cmd_check(args):
    replay.check_tools()
    status = 0
    for name, script_rel, until, extra in CHECKS:
        expected_path = EXPECTED / f"{name}.trace"
        if not expected_path.exists() and not args.record:
            print(f"spu_trace {name}: no expected trace, skipped (check --record)")
            continue
        script_path = ROOT / script_rel
        script = cut_script(replay.load_script(script_path), until, extra)
        out = Path(tempfile.mkdtemp(prefix=f"dw3_spu_trace_{name}_"))
        texts = []
        for i in range(2 if args.record else 1):
            print(f"spu_trace {name} ({script['name']})" + (f", run {i + 1}/2" if args.record else ""))
            try:
                text, stats, elapsed = run_trace(script_path, script, out / f"run{i + 1}", replay.OPENBIOS, calls=True)
            except (RuntimeError, subprocess.TimeoutExpired) as e:
                print(f"  FAIL: {e}")
                status = 1
                break
            texts.append(text)
        else:
            if args.record:
                diff = report_diff(texts[0].splitlines(), texts[1].splitlines(), "run1", "run2")
                if diff:
                    status = 1
                    print("  not recording: the two runs differ:\n    " + "\n    ".join(diff))
                    continue
                EXPECTED.mkdir(exist_ok=True)
                expected_path.write_text(texts[0])
                print(f"  recorded {expected_path.relative_to(ROOT)} ({len(texts[0].encode())} bytes, {elapsed:.0f} s a run)")
            else:
                diff = report_diff(expected_path.read_text().splitlines(), texts[0].splitlines(), "expected", "got")
                if diff:
                    status = 1
                    print(f"  FAIL: differs from {expected_path.relative_to(ROOT)} (trace kept in {out}):\n    "
                          + "\n    ".join(diff))
                    continue
                print(f"  pass ({stats['writes']} writes, {stats['dma']} DMA blocks, {elapsed:.0f} s)")
            shutil.rmtree(out, ignore_errors=True)
    return status


def cmd_diff(args):
    a = events(Path(args.a).read_text(), args.align, not args.no_ticks)
    b = events(Path(args.b).read_text(), args.align, not args.no_ticks)
    diff = report_diff(a, b, "A", "B", max_lines=args.max)
    if not diff:
        print(f"identical ({len(a)} events)")
        return 0
    print("\n".join(diff))
    return 1


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("run", help="trace one script")
    r.add_argument("script")
    r.add_argument("--until", help="cut the script after its checkpoint of this name")
    r.add_argument("--extra-frames", type=int, default=0, help="frames to wait after the (cut) script")
    r.add_argument("--repeat", type=int, default=1, help="run N times and require identical traces")
    r.add_argument("--calls", action="store_true", help="log LIBSND calls as comments, count every LIBSND/LIBSPU call")
    r.add_argument("--detail", action="store_true", help="add the storing pc and the DMA registers as comments")
    r.add_argument("--bios", default="openbios", help="openbios (default), retail, or a BIOS file")
    r.add_argument("--out", help="output directory (default: a temp dir)")
    r.set_defaults(func=cmd_run)
    c = sub.add_parser("check", help="reproduce the committed short traces")
    c.add_argument("--record", action="store_true", help="write them instead (two runs must agree)")
    c.set_defaults(func=cmd_check)
    d = sub.add_parser("diff", help="compare two traces (comments ignored)")
    d.add_argument("a")
    d.add_argument("b")
    d.add_argument("--align", help="rebase ticks at the first '# mark' line containing this text (e.g. 'checkpoint cnty_sel')")
    d.add_argument("--no-ticks", action="store_true", help="compare the order of events only")
    d.add_argument("--max", type=int, default=20, help="lines of context to print")
    d.set_defaults(func=cmd_diff)
    args = ap.parse_args()
    sys.exit(args.func(args))


if __name__ == "__main__":
    main()
