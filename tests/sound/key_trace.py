#!/usr/bin/env python3
"""Per-key SPU write traces: every sound of the game played one at a time in the emulator, the SPU writes of each
recorded with tests/sound/spu_trace.lua. The oracle for the port's LIBSND beyond what the pad scripts reach
(docs/SOUND.md "More coverage"; T14's check).

  tools/venv/bin/python tests/sound/key_trace.py plan            # print the plan (steps, ticks); --write: plan.json
  tools/venv/bin/python tests/sound/key_trace.py gen [--boots 0,1] [--counts]   # run the emulator twice, write the goldens
  tools/venv/bin/python tests/sound/key_trace.py check [--boots 0]              # run it again, compare step by step
  tools/venv/bin/python tests/sound/key_trace.py diff A B [--steps 5,6] [--max N] # two key traces, step by step
  tools/venv/bin/python tests/sound/key_trace.py list [--bank N]                  # the committed steps, their sizes

How it runs (tests/golden/oracle.lua's call mechanism, -debugger -interpreter; one emulator boot per group of banks
of the plan, "boot" in plan.json: 5 boots of at most 12,500 ticks, bank 1 alone in boot 0): the game boots
with OpenBIOS until CNTY_SEL is resident, the oracle stops it in its main loop (pad_update) and calls, one after the
other, routines of this script written into scratch RAM (0x80181000; MIPS, assembled below): first sound_stop_all and
50 vsyncs, then one call per step of the plan:
  - bank step (bank id): sound_load_extra_bank(id), then `cdload_update(); sound_update_loading(); VSync(0)` until
    sound_is_loading() is 0 (what sound_init does for COMMON), then VSync(0). Bank 1 (COMMON, entry 0) needs none.
  - key step (key, play, mode, fade, tail): v = sound_play(key); `play` vsyncs (VSync(0) each); mode "fade":
    sound_fade_out(key) (SsSepSetDecrescendo) and `fade` vsyncs; then a note key (bit 31) sound_key_off(key, v)
    (SsUtKeyOff), a SEP key sound_stop(key) (SsSepStop); `tail` vsyncs; sound_stop_all() (SsSepStop on every
    sequence, SsUtAllKeyOff); one vsync.
The game's own code never runs in between (its main loop is stopped; only its interrupts: the vsync callback with
SsSeqCalledTbyT, LIBSND's tick, and the CD): each step sounds alone. spu_trace.lua logs the call of each step routine,
which becomes the step's marker; `# mark <tick> step <n> <name>` lines in the trace (format: tests/sound/spu_trace.py).

The plan (`plan`, from the disc and src/): the 71 banks of sound_banks in order (bank 1 first, already in entry 0;
the others alternate between entries 1 and 2, as the game does); per bank every sequence of its SEPs, except that of
the 1,016 identical two-event "placeholder" sequences (program 127, one note; FORMATS "SEP") only the first of each
SEP is played; then the keys the game passes to sound_module.play/stop/fade_out as literals in src/ (bit 31: a note
of the VAB; sequence numbers past 15: the score table's aliasing, docs/SOUND.md 1; bit 30: the "current" sound)
that the bank's enumeration did not already play, under the bank the key's entry id names. Ticks (50 a second):
  - a sequence without a loop: play = its length (from the SEP's deltas and tempo) + 10, then stop, tail 25;
  - a looping BGM (NRPN loop, CC 99 = 20/30): play = min(length + 50, 300) (6 s), alternately "fade" (decrescendo,
    fade 60) and "stop"; three loops are played through their loop point (length + 100): the two shortest and
    COMMON's sequence 6;
  - a placeholder: play 5, tail 5; a note key: play 25 (the note held), key off, tail 25; a literal SEP key the
    enumeration did not cover: play 100, tail 25.

The goldens (tests/sound/expected/keys/): plan.json (the steps) and one xz-compressed trace per bank,
bankNN.trace.xz, holding that bank's steps, each from its marker to the next one (ticks absolute, as in the run).
`check` and `diff` compare step by step with each step's ticks rebased to its marker (and with the comments
ignored, as spu_trace.py diff does); a port trace of the same driver must carry the same `# mark <tick> step <n>`
lines. LIBSND's state carries over from step to step within a boot, so a port run of a boot must make the same
steps in the same order from the same start (a fresh boot to CNTY_SEL, sound_stop_all, 50 vsyncs). --boots runs
(and compares) only those boots: each is reproduced exactly on its own.

Why boots: one boot of the whole plan (~45,000 ticks) failed twice at the same step (bank 49's load, near tick
32,500): first a Lua stack overflow in a burst, then a Lua error in the oracle's sentinel breakpoint, after which the
CPU ran into the sentinel; each bank alone runs clean. PCSX-Redux also leaks Lua stack slots per vsync (2; see
psxstack/tools/replay/run.lua): the wrapper raises an error out of a listener every 256 vsyncs to reset the stack (the other
listeners still run for that vsync: checked).

Exit codes: 0 pass, 1 mismatch or emulator failure, 2 usage / missing tool.
"""
import argparse
import lzma
import hashlib
import json
import math
import os
import re
import struct
import subprocess
import sys
import tempfile
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(ROOT / "tests/golden"))
sys.path.insert(0, str(ROOT / "tools"))
sys.path.insert(0, str(HERE))
import oracle  # noqa: E402  (the layer-1 call mechanism: symbols, Lua literals, the emulator's paths)
import sound_formats  # noqa: E402  (the banks and the SEP parser)
import spu_trace  # noqa: E402  (the trace format and the tracer)
import disc_files  # noqa: E402

EXPECTED = HERE / "expected/keys"
PLAN_JSON = EXPECTED / "plan.json"
FORMAT = "dw2003 key trace v1"
CODE_ADDR = 0x80181000     # scratch RAM above the heap's use at CNTY_SEL (tests/spu/envelope_oracle.py's too)
RATE = 50                  # ticks (vsyncs) per second: PAL, SsSetTickMode(0x1032)
SETUP_TICKS = 50
PLACEHOLDER = bytes.fromhex("00c07f01903c0c013c0000ff2f00")   # the 1,016 identical two-event sequences
LOOP_CAP = 300
FULL_LOOPS = 3             # the shortest looping sequences played through their loop point (+ COMMON's sequence 6)
STACK_RESET_FRAMES = 256   # the wrapper's Lua stack reset (PCSX-Redux leaks listener stack slots: psxstack/tools/replay/run.lua)
MODES = {"stop": 0, "fade": 1}
# A boot runs at most this many ticks of steps: one boot of the whole plan (~45,000 ticks) failed twice at the same
# step near tick 32,500 (a Lua error in the oracle's sentinel breakpoint, or before it a Lua stack overflow in a
# burst; each bank alone runs clean), so the plan is cut into boots that each start from a fresh emulator.
BOOT_TICKS = 12500


# ------------------------------------------------------------------------------------------------- the plan
def seq_info(sep, o):
    """(resolution, tempo, size, length in seconds, has a loop, events offset) of the sequence header at o."""
    _, res = struct.unpack_from(">HH", sep, o)
    tempo = int.from_bytes(sep[o + 4:o + 7], "big")
    size = struct.unpack_from(">I", sep, o + 9)[0]
    p, end, status, us, loop = o + 13, o + 13 + size, None, 0.0, False
    while p < end:
        delta, p = sound_formats.read_varlen(sep, p)
        us += delta * tempo / res
        b = sep[p]
        if b == 0xFF:
            if sep[p + 1] == 0x51:
                tempo = int.from_bytes(sep[p + 2:p + 5], "big")
                p += 5
                continue
            break
        if b & 0x80:
            status = b
            p += 1
        kind = status & 0xF0
        if kind == 0xB0 and sep[p] == 99 and sep[p + 1] in (20, 30):
            loop = True
        p += 1 if kind in (0xC0, 0xD0) else 2
    return res, tempo, size, us / 1e6, loop


def literal_keys():
    """{key: set of methods} of the literal keys src/ passes to sound_module.play/stop/fade_out."""
    keys = {}
    for path in sorted((ROOT / "src").rglob("*.c")):
        for m in re.finditer(r"sound_module\.(play|stop|fade_out)\(\s*(0x[0-9A-Fa-f]+)\s*\)", path.read_text()):
            keys.setdefault(int(m.group(2), 16), set()).add(m.group(1))
    return keys


def key_id(key):
    return (key >> 18) & 0x7F


def make_plan():
    exe = disc_files.EXE.read_bytes()
    paths = {e.id: e.path for e in disc_files.files()}
    literals = literal_keys()
    steps, loops = [], []
    for bank_id, b in sound_formats.banks(exe):
        name = paths[b["header_file"]].split("/")[-2]
        if bank_id != 1:
            steps.append({"kind": "bank", "cls": "bank", "bank": bank_id, "name": f"bank{bank_id:02d} load {name}"})
        hdr = disc_files.read(b["header_file"])
        done = set()
        for sep_index, s in enumerate(b["seps"]):
            sep = sound_formats.sub(hdr, s)
            o, placeholder_done = 6, False
            for seq in range(16):
                res, tempo, size, length, loop = seq_info(sep, o)
                body = sep[o + 13:o + 13 + size]
                o += 13 + size
                key = (bank_id << 18) | (sep_index << 8) | seq
                step = {"kind": "key", "bank": bank_id, "key": key, "fade": 0,
                        "name": f"bank{bank_id:02d} {name} sep{sep_index} seq{seq:02d}"}
                if body == PLACEHOLDER:
                    if placeholder_done:
                        continue
                    placeholder_done = True
                    step.update(cls="placeholder", mode="stop", play=5, tail=5, what="placeholder")
                elif loop:
                    step.update(cls="loop", mode="stop", play=min(math.ceil(length * RATE) + 50, LOOP_CAP), tail=25,
                                what=f"loop, {length:.2f} s")
                    loops.append((length, len(steps)))
                else:
                    step.update(cls="sequence", mode="stop", play=math.ceil(length * RATE) + 10, tail=25,
                                what=f"{length:.2f} s")
                done.add(key)
                steps.append(step)
        for key in sorted(k for k in literals if key_id(k) == bank_id and k & 0xC1FFFFFF not in done):
            methods = literals[key]
            step = {"kind": "key", "bank": bank_id, "key": key, "fade": 0, "tail": 25,
                    "name": f"bank{bank_id:02d} {name} key {key:08x}"}
            if key & 0x80000000:
                note = key & 0x7F
                step.update(cls="note", mode="stop", play=25, what=f"note: prog {(key >> 11) & 0x7F} tone {(key >> 7) & 0xF}"
                                                       f" note {note}")
            else:
                step.update(cls="literal", mode="fade" if "fade_out" in methods else "stop", play=100,
                            what="literal in src/ (" + ", ".join(sorted(methods)) + ")")
                if "fade_out" in methods:
                    step["fade"] = 60
            if key & 0x40000000:
                step["what"] += ", current"
            steps.append(step)
    # Looping sequences: the shortest FULL_LOOPS - 1 and COMMON's sequence 6 through their loop point; the others
    # alternately faded out and stopped.
    full = {i for _, i in sorted(loops)[:FULL_LOOPS - 1]}
    full |= {i for _, i in loops if steps[i]["key"] == (1 << 18) | 6}
    for n, (length, i) in enumerate(sorted(loops, key=lambda x: x[1])):
        if i in full:
            steps[i].update(play=math.ceil(length * RATE) + 100, what=steps[i]["what"] + ", through its loop point")
        elif n % 2 == 0:
            steps[i].update(mode="fade", fade=60)
    # Boots: whole banks, at most BOOT_TICKS each (bank 1 alone: it is entry 0's).
    boot, acc, bank_ticks = 0, 0, {}
    for s in steps:
        bank_ticks[s["bank"]] = bank_ticks.get(s["bank"], 0) + step_ticks(s)
    boot_of = {}
    for b in sorted(bank_ticks):
        if b != 1 and (b == 2 or acc + bank_ticks[b] > BOOT_TICKS):
            boot, acc = boot + 1, 0
        boot_of[b] = boot
        acc += bank_ticks[b]
    for n, s in enumerate(steps):
        s["step"] = n
        s["boot"] = boot_of[s["bank"]]
        s["name"] = f"{n:04d} " + s["name"]
    ticks = sum(step_ticks(s) for s in steps)
    return {"format": FORMAT, "comment": "the per-key trace driver's steps (tests/sound/key_trace.py)",
            "rate": RATE, "setup_ticks": SETUP_TICKS, "boots": boot + 1, "steps": steps, "ticks": ticks}


def step_ticks(s):
    """The vsyncs a step takes (a bank load's: unknown here, ~30 for the CD)."""
    if s["kind"] == "bank":
        return 30
    return s["play"] + s["fade"] + s["tail"] + 1


def plan_text(plan):
    """plan.json: one step per line."""
    head = {k: v for k, v in plan.items() if k != "steps"}
    lines = ",\n".join("  " + json.dumps(s, sort_keys=True) for s in plan["steps"])
    return json.dumps(head, indent=1)[:-2] + ',\n "steps": [\n' + lines + "\n ]\n}\n"


# ------------------------------------------------------------------------------------------------- the routines
ZERO, V0, A0, A1, A2, A3, T0, S0, S1, S2, S3, SP, RA = 0, 2, 4, 5, 6, 7, 8, 16, 17, 18, 19, 29, 31


class Asm:
    """A minimal MIPS I assembler with labels (branch offsets and jal targets resolved at the end)."""

    def __init__(self, base):
        self.base, self.words, self.labels, self.fixups = base, [], {}, []

    def here(self):
        return self.base + 4 * len(self.words)

    def label(self, name):
        self.labels[name] = self.here()

    def i(self, op, rs, rt, imm):
        self.words.append((op << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF))

    def r(self, rs, rt, rd, sa, funct):
        self.words.append((rs << 21) | (rt << 16) | (rd << 11) | (sa << 6) | funct)

    def nop(self): self.words.append(0)
    def addiu(self, rt, rs, imm): self.i(0x09, rs, rt, imm)
    def andi(self, rt, rs, imm): self.i(0x0C, rs, rt, imm)
    def lw(self, rt, off, base): self.i(0x23, base, rt, off)
    def sw(self, rt, off, base): self.i(0x2B, base, rt, off)
    def move(self, rd, rs): self.r(rs, ZERO, rd, 0, 0x21)
    def srl(self, rd, rt, sa): self.r(0, rt, rd, sa, 0x02)
    def jr(self, rs): self.r(rs, 0, 0, 0, 0x08)

    def jal(self, target):
        """target: an address or a label."""
        self.fixups.append((len(self.words), "jal", target))
        self.words.append(3 << 26)

    def branch(self, op, rs, rt, label):
        self.fixups.append((len(self.words), "b", label))
        self.i(op, rs, rt, 0)

    def beq(self, rs, rt, label): self.branch(0x04, rs, rt, label)
    def b(self, label): self.branch(0x04, ZERO, ZERO, label)
    def bgez(self, rs, label): self.branch(0x01, rs, 1, label)

    def assemble(self):
        for n, kind, target in self.fixups:
            addr = self.labels[target] if isinstance(target, str) else target
            if kind == "jal":
                self.words[n] |= (addr >> 2) & 0x3FFFFFF
            else:
                off = (addr - (self.base + 4 * n + 4)) >> 2
                assert -0x8000 <= off < 0x8000
                self.words[n] |= off & 0xFFFF
        return b"".join(struct.pack("<I", w) for w in self.words)


def routines(sym):
    """The step routines: {name: address}, code bytes. key(a0 key, a1 step, a2 play, a3 mode << 24 | fade << 12 |
    tail), bank(a0 bank id, a1 step), wait(a0 vsyncs)."""
    a = Asm(CODE_ADDR)
    f = {n: sym[n] for n in ("sound_play", "sound_stop", "sound_fade_out", "sound_key_off", "sound_stop_all",
                             "sound_load_extra_bank", "sound_is_loading", "sound_update_loading", "cdload_update",
                             "VSync")}

    def prologue(regs, mark=None):
        a.addiu(SP, SP, -40)
        if mark:
            a.label(mark)  # the tracer's marker: an exec breakpoint on the routine's first instruction would not
            #                fire, the oracle sets pc there from inside a breakpoint
        a.sw(RA, 36, SP)
        for k, r in enumerate(regs):
            a.sw(r, 16 + 4 * k, SP)

    def epilogue(regs):
        a.lw(RA, 36, SP)
        for k, r in enumerate(regs):
            a.lw(r, 16 + 4 * k, SP)
        a.jr(RA)
        a.addiu(SP, SP, 40)

    regs = (S0, S1, S2, S3)
    a.label("key")
    prologue(regs, "key_mark")
    a.move(S0, A0)
    a.move(S1, A2)
    a.move(S2, A3)
    a.jal(f["sound_play"])          # v = sound_play(key)
    a.nop()
    a.move(S3, V0)
    a.jal("wait")                   # play
    a.move(A0, S1)
    a.srl(T0, S2, 24)
    a.beq(T0, ZERO, "key_stop")
    a.nop()
    a.jal(f["sound_fade_out"])      # mode fade: the decrescendo, then `fade` vsyncs
    a.move(A0, S0)
    a.srl(A0, S2, 12)
    a.jal("wait")
    a.andi(A0, A0, 0xFFF)
    a.label("key_stop")
    a.bgez(S0, "key_sep")
    a.nop()
    a.move(A0, S0)                  # a note: sound_key_off(key, v)
    a.jal(f["sound_key_off"])
    a.move(A1, S3)
    a.b("key_tail")
    a.nop()
    a.label("key_sep")
    a.jal(f["sound_stop"])          # a sequence: sound_stop(key)
    a.move(A0, S0)
    a.label("key_tail")
    a.jal("wait")                   # tail
    a.andi(A0, S2, 0xFFF)
    a.jal(f["sound_stop_all"])
    a.nop()
    a.jal(f["VSync"])
    a.move(A0, ZERO)
    a.move(V0, S3)
    epilogue(regs)

    a.label("bank")
    prologue(regs, "bank_mark")
    a.jal(f["sound_load_extra_bank"])
    a.nop()
    a.label("bank_loop")
    a.jal(f["sound_is_loading"])
    a.nop()
    a.beq(V0, ZERO, "bank_done")
    a.nop()
    a.jal(f["cdload_update"])
    a.nop()
    a.jal(f["sound_update_loading"])
    a.nop()
    a.jal(f["VSync"])
    a.move(A0, ZERO)
    a.b("bank_loop")
    a.nop()
    a.label("bank_done")
    a.jal(f["VSync"])
    a.move(A0, ZERO)
    epilogue(regs)

    a.label("wait")
    prologue((S0,))
    a.move(S0, A0)
    a.label("wait_loop")
    a.beq(S0, ZERO, "wait_done")
    a.nop()
    a.jal(f["VSync"])
    a.move(A0, ZERO)
    a.b("wait_loop")
    a.addiu(S0, S0, -1)
    a.label("wait_done")
    epilogue((S0,))
    code = a.assemble()
    return {n: a.labels[n] for n in ("key", "bank", "wait", "key_mark", "bank_mark")}, code


# ------------------------------------------------------------------------------------------------- the run
def tracer_spec(path, entries, counts):
    """spu_trace.lua's spec: the step routines and the public LIBSND functions logged (their calls become comments),
    with --counts every other LIBSND/LIBSPU function counted (calls.txt: which internals the run reached)."""
    logged = [f"{{ addr = 0x{entries[n + '_mark']:08X}, name = 'step_{n}' }}" for n in ("key", "bank")]
    counted = []
    for addr, name, lib in spu_trace.libsnd_functions():
        entry = f"{{ addr = 0x{addr:08X}, name = '{name}' }}"
        if lib == "LIBSND" and name.startswith("Ss") and name != "SsSeqCalledTbyT":
            logged.append(entry)
        elif counts:
            counted.append(entry)
    path.write_text("return { calls = { " + ", ".join(logged) + " },\n         counts = { " + ", ".join(counted) + " } }\n")


def oracle_jobs(sym, plan, entries, code):
    """One kept job per bank (the oracle restores nothing between them); the first writes the code and silences the
    game (sound_stop_all, SETUP_TICKS vsyncs); the last reads the code back (the heap must not have reached it)."""
    jobs = [{"name": "setup", "keep": True, "saves": [], "writes": [{"addr": CODE_ADDR, "hex": code.hex()}],
             "calls": [{"name": "sound_stop_all", "func": sym["sound_stop_all"], "args": [], "reads": [], "writes": []},
                       {"name": "wait", "func": entries["wait"], "args": [SETUP_TICKS], "reads": [], "writes": []}]}]
    for s in plan["steps"]:
        if s["kind"] == "bank" or not jobs[-1]["name"].startswith(f"bank{s['bank']:02d}"):
            jobs.append({"name": f"bank{s['bank']:02d}", "keep": True, "saves": [], "writes": [], "calls": []})
        if s["kind"] == "bank":
            call = {"name": s["name"], "func": entries["bank"], "args": [s["bank"], s["step"]]}
        else:
            a3 = (MODES[s["mode"]] << 24) | (s["fade"] << 12) | s["tail"]
            call = {"name": s["name"], "func": entries["key"], "args": [s["key"], s["step"], s["play"], a3]}
        call.update(reads=[], writes=[])
        jobs[-1]["calls"].append(call)
    jobs[-1]["calls"][-1]["reads"] = [{"addr": CODE_ADDR, "size": len(code)}]
    return jobs


def run_emulator(plan, out_dir, counts=False, bios=oracle.OPENBIOS):
    """One boot: the plan's steps traced. Returns (raw trace text, results, seconds)."""
    out_dir = Path(out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)
    sym = oracle.Symbols()
    entries, code = routines(sym)
    max_step = max(step_ticks(s) for s in plan["steps"])
    spec = {"call_timeout_frames": max_step + 600, "overlays": []}
    jobs = oracle_jobs(sym, plan, entries, code)
    chunks = [jobs[i:i + 8] for i in range(0, len(jobs), 8)]
    main = ["local spec = " + oracle.lua_literal(spec), "spec.jobs = {}"]
    for n, chunk in enumerate(chunks):
        (out_dir / f"jobs_{n}.lua").write_text("return " + oracle.lua_literal(chunk) + "\n")
        main.append(f'for _, j in ipairs(dofile("{out_dir / f"jobs_{n}.lua"}")) do spec.jobs[#spec.jobs + 1] = j end')
    main.append("return spec")
    (out_dir / "jobs.lua").write_text("\n".join(main) + "\n")
    tracer_spec(out_dir / "spu_trace_spec.lua", entries, counts)
    wrapper = out_dir / "key_trace_wrapper.lua"
    wrapper.write_text(
        f"dofile({json.dumps(str(spu_trace.TRACE_LUA))})\n"
        "-- a Lua error in one of the oracle's breakpoints is printed with its traceback (Redux only says it threw)\n"
        "local add_breakpoint = PCSX.addBreakpoint\n"
        "PCSX.addBreakpoint = function(addr, kind, size, name, fn)\n"
        "    return add_breakpoint(addr, kind, size, name, function(...)\n"
        "        local ok, r = xpcall(fn, debug.traceback, ...)\n"
        "        if not ok then print('key_trace: breakpoint ' .. name .. ': ' .. tostring(r)) error(r) end\n"
        "        return r\n"
        "    end)\n"
        "end\n"
        f"dofile({json.dumps(str(oracle.ORACLE_LUA))})\n"
        "-- PCSX-Redux leaks Lua stack slots per listener call (psxstack/tools/replay/run.lua): an error out of a listener resets\n"
        "-- the stack; the other listeners still run for that vsync (checked), so the ticks are not disturbed.\n"
        "local frames = 0\n"
        "DW3_KEY_TRACE_RESET = PCSX.Events.createEventListener('GPU::Vsync', function()\n"
        "    frames = frames + 1\n"
        f"    if frames % {STACK_RESET_FRAMES} == 0 then\n"
        "        error(string.format('key_trace: Lua stack reset at frame %d (PCSX-Redux listener leak; not a failure)',"
        " frames))\n"
        "    end\n"
        "end)\n")
    env = dict(os.environ, DW3_ORACLE_JOBS=str(out_dir / "jobs.lua"), DW3_ORACLE_OUT=str(out_dir),
               DW3_SPU_TRACE_OUT=str(out_dir), DW3_SPU_TRACE_SPEC=str(out_dir / "spu_trace_spec.lua"))
    mcd = [str(out_dir / "memcard1.mcd"), str(out_dir / "memcard2.mcd")]
    cmd = [str(oracle.REDUX), "-no-ui", "-stdout", "-testmode", "-run", "-debugger", "-interpreter", "-iso",
           str(oracle.ISO), "-bios", str(bios), "-memcard1", mcd[0], "-memcard2", mcd[1], "-dofile", str(wrapper)]
    t0 = time.time()
    timeout = 600 + plan["ticks"] // 5   # the interpreter runs ~45 ticks a second: a generous bound
    with open(out_dir / "emulator.log", "w") as log:
        proc = subprocess.run(cmd, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=timeout)
    elapsed = time.time() - t0
    result_path = out_dir / "results.json"
    if not result_path.exists() or not (out_dir / "raw.txt").exists():
        tail = "\n".join((out_dir / "emulator.log").read_text(errors="replace").splitlines()[-10:])
        raise RuntimeError(f"emulator exited {proc.returncode} without results:\n{tail}")
    results = json.loads(result_path.read_text())
    if results.get("status") != "ok":
        raise RuntimeError(f"oracle failed: {results.get('message')} (log {out_dir / 'emulator.log'})")
    back = results["jobs"][-1]["calls"][-1]["reads"][0]
    if bytes.fromhex(back) != code:
        raise RuntimeError("the step routines were overwritten during the run (the heap reached 0x80181000?)")
    emu = json.loads(oracle.REDUX_VERSION.read_text())
    header = [f"key trace boot {plan['steps'][0]['boot']}: {len(plan['steps'])} steps of"
              f" tests/sound/expected/keys/plan.json",
              f"emulator {emu['version']} build {emu['buildId']} ({emu['changeset'][:8]}), core interpreter,"
              f" bios {bios.name} ({oracle.hashlib.sha1(bios.read_bytes()).hexdigest()[:12]})"]
    text, stats = spu_trace.build_trace(out_dir, header, {}, False)
    if counts:
        (out_dir / "calls.txt").write_text((out_dir / "counts.txt").read_text())
    return text, stats, elapsed


STEP_CALL = re.compile(r"# (\d+) call step_(key|bank)\((.*)\) ra=")


def to_key_trace(text, plan):
    """The tracer's text with the step calls turned into markers; returns {bank: text} (the prelude before the first
    step goes to bank 0, which is not kept)."""
    by_step = {s["step"]: s for s in plan["steps"]}
    out, bank = {0: []}, 0
    head = [line for line in text.splitlines() if line.startswith("# ") and not re.match(r"# \d", line)
            and not line.startswith("# mark")][:4]
    for line in text.splitlines():
        m = STEP_CALL.match(line)
        if m:
            args = [int(a, 16) if a.startswith("0x") else int(a) for a in m.group(3).split(", ")]
            s = by_step[args[1]]
            bank = s["bank"]
            if bank not in out:
                out[bank] = [f"# {FORMAT} (tests/sound/key_trace.py): bank {bank}"] + head[1:]
            out[bank].append(f"# mark {m.group(1)} step {s['name']}")
            continue
        if bank:
            out[bank].append(line)
    return {b: "\n".join(lines) + "\n" for b, lines in out.items() if b}


def segments(text, ticks=True):
    """{step name: [events]} of a key trace: comments dropped, ticks rebased to the step's marker."""
    segs, cur, base = {}, None, 0
    for line in text.splitlines():
        if line.startswith("# mark ") and " step " in line:
            f = line.split(" ", 4)
            base, cur = int(f[2]), f[4]
            segs[cur] = []
            continue
        line = line.split("#", 1)[0].strip()
        if not line or cur is None:
            continue
        tick, rest = line.split(" ", 1)
        segs[cur].append(f"{int(tick) - base} {rest}" if ticks else rest)
    return segs


def compare(expected, got, label_e="expected", label_g="got", max_lines=12, only=None):
    """Step by step; returns (steps compared, list of failing step names, report lines)."""
    e, g = segments(expected), segments(got)
    names = [n for n in e if only is None or n in only]
    bad, report = [], []
    for n in names:
        if n not in g:
            bad.append(n)
            report.append(f"step {n}: missing in {label_g}")
            continue
        diff = spu_trace.report_diff(e[n], g[n], label_e, label_g, max_lines=max_lines)
        if diff:
            bad.append(n)
            report.append(f"step {n}: " + diff[0])
            report += ["    " + d for d in diff[1:]]
    for n in g:
        if n not in e and (only is None or n in only):
            bad.append(n)
            report.append(f"step {n}: only in {label_g}")
    return len(names), bad, report


def golden_path(bank):
    return EXPECTED / f"bank{bank:02d}.trace.xz"


def write_xz(path, text):
    """xz (LZMA2, preset 9e): a third of gzip -9's size on these traces (8 flush stores a tick)."""
    path.write_bytes(lzma.compress(text.encode(), preset=9 | lzma.PRESET_EXTREME))


def read_trace(path):
    """A key trace: plain text, or xz-compressed (the goldens)."""
    data = Path(path).read_bytes()
    return lzma.decompress(data).decode() if data[:6] == b"\xfd7zXZ\x00" else data.decode()


def boot_plans(plan, boots):
    """[(boot, the plan's steps of that boot)] for the selected boots (comma-separated numbers; None: all)."""
    want = {int(b) for b in boots.split(",")} if boots else set(range(plan["boots"]))
    out = []
    for b in sorted(want):
        sub = dict(plan)
        sub["steps"] = [s for s in plan["steps"] if s["boot"] == b]
        if not sub["steps"]:
            raise SystemExit(f"key_trace: no boot {b} (the plan has {plan['boots']})")
        sub["ticks"] = sum(step_ticks(s) for s in sub["steps"])
        out.append((b, sub))
    return out


def run_and_split(plan, args, label):
    """Every selected boot in its own emulator run; returns {bank: key trace text}."""
    base = Path(args.out) if args.out else Path(tempfile.mkdtemp(prefix=f"dw3_key_trace_{label}_"))
    banks = {}
    for boot, sub in boot_plans(plan, args.boots):
        out = base / f"{label}_boot{boot}"
        banks_in = sorted({s["bank"] for s in sub["steps"]})
        print(f"key_trace {label} boot {boot} (banks {banks_in[0]}-{banks_in[-1]}): {len(sub['steps'])} steps,"
              f" ~{sub['ticks']} ticks; outputs in {out}", flush=True)
        text, stats, elapsed = run_emulator(sub, out, counts=getattr(args, "counts", False))
        (out / "spu.trace").write_text(text)
        banks.update(to_key_trace(text, sub))
        print(f"  {stats['ticks']} ticks, {stats['writes']} writes, {stats['dma']} DMA blocks, unknown values"
              f" {stats['unknown_values']}; {elapsed:.0f} s wall ({int(stats['ticks']) / max(elapsed, 1):.0f} ticks/s)",
              flush=True)
    return banks


# ------------------------------------------------------------------------------------------------- commands
def cmd_plan(args):
    plan = make_plan()
    kinds = {}
    for s in plan["steps"]:
        kinds[s["cls"]] = kinds.get(s["cls"], 0) + 1
    print(f"{len(plan['steps'])} steps, ~{plan['ticks']} ticks ({plan['ticks'] / RATE / 60:.1f} min at 50 Hz): "
          + ", ".join(f"{k} {v}" for k, v in sorted(kinds.items())))
    if args.verbose:
        for s in plan["steps"]:
            print(f"  {s['name']}: {s['cls']}" + (f" {s['key']:08x} {s['mode']} play {s['play']} fade {s['fade']}"
                                                   f" tail {s['tail']} ({s['what']})" if s["kind"] == "key" else ""))
    if args.write:
        EXPECTED.mkdir(parents=True, exist_ok=True)
        PLAN_JSON.write_text(plan_text(plan))
        print(f"wrote {PLAN_JSON.relative_to(ROOT)}")
    return 0


def check_plan(plan):
    if not PLAN_JSON.exists():
        return ["no committed plan (gen writes it)"]
    if PLAN_JSON.read_text() != plan_text(plan):
        return [f"the plan differs from {PLAN_JSON.relative_to(ROOT)} (src/ or the tool changed: gen again)"]
    return []


def cmd_gen(args):
    oracle_tools()
    plan = make_plan()
    runs = []
    for i in range(args.repeat):
        runs.append(run_and_split(plan, args, f"gen{i + 1}"))
    for i, other in enumerate(runs[1:], start=2):
        n, bad, report = 0, [], []
        for b in runs[0]:
            k, kb, rep = compare(runs[0][b], other.get(b, ""), "run1", f"run{i}")
            n, bad, report = n + k, bad + kb, report + rep
        if bad:
            print(f"  run 1 vs run {i}: {len(bad)} of {n} steps differ (non-deterministic): not writing\n    "
                  + "\n    ".join(report[:40]))
            return 1
        print(f"  run 1 vs run {i}: all {n} steps identical")
    EXPECTED.mkdir(parents=True, exist_ok=True)
    PLAN_JSON.write_text(plan_text(plan))
    total = 0
    for b, text in sorted(runs[0].items()):
        write_xz(golden_path(b), text)
        total += golden_path(b).stat().st_size
    print(f"  wrote {len(runs[0])} bank traces to {EXPECTED.relative_to(ROOT)} ({total} bytes compressed)")
    return 0


def cmd_check(args):
    oracle_tools()
    plan = make_plan()
    problems = check_plan(plan)
    if problems:
        print("key_trace: " + "; ".join(problems))
        return 1
    banks = run_and_split(plan, args, "check")
    n, bad, report = 0, [], []
    for b in sorted({s["bank"] for _, sub in boot_plans(plan, args.boots) for s in sub["steps"]}):
        path = golden_path(b)
        if not path.exists():
            bad.append(f"bank {b}")
            report.append(f"bank {b}: no golden {path.relative_to(ROOT)}")
            continue
        k, kb, rep = compare(read_trace(path), banks.get(b, ""))
        n, bad, report = n + k, bad + kb, report + rep
    if bad:
        print(f"key_trace check: FAIL: {len(bad)} of {n} steps differ\n  " + "\n  ".join(report[:60]))
        return 1
    print(f"key_trace check: pass ({n} steps identical)")
    return 0


def cmd_diff(args):
    only = None
    if args.steps:
        want = {int(x) for x in args.steps.split(",")}
        plan = json.loads(PLAN_JSON.read_text())
        only = {s["name"] for s in plan["steps"] if s["step"] in want}
    n, bad, report = compare(read_trace(args.a), read_trace(args.b), "A", "B", max_lines=args.max, only=only)
    if bad:
        print(f"{len(bad)} of {n} steps differ:\n" + "\n".join(report))
        return 1
    print(f"identical ({n} steps)")
    return 0


def cmd_list(args):
    plan = json.loads(PLAN_JSON.read_text())
    total = 0
    for path in sorted(EXPECTED.glob("bank*.trace.xz")):
        b = int(path.name[4:6])
        if args.bank is not None and b != args.bank:
            continue
        segs = segments(read_trace(path))
        total += path.stat().st_size
        print(f"{path.name}: {len(segs)} steps, {sum(len(v) for v in segs.values())} events,"
              f" {path.stat().st_size} bytes")
        if args.bank is not None:
            for name, ev in segs.items():
                print(f"  {name}: {len(ev)} events")
    print(f"{len(plan['steps'])} steps in the plan; {total} bytes")
    return 0


def oracle_tools():
    for p in (oracle.REDUX, oracle.ISO):
        if not p.exists():
            print(f"key_trace: missing {p} (scripts/setup.sh redux; scripts/worktree_init.sh)")
            sys.exit(2)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("plan", help="print the plan")
    p.add_argument("--write", action="store_true", help="write tests/sound/expected/keys/plan.json")
    p.add_argument("-v", "--verbose", action="store_true", help="every step")
    p.set_defaults(func=cmd_plan)
    for name, func, hlp in (("gen", cmd_gen, "run the emulator and write the goldens"),
                            ("check", cmd_check, "run the emulator and compare with the goldens")):
        c = sub.add_parser(name, help=hlp)
        c.add_argument("--boots", help="only these boots (comma-separated: 0 is bank 1, COMMON; plan.json's 'boot')")
        c.add_argument("--out", help="output directory (default: a temp dir)")
        if name == "gen":
            c.add_argument("--repeat", type=int, default=2, help="runs that must agree (default 2)")
            c.add_argument("--counts", action="store_true", help="count every LIBSND/LIBSPU call (calls.txt)")
        c.set_defaults(func=func)
    d = sub.add_parser("diff", help="compare two key traces step by step (.trace or .trace.xz)")
    d.add_argument("a")
    d.add_argument("b")
    d.add_argument("--steps", help="only these step numbers (comma-separated)")
    d.add_argument("--max", type=int, default=12, help="lines of context per step")
    d.set_defaults(func=cmd_diff)
    li = sub.add_parser("list", help="the committed goldens")
    li.add_argument("--bank", type=int)
    li.set_defaults(func=cmd_list)
    args = ap.parse_args()
    sys.exit(args.func(args))


if __name__ == "__main__":
    main()
