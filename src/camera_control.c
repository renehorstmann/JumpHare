#include "mathc/float.h"
#include "camera.h"
#include "hare.h"
#include "camera_control.h"

#define MAX_DIFF 50

static struct {
	vec2 pos;
	
} L;

void camera_control_init() {
	L.pos = hare_position();
	camera_set_pos(L.pos.x, L.pos.y);
}

void camera_control_update(float dtime) {
	vec2 h = hare_position();
	vec2 delta = vec2_sub_vec(h, L.pos);
	float diff = vec2_norm(delta);
	if(diff < MAX_DIFF)
	    return;
	    
	diff -= MAX_DIFF;
	delta = vec2_normalize(delta);
	
	// pos += norm(delta) * (diff - MAX_DIFF)
	L.pos = vec2_add_vec(L.pos, vec2_scale(delta, diff));
	
	camera_set_pos(L.pos.x, L.pos.y);
}

