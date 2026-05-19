#!/usr/bin/env bash
set -e
# Use the same bash that is running this script (avoids accidentally picking up
# WSL bash from the Windows system PATH when running inside MSYS2).
exec "${BASH:-bash}" "$(dirname "$0")/build-common.sh" win64-debug build/editor-win-deb
