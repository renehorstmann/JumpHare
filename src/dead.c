#include "r/ro_single.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/sca/float.h"
#include "mathc/utils/random.h"
#include "rhc/log.h"
#include "camera.h"
#include "hudcamera.h"
#include "dead.h"

#define TIME 0.75
#define ANIMATE_TIME 0.65
#define BLEND_START_TIME 0.5
#define BLEND_END_TIME 1.0
#define START_SIZE 2048
#define END_SIZE 512


//
// public
//


Dead *dead_new(DeadFinishedFn callback, void *callback_user_data) {
    Dead *self = rhc_calloc(sizeof *self);
    
    self->L.callback = callback;
    self->L.callback_user_data = callback_user_data;
    self->L.strike_ro = ro_single_new(r_texture_new_file(1, 1, "res/dead_overlay.png"));

    self->L.blend_ro = ro_single_new(r_texture_new_white_pixel());

    u_pose_set_size(&self->L.blend_ro.rect.pose, 1024, 1024);
    self->L.time = -1;
    
    return self;
}

void dead_kill(Dead **self_ptr) {
    Dead *self = *self_ptr;
    if(!self)
        return;
    ro_single_kill(&self->L.strike_ro);
    ro_single_kill(&self->L.blend_ro);
    rhc_free(self);
    *self_ptr = NULL;
}

void dead_update(Dead *self, float dtime) {
    if (self->L.time < 0)
        return;

    self->L.time += dtime;
    if (self->L.time > TIME && !self->L.callback_called) {
        self->L.callback(self->L.callback_user_data);
        self->L.callback_called = true;
    }

    if (self->L.time > BLEND_END_TIME) {
        self->L.time = -1;
    }

    float t = sca_clamp(self->L.time / ANIMATE_TIME, 0, 1);
    float size = sca_mix(START_SIZE, END_SIZE, t);
    u_pose_set_size(&self->L.strike_ro.rect.pose, size, size);
    self->L.strike_ro.rect.color.a = sca_mix(0, 0.75, t);

    if (t < BLEND_START_TIME)
        return;

    float blend_time_half = (BLEND_END_TIME - BLEND_START_TIME) / 2;

    float alpha;
    if (self->L.time < BLEND_START_TIME + blend_time_half) {
        alpha = (self->L.time - BLEND_START_TIME) / blend_time_half;
    } else {
        alpha = 1 - (self->L.time - BLEND_START_TIME - blend_time_half) / blend_time_half;
    }

    self->L.blend_ro.rect.color.a = sca_clamp(alpha, 0, 1);
}

void dead_render(Dead *self, const mat4 *cam_mat, const mat4 *hudcam_mat) {
    if (self->L.time < 0)
        return;
    if (self->L.time <= TIME)
        ro_single_render(&self->L.strike_ro, cam_mat);
    ro_single_render(&self->L.blend_ro, hudcam_mat);
}

void dead_set_dead(Dead *self, float x, float y) {
    if(dead_is_dead(self))
        return;
    log_info("dead: dead");
    self->L.time = 0;
    self->L.callback_called = false;
    self->L.strike_ro.rect.pose = u_pose_new(x, y, START_SIZE, START_SIZE);
    self->L.strike_ro.rect.color.rgb = vec3_random();
    self->L.strike_ro.rect.color.a = 0;

    self->L.blend_ro.rect.color = self->L.strike_ro.rect.color;
}

bool dead_is_dead(const Dead *self) {
    return self->L.time >= 0 && self->L.time < TIME;
}
