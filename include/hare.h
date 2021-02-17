#ifndef JUMPHARE_HARE_H
#define JUMPHARE_HARE_H

#include "mathc/types/float.h"

void hare_init();

void hare_update(float dtime);

void hare_render();

vec2 hare_position();

void hare_set_speed(float dx);

void hare_jump();

#endif //JUMPHARE_HARE_H
