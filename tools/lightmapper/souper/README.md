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
cmake -DSDL_LIBRARIES_DIR="<path to the sdl2 libs>" ..
```

4.) A `bin` directory is being built with the
`soup` exe inside it.

