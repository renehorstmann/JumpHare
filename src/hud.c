#include <stdio.h>
#include "r/ro_single.h"
#include "r/ro_batch.h"
#include "r/ro_text.h"
#include "r/texture.h"
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

static void update_carrot(Hud *self, const Camera_s *cam, const Carrot *carrot) {
    int collected = carrot->RO.collected;
    int eaten = carrot->RO.eaten;
    
    assume(collected>=0 && eaten>=0, "wtf?");
    assume(collected <= 3, "wtf?");
    assume(eaten <= collected, "wtf?");
    
    for(int i=0; i<collected; i++) {
         self->L.carrots.rects[i].pose = u_pose_new_aa(
            sca_floor(cam->RO.left + 2 + i * 9),
            sca_floor(cam->RO.top - 2),
            8, 16);
         self->L.carrots.rects[i].sprite.x = i<eaten? 1 : 0;
    }
    for(int i=collected; i<3; i++) {
        self->L.carrots.rects[i].pose = u_pose_new_hidden();
    }
    ro_batch_update(&self->L.carrots);
}


static void update_butterfly(Hud *self, const Camera_s *cam, const Butterfly *butterfly, float dtime) {
    int collected = butterfly->RO.collected;
    vec3 color = butterfly->RO.last_color;
    
    if(collected > self->L.butterfly_collected) {
        self->L.butterfly_time = BUTTERFLY_TIME;
    }
    self->L.butterfly_collected = collected;
     
    self->L.butterfly_time -= dtime;
    self->L.butterfly_icon.rect.sprite.x = self->L.butterfly_time>0? 2 : 0;
    self->L.butterfly_icon.rect.color.rgb = color;
    self->L.butterfly_icon.rect.pose = u_pose_new_aa(
            sca_floor(cam->RO.right - 16 - 4*6), 
            sca_floor(cam->RO.top), 
            16, 16);
    u_pose_set_xy(&self->L.butterfly_cnt.pose, 
            sca_floor(cam->RO.right -4*6), 
            sca_floor(cam->RO.top - (16-6)/2));
    char buf[5];
    assume(collected>=0 && collected <1000, "?");
    sprintf(buf, "x%i", collected);
    ro_text_set_text(&self->L.butterfly_cnt, buf);
    
}


//
// public
//

Hud *hud_new() {
    Hud *self = rhc_calloc(sizeof *self);
    
    self->L.carrots = ro_batch_new(3,
                    r_texture_new_file(2, 1, "res/carrot_mini.png"));
                    
    
    self->L.butterfly_icon = ro_single_new(r_texture_new_file(12, 2, "res/butterfly.png"));
    
    self->L.butterfly_icon.rect.sprite.y = 1;
    
    self->L.butterfly_cnt = ro_text_new_font55(4);
    ro_text_set_color(&self->L.butterfly_cnt, R_COLOR_BLACK);
    
    return self;
}

void hud_kill(Hud **self_ptr) {
    Hud *self = *self_ptr;
    if(!self)
        return;
        
    ro_batch_kill(&self->L.carrots);
    ro_single_kill(&self->L.butterfly_icon);
    ro_text_kill(&self->L.butterfly_cnt);
    
    rhc_free(self);
    *self_ptr = NULL;
}

void hud_update(Hud *self, 
const Camera_s *cam, 
const Carrot *carrot, 
const Butterfly *butterfly,
float dtime) {
    update_carrot(self, cam, carrot);
    update_butterfly(self, cam, butterfly, dtime);
}

void hud_render(Hud *self, const mat4 *hudcam_mat) {
    ro_batch_render(&self->L.carrots, hudcam_mat);
    ro_single_render(&self->L.butterfly_icon, hudcam_mat);
    ro_text_render(&self->L.butterfly_cnt, hudcam_mat);
}
