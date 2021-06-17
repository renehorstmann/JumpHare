#ifndef JUMPHARE_CONTROLLER_H
#define JUMPHARE_CONTROLLER_H

#include <stdbool.h>
#include "e/input.h"

struct ControllerGlobals_s {
    struct {
        float speed_x;
        bool action;
    } out;
};
extern struct ControllerGlobals_s controller;

void controller_init(eInput *input);

void controller_kill();

void controller_update(float dtime);

void controller_render();

#endif //JUMPHARE_CONTROLLER_H
