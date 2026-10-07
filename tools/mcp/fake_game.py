"""An offline double of `dw2003 --debug SOCKET` for tools/mcp's self-test: speaks the debug protocol over a Unix
socket with a tiny model. frame advances on step/wait/sync pad; stage = frame // 10, map = 0xE00 + frame // 25;
a 64 KB "arena" at PS1 0x80082CB0 and at host 0x10000000 (peek/poke/peek_ps1/poke_ps1), a 4-byte "gamestate_data"
word at PS1 0x80048D34 (state-mapped: len 1/2/4), a synthetic 320x240 PPM for screenshot.

    python tools/mcp/fake_game.py --debug /tmp/x.sock [--exit-early] [any other args, ignored]
"""

from __future__ import annotations

import hashlib
import json
import os
import socket
import struct
import sys

ARENA_PS1 = 0x80082CB0
ARENA_HOST = 0x10000000
ARENA_SIZE = 0x10000
STATE_PS1 = 0x80048D34  # gamestate_data: not in the arena, mapped 1/2/4 only


class Model:
    def __init__(self):
        self.frame = 0
        self.paused = False
        self.pace = 0
        self.arena = bytearray(ARENA_SIZE)
        self.state_word = bytearray(4)
        self.pad_owner = "none"
        self.pad = 0
        self.pad_frames = 0
        self.pad_release = 0
        self.want_reset = False
        self.want_quit = None

    @property
    def stage(self):
        return self.frame // 10

    @property
    def map(self):
        return 0xE00 + self.frame // 25

    def tick(self):
        self.frame += 1
        if self.pad_owner == "debug":
            if self.pad_frames > 0:
                self.pad_frames -= 1
                if self.pad_frames == 0:
                    self.pad = 0
            elif self.pad_release > 0:
                self.pad_release -= 1

    def read(self, ps1, addr, n):
        if ps1:
            if ARENA_PS1 <= addr and addr + n <= ARENA_PS1 + ARENA_SIZE:
                off = addr - ARENA_PS1
                return bytes(self.arena[off:off + n])
            if STATE_PS1 <= addr and addr + n <= STATE_PS1 + 4 and n in (1, 2, 4):
                off = addr - STATE_PS1
                return bytes(self.state_word[off:off + n])
            raise ValueError("unmapped")
        if ARENA_HOST <= addr and addr + n <= ARENA_HOST + ARENA_SIZE:
            off = addr - ARENA_HOST
            return bytes(self.arena[off:off + n])
        raise ValueError("unmapped")

    def write(self, ps1, addr, data):
        n = len(data)
        self.read(ps1, addr, n)  # the range check
        if ps1 and STATE_PS1 <= addr < STATE_PS1 + 4:
            off = addr - STATE_PS1
            self.state_word[off:off + n] = data
        else:
            base = ARENA_PS1 if ps1 else ARENA_HOST
            off = addr - base
            self.arena[off:off + n] = data

    def wait_read(self, args):
        size = int(args.get("size", 4))
        if "ps1_stage" in args:
            return self.stage
        if "ps1_map" in args:
            return self.map
        if "addr" in args:
            raw = self.read(False, num(args["addr"]), size)
        else:
            raw = self.read(True, num(args["ps1"]), size)
        return int.from_bytes(raw, "little", signed=bool(args.get("signed", False)))


def num(v):
    return int(v, 16) if isinstance(v, str) else int(v)


def handle(m: Model, req: dict) -> dict:
    op = req.get("op")
    if op == "status":
        return {"frame": m.frame, "stage": m.stage, "file": 0, "map": m.map, "slot1_word0": 0,
                "paused": int(m.paused), "pace": m.pace, "rate": 50, "window": 0, "script": 0,
                "pad_owner": m.pad_owner, "pad": m.pad}
    if op == "pause":
        m.paused = True
        return {"frame": m.frame}
    if op == "resume":
        m.paused = False
        return {"frame": m.frame}
    if op == "step":
        n = num(req.get("frames", 1))
        if n < 1:
            raise ValueError("frames must be >= 1")
        for _ in range(n):
            m.tick()
        m.paused = True
        return {"frame": m.frame}
    if op in ("peek", "peek_ps1"):
        n = num(req["len"])
        if not 1 <= n <= 65536:
            raise ValueError("len out of range")
        return {"data": m.read(op == "peek_ps1", num(req["addr"]), n).hex()}
    if op in ("poke", "poke_ps1"):
        data = bytes.fromhex(req["data"])
        m.write(op == "poke_ps1", num(req["addr"]), data)
        return {"len": len(data)}
    if op == "pad":
        m.pad_owner = "debug"
        m.pad = num(req["buttons"]) & 0xFFFF
        m.pad_frames = num(req.get("frames", 0))
        m.pad_release = num(req.get("release", 0))
        if req.get("sync"):
            for _ in range(m.pad_frames + m.pad_release):
                m.tick()
        return {"frame": m.frame}
    if op == "pad_free":
        m.pad_owner, m.pad = "none", 0
        return {}
    if op == "wait":
        value = num(req["value"])
        timeout = num(req.get("timeout", 600))
        hit = 0
        if m.wait_read(req) == value:
            hit = 1
        else:
            for _ in range(timeout):
                m.tick()
                if m.wait_read(req) == value:
                    hit = 1
                    break
        m.paused = True
        return {"frame": m.frame, "hit": hit}
    if op == "screenshot":
        w, h = 320, 240
        rows = bytearray()
        for y in range(h):
            for x in range(w):
                rows += bytes((x * 255 // (w - 1), y * 255 // (h - 1), (m.frame * 7) & 0xFF))
        with open(req["path"], "wb") as f:
            f.write(b"P6\n%d %d\n255\n" % (w, h) + rows)
        return {"w": w, "h": h, "path": req["path"]}
    if op == "hash":
        sha = hashlib.sha1(bytes(m.state_word) + bytes(m.arena)).hexdigest()
        return {"frame": m.frame, "sha1": sha, "stable_sha1": sha}
    if op == "pace":
        m.pace = num(req.get("fps", 0))
        return {"pace": m.pace}
    if op == "reset":
        m.want_reset = True
        return {"frame": m.frame}
    if op == "quit":
        m.want_quit = num(req.get("status", 0))
        return {}
    raise ValueError(f"unknown op {op!r}")


def serve(path: str):
    try:
        os.unlink(path)
    except FileNotFoundError:
        pass
    srv = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    srv.bind(path)
    srv.listen(1)
    m = Model()
    print("fake_game: listening on " + path, file=sys.stderr, flush=True)
    while m.want_quit is None:
        conn, _ = srv.accept()
        buf = b""
        with conn:
            while m.want_quit is None:
                chunk = conn.recv(65536)
                if not chunk:
                    break
                buf += chunk
                while b"\n" in buf:
                    line, buf = buf.split(b"\n", 1)
                    if not line.strip():
                        continue
                    try:
                        req = json.loads(line)
                        rid = req.get("id")
                    except ValueError:
                        req, rid = None, None
                    try:
                        if req is None:
                            raise ValueError("malformed request")
                        result = handle(m, req)
                        resp = {"id": rid, "ok": True, **result}
                    except (ValueError, KeyError, TypeError) as e:
                        resp = {"id": rid, "ok": False, "error": str(e)}
                    conn.sendall(json.dumps(resp).encode() + b"\n")
                    print(f"fake_game: {req and req.get('op')} -> frame {m.frame}", file=sys.stderr, flush=True)
                    if m.want_reset:
                        m.want_reset = False
                        m.frame = 0
                        m.state_word[:] = b"\0\0\0\0"
                        m.arena[:] = bytes(ARENA_SIZE)
    try:
        os.unlink(path)
    except FileNotFoundError:
        pass
    print("fake_game: quit", file=sys.stderr, flush=True)
    sys.exit(m.want_quit)


def main(argv):
    path = None
    for i, a in enumerate(argv):
        if a == "--debug" and i + 1 < len(argv):
            path = argv[i + 1]
        elif a == "--exit-early":
            print("fake_game: exiting early as asked", file=sys.stderr, flush=True)
            sys.exit(7)
    if path is None:
        print("usage: fake_game.py --debug SOCKET", file=sys.stderr)
        sys.exit(2)
    serve(path)


if __name__ == "__main__":
    main(sys.argv[1:])
