#ifndef JUMPHARE_CAMERACTRL_H
#define JUMPHARE_CAMERACTRL_H

#include "mathc/types/float.h"

struct CameraControlGlobals_s {
    vec2 pos;
};
extern struct CameraControlGlobals_s cameractrl;

void cameractrl_init();

void cameractrl_kill();

void cameractrl_update(float dtime);

#endif //JUMPHARE_CAMERACTRL_H
