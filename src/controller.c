#include "r/ro_single.h"
#include "r/texture.h"
#include "e/input.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "hare.h"
#include "hud_camera.h"
#include "controller.h"

#define UP_TIME 0.125
#define JUMP_TIME 0.25
#define MULTI_TIME 0.5
#define DISTANCE 30

static struct {
    rRoSingle background_ro;
    ePointer_s pointer[2];
} L;

static bool in_control_area(vec2 pos) {
    return u_pose_aa_contains(L.background_ro.rect.pose, pos);
}

static void pointer_event(ePointer_s pointer, void *ud) {
    if(pointer.id <0 || pointer.id >1)
        return;
        
    pointer.pos = mat4_mul_vec(hud_camera.matrices.p_inv, pointer.pos);

    if (!in_control_area(pointer.pos.xy)) {
        pointer.action = E_POINTER_UP;
    }
    
    L.pointer[pointer.id] = pointer;
}

static void key_ctrl() {
    static bool jumped = false;

    float speed_x = 0;
    if (e_input.keys.right && !e_input.keys.left)
        speed_x = 1;
    if (e_input.keys.left && !e_input.keys.right)
        speed_x = -1;
    hare_set_speed(speed_x);

    if (e_input.keys.space && !jumped) {
        jumped = true;
        hare_jump();
    }
    if (!e_input.keys.space)
        jumped = false;
}

static void pointer_ctrl(float dtime) {
	static float up_time = FLT_MAX;
	static float multi_time = -1;
	static float single_time = 0;
	static int main_pointer = 0;
	
	int pointers_down = 0;
	
	if(L.pointer[0].action != E_POINTER_UP) {
		pointers_down++;
		if(main_pointer != 0 && L.pointer[1].action == E_POINTER_UP) {
			main_pointer = 0;
		}
	}
	
	if(L.pointer[1].action != E_POINTER_UP) {
		pointers_down++;
		if(main_pointer != 1 && L.pointer[0].action == E_POINTER_UP) {
			main_pointer = 1;
		}
	}
	
    // single time
    if(pointers_down == 0) {
    	single_time = 0;
    }
	if(pointers_down == 1) {
		single_time += dtime;
	}
	
	// stop multi time
	if(pointers_down != 2) {
		multi_time = -1;
	}
	
	// stopping?
	if(pointers_down == 0) {
		up_time += dtime;
		if(up_time>=UP_TIME) {
			hare_set_speed(0);
		}
		return;
	}
	
	// jump tap
	if(up_time>0 && up_time<=JUMP_TIME) {
		hare_jump();
	}
	
	// multi tap to jump
	if(multi_time<0 && pointers_down == 2) {
		multi_time = 0;
		hare_jump();
	}
	
	if(multi_time>=0) {
		multi_time += dtime;
	}
	
	vec2 pos;
	if(multi_time>=MULTI_TIME
	    || (single_time<MULTI_TIME && multi_time>=0)) {
		//pos = vec2_mix(L.pointer[0].pos.xy,
		//               L.pointer[1].pos.xy,
		//               0.5);
		pos = vec2_set(0);
	} else {
		pos = L.pointer[main_pointer].pos.xy;
	}
	
	float speed = pos.x / DISTANCE;
	speed = sca_clamp(speed, -1, 1);
	hare_set_speed(speed);
	
	// reset up_time, cause we are moving
	up_time = 0;
}

void controller_init() {
    L.pointer[0].action = E_POINTER_UP;
    L.pointer[1].action = E_POINTER_UP;
    e_input_register_pointer_event(pointer_event, NULL);

    r_ro_single_init(&L.background_ro, hud_camera.gl, r_texture_init_file("res/hud_background.png", NULL));
}

void controller_kill() {
    e_input_unregister_pointer_event(pointer_event);
    r_ro_single_kill(&L.background_ro);
}

void controller_update(float dtime) {
#ifdef GLES
    pointer_ctrl(dtime);
#else
    key_ctrl();
#endif
    
    if (hud_camera_is_portrait_mode()) {
        float w = hud_camera_width();
        float h = hud_camera_height() * HUD_CAMERA_SCREEN_WEIGHT;
        L.background_ro.rect.pose = u_pose_new(hud_camera_left() + w / 2, hud_camera_bottom() + h - w / 2, w, w);
    } else {
        float w = hud_camera_width() * HUD_CAMERA_SCREEN_WEIGHT;
        float h = hud_camera_height();
        L.background_ro.rect.pose = u_pose_new_angle(hud_camera_right() - w + h / 2, hud_camera_bottom() + h / 2, h, h,
                                                     M_PI_2);
    }
}

void controller_render() {
    r_ro_single_render(&L.background_ro);
}
