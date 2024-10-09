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
- OpenGL 4.6

# MacOS

No support for MacOS. OpenGL only supported up to version 4.1.
And this version did not run stable.
Native Metal backend would be nice in the future.

# Some notes and thoughts on code-conventions
Much of this code was written over time and sometimes was taken
from previous projects. My taste of code style has changed quite
often over this period. That is reflected in the code (sorry).
However, I plan to refactor the code to adhere those conventions:

- Classnames are in Upper-case prepended with a 'C', like CWorld.
- Interfaces are in Upper-case prepended with a 'I', like IRenderer.
- Function/Method names are in Pascal style, like 'MyFunction'.
- Class member values are prepended with a 'm_', like 'm_Gravity'.
- Struct member values are in camelCase, like 'normalVector'.
- Getters/Setters are only needed if some logic has to be performed
to get/set a value of an instance. This is the only rule I am a code-Nazi
about. Having setters to just read or assign a value is just... not
smart!
- if-blocks always use {}, like:
```
if ( didCollide ) {
    explode();
}   
```
even if it is just a single expression following the if. But sometimes
I don't follow this rule.

This is all. I am not particularly picky about code style. I also
used Unix snail_case_in_the_past. Mostly it is decided in the morning
what style I am going to use. I know this is not ideal...

If you don't like them, I am okay with it! I am also open to discuss
some other code-styles. But we should agree on at least some small
principles.

# Using CMake to generate Makefiles / Visual Studio Project
- Create a ```build``` folder inside the root folder of this repo and go inside it.
- Run ```cmake ..```. This generates a Makefile on UNIX systems by default. On Windows, usually
a Visual Studio Solution. Use ```cmake -G``` to see what generator is used.
