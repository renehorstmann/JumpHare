#ifndef JUMPHARE_TILEMAP_H
#define JUMPHARE_TILEMAP_H

#include "mathc/types/float.h"
#include "color.h"

void tilemap_init(const char *file);

void tilemap_kill();

void tilemap_update(float dtime);

void tilemap_render_back();

void tilemap_render_front();

// searches for positions of the tile code in the given layer
// positions returned are in the axis aligned format (top left)
int tilemap_get_positions_aa(vec2 *out_positions, int max_positions, Color_s code, int layer);

// searches for positions of the tile code in the given layer
// positions returned are in the tile center
int tilemap_get_positions(vec2 *out_positions, int max_positions, Color_s code, int layer);

float tilemap_width();

float tilemap_height();

// scans all tile pixels until collision
float tilemap_ground(float x, float y, Color_s *opt_id);

// scans all tile pixels until collision
float tilemap_ceiling(float x, float y, Color_s *opt_id);

// scans all tile pixels until collision
float tilemap_wall_left(float x, float y, Color_s *opt_id);

// scans all tile pixels until collision
float tilemap_wall_right(float x, float y, Color_s *opt_id);

// returns the pixel color of the back map
Color_s tilemap_pixel_back(int layer, float x, float y);

// returns the pixel color of the main map
Color_s tilemap_pixel_main(int layer, float x, float y);


static float tilemap_border_left() {
    return 0;   
}
static float tilemap_border_bottom() {
    return 0;
}
static float tilemap_border_right() {
    return tilemap_width();
}
static float tilemap_border_top() {
    return tilemap_height();
}


#endif //JUMPHARE_TILEMAP_H
