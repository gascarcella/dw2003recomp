"""The dw2003 MCP server: drives the PC port (`build/port/dw2003 --debug`) for local development, over stdio.

    tools/venv/bin/python tools/mcp/server.py        # what .mcp.json runs (cwd: the repository root)

One game at a time (a module-level session). Tools: game_* (lifecycle, pause/step/reset/pace, log), pad_* (the
pad), mem_read/mem_write (host or PS1 memory, see the target grammar in symbols.py), symbol_*, wait_*, screenshot,
state_hash. Game errors become tool errors carrying the game's message. tools/mcp/README.md documents it all.

DW3_MCP_GAME (env): the game command, shell-split, instead of the built binary (the self-test runs fake_game.py).
"""

from __future__ import annotations

import os
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from mcp.server.mcpserver import MCPServer  # noqa: E402
from mcp.server.mcpserver.exceptions import ToolError  # noqa: E402
from mcp.server.mcpserver.utilities.types import Image  # noqa: E402

import png  # noqa: E402
from game import PS1_ARENA_START, Game, GameError, GameExited, button_mask, button_names  # noqa: E402
from symbols import Symbols, Target  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
BINARY = ROOT / "build" / "port" / "dw2003"
BINARY_SDL = ROOT / "build" / "port-sdl" / "dw2003"

mcp = MCPServer(
    "dw2003",
    instructions="Drives the Digimon World 2003 PC port (build/port/dw2003) through its debug channel: start it "
                 "(game_start), run frames (game_step, wait_*), press buttons (pad_*), read and write memory by "
                 "symbol name or address (mem_read/mem_write; 'ps1:' targets use PS1 addresses), take screenshots. "
                 "One game at a time. Memory reads happen between two vsyncs, so runs stay deterministic.",
)

_game: Game | None = None
_symbols = Symbols(ROOT)


# ---- helpers ----

def _require() -> Game:
    if _game is None:
        raise ToolError("no game running: game_start first (or game_attach)")
    if not _game.alive:
        msg = _game.log_text(20)
        raise ToolError("the game exited" + (f"; stderr tail:\n{msg}" if msg else ""))
    return _game


def _call(fn, *a, **kw):
    """Run a Game method; the game's errors and death become tool errors."""
    try:
        return fn(*a, **kw)
    except GameError as e:
        raise ToolError(f"game error: {e}") from e
    except GameExited as e:
        raise ToolError(str(e)) from e
    except (OSError, TimeoutError, ValueError) as e:
        raise ToolError(f"{type(e).__name__}: {e}") from e


def _resolve(target: str) -> Target:
    try:
        return _symbols.resolve(target)
    except ValueError as e:
        raise ToolError(str(e)) from e


def _read(g: Game, t: Target, size: int, count: int) -> bytes:
    total = size * count
    if not t.ps1:
        return _call(g.peek, t.addr, total)
    if t.addr >= PS1_ARENA_START:
        return _call(g.peek_ps1, t.addr, total)
    if size not in (1, 2, 4):
        raise ToolError("a state-mapped PS1 address (below the arena) reads 1, 2 or 4 bytes at a time")
    return b"".join(_call(g.peek_ps1, t.addr + i * size, size) for i in range(count))


def _write(g: Game, t: Target, data: bytes, size: int) -> int:
    if not t.ps1:
        return _call(g.poke, t.addr, data)
    if t.addr >= PS1_ARENA_START:
        return _call(g.poke_ps1, t.addr, data)
    if size not in (1, 2, 4):
        raise ToolError("a state-mapped PS1 address (below the arena) writes 1, 2 or 4 bytes at a time")
    n = 0
    for i in range(0, len(data), size):
        n += _call(g.poke_ps1, t.addr + i, data[i:i + size])
    return n


def _game_command(window: bool) -> tuple[list[str], str]:
    override = os.environ.get("DW3_MCP_GAME")
    if override:
        return shlex.split(override), "DW3_MCP_GAME"
    if window and BINARY_SDL.exists():
        return [str(BINARY_SDL)], str(BINARY_SDL.relative_to(ROOT))
    if BINARY.exists():
        return [str(BINARY)], str(BINARY.relative_to(ROOT))
    raise ToolError(f"no binary at {BINARY.relative_to(ROOT)}: game_start with build=True "
                    "(or cmake -S port -B build/port -G Ninja && cmake --build build/port)")


def _build(window: bool) -> str:
    if window:
        cmds = [["cmake", "-S", "port", "-B", "build/port-sdl", "-G", "Ninja", "-DDW3_PORT_SDL=ON"],
                ["cmake", "--build", "build/port-sdl"]]
    else:
        cmds = [["cmake", "-S", "port", "-B", "build/port", "-G", "Ninja"], ["cmake", "--build", "build/port"]]
    for cmd in cmds:
        r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
        if r.returncode != 0:
            raise ToolError(f"{' '.join(cmd)} failed ({r.returncode}):\n{(r.stdout + r.stderr)[-4000:]}")
    return " && ".join(" ".join(c) for c in cmds)


def _status_dict(g: Game) -> dict:
    d = {"running": True, "pid": g.pid, "socket": g.socket_path, "command": g.command}
    d.update(_call(g.status))
    d["pad_buttons"] = button_names(int(d.get("pad", 0)))
    return d


# ---- lifecycle ----

@mcp.tool()
def game_start(window: bool = False, disc: str = "iso/dw2003.cue", cd_speed: str = "instant",
               pace: int | None = None, memcard1: str | None = None, memcard2: str | None = None,
               config: str | None = None, mods: bool = False, extra_args: list[str] = [],
               build: bool = False, hold: bool = False, timeout: float = 60.0) -> dict:
    """Start the PC port with its debug channel and connect. Stops a game already running first.

    hold=True starts the game paused at its first vsync (frame 1), before anything ran: game_resume/game_step/
    wait_* go on from there, so frames and hashes are reproducible run to run. Without it the game runs
    unthrottled until the first command lands (about 150 frames in, past the boot).

    Headless by default; window=True adds --window (uses build/port-sdl/dw2003 when it exists, else
    build/port/dw2003, which fails without SDL; the result says which). cd_speed: "instant" (default) or "realistic".
    pace: vsyncs per second (0 = unthrottled); default 0 headless, 50 (real time) with a window; game_pace changes
    it later. memcard1/memcard2: .mcd images (created if missing) or "none"; default fresh in-memory cards.
    config: a settings file (--config, what the launcher passes: disc, cards, mods). mods=True passes --script-mods
    (the settings' mods stay on even under a --script given in extra_args; without a config there are no mods).
    extra_args: more dw2003 options (--trace, --refresh 60, --log FILE, --script JSON, ...). build=True runs the
    CMake configure+build first. Paths are relative to the repository root. Returns the status (frame, stage, map,
    paused, pad_owner, ...), the binary used and the command line."""
    global _game
    if _game is not None:
        game_stop()
    built = _build(window) if build else None
    command, which = _game_command(window)
    args = ["--disc", disc, "--cd-speed", cd_speed]
    if pace is None:
        pace = 50 if window else 0
    args += ["--fps", str(int(pace))]
    if window:
        args.append("--window")
    if memcard1:
        args += ["--memcard1", memcard1]
    if memcard2:
        args += ["--memcard2", memcard2]
    if config:
        args = ["--config", config] + args
    if mods:
        args.append("--script-mods")
    args += list(extra_args)
    try:
        _game = Game.spawn(command + args, timeout=timeout, cwd=str(ROOT), hold=hold)
    except (GameExited, TimeoutError, OSError) as e:
        raise ToolError(f"game_start failed: {e}") from e
    d = _status_dict(_game)
    d["binary"] = which
    if built:
        d["built"] = built
    return d


@mcp.tool()
def game_attach(socket_path: str) -> dict:
    """Connect to a game the user started themselves with `dw2003 --debug SOCKET` (no process control, no log;
    game_stop then only sends quit). Replaces the current session."""
    global _game
    if _game is not None:
        game_stop()
    try:
        _game = Game.attach(socket_path)
    except OSError as e:
        raise ToolError(f"attach to {socket_path} failed: {e}") from e
    return _status_dict(_game)


@mcp.tool()
def game_stop() -> dict:
    """Quit the game (then kill it after 3 s if it lingers). Returns the exit status and the last stderr lines."""
    global _game
    if _game is None:
        return {"running": False, "stopped": False}
    g, _game = _game, None
    status = g.stop()
    return {"running": False, "stopped": True, "exit_status": status, "log": g.log(10)}


@mcp.tool()
def game_status() -> dict:
    """The game's status: frame, stage (overlay slot index), file (overlay file id), map, paused, pace, rate, window,
    script, pad_owner (debug/script/window/none), pad mask and names; running=false when no game is up."""
    if _game is None:
        return {"running": False}
    if not _game.alive:
        return {"running": False, "exited": True, "log": _game.log(20)}
    return _status_dict(_game)


@mcp.tool()
def game_log(lines: int = 50) -> dict:
    """The last `lines` lines of the game's stderr (its log: port messages, --trace output, mod messages)."""
    if _game is None:
        raise ToolError("no game running")
    return {"lines": _game.log(lines)}


# ---- execution ----

@mcp.tool()
def game_pause() -> dict:
    """Hold the game at the next vsync boundary (idempotent). Returns the frame."""
    return {"frame": _call(_require().pause)}


@mcp.tool()
def game_resume() -> dict:
    """Let the game run (idempotent). Returns the frame."""
    return {"frame": _call(_require().resume)}


@mcp.tool()
def game_step(frames: int = 1) -> dict:
    """Run exactly `frames` vsyncs, then pause (pauses first if running). Returns the frame reached."""
    if frames < 1:
        raise ToolError("frames must be >= 1")
    return {"frame": _call(_require().step, frames)}


@mcp.tool()
def game_reset() -> dict:
    """Reset the console (the game restarts from boot; the channel, its pad ownership and pause state survive)."""
    return {"frame": _call(_require().reset)}


@mcp.tool()
def game_pace(fps: int) -> dict:
    """Set the pace: `fps` vsyncs per second, 0 = unthrottled."""
    return {"pace": _call(_require().pace, fps)}


# ---- the pad ----

@mcp.tool()
def pad_press(buttons: list[str], frames: int = 2, release: int = 2) -> dict:
    """Press buttons for `frames` vsyncs then release for `release` vsyncs; returns after they ran. Names: SELECT,
    L3, R3, START, UP, RIGHT, DOWN, LEFT, L2, R2, L1, R1, TRIANGLE, CIRCLE, CROSS, SQUARE (any case; several at
    once). The channel owns the pad from the first pad_* call until pad_free. A menu needs ~2 frames pressed; the
    opening movie's skip needs a long hold (30/30)."""
    g = _require()
    try:
        mask = button_mask(buttons)
    except ValueError as e:
        raise ToolError(str(e)) from e
    frame = _call(g.pad, mask, frames, release, True)
    return {"frame": frame, "buttons": button_names(mask), "mask": mask}


@mcp.tool()
def pad_hold(buttons: list[str]) -> dict:
    """Hold buttons down until the next pad_* call (returns at once; use game_step / wait_* to run frames)."""
    g = _require()
    try:
        mask = button_mask(buttons)
    except ValueError as e:
        raise ToolError(str(e)) from e
    return {"frame": _call(g.pad, mask, 0, 0, False), "buttons": button_names(mask), "mask": mask}


@mcp.tool()
def pad_release() -> dict:
    """Release every button (the channel keeps owning the pad)."""
    return {"frame": _call(_require().pad, 0, 0, 0, False)}


@mcp.tool()
def pad_free() -> dict:
    """Give the pad back to the window (or to nobody)."""
    _call(_require().pad_free)
    return {"pad_owner": "freed"}


# ---- memory ----

@mcp.tool()
def mem_read(target: str, size: int = 4, count: int = 1, signed: bool = False, fmt: str = "int") -> dict:
    """Read `count` values of `size` bytes (1/2/4/8, little-endian) at `target`. Targets: a symbol of the port's
    ELF ("gamestate_data", "gamestate_data+0x10": host memory), a host address ("0x7f...", decimal), or
    "ps1:0x80048D34" / "ps1:name" / "ps1:name+8" (a PS1 address: the arena directly, other globals through the
    port's state map, 1/2/4 bytes each). fmt: "int" (a list of ints), "hex" (one hex string of all the bytes),
    "str" (the bytes as text up to the first NUL). The read happens between two vsyncs."""
    if size not in (1, 2, 4, 8):
        raise ToolError("size must be 1, 2, 4 or 8")
    if count < 1 or size * count > 65536:
        raise ToolError("count must be >= 1 and size*count <= 65536")
    g = _require()
    t = _resolve(target)
    data = _read(g, t, size, count)
    d = t.as_dict()
    d.update(size=size, count=count)
    if fmt == "hex":
        d["hex"] = data.hex()
    elif fmt == "str":
        d["str"] = data.split(b"\0", 1)[0].decode("latin-1")
    elif fmt == "int":
        vals = [int.from_bytes(data[i:i + size], "little", signed=signed) for i in range(0, len(data), size)]
        d["values"] = vals
        if count == 1:
            d["value"] = vals[0]
            d["value_hex"] = hex(vals[0] & ((1 << (8 * size)) - 1))
    else:
        raise ToolError('fmt must be "int", "hex" or "str"')
    return d


@mcp.tool()
def mem_write(target: str, values: list[int] | str, size: int = 4) -> dict:
    """Write at `target` (grammar as mem_read): `values` is a list of ints (each `size` bytes, little-endian) or
    one hex string of raw bytes ("0a1b2c"; `size` then only matters for state-mapped PS1 globals). The game must
    be alive; writes land between two vsyncs."""
    if size not in (1, 2, 4, 8):
        raise ToolError("size must be 1, 2, 4 or 8")
    g = _require()
    t = _resolve(target)
    try:
        if isinstance(values, str):
            data = bytes.fromhex(values.strip().removeprefix("0x"))
        else:
            data = b"".join(int(v).to_bytes(size, "little", signed=int(v) < 0) for v in values)
    except (ValueError, OverflowError) as e:
        raise ToolError(f"bad values: {e}") from e
    if not data:
        raise ToolError("nothing to write")
    n = _write(g, t, data, size)
    d = t.as_dict()
    d["written"] = n
    return d


# ---- symbols ----

@mcp.tool()
def symbol_lookup(name: str) -> dict:
    """A symbol's host address and size (from build/port/dw2003 via nm) and its PS1 address, size and type (from
    config/symbol_addrs.txt; plus the overlays that define it, from config/<overlay>.symbols.txt)."""
    return _symbols.lookup(name)


@mcp.tool()
def symbol_search(pattern: str, limit: int = 50) -> dict:
    """Symbols whose name matches `pattern` (a case-insensitive regex; a plain substring works), host and PS1
    tables merged, at most `limit`."""
    try:
        matches = _symbols.search(pattern, limit)
    except Exception as e:  # a bad regex
        raise ToolError(f"bad pattern: {e}") from e
    return {"count": len(matches), "matches": matches}


# ---- waits ----

@mcp.tool()
def wait_until(target: str, value: int, size: int = 4, signed: bool = False, timeout_frames: int = 600) -> dict:
    """Run frames until the `size`-byte read at `target` (grammar as mem_read) equals `value`, or `timeout_frames`
    passed; checked once before running and after every vsync. Leaves the game paused. Returns {frame, hit}."""
    if size not in (1, 2, 4):
        raise ToolError("size must be 1, 2 or 4")
    g = _require()
    t = _resolve(target)
    kw = {"ps1": t.addr} if t.ps1 else {"addr": t.addr}
    r = _call(g.wait, value=value, size=size, signed=signed, timeout=timeout_frames, **kw)
    d = t.as_dict()
    d.update(r)
    d["hit"] = bool(r.get("hit"))
    return d


@mcp.tool()
def wait_stage(stage: int, timeout_frames: int = 3000) -> dict:
    """Run frames until the loaded stage (overlay slot index, as the pad scripts' wait_stage: 22 = CNTY_SEL,
    14 = STDWTITL, ...) equals `stage`, or the timeout. Leaves the game paused. Returns {frame, hit}."""
    r = _call(_require().wait, ps1_stage=stage, timeout=timeout_frames)
    return {"stage": stage, "frame": r.get("frame"), "hit": bool(r.get("hit"))}


@mcp.tool()
def wait_map(map: int, timeout_frames: int = 3000) -> dict:
    """Run frames until the current map id equals `map` (0xE02 the opening movie, 0xE00 the title screen, 0x2D7 the
    first field map, ...), or the timeout. Leaves the game paused. Returns {frame, hit}."""
    r = _call(_require().wait, ps1_map=map, timeout=timeout_frames)
    return {"map": map, "frame": r.get("frame"), "hit": bool(r.get("hit"))}


# ---- observation ----

@mcp.tool(structured_output=False)
def screenshot(path: str | None = None) -> list:
    """The current display (320x240 or the game's mode) as a PNG image; with `path`, also saved there (PNG;
    relative to the repository root). Works headless: the frame the game last displayed."""
    g = _require()
    with tempfile.NamedTemporaryFile(prefix="dw2003-shot-", suffix=".ppm", delete=False,
                                     dir=g.tmpdir) as f:
        ppm_path = f.name
    try:
        r = _call(g.screenshot, ppm_path)
        try:
            with open(ppm_path, "rb") as f:
                data = png.ppm_to_png(f.read())
        except (OSError, ValueError) as e:
            raise ToolError(f"reading the game's PPM failed: {e}") from e
    finally:
        try:
            os.unlink(ppm_path)
        except OSError:
            pass
    info = {"w": r.get("w"), "h": r.get("h"), "frame": _call(g.status).get("frame"), "png_bytes": len(data)}
    if path:
        out = Path(path)
        if not out.is_absolute():
            out = ROOT / out
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_bytes(data)
        info["path"] = str(out)
    return [info, Image(data=data, format="png")]


@mcp.tool()
def state_hash() -> dict:
    """The SHA-1 of gamestate_data's PS1 image (full and stable), as a replay checkpoint computes it, plus frame."""
    return _call(_require().hash)


if __name__ == "__main__":
    mcp.run(transport="stdio")
