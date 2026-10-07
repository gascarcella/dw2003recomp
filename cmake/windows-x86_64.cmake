# The Windows x86_64 cross toolchain (DECISIONS "Windows"): llvm-mingw's clang and lld (UCRT) from tools/llvm-mingw
# (scripts/setup.sh llvm-mingw), shared by the port and the launcher:
#
#   cmake -S port -B build/port-win -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/windows-x86_64.cmake -DDW3_PORT_SDL=ON
#   cmake -S launcher -B build/launcher-win -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/windows-x86_64.cmake
#
# scripts/build_windows.sh does both. Everything links statically (winpthreads, libc++ in the launcher: no DLL beside
# the executables); every build carries CodeView debug info and lld writes a PDB beside each executable (the crash
# minidumps' symbols); ASLR stays on. SDL3 for Windows comes from tools/sdl3-windows (scripts/setup.sh sdl3-windows).
# The toolchain is $DW3_LLVM_MINGW when set, else this checkout's tools/llvm-mingw, else the main checkout's
# (scripts/setup.sh installs there and links it into worktrees).
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

get_filename_component(_dw3_root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(_dw3_tool_dirs "${_dw3_root}/tools")
find_program(_dw3_git git)
if(_dw3_git)
    execute_process(COMMAND "${_dw3_git}" -C "${_dw3_root}" rev-parse --path-format=absolute --git-common-dir
                    OUTPUT_VARIABLE _dw3_git_common OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE _r)
    if(_r EQUAL 0 AND _dw3_git_common)
        get_filename_component(_dw3_main_root "${_dw3_git_common}" DIRECTORY)
        list(APPEND _dw3_tool_dirs "${_dw3_main_root}/tools")
    endif()
endif()

set(DW3_LLVM_MINGW "$ENV{DW3_LLVM_MINGW}" CACHE PATH "llvm-mingw's install directory (bin/x86_64-w64-mingw32-clang)")
if(NOT DW3_LLVM_MINGW)
    foreach(d IN LISTS _dw3_tool_dirs)
        if(EXISTS "${d}/llvm-mingw/bin/x86_64-w64-mingw32-clang")
            set(DW3_LLVM_MINGW "${d}/llvm-mingw" CACHE PATH "" FORCE)
            break()
        endif()
    endforeach()
endif()
if(NOT EXISTS "${DW3_LLVM_MINGW}/bin/x86_64-w64-mingw32-clang")
    message(FATAL_ERROR "llvm-mingw not found: run scripts/setup.sh llvm-mingw (or set DW3_LLVM_MINGW)")
endif()

set(_dw3_triple x86_64-w64-mingw32)
set(CMAKE_C_COMPILER "${DW3_LLVM_MINGW}/bin/${_dw3_triple}-clang")
set(CMAKE_CXX_COMPILER "${DW3_LLVM_MINGW}/bin/${_dw3_triple}-clang++")
set(CMAKE_RC_COMPILER "${DW3_LLVM_MINGW}/bin/${_dw3_triple}-windres")
set(CMAKE_AR "${DW3_LLVM_MINGW}/bin/${_dw3_triple}-ar")
set(CMAKE_RANLIB "${DW3_LLVM_MINGW}/bin/${_dw3_triple}-ranlib")
set(CMAKE_NM "${DW3_LLVM_MINGW}/bin/${_dw3_triple}-nm")
set(CMAKE_STRIP "${DW3_LLVM_MINGW}/bin/${_dw3_triple}-strip")

# Libraries and packages only from the Windows prefixes (SDL3 for Windows, the mingw-w64 sysroot), never from the
# host; programs from the host.
set(CMAKE_FIND_ROOT_PATH "")
foreach(d IN LISTS _dw3_tool_dirs)
    if(EXISTS "${d}/sdl3-windows/lib/cmake/SDL3/SDL3Config.cmake")
        list(APPEND CMAKE_FIND_ROOT_PATH "${d}/sdl3-windows")
    endif()
endforeach()
list(APPEND CMAKE_FIND_ROOT_PATH "${DW3_LLVM_MINGW}/${_dw3_triple}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# CodeView debug info in every build type and a PDB beside each executable (lld's --pdb= names it after the output);
# static runtimes.
set(CMAKE_C_FLAGS_INIT "-gcodeview")
set(CMAKE_CXX_FLAGS_INIT "-gcodeview")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static -Wl,--pdb=")
