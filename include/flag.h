#ifndef JUMPHARE_FLAG_H
#define JUMPHARE_FLAG_H

#include <stdbool.h>
#include "e/input.h"
#include "mathc/types/float.h"

#define FLAG_MAX_CALLBACKS 8

typedef void (*flag_activated_callback_fn)(vec2 pos, void *user_daza);

void flag_init(const vec2 *positions, int num, eInput *input);

void flag_kill();

void flag_update(float dtime);

void flag_render();

// returns NAN, NAN if no flag is actice
vec2 flag_active_position();

void flag_register_callback(flag_activated_callback_fn cb, void *ud);

#endif //JUMPHARE_FLAG_H
