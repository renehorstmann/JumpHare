#ifndef JUMPHARE_CONTROLLER_H
#define JUMPHARE_CONTROLLER_H

#include <stdbool.h>

struct ControllerGlobals_s {
    float speed_x;
    bool action;
};
extern struct ControllerGlobals_s controller;

void controller_init();

void controller_kill();

void controller_update(float dtime);

void controller_render();

#endif //JUMPHARE_CONTROLLER_H
