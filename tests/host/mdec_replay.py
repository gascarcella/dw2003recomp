#!/usr/bin/env python3
"""The mdec golden family (tests/golden/mdec.json, made by tests/golden/families/mdec.py) through the port's LIBPRESS
and MDEC (port/psyq/libpress.c, mdec.c). tests/host/replay.py runs it for the mdec family (exit 1 on a new mismatch);
standalone:

  tests/host/mdec_replay.py [-v] [--out DIR] [--m32] [--cflags "..."] [--results RUN/results.json] [--case NAME]

What must match:
- DecDCTvlc2's run-level words and return value, DecDCTin's command word: exactly (LIBPRESS's CPU code on the PS1).
- The pixels DecDCTout wrote: the MDEC is PCSX-Redux's emulation, whose IDCT and colour conversion are not psx-spx's
  bit for bit, so a pixel byte may differ by up to PIXEL_TOLERANCE (a 15-bit pixel's fields by 1, bit 15 exactly).
  The made-up cases keep their pixels in the golden and are compared within that tolerance (the differences inside it
  are counted and printed); a movie frame's columns are game data, so the golden keeps only their SHA-1, which the
  emulator's rounding makes differ: those are known mismatches (tests/host/known_mismatches.json "mdec/<case>: ...
  column"), and --results (an oracle run's raw results: tests/golden/oracle.py gen|check mdec --out RUN) compares
  their bytes within the tolerance instead, with the counts per case.
The frames come from the disc (the family's frame_bitstream, from each fixture's `source`). The harness
(tests/host/mdec_harness.c) is built with the port's flags."""
import argparse
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
GOLDEN = ROOT / "tests/golden/mdec.json"
OUT_DEFAULT = ROOT / "build/host_mdec"
KNOWN = ROOT / "tests/host/known_mismatches.json"
sys.path.insert(0, str(ROOT / "tests/golden"))
sys.path.insert(0, str(ROOT / "tests/golden/families"))
import mdec as family   # noqa: E402

PIXEL_TOLERANCE = 4   # 24-bit: |PS1 - host| per byte (4 seen in MOVIEOPN frame 3); 15-bit: 1 per 5-bit field


def build(out, m32=False, cflags=""):
    out.mkdir(parents=True, exist_ok=True)
    inc = out / "include"
    inc.mkdir(exist_ok=True)
    (inc / "include_asm.h").write_text("#ifndef INCLUDE_ASM_H\n#define INCLUDE_ASM_H\n#define INCLUDE_ASM(FOLDER, NAME)\n"
                                       "#define INCLUDE_RODATA(FOLDER, NAME)\n#endif\n")
    # port_game_gen.h: the game's description, which port/include/psxstack/hooks.h includes (the shim's psyq.h
    # includes that): the same header the port's build generates (tools/port_gen.py game-header).
    sys.path.insert(0, str(ROOT / "tools"))
    import port_gen  # noqa: E402
    (inc / "port_game_gen.h").write_text(port_gen.game_header_text())
    binary = out / ("mdec_harness_m32" if m32 else "mdec_harness")
    psyq = ROOT / "port/psyq"
    cmd = (["gcc", "-m32" if m32 else "-m64", "-std=gnu99", "-O2", "-fsigned-char", "-fwrapv", "-fno-strict-aliasing",
            "-DPC_PORT", "-Wall", "-Wextra", "-Werror", f"-I{inc}", f"-I{ROOT / 'include'}", f"-I{ROOT}", f"-I{ROOT / 'port/include'}", f"-I{psyq}"]
           + cflags.split()
           + [str(ROOT / "tests/host/mdec_harness.c"), str(psyq / "libpress.c"), str(psyq / "mdec.c"),
              str(psyq / "psyq.c"), "-ffunction-sections", "-Wl,--gc-sections", "-o", str(binary)])
    subprocess.run(cmd, check=True)
    return binary


def frame_from_source(source):
    """The bytes of a fixture's `source` ("MOVIEE03.STR frame 100: ...")."""
    m = re.match(r"(\S+\.STR) frame (\d+):", source)
    return family.frame_bitstream(m.group(1), int(m.group(2)))[0]


def pixel_diff(exp_hex, got_hex, mode):
    """(values compared, values that differ, largest difference): bytes for 24-bit, 5-bit fields for 15-bit (a bit 15
    difference counts as 255)."""
    e, g = bytes.fromhex(exp_hex), bytes.fromhex(got_hex)
    if len(e) != len(g):
        return len(e), len(e), 255
    if mode & 1:
        d = [abs(a - b) for a, b in zip(e, g) if a != b]
        return len(e), len(d), max(d, default=0)
    n, worst = 0, 0
    for i in range(0, len(e), 2):
        a, b = e[i] | e[i + 1] << 8, g[i] | g[i + 1] << 8
        if a == b:
            continue
        diffs = [abs((a >> s & 31) - (b >> s & 31)) for s in (0, 5, 10)] + [255 * ((a ^ b) >> 15)]
        n += sum(1 for x in diffs if x)
        worst = max(worst, max(diffs))
    return 3 * len(e) // 2, n, worst


def replay(golden, binary, verbose=False, results=None, only=None, stats=None):
    """Returns (calls, mismatches): mismatches are (case, what, original, host) tuples. stats (a dict), when given,
    collects per case [values compared, values differing, largest difference] over the pixel reads compared by bytes."""
    proc = subprocess.Popen([str(binary)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

    def ask(line):
        proc.stdin.write(line + "\n")
        proc.stdin.flush()
        ans = proc.stdout.readline().strip()
        if ans == "" and proc.poll() is not None:
            raise RuntimeError(f"mdec_harness exited {proc.returncode}")
        return ans

    raw = {}
    if results:
        raw = {j["name"]: j for j in json.loads(Path(results).read_text())["jobs"]}
    calls, mismatches = 0, []
    fixtures = golden["fixtures"]
    for case in golden["cases"]:
        if only and case["name"] not in only:
            continue
        job = raw.get(f"mdec/{case['name']}")
        frame = rl_words = None
        mode = 3
        pixels = ""
        for w in fixtures[case["fixture"]]:
            if w.get("source"):
                frame = frame_from_source(w["source"])
                if hashlib.sha1(frame).hexdigest() != w["sha1"]:
                    raise RuntimeError(f"{case['name']}: the disc's frame is not the golden's ({w['source']})")
            elif w["symbol"] == f"0x{family.RL_ADDR:08X}" and w["offset"] == 0:
                rl_words = w["hex"]
        for ci, call in enumerate(case["calls"]):
            calls += 1
            fn = call["func"]
            got, ret = [], None
            if fn == "DecDCTvlcBuild":
                pass
            elif fn == "DecDCTvlc2":
                size = sum(r["size"] for r in call["reads"])
                ret, data = ask(f"V {frame.hex()} {size}").split(" ")
                ret = int(ret)
                pos = 0
                for r in call["reads"]:
                    got.append(data[2 * pos:2 * (pos + r["size"])])
                    pos += r["size"]
                rl_words = data
            elif fn == "DecDCTReset":
                ask(f"R {call['args'][0]}")
            elif fn == "DecDCTin":
                mode = call["args"][1]
                got.append(ask(f"I {mode} {rl_words}"))
            elif fn == "DecDCTout":
                pixels = ask(f"O {call['args'][1]}")
            elif fn == "DecDCToutSync":
                ret = 0
                got = [pixels[2 * r["offset"]:2 * (r["offset"] + r["size"])] for r in call["reads"]]
            else:
                raise ValueError(f"{case['name']}: no host mirror for {fn}")
            if ret is not None and call["ret_type"] != "void" and ret != call["ret"]:
                mismatches.append((case["name"], f"{fn} return", call["ret"], ret))
            for ri, (rd, g) in enumerate(zip(call.get("reads", []), got)):
                expected = job["calls"][ci]["reads"][ri] if job is not None else rd.get("hex")
                if expected is None:
                    if hashlib.sha1(bytes.fromhex(g)).hexdigest() != rd["sha1"]:
                        mismatches.append((case["name"], f"{fn} read {rd['field']}", rd["sha1"], g))
                    continue
                if g == expected:
                    ok = True
                elif fn == "DecDCToutSync":
                    _, n, worst = pixel_diff(expected, g, mode)
                    ok = worst <= (PIXEL_TOLERANCE if mode & 1 else 1)
                else:
                    ok = False
                if fn == "DecDCToutSync" and stats is not None:
                    total, n, worst = pixel_diff(expected, g, mode)
                    s = stats.setdefault(case["name"], [0, 0, 0])
                    s[0] += total
                    s[1] += n
                    s[2] = max(s[2], worst)
                if not ok:
                    mismatches.append((case["name"], f"{fn} read {rd['field']}", expected, g))
    proc.stdin.close()
    proc.wait()
    if verbose:
        for name, what, exp, got in mismatches[:20]:
            print(f"    {name}: {what}")
            print(f"      PS1 {str(exp)[:120]}\n      host {str(got)[:120]}")
    return calls, mismatches


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("--out", default=str(OUT_DEFAULT))
    ap.add_argument("--m32", action="store_true", help="build the harness -m32")
    ap.add_argument("--cflags", default="", help="extra compiler flags (e.g. -fsanitize=address,undefined)")
    ap.add_argument("--results", help="an oracle run's results.json: compare raw bytes instead of SHA-1s")
    ap.add_argument("--case", action="append", help="only this case (repeatable)")
    args = ap.parse_args()
    golden = json.loads(GOLDEN.read_text())
    binary = build(Path(args.out), args.m32, args.cflags)
    stats = {}
    calls, mismatches = replay(golden, binary, args.verbose, args.results, args.case, stats)
    known = [k["where"] for k in json.loads(KNOWN.read_text()) if k["where"].startswith("mdec/")]
    new = [m for m in mismatches if not any(f"mdec/{m[0]}: {m[1]}".startswith(k) for k in known)]
    for name, (total, n, worst) in stats.items():
        print(f"    {name}: {n} of {total} pixel values differ from the PS1 ({100.0 * n / max(total, 1):.2f}%), "
              f"by at most {worst}")
    print(f"  mdec: {len(golden['cases'])} cases, {calls} calls: "
          + ("matches the PS1" if not mismatches else f"{len(new)} NEW MISMATCH(ES), {len(mismatches) - len(new)} known"))
    for name, what, exp, got in new[:10]:
        print(f"    mdec/{name}: {what}")
    return 1 if new else 0


if __name__ == "__main__":
    sys.exit(main())
