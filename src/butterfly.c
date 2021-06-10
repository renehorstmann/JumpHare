#include "r/ro_particle.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "mathc/utils/color.h"
#include "rhc/error.h"
#include "rhc/allocator.h"
#include "camera.h"
#include "butterfly.h"

#define CHILL_FPS 6.0
#define FLY_FPS 8.0
#define FRAMES 12
#define FLY_SPEED 75
#define FLY_ACC_Y 20
#define COLLECT_DISTANCE 16

static const float RESET_TIME = 5.0 * FRAMES / CHILL_FPS;


//
// private
//

static struct {
    RoParticle ro;
    float time;
    float reset_time;
    int collected;
    float collected_time;
    vec3 last_color;
    
    struct {
        bool *collected;
    } save;
} L;

static bool is_flying(int i) {
    return L.ro.rects[i].sprite.y > 0.5;
}

static void init_stay(int i) {
    L.ro.rects[i].sprite.y = 0;
    L.ro.rects[i].sprite_speed.x = CHILL_FPS;
    int flip = rand()%2;
    float uv_w = -1;
    if(flip) {
        uv_w = 1;
    }
    L.ro.rects[i].uv = u_pose_new(0, 0, uv_w, 1);
    L.ro.rects[i].speed.x = 0;
    L.ro.rects[i].speed.y = 0;
    L.ro.rects[i].acc.y = 0;
    L.ro.rects[i].start_time = L.time;
}

static void init_fly(int i) {
    L.ro.rects[i].sprite.y = 1;
    L.ro.rects[i].sprite_speed.x = FLY_FPS;
    float angle = sca_random_range(15, 60);
    int flip = rand()%2;
    float uv_w = -1;
    if(flip) {
        angle = 180 - angle;
        uv_w = 1;
    }
    L.ro.rects[i].uv = u_pose_new(0, 0, uv_w, 1);
    L.ro.rects[i].speed.x = sca_cos(sca_radians(angle)) * FLY_SPEED;
    L.ro.rects[i].speed.y = sca_sin(sca_radians(angle)) * FLY_SPEED;
    
    L.ro.rects[i].acc.y = sca_random_range(0, FLY_ACC_Y);
    
    L.ro.rects[i].start_time = L.time;
}

static void fly_away(int i) {
    init_fly(i);
    ro_particle_update_sub(&L.ro, i, 1);
}


//
// public
//

void butterfly_init(const vec2 *positions, int num) {
    assume(num>0, "atleast one butterfly in a level?");
    L.ro = ro_particle_new(num, camera.gl_main,
            r_texture_new_file(12, 2, "res/butterfly.png"));
            
    for(int i=0; i<num; i++) {
        L.ro.rects[i].pose = u_pose_new(
                sca_random_noise(positions[i].x, 4),
                sca_random_noise(positions[i].y, 4),
                16, 16);
        vec3 hsv = {{
            sca_random_range(0, 360),
            sca_random_range(0.25, 0.75),
            sca_random_range(0.75, 1)
        }};
        L.ro.rects[i].color.rgb = vec3_hsv2rgb(hsv);
        init_stay(i);
    }
    
    ro_particle_update(&L.ro);
    
    L.last_color = vec3_set(0.75);
   
    L.save.collected = rhc_malloc_raising(num * sizeof(bool));
    memset(L.save.collected, 0, num * sizeof(bool));
}

void butterfly_kill() {
    ro_particle_kill(&L.ro);
    rhc_free(L.save.collected);
    memset(&L, 0, sizeof(L));
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
        ro_particle_update(&L.ro);
    }
    
    
   
}

void butterfly_render() {
    ro_particle_render(&L.ro, L.time);
}

int butterfly_collected() {
    return L.collected;
}

vec3 butterfly_last_color() {
    return L.last_color;
}

bool butterfly_collect(vec2 position) {
    for(int i=0; i<L.ro.num; i++) {
        if(is_flying(i))
            continue;
        if(vec2_distance(position, u_pose_get_xy(L.ro.rects[i].pose)) <= COLLECT_DISTANCE) {
            fly_away(i);
            L.collected++;
            L.last_color = L.ro.rects[i].color.rgb;
            return true;
        }
    }
    return false;
}

void butterfly_save() {
    for(int i=0; i<L.ro.num; i++) {
        L.save.collected[i] = is_flying(i);
    }
}

void butterfly_load() {
    L.collected = 0;
    L.time = -1000;
    for(int i=0; i<L.ro.num; i++) {
        if(L.save.collected[i]) {
            L.collected++;
            init_fly(i);
        } else {
            init_stay(i);
        }
    }
    L.time = 0;
    L.reset_time = 0;
    ro_particle_update(&L.ro);
    L.last_color = vec3_set(0.75);
}

