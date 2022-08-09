#include "m/float.h"
#include "speechbubble.h"
#include "carrot.h"
#include "butterfly.h"
#include "tilemap.h"
#include "enemies.h"
#include "flag.h"
#include "goal.h"
#include "hare.h"
#include "airstroke.h"
#include "cameractrl.h"
#include "background.h"
#include "hud.h"
#include "controller.h"
#include "camera.h"
#include "collision.h"
#include "pixelparticles.h"
#include "dead.h"
#include "scripts.h"

#define GOAL_MAX_DIST 15.0


void scripts_update(float dtime) {

    bool isdead = dead_is_dead();
    
    if(!isdead)
        controller_update( dtime);
    
    // hare
    hare.in.speed = controller.out.speed_x;
    hare.in.jump = controller.out.action;
    if(!isdead)
        hare_update(dtime);
    
    // cameractrl
    cameractrl.in.dst = hare.pos;
    cameractrl_update(dtime);

    // airstroke
    if(hare.out.jump_action) {
        airstroke_add(hare.pos.x, hare.pos.y);
    }
    vec2 as_pos[AIRSTROKE_MAX];
    int as_num = airstroke_positions(as_pos, AIRSTROKE_MAX);
    
    // butterfly
    butterfly_collect(hare.pos);
    for(int i=0; i<as_num; i++) {
        butterfly_collect(as_pos[i]);
    }
    
    // carrot
    carrot_collect(hare.pos);
    for(int i=0; i<as_num; i++) {
        carrot_collect(as_pos[i]);
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
