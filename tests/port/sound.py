#!/usr/bin/env python3
"""The port's sound against the emulator's SPU write traces (docs/SOUND.md section 7, "LIBSND").

Usage: tests/port/sound.py [replay] [TRACE ...] [--m32] [--sanitize] [--rate N] [--out DIR]
       tests/port/sound.py port PORT.trace [EMULATOR.trace] [--rate N]

`replay` (the default; run.py runs it): LIBSND exactly as the PS1's. Each emulator trace (default: the committed
ones, tests/sound/expected/*.trace and tests/port/sound/*.trace.gz; any that tests/sound/spu_trace.py `run --calls`
wrote) becomes a replay script: the game's LIBSND calls with their arguments, the pointers resolved to the sound bank
files they point into (the bank found by its body DMA's SHA-1, its files read from the disc), and the emulator's
vsyncs, at the emulator's ticks. tests/port/sound_replay.c, built from port/psyq/libsnd*.c and port/src/spu*.c, makes
those calls on that timeline (rendering the emulator's 877.3 SPU samples per vsync) and writes its SPU trace, which
must equal the emulator's: every store and DMA block, in order, at the same tick (comments aside, as
tests/sound/spu_trace.py `diff` compares). The game and its timing are out of it: the port does not reproduce the
frames at which the PS1 game calls LIBSND (see `port`).
  --m32 / --sanitize   also the -m32 build and an ASan/UBSan build of the replay: the same trace, no report

`port` (run.py runs it on its new_game run): a port run's own trace (`dw2003 --spu-trace`): (1) LIBSND's replay of the
port's own calls on the port's timeline (882 samples per vsync: the port's audio output) must give it exactly; (2) the
game must make the same LIBSND calls as on the emulator, with the same arguments, in the same order; (3) the stores
after each call, with ticks counted from the call, compared with the emulator's: a report of where the two games'
frames differ (where the PS1 drops frames, or the script's taps land at another frame), not a failure.

Needs the disc (iso/dw2003.bin, extracted/), the host gcc and the venv; no emulator. Exit 0 pass, 1 fail, 2 missing.
"""
import argparse
import gzip
import hashlib
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
sys.path.insert(0, str(ROOT / "tests/sound"))
import disc_files  # noqa: E402
import sound_formats  # noqa: E402
from spu_trace import events, report_diff  # noqa: E402

EXPECTED = ROOT / "tests/sound/expected"
PORT_EXPECTED = ROOT / "tests/port/sound"
SRCS = ["port/psyq/libsnd.c", "port/psyq/libsnd_seq.c", "port/psyq/libsnd_voice.c", "port/psyq/libsnd_spu.c",
        "port/src/spu.c", "port/src/spu_dsp.c", "port/src/sha1.c", "tests/port/sound_replay.c"]
CFLAGS = ["-std=gnu99", "-O2", "-fwrapv", "-fsigned-char", "-fno-strict-aliasing", "-Wall", "-Wextra", "-Werror",
          "-DPC_PORT", "-DNON_MATCHING", "-Iport/include", "-Iport/psyq", "-Iport/src", "-Iinclude", "-I."]
VARIANTS = {"m64": ["-m64"], "m32": ["-m32"],
            "san": ["-m64", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-omit-frame-pointer"]}
# SsUtKeyOn's last three arguments are on the stack, which the trace's call comments (a0..a3) do not show; the game's
# two callers (sound_play, sound_key_on) always pass fine 0, volumes 0x7F.
UT_KEY_ON_REST = (0, 0x7F, 0x7F)
# SPU samples per vsync. LIBSND reads every voice's envelope at the flush and reuses a voice once it reads 0, so a
# replay renders what the run it replays rendered between two vsyncs: the emulator 877.3 (tests/spu/envelope_oracle.py:
# 877.1..877.4 in 11 of its 14 fits), the port 882 (SPU_RATE / 50).
EMULATOR_RATE = 877.3
PORT_RATE = 882.0
CD_INIT = ["d80 mvol.l 3fff", "d82 mvol.r 3fff", "db0 cdvol.l 3fff", "db2 cdvol.r 3fff", "daa spucnt c001"]


class Missing(Exception):
    pass


def s32(text):
    text = text.strip()
    v = int(text, 16) if text.startswith("0x") else int(text)
    return v - (1 << 32) if v >= 1 << 31 else v


# ---- The sound banks on the disc ----

def bank_table():
    """The 71 banks: the header/body files, the VAB header's and body's sub-file offsets, the SEPs' offsets from the VAB
    header, the body DMA's length (whole 64-byte blocks) and its SHA-1, which is None when the last block reads past the
    loaded file (bank 64: 4 bytes of whatever RAM follows the file on the PS1, which no file holds)."""
    exe = disc_files.EXE.read_bytes()
    out = []
    for bank_id, b in sound_formats.banks(exe):
        mp = disc_files.read(b["header_file"])
        mv = disc_files.read(b["body_file"])
        vh_off = disc_files.subfile(mp, b["vab_header"] & 0xFFFF)
        vb_off = disc_files.subfile(mv, b["vab_body"] & 0xFFFF)
        vh = sound_formats.parse_vh(mp[vh_off:])
        size = sum(vh["vag_sizes"][:vh["vags"] + 1])
        length = (size + 63) & ~63
        dma = mv[vb_off:vb_off + length]
        out.append(dict(id=bank_id, mp=b["header_file"], mv=b["body_file"], vh_off=vh_off, vb_off=vb_off, len=length,
                        sha1=hashlib.sha1(dma).hexdigest() if len(dma) == length else None,
                        seps=[disc_files.subfile(mp, s & 0xFFFF) - vh_off for s in b["seps"]]))
    return out


def find_bank(banks, length, sha1, seps):
    """The bank whose body DMA the trace shows (`length`, `sha1`), its SEPs at `seps` from the VAB header: by the
    SHA-1, or for a body whose DMA reads past its file, by the length and the SEP offsets."""
    for b in banks:
        if b["sha1"] == sha1:
            return b
    found = [b for b in banks if b["sha1"] is None and b["len"] == length and b["seps"][:len(seps)] == seps]
    return found[0] if len(found) == 1 else None


# ---- Replay ----

# The emulator's call comments: `# T call Name(a0, a1, a2, a3) ra=...` (hex, the first four argument registers); the
# port's (port/src/spu_trace.c): `# T call Name(args) [= result]` (decimal, every argument; pointers as `head+OFF`).
EMU_CALL_RE = re.compile(r"# (\d+) call (\w+)\(([^)]*)\) ra=([0-9a-f]+)")
PORT_CALL_RE = re.compile(r"# (\d+) call (\w+)\(([^)]*)\)(?: = (-?\d+))?$")
# Per function: the arguments the replay takes, in the emulator's registers (None: not shown there).
CALL_ARGS = {"reset": 0, "SsInit": 0, "SsStart2": 0, "SsUtReverbOn": 0, "SsSetTickMode": 1, "SsSetMVol": 2, "SsSetSerialAttr": 3,
             "SsSetSerialVol": 3, "SsUtSetReverbType": 1, "SsUtSetReverbDepth": 2, "SsVabTransCompleted": 1,
             "SsVabClose": 1, "SsSepPlay": 4, "SsSepStop": 2, "SsSepClose": 1, "SsSepSetVol": 4,
             "SsSepSetDecrescendo": 4, "SsUtAllKeyOff": 1}


def parse_call(line):
    """(tick, name, args) of a call comment of either trace, or None. Pointers: SsVabOpenHeadSticky's and
    SsVabTransBody's are dropped (args: id, sbaddr / id); SsSepOpen's becomes ("abs", PS1 address) or ("rel", offset
    from the VAB header); SsSetTableSize gives (s_max, t_max); SsUtKeyOn all seven."""
    m = EMU_CALL_RE.match(line)
    if m:
        tick, name = int(m.group(1)), m.group(2)
        a = [s32(x) for x in m.group(3).split(",")]
        if name == "SsVabOpenHeadSticky":
            return tick, name, [a[1], a[2], ("abs", a[0] & 0xFFFFFFFF)]
        if name == "SsVabTransBody":
            return tick, name, [a[1]]
        if name == "SsSepOpen":
            return tick, name, [("abs", a[0] & 0xFFFFFFFF), a[1], a[2]]
        if name == "SsSetTableSize":
            return tick, name, a[1:3]
        if name == "SsUtKeyOn":
            return tick, name, a[:4] + list(UT_KEY_ON_REST)
        if name == "SsUtKeyOff":
            return tick, name, a[:4] + [0]
        if name not in CALL_ARGS:
            raise SystemExit(f"sound.py: a call the replay does not know: {line}")
        return tick, name, a[:CALL_ARGS[name]]
    m = PORT_CALL_RE.match(line)
    if m:
        tick, name = int(m.group(1)), m.group(2)
        args = [x.strip() for x in m.group(3).split(",") if x.strip()]
        if name == "SsSepOpen":
            return tick, name, [("rel", int(args[0].split("+")[1]))] + [s32(x) for x in args[1:]]
        a = [s32(x) for x in args]
        if name == "SsVabOpenHeadSticky":
            return tick, name, a + [None]
        return tick, name, a
    return None


def replay_script(trace_text, work):
    """The replay script for a trace (sound_replay.c's format), its data files written into `work`; and the DMA blocks
    whose last bytes no file holds."""
    lines = trace_text.splitlines()
    calls = [(i, parse_call(l)) for i, l in enumerate(lines) if l.startswith("# ") and " call " in l]
    calls = [(i, c) for i, c in calls if c is not None]
    banks = None
    data = {}            # file id -> data number
    vab_files = {}       # VAB id -> (header data number, VAB header offset in it, PS1 address of the header or None)
    body_ptr = {}        # VAB id -> "n+offset" of its body
    out = []
    unknown = set()      # "tick dma4 spu=.. len=.." of DMA blocks whose last bytes no file holds
    count = 0
    first_tick = last_tick = None

    def data_no(fid):
        if fid not in data:
            path = work / f"file_{fid:04x}.bin"
            path.write_bytes(disc_files.read(fid))
            data[fid] = len(data)
            out.append(f"data {data[fid]} {path}")
        return data[fid]

    def sep_rel(arg, vab):
        return arg[1] if arg[0] == "rel" else arg[1] - vab_files[vab][2]

    call_at = {i: c for i, c in calls}
    for i, line in enumerate(lines):
        if i in call_at:
            tick, name, a = call_at[i]
            if name == "SsVabOpenHeadSticky":
                vab, sb = a[0], a[1]
                sha = length = dma_line = None
                for nxt in lines[i + 1:]:
                    f = nxt.split()
                    if len(f) >= 5 and f[1] == "dma4" and f[2] == f"spu={sb:05x}":
                        sha, length, dma_line = f[4].split("=")[1], int(f[3].split("=")[1]), " ".join(f[:4])
                        break
                seps = []
                for j, c in calls:
                    if j > i and c[1] == "SsVabOpenHeadSticky" and c[2][0] == vab:
                        break
                    if j > i and c[1] == "SsSepOpen" and c[2][1] == vab:
                        seps.append(c[2][0][1] - a[2][1] if c[2][0][0] == "abs" else c[2][0][1])
                if banks is None:
                    banks = bank_table()
                bank = find_bank(banks, length, sha, seps)
                if bank is None:
                    raise SystemExit(f"sound.py: no bank's body has the DMA SHA-1 {sha} (VAB open at tick {tick})")
                if bank["sha1"] is None or bank["sha1"] != sha:
                    unknown.add(dma_line)
                n = data_no(bank["mp"])
                vab_files[vab] = (n, bank["vh_off"], a[2][1] if a[2] is not None else None)
                body_ptr[vab] = f"{data_no(bank['mv'])}+{bank['vb_off']}"
                out.append(f"call {tick} {name} {n}+{bank['vh_off']} {vab} {sb}")
            elif name == "SsVabTransBody":
                out.append(f"call {tick} {name} {body_ptr[a[0]]} {a[0]}")
            elif name == "SsSepOpen":
                n, vh_off, _ = vab_files[a[1]]
                out.append(f"call {tick} {name} {n}+{vh_off + sep_rel(a[0], a[1])} {a[1]} {a[2]}")
            else:
                if name == "reset":  # the console's reset (a port trace's): no vsync handler until SsInit's stores
                    first_tick = None
                out.append(f"call {tick} {name} {' '.join(str(x) for x in a)}".rstrip())
            continue
        body = line.split("#", 1)[0].strip()
        if body:
            f = body.split()
            tick = int(f[0])
            if first_tick is None:
                first_tick = tick
            elif tick != last_tick:
                for t in range(last_tick + 1, tick + 1):
                    out.append(f"vsync {count} {t}")
            last_tick = tick
            if [" ".join(x.split("#")[0].split()[1:]) for x in lines[i:i + 5]] == CD_INIT:
                out.append(f"cdinit {tick}")
            count += 1
    # A busy loop of completion polls (sound_init's) whose last poll is at an earlier tick than the call after it: that
    # poll was running when the vsync came, and its answer (1: the DMA's interrupt came with the vsync) is the vsync's.
    # The emulator logs a call at its entry, so the replay moves that poll to the next call's tick.
    for k, line in enumerate(out):
        if " SsVabTransCompleted " not in line or k + 1 >= len(out):
            continue
        nxt = next((x for x in out[k + 1:] if x.startswith("call ")), None)
        if nxt is not None and " SsVabTransCompleted " not in nxt:
            t, t_next = int(line.split()[1]), int(nxt.split()[1])
            if t_next > t:
                out[k] = f"call {t_next} " + line.split(" ", 2)[2]
    script = work / "replay.script"
    script.write_text("\n".join(out) + "\n")
    return script, unknown


def build(variant, out_dir):
    exe = out_dir / variant / "sound_replay"
    exe.parent.mkdir(parents=True, exist_ok=True)
    gen = ROOT / "build/port/gen/include"
    if not (gen / "include_asm.h").exists():
        raise Missing("build/port/gen/include is missing: configure the port first (cmake -S port -B build/port)")
    cmd = ["gcc", *CFLAGS, f"-I{gen}", *VARIANTS[variant], *SRCS, "-o", str(exe), "-lm"]
    proc = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    if proc.returncode != 0:
        raise RuntimeError(f"the {variant} build of sound_replay failed:\n{proc.stdout}{proc.stderr}")
    return exe


def read_trace(path):
    path = Path(path)
    return gzip.decompress(path.read_bytes()).decode() if path.suffix == ".gz" else path.read_text()


def trace_name(path):
    return Path(path).name.split(".")[0]


def replay(trace_path, variants, out_dir, rate, against="the emulator"):
    """Replays a trace's calls on its timeline (sound_replay, each build variant) and compares the result with the
    trace. Returns the failures."""
    name = trace_name(trace_path)
    work = out_dir / name
    work.mkdir(parents=True, exist_ok=True)
    text = read_trace(trace_path)
    script, unknown = replay_script(text, work)

    def tail_unknown(evs):
        """The SHA-1 of a DMA block whose last bytes no file holds is not compared (its address and length are)."""
        return [e.split(" sha1=")[0] + " sha1=(past the file)" if e.split(" sha1=")[0] in unknown else e for e in evs]

    want = tail_unknown(events(text))
    if unknown:
        print(f"  not compared: the SHA-1 of {len(unknown)} DMA block(s) that read past the bank's file (the RAM after"
              f" it): {', '.join(sorted(unknown))}")
    failures = []
    first = None
    for v in variants:
        exe = build(v, out_dir)
        got_path = work / f"replay_{v}.trace"
        env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0", UBSAN_OPTIONS="print_stacktrace=1")
        proc = subprocess.run([str(exe), str(script), str(got_path), str(round(rate * 10))], capture_output=True,
                              text=True, env=env, timeout=1200)
        if proc.returncode != 0 or "runtime error" in proc.stderr or "AddressSanitizer" in proc.stderr:
            failures.append(f"{name} [{v}]: sound_replay exit {proc.returncode}\n{proc.stderr[-3000:]}")
            continue
        got = tail_unknown(events(got_path.read_text()))
        diff = report_diff(want, got, "trace", "replay", max_lines=24)
        if diff:
            failures.append(f"{name} [{v}]: the replay differs from {against}'s trace (replay: {got_path})\n    "
                            + "\n    ".join(diff))
        else:
            print(f"  [{v}] {name}: identical to {against}'s trace ({len(want)} stores and DMA blocks, ticks"
                  f" {want[0].split()[0]}..{want[-1].split()[0]}, {rate} samples per vsync)")
        if first is None:
            first = got
        elif got != first:
            failures.append(f"{name} [{v}]: differs from the {variants[0]} build's replay")
    return failures


def check_libsnd(variants, out_dir, traces=None, rate=EMULATOR_RATE):
    """LIBSND against the committed emulator traces (or `traces`). Returns the failures."""
    if not disc_files.ISO.exists() or not disc_files.EXE.exists():
        raise Missing("no disc (iso/dw2003.bin, extracted/)")
    failures = []
    for t in traces or committed_traces():
        print(f"sound: LIBSND replayed on the emulator's timeline: {Path(t).relative_to(ROOT) if Path(t).is_relative_to(ROOT) else t}")
        failures += replay(t, variants, Path(out_dir), rate)
    return failures


def committed_traces():
    return sorted(EXPECTED.glob("*.trace")) + sorted(PORT_EXPECTED.glob("*.trace.gz"))


# ---- The port's own run ----

def call_list(text):
    """The game's LIBSND calls of a trace as (tick, name, args), the completion polls left out (how many there are is
    CPU time: the emulator's sound_init polls COMMON's transfer 104 times in one frame, the port once), pointers too."""
    out = []
    for line in text.splitlines():
        if line.startswith("# ") and " call " in line:
            c = parse_call(line)
            if c is None or c[1] == "SsVabTransCompleted":
                continue
            args = [a for a in c[2] if not isinstance(a, tuple) and a is not None]
            out.append((c[0], c[1], args))
    return out


def segments(text):
    """{call index: [events, ticks counted from the call]} of the stores after each game call (polls aside)."""
    segs, base, k = {}, None, -1
    for line in text.splitlines():
        if line.startswith("# "):
            if " call " in line and "SsVabTransCompleted" not in line and parse_call(line) is not None:
                k += 1
                base = int(line.split()[1])
                segs[k] = []
            continue
        body = line.split("#", 1)[0].split()
        if body and k >= 0:
            segs[k].append(f"{int(body[0]) - base} {' '.join(body[1:])}")
    return segs


def check_port_run(port_trace, emu_trace, out_dir, rate=PORT_RATE, rendered=True):
    """A port run's trace: (1) LIBSND's replay of the port's own calls on the port's timeline gives it exactly (the
    integration: the vsync handler's tick, the DMA's completion, CdInit's stores); (2) the game made the same LIBSND
    calls as on the emulator, in the same order, with the same arguments; (3) where their frames differ: the
    segments between two calls compared with ticks counted from the call (informational: the game's timing). Returns
    the failures."""
    failures = []
    if rendered:
        print(f"sound: the port's trace against LIBSND's replay of its own calls: {port_trace}")
        failures += replay(port_trace, ["m64"], Path(out_dir), rate, against="the port run")
    else:
        print("sound: the port's trace against LIBSND's replay: skipped: the port's audio output (port/src/audio.c) is"
              " the step-0 stub, which renders nothing, so the envelopes LIBSND reads stay at their key-on values")
    if emu_trace is None:
        return failures
    port_text, emu_text = read_trace(port_trace), read_trace(emu_trace)
    pc, ec = call_list(port_text), call_list(emu_text)
    same_calls = [(a[1], a[2]) for a in pc] == [(b[1], b[2]) for b in ec]
    if not same_calls:
        k = next((i for i, (a, b) in enumerate(zip(pc, ec)) if (a[1], a[2]) != (b[1], b[2])), min(len(pc), len(ec)))
        failures.append(f"the port's LIBSND calls differ from the emulator's at call {k} of {len(pc)} / {len(ec)}: "
                        f"{pc[k] if k < len(pc) else '<end>'} vs {ec[k] if k < len(ec) else '<end>'}")
        return failures
    print(f"  the same {len(pc)} LIBSND calls as the emulator, in the same order, with the same arguments")
    # The game's timing: the frames between two calls, emulator vs port. SsInit's own segment differs by its vsync:
    # on the PS1 the reverb work area's clearing outlasts the frame, so the tick's flush falls inside SsInit.
    changes = {k: (ec[k][0] - ec[k - 1][0]) - (pc[k][0] - pc[k - 1][0]) for k in range(1, len(ec))}
    changes = {k: d for k, d in changes.items() if d != 0}
    print(f"  the game's frames between two calls differ at {len(changes)} of {len(ec) - 1} calls: "
          + ", ".join(f"{ec[k][1]}#{k} {d:+d}" for k, d in sorted(changes.items())))
    ps, es = segments(port_text), segments(emu_text)
    differ = [k for k in sorted(es) if ps.get(k) != es[k]]
    print(f"  segments (the stores after each call, ticks counted from it) identical: {len(es) - len(differ)} of"
          f" {len(es)}")
    for k in differ:
        since = max((j for j in changes if j <= k), default=None)
        diff = report_diff(es[k], ps.get(k, []), "emulator", "port", context=0, max_lines=2)
        first = diff[1].strip() if len(diff) > 1 else ""
        if k == 0:
            why = "SsInit's vsync (the emulator's falls inside SsInit)"
        elif since is not None:
            why = f"the frames differ since call {since} ({ec[since][1]}, {changes[since]:+d})"
        else:
            why = None
            failures.append(f"segment after call {k} ({ec[k][1]}) differs with the same timing: {first}")
        if why:
            print(f"    after call {k} {ec[k][1]}: {why}; {first}")
    return failures


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("mode", nargs="?", default="replay")
    ap.add_argument("traces", nargs="*")
    ap.add_argument("--m32", action="store_true", help="also the -m32 build of the replay")
    ap.add_argument("--sanitize", action="store_true", help="also an ASan/UBSan build of the replay")
    ap.add_argument("--out", default=str(ROOT / "build/port-sound"))
    ap.add_argument("--rate", type=float, help=f"SPU samples rendered per vsync in the replay (default: the emulator's"
                                                f" {EMULATOR_RATE}; `port`: the port's {PORT_RATE})")
    args = ap.parse_args()
    if args.mode not in ("replay", "port"):
        args.traces.insert(0, args.mode)
        args.mode = "replay"
    try:
        if args.mode == "port":
            if len(args.traces) not in (1, 2):
                ap.error("port PORT.trace [EMULATOR.trace]")
            failures = check_port_run(args.traces[0], args.traces[1] if len(args.traces) > 1 else None,
                                      Path(args.out) / "port", args.rate or PORT_RATE)
        else:
            variants = ["m64"] + (["m32"] if args.m32 else []) + (["san"] if args.sanitize else [])
            failures = check_libsnd(variants, args.out, args.traces or None, args.rate or EMULATOR_RATE)
    except Missing as e:
        print(f"sound: {e}")
        return 2
    except RuntimeError as e:
        failures = [str(e)]
    if failures:
        print("sound: FAIL\n  " + "\n  ".join(failures))
        return 1
    print("sound: pass")
    return 0


if __name__ == "__main__":
    sys.exit(main())
