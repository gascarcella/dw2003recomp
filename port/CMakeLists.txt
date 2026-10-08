# The PC port (docs/PORT.md; DECISIONS "PC port architecture"): the game's C units, its adapter (port/game/: the
# probes, the mods, the asm data stand-ins; game.json) and psxstack (the runtime, the Psy-Q shim, the build) linked
# into one host binary, build/port/dw2003.
#
#   cmake -S port -B build/port -G Ninja && cmake --build build/port && build/port/dw2003 --max-frames 60
#
# psxstack is the submodule at ../psxstack (PSXSTACK_DIR; scripts/worktree_init.sh checks it out at the pin, and
# offline links the sibling clone there instead); tools/port_inputs.py writes the
# build inputs psxstack_add_game() takes (build/port/gen/inputs/: the units, the overlays, the tag sites, the
# volatile ranges) from the tracked sources at configure time. The PS1 build (configure.py, build.ninja) is untouched.
cmake_minimum_required(VERSION 3.20)
project(dw2003port C)

get_filename_component(DW3_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(PSXSTACK_DIR "${DW3_ROOT}/psxstack" CACHE PATH "The psxstack checkout (the submodule; or another checkout of the stack)")
if(NOT EXISTS "${PSXSTACK_DIR}/cmake/psxstack.cmake")
    message(FATAL_ERROR "psxstack not found at ${PSXSTACK_DIR}: the submodule is not checked out (git submodule update "
                        "--init psxstack, or scripts/worktree_init.sh; or -DPSXSTACK_DIR=<path>)")
endif()
if(EXISTS "${DW3_ROOT}/tools/venv/bin/python" AND NOT PSXSTACK_PYTHON)
    set(PSXSTACK_PYTHON "${DW3_ROOT}/tools/venv/bin/python" CACHE FILEPATH "" FORCE)
endif()
# The built tools (SDL3, DXC) are this checkout's tools/ (the main checkout's in a worktree, linked there).
if(NOT PSXSTACK_TOOLS_DIR)
    set(PSXSTACK_TOOLS_DIR "${DW3_ROOT}/tools" CACHE PATH "" FORCE)
endif()
set(PSXSTACK_VERSION_ROOT "${DW3_ROOT}" CACHE PATH "" FORCE)   # the game's tags name the build
include("${PSXSTACK_DIR}/cmake/psxstack.cmake")

# ---- The build inputs (tools/port_inputs.py), regenerated when their sources change.
set(DW3_INPUTS "${CMAKE_BINARY_DIR}/gen/inputs")
execute_process(COMMAND "${PSXSTACK_PYTHON}" "${DW3_ROOT}/tools/port_inputs.py" --out "${DW3_INPUTS}"
                RESULT_VARIABLE r OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT r EQUAL 0)
    message(FATAL_ERROR "tools/port_inputs.py failed:\n${out}${err}")
endif()
string(STRIP "${out}" out)
message(STATUS "${out}")
file(GLOB DW3_CONFIG_DEPS "${DW3_ROOT}/config/*.symbols.txt" "${DW3_ROOT}/config/wstag/*.symbols.txt")
file(GLOB DW3_ADAPTER_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/game/*.c")

psxstack_add_game(dw2003
    GAME_JSON "${CMAKE_CURRENT_SOURCE_DIR}/game/game.json"
    UNITS "${DW3_INPUTS}/units.txt"
    MAIN_UNIT "${DW3_ROOT}/src/main/main.c"
    OVERLAYS "${DW3_INPUTS}/overlays.txt"
    TAG_SITES "${DW3_INPUTS}/tag_sites.txt"
    EXE_SYMBOLS "${DW3_ROOT}/config/symbol_addrs.txt"
    VOLATILE "${DW3_INPUTS}/volatile.txt"
    GTEMAC "${DW3_ROOT}/include/psyq/gtemac.h"
    INCLUDE_DIRS "${DW3_ROOT}/include" "${DW3_ROOT}"
    ADAPTER ${DW3_ADAPTER_SRCS}
    MODS_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/mods"
    DEFINES NON_MATCHING
    CONFIGURE_DEPENDS "${DW3_ROOT}/tools/port_inputs.py" "${DW3_ROOT}/config/wstag.txt" "${DW3_ROOT}/config/wstag_c.txt"
                      "${DW3_ROOT}/src/main/overlay.c" "${DW3_ROOT}/src/fieldstg/fieldstg_80087DB0.c"
                      "${DW3_ROOT}/tests/replay/replay.py" ${DW3_CONFIG_DEPS})
