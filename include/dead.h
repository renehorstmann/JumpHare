#ifndef JUMPHARE_DEAD_H
#define JUMPHARE_DEAD_H

#include "s/s.h"
#include "m/types/float.h"


typedef void (*DeadFinishedFn)(void *user_data);

void dead_init(DeadFinishedFn callback, void *callback_user_data);

void dead_kill();

void dead_update(float dtime);

void dead_render(const mat4 *cam_mat, const mat4 *hudcam_mat);

void dead_set_dead(float x, float y);

bool dead_is_dead();


#endif //JUMPHARE_DEAD_H
