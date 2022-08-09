#ifndef JUMPHARE_LEVEL_H
#define JUMPHARE_LEVEL_H

#include "s/s.h"
#include "m/types/float.h"


void level_init(int lvl);

void level_kill();

void level_update(float dtime);

void level_render(const mat4 *hudcam_mat);


#endif //JUMPHARE_LEVEL_H
