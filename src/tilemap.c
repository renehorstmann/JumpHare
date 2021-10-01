#include <assert.h>
#include "r/ro_batch.h"
#include "u/pose.h"
#include "u/image.h"
#include "mathc/sca/int.h"
#include "mathc/sca/float.h"
#include "rhc/error.h"

#include "self->tiles_ref->h"
#include "camera.h"
#include "tilemap.h"


#define MAP_LAYERS 3


//
// private
//


// coord to tile grid index
static int tile_c(const Tilemap *self, float x) {
    return x / TILES_SIZE;
}

// coord to tile grid index
static int tile_r(const Tilemap *self, float y) {
    return self->L.map.rows - y / TILES_SIZE;
}

// coord to tile pixel index within a tile
static int tile_pixel_c(const Tilemap *self, float x) {
    return (int) x % TILES_SIZE;
}

// coord to tile pixel index within a tile
static int tile_pixel_r(const Tilemap *self, float y) {
    return (int) (self->L.map.rows * TILES_SIZE - y) % TILES_SIZE;
}

// tile grid index to coord (left)
static float tile_x(const Tilemap *self, int c) {
    return c * TILES_SIZE;
}

// tile grid index to coord (top)
static float tile_y(const Tilemap *self, int r) {
    return (self->L.map.rows - r) * TILES_SIZE;
}

// tile grid index to coord pose
static mat4 tile_pose(const Tilemap *self, int c, int r) {
    return u_pose_new_aa(tile_x(self, c), tile_y(self, r), TILES_SIZE, TILES_SIZE);
}

// tile id to sprite map
static vec2 tile_sprite(int id) {
    float x = id % TILES_COLS;
    float y = id / TILES_COLS;
    return (vec2) {{x, y}};
}

// returns true if a tile pixel is not transparent in the collision layer
static bool pixel_collision(const Tilemap *self, uColor_s code, int pixel_c, int pixel_r) {
    return !u_color_equals(tiles_pixel(self->tiles_ref, code, pixel_c, pixel_r, 1),
                           U_COLOR_TRANSPARENT);
}

// returns the color of a tile pixel
static uColor_s pixel_color(const Tilemap *self, int map_layer, int tile_layer, float x, float y) {
    assert(tile_layer >= 0 && tile_layer <= 1
            && map_layer >= 0 && map_layer <= 2);

    int c = tile_c(self, x);
    int r = tile_r(self, y);

    if (c < 0 || c >= self->L.map.cols
        || r < 0 || r >= self->L.map.rows) {
        return U_COLOR_TRANSPARENT;
    }

    uColor_s tile = *u_image_pixel(self->L.map, c, r, map_layer);
    int pc = tile_pixel_c(self, x);
    int pr = tile_pixel_r(self, y);

    return tiles_pixel(self->tiles_ref, tile, pc, pr, tile_layer);
}



//
// public
//

Tilemap *tilemap_new(const Tiles *tiles, const char *file) {
    Tilemap *self = rhc_calloc(sizeof *self);
    
    self->tiles_ref = tiles;
    
    self->L.map = u_image_new_file(MAP_LAYERS, file);
    
    int tile_back_nums[MAX_TILES] = {0};
    int tile_main_nums[MAX_TILES] = {0};
    for (int r = 0; r < self->L.map.rows; r++) {
        for (int c = 0; c < self->L.map.cols; c++) {
            uColor_s code;
            code = *u_image_pixel(self->L.map, c, r, 0);
            if (!u_color_equals(code, U_COLOR_TRANSPARENT)) {
                tile_back_nums[code.b - 1]++;
            }
            
            code = *u_image_pixel(self->L.map, c, r, 1);
            if (!u_color_equals(code, U_COLOR_TRANSPARENT)) {
                tile_main_nums[code.b - 1]++;
            }
        }
    }
    
    for (int i = 0; i < tiles->size; i++) {
        self->L.ro_back_active[i] = tile_back_nums[i] > 0;
        if(self->L.ro_back_active[i]) {
            self->L.ro_back[i] = ro_batch_new(tile_back_nums[i], tiles->textures[i]);
            self->L.ro_back[i].owns_tex = false;
        }
        
        self->L.ro_main_active[i] = tile_main_nums[i] > 0;
        if(self->L.ro_main_active[i]) {
            self->L.ro_main[i] = ro_batch_new(tile_main_nums[i], tiles->textures[i]);
            self->L.ro_main[i].owns_tex = false;
        }
    }

    
    memset(tile_back_nums, 0, sizeof(tile_back_nums));
    memset(tile_main_nums, 0, sizeof(tile_main_nums));

    for (int r = 0; r < self->L.map.rows; r++) {
        for (int c = 0; c < self->L.map.cols; c++) {
            uColor_s code;
            code = *u_image_pixel(self->L.map, c, r, 0);
            if (!u_color_equals(code, U_COLOR_TRANSPARENT)) {
                int tile_id = code.b - 1;
                int tile = code.a;
                rRect_s *rect = &self->L.ro_back[tile_id].rects[tile_back_nums[tile_id]];
                rect->pose = tile_pose(self, c, r);
                rect->sprite = tile_sprite(tile);
                tile_back_nums[tile_id]++;
            }
            
            code = *u_image_pixel(self->L.map, c, r, 1);
            if (!u_color_equals(code, U_COLOR_TRANSPARENT)) {
                int tile_id = code.b - 1;
                int tile = code.a;
                rRect_s *rect = &self->L.ro_main[tile_id].rects[tile_main_nums[tile_id]];
                rect->pose = tile_pose(self, c, r);
                rect->sprite = tile_sprite(tile);
                tile_main_nums[tile_id]++;
            }
        }
    }
    

    for (int i = 0; i < tiles->size; i++) {
        if(self->L.ro_back_active[i])
            ro_batch_update(&self->L.ro_back[i]);
        if(self->L.ro_main_active[i])
            ro_batch_update(&self->L.ro_main[i]);
//        ro_batch_update(&self->L.ro_front[i]);
    }
    
    return self;
}

void tilemap_kill(Tilemap **self_ptr) {
    Tilemap *self = *self_ptr;
    if(!self)
        return;
        
    u_image_kill(&self->L.map);
    for (int i = 0; i < self->tiles_ref->size; i++) {
        if(self->L.ro_back_active[i])
            ro_batch_kill(&self->L.ro_back[i]);
        if(self->L.ro_main_active[i])
            ro_batch_kill(&self->L.ro_main[i]);
//        ro_batch_kill(&self->L.ro_front[i]);
    }
    
    rhc_free(self);
    *self_ptr = NULL;
}

void tilemap_update(Tilemap *self, float dtime) {

}

void tilemap_render_back(Tilemap *self, const mat4 *cam_mat) {
    for (int i = 0; i < self->tiles_ref->size; i++) {
        if(self->L.ro_back_active[i])
            ro_batch_render(&self->L.ro_back[i], cam_mat);
    }
    for (int i = 0; i < self->tiles_ref->size; i++) {
        if(self->L.ro_main_active[i])
            ro_batch_render(&self->L.ro_main[i], cam_mat);
    }
}

void tilemap_render_front() {
    for (int i = 0; i < self->tiles_ref->size; i++) {
//        ro_batch_render(&self->L.ro_front[i]);
    }
}

int tilemap_get_positions_aa(const Tilemap *self, vec2 *out_positions, int max_positions, uColor_s code, int layer) {
    assume(layer <= self->L.map.layers, "invalid layer");
    
    int idx = 0;
    for(int r=0; r<self->L.map.rows; r++) {
        for(int c=0; c<self->L.map.cols; c++) {
            if(u_color_equals(code, *u_image_pixel(self->L.map, c, r, layer))) {
                out_positions[idx].x = tile_x(c);
                out_positions[idx].y = tile_y(r);
                if(++idx>=max_positions)
                    return idx;
            }
        }
    }
    return idx;
}

int tilemap_get_positions(const Tilemap *self, vec2 *out_positions, int max_positions, uColor_s code, int layer) {
    int cnt = tilemap_get_positions_aa(out_positions, max_positions, code, layer);
    
    for(int i=0; i<cnt; i++) {
        out_positions[i].x += TILES_SIZE/2.0;
        out_positions[i].y -= TILES_SIZE/2.0;
    }
    return cnt;
}

float tilemap_width(const Tilemap *self) {
    return self->L.map.cols * TILES_SIZE;
}

float tilemap_height(const Tilemap *self) {
    return self->L.map.rows * TILES_SIZE;
}

float tilemap_ground(const Tilemap *self, float x, float y, uColor_s *opt_id) {
    // first tile to check
    int c = tile_c(self, x);
    int r = tile_r(self, y);

    // first pixel row in the first tile
    int pr_init = tile_pixel_r(self, y);

    // test all tiles and pixels of a tile, until a collision is found
    while (c >= 0 && c < self->L.map.cols
           && r >= 0 && r < self->L.map.rows) {
        uColor_s tile = *u_image_pixel(self->L.map, c, r, 1);
        if (!u_color_equals(tile, U_COLOR_TRANSPARENT)) {
            int pc = tile_pixel_c(self, x);
            for (int pr = pr_init; pr < TILES_SIZE; pr++) {
                uColor_s id = tiles_pixel(tile, pc, pr, 1);
                if (!u_color_equals(id, U_COLOR_TRANSPARENT)) {
                    if (opt_id)
                        *opt_id = id;
                    return sca_min(tile_y(r) - pr, y);
                }
            }
        }

        // next row, test all pixels
        r++;
        pr_init = 0;
    }
    if (opt_id)
        *opt_id = U_COLOR_TRANSPARENT;
    return -FLT_MAX;
}

float tilemap_ceiling(const Tilemap *self, (lfloat x, float y, uColor_s *opt_id) {
    int c = tile_c(self, x);
    int r = tile_r(self, y);
    int pr_init = tile_pixel_r(self, y);
    while (c >= 0 && c < self->L.map.cols
           && r >= 0 && r < self->L.map.rows) {
        uColor_s tile = *u_image_pixel(self->L.map, c, r, 1);
        if (!u_color_equals(tile, U_COLOR_TRANSPARENT)) {
            int pc = tile_pixel_c(self, x);
            for (int pr = pr_init; pr >= 0; pr--) {
                uColor_s id = tiles_pixel(tile, pc, pr, 1);
                if (!u_color_equals(id, U_COLOR_TRANSPARENT)) {
                    if (opt_id)
                        *opt_id = id;
                    return sca_max(tile_y(r) - pr - 1, y);
                }
            }
        }
        r--;
        pr_init = TILES_SIZE - 1;
    }
    if (opt_id)
        *opt_id = U_COLOR_TRANSPARENT;
    return FLT_MAX;
}

float tilemap_wall_left(const Tilemap *self, float x, float y, uColor_s *opt_id) {
    int c = tile_c(self, x);
    int r = tile_r(self, y);
    int pc_init = tile_pixel_c(self, x);
    while (c >= 0 && c < self->L.map.cols
           && r >= 0 && r < self->L.map.rows) {
        uColor_s tile = *u_image_pixel(self->L.map, c, r, 1);
        if (!u_color_equals(tile, U_COLOR_TRANSPARENT)) {
            int pr = tile_pixel_r(self, y);
            for (int pc = pc_init; pc >= 0; pc--) {
                uColor_s id = tiles_pixel(tile, pc, pr, 1);
                if (!u_color_equals(id, U_COLOR_TRANSPARENT)) {
                    if (opt_id)
                        *opt_id = id;
                    return sca_min(tile_x(c) + pc + 1, x);
                }
            }
        }
        c--;
        pc_init = TILES_SIZE-1;
    }
    if (opt_id)
        *opt_id = U_COLOR_TRANSPARENT;
    return tile_x(0);
}

float tilemap_wall_right(const Tilemap *self, float x, float y, uColor_s *opt_id) {
    int c = tile_c(self, x);
    int r = tile_r(self, y);
    int pc_init = tile_pixel_c(self, x);
    while (c >= 0 && c < self->L.map.cols
           && r >= 0 && r < self->L.map.rows) {
        uColor_s tile = *u_image_pixel(self->L.map, c, r, 1);
        if (!u_color_equals(tile, U_COLOR_TRANSPARENT)) {
            int pr = tile_pixel_r(self, y);
            for (int pc = pc_init; pc < TILES_SIZE; pc++) {
                uColor_s id = tiles_pixel(tile, pc, pr, 1);
                if (!u_color_equals(id, U_COLOR_TRANSPARENT)) {
                    if (opt_id)
                        *opt_id = id;
                    return sca_max(tile_x(c) + pc, x);
                }
            }
        }
        c++;
        pc_init = 0;
    }
    if (opt_id)
        *opt_id = U_COLOR_TRANSPARENT;
    return tile_x(self->L.map.cols);
}

uColor_s tilemap_pixel_back(const Tilemap *self, int layer, float x, float y) {
    return pixel_color(self, 0, layer, x, y);
}

uColor_s tilemap_pixel_main(const Tilemap *self, int layer, float x, float y) {
    return pixel_color(self, 1, layer, x, y);
}
