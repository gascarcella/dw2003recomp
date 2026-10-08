"""This game's configuration of psxstack's debug tools (psxstack/tools/mcp: the MCP server, the Game client, the
symbols): the same values .mcp.json passes the server, for the tests that drive the port themselves
(tests/port/debug.py, tests/port/mods.py).

    import mcp_game                      # after sys.path.insert(0, str(ROOT / "tools"))
    g = mcp_game.Game.spawn([str(mcp_game.BINARY), "--disc", str(mcp_game.DISC)])
    syms = mcp_game.symbols()            # host symbols from build/port/dw2003, PS1 symbols from config/
"""
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PSXSTACK = ROOT / "psxstack"
sys.path.insert(0, str(PSXSTACK / "tools/mcp"))
from game import Game, GameError, GameExited, button_mask, button_names  # noqa: E402,F401
from symbols import Symbols, Target  # noqa: E402,F401

GAME_JSON = ROOT / "port/game/game.json"
BINARY = ROOT / "build/port/dw2003"
BINARY_SDL = ROOT / "build/port-sdl/dw2003"
DISC = ROOT / "iso/dw2003.cue"
EXE_SYMBOLS = ROOT / "config/symbol_addrs.txt"
SELFTEST = PSXSTACK / "tools/mcp/selftest.py"
_base = json.loads(GAME_JSON.read_text())["memory"]["slots"][0]["base"]
ARENA_START = int(_base, 0) if isinstance(_base, str) else int(_base)   # peek_ps1 reads the arena from here


def overlay_symbols():
    return sorted(ROOT.glob("config/*.symbols.txt"))


def symbols(binary=BINARY):
    return Symbols(binary, EXE_SYMBOLS, overlay_symbols())
