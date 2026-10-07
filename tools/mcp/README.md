# tools/mcp: the MCP server for the PC port

A [Model Context Protocol](https://modelcontextprotocol.io) server that drives `build/port/dw2003` through its
debug channel (`--debug SOCKET`: newline-delimited JSON over a Unix socket, polled by the game once per vsync, so
every read, write and pad press lands between two frames and runs stay deterministic). An agent uses it to start
the game, run frames, press buttons, read and write memory by symbol name and take screenshots, headless or in a
window.

## Running it

`.mcp.json` at the repository root registers it for Claude Code (`tools/venv/bin/python tools/mcp/server.py`,
cwd the repository root; `scripts/setup.sh venv` installs `mcp` from `tools/requirements.txt`). By hand:
`tools/venv/bin/python tools/mcp/server.py` speaks MCP on stdio. `tools/venv/bin/python tools/mcp/selftest.py`
tests everything offline against `fake_game.py` (the protocol double; `DW3_MCP_GAME` in the environment points
`game_start` at any command instead of the built binary).

`game.py` is a plain client with no MCP dependency, for scripts: `Game.spawn([binary, ...args])` /
`Game.attach(socket)`, one method per protocol op (`status`, `step`, `peek`, `poke_ps1`, `pad`, `wait`,
`screenshot`, `hash`, ...), `log()` (the game's stderr), `stop()`.

## The tools

| Tool | What |
|---|---|
| `game_start(window, disc, cd_speed, pace, memcard1, memcard2, config, mods, extra_args, build, hold)` | Start the port with `--debug` and connect (stops a running one first). Headless and unthrottled by default; `hold=True` starts it paused at frame 1 (`--debug-hold`: reproducible frames and hashes); `window=True` uses `build/port-sdl/dw2003` when present and paces at 50; `build=True` runs CMake first. |
| `game_attach(socket_path)` | A game the user started with `--debug SOCKET`. |
| `game_stop()`, `game_status()`, `game_log(lines)` | Quit (kill after 3 s); frame/stage/map/paused/pad owner; the game's stderr tail. |
| `game_pause()`, `game_resume()`, `game_step(frames)`, `game_reset()`, `game_pace(fps)` | Execution control; `step` runs exactly N vsyncs then pauses. |
| `pad_press(buttons, frames, release)`, `pad_hold(buttons)`, `pad_release()`, `pad_free()` | The pad (names: SELECT L3 R3 START UP RIGHT DOWN LEFT L2 R2 L1 R1 TRIANGLE CIRCLE CROSS SQUARE). The channel owns the pad from the first call until `pad_free`. `pad_press` returns after the frames ran. |
| `mem_read(target, size, count, signed, fmt)`, `mem_write(target, values, size)` | Memory by target (below); `fmt` int/hex/str; `values` a list of ints or a hex string. |
| `symbol_lookup(name)`, `symbol_search(pattern, limit)` | Host address and size (`nm` on the ELF, cached by mtime), PS1 address/size/type (`config/symbol_addrs.txt`, the overlays' `config/*.symbols.txt`). |
| `wait_until(target, value, size, signed, timeout_frames)`, `wait_stage(stage, timeout_frames)`, `wait_map(map, timeout_frames)` | Run until a read, the stage (overlay slot, as the pad scripts' `wait_stage`) or the map matches, or the timeout; leaves the game paused; returns `{frame, hit}`. |
| `screenshot(path)` | The current display as a PNG image (and saved to `path` if given). |
| `state_hash()` | The SHA-1 of `gamestate_data`'s PS1 image, as a replay checkpoint. |

Errors from the game (an unmapped address, "script owns the pad", ...) come back as tool errors with the game's
message; a game that dies comes back with its stderr tail.

## Targets

`name` or `name+0x10`: a global of the port's ELF (host memory, `nm`). `0x7f...` or a decimal number: a host
address. `ps1:0x80048D34`, `ps1:name`, `ps1:name+8`: a PS1 address (the arena from `0x80082CB0` is read directly,
any length; other globals go through the port's state map, 1, 2 or 4 bytes at a time; `ps1:name` uses
`config/symbol_addrs.txt`, or an overlay's table when only one overlay defines the name).

## Rules

One game at a time: the server holds a single session, `game_start` replaces it. The game's socket lives in a
per-process temp dir that `game_stop` removes. Relative paths (`disc`, memory cards, `config`, `screenshot`'s
`path`) resolve against the repository root. Nothing here touches `src/`, `include/` or `config/`.
