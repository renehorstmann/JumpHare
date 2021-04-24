#include "u/pose.h"
#include "e/window.h"
#include "mathc/float.h"
#include "mathc/sca/int.h"
#include "mathc/utils/camera.h"
#include "hudcamera.h"

#define MIN_PIXEL_SIZE 2


// example: real_pixel_per_pixel    = 3.68
//#define PIXEL_PERFECT            // = 3.0
//#define ALLOW_HALF_PIXEL_SIZE  // = 3.5

struct HudCameraGlobals_s hudcamera;

static struct {
    float real_pixel_per_pixel;
    float left, right, bottom, top;
} L;


void hudcamera_init() {
    assert(HUDCAMERA_SIZE % 2 == 0 && "HUDCAMERA_SIZE must be even");
    hudcamera.gl = &hudcamera.matrices.p.m00;
    hudcamera.matrices.p = mat4_eye();
    hudcamera.matrices.p_inv = mat4_eye();

    hudcamera_update();
}

void hudcamera_update() {
    int wnd_width = e_window.size.x;
    int wnd_height = e_window.size.y;


    float smaller_size = wnd_width < wnd_height ? wnd_width : wnd_height;
    
   L.real_pixel_per_pixel = smaller_size / HUDCAMERA_SIZE;
    
#ifdef PIXEL_PERFECT
#ifdef ALLOW_HALF_PIXEL_SIZE
    L.real_pixel_per_pixel = sca_floor(L.real_pixel_per_pixel *2.0)/2.0;
#else
    L.real_pixel_per_pixel = sca_floor(L.real_pixel_per_pixel);
#endif
#endif
    
    L.real_pixel_per_pixel = sca_max(MIN_PIXEL_SIZE, L.real_pixel_per_pixel);

    float width_2 = wnd_width / (2 * L.real_pixel_per_pixel);
    float height_2 = wnd_height / (2 * L.real_pixel_per_pixel);

    // begin: (top, left) with a full pixel
    // end: (bottom, right) with a maybe splitted pixel
    L.left = -floorf(width_2);
    L.top = floorf(height_2);
    L.right = width_2 + (width_2 - floorf(width_2));
    L.bottom = -height_2 - (height_2 - floorf(height_2));

    hudcamera.matrices.p = mat4_camera_ortho(L.left, L.right, L.bottom, L.top, -1, 1);
    hudcamera.matrices.p_inv = mat4_inv(hudcamera.matrices.p);
}

float hudcamera_real_pixel_per_pixel() {
    return L.real_pixel_per_pixel;
}

float hudcamera_left() {
    return L.left;
}

float hudcamera_right() {
    return L.right;
}

float hudcamera_bottom() {
    return L.bottom;
}

float hudcamera_top() {
    return L.top;
}
