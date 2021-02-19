#ifndef JUMPHARE_TILEMAP_H
#define JUMPHARE_TILEMAP_H

#include "tile.h"


float tilemap_ground(float x, float y, float w);
float tilemap_ceiling(float x, float y, float w);
float tilemap_wall_left(float x, float y, float h);
float tilemap_wall_right(float x, float y, float h);


#endif //JUMPHARE_TILEMAP_H
