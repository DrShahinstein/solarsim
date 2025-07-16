#!/bin/bash

mkdir -p build

INCLUDES="-Iinclude -Iglad/include -Iglfw/include -Iglm -Iimgui/include -Iimgui/include/backends"

g++ src/main.cpp src/helper.cpp src/mainloop.cpp src/simulation.cpp src/gui.cpp glad/src/glad.c \
    $INCLUDES \
    -Llib \
    -lglfw3 -limgui -lopengl32 -lgdi32 \
    -o build/app.exe

if [ $? -eq 0 ]; then
    echo "build done."
else
    echo "build failed."
    exit 1
fi