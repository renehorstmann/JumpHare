#ifndef JUMPHARE_BUTTERFLY_H
#define JUMPHARE_BUTTERFLY_H

#include "r/ro_types.h"

typedef struct {
    struct {
        int collected;
        vec3 last_color;
    } RO; // read only
    
    struct {
        RoParticle ro;
        float time;
        float reset_time;
        float collected_time;
    
        struct {
            bool *collected;
        } save;
    } L;
} Butterfly;

Butterfly *butterfly_new(const vec2 *positions, int num);

void butterfly_kill(Butterfly **self_ptr);

void butterfly_update(Butterfly *self, float dtime);

void butterfly_render(const Butterfly *self, const mat4 *cam_mat);

bool butterfly_collect(Butterfly *self, vec2 position);

void butterfly_save(Butterfly *self);

void butterfly_load(Butterfly *self);

#endif //JUMPHARE_BUTTERFLY_H
