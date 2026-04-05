# RayLua Linux Build Notes

This file documents the native Linux/GCC workflow that was made to work in this repo.

## Current status

`RayLua/build/RayGame` builds successfully on Arch Linux with:

- `gcc`
- `g++`
- `cmake`
- `ninja`
- system `raylib 5.5`
- OpenGL runtime libraries

The binary produced in this session is:

- `RayLua/build/RayGame`

## Build command

From the repository root:

```bash
cmake -S RayLua -B RayLua/build -G Ninja \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++ \
  -DDISABLE_LUA=ON \
  -DBUILD_DUKEGAME=OFF

cmake --build RayLua/build -j4
```

## Why these options matter

- `DISABLE_LUA=ON`
  The checked-in Lua path was Windows/MSVC-only and the vendored LuaJIT tree is not present in this checkout.

- `BUILD_DUKEGAME=OFF`
  `DukeGame` is still much more DOS/Win32-oriented than `RayGame`. `RayGame` was the correct first native target.

## Local dependency layout

The CMake setup now prefers local vendored dependencies in `RayLua/External` before trying to fetch from the network.

Used in this session:

- `RayLua/External/imgui`
- `RayLua/External/rlImGui`

Pinned revisions:

- `imgui`: `v1.92.1`
- `rlImGui`: `4d8a618`

## Important Linux-specific fixes already applied

- Added a real `RayLua/CMakeLists.txt`
- Removed hardcoded Win32/32-bit CMake assumptions
- Added Linux/OpenGL linkage
- Added `-fcommon` because old Build-style headers define globals in headers
- Fixed case-sensitive paths like `Core` vs `core`
- Fixed Windows path separators in includes
- Replaced several MSVC-only functions and inline asm fragments with portable code
- Stopped CMake from compiling header files as standalone translation units

## Known limitations

- `RayGame` builds, but some warnings remain in old math/shared headers.
- `DukeGame` is not part of the verified Linux build yet.
- Lua is still disabled for the verified Linux build.
- The code still carries a lot of legacy Build-era global state and header-defined storage.

## Recommended workflow

For Neovim:

1. Configure once with the command above.
2. Point your LSP at `RayLua/build/compile_commands.json`.
3. Rebuild with:

```bash
cmake --build RayLua/build -j4
```

4. Run with:

```bash
./RayLua/build/RayGame
```

If you want the next step, the best target is:

- native Linux support for `DukeGame`
- or Linux LuaJIT support for `RayGame`
