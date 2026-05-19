#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

# On MSYS2/Windows the MinGW64 toolchain (cmake, gcc, g++, mingw32-make)
# lives in /mingw64/bin.  When the script is invoked from a plain MSYS2 bash
# session (e.g. via mingw64.exe or usr/bin/bash -c "...") that directory is
# not always on PATH.  Prepend it when we are running under MSYS2/MinGW/Cygwin.
case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*)
        export PATH="/mingw64/bin:$PATH"
        ;;
esac

PRESET="$1"
BUILD_DIR="$2"

if [ -z "$PRESET" ] || [ -z "$BUILD_DIR" ]; then
    echo "Usage: $0 <preset> <build_dir>"
    echo "Presets: linux-debug, win64-debug, win64-release"
    exit 1
fi

# fix CMake cache path case (case-insensitive FS like VirtualBox shared folders)
if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
    current="$(pwd)"
    cached=$(grep -m1 '^CMAKE_HOME_DIRECTORY' "$BUILD_DIR/CMakeCache.txt" 2>/dev/null | cut -d= -f2)
    if [ -n "$cached" ] && [ "$cached" != "$current" ] && [ "$(printf '%s' "$cached" | tr 'A-Z' 'a-z')" = "$(printf '%s' "$current" | tr 'A-Z' 'a-z')" ]; then
        sed -i "s|$cached|$current|g" "$BUILD_DIR/CMakeCache.txt"
    fi
fi

# detect stale cache from different machine (path mismatch after case fix)
if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
    current="$(pwd)"
    cached=$(grep -m1 '^CMAKE_HOME_DIRECTORY' "$BUILD_DIR/CMakeCache.txt" 2>/dev/null | cut -d= -f2)
    if [ -n "$cached" ] && [ "$cached" != "$current" ]; then
        echo "==> Stale cache detected (different machine), removing $BUILD_DIR"
        rm -rf "$BUILD_DIR"
    fi
fi

echo "==> Building preset '$PRESET' into $BUILD_DIR"
cmake -B "$BUILD_DIR" --preset "$PRESET"
# MinGW Makefiles can race on dep-file directories with --parallel;
# use $(nproc) explicit jobs instead which is stable across generators.
cmake --build "$BUILD_DIR" --target RayGame -- -j"$(nproc 2>/dev/null || echo 4)"
echo "==> Done: $BUILD_DIR/RayGame"
