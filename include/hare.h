#ifndef JUMPHARE_HARE_H
#define JUMPHARE_HARE_H

#include <stdbool.h>
#include "mathc/types/float.h"

enum hare_state {
    HARE_GROUNDED,
    HARE_FALLING,
    HARE_JUMPING,
    HARE_DOUBLE_JUMP,
    HARE_SLEEPING,
    HARE_NUM_STATES
};

struct HareGlobals_s {
    enum hare_state state;
    vec2 pos;
    vec2 speed;
    bool looking_left;
};
extern struct HareGlobals_s hare;

void hare_init(float pos_x, float pos_y);

void hare_kill();

void hare_update(float dtime);

void hare_render();


// [-1 : 1]
void hare_set_speed(float dx);

void hare_jump();

void hare_set_sleep(bool instant);

#endif //JUMPHARE_HARE_H
