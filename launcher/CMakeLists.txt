# The launcher (docs/LAUNCHER.md "Launcher"; launcher/README.md): Dear ImGui on SDL3 + SDL_Renderer, C++ (the game stays
# C). Its own CMake project, beside the port's: the game's build and tests are untouched by it.
#
#   scripts/setup.sh sdl3 imgui
#   cmake -S launcher -B build/launcher -G Ninja && cmake --build build/launcher
#   build/launcher/dw2003-launcher            # SDL_VIDEO_DRIVER=offscreen ... --self-test DIR: the headless self-test
#
# It shares two files of the port's runtime, compiled here as they are (never modified for the launcher): the JSON
# reader (port/runtime/json.c) and SHA-1 (port/runtime/sha1.c).
cmake_minimum_required(VERSION 3.20)
project(dw2003launcher C CXX)

get_filename_component(DW3_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

# Built tools live in this checkout's tools/, or the main checkout's when a worktree has no link to them yet
# (scripts/setup.sh installs into the main checkout).
set(DW3_TOOL_DIRS "${DW3_ROOT}/tools")
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${DW3_ROOT}" rev-parse --path-format=absolute --git-common-dir
                    OUTPUT_VARIABLE DW3_GIT_COMMON OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE r)
    if(r EQUAL 0 AND DW3_GIT_COMMON)
        get_filename_component(DW3_MAIN_ROOT "${DW3_GIT_COMMON}" DIRECTORY)
        list(APPEND DW3_TOOL_DIRS "${DW3_MAIN_ROOT}/tools")
    endif()
endif()

# SDL3: SDL3_DIR or CMAKE_PREFIX_PATH when given, else the pinned build (scripts/setup.sh sdl3).
set(DW3_SDL_HINTS "")
foreach(d IN LISTS DW3_TOOL_DIRS)
    list(APPEND DW3_SDL_HINTS "${d}/sdl3")
endforeach()
find_package(SDL3 3.2 CONFIG REQUIRED HINTS ${DW3_SDL_HINTS})
message(STATUS "SDL3 ${SDL3_VERSION}: ${SDL3_DIR}")

# Dear ImGui: DW3_IMGUI_DIR when given, else the pinned clone (scripts/setup.sh imgui).
set(DW3_IMGUI_DIR "" CACHE PATH "Dear ImGui's source directory (imgui.cpp, backends/)")
if(NOT DW3_IMGUI_DIR)
    foreach(d IN LISTS DW3_TOOL_DIRS)
        if(EXISTS "${d}/imgui/imgui.cpp")
            set(DW3_IMGUI_DIR "${d}/imgui")
            break()
        endif()
    endforeach()
endif()
if(NOT EXISTS "${DW3_IMGUI_DIR}/imgui.cpp")
    message(FATAL_ERROR "Dear ImGui not found: run scripts/setup.sh imgui (or set DW3_IMGUI_DIR)")
endif()
message(STATUS "Dear ImGui: ${DW3_IMGUI_DIR}")

add_library(dw3_imgui STATIC
            "${DW3_IMGUI_DIR}/imgui.cpp" "${DW3_IMGUI_DIR}/imgui_draw.cpp" "${DW3_IMGUI_DIR}/imgui_tables.cpp"
            "${DW3_IMGUI_DIR}/imgui_widgets.cpp" "${DW3_IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"
            "${DW3_IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp")
target_include_directories(dw3_imgui SYSTEM PUBLIC "${DW3_IMGUI_DIR}" "${DW3_IMGUI_DIR}/backends")
target_compile_features(dw3_imgui PUBLIC cxx_std_17)
target_link_libraries(dw3_imgui PUBLIC SDL3::SDL3)

# The launcher's version stamp (launcher_version, launcher_commit: the Copy text's header), from git describe at every
# build, as the game's (port/cmake/version.cmake rewrites the file only when it changed).
set(DW3_LAUNCHER_VERSION_C "${CMAKE_BINARY_DIR}/gen/launcher_version.c")
add_custom_target(dw3_launcher_version COMMAND "${CMAKE_COMMAND}" -DROOT=${DW3_ROOT} -DOUT=${DW3_LAUNCHER_VERSION_C}
                  -DPREFIX=launcher -P "${DW3_ROOT}/port/cmake/version.cmake" BYPRODUCTS "${DW3_LAUNCHER_VERSION_C}"
                  COMMENT "port/cmake/version.cmake (the launcher's version stamp)" VERBATIM)

file(GLOB DW3_LAUNCHER_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
set(DW3_LAUNCHER_SHARED "${DW3_ROOT}/port/runtime/json.c" "${DW3_ROOT}/port/runtime/sha1.c" "${DW3_LAUNCHER_VERSION_C}")
add_executable(dw2003-launcher ${DW3_LAUNCHER_SRCS} ${DW3_LAUNCHER_SHARED})
add_dependencies(dw2003-launcher dw3_launcher_version)
set_target_properties(dw2003-launcher PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF
                      C_STANDARD 99)
target_include_directories(dw2003-launcher PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src" "${DW3_ROOT}/port/include")
target_link_libraries(dw2003-launcher PRIVATE dw3_imgui SDL3::SDL3)
set_source_files_properties(${DW3_LAUNCHER_SRCS} PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra;-Werror")
# The release (scripts/package_appimage.sh) links the C++ runtime statically, so the AppImage needs only the C library
# of the player's system (libstdc++ and libgcc_s are newer on the build machine than on older desktops). The GCC
# runtime library exception covers it: no notice is owed.
option(DW3_LAUNCHER_STATIC_RUNTIME "Link libstdc++ and libgcc statically (the release)" OFF)
if(DW3_LAUNCHER_STATIC_RUNTIME)
    target_link_options(dw2003-launcher PRIVATE -static-libstdc++ -static-libgcc)
endif()

# Windows (cmake/windows-x86_64.cmake): a GUI-subsystem program (no console window behind the launcher; SDL_main.h
# keeps `main` the entry) with the manifest resource (launcher/windows/: the UTF-8 code page, long paths, per-monitor
# DPI awareness), compiled by the toolchain's windres, as the game's (port/windows/).
if(WIN32)
    enable_language(RC)
    target_sources(dw2003-launcher PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/windows/dw2003-launcher.rc")
    set_target_properties(dw2003-launcher PROPERTIES WIN32_EXECUTABLE ON)
endif()
