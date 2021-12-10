#ifndef JUMPHARE_HUDCAMERA_H
#define JUMPHARE_HUDCAMERA_H

/*
 * PixelPerfect hudcamera for hud elements
 */

#include <stdbool.h>
#include "mathc/types/float.h"
#include "mathc/types/int.h"


#define HUDCAMERA_SIZE 180 // *4=720; *6=1080; *8=1440
#define HUDCAMERA_SCREEN_WEIGHT (2.0/5.0)

struct HudCameraMatrices_s {
    mat4 p;
    mat4 p_inv;
};

typedef struct {
    struct HudCameraMatrices_s matrices;
    struct {
        float scale;
        float left, right, bottom, top;
    } RO;   // read only
} HudCamera_s;

HudCamera_s *hudcamera_new();

void hudcamera_update(HudCamera_s *self, ivec2 window_size);


static float hudcamera_width(const HudCamera_s *self) {
    return -self->RO.left + self->RO.right;
}

static float hudcamera_height(const HudCamera_s *self) {
    return -self->RO.bottom + self->RO.top;
}

static bool hudcamera_is_portrait_mode(const HudCamera_s *self) {
    return hudcamera_height(self) > hudcamera_width(self);
}


#endif //JUMPHARE_HUDCAMERA_H
