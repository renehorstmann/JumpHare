#include "mathc/float.h"
#include "r/ro_single.h"
#include "r/texture.h"
#include "e/input.h"
#include "u/pose.h"
#include "hare.h"
#include "camera.h"
#include "controller.h"

static struct {
	rRoSingle background_ro;
	vec4 start_pos;
	vec4 actual_pos;
	vec4 last_pos;
} L;


static void pointer_event(ePointer_s pointer, void *ud) {
	L.actual_pos = pointer.pos;
	
	if(pointer.action == E_POINTER_UP) {
		hare_set_speed(0, 0);
		return;
	}
	ePointer_s c_pointer = pointer;
    c_pointer.pos = mat4_mul_vec(camera.matrices_hud_v_p_inv, pointer.pos);
    
    if(pointer.action == E_POINTER_DOWN) {
    	L.start_pos = c_pointer.pos;
    	return;
    }
    
    float dx = c_pointer.pos.x - L.start_pos.x;
    if(sca_abs(dx)>80) {
    	float diff = sca_sign(dx) * (sca_abs(dx) - 80);
    	L.start_pos.x += diff;
    }
    
    float dy = c_pointer.pos.y - L.start_pos.y;
    hare_set_speed(dx, dy);
}

void controller_init() {
	e_input_register_pointer_event(pointer_event, NULL);
	
	r_ro_single_init(&L.background_ro, camera.gl_hud, r_texture_init_file("res/hud_background.png", NULL));
}

void controller_update(float dtime) {
	float dy = L.actual_pos.y - L.last_pos.y;
	float speed = dy / dtime;
	if(speed>2)
	    hare_jump();
	
	
	
	L.last_pos = L.actual_pos;
	
	if(camera_is_portrait_mode()) {
	    float w = camera_width();
	    float h = camera_height()*2/5;
	    L.background_ro.rect.pose = u_pose_new(camera_left()+w/2, camera_bottom() + h-w/2, w, w);
	} else {
		float w = camera_width()*2/5;
	    float h = camera_height();
	    L.background_ro.rect.pose = u_pose_new_angle(camera_right() - w+h/2, camera_bottom()+h/2, h, h, M_PI_2);
	}
}

void controller_render() {
	r_ro_single_render(&L.background_ro);
}
