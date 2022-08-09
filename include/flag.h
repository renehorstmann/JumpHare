#ifndef JUMPHARE_FLAG_H
#define JUMPHARE_FLAG_H

#include "s/s.h"
#include "m/types/float.h"

#define FLAG_MAX_CALLBACKS 8

typedef void (*flag_activated_callback_fn)(vec2 pos, void *user_daza);

struct Flag_Globals {
    struct {
        vec2 active_pos;
    } RO;  // read only
};
extern struct Flag_Globals flag;

void flag_init(const vec2 *positions, int num);

void flag_kill();

void flag_update(float dtime);

void flag_render(const mat4 *cam_mat);

// returns NAN, NAN if no flag is actice
vec2 flag_active_position();

void flag_register_callback(flag_activated_callback_fn cb, void *ud);

#endif //JUMPHARE_FLAG_H
