#include "r/ro_batch.h"
#include "r/ro_particle.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "camera.h"
#include "carrot.h"

#define FPS 3.0
#define FRAMES 4

#define COLLECT_SHRINK_SPEED 96

#define NUM_PARTICLES 256
#define PARTICLE_TIME 1.0

static const vec3 PARTICLE_COLOR = {{1, 0.65, 0}};


static struct {
    rRoBatch ro;
    rRoParticle collect_ro;
    bool collected[3];
    float time;
} L;


static void collect_particles(float x, float y) {
    for(int i=0; i<L.collect_ro.num; i++) {
        rParticleRect_s *r = &L.collect_ro.rects[i];
        r->pose = u_pose_new(x, y, 1, 1);
        float angle = sca_random_range(0, 2*M_PI);
        float speed = sca_random_range(10, 100);
        r->speed.x = sca_cos(angle) * speed;
        r->speed.y = sca_sin(angle) * speed;
        r->acc.xy = vec2_scale(r->speed.xy, -0.5/PARTICLE_TIME);
        r->color.rgb = vec3_random_noise_vec(
                PARTICLE_COLOR,
                (vec3) {{0.2, 0.2, 0.2}});
        r->color.a = 1;
;       r->start_time = L.time;
    }
    r_ro_particle_update(&L.collect_ro);
}

void carrot_init(const vec2 *positions_3) {
    r_ro_batch_init(&L.ro, 3, camera.gl_main,
            r_texture_init_file("res/carrot.png", NULL));
            
    for(int i=0; i<3; i++) {
        L.ro.rects[i].pose = u_pose_new(
                positions_3[i].x,
                positions_3[i].y,
                16, 32);
                
       u_pose_set_w(&L.ro.rects[i].uv, 1.0/FRAMES); 
    }
    
    r_ro_batch_update(&L.ro);
    
    
    GLuint white_pixel = r_texture_init(1, 1, (u_int8_t[]) {255, 255, 255, 255});
    r_ro_particle_init(&L.collect_ro, NUM_PARTICLES, camera.gl_main, white_pixel);
    
    for(int i=0; i<L.collect_ro.num; i++) {
        L.collect_ro.rects[i].pose = u_pose_new_hidden();
        L.collect_ro.rects[i].color = vec4_set(0);
        L.collect_ro.rects[i].color_speed.a = -1.0/PARTICLE_TIME;
    }
    
    r_ro_particle_update(&L.collect_ro);
}

void carrot_kill() {
    r_ro_batch_kill(&L.ro);
    memset(&L, 0, sizeof(L));
}

void carrot_update(float dtime) {
    L.time += dtime;
    float animate_time = sca_mod(L.time, FRAMES / FPS);
    int frame = animate_time * FPS;
    float u = (float) frame / FRAMES;
    for(int i=0; i<3; i++)
        u_pose_set_x(&L.ro.rects[i].uv, u);
          
    for(int i=0; i<3; i++) {
        if(!L.collected[i])
            continue;
        
        float h = u_pose_get_h(L.ro.rects[i].pose);
        h = sca_max(0, h - COLLECT_SHRINK_SPEED * dtime);
        
        u_pose_set_size(&L.ro.rects[i].pose, h/2, h);
    }
    
    r_ro_batch_update(&L.ro);
}

void carrot_render() {
    r_ro_batch_render(&L.ro);
    r_ro_particle_render(&L.collect_ro, L.time);
}

bool carrot_collect(float x, float y) {
    for(int i=0; i<3; i++) {
        if(L.collected[i])
            continue;
        if(u_pose_aa_contains(L.ro.rects[i].pose, 
                (vec2){{x, y}})) {
            L.collected[i] = true;
            float cx = u_pose_get_x(L.ro.rects[i].pose);
            float cy = u_pose_get_y(L.ro.rects[i].pose);
            collect_particles(cx, cy);
            return true;
        }
    }
    return false;
}
