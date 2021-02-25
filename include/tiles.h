#ifndef JUMPHARE_TILES_H
#define JUMPHARE_TILES_H

#include "r/core.h"

#define MAX_TILES 128
#define TILES_COLS 8
#define TILES_ROWS 8
#define TILES_SIZE 16.0f

struct TilesGlobals_s {
    GLuint textures[MAX_TILES];
    int ids[MAX_TILES];
    int size;
};
extern struct TilesGlobals_s tiles;

void tiles_init();


#endif //JUMPHARE_TILES_H
