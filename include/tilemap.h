#ifndef JUMPHARE_TILEMAP_H
#define JUMPHARE_TILEMAP_H

#include "r/ro_types.h"
#include "u/color.h"
#include "tiles.h"

typedef struct {
    const Tiles *tiles_ref;
    struct {
        RoBatch ro_back[MAX_TILES];
        RoBatch ro_main[MAX_TILES];
    //    RoBatch ro_front[MAX_TILES];
        bool ro_back_active[MAX_TILES];
        bool ro_main_active[MAX_TILES];
        uImage map;
    } L;
} Tilemap;

Tilemap *tilemap_new(const Tiles *tiles, const char *file);

void tilemap_kill(Tilemap **self_ptr);

void tilemap_update(Tilemap *self, float dtime);

void tilemap_render_back(Tilemap *self, const mat4 *cam_mat);

void tilemap_render_front(Tilemap *self, const mat4 *cam_mat);

// searches for positions of the tile code in the given layer
// positions returned are in the axis aligned format (top left)
int tilemap_get_positions_aa(const Tilemap *self, vec2 *out_positions, int max_positions, uColor_s code, int layer);

// searches for positions of the tile code in the given layer
// positions returned are in the tile center
int tilemap_get_positions(const Tilemap *self, vec2 *out_positions, int max_positions, uColor_s code, int layer);

float tilemap_width(const Tilemap *self);

float tilemap_height(const Tilemap *self);

// scans all tile pixels until collision
float tilemap_ground(const Tilemap *self, float x, float y, uColor_s *opt_id);

// scans all tile pixels until collision
float tilemap_ceiling(const Tilemap *self,  float x, float y, uColor_s *opt_id);

// scans all tile pixels until collision
float tilemap_wall_left(const Tilemap *self,  float x, float y, uColor_s *opt_id);

// scans all tile pixels until collision
float tilemap_wall_right(const Tilemap *self, float x, float y, uColor_s *opt_id);

// returns the pixel color of the back map
uColor_s tilemap_pixel_back(const Tilemap *self, int layer, float x, float y);

// returns the pixel color of the main map
uColor_s tilemap_pixel_main(const Tilemap *self, int layer, float x, float y);


static float tilemap_border_left(const Tilemap *self) {
    return 0;   
}
static float tilemap_border_bottom(const Tilemap *self) {
    return 0;
}
static float tilemap_border_right(const Tilemap *self) {
    return tilemap_width(self);
}
static float tilemap_border_top(const Tilemap *self) {
    return tilemap_height(self);
}


#endif //JUMPHARE_TILEMAP_H
