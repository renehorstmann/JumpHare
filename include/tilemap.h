#ifndef JUMPHARE_TILEMAP_H
#define JUMPHARE_TILEMAP_H

#include "color.h"

void tilemap_init(const char *file);

void tilemap_kill();

void tilemap_update(float dtime);

void tilemap_render_back();
void tilemap_render_front();

float tilemap_border_left();
float tilemap_border_right();
float tilemap_border_top();
float tilemap_border_bottom();

float tilemap_ground(float x, float y, Color_s *opt_id);
float tilemap_ceiling(float x, float y, Color_s *opt_id);
float tilemap_wall_left(float x, float y, Color_s *opt_id);
float tilemap_wall_right(float x, float y, Color_s *opt_id);

#endif //JUMPHARE_TILEMAP_H
