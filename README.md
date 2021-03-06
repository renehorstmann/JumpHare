# JumpHare
An SDL2 OpenGL game written in C, also working on Web and Android.
Based on [some](https://github.com/renehorstmann/some) framework.

## Art
Pixelart was made with my [Pixelc](https://github.com/renehorstmann/pixelc) App.
Tile maps are designed with [Tilec](https://github.com/renehorstmann/tilec).
Both Apps are also based on [some](https://github.com/renehorstmann/some) framework.


## Warning
In active developmemt

## Status
![example](example.jpg)

## Todo
- draw foreground
- update fixed 200 hz
  - ```c
    u_time += delta_time;
    while(u_time>0) {
      u_time -= 1.0/200.0;
      *_update(1.0/200.0);
      // ...
    }
    ```
    
- hare physics 
  - aabb structs for sizes?
  - state struct
    - grounded, jumping, ...
  - collision for grounded is different from falling
  - state of previos frame
    - for sound effects, etc. 
      !prev.grounded && grounded
  - interpolate positions
  - jump should work if nearly grounded (0.5 seconds or so)
- control optimization
  - visualization

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
