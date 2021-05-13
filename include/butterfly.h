#ifndef JUMPHARE_BUTTERFLY_H
#define JUMPHARE_BUTTERFLY_H

#include <stdbool.h>
#include "mathc/types/float.h"

void butterfly_init(const vec2 *positions, int num);

void butterfly_kill();

void butterfly_update(float dtime);

void butterfly_render();

int butterfly_collected();

vec3 butterfly_last_color();

bool butterfly_collect(vec2 position);

void butterfly_save();

void butterfly_load();

#endif //JUMPHARE_BUTTERFLY_H
