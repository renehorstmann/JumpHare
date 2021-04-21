#ifndef JUMPHARE_TILES_H
#define JUMPHARE_TILES_H

#include "r/core.h"
#include "r/texture.h"
#include "u/image.h"

#define MAX_TILES 128
#define TILES_COLS 8
#define TILES_ROWS 8
#define TILES_SIZE 16

enum tiles_pixel_state {
    TILES_PIXEL_EMPTY,
    TILES_PIXEL_SOLID_DIRTY,
    TILES_PIXEL_SOLID_CLEAN,
    TILES_PIXEL_ONEWAY_UP,
    TILES_PIXEL_KILL,
    TILES_PIXEL_NUM_STATES
};

struct TilesGlobals_s {
    uImage *imgs[MAX_TILES];
    rTexture textures[MAX_TILES];
    int ids[MAX_TILES];
    int size;
};
extern struct TilesGlobals_s tiles;

void tiles_init();

uColor_s tiles_pixel(uColor_s code, int pixel_c, int pixel_r, int layer);

enum tiles_pixel_state tiles_get_state(uColor_s id);

#endif //JUMPHARE_TILES_H
