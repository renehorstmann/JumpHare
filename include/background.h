#ifndef JUMPHARE_BACKGROUND_H
#define JUMPHARE_BACKGROUND_H

#include <stdbool.h>
#include "r/ro_types.h"
#include "r/render.h"
#include "camera.h"

typedef struct {
    struct {
        RoBatch ro[CAMERA_BACKGROUNDS];
    } L;
} Background;

Background *background_new(float level_width, float level_height, 
        bool repeat_h, bool repeat_v,
        rRender *render,
        const char *file);

void background_kill(Background **self_ptr);

void background_update(Background *self, float dtime);

void background_render(Background *self, const Camera_s *cam);

#endif //JUMPHARE_BACKGROUND_H
