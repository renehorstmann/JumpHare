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
// private
//

static struct {
    RoSingle strike_ro, blend_ro;
    float time;
    DeadFinishedFn callback;
    void *callback_user_data;
    bool callback_called;
} L;



//
// public
//


void dead_init(DeadFinishedFn callback, void *callback_user_data) {
    L.callback = callback;
    L.callback_user_data = callback_user_data;
    L.strike_ro = ro_single_new(r_texture_new_file(1, 1, "res/dead_overlay.png"));

    L.blend_ro = ro_single_new(r_texture_new_white_pixel());

    u_pose_set_size(&L.blend_ro.rect.pose, 1024, 1024);
    L.time = -1;
}

void dead_kill() {
    ro_single_kill(&L.strike_ro);
    ro_single_kill(&L.blend_ro);
}

void dead_update(float dtime) {
    if (L.time < 0)
        return;

    L.time += dtime;
    if (L.time > TIME && !L.callback_called) {
        L.callback(L.callback_user_data);
        L.callback_called = true;
    }

    if (L.time > BLEND_END_TIME) {
        L.time = -1;
    }

    float t = sca_clamp(L.time / ANIMATE_TIME, 0, 1);
    float size = sca_mix(START_SIZE, END_SIZE, t);
    u_pose_set_size(&L.strike_ro.rect.pose, size, size);
    L.strike_ro.rect.color.a = sca_mix(0, 0.75, t);

    if (t < BLEND_START_TIME)
        return;

    float blend_time_half = (BLEND_END_TIME - BLEND_START_TIME) / 2;

    float alpha;
    if (L.time < BLEND_START_TIME + blend_time_half) {
        alpha = (L.time - BLEND_START_TIME) / blend_time_half;
    } else {
        alpha = 1 - (L.time - BLEND_START_TIME - blend_time_half) / blend_time_half;
    }

    L.blend_ro.rect.color.a = sca_clamp(alpha, 0, 1);
}

void dead_render() {
    if (L.time < 0)
        return;
    if (L.time <= TIME)
        ro_single_render(&L.strike_ro, camera.gl_main);
    ro_single_render(&L.blend_ro, hudcamera.gl);
}

void dead_set_dead(float x, float y) {
    if(dead_is_dead())
        return;
    log_info("dead: dead");
    L.time = 0;
    L.callback_called = false;
    L.strike_ro.rect.pose = u_pose_new(x, y, START_SIZE, START_SIZE);
    L.strike_ro.rect.color.rgb = vec3_random();
    L.strike_ro.rect.color.a = 0;

    L.blend_ro.rect.color = L.strike_ro.rect.color;
}

bool dead_is_dead() {
    return L.time >= 0 && L.time < TIME;
}
