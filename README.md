Office
===
Exploratory implementation of fake ascii rendering and gameplay

# Compiling
Fetch submodules first.
```bash
git submodule init
git submodule update
```

## Compiling on linux
> Requires clang

First build the glfw library.
```bash
cd deps/glfw
mkdir build
cd build
cmake ..
make
```
After this a, make -f Makefile.lin build will work.

## Compiling on windows
> Requires VS Sdk or Visual Studio 14+

First build the glfw library.
```bash
cd deps\glfw
mkdir build
cd build
cmake -DBUILD_SHARED_LIBS=OFF -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF ..
nmake
```
After this a simple nmake build will work.
