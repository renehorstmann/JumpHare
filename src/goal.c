#include "e/input.h"
#include "r/ro_single.h"
#include "r/ro_particle.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "utilc/assume.h"
#include "camera.h"
#include "hare.h"
#include "goal.h"

#define GOAL_OFFSET_Y 8.0

#define FPS 4.0
#define FRAMES 4

#define MAX_DIST 15.0

#define MAX_PARTICLES 128
#define PARTICLE_SIZE 2.0
#define PARTICLE_SPEED 100.0
#define PARTICLE_TIME 1.5
#define PARTICLE_ALPHA 2.0


static struct {
    RoSingle goal_ro;
    RoParticle particle_ro;
    float time;
} L;

static void emit_particles(float x, float y) {
    for(int i=0; i<L.particle_ro.num; i++) {
        rParticleRect_s *r = &L.particle_ro.rects[i];
        r->pose = u_pose_new(x, y, PARTICLE_SIZE, PARTICLE_SIZE);
        float angle = sca_random_range(
                sca_radians(90-20),
                sca_radians(90+20));
        float speed = sca_random_range(0.1*PARTICLE_SPEED, PARTICLE_SPEED);
        r->speed.x = sca_cos(angle) * speed;
        r->speed.y = sca_sin(angle) * speed;
        r->acc.y = - speed * 0.33;
        
        if(rand()%2==0) {
            r->color.rgb = vec3_set(sca_random_noise(0.9, 0.1));
        } else {
            r->color.rgb = vec3_set(sca_random_noise(0.1, 0.1));
        }
        r->color.a = PARTICLE_ALPHA;
        
        r->start_time = L.time;
    }
    ro_particle_update(&L.particle_ro);
}

static void activate() {
    u_pose_set_y(&L.goal_ro.rect.uv, 0);

    vec2 pos = u_pose_get_xy(L.goal_ro.rect.pose);

    pos.y -= GOAL_OFFSET_Y;

    pos.y += 8 + GOAL_OFFSET_Y;
    emit_particles(pos.x, pos.y);
}

void goal_init(vec2 position) {
    
    ro_single_init(&L.goal_ro, camera.gl_main,
                    r_texture_new_file("res/goal_flag.png", NULL));
    L.goal_ro.rect.pose = u_pose_new(
            position.x,
            position.y+GOAL_OFFSET_Y,
            32, 48);
                
    u_pose_set_size(&L.goal_ro.rect.uv, 1.0/FRAMES, 0.5);
    u_pose_set_y(&L.goal_ro.rect.uv, 0.5);   
    
    ro_particle_init(&L.particle_ro, MAX_PARTICLES,
            camera.gl_main, r_texture_new_white_pixel());
    for(int i=0; i<L.particle_ro.num; i++) {
        L.particle_ro.rects[i].pose = u_pose_new_hidden();
        L.particle_ro.rects[i].color = vec4_set(0);
        L.particle_ro.rects[i].color_speed.a = 
            (float)-PARTICLE_ALPHA/PARTICLE_TIME;
    }
    ro_particle_update(&L.particle_ro);
    
}

void goal_kill() {
    ro_single_kill(&L.goal_ro);
    ro_particle_kill(&L.particle_ro);
}

void goal_update(float dtime) {
    L.time += dtime;


    float animate_time = sca_mod(L.time, FRAMES / FPS);
    int frame = animate_time * FPS;
    float u = (float) frame / FRAMES;
    u_pose_set_x(&L.goal_ro.rect.uv, u);
    
    
    // check reached
    if(goal_reached())
        return;
    
    if(hare_state() != HARE_GROUNDED)
        return;

    vec2 hare_pos = hare_position();    
    vec2 center = u_pose_get_xy(L.goal_ro.rect.pose);
    center.y -= GOAL_OFFSET_Y;
    if(vec2_distance(hare_pos, center) <= MAX_DIST) {
        activate();
    }
}

void goal_render() {
    if(goal_reached())
        ro_particle_render(&L.particle_ro, L.time);
    ro_single_render(&L.goal_ro);
}

bool goal_reached() {
    return u_pose_get_y(L.goal_ro.rect.uv) < 0.25;
}
