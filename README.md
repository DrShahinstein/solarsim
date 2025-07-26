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

<img width="1920" height="1080" alt="Image" src="https://github.com/user-attachments/assets/c0dc562a-703f-46c0-b66f-1d64f529cc80" />

## Black Holes

https://github.com/user-attachments/assets/032a55b1-4358-4ba0-a9fb-3b649fbc8493

<br/>

## Keys

| Key / Input       | Action                                    |
| ----------------- | ----------------------------------------- |
| <kbd>WASD</kbd>   | Move camera horizontally                  |
| <kbd>SPACE</kbd>  | Move camera upward                        |
| <kbd>LSHIFT</kbd> | Move camera downward                      |
| <kbd>ESC</kbd>    | Toggle mouse capture and GUI visibility   |
| <kbd>R</kbd>      | Reset simulation                          |
| <kbd>B</kbd>      | Add a custom celestial body at the cursor |
| <kbd>P</kbd>      | Toggle pause/resume simulation            |
| <kbd>C</kbd>      | Remove last celestial body                |
