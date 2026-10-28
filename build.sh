#!/bin/bash
set -e

# Set WRAP_MODE=default to prefer system packages when they are available.
WRAP_MODE="${WRAP_MODE:-forcefallback}"

if [[ "$1" == "--run" ]]; then
    ./build/dist/solarsim
else
    if [ ! -d build ]; then
        meson setup build --prefix "${PWD}/build" --wrap-mode "${WRAP_MODE}"
    fi
    meson compile -C build
    meson install -C build --skip-subprojects
fi

