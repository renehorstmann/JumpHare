# JumpHare
An SDL2 OpenGL game written in C, also working on Web and Android.
Based on [some](https://github.com/renehorstmann/some) framework.

## Warning
In active developmemt

## Todo
- mathc sca functions
  - sca_*, dsca_*, min, max, clamp, radians, ...
- hare set speed takes [-1 : 1]
- hare pause speed smoothly
- draw high speed
- draw jump
- hare jump function

## Compiling on Windows
Compiling with Mingw (msys2).
Currently not working with cmake, but with the following gcc call.
I had to put all source files into one dir (from src/e/*, r/*, p/*, u/* into src/*) to get the linker happy.
```
gcc -o some src/* -Iinclude $(sdl2-config --cflags --libs) -lSDL2_image -lSDL2_ttf -lglew32 -lopengl32 -lglu32 -DUSING_GLEW
```

## Author
René Horstmann

## Licence
The game ane its assets are licenced under GPLv3, see LICENCE.
The [some](https://github.com/renehorstmann/some) framework is under MIT licence, see someLICENCE.
