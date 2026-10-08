#!/usr/bin/env python3
"""The crash report (psxstack/runtime/crash.c; docs/PORT.md "Crash report"): what a tester's report must hold.

Usage: tests/port/crash.py [--out DIR] [--wine [--exe PATH]]

Needs build/port/dw2003 (tests/port/run.py builds it), the disc and addr2line. Three short runs (~3 s):
  - DW3_PORT_CRASH_AT=30 (a NULL write at vsync 30): the process dies of SIGSEGV (status -11, as without the handler),
    its last stderr line names the report, the report has the build, the platform, the vsync (30), the signal, the
    fault address (0x0), a pc and a stack, and scripts/symbolize.py resolves the pc to port_crash_test_write with the
    unstripped build;
  - a fatal error (--disc of a missing file): status 1, a report of kind `fatal` with the reason and the log's tail;
  - `--version` prints the stamp the report's `build:` line carries.
--wine: the same three runs with the Windows build (--exe, default build/port-win/dw2003.exe; scripts/build_windows.sh)
through `wine` (headless: SDL's dummy drivers, the prefix in build/wine-prefix/). The crash is then an access
violation (0xC0000005: the process's exit status, which wine's own exit truncates to 5), the report names the
exception, the fault address and the access, its stack is walked from the exception's context (frame 0 the faulting
instruction, resolved with llvm-symbolizer and the PDB beside the exe), and a minidump lies beside it: the test reads
its streams (the exception record's code and address, the module list with dw2003.exe, the thread list) itself.
Exit codes: 0 pass, 1 fail, 2 something missing.
"""
import argparse
import os
import re
import shutil
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DISC = ROOT / "iso/dw2003.cue"
SYMBOLIZE = ROOT / "scripts/symbolize.py"

FAILURES = []
BINARY = ROOT / "build/port/dw2003"
RUNNER = []
RUNNER_ENV = {}
WINE = False


def check(cond, what):
    print(f"  {'ok  ' if cond else 'FAIL'} {what}")
    if not cond:
        FAILURES.append(what)
    return cond


def run(args, out_dir, env=None):
    e = dict(os.environ)
    e.update(RUNNER_ENV)
    e.update(env or {})
    r = subprocess.run([*RUNNER, str(BINARY), *args, "--crash-dir", str(out_dir)], cwd=ROOT, env=e,
                       capture_output=True, text=True, errors="replace", timeout=300)
    reports = sorted(out_dir.glob("crash-*.txt"))
    return r, reports


def port_lines(stderr):
    """The port's own stderr lines (wine prints its loader's and Mesa's around them)."""
    return [l.rstrip("\r") for l in stderr.splitlines() if l.startswith("port:")]


def fields(text):
    return {m.group(1): m.group(2) for m in re.finditer(r"^([a-z 0-9]+): (.*)$", text, re.M)}


def symbolize(report):
    s = subprocess.run([sys.executable, str(SYMBOLIZE), str(report), "--binary", str(BINARY)], capture_output=True,
                       text=True)
    return s, s.stdout.splitlines()


# ---- a minidump reader: the header, the stream directory, and the three streams the test looks at
# (https://learn.microsoft.com/windows/win32/api/minidumpapiset/).
STREAM_THREAD_LIST, STREAM_MODULE_LIST, STREAM_EXCEPTION = 3, 4, 6


def read_minidump(path):
    d = path.read_bytes()
    sig, _, nstreams, dir_rva, _, _, flags = struct.unpack_from("<4sIIIIIQ", d, 0)
    info = {"signature": sig, "size": len(d), "flags": flags, "streams": [], "modules": [], "threads": []}
    for i in range(nstreams):
        stype, size, rva = struct.unpack_from("<III", d, dir_rva + 12 * i)
        info["streams"].append(stype)
        if stype == STREAM_EXCEPTION:
            tid, _, code, _, _, addr, nparams = struct.unpack_from("<IIIIQQI", d, rva)
            params = struct.unpack_from("<15Q", d, rva + 40)[:nparams]
            ctx_size, ctx_rva = struct.unpack_from("<II", d, rva + 160)
            rip = struct.unpack_from("<Q", d, ctx_rva + 0xF8)[0] if ctx_size >= 0x4D0 else None  # x64 CONTEXT.Rip
            info["exception"] = {"thread": tid, "code": code, "address": addr, "params": params, "rip": rip}
        elif stype == STREAM_MODULE_LIST:
            n = struct.unpack_from("<I", d, rva)[0]
            for j in range(n):
                base, msize, _, _, name_rva = struct.unpack_from("<QIIII", d, rva + 4 + 108 * j)
                ln = struct.unpack_from("<I", d, name_rva)[0]
                name = d[name_rva + 4:name_rva + 4 + ln].decode("utf-16-le", "replace")
                info["modules"].append((base, msize, name))
        elif stype == STREAM_THREAD_LIST:
            n = struct.unpack_from("<I", d, rva)[0]
            for j in range(n):
                tid, _, _, _, _, stack_start, stack_size, _, ctx_size, _ = struct.unpack_from("<IIIIQQIIII", d,
                                                                                                rva + 4 + 48 * j)
                info["threads"].append((tid, stack_start, stack_size, ctx_size))
    return info


def check_minidump(text, out_dir, pc_offset):
    m = re.search(r"^minidump: (.*)$", text, re.M)
    if not check(m is not None, "the report names its minidump"):
        return
    dmp = Path(m.group(1))
    if not check(dmp.is_file() and dmp.parent == out_dir, f"the minidump exists beside the report: {dmp.name}"):
        return
    info = read_minidump(dmp)
    print(f"       {info['size']} bytes, streams {info['streams']}")
    check(info["signature"] == b"MDMP", "the MDMP signature")
    exc = info.get("exception")
    if check(exc is not None, "an exception stream"):
        check(exc["code"] == 0xC0000005, f"its exception code: {exc['code']:#x}")
        check(exc["params"][:2] == (1, 0), f"its parameters, a write at 0: {[hex(p) for p in exc['params']]}")
        exe = next((mod for mod in info["modules"] if mod[2].lower().endswith("dw2003.exe")), None)
        if check(exe is not None, f"the module list names dw2003.exe ({len(info['modules'])} modules)"):
            check(exe[0] <= exc["address"] < exe[0] + exe[1] and exc["address"] - exe[0] == pc_offset,
                  f"the exception address is the report's pc inside dw2003.exe: {exc['address']:#x} = base "
                  f"{exe[0]:#x} + {pc_offset:#x}")
        check(exc["rip"] == exc["address"], f"the thread context's rip is the exception address ({exc['rip']:#x})")
        check(any(t[0] == exc["thread"] and t[3] > 0 for t in info["threads"]),
              f"the thread list has the crashing thread with its context ({len(info['threads'])} threads)")


def main():
    global BINARY, WINE
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--out", default=None)
    ap.add_argument("--wine", action="store_true", help="the Windows build through wine")
    ap.add_argument("--exe", default=None, help="with --wine: the Windows build (default build/port-win/dw2003.exe)")
    args = ap.parse_args()
    WINE = args.wine
    if WINE:
        BINARY = Path(args.exe) if args.exe else ROOT / "build/port-win/dw2003.exe"
        if not BINARY.is_file():
            print(f"crash test: no {BINARY} (scripts/build_windows.sh)")
            return 2
        if not BINARY.with_suffix(".pdb").is_file():
            print(f"crash test: no PDB beside {BINARY}")
            return 2
        if shutil.which("wine") is None:
            print("crash test: no wine on PATH")
            return 2
        prefix = ROOT / "build/wine-prefix"
        prefix.mkdir(parents=True, exist_ok=True)
        RUNNER[:] = ["wine"]
        RUNNER_ENV.update(WINEPREFIX=str(prefix), WINEDEBUG="-all", SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy")
    else:
        if not BINARY.is_file():
            print(f"crash test: no {BINARY.relative_to(ROOT)} (tests/port/run.py builds it)")
            return 2
        if shutil.which("addr2line") is None:
            print("crash test: no addr2line on PATH")
            return 2
    if not DISC.is_file():
        print("crash test: no disc (iso/dw2003.cue)")
        return 2
    out = Path(args.out) if args.out else ROOT / ("build/port-test/crash-wine" if WINE else "build/port-test/crash")
    shutil.rmtree(out, ignore_errors=True)
    out.mkdir(parents=True)
    platform = "Windows" if WINE else "Linux"

    print(f"crash test: --version{' (under wine)' if WINE else ''}")
    r = subprocess.run([*RUNNER, str(BINARY), "--version"], capture_output=True, text=True, errors="replace",
                       env={**os.environ, **RUNNER_ENV})
    m = re.match(r"^dw2003 (\S+) \(([0-9a-f]{40}|unknown)\)$", r.stdout.strip())
    check(m is not None, f"--version prints the version and the commit: {r.stdout.strip()!r}")
    version = m.group(1) if m else ""

    print("crash test: a NULL write at vsync 30 (DW3_PORT_CRASH_AT)")
    d = out / "segv"
    d.mkdir()
    r, reports = run(["--disc", str(DISC), "--max-frames", "100"], d, {"DW3_PORT_CRASH_AT": "30"})
    if WINE:
        check(r.returncode == 0xC0000005 & 0xFF, f"the process ends with the access violation's code (wine: 5; got "
                                                 f"{r.returncode})")
    else:
        check(r.returncode == -11, f"the process dies of SIGSEGV (status {r.returncode})")
    lines = port_lines(r.stderr)
    last = lines[-1] if lines else ""
    check(len(reports) == 1, f"one report written: {[p.name for p in reports]}")
    if reports:
        if WINE:
            check(lines[-2:-1] == [f"port: crash report: {reports[0]}"] and last.startswith("port: minidump: "),
                  f"the last stderr lines name the report and the minidump: {lines[-2:]!r}")
        else:
            check(last == f"port: crash report: {reports[0]}", f"the last stderr line names it: {last!r}")
        text = reports[0].read_text()
        f = fields(text)
        status = str(-0x3FFFFFFB) if WINE else "-11"  # 0xC0000005 as the signed int the launcher reads
        check(f.get("kind") == "crash" and f.get("status") == status, f"kind crash, status {status}")
        check(f.get("build", "").startswith(version + " ("), f"the build line carries --version's stamp: {f.get('build')}")
        check(f.get("platform", "").startswith(platform), f"the platform: {f.get('platform')}")
        check(f.get("vsync") == "30", f"the vsync: {f.get('vsync')}")
        if WINE:
            check(f.get("exception") == "EXCEPTION_ACCESS_VIOLATION (0xc0000005)", f"the exception: {f.get('exception')}")
            check(f.get("access") == "write", f"the access: {f.get('access')}")
        else:
            check(f.get("signal") == "SIGSEGV (11)", f"the signal: {f.get('signal')}")
        check(f.get("fault address") == "0x0", f"the fault address: {f.get('fault address')}")
        pc = re.search(r"^pc: exe\+(0x[0-9a-f]+)$", text, re.M)
        check(pc is not None, "a pc relative to the executable")
        check("crash test: DW3_PORT_CRASH_AT=30" in text, "the log's tail holds the hook's line")
        check(len(re.findall(r"^  #\d+ exe\+0x", text, re.M)) >= 3, "a stack with frames inside the executable")
        s, sym = symbolize(reports[0])
        pcline = next((l for l in sym if l.startswith("pc: ")), "")
        check(s.returncode == 0 and "port_crash_test_write" in pcline,
              f"symbolize.py names the pc's function: {pcline!r}" + (f" (stderr: {s.stderr.strip()})" if s.stderr else ""))
        frames = [l for l in sym if l.startswith("  #")]
        check(any("port_frame" in l for l in frames), "the stack reaches the pump's port_frame")
        if WINE:
            check(frames and "port_crash_test_write" in frames[0],
                  f"frame 0 is the faulting instruction: {frames[0] if frames else '(none)'!r}")
            check(any(re.search(r"\b(game_main|main)\b", l) for l in frames),
                  "the stack walks through the game's frames to main (the units' unwind tables)")
            check_minidump(text, d, int(pc.group(1), 16) if pc else -1)

    print("crash test: a fatal error")
    d = out / "fatal"
    d.mkdir()
    r, reports = run(["--disc", str(out / "missing.cue"), "--max-frames", "10"], d)
    check(r.returncode == 1, f"status 1 (got {r.returncode})")
    check(len(reports) == 1, f"one report written: {[p.name for p in reports]}")
    if reports:
        f = fields(reports[0].read_text())
        check(f.get("kind") == "fatal" and f.get("status") == "1", "kind fatal, status 1")
        check("missing.cue" in f.get("reason", ""), f"the reason: {f.get('reason')}")
        check("signal" not in f and "exception" not in f, "no signal or exception line")
        check("minidump" not in f, "no minidump for a fatal stop")
    lines = port_lines(r.stderr)
    check(len(lines) >= 2 and lines[-2].startswith("port: crash report: ") and lines[-1].startswith("port: exit 1"),
          "the report line comes before the exit line")

    print("crash test: no report on a normal run")
    d = out / "normal"
    d.mkdir()
    r, reports = run(["--disc", str(DISC), "--max-frames", "20"], d)
    check(r.returncode == 0 and not reports, f"status 0 and no report (status {r.returncode}, {len(reports)} report(s))")

    if FAILURES:
        print(f"crash test: FAIL ({len(FAILURES)}): " + "; ".join(FAILURES))
        return 1
    print(f"crash test: pass (outputs: {out})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
