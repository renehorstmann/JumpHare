#include "r/ro_batch.h"
#include "r/ro_particle.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "rhc/log.h"
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
#define PARTICLE_ALPHA 1.5

static const vec3 PARTICLE_COLOR = {{1, 0.65, 0}};


static struct {
    RoBatch carrot_ro;
    RoBatch cnt_ro;
    RoParticle particle_ro;
    bool collected[3];
    int collected_cnt;
    float time;
    
    struct {
        bool collected[3];
        int collected_cnt;
    } save;
} L;


static void emit_particles(float x, float y) {
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
        r->color.a = PARTICLE_ALPHA;
        r->start_time = L.time;
    }
    ro_particle_update(&L.particle_ro);
}

static void update_cnt() {
    if(L.collected_cnt<0) {
        L.collected_cnt = 0;
        log_wtf("carrot: collected_cnt < 0?");
    } else if(L.collected_cnt>3) {
        L.collected_cnt = 3;
        log_wtf("carrot: collected_cnt > 3?");
    }
    
    for(int i=0; i<L.collected_cnt; i++) {
         L.cnt_ro.rects[i].pose = u_pose_new_aa(
            camera_left() + 2 + i * 8,
            camera_top() - 2,
            8, 16);
    }
    for(int i=L.collected_cnt; i<3; i++) {
        L.cnt_ro.rects[i].pose = u_pose_new_hidden();
    }
    ro_batch_update(&L.cnt_ro);
}


void carrot_init(const vec2 *positions_3) {
    
    // in game carrots
    L.carrot_ro = ro_batch_new(3, camera.gl_main,
                    r_texture_new_file(4, 1, "res/carrot.png"));
            
    for(int i=0; i<3; i++) {
        L.carrot_ro.rects[i].pose = u_pose_new(
                positions_3[i].x,
                positions_3[i].y,
                16, 32);
    }
    ro_batch_update(&L.carrot_ro);

    
    // mini hud carrot
    L.cnt_ro = ro_batch_new(3, hud_camera.gl,
                    r_texture_new_file(1, 1, "res/carrot_mini.png"));
    update_cnt();
    
    
    // particles
    L.particle_ro = ro_particle_new(NUM_PARTICLES, camera.gl_main, r_texture_new_white_pixel());

    for(int i=0; i<L.particle_ro.num; i++) {
        L.particle_ro.rects[i].pose = u_pose_new_hidden();
        L.particle_ro.rects[i].color = vec4_set(0);
        L.particle_ro.rects[i].color_speed.a = 
                (float)-PARTICLE_ALPHA/PARTICLE_TIME;
    }

    ro_particle_update(&L.particle_ro);
}

void carrot_kill() {
    ro_batch_kill(&L.carrot_ro);
    memset(&L, 0, sizeof(L));
}

void carrot_update(float dtime) {
    L.time += dtime;
    float animate_time = sca_mod(L.time, FRAMES / FPS);
    int frame = animate_time * FPS;
    for(int i=0; i<3; i++)
        L.carrot_ro.rects[i].sprite.x = frame;

    for(int i=0; i<3; i++) {
        if(!L.collected[i])
            continue;
        
        float h = u_pose_get_h(L.carrot_ro.rects[i].pose);
        h = sca_max(0, h - COLLECT_SHRINK_SPEED * dtime);
        
        u_pose_set_size(&L.carrot_ro.rects[i].pose, h/2, h);
    }
    
    ro_batch_update(&L.carrot_ro);
    update_cnt();
}

void carrot_render() {
    ro_particle_render(&L.particle_ro, L.time);
    ro_batch_render(&L.carrot_ro);
}

void carrot_render_hud() {
    ro_batch_render(&L.cnt_ro);
}

bool carrot_collect(vec2 position) {
    for(int i=0; i<3; i++) {
        if(L.collected[i])
            continue;
        if(u_pose_aa_contains(L.carrot_ro.rects[i].pose, position)) {
            L.collected[i] = true;
            vec2 cxy = u_pose_get_xy(L.carrot_ro.rects[i].pose);
            emit_particles(cxy.x, cxy.y);
            L.collected_cnt++;
            update_cnt();
            return true;
        }
    }
    return false;
}

int carrot_collected() {
    return L.collected_cnt;
}


void carrot_eat() {
    if(L.collected_cnt <= 0) {
        log_error("carrot: failed to eat, collected_cnt <= 0");
        return;
    }
    L.collected_cnt--;
    update_cnt();
}

void carrot_save() {
    memcpy(L.save.collected, L.collected, sizeof(L.collected));
    L.save.collected_cnt = L.collected_cnt;
}

void carrot_load() {
    memcpy(L.collected, L.save.collected, sizeof(L.collected));
    L.collected_cnt = L.save.collected_cnt;
    
    
    // particles
    for(int i=0; i<L.particle_ro.num; i++) {
        L.particle_ro.rects[i].pose = u_pose_new_hidden();
    }
    ro_particle_update(&L.particle_ro);
    
    // in game carrots
    for(int i=0; i<3; i++) {
        if(L.collected[i]) {
            u_pose_set_size(&L.carrot_ro.rects[i].pose, 0, 0);
        } else {
            u_pose_set_size_angle(&L.carrot_ro.rects[i].pose, 16, 32, 0);
        }
    }
    ro_batch_update(&L.carrot_ro);
    
    // hud carrots
    update_cnt();
}

