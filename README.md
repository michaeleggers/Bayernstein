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

# Some notes and thoughts on code-conventions
Much of this code was written over time and sometimes was taken
from previous projects. My taste of code style has changed quite
often over this period. That is reflected in the code (sorry).
However, I plan to refactor the code to adhere those conventions:

- Classnames are in Upper-case prepended with a 'C', like ```CWorld```.
- Interfaces are in Upper-case prepended with a 'I', like ```IRenderer```.
- Function/Method names are in Pascal style, like '```MyFunction```'.
- Class member values are prepended with a 'm_', like '```m_Gravity```'.
- Struct member values are in camelCase, like '```normalVector```'.
- Getters/Setters are only needed if some logic has to be performed
to get/set a value of an instance. 
- if-blocks always use ```{}```, like:
```
if ( didCollide ) {
    explode();
}   
```
even if it is just a single expression following the if. But sometimes
I don't follow this rule.
- File Global variables are prepended with '```g_```' and declared as ```static```.
- Compilation Unit Global variables are prepended with '```g_```' (and referenced with explicit
```extern``` in other files.
- Initialize objects with ```{}```, when it makes sense. 0-initialization also shows intent and
most of the time this is what one wants. But sometimes it also makes sense to just reserve the
memory and not init it with anything!
- Filenames are lowercase but I have not followed this rule strictly. I am open
to suggestions. However, I found it useful to prepend the file with a letter 
(or more) to indicate what part of the codebase it belongs to, like ```r_gl.cpp```
which is the Render implementation for the OpenGL backend. I prefer this to
having a lot of folders as this reduces search-fatigue. Especially for people
who navigate their files by searching them (in vim) we can just type ```r_*```
to see all the files that belong to the renderer. I would like to keep it this way.
What we can do is to divide the engine code and the game code into folders later,
but for now just keep it loose to not restrict ourselves by imposing arbitrary
borders in the codebase.

This is all. I am not particularly picky about code style. I also
used Unix ```snail_case_in_the_past```. Mostly it is decided in the morning
what style I am going to use. I know this is not ideal...

If you don't like them, I am okay with it! I am also open to discuss
some other code-styles. But we should agree on at least some small
principles.

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
