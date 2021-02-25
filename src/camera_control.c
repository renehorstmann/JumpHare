#include "mathc/float.h"
#include "camera.h"
#include "hare.h"
#include "tilemap.h"
#include "camera_control.h"

#define MAX_DIFF 50

struct CameraControlGlobals_s camera_control;


static void check_limits() {
	
	vec2 min, max;
	if(camera_is_portrait_mode()) {
	    min.x = tilemap_left()-camera_left();
	    max.x = tilemap_right()-camera_right();
	    min.y = tilemap_bottom()+camera_height()*3/10;
	    max.y = tilemap_top()-camera_height()*3/10;
	} else {
		min.x = tilemap_left()+camera_width()*3/10;
	    max.x = tilemap_right()-camera_width()*3/10;
	    min.y = tilemap_bottom()-camera_bottom();
	    max.y = tilemap_top()-camera_top();
	}
	
	min = vec2_ceil(min);
	max = vec2_floor(max);
	
	camera_control.pos = vec2_clamp_vec(camera_control.pos, min, max);
	
	vec2_println(camera_control.pos);
}


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
	
	
	check_limits();
	
	camera_set_pos(camera_control.pos.x, camera_control.pos.y);
	
	static float t;
	t+=dtime;
	//camera_set_angle(t*M_PI/4);
}

