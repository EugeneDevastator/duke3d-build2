#!/bin/sh
set -e
exec "$(dirname "$0")/build-common.sh" linux-debug build/editor-linux-deb
