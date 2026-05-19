#!/bin/sh
set -e
exec "$(dirname "$0")/build-common.sh" win64-debug build/editor-win-deb
