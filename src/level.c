#include <float.h>
#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "camera.h"
#include "level.h"


static const char lvl[11][11] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 1, 2, 3, 0, 0, 0, 1, 3, 0},
	{0, 0, 4, 5, 6, 0, 0, 1, 7, 6, 0},
	{0, 1, 7, 5, 9, 2, 2, 7, 5, 6, 0},
	{2, 7, 5, 5, 5, 5, 5, 5, 5, 9, 2},
	{5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
	{5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}
};


static struct {
	rRoBatch ro;
	
} L;


void level_init() {
	r_ro_batch_init(&L.ro, 256, camera.gl, r_texture_init_file("res/tile_leafes.png", NULL));
	
	for(int r=0;r<11;r++) {
        for(int c=0; c<11; c++) {
        	rRect_s *rect = &L.ro.rects[r*11+c];
        	int tile = lvl[r][c];
        	
        	if(tile == 0) {
        		u_pose_set(&rect->pose, FLT_MAX, FLT_MAX, 0, 0, 0);
        		continue;
        	}
        	tile--;
        	
        	printf("tile %d @ rc %d %d \n", tile, r, c);
        	u_pose_aa_set(&rect->pose, -90+c*16, 90-r*16, 16, 16);
        	
        	
        	float w = 1.0/4.0;
        	float h = 1.0/4.0;
        	float u = tile % 3;
        	float v = tile / 3;
        	u_pose_set(&rect->uv, u * w, v * h, w, h, 0);
        }		
	}
	
	for(int i=11*11; i<L.ro.num; i++) {
		u_pose_set(&L.ro.rects[i].pose, FLT_MAX, FLT_MAX, 0, 0, 0);
	}
	
	r_ro_batch_update(&L.ro);
}

void level_update(float dtime) {
	
}

void level_render() {
	r_ro_batch_render(&L.ro);
}

