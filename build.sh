#!/bin/bash

if [[ "$1" == "--run" ]]; then
    ./build/dist/solarsim
else
    meson setup build --prefix ${PWD}/build
    meson compile -C build
    cd build
    meson install
fi