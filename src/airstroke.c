#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "camera.h"
#include "tilemap.h"
#include "airstroke.h"


#define SPEED -350
#define FRAMES 4
#define FPS 18

//
// private
//

typedef struct Airstroke_Stroke Stroke;


//
// public
//

Airstroke *airstroke_new() {
    Airstroke *self = rhc_calloc(sizeof *self);
    
    self->L.ro = ro_batch_new(AIRSTROKE_MAX, r_texture_new_file(1, 1, "res/airstroke.png"));

    for (int i = 0; i < AIRSTROKE_MAX; i++) {
        self->L.ro.rects[i].pose = u_pose_new_hidden();
        self->L.strokes[i].rect = &self->L.ro.rects[i];
        self->L.strokes[i].time = -1;
    }
    ro_batch_update(&self->L.ro);
    
    return self;
}

void airstroke_kill(Airstroke **self_ptr) {
    Airstroke *self = *self_ptr;
    if(!self)
        return;
        
    ro_batch_kill(&self->L.ro);
    rhc_free(self);
    *self_ptr = NULL;
}

void airstroke_update(Airstroke *self, const Tilemap *tilemap, float dtime) {
    for (int i = 0; i < AIRSTROKE_MAX; i++) {
        Stroke *s = &self->L.strokes[i];
       
        // check dead
        if (s->time < 0) {
            continue;
        }
        
        s->prev_pos = u_pose_get_xy(s->rect->pose);


        if (!s->hit) {
            vec2 pos = u_pose_get_xy(s->rect->pose);
            float ground = tilemap_ground(tilemap, pos.x, pos.y, NULL);

            pos.y += SPEED * dtime;
            if (pos.y <= ground + 12) {
                s->hit = true;
                s->time = 0;
                pos.y = ground + 12;
            }

            u_pose_set_y(&s->rect->pose, pos.y);
        }

        s->time += dtime;

        float u = (int) (s->time * FPS) % FRAMES;
        float v = s->hit ? 1 : 0;

        u_pose_set_xy(&s->rect->uv, u / FRAMES, v / 2.0);

        // check dead
        if (s->hit && s->time > (float) FRAMES / FPS) {
            s->time = -1;
            s->rect->pose = u_pose_new_hidden();
        }
    }

    ro_batch_update(&self->L.ro);
}

void airstroke_render(Airstroke *self, const mat4 *cam_mat) {
    ro_batch_render(&self->L.ro, cam_mat);
}

void airstroke_add(Airstroke *self, float x, float y) {
    static int next = 0;

    Stroke *s = &self->L.strokes[next];
    s->rect->pose = u_pose_new(x, y, 32, 32);
    s->rect->uv = u_pose_new(0, 0, 1.0 / FRAMES, 1.0 / 2.0);
    s->rect->color = vec4_random_range(0.9, 1.0);
    s->time = 0;
    s->hit = 0;

    next = (next + 1) % AIRSTROKE_MAX;
}

int airstroke_positions(const Airstroke *self, vec2 *out_positions, int max_positions) {
    int idx = 0;
    for(int i=0; i<AIRSTROKE_MAX; i++) {
        if(self->L.strokes[i].time < 0)
            continue;
        out_positions[idx] = u_pose_get_xy(self->L.ro.rects[i].pose);
        if(++idx >= max_positions)
            return idx;
    }
    return idx;
}

int airstroke_prev_positions(const Airstroke *self, vec2 *out_prev_positions, int max_positions) {
    int idx = 0;
    for(int i=0; i<AIRSTROKE_MAX; i++) {
        if(self->L.strokes[i].time < 0)
            continue;
        out_prev_positions[idx] = self->L.strokes[i].prev_pos;
        if(++idx >= max_positions)
            return idx;
    }
    return idx;
}
