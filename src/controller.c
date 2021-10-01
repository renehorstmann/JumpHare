#include "r/ro_batch.h"
#include "r/texture.h"
#include "e/input.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/bool.h"
#include "mathc/sca/int.h"
#include "camera.h"
#include "hudcamera.h"
#include "controller.h"

#define UP_TIME 0.125
#define JUMP_TIME 0.25
#define MULTI_TIME 0.125
#define DISTANCE 30
#define MIN_SPEED_X 0.2
#define MAX_JUMP_TAP_DISTANCE 50
#define LANDSCAPE_INGAME_AREA 30

#define BACKGROUND_SIZE 512


//
// private
//

static bool in_control_area(const Camera_s *camera, vec2 pos) {
    return pos.x < camera->RO.left + LANDSCAPE_INGAME_AREA 
            || pos.x > camera->RO.right - LANDSCAPE_INGAME_AREA
            || pos.y > camera->RO.top || pos.y < camera->RO.bottom;
}

static void pointer_event(ePointer_s pointer, void *ud) {
    if (pointer.id < 0 || pointer.id > 1)
        return;
        
    Controller *self = ud;

    pointer.pos = mat4_mul_vec(self->hudcam_ref->matrices.p_inv, pointer.pos);

    if(in_control_area(self->camera_ref, pointer.pos.xy)) {
        if(pointer.action == E_POINTER_DOWN) {
            self->L.pointer_down_map.v[pointer.id] = true;
        }
    } 
    if(pointer.action == E_POINTER_UP) {
        self->L.pointer_down_map.v[pointer.id] = false;
    }
    
    self->L.pointer_down = bvec2_sum(self->L.pointer_down_map);
    if(self->L.pointer_down == 1) {
        self->L.main_pointer = self->L.pointer_down_map.v0 == 0? 1 : 0;
    }

    self->L.pointer[pointer.id] = pointer;
}

static void key_ctrl(Controller *self) {
    static bool action = false;

    eInputKeys keys = e_input_get_keys(self->input_ref);
    if (keys.right && !keys.left) {
        self->out.speed_x = 1;
    } else if (keys.left && !keys.right) {
        self->out.speed_x = -1;
    } 

    // only once
    if (keys.space && !action) {
        action = true;
        self->out.action = true;
    }
    
    if (!keys.space) {
        action = false;
    }
        
}

static void pointer_ctrl(Controller *self, float dtime) {
    static float up_time = FLT_MAX;
    static float multi_time = -1;
    static float single_time = 0;
    static vec2 last_pointer_pos;
    static float speed = 0;

    // single time
    if (self->L.pointer_down == 0) {
        single_time = 0;
    }
    if (self->L.pointer_down == 1) {
        single_time += dtime;
    }

    // stop multi time
    if (self->L.pointer_down != 2) {
        multi_time = -1;
    }

    // stopping?
    if (self->L.pointer_down == 0) {
        up_time += dtime;
        if (up_time < UP_TIME) {
            self->out.speed_x = speed;
        }
        return;
    }

    // jump tap
    if (up_time > 0 && up_time <= JUMP_TIME) {
        
        // check distance
        if(vec2_distance(self->L.pointer[self->L.main_pointer].pos.xy,
                last_pointer_pos) 
                <= MAX_JUMP_TAP_DISTANCE) {
            self->out.action = true;
        }
    }

    // multi tap to jump
    if (multi_time < 0 && self->L.pointer_down == 2) {
        multi_time = 0;
        self->out.action = true;
    }

    if (multi_time >= 0) {
        multi_time += dtime;
    }

    vec2 pos = self->L.pointer[self->L.main_pointer].pos.xy;

    if(!hudcamera_is_portrait_mode(self->hudcam_ref)) {
        float center;
        if(pos.x > 0) {
            center = sca_mix(self->camera_ref->RO.right, self->hudcam_ref->RO.right, 0.33);
            center = sca_min(center, self->hudcam_ref->RO.right-DISTANCE*1.5);
        } else {
            center = sca_mix(self->camera_ref->RO.left, self->hudcam_ref->RO.left, 0.33);
            center = sca_max(center, self->hudcam_ref->RO.left+DISTANCE*1.5);
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

    self->out.speed_x = speed;

    // reset up_time, cause we are moving
    up_time = 0;
    last_pointer_pos = self->L.pointer[self->L.main_pointer].pos.xy;
}


//j
// public
//

Controller *controller_new(eInput *input, const Camera_s *camera, const HudCamera_s *hudcam) {
    Controller *self = rhc_calloc(sizeof *self);
    
    self->input_ref = input;
    self->camera_ref = camera;
    self->hudcam_ref = hudcam;
    
    self->L.pointer[0].action = E_POINTER_UP;
    self->L.pointer[1].action = E_POINTER_UP;
    e_input_register_pointer_event(input, pointer_event, self);

    self->L.background_ro = ro_batch_new(2, r_texture_new_file(1, 1, "res/hud_background.png"));
    //self->L.background_ro.rect.color.a = 0.0;
    
    return self;
}

void controller_kill(Controller **self_ptr) {
    Controller *self = *self_ptr;
    if(!self)
        return;
        
    e_input_unregister_pointer_event(self->input_ref, pointer_event);
    ro_batch_kill(&self->L.background_ro);
    
    rhc_free(self);
    *self_ptr = NULL;
}

void controller_update(Controller *self, float dtime) {
    self->out.speed_x = 0;
    self->out.action = false;
    
    key_ctrl(self);
    pointer_ctrl(self, dtime);
    


    // ro
    mat4 pose = u_pose_new(0, 0, BACKGROUND_SIZE, BACKGROUND_SIZE);
    if (hudcamera_is_portrait_mode(self->hudcam_ref)) {
        u_pose_set_y(&pose, self->camera_ref->RO.bottom - BACKGROUND_SIZE/2);
        self->L.background_ro.rects[0].pose = pose;
        
        self->L.background_ro.rects[1].pose = u_pose_new_hidden();
    } else {
        u_pose_set_x(&pose, self->camera_ref->RO.left - BACKGROUND_SIZE/2);
        self->L.background_ro.rects[0].pose = pose;
        u_pose_set_x(&pose, self->camera_ref->RO.right + BACKGROUND_SIZE/2);
        self->L.background_ro.rects[1].pose = pose;
    }
    
    ro_batch_update(&self->L.background_ro);
}

void controller_render(Controller *self, const mat4 *hudcam_mat) {
    ro_batch_render(&self->L.background_ro, hudcam_mat);
}
