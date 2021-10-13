#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "rhc/error.h"
#include "rhc/log.h"
#include "button.h"
#include "flag.h"

#define FLAG_OFFSET_Y 8.0

#define FPS 4.0
#define FRAMES 4

#define MAX_DIST 40.0
#define MIN_DIST 15.0

#define NUM_PARTICLES 128
#define PARTICLE_SIZE 2.0
#define PARTICLE_SPEED 100.0
#define PARTICLE_TIME 1.5
#define PARTICLE_ALPHA 2.0


//
// private
//

static bool flag_reached(const Flag *self, int index) {
    return self->L.flag_ro.rects[index].sprite.y < 0.5;
}

static void emit_particles(Flag *self, float x, float y) {
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

        rects[i].color.rgb = vec3_set(sca_random_noise(0.9, 0.1));
        rects[i].color.a = PARTICLE_ALPHA;
        rects[i].color_speed.a = (float)-PARTICLE_ALPHA/PARTICLE_TIME;
        rects[i].start_time = self->carrot_ref->particles_ref->time;
    }
    pixelparticles_add(self->carrot_ref->particles_ref, rects, NUM_PARTICLES);
}

static void activate(Flag *self, int flag_index) {
    log_info("flag: activated %i", flag_index);
    self->L.flag_ro.rects[flag_index].sprite.y = 0;

    vec2 pos = u_pose_get_xy(self->L.flag_ro.rects[flag_index].pose);

    pos.y -= FLAG_OFFSET_Y;
    self->RO.active_pos = pos;

    pos.y += 8 + FLAG_OFFSET_Y;
    emit_particles(self, pos.x, pos.y);

    carrot_eat(self->carrot_ref);
    for(int i=0; i<self->L.callbacks_size; i++) {
        self->L.callbacks[i].cb(self->RO.active_pos, self->L.callbacks[i].ud);
    }
}

static void pointer_callback(ePointer_s pointer, void *user_data) {
    Flag *self = user_data;
    pointer.pos = mat4_mul_vec(self->cam_ref->matrices_main.v_p_inv, pointer.pos);
    
    for(int i=0; i<self->L.btn_ro.num; i++) {
        if(self->L.btn_ro.rects[i].color.a<0.99)
            continue;
        
        if(button_clicked(&self->L.btn_ro.rects[i], pointer)) {
            activate(self, i);
        }
    }
}

static void check_key_click(Flag *self) {
    static bool key_was_pressed = false;
    
    for(int i=0; i<self->L.btn_ro.num; i++) {
        if(self->L.btn_ro.rects[i].color.a<0.99)
            continue;

        eInputKeys keys = e_input_get_keys(self->input_ref);
        if(keys.enter) {
            button_set_pressed(&self->L.btn_ro.rects[i], true);
            key_was_pressed = true;
        }
        
        if(key_was_pressed && !keys.enter) {
            key_was_pressed = false;
            if(button_is_pressed(&self->L.btn_ro.rects[i])) {
                activate(self, i);
            }
        }
    }
}


//
// public
//

Flag *flag_new(const vec2 *positions, int num, const Camera_s *cam, Carrot *carrot,  eInput *input) {
    assume(num>=1, "a level needs at least one flag");
    
    Flag *self = rhc_calloc(sizeof *self);
    
    self->input_ref = input;
    self->cam_ref = cam;
    self->carrot_ref = carrot;

    e_input_register_pointer_event(input, pointer_callback, NULL);

    self->RO.active_pos = (vec2) {{NAN, NAN}};

    self->L.flag_ro = ro_batch_new(num,
                    r_texture_new_file(4, 2, "res/flag.png"));
    for(int i=0; i<num; i++) {
        self->L.flag_ro.rects[i].pose = u_pose_new(
                positions[i].x,
                positions[i].y+FLAG_OFFSET_Y,
                32, 48);

        self->L.flag_ro.rects[i].sprite.y = 1;
    }
    ro_batch_update(&self->L.flag_ro);
    
    
    self->L.btn_ro = ro_batch_new(num,
                    r_texture_new_file(2, 1, "res/carrot_btn.png"));
    for(int i=0; i<num; i++) {
        
        self->L.btn_ro.rects[i].pose = u_pose_new(
                positions[i].x,
                positions[i].y+56,
                32, 32);
        self->L.btn_ro.rects[i].color.a = 1;
    }
    ro_batch_update(&self->L.btn_ro);
    
    return self;
}

void flag_kill(Flag **self_ptr) {
    Flag *self = *self_ptr;
    if(!self)
        return;
    e_input_unregister_pointer_event(self->input_ref,
            pointer_callback);
    ro_batch_kill(&self->L.flag_ro);
    ro_batch_kill(&self->L.btn_ro);
    
    rhc_free(self);
    *self_ptr = NULL;
}

void flag_update(Flag *self, const Hare *hare, float dtime) {
    self->L.time += dtime;

    check_key_click(self);

    float animate_time = sca_mod(self->L.time, FRAMES / FPS);
    int frame = animate_time * FPS;
    for(int i=0; i<self->L.flag_ro.num; i++)
        self->L.flag_ro.rects[i].sprite.x = frame;
    
    ro_batch_update(&self->L.flag_ro);
    
    
    vec2 hare_pos = hare->pos;
    for(int i=0; i<self->L.flag_ro.num; i++) {
        if(flag_reached(self, i) || self->carrot_ref->RO.collected == 0) {
            self->L.btn_ro.rects[i].color.a = 0;
            continue;
        }
        
        
        vec2 center = u_pose_get_xy(self->L.flag_ro.rects[i].pose);
        center.y -= FLAG_OFFSET_Y;
        float dist = vec2_distance(hare_pos, center);
        if(dist < MIN_DIST) {
            self->L.btn_ro.rects[i].color.a = 1;
        } else {
            float t = dist / MAX_DIST;
            self->L.btn_ro.rects[i].color.a = sca_clamp(sca_mix(1, 0, t), 0, 1);

            button_set_pressed(&self->L.btn_ro.rects[i], false);
        }
    }
    
    ro_batch_update(&self->L.btn_ro);
}

void flag_render(Flag *self, const mat4 *cam_mat) {
    ro_batch_render(&self->L.flag_ro, cam_mat);
    ro_batch_render(&self->L.btn_ro, cam_mat);
}


void flag_register_callback(Flag *self, flag_activated_callback_fn cb, void *ud) {
    assume(self->L.callbacks_size < FLAG_MAX_CALLBACKS, "too many");
    self->L.callbacks[self->L.callbacks_size].cb = cb;
    self->L.callbacks[self->L.callbacks_size++].ud = ud;
}
