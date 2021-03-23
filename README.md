# JumpHare
An SDL2 OpenGL game written in C, also working on Web and Android.
Based on [some](https://github.com/renehorstmann/some) framework.

## Art
Pixelart was made with my [Pixelc](https://github.com/renehorstmann/pixelc) App.
Tile maps are designed with [Tilec](https://github.com/renehorstmann/tilec).
Both Apps are also based on [some](https://github.com/renehorstmann/some) framework.

## Live demo
Compiled with Emscripten [livedemo](https://renehorstmann.github.io)

## Warning
In active developmemt

## Status
![example](example.jpg)

## Todo
- check browser size camera position error
- draw foreground
- hare double jump animation
- draw tiles
- non final sound effects?
- enemies
- control optimization
  - visualization


## Compiling for Web
Using Emscripten:
```
emcc -I../include/ -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s FULL_ES3=1 -s SDL2_IMAGE_FORMATS='["png"]' --preload-file ../res -s ALLOW_MEMORY_GROWTH=1 -DUSING_GLES -DUSING_TOUCH ../src/e/*.c ../src/p/*.c ../src/r/*.c ../src/u/*.c ../src/*.c -o index.html
```

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
The game and its assets are licenced under GPLv3, see LICENCE.

The [some](https://github.com/renehorstmann/some) framework is under MIT licence, see someLICENCE.
