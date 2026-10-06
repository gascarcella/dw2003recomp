# The launcher (LAUNCHER_MODS_PLAN 4.6; launcher/README.md): Dear ImGui on SDL3 + SDL_Renderer, C++ (the game stays
# C). Its own CMake project, beside the port's: the game's build and tests are untouched by it.
#
#   scripts/setup.sh sdl3 imgui
#   cmake -S launcher -B build/launcher -G Ninja && cmake --build build/launcher
#   build/launcher/dw2003-launcher            # SDL_VIDEO_DRIVER=offscreen ... --self-test DIR: the headless self-test
#
# It shares two files of the port's runtime, compiled here as they are (never modified for the launcher): the JSON
# reader (port/src/json.c) and SHA-1 (port/src/sha1.c).
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

file(GLOB DW3_LAUNCHER_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
set(DW3_LAUNCHER_SHARED "${DW3_ROOT}/port/src/json.c" "${DW3_ROOT}/port/src/sha1.c")
add_executable(dw2003-launcher ${DW3_LAUNCHER_SRCS} ${DW3_LAUNCHER_SHARED})
set_target_properties(dw2003-launcher PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF
                      C_STANDARD 99)
target_include_directories(dw2003-launcher PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src" "${DW3_ROOT}/port/src")
target_link_libraries(dw2003-launcher PRIVATE dw3_imgui SDL3::SDL3)
set_source_files_properties(${DW3_LAUNCHER_SRCS} PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra;-Werror")
