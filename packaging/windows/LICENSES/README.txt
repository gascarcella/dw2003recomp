The licences of what this package contains.

  dw2003recomp.txt        dw2003recomp: the launcher, the PC port's runtime and the game's decompiled C
                          (https://github.com/gascarcella/dw2003recomp)
  SDL3.txt                SDL 3 (zlib licence), linked statically into both programs (dw2003.exe and
                          dw2003-launcher.exe)
  imgui.txt               Dear ImGui (MIT), compiled into the launcher
  ProggyForever.txt       the launcher's font (MIT), embedded in Dear ImGui
  ProggyClean.txt         Dear ImGui's other embedded font (MIT)
  llvm.txt                the LLVM project (Apache 2.0 with the LLVM exception): the parts of its runtime linked
                          statically into both programs by llvm-mingw (compiler-rt's builtins, libunwind; libc++ in
                          the launcher) (https://github.com/mstorsjo/llvm-mingw)
  mingw-w64-runtime.txt   the mingw-w64 runtime (its own notices), linked statically into both programs
                          (https://www.mingw-w64.org/)
  winpthreads.txt         mingw-w64's winpthreads (MIT and BSD), linked statically into both programs

The Windows C runtime itself (the UCRT) is part of Windows and is not included.
No game data is included: the game is read from your own disc image of Digimon World 2003 (PS1, Europe).
