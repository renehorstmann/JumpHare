#include "r/ro_particle.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "camera.h"
#include "dirtparticles.h"


#define MAX_PARTCLES 64
#define LIFETIME 1.0
#define SIZE 2.0
#define PARTICLE_ALPHA 1.0

static struct {
    RoParticle ro;
    float time;
    int next;
} L;

void dirtparticles_init() {
    L.ro = ro_particle_new(MAX_PARTCLES, camera.gl_main, r_texture_new_white_pixel());

    for (int i = 0; i < L.ro.num; i++) {
        u_pose_set_size(&L.ro.rects[i].pose, SIZE, SIZE);
        L.ro.rects[i].color = vec4_set(0);
        L.ro.rects[i].acc.y = -20;
        L.ro.rects[i].color_speed.a = 
                (float)-PARTICLE_ALPHA / LIFETIME;
    }
    ro_particle_update(&L.ro);
}

void dirtparticles_kill() {
    ro_particle_kill(&L.ro);
    memset(&L, 0, sizeof(L));
}

void dirtparticles_update(float dtime) {
    L.time += dtime;
}

void dirtparticles_render() {
    ro_particle_render(&L.ro, L.time);
}

#include "mathc/uchar.h"

void dirtparticles_add(vec2 pos, vec2 dir, uColor_s color, int n) {
    log_trace("dirtparticles: add %i", n);
    vec4 col = vec4_cast_from_uchar_1(color.v);
    int start_idx = L.next;
    for (int i = 0; i < n; i++) {
        int idx = L.next++;
        if (L.next >= L.ro.num)
            L.next = 0;

        float x = sca_random_noise(pos.x, 10);
        float y = sca_random_noise(pos.y, 4);
        u_pose_set_xy(&L.ro.rects[idx].pose, x, y);

        L.ro.rects[idx].speed.x = sca_random_noise(dir.x, 5);
        L.ro.rects[idx].speed.y = sca_random_noise(dir.y, 5);

        L.ro.rects[idx].color = vec4_random_noise_vec(col, vec4_set(0.1));
        L.ro.rects[idx].color.a *= PARTICLE_ALPHA;

        L.ro.rects[idx].start_time = L.time;
    }

    ro_particle_update_sub(&L.ro, start_idx, n);
}
