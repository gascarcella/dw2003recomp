#!/usr/bin/env python3
"""Names the addresses of a crash report (docs/PORT.md "Crash report"): `pc: exe+0x...` and the stack's `#N exe+0x...`
lines are resolved against the build that made it. A Linux report: addr2line over the unstripped game (build/port*/dw2003)
or the release's debug file (dw2003-<version>-x86_64.debug beside the AppImage on the release page; the report's
`build:` line says which release). A Windows report (its `platform:` line says Windows, or --binary ends in .exe):
llvm-symbolizer (llvm-mingw's, tools/llvm-mingw) over the executable and its PDB (dw2003.pdb beside it, or --pdb; the
release's symbols zip holds it). A return address is resolved one byte back (the call site's line); the pc as it is.

Usage: scripts/symbolize.py REPORT [--binary FILE] [--pdb FILE] [--addr2line TOOL] [--symbolizer TOOL]
  --binary FILE   the unstripped game or its .debug file (default: build/port/dw2003), or dw2003.exe
  --pdb FILE      the PDB of a Windows binary (default: <binary stem>.pdb beside it)
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


def tool_dirs():
    """This checkout's tools/, then the main checkout's (scripts/setup.sh installs there; worktrees link it)."""
    dirs = [ROOT / "tools"]
    r = subprocess.run(["git", "-C", str(ROOT), "rev-parse", "--path-format=absolute", "--git-common-dir"],
                       capture_output=True, text=True)
    if r.returncode == 0 and r.stdout.strip():
        dirs.append(Path(r.stdout.strip()).parent / "tools")
    return dirs


def find_symbolizer(given):
    if given:
        return given if shutil.which(given) else None
    for d in tool_dirs():
        p = d / "llvm-mingw/bin/llvm-symbolizer"
        if p.is_file():
            return str(p)
    return shutil.which("llvm-symbolizer")


def resolve_linux(addr2line, binary, addrs):
    """[(function, where)] through addr2line."""
    out = subprocess.run([addr2line, "-f", "-C", "-e", str(binary)] + [hex(a) for a in addrs],
                         capture_output=True, text=True, check=True).stdout.splitlines()
    return list(zip(out[0::2], out[1::2]))


def resolve_windows(symbolizer, binary, pdb, addrs):
    """[(function, where)] through llvm-symbolizer with the PDB; the addresses are image-relative (RVAs)."""
    out = subprocess.run([symbolizer, f"--obj={binary}", f"--pdb={pdb}", "--relative-address", "--functions",
                          "--inlining=false", "--relativenames"] + [hex(a) for a in addrs],
                         capture_output=True, text=True, check=True).stdout
    pairs = []
    for block in out.strip("\n").split("\n\n"):
        lines = block.splitlines() + ["", ""]
        func, where = lines[0] or "??", lines[1] or "??:0"
        pairs.append((func, re.sub(r":\d+$", "", where)))  # drop the column
    return pairs


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("report")
    ap.add_argument("--binary", default=str(ROOT / "build/port/dw2003"))
    ap.add_argument("--pdb", default=None)
    ap.add_argument("--addr2line", default="addr2line")
    ap.add_argument("--symbolizer", default=None, help="llvm-symbolizer (default: tools/llvm-mingw's, else PATH)")
    args = ap.parse_args()
    binary = Path(args.binary)
    if not binary.is_file():
        sys.exit(f"symbolize: no such binary: {binary}")
    lines = Path(args.report).read_text(errors="replace").splitlines()
    windows = binary.suffix.lower() == ".exe" or any(l.startswith("platform: Windows") for l in lines)
    wanted = []  # (line index, address to resolve)
    for i, line in enumerate(lines):
        m = ADDR.match(line)
        if m:
            addr = int(m.group(2), 16)
            wanted.append((i, addr if m.group(1).startswith("pc") else addr - 1))
    resolved = {}
    if wanted:
        if windows:
            pdb = Path(args.pdb) if args.pdb else binary.with_suffix(".pdb")
            if not pdb.is_file():
                sys.exit(f"symbolize: no PDB for {binary}: {pdb} (the release's symbols zip; or --pdb)")
            symbolizer = find_symbolizer(args.symbolizer)
            if symbolizer is None:
                sys.exit("symbolize: no llvm-symbolizer (scripts/setup.sh llvm-mingw, or --symbolizer)")
            pairs = resolve_windows(symbolizer, binary, pdb, [a for _, a in wanted])
        else:
            if shutil.which(args.addr2line) is None:
                sys.exit(f"symbolize: no {args.addr2line} on PATH (binutils)")
            pairs = resolve_linux(args.addr2line, binary, [a for _, a in wanted])
        for (i, _), (func, where) in zip(wanted, pairs):
            resolved[i] = (func, where)
    unresolved = 0
    for i, line in enumerate(lines):
        if i in resolved:
            func, where = resolved[i]
            if func in ("??", ""):
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
