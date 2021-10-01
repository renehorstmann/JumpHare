#ifndef JUMPHARE_HUD_H
#define JUMPHARE_HUD_H


#include "r/ro_types.h"
#include "camera.h"
#include "butterfly.h"
#include "carrot.h"

typedef struct {
    struct {
        RoBatch carrots;
        RoSingle butterfly_icon;
        RoText butterfly_cnt;
        float butterfly_time;
        int butterfly_collected;
    } L;
} Hud;

Hud *hud_new();

void hud_kill(Hud **self_ptr);

void hud_update(Hud *self, 
const Camera_s *cam, 
const Carrot *carrot, 
const Butterfly *butterfly,
float dtime);

void hud_render(Hud *self, const mat4 *hudcam_mat);


#endif //JUMPHARE_HUD_H
