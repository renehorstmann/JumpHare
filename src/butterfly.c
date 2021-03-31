#include "r/ro_particle.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "mathc/utils/color.h"
#include "utilc/assume.h"
#include "camera.h"
#include "hare.h"
#include "butterfly.h"

#define CHILL_FPS 6.0
#define FLY_FPS 8.0
#define FRAMES 12
#define FLY_SPEED 100
#define COLLECT_DISTANCE 16

static const float RESET_TIME = 10.0 * FRAMES / CHILL_FPS;

static struct {
    rRoParticle ro;
    float time;
    float reset_time;
} L;

static bool is_flying(int i) {
    return u_pose_get_y(L.ro.rects[i].uv) > 0.25;
}

static void fly_away(int i) {
    L.ro.rects[i].uv_time = 1.0/FLY_FPS;
    float angle = sca_random_range(10, 60);
    int flip = rand()%2;
    float uv_w = -1.0/FRAMES;
    if(flip) {
        angle = 180 - angle;
        uv_w = -uv_w;
    }
    L.ro.rects[i].uv = u_pose_new(0, 0.5, uv_w, 0.5);
    L.ro.rects[i].speed.x = sca_cos(sca_radians(angle)) * FLY_SPEED;
    L.ro.rects[i].speed.y = sca_sin(sca_radians(angle)) * FLY_SPEED;
    L.ro.rects[i].start_time = L.time;
    r_ro_particle_update_sub(&L.ro, i, 1);
}

void butterfly_init(const vec2 *positions, int num) {
    assume(num>0, "atleast one butterfly in a level?");
    r_ro_particle_init(&L.ro, num, camera.gl_main, 
            r_texture_new_file("res/butterfly.png", NULL));
            
    for(int i=0; i<num; i++) {
        L.ro.rects[i].pose = u_pose_new(
                sca_random_noise(positions[i].x, 4),
                sca_random_noise(positions[i].y, 4),
                16, 16);
        float uv_w = 1.0/FRAMES;
        if(rand()%2==0)
            uv_w = -uv_w;
        L.ro.rects[i].uv = u_pose_new(
                (float) (rand()%FRAMES) / FRAMES,
                0, uv_w, 0.5);
        L.ro.rects[i].uv_step.x = 1.0/FRAMES;
        L.ro.rects[i].uv_time = 1.0/CHILL_FPS;
        vec3 hsv = {{
            sca_random_range(0, 360),
            sca_random_range(0.25, 0.75),
            sca_random_range(0.75, 1)
        }};
        L.ro.rects[i].color.rgb = vec3_hsv2rgb(hsv);
    }
    
    r_ro_particle_update(&L.ro);
}

void butterfly_kill() {
    r_ro_particle_kill(&L.ro);
}

void butterfly_update(float dtime) {
    L.time += dtime;
    L.reset_time += dtime;
    
    if(L.reset_time >= RESET_TIME) {
        L.reset_time -= RESET_TIME;
        for(int i=0; i<L.ro.num; i++) {
            if(is_flying(i))
                continue;
            L.ro.rects[i].start_time = L.time;
        }
        r_ro_particle_update(&L.ro);
    }
}

void butterfly_render() {
    r_ro_particle_render(&L.ro, L.time);
}


bool butterfly_collect(vec2 position) {
    for(int i=0; i<L.ro.num; i++) {
        if(is_flying(i))
            continue;
        if(vec2_distance(position, u_pose_get_xy(L.ro.rects[i].pose)) <= COLLECT_DISTANCE) {
            fly_away(i);
            return true;
        }
    }
    return false;
}
