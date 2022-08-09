#include "e/window.h"
#include "r/ro_particle.h"
#include "u/pose.h"
#include "m/float.h"
#include "m/utils/random.h"
#include "m/utils/color.h"
#include "butterfly.h"

#define CHILL_FPS 6.0
#define FLY_FPS 8.0
#define FRAMES 12
#define FLY_SPEED 75
#define FLY_ACC_Y 20
#define COLLECT_DISTANCE 16

static const float RESET_TIME = 5.0 * FRAMES / CHILL_FPS;


struct Butterfly_Globals butterfly;


static struct {
    RoParticle ro;
    float reset_time;
    float collected_time;

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
    float w_flip = 1;
    if(flip) {
        w_flip = -1;
    }
    // m00 := width for an aa pose
    L.ro.rects[i].pose.m00 = sca_abs(L.ro.rects[i].pose.m00) * w_flip;
    L.ro.rects[i].speed.x = 0;
    L.ro.rects[i].speed.y = 0;
    L.ro.rects[i].acc.y = 0;
    L.ro.rects[i].start_time_ms = e_window.time_ms;
}

static void init_fly(int i) {
    L.ro.rects[i].sprite.y = 1;
    L.ro.rects[i].sprite_speed.x = FLY_FPS;
    float angle = sca_random_range(15, 60);
    int flip = rand()%2;
    float w_flip = -1;
    if(flip) {
        w_flip = 1;
        angle = 180 - angle;
    }
    // m00 := width for an aa pose
    L.ro.rects[i].pose.m00 = sca_abs(L.ro.rects[i].pose.m00) * w_flip;
    L.ro.rects[i].speed.x = sca_cos(sca_radians(angle)) * FLY_SPEED;
    L.ro.rects[i].speed.y = sca_sin(sca_radians(angle)) * FLY_SPEED;
    
    L.ro.rects[i].acc.y = sca_random_range(0, FLY_ACC_Y);

    L.ro.rects[i].start_time_ms = e_window.time_ms;
}

static void fly_away(int i) {
    init_fly(i);
}


//
// public
//

void butterfly_init(const vec2 *positions, int num) {
    s_assume(num>0, "atleast one butterfly in a level?");

    L.ro = ro_particle_new(num,
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
    
    butterfly.RO.last_color = vec3_set(0.75);
   
    L.save.collected = s_new0(bool, num);
}

void butterfly_kill() {
    ro_particle_kill(&L.ro);
    s_free(L.save.collected);
    memset(&L, 0, sizeof L);
    memset(&butterfly, 0, sizeof butterfly);
}

void butterfly_update(float dtime) {
    L.reset_time += dtime;
    
    if(L.reset_time >= RESET_TIME) {
        L.reset_time -= RESET_TIME;
        for(int i=0; i<L.ro.num; i++) {
            if(is_flying(i))
                continue;
            L.ro.rects[i].start_time_ms = e_window.time_ms;
        }
    }
}

void butterfly_render(const mat4 *cam_mat) {
    ro_particle_render(&L.ro, e_window.time_ms, cam_mat);
}

bool butterfly_collect(vec2 position) {
    for(int i=0; i<L.ro.num; i++) {
        if(is_flying(i))
            continue;
        if(vec2_distance(position, u_pose_get_xy(L.ro.rects[i].pose)) <= COLLECT_DISTANCE) {
            fly_away(i);
            butterfly.RO.collected++;
            butterfly.RO.last_color = L.ro.rects[i].color.rgb;
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
    butterfly.RO.collected = 0;
    for(int i=0; i<L.ro.num; i++) {
        if(L.save.collected[i]) {
            butterfly.RO.collected++;
            init_fly(i);
        } else {
            init_stay(i);
        }
    }
    L.reset_time = 0;
    butterfly.RO.last_color = vec3_set(0.75);
}

