#include "u/pose.h"
#include "e/window.h"
#include "mathc/float.h"
#include "mathc/sca/int.h"
#include "mathc/utils/camera.h"
#include "camera.h"

#define MIN_PIXEL_SIZE 2
#define BACKGROUND_SPEED_FACTOR 0.2

struct CameraGlobals_s camera;

static struct {
    float real_pixel_per_pixel;
    float left, right, bottom, top;
    vec2 offset;
    vec4 view_aabb;
} L;

static void camera_matrices_init(struct CameraMatrices_s *self) {
    self->v = mat4_eye();
    self->v_inv = mat4_eye();
    self->vp = mat4_eye();
    self->v_p_inv = mat4_eye();
}

static void camera_matrices_update(struct CameraMatrices_s *self) {
    self->v_inv = mat4_inv(self->v);
    self->vp = mat4_mul_mat(camera.matrices_p, self->v_inv);
    self->v_p_inv = mat4_mul_mat(self->v, camera.matrices_p_inv);
}

void camera_init() {
    assert(CAMERA_SIZE % 2 == 0 && "CAMERA_SIZE must be even");

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        camera.gl_background[i] = &camera.matrices_background[i].vp.m00;
    camera.gl_main = &camera.matrices_main.vp.m00;
    
    camera.gl_scale = &L.real_pixel_per_pixel;
    camera.gl_view_aabb = &L.view_aabb.v0;

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        camera_matrices_init(&camera.matrices_background[i]);
    camera_matrices_init(&camera.matrices_main);
    
    camera.matrices_p = mat4_eye();
    camera.matrices_p_inv = mat4_eye();

    camera_update();
}

void camera_update() {
    int wnd_width = e_window.size.x;
    int wnd_height = e_window.size.y;

    float smaller_size = wnd_width < wnd_height ? wnd_width : wnd_height;
    L.real_pixel_per_pixel = sca_floor(smaller_size / CAMERA_SIZE);
    L.real_pixel_per_pixel = isca_max(MIN_PIXEL_SIZE, L.real_pixel_per_pixel);

    float cam_width = (float)wnd_width / L.real_pixel_per_pixel;
    float cam_height = (float)wnd_height / L.real_pixel_per_pixel;

    float cam_width_2 = cam_width / 2;
    float cam_height_2 = cam_height / 2;

    // begin: (top, left) with a full pixel
    // end: (bottom, right) with a maybe splitted pixel
    float cam_left = -floorf(cam_width_2);
    float cam_top = floorf(cam_height_2);
    float cam_right = cam_width_2 + (cam_width_2 - floorf(cam_width_2));
    float cam_bottom = -cam_height_2 - (cam_height_2 - floorf(cam_height_2));

    camera.matrices_p = mat4_camera_ortho(
            cam_left, cam_right, cam_bottom, cam_top,
            -1, 1);
    camera.matrices_p_inv = mat4_inv(camera.matrices_p);

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        camera_matrices_update(&camera.matrices_background[i]);
    camera_matrices_update(&camera.matrices_main);
    
    float left = cam_left;
    float top = cam_top;
    float right = cam_right;
    float bottom = cam_bottom;
    
    if (wnd_width < wnd_height) {
        float screen = cam_height * CAMERA_SCREEN_WEIGHT;
        bottom = top - screen;
        L.offset = (vec2) {{0, -(cam_height - screen)/2}};
    } else {
        float screen = cam_width * CAMERA_SCREEN_WEIGHT;
        left += sca_floor((cam_width-screen)/2);
        right -= sca_ceil((cam_width-screen)/2);
        L.offset = (vec2) {{0, 0}};
    }

    L.left = left;
    L.right = right;
    L.top = top;
    L.bottom = bottom;
   
   
    // view_aabb:
    // center_x, _y        
    L.view_aabb.x = (left - cam_left + (-left + right)/2) / cam_width;
    L.view_aabb.y = -(top - cam_top + (-top + bottom)/2) / cam_height;
    
    // radius_x, _y
    L.view_aabb.z = ((-left+right)/cam_width)/2;
    L.view_aabb.w = ((-bottom+top)/cam_height)/2;
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

vec2 camera_center_offset() {
    return L.offset;
}

void camera_set_pos(float x, float y) {
    // bottom left 'c'orner
    float cx = x + L.left;
    float cy = y + L.bottom;
    
    x = floorf(x * L.real_pixel_per_pixel) / L.real_pixel_per_pixel;
    y = floorf(y * L.real_pixel_per_pixel) / L.real_pixel_per_pixel;

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++) {
        float t = (float)i / CAMERA_BACKGROUNDS;
        float scale = sca_mix(BACKGROUND_SPEED_FACTOR, 1, t);
        float bg_x = scale * cx - L.left;
        float bg_y = scale * cy - L.bottom;
        u_pose_set_xy(&camera.matrices_background[i].v, bg_x, bg_y);
    }
    
    u_pose_set_xy(&camera.matrices_main.v, x, y);
}

void camera_set_size(float size) {
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        u_pose_set_size(&camera.matrices_background[i].v, size, size);
        
    u_pose_set_size(&camera.matrices_main.v, size, size);
}

void camera_set_angle(float alpha) {
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        u_pose_set_angle(&camera.matrices_background[i].v, alpha);
        
    u_pose_set_angle(&camera.matrices_main.v, alpha);
}
