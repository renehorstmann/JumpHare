#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "rhc/log.h"
#include "rhc/error.h"
#include "carrot.h"

#define FPS 3.0
#define FRAMES 4

#define COLLECT_SHRINK_SPEED 96

#define NUM_PARTICLES 256
#define PARTICLE_SIZE 2.0
#define PARTICLE_SPEED 100.0
#define PARTICLE_TIME 1.0
#define PARTICLE_ALPHA 1.5

static const vec3 PARTICLE_COLOR = {{1, 0.65, 0}};


//
// private
//

static void emit_particles(Carrot *self, float x, float y) {
    rParticleRect_s rects[NUM_PARTICLES];
    for(int i=0; i<NUM_PARTICLES; i++) {
        rects[i] = r_particlerect_new();
        rects[i].pose = u_pose_new(x, y, PARTICLE_SIZE, PARTICLE_SIZE);
        float angle = sca_random_range(0, 2*M_PI);
        float speed = sca_random_range(0.1*PARTICLE_SPEED, PARTICLE_SPEED);
        rects[i].speed.x = sca_cos(angle) * speed;
        rects[i].speed.y = sca_sin(angle) * speed;
        rects[i].acc.xy = vec2_scale(rects[i].speed.xy, -0.5 / PARTICLE_TIME);
        rects[i].color.rgb = vec3_random_noise_vec(
                PARTICLE_COLOR,
                (vec3) {{0.2, 0.2, 0.2}});
        rects[i].color.a = PARTICLE_ALPHA;
        rects[i].color_speed.a = (float) -PARTICLE_ALPHA / PARTICLE_TIME;
        rects[i].start_time = self->particles_ref->time;
    }
    pixelparticles_add(self->particles_ref, rects, NUM_PARTICLES);
}



//
// public
//

Carrot *carrot_new(PixelParticles *particles, const vec2 *positions_3) {
    Carrot *self = rhc_calloc(sizeof *self);
    
    self->particles_ref = particles;
    
    // in game carrots
    self->L.carrot_ro = ro_batch_new(3,
                    r_texture_new_file(4, 1, "res/carrot.png"));
            
    for(int i=0; i<3; i++) {
        self->L.carrot_ro.rects[i].pose = u_pose_new(
                positions_3[i].x,
                positions_3[i].y,
                16, 32);
    }
    ro_batch_update(&self->L.carrot_ro);

    return self;
}

void carrot_kill(Carrot **self_ptr) {
    Carrot *self = *self_ptr;
    if(!self)
        return;
    ro_batch_kill(&self->L.carrot_ro);
    
    rhc_free(self);
    *self_ptr = NULL;
}

void carrot_update(Carrot *self, float dtime) {
    self->L.time += dtime;
    float animate_time = sca_mod(self->L.time, FRAMES / FPS);
    int frame = animate_time * FPS;
    for(int i=0; i<3; i++)
        self->L.carrot_ro.rects[i].sprite.x = frame;

    for(int i=0; i<3; i++) {
        if(!self->L.collected[i])
            continue;
        
        float h = u_pose_get_h(self->L.carrot_ro.rects[i].pose);
        h = sca_max(0, h - COLLECT_SHRINK_SPEED * dtime);
        
        u_pose_set_size(&self->L.carrot_ro.rects[i].pose, h/2, h);
    }
    
    ro_batch_update(&self->L.carrot_ro);
}

void carrot_render(const Carrot *self, const mat4 *cam_mat) {
    ro_batch_render(&self->L.carrot_ro, cam_mat);
}


bool carrot_collect(Carrot *self, vec2 position) {
    for(int i=0; i<3; i++) {
        if(self->L.collected[i])
            continue;
        if(u_pose_aa_contains(self->L.carrot_ro.rects[i].pose, position)) {
            log_info("carrot: collected %i", i);
            self->L.collected[i] = true;
            vec2 cxy = u_pose_get_xy(self->L.carrot_ro.rects[i].pose);
            emit_particles(self, cxy.x, cxy.y);
            self->RO.collected++;
            return true;
        }
    }
    return false;
}


void carrot_eat(Carrot *self) {
    if(self->RO.collected <= 0 || self->RO.eaten >= self->RO.collected) {
        log_error("carrot: failed to eat, collected <= 0");
        return;
    }
    self->RO.eaten++;
}

void carrot_save(Carrot *self) {
    memcpy(self->L.save.collected, self->L.collected, sizeof(self->L.collected));
    self->L.save.collected_cnt = self->RO.collected;
    self->L.save.eaten_cnt = self->RO.eaten;
}

void carrot_load(Carrot *self) {
    memcpy(self->L.collected, self->L.save.collected, sizeof(self->L.collected));
    self->RO.collected = self->L.save.collected_cnt;
    self->RO.eaten = self->L.save.eaten_cnt;
    
    // in game carrots
    for(int i=0; i<3; i++) {
        if(self->L.collected[i]) {
            u_pose_set_size(&self->L.carrot_ro.rects[i].pose, 0, 0);
        } else {
            u_pose_set_size_angle(&self->L.carrot_ro.rects[i].pose, 16, 32, 0);
        }
    }
    ro_batch_update(&self->L.carrot_ro);
}

