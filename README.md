# EiCP
XPBD implementation for cloth simulations

## Installation
Running this software requires OpenGL 4.6 support.

### Windows
As long as `XPBDCloth.exe` and `glfw3.dll` in the build directory are in the same
directory you can start the application without further modifications.

## Development setup
All .obj files you load in must specify their face vertices in counterclockwise order.

### Windows
The `CMakeLists.txt` assumes you have installed MingW / [MSYS2](https://www.msys2.org/#installation)
with the standard gcc toolchain, as well as glfw 3.4, cmake and ninja.
You will further need to ensure that the used cmake executable is the MSYS2 one. 
The setup is currently only tested in [VSCode](https://code.visualstudio.com/).
Make sure you have a C++ toolchain configured.

This project uses C++20 and CMake >= 3.12.

### Errors
The glfw3.dll can sometimes not be found even when you have installed it.
In this case copy the dll into the build folder. The default location in MSys2 is `C:\msys64\mingw64\bin`.
