#ifndef JUMPHARE_GOAL_H
#define JUMPHARE_GOAL_H

#include "s/s.h"
#include "m/types/float.h"


void goal_init(vec2 position);

void goal_kill();

void goal_update(float dtime);

void goal_render(const mat4 *cam_mat);

bool goal_reached();

vec2 goal_position();

void goal_activate();

#endif //JUMPHARE_GOAL_H
