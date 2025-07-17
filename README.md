```
pacman -S --needed \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-pkg-config \
    mingw-w64-x86_64-glfw \
    mingw-w64-x86_64-glm \
    mingw-w64-x86_64-mesa

sudo apt install \
    build-essential \
    pkg-config \
    libglfw3-dev \
    libglm-dev \
    libgl-dev \
    libglu1-mesa-dev

brew install \
    gcc \
    pkg-config \
    glfw \
    glm
```

```
$ ./build.sh
$ ./build.sh --run
```