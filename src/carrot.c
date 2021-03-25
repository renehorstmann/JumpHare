#include "r/ro_batch.h"
#include "r/ro_particle.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "utilc/assume.h"
#include "camera.h"
#include "hud_camera.h"
#include "carrot.h"

#define FPS 3.0
#define FRAMES 4

#define COLLECT_SHRINK_SPEED 96

#define NUM_PARTICLES 256
#define PARTICLE_SIZE 2.0
#define PARTICLE_SPEED 100.0
#define PARTICLE_TIME 1.0

static const vec3 PARTICLE_COLOR = {{1, 0.65, 0}};


static struct {
    rRoBatch carrot_ro;
    rRoBatch cnt_ro;
    rRoParticle particle_ro;
    bool collected[3];
    int collected_cnt;
    float time;
    
    struct {
        bool collected[3];
    } save;
} L;


static void collect_particles(float x, float y) {
    for(int i=0; i<L.particle_ro.num; i++) {
        rParticleRect_s *r = &L.particle_ro.rects[i];
        r->pose = u_pose_new(x, y, PARTICLE_SIZE, PARTICLE_SIZE);
        float angle = sca_random_range(0, 2*M_PI);
        float speed = sca_random_range(0.1*PARTICLE_SPEED, PARTICLE_SPEED);
        r->speed.x = sca_cos(angle) * speed;
        r->speed.y = sca_sin(angle) * speed;
        r->acc.xy = vec2_scale(r->speed.xy, -0.5/PARTICLE_TIME);
        r->color.rgb = vec3_random_noise_vec(
                PARTICLE_COLOR,
                (vec3) {{0.2, 0.2, 0.2}});
        r->color.a = 1;
;       r->start_time = L.time;
    }
    r_ro_particle_update(&L.particle_ro);
}

static void collect_cnt(float x, float y) {
    assume(L.collected_cnt < 3, "already all collected?");
    
    L.cnt_ro.rects[L.collected_cnt].pose = u_pose_new_aa(
            hud_camera_left() + 2 + L.collected_cnt * 8,
            hud_camera_top() - 2,
            8, 16);
    r_ro_batch_update(&L.cnt_ro);
    
    L.collected_cnt++;
}

void carrot_init(const vec2 *positions_3) {
    
    // in game carrots
    r_ro_batch_init(&L.carrot_ro, 3, camera.gl_main,
            r_texture_init_file("res/carrot.png", NULL));
            
    for(int i=0; i<3; i++) {
        L.carrot_ro.rects[i].pose = u_pose_new(
                positions_3[i].x,
                positions_3[i].y,
                16, 32);
                
       u_pose_set_w(&L.carrot_ro.rects[i].uv, 1.0/FRAMES); 
    }
    
    r_ro_batch_update(&L.carrot_ro);
    
    
    // mini hud carrot
    r_ro_batch_init(&L.cnt_ro, 3, hud_camera.gl,
            r_texture_init_file("res/carrot_mini.png", NULL));
            
    for(int i=0; i<3; i++)
        L.cnt_ro.rects[i].pose = u_pose_new_hidden();
    r_ro_batch_update(&L.cnt_ro);
    
    
    // particles
    GLuint white_pixel = r_texture_init(1, 1, (u_int8_t[]) {255, 255, 255, 255});
    r_ro_particle_init(&L.particle_ro, NUM_PARTICLES, camera.gl_main, white_pixel);
    
    for(int i=0; i<L.particle_ro.num; i++) {
        L.particle_ro.rects[i].pose = u_pose_new_hidden();
        L.particle_ro.rects[i].color = vec4_set(0);
        L.particle_ro.rects[i].color_speed.a = -1.0/PARTICLE_TIME;
    }
    
    r_ro_particle_update(&L.particle_ro);
}

void carrot_kill() {
    r_ro_batch_kill(&L.carrot_ro);
    memset(&L, 0, sizeof(L));
}

void carrot_update(float dtime) {
    L.time += dtime;
    float animate_time = sca_mod(L.time, FRAMES / FPS);
    int frame = animate_time * FPS;
    float u = (float) frame / FRAMES;
    for(int i=0; i<3; i++)
        u_pose_set_x(&L.carrot_ro.rects[i].uv, u);
          
    for(int i=0; i<3; i++) {
        if(!L.collected[i])
            continue;
        
        float h = u_pose_get_h(L.carrot_ro.rects[i].pose);
        h = sca_max(0, h - COLLECT_SHRINK_SPEED * dtime);
        
        u_pose_set_size(&L.carrot_ro.rects[i].pose, h/2, h);
    }
    
    r_ro_batch_update(&L.carrot_ro);
}

void carrot_render() {
    r_ro_particle_render(&L.particle_ro, L.time);
    r_ro_batch_render(&L.carrot_ro);
    r_ro_batch_render(&L.cnt_ro);
}

bool carrot_collect(vec2 position) {
    for(int i=0; i<3; i++) {
        if(L.collected[i])
            continue;
        if(u_pose_aa_contains(L.carrot_ro.rects[i].pose, position)) {
            L.collected[i] = true;
            vec2 cxy = u_pose_get_xy(L.carrot_ro.rects[i].pose);
            collect_particles(cxy.x, cxy.y);
            collect_cnt(cxy.x, cxy.y);
            return true;
        }
    }
    return false;
}

void carrot_save() {
    memcpy(L.save.collected, L.collected, sizeof(L.collected));
}

void carrot_load() {
    memcpy(L.collected, L.save.collected, sizeof(L.collected));
    L.collected_cnt = 0;
    
    
    // particles
    for(int i=0; i<L.particle_ro.num; i++) {
        L.particle_ro.rects[i].pose = u_pose_new_hidden();
    }
    r_ro_particle_update(&L.particle_ro);
    
    // hud carrots
    for(int i=0; i<3; i++) {
        L.cnt_ro.rects[i].pose = u_pose_new_hidden();
    }
    r_ro_batch_update(&L.cnt_ro);
    for(int i=0; i<3; i++) {
        if(L.collected[i])
            collect_cnt(-1, -1);
    }
    
    // in game carrots
    for(int i=0; i<3; i++) {
        if(L.collected[i]) {
            u_pose_set_size(&L.carrot_ro.rects[i].pose, 0, 0);
        } else {
            u_pose_set_size_angle(&L.carrot_ro.rects[i].pose, 16, 32, 0);
        }
    }
    r_ro_batch_update(&L.carrot_ro);
}

