#ifndef JUMPHARE_CONTROLLER_H
#define JUMPHARE_CONTROLLER_H

#include "s/s.h"
#include "m/types/float.h"


struct Controller_Globals {
    struct {
        float speed_x;
        bool action;
    } out;
};
extern struct Controller_Globals controller;


void controller_init();

void controller_kill();

void controller_update(float dtime);

void controller_render(const mat4 *hudcam_mat);

#endif //JUMPHARE_CONTROLLER_H
