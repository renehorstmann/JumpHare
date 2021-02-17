#include <float.h>
#include "mathc/float.h"
#include "utilc/assume.h"
#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "camera.h"
#include "io.h"
#include "level.h"


enum tile_modes {
	TILE_NONE,
	TILE_FULL,
	TILE_TOP,
	TILE_LEFT,
	TILE_RIGHT,
	TILE_TOP_LEFT,
	TILE_TOP_RIGHT,
	TILE_BOTTOM_LEFT,
	TILE_BOTTOM_RIGHT,
	NUM_TILE_MODES
};


static struct {
	rRoBatch ro;
	
} L;


static mat4 tile_pose(int c, int r) {
	return u_pose_new_aa(-90+c*16, 90-r*16, 16, 16);
}

static mat4 tile_uv(enum tile_modes mode) {
	float w = 1.0/4.0;
	float h = 1.0/4.0;
	switch(mode) {
		case TILE_NONE:
		    return u_pose_new(3*w, 0, w, h);
		case TILE_FULL:
		    return u_pose_new(1*w, 1*h, w, h);
		case TILE_TOP:
		    return u_pose_new(1*w, 0, w, h);
		case TILE_LEFT:
		    return u_pose_new(0, 1*h, w, h);
		case TILE_RIGHT:
		    return u_pose_new(2*w, 1*h, w, h);
		case TILE_TOP_LEFT:
		    return u_pose_new(0, 0, w, h);
		case TILE_TOP_RIGHT:
		    return u_pose_new(2*w, 0, w, h);
		case TILE_BOTTOM_LEFT:
		    return u_pose_new(0, 2*h, w, h);
		case TILE_BOTTOM_RIGHT:
		    return u_pose_new(2*w, 2*h, w, h);
		
		default:
		    assume(false, "invalid tilemode");
	};
	return (mat4) {{0}};
}

static bool is_block(Color_s c) {
	return c.r > 128;
}

static enum tile_modes get_mode(Image *lvl, int c, int r) {
    if(is_block(*image_pixel(lvl, 0, c, r)))
        return TILE_FULL;
    return TILE_NONE;
}

void level_init() {
	Image *lvl = io_load_image("res/level_001.png");
	assume(lvl, "level not found");
	
	r_ro_batch_init(&L.ro, lvl->rows * lvl->cols, camera.gl, r_texture_init_file("res/tile_leafes.png", NULL));
	
	for(int r=0; r<lvl->rows; r++) {
	    for(int c=0; c<lvl->cols; c++) {
		    rRect_s *rect = &L.ro.rects[r*lvl->cols + c];
		    rect->pose = tile_pose(c, r);
		    rect->uv = tile_uv(get_mode(lvl, c, r));
		}
	}
	
	
	image_delete(lvl);
	
	r_ro_batch_update(&L.ro);
}

void level_update(float dtime) {
	
}

void level_render() {
	r_ro_batch_render(&L.ro);
}

