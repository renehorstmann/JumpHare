#include "r/ro_single.h"
#include "r/texture.h"
#include "u/pose.h"
#include "camera.h"
#include "background.h"

static struct {
	rRoSingle ro;
	
} L;

void background_init() {
	r_ro_single_init(&L.ro, camera.gl, r_texture_init_file("res/grass.png", NULL));
	u_pose_set(&L.ro.rect.pose, 0, 0, 180, 180, 0);
}

void background_update(float dtime) {
	
}

void background_render() {
	r_ro_single_render(&L.ro);
}

