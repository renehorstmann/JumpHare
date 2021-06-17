#ifndef JUMPHARE_LEVEL_H
#define JUMPHARE_LEVEL_H

#include "e/window.h"
#include "e/input.h"
#include "r/render.h"

void level_init(int lvl, eWindow *window, eInput *input, rRender *render);

void level_kill();

void level_update(float dtime);

void level_render();


#endif //JUMPHARE_LEVEL_H
