# RayLua Architecture Notes

This is a high-level map of `RayLua` as it exists in this repo.

## What RayLua is

`RayLua` is the native rendering/editor side of this project: a modernized Build2-based experiment using:

- Ken Silverman's Build2-style geometry/runtime ideas
- `raylib` for windowing/input/basic rendering setup
- `rlImGui` + `imgui` for editor UI

The important point is that this is not a clean engine rewrite. It is a hybrid:

- old Build-style world/math/core code
- new raylib-facing renderer/editor code
- optional Duke integration behind compile-time paths

## Main layers

### `main.cpp`

Owns the application loop:

- window creation
- render target setup
- postprocessing passes
- editor UI flow
- map loading validation

This is the app shell around the rest of the engine/editor.

### `DumbRender.hpp`

This is the main bridge from Build-style data to raylib/OpenGL drawing.

It handles:

- shader setup
- geometry drawing
- lights pass
- map texture conversion
- camera-space conversions

If you want to understand "how the old world becomes modern pixels", start here.

### `DumbCore.hpp`

Owns higher-level engine control:

- initialization of loaded maps
- free camera mode
- optional Duke-driven update path when Duke is enabled

Think of it as runtime control/state glue.

### `DumbEdit.hpp`

Editor behavior and manipulation logic:

- selection
- movement
- loop/sector editing
- interaction logic

This is editor-side stateful tooling, not low-level rendering.

### `Core/`

This is the heavy legacy/runtime layer:

- `mapcore.*`
- `shadowtest2.*`
- `monoclip.*`
- `physics.*`
- `loaders.*`
- `artloader.*`
- `kplib.*`

These files contain the most Build2-like logic and the hardest portability issues.

### `interfaces/`

Thin shared API/event layer between systems.

Important idea:

- it is trying to define a stable bridge between runtime/editor/game logic

### `DukeGame/`

Optional Duke-specific integration layer.

This is not yet part of the verified Linux build in this session.

## Practical mental model

Use this model when reading the code:

1. `loaders`/`artloader` build the map and tile data.
2. `DumbCore` initializes runtime state around that map.
3. `shadowtest2` + `mapcore` + `scenerender` generate visible geometry/light data.
4. `DumbRender` pushes that into raylib/OpenGL drawing.
5. `main.cpp` wraps it all in framebuffers, postprocess passes, and editor UI.

## Porting lesson from this session

The real Linux blockers were not raylib itself. They were:

- Windows-first CMake
- case sensitivity
- MSVC-specific helper code
- legacy headers that define storage directly
- old CRT calls like `stricmp`, `fopen_s`, `strcpy_s`

That means future Linux work should focus less on the outer app shell and more on:

- `Core/`
- `DukeGame/source/`
- global/header hygiene

## Good next milestones

- isolate Build globals out of headers into one translation unit
- make LuaJIT optional on Linux through system packages
- add a Linux build path for `DukeGame`
- reduce direct raw OpenGL usage where raylib/rlgl equivalents exist
