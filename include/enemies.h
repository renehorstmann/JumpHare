#ifndef JUMPHARE_ENEMIES_H
#define JUMPHARE_ENEMIES_H

#include "mathc/types/float.h"


void enemies_init();

void enemies_kill();

void enemies_update(float dtime);

void enemies_render();

void enemies_add_hedgehogs(const vec2 *positions, int n);

#endif //JUMPHARE_ENEMIES_H
