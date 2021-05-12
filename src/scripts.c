#include "controller.h"
#include "hare.h"
#include "airstroke.h"
#include "butterfly.h"
#include "carrot.h"
#include "flag.h"
#include "cameractrl.h"

#include "scripts.h"


void scripts_init() {
    
}

void scripts_kill() {
    
}

void scripts_update(float dtime) {

    
    // hare
    hare_set_speed(controller.out.speed_x);
    if(controller.out.action) {
        hare_jump();
    }

    cameractrl.in.dst = hare.pos;

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
    
}
