"""A client for the PC port's debug channel (`dw2003 --debug SOCKET`; the protocol is in tools/mcp/README.md).

Plain Python, no MCP dependency: usable from scripts.

    from game import Game
    g = Game.spawn(["build/port/dw2003", "--disc", "iso/dw2003.cue", "--cd-speed", "instant"])
    g.wait(ps1_stage=22, timeout=3000)
    g.pad(Game.mask(["START"]), frames=2, release=2, sync=True)
    print(g.status(), g.peek(0x12345678, 4).hex())
    g.stop()

Transport: newline-delimited JSON over a Unix stream socket, one request per line, answered in order. Deferred ops
(`step`, `wait`, `pad` with sync) answer only when done: the client waits for them, polling the process meanwhile so
a crash surfaces as GameExited with the captured stderr instead of a hang.
"""

from __future__ import annotations

import collections
import json
import os
import shutil
import socket
import subprocess
import tempfile
import threading
import time

# script.c's bit order (active high).
BUTTONS = ["SELECT", "L3", "R3", "START", "UP", "RIGHT", "DOWN", "LEFT",
           "L2", "R2", "L1", "R1", "TRIANGLE", "CIRCLE", "CROSS", "SQUARE"]

SUN_PATH_MAX = 108  # sizeof(sockaddr_un.sun_path), NUL included: a longer socket path cannot be bound

PS1_ARENA_START = 0x80082CB0  # peek_ps1/poke_ps1 read the arena directly from here (any length)

DEFERRED_OPS = ("step", "wait")  # and "pad" with sync


class GameError(Exception):
    """An error response from the game (`ok: false`): the game's message."""


class GameExited(Exception):
    """The game process is gone (or never came up); the message carries its exit status and stderr tail."""


def button_mask(buttons) -> int:
    """Button names (any case; a list, or one space/|/+-separated string) -> the pad's u16 mask."""
    if isinstance(buttons, int):
        return buttons & 0xFFFF
    if isinstance(buttons, str):
        buttons = buttons.replace("|", " ").replace("+", " ").replace(",", " ").split()
    mask = 0
    for b in buttons:
        name = str(b).strip().upper()
        if name == "X":
            name = "CROSS"
        elif name == "O":
            name = "CIRCLE"
        if name not in BUTTONS:
            raise ValueError(f"unknown button {b!r}; known: {', '.join(BUTTONS)}")
        mask |= 1 << BUTTONS.index(name)
    return mask


def button_names(mask: int) -> list[str]:
    return [n for i, n in enumerate(BUTTONS) if mask & (1 << i)]


class Game:
    """One debug channel to one game process (spawned by `spawn`, or an existing one by `attach`)."""

    LOG_LINES = 4000

    def __init__(self, sock: socket.socket, socket_path: str, proc: subprocess.Popen | None = None,
                 tmpdir: str | None = None, command: list[str] | None = None):
        self.sock = sock
        self.socket_path = socket_path
        self.proc = proc
        self.tmpdir = tmpdir
        self.command = command
        self._buf = b""
        self._next_id = 1
        self._lock = threading.Lock()
        self._log: collections.deque[str] = collections.deque(maxlen=self.LOG_LINES)
        self._log_thread = None
        if proc is not None and proc.stderr is not None:
            self._log_thread = threading.Thread(target=self._pump_stderr, daemon=True)
            self._log_thread.start()

    # ---- lifecycle ----

    @classmethod
    def spawn(cls, command: list[str], socket_path: str | None = None, timeout: float = 30.0,
              cwd: str | None = None, env: dict | None = None, hold: bool = False) -> "Game":
        """Start `command + ["--debug", SOCKET]` (and `--debug-hold` with `hold`: the game waits paused at its first
        vsync, so a run is reproducible from frame 1; resume()/step() go on) and connect. SOCKET defaults to a per-process temp dir (removed by
        stop()). Raises GameExited (with the stderr tail) if the process ends first, TimeoutError if the socket never
        appears."""
        tmpdir = None
        if socket_path is None:
            # sun_path holds 108 bytes with the NUL: under a long TMPDIR fall back to /tmp.
            tmpdir = tempfile.mkdtemp(prefix="dw2003-debug-")
            socket_path = os.path.join(tmpdir, "debug.sock")
            if len(socket_path.encode()) >= SUN_PATH_MAX:
                shutil.rmtree(tmpdir, ignore_errors=True)
                tmpdir = tempfile.mkdtemp(prefix="dw2003-debug-", dir="/tmp")
                socket_path = os.path.join(tmpdir, "debug.sock")
        if len(socket_path.encode()) >= SUN_PATH_MAX:
            raise ValueError(f"socket path too long for AF_UNIX ({len(socket_path.encode())} >= {SUN_PATH_MAX}): {socket_path}")
        argv = list(command) + ["--debug", socket_path] + (["--debug-hold"] if hold else [])
        proc = subprocess.Popen(argv, cwd=cwd, env=env, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                                stderr=subprocess.PIPE)
        game = cls(None, socket_path, proc=proc, tmpdir=tmpdir, command=argv)
        deadline = time.monotonic() + timeout
        while True:
            if proc.poll() is not None:
                time.sleep(0.05)  # let the stderr thread drain
                raise GameExited(game._exit_message("exited before the debug socket came up"))
            if os.path.exists(socket_path):
                try:
                    game.sock = cls._connect(socket_path)
                    break
                except (ConnectionRefusedError, FileNotFoundError):
                    pass
            if time.monotonic() > deadline:
                game.stop(timeout=1.0)
                raise TimeoutError(f"no debug socket at {socket_path} after {timeout:.0f} s\n" + game.log_text(30))
            time.sleep(0.05)
        return game

    @classmethod
    def attach(cls, socket_path: str) -> "Game":
        """Connect to a game the user started with `--debug SOCKET` (no process control: stop() only sends quit)."""
        return cls(cls._connect(socket_path), socket_path)

    @staticmethod
    def _connect(path: str) -> socket.socket:
        s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s.connect(path)
        s.settimeout(0.5)
        return s

    @property
    def alive(self) -> bool:
        if self.proc is not None:
            return self.proc.poll() is None
        return self.sock is not None

    @property
    def pid(self) -> int | None:
        return self.proc.pid if self.proc is not None else None

    def close(self):
        if self.sock is not None:
            try:
                self.sock.close()
            except OSError:
                pass
            self.sock = None

    def stop(self, timeout: float = 3.0) -> int | None:
        """quit, then wait up to `timeout` s; a spawned process that is still there gets killed. Returns the exit
        status (None when attached)."""
        if self.sock is not None:
            try:
                self.request("quit", timeout_s=2.0, status=0)
            except (GameError, GameExited, OSError, TimeoutError):
                pass
        self.close()
        status = None
        if self.proc is not None:
            try:
                status = self.proc.wait(timeout=timeout)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                status = self.proc.wait()
            if self._log_thread is not None:
                self._log_thread.join(timeout=1.0)
        if self.tmpdir is not None:
            shutil.rmtree(self.tmpdir, ignore_errors=True)
            self.tmpdir = None
        return status

    # ---- stderr ----

    def _pump_stderr(self):
        for raw in self.proc.stderr:
            self._log.append(raw.decode("utf-8", "replace").rstrip("\n"))

    def log(self, lines: int = 50) -> list[str]:
        """The last `lines` lines of the game's stderr (spawned games only)."""
        items = list(self._log)
        return items[-lines:] if lines > 0 else items

    def log_text(self, lines: int = 50) -> str:
        return "\n".join(self.log(lines))

    def _exit_message(self, what: str) -> str:
        code = self.proc.returncode if self.proc is not None else None
        tail = self.log_text(30)
        return f"game {what} (exit status {code})" + (f"; stderr tail:\n{tail}" if tail else "")

    # ---- the channel ----

    def _readline(self, timeout: float | None) -> bytes:
        deadline = None if timeout is None else time.monotonic() + timeout
        while True:
            nl = self._buf.find(b"\n")
            if nl >= 0:
                line, self._buf = self._buf[:nl], self._buf[nl + 1:]
                return line
            if self.proc is not None and self.proc.poll() is not None:
                time.sleep(0.05)
                self.close()
                raise GameExited(self._exit_message("exited while a request was pending"))
            if deadline is not None and time.monotonic() > deadline:
                raise TimeoutError(f"no response from the game after {timeout:.0f} s")
            try:
                chunk = self.sock.recv(65536)
            except socket.timeout:
                continue
            if not chunk:
                self.close()
                if self.proc is not None:
                    try:
                        self.proc.wait(timeout=1.0)
                    except subprocess.TimeoutExpired:
                        pass
                    raise GameExited(self._exit_message("closed the debug socket"))
                raise GameExited("the game closed the debug socket")
            self._buf += chunk

    def request(self, op: str, timeout_s: float | None = 10.0, **args) -> dict:
        """Send one request and return its result dict (without id/ok). `timeout_s=None` waits as long as it takes
        (deferred ops); the process is polled meanwhile. Raises GameError on `ok: false`."""
        if self.sock is None:
            raise GameExited("not connected")
        req = {"id": self._next_id, "op": op}
        self._next_id += 1
        req.update({k: v for k, v in args.items() if v is not None})
        line = json.dumps(req, separators=(",", ":")).encode() + b"\n"
        with self._lock:
            try:
                self.sock.sendall(line)
            except OSError as e:
                self.close()
                raise GameExited(f"send failed: {e}") from e
            while True:
                raw = self._readline(timeout_s)
                try:
                    resp = json.loads(raw)
                except ValueError as e:
                    raise GameError(f"unparsable response {raw[:200]!r}") from e
                if resp.get("id") == req["id"]:
                    break
                # A stale answer (a previous client's, or one we gave up on): skip it.
        if not resp.get("ok"):
            raise GameError(str(resp.get("error", "unknown error")))
        return {k: v for k, v in resp.items() if k not in ("id", "ok")}

    # ---- one method per op ----

    def status(self) -> dict:
        return self.request("status")

    def pause(self) -> int:
        return self.request("pause")["frame"]

    def resume(self) -> int:
        return self.request("resume")["frame"]

    def step(self, frames: int = 1, timeout: float | None = None) -> int:
        """Run `frames` vsyncs and pause (deferred)."""
        return self.request("step", timeout_s=timeout, frames=int(frames))["frame"]

    def peek(self, addr: int, length: int) -> bytes:
        return bytes.fromhex(self.request("peek", addr=int(addr), len=int(length))["data"])

    def poke(self, addr: int, data: bytes) -> int:
        return self.request("poke", addr=int(addr), data=bytes(data).hex())["len"]

    def peek_ps1(self, addr: int, length: int) -> bytes:
        return bytes.fromhex(self.request("peek_ps1", addr=int(addr), len=int(length))["data"])

    def poke_ps1(self, addr: int, data: bytes) -> int:
        return self.request("poke_ps1", addr=int(addr), data=bytes(data).hex())["len"]

    def pad(self, buttons, frames: int = 0, release: int = 0, sync: bool = False,
            timeout: float | None = None) -> int:
        """Own the pad and press `buttons` (a mask or names) for `frames` vsyncs (0 = until the next pad op), then
        `release` frames of nothing. With sync the call returns after frames + release vsyncs (deferred)."""
        return self.request("pad", timeout_s=timeout if sync else 10.0, buttons=button_mask(buttons),
                            frames=int(frames), release=int(release), sync=bool(sync))["frame"]

    def pad_free(self) -> dict:
        return self.request("pad_free")

    def wait(self, addr: int | None = None, ps1: int | None = None, ps1_stage: int | None = None,
             ps1_map: int | None = None, value: int | None = None, size: int = 4, signed: bool = False,
             timeout: int = 600, timeout_s: float | None = None) -> dict:
        """Run until a read equals `value` or `timeout` frames passed (deferred): {frame, hit}. Exactly one of
        addr (host), ps1 (PS1 address), ps1_stage, ps1_map; the stage/map forms take their value from that arg."""
        kinds = {"addr": addr, "ps1": ps1, "ps1_stage": ps1_stage, "ps1_map": ps1_map}
        given = [k for k, v in kinds.items() if v is not None]
        if len(given) != 1:
            raise ValueError("wait: exactly one of addr/ps1/ps1_stage/ps1_map")
        kind = given[0]
        args = {"timeout": int(timeout)}
        if kind in ("ps1_stage", "ps1_map"):
            # The spec says these "need only value"; the key carries the wanted value too, so either reading works.
            args[kind] = int(kinds[kind])
            args["value"] = int(kinds[kind])
        else:
            if value is None:
                raise ValueError("wait: value is required")
            args[kind] = int(kinds[kind])
            args.update(value=int(value), size=int(size), signed=bool(signed))
        return self.request("wait", timeout_s=timeout_s, **args)

    def screenshot(self, path: str) -> dict:
        """The game writes the current display as a binary PPM to `path`: {w, h, path}."""
        return self.request("screenshot", path=path)

    def hash(self) -> dict:
        return self.request("hash")

    def pace(self, fps: int) -> int:
        return self.request("pace", fps=int(fps))["pace"]

    def reset(self) -> int:
        return self.request("reset")["frame"]

    def quit(self, status: int = 0):
        try:
            return self.request("quit", timeout_s=2.0, status=int(status))
        finally:
            self.close()
