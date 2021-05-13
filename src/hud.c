#include <stdio.h>
#include "r/ro_single.h"
#include "r/ro_batch.h"
#include "r/ro_text.h"
#include "u/pose.h"
#include "mathc/float.h"
#include "rhc/error.h"
#include "camera.h"
#include "hudcamera.h"
#include "carrot.h"
#include "butterfly.h"
#include "hud.h"


#define BUTTERFLY_TIME 0.2

//
// private
//

static struct {
    RoBatch carrots;
    RoSingle butterfly_icon;
    RoText butterfly_cnt;
    float butterfly_time;
    int butterfly_collected;
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


static void update_butterfly(float dtime) {
    int collected = butterfly_collected();
    vec3 color = butterfly_last_color();
    
    if(collected > L.butterfly_collected) {
        L.butterfly_time = BUTTERFLY_TIME;
    }
    L.butterfly_collected = collected;
     
    L.butterfly_time -= dtime;
    L.butterfly_icon.rect.sprite.x = L.butterfly_time>0? 2 : 0;
    L.butterfly_icon.rect.color.rgb = color;
    L.butterfly_icon.rect.pose = u_pose_new_aa(
            sca_floor(camera_right() - 16 - 4*6), 
            sca_floor(camera_top()), 
            16, 16);
    u_pose_set_xy(&L.butterfly_cnt.pose, 
            sca_floor(camera_right() -4*6), 
            sca_floor(camera_top() - (16-6)/2));
    char buf[5];
    assume(collected>=0 && collected <1000, "?");
    sprintf(buf, "x%i", collected);
    ro_text_set_text(&L.butterfly_cnt, buf);
    
}


//
// public
//

void hud_init() {
    L.carrots = ro_batch_new(3, hudcamera.gl,
                    r_texture_new_file(2, 1, "res/carrot_mini.png"));
                    
    
    L.butterfly_icon = ro_single_new(hudcamera.gl, r_texture_new_file(12, 2, "res/butterfly.png"));
    
    L.butterfly_icon.rect.sprite.y = 1;
    
    L.butterfly_cnt = ro_text_new_font55(4, hudcamera.gl);
    ro_text_set_color(&L.butterfly_cnt, R_COLOR_BLACK);
    
}

void hud_kill() {
    ro_batch_kill(&L.carrots);
    ro_single_kill(&L.butterfly_icon);
    ro_text_kill(&L.butterfly_cnt);
    memset(&L, 0, sizeof(L));
}

void hud_update(float dtime) {
    update_carrot();
    update_butterfly(dtime);
}

void hud_render() {
    ro_batch_render(&L.carrots);
    ro_single_render(&L.butterfly_icon);
    ro_text_render(&L.butterfly_cnt);
}
