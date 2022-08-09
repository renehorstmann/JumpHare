#include "e/input.h"
#include "r/ro_batch.h"
#include "u/pose.h"
#include "m/float.h"
#include "m/bool.h"
#include "m/sca/int.h"
#include "camera.h"
#include "controller.h"

#define UP_TIME 0.125
#define JUMP_TIME 0.25
#define MULTI_TIME 0.125
#define DISTANCE 30
#define MIN_SPEED_X 0.2
#define MAX_JUMP_TAP_DISTANCE 50
#define LANDSCAPE_INGAME_AREA 30

#define BACKGROUND_SIZE 512


struct Controller_Globals controller;

static struct {
    RoBatch background_ro;
    ePointer_s pointer[2];
    bvec2 pointer_down_map;
    int pointer_down;
    int main_pointer;
} L;

static bool in_control_area(vec2 pos) {
    return pos.x < camera.RO.left + LANDSCAPE_INGAME_AREA 
            || pos.x > camera.RO.right - LANDSCAPE_INGAME_AREA
            || pos.y > camera.RO.top || pos.y < camera.RO.bottom;
}

static void pointer_event(ePointer_s pointer, void *ud) {
    if (pointer.id < 0 || pointer.id > 1)
        return;

    pointer.pos = mat4_mul_vec(camera.matrices_p_inv, pointer.pos);

    if(in_control_area(pointer.pos.xy)) {
        if(pointer.action == E_POINTER_DOWN) {
            L.pointer_down_map.v[pointer.id] = true;
        }
    } 
    if(pointer.action == E_POINTER_UP) {
        L.pointer_down_map.v[pointer.id] = false;
    }
    
    L.pointer_down = bvec2_sum(L.pointer_down_map);
    if(L.pointer_down == 1) {
        L.main_pointer = L.pointer_down_map.v0 == 0? 1 : 0;
    }

    L.pointer[pointer.id] = pointer;
}

static void key_ctrl() {
    static bool action = false;
    
    eInputKeys keys = e_input.keys;
    if (keys.right && !keys.left) {
        controller.out.speed_x = 1;
    } else if (keys.left && !keys.right) {
        controller.out.speed_x = -1;
    } 

    // only once
    if (keys.space && !action) {
        action = true;
        controller.out.action = true;
    }
    
    if (!keys.space) {
        action = false;
    }
        
}

static void pointer_ctrl(float dtime) {
    static float up_time = FLT_MAX;
    static float multi_time = -1;
    static float single_time = 0;
    static vec2 last_pointer_pos;
    static float speed = 0;

    // single time
    if (L.pointer_down == 0) {
        single_time = 0;
    }
    if (L.pointer_down == 1) {
        single_time += dtime;
    }

    // stop multi time
    if (L.pointer_down != 2) {
        multi_time = -1;
    }

    // stopping?
    if (L.pointer_down == 0) {
        up_time += dtime;
        if (up_time < UP_TIME) {
            controller.out.speed_x = speed;
        }
        return;
    }

    // jump tap
    if (up_time > 0 && up_time <= JUMP_TIME) {
        
        // check distance
        if(vec2_distance(L.pointer[L.main_pointer].pos.xy,
                last_pointer_pos) 
                <= MAX_JUMP_TAP_DISTANCE) {
            controller.out.action = true;
        }
    }

    // multi tap to jump
    if (multi_time < 0 && L.pointer_down == 2) {
        multi_time = 0;
        controller.out.action = true;
    }

    if (multi_time >= 0) {
        multi_time += dtime;
    }

    vec2 pos = L.pointer[L.main_pointer].pos.xy;

    if(!camera_is_portrait_mode()) {
        float center;
        if(pos.x > 0) {
            center = sca_mix(camera.RO.hud_right, camera.RO.hud_right, 0.33);
            center = sca_min(center, camera.RO.hud_right-DISTANCE*1.5);
        } else {
            center = sca_mix(camera.RO.hud_left, camera.RO.hud_left, 0.33);
            center = sca_max(center, camera.RO.hud_left+DISTANCE*1.5);
        }
        
        pos.x -= center;
    }

    
    speed = pos.x / DISTANCE;
    speed = sca_clamp(speed, -1, 1);

    if(sca_abs(speed) < MIN_SPEED_X) {
        speed = 0;
    }
    if (single_time < MULTI_TIME && multi_time >= 0) {
        speed = 0;
    }

    controller.out.speed_x = speed;

    // reset up_time, cause we are moving
    up_time = 0;
    last_pointer_pos = L.pointer[L.main_pointer].pos.xy;
}


//j
// public
//

void controller_init() {
    L.pointer[0].action = E_POINTER_UP;
    L.pointer[1].action = E_POINTER_UP;
    e_input_register_pointer_event(pointer_event, NULL);

    L.background_ro = ro_batch_new(2, r_texture_new_file(1, 1, "res/hud_background.png"));
    //L.background_ro.rect.color.a = 0.0;
}

void controller_kill() {
    e_input_unregister_pointer_event(pointer_event);
    ro_batch_kill(&L.background_ro);
    memset(&L, 0, sizeof L);
    memset(&controller, 0, sizeof controller);
}

void controller_update(float dtime) {
    controller.out.speed_x = 0;
    controller.out.action = false;
    
    key_ctrl();
    pointer_ctrl(dtime);
    


    // ro
    mat4 pose = u_pose_new(0, 0, BACKGROUND_SIZE, BACKGROUND_SIZE);
    if (camera_is_portrait_mode()) {
        u_pose_set_y(&pose, camera.RO.bottom - BACKGROUND_SIZE/2);
        L.background_ro.rects[0].pose = pose;
        
        L.background_ro.rects[1].pose = u_pose_new_hidden();
    } else {
        u_pose_set_x(&pose, camera.RO.left - BACKGROUND_SIZE/2);
        L.background_ro.rects[0].pose = pose;
        u_pose_set_x(&pose, camera.RO.right + BACKGROUND_SIZE/2);
        L.background_ro.rects[1].pose = pose;
    }
}

void controller_render(const mat4 *hudcam_mat) {
    ro_batch_render(&L.background_ro, hudcam_mat, true);
}
