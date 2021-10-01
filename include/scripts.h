#ifndef JUMPHARE_SCRIPTS_H
#define JUMPHARE_SCRIPTS_H

#include "controller.h"
#include "cameractrl.h"
#include "airstroke.h"
#include "butterfly.h"
#include "carrot.h"

typedef struct {
    Controller *controller_ref;
    Camera_s *cam_ref;
    CameraControl_s *camctrl_ref;
    Airstroke *airstroke_ref;
    Butterfly *butterfly_ref;
    Carrot *carrot_ref;
} Scripts;

Scripts *scripts_new(Controller *controller, 
Camera_s *cam,
CameraControl_s *camctrl,
Airstroke *airstroke,
Butterfly *butterfly_ref,
Carrot *carrot_ref);

void scripts_kill(Scripts **self_ptr);

void scripts_update(Scripts *self, float dtime);


#endif //JUMPHARE_SCRIPTS_H
