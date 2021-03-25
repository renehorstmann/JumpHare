#include "e/input.h"
#include "r/ro_batch.h"
#include "r/ro_particle.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "utilc/assume.h"
#include "camera.h"
#include "hare.h"
#include "carrot.h"
#include "button.h"
#include "flag.h"

#define FPS 4.0
#define FRAMES 4

#define MAX_DIST 40.0
#define MIN_DIST 15.0

static struct {
    rRoBatch flag_ro;
    rRoBatch btn_ro;
    float time;
} L;


static void pointer_callback(ePointer_s pointer, void *user_data) {
    pointer.pos = mat4_mul_vec(camera.matrices_main.v_p_inv, pointer.pos);
    
    for(int i=0; i<L.btn_ro.num; i++) {
        if(L.btn_ro.rects[i].color.a<0.99)
            continue;
        
        if(button_clicked(&L.btn_ro.rects[i], pointer)) {
            u_pose_set_y(&L.flag_ro.rects[i].uv, 0);
        }
    }
}


void flag_init(const vec2 *positions, int num) {
    e_input_register_pointer_event(pointer_callback, NULL);
    
    r_ro_batch_init(&L.flag_ro, num, camera.gl_main, 
            r_texture_init_file("res/flag.png", NULL));
    for(int i=0; i<num; i++) {
        L.flag_ro.rects[i].pose = u_pose_new(
                positions[i].x,
                positions[i].y+8,
                32, 48);
                
        u_pose_set_size(&L.flag_ro.rects[i].uv, 1.0/FRAMES, 0.5);
        u_pose_set_y(&L.flag_ro.rects[i].uv, 0.5);   
    }
    r_ro_batch_update(&L.flag_ro);
    
    
    r_ro_batch_init(&L.btn_ro, num, camera.gl_main,
            r_texture_init_file("res/carrot_btn.png", NULL));
    for(int i=0; i<num; i++) {
        
        L.btn_ro.rects[i].pose = u_pose_new(
                positions[i].x,
                positions[i].y+56,
                32, 32);
        button_init_uv(&L.btn_ro.rects[i]);
        L.btn_ro.rects[i].color.a = 1;
    }
    r_ro_batch_update(&L.btn_ro);
}

void flag_kill() {
    e_input_unregister_pointer_event(pointer_callback);
    r_ro_batch_kill(&L.flag_ro);
    r_ro_batch_kill(&L.btn_ro);
}

void flag_update(float dtime) {
    L.time += dtime;
    float animate_time = sca_mod(L.time, FRAMES / FPS);
    int frame = animate_time * FPS;
    float u = (float) frame / FRAMES;
    for(int i=0; i<L.flag_ro.num; i++)
        u_pose_set_x(&L.flag_ro.rects[i].uv, u);
    
    r_ro_batch_update(&L.flag_ro);
    
    
    vec2 hare_pos = hare_position();
    for(int i=0; i<L.flag_ro.num; i++) {
        if(u_pose_get_y(L.flag_ro.rects[i].uv) < 0.25
                || carrot_collected() == 0) {
            L.btn_ro.rects[i].color.a = 0;
            continue;
        }
        
        
        vec2 center = u_pose_get_xy(L.flag_ro.rects[i].pose);
        center.y -= 8;
        float dist = vec2_distance(hare_pos, center);        if(dist < MIN_DIST) {
            L.btn_ro.rects[i].color.a = 1;
        } else {
            L.btn_ro.rects[i].color.a 
                    = sca_max(0,
                    1-(dist-MIN_DIST) / (MAX_DIST-MIN_DIST)
            );
            button_set_pressed(&L.btn_ro.rects[i], false);
        }
    }
    
    r_ro_batch_update(&L.btn_ro);
}

void flag_render() {
    r_ro_batch_render(&L.flag_ro);
    r_ro_batch_render(&L.btn_ro);
}
