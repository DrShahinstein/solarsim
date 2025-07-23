# Solarsim
3D solar system simulator using OpenGL for graphics.

## Dependencies Installation

Get [meson](https://mesonbuild.com/Getting-meson.html) first.

### Linux

#### => pacman
```bash
$ sudo pacman -S glfw glm glew mesa libx11 libxi libxrandr \
                 libxinerama libxcursor pkgconf base-devel
```

#### => apt
```bash
$ sudo apt install libglfw3-dev libgl1-mesa-dev \
                   libx11-dev libxi-dev libxrandr-dev libxinerama-dev \
                   libxcursor-dev build-essential pkg-config libglm-dev \
                   libgl-dev libglu1-mesa-dev
```

#### => dnf
```bash
$ sudo dnf install glfw-devel mesa-libGL-devel \
                   libX11-devel libXi-devel libXrandr-devel \
                   libXinerama-devel libXcursor-devel
```

### Macos

```bash
$ brew install gcc pkg-config glfw glm 
```

### Windows

Just make sure you have meson installed and you are building from [w64devkit terminal](https://github.com/DrShahinstein/solarsim/tree/main/glfw/lib-mingw).
That is also what I did in my case for Windows which was easy enough. Have no knowledge about msys2 stuff.

## Build

```
$ ./build.sh
$ ./build.sh --run
```

## Screenshot

<img width="2572" height="1520" alt="Image" src="https://github.com/user-attachments/assets/d63f097d-c256-4424-838b-0b2c40ae9de3" />

Press <kbd>esc</kbd> to view controls and un-capture cursor.
