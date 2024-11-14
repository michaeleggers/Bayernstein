# Requirements Windows

- Visual Studio 2022
- CMake
- OpenGL 4.6

# Requirements Linux

- Clang(++)
- Cmake
- make
- libXext-devel
- mesa-libGL-devel
- TODO: Complete this list
- OpenGL 4.6

# MacOS

No support for MacOS. OpenGL only supported up to version 4.1.
And this version did not run stable.
Native Metal backend would be nice in the future.

# Using CMake to generate Makefiles / Visual Studio Project
- Create a ```build``` folder inside the root folder of this repo and go inside it.
## Linux
- Run
```
cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
```
This generates a Makefile on UNIX systems by default. 
Or just open the project in CLion (without creating the ```build``` folder)
and CLion will take care of it.
# Windows
Visual Studio Solution is created by default if installed. Use ```cmake -G``` to see what generator is used.
