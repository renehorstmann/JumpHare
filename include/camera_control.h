#ifndef JUMPHARE_CAMERA_CONTROL_H
#define JUMPHARE_CAMERA_CONTROL_H

#include "mathc/types/float.h"

struct CameraControlGlobals_s {
    vec2 pos;
};
extern struct CameraControlGlobals_s camera_control;

void camera_control_init();

void camera_control_kill();

void camera_control_update(float dtime);

#endif //JUMPHARE_CAMERA_CONTROL_H
