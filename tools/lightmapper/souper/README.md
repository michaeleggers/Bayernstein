# Souper docs

## How to build with CMake

1.) Download SDL2 development libraries from:
https://github.com/libsdl-org/SDL/releases/tag/release-2.30.8

and extract it somehere you find it (not inside
this repo because then git might want to track it).

2.) Make a `build` directory next to this `CMakeLists`
and the sourcefiles and `cd` into it.

3.) Generate with CMake:
```bash
cmake -DSDL_LIBRARIES_DIR="<relative path to the sdl2 libs>" ..
```
For example, if you downloaded the SDL2 libs next to the `CMakeLists` file and extracted
its contents to `sdl`, then, on Windows you would do:
```bash
cmake -DSDL_LIBRARIES_DIR="sdl/lib/x64" ..
```

4.) A `bin` directory is being built with the
`souper` exe inside it.

5.) Run the program:
```bash
souper <path to the assets dir> maps/<.MAP-file> <output.plys-file>
```



