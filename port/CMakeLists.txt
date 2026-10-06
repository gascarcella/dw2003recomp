# The PC port (docs/PC_PORT_PLAN.md; DECISIONS "PC port decisions (session 15)"): the game's 388 C units, the port
# runtime (port/src/) and the Psy-Q shim (port/psyq/) linked into one host binary, build/port/dw2003.
#
#   cmake -S port -B build/port -G Ninja && cmake --build build/port && build/port/dw2003 --max-frames 60
#
# tools/port_gen.py generates, under build/port/gen/: the unit list (configure time), the override headers and the
# ld script (configure time), and the overlay address tables (build time, from nm of the compiled units). The PS1
# build (configure.py, build.ninja) is untouched. port/README.md has the details.
cmake_minimum_required(VERSION 3.20)
project(dw2003port C)

option(DW3_PORT_ALLOW_UNRESOLVED "Link with --unresolved-symbols=ignore-all (a private build without port/psyq/)" OFF)
option(DW3_PORT_SANITIZE "Build with -fsanitize=address,undefined" OFF)
option(DW3_PORT_PSYQ_WERROR "-Werror for port/psyq/ too (port/src/ always has it; port/psyq/check.sh uses it)" ON)
set(DW3_PORT_PSYQ_DIR "${CMAKE_CURRENT_SOURCE_DIR}/psyq" CACHE PATH "The Psy-Q shim's sources (psyq.h and *.c)")
set(DW3_PORT_UNIT_OVERRIDES "" CACHE STRING
    "Experiments: units to build from another file, as src/<t>/<unit>.c=<path> entries (the game's tree is untouched)")

get_filename_component(DW3_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(DW3_GEN "${CMAKE_BINARY_DIR}/gen")
set(DW3_PORT_GEN "${DW3_ROOT}/tools/port_gen.py")

if(EXISTS "${DW3_ROOT}/tools/venv/bin/python")
    set(DW3_PYTHON "${DW3_ROOT}/tools/venv/bin/python")
else()
    find_program(DW3_PYTHON NAMES python3 python REQUIRED)
endif()

# ---- Configure-time generation: the unit list, the override headers, the ld script, the arena's layout.
function(dw3_generate)
    execute_process(COMMAND "${DW3_PYTHON}" "${DW3_PORT_GEN}" ${ARGN} RESULT_VARIABLE r OUTPUT_VARIABLE out
                    ERROR_VARIABLE err)
    if(NOT r EQUAL 0)
        message(FATAL_ERROR "port_gen.py ${ARGN} failed:\n${out}${err}")
    endif()
    string(STRIP "${out}" out)
    message(STATUS "${out}")
endfunction()
dw3_generate(units --out "${DW3_GEN}/units.cmake")
dw3_generate(overrides --out "${DW3_GEN}/include")
dw3_generate(ldscript --out "${DW3_GEN}/overlays.ld")
dw3_generate(arena-header --out "${DW3_GEN}/include/port_arena_gen.h")
include("${DW3_GEN}/units.cmake")
foreach(override IN LISTS DW3_PORT_UNIT_OVERRIDES)
    string(REPLACE "=" ";" parts "${override}")
    list(GET parts 0 unit)
    list(GET parts 1 path)
    list(FIND DW3_GAME_UNITS "${DW3_ROOT}/${unit}" i)
    if(i LESS 0)
        message(FATAL_ERROR "DW3_PORT_UNIT_OVERRIDES: ${unit} is not a game unit")
    endif()
    list(REMOVE_AT DW3_GAME_UNITS ${i})
    list(INSERT DW3_GAME_UNITS ${i} "${path}")
    message(STATUS "unit override: ${unit} <- ${path}")
endforeach()
file(GLOB DW3_CONFIG_DEPS "${DW3_ROOT}/config/*.symbols.txt" "${DW3_ROOT}/config/wstag/*.symbols.txt")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             "${DW3_PORT_GEN}" "${DW3_ROOT}/config/wstag.txt" "${DW3_ROOT}/config/wstag_c.txt"
             "${DW3_ROOT}/include/psyq/gtemac.h" "${DW3_ROOT}/include/asm_generated/include_asm.h"
             "${DW3_ROOT}/src/main/overlay.c" "${DW3_ROOT}/src/fieldstg/fieldstg_80087DB0.c")

# ---- Flags shared by everything that includes the game's headers (the units, the runtime, the shim).
set(DW3_GAME_DEFINES PC_PORT NON_MATCHING)
set(DW3_GAME_INCLUDES "${DW3_GEN}/include" "${DW3_ROOT}/include" "${DW3_ROOT}")
set(DW3_COMMON_FLAGS -fsigned-char -fwrapv -fno-strict-aliasing -fno-pie)
if(DW3_PORT_SANITIZE)
    list(APPEND DW3_COMMON_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()
add_link_options(-no-pie)

# ---- The game's units: one object library, each overlay's data in its own sections (-fdata-sections) for the ld
# script to collect. The game's own main() becomes game_main (port/src/main.c has the host's).
add_library(dw3_game OBJECT ${DW3_GAME_UNITS})
set_target_properties(dw3_game PROPERTIES C_STANDARD 99 C_EXTENSIONS ON C_STANDARD_REQUIRED ON)
target_compile_definitions(dw3_game PRIVATE ${DW3_GAME_DEFINES})
target_include_directories(dw3_game BEFORE PRIVATE ${DW3_GAME_INCLUDES})
target_compile_options(dw3_game PRIVATE ${DW3_COMMON_FLAGS} -fno-builtin -fno-common -fdata-sections
                       -ffunction-sections -Wall -Wno-pointer-sign -Wno-unused-but-set-variable -Wno-unused-variable)
# GCC 14+ makes some old-C constructs errors; the probe (tools/port_inventory.py) keeps them warnings the same way.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 14)
    target_compile_options(dw3_game PRIVATE -fpermissive)
endif()
set_source_files_properties("${DW3_ROOT}/src/main/main.c" PROPERTIES COMPILE_DEFINITIONS "main=game_main")

# ---- The overlay address tables: from nm of the compiled units (a static function has no global symbol).
file(GENERATE OUTPUT "${DW3_GEN}/game_objects.rsp" CONTENT "$<TARGET_OBJECTS:dw3_game>")
add_custom_command(OUTPUT "${DW3_GEN}/overlay_tables.c"
                   COMMAND "${DW3_PYTHON}" "${DW3_PORT_GEN}" tables --nm "${CMAKE_NM}"
                           --objects "${DW3_GEN}/game_objects.rsp" --out "${DW3_GEN}/overlay_tables.c"
                   DEPENDS $<TARGET_OBJECTS:dw3_game> "${DW3_PORT_GEN}" ${DW3_CONFIG_DEPS}
                           "${DW3_ROOT}/config/wstag.txt" "${DW3_ROOT}/src/main/overlay.c"
                           "${DW3_ROOT}/src/fieldstg/fieldstg_80087DB0.c"
                   COMMENT "port_gen.py tables (the overlay address tables)" VERBATIM)

# ---- The Psy-Q shim (port/psyq/, T9's): built when the directory has sources; without it the link needs
# DW3_PORT_ALLOW_UNRESOLVED (a stand-in directory can be given as DW3_PORT_PSYQ_DIR).
file(GLOB DW3_PSYQ_SRCS CONFIGURE_DEPENDS "${DW3_PORT_PSYQ_DIR}/*.c")
if(NOT DW3_PSYQ_SRCS AND NOT DW3_PORT_ALLOW_UNRESOLVED)
    message(FATAL_ERROR "no Psy-Q shim sources in ${DW3_PORT_PSYQ_DIR}: set DW3_PORT_PSYQ_DIR, or "
                        "-DDW3_PORT_ALLOW_UNRESOLVED=ON for a link that ignores undefined symbols")
endif()

# ---- The binary: the runtime, the tables, the shim, the units.
file(GLOB DW3_RUNTIME_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_executable(dw2003 ${DW3_RUNTIME_SRCS} "${DW3_GEN}/overlay_tables.c" ${DW3_PSYQ_SRCS} $<TARGET_OBJECTS:dw3_game>)
set_target_properties(dw2003 PROPERTIES C_STANDARD 99 C_EXTENSIONS ON C_STANDARD_REQUIRED ON
                      LINK_DEPENDS "${DW3_GEN}/overlays.ld")
target_compile_definitions(dw2003 PRIVATE ${DW3_GAME_DEFINES})
target_include_directories(dw2003 BEFORE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" "${DW3_PORT_PSYQ_DIR}"
                           ${DW3_GAME_INCLUDES})
target_compile_options(dw2003 PRIVATE ${DW3_COMMON_FLAGS} -Wall -Wextra)
set_source_files_properties(${DW3_RUNTIME_SRCS} "${DW3_GEN}/overlay_tables.c" PROPERTIES COMPILE_OPTIONS "-Werror")
if(DW3_PORT_PSYQ_WERROR AND DW3_PSYQ_SRCS)
    set_source_files_properties(${DW3_PSYQ_SRCS} PROPERTIES COMPILE_OPTIONS "-Werror")
endif()
# The ld script: per-overlay .data/.bss sections and the arena's symbols.
target_link_options(dw2003 PRIVATE "-Wl,-T,${DW3_GEN}/overlays.ld")
if(DW3_PORT_ALLOW_UNRESOLVED)
    target_link_options(dw2003 PRIVATE "-Wl,--unresolved-symbols=ignore-all")
endif()
target_link_libraries(dw2003 PRIVATE m)
