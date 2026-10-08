#!/usr/bin/env python3
"""The port's debug channel (`--debug SOCKET`, psxstack/runtime/debug.c) and the MCP server's offline self-test (tools/mcp).

Usage: tests/port/debug.py [--out DIR] [--no-selftest]

Needs build/port/dw2003 (tests/port/run.py builds it) and the disc. Starts the game headless with `--debug` and
`--cd-speed instant` through tools/mcp/game.py (the plain client), and checks, on one run (~5 s):
  - the socket path fits an AF_UNIX address (sun_path is 108 bytes);
  - `wait` on stage 22 (CNTY_SEL, new_game.json's first wait_stage) hits and leaves the game paused;
  - `step 10` advances the frame by exactly 10; `pad START` with sync advances it by frames + release;
  - `peek_ps1` of overlay_module.stage through the state map equals `peek` of the host global (nm), and both are 22;
  - `poke_ps1` of a byte in the arena reads back through `peek_ps1` and the host alias;
  - `screenshot` writes a binary PPM (P6, 320 wide); `hash` gives two 40-hex SHA-1s;
  - `quit` with status 3 ends the process with exit status 3.
Then runs tools/mcp/selftest.py (the client, the symbols and the server's tools against fake_game.py, offline); it is
skipped with a message when the `mcp` package is not installed (scripts/setup.sh venv).
Exit codes: 0 pass, 1 fail, 2 something missing.
"""
import argparse
import importlib.util
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import mcp_game  # noqa: E402
from mcp_game import Game, GameError, GameExited  # noqa: E402  (the debug channel's client; stdlib only)

BINARY = ROOT / "build/port/dw2003"
DISC = ROOT / "iso/dw2003.cue"
SELFTEST = ROOT / "psxstack/tools/mcp/selftest.py"
STAGE_CNTY_SEL = 22  # new_game.json's first wait_stage
SUN_PATH_MAX = 108  # sizeof(sockaddr_un.sun_path), NUL included

FAILURES = []


def check(cond, what):
    print(f"  {'ok  ' if cond else 'FAIL'} {what}")
    if not cond:
        FAILURES.append(what)


def channel(out):
    """One game through the channel's ops."""
    syms = mcp_game.symbols(BINARY)
    ps1_stage = syms.ps1("overlay_module")
    host_stage = syms.host("overlay_module")
    if ps1_stage is None or host_stage is None:
        raise RuntimeError("overlay_module: not in config/symbol_addrs.txt or not in the ELF (nm)")

    g = Game.spawn([str(BINARY), "--disc", str(DISC), "--cd-speed", "instant"], cwd=str(ROOT))
    try:
        check(len(g.socket_path.encode()) < SUN_PATH_MAX, f"socket path fits sun_path: {g.socket_path}")
        r = g.wait(ps1_stage=STAGE_CNTY_SEL, timeout=3000)
        check(r["hit"] == 1, f"wait stage {STAGE_CNTY_SEL} (CNTY_SEL): {r}")
        st = g.status()
        check(st["paused"] == 1 and st["stage"] == STAGE_CNTY_SEL, f"paused at stage {st['stage']}, frame {st['frame']}")
        f0 = g.pause()  # idempotent while paused
        f1 = g.step(10)
        check(f1 == f0 + 10, f"step 10: frame {f0} -> {f1}")
        f2 = g.pad("START", frames=2, release=2, sync=True)
        check(f2 == f1 + 4, f"pad START 2+2 sync: frame {f1} -> {f2}")
        check(g.status()["pad_owner"] == "debug", "the channel owns the pad")
        # overlay_module.stage: the state map (PS1 address) and the host global (nm) must agree.
        via_ps1 = int.from_bytes(g.peek_ps1(ps1_stage.addr, 4), "little", signed=True)
        via_host = int.from_bytes(g.peek(host_stage.addr, 4), "little", signed=True)
        check(via_ps1 == via_host == STAGE_CNTY_SEL,
              f"overlay_module.stage: ps1:{ps1_stage.addr:#x} -> {via_ps1}, host {host_stage.addr:#x} -> {via_host}")
        # A byte of the arena, written by its PS1 address, read back by both routes (a heap address past the slots'
        # start; a byte the game is not using at the country select: the top of the heap).
        addr = 0x80082CB0 + 0x100000
        before = g.peek_ps1(addr, 1)
        poked = bytes([(before[0] + 1) & 0xFF])
        check(g.poke_ps1(addr, poked) == 1 and g.peek_ps1(addr, 1) == poked, f"poke_ps1/peek_ps1 arena {addr:#x}")
        arena = syms.host("port_arena")
        if arena is not None:
            check(g.peek(arena.addr + 0x100000, 1) == poked, "the same byte through the host arena (port_arena)")
        g.poke_ps1(addr, before)
        shot = out / "cnty_sel.ppm"
        r = g.screenshot(str(shot))
        data = shot.read_bytes() if shot.exists() else b""
        head = data.split(b"\n", 3)
        check(len(head) >= 4 and head[0] == b"P6" and head[1].split()[0] == b"320" and r.get("w") == 320,
              f"screenshot: {r}, {len(data)} bytes, header {head[:3]}")
        h = g.hash()
        hexes = [h.get("sha1", ""), h.get("stable_sha1", "")]
        check(all(len(x) == 40 and all(c in "0123456789abcdef" for c in x) for x in hexes), f"hash: {h}")
        g.pad_free()
        g.quit(3)
        status = g.proc.wait(timeout=5)
        check(status == 3, f"quit 3: exit status {status}")
    except (GameError, GameExited, TimeoutError) as e:
        check(False, f"{type(e).__name__}: {e}")
    finally:
        g.stop()


def selftest():
    """tools/mcp/selftest.py (offline, against fake_game.py); needs the mcp package."""
    print("--- tools/mcp/selftest.py")
    if importlib.util.find_spec("mcp") is None:
        print("  skipped: no mcp package in this Python (scripts/setup.sh venv installs tools/requirements.txt)")
        return
    proc = subprocess.run([sys.executable, str(SELFTEST)], cwd=ROOT, capture_output=True, text=True, timeout=120)
    lines = (proc.stdout + proc.stderr).splitlines()
    fails = [l for l in lines if l.startswith("FAIL")]
    check(proc.returncode == 0, f"selftest exit {proc.returncode}: {lines[-1] if lines else ''}"
          + ("".join("\n       " + f for f in fails)))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=str(ROOT / "build/port-test/debug"))
    ap.add_argument("--no-selftest", action="store_true")
    args = ap.parse_args()
    if not BINARY.exists():
        print(f"missing: {BINARY.relative_to(ROOT)} (tests/port/run.py builds it)")
        return 2
    if not DISC.exists():
        print(f"missing: {DISC.relative_to(ROOT)} (scripts/setup.sh disc)")
        return 2
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    print(f"--- the debug channel: {BINARY.relative_to(ROOT)} --debug, --cd-speed instant, to CNTY_SEL")
    channel(out)
    if not args.no_selftest:
        selftest()
    if FAILURES:
        print(f"debug.py: {len(FAILURES)} failure(s)")
        for f in FAILURES:
            print(f"  {f}")
        return 1
    print("debug.py: pass")
    return 0


if __name__ == "__main__":
    sys.exit(main())
