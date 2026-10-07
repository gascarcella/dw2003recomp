# The PC port (docs/PORT.md; DECISIONS "PC port architecture"): the game's 388 C units, the port
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
option(DW3_PORT_M32 "Build a 32-bit binary (-m32, gcc-multilib): its --log must equal the 64-bit build's" OFF)
option(DW3_PORT_SDL "The window (port/src/video.c, input.c) over SDL3 (tools/sdl3: scripts/setup.sh sdl3)" OFF)
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
set(DW3_COMMON_FLAGS -fsigned-char -fwrapv -fno-strict-aliasing)
if(DW3_PORT_SANITIZE)
    list(APPEND DW3_COMMON_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()
# No -no-pie: the arena needs no fixed load address (port/src/arena.c, DECISIONS "The arena assumes no alignment and no
# link address"). The Windows cross build (cmake/windows-x86_64.cmake) still links with the ld script and checks the
# map below, which lld for PE cannot do: "Windows 2" replaces them. Until then that build compiles every object and
# fails at the link.
# -m32: everything compiled and linked 32-bit (nm reads both).
if(DW3_PORT_M32)
    if(WIN32)
        message(FATAL_ERROR "DW3_PORT_M32: the Windows build is x86_64 only")
    endif()
    add_compile_options(-m32)
    add_link_options(-m32)
endif()

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
# Clang (the Windows cross build) makes the same constructs errors, each under its own flag: back to warnings. The
# probe's --target windows (tools/port_inventory.py CLANG_GAME_FLAGS) uses the same list.
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(dw3_game PRIVATE -Wno-error=implicit-function-declaration -Wno-error=implicit-int
                           -Wno-error=int-conversion -Wno-error=incompatible-pointer-types
                           -Wno-error=incompatible-function-pointer-types -Wno-error=return-type)
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

# ---- The game-state tables (port/src/state.c): the EXE's functions and sized data symbols, from nm of the EXE's
# objects and config/symbol_addrs.txt; the pointer test compiles the defining units at -m64 (port_gen.py state).
add_custom_command(OUTPUT "${DW3_GEN}/port_state_tables.c"
                   COMMAND "${DW3_PYTHON}" "${DW3_PORT_GEN}" state --nm "${CMAKE_NM}" --cc "${CMAKE_C_COMPILER}"
                           "--cflags=$<$<BOOL:${DW3_PORT_M32}>:-m32>"
                           --gen-include "${DW3_GEN}/include" --objects "${DW3_GEN}/game_objects.rsp"
                           --out "${DW3_GEN}/port_state_tables.c"
                   DEPENDS $<TARGET_OBJECTS:dw3_game> "${DW3_PORT_GEN}" "${DW3_ROOT}/config/symbol_addrs.txt"
                           "${DW3_ROOT}/tests/replay/replay.py"
                   COMMENT "port_gen.py state (the EXE's function and data tables)" VERBATIM)

# ---- The Psy-Q shim (port/psyq/, T9's): built when the directory has sources; without it the link needs
# DW3_PORT_ALLOW_UNRESOLVED (a stand-in directory can be given as DW3_PORT_PSYQ_DIR).
file(GLOB DW3_PSYQ_SRCS CONFIGURE_DEPENDS "${DW3_PORT_PSYQ_DIR}/*.c")
if(NOT DW3_PSYQ_SRCS AND NOT DW3_PORT_ALLOW_UNRESOLVED)
    message(FATAL_ERROR "no Psy-Q shim sources in ${DW3_PORT_PSYQ_DIR}: set DW3_PORT_PSYQ_DIR, or "
                        "-DDW3_PORT_ALLOW_UNRESOLVED=ON for a link that ignores undefined symbols")
endif()

# ---- The build's version stamp (port_version, port_commit: `--version`, the start line, the crash report), from git
# describe at every build (port/cmake/version.cmake rewrites the file only when it changed).
add_custom_target(dw3_version COMMAND "${CMAKE_COMMAND}" -DROOT=${DW3_ROOT} -DOUT=${DW3_GEN}/port_version.c
                  -DPREFIX=port -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.cmake" BYPRODUCTS "${DW3_GEN}/port_version.c"
                  COMMENT "port/cmake/version.cmake (the build's version stamp)" VERBATIM)
# A release build keeps the debug info in the build tree: scripts/package_appimage.sh strips the shipped binary and
# saves it as the release's .debug file, which symbolizes crash reports (docs/PORT.md "Crash report"). -g changes no code.
if(CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT CMAKE_C_FLAGS_RELEASE MATCHES "(^| )-g( |$)")
    string(APPEND CMAKE_C_FLAGS_RELEASE " -g")
endif()

# ---- The binary: the runtime, the tables, the shim, the units.
file(GLOB DW3_RUNTIME_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_executable(dw2003 ${DW3_RUNTIME_SRCS} "${DW3_GEN}/overlay_tables.c" "${DW3_GEN}/port_state_tables.c"
               "${DW3_GEN}/port_version.c" ${DW3_PSYQ_SRCS} $<TARGET_OBJECTS:dw3_game>)
add_dependencies(dw2003 dw3_version)
set_target_properties(dw2003 PROPERTIES C_STANDARD 99 C_EXTENSIONS ON C_STANDARD_REQUIRED ON
                      LINK_DEPENDS "${DW3_GEN}/overlays.ld")
target_compile_definitions(dw2003 PRIVATE ${DW3_GAME_DEFINES})
target_include_directories(dw2003 BEFORE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" "${DW3_PORT_PSYQ_DIR}"
                           ${DW3_GAME_INCLUDES})
target_compile_options(dw2003 PRIVATE ${DW3_COMMON_FLAGS} -Wall -Wextra)
set_source_files_properties(${DW3_RUNTIME_SRCS} "${DW3_GEN}/overlay_tables.c" "${DW3_GEN}/port_state_tables.c"
                            PROPERTIES COMPILE_OPTIONS "-Werror")
if(DW3_PORT_PSYQ_WERROR AND DW3_PSYQ_SRCS)
    set_source_files_properties(${DW3_PSYQ_SRCS} PROPERTIES COMPILE_OPTIONS "-Werror")
endif()
# The software GPU runs every frame: optimised in every build (-O3: its span loops rely on inlining and on the
# vectorised select loops; port/psyq/gpu.c, "Speed").
set_property(SOURCE "${DW3_PORT_PSYQ_DIR}/gpu.c" APPEND PROPERTY COMPILE_OPTIONS -O3)
# The SPU core renders every audio frame: -O2 in every build (about 4x the port's default; port/src/spu.c).
file(GLOB DW3_SPU_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/spu*.c")
set_property(SOURCE ${DW3_SPU_SRCS} APPEND PROPERTY COMPILE_OPTIONS -O2)
# The ld script: the EXE's and each overlay's .data/.bss sections and the arena's symbols. The link map is checked
# after every link: a game object's writable data outside those sections would survive the console's reset
# (port/src/reset.c; port_gen.py sections).
target_link_options(dw2003 PRIVATE "-Wl,-T,${DW3_GEN}/overlays.ld" "-Wl,-Map,${CMAKE_BINARY_DIR}/dw2003.map")
add_custom_command(TARGET dw2003 POST_BUILD
                   COMMAND "${DW3_PYTHON}" "${DW3_PORT_GEN}" sections --map "${CMAKE_BINARY_DIR}/dw2003.map"
                   COMMENT "port_gen.py sections (the game's data is all in the reset's ranges)" VERBATIM)
# The built-in mods' manifests (port/mods/<id>/mod.json; docs/LAUNCHER.md "Mod manifest") beside the binary, where the
# launcher lists them: copied on every build (a few small files).
add_custom_target(dw3_mods ALL COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/mods"
                  "${CMAKE_BINARY_DIR}/mods" COMMENT "the mods' manifests -> ${CMAKE_BINARY_DIR}/mods" VERBATIM)
if(DW3_PORT_ALLOW_UNRESOLVED)
    target_link_options(dw2003 PRIVATE "-Wl,--unresolved-symbols=ignore-all")
endif()
target_link_libraries(dw2003 PRIVATE m)

# ---- The window (DW3_PORT_SDL): SDL3, found as a CMake package: SDL3_DIR or CMAKE_PREFIX_PATH when given (a system
# SDL3, e.g. -DSDL3_DIR=/usr/lib/x86_64-linux-gnu/cmake/SDL3), else the pinned build in tools/sdl3 (this checkout's,
# or the main checkout's when a worktree has no link to it yet). Off by default: the headless build and the tests
# stay SDL-free (video.c and input.c then compile without it: screenshots only, no window).
if(DW3_PORT_SDL)
    if(DW3_PORT_M32)
        message(FATAL_ERROR "DW3_PORT_SDL: tools/sdl3 is a 64-bit build; configure the -m32 variant without the window")
    endif()
    set(DW3_SDL_HINTS "${DW3_ROOT}/tools/sdl3")
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${DW3_ROOT}" rev-parse --path-format=absolute --git-common-dir
                        OUTPUT_VARIABLE DW3_GIT_COMMON OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE r)
        if(r EQUAL 0 AND DW3_GIT_COMMON)
            get_filename_component(DW3_MAIN_ROOT "${DW3_GIT_COMMON}" DIRECTORY)
            list(APPEND DW3_SDL_HINTS "${DW3_MAIN_ROOT}/tools/sdl3")
        endif()
    endif()
    find_package(SDL3 3.2 CONFIG REQUIRED HINTS ${DW3_SDL_HINTS})
    message(STATUS "SDL3 ${SDL3_VERSION}: ${SDL3_DIR}")
    target_link_libraries(dw2003 PRIVATE SDL3::SDL3)
    target_compile_definitions(dw2003 PRIVATE DW3_PORT_SDL)
endif()
