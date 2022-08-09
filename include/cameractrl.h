#ifndef JUMPHARE_CAMERACTRL_H
#define JUMPHARE_CAMERACTRL_H

#include "s/s.h"
#include "m/types/float.h"

struct CameraControl_Globals{
    vec2 pos;
    vec2 max_diff;
    vec2 diff_offset;

    struct {
        vec2 dst;
    } in;
};
extern struct CameraControl_Globals cameractrl;

void cameractrl_init();

void cameractrl_kill();

void cameractrl_update(float dtime);

#endif //JUMPHARE_CAMERACTRL_H
