#ifndef JUMPHARE_HUD_H
#define JUMPHARE_HUD_H

#include "s/s.h"
#include "m/types/float.h"

void hud_init();

void hud_kill();

void hud_update(float dtime);

void hud_render(const mat4 *hudcam_mat);


#endif //JUMPHARE_HUD_H
