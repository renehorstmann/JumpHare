#ifndef JUMPHARE_CARROT_H
#define JUMPHARE_CARROT_H

#include <stdbool.h>
#include "mathc/types/float.h"

void carrot_init(const vec2 *positions_3);

void carrot_kill();

void carrot_update(float dtime);

void carrot_render();

bool carrot_collect(vec2 pos, vec2 prev_pos);

void carrot_save();

void carrot_load();

#endif //JUMPHARE_CARROT_H
