# port/: the PC port

The game's C (`src/`, 388 units) linked with psxstack's runtime and Psy-Q shim and this game's adapter (`port/game/`)
into one 64-bit host binary, `build/port/dw2003` (`docs/PORT.md`: what is this game's; psxstack's `docs/PORT.md` and
`docs/RUNTIME.md`: the runtime, its options and formats). The PS1 build (`configure.py`, `build.ninja`) is untouched.

## Build and run
```sh
scripts/worktree_init.sh                  # links psxstack/ to the sibling clone (../psxstack) when there is no submodule
cmake -S port -B build/port -G Ninja      # tools/port_inputs.py, then psxstack_add_game(dw2003 ...)
cmake --build build/port                  # ~30 s from scratch with -j6
build/port/dw2003 --disc iso/dw2003.cue --max-frames 60
build/port/dw2003 --help                  # every option
scripts/setup.sh sdl3 dxc && cmake -S port -B build/port-sdl -G Ninja -DPSXSTACK_SDL=ON && cmake --build build/port-sdl
build/port-sdl/dw2003 --disc iso/dw2003.cue --window
```
`-DPSXSTACK_DIR=<path>` names another psxstack checkout; the options (`PSXSTACK_SANITIZE`, `PSXSTACK_M32`,
`PSXSTACK_SDL`, ...) are psxstack's (`docs/RUNTIME.md` "Build and run"). The built tools (SDL3, DXC) are this
checkout's `tools/` (`PSXSTACK_TOOLS_DIR`, set by `port/CMakeLists.txt`).

## Layout
| Path | Contents |
|---|---|
| `CMakeLists.txt` | Finds psxstack, runs `tools/port_inputs.py`, calls `psxstack_add_game()` with this game's inputs |
| `game/game.json` | The game's description (psxstack's `schema/game.schema.json`) |
| `game/state.c`, `game/game.c` | The adapter: the probes, the checkpoint image, the state map, `game_apply_rate` |
| `game/game_mods.c`, `game/battle_scan.c` | The seven mods that change the game; the battle scripts' scanner |
| `game/asmdata.c` | Weak stand-ins for the data the matching build keeps in asm |
| `mods/<id>/mod.json` | The game's mods' manifests (copied beside the binary with psxstack's `fast_forward`) |

The mods' behaviour: `docs/LAUNCHER.md` "Built-in mods". The launcher is psxstack's, built for this game with
`cmake -S psxstack/launcher -B build/launcher -G Ninja -DPSXSTACK_GAME_JSON=$PWD/port/game/game.json
-DPSXSTACK_VERSION_ROOT=$PWD -DPSXSTACK_TOOLS_DIR=$PWD/tools`.
