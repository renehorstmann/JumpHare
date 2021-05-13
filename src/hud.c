#include "r/ro_single.h"
#include "r/ro_batch.h"
#include "r/ro_text.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "rhc/error.h"
#include "camera.h"
#include "hudcamera.h"
#include "carrot.h"
#include "hud.h"


//
// private
//

static struct {
    RoBatch carrots;
    RoSingle butterfly;
    RoText butterfly_cnt;
} L;

static void update_carrot() {
    int collected = carrot_collected();
    int eaten = carrot_eaten();
    
    assume(collected>=0 && eaten>=0, "wtf?");
    assume(collected <= 3, "wtf?");
    assume(eaten <= collected, "wtf?");
    
    for(int i=0; i<collected; i++) {
         L.carrots.rects[i].pose = u_pose_new_aa(
            sca_floor(camera_left() + 2 + i * 9),
            sca_floor(camera_top() - 2),
            8, 16);
         L.carrots.rects[i].sprite.x = i<eaten? 1 : 0;
    }
    for(int i=collected; i<3; i++) {
        L.carrots.rects[i].pose = u_pose_new_hidden();
    }
    ro_batch_update(&L.carrots);
}


//
// public
//

void hud_init() {
    L.carrots = ro_batch_new(3, hudcamera.gl,
                    r_texture_new_file(2, 1, "res/carrot_mini.png"));
}

void hud_kill() {
    ro_batch_kill(&L.carrots);
    ro_single_kill(&L.butterfly);
    ro_text_kill(&L.butterfly_cnt);
}

void hud_update(float dtime) {
    update_carrot();
}

void hud_render() {
    ro_batch_render(&L.carrots);
    //ro_single_render(&L.butterfly);
    //ro_text_render(&L.butterfly_cnt);
}
