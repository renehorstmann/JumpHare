#ifndef JUMPHARE_DIRT_PARTICLES_H
#define JUMPHARE_DIRT_PARTICLES_H

#include "mathc/types/float.h"
#include "color.h"

void dirt_particles_init();

void dirt_particles_kill();

void dirt_particles_update(float dtime);

void dirt_particles_render();

void dirt_particles_add(vec2 pos, vec2 dir, Color_s color, int n);

#endif //JUMPHARE_DIRT_PARTICLES_H
