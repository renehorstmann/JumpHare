#ifndef JUMPHARE_TILEMAP_H
#define JUMPHARE_TILEMAP_H

#include "mathc/types/float.h"
#include "color.h"

void tilemap_init(const char *file);

void tilemap_kill();

void tilemap_update(float dtime);

void tilemap_render_back();

void tilemap_render_front();

int tilemap_get_positions_aa(vec2 *out_positions, int max_positions, Color_s code, int layer);

int tilemap_get_positions(vec2 *out_positions, int max_positions, Color_s code, int layer);

float tilemap_border_left();

float tilemap_border_right();

float tilemap_border_top();

float tilemap_border_bottom();

float tilemap_ground(float x, float y, Color_s *opt_id);

float tilemap_ceiling(float x, float y, Color_s *opt_id);

float tilemap_wall_left(float x, float y, Color_s *opt_id);

float tilemap_wall_right(float x, float y, Color_s *opt_id);

Color_s tilemap_pixel(int layer, float x, float y);

#endif //JUMPHARE_TILEMAP_H
