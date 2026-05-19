#!/bin/sh
set -e
exec "$(dirname "$0")/build-common.sh" win64-release build/editor-win-rel
