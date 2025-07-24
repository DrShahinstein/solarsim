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

<img width="1920" height="1078" alt="Image" src="https://github.com/user-attachments/assets/5d307a4b-d7f8-40eb-bd14-91ce7dbe9ee6" />
<br/>
Press <kbd>esc</kbd> to view controls and un-capture cursor.

## Controls

| Key / Input       | Action                                    |
| ----------------- | ----------------------------------------- |
| <kbd>WASD</kbd>   | Move camera horizontally                  |
| <kbd>SPACE</kbd>  | Move camera upward                        |
| <kbd>LSHIFT</kbd> | Move camera downward                      |
| <kbd>ESC</kbd>    | Toggle mouse capture and GUI visibility   |
| <kbd>R</kbd>      | Reset simulation to the solar system      |
| <kbd>B</kbd>      | Add a custom celestial body at the cursor |
| <kbd>P</kbd>      | Toggle pause/resume simulation            |
| <kbd>C</kbd>      | Remove last celestial body                |
