#ifndef JUMPHARE_HUDCAMERA_H
#define JUMPHARE_HUDCAMERA_H

/*
 * PixelPerfect hudcamera for hud elements
 */

#include <stdbool.h>
#include "mathc/types/float.h"


#define HUD_CAMERA_SIZE 180 // *4=720; *6=1080; *8=1440
#define HUD_CAMERA_SCREEN_WEIGHT (2.0/5.0)

struct HudCameraMatrices_s {
    mat4 p;
    mat4 p_inv;
};

struct HudCameraGlobals_s {
    struct HudCameraMatrices_s matrices;
    const float *gl;
};
extern struct HudCameraGlobals_s hudcamera;


void hudcamera_init();

void hudcamera_update();

float hudcamera_real_pixel_per_pixel();

float hudcamera_left();

float hudcamera_right();

float hudcamera_bottom();

float hudcamera_top();

static float hudcamera_width() {
    return -hudcamera_left() + hudcamera_right();
}

static float hudcamera_height() {
    return -hudcamera_bottom() + hudcamera_top();
}

static bool hudcamera_is_portrait_mode() {
    return hudcamera_height() > hudcamera_width();
}


#endif //JUMPHARE_HUDCAMERA_H
