#ifndef JUMPHARE_HARE_H
#define JUMPHARE_HARE_H

#include <stdbool.h>
#include "e/window.h"
#include "mathc/types/float.h"

enum hare_state {
    HARE_GROUNDED,
    HARE_FALLING,
    HARE_JUMPING,
    HARE_DOUBLE_JUMP,
    HARE_SLEEPING,
    HARE_DEAD,
    HARE_NUM_STATES
};

struct HareGlobals_s {
    enum hare_state state;
    vec2 pos;
    vec2 speed;
    bool looking_left;
    
    struct {
        float speed;  // [-1 : 1]
        bool jump;
    } in;
    
    struct {
        bool jump_action;
    } out;
};
extern struct HareGlobals_s hare;

void hare_init(float pos_x, float pos_y, eWindow *window);

void hare_kill();

void hare_update(float dtime);

void hare_render();

void hare_set_sleep(bool instant);

#endif //JUMPHARE_HARE_H
