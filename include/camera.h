#ifndef JUMPHARE_CAMERA_H
#define JUMPHARE_CAMERA_H

/*
 * PixelPerfect canvas camera with view matrix.
 * To control the camera position and size
 */

#include <stdbool.h>
#include "mathc/types/float.h"


#define CAMERA_SIZE 180 // *4=720; *6=1080; *8=1440
#define CAMERA_SCREEN_WEIGHT (3.0/5.0)
#define CAMERA_BACKGROUNDS 6

struct CameraMatrices_s {
    mat4 v;
    mat4 v_inv;
    mat4 vp;
    mat4 v_p_inv;   // v @ p_inv
};

struct CameraGlobals_s {
    mat4 matrices_p;
    mat4 matrices_p_inv;

    struct CameraMatrices_s matrices_background[CAMERA_BACKGROUNDS];
    struct CameraMatrices_s matrices_main;
    
    const mat4 *gl_background[CAMERA_BACKGROUNDS];
    const mat4 *gl_main;
    
    const float *gl_scale;
    // in texture space (origin is top left) [0:1]
    // as center_x, _y, radius_x, _y
    const float *gl_view_aabb;
};
extern struct CameraGlobals_s camera;


void camera_init();

void camera_update(int wnd_width, int wnd_height);

float camera_real_pixel_per_pixel();

float camera_left();

float camera_right();

float camera_bottom();

float camera_top();

vec2 camera_center_offset();

static float camera_width() {
    return -camera_left() + camera_right();
}

static float camera_height() {
    return -camera_bottom() + camera_top();
}

void camera_set_pos(float x, float y);

void camera_set_size(float size);

void camera_set_angle(float alpha);

#endif //JUMPHARE_CAMERA_H
