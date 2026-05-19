#!/bin/sh
set -e
cd "$(dirname "$0")"

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
cmake --build "$BUILD_DIR" --target RayGame --parallel
echo "==> Done: $BUILD_DIR/RayGame"
