#ifndef JUMPHARE_DEAD_H
#define JUMPHARE_DEAD_H

#include <stdbool.h>

void dead_init();

void dead_update(float dtime);

void dead_render();

void dead_set_dead(float x, float y);

bool dead_is_dead();


#endif //JUMPHARE_DEAD_H
