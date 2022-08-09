#ifndef JUMPHARE_BUTTERFLY_H
#define JUMPHARE_BUTTERFLY_H

#include "s/s.h"
#include "m/types/float.h"

struct Butterfly_Globals {
    struct {
        int collected;
        vec3 last_color;
    } RO; // read only
};
extern struct Butterfly_Globals butterfly;

void butterfly_init(const vec2 *positions, int num);

void butterfly_kill();

void butterfly_update(float dtime);

void butterfly_render(const mat4 *cam_mat);

bool butterfly_collect(vec2 position);

void butterfly_save();

void butterfly_load();

#endif //JUMPHARE_BUTTERFLY_H
