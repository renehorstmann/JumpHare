#ifndef JUMPHARE_DEAD_H
#define JUMPHARE_DEAD_H

#include <stdbool.h>
#include "r/ro_types.h"


typedef void (*DeadFinishedFn)(void *user_data);

typedef struct {
    struct {
        RoSingle strike_ro, blend_ro;
        float time;
        DeadFinishedFn callback;
        void *callback_user_data;
        bool callback_called;
    } L;
} Dead;


Dead *dead_new(DeadFinishedFn callback, void *callback_user_data);

void dead_kill(Dead **self_ptr);

void dead_update(Dead *self, float dtime);

void dead_render(Dead *self, const mat4 *cam_mat, const mat4 *hudcam_mat);

void dead_set_dead(Dead *self, float x, float y);

bool dead_is_dead(const Dead *self);


#endif //JUMPHARE_DEAD_H
