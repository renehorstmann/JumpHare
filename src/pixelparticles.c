#include "r/ro_particle.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/sca/int.h"
#include "mathc/utils/random.h"
#include "camera.h"
#include "pixelparticles.h"


#define MAX_PARTCLES 512

#define DIRT_LIFETIME 1.0
#define DIRT_SIZE 2.0
#define DIRT_ALPHA 1.0

struct PixelParticlesGlobals_s pixelparticles;

//
// private
//

static struct {
    RoParticle ro;
    int next;
} L;


//
// public
//

void pixelparticles_init() {
    L.ro = ro_particle_new(MAX_PARTCLES, camera.gl_main, r_texture_new_white_pixel());

    for (int i = 0; i < L.ro.num; i++) {
        L.ro.rects[i].pose = u_pose_new_hidden();
        L.ro.rects[i].color = vec4_set(0);
    }
    ro_particle_update(&L.ro);
}

void pixelparticles_kill() {
    ro_particle_kill(&L.ro);
    memset(&L, 0, sizeof(L));
}

void pixelparticles_update(float dtime) {
    pixelparticles.time += dtime;
}

void pixelparticles_render() {
    ro_particle_render(&L.ro, pixelparticles.time);
}

void pixelparticles_add(const rParticleRect_s *particles, int n) {
    int start_idx = L.next;
    n = isca_min(n, L.ro.num);

    if(L.next + n <= L.ro.num) {
        memcpy(&L.ro.rects[L.next], particles, n * sizeof(rParticleRect_s));
        L.next += n;
    } else {
        int n_b = L.next + n - L.ro.num;
        int n_a = n - n_b;
        memcpy(&L.ro.rects[L.next], particles, n_a * sizeof(rParticleRect_s));
        memcpy(L.ro.rects, particles+n_a, n_b * sizeof(rParticleRect_s));
        L.next = n_b;
    }
    if (L.next >= L.ro.num)
        L.next = 0;

    ro_particle_update_sub(&L.ro, start_idx, n);
}

void pixelparticles_add_dirt(vec2 pos, vec2 dir, uColor_s color, int n) {
    rParticleRect_s rects[n];

    vec4 col = vec4_cast_from_uchar_1(color.v);
    for (int i = 0; i < n; i++) {
        rects[i] = r_particlerect_new();

        float x = sca_random_noise(pos.x, 10);
        float y = sca_random_noise(pos.y, 4);
        rects[i].pose = u_pose_new(x, y, DIRT_SIZE, DIRT_SIZE);
        rects[i].acc.y = -20;
        rects[i].color_speed.a = (float) -DIRT_ALPHA / DIRT_LIFETIME;

        rects[i].speed.x = sca_random_noise(dir.x, 5);
        rects[i].speed.y = sca_random_noise(dir.y, 5);

        rects[i].color = vec4_random_noise_vec(col, vec4_set(0.1));
        rects[i].color.a *= DIRT_ALPHA;

        rects[i].start_time = pixelparticles.time;
    }

    pixelparticles_add(rects, n);
}
