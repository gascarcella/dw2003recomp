#!/usr/bin/env python3
"""The crash report (port/src/crash.c; docs/PORT.md "Crash report"): what a tester's report must hold.

Usage: tests/port/crash.py [--out DIR]

Needs build/port/dw2003 (tests/port/run.py builds it), the disc and addr2line. Three short runs (~3 s):
  - DW3_PORT_CRASH_AT=30 (a NULL write at vsync 30): the process dies of SIGSEGV (status -11, as without the handler),
    its last stderr line names the report, the report has the build, the platform, the vsync (30), the signal, the
    fault address (0x0), a pc and a stack, and scripts/symbolize.py resolves the pc to port_crash_test_write with the
    unstripped build;
  - a fatal error (--disc of a missing file): status 1, a report of kind `fatal` with the reason and the log's tail;
  - `--version` prints the stamp the report's `build:` line carries.
Exit codes: 0 pass, 1 fail, 2 something missing.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BINARY = ROOT / "build/port/dw2003"
DISC = ROOT / "iso/dw2003.cue"
SYMBOLIZE = ROOT / "scripts/symbolize.py"

FAILURES = []


def check(cond, what):
    print(f"  {'ok  ' if cond else 'FAIL'} {what}")
    if not cond:
        FAILURES.append(what)
    return cond


def run(args, out_dir, env=None):
    e = dict(os.environ)
    e.update(env or {})
    r = subprocess.run([str(BINARY), *args, "--crash-dir", str(out_dir)], cwd=ROOT, env=e, capture_output=True,
                       text=True, timeout=120)
    reports = sorted(out_dir.glob("crash-*.txt"))
    return r, reports


def fields(text):
    return {m.group(1): m.group(2) for m in re.finditer(r"^([a-z 0-9]+): (.*)$", text, re.M)}


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--out", default=str(ROOT / "build/port-test/crash"))
    args = ap.parse_args()
    if not BINARY.is_file():
        print(f"crash test: no {BINARY.relative_to(ROOT)} (tests/port/run.py builds it)")
        return 2
    if not DISC.is_file():
        print("crash test: no disc (iso/dw2003.cue)")
        return 2
    if shutil.which("addr2line") is None:
        print("crash test: no addr2line on PATH")
        return 2
    out = Path(args.out)
    shutil.rmtree(out, ignore_errors=True)
    out.mkdir(parents=True)

    print("crash test: --version")
    r = subprocess.run([str(BINARY), "--version"], capture_output=True, text=True, check=True)
    m = re.match(r"^dw2003 (\S+) \(([0-9a-f]{40}|unknown)\)$", r.stdout.strip())
    check(m is not None, f"--version prints the version and the commit: {r.stdout.strip()!r}")
    version = m.group(1) if m else ""

    print("crash test: a NULL write at vsync 30 (DW3_PORT_CRASH_AT)")
    d = out / "segv"
    d.mkdir()
    r, reports = run(["--disc", str(DISC), "--max-frames", "100"], d, {"DW3_PORT_CRASH_AT": "30"})
    check(r.returncode == -11, f"the process dies of SIGSEGV (status {r.returncode})")
    last = r.stderr.strip().splitlines()[-1] if r.stderr.strip() else ""
    check(len(reports) == 1, f"one report written: {[p.name for p in reports]}")
    if reports:
        check(last == f"port: crash report: {reports[0]}", f"the last stderr line names it: {last!r}")
        text = reports[0].read_text()
        f = fields(text)
        check(f.get("kind") == "crash" and f.get("status") == "-11", "kind crash, status -11")
        check(f.get("build", "").startswith(version + " ("), f"the build line carries --version's stamp: {f.get('build')}")
        check(f.get("platform", "").startswith("Linux"), f"the platform: {f.get('platform')}")
        check(f.get("vsync") == "30", f"the vsync: {f.get('vsync')}")
        check(f.get("signal") == "SIGSEGV (11)", f"the signal: {f.get('signal')}")
        check(f.get("fault address") == "0x0", f"the fault address: {f.get('fault address')}")
        check(re.search(r"^pc: exe\+0x[0-9a-f]+$", text, re.M) is not None, "a pc relative to the executable")
        check("crash test: DW3_PORT_CRASH_AT=30" in text, "the log's tail holds the hook's line")
        check(len(re.findall(r"^  #\d+ exe\+0x", text, re.M)) >= 3, "a stack with frames inside the executable")
        s = subprocess.run([sys.executable, str(SYMBOLIZE), str(reports[0]), "--binary", str(BINARY)],
                           capture_output=True, text=True)
        pc = next((l for l in s.stdout.splitlines() if l.startswith("pc: ")), "")
        check(s.returncode == 0 and "port_crash_test_write" in pc,
              f"symbolize.py names the pc's function: {pc!r}" + (f" (stderr: {s.stderr.strip()})" if s.stderr else ""))
        check(any("port_frame" in l for l in s.stdout.splitlines() if l.startswith("  #")),
              "the stack reaches the pump's port_frame")

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
        check("signal" not in f, "no signal line")
    lines = r.stderr.strip().splitlines()
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
