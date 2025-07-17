```
pacman -S --needed \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-pkg-config \
    mingw-w64-x86_64-glfw \
    mingw-w64-x86_64-glm \
    mingw-w64-x86_64-mesa \

sudo apt install libglfw3-dev libgl1-mesa-dev \
                 libx11-dev libxi-dev libxrandr-dev libxinerama-dev \
                 libxcursor-dev build-essential pkg-config libglm-dev \
                 libgl-dev libglu1-mesa-dev

sudo dnf install glfw-devel mesa-libGL-devel \
                 libX11-devel libXi-devel libXrandr-devel \
                 libXinerama-devel libXcursor-devel

brew install \
    gcc \
    pkg-config \
    glfw \
    glm \
```

```
$ ./build.sh
$ ./build.sh --run
```