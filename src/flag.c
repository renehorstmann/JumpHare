#include "e/input.h"
#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "rhc/error.h"
#include "rhc/log.h"
#include "pixelparticles.h"
#include "camera.h"
#include "hare.h"
#include "carrot.h"
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

static struct {
    eInput *input;
    RoBatch flag_ro;
    RoBatch btn_ro;
    float time;
    vec2 active_pos;
    
    struct {
        flag_activated_callback_fn cb;
        void *ud;
    } callbacks[FLAG_MAX_CALLBACKS];
    int callbacks_size;
} L;

static bool flag_reached(int index) {
    return L.flag_ro.rects[index].sprite.y < 0.5;
}

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

        rects[i].color.rgb = vec3_set(sca_random_noise(0.9, 0.1));
        rects[i].color.a = PARTICLE_ALPHA;
        rects[i].color_speed.a = (float)-PARTICLE_ALPHA/PARTICLE_TIME;
        rects[i].start_time = pixelparticles.time;
    }
    pixelparticles_add(rects, NUM_PARTICLES);
}

static void activate(int flag_index) {
    log_info("flag: activated %i", flag_index);
    L.flag_ro.rects[flag_index].sprite.y = 0;

    vec2 pos = u_pose_get_xy(L.flag_ro.rects[flag_index].pose);

    pos.y -= FLAG_OFFSET_Y;
    L.active_pos = pos;

    pos.y += 8 + FLAG_OFFSET_Y;
    emit_particles(pos.x, pos.y);

    carrot_eat();
    for(int i=0; i<L.callbacks_size; i++) {
        L.callbacks[i].cb(L.active_pos, L.callbacks[i].ud);
    }
}

static void pointer_callback(ePointer_s pointer, void *user_data) {
    pointer.pos = mat4_mul_vec(camera.matrices_main.v_p_inv, pointer.pos);
    
    for(int i=0; i<L.btn_ro.num; i++) {
        if(L.btn_ro.rects[i].color.a<0.99)
            continue;
        
        if(button_clicked(&L.btn_ro.rects[i], pointer)) {
            activate(i);
        }
    }
}

static void check_key_click() {
    static bool key_was_pressed = false;
    
    for(int i=0; i<L.btn_ro.num; i++) {
        if(L.btn_ro.rects[i].color.a<0.99)
            continue;

        eInputKeys keys = e_input_get_keys(L.input);
        if(keys.enter) {
            button_set_pressed(&L.btn_ro.rects[i], true);
            key_was_pressed = true;
        }
        
        if(key_was_pressed && !keys.enter) {
            key_was_pressed = false;
            if(button_is_pressed(&L.btn_ro.rects[i])) {
                activate(i);
            }
        }
    }
}


//
// public
//

void flag_init(const vec2 *positions, int num, eInput *input) {
    assume(num>=1, "a level needs at least one flag");
    L.input = input;

    e_input_register_pointer_event(input, pointer_callback, NULL);

    L.active_pos = (vec2) {{NAN, NAN}};

    L.flag_ro = ro_batch_new(num,
                    r_texture_new_file(4, 2, "res/flag.png"));
    for(int i=0; i<num; i++) {
        L.flag_ro.rects[i].pose = u_pose_new(
                positions[i].x,
                positions[i].y+FLAG_OFFSET_Y,
                32, 48);

        L.flag_ro.rects[i].sprite.y = 1;
    }
    ro_batch_update(&L.flag_ro);
    
    
    L.btn_ro = ro_batch_new(num,
                    r_texture_new_file(2, 1, "res/carrot_btn.png"));
    for(int i=0; i<num; i++) {
        
        L.btn_ro.rects[i].pose = u_pose_new(
                positions[i].x,
                positions[i].y+56,
                32, 32);
        L.btn_ro.rects[i].color.a = 1;
    }
    ro_batch_update(&L.btn_ro);
}

void flag_kill() {
    e_input_unregister_pointer_event(L.input,
            pointer_callback);
    ro_batch_kill(&L.flag_ro);
    ro_batch_kill(&L.btn_ro);
    memset(&L, 0, sizeof(L));
}

void flag_update(float dtime) {
    L.time += dtime;

    check_key_click();

    float animate_time = sca_mod(L.time, FRAMES / FPS);
    int frame = animate_time * FPS;
    for(int i=0; i<L.flag_ro.num; i++)
        L.flag_ro.rects[i].sprite.x = frame;
    
    ro_batch_update(&L.flag_ro);
    
    
    vec2 hare_pos = hare.pos;
    for(int i=0; i<L.flag_ro.num; i++) {
        if(flag_reached(i) || carrot_collected() == 0) {
            L.btn_ro.rects[i].color.a = 0;
            continue;
        }
        
        
        vec2 center = u_pose_get_xy(L.flag_ro.rects[i].pose);
        center.y -= FLAG_OFFSET_Y;
        float dist = vec2_distance(hare_pos, center);
        if(dist < MIN_DIST) {
            L.btn_ro.rects[i].color.a = 1;
        } else {
            float t = dist / MAX_DIST;
            L.btn_ro.rects[i].color.a = sca_clamp(sca_mix(1, 0, t), 0, 1);

            button_set_pressed(&L.btn_ro.rects[i], false);
        }
    }
    
    ro_batch_update(&L.btn_ro);
}

void flag_render() {
    ro_batch_render(&L.flag_ro, camera.gl_main);
    ro_batch_render(&L.btn_ro, camera.gl_main);
}

vec2 flag_active_position() {
    return L.active_pos;
}

void flag_register_callback(flag_activated_callback_fn cb, void *ud) {
    assume(L.callbacks_size < FLAG_MAX_CALLBACKS, "too many");
    L.callbacks[L.callbacks_size].cb = cb;
    L.callbacks[L.callbacks_size++].ud = ud;
}
