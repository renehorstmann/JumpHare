#ifndef JUMPHARE_GOAL_H
#define JUMPHARE_GOAL_H

#include <stdbool.h>
#include "mathc/types/float.h"

void goal_init(vec2 position);

void goal_kill();

void goal_update(float dtime);

void goal_render();

bool goal_reached();

#endif //JUMPHARE_GOAL_H
