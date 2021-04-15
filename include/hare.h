#ifndef JUMPHARE_HARE_H
#define JUMPHARE_HARE_H

#include <stdbool.h>
#include "mathc/types/float.h"

enum hare_state {
    HARE_GROUNDED,
    HARE_FALLING,
    HARE_JUMPING,
    HARE_DOUBLE_JUMP,
    HARE_SLIPPING,
    HARE_NUM_STATES
};

void hare_init(float pos_x, float pos_y);

void hare_kill();

void hare_update(float dtime);

void hare_render();

enum hare_state hare_state();

vec2 hare_position();

vec2 hare_prev_position();

bool hare_looking_left();

// [-1 : 1]
void hare_set_speed(float dx);

void hare_jump();

#endif //JUMPHARE_HARE_H
