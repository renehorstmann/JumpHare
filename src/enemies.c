#include "r/ro_batch.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "rhc/error.h"
#include "rhc/log.h"
#include "camera.h"
#include "enemies.h"


#define MAX_ENEMIE_TYPES 32

struct Type;

typedef void (*update_fn)(struct Type *self, float dtime);

typedef struct Type {
    RoBatch ro;
    update_fn update;
    float time;
} Type;

static struct {
    Type types[MAX_ENEMIE_TYPES];
    int types_num;
} L;


static void add_type(Type add) {
    assume(L.types_num < MAX_ENEMIE_TYPES, "wtf?");
    L.types[L.types_num++] = add;
}


void enemies_init() {
    L.types_num = 0; // to be safe...
}

void enemies_kill() {
    for(int i=0; i<L.types_num; i++) {
        ro_batch_kill(&L.types[i].ro);
    }
    memset(&L, 0, sizeof(L));
}

void enemies_update(float dtime) {
    for(int i=0; i<L.types_num; i++) {
        L.types[i].update(&L.types[i], dtime);
    }
}

void enemies_render() {
    for(int i=0; i<L.types_num; i++) {
        ro_batch_render(&L.types[i].ro);
    }
}



static void hedgehog_update(Type *self, float dtime) {
    const int frames = 4;
    const float fps = 6;
    self->time = sca_mod(self->time + dtime, frames / fps);
    int frame = self->time * fps;
    for(int i=0; i<self->ro.num; i++) {
        self->ro.rects[i].sprite.x = frame;
    }
    ro_batch_update(&self->ro);
}

void enemies_add_hedgehogs(const vec2 *positions, int n) {
    if(n<=0)
        return;
    assume(positions, "wtf?");
    log_info("enemies_add_hedgehogs: %i", n);
    Type t = {0};
    t.ro = ro_batch_new(n, camera.gl_main,
            r_texture_new_file(4, 2, "res/enemies/hedgehog.png"));
    t.update = hedgehog_update;
    
    for(int i=0; i<n; i++) {
        t.ro.rects[i].pose = u_pose_new(
                positions[i].x, 
                positions[i].y+8,
                32, 32);
    }
    
    add_type(t);
}
