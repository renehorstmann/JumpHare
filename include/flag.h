#ifndef JUMPHARE_FLAG_H
#define JUMPHARE_FLAG_H

#include <stdbool.h>
#include "mathc/types/float.h"

void flag_init(const vec2 *positions, int num);

void flag_kill();

void flag_update(float dtime);

void flag_render();

// returns NAN, NAN if no flag is actice
vec2 flag_active_position();

#endif //JUMPHARE_FLAG_H
