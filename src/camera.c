#include "mathc/float.h"
#include "mathc/utils/camera.h"
#include "r/rect.h"
#include "u/pose.h"
#include "e/window.h"
#include "camera.h"


#define BACKGROUND_SPEED_FACTOR 0.2
#define FOREGROUND_SPEED_FACTOR 1.5

struct CameraGlobals_s camera;

static struct {
    float real_pixel_per_pixel;
    float left, right, bottom, top;
} L;

static void camera_matrices_init(CameraMatrices_s *self) {
    self->v = mat4_eye();
    self->v_inv = mat4_eye();
    self->vp = mat4_eye();
}

static void camera_matrices_update(CameraMatrices_s *self) {
    self->v_inv = mat4_inv(self->v);
    self->vp = mat4_mul_mat(camera.matrices_p, self->v_inv);
//    camera.matrices.v_p_inv = mat4_mul_mat(camera.matrices.v, camera.matrices.p_inv);
}

void camera_init() {
    camera.gl_hud = &camera.matrices_hud.vp.m00;
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        camera.gl_background[i] = &camera.matrices_background[i].vp.m00;
    camera.gl_main = &camera.matrices_main.vp.m00;
    camera.gl_foreground = &camera.matrices_foreground.vp.m00;

    camera_matrices_init(&camera.matrices_hud);
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        camera_matrices_init(&camera.matrices_background[i]);
    camera_matrices_init(&camera.matrices_main);
    camera_matrices_init(&camera.matrices_foreground);

    camera.matrices_p = mat4_eye();
    camera.matrices_p_inv = mat4_eye();

    camera.matrices_hud_v_p_inv = mat4_eye();

    camera_update();
}

void camera_update() {
    int wnd_width = e_window.size.x;
    int wnd_height = e_window.size.y;

    float smaller_size = wnd_width < wnd_height ? wnd_width : wnd_height;
    L.real_pixel_per_pixel = floorf(smaller_size / CAMERA_SIZE);

    float width_2 = wnd_width / (2 * L.real_pixel_per_pixel);
    float height_2 = wnd_height / (2 * L.real_pixel_per_pixel);

    // begin: (top, left) with a full pixel
    // end: (bottom, right) with a maybe splitted pixel
    L.left = -floorf(width_2);
    L.top = floorf(height_2);
    L.right = width_2 + (width_2 - floorf(width_2));
    L.bottom = -height_2 - (height_2 - floorf(height_2));

    camera.matrices_p = mat4_camera_ortho(L.left, L.right, L.bottom, L.top, -1, 1);
    camera.matrices_p_inv = mat4_inv(camera.matrices_p);

    camera_matrices_update(&camera.matrices_hud);
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        camera_matrices_update(&camera.matrices_background[i]);
    camera_matrices_update(&camera.matrices_main);
    camera_matrices_update(&camera.matrices_foreground);

    camera.matrices_hud_v_p_inv = mat4_mul_mat(camera.matrices_hud.v, camera.matrices_p_inv);
}

float camera_real_pixel_per_pixel() {
    return L.real_pixel_per_pixel;
}

float camera_left() {
    return L.left;
}

float camera_right() {
    return L.right;
}

float camera_bottom() {
    return L.bottom;
}

float camera_top() {
    return L.top;
}

void camera_set_pos(float x, float y) {
    x = floorf(x * L.real_pixel_per_pixel) / L.real_pixel_per_pixel;
    y = floorf(y * L.real_pixel_per_pixel) / L.real_pixel_per_pixel;

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++) {
        float t = (float) i / CAMERA_BACKGROUNDS;
        float scale = sca_mix(BACKGROUND_SPEED_FACTOR, 1, t);
        u_pose_set_xy(&camera.matrices_background[i].v,
                      scale * x, scale * y);
    }
    u_pose_set_xy(&camera.matrices_main.v, x, y);
    u_pose_set_xy(&camera.matrices_foreground.v,
                  FOREGROUND_SPEED_FACTOR * x,
                  FOREGROUND_SPEED_FACTOR * y);
}

void camera_set_size(float size) {
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        u_pose_set_size(&camera.matrices_background[i].v, size, size);
    u_pose_set_size(&camera.matrices_main.v, size, size);
    u_pose_set_size(&camera.matrices_foreground.v, size, size);
}

void camera_set_angle(float alpha) {
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        u_pose_set_angle(&camera.matrices_background[i].v, alpha);
    u_pose_set_angle(&camera.matrices_main.v, alpha);
    u_pose_set_angle(&camera.matrices_foreground.v, alpha);
}
