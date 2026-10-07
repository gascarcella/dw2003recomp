#!/usr/bin/env python3
"""Names the addresses of a crash report (docs/PORT.md "Crash report"): `pc: exe+0x...` and the stack's `#N exe+0x...`
lines are resolved with addr2line against the build that made it: the unstripped game (build/port*/dw2003) or the
release's debug file (dw2003-<version>-x86_64.debug beside the AppImage on the release page; the report's `build:` line
says which release). A return address is resolved one byte back (the call site's line); the pc as it is.

Usage: scripts/symbolize.py REPORT [--binary FILE] [--addr2line TOOL]
  --binary FILE   the unstripped game or its .debug file (default: build/port/dw2003)
Prints the report with each resolved line followed by `function (file:line)`. Exit 0 when every exe+ address resolved
to a function, 1 when the binary is missing or an address did not resolve (a report from another build?).
"""
import argparse
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ADDR = re.compile(r"^(pc: |  #\d+ )exe\+(0x[0-9a-fA-F]+)")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("report")
    ap.add_argument("--binary", default=str(ROOT / "build/port/dw2003"))
    ap.add_argument("--addr2line", default="addr2line")
    args = ap.parse_args()
    binary = Path(args.binary)
    if not binary.is_file():
        sys.exit(f"symbolize: no such binary: {binary}")
    if shutil.which(args.addr2line) is None:
        sys.exit(f"symbolize: no {args.addr2line} on PATH (binutils)")
    lines = Path(args.report).read_text(errors="replace").splitlines()
    wanted = []  # (line index, address to resolve)
    for i, line in enumerate(lines):
        m = ADDR.match(line)
        if m:
            addr = int(m.group(2), 16)
            wanted.append((i, addr if m.group(1).startswith("pc") else addr - 1))
    resolved = {}
    if wanted:
        out = subprocess.run([args.addr2line, "-f", "-C", "-e", str(binary)] + [hex(a) for _, a in wanted],
                             capture_output=True, text=True, check=True).stdout.splitlines()
        for (i, _), func, where in zip(wanted, out[0::2], out[1::2]):
            resolved[i] = (func, where)
    unresolved = 0
    for i, line in enumerate(lines):
        if i in resolved:
            func, where = resolved[i]
            if func == "??":
                unresolved += 1
            print(f"{line}  {func} ({where})")
        else:
            print(line)
    if unresolved:
        print(f"symbolize: {unresolved} address(es) did not resolve: is {binary} the build of the report's `build:` "
              f"line?", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
