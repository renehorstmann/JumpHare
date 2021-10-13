#include "mathc/float.h"

#include "scripts.h"

#define GOAL_MAX_DIST 15.0


void scripts_update(Level *lvl, float dtime) {
    Camera_s *cam = lvl->camera_ref;
    CameraControl_s *camctrl = lvl->game.camctrl;
    Controller *ctrl = lvl->controller;
    Tilemap *tilemap = lvl->tilemap;
    Dead *dead = lvl->dead;
    Hare *hare = lvl->game.hare;
    Airstroke *airstroke = lvl->game.airstroke;
    Butterfly *butterfly = lvl->butterfly;
    Carrot *carrot = lvl->carrot;
    Flag *flag = lvl->flag;
    Goal *goal = lvl->goal;
    

    bool isdead = dead_is_dead(dead);
    
    if(!isdead)
        controller_update(ctrl, dtime);
    
    // hare
    hare->in.speed = ctrl->out.speed_x;
    hare->in.jump = ctrl->out.action;
    if(!isdead)
        hare_update(hare, dtime);
    
    // cameractrl
    camctrl->in.dst = hare->pos;
    cameractrl_update(camctrl, cam, tilemap, dtime);

    // airstroke
    if(hare->out.jump_action) {
        airstroke_add(airstroke, hare->pos.x, hare->pos.y);
    }
    vec2 as_pos[AIRSTROKE_MAX];
    int as_num = airstroke_positions(airstroke, as_pos, AIRSTROKE_MAX);
    
    // butterfly
    butterfly_collect(butterfly, hare->pos);
    for(int i=0; i<as_num; i++) {
        butterfly_collect(butterfly, as_pos[i]);
    }
    
    // carrot
    carrot_collect(carrot, hare->pos);
    for(int i=0; i<as_num; i++) {
        carrot_collect(carrot, as_pos[i]);
    }
    
    // goal
    if(hare->state == HARE_GROUNDED 
       && vec2_distance(hare->pos, goal_position(goal)) <= GOAL_MAX_DIST) {
           goal_activate(goal);
    }
    
    
    // dead
    if(hare->state == HARE_DEAD) {
        dead_set_dead(dead, hare->pos.x, hare->pos.y);
    }
}
