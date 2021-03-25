#include "r/ro_batch.h"
#include "r/ro_particle.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "utilc/assume.h"
#include "camera.h"

#include "flag.h"

#define FPS 4.0
#define FRAMES 4

static struct {
    rRoBatch flag_ro;
    float time;
} L;

void flag_init(const vec2 *positions, int num) {
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
    u_pose_set_y(&L.flag_ro.rects[0].uv, 0);
}

void flag_kill() {
    r_ro_batch_kill(&L.flag_ro);
}

void flag_update(float dtime) {
    L.time += dtime;
    float animate_time = sca_mod(L.time, FRAMES / FPS);
    int frame = animate_time * FPS;
    float u = (float) frame / FRAMES;
    for(int i=0; i<L.flag_ro.num; i++)
        u_pose_set_x(&L.flag_ro.rects[i].uv, u);
    
    r_ro_batch_update(&L.flag_ro);
}

void flag_render() {
    r_ro_batch_render(&L.flag_ro);
}
