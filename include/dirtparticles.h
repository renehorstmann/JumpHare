#ifndef JUMPHARE_DIRTPARTICLES_H
#define JUMPHARE_DIRTPARTICLES_H

#include "u/color.h"
#include "mathc/types/float.h"

void dirtparticles_init();

void dirtparticles_kill();

void dirtparticles_update(float dtime);

void dirtparticles_render();

void dirtparticles_add(vec2 pos, vec2 dir, uColor_s color, int n);

#endif //JUMPHARE_DIRTPARTICLES_H
