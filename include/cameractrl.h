#ifndef JUMPHARE_CAMERACTRL_H
#define JUMPHARE_CAMERACTRL_H

#include "camera.h"
#include "tilemap.h"

typedef struct {
    vec2 pos;
    vec2 max_diff;
    vec2 diff_offset;

    struct {
        vec2 dst;
    } in;
    
    struct {
        vec2 pos;
    } L;
} CameraControl_s;

CameraControl_s *cameractrl_new();

void cameractrl_kill(CameraControl_s **self_ptr);

void cameractrl_update(CameraControl_s *self, Camera_s *camera, const Tilemap *tilemap, float dtime);

#endif //JUMPHARE_CAMERACTRL_H
