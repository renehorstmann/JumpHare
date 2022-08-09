#include "e/window.h"
#include "e/gui.h"
#include "u/pose.h"
#include "m/utils/camera.h"
#include "m/sca/float.h"
#include "camera.h"


#define BACKGROUND_SPEED_FACTOR 0.2


// comment out to not use a pixel perfect camera
#define PIXEL_PERFECT


struct Camera_Globals camera;

//
// private
//

static void camera_matrices_init(CameraMatrices_s *self) {
    self->v = mat4_eye();
    self->v_inv = mat4_eye();
    self->vp = mat4_eye();
    self->v_p_inv = mat4_eye();
}

static void camera_matrices_update(CameraMatrices_s *self) {
    self->v_inv = mat4_inv(self->v);
    self->vp = mat4_mul_mat(camera.matrices_p, self->v_inv);
    self->v_p_inv = mat4_mul_mat(self->v, camera.matrices_p_inv);
}


//
// public
//

void camera_init() {
    assert(CAMERA_SIZE % 2 == 0 && "CAMERA_SIZE must be even");
    
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        camera_matrices_init(&camera.matrices_background[i]);
    camera_matrices_init(&camera.matrices_main);

    camera.matrices_p = mat4_eye();
    camera.matrices_p_inv = mat4_eye();
}

void camera_update() {

    int wnd_width = e_window.size.x;
    int wnd_height = e_window.size.y;

    float smaller_size = wnd_width < wnd_height ? wnd_width : wnd_height;
    camera.RO.scale = smaller_size / CAMERA_SIZE;

#ifdef PIXEL_PERFECT
    if (camera.RO.scale > 1) {
        camera.RO.scale = sca_floor(camera.RO.scale);
    }
#endif

    // set nuklear scale for the debug gui windows
    e_gui.scale = camera.RO.scale/3;

    float cam_width = (float)wnd_width / camera.RO.scale;
    float cam_height = (float)wnd_height / camera.RO.scale;

    float width_2 = cam_width / 2;
    float height_2 = cam_height / 2;

    // begin: (top, left) with a full pixel
    // end: (bottom, right) with a maybe splitted pixel
    camera.RO.screen_left = -floorf(width_2);
    camera.RO.screen_top = floorf(height_2);
    camera.RO.screen_right = width_2 + (width_2 - floorf(width_2));
    camera.RO.screen_bottom = -height_2 - (height_2 - floorf(height_2));

    camera.matrices_p = mat4_camera_ortho(camera.RO.screen_left, camera.RO.screen_right, camera.RO.screen_bottom,
                                          camera.RO.screen_top, -1, 1);

    camera.matrices_p_inv = mat4_inv(camera.matrices_p);

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        camera_matrices_update(&camera.matrices_background[i]);
    camera_matrices_update(&camera.matrices_main);

    camera.RO.hud_left = sca_floor(camera.RO.screen_left);
    camera.RO.hud_right = sca_ceil(camera.RO.screen_right);
    camera.RO.hud_bottom = sca_floor(camera.RO.screen_bottom);
    camera.RO.hud_top = sca_ceil(camera.RO.screen_top);

    float left = camera.RO.hud_left;
    float top = camera.RO.hud_top;
    float right = camera.RO.hud_right;
    float bottom = camera.RO.hud_bottom;
    
    if (wnd_width < wnd_height) {
        float screen = cam_height * CAMERA_SCREEN_WEIGHT;
        bottom = top - screen;
        camera.RO.offset = (vec2) {{0, -(cam_height - screen)/2}};
    } else {
        float screen = cam_width * CAMERA_SCREEN_WEIGHT;
        left += sca_floor((cam_width-screen)/2);
        right -= sca_ceil((cam_width-screen)/2);
        camera.RO.offset = (vec2) {{0, 0}};
    }

    camera.RO.left = left;
    camera.RO.right = right;
    camera.RO.top = top;
    camera.RO.bottom = bottom;
   
   
    // view_aabb:
    // center_x, _y        
    camera.RO.view_aabb.x = (left - camera.RO.left + (-left + right)/2) / cam_width;
    camera.RO.view_aabb.y = -(top - camera.RO.top + (-top + bottom)/2) / cam_height;
    
    // radius_x, _y
    camera.RO.view_aabb.z = ((-left+right)/cam_width)/2;
    camera.RO.view_aabb.w = ((-bottom+top)/cam_height)/2;
}

void camera_set_pos(float x, float y) {
    // bottom left 'c'orner
    float cx = x + camera.RO.left;
    float cy = y + camera.RO.bottom;
    
    x = floorf(x * camera.RO.scale) / camera.RO.scale;
    y = floorf(y * camera.RO.scale) / camera.RO.scale;

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++) {
        float t = (float)i / CAMERA_BACKGROUNDS;
        float scale = sca_mix(BACKGROUND_SPEED_FACTOR, 1, t);
        float bg_x = scale * cx - camera.RO.left;
        float bg_y = scale * cy - camera.RO.bottom;
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
