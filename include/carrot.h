#ifndef JUMPHARE_CARROT_H
#define JUMPHARE_CARROT_H

#include "s/s.h"
#include "m/types/float.h"


struct Carrot_Globals{
    struct {
        int collected;  // eaten + available
        int eaten;
    } RO; // read only
};
extern struct Carrot_Globals carrot;

void carrot_init(const vec2 *positions_3);

void carrot_kill();

void carrot_update(float dtime);

void carrot_render(const mat4 *cam_mat);

bool carrot_collect(vec2 position);

void carrot_eat();

void carrot_save();

void carrot_load();

#endif //JUMPHARE_CARROT_H
