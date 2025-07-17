#!/bin/bash

if [[ "$1" == "--run" ]]; then
    ./build/app/solarsim
else
    meson setup build
    meson compile -C build
    cd build
    meson install --destdir .
fi