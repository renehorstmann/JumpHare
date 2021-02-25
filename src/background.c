#include "r/ro_single.h"
#include "r/texture.h"
#include "u/pose.h"
#include "camera.h"
#include "background.h"

static struct {
	rRoSingle ro;
	
} L;

void background_init() {
	r_ro_single_init(&L.ro, camera.gl_background[0], r_texture_init_file("res/grass.png", NULL));
	
}

void background_update(float dtime) {
	L.ro.rect.pose = u_pose_new(0, 0, camera_width(), camera_height());
}

void background_render() {
	r_ro_single_render(&L.ro);
}

