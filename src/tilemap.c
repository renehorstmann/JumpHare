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


#define MAP_LAYERS 2

static struct {
    rRoBatch ro_back[MAX_TILES];
//    rRoBatch ro_front[MAX_TILES];
    Image *map;
} L;

static int tile_c(float x) {
    return x / TILES_SIZE;
}

static int tile_r(float y) {
    return L.map->rows / 2 - y / TILES_SIZE;
}

static int tile_pixel_c(float x) {
    return (int) x % TILES_SIZE;
}

static int tile_pixel_r(float y) {
    return (int) (L.map->rows / 2 * TILES_SIZE - y) % TILES_SIZE;
}

static float tile_x(int c) {
    return c * TILES_SIZE;
}

static float tile_y(int r) {
    return (L.map->rows / 2 - r) * TILES_SIZE;
}

static mat4 tile_pose(int c, int r) {
    return u_pose_new_aa(tile_x(c), tile_y(r), TILES_SIZE, TILES_SIZE);
}

static mat4 tile_uv(int id) {
    float w = 1.0 / TILES_COLS;
    float h = 1.0 / TILES_ROWS;
    float u = w * (id % TILES_COLS);
    float v = h * (id / TILES_COLS);
    return u_pose_new(u, v, w, h);
}


static bool pixel_collision(Color_s code, int pixel_c, int pixel_r) {
    return !color_equals(tiles_pixel(code, 1, pixel_c, pixel_r),
                         COLOR_TRANSPARENT);
}


void tilemap_init(const char *file) {
    L.map = io_load_image(file, MAP_LAYERS);

    int tile_nums[MAX_TILES] = {0};
    for (int r = 0; r < L.map->rows; r++) {
        for (int c = 0; c < L.map->cols; c++) {
            Color_s code;
            code = *image_pixel(L.map, 0, c, r);
            if (color_equals(code, (Color_s) {0})) {
                continue;
            }
            tile_nums[code.b - 1]++;
            // todo more layers...
        }
    }

    for (int i = 0; i < tiles.size; i++) {
        r_ro_batch_init(&L.ro_back[i], tile_nums[i], camera.gl_main, tiles.textures[i]);
        L.ro_back[i].owns_tex = false;
    }

    memset(tile_nums, 0, sizeof(tile_nums));

    for (int r = 0; r < L.map->rows; r++) {
        for (int c = 0; c < L.map->cols; c++) {
            Color_s code = *image_pixel(L.map, 0, c, r);
            if (color_equals(code, COLOR_TRANSPARENT)) {
                continue;
            }
            int tile_id = code.b - 1;
            int tile = code.a;
            rRect_s *rect = &L.ro_back[tile_id].rects[tile_nums[tile_id]];
            rect->pose = tile_pose(c, r);
            rect->uv = tile_uv(tile);
            tile_nums[tile_id]++;
        }
    }

    for (int i = 0; i < tiles.size; i++) {
        r_ro_batch_update(&L.ro_back[i]);
//        r_ro_batch_update(&L.ro_front[i]);
    }
}

void tilemap_kill() {
    image_delete(L.map);
    for (int i = 0; i < tiles.size; i++) {
        r_ro_batch_kill(&L.ro_back[i]);
//        r_ro_batch_kill(&L.ro_front[i]);
    }
    memset(&L, 0, sizeof(L));
}

void tilemap_update(float dtime) {

}

void tilemap_render_back() {
    for (int i = 0; i < tiles.size; i++) {
        r_ro_batch_render(&L.ro_back[i]);
    }
}

void tilemap_render_front() {
    for (int i = 0; i < tiles.size; i++) {
//        r_ro_batch_render(&L.ro_front[i]);
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

float tilemap_border_left() {
    return 0;
}

float tilemap_border_right() {
    return L.map->cols * TILES_SIZE;
}

float tilemap_border_top() {
    return floor(L.map->rows / 2.0) * TILES_SIZE;
}

float tilemap_border_bottom() {
    return -ceilf(L.map->rows / 2.0) * TILES_SIZE;
}


float tilemap_ground(float x, float y, Color_s *opt_id) {
    int c = tile_c(x);
    int r = tile_r(y);
    while (c >= 0 && c < L.map->cols
           && r >= 0 && r < L.map->rows) {
        Color_s tile = *image_pixel(L.map, 0, c, r);
        if (!color_equals(tile, COLOR_TRANSPARENT)) {
            int pc = tile_pixel_c(x);
            for (int pr = 0; pr < TILES_SIZE; pr++) {
                Color_s id = tiles_pixel(tile, 1, pc, pr);
                if (!color_equals(id, COLOR_TRANSPARENT)) {
                    if (opt_id)
                        *opt_id = id;
                    return sca_min(tile_y(r) - pr, y);
                }
            }
        }
        r++;
    }
    if (opt_id)
        *opt_id = COLOR_TRANSPARENT;
    return -FLT_MAX;
}

float tilemap_ceiling(float x, float y, Color_s *opt_id) {
    int c = tile_c(x);
    int r = tile_r(y);
    while (c >= 0 && c < L.map->cols
           && r >= 0 && r < L.map->rows) {
        Color_s tile = *image_pixel(L.map, 0, c, r);
        if (!color_equals(tile, COLOR_TRANSPARENT)) {
            int pc = tile_pixel_c(x);
            for (int pr = TILES_SIZE - 1; pr >= 0; pr--) {
                Color_s id = tiles_pixel(tile, 1, pc, pr);
                if (!color_equals(id, COLOR_TRANSPARENT)) {
                    if (opt_id)
                        *opt_id = id;
                    return sca_max(tile_y(r) - pr - 1, y);
                }
            }
        }
        r--;
    }
    if (opt_id)
        *opt_id = COLOR_TRANSPARENT;
    return FLT_MAX;
}

float tilemap_wall_left(float x, float y, Color_s *opt_id) {
    int c = tile_c(x);
    int r = tile_r(y);
    while (c >= 0 && c < L.map->cols
           && r >= 0 && r < L.map->rows) {
        Color_s tile = *image_pixel(L.map, 0, c, r);
        if (!color_equals(tile, COLOR_TRANSPARENT)) {
            int pr = tile_pixel_r(y);
            for (int pc = TILES_SIZE - 1; pc >= 0; pc--) {
                Color_s id = tiles_pixel(tile, 1, pc, pr);
                if (!color_equals(id, COLOR_TRANSPARENT)) {
                    if (opt_id)
                        *opt_id = id;
                    return sca_min(tile_x(c) + pc + 1, x);
                }
            }
        }
        c--;
    }
    if (opt_id)
        *opt_id = COLOR_TRANSPARENT;
    return tile_x(0);
}

float tilemap_wall_right(float x, float y, Color_s *opt_id) {
    int c = tile_c(x);
    int r = tile_r(y);
    while (c >= 0 && c < L.map->cols
           && r >= 0 && r < L.map->rows) {
        Color_s tile = *image_pixel(L.map, 0, c, r);
        if (!color_equals(tile, COLOR_TRANSPARENT)) {
            int pr = tile_pixel_r(y);
            for (int pc = 0; pc < TILES_SIZE; pc++) {
                Color_s id = tiles_pixel(tile, 1, pc, pr);
                if (!color_equals(id, COLOR_TRANSPARENT)) {
                    if (opt_id)
                        *opt_id = id;
                    return sca_max(tile_x(c) + pc, x);
                }
            }
        }
        c++;
    }
    if (opt_id)
        *opt_id = COLOR_TRANSPARENT;
    return tile_x(L.map->cols);
}

Color_s tilemap_pixel(int layer, float x, float y) {
    assert(layer >= 0 && layer <= 1);

    int c = tile_c(x);
    int r = tile_r(y);

    if (c < 0 || c >= L.map->cols
        || r < 0 || r >= L.map->rows) {
        return COLOR_TRANSPARENT;
    }

    Color_s tile = *image_pixel(L.map, 0, c, r);
    int pc = tile_pixel_c(x);
    int pr = tile_pixel_r(y);

    return tiles_pixel(tile, layer, pc, pr);
}