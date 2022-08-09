#ifndef JUMPHARE_CAMERA_H
#define JUMPHARE_CAMERA_H

//
// PixelPerfect canvas camera with view matrix.
// To control the camera position and size
//

#include "s/s.h"
#include "m/types/float.h"
#include "m/types/int.h"


// the camera will unsure a canvas of at least CAMERA_SIZE * CAMERA_SIZE units
#define CAMERA_SIZE 180 // *4=720; *6=1080; *8=1440
#define CAMERA_SCREEN_WEIGHT (3.0/5.0)
#define CAMERA_BACKGROUNDS 6

typedef struct {
    mat4 v;
    mat4 v_inv;
    mat4 vp;
    mat4 v_p_inv;   // v @ p_inv
} CameraMatrices_s;

struct Camera_Globals {
    mat4 matrices_p;
    mat4 matrices_p_inv;

    CameraMatrices_s matrices_background[CAMERA_BACKGROUNDS];
    CameraMatrices_s matrices_main;
    
    
    struct {

        // units per pixel
        float scale;

        // view size
        int left, right, bottom, top;

        // static camera borders in units
        // bottom and right may be a little behind the actual screen border (depending on the real resolution)
        int hud_left, hud_right, hud_bottom, hud_top;

        // static camera borders in units
        // bottom and right are on the actual screen borders
        float screen_left, screen_right, screen_bottom, screen_top;
        
        vec2 offset;
        
        // in texture space (origin is top left) [0:1]
        // as center_x, _y, radius_x, _y
        vec4 view_aabb;
    } RO; // read only

};
extern struct Camera_Globals camera;


// initializes camera
void camera_init();

// updates the camera matrices
void camera_update();

// set the position of the camera in the world
void camera_set_pos(float x, float y);

// set the size / zoom of the camera (default = 1.0)
void camera_set_size(float size);

// set the rotation of the camera
void camera_set_angle(float alpha);

// returns the width in game pixel (!=real pixel)
static int camera_hud_width() {
    return -camera.RO.hud_left + camera.RO.hud_right;
}

// returns the height in game pixel (!=real pixel)
static int camera_hud_height() {
    return -camera.RO.hud_bottom + camera.RO.hud_top;
}

// returns the width in game pixel (!=real pixel)
static int camera_width() {
    return -camera.RO.left + camera.RO.right;
}

// returns the height in game pixel (!=real pixel)
static int camera_height() {
    return -camera.RO.bottom + camera.RO.top;
}

// returns true if the camera is in portrait mode (smartphone)
static bool camera_is_portrait_mode() {
    return camera_hud_height() > camera_hud_width();
}

#endif //JUMPHARE_CAMERA_H
