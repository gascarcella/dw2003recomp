# Play-tests of the PC port

The port played by a person on a desktop: the window, the keyboard and gamepads, the audio device, real time. The
automated tests (`tests/port/`, `scripts/test.sh`) check the same paths headlessly; this file is what only a play-test
shows. Newest first.

## 2026-10-06: first desktop play-test (session 17)

**Machine:** Nobara Linux 44 (Fedora 44, KDE Plasma, Wayland session), kernel 7.2.8, GCC 16.2.1, NVIDIA GeForce RTX 4070
Ti SUPER with the proprietary driver 595.104.02, PipeWire. The unpatched EU disc (SHA-1 `457cb233…`), linked by
`scripts/setup.sh` from `tools/local.env`'s `DW3_DISC_BIN`.

**Build:** the SDL `-dev` headers installed by the user (`dnf`: libXrandr, libXcursor, libXi, libXfixes, libXScrnSaver,
libXtst, wayland, wayland-protocols, libxkbcommon, libdecor, mesa EGL/GL/gbm, libdrm, alsa-lib, pulseaudio-libs,
pipewire, dbus; libX11, libXext and systemd's libudev were already there), then `rm -rf tools/sdl3 && scripts/setup.sh
sdl3`: no optional feature turned off; **video** `dummy kmsdrm offscreen wayland x11`, **audio** `alsa disk dummy pipewire
pulseaudio`, **joystick** `hidapi linux virtual`. `cmake -S port -B build/port-sdl -G Ninja -DDW3_PORT_SDL=ON && cmake
--build build/port-sdl`: clean, 114 warnings, all in the game's matching C under GCC 16 (`-Wmissing-braces` 44,
`-Wreturn-mismatch` 34, `-Wreturn-type` 34, `-Wdiscarded-qualifiers` 2), none in `port/`.

**What was played** (`--window --scale 3`: Wayland, SDL's OpenGL renderer, 960x720; keyboard only, no gamepad
attached), two runs on one `.mcd` card (`--memcard1`):
1. Run 1, 1,028 s: CNTY_SEL (English), the logo movie, the title, New Game, the registration and the city to the story's
   first battle (FIGHTSTG + WFIGHTMN, won), STFGTREP, back on the field, the save at the Asuka Inn (map 0x20A,
   STGMCARD), the window closed.
2. Run 2 (the "reset": the window has no reset key, so the port was started again on the same card): title →
   Continue → STGMCARD → the Asuka Inn (map 0x20A) at frame 1,598. The load worked.

The save (`BESLES-03936DMW3-EUR`, 4 blocks) is kept outside the repository by the user.

### Findings

| # | Area | Severity | Finding | State |
|---|---|---|---|---|
| 1 | Exit | crash | The SDL build segfaulted after `port_exit` on Wayland and offscreen video | **fixed here** |
| 2 | Battle picture | major | The battle camera is wrong: at floor level between the two Digimon | open (issue #7) |
| 3 | Audio | minor | One 63 ms queue refill near the first battle's start in a 17-minute run | open, watch |
| 4 | Screenshots | note | `--screenshot` writes the display's own pixels: the movies' 320x480 frames come out tall | as designed |

**1. The crash at exit (fixed in this change).** `build/port-sdl/dw2003 --input-test` passed all 70 checks, then died
with SIGSEGV (exit 139) after its "exit 0" line. Narrowed by driver: `SDL_VIDEO_DRIVER=wayland` and `offscreen` crash
(with any audio driver, or `--mute`); `x11` does not. The SDL build's only teardown was `atexit(SDL_Quit)` in
`port_video_open`, so SDL unloaded its video libraries inside `exit()`. `LD_DEBUG=files` shows NVIDIA's EGL stack
(`libnvidia-egl-wayland`, `-gbm`, `-xcb`, `-xlib`, `libnvidia-eglcore.so.595.104.02`, `libnvidia-gpucomp`) finalised and
unmapped there, and the core dump's only frame is a PC in unmapped memory. So something still called into the unloaded
NVIDIA library. What called it was not identified (no gdb on the host). X11 goes through GLX and unloads none of this.
The fix: `port_video_quit` (`port/src/video.c`) destroys the texture, the renderer and the window and calls `SDL_Quit`
from `port_exit` (after the audio's close, before `exit()`). After the fix the input test exits 0 on wayland, offscreen
and x11, and both play runs ended with exit 0 when the window was closed. The WAV, the log and the
record were never at risk: `port_exit` writes them before `exit()`.

**2. The battle camera (open, issue #7).** Reported by the user in the first battle: the camera is "in the floor, just in front of
my digimon, seeing the enemy feet; my digimon feet do overlay, so it may be a bit behind my digimon". The
battle works (menus, damage, HP bars, the win, the experience screen); only the 3D view is wrong. Cause, from the code:
`GsSetRefView2` (`port/psyq/libgs.c`) is a stub that leaves LIBGS's world-screen matrix `D_80081358` as it is (the
identity), and FIGHTSTG (`fightstg_8008D3B4.c`, its two `GsSetRefView2` calls and the six uses of `D_80081358`) composes
every model with that matrix. An identity view puts the camera at the world origin looking along +z with no height
and no tilt, which is what the user saw. The PS1 version is `asm/main/psyq/libgs/gs_131.s` (it builds the matrix from the
viewpoint, the reference point and the twist, in `pv->super`'s coordinate system, starting from the identity at
`0x80081398`, and copies the result to `0x80081338`). The fix wants a golden family like `gte`'s (`D_80081358`'s words
after `GsSetRefView2` in the emulator) and a battle VRAM comparison; the M2 VRAM checks were on `new_game` only (no
battle). The screenshots in the battle (frames 36,000 to 42,000) show the HUD right and the scene filled by floor and
effect quads seen from ground level.

**3. One audio refill (minor).** Run 1's audio queue held 37-102 ms (mean 66.3) against the 63.2 ms target over 51,400
vsyncs, with no drop and no ratio change, but one refill (63.2 ms of silence) between vsync 35,001 and 35,501. FIGHTSTG
and WFIGHTMN were loaded at vsync 34,705, so it was one host-side stall at the battle's start. The user did not report
hearing it. Run 2: 4,471 vsyncs, 0 refills, 2 ratio changes. To watch: if it recurs at every battle, look at what the
host does there (an overlay load, the first draw of the battle's textures through the GL renderer).

**4. Screenshots of the movies.** A `--screenshot` taken in the attract movie is 320x480 (the movie's interlaced
mode), not 320x240: the PPM holds the display's own pixels (port/README.md "The picture"). Drawn at 4:3, as the window
does, the frames are right. A shot 56 frames after the title's map started was black: the title's fade-in. Neither is
a bug.

### What worked
- **Picture:** the language select, the logo movie, the title, the field maps, the dialogue boxes, the menus, the
  battle's HUD and the save/load screens. The attract movie (watched to the end in a headless replay with screenshots:
  the Bandai logo and the CG intro) decodes cleanly, and its XA soundtrack is in the WAV (RMS 1,500-7,100 per 4 s,
  no sample at full scale).
- **Sound:** music and effects through PipeWire (44,100 Hz stereo, 1,024-frame periods), no crackle reported.
- **Timing:** 51,400 frames presented in 1,028.15 s = 49.99 per second (PAL's 50).
- **Input:** `--input-test` 70 of 70 (16 keys, 12 gamepad buttons, 6 axes, a chord; the gamepad is SDL's virtual one).
  The game was played on the keyboard without a problem reported. A physical gamepad was not tried.
- **Saves:** a save to the `.mcd` card, the port restarted, Continue loaded it.
- **No crash in play**, no watchdog, exit 0 on closing the window, both runs.

### Not covered
A physical gamepad; fullscreen (F11); `--scale` other than 3; the X11 session (only `SDL_VIDEO_DRIVER=x11` under
XWayland for the input test); other GPUs and drivers; the window's console reset (it has none: a reset key would make
the reset testable in play, as the scripts' `reset` step does headlessly).

### How it was run
```sh
build/port-sdl/dw2003 --input-test
build/port-sdl/dw2003 --disc iso/dw2003.cue --window --scale 3 --memcard1 card1.mcd \
  --log frames.log --record record.json --wav audio.wav --watchdog 30 --screenshot 1500:f1500.ppm ...   # one per 1,500 frames
```
The captures (logs, WAVs, screenshots) stayed on the machine: they are the game's pictures and sound.
