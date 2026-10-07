"""tools/mcp's self-test, offline: runs fake_game.py (the protocol double), exercises the Game client, then the
server's tool functions in-process (DW3_MCP_GAME points game_start at the fake). Exit 0 = passed.

    tools/venv/bin/python tools/mcp/selftest.py
"""

from __future__ import annotations

import os
import shlex
import sys
import tempfile
import zlib
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
FAKE = [sys.executable, str(HERE / "fake_game.py")]
os.environ["DW3_MCP_GAME"] = " ".join(shlex.quote(a) for a in FAKE)

import png  # noqa: E402
from game import Game, GameError, GameExited, button_mask  # noqa: E402
from symbols import Symbols  # noqa: E402

failures = 0


def check(cond, what):
    global failures
    print(("ok   " if cond else "FAIL ") + what)
    if not cond:
        failures += 1


def test_client():
    print("== Game client against fake_game.py")
    g = Game.spawn(FAKE)
    try:
        st = g.status()
        check(st["frame"] == 0 and st["pad_owner"] == "none", f"status {st}")
        check(g.step(5) == 5, "step 5")
        check(g.pause() == 5 and g.resume() == 5, "pause/resume")
        g.poke(0x10000010, b"\x01\x02\x03\x04")
        check(g.peek(0x10000010, 4) == b"\x01\x02\x03\x04", "poke/peek host")
        check(g.peek_ps1(0x80082CB0 + 0x10, 4) == b"\x01\x02\x03\x04", "peek_ps1 sees the same arena")
        g.poke_ps1(0x80048D34, (0x1234).to_bytes(2, "little"))
        check(g.peek_ps1(0x80048D34, 2) == b"\x34\x12", "poke_ps1/peek_ps1 state word")
        try:
            g.peek(0x1, 4)
            check(False, "peek unmapped raises")
        except GameError as e:
            check("unmapped" in str(e), f"peek unmapped -> GameError({e})")
        f = g.pad(["START"], frames=2, release=3, sync=True)
        check(f == 10, f"pad sync advanced to frame {f}")
        check(g.status()["pad_owner"] == "debug", "pad owner is debug")
        g.pad_free()
        check(g.status()["pad_owner"] == "none", "pad_free")
        r = g.wait(ps1_stage=3, timeout=100)
        check(r["hit"] == 1 and r["frame"] == 30, f"wait ps1_stage 3 -> {r}")
        r = g.wait(ps1_map=0xE02, timeout=100)
        check(r["hit"] == 1 and r["frame"] == 50, f"wait ps1_map 0xE02 -> {r}")
        r = g.wait(ps1=0x80048D34, value=0x1234, size=2, timeout=10)
        check(r["hit"] == 1 and r["frame"] == 50, f"wait ps1 value (already true) -> {r}")
        r = g.wait(addr=0x10000010, value=99, size=1, timeout=7)
        check(r["hit"] == 0 and r["frame"] == 57, f"wait host timeout -> {r}")
        h = g.hash()
        check(len(h["sha1"]) == 40 and h["frame"] == 57, "hash")
        check(g.pace(50) == 50 and g.status()["pace"] == 50, "pace")
        with tempfile.TemporaryDirectory() as d:
            p = os.path.join(d, "shot.ppm")
            r = g.screenshot(p)
            data = open(p, "rb").read()
            w, h_, pix = png.parse_ppm(data)
            check((w, h_) == (320, 240) and len(pix) == 320 * 240 * 3, "screenshot PPM")
            out = png.ppm_to_png(data)
            check(out.startswith(b"\x89PNG") and out.endswith(b"IEND\xae\x42\x60\x82"), "PNG framing")
            # decode back: IDAT is one chunk here
            idat = out[out.index(b"IDAT") + 4:]
            raw = zlib.decompress(idat[: len(idat) - 12])
            check(len(raw) == 240 * (1 + 320 * 3) and raw[1:4] == pix[:3], "PNG rows round-trip")
        check(g.reset() == 57 and g.status()["frame"] == 0, "reset")
        check(len(g.log()) > 0 and "fake_game" in g.log(1)[0], "stderr log captured")
        g2 = Game.attach(g.socket_path) if False else None  # the fake accepts one client at a time
    finally:
        status = g.stop()
    check(status == 0 and not g.alive, f"stop -> exit {status}")
    check(g.tmpdir is None, "temp dir removed")

    print("== failure paths")
    try:
        Game.spawn(FAKE + ["--exit-early"], timeout=5)
        check(False, "exit-early raises")
    except GameExited as e:
        check("exiting early" in str(e) and "status 7" in str(e), "exit-early -> GameExited with stderr")
    try:
        button_mask(["NOPE"])
        check(False, "bad button raises")
    except ValueError:
        check(True, "bad button -> ValueError")
    check(button_mask("start+cross") == (1 << 3) | (1 << 14), "button mask")


def test_symbols():
    print("== symbols")
    root = HERE.parents[1]
    s = Symbols(root)
    p = s.ps1("gamestate_data")
    check(p is not None and p.addr == 0x80048D34 and p.size == 0x275C, f"ps1 gamestate_data {p}")
    t = s.resolve("ps1:gamestate_data+0x10")
    check(t.ps1 and t.addr == 0x80048D44 and t.name == "gamestate_data", f"resolve ps1:name+off {t}")
    t = s.resolve("ps1:0x80082CB0")
    check(t.ps1 and t.addr == 0x80082CB0, "resolve ps1:hex")
    t = s.resolve("0x7f0000001000")
    check(not t.ps1 and t.addr == 0x7F0000001000, "resolve host hex")
    check(s.resolve("4096").addr == 4096, "resolve decimal")
    m = s.search("^gamestate_d", 5)
    check(any(e["name"] == "gamestate_data" for e in m), f"search {m[:2]}")
    check(s.ps1_overlay("fieldstg_stage") and s.ps1_overlay("fieldstg_stage")[0].source == "fieldstg",
          "overlay symbol table")
    elf = root / "build" / "port" / "dw2003"
    if elf.exists():
        h = s.host("gamestate_data")
        check(h is not None and h.size is not None and h.size >= 0x275C, f"host gamestate_data {h} (-m64 pads the PS1 0x275C)")
    else:
        try:
            s.resolve("gamestate_data")
            check(False, "host resolve without ELF raises")
        except ValueError as e:
            check("build" in str(e), f"no ELF -> {e}")
    try:
        s.resolve("12 monkeys")
        check(False, "bad target raises")
    except ValueError:
        check(True, "bad target -> ValueError")


def test_server():
    print("== server tool functions in-process (DW3_MCP_GAME = fake)")
    import server
    from mcp.server.mcpserver.exceptions import ToolError

    check(server.game_status() == {"running": False}, "status without a game")
    try:
        server.game_step()
        check(False, "step without a game is a tool error")
    except ToolError as e:
        check("game_start" in str(e), f"no game -> ToolError({e})")
    st = server.game_start()
    check(st["running"] and st["binary"] == "DW3_MCP_GAME" and "--fps" in st["command"], f"game_start {st['command']}")
    check(server.game_step(3)["frame"] == 3, "game_step")
    check(server.pad_press(["START"], 2, 2)["frame"] == 7, "pad_press")
    check(server.pad_hold(["cross", "UP"])["mask"] == (1 << 14) | (1 << 4), "pad_hold")
    check(server.pad_release()["frame"] == 7, "pad_release")
    check(server.pad_free()["pad_owner"] == "freed", "pad_free")
    r = server.mem_write("ps1:0x80082CC0", [1, -2], 4)
    check(r["written"] == 8, f"mem_write ints {r}")
    r = server.mem_read("ps1:0x80082CC0", 4, 2, signed=True)
    check(r["values"] == [1, -2], f"mem_read signed {r}")
    r = server.mem_read("0x10000010", 4, 2, fmt="hex")
    check(r["hex"] == "01000000feffffff", f"mem_read hex via host alias {r}")
    r = server.mem_write("0x10000020", "48690000")
    r = server.mem_read("0x10000020", 1, 4, fmt="str")
    check(r["str"] == "Hi", f"mem_read str {r}")
    r = server.mem_write("ps1:gamestate_data", [0x55], 1)
    r = server.mem_read("ps1:gamestate_data", 1)
    check(r["value"] == 0x55 and r["symbol"] == "gamestate_data", f"state-mapped PS1 symbol {r}")
    try:
        server.mem_read("ps1:0x80000000")
        check(False, "unmapped is a tool error")
    except ToolError as e:
        check("unmapped" in str(e), f"unmapped -> ToolError({e})")
    try:
        server.mem_read("no_such_symbol_xyz")
        check(False, "unknown symbol is a tool error")
    except ToolError as e:
        check(True, f"unknown symbol -> ToolError({e})")
    r = server.wait_stage(2, 100)
    check(r["hit"] and r["frame"] == 20, f"wait_stage {r}")
    r = server.wait_map(0xE01, 100)
    check(r["hit"] and r["frame"] == 25, f"wait_map {r}")
    r = server.wait_until("ps1:0x80082CC0", 1, 4, False, 5)
    check(r["hit"] and r["frame"] == 25, f"wait_until (already true) {r}")
    r = server.wait_until("0x10000010", 77, 1, False, 4)
    check(not r["hit"] and r["frame"] == 29, f"wait_until timeout {r}")
    check(server.game_pause()["frame"] == 29 and server.game_resume()["frame"] == 29, "pause/resume")
    check(server.game_pace(0)["pace"] == 0, "pace")
    h = server.state_hash()
    check(len(h["sha1"]) == 40, "state_hash")
    with tempfile.TemporaryDirectory() as d:
        out = os.path.join(d, "shot.png")
        res = server.screenshot(out)
        info, img = res[0], res[1]
        check(info["w"] == 320 and os.path.getsize(out) == info["png_bytes"], f"screenshot saved {info}")
        check(img.to_image_content().mime_type == "image/png", "screenshot image content")
    lk = server.symbol_lookup("gamestate_data")
    check(lk["ps1"]["addr"] == "0x80048d34", f"symbol_lookup {lk}")
    check(server.symbol_search("gamestate_data", 3)["count"] >= 1, "symbol_search")
    check(server.game_reset()["frame"] == 29 and server.game_status()["frame"] == 0, "game_reset")
    lg = server.game_log(5)["lines"]
    check(len(lg) == 5 and all("fake_game" in l for l in lg), f"game_log {lg[-1]}")
    r = server.game_stop()
    check(r["stopped"] and r["exit_status"] == 0, f"game_stop {r}")
    check(server.game_status() == {"running": False}, "stopped")
    os.environ["DW3_MCP_GAME"] = " ".join(shlex.quote(a) for a in FAKE + ["--exit-early"])
    try:
        server.game_start()
        check(False, "game_start failure is a tool error")
    except ToolError as e:
        check("status 7" in str(e), f"game dies -> ToolError({str(e).splitlines()[0]})")
    os.environ["DW3_MCP_GAME"] = " ".join(shlex.quote(a) for a in FAKE)


if __name__ == "__main__":
    test_client()
    test_symbols()
    test_server()
    print("PASSED" if failures == 0 else f"FAILED: {failures}")
    sys.exit(1 if failures else 0)
