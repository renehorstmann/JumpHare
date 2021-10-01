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

static bool is_flying(const Butterfly *self, int i) {
    return self->L.ro.rects[i].sprite.y > 0.5;
}

static void init_stay(Butterfly *self, int i) {
    self->L.ro.rects[i].sprite.y = 0;
    self->L.ro.rects[i].sprite_speed.x = CHILL_FPS;
    int flip = rand()%2;
    float uv_w = -1;
    if(flip) {
        uv_w = 1;
    }
    self->L.ro.rects[i].uv = u_pose_new(0, 0, uv_w, 1);
    self->L.ro.rects[i].speed.x = 0;
    self->L.ro.rects[i].speed.y = 0;
    self->L.ro.rects[i].acc.y = 0;
    self->L.ro.rects[i].start_time = self->L.time;
}

static void init_fly(Butterfly *self, int i) {
    self->L.ro.rects[i].sprite.y = 1;
    self->L.ro.rects[i].sprite_speed.x = FLY_FPS;
    float angle = sca_random_range(15, 60);
    int flip = rand()%2;
    float uv_w = -1;
    if(flip) {
        angle = 180 - angle;
        uv_w = 1;
    }
    self->L.ro.rects[i].uv = u_pose_new(0, 0, uv_w, 1);
    self->L.ro.rects[i].speed.x = sca_cos(sca_radians(angle)) * FLY_SPEED;
    self->L.ro.rects[i].speed.y = sca_sin(sca_radians(angle)) * FLY_SPEED;
    
    self->L.ro.rects[i].acc.y = sca_random_range(0, FLY_ACC_Y);
    
    self->L.ro.rects[i].start_time = self->L.time;
}

static void fly_away(Butterfly *self, int i) {
    init_fly(self, i);
    ro_particle_update_sub(&self->L.ro, i, 1);
}


//
// public
//

Butterfly *butterfly_new(const vec2 *positions, int num) {
    assume(num>0, "atleast one butterfly in a level?");
    
    Butterfly *self = rhc_calloc_raising(sizeof *self);
    
    self->L.ro = ro_particle_new(num,
            r_texture_new_file(12, 2, "res/butterfly.png"));
            
    for(int i=0; i<num; i++) {
        self->L.ro.rects[i].pose = u_pose_new(
                sca_random_noise(positions[i].x, 4),
                sca_random_noise(positions[i].y, 4),
                16, 16);
        vec3 hsv = {{
            sca_random_range(0, 360),
            sca_random_range(0.25, 0.75),
            sca_random_range(0.75, 1)
        }};
        self->L.ro.rects[i].color.rgb = vec3_hsv2rgb(hsv);
        init_stay(self, i);
    }
    
    ro_particle_update(&self->L.ro);
    
    self->RO.last_color = vec3_set(0.75);
   
    self->L.save.collected = rhc_calloc_raising(num * sizeof(bool));
    
    return self;
}

void butterfly_kill(Butterfly **self_ptr) {
    Butterfly *self = *self_ptr;
    if(!self)
        return;
    ro_particle_kill(&self->L.ro);
    rhc_free(self->L.save.collected);
    
    rhc_free(self);
    *self_ptr = NULL;
}

void butterfly_update(Butterfly *self, float dtime) {
    self->L.time += dtime;
    self->L.reset_time += dtime;
    
    if(self->L.reset_time >= RESET_TIME) {
        self->L.reset_time -= RESET_TIME;
        for(int i=0; i<self->L.ro.num; i++) {
            if(is_flying(self, i))
                continue;
            self->L.ro.rects[i].start_time = self->L.time;
        }
        ro_particle_update(&self->L.ro);
    }
    
    
   
}

void butterfly_render(Butterfly *self, const mat4 *cam_mat) {
    ro_particle_render(&self->L.ro, self->L.time, cam_mat);
}

bool butterfly_collect(Butterfly *self, vec2 position) {
    for(int i=0; i<self->L.ro.num; i++) {
        if(is_flying(self, i))
            continue;
        if(vec2_distance(position, u_pose_get_xy(self->L.ro.rects[i].pose)) <= COLLECT_DISTANCE) {
            fly_away(self, i);
            self->RO.collected++;
            self->RO.last_color = self->L.ro.rects[i].color.rgb;
            return true;
        }
    }
    return false;
}

void butterfly_save(Butterfly *self) {
    for(int i=0; i<self->L.ro.num; i++) {
        self->L.save.collected[i] = is_flying(self, i);
    }
}

void butterfly_load(Butterfly *self) {
    self->RO.collected = 0;
    self->L.time = -1000;
    for(int i=0; i<self->L.ro.num; i++) {
        if(self->L.save.collected[i]) {
            self->RO.collected++;
            init_fly(self, i);
        } else {
            init_stay(self, i);
        }
    }
    self->L.time = 0;
    self->L.reset_time = 0;
    ro_particle_update(&self->L.ro);
    self->RO.last_color = vec3_set(0.75);
}

