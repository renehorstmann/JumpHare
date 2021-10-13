#ifndef JUMPHARE_CONTROLLER_H
#define JUMPHARE_CONTROLLER_H

#include <stdbool.h>
#include "e/input.h"
#include "r/ro_types.h"
#include "mathc/types/bool.h"
#include "camera.h"
#include "hudcamera.h"


typedef struct {
    eInput *input_ref;
    const Camera_s *camera_ref;
    const HudCamera_s *hudcam_ref;
    
    struct {
        float speed_x;
        bool action;
    } out;
    
    struct {
        RoBatch background_ro;
        ePointer_s pointer[2];
        bvec2 pointer_down_map;
        int pointer_down;
        int main_pointer;
    } L;
} Controller;


Controller *controller_new(eInput *input, const Camera_s *camera, const HudCamera_s *hudcam);

void controller_kill(Controller **self_ptr);

void controller_update(Controller *self, float dtime);

void controller_render(const Controller *self, const mat4 *hudcam_mat);

#endif //JUMPHARE_CONTROLLER_H
