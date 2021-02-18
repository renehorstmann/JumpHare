#include "mathc/float.h"
#include "camera.h"
#include "hare.h"
#include "camera_control.h"

#define MAX_DIFF 50

struct CameraControlGlobals_s camera_control;


void camera_control_init() {
	camera_control.pos = hare_position();
	camera_set_pos(camera_control.pos.x, camera_control.pos.y);
}

void camera_control_update(float dtime) {
//	vec2_println(camera_control.pos);
	vec2 h = hare_position();
	vec2 delta = vec2_sub_vec(h, camera_control.pos);
	float diff = vec2_norm(delta);
	diff -= MAX_DIFF;
	
	if(diff>0) {
	    delta = vec2_normalize(delta);
	
	     // pos += norm(delta) * (diff - MAX_DIFF)
	    camera_control.pos = vec2_add_vec(camera_control.pos, vec2_scale(delta, diff));
	}
	
	//camera_control.pos = vec2_scale(vec2_floor(vec2_scale(camera_control.pos, 4)), 0.25);
	
	camera_set_pos(camera_control.pos.x, camera_control.pos.y);
	
	static float t;
	t+=dtime;
	//camera_set_angle(t*M_PI/4);
}

