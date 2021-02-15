#include "mathc/float.h"
#include "e/input.h"
#include "hare.h"
#include "camera.h"
#include "controller.h"

static struct {
	vec4 start_pos;
	
} L;

static void pointer_event(ePointer_s pointer, void *ud) {
	if(pointer.action == E_POINTER_UP) {
		hare_set_speed(0);
		return;
	}
	ePointer_s c_pointer = pointer;
    c_pointer.pos = mat4_mul_vec(camera.matrices.v_p_inv, pointer.pos);
    
    if(pointer.action == E_POINTER_DOWN) {
    	L.start_pos = c_pointer.pos;
    	return;
    }
    
    float dx = (c_pointer.pos.x - L.start_pos.x);
    if(sca_abs(dx)>80) {
    	float diff = sca_sign(dx) * (sca_abs(dx) - 80);
    	L.start_pos.x += diff;
    }
    hare_set_speed(dx);
}

void conttoller_init() {
	e_input_register_pointer_event(pointer_event, NULL);
}

void conttoller_update(float dtime) {
	
}

void conttoller_render() {
	
}
