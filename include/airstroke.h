#ifndef JUMPHARE_AIRSTROKE_H
#define JUMPHARE_AIRSTROKE_H

#include "mathc/types/float.h"

#define AIRSTROKE_MAX 8

void airstroke_init();

void airstroke_kill();

void airstroke_update(float dtime);

void airstroke_render();

// used by hare
void airstroke_add(float x, float y);

int airstroke_positions(vec2 *out_positions, int max_positions);

int airstroke_prev_positions(vec2 *out_prev_positions, int max_positions);


#endif //JUMPHARE_AIRSTROKE_H
