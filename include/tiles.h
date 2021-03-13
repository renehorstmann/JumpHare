#ifndef JUMPHARE_TILES_H
#define JUMPHARE_TILES_H

#include "r/core.h"
#include "image.h"

#define MAX_TILES 128
#define TILES_COLS 8
#define TILES_ROWS 8
#define TILES_SIZE 16

enum tiles_pixel_state {
    TILES_PIXEL_EMPTY,
    TILES_PIXEL_SOLID_DIRTY,
    TILES_PIXEL_SOLID_CLEAN,
    TILES_PIXEL_KILL,
    TILES_PIXEL_NUM_STATES
};

struct TilesGlobals_s {
    Image *imgs[MAX_TILES];
    GLuint textures[MAX_TILES];
    int ids[MAX_TILES];
    int size;
};
extern struct TilesGlobals_s tiles;

void tiles_init();

Color_s tiles_pixel(Color_s code, int layer, int pixel_c, int pixel_r);

enum tiles_pixel_state tiles_get_state(Color_s id);

#endif //JUMPHARE_TILES_H
