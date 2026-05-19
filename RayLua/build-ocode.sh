#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*)
        export PATH="/mingw64/bin:$PATH"
        ;;
esac

BUILD_DIR="build/editor-win-deb"

# detect stale cache + fix path format (MSYS2 /c/... vs Windows C:/...)
if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
    current="$(pwd)"
    cached=$(grep -m1 '^CMAKE_HOME_DIRECTORY' "$BUILD_DIR/CMakeCache.txt" 2>/dev/null | cut -d= -f2 | tr -d '\r')
    if [ -n "$cached" ]; then
        norm_cached=$(cygpath -m "$cached"   2>/dev/null || echo "$cached")
        norm_current=$(cygpath -m "$current" 2>/dev/null || echo "$current")
        lc_cached=$(printf '%s' "$norm_cached"   | tr 'A-Z' 'a-z')
        lc_current=$(printf '%s' "$norm_current" | tr 'A-Z' 'a-z')

        if [ "$lc_cached" != "$lc_current" ]; then
            echo "==> Stale cache detected, removing $BUILD_DIR"
            rm -rf "$BUILD_DIR"
        elif [ "$cached" != "$norm_cached" ]; then
            # same dir, wrong format — fix to Windows format for CMake
            sed -i "s|$cached|$norm_cached|g" "$BUILD_DIR/CMakeCache.txt"
        fi
    fi
fi

echo "==> Building RayGame..."
cmake -B "$BUILD_DIR" --preset win64-debug
cmake --build "$BUILD_DIR" --target RayGame -- -j"$(nproc 2>/dev/null || echo 4)"
echo "==> Done: $BUILD_DIR/RayGame"
