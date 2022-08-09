#include "r/ro_batch.h"
#include "u/pose.h"
#include "m/float.h"
#include "m/utils/random.h"
#include "pixelparticles.h"
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


struct Carrot_Globals carrot;

static struct {
    RoBatch carrot_ro;
    bool collected[3];

    float time;

    struct {
        bool collected[3];
        int collected_cnt;
        int eaten_cnt;
    } save;
} L;

static void emit_particles(float x, float y) {
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
        rects[i].start_time_ms = pixelparticles.time;
    }
    pixelparticles_add(rects, NUM_PARTICLES);
}



//
// public
//

void carrot_init(const vec2 *positions_3) {
    
    // in game carrots
    L.carrot_ro = ro_batch_new(3,
                    r_texture_new_file(4, 1, "res/carrot.png"));
            
    for(int i=0; i<3; i++) {
        L.carrot_ro.rects[i].pose = u_pose_new(
                positions_3[i].x,
                positions_3[i].y,
                16, 32);
    }
    ro_batch_update(&L.carrot_ro);
}

void carrot_kill() {
    ro_batch_kill(&L.carrot_ro);
    memset(&L, 0, sizeof L);
    memset(&carrot, 0, sizeof carrot);
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

}

void carrot_render(const mat4 *cam_mat) {
    ro_batch_render(&L.carrot_ro, cam_mat, true);
}


bool carrot_collect(vec2 position) {
    for(int i=0; i<3; i++) {
        if(L.collected[i])
            continue;
        if(u_pose_aa_contains(L.carrot_ro.rects[i].pose, position)) {
            s_log_info("carrot: collected %i", i);
            L.collected[i] = true;
            vec2 cxy = u_pose_get_xy(L.carrot_ro.rects[i].pose);
            emit_particles(cxy.x, cxy.y);
            carrot.RO.collected++;
            return true;
        }
    }
    return false;
}


void carrot_eat() {
    if(carrot.RO.collected <= 0 || carrot.RO.eaten >= carrot.RO.collected) {
        s_log_error("carrot: failed to eat, collected <= 0");
        return;
    }
    carrot.RO.eaten++;
}

void carrot_save() {
    memcpy(L.save.collected, L.collected, sizeof(L.collected));
    L.save.collected_cnt = carrot.RO.collected;
    L.save.eaten_cnt = carrot.RO.eaten;
}

void carrot_load() {
    memcpy(L.collected, L.save.collected, sizeof(L.collected));
    carrot.RO.collected = L.save.collected_cnt;
    carrot.RO.eaten = L.save.eaten_cnt;
    
    // in game carrots
    for(int i=0; i<3; i++) {
        if(L.collected[i]) {
            u_pose_set_size(&L.carrot_ro.rects[i].pose, 0, 0);
        } else {
            u_pose_set_size_angle(&L.carrot_ro.rects[i].pose, 16, 32, 0);
        }
    }
    ro_batch_update(&L.carrot_ro);
}

