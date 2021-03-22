#include "u/pose.h"
#include "e/window.h"
#include "mathc/float.h"
#include "mathc/utils/camera.h"
#include "camera.h"


#define BACKGROUND_SPEED_FACTOR 0.2
#define FOREGROUND_SPEED_FACTOR 1.5

struct CameraGlobals_s camera;

static struct {
    float real_pixel_per_pixel;
    float left, right, bottom, top;
} L;

static void camera_matrices_init(struct CameraMatrices_s *self) {
    self->v = mat4_eye();
    self->v_inv = mat4_eye();
    self->vp = mat4_eye();
}

static void camera_matrices_update(struct CameraMatrices_s *self) {
    self->v_inv = mat4_inv(self->v);
    self->vp = mat4_mul_mat(camera.matrices_p, self->v_inv);
//    camera.matrices.v_p_inv = mat4_mul_mat(camera.matrices.v, camera.matrices.p_inv);
}

void camera_init() {
    assert(CAMERA_SIZE % 2 == 0 && "CAMERA_SIZE must be even");

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        camera.gl_background[i] = &camera.matrices_background[i].vp.m00;
    camera.gl_main = &camera.matrices_main.vp.m00;
    camera.gl_foreground = &camera.matrices_foreground.vp.m00;

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        camera_matrices_init(&camera.matrices_background[i]);
    camera_matrices_init(&camera.matrices_main);
    camera_matrices_init(&camera.matrices_foreground);

    camera.matrices_p = mat4_eye();
    camera.matrices_p_inv = mat4_eye();

    camera_update();
}

void camera_update() {
    int wnd_width = e_window.size.x;
    int wnd_height = e_window.size.y;

    float smaller_size = wnd_width < wnd_height ? wnd_width : wnd_height;
    L.real_pixel_per_pixel = floorf(smaller_size / CAMERA_SIZE);

    float width = (float) wnd_width / L.real_pixel_per_pixel;
    float height = (float) wnd_height / L.real_pixel_per_pixel;

    // begin: (top, left) with a full pixel
    // end: (bottom, right) with a maybe splitted pixel
    float left = -CAMERA_SIZE / 2;
    float top = CAMERA_SIZE / 2;
    float right = left + width;;
    float bottom = top - height;


    camera.matrices_p = mat4_camera_ortho(left, right, bottom, top, -1, 1);
    camera.matrices_p_inv = mat4_inv(camera.matrices_p);

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        camera_matrices_update(&camera.matrices_background[i]);
    camera_matrices_update(&camera.matrices_main);
    camera_matrices_update(&camera.matrices_foreground);

    if (wnd_width < wnd_height) {
        bottom = top - height * CAMERA_SCREEN_WEIGHT;
    } else {
        right = left + width * CAMERA_SCREEN_WEIGHT;
    }

    L.left = left;
    L.right = right;
    L.top = top;
    L.bottom = bottom;
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
