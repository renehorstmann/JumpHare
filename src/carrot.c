#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/sca/float.h"
#include "camera.h"
#include "carrot.h"

#define FPS 3.0
#define FRAMES 4

static struct {
    rRoBatch ro;
    float time;
} L;

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
}

void carrot_kill() {
    r_ro_batch_kill(&L.ro);
    memset(&L, 0, sizeof(L));
}

void carrot_update(float dtime) {
    L.time = sca_mod(L.time + dtime, FRAMES / FPS);
    int frame = L.time * FPS;
    float u = (float) frame / FRAMES;
    for(int i=0; i<3; i++)
        u_pose_set_x(&L.ro.rects[i].uv, u);
        
    r_ro_batch_update(&L.ro);
}

void carrot_render() {
    r_ro_batch_render(&L.ro);   
}

