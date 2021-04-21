#ifndef JUMPHARE_DIRT_PARTICLES_H
#define JUMPHARE_DIRT_PARTICLES_H

#include "u/color.h"
#include "mathc/types/float.h"

void dirt_particles_init();

void dirt_particles_kill();

void dirt_particles_update(float dtime);

void dirt_particles_render();

void dirt_particles_add(vec2 pos, vec2 dir, uColor_s color, int n);

#endif //JUMPHARE_DIRT_PARTICLES_H
