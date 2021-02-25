#ifndef JUMPHARE_TILEMAP_H
#define JUMPHARE_TILEMAP_H

void tilemap_init();

void tilemap_update(float dtime);

void tilemap_render_back();
void tilemap_render_front();

void tilemap_load_level(const char *file);

float tilemap_left();
float tilemap_right();
float tilemap_top();
float tilemap_bottom();

float tilemap_ground(float x, float y);
float tilemap_ceiling(float x, float y);
float tilemap_wall_left(float x, float y);
float tilemap_wall_right(float x, float y);

#endif //JUMPHARE_TILEMAP_H
