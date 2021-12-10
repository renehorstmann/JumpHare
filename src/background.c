#include "r/render.h"
#include "r/ro_single.h"
#include "r/ro_batch.h"
#include "r/texture.h"
#include "u/pose.h"
#include "u/image.h"
#include "mathc/float.h"
#include "background.h"

#define PIXEL_SIZE 2.0



//
// public
//

Background *background_new(float level_width, float level_height, 
        bool repeat_h, bool repeat_v,
        rRender *render,
        const char *file) {
    Background *self = rhc_calloc(sizeof *self);
            
    uImage img = u_image_new_file(CAMERA_BACKGROUNDS, file);

    // top left pixel will be clear color
    r_render_clear_color(render)->rgb = vec3_cast_from_uchar_1(u_image_pixel(img, 0, 0, 0)->v);

    float rows = img.rows * PIXEL_SIZE;
    float cols = img.cols * PIXEL_SIZE;

    int size_h = repeat_h? ceilf(level_width / cols) : 1;
    int size_v = repeat_v? ceilf(level_height / rows) : 1;

    for (int i = 0; i < CAMERA_BACKGROUNDS; i++) {
        rTexture tex = r_texture_new(img.cols, img.rows, 1, 1, u_image_layer(img, i));
        self->L.ro[i] = ro_batch_new(size_h*size_v, tex);

        for (int v = 0; v < size_v; v++) {
            for(int h = 0; h < size_h; h++) {
                self->L.ro[i].rects[v*size_h+h].pose = 
                        u_pose_new_aa(
                        cols * h, rows *(v+1), 
                        cols, rows);
            }
        }
        ro_batch_update(&self->L.ro[i]);
    }

    u_image_kill(&img);
    return self;
}


void background_kill(Background **self_ptr) {
    Background *self = *self_ptr;
    if(!self)
        return;
        
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++) {
        ro_batch_kill(&self->L.ro[i]);
    }
    
    rhc_free(self);
    *self_ptr = NULL;
}


void background_update(Background *self, float dtime) {

}

void background_render(const Background *self, const Camera_s *cam) {
    for (int i = 0; i < CAMERA_BACKGROUNDS; i++)
        ro_batch_render(&self->L.ro[i], &cam->matrices_background[i].vp, false);
}

