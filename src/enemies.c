#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "rhc/error.h"
#include "rhc/log.h"
#include "hare.h"
#include "collision.h"
#include "camera.h"
#include "enemies.h"


//
// private
//

typedef struct Enemies_Type Type;


static void add_type(Enemies *self, Type add) {
    assume(self->L.types_num < ENEMIES_MAX_TYPES, "wtf?");
    self->L.types[self->L.types_num++] = add;
}


//
// public
//


Enemies *enemies_new(const Collision *collision) {
    Enemies *self = rhc_calloc(sizeof *self);
    self->collision_ref = collision;
    return self;
}

void enemies_kill(Enemies **self_ptr) {
    Enemies *self = *self_ptr;
    if(!self)
        return;
    for(int i=0; i<self->L.types_num; i++) {
        ro_batch_kill(&self->L.types[i].ro);
    }
    rhc_free(self);
    *self_ptr = NULL;
}

void enemies_update(Enemies *self, float dtime) {
    for(int i=0; i<self->L.types_num; i++) {
        self->L.types[i].update(&self->L.types[i], dtime);
    }
}

void enemies_render(Enemies *self, const mat4 *cam_mat) {
    for(int i=0; i<self->L.types_num; i++) {
        ro_batch_render(&self->L.types[i].ro, cam_mat);
    }
}



//
// private
//


static void hedgehog_collision_cb(vec2 delta, enum collision_state state, void *ud) {
    rRect_s *rect = ud;
    bool left = u_pose_aa_get_w(rect->uv) > 0;
    vec2 pos = u_pose_get_xy(rect->pose);
    
    if(state != COLLISION_BOTTOM) {
        pos.x += left? 3 : -3;
        rect->uv = u_pose_new(0, 0, 
                left? -1 : 1,
                1);
        return;
    }
    
    pos = vec2_add_vec(pos, delta);
    
    u_pose_set_xy(&rect->pose, pos.x, pos.y);
} 

static void hedgehog_update(Type *self, float dtime) {
    const int frames = 4;
    const float fps = 6;
    self->time = sca_mod(self->time + dtime, frames / fps);
    int frame = self->time * fps;
    
    for(int i=0; i<self->ro.num; i++) {
        vec2 pos = u_pose_get_xy(self->ro.rects[i].pose);
        float distance = vec2_distance(
                hare.pos, 
                pos);
        if(distance < 45) {
            int roll_frame = 0;
            if(distance < 30)
                roll_frame = 3;
            else if(distance < 35)
                roll_frame = 2;
            else if(distance < 40)
                roll_frame = 1;
            self->ro.rects[i].sprite = (vec2) {{roll_frame, 1}};
        } else {
            self->ro.rects[i].sprite = (vec2) {{frame, 0}};
            
            bool left = u_pose_aa_get_w(self->ro.rects[i].uv) > 0;
            
            float speed_x = left? -10 : 10;
            pos.x += speed_x * dtime;
            
            u_pose_set_xy(&self->ro.rects[i].pose, pos.x, pos.y);
            
            CollisionCallback_s coll = {hedgehog_collision_cb, &self->ro.rects[i]};
            vec2 center = {{pos.x, pos.y-6}};
            vec2 radius = {{8, 8}};
            vec2 speed = {.x = speed_x};
            
            collision_tilemap_grounded(self->enemies_ref->collision_ref, coll, center, radius, speed);
        }
    }
    
    ro_batch_update(&self->ro);
}


//
// public
//

void enemies_add_hedgehogs(Enemies *self, const vec2 *positions, int n) {
    if(n<=0)
        return;
    assume(positions, "wtf?");
    log_info("enemies_add_hedgehogs: %i", n);
    Type t = {0};
    t.enemies_ref = self;
    t.ro = ro_batch_new(n,
            r_texture_new_file(4, 2, "res/enemies/hedgehog.png"));
    t.update = hedgehog_update;
    
    for(int i=0; i<n; i++) {
        t.ro.rects[i].pose = u_pose_new(
                positions[i].x, 
                positions[i].y+8,
                32, 32);
    }
    
    add_type(self, t);
}
