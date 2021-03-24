#ifndef JUMPHARE_CARROT_H
#define JUMPHARE_CARROT_H

#include "mathc/types/float.h"

void carrot_init(const vec2 *positions_3);

void carrot_kill();

void carrot_update(float dtime);

void carrot_render();


#endif //JUMPHARE_CARROT_H
