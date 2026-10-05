# PC port research notes

**Status: exploratory, nothing here is decided.** These are notes from a discussion (2026-10-01) about the
later PC port goal (Linux + Windows). When a choice is actually made, record it in `docs/DECISIONS.md`.

**The concrete proposal is `docs/PC_PORT_PLAN.md`** (2026-10-04): a measured inventory of what the game C needs
(123 Psy-Q functions, GTE, overlays, 32-bit assumptions), the approaches compared, milestones and the decisions
needed. This file stays as background; where they differ (e.g. forking PsyCross), the plan has the newer reasoning.

## Platform strategy

- **Native cross-platform, not Windows-first + Proton.** The port is plain C, so a native Linux build is the
  default on the dev machine; Windows is another compile target (MinGW-w64, or clang + MSVC libs), tested under
  Wine/Proton and built in a CI matrix. Proton compatibility comes for free as a fallback.
  (Windows-first + Proton made sense for a MonoGame/.NET project; it doesn't carry over to C.)
- **SDL3** for window, input, audio and graphics.
- **Graphics API:** raw Vulkan is a lot of boilerplate for a PS1 workload (textured/flat tris and quads from a
  1 MB VRAM, 4/8/15-bit CLUT textures, a few semi-transparency modes). Preferred: **SDL3 GPU API** (Vulkan on
  Linux, D3D12 on Windows). Simpler alternative: OpenGL 3.3. Either way, abstract at the PS1-primitive level so
  the backend can be swapped.

## The core work: replacing the Psy-Q libraries

Psy-Q code stays as split asm in the matching build, so the port needs a PC implementation of every library
the game calls:

| Library | PC replacement |
|---|---|
| libgpu / libgs (drawing, ordering tables) | Renderer |
| libgte (geometry coprocessor) | C math; matching its fixed-point quirks keeps visuals faithful |
| libspu, **libsnd** (sound) | Software mixer into SDL audio, plus a SEQ/VAB sequence player |
| libcd (disc reads) | File reads from the user's extracted disc (later: mod VFS, below) |
| libpad / libetc (pads, vsync) | SDL input, frame timing |
| libpress (MDEC, title overlay only) | FMV decoder for `MOVIE*.STR` (MDEC video + XA audio) |

Other known port problems:
- **32-bit assumptions:** structs and file formats that store pointers break on 64-bit. Either build 32-bit
  first or convert stored pointers to offsets/indices.
- **Overlays** become normal, always-loaded code (the mapped overlay bases help here).
- **Frame timing** is tied to vsync; widescreen, higher frame rate and 50/60 Hz are each their own project.

## PsyCross

[OpenDriver2/PsyCross](https://github.com/OpenDriver2/PsyCross): a Psy-Q-compatible layer used by the
REDRIVER2 PC port. Facts from its README (checked 2026-10-01):
- **MIT** license (forking/vendoring is fine; record it in `docs/THIRD_PARTY.md` when borrowed).
- Implements libgte, libgpu, libspu (with ADPCM), libcd (BIN/CUE), libpad; SDL2 + OpenGL + OpenAL-soft;
  PGXP-Z (precise GTE vertices, z-buffer). Claims ~95% Psy-Q compatibility. Emscripten/web support.
- TODO list: missing libgte functions, **MDEC**, **CD audio / XA**, **ADSR**.
- 64-bit support is not stated; check early.

Gaps for this game specifically:
- **libsnd is not covered.** It's the largest Sony library in our EXE (91 objects, 35 KB; `docs/TOOLCHAIN.md`),
  and music uses it (VAB banks in `DAT/SOUND/`, `docs/FORMATS.md`). Without ADSR, instruments sound wrong.
- **FMV:** MDEC + XA are both TODO in PsyCross.

So even with PsyCross, the audio side (sequence player, VAB, ADSR, XA) and FMV are ours to write.

**Leaning:** fork PsyCross for bring-up (fast path to "boots to title / plays"), use it as the reference,
and plan to replace its renderer and most of its audio with our own.

## Graphics improvements

Heavy improvements are the point where owning the renderer stops being optional. Where each one hooks in:

| Improvement | Hook point | Notes |
|---|---|---|
| Higher internal resolution, MSAA | Renderer | Any hardware renderer |
| No wobble, perspective-correct textures | GTE layer | Almost free in a port: keep the math in float and pass it through (PGXP-style) |
| CRT, xBR, FXAA, other post-processing | Final output pass | **librashader** runs RetroArch shader presets on Vulkan/GL/D3D, giving the whole RetroArch shader catalog |
| Texture filtering on paletted textures | Shader, after the CLUT lookup | Bilinear on palette indices is garbage; filter after the lookup (as DuckStation does) |
| Texture replacement / HD packs | **Asset loading**, not VRAM | The decomp's big advantage: emulators hash VRAM regions and guess, we know which file/texture is being loaded |
| Widescreen | Game code (projection, HUD anchoring, culling) | Only practical with source; mostly game-side work |
| Higher frame rate | Game logic | Separate project |

Two-path renderer idea:
1. **Faithful:** VRAM emulated as a texture, the game's ordering tables drawn as-is. Original look, and a
   reference to compare against the real game.
2. **Enhanced:** same primitives with float precision, replacement textures and post-processing. Over time,
   specific systems (field 3D, battle models) can hand richer data to the renderer and skip the PS1 primitive
   format.

## Modding

- **Data mods:** the libcd replacement becomes a virtual filesystem: `mods/` first (by path), then the user's
  disc. The file table (`filetable_sectors` and the LBA lookups, `docs/DISC_LAYOUT.md`) is the one place to
  intercept.
- **Bigger assets:** game buffers are sized for 2 MB of RAM; limits get lifted one at a time.
- **Code mods:** the source is open, so direct code mods work; a scripting/hook API only if there's demand.

## Rough order (if this direction holds)

1. Bring-up with forked PsyCross: title screen, then gameplay.
2. Fill the gaps: libsnd, ADSR, XA, MDEC.
3. Own SDL3 GPU renderer with faithful + enhanced paths; keep PsyCross's renderer to compare during the swap.
4. Mod VFS, load-time texture replacement, librashader.
