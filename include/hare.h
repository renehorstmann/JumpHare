#ifndef JUMPHARE_HARE_H
#define JUMPHARE_HARE_H

#include "s/s.h"
#include "m/types/float.h"

enum hare_state {
    HARE_GROUNDED,
    HARE_FALLING,
    HARE_JUMPING,
    HARE_DOUBLE_JUMP,
    HARE_SLEEPING,
    HARE_DEAD,
    HARE_NUM_STATES
};

struct Hare_Globals {
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
extern struct Hare_Globals hare;

void hare_init(float pos_x, float pos_y);

void hare_kill();

void hare_update(float dtime);

void hare_render(const mat4 *cam_mat);

void hare_set_sleep(bool instant);

#endif //JUMPHARE_HARE_H
