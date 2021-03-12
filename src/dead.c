#include "mathc/sca/float.h"
#include "r/ro_single.h"
#include "r/texture.h"
#include "u/pose.h"
#include "camera.h"
#include "dead.h"

#define TIME 0.75
#define START_SIZE 2048
#define END_TIME 512

static struct {
	rRoSingle ro;
	float time;
	DeadFinishedFn callback;
	void *callback_user_data;
	bool callback_called;
} L;


void dead_init(DeadFinishedFn callback, void *callback_user_data) {
    L.callback = callback;
    L.callback_user_data = callback_user_data;
	r_ro_single_init(&L.ro, camera.gl_main, r_texture_init_file("res/dead_overlay.png", NULL));
	L.time = -1;
}

void dead_kill() {
    r_ro_single_kill(&L.ro);
}

void dead_update(float dtime) {
	if(L.time<0)
	    return;
	    
	L.time+=dtime;
	if(L.time>TIME && !L.callback_called) {
	    L.callback(L.callback_user_data);
	    L.callback_called = true;
    }

	float t = L.time/TIME;
	float size = sca_mix(START_SIZE, END_TIME, t);
	u_pose_set_size(&L.ro.rect.pose, size, size);
	L.ro.rect.color.a = sca_mix(0, 0.75, t);
}

void dead_render() {
	if(L.time<0)
	    return;
	r_ro_single_render(&L.ro);
}

void dead_set_dead(float x, float y) {
	L.time = 0;
	L.callback_called = false;
	L.ro.rect.pose = u_pose_new(x, y, START_SIZE, START_SIZE);
	L.ro.rect.color.r = 1.0*rand()/RAND_MAX;
	L.ro.rect.color.g = 1.0*rand()/RAND_MAX;
	L.ro.rect.color.b = 1.0*rand()/RAND_MAX;
	L.ro.rect.color.a = 0;
}

bool dead_is_dead() {
	return L.time>=0;
}
