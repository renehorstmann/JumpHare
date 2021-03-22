#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "mathc/utils/random.h"
#include "camera.h"
#include "tilemap.h"
#include "airstroke.h"

#define MAX_STROKES 8

#define SPEED -350
#define FRAMES 4
#define FPS 18

typedef struct {
    rRect_s *rect;
    float time;
    bool hit;
} Stroke;

static struct {
    rRoBatch ro;
    Stroke strokes[MAX_STROKES];
} L;

void airstroke_init() {
    r_ro_batch_init(&L.ro, MAX_STROKES, camera.gl_main, r_texture_init_file("res/airstroke.png", NULL));

    for (int i = 0; i < MAX_STROKES; i++) {
        L.ro.rects[i].pose = u_pose_new_hidden();
        L.strokes[i].rect = &L.ro.rects[i];
    }
    r_ro_batch_update(&L.ro);
}

void airstroke_kill() {
    r_ro_batch_kill(&L.ro);
    memset(&L, 0, sizeof(L));
}

void airstroke_update(float dtime) {
    for (int i = 0; i < MAX_STROKES; i++) {
        Stroke *s = &L.strokes[i];

        // check dead
        if (s->time < 0) {
            continue;
        }

        if (!s->hit) {
            float x = u_pose_get_x(s->rect->pose);
            float y = u_pose_get_y(s->rect->pose);
            float ground = tilemap_ground(x, y, NULL);

            y += SPEED * dtime;
            if (y <= ground + 12) {
                s->hit = true;
                s->time = 0;
                y = ground + 12;
            }

            u_pose_set_y(&s->rect->pose, y);
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

    r_ro_batch_update(&L.ro);
}

void airstroke_render() {
    r_ro_batch_render(&L.ro);
}

void airstroke_add(float x, float y) {
    static int next = 0;

    Stroke *s = &L.strokes[next];
    s->rect->pose = u_pose_new(x, y, 32, 32);
    s->rect->uv = u_pose_new(0, 0, 1.0 / FRAMES, 1.0 / 2.0);
    s->rect->color = vec4_random_range(0.9, 1.0);
    s->time = 0;
    s->hit = 0;

    next = (next + 1) % MAX_STROKES;
}

