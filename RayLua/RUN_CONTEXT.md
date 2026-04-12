## RayGame Runtime Context

Date: 2026-04-12

### Current state

- `RayGame` no longer depends on the old manual OpenGL FBO path for the main frame pipeline.
- The main passes now use `raylib` `RenderTexture2D` targets in [main.cpp](./main.cpp).
- `F6` cycles the visible buffer: `final`, `combined`, `albedo`, `light`.
- `F7` exports the current debug targets to `RayLua/build/debug_captures/`.
- `F8` shows a simple 2D image diagnostic using `raylib` textures.
- The experimental `F9/F10/F11/F12` debug paths were removed after becoming unreliable and noisy during testing.

### Renderer changes in progress

- [DumbRender.hpp](./DumbRender.hpp) now enables walls, ceilings, and the floor/ceiling draw path that had been effectively disabled.
- A checkerboard fallback texture is generated in runtime and used as a debug fill for map geometry where the real material path is still not trustworthy.
- Wall fallback now also tries to use real images from the repository `tex/` directory so debug geometry is easier to read than a single checkerboard pattern.
- Portal initialization was hardened to avoid crashes when simple maps contain incomplete portal metadata.

### KSB2 compatibility work

- `doortest.map` is not a Duke/Build1 map; it starts with `KSB2`.
- The old failure mode on Linux 64-bit was caused by trying to read `KSB2` data directly into the modern `RayLua` runtime structs.
- The current loader now reads the legacy `build2.exe` `KSB2` layout through compatibility structs in [Core/loaders.c](./Core/loaders.c) and converts it into the current runtime structures.
- Tile filename records are now consumed correctly in the `KSB2` path; they had previously been skipped, which misaligned the file stream before sprite loading.
- Additional safety checks were added around sprite sector remapping and tile-index compaction in:
  - [Core/mapcore.h](./Core/mapcore.h)
  - [Core/loaders.h](./Core/loaders.h)

### What is working

- Running with a map inside `utils/DUKE3D-grp/` auto-fills the asset directory correctly because the same folder contains `palette.dat` and `TILES*.ART`.
- `E1L1.MAP` starts without the earlier startup crash.
- After the `RenderTexture2D` migration, the user can see a significant part of the map linework instead of a fully black frame.
- `albedo` captures contain scene data, which confirms the renderer is no longer completely blank.
- The wall debug fallback successfully loads project textures from `tex/` during startup.

### What is still broken

- The map is still visually incomplete.
- Some surfaces still appear black or only partially textured.
- The fallback checkerboard helps debug geometry visibility, but it is not a real material fix.
- Simple maps such as `onespr.MAP` had crashed earlier; portal-related guards were added, but this path should still be treated as unstable until re-tested.
- `doortest.map` still ends in a segmentation fault even after the legacy `KSB2` compatibility loader work, so the remaining fault is likely deeper in runtime post-load processing.
- The next required step for `doortest.map` is a `gdb` backtrace from the current binary.

### Recommended test command

From repo root:

```bash
./RayLua/build/RayGame /home/jaba/Documentos/raygame/duke3d-build2-main/utils/DUKE3D-grp/E1L1.MAP
```

### Current debugging workflow

- Use `F6` to inspect `albedo` first.
- Use `F7` to export `final.png`, `combined.png`, `albedo.png`, and `light.png`.
- Use `F8` to confirm the basic 2D `raylib` presentation path is still alive.

### Next likely work

- Finish replacing black material paths with a controlled debug/fallback texture path for all visible surfaces.
- Verify floor and ceiling UV/material assignment.
- Re-test simple maps after the portal guards.
- Capture a `gdb bt` for `doortest.map` from the current build and continue from the exact crash site.
