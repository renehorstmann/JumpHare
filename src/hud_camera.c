#include "u/pose.h"
#include "e/window.h"
#include "mathc/float.h"
#include "mathc/sca/int.h"
#include "mathc/utils/camera.h"
#include "hud_camera.h"

#define MIN_PIXEL_SIZE 2

struct HudCameraGlobals_s hud_camera;

static struct {
    float real_pixel_per_pixel;
    float left, right, bottom, top;
} L;


void hud_camera_init() {
    assert(HUD_CAMERA_SIZE % 2 == 0 && "HUD_CAMERA_SIZE must be even");
    hud_camera.gl = &hud_camera.matrices.p.m00;
    hud_camera.matrices.p = mat4_eye();
    hud_camera.matrices.p_inv = mat4_eye();

    hud_camera_update();
}

void hud_camera_update() {
    int wnd_width = e_window.size.x;
    int wnd_height = e_window.size.y;


    float smaller_size = wnd_width < wnd_height ? wnd_width : wnd_height;
    L.real_pixel_per_pixel = sca_floor(smaller_size / HUD_CAMERA_SIZE);
    L.real_pixel_per_pixel = isca_max(MIN_PIXEL_SIZE, L.real_pixel_per_pixel);

    float width_2 = wnd_width / (2 * L.real_pixel_per_pixel);
    float height_2 = wnd_height / (2 * L.real_pixel_per_pixel);

    // begin: (top, left) with a full pixel
    // end: (bottom, right) with a maybe splitted pixel
    L.left = -floorf(width_2);
    L.top = floorf(height_2);
    L.right = width_2 + (width_2 - floorf(width_2));
    L.bottom = -height_2 - (height_2 - floorf(height_2));

    hud_camera.matrices.p = mat4_camera_ortho(L.left, L.right, L.bottom, L.top, -1, 1);
    hud_camera.matrices.p_inv = mat4_inv(hud_camera.matrices.p);
}

float hud_camera_real_pixel_per_pixel() {
    return L.real_pixel_per_pixel;
}

float hud_camera_left() {
    return L.left;
}

float hud_camera_right() {
    return L.right;
}

float hud_camera_bottom() {
    return L.bottom;
}

float hud_camera_top() {
    return L.top;
}
