#!/bin/bash

mkdir -p build
mkdir -p lib

g++ -c imgui/imgui.cpp -o build/imgui.o -Iimgui
g++ -c imgui/imgui_draw.cpp -o build/imgui_draw.o -Iimgui
g++ -c imgui/imgui_tables.cpp -o build/imgui_tables.o -Iimgui
g++ -c imgui/imgui_widgets.cpp -o build/imgui_widgets.o -Iimgui
g++ -c imgui/backends/imgui_impl_glfw.cpp -o build/imgui_impl_glfw.o -Iimgui -Iglfw/include
g++ -c imgui/backends/imgui_impl_opengl3.cpp -o build/imgui_impl_opengl3.o -Iimgui -Iglfw/include

ar rcs lib/libimgui.a build/imgui.o build/imgui_draw.o build/imgui_tables.o build/imgui_widgets.o build/imgui_impl_glfw.o build/imgui_impl_opengl3.o
echo "lib/libimgui.a built."
