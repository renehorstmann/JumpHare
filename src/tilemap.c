#include <assert.h>
#include "r/ro_batch.h"
#include "u/pose.h"
#include "mathc/sca/int.h"
#include "mathc/sca/float.h"
#include "utilc/assume.h"

#include "tiles.h"
#include "io.h"
#include "camera.h"
#include "tilemap.h"


#define MAP_LAYERS 3

static struct {
    RoBatch ro_back[MAX_TILES];
    RoBatch ro_main[MAX_TILES];
//    RoBatch ro_front[MAX_TILES];
    bool ro_back_active[MAX_TILES];
    bool ro_main_active[MAX_TILES];
    Image *map;
} L;


// coord to tile grid index
static int tile_c(float x) {
    return x / TILES_SIZE;
}

// coord to tile grid index
static int tile_r(float y) {
    return L.map->rows - y / TILES_SIZE;
}

// coord to tile pixel index within a tile
static int tile_pixel_c(float x) {
    return (int) x % TILES_SIZE;
}

// coord to tile pixel index within a tile
static int tile_pixel_r(float y) {
    return (int) (L.map->rows * TILES_SIZE - y) % TILES_SIZE;
}

// tile grid index to coord (left)
static float tile_x(int c) {
    return c * TILES_SIZE;
}

// tile grid index to coord (top)
static float tile_y(int r) {
    return (L.map->rows - r) * TILES_SIZE;
}

// tile grid index to coord pose
static mat4 tile_pose(int c, int r) {
    return u_pose_new_aa(tile_x(c), tile_y(r), TILES_SIZE, TILES_SIZE);
}

// tile id to uv map
static mat4 tile_uv(int id) {
    float w = 1.0 / TILES_COLS;
    float h = 1.0 / TILES_ROWS;
    float u = w * (id % TILES_COLS);
    float v = h * (id / TILES_COLS);
    return u_pose_new(u, v, w, h);
}

// returns true if a tile pixel is not transparent in the collision layer
static bool pixel_collision(Color_s code, int pixel_c, int pixel_r) {
    return !color_equals(tiles_pixel(code, 1, pixel_c, pixel_r),
                         COLOR_TRANSPARENT);
}

// returns the color of a tile pixel
static Color_s pixel_color(int map_layer, int tile_layer, float x, float y) {
    assert(tile_layer >= 0 && tile_layer <= 1
            && map_layer >= 0 && map_layer <= 2);

    int c = tile_c(x);
    int r = tile_r(y);

    if (c < 0 || c >= L.map->cols
        || r < 0 || r >= L.map->rows) {
        return COLOR_TRANSPARENT;
    }

    Color_s tile = *image_pixel(L.map, map_layer, c, r);
    int pc = tile_pixel_c(x);
    int pr = tile_pixel_r(y);

    return tiles_pixel(tile, tile_layer, pc, pr);
}


void tilemap_init(const char *file) {
    L.map = io_load_image(file, MAP_LAYERS);
    
    int tile_back_nums[MAX_TILES] = {0};
    int tile_main_nums[MAX_TILES] = {0};
    for (int r = 0; r < L.map->rows; r++) {
        for (int c = 0; c < L.map->cols; c++) {
            Color_s code;
            code = *image_pixel(L.map, 0, c, r);
            if (!color_equals(code, COLOR_TRANSPARENT)) {
                tile_back_nums[code.b - 1]++;
            }
            
            code = *image_pixel(L.map, 1, c, r);
            if (!color_equals(code, COLOR_TRANSPARENT)) {
                tile_main_nums[code.b - 1]++;
            }
        }
    }
    
    for (int i = 0; i < tiles.size; i++) {
        L.ro_back_active[i] = tile_back_nums[i] > 0;
        if(L.ro_back_active[i]) {
            ro_batch_init(&L.ro_back[i], tile_back_nums[i], camera.gl_main, tiles.textures[i]);
            L.ro_back[i].owns_tex = false;
        }
        
        L.ro_main_active[i] = tile_main_nums[i] > 0;
        if(L.ro_main_active[i]) {
            ro_batch_init(&L.ro_main[i], tile_main_nums[i], camera.gl_main, tiles.textures[i]);
            L.ro_main[i].owns_tex = false;
        }
    }

    
    memset(tile_back_nums, 0, sizeof(tile_back_nums));
    memset(tile_main_nums, 0, sizeof(tile_main_nums));

    for (int r = 0; r < L.map->rows; r++) {
        for (int c = 0; c < L.map->cols; c++) {
            Color_s code;
            code = *image_pixel(L.map, 0, c, r);
            if (!color_equals(code, COLOR_TRANSPARENT)) {
                int tile_id = code.b - 1;
                int tile = code.a;
                rRect_s *rect = &L.ro_back[tile_id].rects[tile_back_nums[tile_id]];
                rect->pose = tile_pose(c, r);
                rect->uv = tile_uv(tile);
                tile_back_nums[tile_id]++;
            }
            
            code = *image_pixel(L.map, 1, c, r);
            if (!color_equals(code, COLOR_TRANSPARENT)) {
                int tile_id = code.b - 1;
                int tile = code.a;
                rRect_s *rect = &L.ro_main[tile_id].rects[tile_main_nums[tile_id]];
                rect->pose = tile_pose(c, r);
                rect->uv = tile_uv(tile);
                tile_main_nums[tile_id]++;
            }
        }
    }
    

    for (int i = 0; i < tiles.size; i++) {
        if(L.ro_back_active[i])
            ro_batch_update(&L.ro_back[i]);
        if(L.ro_main_active[i])
            ro_batch_update(&L.ro_main[i]);
//        ro_batch_update(&L.ro_front[i]);
    }
}

void tilemap_kill() {
    image_delete(L.map);
    for (int i = 0; i < tiles.size; i++) {
        if(L.ro_back_active[i])
            ro_batch_kill(&L.ro_back[i]);
        if(L.ro_main_active[i])
            ro_batch_kill(&L.ro_main[i]);
//        ro_batch_kill(&L.ro_front[i]);
    }
    memset(&L, 0, sizeof(L));
}

void tilemap_update(float dtime) {

}

void tilemap_render_back() {
    for (int i = 0; i < tiles.size; i++) {
        if(L.ro_back_active[i])
            ro_batch_render(&L.ro_back[i]);
    }
    for (int i = 0; i < tiles.size; i++) {
        if(L.ro_main_active[i])
            ro_batch_render(&L.ro_main[i]);
    }
}

void tilemap_render_front() {
    for (int i = 0; i < tiles.size; i++) {
//        ro_batch_render(&L.ro_front[i]);
    }
}

int tilemap_get_positions_aa(vec2 *out_positions, int max_positions, Color_s code, int layer) {
    assume(layer <= L.map->layers, "invalid layer");
    
    int idx = 0;
    for(int r=0; r<L.map->rows; r++) {
        for(int c=0; c<L.map->cols; c++) {
            if(color_equals(code, *image_pixel(L.map, layer, c, r))) {
                out_positions[idx].x = tile_x(c);
                out_positions[idx].y = tile_y(r);
                if(++idx>=max_positions)
                    return idx;
            }
        }
    }
    return idx;
}

int tilemap_get_positions(vec2 *out_positions, int max_positions, Color_s code, int layer) {
    int cnt = tilemap_get_positions_aa(out_positions, max_positions, code, layer);
    
    for(int i=0; i<cnt; i++) {
        out_positions[i].x += TILES_SIZE/2.0;
        out_positions[i].y -= TILES_SIZE/2.0;
    }
    return cnt;
}

float tilemap_width() {
    return L.map->cols * TILES_SIZE;
}

float tilemap_height() {
    return L.map->rows * TILES_SIZE;
}

float tilemap_ground(float x, float y, Color_s *opt_id) {
    // first tile to check
    int c = tile_c(x);
    int r = tile_r(y);

    // first pixel row in the first tile
    int pr_init = tile_pixel_r(y);

    // test all tiles and pixels of a tile, until a collision is found
    while (c >= 0 && c < L.map->cols
           && r >= 0 && r < L.map->rows) {
        Color_s tile = *image_pixel(L.map, 1, c, r);
        if (!color_equals(tile, COLOR_TRANSPARENT)) {
            int pc = tile_pixel_c(x);
            for (int pr = pr_init; pr < TILES_SIZE; pr++) {
                Color_s id = tiles_pixel(tile, 1, pc, pr);
                if (!color_equals(id, COLOR_TRANSPARENT)) {
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
        *opt_id = COLOR_TRANSPARENT;
    return -FLT_MAX;
}

float tilemap_ceiling(float x, float y, Color_s *opt_id) {
    int c = tile_c(x);
    int r = tile_r(y);
    int pr_init = tile_pixel_r(y);
    while (c >= 0 && c < L.map->cols
           && r >= 0 && r < L.map->rows) {
        Color_s tile = *image_pixel(L.map, 1, c, r);
        if (!color_equals(tile, COLOR_TRANSPARENT)) {
            int pc = tile_pixel_c(x);
            for (int pr = pr_init; pr >= 0; pr--) {
                Color_s id = tiles_pixel(tile, 1, pc, pr);
                if (!color_equals(id, COLOR_TRANSPARENT)) {
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
        *opt_id = COLOR_TRANSPARENT;
    return FLT_MAX;
}

float tilemap_wall_left(float x, float y, Color_s *opt_id) {
    int c = tile_c(x);
    int r = tile_r(y);
    int pc_init = tile_pixel_c(x);
    while (c >= 0 && c < L.map->cols
           && r >= 0 && r < L.map->rows) {
        Color_s tile = *image_pixel(L.map, 1, c, r);
        if (!color_equals(tile, COLOR_TRANSPARENT)) {
            int pr = tile_pixel_r(y);
            for (int pc = pc_init; pc >= 0; pc--) {
                Color_s id = tiles_pixel(tile, 1, pc, pr);
                if (!color_equals(id, COLOR_TRANSPARENT)) {
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
        *opt_id = COLOR_TRANSPARENT;
    return tile_x(0);
}

float tilemap_wall_right(float x, float y, Color_s *opt_id) {
    int c = tile_c(x);
    int r = tile_r(y);
    int pc_init = tile_pixel_c(x);
    while (c >= 0 && c < L.map->cols
           && r >= 0 && r < L.map->rows) {
        Color_s tile = *image_pixel(L.map, 1, c, r);
        if (!color_equals(tile, COLOR_TRANSPARENT)) {
            int pr = tile_pixel_r(y);
            for (int pc = pc_init; pc < TILES_SIZE; pc++) {
                Color_s id = tiles_pixel(tile, 1, pc, pr);
                if (!color_equals(id, COLOR_TRANSPARENT)) {
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
        *opt_id = COLOR_TRANSPARENT;
    return tile_x(L.map->cols);
}

Color_s tilemap_pixel_back(int layer, float x, float y) {
    return pixel_color(0, layer, x, y);
}

Color_s tilemap_pixel_main(int layer, float x, float y) {
    return pixel_color(1, layer, x, y);
}
