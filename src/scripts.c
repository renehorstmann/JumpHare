#include "mathc/float.h"
#include "controller.h"
#include "hare.h"
#include "dead.h"
#include "airstroke.h"
#include "butterfly.h"
#include "carrot.h"
#include "flag.h"
#include "goal.h"
#include "cameractrl.h"

#include "scripts.h"

#define GOAL_MAX_DIST 15.0


Scripts *scripts_new(Controller *controller,
Camera_s *cam,
CameraControl_s *camctrl,
Airstroke *airstroke,
Butterfly *butterfly,
Carrot *carrot) {
    Scripts *self = rhc_calloc(sizeof *self);
    self->controller_ref = controller;
    self->cam_ref = cam;
    self->camctrl_ref = camctrl;
    self->airstroke_ref = airstroke;
    self->butterfly_ref = butterfly;
    self->carrot_ref = carrot;
    return self;
}

void scripts_kill(Scripts **self_ptr) {
    rhc_free(*self_ptr);
    *self_ptr = NULL;
}

void scripts_update(Scripts *self, float dtime) {

    bool dead = dead_is_dead();
    
    if(!dead)
        controller_update(self->controller_ref, dtime);
    
    // hare
    hare.in.speed = self->controller_ref->out.speed_x;
    hare.in.jump = self->controller_ref->out.action;
    if(!dead)
        hare_update(dtime);
    
    // cameractrl
    self->camctrl_ref->in.dst = hare.pos;
    cameractrl_update(self->camctrl_ref, self->cam_ref, dtime);

    // airstroke
    if(hare.out.jump_action) {
        airstroke_add(self->airstroke_ref, hare.pos.x, hare.pos.y);
    }
    vec2 as_pos[AIRSTROKE_MAX];
    int as_num = airstroke_positions(self->airstroke_ref, as_pos, AIRSTROKE_MAX);
    
    // butterfly
    butterfly_collect(self->butterfly_ref, hare.pos);
    for(int i=0; i<as_num; i++) {
        butterfly_collect(self->butterfly_ref, as_pos[i]);
    }
    
    // carrot
    carrot_collect(self->carrot_ref, hare.pos);
    for(int i=0; i<as_num; i++) {
        carrot_collect(self->carrot_ref, as_pos[i]);
    }
    
    // goal
    if(hare.state == HARE_GROUNDED 
       && vec2_distance(hare.pos, goal_position()) <= GOAL_MAX_DIST) {
           goal_activate();
    }
    
    
    // dead
    if(hare.state == HARE_DEAD) {
        dead_set_dead(hare.pos.x, hare.pos.y);
    }
}
