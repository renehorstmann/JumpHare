#ifndef JUMPHARE_AIRSTROKE_H
#define JUMPHARE_AIRSTROKE_H

#include "r/ro_types.h"
#include "tilemap.h"

#define AIRSTROKE_MAX 8

struct Airstroke_Stroke {
    rRect_s *rect;
    float time;
    bool hit;
    vec2 prev_pos;
} ;

typedef struct {
    struct {
        RoBatch ro;
        struct Airstroke_Stroke strokes[AIRSTROKE_MAX];
    } L;
} Airstroke;

Airstroke *airstroke_new();

void airstroke_kill(Airstroke **self_ptr);

void airstroke_update(Airstroke *self, const Tilemap *tilemap, float dtime);

void airstroke_render(Airstroke *self, const mat4 *cam_mat);

// used by hare
void airstroke_add(Airstroke *self, float x, float y);

int airstroke_positions(const Airstroke *self, vec2 *out_positions, int max_positions);

int airstroke_prev_positions(const Airstroke *self, vec2 *out_prev_positions, int max_positions);


#endif //JUMPHARE_AIRSTROKE_H
