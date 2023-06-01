# NaiveView
My personal project for making a car blind-point monitor system using Raspberry Pi.

# Introduction



# Prerequisites

The libraries below are used in this project. Please install them using `apt`. 

- libgpiod: access GPIO to adjust camera IR cut.
- libcamera: handle with camera.
- libegl, libdrm, libgbm: provide zero-copy texture that directly use DMA buffer from libcamera.
- libepoxy: obtain OpenGL and EGL API entries.

If you want to build from source, please install their *-dev* packages. In addition, install following packages that is used in build:

- cmake: the build system used in this project.
- perl: used in some source code generation script.
- pkg-config: find libraries.

# Build

The project uses CMake to manage building progress, please refer to CMake's documentation for more details.

Briefly, firstly you need create a directory for building:

```
cd some_place_you_like
mkdir NaiveView-build
cd NaiveView-build
```

Then run CMake to setup the build (you probably want to have release build):
```
cmake where_you_download_the_source/NaiveView -DCMAKE_BUILD_TYPE=Release
```

If CMake reports no error, you could build everything:
```
make
```

The main program `naive_view` is located in `src` subdirectory.