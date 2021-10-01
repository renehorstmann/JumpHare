#ifndef JUMPHARE_FLAG_H
#define JUMPHARE_FLAG_H

#include <stdbool.h>
#include "e/input.h"
#include "r/ro_types.h"
#include "camera.h"
#include "carrot.h"

#define FLAG_MAX_CALLBACKS 8

typedef void (*flag_activated_callback_fn)(vec2 pos, void *user_daza);

typedef struct {
    eInput *input_ref;
    const Camera_s *cam_ref;
    Carrot *carrot_ref;
    
    struct {
        RoBatch flag_ro;
        RoBatch btn_ro;
        float time;
        vec2 active_pos;
    
        struct {
            flag_activated_callback_fn cb;
            void *ud;
        } callbacks[FLAG_MAX_CALLBACKS];
        int callbacks_size;
    } L;    
} Flag;

Flag *flag_new(const vec2 *positions, int num, const Camera_s *cam, Carrot *carrot, eInput *input);

void flag_kill(Flag **self_ptr);

void flag_update(Flag *self, float dtime);

void flag_render(Flag *self, const mat4 *cam_mat);

// returns NAN, NAN if no flag is actice
vec2 flag_active_position(const Flag *self);

void flag_register_callback(Flag *self, flag_activated_callback_fn cb, void *ud);

#endif //JUMPHARE_FLAG_H
