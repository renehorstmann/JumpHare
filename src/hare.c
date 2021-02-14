#include "r/ro_single.h"
#include "r/texture.h"
#include "u/pose.h"
#include "camera.h"
#include "hare.h"


static struct {
	rRoSingle ro;
	
	rRoSingle bg;
} L;

void hare_init() {
	r_ro_single_init(&L.ro, camera.gl, r_texture_init_file("res/hare.png", NULL));
	
	r_ro_single_init(&L.bg, camera.gl, r_texture_init_file("res/grass.png", NULL));
}

void hare_update(float dtime) {
	static float x=0;
	//x+=dtime * 20;
	if(x>camera_right())
	    x = camera_left();
	    
	float fps = 6;
	int frames = 4;
	static float time=0;
	time = fmodf(time + dtime, frames/fps);
	int frame = time*fps;
	
	float w = 1.0/4.0;
	float h = 1.0/2.0;
	
	u_pose_set(&L.ro.rect.pose, x, 0, 32, 32, 0);
	u_pose_set(&L.ro.rect.uv, frame*w, 0*h, w, h, 0);
	
	u_pose_set(&L.bg.rect.pose, 0, -10, camera_width(), 64, 0);
}

void hare_render() {
	r_ro_single_render(&L.bg);
	r_ro_single_render(&L.ro);
}

