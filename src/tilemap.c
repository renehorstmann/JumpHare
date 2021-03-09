#include "mathc/sca/int.h"
#include "mathc/sca/float.h"
#include "r/ro_batch.h"
#include "u/pose.h"

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
	return x/TILES_SIZE;
}

static int tile_r(float y) {
	return L.map->rows/2 - y / TILES_SIZE;
}

static int tile_pixel_c(float x) {
    return (int) x%TILES_SIZE;
}

static int tile_pixel_r(float y) {
	return (int) (L.map->rows/2*TILES_SIZE-y)%TILES_SIZE;
}

static float tile_x(int c) {
	return c * TILES_SIZE;
}

static float tile_y(int r) {
	return (L.map->rows/2- r) * TILES_SIZE;
}

static mat4 tile_pose(int c, int r) {
    return u_pose_new_aa(tile_x(c), tile_y(r), TILES_SIZE, TILES_SIZE);
}

static mat4 tile_uv(int id) {
    float w = 1.0/TILES_COLS;
    float h = 1.0/TILES_ROWS;
    float u = w * (id%TILES_COLS);
    float v = h * (id/TILES_COLS);
    return u_pose_new(u, v, w, h);
}


static bool pixel_collision(Color_s code, int pixel_c, int pixel_r) {
	return !color_equals(tiles_pixel(code, 1, pixel_c, pixel_r), 
	    COLOR_TRANSPARENT);
}


void tilemap_init() {

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

void tilemap_load_level(const char *file) {
    if (L.map) {
        image_delete(L.map);
        for (int i = 0; i < tiles.size; i++) {
            r_ro_batch_kill(&L.ro_back[i]);
//            r_ro_batch_kill(&L.ro_front[i]);
        }
    }
    L.map = io_load_image(file, MAP_LAYERS);

    int tile_nums[MAX_TILES] = {0};
    for (int r = 0; r < L.map->rows; r++) {
        for (int c = 0; c < L.map->cols; c++) {
            Color_s code;
            code = *image_pixel(L.map, 0, c, r);
            if(color_equals(code, (Color_s){0})) {
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
            if(color_equals(code, COLOR_TRANSPARENT)) {
                continue;
            }
            int tile_id = code.b-1;
            int tile = code.a;
            rRect_s *rect = &L.ro_back[tile_id].rects[tile_nums[tile_id]];
            rect->pose = tile_pose(c, r);
            rect->uv = tile_uv(tile);
            tile_nums[tile_id]++;
        }
    }

    for(int i=0; i<tiles.size; i++) {
        r_ro_batch_update(&L.ro_back[i]);
//        r_ro_batch_update(&L.ro_front[i]);
    }
}


float tilemap_border_left() {
	return 0;
}
float tilemap_border_right() {
	return L.map->cols * TILES_SIZE;
}
float tilemap_border_top() {
	return floor(L.map->rows/2.0) * TILES_SIZE;
}
float tilemap_border_bottom() {
	return -ceilf(L.map->rows/2.0) * TILES_SIZE;
}


float tilemap_ground(float x, float y) {
    int c = tile_c(x);
    int r = tile_r(y);
    Color_s code = COLOR_TRANSPARENT;
    while(c>=0 && c<L.map->cols 
        && r>=0 && r<L.map->rows) {
        code = *image_pixel(L.map, 0, c, r);
        if(!color_equals(code, COLOR_TRANSPARENT)) {
            int pc = tile_pixel_c(x);
            for(int pr=0; pr<TILES_SIZE; pr++) {
        	    if(pixel_collision(code, pc, pr))
        		    return sca_min(tile_y(r) - pr, y);
            }
        }
        r++;
    }
    return tile_y(L.map->rows);
}

float tilemap_ceiling(float x, float y) {
    int c = tile_c(x);
    int r = tile_r(y);
    Color_s code = COLOR_TRANSPARENT;
    while(c>=0 && c<L.map->cols 
        && r>=0 && r<L.map->rows) {
        code = *image_pixel(L.map, 0, c, r);
        if(!color_equals(code, COLOR_TRANSPARENT)) {
            int pc = tile_pixel_c(x);
            for(int pr=TILES_SIZE-1; pr>=0; pr--) {
        	    if(pixel_collision(code, pc, pr))
        		    return sca_max(tile_y(r) - pr - 1, y);
            }
        }
        r--;
    }
    return tile_y(0);
}

float tilemap_wall_left(float x, float y) {
    int c = tile_c(x);
    int r = tile_r(y);
    Color_s code = COLOR_TRANSPARENT;
    while(c>=0 && c<L.map->cols 
        && r>=0 && r<L.map->rows) {
        code = *image_pixel(L.map, 0, c, r);
        if(!color_equals(code, COLOR_TRANSPARENT)) {
            int pr = tile_pixel_r(y);
            for(int pc=TILES_SIZE-1; pc>=0; pc--) {
        	    if(pixel_collision(code, pc, pr))
        		    return sca_min(tile_x(c) + pc + 1, x);
            }
        }
        c--;
    }
    return tile_x(0);
}

float tilemap_wall_right(float x, float y) {
    int c = tile_c(x);
    int r = tile_r(y);
    Color_s code = COLOR_TRANSPARENT;
    while(c>=0 && c<L.map->cols 
        && r>=0 && r<L.map->rows) {
        code = *image_pixel(L.map, 0, c, r);
        if(!color_equals(code, COLOR_TRANSPARENT)) {
            int pr = tile_pixel_r(y);
            for(int pc=0; pc<TILES_SIZE; pc++) {
        	    if(pixel_collision(code, pc, pr))
        		    return sca_max(tile_x(c) + pc, x);
            }
        }
        c++;
    }
    return tile_x(L.map->cols);
}
