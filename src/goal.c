#include "r/ro_single.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "rhc/error.h"
#include "rhc/log.h"
#include "pixelparticles.h"
#include "camera.h"
#include "goal.h"

#define GOAL_OFFSET_Y 8.0

#define FPS 4.0
#define FRAMES 4

#define NUM_PARTICLES 128
#define PARTICLE_SIZE 2.0
#define PARTICLE_SPEED 100.0
#define PARTICLE_TIME 1.5
#define PARTICLE_ALPHA 2.0


//
// private
//

static void emit_particles(float x, float y) {
    rParticleRect_s rects[NUM_PARTICLES];
    for(int i=0; i<NUM_PARTICLES; i++) {
        rects[i] = r_particlerect_new();
        rects[i].pose = u_pose_new(x, y, PARTICLE_SIZE, PARTICLE_SIZE);
        float angle = sca_random_range(
                sca_radians(90-20),
                sca_radians(90+20));
        float speed = sca_random_range(0.1*PARTICLE_SPEED, PARTICLE_SPEED);
        rects[i].speed.x = sca_cos(angle) * speed;
        rects[i].speed.y = sca_sin(angle) * speed;
        rects[i].acc.y = - speed * 0.33;
        
        if(rand()%2==0) {
            rects[i].color.rgb = vec3_set(sca_random_noise(0.9, 0.1));
        } else {
            rects[i].color.rgb = vec3_set(sca_random_noise(0.1, 0.1));
        }
        rects[i].color.a = PARTICLE_ALPHA;
        rects[i].color_speed.a = (float)-PARTICLE_ALPHA/PARTICLE_TIME;
        rects[i].start_time = pixelparticles.time;
    }
    pixelparticles_add(rects, NUM_PARTICLES);
}

static void activate() {
    
}


//
// public
//

Goal *goal_new(vec2 position) {
    Goal *self = rhc_calloc(sizeof *self);
    
    self->L.goal_ro = ro_single_new(
                    r_texture_new_file(4, 2, "res/goal_flag.png"));
    self->L.goal_ro.rect.pose = u_pose_new(
            position.x,
            position.y+GOAL_OFFSET_Y,
            32, 48);

    self->L.goal_ro.rect.sprite.y = 1;
    
    return self;
}

void goal_kill(Goal **self_ptr) {
    Goal *self = *self_ptr;
    if(!self)
        return;
    ro_single_kill(&self->L.goal_ro);
    
    rhc_free(self);
    *self_ptr = NULL;
}

void goal_update(Goal *self, float dtime) {
    self->L.time += dtime;


    float animate_time = sca_mod(self->L.time, FRAMES / FPS);
    int frame = animate_time * FPS;
    self->L.goal_ro.rect.sprite.x = frame;    
}

void goal_render(Goal *self, const mat4 *cam_mat) {
    ro_single_render(&self->L.goal_ro, cam_mat);
}

bool goal_reached(const Goal *self) {
    return self->L.goal_ro.rect.sprite.y < 0.5;
}

vec2 goal_position(const Goal *self) {
    vec2 center = u_pose_get_xy(self->L.goal_ro.rect.pose);
    center.y -= GOAL_OFFSET_Y;
    return center;
}

void goal_activate(Goal *self) {
    if(goal_reached(self))
        return;
    log_info("goal_activate");
    self->L.goal_ro.rect.sprite.y = 0;

    vec2 pos = u_pose_get_xy(self->L.goal_ro.rect.pose);

    pos.y -= GOAL_OFFSET_Y;

    pos.y += 8 + GOAL_OFFSET_Y;
    emit_particles(pos.x, pos.y);
}
