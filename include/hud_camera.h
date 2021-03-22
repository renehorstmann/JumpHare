#ifndef JUMPHARE_HUD_CAMERA_H
#define JUMPHARE_HUD_CAMERA_H

/*
 * PixelPerfect hud_camera for hud elements
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
extern struct HudCameraGlobals_s hud_camera;


void hud_camera_init();

void hud_camera_update();

float hud_camera_real_pixel_per_pixel();

float hud_camera_left();

float hud_camera_right();

float hud_camera_bottom();

float hud_camera_top();

static float hud_camera_width() {
    return -hud_camera_left() + hud_camera_right();
}

static float hud_camera_height() {
    return -hud_camera_bottom() + hud_camera_top();
}

static bool hud_camera_is_portrait_mode() {
    return hud_camera_height() > hud_camera_width();
}


#endif //JUMPHARE_HUD_CAMERA_H
